#pragma once

#include "controls.h"
#include "z3D/z3D.h"

// Routes physical D-pad presses. Camera configuration is applied immediately;
// gameplay actions are forwarded to the target adapter, which must invoke the
// corresponding vanilla OoT3D UI action rather than duplicating item logic.
void InputRemap_Update(GlobalContext* globalCtx);

// Target adapter. This remains deliberately isolated while the USA vanilla
// touchscreen-action flags and their update timing are being verified.
void InputRemap_ApplyVanillaAction(GlobalContext* globalCtx, ControlAction action);

