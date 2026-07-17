#include "controls.h"

#include "hid.h"

ControlDecision Controls_Resolve(uint32_t held, uint32_t pressed) {
    const uint32_t configChord = BUTTON_L1 | BUTTON_R1;
    ControlDecision result = { CONTROL_ACTION_NONE, 0 };

    if ((held & configChord) == configChord) {
        if (pressed & BUTTON_UP) {
            result.action = CONTROL_ACTION_CAMERA_SENSITIVITY_UP;
        } else if (pressed & BUTTON_DOWN) {
            result.action = CONTROL_ACTION_CAMERA_SENSITIVITY_DOWN;
        } else if (pressed & BUTTON_LEFT) {
            result.action = CONTROL_ACTION_CAMERA_INVERT_PREVIOUS;
        } else if (pressed & BUTTON_RIGHT) {
            result.action = CONTROL_ACTION_CAMERA_INVERT_NEXT;
        }

        if (result.action != CONTROL_ACTION_NONE) {
            result.consumedButtons = pressed &
                (BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT);
        }
        return result;
    }

    if (held & BUTTON_LEFT) {
        result.action = CONTROL_ACTION_ITEM_I;
    } else if (held & BUTTON_DOWN) {
        result.action = CONTROL_ACTION_ITEM_II;
    } else if (held & BUTTON_UP) {
        result.action = CONTROL_ACTION_NAVI;
    } else if (held & BUTTON_RIGHT) {
        result.action = CONTROL_ACTION_OCARINA;
    }

    if (result.action != CONTROL_ACTION_NONE) {
        result.consumedButtons = held &
            (BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT);
    }
    return result;
}
