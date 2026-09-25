#include <assert.h>
#include "gear_preview_rotation.h"

int main(void) {
    assert(GearPreview_UpdateYaw(1234, 156, 0, 1) == 0);
    assert(GearPreview_UpdateYaw(1234, 156, 1, 0) == 0);
    for (int x = -30; x <= 30; ++x)
        assert(GearPreview_UpdateYaw(1234, x, 1, 1) == 1234);
    assert(GearPreview_UpdateYaw(1000, 156, 1, 1) == 345);
    assert(GearPreview_UpdateYaw(1000, -156, 1, 1) == 1655);
    assert(GearPreview_UpdateYaw(1000, 32767, 1, 1) == 345);
    assert(GearPreview_UpdateYaw(1000, -32768, 1, 1) == 1655);
    assert(GearPreview_UpdateYaw(65535, -156, 1, 1) == 654);
    assert(GearPreview_UpdateYaw(0, 156, 1, 1) == 64881);
    assert(GearPreview_UpdateYaw(1000, 31, 1, 1) < 1000);
    assert(GearPreview_UpdateYaw(1000, -31, 1, 1) > 1000);
    for (int x = 31; x <= 156; ++x) {
        int expected = (x - 30) * 546 * 6 / (126 * 5);
        assert(GearPreview_UpdateYaw(1000, x, 1, 1) == 1000 - expected);
        assert(GearPreview_UpdateYaw(1000, -x, 1, 1) == 1000 + expected);
    }
    return 0;
}
