#pragma once

#include "z3D/z3D.h"

// Updates the repurposed native top-screen board from live game state.
void NativeHud_Update(GlobalContext* globalCtx);

// Captures the base geometry and applies the default size before the game
// constructs the native board.
void NativeHud_InitializeScale(void);

// Cycles the live HUD between 75%, 100%, 125%, and hidden.
void NativeHud_CycleScale(void);

// Temporarily moves the live A-action source before the game's normal sync,
// then restores it after the transformed geometry has been uploaded.
void NativeHud_PrepareActionPrompt(void);
void NativeHud_RestoreActionPrompt(void);
