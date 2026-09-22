#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hid.h"

typedef enum {
    CONTROLLER_GLYPH_NINTENDO_A = 0,
    CONTROLLER_GLYPH_XBOX_A,
    CONTROLLER_GLYPH_XBOX_B,
    CONTROLLER_GLYPH_PLAYSTATION_CROSS,
    CONTROLLER_GLYPH_PLAYSTATION_CIRCLE,
    CONTROLLER_GLYPH_COUNT,
} ControllerGlyph;

bool ControllerSettings_IsOpen(void);
void ControllerSettings_Open(void);
bool ControllerSettings_HandleOptionsInput(uint32_t pressed);
void ControllerSettings_Close(void);
void ControllerSettings_Draw(void);

ControllerGlyph ControllerSettings_GetGlyph(void);
bool ControllerSettings_ShouldSwapAB(void);

uint32_t ControllerSettings_SwapABBits(uint32_t buttons);
void ControllerSettings_ApplyABSwap(
    pad_t* pad, volatile uint32_t* held, volatile uint32_t* pressed,
    volatile uint32_t* released, uint32_t* sampledHeld,
    uint32_t* sampledPressed, uint32_t* sampledReleased);
