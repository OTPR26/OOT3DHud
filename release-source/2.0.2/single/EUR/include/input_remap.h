#pragma once

#include "controls.h"
#include "z3D/z3D.h"

// Routes physical buttons to camera settings and native OoT3D actions.
void InputRemap_Update(GlobalContext* globalCtx);

// Sends a D-pad shortcut through the game's existing touchscreen handler.
void InputRemap_ApplyVanillaAction(ControlAction action);

// Called from OoT3D's native HUD menu manager at the point where it would
// otherwise treat Select as Start and open the Save prompt.
u32 InputRemap_TryOpenItemsMenu(void* menuManager);

// Tracks the lower-screen interfaces replayed by the single-screen renderer.
u32 InputRemap_IsItemsMenuOpen(void);
u32 InputRemap_IsItemsPageActive(void);
u32 InputRemap_IsGearPageActive(void);
void InputRemap_OnPausePageActivated(u32 page);
u32 InputRemap_IsOcarinaUiOpen(void);
u32 InputRemap_IsOcarinaUiReady(void);
u32 InputRemap_IsSaveMenuOpen(void);
u32 InputRemap_IsGameOverUiOpen(void);
void InputRemap_CloseOcarinaUi(void);
void InputRemap_CloseSaveMenu(void);
