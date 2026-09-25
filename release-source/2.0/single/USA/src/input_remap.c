#include "input_remap.h"

#include "camera.h"
#include "menu_options.h"
#include "menu_touch.h"
#include "ab_swap.h"
#if CONTROLLER_OPTIONS_ENABLED
#include "controller_settings.h"
#endif
#include "hid.h"
#include "input.h"
#include "input_consume.h"
#include "native_hud.h"
#include "native_mods_menu.h"
#include "unified_menu.h"
#include "unified_menu_navigation.h"
#include "unified_menu_layout.h"

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
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
/* Native transitions remain authoritative. 1 waits for pause to close,
 * 2 waits for Save to close, 3 sends the return-tab touch. */
static u8 sFooterTransition;
static u8 sFooterReturnPage;
static u8 sFooterTouchFrames;
static u16 sFooterWaitFrames;
static u8 sFooterFocus;
static u8 sPauseOpening;
static u8 sFooterOpenOptions;
static u8 sFooterNavigationLatch;
static u8 sDirectOptions;
static u8 sDialogEntryLatch;
static u8 sPauseClosing;
static u8 sPauseCloseSettled;
static volatile u32* sSuspendedPageState;
static u32 sSuspendedPageValue;
static u32 sSuspendedTitle;
static u64 sPhysicalTouchStamp;
static u8 sPhysicalTouchHeld, sTouchTabTarget, sTouchTabFrames;

static bool UnifiedMenu_IsAtBottom(void) {
    if (sActivePausePage == 1) {
        const volatile u32* items = (const volatile u32*)0x005066F8;
        return items[0x34/4] == 2 && items[0x7C/4] == 3;
    }
    if (sActivePausePage == 2) {
        const volatile u32* gear = (const volatile u32*)0x0050446C;
        const u32 selected = gear[0x20/4];
        const signed char* const* neighbours =
            (const signed char* const*)0x00504A74;
        /* Native Gear navigation table: byte 4 is the Down neighbour. */
        return gear[0x18/4] == 2 && selected < 27 &&
            neighbours[selected][4] == -1;
    }
    if (sActivePausePage == 3) {
        const volatile u32* map = (const volatile u32*)0x00506CB0;
        if (map[0x38/4] != 2) return false;
        /* Native map-kind flag belongs to a different manager (4FDA6C),
         * not the Map presentation object's +0x50 field. */
        if (((int (*)(void))0x002F1268)()) {
            const int selected = (int)map[0x30/4];
            if (selected < 0 || selected > 7) return false;
            return !MenuMap_DungeonCanMoveDown(selected,
                    (const volatile unsigned*)0x0055B638);
        }
        if (map[0x5C/4]) return true; /* Native world cursor disabled here. */
        if (map[0x48/4] >= 12 || !map[0x2C/4]) return false;
        return !MenuMap_WorldCanMoveDown(map[0x48/4],
                (const volatile unsigned char*)0x0050778E,
                (const volatile unsigned*)(map[0x2C/4] + 0x18));
    }
    return false;
}
#endif

unsigned UnifiedMenu_GetFooterFocus(void) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    return sFooterFocus;
#else
    return 0;
#endif
}

/* USA native menu-repeat getter (33B5D0). Its cached directional repeats
 * survive clearing the raw HID masks, so gate them at their consumer. */
u32 UnifiedMenu_GetNativeMenuRepeat(void) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    if (UnifiedMenu_IsOptionsDialog()) return 0;
    if (sPauseClosing && MenuPause_CanDeliverClose(
            ((volatile u32*)0x0050AF34)[0x0D])) return BUTTON_B;
    if (sDialogEntryLatch) return 0;
    if (sItemsMenuOpen && (sFooterFocus || sFooterNavigationLatch)) return 0;
#endif
    const volatile u32* input = (const volatile u32*)0x0050AF0C;
    return input[0x24/4] ? input[0x24/4] : input[0x20/4];
}
u32 UnifiedMenu_GetNativeMenuButtons(void) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    if (UnifiedMenu_IsOptionsDialog()) return MenuOptions_NativeButtons();
    if (sPauseClosing && MenuPause_CanDeliverClose(
            ((volatile u32*)0x0050AF34)[0x0D])) return BUTTON_B;
    if (sDialogEntryLatch) return 0;
#endif
    const volatile u32* input = (const volatile u32*)0x0050AF0C;
    return input[3] ? input[3] : input[2];
}
static u8 sOcarinaUiOpen = 0;
static u8 sSaveMenuOpen = 0;
static u8 sOcarinaOpenTouchFrames = 0;
static u8 sOcarinaSongButtonDelay = 0;
static u8 sOcarinaSongTouchFrames = 0;
#if defined(Version_USA) || (defined(Version_JP) && JP_SINGLE_SCREEN_PROBE) || (defined(Version_TWN) && TWN_SINGLE_SCREEN_PROBE)
static u8 sOcarinaSongsPending = 0;
static u8 sOcarinaOpenWatchdog = 0;
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
static u8 sOcarinaLayoutSettled;
#endif

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
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    sOcarinaLayoutSettled = 0;
#endif
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

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    touch_t* menuTouch=&real_hid.touch.touches[real_hid.touch.index];
    const u8 freshTouch=real_hid.touch.timestamp!=sPhysicalTouchStamp;
    const int touchX=MenuTouch_X(menuTouch->touch.x), touchY=menuTouch->touch.y;
    const u8 touchPress=freshTouch && menuTouch->updated && !sPhysicalTouchHeld;
    if (freshTouch) {
        sPhysicalTouchStamp=real_hid.touch.timestamp;
        sPhysicalTouchHeld=menuTouch->updated!=0;
    }
    /* Shell controls own their taps. Never feed their untransformed screen
     * coordinates into the old bottom-screen Back or tab buttons. */
    if (sItemsMenuOpen || sDirectOptions || sPauseOpening || sPauseClosing) {
        menuTouch->updated=0;
        if (freshTouch && sPhysicalTouchHeld && sSaveMenuOpen && !sSaveOptionsOpen) {
            int nx,ny;
            if (MenuTouch_Save(touchX,touchY,&nx,&ny))
                InputRemap_InjectTouchPoint((TouchPoint){nx,ny});
        }
    }
    if (sItemsMenuOpen && !sPauseOpening && !sPauseClosing && touchPress) {
        const int tab=MenuTouch_Tab(touchX,touchY);
        const int footer=MenuTouch_Footer(touchX,touchY);
        if (tab && tab!=sActivePausePage) {
            sFooterFocus=0; sTouchTabTarget=tab; sTouchTabFrames=3;
        } else if (footer) {
            sFooterFocus=footer;
            rInputCtx.pressed.val|=BUTTON_A;
        }
    }
    if (sItemsMenuOpen && !sPauseOpening && !sPauseClosing &&
        freshTouch && sPhysicalTouchHeld) {
        const u8 dungeon=sActivePausePage==3 && ((int(*)(void))0x002F1268)();
        const MenuRect surface=sActivePausePage==1 ? gMenuItemsSurface :
            sActivePausePage==2 ? gMenuGearSurface : dungeon ? gMenuDungeonSurface : gMenuWideSurface;
        const MenuRect clip=sActivePausePage==1 ? gMenuItemsClip :
            sActivePausePage==2 ? gMenuGearClip : dungeon ? gMenuDungeonSurface : gMenuWideClip;
        if (MenuTouch_In(touchX,touchY,clip.left,clip.top,clip.width,clip.height)) {
            sFooterFocus=0;
            InputRemap_InjectTouchPoint((TouchPoint){
                (touchX-surface.left)*320/surface.width,
                (touchY-surface.top)*240/surface.height});
        }
    }
    if (!sItemsMenuOpen || sActivePausePage==sTouchTabTarget) sTouchTabFrames=0;
    if (sTouchTabFrames) {
        --sTouchTabFrames;
        InputRemap_InjectTouchPoint((TouchPoint){
            sTouchTabTarget==1 ? 220 : sTouchTabTarget==2 ? 92 : 150,225});
        return;
    }
    /* Pause owns D-pad input: never let gameplay corner touches (especially
     * Ocarina) run against a native menu. Keep only a fresh assignment edge;
     * navigation uses the circle pad, including the footer and dialogs. */
    const u32 pauseDpadMask = BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT;
    const u32 pauseDpadPressed = rInputCtx.pressed.val & pauseDpadMask;
    const u8 pauseOwnsDpad = sItemsMenuOpen || sPauseOpening || sPauseClosing ||
        sDirectOptions || sFooterTransition;
    if (pauseOwnsDpad) {
        InputRemap_ClearButtonMasks(hidPad, gameHeld, gamePressed, gameReleased,
                                   pauseDpadMask);
        rInputCtx.cur.val &= ~pauseDpadMask;
        rInputCtx.pressed.val &= ~pauseDpadMask;
        rInputCtx.up.val &= ~pauseDpadMask;
        volatile u32* cached = (volatile u32*)0x0050AF0C;
        cached[2] &= ~pauseDpadMask;
        cached[3] &= ~pauseDpadMask;
        cached[8] &= ~pauseDpadMask;
        cached[9] &= ~pauseDpadMask;
    }
    if (!(rInputCtx.cur.val & (BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT |
            BUTTON_RIGHT | CPAD_UP | CPAD_DOWN | CPAD_LEFT | CPAD_RIGHT)))
        sFooterNavigationLatch = 0;
    /* Poll from the input tick, including native submenu-only frames. */
    if (sPauseOpening > 1) --sPauseOpening;
    else if (sPauseOpening && (*(volatile u32*)0x0050672C == 2 ||
                               *(volatile u32*)0x0050672C == 8))
        sPauseOpening = 0;
    if (InputRemap_IsGameOverUiOpen() || gSaveContext.gameMode != 0) {
        sFooterTransition = 0;
        sFooterReturnPage = 0;
        sFooterFocus = 0;
        sFooterOpenOptions = 0;
        sFooterNavigationLatch = 0;
        sDirectOptions = 0;
        sDialogEntryLatch = 0;
        sPauseClosing = sPauseCloseSettled = 0;
        sPauseOpening = 0;
        sSuspendedPageState = 0;
    }
    /* Both gameplay Start and the footer use the same two-choice Save
     * presentation. Do not leave a hidden Options row navigable on Start. */
    if (sSaveMenuOpen && !sSaveOptionsOpen) {
        volatile u32* save = (volatile u32*)0x0050A508;
        if (save[0x28/4] == 2 && save[0x24/4] <= 1) {
            /* Options belongs to the shell, not this confirmation. */
            save[0x38/4] = 0;
            const u32 vertical = BUTTON_UP | BUTTON_DOWN | CPAD_UP | CPAD_DOWN;
            InputRemap_ClearButtonMasks(hidPad, gameHeld, gamePressed, gameReleased, vertical);
            rInputCtx.cur.val &= ~vertical;
            rInputCtx.pressed.val &= ~vertical;
            volatile u32* cached = (volatile u32*)0x0050AF0C;
            cached[2] &= ~vertical; cached[3] &= ~vertical;
            cached[8] &= ~vertical; cached[9] &= ~vertical;
            touch_t* touch = &real_hid.touch.touches[real_hid.touch.index];
            if (touch->updated && touch->touch.y >= 190) touch->updated = 0;
        }
    }
    if (sDirectOptions) {
        volatile u32* save = (volatile u32*)0x0050A508;
        if (!(rInputCtx.cur.val & BUTTON_A)) sDialogEntryLatch = 0;
        const u32 dialogKind = sDirectOptions;
        if (dialogKind >= 2 && save[0x28/4] == 0) {
            /* Quit has completed native cleanup. Never revive a tab from
             * the discarded gameplay context or retain a modal input lock. */
            sDirectOptions = sDialogEntryLatch = 0;
            sSuspendedPageState = 0;
            sFooterReturnPage = sFooterFocus = 0;
            InputRemap_CloseSaveMenu();
            return;
        }
        sSaveOptionsOpen = save[0x28/4] == 3;
        if (dialogKind==1 && sSaveOptionsOpen && save[0x3c/4]!=3) {
            const u32 pressed=rInputCtx.pressed.val|pauseDpadPressed;
            touch_t* touch=&real_hid.touch.touches[real_hid.touch.index];
            touch->updated=0;
            int decision=-1;
            if (!sDialogEntryLatch && save[0x3c/4]==2) {
                decision=touchPress ? MenuOptions_Touch(touchX,touchY) :
                    MenuOptions_Update(pressed);
            }
            InputRemap_ClearButtonMasks(hidPad,gameHeld,gamePressed,gameReleased,0xffffffff);
            rInputCtx.cur.val=rInputCtx.pressed.val=rInputCtx.up.val=0;
            volatile u32* cached=(volatile u32*)0x0050AF0C;
            cached[2]=cached[3]=cached[8]=cached[9]=0;
            if (decision>=0) MenuOptions_ExitNative(decision);
        }
        if (dialogKind == 3 && save[0x28/4] == 2 &&
            (save[0x24/4] == 2 || save[0x24/4] == 14)) {
            /* Start has no suspended tab. Finish native Save cleanup and
             * release its pause flag directly, before the legacy exit can
             * request the intervening Map screen. Save writes/Continue/Quit
             * decisions have already been handled by the native manager. */
            ((void (*)(int,int))0x002E9920)(0,0);
            /* Complete the native parent-HUD return as well, but execute
             * its final geometry frame without the intervening animation. */
            ((void (*)(int))0x002F87EC)(5);
            ((volatile u32*)0x0050AF34)[0x5C/4] = 0;
            /* Cleanup selects native title 6 (hidden). Unlike a suspended
             * tab, gameplay must not restore the pre-entry title value:
             * Start may already have selected the Save heading. */
            InputRemap_CloseSaveMenu();
            sDirectOptions = sDialogEntryLatch = 0;
            sSuspendedPageState = 0;
            sFooterReturnPage = sFooterFocus = 0;
            InputRemap_ClearButtonMasks(hidPad, gameHeld, gamePressed,
                                        gameReleased, 0xFFFFFFFF);
            rInputCtx.cur.val = rInputCtx.pressed.val = rInputCtx.up.val = 0;
            volatile u32* cached = (volatile u32*)0x0050AF0C;
            cached[2] = cached[3] = cached[8] = cached[9] = 0;
            return;
        }
        /* Native Options handles edits, Cancel rollback, and OK itself.
         * State 3 is entered only after its exit action has completed.
         * Restore our suspended tab before the Save parent can animate in. */
        if ((dialogKind == 1 && (save[0x3C/4] == 3 || save[0x28/4] != 3)) ||
            (dialogKind == 2 && save[0x28/4] == 2 &&
             (save[0x24/4] == 2 || save[0x24/4] == 14))) {
            if (dialogKind==1) MenuOptions_End();
            ((void (*)(int,int))0x002E9920)(1,0);
            if (sSuspendedPageState) *sSuspendedPageState = sSuspendedPageValue;
            ((void (*)(int))0x002F74A4)(sSuspendedTitle);
            sItemsMenuOpen = 1;
            sActivePausePage = sFooterReturnPage;
            ((void (*)(void))0x002E9A00)();
            if (sActivePausePage == 1) {
                ((void (*)(int,int))0x002EB72C)(*(volatile u32*)0x0050675C,1);
            } else if (sActivePausePage == 2) {
                *(volatile u32*)0x00504490 = 0xFFFFFFFF;
            } else {
                *(volatile u32*)0x00506CE4 = 0xFFFFFFFF;
                *(volatile u32*)0x00506CF0 = 0xFFFFFFFF;
            }
            InputRemap_CloseSaveMenu();
            sFooterReturnPage = 0;
            sFooterFocus = dialogKind == 1 ? 2 : 1;
            sDirectOptions = 0;
            sDialogEntryLatch = 0;
            sSuspendedPageState = 0;
            sFooterNavigationLatch = 1;
            InputRemap_ClearButtonMasks(hidPad, gameHeld, gamePressed,
                                        gameReleased, 0xFFFFFFFF);
            rInputCtx.cur.val = rInputCtx.pressed.val = rInputCtx.up.val = 0;
            volatile u32* cached = (volatile u32*)0x0050AF0C;
            cached[2] = cached[3] = cached[8] = cached[9] = 0;
        }
        /* Do not let legacy Save button counting close Options when A
         * merely changes a setting. The native Options state is authoritative. */
        return;
    }
    if (sFooterTransition) {
        const u32 nativeMode = ((volatile u32*)0x0050AF34)[0x0D];
        /* Bound a failed transition; never keep inputs captured indefinitely. */
        if (++sFooterWaitFrames > 180) {
            sFooterTransition = 0;
            sFooterReturnPage = 0;
        } else if (sFooterTransition == 4) {
            /* Wait for the native Save root before choosing Options. Never
             * send A until the native bottom-row selection is observed. */
            volatile u32* save = (volatile u32*)0x0050A508;
            if (save[0x28/4] == 2 && save[0x24/4] == 1) {
                const u32 button = save[0x38/4] ? BUTTON_A : BUTTON_DOWN;
                InputRemap_ReplaceButtonMasks(hidPad, gameHeld, gamePressed,
                                             gameReleased, 0, button);
                rInputCtx.cur.val |= button;
                rInputCtx.pressed.val |= button;
                sSaveOptionsSelected = 1;
                if (button == BUTTON_A) {
                    sFooterTransition = 0;
                    sSaveOptionsOpen = 1;
                    sSaveConfirmPressCount = 1;
                    sFooterOpenOptions = 0;
                }
            }
            return;
        } else if (sFooterTransition == 3) {
            if (sItemsMenuOpen && sActivePausePage == sFooterReturnPage) {
                sFooterTransition = 0;
                sFooterReturnPage = 0;
            } else if (sFooterTouchFrames) {
                --sFooterTouchFrames;
                InputRemap_InjectTouchPoint((TouchPoint){
                    sFooterReturnPage == 1 ? 220 :
                    sFooterReturnPage == 2 ? 92 : 150, 225 });
                return;
            }
        } else if (nativeMode == 2) {
            if (sFooterTransition == 1) {
                sFooterTransition = sFooterOpenOptions ? 4 : 0;
                sSaveMenuOpen = 1;
                sSaveConfirmPressCount = 0;
                sSaveOptionsSelected = 0;
                sSaveOptionsOpen = 0;
                sSaveModsOpen = 0;
                sSaveDismissFrames = 0;
                sSaveDismissPulseFrames = 0;
                NativeModsMenu_Reset();
                InputRemap_ReplaceButtonMasks(hidPad, gameHeld, gamePressed,
                                             gameReleased, 0, BUTTON_START);
                rInputCtx.cur.val |= BUTTON_START;
                rInputCtx.pressed.val |= BUTTON_START;
                return;
            }
            sFooterTransition = 3;
            sFooterTouchFrames = 3;
        }
        if (sFooterTransition) return;
    }
    if (!sItemsMenuOpen) sFooterFocus = 0;
    if (sItemsMenuOpen && !sFooterFocus && !sFooterNavigationLatch &&
        (rInputCtx.pressed.val & (BUTTON_DOWN | CPAD_DOWN)) &&
        UnifiedMenu_IsAtBottom()) {
        sFooterFocus = 1;
    }
    if (sItemsMenuOpen && (rInputCtx.pressed.val &
            (BUTTON_B | BUTTON_START | BUTTON_SELECT))) {
            /* One exit path regardless of cursor location or dialog return.
             * Native getters retain B until the page accepts the close;
             * clearing our menu flag alone does not close the native page. */
            sFooterFocus = 0;
            sFooterNavigationLatch = 0;
            sDialogEntryLatch = 0;
            sPauseClosing = 1;
            sPauseCloseSettled = 0;
            sItemsMenuOpen = 0;
            sActivePausePage = 0;
            InputRemap_ReplaceButtonMasks(hidPad, gameHeld, gamePressed,
                                         gameReleased, BUTTON_START | BUTTON_SELECT, BUTTON_B);
            rInputCtx.cur.val = (rInputCtx.cur.val & ~(BUTTON_START | BUTTON_SELECT)) | BUTTON_B;
            rInputCtx.pressed.val = (rInputCtx.pressed.val & ~(BUTTON_START | BUTTON_SELECT)) | BUTTON_B;
            return;
    }
    /* Keep footer focus across native tab changes. Route before capture. */
    static u8 footerTabFrames;
    static u8 footerTabTarget;
    if (!sItemsMenuOpen) footerTabFrames = 0;
    if (footerTabFrames && sActivePausePage == footerTabTarget) footerTabFrames = 0;
    if (sItemsMenuOpen && sFooterFocus) {
        const u32 edge = rInputCtx.pressed.val & (BUTTON_L1 | BUTTON_R1);
        if (!footerTabFrames && (edge == BUTTON_L1 || edge == BUTTON_R1)) {
            const u8 page = sActivePausePage >= 1 && sActivePausePage <= 3 ? sActivePausePage : 1;
            footerTabTarget = edge == BUTTON_R1 ? page % 3 + 1 : (page + 1) % 3 + 1;
            footerTabFrames = 3;
        }
        if (footerTabFrames) {
            InputRemap_ClearButtonMasks(hidPad, gameHeld, gamePressed, gameReleased, 0xFFFFFFFF);
            rInputCtx.cur.val = rInputCtx.pressed.val = rInputCtx.up.val = 0;
            --footerTabFrames;
            InputRemap_InjectTouchPoint((TouchPoint){
                footerTabTarget == 1 ? 220 : footerTabTarget == 2 ? 92 : 150, 225 });
            return;
        }
    }
    if (sItemsMenuOpen && sFooterFocus) {
        const u32 pressed = rInputCtx.pressed.val;
        if (pressed & (BUTTON_LEFT | CPAD_LEFT)) sFooterFocus = 1;
        if (pressed & (BUTTON_RIGHT | CPAD_RIGHT)) sFooterFocus = 2;
        const u32 capture = BUTTON_A | BUTTON_B | BUTTON_LEFT | BUTTON_RIGHT |
            BUTTON_UP | BUTTON_DOWN | CPAD_LEFT | CPAD_RIGHT | CPAD_UP |
            CPAD_DOWN | BUTTON_L1 | BUTTON_R1 | BUTTON_SELECT;
        InputRemap_ClearButtonMasks(hidPad, gameHeld, gamePressed, gameReleased,
                                   capture);
        rInputCtx.cur.val &= ~capture;
        rInputCtx.pressed.val &= ~capture;
        if (pressed & (BUTTON_UP | CPAD_UP)) {
            /* Leave the original selection in place and consume the exit
             * direction until release, so Up cannot also move that cursor. */
            sFooterFocus = 0;
            sFooterNavigationLatch = 1;
            return;
        }
        if (pressed & BUTTON_B) { sFooterFocus = 0; return; }
        if (!(pressed & BUTTON_A)) return;
        sFooterOpenOptions = sFooterFocus == 2;
        sFooterFocus = 0;
        sFooterReturnPage = sActivePausePage;
        {
            /* Suspend, rather than close/reopen, the current page. Native
             * page update/draw routines gate on these active-state fields. */
            sSuspendedPageState = (volatile u32*)(sActivePausePage == 1 ?
                0x0050672C : sActivePausePage == 2 ? 0x00504484 : 0x00506CE8);
            sSuspendedPageValue = *sSuspendedPageState;
            sSuspendedTitle = *(volatile u32*)0x004FC68C;
            *sSuspendedPageState = 0;
            ((void (*)(void))0x0043AC88)();
            volatile u32* save = (volatile u32*)0x0050A508;
            if (!sFooterOpenOptions) {
                /* Initialize Save over the suspended tab. Leave its actual
                 * save/write/confirmation states entirely native. */
                save[0x28/4] = 2;
                save[0x24/4] = 0;
                save[0x34/4] = 1;
                save[0x38/4] = 0;
                save[0x2C/4] = 4;
                ((void (*)(GlobalContext*))0x004392A8)(globalCtx);
                sDirectOptions = 2;
                sSaveOptionsOpen = 0;
            } else {
            /* Run native Options setup at its final animation frame. This
             * retains settings backup and resource initialization, without
             * ever presenting or navigating through Save. */
            save[0x28/4] = 2;
            save[0x24/4] = 15;
            save[0x2C/4] = 4;
            ((void (*)(GlobalContext*))0x004392A8)(globalCtx);
            save[0x30/4] = 0;
            save[0x2C/4] = 4;
            ((void (*)(void))0x0043AE94)();
            /* Consume the activation frame in the native release-wait state;
             * otherwise its later input sample can reuse A on setting zero. */
            save[0x3C/4] = 0;
            sDirectOptions = 1;
            sSaveOptionsOpen = 1;
            MenuOptions_Begin();
            }
            sDialogEntryLatch = 1;
            sSaveMenuOpen = 1;
            sSaveConfirmPressCount = 0;
            sFooterOpenOptions = 0;
            sItemsMenuOpen = 0;
            sActivePausePage = 0;
            volatile u32* cached = (volatile u32*)0x0050AF0C;
            cached[2] = cached[3] = cached[8] = cached[9] = 0;
            return;
        }
    }
    if (sSaveMenuOpen && !sSaveOptionsOpen && sFooterReturnPage &&
        (rInputCtx.pressed.val & BUTTON_B)) {
        sFooterTransition = 2;
        sFooterWaitFrames = 0;
    }
#endif

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

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    if (pauseOwnsDpad && InputRemap_IsItemsPageActive() && !sFooterFocus &&
        !sPauseOpening && !sPauseClosing && !sDirectOptions && !sFooterTransition &&
        !(rInputCtx.pressed.val & (BUTTON_B | BUTTON_START | BUTTON_SELECT |
                                  BUTTON_L1 | BUTTON_R1))) {
        /* Ignore ambiguous chords, holds, Up and Right. The destination
         * hook routes this native assignment to I/II, not X/Y. */
        if (pauseDpadPressed == BUTTON_LEFT || pauseDpadPressed == BUTTON_DOWN) {
            InputRemap_BeginItemAssignment(hidPad, gameHeld, gamePressed,
                gameReleased, pauseDpadPressed,
                pauseDpadPressed == BUTTON_LEFT ? BUTTON_X : BUTTON_Y);
            return;
        }
    }
#endif
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
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        sPauseClosing = 1;
        sPauseCloseSettled = 0;
#endif
        sItemsMenuOpen = 0;
        sActivePausePage = 0;
    }
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    /* Route shoulder tab changes through native touch targets, so the game
     * owns closing/opening pages and selection state. No forced manager state.
     */
    static u8 tabTouchFrames;
    static u8 tabTarget;
    if (!sItemsMenuOpen) tabTouchFrames = 0;
    /* A native page can activate before the touch pulse expires. Its active
     * tab is also a close button: never replay the pulse on the destination. */
    if (tabTouchFrames && sActivePausePage == tabTarget) tabTouchFrames = 0;
    if (sItemsMenuOpen && !tabTouchFrames) {
        const u32 edge = rInputCtx.pressed.val & (BUTTON_L1 | BUTTON_R1);
        if (edge == BUTTON_L1 || edge == BUTTON_R1) {
            const u8 page = sActivePausePage >= 1 && sActivePausePage <= 3 ?
                            sActivePausePage : 1;
            tabTarget = edge == BUTTON_R1 ? page % 3 + 1 : (page + 1) % 3 + 1;
            tabTouchFrames = 3;
        }
    }
    if (sItemsMenuOpen) {
        InputRemap_ClearButtonMasks(hidPad, gameHeld, gamePressed, gameReleased,
                                    BUTTON_L1 | BUTTON_R1);
        rInputCtx.cur.val &= ~(BUTTON_L1 | BUTTON_R1);
        rInputCtx.pressed.val &= ~(BUTTON_L1 | BUTTON_R1);
        if (tabTouchFrames) {
            --tabTouchFrames;
            InputRemap_InjectTouchPoint((TouchPoint){
                tabTarget == 1 ? 220 : tabTarget == 2 ? 92 : 150, 225 });
            return;
        }
    }
#endif
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
#if UNIFIED_MENU_PROTOTYPE
        /* Complete only the menu-board transitions here. Never fast-forward
         * Link, note playback, or resource-readiness waits. */
        if (nativeState == 13 || nativeState == 14) {
            ((void (*)(GlobalContext*))0x00425AB4)(globalCtx);
            if (sNativeSongs[0x14/4] == 14) {
                sNativeSongs[0x38/4] = 0;
                ((void (*)(GlobalContext*))0x00425AB4)(globalCtx);
            }
        }
#endif
#endif
        if (sNativeSongs[0x14 / 4] == 4 &&
            sNativeSongs[0x28 / 4] == 0) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
            /* State 4 can be reached by the accelerated update above in
             * this very tick. Allow the native board/render preparation one
             * complete update before replaying its final geometry. */
            if (!sOcarinaLayoutSettled) {
                sOcarinaLayoutSettled = 1;
                return;
            }
#endif
            sOcarinaSongsPending = 0;
            sOcarinaSongTouchFrames = 0;
            return;
        }
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        sOcarinaLayoutSettled = 0;
#endif
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
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
            sOcarinaSongTouchFrames = 0;
#endif
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
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
                    sOcarinaLayoutSettled = 0;
#endif
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
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
                sPauseClosing = 1;
                sPauseCloseSettled = 0;
#endif
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
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
        sPauseOpening = 2;
#endif
    }

    // Always suppress the vanilla Select-as-Start path for this press.
    return 1;
}

// Called at the end of the USA native gameplay-menu update, after its state
// transitions. Observe confirmed native activation, not touchscreen coordinates
// or page constructors (which also run while a save is loading).
void InputRemap_SyncNativeMenu(void* menuManager) {
    const volatile u32* const state = (const volatile u32*)menuManager;
    const u32 mode = state[0x0D];
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    if (sSaveMenuOpen && !sFooterReturnPage && !sDirectOptions &&
        !InputRemap_IsGameOverUiOpen()) {
        /* Start must first complete the native parent handoff (mode 14),
         * including its bookkeeping, before settling the Save manager.
         * Initializing from the raw input tick races that handoff. */
        /* 2EF9B4 increments before testing equality with 4. Supply 3,
         * not 4, so the next native update actually completes the handoff. */
        if (mode == 0x0E) ((volatile u32*)menuManager)[0x17] = 3;
        volatile u32* save = (volatile u32*)0x0050A508;
        if (mode == 0 && save[0x28/4] == 1) {
            extern GlobalContext* gGlobalContext;
            save[0x28/4] = 2;
            save[0x24/4] = 0;
            save[0x2C/4] = 4;
            ((void (*)(GlobalContext*))0x004392A8)(gGlobalContext);
            sDirectOptions = 3;
            sDialogEntryLatch = 1;
            sSuspendedPageState = 0;
        }
    }
    if (sPauseClosing) {
        /* Finish only identified visual exit animations. Native update
         * functions still execute their final geometry and cleanup paths. */
        if (*(volatile u32*)0x0050672C == 14)
            *(volatile u32*)0x00506758 = 4; /* Items close, 445298. */
        if (*(volatile u32*)0x00504484 == 10)
            *(volatile u32*)0x00504488 = 4; /* Gear close, 4385E0. */
        if (mode == 0x0D || mode == 0x11 || mode == 0x15)
            ((volatile u32*)menuManager)[0x17] = 0;
        /* Keep stale preview passes hidden through native teardown and one
         * complete gameplay update. Do not resurrect the outgoing tab. */
        if (mode == 2) {
            if (sPauseCloseSettled) sPauseClosing = 0;
            else sPauseCloseSettled = 1;
        } else sPauseCloseSettled = 0;
        sItemsMenuOpen = 0;
        sActivePausePage = 0;
        return;
    }
#endif
    if (mode == 0x0C || mode == 0x10 || mode == 0x14) {
        sItemsMenuOpen = 1;
        sActivePausePage = mode == 0x0C ? 1 : mode == 0x10 ? 2 : 3;
    } else if (mode == 2) {
        sItemsMenuOpen = 0;
        sActivePausePage = 0;
    }
}

u32 InputRemap_IsItemsMenuOpen(void) {
    // The native transition flag also becomes active while loading a save.
    // Only replay the pause UI after an explicit menu-open request.
    return sItemsMenuOpen;
}

unsigned UnifiedMenu_IsClosing(void) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    return sPauseClosing;
#else
    return 0;
#endif
}

unsigned UnifiedMenu_IsOpening(void) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    return sPauseOpening;
#else
    return 0;
#endif
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
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    /* Native managers can update the destination in this same frame, before
     * InputRemap_Update runs again. Retire our exact touch at activation. */
    if (sSyntheticTouchActive && real_hid.touch.index == sSyntheticTouchIndex) {
        touch_t* touch = &real_hid.touch.touches[sSyntheticTouchIndex];
        if (touch->touch.x == sSyntheticTouchPoint.x &&
            touch->touch.y == sSyntheticTouchPoint.y) touch->updated = 0;
        sSyntheticTouchActive = 0;
    }
#endif
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

unsigned UnifiedMenu_GetDialogPage(void) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    /* Presentation selector, not the navigation return target. Start has
     * no suspended tab, but must use the same modal render path. Keep the
     * actual sFooterReturnPage at zero so cancel returns to gameplay. */
    return sSaveMenuOpen && !InputRemap_IsGameOverUiOpen()
        ? (sFooterReturnPage ? sFooterReturnPage : 1) : 0;
#else
    return 0;
#endif
}

unsigned UnifiedMenu_IsOptionsDialog(void) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    return UnifiedMenu_GetDialogPage() && sSaveOptionsOpen;
#else
    return 0;
#endif
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
