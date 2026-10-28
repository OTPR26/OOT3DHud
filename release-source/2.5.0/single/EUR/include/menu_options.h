#pragma once
#include "z3D/z3D.h"
void MenuOptions_Begin(void);
void MenuOptions_End(void);
int MenuOptions_Update(u32 pressed);
int MenuOptions_Touch(int x,int y);
void MenuOptions_Draw(void);
u32 MenuOptions_Lookup(void* context, u32 id, u32* record);
u32 MenuOptions_NativeButtons(void);
void MenuOptions_ExitNative(int accept);
