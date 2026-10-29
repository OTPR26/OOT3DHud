#include <assert.h>
#include "ocarina_presentation.h"

int main(void) {
    unsigned active=0;
    /* Ordinary dialogue must never acquire the ocarina placement. */
    assert(!OcarinaPresentation_Update(active,1,0,1));
    active=OcarinaPresentation_Update(active,1,1,0);
    assert(active); /* Opening, before a native staff is displayed. */
    active=OcarinaPresentation_Update(active,1,1,1);
    assert(active); /* Native contextual staff / song result. */
    for (unsigned frame=0; frame<120; ++frame) {
        active=OcarinaPresentation_Update(active,1,0,1);
        assert(active); /* B dismisses menu; native result still fading. */
    }
    active=OcarinaPresentation_Update(active,1,0,0);
    assert(!active);
    assert(!OcarinaPresentation_Update(active,1,0,1)); /* Next NPC. */
    assert(!OcarinaPresentation_Update(1,0,1,1)); /* Death/pause/title. */
    assert(!OcarinaPresentation_Update(1,1,0,0)); /* Cancel without staff. */
    return 0;
}
