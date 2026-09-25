#pragma once
static inline float MovementOptions_Scale(unsigned value) {
    return value < 3 ? 1.0f + 0.1f * value : 1.0f;
}
/* The normal roll permits recovery actions at frame 15 and exits at 20.
 * Shorten only that final five-frame window; preserve earlier events. */
static inline float MovementOptions_EndFrame(unsigned value) {
    return value < 3 ? 20.0f - 0.5f * value : 20.0f;
}
