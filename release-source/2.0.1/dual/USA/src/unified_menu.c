#include "common.h"
#include "input_remap.h"
#include "unified_menu.h"
#include "unified_menu_layout.h"

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
/* USA Items manager: constructor 46B534, draw 434C5C. Its 37-quad
 * board uses menu_item_parts00, including native Gear/Map/Items lettering.
 * Never overwrite source geometry or allocate beyond the native vertex count.
 */
#define MANAGER ((u8*)0x005066F8)
static float sHeaderPositions[37 * 12] __attribute__((aligned(16)));
static float sHeaderColors[37 * 16] __attribute__((aligned(16)));
static float sHeaderUvs[37 * 8] __attribute__((aligned(16)));
static u32 sHeaderResources[3][0x1b8 / 4] __attribute__((aligned(16)));
static u32 sHeaderDescriptor[0x120 / 4];
static u16 sHeaderIndices[37 * 6];
static void* sHeaderNodes[3];

/* Separate native Save-atlas board. Never mutate the native menu, and keep
 * an immutable resource for each focus state like the tab header. */
static float sFooterPositions[78 * 12] __attribute__((aligned(16)));
static float sFooterUvs[78 * 8] __attribute__((aligned(16)));
static float sFooterColors[78 * 16] __attribute__((aligned(16)));
static u16 sFooterIndices[78 * 6];
static u32 sFooterResources[3][0x1b8 / 4] __attribute__((aligned(16)));
static void* sFooterNodes[3];
static float sFocusPositions[78 * 12] __attribute__((aligned(16)));
static float sFocusUvs[78 * 8] __attribute__((aligned(16)));
static u32 sFocusResources[2][0x1b8 / 4] __attribute__((aligned(16)));
static void* sFocusNodes[2];

static void TintShelf(void* board, u32 expected, u32 first, u32 count, float green, float blue) {
    if (!board || *(u32*)board != expected)
        return;
    float* colors = *(float**)((u8*)board + 0x18);
    if (!colors)
        return;
    for (u32 q = first; q < first + count; ++q)
        for (u32 v = 0; v < 4; ++v) {
            colors[q * 16 + v * 4] = 1.0f;
            colors[q * 16 + v * 4 + 1] = green;
            colors[q * 16 + v * 4 + 2] = blue;
            /* Keep native transition alpha. Never tint icon renderers. */
        }
}

void UnifiedMenu_TintShelves(void) {
    if (!InputRemap_IsItemsMenuOpen())
        return;
    /* Items parts quads0/1: two halves of the blue shelf. Gear parts0..17:
     * stone panels, grooves and empty-slot silhouettes. Actual equipment,
     * heart pieces, numbers and gold cursor use separate draws. */
    TintShelf(*(void**)(MANAGER + 0x24), 37, 0, 2, 1.0f, 1.0f);
    /* Both shelves retain their native palettes. */
}

void UnifiedMenu_InitShelfColors(void* board, const float* alpha, int count, int first) {
    ((void (*)(void*, const float*, int, int))0x002FCDEC)(board, alpha, count, first);
    /* At construction tint only this new board, not other menu lifetimes. */
    TintShelf(board, 37, 0, 2, 1.0f, 1.0f);
}

/* One continuous stone sample, rotated to fit the wide panel. The geometry
 * grid is only for edge shading; UVs never restart or mirror at its joins.
 * Preserve the independent full-screen dim and inset frame bounds. */
void UnifiedMenu_DrawBackdrop(void) {
    if (!InputRemap_IsItemsMenuOpen() ||
        (UnifiedMenu_GetDialogPage() && !UnifiedMenu_IsOptionsDialog()) ||
        UnifiedMenu_IsOpening() || UnifiedMenu_IsClosing())
        return;
    static float positions[78 * 12] __attribute__((aligned(16)));
    static float uvs[78 * 8] __attribute__((aligned(16)));
    static float colors[78 * 16] __attribute__((aligned(16)));
    static u16 indices[78 * 6];
    static u32 resource[0x1b8 / 4] __attribute__((aligned(16)));
    static void* node;
    if (!node) {
        const u16 offsets[6] = {0, 2, 1, 1, 2, 3};
        for (u32 q = 0; q < 78; ++q) {
            for (u32 v = 0; v < 4; ++v) {
                const u32 col = q % 4, row = q / 4;
                const u32 x = col + (v & 1), y = row + (v >= 2);
                const u8 edge = x == 0 || x == 4 || y == 0 || y == 2;
                positions[q * 12 + v * 3] = q < 8 ? 12 + x * 94.0f : -100;
                positions[q * 12 + v * 3 + 1] = q < 8 ? 31.5f + y * 84.25f : -100;
                positions[q * 12 + v * 3 + 2] = 0;
                uvs[q * 8 + v * 2] = (0.5f + 63.0f * y / 2.0f) / 512;
                uvs[q * 8 + v * 2 + 1] = 1 - (8.5f + 231.0f * x / 4.0f) / 256;
                /* Larger-scale mineral variation, cool slate highlights,
                 * and a darker perimeter keep the shelves prominent. */
                const float shade = edge ? 0.36f : 0.56f;
                colors[q * 16 + v * 4] = shade * 0.86f;
                colors[q * 16 + v * 4 + 1] = shade * 0.94f;
                colors[q * 16 + v * 4 + 2] = shade;
                colors[q * 16 + v * 4 + 3] = q < 8 ? (edge ? 0.95f : 0.92f) : 0;
            }
            for (u32 i = 0; i < 6; ++i)
                indices[q * 6 + i] = q * 4 + offsets[i];
        }
        u32 descriptor[0x120 / 4];
        const u32* nativeTemplate = (const u32*)0x004D54D0;
        for (u32 i = 0; i < 0x118 / 4; ++i)
            descriptor[i] = nativeTemplate[i];
        descriptor[0] = (u32)positions;
        descriptor[1] = (u32)uvs;
        descriptor[2] = (u32)colors;
        descriptor[4] = (u32)indices;
        ((void* (*)(void*, void*))0x00348F34)(resource, descriptor);
        void* texture = ((void* (*)(u32))0x002E11D0)(6);
        ((void (*)(void*, u32, void*, u32, u32, u32, u32))0x00348A64)(resource, 0, texture, 0x2601,
                                                                      0x2601, 0x812f, 0x812f);
        node =
            ((void* (*)(void*, void*, void*, u32))0x0034897C)(*(void**)0x005C0A34, resource, 0, 0);
        if (node)
            *(u32*)((u8*)node + 0x178) |= 2;
    }
    if (node) {
        ((void (*)(unsigned, int, unsigned, unsigned))0x002FEABC)(0, 0, 480, 400);
        ((void (*)(void*))(*(u32*)(*(u32*)node + 12)))(node);
    }
}

/* Subtle portrait shadow: transparent outer vertices, 18%-opaque interior.
 * The existing solid stone texel provides only texture alpha;
 * RGB is multiplied by zero. Separate storage from all menu boards. */
void UnifiedMenu_DrawDollBackdrop(void) {
    if (!InputRemap_IsGearPageActive() || UnifiedMenu_IsOpening() || UnifiedMenu_IsClosing() ||
        UnifiedMenu_GetDialogPage())
        return;
    static float positions[78 * 12] __attribute__((aligned(16)));
    static float uvs[78 * 8] __attribute__((aligned(16)));
    static float colors[78 * 16] __attribute__((aligned(16)));
    static u16 indices[78 * 6];
    static u32 resource[0x1b8 / 4] __attribute__((aligned(16)));
    static void* node;
    if (!node) {
        const float xs[4] = {240, 278, 332, 370};
        const float ys[4] = {28, 52, 144, 176};
        const u16 offsets[6] = {0, 2, 1, 1, 2, 3};
        for (u32 q = 0; q < 78; ++q) {
            for (u32 v = 0; v < 4; ++v) {
                u32 x = q % 3 + (v & 1), y = q / 3 + (v >= 2);
                /* This board draws in the top-screen 400-unit projection,
                 * not the secondary UI's 320-unit projection. */
                positions[q * 12 + v * 3] = q < 9 ? xs[x] + MENU_GEAR_PORTRAIT_SHIFT : -100;
                positions[q * 12 + v * 3 + 1] = q < 9 ? ys[y] : -100;
                positions[q * 12 + v * 3 + 2] = 0;
                uvs[q * 8 + v * 2] = 160.5f / 256;
                uvs[q * 8 + v * 2 + 1] = 1 - 142.5f / 256;
                colors[q * 16 + v * 4 + 3] =
                    (q < 9 && x > 0 && x < 3 && y > 0 && y < 3) ? 0.18f : 0;
            }
            for (u32 i = 0; i < 6; ++i)
                indices[q * 6 + i] = q * 4 + offsets[i];
        }
        u32 descriptor[0x120 / 4];
        const u32* nativeTemplate = (const u32*)0x004D54D0;
        for (u32 i = 0; i < 0x118 / 4; ++i)
            descriptor[i] = nativeTemplate[i];
        descriptor[0] = (u32)positions;
        descriptor[1] = (u32)uvs;
        descriptor[2] = (u32)colors;
        descriptor[4] = (u32)indices;
        ((void* (*)(void*, void*))0x00348F34)(resource, descriptor);
        void* texture = ((void* (*)(u32))0x002E11D0)(3);
        ((void (*)(void*, u32, void*, u32, u32, u32, u32))0x00348A64)(resource, 0, texture, 0x2601,
                                                                      0x2601, 0x812f, 0x812f);
        node =
            ((void* (*)(void*, void*, void*, u32))0x0034897C)(*(void**)0x005C0A34, resource, 0, 0);
        if (node)
            *(u32*)((u8*)node + 0x178) |= 2;
    }
    if (node) {
        ((void (*)(unsigned, int, unsigned, unsigned))0x002FEABC)(0, 0, 480, 400);
        ((void (*)(void*))(*(u32*)(*(u32*)node + 12)))(node);
    }
}

static void DrawFooterBrackets(u32 focus) {
    if (!focus || focus > 2)
        return;
    const u32 index = focus - 1;
    if (!sFocusNodes[index]) {
        /* menu_cursor00's first 32x32 tile is the native gold cursor.
         * Keep its four corners square rather than stretching the artwork. */
        for (u32 q = 0; q < 78; ++q) {
            for (u32 v = 0; v < 4; ++v) {
                const float x = (focus == 1 ? 44 : 284) + (q & 1 ? 65 : 0);
                const float y = 208 + (q & 2 ? 15 : 0);
                sFocusPositions[q * 12 + v * 3] = q < 4 ? (x + (v & 1 ? 7 : 0)) * 0.8f : -100;
                sFocusPositions[q * 12 + v * 3 + 1] = q < 4 ? y + (v >= 2 ? 7 : 0) : -100;
                sFocusPositions[q * 12 + v * 3 + 2] = 0;
                sFocusUvs[q * 8 + v * 2] = ((q & 1 ? 16 : 0) + (v & 1 ? 15.5f : 0.5f)) / 128;
                sFocusUvs[q * 8 + v * 2 + 1] =
                    1 - ((q & 2 ? 16 : 0) + (v >= 2 ? 15.5f : 0.5f)) / 128;
            }
        }
        u32 descriptor[0x120 / 4];
        const u32* nativeTemplate = (const u32*)0x004D54D0;
        for (u32 i = 0; i < 0x118 / 4; ++i)
            descriptor[i] = nativeTemplate[i];
        descriptor[0] = (u32)sFocusPositions;
        descriptor[1] = (u32)sFocusUvs;
        descriptor[2] = (u32)sFooterColors;
        descriptor[4] = (u32)sFooterIndices;
        void* resource = sFocusResources[index];
        ((void* (*)(void*, void*))0x00348F34)(resource, descriptor);
        void* texture = ((void* (*)(u32))0x002E11D0)(13);
        ((void (*)(void*, u32, void*, u32, u32, u32, u32))0x00348A64)(resource, 0, texture, 0x2601,
                                                                      0x2601, 0x812f, 0x812f);
        sFocusNodes[index] =
            ((void* (*)(void*, void*, void*, u32))0x0034897C)(*(void**)0x005C0A34, resource, 0, 0);
        if (sFocusNodes[index])
            *(u32*)((u8*)sFocusNodes[index] + 0x178) |= 2;
    }
    void* node = sFocusNodes[index];
    if (node)
        ((void (*)(void*))(*(u32*)(*(u32*)node + 12)))(node);
}

static void DrawFooter(void) {
    const u32 focus = UnifiedMenu_GetFooterFocus();
    if (focus > 2)
        return;
    if (!sFooterNodes[focus]) {
        u8* board = *(u8**)0x0050A510;
        if (!board || *(u32*)board != 78)
            return;
        const float* uv = *(float**)(board + 0x14);
        if (!uv)
            return;
        /* Constructor's immutable XY and size tables avoid capturing a
         * native slide animation or a hidden Save panel's zeroed geometry. */
        const float* xy = (const float*)0x0050A54C;
        const float* size = (const float*)0x0050A7BC;
        for (u32 q = 0; q < 78; ++q) {
            const bool save = q >= 6 && q <= 9;
            const bool options = q >= 18 && q <= 21;
            for (u32 v = 0; v < 4; ++v) {
                const float px = xy[q * 2] + (v & 1 ? size[q * 2] : 0);
                const float py = xy[q * 2 + 1] + (v >= 2 ? size[q * 2 + 1] : 0);
                sFooterPositions[q * 12 + v * 3] = save      ? (45 + (px - 162) * 70 / 124) * 0.8f
                                                   : options ? (285 + (px - 110) * 70 / 100) * 0.8f
                                                             : -100;
                sFooterPositions[q * 12 + v * 3 + 1] = save      ? 209 + (py - 94) * 20 / 52
                                                       : options ? 209 + (py - 202) * 20 / 36
                                                                 : -100;
                sFooterPositions[q * 12 + v * 3 + 2] = 0;
                for (u32 c = 0; c < 4; ++c)
                    sFooterColors[q * 16 + v * 4 + c] = 1.0f;
            }
            for (u32 i = 0; i < 8; ++i)
                sFooterUvs[q * 8 + i] = uv[q * 8 + i];
            const u16 offsets[6] = {0, 2, 1, 1, 2, 3};
            for (u32 i = 0; i < 6; ++i)
                sFooterIndices[q * 6 + i] = q * 4 + offsets[i];
        }
        u32 descriptor[0x120 / 4];
        const u32* nativeTemplate = (const u32*)0x004D54D0;
        for (u32 i = 0; i < 0x118 / 4; ++i)
            descriptor[i] = nativeTemplate[i];
        descriptor[0] = (u32)sFooterPositions;
        descriptor[1] = (u32)sFooterUvs;
        descriptor[2] = (u32)sFooterColors;
        descriptor[4] = (u32)sFooterIndices;
        void* resource = sFooterResources[focus];
        ((void* (*)(void*, void*))0x00348F34)(resource, descriptor);
        void* texture = ((void* (*)(u32))0x002E11D0)(12);
        ((void (*)(void*, u32, void*, u32, u32, u32, u32))0x00348A64)(resource, 0, texture, 0x2601,
                                                                      0x2601, 0x812f, 0x812f);
        sFooterNodes[focus] =
            ((void* (*)(void*, void*, void*, u32))0x0034897C)(*(void**)0x005C0A34, resource, 0, 0);
        if (sFooterNodes[focus])
            *(u32*)((u8*)sFooterNodes[focus] + 0x178) |= 2;
    }
    void* node = sFooterNodes[focus];
    if (node)
        ((void (*)(void*))(*(u32*)(*(u32*)node + 12)))(node);
    DrawFooterBrackets(focus);
}

static float* GetPositions(void) {
    void* board = *(void**)(MANAGER + 0x24);
    if (!board || *(u32*)board != 37)
        return 0;
    /* The native constructor uses +0x0c. +0x10 is a separate optional
     * translated stream and is not populated by this static Items board. */
    return *(float**)((u8*)board + 0x0c);
}

void UnifiedMenu_RestoreItemsBoard(void) {
    /* Header has its own GPU resource; the native shelf is never modified. */
}

static void* CreateHeader(u32 active) {
    void* resource = sHeaderResources[active];
    u8* board = *(u8**)(MANAGER + 0x24);
    const u32* nativeTemplate = (const u32*)0x004D4984;
    for (u32 i = 0; i < 0x118 / 4; ++i)
        sHeaderDescriptor[i] = nativeTemplate[i];
    sHeaderDescriptor[0] = (u32)sHeaderPositions;
    const float* nativeUvs = *(float**)(board + 0x14);
    for (u32 i = 0; i < 37 * 8; ++i)
        sHeaderUvs[i] = nativeUvs[i];
    /* Map's artwork begins at atlas row 222, directly after Gear's last
     * row. Linear filtering across that boundary leaks a bright line into
     * the transparent top corners, most visible on the inactive tab.
     * Sample half a source texel inside the top edge; keep geometry and
     * the separate three-slice frame unchanged. */
    for (u32 v = 0; v < 2; ++v)
        sHeaderUvs[10 * 8 + v * 2 + 1] -= 0.5f / 256.0f;
    /* Verified in the decoded 256x256 menu_item_parts00 atlas: the separate
     * stone tile, not native quad 35. Use texel centers to avoid edge bleed.
     * Native 2FC40C uses U=x/width, V=1-y/height; top vertices come first. */
    const u32 rails[4] = {0, 6, 1, 2};
    for (u32 r = 0; r < 4; ++r) {
        const u32 q = rails[r];
        for (u32 v = 0; v < 4; ++v) {
            sHeaderUvs[q * 8 + v * 2] =
                (r < 2 ? (v & 1 ? 178.5f : 150.5f) : (v >= 2 ? 178.5f : 150.5f)) / 256.0f;
            sHeaderUvs[q * 8 + v * 2 + 1] =
                1.0f - (r < 2 ? (v < 2 ? 140.5f : 144.5f) : (v & 1 ? 144.5f : 140.5f)) / 256.0f;
        }
    }
    sHeaderDescriptor[1] = (u32)sHeaderUvs;
    /* Native page-transition tint must not be captured permanently by an
     * immutable header. Own the RGBA stream as well as the geometry. */
    for (u32 i = 0; i < 37 * 16; ++i)
        sHeaderColors[i] = 1.0f;
    /* Quiet stone bevels: emphasize content, not full-width bright stripes. */
    for (u32 r = 0; r < 4; ++r)
        for (u32 v = 0; v < 4; ++v) {
            const u32 i = rails[r] * 16 + v * 4;
            const float shade = (r < 2 ? v < 2 : !(v & 1)) ? 0.82f : 0.56f;
            sHeaderColors[i] = shade;
            sHeaderColors[i + 1] = shade;
            sHeaderColors[i + 2] = shade;
            sHeaderColors[i + 3] = 0.88f;
        }
    sHeaderDescriptor[2] = (u32)sHeaderColors;
    sHeaderDescriptor[4] = (u32)sHeaderIndices;
    for (u32 q = 0; q < 37; ++q) {
        const u16 offsets[6] = {0, 2, 1, 1, 2, 3};
        for (u32 i = 0; i < 6; ++i)
            sHeaderIndices[q * 6 + i] = q * 4 + offsets[i];
    }
    ((void* (*)(void*, void*))0x00348F34)(resource, sHeaderDescriptor);
    void* texture = ((void* (*)(u32))0x002E11D0)(3);
    ((void (*)(void*, u32, void*, u32, u32, u32, u32))0x00348A64)(resource, 0, texture, 0x2601,
                                                                  0x2601, 0x812f, 0x812f);
    void* node =
        ((void* (*)(void*, void*, void*, u32))0x0034897C)(*(void**)0x005C0A34, resource, 0, 0);
    if (node)
        *(u32*)((u8*)node + 0x178) |= 2;
    return node;
}

/* Separate immutable board so the Map frame never leaks into Save/Options
 * or other tabs through the cached header resources. All edges sit outside
 * the native map clip; labels and cursor remain untouched. */
/* Complete the native shelf bevels without changing selection geometry.
 * Gear's narrow joins use its own green atlas, not the shared backdrop. */
static void DrawShelfFrame(void) {
    if (!InputRemap_IsItemsMenuOpen() || UnifiedMenu_GetDialogPage() || UnifiedMenu_IsOpening() ||
        UnifiedMenu_IsClosing())
        return;
    const u32 gear = InputRemap_IsGearPageActive() ? 1 : 0;
    if (!gear && !InputRemap_IsItemsPageActive())
        return;
    static float pos[2][37 * 12], uv[2][37 * 8], color[2][37 * 16];
    static u16 indices[37 * 6];
    static u32 resources[2][0x1b8 / 4] __attribute__((aligned(16)));
    static void* nodes[2];
    if (!nodes[gear]) {
        /* Gear spans x20..236 and y48..180. Two green bridges close
         * the column/row gaps; the four outer bevels enclose the whole. */
        const float rect[6][4] = {{126, 48, 4, 132},       {20, 148.5f, 216, 4},
                                  {20, 48, 216, 2.5f},     {20, 48, 2.5f, 132},
                                  {233.5f, 48, 2.5f, 132}, {20, 177.5f, 216, 2.5f}};
        const u16 order[6] = {0, 2, 1, 1, 2, 3};
        for (u32 q = 0; q < 37; ++q) {
            const u8 visible = gear ? q < 10 : q == 0;
            for (u32 v = 0; v < 4; ++v) {
                float x = -100, y = -100;
                if (visible) {
                    if (gear) {
                        if (q < 2) {
                            x = rect[q][0] + ((v & 1) ? rect[q][2] : 0);
                            y = rect[q][1] + (v >= 2 ? rect[q][3] : 0);
                        } else {
                            /* Four trapezoids share exact corner diagonals.
                             * A fine inner lip separates bevel and shelf. */
                            const u32 side = (q - 2) % 4;
                            const float inset = q >= 6 ? 2.5f : 0;
                            const float width = q >= 6 ? 0.4f : 2.5f;
                            const float l = 20 + inset, r = 236 - inset;
                            /* Keep the complete top bevel and inner lip
                             * above the native sword equipped-highlight.
                             * Its upper edge extends above the icon slot. */
                            const float t = 45.5f + inset, b = 180 - inset;
                            if (side == 0 || side == 3) {
                                const u8 inner = side == 0 ? v >= 2 : v < 2;
                                x = (v & 1) ? r : l;
                                if (inner)
                                    x += (v & 1) ? -width : width;
                                y = side == 0 ? t + (v >= 2 ? width : 0) : b - (v < 2 ? width : 0);
                            } else {
                                const u8 inner = side == 1 ? (v & 1) : !(v & 1);
                                y = v >= 2 ? b : t;
                                if (inner)
                                    y += v >= 2 ? -width : width;
                                x = side == 1 ? l + ((v & 1) ? width : 0)
                                              : r - (!(v & 1) ? width : 0);
                            }
                        }
                    } else {
                        /* Match the native Items side rails with mitered
                         * corners; extend below the cropped flat edge. */
                        x = (v & 1) ? 213 : 27;
                        if (v < 2)
                            x += (v & 1) ? -2.5f : 2.5f;
                        y = v < 2 ? 180 : 182.5f;
                    }
                }
                pos[gear][q * 12 + v * 3] = x * 0.8f;
                pos[gear][q * 12 + v * 3 + 1] = y;
                /* Sample plain shelf stone, leaving icons/slots untouched. */
                uv[gear][q * 8 + v * 2] =
                    (gear ? (v & 1 ? 100.5f : 80.5f) / 512 : (v & 1 ? 90.5f : 70.5f) / 256);
                uv[gear][q * 8 + v * 2 + 1] =
                    1 - (gear ? (q >= 2 ? 0.5f : (v >= 2 ? 50.5f : 30.5f)) : 3.5f) / 256;
                if (gear) {
                    /* No long, stretched atlas rows on the trim. Uniform
                     * texel color plus vertex shading produces clean bevels.
                     * Bridge samples follow their physical aspect ratios. */
                    const float u = q == 0   ? ((v & 1) ? 88.5f : 82.5f)
                                    : q == 1 ? ((v & 1) ? 154.5f : 4.5f)
                                             : 84.5f;
                    const float ty = q == 0   ? (v >= 2 ? 140.5f : 4.5f)
                                     : q == 1 ? (v >= 2 ? 74.5f : 70.5f)
                                              : 0.5f;
                    uv[gear][q * 8 + v * 2] = u / 512;
                    uv[gear][q * 8 + v * 2 + 1] = 1 - ty / 256;
                }
                float shade = 1;
                if (!gear)
                    shade = v < 2 ? 1.0f : 0.62f;
                else if (q == 2)
                    shade = v < 2 ? 1.0f : 0.65f;
                else if (q == 3)
                    shade = (v & 1) ? 0.60f : 0.90f;
                else if (q == 4)
                    shade = (v & 1) ? 0.35f : 0.65f;
                else if (q == 5)
                    shade = v < 2 ? 0.70f : 0.35f;
                else if (q >= 6)
                    shade = 0.28f;
                for (u32 c = 0; c < 3; ++c)
                    color[gear][q * 16 + v * 4 + c] = shade;
                color[gear][q * 16 + v * 4 + 3] = visible ? 1 : 0;
            }
            for (u32 i = 0; i < 6; ++i)
                indices[q * 6 + i] = q * 4 + order[i];
        }
        u32 descriptor[0x120 / 4];
        const u32* nativeTemplate = (const u32*)0x004D4984;
        for (u32 i = 0; i < 0x118 / 4; ++i)
            descriptor[i] = nativeTemplate[i];
        descriptor[0] = (u32)pos[gear];
        descriptor[1] = (u32)uv[gear];
        descriptor[2] = (u32)color[gear];
        descriptor[4] = (u32)indices;
        ((void* (*)(void*, void*))0x00348F34)(resources[gear], descriptor);
        /* USA texture-name table 4D4010: Items slot3, Gear slot4. */
        void* texture = ((void* (*)(u32))0x002E11D0)(gear ? 4 : 3);
        ((void (*)(void*, u32, void*, u32, u32, u32, u32))0x00348A64)(
            resources[gear], 0, texture, 0x2601, 0x2601, 0x812f, 0x812f);
        nodes[gear] = ((void* (*)(void*, void*, void*, u32))0x0034897C)(*(void**)0x005C0A34,
                                                                        resources[gear], 0, 0);
        if (nodes[gear])
            *(u32*)((u8*)nodes[gear] + 0x178) |= 2;
    }
    void* node = nodes[gear];
    if (node)
        ((void (*)(void*))(*(u32*)(*(u32*)node + 12)))(node);
}

static void DrawMapFrame(void) {
    if (!InputRemap_IsItemsMenuOpen() || InputRemap_IsItemsPageActive() ||
        InputRemap_IsGearPageActive() || UnifiedMenu_GetDialogPage() || UnifiedMenu_IsOpening() ||
        UnifiedMenu_IsClosing())
        return;
    static float positions[2][37 * 12], uvs[2][37 * 8], colors[2][37 * 16];
    static u16 indices[37 * 6];
    static u32 resources[2][0x1b8 / 4] __attribute__((aligned(16)));
    static void* nodes[2];
    const u32 dungeon = ((int (*)(void))0x002F1268)() ? 1 : 0;
    float* pos = positions[dungeon];
    float* uv = uvs[dungeon];
    float* color = colors[dungeon];
    void* node = nodes[dungeon];
    void* resource = resources[dungeon];
    if (!node) {
        /* Four stone edges, followed by a one-unit dark inner recess. */
        const float rect[8][4] = {{9, 41, 222, 2},   {9, 43, 2, 140},  {229, 43, 2, 140},
                                  {9, 183, 222, 2},  {11, 43, 218, 1}, {11, 44, 1, 138},
                                  {228, 44, 1, 138}, {11, 182, 218, 1}};
        const u16 order[6] = {0, 2, 1, 1, 2, 3};
        for (u32 q = 0; q < 37; ++q) {
            for (u32 v = 0; v < 4; ++v) {
                pos[q * 12 + v * 3] = q < 8 ? (rect[q][0] + (v & 1 ? rect[q][2] : 0)) * 0.8f : -100;
                pos[q * 12 + v * 3 + 1] = q < 8 ? rect[q][1] + (v >= 2 ? rect[q][3] : 0) : -100;
                if (q < 8 && dungeon) {
                    /* Map content moves inward 17 units per side and gains
                     * four units of visible height; keep border thickness. */
                    float x = pos[q * 12 + v * 3] / 0.8f;
                    pos[q * 12 + v * 3] = (x < 120 ? x + 17 : x - 17) * 0.8f;
                    if (pos[q * 12 + v * 3 + 1] >= 182)
                        pos[q * 12 + v * 3 + 1] += 4;
                }
                if (q < 8 && !dungeon)
                    pos[q * 12 + v * 3] += (gMenuWideSurface.left - 12) * 0.8f;
                uv[q * 8 + v * 2] = (v & 1 ? 178.5f : 150.5f) / 256;
                uv[q * 8 + v * 2 + 1] = 1 - (v >= 2 ? 144.5f : 140.5f) / 256;
                const float shade = q >= 4   ? 0.08f
                                    : q == 0 ? (v < 2 ? 0.95f : 0.64f)
                                    : q == 1 ? (v & 1 ? 0.64f : 0.88f)
                                    : q == 2 ? (v & 1 ? 0.42f : 0.70f)
                                             : (v < 2 ? 0.68f : 0.42f);
                for (u32 c = 0; c < 3; ++c)
                    color[q * 16 + v * 4 + c] = shade;
                color[q * 16 + v * 4 + 3] = q < 8 ? 1 : 0;
            }
            for (u32 i = 0; i < 6; ++i)
                indices[q * 6 + i] = q * 4 + order[i];
        }
        u32 descriptor[0x120 / 4];
        const u32* nativeTemplate = (const u32*)0x004D4984;
        for (u32 i = 0; i < 0x118 / 4; ++i)
            descriptor[i] = nativeTemplate[i];
        descriptor[0] = (u32)pos;
        descriptor[1] = (u32)uv;
        descriptor[2] = (u32)color;
        descriptor[4] = (u32)indices;
        ((void* (*)(void*, void*))0x00348F34)(resource, descriptor);
        void* texture = ((void* (*)(u32))0x002E11D0)(3);
        ((void (*)(void*, u32, void*, u32, u32, u32, u32))0x00348A64)(resource, 0, texture, 0x2601,
                                                                      0x2601, 0x812f, 0x812f);
        node =
            ((void* (*)(void*, void*, void*, u32))0x0034897C)(*(void**)0x005C0A34, resource, 0, 0);
        if (node)
            *(u32*)((u8*)node + 0x178) |= 2;
        nodes[dungeon] = node;
    }
    if (node)
        ((void (*)(void*))(*(u32*)(*(u32*)node + 12)))(node);
}

void UnifiedMenu_PrepareDungeonMaterials(void) {
    if (!InputRemap_IsItemsMenuOpen() || InputRemap_IsItemsPageActive() ||
        InputRemap_IsGearPageActive() || !((int (*)(void))0x002F1268)())
        return;
    /* Native dungeon board: 50 quads. Only 17/18 form the key-counter
     * backing; 19 and the independent number renderer carry its contents.
     * Sample the underlying stone at matching coordinates, keeping the
     * original atlas identity (including emulator HD replacements). */
    void* board = *(void**)0x00506CC4;
    if (!board || *(u32*)board != 50)
        return;
    float* uv = *(float**)((u8*)board + 0x14);
    if (!uv)
        return;
    for (u32 q = 17; q <= 18; ++q)
        for (u32 v = 0; v < 4; ++v) {
            const float x = q == 17 ? (v & 1 ? 48 : 0) : (v & 1 ? 56 : 48);
            uv[q * 8 + v * 2] = x / 512;
            uv[q * 8 + v * 2 + 1] = 1 - (v >= 2 ? 194.0f : 162.0f) / 256;
        }
}

void UnifiedMenu_DrawDungeonFooterMask(void) {
    if (!InputRemap_IsItemsMenuOpen() || InputRemap_IsItemsPageActive() ||
        InputRemap_IsGearPageActive() || UnifiedMenu_GetDialogPage() || UnifiedMenu_IsOpening() ||
        UnifiedMenu_IsClosing() || !((int (*)(void))0x002F1268)())
        return;
    static float pos[37 * 12], uv[37 * 8], color[37 * 16];
    static u16 indices[37 * 6];
    static u32 resource[0x1b8 / 4] __attribute__((aligned(16)));
    static void* node;
    if (!node) {
        const u16 order[6] = {0, 2, 1, 1, 2, 3};
        for (u32 q = 0; q < 37; ++q) {
            for (u32 v = 0; v < 4; ++v) {
                /* Continue the native materials through the old navigation
                 * lane. Boundaries follow native x=0,64,256 in the dungeon
                 * viewport; floor selectors start at x=256. Mirror the last
                 * parchment rows at the join so its grain remains continuous. */
                const float left = q == 0 ? 29.0f : 65.4f;
                const float width = q == 0 ? 36.4f : 109.2f;
                pos[q * 12 + v * 3] = q < 2 ? (left + (v & 1 ? width : 0)) * 0.8f : -100;
                const float top = 161.0f;
                pos[q * 12 + v * 3 + 1] = q < 2 ? top + (v >= 2 ? 186 - top : 0) : -100;
                pos[q * 12 + v * 3 + 2] = 0;
                uv[q * 8 + v * 2] =
                    (q == 0 ? (v & 1 ? 63.5f : 0.5f) : (v & 1 ? 463.5f : 256.5f)) / 512;
                uv[q * 8 + v * 2 + 1] =
                    1 - (q == 0 ? (v >= 2 ? 239.5f : 198.5f) : (v >= 2 ? 131.5f : 168.5f)) / 256;
                color[q * 16 + v * 4] = 1;
                color[q * 16 + v * 4 + 1] = 1;
                color[q * 16 + v * 4 + 2] = 1;
                color[q * 16 + v * 4 + 3] = q < 2 ? 1.0f : 0;
            }
            for (u32 i = 0; i < 6; ++i)
                indices[q * 6 + i] = q * 4 + order[i];
        }
        u32 descriptor[0x120 / 4];
        const u32* nativeTemplate = (const u32*)0x004D4984;
        for (u32 i = 0; i < 0x118 / 4; ++i)
            descriptor[i] = nativeTemplate[i];
        descriptor[0] = (u32)pos;
        descriptor[1] = (u32)uv;
        descriptor[2] = (u32)color;
        descriptor[4] = (u32)indices;
        ((void* (*)(void*, void*))0x00348F34)(resource, descriptor);
        void* texture = ((void* (*)(u32))0x002E11D0)(6);
        ((void (*)(void*, u32, void*, u32, u32, u32, u32))0x00348A64)(resource, 0, texture, 0x2601,
                                                                      0x2601, 0x812f, 0x812f);
        node =
            ((void* (*)(void*, void*, void*, u32))0x0034897C)(*(void**)0x005C0A34, resource, 0, 0);
        if (node)
            *(u32*)((u8*)node + 0x178) |= 2;
    }
    if (node) {
        ((void (*)(unsigned, int, unsigned, unsigned))0x002FEABC)(0, 0, 480, 400);
        ((void (*)(void*))(*(u32*)(*(u32*)node + 12)))(node);
    }
}

static void PlaceQuad(float* source, u32 q, float sx, float sy, float sw, float sh, float x,
                      float y, float w, float h) {
    for (u32 v = 0; v < 4; ++v) {
        u32 i = q * 12 + v * 3;
        /* Secondary projection is 320 units across; full top viewport is 400. */
        sHeaderPositions[i] = (x + (source[i] - sx) * w / sw) * 0.8f;
        sHeaderPositions[i + 1] = y + (source[i + 1] - sy) * h / sh;
        sHeaderPositions[i + 2] = source[i + 2];
    }
}

void UnifiedMenu_DrawHeader(void) {
    const unsigned dialogPage = UnifiedMenu_GetDialogPage();
    /* Save and Options are standalone dialogs, not inventory pages.
     * Neither inherits the pause tabs, perimeter rails or footer. */
    if (InputRemap_IsSaveMenuOpen() || UnifiedMenu_IsOptionsDialog())
        return;
    if (!InputRemap_IsItemsMenuOpen() && !dialogPage)
        return;
    float* p = GetPositions();
    if (!p)
        return;
    for (u32 q = 0; q < 37; ++q)
        for (u32 v = 0; v < 4; ++v) {
            sHeaderPositions[q * 12 + v * 3] = -100;
            sHeaderPositions[q * 12 + v * 3 + 1] = -100;
            sHeaderPositions[q * 12 + v * 3 + 2] = 0;
        }
    const u32 active = dialogPage                       ? dialogPage - 1
                       : InputRemap_IsItemsPageActive() ? 0
                       : InputRemap_IsGearPageActive()  ? 1
                                                        : 2;
    /* Shared native stone rails join the tabs and reserve a common footer.
     * Keep clear of the content clip and the rotating item description. */
    for (u32 r = 0; r < 2; ++r) {
        const u32 q = r ? 0 : 6;
        for (u32 v = 0; v < 4; ++v) {
            const u32 to = q * 12 + v * 3;
            sHeaderPositions[to] = (v & 1 ? 388.0f : 12.0f) * 0.8f;
            sHeaderPositions[to + 1] = (r ? 200.0f : 30.0f) + (v < 2 ? 0 : 1.5f);
            sHeaderPositions[to + 2] = 0;
        }
    }
    /* Side rails close only the main content frame, not tabs or footer. */
    if (!dialogPage)
        for (u32 side = 0; side < 2; ++side)
            for (u32 v = 0; v < 4; ++v) {
                const u32 i = (side + 1) * 12 + v * 3;
                sHeaderPositions[i] = ((side ? 386.5f : 12.0f) + (v & 1 ? 1.5f : 0)) * 0.8f;
                sHeaderPositions[i + 1] = v >= 2 ? 200 : 31.5f;
                sHeaderPositions[i + 2] = 0;
            }
    /* Reorder native tabs to Items, Gear, Map. Active tab is slightly taller. */
    const u32 labels[3] = {11, 9, 10};
    const u32 frames[3] = {34, 28, 31};
    const float sx[3] = {182, 66, 124};
    const float sy[3] = {198, 206, 206};
    const float sw[3] = {72, 54, 54};
    const float sh[3] = {42, 34, 34};
    for (u32 t = 0; t < 3; ++t) {
        const float y = t == active ? 9 : 12;
        const float h = t == active ? 21 : 18;
        const float x = 12 + t * 56;
        PlaceQuad(p, labels[t], sx[t], sy[t], sw[t], sh[t], x, y, 50, h);
        for (u32 q = frames[t]; q < frames[t] + 3; ++q)
            PlaceQuad(p, q, sx[t], sy[t], sw[t], sh[t], x, y, 50, h);
    }
    /* Back quads remain hidden. Physical B retains its native navigation;
     * the footer presents only symmetric Save and Options actions. */
    /* These nodes are drawn directly, outside the engine's buffer-advance
     * update list. Keep one immutable GPU board per active tab instead of
     * uploading into a next-frame buffer that is never selected. */
    if (!sHeaderNodes[active])
        sHeaderNodes[active] = CreateHeader(active);
    void* node = sHeaderNodes[active];
    if (!node)
        return;
    ((void (*)(unsigned, int, unsigned, unsigned))0x002FEABC)(0, 0, 480, 400);
    void (*draw)(void*) = (void (*)(void*))(*(u32*)(*(u32*)node + 12));
    draw(node);
    if (!dialogPage)
        DrawFooter();
    DrawShelfFrame();
    DrawMapFrame();
    /* Keep the native dungeon key icon/count; no duplicate overlay. */
}
#else
void UnifiedMenu_TintShelves(void) {}
void UnifiedMenu_DrawBackdrop(void) {}
void UnifiedMenu_DrawDollBackdrop(void) {}
void UnifiedMenu_RestoreItemsBoard(void) {}
void UnifiedMenu_DrawDungeonFooterMask(void) {}
void UnifiedMenu_PrepareDungeonMaterials(void) {}
void UnifiedMenu_DrawHeader(void) {}
#endif
