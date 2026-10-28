#include "regional_menu_bindings.h"
#include "menu_options.h"
#include "unified_menu.h"

void DualMenu_DrawDialogue(void* message) {
    if (UnifiedMenu_IsOptionsDialog()) {
        MenuOptions_Draw();
        return;
    }
    ((void (*)(void*))MENU_DIALOGUE_TEXT)(message);
}
