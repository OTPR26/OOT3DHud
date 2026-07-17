#include "z3D/z3D.h"
#include "3ds/srv.h"
#include "3ds/services/irrst.h"
#include "camera.h"
#include "draw.h"
#include "hud.h"
#include "input.h"
#include "input_remap.h"
#include "native_hud.h"

extern GlobalContext* gGlobalContext;

static void Project_Init(GlobalContext* globalCtx) {
    static u8 initialized = 0;
    if (initialized) {
        return;
    }

    srvInit();
    irrstInit();
    Draw_SetupFramebuffer();
    gGlobalContext = globalCtx;
    initialized = 1;
}

void before_GlobalContext_Update(GlobalContext* globalCtx) {
    Project_Init(globalCtx);
    gGlobalContext = globalCtx;
    Input_Update();

    // Keep synthetic touch injection adjacent to OoT3D's update. Textured HUD
    // alpha blending is intentionally restricted to the post-update hook so
    // the HID ring cannot advance during expensive pre-update drawing.
    InputRemap_Update(globalCtx);
}

void after_GlobalContext_Update(GlobalContext* globalCtx) {
#ifndef PLUS_CONTROLS_ONLY
#if !defined(PLUS_MINIMAL_HUD) && !defined(PLUS_NATIVE_MAGIC_STAGE) && \
    (!defined(PLUS_HEARTS_ONLY) || defined(PLUS_RUPEES_STAGE))
    Hud_Draw(globalCtx);
#endif
    NativeHud_UpdateProof(globalCtx);
#endif
    Camera_DrawSettingsOverlay();
}
