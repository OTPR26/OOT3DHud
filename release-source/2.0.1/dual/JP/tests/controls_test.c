#include <assert.h>
#include <stdio.h>

#include "ab_swap.h"
#include "controls.h"
#include "controller_settings.h"
#include "hid.h"
#include "input_consume.h"

static void expect(uint32_t held, uint32_t pressed, ControlAction action) {
    assert(Controls_Resolve(held, pressed) == action);
}

int main(void) {
    assert(ABSwap_Bits(0) == 0);
    assert(ABSwap_Bits(BUTTON_A) == BUTTON_B);
    assert(ABSwap_Bits(BUTTON_B) == BUTTON_A);
    assert(ABSwap_Bits(BUTTON_A | BUTTON_B) == (BUTTON_A | BUTTON_B));
    assert(ABSwap_Bits(BUTTON_A | BUTTON_X) == (BUTTON_B | BUTTON_X));

    expect(BUTTON_LEFT, BUTTON_LEFT, CONTROL_ACTION_ITEM_I);
    expect(BUTTON_DOWN, BUTTON_DOWN, CONTROL_ACTION_ITEM_II);
    expect(BUTTON_UP, BUTTON_UP, CONTROL_ACTION_NAVI);
    expect(BUTTON_RIGHT, BUTTON_RIGHT, CONTROL_ACTION_OCARINA);
    expect(BUTTON_SELECT, BUTTON_SELECT, CONTROL_ACTION_ITEMS_MENU);
    expect(BUTTON_SELECT, 0, CONTROL_ACTION_NONE);
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
    expect(BUTTON_L1 | BUTTON_R1 | BUTTON_SELECT, BUTTON_SELECT,
           CONTROL_ACTION_NONE);
    expect(BUTTON_L1 | BUTTON_LEFT, BUTTON_LEFT, CONTROL_ACTION_ITEM_I);
    expect(0, 0, CONTROL_ACTION_NONE);

    assert(Controls_IsHudScaleHold(BUTTON_L1 | BUTTON_R1, IRRST_BUTTON_ZR));
    assert(!Controls_IsHudScaleHold(BUTTON_L1 | BUTTON_R1, 0));
    assert(!Controls_IsHudScaleHold(BUTTON_L1, IRRST_BUTTON_ZR));
    assert(!Controls_IsHudScaleHold(BUTTON_L1 | BUTTON_R1 | BUTTON_UP,
                                    IRRST_BUTTON_ZR));
    assert(!Controls_IsHudScaleHold(BUTTON_L1 | BUTTON_R1,
                                    IRRST_BUTTON_ZL | IRRST_BUTTON_ZR));

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

    InputRemap_ReplaceButtonMasks(&pad, &held, &pressed, &released,
                                  BUTTON_SELECT, BUTTON_B);
    assert((pad.curr.val & BUTTON_SELECT) == 0);
    assert((pad.pressed.val & BUTTON_SELECT) == 0);
    assert((held & BUTTON_SELECT) == 0);
    assert((pressed & BUTTON_SELECT) == 0);
    assert((pad.curr.val & BUTTON_B) != 0);
    assert((pad.pressed.val & BUTTON_B) != 0);
    assert((held & BUTTON_B) != 0);
    assert((pressed & BUTTON_B) != 0);

    assert(ControllerSettings_SwapABBits(0) == 0);
    assert(ControllerSettings_SwapABBits(BUTTON_A) == BUTTON_B);
    assert(ControllerSettings_SwapABBits(BUTTON_B) == BUTTON_A);
    assert(ControllerSettings_SwapABBits(BUTTON_A | BUTTON_B) ==
           (BUTTON_A | BUTTON_B));
    assert(ControllerSettings_SwapABBits(BUTTON_A | BUTTON_X) ==
           (BUTTON_B | BUTTON_X));

    assert(!ControllerSettings_IsOpen());
    ControllerSettings_Open();
    assert(ControllerSettings_IsOpen());
    assert(ControllerSettings_HandleOptionsInput(BUTTON_RIGHT));
    assert(ControllerSettings_HandleOptionsInput(BUTTON_DOWN));
    assert(ControllerSettings_HandleOptionsInput(BUTTON_RIGHT));
    assert(ControllerSettings_HandleOptionsInput(BUTTON_B));
    assert(!ControllerSettings_IsOpen());

    puts("controls_test: all mappings passed");
    return 0;
}
