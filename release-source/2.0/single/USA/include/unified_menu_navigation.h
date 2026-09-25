#ifndef UNIFIED_MENU_NAVIGATION_H
#define UNIFIED_MENU_NAVIGATION_H

/* Only stable pause pages may consume a pending close edge. Never inject
 * that edge into gameplay or a Save/Options confirmation. */
static inline int MenuPause_CanDeliverClose(unsigned mode) {
    return mode == 0x0C || mode == 0x10 || mode == 0x14;
}

/* Match USA native Map Down traversal without changing native selection. */
static inline int MenuMap_DungeonCanMoveDown(int selected, const volatile unsigned* floors) {
    if (selected < 0 || selected > 7)
        return 0;
    if (selected < 4)
        return floors[selected + 1] != 0;
    return selected == 5 || selected == 6;
}

static inline int MenuMap_WorldCanMoveDown(unsigned selected,
                                           const volatile unsigned char* neighbours,
                                           const volatile unsigned* available) {
    if (selected >= 12)
        return 0;
    unsigned next = neighbours[selected * 8 + 4];
    if (next >= 12)
        return 0;
    if (available[next])
        return 1;
    next = neighbours[next * 8 + 4];
    return next < 12 && available[next];
}
#endif
