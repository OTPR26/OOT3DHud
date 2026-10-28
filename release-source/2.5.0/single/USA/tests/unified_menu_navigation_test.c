#include <assert.h>
#include "unified_menu_navigation.h"

int main(void) {
    for (unsigned mode = 0; mode < 32; ++mode)
        assert(MenuPause_CanDeliverClose(mode) == (mode == 0x0C || mode == 0x10 || mode == 0x14));
    unsigned floors[5] = {1, 1, 1, 0, 0};
    assert(MenuMap_DungeonCanMoveDown(0, floors));
    assert(MenuMap_DungeonCanMoveDown(1, floors));
    assert(!MenuMap_DungeonCanMoveDown(2, floors));
    assert(!MenuMap_DungeonCanMoveDown(4, floors));
    assert(MenuMap_DungeonCanMoveDown(5, floors));
    assert(MenuMap_DungeonCanMoveDown(6, floors));
    assert(!MenuMap_DungeonCanMoveDown(7, floors));
    assert(!MenuMap_DungeonCanMoveDown(-1, floors));
    unsigned char neighbours[96];
    unsigned available[12] = {0};
    for (unsigned i = 0; i < 96; ++i)
        neighbours[i] = 255;
    neighbours[4] = 1;
    neighbours[12] = 2;
    assert(!MenuMap_WorldCanMoveDown(0, neighbours, available));
    available[2] = 1;
    assert(MenuMap_WorldCanMoveDown(0, neighbours, available));
    available[2] = 0;
    available[1] = 1;
    assert(MenuMap_WorldCanMoveDown(0, neighbours, available));
    assert(!MenuMap_WorldCanMoveDown(2, neighbours, available));
    assert(!MenuMap_WorldCanMoveDown(12, neighbours, available));
    return 0;
}
