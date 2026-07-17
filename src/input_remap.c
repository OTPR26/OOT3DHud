#include "input_remap.h"

#include "camera.h"
#include "hid.h"
#include "input.h"

typedef struct {
    s16 x;
    s16 y;
} TouchPoint;

static u32 sSyntheticTouchIndex = 0;
static TouchPoint sSyntheticTouchPoint = { -1, -1 };
static u8 sSyntheticTouchActive = 0;

static TouchPoint InputRemap_GetTouchPoint(ControlAction action) {
    // OoT3D's gameplay touch buttons occupy the corners of the 320x240
    // touchscreen. These points match established emulator touch mappings.
    switch (action) {
        case CONTROL_ACTION_ITEM_I:
            return (TouchPoint){ 316, 3 };
        case CONTROL_ACTION_ITEM_II:
            return (TouchPoint){ 318, 238 };
        case CONTROL_ACTION_NAVI:
            return (TouchPoint){ 3, 3 };
        case CONTROL_ACTION_OCARINA:
            return (TouchPoint){ 8, 235 };
        default:
            return (TouchPoint){ -1, -1 };
    }
}

void InputRemap_Update(GlobalContext* globalCtx) {
    // If HID has not advanced to a new sample since our previous injection,
    // clear only the exact sample we created. Never clear a newer physical
    // touch sample from the player.
    if (sSyntheticTouchActive && real_hid.touch.index == sSyntheticTouchIndex) {
        touch_t* const previous = &real_hid.touch.touches[sSyntheticTouchIndex];
        if (previous->touch.x == sSyntheticTouchPoint.x &&
            previous->touch.y == sSyntheticTouchPoint.y) {
            previous->updated = 0;
        }
    }
    sSyntheticTouchActive = 0;

    const ControlDecision decision = Controls_Resolve(rInputCtx.cur.val, rInputCtx.pressed.val);

    switch (decision.action) {
        case CONTROL_ACTION_CAMERA_SENSITIVITY_UP:
        case CONTROL_ACTION_CAMERA_SENSITIVITY_DOWN:
        case CONTROL_ACTION_CAMERA_INVERT_PREVIOUS:
        case CONTROL_ACTION_CAMERA_INVERT_NEXT:
            Camera_ApplyControlAction(decision.action);
            break;
        case CONTROL_ACTION_ITEM_I:
        case CONTROL_ACTION_ITEM_II:
        case CONTROL_ACTION_NAVI:
        case CONTROL_ACTION_OCARINA:
            InputRemap_ApplyVanillaAction(globalCtx, decision.action);
            break;
        case CONTROL_ACTION_NONE:
            break;
    }
}

void InputRemap_ApplyVanillaAction(GlobalContext* globalCtx, ControlAction action) {
    const TouchPoint point = InputRemap_GetTouchPoint(action);
    (void)globalCtx;

    if (point.x < 0 || point.y < 0) {
        return;
    }

    // This hook runs at the start of GlobalContext_Update, before OoT3D reads
    // the current HID touch sample. Synthesizing a touch here delegates the
    // entire action to the vanilla handler, including hold/release behavior,
    // item-usability rules, contextual Navi/View behavior, and animations.
    touch_t* const touch = &real_hid.touch.touches[real_hid.touch.index];
    touch->touch.x = point.x;
    touch->touch.y = point.y;
    touch->updated = 1;
    sSyntheticTouchIndex = real_hid.touch.index;
    sSyntheticTouchPoint = point;
    sSyntheticTouchActive = 1;
}
