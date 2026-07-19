#include "native_hud.h"

#include "common.h"
#include "z3D/z3Ditem.h"

// The native motion-control board is converted into a multi-quad triangle
// strip. These buffers stay resident for the lifetime of the game because the
// board's descriptor retains their addresses while uploading GPU data.
#ifdef PLUS_MINIMAL_HUD
#define NATIVE_HUD_QUAD_COUNT 13
#elif defined(PLUS_HEARTS_ONLY)
#define NATIVE_HUD_QUAD_COUNT 33
#else
#define NATIVE_HUD_QUAD_COUNT 106
#endif
#define NATIVE_HUD_VERTEX_COUNT (NATIVE_HUD_QUAD_COUNT * 4)
#define NATIVE_HUD_INDEX_COUNT (4 + (NATIVE_HUD_QUAD_COUNT - 1) * 6)

// Vertex order per quad: lower-left, lower-right, upper-left, upper-right.
// The face-button size is intentionally smaller than the initial 40x40 proof
// and follows the visual scale of Project Restoration's widescreen HUD.
float gNativeHudPositions[NATIVE_HUD_VERTEX_COUNT * 3]
    __attribute__((aligned(16), used)) = {
    // Native bottom-right rupee. This old A-placeholder quad is drawn early,
    // but the rupee occupies an unrelated screen area and needs no overlap.
    345.0f, 227.0f, 0.0f, 355.0f, 227.0f, 0.0f,
    345.0f, 215.0f, 0.0f, 355.0f, 215.0f, 0.0f,
    // B base (bottom)
    357.0f, 62.0f, 0.0f, 373.0f, 62.0f, 0.0f,
    357.0f, 46.0f, 0.0f, 373.0f, 46.0f, 0.0f,
    // X base (top)
    357.0f, 28.0f, 0.0f, 373.0f, 28.0f, 0.0f,
    357.0f, 12.0f, 0.0f, 373.0f, 12.0f, 0.0f,
    // Y base (left)
    340.0f, 45.0f, 0.0f, 356.0f, 45.0f, 0.0f,
    340.0f, 29.0f, 0.0f, 356.0f, 29.0f, 0.0f,
    // D-pad cross (upper-left, beneath the vitals)
    30.0f, 89.0f, 0.0f, 50.0f, 89.0f, 0.0f,
    30.0f, 68.0f, 0.0f, 50.0f, 68.0f, 0.0f,
    // Live B item, centered inside the bottom face-button base.
    359.0f, 60.0f, 0.0f, 371.0f, 60.0f, 0.0f,
    359.0f, 48.0f, 0.0f, 371.0f, 48.0f, 0.0f,
    // Live X item, centered inside the top face-button base.
    359.0f, 26.0f, 0.0f, 371.0f, 26.0f, 0.0f,
    359.0f, 14.0f, 0.0f, 371.0f, 14.0f, 0.0f,
    // Live Y item, centered inside the left face-button base.
    342.0f, 43.0f, 0.0f, 354.0f, 43.0f, 0.0f,
    342.0f, 31.0f, 0.0f, 354.0f, 31.0f, 0.0f,
    // Ocarina (D-pad right), moved onto the obsolete static A-label quad.
    50.0f, 85.0f, 0.0f, 64.0f, 85.0f, 0.0f,
    50.0f, 71.0f, 0.0f, 64.0f, 71.0f, 0.0f,
    // Navi marker (D-pad up)
    34.0f, 64.0f, 0.0f, 46.0f, 64.0f, 0.0f,
    34.0f, 52.0f, 0.0f, 46.0f, 52.0f, 0.0f,
    // I-slot equipped item (D-pad left)
    16.0f, 85.0f, 0.0f, 30.0f, 85.0f, 0.0f,
    16.0f, 71.0f, 0.0f, 30.0f, 71.0f, 0.0f,
    // II-slot equipped item (D-pad down)
    33.0f, 105.0f, 0.0f, 47.0f, 105.0f, 0.0f,
    33.0f, 91.0f, 0.0f, 47.0f, 91.0f, 0.0f,
    // I ammo count. Quad 12 is drawn after both D-pad item quads, so the
    // digits remain visibly layered over the equipped item artwork.
    23.0f, 85.0f, 0.0f, 30.0f, 85.0f, 0.0f,
    23.0f, 80.0f, 0.0f, 30.0f, 80.0f, 0.0f,
#ifndef PLUS_MINIMAL_HUD
    // Ten native pair cells cover all twenty hearts while staying below the
    // board's proven 33-quad upload boundary. Runtime UV updates select one
    // of fifteen lossless two-heart states from the HD atlas.
#define NATIVE_HEART_PAIR_POS(x, y) \
    (x), (y) + 11.0f, 0.0f, (x) + 22.0f, (y) + 11.0f, 0.0f, \
    (x), (y), 0.0f, (x) + 22.0f, (y), 0.0f
    NATIVE_HEART_PAIR_POS(8.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(30.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(52.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(74.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(96.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(8.0f, 19.0f),
    NATIVE_HEART_PAIR_POS(30.0f, 19.0f),
    NATIVE_HEART_PAIR_POS(52.0f, 19.0f),
    NATIVE_HEART_PAIR_POS(74.0f, 19.0f),
    NATIVE_HEART_PAIR_POS(96.0f, 19.0f),
    // II ammo count plus three live PR rupee-number glyphs. Quad 23 is
    // safely inside the proven upload range and renders after the item quad.
    40.0f, 105.0f, 0.0f, 47.0f, 105.0f, 0.0f,
    40.0f, 100.0f, 0.0f, 47.0f, 100.0f, 0.0f,
    359.0f, 226.0f, 0.0f, 365.0f, 226.0f, 0.0f,
    359.0f, 216.0f, 0.0f, 365.0f, 216.0f, 0.0f,
    366.0f, 226.0f, 0.0f, 372.0f, 226.0f, 0.0f,
    366.0f, 216.0f, 0.0f, 372.0f, 216.0f, 0.0f,
    373.0f, 226.0f, 0.0f, 379.0f, 226.0f, 0.0f,
    373.0f, 216.0f, 0.0f, 379.0f, 216.0f, 0.0f,
    // The board only uploads its first 33 quads reliably in Azahar Plus.
    // Magic needs two cells (frame and fill); the other four reliable cells
    // carry live ammo counts for X, Y, I, and II.
    8.0f, 42.0f, 0.0f, 118.0f, 42.0f, 0.0f,
    8.0f, 33.0f, 0.0f, 118.0f, 33.0f, 0.0f,
    // X ammo count
    366.0f, 27.0f, 0.0f, 373.0f, 27.0f, 0.0f,
    366.0f, 22.0f, 0.0f, 373.0f, 22.0f, 0.0f,
    // Y ammo count
    349.0f, 44.0f, 0.0f, 356.0f, 44.0f, 0.0f,
    349.0f, 39.0f, 0.0f, 356.0f, 39.0f, 0.0f,
    10.0f, 40.0f, 0.0f, 116.0f, 40.0f, 0.0f,
    10.0f, 35.0f, 0.0f, 116.0f, 35.0f, 0.0f,
    // Quads 31 and 32 cannot safely carry visible geometry on every Azahar
    // renderer. Keep them degenerate; I/II use safe quads 12 and 23 instead.
    -100.0f, -100.0f, 0.0f, -100.0f, -100.0f, 0.0f,
    -100.0f, -100.0f, 0.0f, -100.0f, -100.0f, 0.0f,
    -100.0f, -100.0f, 0.0f, -100.0f, -100.0f, 0.0f,
    -100.0f, -100.0f, 0.0f, -100.0f, -100.0f, 0.0f,
#undef NATIVE_HEART_PAIR_POS
    // Quads 33-105 are an obsolete research tail. Their implicit zero
    // initialization makes every triangle degenerate while preserving the
    // descriptor's legacy buffer footprint without spending IPS payload.
#endif
};

// UVs address Project Restoration's logical 512x256 atlas. V is flipped
// because the native board uses a bottom-left texture origin.
float gNativeHudUVs[NATIVE_HUD_VERTEX_COUNT * 2]
    __attribute__((aligned(16), used)) = {
    // Rupee starts transparent until the first live state upload.
    0.99609375f, 0.0078125f, 0.99609375f, 0.0078125f,
    0.99609375f, 0.0078125f, 0.99609375f, 0.0078125f,
    // Blank circular base: (48,0)-(85,38), used by B/X/Y.
    0.09375f, 0.8515625f, 0.166015625f, 0.8515625f,
    0.09375f, 1.0f,       0.166015625f, 1.0f,
    0.09375f, 0.8515625f, 0.166015625f, 0.8515625f,
    0.09375f, 1.0f,       0.166015625f, 1.0f,
    0.09375f, 0.8515625f, 0.166015625f, 0.8515625f,
    0.09375f, 1.0f,       0.166015625f, 1.0f,
    // D-pad: (40,39)-(67,68)
    0.078125f, 0.734375f, 0.130859375f, 0.734375f,
    0.078125f, 0.84765625f, 0.130859375f, 0.84765625f,
    // Current-save B/Master Sword, X/Bow, and Y/Bottle defaults. Runtime
    // rebinding below keeps these synchronized with the second screen.
    0.2475585938f, 0.25f,         0.2683919271f, 0.25f,
    0.2475585938f, 0.2916666667f, 0.2683919271f, 0.2916666667f,
    0.3100585938f, 0.4583333333f, 0.3308919271f, 0.4583333333f,
    0.3100585938f, 0.5f,          0.3308919271f, 0.5f,
    0.4142252604f, 0.4166666667f, 0.4350585938f, 0.4166666667f,
    0.4142252604f, 0.4583333333f, 0.4350585938f, 0.4583333333f,
    // Ocarina of Time (item-atlas cell 8).
    0.4142252604f, 0.4583333333f, 0.4350585938f, 0.4583333333f,
    0.4142252604f, 0.5f,          0.4350585938f, 0.5f,
    // Navi: (88,0)-(116,26)
    0.171875f, 0.8984375f, 0.2265625f, 0.8984375f,
    0.171875f, 1.0f,       0.2265625f, 1.0f,
    // Active-save constructor defaults from the OoT atlas's 12x12 grid:
    // I/Bombchu (cell 9), II/Longshot (cell 11).
    0.4350585938f, 0.4583333333f, 0.4558919271f, 0.4583333333f,
    0.4350585938f, 0.5f,          0.4558919271f, 0.5f,
    0.4767252604f, 0.4583333333f, 0.4975585938f, 0.4583333333f,
    0.4767252604f, 0.5f,          0.4975585938f, 0.5f,
    // I count starts transparent until the first live state upload.
    0.99609375f, 0.0078125f, 0.99609375f, 0.0078125f,
    0.99609375f, 0.0078125f, 0.99609375f, 0.0078125f,
#ifndef PLUS_MINIMAL_HUD
    // Default to atlas tile 14: two full hearts. Runtime replaces all ten
    // pair cells from live capacity and health before the board is displayed.
#define NATIVE_FULL_HEART_PAIR_UV \
    0.75f, 0.3125f, 0.8125f, 0.3125f, \
    0.75f, 0.375f,  0.8125f, 0.375f
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
#undef NATIVE_FULL_HEART_PAIR_UV
    // Quads 23-32 are overwritten with live rupee/magic UVs. Initialize them
    // transparently so no placeholder art can flash during startup.
#define NATIVE_TRANSPARENT_UV \
    0.802734375f, 0.953125f, 0.802734375f, 0.953125f, \
    0.802734375f, 0.953125f, 0.802734375f, 0.953125f
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
#undef NATIVE_TRANSPARENT_UV
    // The unused research UV tail is implicitly zeroed to match its
    // degenerate position data above.
#endif
};

// Join independent four-vertex strips with repeated degenerate vertices.
#define NATIVE_HUD_JOIN(n) \
    (4 * (n) - 1), (4 * (n)), (4 * (n)), (4 * (n) + 1), \
    (4 * (n) + 2), (4 * (n) + 3)
u16 gNativeHudIndices[NATIVE_HUD_INDEX_COUNT]
    __attribute__((aligned(16), used)) = {
    0, 1, 2, 3,
    NATIVE_HUD_JOIN(1), NATIVE_HUD_JOIN(2), NATIVE_HUD_JOIN(3),
    NATIVE_HUD_JOIN(4), NATIVE_HUD_JOIN(5), NATIVE_HUD_JOIN(6),
    NATIVE_HUD_JOIN(7), NATIVE_HUD_JOIN(8), NATIVE_HUD_JOIN(9),
    NATIVE_HUD_JOIN(10), NATIVE_HUD_JOIN(11), NATIVE_HUD_JOIN(12),
#ifndef PLUS_MINIMAL_HUD
    NATIVE_HUD_JOIN(13), NATIVE_HUD_JOIN(14), NATIVE_HUD_JOIN(15),
    NATIVE_HUD_JOIN(16), NATIVE_HUD_JOIN(17), NATIVE_HUD_JOIN(18),
    NATIVE_HUD_JOIN(19), NATIVE_HUD_JOIN(20), NATIVE_HUD_JOIN(21),
    NATIVE_HUD_JOIN(22), NATIVE_HUD_JOIN(23), NATIVE_HUD_JOIN(24),
    NATIVE_HUD_JOIN(25), NATIVE_HUD_JOIN(26), NATIVE_HUD_JOIN(27),
    NATIVE_HUD_JOIN(28), NATIVE_HUD_JOIN(29), NATIVE_HUD_JOIN(30),
    NATIVE_HUD_JOIN(31), NATIVE_HUD_JOIN(32),
#ifndef PLUS_HEARTS_ONLY
    NATIVE_HUD_JOIN(33),
    NATIVE_HUD_JOIN(34),
    NATIVE_HUD_JOIN(35),
    NATIVE_HUD_JOIN(36),
    NATIVE_HUD_JOIN(37),
    NATIVE_HUD_JOIN(38),
    NATIVE_HUD_JOIN(39),
    NATIVE_HUD_JOIN(40),
    NATIVE_HUD_JOIN(41),
    NATIVE_HUD_JOIN(42),
    NATIVE_HUD_JOIN(43),
    NATIVE_HUD_JOIN(44),
    NATIVE_HUD_JOIN(45),
    NATIVE_HUD_JOIN(46),
    NATIVE_HUD_JOIN(47),
    NATIVE_HUD_JOIN(48),
    NATIVE_HUD_JOIN(49),
    NATIVE_HUD_JOIN(50),
    NATIVE_HUD_JOIN(51),
    NATIVE_HUD_JOIN(52),
    NATIVE_HUD_JOIN(53),
    NATIVE_HUD_JOIN(54),
    NATIVE_HUD_JOIN(55),
    NATIVE_HUD_JOIN(56),
    NATIVE_HUD_JOIN(57),
    NATIVE_HUD_JOIN(58),
    NATIVE_HUD_JOIN(59),
    NATIVE_HUD_JOIN(60),
    NATIVE_HUD_JOIN(61),
    NATIVE_HUD_JOIN(62),
    NATIVE_HUD_JOIN(63),
    NATIVE_HUD_JOIN(64),
    NATIVE_HUD_JOIN(65),
    NATIVE_HUD_JOIN(66),
    NATIVE_HUD_JOIN(67),
    NATIVE_HUD_JOIN(68),
    NATIVE_HUD_JOIN(69),
    NATIVE_HUD_JOIN(70),
    NATIVE_HUD_JOIN(71),
    NATIVE_HUD_JOIN(72),
    NATIVE_HUD_JOIN(73),
    NATIVE_HUD_JOIN(74),
    NATIVE_HUD_JOIN(75),
    NATIVE_HUD_JOIN(76),
    NATIVE_HUD_JOIN(77),
    NATIVE_HUD_JOIN(78),
    NATIVE_HUD_JOIN(79),
    NATIVE_HUD_JOIN(80),
    NATIVE_HUD_JOIN(81),
    NATIVE_HUD_JOIN(82),
    NATIVE_HUD_JOIN(83),
    NATIVE_HUD_JOIN(84),
    NATIVE_HUD_JOIN(85),
    NATIVE_HUD_JOIN(86),
    NATIVE_HUD_JOIN(87),
    NATIVE_HUD_JOIN(88),
    NATIVE_HUD_JOIN(89),
    NATIVE_HUD_JOIN(90),
    NATIVE_HUD_JOIN(91),
    NATIVE_HUD_JOIN(92),
    NATIVE_HUD_JOIN(93),
    NATIVE_HUD_JOIN(94),
    NATIVE_HUD_JOIN(95),
    NATIVE_HUD_JOIN(96),
    NATIVE_HUD_JOIN(97),
    NATIVE_HUD_JOIN(98),
    NATIVE_HUD_JOIN(99),
    NATIVE_HUD_JOIN(100),
    NATIVE_HUD_JOIN(101),
    NATIVE_HUD_JOIN(102),
    NATIVE_HUD_JOIN(103),
    NATIVE_HUD_JOIN(104),
    NATIVE_HUD_JOIN(105),
#endif
#endif
};
#undef NATIVE_HUD_JOIN

// OoT3D's motion-control HUD flag. The standalone free camera does not use
// the icon, so the native proof repurposes its existing stereoscopic quad.
#define NATIVE_HUD_PROOF_VISIBLE (*(volatile u8*)0x004FC648)
#define NATIVE_HUD_BOARD (*(void**)0x004FC6BC)

typedef float* (*NativeHudMappedBufferFn)(void* board, u32 layer);
#define NATIVE_HUD_SOURCE_POSITIONS ((NativeHudMappedBufferFn)0x002FC3FC)

typedef void (*NativeHudUploadBufferFn)(void* board, u32 byteCount, const void* source);
#define NATIVE_HUD_UPLOAD_UVS ((NativeHudUploadBufferFn)0x00317D1C)

#define NATIVE_ACTION_VERTEX_COUNT 8
static float sNativeActionOriginal[NATIVE_ACTION_VERTEX_COUNT * 3];
static float* sNativeActionSource;

void NativeHud_PrepareActionPrompt(void) {
    // Quads 0 and 1 are alternate live A states. One may be collapsed while
    // the other is active, so transform them independently instead of using a
    // combined bound (which made the visible artwork tiny and malformed).
    void* actionSource = *(void**)0x004FC660;
    float* source = actionSource != 0 ? NATIVE_HUD_SOURCE_POSITIONS(actionSource, 0) : 0;
    sNativeActionSource = 0;
    if (source == 0) {
        return;
    }
    for (u32 i = 0; i < NATIVE_ACTION_VERTEX_COUNT * 3; i++) {
        sNativeActionOriginal[i] = source[i];
    }
    for (u32 quad = 0; quad < 2; quad++) {
        float* first = source + quad * 4 * 3;
        float minX = first[0];
        float maxX = first[0];
        float minY = first[1];
        float maxY = first[1];
        for (u32 i = 1; i < 4; i++) {
            float x = first[i * 3];
            float y = first[i * 3 + 1];
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
        float width = maxX - minX;
        float height = maxY - minY;
        if (width < 1.0f || height < 1.0f) {
            continue;
        }
        float sourceCenterX = (minX + maxX) * 0.5f;
        float sourceCenterY = (minY + maxY) * 0.5f;
        // The source contains separate geometry for the nearly-square A
        // circle and the wide, changing action verb. They need independent
        // scale and placement: Project Restoration keeps the circle in the
        // diamond and puts the smaller verb immediately below it.
        bool isActionText = width > height * 1.35f;
        float scale = (isActionText ? 9.0f : 16.0f) / height;
        if (isActionText && width * scale > 48.0f) {
            scale = 48.0f / width;
        }
        float scaledWidth = width * scale;
        float targetCenterX = isActionText ? 392.0f - scaledWidth * 0.5f : 382.0f;
        float targetCenterY = isActionText ? 40.0f : 37.0f;
        for (u32 i = 0; i < 4; i++) {
            float* vertex = first + i * 3;
            vertex[0] = targetCenterX + (vertex[0] - sourceCenterX) * scale;
            vertex[1] = targetCenterY + (vertex[1] - sourceCenterY) * scale;
        }
    }
    sNativeActionSource = source;
}

void NativeHud_RestoreActionPrompt(void) {
    float* source = sNativeActionSource;
    if (source == 0) {
        return;
    }
    for (u32 i = 0; i < NATIVE_ACTION_VERTEX_COUNT * 3; i++) {
        source[i] = sNativeActionOriginal[i];
    }
    sNativeActionSource = 0;
}

static void NativeHud_WriteItemUV(float* uv, u32 quad, u8 item) {
    u32 index = item <= ITEM_HEART_PIECE_2 ? item : 0xFF;
    // The 512x512 OoT item atlas is embedded unscaled at (512,512) in the
    // 2048x1024 Project Restoration replacement. The working HUD mapping
    // addresses it as a 12x12 grid and shifts the sampling window five source
    // pixels left to align the live item artwork.
    u32 column = index % 12;
    u32 row = index / 12;
    float u0 = 0.25f + (float)column / 48.0f - (5.0f / 2048.0f);
    float u1 = u0 + (1.0f / 48.0f);
    float v1 = 0.5f - (float)row / 24.0f;
    float v0 = v1 - (1.0f / 24.0f);
    uv += quad * 8;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1;
    uv[6] = u1; uv[7] = v1;
}

static u32 NativeHud_HeartProgress(s32 health) {
    if (health <= 0) return 0;
    if (health <= 4) return 1;
    if (health <= 8) return 2;
    if (health < 16) return 3;
    return 4;
}

static void NativeHud_WriteHeartPairUV(float* uv, u32 quad,
                                       u32 capacity, s32 health) {
    uv += quad * 8;
    if (capacity > 2) capacity = 2;
    s32 maxHealth = (s32)capacity * 16;
    if (health < 0) health = 0;
    if (health > maxHealth) health = maxHealth;

    u32 tile = 0;
    if (capacity == 1) {
        tile = 1 + NativeHud_HeartProgress(health);
    } else if (capacity == 2) {
        tile = health <= 16 ? 6 + NativeHud_HeartProgress(health) :
               10 + NativeHud_HeartProgress(health - 16);
    }

    const u32 column = tile % 5;
    const u32 row = tile / 5;
    // Half-pixel insets prevent bilinear filtering from sampling an adjacent
    // pair tile at the bank boundaries.
    float u0 = (1024.5f + (float)column * 128.0f) / 2048.0f;
    float u1 = u0 + 127.0f / 2048.0f;
    float v1 = 1.0f - (512.5f + (float)row * 64.0f) / 1024.0f;
    float v0 = v1 - 63.0f / 1024.0f;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1;
    uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteSolidUV(float* uv, u32 quad, float pixelX, float pixelY) {
    const float u = pixelX / 2048.0f;
    const float v = 1.0f - pixelY / 1024.0f;
    uv += quad * 8;
    for (u32 i = 0; i < 4; i++) {
        uv[i * 2] = u;
        uv[i * 2 + 1] = v;
    }
}

static void NativeHud_WriteMagicFrameUV(float* uv, u32 quad, bool doubleMagic) {
    uv += quad * 8;
    const float u0 = 1024.5f / 2048.0f;
    const float u1 = u0 + 439.0f / 2048.0f;
    const float y = doubleMagic ? 756.5f : 720.5f;
    const float v1 = 1.0f - y / 1024.0f;
    const float v0 = v1 - 35.0f / 1024.0f;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1;
    uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteMagicFillUV(float* uv, u32 quad, u32 width) {
    if (width > 106) width = 106;
    uv += quad * 8;
    const float u0 = (1024.5f + (float)(106 - width) * 4.0f) / 2048.0f;
    const float u1 = u0 + 423.0f / 2048.0f;
    const float v1 = 1.0f - 800.5f / 1024.0f;
    const float v0 = v1 - 19.0f / 1024.0f;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1;
    uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteRupeeUV(float* uv, u32 quad) {
    uv += quad * 8;
    const float u0 = 456.0f / 512.0f;
    const float u1 = 474.0f / 512.0f;
    const float v0 = 1.0f - 78.0f / 256.0f;
    const float v1 = 1.0f - 60.0f / 256.0f;
    uv[0] = u0; uv[1] = v0; uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1; uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteDigitUV(float* uv, u32 quad, u32 digit) {
    uv += quad * 8;
    // PR's 0-9 strip uses fixed 8x16 logical cells at y=72.
    const float u0 = (float)(digit * 8) / 512.0f;
    const float u1 = (float)(digit * 8 + 8) / 512.0f;
    const float v0 = 1.0f - 88.0f / 256.0f;
    const float v1 = 1.0f - 72.0f / 256.0f;
    uv[0] = u0; uv[1] = v0; uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1; uv[6] = u1; uv[7] = v1;
}

static u8 NativeHud_AmmoSlotForItem(u8 item) {
    switch (item) {
        case ITEM_STICK: return SLOT_STICK;
        case ITEM_NUT: return SLOT_NUT;
        case ITEM_BOMB: return SLOT_BOMB;
        case ITEM_BOW:
        case ITEM_ARROW_FIRE:
        case ITEM_ARROW_ICE:
        case ITEM_ARROW_LIGHT:
        case ITEM_BOW_ARROW_FIRE:
        case ITEM_BOW_ARROW_ICE:
        case ITEM_BOW_ARROW_LIGHT: return SLOT_BOW;
        case ITEM_SLINGSHOT: return SLOT_SLINGSHOT;
        case ITEM_BOMBCHU: return SLOT_BOMBCHU;
        case ITEM_BEAN: return SLOT_BEAN;
        default: return SLOT_NONE;
    }
}

static void NativeHud_WriteAmmoUV(float* uv, u32 quad, u8 item) {
    const u8 slot = NativeHud_AmmoSlotForItem(item);
    if (slot == SLOT_NONE) {
        NativeHud_WriteSolidUV(uv, quad, 2040.0f, 1016.0f);
        return;
    }

    s32 ammo = gSaveContext.ammo[slot];
    if (ammo < 0) ammo = 0;
    if (ammo > 50) ammo = 50;

    // The atlas stores 0-50 as lossless two-digit PR tiles in an 18-column
    // bank. Single-digit values are right-aligned with a transparent tens
    // cell, matching the native OoT3D counter treatment.
    const u32 column = (u32)ammo % 18;
    const u32 row = (u32)ammo / 18;
    const float u0 = (1024.5f + (float)column * 56.0f) / 2048.0f;
    const float u1 = u0 + 55.0f / 2048.0f;
    const float v1 = 1.0f - (832.5f + (float)row * 64.0f) / 1024.0f;
    const float v0 = v1 - 63.0f / 1024.0f;
    uv += quad * 8;
    uv[0] = u0; uv[1] = v0; uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1; uv[6] = u1; uv[7] = v1;
}

static void NativeHud_UpdateMappedItems(u8 visible) {
    void* board = NATIVE_HUD_BOARD;
    if (!visible || board == 0) {
        return;
    }

    // These are the live values used by the current second-screen I/II
    // buttons. Avoid reconstructing them through inventory-slot indices.
    u8 itemI = gSaveContext.equips.buttonItems[3];
    u8 itemII = gSaveContext.equips.buttonItems[4];
    // Some touchless layouts leave the cached I/II button-item bytes at
    // ITEM_NONE while retaining the equipped inventory slots. Resolve those
    // two slots from the live inventory so the native HUD mirrors the action
    // that the corner touch targets actually trigger.
    ItemEquips* ageEquips = gSaveContext.linkAge == 0 ?
        &gSaveContext.adultEquips : &gSaveContext.childEquips;
    if (itemI > ITEM_HEART_PIECE_2) itemI = ageEquips->buttonItems[3];
    if (itemII > ITEM_HEART_PIECE_2) itemII = ageEquips->buttonItems[4];
    u8 slotI = gSaveContext.equips.buttonSlots[2];
    u8 slotII = gSaveContext.equips.buttonSlots[3];
    if (slotI == SLOT_NONE) slotI = ageEquips->buttonSlots[2];
    if (slotII == SLOT_NONE) slotII = ageEquips->buttonSlots[3];
    if (itemI > ITEM_HEART_PIECE_2 && slotI < 26) itemI = gSaveContext.items[slotI];
    if (itemII > ITEM_HEART_PIECE_2 && slotII < 26) itemII = gSaveContext.items[slotII];
    NativeHud_WriteItemUV(gNativeHudUVs, 5, gSaveContext.equips.buttonItems[0]);
    NativeHud_WriteItemUV(gNativeHudUVs, 6, gSaveContext.equips.buttonItems[2]);
    NativeHud_WriteItemUV(gNativeHudUVs, 7, gSaveContext.equips.buttonItems[1]);
    NativeHud_WriteItemUV(gNativeHudUVs, 10, itemI);
    NativeHud_WriteItemUV(gNativeHudUVs, 11, itemII);
    NativeHud_WriteItemUV(gNativeHudUVs, 8, ITEM_OCARINA_TIME);

#ifndef PLUS_MINIMAL_HUD
    s32 maxHearts = gSaveContext.healthCapacity > 0 ? gSaveContext.healthCapacity / 16 : 0;
    if (maxHearts > 20) maxHearts = 20;
    s32 health = gSaveContext.health > 0 ? gSaveContext.health : 0;
    for (u32 i = 0; i < 10; i++) {
        s32 remainingCapacity = maxHearts - (s32)i * 2;
        u32 pairCapacity = remainingCapacity <= 0 ? 0 :
                           remainingCapacity == 1 ? 1 : 2;
        NativeHud_WriteHeartPairUV(gNativeHudUVs, 13 + i, pairCapacity,
                                   health - (s32)i * 32);
    }

#ifndef PLUS_HEARTS_ONLY
    u32 rupees = gSaveContext.rupees > 0 ? (u32)gSaveContext.rupees : 0;
    if (rupees > 999) rupees = 999;
    NativeHud_WriteRupeeUV(gNativeHudUVs, 0);
    if (rupees >= 100) {
        NativeHud_WriteDigitUV(gNativeHudUVs, 24, rupees / 100);
    } else {
        NativeHud_WriteSolidUV(gNativeHudUVs, 24, 1644.0f, 48.0f);
    }
    if (rupees >= 10) {
        NativeHud_WriteDigitUV(gNativeHudUVs, 25, (rupees / 10) % 10);
    } else {
        NativeHud_WriteSolidUV(gNativeHudUVs, 25, 1644.0f, 48.0f);
    }
    NativeHud_WriteDigitUV(gNativeHudUVs, 26, rupees % 10);

    NativeHud_WriteAmmoUV(gNativeHudUVs, 28,
                          gSaveContext.equips.buttonItems[2]);
    NativeHud_WriteAmmoUV(gNativeHudUVs, 29,
                          gSaveContext.equips.buttonItems[1]);
    NativeHud_WriteAmmoUV(gNativeHudUVs, 12, itemI);
    NativeHud_WriteAmmoUV(gNativeHudUVs, 23, itemII);

    // The upgrade state lives in dedicated save flags. magicLevel is not a
    // reliable indication of double magic in a naturally progressed save.
    const bool hasMagic = gSaveContext.magicAcquired != 0 || gSaveContext.magicLevel > 0;
    const bool doubleMagic = gSaveContext.doubleMagic != 0;
    if (hasMagic) {
        NativeHud_WriteMagicFrameUV(gNativeHudUVs, 27, doubleMagic);
    } else {
        NativeHud_WriteSolidUV(gNativeHudUVs, 27, 1644.0f, 48.0f);
    }
    u32 magicWidth = 0;
    if (hasMagic) {
        const u32 maxMagic = doubleMagic ? 0x60 : 0x30;
        u32 magic = gSaveContext.magic > 0 ? (u32)gSaveContext.magic : 0;
        if (magic > maxMagic) magic = maxMagic;
        const u32 displayWidth = doubleMagic ? 106 : 70;
        magicWidth = (magic * displayWidth) / maxMagic;
    }
    if (hasMagic) {
        NativeHud_WriteMagicFillUV(gNativeHudUVs, 30, magicWidth);
    } else {
        NativeHud_WriteSolidUV(gNativeHudUVs, 30, 1644.0f, 48.0f);
    }
#endif
#endif

    // Upload the complete UV stream through the board's native setter.
    // 0x4546F4, used previously, is an internal address calculator rather
    // than a UV getter and caused the correct values to land in unrelated
    // GPU memory.
    NATIVE_HUD_UPLOAD_UVS(board, sizeof(gNativeHudUVs), gNativeHudUVs);
}

void NativeHud_UpdateProof(GlobalContext* globalCtx) {
    u8 visible = globalCtx != 0 && IsInGameOrBossChallenge();
    NativeHud_UpdateMappedItems(visible);
    NATIVE_HUD_PROOF_VISIBLE = visible ? 1 : 0;
}
