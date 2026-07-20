#ifndef INPUT_CONSUME_H
#define INPUT_CONSUME_H

#include <stdint.h>

#include "hid.h"

// Consume a physical button from both the active HID ring sample and the
// controller state OoT3D may already have sampled for the current frame.
// Select must be removed from every mask because vanilla treats it as Start.
static inline void InputRemap_ClearButtonMasks(
    pad_t* pad, volatile uint32_t* held, volatile uint32_t* pressed,
    volatile uint32_t* released, uint32_t button) {
    pad->curr.val &= ~button;
    pad->pressed.val &= ~button;
    pad->released.val &= ~button;
    *held &= ~button;
    *pressed &= ~button;
    *released &= ~button;
}

#endif // INPUT_CONSUME_H
