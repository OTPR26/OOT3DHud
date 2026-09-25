#include <assert.h>
#include "movement_options_model.h"
int main(void) {
    assert(MovementOptions_Scale(0) == 1.0f);
    assert(MovementOptions_Scale(1) > 1.099f && MovementOptions_Scale(1) < 1.101f);
    assert(MovementOptions_Scale(2) > 1.199f && MovementOptions_Scale(2) < 1.201f);
    assert(MovementOptions_Scale(3) == 1.0f);
    assert(MovementOptions_EndFrame(0) == 20.0f);
    assert(MovementOptions_EndFrame(1) == 19.5f);
    assert(MovementOptions_EndFrame(2) == 19.0f);
    assert(MovementOptions_EndFrame(99) == 20.0f);
    return 0;
}
