#pragma once

#include "z3D/z3D.h"

// Updates the repurposed native top-screen board from live game state.
void NativeHud_Update(GlobalContext* globalCtx);

// Temporarily moves the live A-action source before the game's normal sync,
// then restores it after the transformed geometry has been uploaded.
void NativeHud_PrepareActionPrompt(void);
void NativeHud_RestoreActionPrompt(void);
