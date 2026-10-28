#ifndef UNIFIED_MENU_LAYOUT_H
#define UNIFIED_MENU_LAYOUT_H

/* Logical top-screen coordinates: 400 x 240, origin at upper left.
 * Keep GPU rotation out of layout decisions. No native state or input writes.
 */
typedef struct { int left, top, width, height; } MenuRect;
typedef struct { unsigned x; int y; unsigned width, height; } MenuViewport;
typedef struct { unsigned left, top, right, bottom; } MenuScissor;

/* Center the visible Gear description (which has native inner margins)
 * between the shelf right (236) and inner frame right (388): center 312. */
#define MENU_GEAR_DETAIL_SHIFT 14
/* Center Map's local-area preview and location banner in the right lane. */
#define MENU_MAP_DETAIL_SHIFT 16
/* Portrait/backdrop previously centered near 305; description centers at
 * 298 before the detail shift. Align both centers at 312. */
#define MENU_GEAR_PORTRAIT_SHIFT 7

extern const MenuRect gMenuItemsSurface;
extern const MenuRect gMenuItemsClip;
extern const MenuRect gMenuGearSurface;
extern const MenuRect gMenuGearClip;
extern const MenuRect gMenuItemPreview;
extern const MenuRect gMenuWideSurface;
extern const MenuRect gMenuWideClip;
extern const MenuRect gMenuWidePreview;
extern const MenuRect gMenuDungeonSurface;
int MenuLayout_Viewport(MenuRect rect, MenuViewport* out);
int MenuLayout_Scissor(MenuRect rect, MenuScissor* out);

#endif
