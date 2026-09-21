#include "hud.h"

// Ocarina Reframed renders through native_hud.c. Keep this no-op entry point
// for source compatibility with integrations built on the original loader.
void Hud_Draw(GlobalContext* globalCtx) {
    (void)globalCtx;
}
