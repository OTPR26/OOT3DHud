#ifndef INPUT_CONSUME_H
#define INPUT_CONSUME_H

#include <stdint.h>

#include "hid.h"

// Consume a physical button from the game's writable controller state.
// Select must be removed from every mask because vanilla treats it as Start.
static inline void InputRemap_ClearButtonMasks(
    volatile uint32_t* held, volatile uint32_t* pressed,
    volatile uint32_t* released, uint32_t button) {
    *held &= ~button;
    *pressed &= ~button;
    *released &= ~button;
}

// Replace a physical button edge with the button a native menu already
// understands. This keeps menu behavior inside Grezzo's own state machine.
static inline void InputRemap_ReplaceButtonMasks(
    volatile uint32_t* held, volatile uint32_t* pressed,
    volatile uint32_t* released, uint32_t from, uint32_t to) {
    InputRemap_ClearButtonMasks(held, pressed, released, from);
    *held |= to;
    *pressed |= to;
}

#endif // INPUT_CONSUME_H
