#if defined(Version_KOR)
#include "common.h"

// Korean native top-pass call and motion-node draw verified independently.
// Replay only the missing instrument HUD, not the secondary screen.
void KoreaDual_AfterTopPass(void* pass) {
    ((void (*)(void*))0x0010C66C)(pass);
    if (!IsInGameOrBossChallenge() || !*(volatile unsigned*)0x005144E4)
        return;
    void* controls = *(void**)0x00508AA8;
    if (controls) {
        void** vtable = *(void***)controls;
        ((void (*)(void*))vtable[3])(controls);
    }
}
#endif
