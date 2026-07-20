#include <assert.h>
#include <stdio.h>

#include "controls.h"
#include "hid.h"
#include "input_consume.h"

static void expect(uint32_t held, uint32_t pressed, ControlAction action) {
    const ControlDecision decision = Controls_Resolve(held, pressed);
    assert(decision.action == action);
}

int main(void) {
    expect(BUTTON_LEFT, BUTTON_LEFT, CONTROL_ACTION_ITEM_I);
    expect(BUTTON_DOWN, BUTTON_DOWN, CONTROL_ACTION_ITEM_II);
    expect(BUTTON_UP, BUTTON_UP, CONTROL_ACTION_NAVI);
    expect(BUTTON_RIGHT, BUTTON_RIGHT, CONTROL_ACTION_OCARINA);
    expect(BUTTON_SELECT, BUTTON_SELECT, CONTROL_ACTION_ITEMS_MENU);

    // Opening Items is edge-triggered. Holding Select must not repeatedly
    // synthesize touches or reopen the screen.
    expect(BUTTON_SELECT, 0, CONTROL_ACTION_NONE);

    // Gameplay actions stay active while held so bows, hookshot, bottles, and
    // other hold-sensitive vanilla items receive a continuous virtual touch.
    expect(BUTTON_LEFT, 0, CONTROL_ACTION_ITEM_I);
    expect(BUTTON_DOWN, 0, CONTROL_ACTION_ITEM_II);

    expect(BUTTON_L1 | BUTTON_R1 | BUTTON_UP, BUTTON_UP,
           CONTROL_ACTION_CAMERA_SENSITIVITY_UP);
    expect(BUTTON_L1 | BUTTON_R1 | BUTTON_DOWN, BUTTON_DOWN,
           CONTROL_ACTION_CAMERA_SENSITIVITY_DOWN);
    expect(BUTTON_L1 | BUTTON_R1 | BUTTON_LEFT, BUTTON_LEFT,
           CONTROL_ACTION_CAMERA_INVERT_PREVIOUS);
    expect(BUTTON_L1 | BUTTON_R1 | BUTTON_RIGHT, BUTTON_RIGHT,
           CONTROL_ACTION_CAMERA_INVERT_NEXT);

    // A partial chord must not steal the gameplay binding.
    expect(BUTTON_L1 | BUTTON_LEFT, BUTTON_LEFT, CONTROL_ACTION_ITEM_I);
    expect(BUTTON_L1 | BUTTON_R1 | BUTTON_SELECT, BUTTON_SELECT,
           CONTROL_ACTION_NONE);
    expect(0, 0, CONTROL_ACTION_NONE);

    // Consuming Select must remove it from every source and sampled mask. If
    // the pressed mask survives, vanilla OoT3D opens its save prompt even
    // though the mod also injects the native Items touch target.
    pad_t pad = { 0 };
    uint32_t held = BUTTON_SELECT | BUTTON_A;
    uint32_t pressed = BUTTON_SELECT | BUTTON_B;
    uint32_t released = BUTTON_SELECT | BUTTON_X;
    pad.curr.val = BUTTON_SELECT | BUTTON_A;
    pad.pressed.val = BUTTON_SELECT | BUTTON_B;
    pad.released.val = BUTTON_SELECT | BUTTON_Y;
    InputRemap_ClearButtonMasks(&pad, &held, &pressed, &released,
                                BUTTON_SELECT);
    assert((pad.curr.val & BUTTON_SELECT) == 0);
    assert((pad.pressed.val & BUTTON_SELECT) == 0);
    assert((pad.released.val & BUTTON_SELECT) == 0);
    assert((held & BUTTON_SELECT) == 0);
    assert((pressed & BUTTON_SELECT) == 0);
    assert((released & BUTTON_SELECT) == 0);
    assert((pad.curr.val & BUTTON_A) != 0);
    assert((pad.pressed.val & BUTTON_B) != 0);
    assert((pad.released.val & BUTTON_Y) != 0);
    assert((held & BUTTON_A) != 0);
    assert((pressed & BUTTON_B) != 0);
    assert((released & BUTTON_X) != 0);

    puts("controls_test: all mappings passed");
    return 0;
}
