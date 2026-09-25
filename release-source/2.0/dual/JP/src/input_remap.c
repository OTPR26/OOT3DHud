#include "input_remap.h"

#include "camera.h"
#include "hid.h"
#include "input.h"
#include "input_consume.h"
#include "native_hud.h"

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
#define HUD_SCALE_HOLD_FRAMES 45

typedef struct {
    s16 x;
    s16 y;
} TouchPoint;

static TouchPoint sSyntheticTouchPoint = { -1, -1 };
static u8 sSyntheticTouchActive = 0;
static u8 sSyntheticTouchWasActive = 0;
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
static u8 sHudScaleHoldFrames = 0;
static u8 sHudScaleHoldTriggered = 0;

static void InputRemap_UpdateHudScaleShortcut(u32 irrstHeld) {
    const u32 held = rInputCtx.cur.val;

    // Require ZR in addition to the two vanilla shoulder buttons so ordinary
    // targeting with the shield raised cannot resize the HUD.
    if (!Controls_IsHudScaleHold(held, irrstHeld)) {
        sHudScaleHoldFrames = 0;
        sHudScaleHoldTriggered = 0;
        return;
    }

    if (sHudScaleHoldTriggered) {
        return;
    }
    if (++sHudScaleHoldFrames >= HUD_SCALE_HOLD_FRAMES) {
        NativeHud_CycleScale();
        sHudScaleHoldTriggered = 1;
    }
}

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


#include "menu_options.h"
#include "unified_menu.h"

/* Observe the native Options lifecycle; do not synthesize Save transitions or
 * suspend inventory pages. Game-owned input is consumed at its menu getters. */
/* Replaceable bindings allow host tests to exercise this exact adapter. */
#ifndef DUAL_OPTIONS_STATE
#define DUAL_OPTIONS_STATE ((const volatile u32*)0x0050A508)
#define DUAL_OPTIONS_MODE (gSaveContext.gameMode)
#define DUAL_OPTIONS_INPUT ((const volatile u32*)0x0050AF0C)
#endif
static u8 sDualOptions, sDualEntryHeld, sDualExitHeld;
static int DualOptions_Update(void) {
    const volatile u32* save=DUAL_OPTIONS_STATE;
    const u8 nativeOptions=DUAL_OPTIONS_MODE==0 && save[0x28/4]==3;
    if (sDualExitHeld) {
        if (rInputCtx.cur.val & (BUTTON_A|BUTTON_B|BUTTON_START)) return 1;
        sDualExitHeld=0;
    }
    if (sDualOptions && (!nativeOptions || save[0x3c/4]==3)) {
        MenuOptions_End();
        sDualOptions=sDualEntryHeld=0;
        sDualExitHeld=1;
        return 1;
    }
    if (!nativeOptions || save[0x3c/4]>2) return 0;
    if (!sDualOptions) {
        MenuOptions_Begin(); sDualOptions=1; sDualEntryHeld=1;
    }
    if (!(rInputCtx.cur.val & (BUTTON_A|BUTTON_B|BUTTON_START))) sDualEntryHeld=0;
    if (!sDualEntryHeld && save[0x3c/4]==2) {
        int decision=MenuOptions_Update(rInputCtx.pressed.val);
        if (decision>=0) {
            MenuOptions_ExitNative(decision);
            /* Native OK/Cancel can finish immediately. Retire our renderer
             * before the native parent frees its Options resources this frame.
             * Failed persistence leaves phase 2 active and must stay open. */
            if (save[0x28/4]!=3 || save[0x3c/4]==3) {
                MenuOptions_End(); sDualOptions=sDualEntryHeld=0; sDualExitHeld=1;
            }
        }
    }
    return 1;
}

u32 UnifiedMenu_GetNativeMenuRepeat(void) {
    if (sDualOptions || sDualExitHeld) return 0;
    const volatile u32* input=DUAL_OPTIONS_INPUT;
    return input[9] ? input[9] : input[8];
}
u32 UnifiedMenu_GetNativeMenuButtons(void) {
    if (sDualOptions) return MenuOptions_NativeButtons();
    if (sDualExitHeld) return 0;
    const volatile u32* input=DUAL_OPTIONS_INPUT;
    return input[3] ? input[3] : input[2];
}
unsigned UnifiedMenu_IsOptionsDialog(void) { return sDualOptions; }
unsigned UnifiedMenu_GetDialogPage(void) { return sDualOptions; }
unsigned UnifiedMenu_GetFooterFocus(void) { return 0; }
unsigned UnifiedMenu_IsOpening(void) { return 0; }
unsigned UnifiedMenu_IsClosing(void) { return 0; }
/* These queries belong to the retained presentation renderer. Native Dual
 * Screen inventory, Songs, Save and Game Over never enter its replay paths. */
u32 InputRemap_IsItemsMenuOpen(void) { return 0; }
u32 InputRemap_IsItemsPageActive(void) { return 0; }
u32 InputRemap_IsGearPageActive(void) { return 0; }
u32 InputRemap_IsOcarinaUiOpen(void) { return 0; }
u32 InputRemap_IsOcarinaUiReady(void) { return 0; }
u32 InputRemap_IsGameOverUiOpen(void) { return 0; }
u32 InputRemap_IsSaveMenuOpen(void) { return sDualOptions; }
void InputRemap_OnPausePageActivated(u32 page) { (void)page; }
void InputRemap_SyncNativeMenu(void* context) { (void)context; }
void InputRemap_CloseOcarinaUi(void) {}
void InputRemap_CloseSaveMenu(void) {}
u8 gItemAssignmentDestination;

void InputRemap_Update(GlobalContext* globalCtx) {
    // Menu requests are valid only for the current gameplay update. The
    // native menu hook consumes this later in the same frame.
    sItemsMenuRequested = 0;

    // Each gameplay update starts without a queued shortcut. HID is read-only.
    sSyntheticTouchActive = 0;
    if (DualOptions_Update()) return;

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

    InputRemap_UpdateHudScaleShortcut(irrstButtons);

    const ControlAction action = Controls_Resolve(rInputCtx.cur.val,
                                                  rInputCtx.pressed.val);

    // Match Project Restoration's menu behavior: Select opens Items, then the
    // same button activates that screen's native close command. OoT3D closes
    // Items with B, so translate only the fresh Select edge while it is open.
    volatile u32* const gameHeld =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_BUTTONS_OFFSET);
    volatile u32* const gamePressed =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_NEW_BUTTONS_OFFSET);
    volatile u32* const gameReleased =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_RELEASED_BUTTONS_OFFSET);
    // Consume only the game's writable sampled state, never the HID ring.

    // Keep the toggle synchronized when the player uses the screen's normal
    // B exit instead of pressing Select again.
    if (sItemsMenuOpen && (rInputCtx.pressed.val & BUTTON_B)) {
        sItemsMenuOpen = 0;
    }

    switch (action) {
        case CONTROL_ACTION_CAMERA_SENSITIVITY_UP:
        case CONTROL_ACTION_CAMERA_SENSITIVITY_DOWN:
        case CONTROL_ACTION_CAMERA_INVERT_PREVIOUS:
        case CONTROL_ACTION_CAMERA_INVERT_NEXT:
            Camera_ApplyControlAction(action);
            break;
        case CONTROL_ACTION_ITEM_I:
        case CONTROL_ACTION_ITEM_II:
        case CONTROL_ACTION_NAVI:
        case CONTROL_ACTION_OCARINA:
            InputRemap_ApplyVanillaAction(action);
            break;
        case CONTROL_ACTION_ITEMS_MENU:
            if (sItemsMenuOpen) {
                InputRemap_ReplaceButtonMasks(gameHeld, gamePressed,
                                              gameReleased, BUTTON_SELECT,
                                              BUTTON_B);
                sItemsMenuOpen = 0;
            } else {
                InputRemap_ClearButtonMasks(gameHeld, gamePressed,
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

void InputRemap_ApplyVanillaAction(ControlAction action) {
    const TouchPoint point = InputRemap_GetTouchPoint(action);

    if (point.x < 0 || point.y < 0) {
        return;
    }

    // Queue a touch for the native sampler's stack-owned output. Physical
    // touchscreen input takes priority; no service-owned bytes are modified.
    if (rInputCtx.touchHeld) return;
    sSyntheticTouchPoint = point;
    sSyntheticTouchActive = 1;
}

void InputRemap_EndUpdate(void) { sSyntheticTouchActive = 0; }

// Called after native sampling, BEFORE native applet/input-disable checks.
// The native output is two s16 coordinates and a one-byte contact flag.
void InputRemap_FilterTouch(void* output, s32* count) {
    u8* sample = (u8*)output;
    if (sDualOptions || sDualExitHeld) { sample[4]=0; *count=1; return; }
    if (sSyntheticTouchActive) {
        *(s16*)(sample+0) = sSyntheticTouchPoint.x;
        *(s16*)(sample+2) = sSyntheticTouchPoint.y;
        sample[4] = 1;
        *count = 1;
        sSyntheticTouchWasActive = 1;
    } else if (sSyntheticTouchWasActive) {
        // A stale native ring must still release our previous synthetic hold.
        // A fresh physical sample (including a real touch) passes unchanged.
        if (*count <= 0) {
            *(s16*)(sample+0) = sSyntheticTouchPoint.x;
            *(s16*)(sample+2) = sSyntheticTouchPoint.y;
            sample[4] = 0;
            *count = 1;
        }
        sSyntheticTouchWasActive = 0;
    }
}
