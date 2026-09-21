#pragma once

#include <stdbool.h>

// USA test implementation of the native Save-screen Mods button. The feature
// is compiled only in controller-options builds; release/default builds remain
// byte-for-byte unaffected by this module.
void NativeModsMenu_Reset(void);
void NativeModsMenu_SetSelected(bool selected);
bool NativeModsMenu_IsSelected(void);
bool NativeModsMenu_IsVisible(void);
void NativeModsMenu_Update(void);
