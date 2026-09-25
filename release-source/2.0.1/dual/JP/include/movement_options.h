#pragma once
#include "z3D/z3D.h"
u8 MovementOptions_Get(unsigned option);
void MovementOptions_Set(unsigned option,unsigned value);
float MovementOptions_Multiplier(const Player* player);
float MovementOptions_RecoveryEnd(const Player* player);
