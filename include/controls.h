#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    CONTROL_ACTION_NONE = 0,
    CONTROL_ACTION_ITEM_I,
    CONTROL_ACTION_ITEM_II,
    CONTROL_ACTION_NAVI,
    CONTROL_ACTION_OCARINA,
    CONTROL_ACTION_ITEMS_MENU,
    CONTROL_ACTION_CAMERA_SENSITIVITY_UP,
    CONTROL_ACTION_CAMERA_SENSITIVITY_DOWN,
    CONTROL_ACTION_CAMERA_INVERT_PREVIOUS,
    CONTROL_ACTION_CAMERA_INVERT_NEXT,
} ControlAction;

ControlAction Controls_Resolve(uint32_t held, uint32_t pressed);
bool Controls_IsHudScaleHold(uint32_t held, uint32_t irrstHeld);
