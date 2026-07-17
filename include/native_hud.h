#pragma once

#include "z3D/z3D.h"

// Enables the repurposed native top-screen quad during the HD renderer proof.
void NativeHud_UpdateProof(GlobalContext* globalCtx);

// Temporarily moves the live A-action source before the game's normal sync,
// then restores it after the transformed geometry has been uploaded.
void NativeHud_PrepareActionPrompt(void);
void NativeHud_RestoreActionPrompt(void);
