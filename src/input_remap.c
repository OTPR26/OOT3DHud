#include "input_remap.h"

#include "camera.h"
#include "hid.h"
#include "input.h"
#include "input_consume.h"

// OoT3D's GameState owns a sampled controller state beginning at offset 0x14.
// The sampler copies its source block at +0x04 into the GameState destination
// at +0x00, so the destination's held/new/released masks begin at
// PlayState+0x14/+0x18/+0x1C. This differs from MM3D's later pad::State layout.
#define PLAY_PAD_BUTTONS_OFFSET     0x14
#define PLAY_PAD_NEW_BUTTONS_OFFSET 0x18
#define PLAY_PAD_RELEASED_BUTTONS_OFFSET 0x1C

// Vanilla OoT3D hides the minimap only after D-Pad Down has been held for
// about three seconds. A later D-Pad press shows it immediately. Run the hide
// gesture a little longer than the nominal threshold so frame-rate jitter
// cannot leave the map visible.
#define MINIMAP_HIDE_HOLD_FRAMES 110

typedef struct {
    s16 x;
    s16 y;
} TouchPoint;

static u32 sSyntheticTouchIndex = 0;
static TouchPoint sSyntheticTouchPoint = { -1, -1 };
static u8 sSyntheticTouchActive = 0;
static u8 sItemsMenuRequested = 0;
static u8 sItemsMenuOpen = 0;

#if defined(Version_JP)
    #define NATIVE_ITEMS_OPEN_ADDR 0x002F3EF0
#elif defined(Version_TWN)
    #define NATIVE_ITEMS_OPEN_ADDR 0x003118EC
#elif defined(Version_KOR)
    #define NATIVE_ITEMS_OPEN_ADDR 0x003117EC
#else
    // USA and Europe share this native menu transition.
    #define NATIVE_ITEMS_OPEN_ADDR 0x002F43D8
#endif

static u32 sPreviousIrrstButtons = 0;
static u16 sMinimapHideFrames = 0;
static u8 sMinimapAssumedVisible = 1;

static void InputRemap_InjectMinimapButton(GlobalContext* globalCtx, u32 button,
                                           u8 pressed) {
    volatile u32* const buttons =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_BUTTONS_OFFSET);
    volatile u32* const newButtons =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_NEW_BUTTONS_OFFSET);

    *buttons |= button;
    if (pressed) {
        *newButtons |= button;
    }
}

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
        case CONTROL_ACTION_ITEMS_MENU:
            return (TouchPoint){ 222, 236 };
        default:
            return (TouchPoint){ -1, -1 };
    }
}

void InputRemap_Update(GlobalContext* globalCtx) {
    // Menu requests are valid only for the current gameplay update. The
    // native menu hook consumes this later in the same frame.
    sItemsMenuRequested = 0;

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

    // Complete an in-progress vanilla hide gesture before handling any other
    // remap. OoT3D expects D-Pad Down to remain held for roughly three seconds.
    if (sMinimapHideFrames != 0) {
        InputRemap_InjectMinimapButton(globalCtx, BUTTON_DOWN,
                                       sMinimapHideFrames == MINIMAP_HIDE_HOLD_FRAMES);
        --sMinimapHideFrames;
        if (sMinimapHideFrames == 0) {
            sMinimapAssumedVisible = 0;
        }
        return;
    }

    // ZL is unused by vanilla OoT3D. IRRST reports it separately from the
    // original 12-button HID block. Reproduce the game's asymmetric minimap
    // controls: hold Down to hide, or send one fresh direction to show.
    const u32 irrstButtons = irrstKeysHeld();
    const u32 irrstPressed = irrstButtons & ~sPreviousIrrstButtons;
    sPreviousIrrstButtons = irrstButtons;
    if (irrstPressed & IRRST_BUTTON_ZL) {
        if (sMinimapAssumedVisible) {
            sMinimapHideFrames = MINIMAP_HIDE_HOLD_FRAMES;
            InputRemap_InjectMinimapButton(globalCtx, BUTTON_DOWN, 1);
            --sMinimapHideFrames;
        } else {
            InputRemap_InjectMinimapButton(globalCtx, BUTTON_UP, 1);
            sMinimapAssumedVisible = 1;
        }
        return;
    }

    const ControlDecision decision = Controls_Resolve(rInputCtx.cur.val, rInputCtx.pressed.val);

    // Match Project Restoration's menu behavior: Select opens Items, then the
    // same button activates that screen's native close command. OoT3D closes
    // Items with B, so translate only the fresh Select edge while it is open.
    volatile u32* const gameHeld =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_BUTTONS_OFFSET);
    volatile u32* const gamePressed =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_NEW_BUTTONS_OFFSET);
    volatile u32* const gameReleased =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_RELEASED_BUTTONS_OFFSET);
    pad_t* const hidPad = &real_hid.pad.pads[real_hid.pad.index];

    // Keep the toggle synchronized when the player uses the screen's normal
    // B exit instead of pressing Select again.
    if (sItemsMenuOpen && (rInputCtx.pressed.val & BUTTON_B)) {
        sItemsMenuOpen = 0;
    }

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
        case CONTROL_ACTION_ITEMS_MENU:
            if (sItemsMenuOpen) {
                InputRemap_ReplaceButtonMasks(hidPad, gameHeld, gamePressed,
                                              gameReleased, BUTTON_SELECT,
                                              BUTTON_B);
                sItemsMenuOpen = 0;
            } else {
                InputRemap_ClearButtonMasks(hidPad, gameHeld, gamePressed,
                                            gameReleased, BUTTON_SELECT);
                sItemsMenuRequested = 1;
            }
            break;
        case CONTROL_ACTION_NONE:
            break;
    }
}

u32 InputRemap_TryOpenItemsMenu(void* menuManager) {
    volatile u32* const state = (volatile u32*)menuManager;

    if (!sItemsMenuRequested) {
        return 0;
    }
    sItemsMenuRequested = 0;

    // State 2 is the normal gameplay HUD. The native Items touch target moves
    // through state 7 and reaches 0x0C when released. Enter that completed
    // activation state directly so no touchscreen sample can be missed.
    if (state[0x0D] == 2) {
        ((void (*)(u32))NATIVE_ITEMS_OPEN_ADDR)(1);
        state[0x0D] = 0x0C;
        state[0x17] = 0;
        sItemsMenuOpen = 1;
    }

    // Always suppress the vanilla Select-as-Start path for this press.
    return 1;
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
