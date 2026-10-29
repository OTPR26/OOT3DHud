#include <assert.h>
#include <limits.h>
#include "unified_menu_layout.h"

int main(void) {
    MenuViewport v;
    MenuScissor s;
    /* Reproduce validated 1.1 geometry before changing the layout. */
    assert(MenuLayout_Viewport((MenuRect){0, 30, 240, 180}, &v));
    assert(v.x == 60 && v.y == 160 && v.width == 360 && v.height == 240);
    assert(MenuLayout_Scissor((MenuRect){0, 30, 190, 180}, &s));
    assert(s.left == 60 && s.top == 210 && s.right == 419 && s.bottom == 399);
    assert(MenuLayout_Viewport((MenuRect){180, 80, 220, 132}, &v));
    assert(v.x == 56 && v.y == 0 && v.width == 264 && v.height == 220);
    assert(MenuLayout_Scissor((MenuRect){0, 0, 400, 240}, &s));
    assert(s.left == 0 && s.top == 0 && s.right == 479 && s.bottom == 399);
    assert(MenuLayout_Viewport(gMenuItemsSurface, &v));
    assert(MenuLayout_Scissor(gMenuItemsClip, &s));
    assert(s.left >= v.x && s.right == v.x + v.width - 1);
    assert(s.top >= (unsigned)v.y && s.bottom == v.y + v.height - 1);
    assert(MenuLayout_Viewport(gMenuItemPreview, &v));
    /* Preview viewport has native internal padding; visible shelf ends214. */
    assert(gMenuItemsClip.left == 24 && gMenuItemsClip.left + gMenuItemsClip.width == 214);
    assert(gMenuGearSurface.left == 20 && gMenuWideSurface.left == 24);
    const int gearLaneCenter = (gMenuGearSurface.left + gMenuGearSurface.width + 388) / 2;
    assert(298 + MENU_GEAR_DETAIL_SHIFT == gearLaneCenter);
    assert(305 + MENU_GEAR_PORTRAIT_SHIFT == gearLaneCenter);
    assert(MenuLayout_Viewport(gMenuGearSurface, &v));
    assert(MenuLayout_Scissor(gMenuGearClip, &s));
    assert(s.bottom == v.y + v.height - 1);
    assert(MenuLayout_Viewport(gMenuWideSurface, &v));
    assert(MenuLayout_Scissor(gMenuWideClip, &s));
    assert(s.left >= v.x && s.right == v.x + v.width - 1);
    assert(s.top >= (unsigned)v.y && s.bottom == v.y + v.height - 1);
    assert(MenuLayout_Viewport(gMenuWidePreview, &v));
    assert(MenuLayout_Viewport(gMenuDungeonSurface, &v));
    assert(MenuLayout_Scissor(gMenuDungeonSurface, &s));
    assert(s.left == v.x && s.right == v.x + v.width - 1);
    assert(s.top == (unsigned)v.y && s.bottom == v.y + v.height - 1);
    assert(gMenuDungeonSurface.top + gMenuDungeonSurface.height + 3 < 200);
    assert(gMenuWidePreview.width * 120 == gMenuWidePreview.height * 180);
    assert(gMenuWidePreview.left + gMenuWidePreview.width ==
           gMenuItemPreview.left + gMenuItemPreview.width);
    assert(gMenuWidePreview.top + gMenuWidePreview.height == 190);
    assert(gMenuItemPreview.top + gMenuItemPreview.height == 190);
    assert(gMenuItemPreview.left + gMenuItemPreview.width == 388);
    assert(gMenuItemPreview.width * 120 == gMenuItemPreview.height * 180);
    /* Ten logical pixels clear of the shared footer rail at y=200. */
    assert(gMenuWidePreview.top + gMenuWidePreview.height + 10 == 200);
    assert(!MenuLayout_Viewport((MenuRect){-1, 0, 1, 1}, &v));
    assert(!MenuLayout_Viewport((MenuRect){0, 0, 0, 1}, &v));
    assert(!MenuLayout_Viewport((MenuRect){0, 0, INT_MAX, 1}, &v));
    assert(!MenuLayout_Viewport((MenuRect){399, 0, 2, 1}, &v));
    assert(!MenuLayout_Viewport(gMenuItemsSurface, 0));
    assert(!MenuLayout_Scissor(gMenuItemsClip, 0));
    return 0;
}
