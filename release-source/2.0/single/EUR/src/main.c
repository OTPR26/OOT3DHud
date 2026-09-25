#include "z3D/z3D.h"
#include "3ds/srv.h"
#include "3ds/services/irrst.h"
#include "camera.h"
#if CONTROLLER_OPTIONS_ENABLED
#include "controller_settings.h"
#endif
#include "draw.h"
#include "input.h"
#include "input_remap.h"
#include "native_hud.h"
#include "native_mods_menu.h"
#include "single_screen.h"
#include "gear_preview.h"
#include "unified_menu.h"
#include "menu_preferences.h"
#include "memory_probe.h"

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
    MenuPreferences_Update(globalCtx);

    // Inject shortcuts immediately before OoT3D samples the current input.
    InputRemap_Update(globalCtx);
    GearPreview_Probe(globalCtx);
    MemoryProbe_Update(globalCtx);
}

void after_GlobalContext_Update(GlobalContext* globalCtx) {
    UnifiedMenu_TintShelves();
    NativeHud_Update(globalCtx);
    Camera_DrawSettingsOverlay();
}
