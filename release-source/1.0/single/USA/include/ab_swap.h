#ifndef AB_SWAP_H
#define AB_SWAP_H

#include <stdint.h>

#include "hid.h"

static inline uint32_t ABSwap_Bits(uint32_t buttons) {
    const uint32_t a = buttons & BUTTON_A;
    const uint32_t b = buttons & BUTTON_B;
    buttons &= ~(BUTTON_A | BUTTON_B);
    if (a) buttons |= BUTTON_B;
    if (b) buttons |= BUTTON_A;
    return buttons;
}

static inline void ABSwap_Apply(
    pad_t* pad, volatile uint32_t* held, volatile uint32_t* pressed,
    volatile uint32_t* released, uint32_t* sampledHeld,
    uint32_t* sampledPressed, uint32_t* sampledReleased) {
    pad->curr.val = ABSwap_Bits(pad->curr.val);
    pad->pressed.val = ABSwap_Bits(pad->pressed.val);
    pad->released.val = ABSwap_Bits(pad->released.val);
    *held = ABSwap_Bits(*held);
    *pressed = ABSwap_Bits(*pressed);
    *released = ABSwap_Bits(*released);
    *sampledHeld = ABSwap_Bits(*sampledHeld);
    *sampledPressed = ABSwap_Bits(*sampledPressed);
    *sampledReleased = ABSwap_Bits(*sampledReleased);
}

#endif
