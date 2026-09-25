#include "controller_settings.h"

#include "camera.h"
#include "draw.h"

// Keep this module host-testable without importing the game's pointer-sized
// GlobalContext definitions through native_hud.h.
uint8_t NativeHud_GetScalePercent(void);
void NativeHud_SetScalePercent(uint8_t percent);

typedef enum {
    CONTROLLER_SETTING_HUD_SIZE = 0,
    CONTROLLER_SETTING_CAMERA_MODE,
    CONTROLLER_SETTING_CAMERA_SPEED,
    CONTROLLER_SETTING_COUNT,
} ControllerSettingRow;

static uint8_t sSettingsOpen;
static uint8_t sSelectedRow;
static ControllerGlyph sGlyph = CONTROLLER_GLYPH_NINTENDO_A;
static uint8_t sSwapAB;

bool ControllerSettings_IsOpen(void) {
    return sSettingsOpen != 0;
}

void ControllerSettings_Open(void) {
    sSettingsOpen = 1;
    sSelectedRow = 0;
}

static const char* ControllerSettings_CameraModeName(void) {
    switch (Camera_GetControlMode()) {
        case 1: return "Invert X";
        case 2: return "Invert Y";
        case 3: return "Invert X+Y";
        default: return "Normal";
    }
}

void ControllerSettings_Close(void) {
    sSettingsOpen = 0;
    sSelectedRow = 0;
}

bool ControllerSettings_HandleOptionsInput(uint32_t pressed) {
    if (!sSettingsOpen) {
        return false;
    }

    if (pressed & (BUTTON_L1 | BUTTON_R1 | BUTTON_B)) {
        ControllerSettings_Close();
        return true;
    }

    if (pressed & (BUTTON_UP | CPAD_UP)) {
        sSelectedRow = sSelectedRow == 0 ?
            CONTROLLER_SETTING_COUNT - 1 : sSelectedRow - 1;
    } else if (pressed & (BUTTON_DOWN | CPAD_DOWN)) {
        sSelectedRow = (sSelectedRow + 1) % CONTROLLER_SETTING_COUNT;
    }

    const bool previous = (pressed & (BUTTON_LEFT | CPAD_LEFT)) != 0;
    const bool next = (pressed & (BUTTON_RIGHT | CPAD_RIGHT)) != 0;
    if (!previous && !next) {
        return true;
    }

    if (sSelectedRow == CONTROLLER_SETTING_HUD_SIZE) {
        const u8 scale = NativeHud_GetScalePercent();
        if (previous) {
            NativeHud_SetScalePercent(scale == 0 ? 125 :
                                      scale == 75 ? 0 :
                                      scale == 100 ? 75 : 100);
        } else {
            NativeHud_SetScalePercent(scale == 0 ? 75 :
                                      scale == 75 ? 100 :
                                      scale == 100 ? 125 : 0);
        }
    } else if (sSelectedRow == CONTROLLER_SETTING_CAMERA_MODE) {
        const u8 mode = Camera_GetControlMode();
        Camera_SetControlMode(previous ? (mode + 3) & 3 : (mode + 1) & 3);
    } else {
        const u8 option = Camera_GetSpeedOption();
        Camera_SetSpeedOption(previous ? (option == 0 ? 6 : option - 1) :
                                        (option == 6 ? 0 : option + 1));
    }
    return true;
}

void ControllerSettings_Draw(void) {
    if (!sSettingsOpen) {
        return;
    }

    Draw_DrawRectTop(55, 34, 290, 174, COLOR_BLACK);
    Draw_DrawRectOutlineTop(55, 34, 290, 174, COLOR_WHITE);
    Draw_DrawStringTop(82, 48, COLOR_TITLE, "REFRAMED OPTIONS");
    Draw_DrawStringTop(279, 48, COLOR_GRAY, "2 / 2");
    Draw_DrawFormattedStringTop(
        72, 79,
        sSelectedRow == CONTROLLER_SETTING_HUD_SIZE ? COLOR_GREEN : COLOR_WHITE,
        "%c HUD size: %s",
        sSelectedRow == CONTROLLER_SETTING_HUD_SIZE ? '>' : ' ',
        NativeHud_GetScalePercent() == 0 ? "Off" :
        NativeHud_GetScalePercent() == 75 ? "75%" :
        NativeHud_GetScalePercent() == 125 ? "125%" : "100%");
    Draw_DrawFormattedStringTop(
        72, 105,
        sSelectedRow == CONTROLLER_SETTING_CAMERA_MODE ? COLOR_GREEN : COLOR_WHITE,
        "%c Free camera: %s",
        sSelectedRow == CONTROLLER_SETTING_CAMERA_MODE ? '>' : ' ',
        ControllerSettings_CameraModeName());
    Draw_DrawFormattedStringTop(
        72, 131,
        sSelectedRow == CONTROLLER_SETTING_CAMERA_SPEED ? COLOR_GREEN : COLOR_WHITE,
        "%c Camera speed: %u / 7",
        sSelectedRow == CONTROLLER_SETTING_CAMERA_SPEED ? '>' : ' ',
        Camera_GetSpeedOption() + 1);
    Draw_DrawStringTop(72, 164, COLOR_GRAY, "D-pad changes options");
    Draw_DrawStringTop(72, 181, COLOR_GRAY, "L / R or B: game options");
}

ControllerGlyph ControllerSettings_GetGlyph(void) {
    return sGlyph;
}

bool ControllerSettings_ShouldSwapAB(void) {
    return sSwapAB != 0;
}

uint32_t ControllerSettings_SwapABBits(uint32_t buttons) {
    const uint32_t a = buttons & BUTTON_A;
    const uint32_t b = buttons & BUTTON_B;
    buttons &= ~(BUTTON_A | BUTTON_B);
    if (a) buttons |= BUTTON_B;
    if (b) buttons |= BUTTON_A;
    return buttons;
}

void ControllerSettings_ApplyABSwap(
    pad_t* pad, volatile uint32_t* held, volatile uint32_t* pressed,
    volatile uint32_t* released, uint32_t* sampledHeld,
    uint32_t* sampledPressed, uint32_t* sampledReleased) {
    if (!sSwapAB) {
        return;
    }
    pad->curr.val = ControllerSettings_SwapABBits(pad->curr.val);
    pad->pressed.val = ControllerSettings_SwapABBits(pad->pressed.val);
    pad->released.val = ControllerSettings_SwapABBits(pad->released.val);
    *held = ControllerSettings_SwapABBits(*held);
    *pressed = ControllerSettings_SwapABBits(*pressed);
    *released = ControllerSettings_SwapABBits(*released);
    *sampledHeld = ControllerSettings_SwapABBits(*sampledHeld);
    *sampledPressed = ControllerSettings_SwapABBits(*sampledPressed);
    *sampledReleased = ControllerSettings_SwapABBits(*sampledReleased);
}
