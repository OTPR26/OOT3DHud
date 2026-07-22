#include "z3D/z3D.h"
#include "3ds/srv.h"
#include "3ds/services/irrst.h"
#include "camera.h"
#include "draw.h"
#include "input.h"
#include "input_remap.h"
#include "native_hud.h"

extern GlobalContext* gGlobalContext;

static void Project_Init(void) {
    static u8 initialized = 0;
    if (initialized) {
        return;
    }

    srvInit();
    irrstInit();
    Draw_SetupFramebuffer();
    initialized = 1;
}

void before_GlobalContext_Update(GlobalContext* globalCtx) {
    Project_Init();
    gGlobalContext = globalCtx;
    Input_Update();

    // Inject shortcuts immediately before OoT3D samples the current input.
    InputRemap_Update(globalCtx);
}

void after_GlobalContext_Update(GlobalContext* globalCtx) {
    NativeHud_Update(globalCtx);
    Camera_DrawSettingsOverlay();
}
