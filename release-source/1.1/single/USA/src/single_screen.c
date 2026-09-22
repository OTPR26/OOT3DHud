#include "single_screen.h"

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

static u8 IsFilePresentationActive(void) {
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
    // Align the pedestal bottom with the bottom file slot. Move the caption
    // by the same amount below, preserving the model-to-caption spacing.
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
    // This callback owns only the guidance glyphs (foreground and shadow).
    // The logo is drawn by the following native pass, not this callback.
    if (IsFilePresentationActive()) {
        // The native file-list caption is enabled only in presentation 2.
        sFileGuidanceVisible = context != 0 &&
            *(u32*)((u8*)context + 0x890) == 2;
        SetViewport(39, 14, 220, 160);
    }
    callback(context);
}

void SingleScreen_BeforeFileBackdrop(void) {
    // Native pass 3 precedes the text. Keep its rounded translucent board
    // in precisely the same viewport as the caption below Link.
    if (IsFilePresentationActive()) SetViewport(39, 14, 220, 160);
}

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

void SingleScreen_UpdateFileBoard(void* board) {
    ((void (*)(void*))UPDATE_FILE_BOARD_ADDR)(board);
    if (!IsFilePresentationActive() || board == 0) return;
    const u32 count = *(u32*)board;
    float* positions = *(float**)((u8*)board + 0x10);
    if (positions == 0 || count != 184) return;
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
            p[v * 3] = 160.0f + (p[v * 3] - 160.0f) * 0.60f;
#endif
            p[v * 3 + 1] = 24.0f + (p[v * 3 + 1] - 24.0f) * 0.70f;
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
    if (IsFilePresentationActive()) {
        // Restore the late file-selection overlay viewport after the logo.
        SetViewport(48, 0, 384, 180);
    } else if (InputRemap_IsItemsMenuOpen()) {
        // The selected-item presentation is submitted in the final native
        // top-screen overlay group. Restrict that group to the lower-left
        // quarter before its draw commands are emitted.
        SetViewport(56, InputRemap_IsItemsPageActive() ? 0 : -10,
                    InputRemap_IsItemsPageActive() ? 264 : 240,
                    InputRemap_IsItemsPageActive() ? 220 : 180);
    }
}

void SingleScreen_BeforeTopPresentation(void) {
    if (IsFilePresentationActive()) {
        SetViewport(290, 114, 324, 324);
    } else if (InputRemap_IsItemsMenuOpen()) {
        // The top HUD manager owns the rotating selected-item presentation.
        // Its title board is temporarily restored to the full viewport by the
        // paired menu-title hooks below.
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

void SingleScreen_AfterMenuTitleBoard(void) {
    // The controller diagram shares the hook, but must not follow the title.
    if (InputRemap_IsItemsMenuOpen()) SetViewport(0, 0, 480, 400);
}

void SingleScreen_AfterMenuTitle(void) {
    if (InputRemap_IsItemsMenuOpen()) {
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

void SingleScreen_AfterTopPass(void* pass) {
    FinishTopPass(pass);

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
        SetViewport(showQuestChoices ? 8 : 175,
                    showQuestChoices ? 244 : 124, 132, 150);
        // The rotated surface's Y axis runs right-to-left. Split the two
        // native cards at their midpoint; do not split the Press Start text.
        if (showQuestChoices)
            SetScissor(3, 8, 319, 114, 393);
        else
            SetScissor(3, 175, 124, 281, 273);
        ((void (*)(void*))DRAW_TITLE_ADDR)(pass);
        if (showQuestChoices) {
            SetViewport(8, 6, 132, 150);
            SetScissor(3, 8, 6, 114, 81);
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
        // Preserve the complete native Items interface at 75% scale and
        // anchor it to the left. The uncovered right column retains the
        // game's original top-screen item presentation; the native HUD board
        // supplies the live controller diagram above it.
        //
        // OoT3D's presentation surface is rotated: the first coordinate and
        // width control vertical placement/size, while the second coordinate
        // and height control horizontal placement/size.
        SetViewport(60, 160, 360, 240);
        // Crop away the native I/II assignment strip while retaining the
        // complete item grid and lower navigation. Exclude the assignment
        // column's remaining silver edge as well as its contents.
        const u8 itemsPageActive = InputRemap_IsItemsPageActive();
        if (itemsPageActive) {
            SetScissor(3, 60, 210, 419, 399);
        }
        DrawSecondaryUi();
        if (itemsPageActive) {
            SetScissor(0, 0, 0, 479, 399);
        }
    } else if (InputRemap_IsOcarinaUiOpen()) {
        // Keep the live scene visible while the native ocarina initializes
        // and changes to Songs. Never replay Map or the intermediate layout.
        if (!InputRemap_IsOcarinaUiReady()) return;
        // Keep gameplay visible while playing the ocarina. Half-sized width
        // and height occupy one quarter of the screen area at bottom left.
        // OoT3D's presentation surface is rotated: X moves the viewport
        // vertically and Y moves it horizontally.
        SetViewport(0, 240, 240, 160);
        // Keep the song grid and the two corner controls below. The staff
        // and selected-song caption are replayed separately at the top.
        SetScissor(3, 38, 240, 169, 399);
        DrawOcarinaPanel();
        SetScissor(3, 0, 368, 37, 399);
        DrawOcarinaPanel();
        SetScissor(3, 0, 240, 37, 268);
        DrawOcarinaPanel();
        // Center the native live notation between the hearts and diamond.
        SetViewport(224, 92, 240, 160);
        SetScissor(3, 396, 92, 463, 251);
        DrawOcarinaPanel();
        // Same horizontal center, with the title just below the notation.
        SetViewport(350, 92, 240, 160);
        SetScissor(3, 350, 122, 387, 219);
        DrawOcarinaPanel();
        SetScissor(0, 0, 0, 479, 399);
    } else if (InputRemap_IsSaveMenuOpen()) {
        // Keep the save/options interface above top-screen tutorial text.
        // OoT3D's presentation surface is rotated, so the first coordinate
        // moves the interface vertically. This is slightly smaller than the
        // ocarina panel and shifted toward the top of the screen.
        SetViewport(184, 130, 210, 140);
        DrawSecondaryUi();
        DrawSecondaryWidgets();
        if (sSecondaryCallback != 0) {
            sSecondaryCallback(sSecondaryCallbackContext);
        }
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
        SetScissor(1, 54, 346, 109, 399);
#endif
        DrawFileSelectionUi();
        SetViewport(0, 176, 336, 224);
        SetScissor(3, 0, 360, 51, 399);
        DrawFileSelectionUi();
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
