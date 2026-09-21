#pragma once

#include "controls.h"

void Camera_ApplyControlAction(ControlAction action);
void Camera_DrawSettingsOverlay(void);

uint8_t Camera_GetControlMode(void);
void Camera_SetControlMode(uint8_t mode);
uint8_t Camera_GetSpeedOption(void);
void Camera_SetSpeedOption(uint8_t option);
