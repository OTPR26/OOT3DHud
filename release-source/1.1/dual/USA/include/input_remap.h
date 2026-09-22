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
