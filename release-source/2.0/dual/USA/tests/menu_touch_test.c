#include <assert.h>
#include "menu_touch.h"
int main(void) {
    assert(MenuTouch_X(160) == 200);
    assert(MenuTouch_Tab(37, 20) == 1);
    assert(MenuTouch_Tab(93, 20) == 2);
    assert(MenuTouch_Tab(149, 20) == 3);
    assert(!MenuTouch_Tab(65, 20));
    assert(!MenuTouch_Tab(149, 32));
    assert(MenuTouch_Footer(80, 219) == 1);
    assert(MenuTouch_Footer(320, 219) == 2);
    assert(!MenuTouch_Footer(200, 219));
    int x, y;
    assert(MenuTouch_Save(172, 95, &x, &y) && x >= 39 && x < 159);
    assert(MenuTouch_Save(227, 95, &x, &y) && x >= 164 && x < 284);
    assert(!MenuTouch_Save(80, 219, &x, &y));
    assert(!MenuTouch_Save(200, 95, &x, &y));
    assert(!MenuTouch_Save(200, 140, &x, &y));
    return 0;
}
