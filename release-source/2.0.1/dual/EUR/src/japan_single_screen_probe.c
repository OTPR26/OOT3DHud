// Opt-in Japanese pause/Songs test. Title/file selection remain native.
#if defined(Version_JP) && JP_SINGLE_SCREEN_PROBE
#include "input_remap.h"
#include "single_screen.h"
#include "z3D/z3D.h"
void SingleScreen_DrawFileLogoPass(void*);
void SingleScreen_DrawSecondaryBackdrop(void*, void (*)(void*));

typedef void (*Viewport)(unsigned, int, unsigned, int);
static const Viewport viewport = (Viewport)0x002FE5D4;
static void* secondaryContext;
static void (*secondaryDraw)(void*);

void JapanProbe_Callback(void* context, void (*draw)(void*)) {
    secondaryContext = context;
    secondaryDraw = draw;
    SingleScreen_DrawSecondaryCallback(context, draw);
}

static void DrawSongs(void) {
    ((void (*)(void))0x0041EC04)();
    ((void (*)(void))0x0041F2BC)();
    if (secondaryDraw) secondaryDraw(secondaryContext);
}

void JapanProbe_Position(void) {
    if (gSaveContext.gameMode == 2) {
        SingleScreen_BeforeTopPresentation();
        return;
    }
    if (!InputRemap_IsItemsMenuOpen()) return;
    if (InputRemap_IsItemsPageActive()) viewport(56, 0, 264, 220);
    else viewport(56, -10, 240, 180);
}

static void Scissor(unsigned mode, unsigned left, unsigned right,
                    unsigned top, unsigned bottom) {
    volatile unsigned** current = (volatile unsigned**)0x0054CC4C;
    volatile unsigned** end = (volatile unsigned**)0x0054CC50;
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

void JapanProbe_Backdrop(void* node, void (*draw)(void*)) {
    if (gSaveContext.gameMode == 2) {
        SingleScreen_DrawSecondaryBackdrop(node, draw);
        return;
    }
    if (!InputRemap_IsItemsPageActive() && !InputRemap_IsGearPageActive() &&
        !InputRemap_IsOcarinaUiOpen())
        draw(node);
}

void JapanProbe_Overlay(void* pass) {
    if (gSaveContext.gameMode == 2) {
        SingleScreen_DrawFileLogoPass(pass);
        SingleScreen_BeforeTopOverlay();
        return;
    }
    ((void (*)(void*))0x00422898)(pass);
    JapanProbe_Position();
}

void JapanProbe_Title(void* node, void (*draw)(void*)) {
    if (InputRemap_IsItemsMenuOpen())
        viewport(0, InputRemap_IsItemsPageActive() ? 58 : 0, 480, 400);
    draw(node);
    if (InputRemap_IsItemsMenuOpen()) {
        viewport(0, 0, 480, 400);
        // Native HUD manager +78, drawn through vtable+0c at 42BE30.
        void* controls = *(void**)0x004FC6C0;
        if (controls) {
            void** vtable = *(void***)controls;
            ((void (*)(void*))vtable[3])(controls);
        }
        if (InputRemap_IsItemsPageActive()) viewport(40, 0, 264, 220);
        else viewport(40, -10, 240, 180);
    }
}

void JapanProbe_AfterTop(void* pass) {
    if (gSaveContext.gameMode == 1 || gSaveContext.gameMode == 2) {
        SingleScreen_AfterTopPass(pass);
        return;
    }
    ((void (*)(void*))0x00422888)(pass);
    if (InputRemap_IsOcarinaUiOpen()) {
        if (!InputRemap_IsOcarinaUiReady()) return;
        viewport(0, 240, 240, 160);
        // Instrument controls are not the Songs grid. Replay that complete
        // panel once; the grid's staff/caption crops would duplicate slivers
        // of its ocarina and shoulder buttons above Link.
        if (*(volatile unsigned*)0x005093F8 == 12) {
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
    secondaryContext = 0;
    secondaryDraw = 0;
    if (!InputRemap_IsItemsMenuOpen()) return;
    viewport(60, 160, 360, 240);
    const unsigned items = InputRemap_IsItemsPageActive();
    if (items) Scissor(3, 60, 210, 419, 399);
    ((void (*)(void))0x0041EC04)();
    if (items) Scissor(0, 0, 0, 479, 399);
    viewport(0, 0, 480, 400);
}
#endif
