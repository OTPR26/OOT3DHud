#if defined(Version_TWN)
#include "common.h"

// Preserve the native top-pass call, then draw the existing HUD node only
// during the instrument scene, where Taiwan omits its normal HUD pass.
// No secondary-screen replay or viewport changes belong in Dual Screen.
void TaiwanDual_AfterTopPass(void* pass) {
    ((void (*)(void*))0x0010C744)(pass);
    if (!IsInGameOrBossChallenge() || !*(volatile unsigned*)0x005144E4)
        return;
    void* controls = *(void**)0x00508AA8;
    if (controls) {
        void** vtable = *(void***)controls;
        ((void (*)(void*))vtable[3])(controls);
    }
}
#endif
