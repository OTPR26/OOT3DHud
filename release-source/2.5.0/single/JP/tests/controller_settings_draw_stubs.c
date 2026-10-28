#include <stdarg.h>

#include "draw.h"

static u8 sHudScale = 100;
static u8 sCameraMode;
static u8 sCameraSpeed = 3;

u8 NativeHud_GetScalePercent(void) { return sHudScale; }
void NativeHud_SetScalePercent(u8 percent) { sHudScale = percent; }
u8 Camera_GetControlMode(void) { return sCameraMode; }
void Camera_SetControlMode(u8 mode) { sCameraMode = mode; }
u8 Camera_GetSpeedOption(void) { return sCameraSpeed; }
void Camera_SetSpeedOption(u8 option) { sCameraSpeed = option; }

void Draw_DrawRectTop(u32 x, u32 y, u32 width, u32 height, u32 color) {
    (void)x; (void)y; (void)width; (void)height; (void)color;
}

void Draw_DrawRectOutlineTop(u32 x, u32 y, u32 width, u32 height, u32 color) {
    (void)x; (void)y; (void)width; (void)height; (void)color;
}

u32 Draw_DrawStringTop(u32 x, u32 y, u32 color, const char* string) {
    (void)x; (void)color; (void)string; return y;
}

u32 Draw_DrawFormattedStringTop(u32 x, u32 y, u32 color,
                                const char* format, ...) {
    (void)x; (void)color; (void)format; return y;
}

void Draw_FlushFramebufferTop(void) {}
