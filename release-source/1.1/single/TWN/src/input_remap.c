#include "input_remap.h"

#include "camera.h"
#include "ab_swap.h"
#if CONTROLLER_OPTIONS_ENABLED
#include "controller_settings.h"
#endif
#include "hid.h"
#include "input.h"
#include "input_consume.h"
#include "native_hud.h"
#include "native_mods_menu.h"

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
#define OCARINA_OPEN_TOUCH_FRAMES 4
#define OCARINA_SONG_BUTTON_DELAY_FRAMES 32
#define OCARINA_SONG_TOUCH_FRAMES 4
#define SAVE_CONTINUE_DISMISS_FRAMES 12
#define SAVE_DISMISS_PULSE_FRAMES 4

typedef struct {
    s16 x;
    s16 y;
} TouchPoint;

static u32 sSyntheticTouchIndex = 0;
static TouchPoint sSyntheticTouchPoint = { -1, -1 };
static u8 sSyntheticTouchActive = 0;
static u8 sItemsMenuRequested = 0;
static u8 sItemsMenuOpen = 0;
static u8 sActivePausePage = 0;
static u8 sOcarinaUiOpen = 0;
static u8 sSaveMenuOpen = 0;
static u8 sOcarinaOpenTouchFrames = 0;
static u8 sOcarinaSongButtonDelay = 0;
static u8 sOcarinaSongTouchFrames = 0;
#if defined(Version_USA) || (defined(Version_JP) && JP_SINGLE_SCREEN_PROBE) || (defined(Version_TWN) && TWN_SINGLE_SCREEN_PROBE)
static u8 sOcarinaSongsPending = 0;
static u8 sOcarinaOpenWatchdog = 0;

// USA native Songs/ocarina manager, updated by 0x425AB4. State 12 is
// interactive ocarina controls; state 4 is the interactive song grid.
#if defined(Version_TWN)
static volatile u32* const sNativeSongs = (volatile u32*)0x005144D0;
#else
static volatile u32* const sNativeSongs = (volatile u32*)0x005093E4;
#endif
#endif
static u8 sSaveConfirmPressCount = 0;
static u8 sSaveOptionsSelected = 0;
static u8 sSaveOptionsOpen = 0;
static u8 sSaveModsOpen = 0;
static u8 sSaveDismissFrames = 0;
static u8 sSaveDismissPulseFrames = 0;
u8 gItemAssignmentDestination = 0;

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
    // touchscreen.
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

static void InputRemap_BeginItemAssignment(
    pad_t* pad, volatile u32* held, volatile u32* pressed,
    volatile u32* released, u32 dpadButton, u32 nativeButton) {
    // OoT3D has native assignment destinations for I and II, even though
    // physical buttons cannot request them. The destination hooks below
    // substitute those native IDs only for this synthetic button edge.
    gItemAssignmentDestination =
        dpadButton == BUTTON_LEFT ? 5 : 0x17;

    InputRemap_ReplaceButtonMasks(pad, held, pressed, released, dpadButton,
                                  nativeButton);
}

static void InputRemap_InjectTouchPoint(TouchPoint point) {
    touch_t* const touch = &real_hid.touch.touches[real_hid.touch.index];
    touch->touch.x = point.x;
    touch->touch.y = point.y;
    touch->updated = 1;
    sSyntheticTouchIndex = real_hid.touch.index;
    sSyntheticTouchPoint = point;
    sSyntheticTouchActive = 1;
}

void InputRemap_CloseOcarinaUi(void) {
    sOcarinaUiOpen = 0;
    sOcarinaOpenTouchFrames = 0;
    sOcarinaSongButtonDelay = 0;
    sOcarinaSongTouchFrames = 0;
#if defined(Version_USA) || (defined(Version_JP) && JP_SINGLE_SCREEN_PROBE) || (defined(Version_TWN) && TWN_SINGLE_SCREEN_PROBE)
    sOcarinaSongsPending = 0;
    sOcarinaOpenWatchdog = 0;
#endif
}

void InputRemap_CloseSaveMenu(void) {
    sSaveMenuOpen = 0;
    sSaveConfirmPressCount = 0;
    sSaveOptionsSelected = 0;
    sSaveOptionsOpen = 0;
    sSaveModsOpen = 0;
    sSaveDismissFrames = 0;
    sSaveDismissPulseFrames = 0;
    NativeModsMenu_Reset();
#if CONTROLLER_OPTIONS_ENABLED
    ControllerSettings_Close();
#endif
}

u32 InputRemap_IsGameOverUiOpen(void) {
    extern GlobalContext* gGlobalContext;
    // Verified native regional death dispatch: state 5 at PlayState+318C
    // waits for the dialogue result. Revival uses separate 20+ states.
    return gSaveContext.gameMode == 0 && gGlobalContext != 0 &&
        *(volatile u16*)((u8*)gGlobalContext + 0x318C) == 5;
}

void InputRemap_Update(GlobalContext* globalCtx) {
    // Menu requests are valid only for the current gameplay update. The
    // native menu hook consumes this later in the same frame.
    sItemsMenuRequested = 0;
    gItemAssignmentDestination = 0;

    volatile u32* const gameHeld =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_BUTTONS_OFFSET);
    volatile u32* const gamePressed =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_NEW_BUTTONS_OFFSET);
    volatile u32* const gameReleased =
        (volatile u32*)((u8*)globalCtx + PLAY_PAD_RELEASED_BUTTONS_OFFSET);
    pad_t* const hidPad = &real_hid.pad.pads[real_hid.pad.index];

#if CONTROLLER_OPTIONS_ENABLED
    ControllerSettings_ApplyABSwap(
        hidPad, gameHeld, gamePressed, gameReleased,
        &rInputCtx.cur.val, &rInputCtx.pressed.val, &rInputCtx.up.val);
#endif

#if CONTROLLER_OPTIONS_ENABLED
    // The Reframed page is an independent modal above the native Save menu.
    // While it is visible, its navigation never reaches the game underneath.
    if (sSaveModsOpen) {
        const bool closing = (rInputCtx.pressed.val &
                              (BUTTON_L1 | BUTTON_R1 | BUTTON_B)) != 0;
        const bool handled =
            ControllerSettings_HandleOptionsInput(rInputCtx.pressed.val);
        if (closing) {
            sSaveConfirmPressCount = 0;
            sSaveOptionsSelected = 1;
            sSaveOptionsOpen = 0;
            sSaveModsOpen = 0;
            NativeModsMenu_SetSelected(true);
        }
        if (handled) {
            u32 consumed =
                BUTTON_A | BUTTON_SELECT | BUTTON_START |
                BUTTON_RIGHT | BUTTON_LEFT | BUTTON_UP | BUTTON_DOWN |
                BUTTON_R1 | BUTTON_L1 | BUTTON_B;
            InputRemap_ClearButtonMasks(
                hidPad, gameHeld, gamePressed, gameReleased,
                consumed);
            return;
        }
    }
#endif

#if AB_SWAP_ENABLED
    ABSwap_Apply(hidPad, gameHeld, gamePressed, gameReleased,
                 &rInputCtx.cur.val, &rInputCtx.pressed.val,
                 &rInputCtx.up.val);
#endif

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

    if (InputRemap_IsGameOverUiOpen()) {
        // Death can interrupt a shortcut or menu. Preserve native input for
        // the death menu and discard only our stale modal/gesture bookkeeping.
        sItemsMenuOpen = 0;
        sActivePausePage = 0;
        sMinimapHideFrames = 0;
        InputRemap_CloseOcarinaUi();
        InputRemap_CloseSaveMenu();
        return;
    }

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
    // Never infer song completion from note count or a pause between notes.
    // Longer songs, held notes, mistakes and free play all remain interactive.
    // Native song recognition owns playback; only explicit exit input should
    // send B. Any future automatic overlay cleanup must observe native state
    // without injecting a cancel into an unfinished performance.

    // The Save prompt begins on Save. Down enters the bottom row at Options;
    // Left/Right then selects the adjacent native Mods button.
    if (sSaveMenuOpen && sSaveConfirmPressCount == 0) {
        if (rInputCtx.pressed.val & (BUTTON_DOWN | CPAD_DOWN)) {
            sSaveOptionsSelected = 1;
            NativeModsMenu_SetSelected(false);
        } else if (sSaveOptionsSelected &&
                   (rInputCtx.pressed.val & (BUTTON_RIGHT | CPAD_RIGHT))) {
            NativeModsMenu_SetSelected(true);
        } else if (sSaveOptionsSelected &&
                   (rInputCtx.pressed.val & (BUTTON_LEFT | CPAD_LEFT))) {
            NativeModsMenu_SetSelected(false);
        } else if (rInputCtx.pressed.val & (BUTTON_UP | CPAD_UP)) {
            sSaveOptionsSelected = 0;
            NativeModsMenu_SetSelected(false);
        }
    }

    // Save and Don't Save both lead to a second A confirmation for Continue
    // or Quit. Send B once that transition reaches the otherwise hidden Map
    // screen. Options is different: after OK, leave its restored parent Save
    // prompt visible so the player can choose Back normally.
    if (sSaveMenuOpen && sSaveConfirmPressCount == 0 &&
        sSaveOptionsSelected && NativeModsMenu_IsSelected() &&
        (rInputCtx.pressed.val & BUTTON_A)) {
#if CONTROLLER_OPTIONS_ENABLED
        sSaveModsOpen = 1;
        ControllerSettings_Open();
#endif
        // Our modal is independent of OoT3D's Options state. Do not let the
        // same A edge activate the native button underneath it.
        InputRemap_ClearButtonMasks(
            hidPad, gameHeld, gamePressed, gameReleased, BUTTON_A);
        return;
    }

    if (sSaveMenuOpen && (rInputCtx.pressed.val & BUTTON_A)) {
        if (sSaveConfirmPressCount != 0xFF) {
            ++sSaveConfirmPressCount;
        }
        if (sSaveConfirmPressCount == 1) {
            sSaveOptionsOpen = sSaveOptionsSelected;
            sSaveModsOpen = 0;
        } else if (sSaveConfirmPressCount == 2 && sSaveOptionsOpen) {
            sSaveConfirmPressCount = 0;
            sSaveOptionsSelected = 0;
            sSaveOptionsOpen = 0;
            sSaveModsOpen = 0;
            NativeModsMenu_Reset();
#if CONTROLLER_OPTIONS_ENABLED
            ControllerSettings_Close();
#endif
        } else if (sSaveConfirmPressCount == 2) {
            sSaveDismissFrames = SAVE_CONTINUE_DISMISS_FRAMES;
        }
    }
    if (sSaveDismissFrames != 0 && --sSaveDismissFrames == 0) {
        sSaveDismissPulseFrames = SAVE_DISMISS_PULSE_FRAMES;
    }
    if (sSaveDismissPulseFrames != 0) {
        InputRemap_ReplaceButtonMasks(hidPad, gameHeld, gamePressed,
                                      gameReleased, 0, BUTTON_B);
        // Save and Options consume the mod's global input sample rather than
        // PlayState's controller copy. Mirror the synthetic edge there too,
        // exactly as Input_Update represents a physical B press.
        rInputCtx.cur.val |= BUTTON_B;
        rInputCtx.pressed.val |= BUTTON_B;
        if (--sSaveDismissPulseFrames == 0) {
            InputRemap_CloseSaveMenu();
        }
        return;
    }

    // Keep the toggle synchronized when the player uses the screen's normal
    // B exit instead of pressing Select again.
    if (sItemsMenuOpen && (rInputCtx.pressed.val & BUTTON_B)) {
        sItemsMenuOpen = 0;
        sActivePausePage = 0;
    }
    if (sOcarinaUiOpen && (rInputCtx.pressed.val & BUTTON_B)) {
        InputRemap_CloseOcarinaUi();
    }
    if (sSaveMenuOpen && (rInputCtx.pressed.val & BUTTON_B)) {
        if (sSaveOptionsOpen) {
            // Cancel closes only the nested Options screen. Keep the overlay
            // alive while the native parent Save prompt is restored.
            sSaveConfirmPressCount = 0;
            sSaveOptionsSelected = 0;
            sSaveOptionsOpen = 0;
            sSaveModsOpen = 0;
            NativeModsMenu_Reset();
#if CONTROLLER_OPTIONS_ENABLED
            ControllerSettings_Close();
#endif
        } else {
            InputRemap_CloseSaveMenu();
        }
    }
    if (gSaveContext.gameMode == 0 &&
        !sItemsMenuOpen && !sOcarinaUiOpen &&
        (rInputCtx.pressed.val & BUTTON_START)) {
        if (sSaveMenuOpen) {
            InputRemap_ReplaceButtonMasks(hidPad, gameHeld, gamePressed,
                                          gameReleased, BUTTON_START,
                                          BUTTON_B);
            InputRemap_CloseSaveMenu();
        } else {
            sSaveMenuOpen = 1;
            sSaveConfirmPressCount = 0;
            sSaveOptionsSelected = 0;
            sSaveOptionsOpen = 0;
            sSaveModsOpen = 0;
            NativeModsMenu_Reset();
            sSaveDismissFrames = 0;
            sSaveDismissPulseFrames = 0;
        }
    }

    // Drive both native touchscreen buttons for several complete game updates.
    // A single synthetic sample can be missed at a transition boundary.
    if (sOcarinaOpenTouchFrames != 0) {
        --sOcarinaOpenTouchFrames;
        InputRemap_InjectTouchPoint(
            InputRemap_GetTouchPoint(CONTROL_ACTION_OCARINA));
        return;
    }
#if defined(Version_USA) || (defined(Version_JP) && JP_SINGLE_SCREEN_PROBE) || (defined(Version_TWN) && TWN_SINGLE_SCREEN_PROBE)
    if (sOcarinaUiOpen && sOcarinaSongsPending) {
        // These hidden six-frame board animations are presentation only.
        // Let the native update execute its final frame and normal cleanup,
        // rather than skipping a state or changing Link's instrument logic.
#if defined(Version_USA)
        const u32 nativeState = sNativeSongs[0x14 / 4];
        if (nativeState == 16 || nativeState == 13)
            sNativeSongs[0x38 / 4] = 5;
        else if (nativeState == 14)
            sNativeSongs[0x38 / 4] = 0;
#endif
        if (sNativeSongs[0x14 / 4] == 4 &&
            sNativeSongs[0x28 / 4] == 0) {
            sOcarinaSongsPending = 0;
            sOcarinaSongTouchFrames = 0;
            return;
        }
        // If the game rejects ocarina use, abandon the request instead of
        // leaving an invisible modal or sending taps to some later menu.
        if (++sOcarinaOpenWatchdog >= 120) {
            InputRemap_CloseOcarinaUi();
            return;
        }
        if (sOcarinaSongTouchFrames != 0) {
            --sOcarinaSongTouchFrames;
            InputRemap_InjectTouchPoint((TouchPoint){ 290, 222 });
        } else if (sNativeSongs[0x14 / 4] == 12 &&
                   sNativeSongs[0x28 / 4] == 1 &&
                   sNativeSongs[0x40 / 4] == 1 &&
                   sNativeSongs[0x44 / 4] == 1) {
            // Same readiness checks as the native Songs button, with no
            // fixed 32-frame pause before requesting its normal transition.
            sOcarinaSongTouchFrames = OCARINA_SONG_TOUCH_FRAMES - 1;
            InputRemap_InjectTouchPoint((TouchPoint){ 290, 222 });
        }
        return;
    }
#else
    if (sOcarinaUiOpen && sOcarinaSongButtonDelay != 0) {
        --sOcarinaSongButtonDelay;
        if (sOcarinaSongButtonDelay == 0) {
            sOcarinaSongTouchFrames = OCARINA_SONG_TOUCH_FRAMES;
        }
        return;
    }
#endif
    if (sOcarinaSongTouchFrames != 0) {
        --sOcarinaSongTouchFrames;
        InputRemap_InjectTouchPoint((TouchPoint){ 290, 222 });
        return;
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
            if (InputRemap_IsItemsPageActive()) {
                const u32 dpadButton =
                    action == CONTROL_ACTION_ITEM_I ? BUTTON_LEFT : BUTTON_DOWN;
                const u32 nativeButton =
                    action == CONTROL_ACTION_ITEM_I ? BUTTON_X : BUTTON_Y;

                if (rInputCtx.pressed.val & dpadButton) {
                    InputRemap_BeginItemAssignment(
                        hidPad, gameHeld, gamePressed, gameReleased,
                        dpadButton, nativeButton);
                } else {
                    InputRemap_ClearButtonMasks(
                        hidPad, gameHeld, gamePressed, gameReleased,
                        dpadButton);
                }
                break;
            }
            InputRemap_ApplyVanillaAction(action);
            break;
        case CONTROL_ACTION_NAVI:
            InputRemap_ApplyVanillaAction(action);
            break;
        case CONTROL_ACTION_OCARINA:
            if (!sOcarinaUiOpen) {
                if (rInputCtx.pressed.val & BUTTON_RIGHT) {
                    sSaveMenuOpen = 0;
                    sOcarinaUiOpen = 1;
                    sOcarinaOpenTouchFrames =
                        OCARINA_OPEN_TOUCH_FRAMES - 1;
                    sOcarinaSongButtonDelay =
                        OCARINA_SONG_BUTTON_DELAY_FRAMES;
#if defined(Version_USA) || (defined(Version_JP) && JP_SINGLE_SCREEN_PROBE) || (defined(Version_TWN) && TWN_SINGLE_SCREEN_PROBE)
                    sOcarinaSongButtonDelay = 0;
                    sOcarinaSongsPending = 1;
                    sOcarinaOpenWatchdog = 0;
#endif
                    sOcarinaSongTouchFrames = 0;
                    InputRemap_ApplyVanillaAction(action);
                }
            }
            break;
        case CONTROL_ACTION_ITEMS_MENU:
            InputRemap_CloseOcarinaUi();
            InputRemap_CloseSaveMenu();
            if (sItemsMenuOpen) {
                InputRemap_ReplaceButtonMasks(hidPad, gameHeld, gamePressed,
                                              gameReleased, BUTTON_SELECT,
                                              BUTTON_B);
                sItemsMenuOpen = 0;
                sActivePausePage = 0;
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
        sActivePausePage = 1;
    }

    // Always suppress the vanilla Select-as-Start path for this press.
    return 1;
}

u32 InputRemap_IsItemsMenuOpen(void) {
    // The native transition flag also becomes active while loading a save.
    // Only replay the pause UI after an explicit menu-open request.
    return sItemsMenuOpen;
}

// Observe completed native menu activation and return to gameplay, including
// touchscreen paths. Do not use page constructors, which also run on loading.
void InputRemap_SyncNativeMenu(void* menuManager) {
    const volatile u32* const state = (const volatile u32*)menuManager;
    const u32 mode = state[0x0D];
    if (mode == 0x0C || mode == 0x10 || mode == 0x14) {
        sItemsMenuOpen = 1;
        sActivePausePage = mode == 0x0C ? 1 : mode == 0x10 ? 2 : 3;
    } else if (mode == 2) {
        sItemsMenuOpen = 0;
        sActivePausePage = 0;
    }
}

u32 InputRemap_IsItemsPageActive(void) {
    return InputRemap_IsItemsMenuOpen() && sActivePausePage == 1;
}

u32 InputRemap_IsGearPageActive(void) {
    return InputRemap_IsItemsMenuOpen() && sActivePausePage == 2;
}

void InputRemap_OnPausePageActivated(u32 page) {
    // Page activation is reported by the native Items, Gear, and Map manager
    // entry points. Unlike their internal animation states, these calls are
    // mutually ordered when the player changes tabs.
    sActivePausePage = (u8)page;
}

u32 InputRemap_IsOcarinaUiOpen(void) {
    return sOcarinaUiOpen;
}

u32 InputRemap_IsOcarinaUiReady(void) {
#if defined(Version_USA) || (defined(Version_JP) && JP_SINGLE_SCREEN_PROBE) || (defined(Version_TWN) && TWN_SINGLE_SCREEN_PROBE)
    return sOcarinaUiOpen && !sOcarinaSongsPending &&
           sNativeSongs[0x14 / 4] != 0;
#else
    return sOcarinaUiOpen;
#endif
}

u32 InputRemap_IsSaveMenuOpen(void) {
    return sSaveMenuOpen;
}

void InputRemap_ApplyVanillaAction(ControlAction action) {
    const TouchPoint point = InputRemap_GetTouchPoint(action);

    if (point.x < 0 || point.y < 0) {
        return;
    }

    // This hook runs at the start of GlobalContext_Update, before OoT3D reads
    // the current HID touch sample. Synthesizing a touch here delegates the
    // entire action to the vanilla handler, including hold/release behavior,
    // item-usability rules, contextual Navi/View behavior, and animations.
    InputRemap_InjectTouchPoint(point);
}
