// Opt-in Korea Single Screen candidate; runtime validation pending.
// Viewport hooks are verified against the Korea executable, not USA offsets.
#if defined(Version_KOR) && KOR_SINGLE_SCREEN_PROBE
#include "input_remap.h"
#include "single_screen.h"
#include "z3D/z3D.h"
void SingleScreen_DrawFileLogoPass(void*);
void SingleScreen_DrawSecondaryBackdrop(void*, void (*)(void*));

typedef void (*Viewport)(unsigned int, int, unsigned int, int);
static const Viewport viewport = (Viewport)0x0031DBE4;
static void* secondaryContext;
static void (*secondaryCallback)(void*);

void KoreaProbe_SecondaryCallback(void* context, void (*draw)(void*)) {
    secondaryContext = context;
    secondaryCallback = draw;
    SingleScreen_DrawSecondaryCallback(context, draw);
}

static void DrawSongs(void) {
    ((void (*)(void))0x001088DC)();
    ((void (*)(void))0x00108F5C)();
    if (secondaryCallback) secondaryCallback(secondaryContext);
}

static void DrawControls(void) {
    // Verified Korea motion-control node, also used by the pause replay.
    // The native ocarina scene omits its normal HUD pass, so explicitly
    // replay this board before Songs changes the native projection to 320px.
    void* controls = *(void**)0x00508AA8;
    if (controls) {
        void** vtable = *(void***)controls;
        ((void (*)(void*))vtable[3])(controls);
    }
}

void KoreaProbe_PositionPresentation(void) {
    if (gSaveContext.gameMode == 2) {
        SingleScreen_BeforeTopPresentation();
        return;
    }
    if (!InputRemap_IsItemsMenuOpen()) return;
    if (InputRemap_IsItemsPageActive()) viewport(56, 0, 264, 220);
    else viewport(56, -10, 240, 180);
}

// Same GPU register protocol as USA; command pointers verified in Korea's
// viewport routine. Crop only the Items replay, never the native lower target.
static void Scissor(unsigned mode, unsigned left, unsigned right,
                    unsigned top, unsigned bottom) {
    volatile unsigned** current = (volatile unsigned**)0x0055ACD0;
    volatile unsigned** end = (volatile unsigned**)0x0055ACD4;
    volatile unsigned* command = *current;
    if (command + 6 > *end) return;
    command[0] = mode;
    command[1] = 0x000F0065;
    command[2] = (right << 16) | left;
    command[3] = 0x000F0066;
    command[4] = (bottom << 16) | top;
    command[5] = 0x000F0067;
    *current = command + 6;
}

void KoreaProbe_Backdrop(void* node, void (*draw)(void*)) {
    if (gSaveContext.gameMode == 2) {
        SingleScreen_DrawSecondaryBackdrop(node, draw);
        return;
    }
    if (!InputRemap_IsItemsPageActive() && !InputRemap_IsGearPageActive() &&
        !InputRemap_IsOcarinaUiOpen())
        draw(node);
}

void KoreaProbe_Overlay(void* pass) {
    if (gSaveContext.gameMode == 2) {
        SingleScreen_DrawFileLogoPass(pass);
        SingleScreen_BeforeTopOverlay();
        return;
    }
    ((void (*)(void*))0x0010C67C)(pass);
    KoreaProbe_PositionPresentation();
}

void KoreaProbe_Title(void* node, void (*draw)(void*)) {
    if (InputRemap_IsItemsMenuOpen())
        viewport(0, InputRemap_IsItemsPageActive() ? 58 : 0, 480, 400);
    draw(node);
    if (InputRemap_IsItemsMenuOpen()) {
        // Korea's native HUD manager draws its motion-control node at +78
        // (115944). Replay it at full scale after the relocated title.
        viewport(0, 0, 480, 400);
        DrawControls();
        if (InputRemap_IsItemsPageActive()) viewport(40, 0, 264, 220);
        else viewport(40, -10, 240, 180);
    }
}

void KoreaProbe_AfterTopPass(void* pass) {
    // Save and its nested Options/Continue dialogs use the shared replay.
    // Without this dispatch, Korea's gameplay-only path finishes the top
    // pass and returns before drawing any of those lower-screen controls.
    if (gSaveContext.gameMode == 1 || gSaveContext.gameMode == 2 ||
        InputRemap_IsSaveMenuOpen()) {
        SingleScreen_AfterTopPass(pass);
        return;
    }
    // Verified original BL at 0x31fe08, not the byte-voter's false match.
    ((void (*)(void*))0x0010C66C)(pass);
    if (InputRemap_IsOcarinaUiOpen()) {
        // Korea Songs manager, verified in 10F984's state dispatcher.
        // Wait for native Songs readiness, not a fixed frame delay.
        volatile unsigned* songs = (volatile unsigned*)0x005144D0;
        if (!InputRemap_IsOcarinaUiReady()) return;
        viewport(0, 0, 480, 400);
        DrawControls();
        viewport(0, 240, 240, 160);
        if (songs[0x14 / 4] == 12) {
            DrawSongs();
            viewport(0, 0, 480, 400);
            return;
        }
        Scissor(3, 38, 240, 169, 399);
        DrawSongs();
        Scissor(3, 0, 368, 37, 399);
        DrawSongs();
        Scissor(3, 0, 240, 37, 268);
        DrawSongs();
        viewport(224, 92, 240, 160);
        Scissor(3, 396, 92, 463, 251);
        DrawSongs();
        viewport(350, 92, 240, 160);
        Scissor(3, 350, 122, 387, 219);
        DrawSongs();
        Scissor(0, 0, 0, 479, 399);
        viewport(0, 0, 480, 400);
        return;
    }
    if (!InputRemap_IsItemsMenuOpen()) return;

    viewport(60, 160, 360, 240);
    const unsigned items = InputRemap_IsItemsPageActive();
    if (items) Scissor(3, 60, 210, 419, 399);
    ((void (*)(void))0x001088DC)();
    if (items) Scissor(0, 0, 0, 479, 399);
    viewport(0, 0, 480, 400);
}
#endif
