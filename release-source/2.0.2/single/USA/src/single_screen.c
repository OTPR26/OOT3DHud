#include "single_screen.h"
#include "unified_menu_layout.h"
#include "unified_menu.h"
#include "gear_preview.h"
#include "menu_options.h"
#include "ocarina_presentation.h"

#include "input_remap.h"
#include "draw.h"
#include "native_hud.h"
#include "z3D/z3D.h"
#if CONTROLLER_OPTIONS_ENABLED
#include "controller_settings.h"
#endif

#if defined(Version_USA) || (defined(Version_JP) && JP_SINGLE_SCREEN_PROBE) || (defined(Version_TWN) && TWN_SINGLE_SCREEN_PROBE)

#if defined(Version_TWN)
#define SET_VIEWPORT_ADDR 0x0031DCE4
#define FINISH_TOP_PASS_ADDR 0x0010C744
#define DRAW_SECONDARY_UI_ADDR 0x001089B4
#define DRAW_SECONDARY_WIDGETS_ADDR 0x00109034
#define GPU_COMMAND_CURRENT_ADDR 0x0055ACD0
#define GPU_COMMAND_END_ADDR 0x0055ACD4
#define BOARD_VTABLE_ADDR 0x004F431C
#define DRAW_FILE_LOGO_ADDR 0x0010C754
#define UPDATE_FILE_BOARD_ADDR 0x00317130
#define DRAW_TITLE_ADDR 0x00105184
#else
#define SET_VIEWPORT_ADDR       ADDR(0x002FEABC)
#define FINISH_TOP_PASS_ADDR    ADDR(0x004228B0)
#define DRAW_SECONDARY_UI_ADDR  ADDR(0x0041EC2C)
#define DRAW_SECONDARY_WIDGETS_ADDR ADDR(0x0041F2E4)
#define GPU_COMMAND_CURRENT_ADDR 0x0054CC4C
#define GPU_COMMAND_END_ADDR     0x0054CC50
#define BOARD_VTABLE_ADDR 0x004EBD60
#define DRAW_FILE_LOGO_ADDR ADDR(0x004228C0)
#define UPDATE_FILE_BOARD_ADDR ADDR(0x002F9A1C)
#define DRAW_TITLE_ADDR ADDR(0x0041AF88)
#endif

typedef void (*SetViewportFn)(unsigned int x, int y,
                              unsigned int width, int height);
typedef void (*FinishTopPassFn)(void* pass);
typedef void (*DrawSecondaryUiFn)(void);
typedef void (*DrawSecondaryWidgetsFn)(void);
typedef void (*DrawSecondaryCallbackFn)(void* context);

static const SetViewportFn SetViewport =
    (SetViewportFn)SET_VIEWPORT_ADDR;
static const FinishTopPassFn FinishTopPass =
    (FinishTopPassFn)FINISH_TOP_PASS_ADDR;
static const DrawSecondaryUiFn DrawSecondaryUi =
    (DrawSecondaryUiFn)DRAW_SECONDARY_UI_ADDR;
static const DrawSecondaryWidgetsFn DrawSecondaryWidgets =
    (DrawSecondaryWidgetsFn)DRAW_SECONDARY_WIDGETS_ADDR;
static void* sSecondaryCallbackContext = 0;
static DrawSecondaryCallbackFn sSecondaryCallback = 0;
static u8 sFileGuidanceVisible = 1;
static void* sFilePresentationContext = 0;
static void* sHintContext = 0;

static u8 IsNameEntryActive(void) {
#if defined(Version_USA) || defined(Version_EUR) || defined(Version_JP)
    return ((u32 (*)(void))ADDR(0x00427CA8))() != 0;
#else
    return 0;
#endif
}
static void SetScissor(u32 mode, u32 left, u32 right, u32 top, u32 bottom);

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
static u8 sOcarinaPresentation;

static void UpdateOcarinaPresentation(void) {
    const u8 allowed = gSaveContext.gameMode == 0 &&
        !InputRemap_IsItemsMenuOpen() && !InputRemap_IsSaveMenuOpen() &&
        !UnifiedMenu_GetDialogPage() && !InputRemap_IsGameOverUiOpen();
    sOcarinaPresentation = OcarinaPresentation_Update(sOcarinaPresentation,
        allowed, InputRemap_IsOcarinaUiOpen(),
        *(volatile u8*)(0x005C1878 + 0xF38) != 0);
}
#endif

static u8 ApplyPrototypeItemsViewport(MenuRect rect) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    MenuViewport v;
    if (InputRemap_IsItemsPageActive() && MenuLayout_Viewport(rect, &v)) {
        SetViewport(v.x, v.y, v.width, v.height);
        return 1;
    }
#endif
    return 0;
}

static u8 ApplyPrototypePreviewViewport(void) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    MenuViewport v;
    const MenuRect rect = InputRemap_IsItemsPageActive() ?
        gMenuItemPreview : gMenuWidePreview;
    if (InputRemap_IsItemsMenuOpen() && MenuLayout_Viewport(rect, &v)) {
        if (!InputRemap_IsItemsPageActive() && !InputRemap_IsGearPageActive() &&
            ((int (*)(void))0x002F1268)()) {
            /* Dungeon's native small map occupies the lower-left of the
             * presentation viewport. Restore the verified 113 transform;
             * larger off-screen viewports in 114/115 lose the preview. Any
             * further enlargement must happen in preview geometry instead.
             * Leave the separate large map/floor-selector replay untouched. */
            SetViewport(190,-411,720,540);
            MenuScissor clip;
            if (MenuLayout_Scissor((MenuRect){222,44,160,148}, &clip))
                SetScissor(3,clip.left,clip.top,clip.right,clip.bottom);
            return 1;
        }
        /* Translate only Gear's presentation. The native viewport has blank
         * side margins; its visible panel stays on-screen at unchanged scale.
         * Signed GPU Y is the rotated horizontal offset (portrait uses it too). */
        if (InputRemap_IsGearPageActive()) v.y -= MENU_GEAR_DETAIL_SHIFT;
        else if (!InputRemap_IsItemsPageActive() &&
                 !((int (*)(void))0x002F1268)())
            v.y -= MENU_MAP_DETAIL_SHIFT;
        SetViewport(v.x, v.y, v.width, v.height);
        return 1;
    }
#endif
    return 0;
}

static u8 IsFilePresentationActive(void) {
    if (IsNameEntryActive()) return 0;
#if defined(Version_TWN)
    // Taiwan widgets gate at 10903C; caption state independently verified
    // at 148C38 and retains the shared context+890 layout.
    return gSaveContext.gameMode == 2 &&
        *(volatile u32*)0x00511448 != 0;
#else
    // gameMode changes before the outgoing title scene has finished drawing.
    // The native file-board state stays zero until the menu is initialized,
    // and returns to zero when leaving it. Do not resize the title/fade pass.
    return gSaveContext.gameMode == 2 &&
        *(volatile u32*)0x00504FB0 != 0;
#endif
}

void SingleScreen_BeforeFileScene(void) {
    // Align the pedestal bottom with the bottom file slot. The caption
    // overlaps its lower edge, hiding the native model viewport cutoff.
    if (IsFilePresentationActive()) SetViewport(119, -8, 300, 200);
}

void SingleScreen_AfterFileScene(void) {
    if (gSaveContext.gameMode == 2) SetViewport(0, 0, 480, 400);
}

static void SetScissor(u32 mode, u32 left, u32 right,
                       u32 top, u32 bottom) {
    volatile u32** currentAddress =
        (volatile u32**)GPU_COMMAND_CURRENT_ADDR;
    volatile u32** endAddress =
        (volatile u32**)GPU_COMMAND_END_ADDR;
    volatile u32* command = *currentAddress;
    if (command + 6 > *endAddress) {
        return;
    }

    command[0] = mode;
    command[1] = 0x000F0065;
    command[2] = (right << 16) | left;
    command[3] = 0x000F0066;
    command[4] = (bottom << 16) | top;
    command[5] = 0x000F0067;
    *currentAddress = command + 6;
}

void SingleScreen_DrawFileText(void* context, void (*callback)(void*)) {
    sHintContext = 0;
#if defined(Version_USA) || defined(Version_EUR) || defined(Version_JP)
    if (context && callback == (DrawSecondaryCallbackFn)ADDR(0x00471F40)) {
        sHintContext = context;
        // Native upper image: 200 x 120, centered in the left half.
        SetViewport(120, 200, 240, 200);
        callback(context);
        return;
    }
#endif
    if (UnifiedMenu_IsOptionsDialog()) return;
    // This callback owns only the guidance glyphs (foreground and shadow).
    // The logo is drawn by the following native pass, not this callback.
    if (IsFilePresentationActive()) {
        sFilePresentationContext = context;
        // The native file-list caption is enabled only in presentation 2.
        sFileGuidanceVisible = context != 0 &&
            *(u32*)((u8*)context + 0x890) == 2;
        // Enlarge the caption 20%, preserving its raised visual center.
        SetViewport(56, 2, 264, 192);
    }
    callback(context);
}

void SingleScreen_BeforeFileBackdrop(void) {
    // Native pass 3 precedes the text. Keep its rounded translucent board
    // in precisely the same viewport as the caption below Link.
    if (IsFilePresentationActive()) SetViewport(56, 2, 264, 192);
}

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
static u8 IsGameplayDialogueActive(void) {
    /* Native MessageContext at PlayState+0x32C0. Never rescale pause,
     * file-selection, or Game Over presentation through this path. */
    return gSaveContext.gameMode == 0 && !InputRemap_IsItemsMenuOpen() &&
        !InputRemap_IsSaveMenuOpen() && !UnifiedMenu_GetDialogPage() &&
        !InputRemap_IsGameOverUiOpen() &&
        *(volatile u8*)(0x005C1878 + 0xF38) != 0;
}

static void ApplyDialogueViewport(void) {
    if (sOcarinaPresentation) {
        /* Native contextual notation/result belongs to MessageContext,
         * not the Songs reference panel. Move board and text together.
         * Half the usual dialogue scale gives the visible board the same
         * ~128-unit width as the song reference. The virtual viewport starts
         * above the screen so the bottom-anchored native board lands at the
         * top, above Link, without moving any gameplay HUD elements. */
        /* Center the 160-unit viewport in the 400-unit display: the
         * rotated horizontal offset is (400 - 160) / 2 = 120. */
        SetViewport(388, 120, 192, 160);
        SetScissor(0, 0, 0, 479, 399);
        return;
    }
    /* 400x240 -> 320x192, horizontally centered and bottom anchored.
     * GPU coordinates are rotated and the vertical axis is doubled. */
    SetViewport(0, 40, 384, 320);
}

void SingleScreen_DrawDialogueBoards(void* pass) {
    UpdateOcarinaPresentation();
    const u8 compact = IsGameplayDialogueActive();
    SingleScreen_BeforeFileBackdrop();
    if (compact) ApplyDialogueViewport();
    ((FinishTopPassFn)0x004228B8)(pass);
    if (IsFilePresentationActive() && sFileGuidanceVisible) {
        /* This file-select pass contains the rounded caption board only.
         * Its translucency is baked into the native texture. Composite it
         * four times for near-opaque coverage without changing that shared
         * texture, its rounded edge, or any gameplay dialogue. Glyphs are
         * a separate, single draw. Effective alpha: 1 - (1 - a)^4. */
        for (u32 layer = 1; layer < 4; ++layer)
            ((FinishTopPassFn)0x004228B8)(pass);
    }
    if (compact) SetViewport(0, 0, 480, 400);
}

void SingleScreen_DrawDialogueText(void* message) {
    if (UnifiedMenu_IsOptionsDialog()) {
        SetScissor(0,0,0,479,399);
        MenuOptions_Draw();
        return;
    }
    const u8 compact = IsGameplayDialogueActive();
    if (compact) ApplyDialogueViewport();
    ((void (*)(void*))0x0042CBA8)(message);
    if (compact) SetViewport(0, 0, 480, 400);
}
#endif

void SingleScreen_DrawFileLogoPass(void* pass) {
    const FinishTopPassFn drawPass = (FinishTopPassFn)DRAW_FILE_LOGO_ADDR;
    if (!IsFilePresentationActive()) {
        drawPass(pass);
        return;
    }

    u32* count = (u32*)((u8*)pass + 0x24);
    u32** nodes = (u32**)((u8*)pass + 0x394);
    // USA pass 4 contains the logo followed by the full-screen fade.
    // Validate the native pair; do not infer identity from allocated addresses.
    // Both are board nodes, with adjacent resources (fade first, then logo).
    u8 splitFade = *count == 2 && *(u32*)((u8*)pass + 0x764) == 0 &&
        nodes[0] != 0 && nodes[1] != 0 &&
        nodes[0][0] == BOARD_VTABLE_ADDR && nodes[1][0] == BOARD_VTABLE_ADDR &&
        nodes[0][1] == nodes[1][1] + 0x1C8 &&
        ((float*)nodes[1])[15] == 200.0f &&
        ((float*)nodes[1])[16] == 120.0f;

    SetViewport(sFileGuidanceVisible ? 290 : 392, 114, 324, 324);
    if (splitFade) {
        u32* logo = nodes[0];
        *count = 1;
        drawPass(pass);
        SetViewport(0, 0, 480, 400);
        nodes[0] = nodes[1];
        drawPass(pass);
        nodes[0] = logo;
        *count = 2;
    } else {
        drawPass(pass);
    }
}

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
static void TranslatedQuadCenter(const float* p, const float* t, u32 first,
                                 u32 count, float* cx, float* cy) {
    float l = 1000.0f, r = -1000.0f, top = 1000.0f, bottom = -1000.0f;
    for (u32 q = first; q < first + count; ++q) {
        for (u32 v = 0; v < 4; ++v) {
            const float x = p[q*12+v*3] - t[q*2];
            const float y = p[q*12+v*3+1] - t[q*2+1];
            if (x < l) l = x;
            if (x > r) r = x;
            if (y < top) top = y;
            if (y > bottom) bottom = y;
        }
    }
    *cx = (l + r) * 0.5f;
    *cy = (top + bottom) * 0.5f;
}

static void ScaleTranslatedQuads(float* p, const float* t, u32 first, u32 count,
                                 float cx, float cy) {
    if (!p || !t) return;
    for (u32 q = first; q < first + count; ++q) {
        for (u32 v = 0; v < 4; ++v) {
            float* x = &p[q*12+v*3];
            float* y = &p[q*12+v*3+1];
            *x = t[q*2] + cx + (*x - t[q*2] - cx) * 0.6f;
            *y = t[q*2+1] + cy + (*y - t[q*2+1] - cy) * 0.6f;
        }
    }
}
#endif

void SingleScreen_UpdateFileBoard(void* board) {
    ((void (*)(void*))UPDATE_FILE_BOARD_ADDR)(board);
    if (!IsFilePresentationActive() || board == 0) return;
    const u32 count = *(u32*)board;
    float* positions = *(float**)((u8*)board + 0x10);
    if (positions == 0 || count != 184) return;
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    const u32 fileState = *(volatile u32*)0x00504FB0;
    const u8 fileActions = fileState >= 7 && fileState <= 10;
    /* State 4 starts the occupied-file exit animation, before state 7
     * introduces the action buttons. State 5 is the empty-file exit.
     * Hide both the prompt and its board at that first transition tick,
     * not after the other slots have already finished moving away. */
    const u8 hideFileHeading = fileState == 4 || fileState == 5 || fileActions;
    u32* listHeading = ((u32**)0x0055B548)[6];
    if (listHeading && listHeading[0x40/4] == 0) {
        for (u32 n = 0; n < 2; ++n) {
            float* node = (float*)listHeading[0x10/4+n];
            if (node) node[0xFC/4] = hideFileHeading ? 0.0f : 1.0f;
        }
    }
    if (fileActions) {
        /* Preserve the native caption lifecycle on the Start/Copy/Erase
         * page as well as the three-file list. The context is captured by
         * its own live draw callback before a file can be selected. */
        if (sFilePresentationContext)
            *(u32*)((u8*)sFilePresentationContext + 0x89C) = 1;
        /* Entry seven is exclusively the redundant File 1/2/3 heading.
         * Its two persistent nodes are destroyed when leaving this page. */
        u32* heading = ((u32**)0x0055B548)[7];
        if (heading && heading[0x40/4] == 0) {
            for (u32 n = 0; n < 2; ++n) {
                float* node = (float*)heading[0x10/4+n];
                if (node) node[0xFC/4] = 0.0f;
            }
        }
    }
    /* Each action has three stone slices and one lettering quad.
     * Start is 124x52 at 60%; Copy/Erase are natively 84x36. Normalize
     * their final bounds to 90% of the resized Start (66.96 x 28.08).
     * Preserve each center and native transition translation. */
    const float actionX[3] = {160.0f, 112.0f, 208.0f};
    const float actionY[3] = {132.0f, 180.0f, 180.0f};
    const float actionScaleX[3] = {0.6f, 66.96f/84.0f, 66.96f/84.0f};
    const float actionScaleY[3] = {0.6f, 28.08f/36.0f, 28.08f/36.0f};
    const float* offsets = *(float**)((u8*)board + 0x1C);
    if (offsets) {
        for (u32 action = 0; action < 3; ++action) {
            const u32 first = 122 + action*4;
            const float cx = actionX[action] + offsets[first*2];
            const float cy = actionY[action] + offsets[first*2+1];
            for (u32 q = first; q < first+4; ++q) {
                float* p = positions + q*12;
                for (u32 v = 0; v < 4; ++v) {
                    p[v*3] = cx + (p[v*3] - cx)*actionScaleX[action];
                    p[v*3+1] = cy + (p[v*3+1] - cy)*actionScaleY[action];
                }
            }
        }
    }
    const u32 focusedAction = *(volatile u32*)0x00504FB8;
    if ((fileState == 8 || fileState == 10) && focusedAction < 3) {
        /* 445088 freshly rebuilds the cursor's source quads and offsets
         * before this hook; 2F94A8 uploads them afterwards. Scale both
         * the corner marks and spacing around the focused action. */
        u32* cursor = *(u32**)0x00504FD4;
        u32* cursorBoard = cursor ? (u32*)cursor[2] : 0;
        // Native cursor mode 0 has eight quads: four corners + shadows.
        if (cursorBoard && cursor[9] == 0 && cursorBoard[0] == 8) {
            float* p = (float*)cursorBoard[3];
            float* t = (float*)cursorBoard[7];
            if (p && t) {
                const float cx = actionX[focusedAction];
                const float cy = actionY[focusedAction];
                const float sx = actionScaleX[focusedAction];
                const float sy = actionScaleY[focusedAction];
                for (u32 q = 0; q < 8; ++q) {
                    t[q*2] = cx + (t[q*2] - cx)*sx;
                    t[q*2+1] = cy + (t[q*2+1] - cy)*sy;
                    for (u32 v = 0; v < 4; ++v) {
                        p[q*12+v*3] *= sx;
                        p[q*12+v*3+1] *= sy;
                    }
                }
            }
        }
    }
    if (fileState >= 11 && fileState <= 42 && offsets) {
        // Each confirmation button has three stone slices, a label, and
        // three selected-state slices. Keep all seven on one scale/center.
        for (u32 action = 0; action < 4; ++action) {
            const u32 first = 134 + action * 7;
            float cx, cy;
            TranslatedQuadCenter(positions, offsets, first, 7, &cx, &cy);
            ScaleTranslatedQuads(positions, offsets, first, 7, cx, cy);
        }
        u32* cursor = *(u32**)0x00504FD4;
        u32* cursorBoard = cursor ? (u32*)cursor[2] : 0;
        if (cursorBoard && cursor[9] == 0 && cursorBoard[0] == 8) {
            float* p = (float*)cursorBoard[3];
            float* t = (float*)cursorBoard[7];
            if (p && t) {
                float cx, cy;
                TranslatedQuadCenter(p, t, 0, 8, &cx, &cy);
                ScaleTranslatedQuads(p, t, 0, 8, cx, cy);
            }
        }
    }
    /* File names are kind-2 text: eight outline nodes and one foreground.
     * Scale their freshly regenerated offsets as a group so the outline
     * stays aligned. Keep the left edge and center vertically in its row. */
    // The native second confirmation animates the selected filename down
    // and fades it out. Hold it at the action page's slot position instead.
    static float confirmationNameY[3];
    for (u32 slot = 0; slot < 3; ++slot) {
        u32* text = ((u32**)0x0055B548)[slot];
        if (text && text[0x40/4] == 2) {
            float* foreground = (float*)text[0x30/4];
            if (foreground) {
                if ((fileState == 8 || fileState == 10) &&
                    foreground[0x3C/4] > 0.0f)
                    confirmationNameY[slot] = foreground[0x40/4];
                const u8 holdName = fileState >= 35 && fileState <= 42 &&
                    confirmationNameY[slot] != 0.0f &&
                    foreground[0x3C/4] > 0.0f;
                const float shiftY = holdName ?
                    confirmationNameY[slot] - foreground[0x40/4] : 0.0f;
                const float left = foreground[0x3C/4];
                const float centerY = foreground[0x40/4] + 8.0f;
                for (u32 n = 0; n < 9; ++n) {
                    float* node = (float*)text[0x10/4+n];
                    if (!node) continue;
                    node[0x3C/4] = left + (node[0x3C/4] - left) * 0.8f;
                    node[0x40/4] = centerY +
                        (node[0x40/4] - centerY) * 0.8f + shiftY;
                    node[0x48/4] = node[0x4C/4] = 0.8f;
                    if (holdName) node[0xFC/4] = 1.0f;
                }
            }
        }
        /* The numbered round badge is quad 10 of each 37-quad slot.
         * Other slot decoration, especially the nine pendants, is untouched. */
        float* badge = positions + (10 + slot * 37) * 12;
        const float centerX = (badge[0] + badge[9]) * 0.5f;
        const float centerY = (badge[1] + badge[10]) * 0.5f;
        for (u32 v = 0; v < 4; ++v) {
            badge[v*3] = centerX + (badge[v*3] - centerX) * 0.8f;
            badge[v*3+1] = centerY + (badge[v*3+1] - centerY) * 0.8f;
        }
    }
    /* Timestamp constructor 44CE70 creates one kind-3 board node.
     * 2EE5E4 has just regenerated its translation. Adjust here, before
     * the engine prepares its draw matrix; replay-time changes are late.
     * Assign absolute scale, since only translation resets each tick. */
    for (u32 slot = 0; slot < 3; ++slot) {
        u32* text = ((u32**)0x0055B554)[slot];
        if (!text || text[0x40/4] != 3) continue;
        float* node = (float*)text[0x10/4];
        if (!node) continue;
        node[0x3C/4] += 11.0f;
        node[0x40/4] -= 4.0f;
        node[0x48/4] = 0.8f;
        node[0x4C/4] = 0.8f;
    }
    /* Native 2ED224 owns twenty heart quads per file, starting at 11
     * with a 37-quad slot stride. Scale the regenerated group, not its
     * persistent source, so repeated updates never accumulate shrinkage. */
    for (u32 slot = 0; slot < 3; ++slot) {
        float* first = positions + (11 + slot * 37) * 12;
        const float centerX = first[0] + 55.0f;
        const float topY = first[1];
        for (u32 heart = 0; heart < 20; ++heart) {
            float* p = first + heart * 12;
            for (u32 v = 0; v < 4; ++v) {
                p[v*3] = centerX + (p[v*3] - centerX) * 0.8f;
                p[v*3+1] = topY + (p[v*3+1] - topY) * 0.8f;
            }
        }
    }
#endif
    // Work on the newly generated output, not the persistent source quads:
    // every native update regenerates these, so the scale cannot accumulate.
    // The heading is six slices (two rows of three), native x=12..308,
    // y=4..44. The visible inset is not the geometry's actual boundary.
    // Glyphs are a separate draw and retain their readable size.
    for (u32 q = 0; q < 6; ++q) {
        float* p = positions + q * 12;
        for (u32 v = 0; v < 4; ++v) {
#if defined(Version_JP) || defined(Version_TWN)
            // Japanese guidance is longer than the English label.
            p[v * 3] = 160.0f + (p[v * 3] - 160.0f) * 0.75f;
#else
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
            const float width = fileState >= 35 && fileState <= 42 ?
                1.12f : fileState >= 11 ? 1.0f : 0.60f;
            p[v * 3] = 160.0f + (p[v * 3] - 160.0f) * width;
#else
            p[v * 3] = 160.0f + (p[v * 3] - 160.0f) * 0.60f;
#endif
#endif
            p[v * 3 + 1] = 24.0f + (p[v * 3 + 1] - 24.0f) * 0.70f;
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
            if (hideFileHeading) p[v*3] = p[v*3+1] = -1000.0f;
#endif
        }
    }
    // The middle stretches atlas column 286. Sample its texel center and
    // end both rounded caps at that same opaque sample. The native cap edge
    // at 288 can filter in transparent neighboring pixels, exposing a seam.
    float* uv = *(float**)((u8*)board + 0x14);
    const float atlasWidth = *(float*)((u8*)board + 4);
    if (uv != 0 && atlasWidth > 0.0f) {
        const float joinU = 286.5f / atlasWidth;
        for (u32 row = 0; row < 2; ++row) {
            float* left = uv + row * 24;
            float* middle = left + 8;
            float* right = left + 16;
            left[2] = left[6] = joinU;
            middle[0] = middle[2] = middle[4] = middle[6] = joinU;
            right[0] = right[4] = joinU;
        }
    }
}

void SingleScreen_DrawSecondaryBackdrop(void* node, void (*draw)(void*)) {
    if (UnifiedMenu_GetDialogPage()) return;
    if (InputRemap_IsGameOverUiOpen()) {
        // Game Over retains its own native dialogue, not the shared leather.
        return;
    }
    // Hide the leather base board, not the separate file slots or Items
    // shelf or Gear equipment. Map keeps its complete native background.
    if (!IsFilePresentationActive() && !InputRemap_IsItemsPageActive() &&
        !InputRemap_IsGearPageActive() &&
        !InputRemap_IsOcarinaUiOpen())
        draw(node);
}

static void DrawOcarinaPanel(void) {
    DrawSecondaryUi();
    DrawSecondaryWidgets();
    if (sSecondaryCallback != 0)
        sSecondaryCallback(sSecondaryCallbackContext);
}

void SingleScreen_BeforeTopOverlay(void) {
    if (UnifiedMenu_IsClosing() || UnifiedMenu_IsOpening()) {
        SetViewport(0, 0, 480, 400);
        SetScissor(1, 0, 0, 479, 399);
        return;
    }
    if (UnifiedMenu_IsOptionsDialog()) {
        // Native help occupies the bottom of the top-screen pass. Scale
        // both its text and panel together into the space above the footer.
        SetViewport(58, 60, 336, 280);
        SetScissor(1, 320, 0, 479, 399);
        return;
    }
    if (UnifiedMenu_GetDialogPage()) {
        /* Save has no item preview/help. Exclude the whole presentation
         * pass; the dialog replay below restores its own scissor. */
        SetViewport(0, 0, 480, 400);
        SetScissor(1, 0, 0, 479, 399);
        return;
    }
    if (IsFilePresentationActive()) {
        // Restore the late file-selection overlay viewport after the logo.
        SetViewport(48, 0, 384, 180);
    } else if (InputRemap_IsItemsMenuOpen()) {
        // The selected-item presentation is submitted in the final native
        // top-screen overlay group. Restrict that group to the lower-left
        // quarter before its draw commands are emitted.
        if (ApplyPrototypePreviewViewport()) return;
        SetViewport(56, InputRemap_IsItemsPageActive() ? 0 : -10,
                    InputRemap_IsItemsPageActive() ? 264 : 240,
                    InputRemap_IsItemsPageActive() ? 220 : 180);
    }
}

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
void SingleScreen_DrawPauseDim(void* context, u32 kind, const float* color, u32 flags) {
    /* Pause-specific color submission in 41E944 (black, alpha 0.7).
     * Keep unrelated scene fades intact. Match the menu's opening readiness
     * as well as its closing gate: no dim-only frame before the HUD exists. */
    const float clear[4] = { 0, 0, 0, 0 };
    /* Keep full-screen dimming separate from the inset stone panel.
     * Preserve the original dim for native dialogs, lighten main menus. */
    const float menuDim[4] = { 0, 0, 0, 0.45f };
    const float* settled = InputRemap_IsItemsMenuOpen() &&
        !UnifiedMenu_GetDialogPage() ? menuDim : color;
    ((void (*)(void*, u32, const float*, u32))0x003339E8)
        (context, kind, (UnifiedMenu_IsClosing() || UnifiedMenu_IsOpening()) ?
            clear : settled, flags);
}
#endif

void SingleScreen_BeforeTopPresentation(void) {
    UnifiedMenu_DrawBackdrop();
    UnifiedMenu_DrawDollBackdrop();
    if (UnifiedMenu_IsClosing() || UnifiedMenu_IsOpening()) {
        SetViewport(0, 0, 480, 400);
        SetScissor(1, 0, 0, 479, 399);
        return;
    }
    if (UnifiedMenu_IsOptionsDialog()) {
        SetViewport(58, 60, 336, 280);
        SetScissor(1, 320, 0, 479, 399);
        return;
    }
    if (UnifiedMenu_GetDialogPage()) {
        SetViewport(0, 0, 480, 400);
        SetScissor(1, 0, 0, 479, 399);
        return;
    }
    if (IsFilePresentationActive()) {
        SetViewport(290, 114, 324, 324);
    } else if (InputRemap_IsItemsMenuOpen()) {
        // The top HUD manager owns the rotating selected-item presentation.
        // Its title board is temporarily restored to the full viewport by the
        // paired menu-title hooks below.
        if (ApplyPrototypePreviewViewport()) return;
        SetViewport(56, InputRemap_IsItemsPageActive() ? 0 : -10,
                    InputRemap_IsItemsPageActive() ? 264 : 240,
                    InputRemap_IsItemsPageActive() ? 220 : 180);
    }
}

void SingleScreen_BeforeMenuTitle(void) {
    if (InputRemap_IsItemsMenuOpen()) {
        // Items is cropped narrower than Gear/Map. Move just its heading
        // left, with the same small right-edge inset as Gear/Map.
        SetViewport(0, InputRemap_IsItemsPageActive() ? 58 : 0, 480, 400);
    }
}

void SingleScreen_DrawMenuTitle(void* node, void (*draw)(void*)) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    if (InputRemap_IsItemsMenuOpen() || UnifiedMenu_GetDialogPage()) return;
#endif
    draw(node);
}

void SingleScreen_AfterMenuTitleBoard(void) {
    // The controller diagram shares the hook, but must not follow the title.
    if (InputRemap_IsItemsMenuOpen()) SetViewport(0, 0, 480, 400);
}

void SingleScreen_AfterMenuTitle(void) {
    if (InputRemap_IsItemsMenuOpen()) {
        if (ApplyPrototypePreviewViewport()) return;
        SetViewport(40, InputRemap_IsItemsPageActive() ? 0 : -10,
                    InputRemap_IsItemsPageActive() ? 264 : 240,
                    InputRemap_IsItemsPageActive() ? 220 : 180);
    }
}

void SingleScreen_DrawSecondaryCallback(void* context,
                                        void (*callback)(void*)) {
    // The active lower-screen interface supplies its foreground through this
    // callback. Retain both values so the top pass can replay the exact live
    // screen, including the ocarina song grid and note controls.
    sSecondaryCallbackContext = context;
    sSecondaryCallback = (DrawSecondaryCallbackFn)callback;
    if (sSecondaryCallback != 0) {
        sSecondaryCallback(context);
    }
}

static void DrawFileSelectionUi(void) {
    DrawSecondaryUi();
    DrawSecondaryWidgets();
    if (sSecondaryCallback != 0) {
        sSecondaryCallback(sSecondaryCallbackContext);
    }
}

static u8 DrawAuxiliaryUi(void* pass) {
    void* hint = sHintContext;
    sHintContext = 0;
    if (!hint && !IsNameEntryActive()) return 0;
    SetScissor(0, 0, 0, 479, 399);
    if (hint) {
#if defined(Version_USA) || defined(Version_EUR) || defined(Version_JP)
        // Lower Visions list and queued controls, beside the native video.
        SetViewport(90, 0, 300, 200);
        ((DrawSecondaryCallbackFn)ADDR(0x00477E10))(hint);
        ((FinishTopPassFn)DRAW_TITLE_ADDR)(pass);
#endif
    } else {
        // Keep the full keyboard, entered name, and confirmation controls.
        SetViewport(0, 40, 480, 320);
        DrawSecondaryUi();
        DrawSecondaryWidgets();
    }
    SetViewport(0, 0, 480, 400);
    return 1;
}

void SingleScreen_AfterTopPass(void* pass) {
    GearPreview_TracePresentation(pass);
    FinishTopPass(pass);
    if (DrawAuxiliaryUi(pass)) return;

    if (gSaveContext.gameMode == 1 && pass != 0) {
        // Replay the native choices in opposite bottom corners. The native
        // list and its input/selection state remain unchanged after replay.
        u32* count = (u32*)((u8*)pass + 0x2c);
        void** nodes = (void**)((u8*)pass + 0x454);
        void* saved[24];
        const u32 originalCount = *count;
        if (originalCount == 0 || originalCount > 24) return;
        // The leading fade node disappears before its duplicate at the end
        // of the list. Never use nodes[0] as the resource-table origin: that
        // shifts the filter by one entry and briefly replays the black fade.
        // Entry six is the stable native heading at (160,24), including while
        // its glyphs are clipped out of our compact title presentation.
        u32 backdropResource = 0;
        for (u32 i = 0; i < originalCount; ++i) {
            if (((u32*)nodes[i])[0] == BOARD_VTABLE_ADDR &&
                ((float*)nodes[i])[15] == 160.0f &&
                ((float*)nodes[i])[16] == 24.0f) {
                backdropResource = ((u32*)nodes[i])[1] - 6 * 0x1c8;
                break;
            }
        }
        // Do not replay an incompletely initialized title board.
        if (backdropResource == 0) return;
        u8 showQuestChoices = 0;
        u32 replayCount = 0;
        for (u32 i = 0; i < originalCount; ++i) {
            saved[i] = nodes[i];
            // USA title resources have 0x1c8-byte entries. Entry two is
            // the first quest card; offscreen at x=403 during Press Start.
            if (((u32*)nodes[i])[1] == backdropResource + 2 * 0x1c8 &&
                ((float*)nodes[i])[15] < 320.0f) showQuestChoices = 1;
        }
        // The title submits a base board plus the next resource's opening
        // leather fade. Suppress both, including duplicated base nodes, so
        // Press Start is transparent from its first frame, not just settled.
        for (u32 i = 0; i < originalCount; ++i) {
            const u32 resource = ((u32*)saved[i])[1];
            if (resource != backdropResource &&
                resource != backdropResource + 0x1c8)
                nodes[replayCount++] = saved[i];
        }
        *count = replayCount;
        // Center Press Start in both axes; quest choices keep their corners.
        // On the rotated surface viewport width controls display height.
        // Raise only the quest cards by ~35%, keeping their horizontal size
        // and bottom anchor. Grow the matching clips to avoid cutting them.
        SetViewport(showQuestChoices ? 8 : 175,
                    showQuestChoices ? 244 : 124,
                    showQuestChoices ? 178 : 132, 150);
        // The rotated surface's Y axis runs right-to-left. Split the two
        // native cards at their midpoint; do not split the Press Start text.
        if (showQuestChoices)
            SetScissor(3, 8, 319, 151, 393);
        else
            SetScissor(3, 175, 124, 281, 273);
        ((void (*)(void*))DRAW_TITLE_ADDR)(pass);
        if (showQuestChoices) {
            SetViewport(8, 6, 178, 150);
            SetScissor(3, 8, 6, 151, 81);
            ((void (*)(void*))DRAW_TITLE_ADDR)(pass);
        }
        SetScissor(0, 0, 0, 479, 399);
        for (u32 i = 0; i < originalCount; ++i) nodes[i] = saved[i];
        *count = originalCount;
        SetViewport(0, 0, 480, 400);
        return;
    }

    const u8 showFileSelect = IsFilePresentationActive();

    if (InputRemap_IsGameOverUiOpen()) {
        // Replay only the dedicated death dialogue. The generic secondary
        // passes add the leather board and unrelated Save guidance on top of
        // the native primary-screen Game Over presentation.
        SetScissor(0, 0, 0, 479, 399);
        // The previous full-height choices were about 1.5x the requested
        // size. Use two-thirds scale, centered on the same screen midpoint.
        SetViewport(80, 93, 320, 214);
        // Dedicated native regional death-menu renderer and menu pointer.
        extern GlobalContext* gGlobalContext;
        void* const deathMenu = *(void* volatile*)0x005C588C;
        if (deathMenu != 0 && gGlobalContext != 0) {
            ((void (*)(void*, GlobalContext*))0x0041C4DC)(
                deathMenu, gGlobalContext);
        }
        SetViewport(0, 0, 480, 400);
        return;
    }

    if (!InputRemap_IsItemsMenuOpen() &&
        !InputRemap_IsOcarinaUiOpen() &&
        !InputRemap_IsSaveMenuOpen() &&
        !showFileSelect) {
        sSecondaryCallbackContext = 0;
        sSecondaryCallback = 0;
        return;
    }

    if (InputRemap_IsItemsMenuOpen()) {
        if (UnifiedMenu_IsOpening()) return;
        // Preserve the complete native Items interface at 75% scale and
        // anchor it to the left. The uncovered right column retains the
        // game's original top-screen item presentation; the native HUD board
        // supplies the live controller diagram above it.
        //
        // OoT3D's presentation surface is rotated: the first coordinate and
        // width control vertical placement/size, while the second coordinate
        // and height control horizontal placement/size.
        SetViewport(60, 160, 360, 240);
        ApplyPrototypeItemsViewport(gMenuItemsSurface);
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        MenuViewport pageViewport;
        const u8 dungeonMap = !InputRemap_IsItemsPageActive() &&
            !InputRemap_IsGearPageActive() && ((int (*)(void))0x002F1268)();
        if (MenuLayout_Viewport(InputRemap_IsItemsPageActive() ?
                               gMenuItemsSurface : InputRemap_IsGearPageActive() ?
                               gMenuGearSurface : dungeonMap ? gMenuDungeonSurface :
                               gMenuWideSurface, &pageViewport))
            SetViewport(pageViewport.x, pageViewport.y,
                        pageViewport.width, pageViewport.height);
#endif
        // Crop away the native I/II assignment strip while retaining the
        // complete item grid and lower navigation. Exclude the assignment
        // column's remaining silver edge as well as its contents.
        const u8 itemsPageActive = InputRemap_IsItemsPageActive();
        if (itemsPageActive) {
            SetScissor(3, 60, 210, 419, 399);
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
            MenuScissor clip;
            if (MenuLayout_Scissor(gMenuItemsClip, &clip))
                SetScissor(3, clip.left, clip.top, clip.right, clip.bottom);
#endif
        }
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        if (!itemsPageActive) {
            MenuScissor clip;
            const MenuRect pageClip = InputRemap_IsGearPageActive() ? gMenuGearClip :
                dungeonMap ? gMenuDungeonSurface : gMenuWideClip;
            if (MenuLayout_Scissor(pageClip, &clip))
                SetScissor(3,clip.left,clip.top,clip.right,clip.bottom);
        }
#endif
        UnifiedMenu_RestoreItemsBoard();
        UnifiedMenu_PrepareDungeonMaterials();
        DrawSecondaryUi();
        UnifiedMenu_DrawDungeonFooterMask();
        if (itemsPageActive || UNIFIED_MENU_PROTOTYPE) {
            SetScissor(0, 0, 0, 479, 399);
        }
        UnifiedMenu_DrawHeader();
    } else if (InputRemap_IsOcarinaUiOpen()) {
        // Keep the live scene visible while the native ocarina initializes
        // and changes to Songs. Never replay Map or the intermediate layout.
        if (!InputRemap_IsOcarinaUiReady()) return;
        // Keep gameplay visible while playing the ocarina. Half-sized width
        // and height occupy one quarter of the screen area at bottom left.
        // OoT3D's presentation surface is rotated: X moves the viewport
        // vertically and Y moves it horizontally.
        SetViewport(0, 240, 240, 160);
        // Keep the song grid and the two corner controls below. Replay the
        // reference staff and caption directly above this grid; the live
        // input staff is positioned separately by ApplyDialogueViewport.
        SetScissor(3, 38, 240, 169, 399);
        DrawOcarinaPanel();
        SetScissor(3, 0, 368, 37, 399);
        DrawOcarinaPanel();
        SetScissor(3, 0, 240, 37, 268);
        DrawOcarinaPanel();
        // Selected-song reference: same scale, aligned with the song grid.
        SetViewport(46, 240, 240, 160);
        SetScissor(3, 218, 240, 285, 399);
        DrawOcarinaPanel();
        // Song title sits between its reference notes and the song buttons.
        SetViewport(172, 240, 240, 160);
        SetScissor(3, 172, 270, 209, 367);
        DrawOcarinaPanel();
        SetScissor(0, 0, 0, 479, 399);
    } else if (InputRemap_IsSaveMenuOpen()) {
        // Keep the save/options interface above top-screen tutorial text.
        // OoT3D's presentation surface is rotated, so the first coordinate
        // moves the interface vertically. This is slightly smaller than the
        // ocarina panel and shifted toward the top of the screen.
        SetViewport(184, 130, 210, 140);
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        if (UnifiedMenu_GetDialogPage()) {
            // Keep the native dialog scale until its internal clip and text
            // passes can be transformed together.
            // Shared frame supplies Back at the normal bottom-left position.
            if (UnifiedMenu_IsOptionsDialog())
                SetScissor(3, 184, 0, 218, 399);
            else
                SetScissor(1, 184, 0, 218, 399);
        }
#endif
        DrawSecondaryUi();
        DrawSecondaryWidgets();
        if (sSecondaryCallback != 0) {
            sSecondaryCallback(sSecondaryCallbackContext);
        }
        SetScissor(0, 0, 0, 479, 399);
        UnifiedMenu_DrawHeader();
    } else if (showFileSelect) {
        // Enlarge the complete interactive file interface to 56% of the
        // display width and 70% of its height, with a 3% left margin. Keep
        // every native file operation inside the same readable presentation.
        SetViewport(56, 164, 336, 224);
        // Back shares the file board with other native controls. Exclude its
        // original corner, then replay only that corner against the display's
        // bottom-left edge at the same size. This moves the full live button,
        // including its label/highlight, without touching the shared assets.
#if defined(Version_JP) || defined(Version_TWN)
        SetScissor(1, 56, 335, 110, 399);
#else
        // Include the filtered border around the original Back button.
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        // Keep the full warning frame during confirmation. Once deletion
        // finishes, return Back to its normal bottom-left replay position.
        const u32 fileState = *(volatile u32*)0x00504FB0;
        const u8 replayBack = fileState <= 10 || fileState >= 43;
        if (!replayBack)
            SetScissor(0, 0, 0, 479, 399);
        else
#endif
        SetScissor(1, 54, 346, 109, 399);
#endif
        DrawFileSelectionUi();
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        if (replayBack) {
#endif
            SetViewport(0, 176, 336, 224);
            SetScissor(3, 0, 360, 51, 399);
            DrawFileSelectionUi();
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        }
#endif
        SetScissor(0, 0, 0, 479, 399);
    }
    SetViewport(0, 0, 480, 400);
#if CONTROLLER_OPTIONS_ENABLED
    // Draw the Reframed page after the native secondary interface has been
    // replayed so the game's GPU pass cannot cover it on single-screen setups.
    ControllerSettings_Draw();
    if (InputRemap_IsSaveMenuOpen() || ControllerSettings_IsOpen()) {
        Draw_FlushFramebufferTop();
    }
#endif
}

#else

void SingleScreen_DrawSecondaryCallback(void* context,
                                        void (*callback)(void*)) {
    (void)context;
    (void)callback;
}

void SingleScreen_AfterTopPass(void* pass) {
    (void)pass;
}

void SingleScreen_BeforeTopOverlay(void) {
}

void SingleScreen_BeforeTopPresentation(void) {
}

void SingleScreen_BeforeMenuTitle(void) {
}

void SingleScreen_AfterMenuTitle(void) {
}

#endif
