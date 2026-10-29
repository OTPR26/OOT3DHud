#include "controls.h"

#include "hid.h"

bool Controls_IsHudScaleHold(uint32_t held, uint32_t irrstHeld) {
    const uint32_t shoulderButtons = IRRST_BUTTON_ZL | IRRST_BUTTON_ZR;

    return held == (BUTTON_L1 | BUTTON_R1) &&
           (irrstHeld & shoulderButtons) == IRRST_BUTTON_ZR;
}

ControlAction Controls_Resolve(uint32_t held, uint32_t pressed) {
    const uint32_t configChord = BUTTON_L1 | BUTTON_R1;

    if ((held & configChord) == configChord) {
        if (pressed & BUTTON_UP) {
            return CONTROL_ACTION_CAMERA_SENSITIVITY_UP;
        }
        if (pressed & BUTTON_DOWN) {
            return CONTROL_ACTION_CAMERA_SENSITIVITY_DOWN;
        }
        if (pressed & BUTTON_LEFT) {
            return CONTROL_ACTION_CAMERA_INVERT_PREVIOUS;
        }
        if (pressed & BUTTON_RIGHT) {
            return CONTROL_ACTION_CAMERA_INVERT_NEXT;
        }
        return CONTROL_ACTION_NONE;
    }

    if (pressed & BUTTON_SELECT) {
        return CONTROL_ACTION_ITEMS_MENU;
    }
    if (held & BUTTON_LEFT) {
        return CONTROL_ACTION_ITEM_I;
    }
    if (held & BUTTON_DOWN) {
        return CONTROL_ACTION_ITEM_II;
    }
    if (held & BUTTON_UP) {
        return CONTROL_ACTION_NAVI;
    }
    if (held & BUTTON_RIGHT) {
        return CONTROL_ACTION_OCARINA;
    }
    return CONTROL_ACTION_NONE;
}
