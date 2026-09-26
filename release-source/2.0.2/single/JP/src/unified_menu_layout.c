#include "unified_menu_layout.h"

/* Shared pause-menu content band. Preserve native inventory hit geometry;
 * use a common preview baseline and twelve-unit outside margin on all tabs.
 */
const MenuRect gMenuItemsSurface = {24, 44, 240, 168};
const MenuRect gMenuItemsClip = {24, 44, 190, 138};
const MenuRect gMenuGearSurface = {20, 44, 216, 168};
const MenuRect gMenuGearClip = {20, 44, 216, 138};
const MenuRect gMenuItemPreview = {208, 70, 180, 120};
/* Gear/Map need their full native width, unlike the cropped Items shelf.
 * Reserve a separate preview lane with the same bottom/right inset. */
const MenuRect gMenuWideSurface = {24, 44, 216, 168};
const MenuRect gMenuWideClip = {24, 44, 216, 138};
/* Preserve the validated 1.1 preview ratio (180:120). The native preview
 * includes its own left inset; its viewport may overlap the content lane
 * without its visible model or description touching the Gear panel. */
const MenuRect gMenuWidePreview = {208, 70, 180, 120};
/* Dungeon floor selectors extend below the overworld's content cutoff.
 * Fit the entire native board, preserving approximately the previous aspect
 * ratio, rather than clipping the last floor to the common shelf height. */
const MenuRect gMenuDungeonSurface = {29, 44, 182, 142};

static int ValidRect(MenuRect r) {
    return r.left >= 0 && r.top >= 0 && r.width > 0 && r.height > 0 &&
           r.left <= 400 && r.top <= 240 &&
           r.width <= 400 - r.left && r.height <= 240 - r.top;
}

int MenuLayout_Viewport(MenuRect r, MenuViewport* out) {
    if (!out || !ValidRect(r)) return 0;
    out->x = 2 * (240 - r.top - r.height);
    out->y = 400 - r.left - r.width;
    out->width = 2 * r.height;
    out->height = r.width;
    return 1;
}

int MenuLayout_Scissor(MenuRect r, MenuScissor* out) {
    MenuViewport v;
    if (!out || !MenuLayout_Viewport(r, &v)) return 0;
    out->left = v.x;
    out->top = v.y;
    out->right = v.x + v.width - 1;
    out->bottom = v.y + v.height - 1;
    return 1;
}
