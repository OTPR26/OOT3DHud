#include "hud.h"

// The release HUD is rendered entirely by native_hud.c. This translation
// unit remains as a compatibility stub for downstream builds that still
// include hud.h, without linking the retired framebuffer prototype assets.
void Hud_Draw(GlobalContext* globalCtx) {
    (void)globalCtx;
}
