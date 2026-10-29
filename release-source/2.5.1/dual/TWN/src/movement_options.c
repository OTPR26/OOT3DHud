#include "movement_options.h"
#include "movement_options_model.h"
#include "input_remap.h"
#include "regional_menu_bindings.h"
#include "manual_jump_bindings.h"

static u8 sOptions[3];
u8 MovementOptions_Get(unsigned option) {
    return option < 3 ? sOptions[option] : 0;
}
void MovementOptions_Set(unsigned option, unsigned value) {
    if (option < 3)
        sOptions[option] = value < 3 ? value : 0;
}
static int Active(const Player* player) {
    return player && gSaveContext.gameMode == 0 && !InputRemap_IsItemsMenuOpen() &&
           !InputRemap_IsSaveMenuOpen();
}
float MovementOptions_Multiplier(const Player* player) {
#if UNIFIED_MENU_PROTOTYPE && (defined(Version_TWN) || defined(Version_KOR))
    if (Active(player)) {
        /* Reviewed regional action pointers. Scale the velocity submitted
         * to native movement, never multiply the stored speed every tick. */
        if ((u32)player->stateFuncPtr == MANUAL_JUMP_ACTION_A)
            return MovementOptions_Scale(sOptions[0]);
        if ((u32)player->stateFuncPtr == MENU_ROLL_ACTION &&
            *(const s16*)((const u8*)player + 0x2238) == 0)
            return MovementOptions_Scale(sOptions[1]);
    }
#endif
    return 1.0f;
}
float MovementOptions_RecoveryEnd(const Player* player) {
#if UNIFIED_MENU_PROTOTYPE && (defined(Version_TWN) || defined(Version_KOR))
    if (Active(player) && (u32)player->stateFuncPtr == MENU_ROLL_ACTION &&
        *(const s16*)((const u8*)player + 0x2238) == 0)
        return MovementOptions_EndFrame(sOptions[2]);
#endif
    return 20.0f;
}
