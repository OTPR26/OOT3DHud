#include "hud.h"

#include "common.h"
#include "draw.h"
#include "z3D/z3Ditem.h"

#if defined(__has_include)
#if __has_include("generated/project_restoration_hud.h")
#include "generated/project_restoration_hud.h"
#endif
#endif

#define COLOR_A RGB8(0x44, 0xD1, 0x78)
#define COLOR_B RGB8(0xF0, 0x55, 0x55)
#define COLOR_X RGB8(0x58, 0x9D, 0xE8)
#define COLOR_Y RGB8(0xF1, 0xC7, 0x4B)
#define COLOR_DPAD RGB8(0xD8, 0xDD, 0xE6)
#define COLOR_MUTED RGB8(0x68, 0x70, 0x7C)
#define COLOR_HEART RGB8(0xF1, 0x48, 0x62)
#define COLOR_MAGIC RGB8(0x35, 0xD0, 0x6F)
#define COLOR_RUPEE RGB8(0x45, 0xDE, 0xA1)

typedef enum {
    HUD_ICON_NONE,
    HUD_ICON_STICK,
    HUD_ICON_NUT,
    HUD_ICON_BOMB,
    HUD_ICON_BOW,
    HUD_ICON_MAGIC,
    HUD_ICON_SLINGSHOT,
    HUD_ICON_OCARINA,
    HUD_ICON_BOMBCHU,
    HUD_ICON_HOOKSHOT,
    HUD_ICON_BOOMERANG,
    HUD_ICON_LENS,
    HUD_ICON_BEAN,
    HUD_ICON_HAMMER,
    HUD_ICON_BOTTLE,
    HUD_ICON_TRADE,
    HUD_ICON_BOOTS,
} HudIcon;

// Original monochrome 8x8 silhouettes. They keep the first live-HUD build
// independent of extracted Nintendo textures.
static const u8 sHudIcons[][8] = {
    [HUD_ICON_NONE]      = {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00},
    [HUD_ICON_STICK]     = {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80},
    [HUD_ICON_NUT]       = {0x18, 0x3C, 0x7E, 0xDB, 0xFF, 0x7E, 0x3C, 0x18},
    [HUD_ICON_BOMB]      = {0x0C, 0x1A, 0x08, 0x3C, 0x7E, 0x7E, 0x3C, 0x18},
    [HUD_ICON_BOW]       = {0x42, 0x64, 0x58, 0xFF, 0x58, 0x64, 0x42, 0x00},
    [HUD_ICON_MAGIC]     = {0x18, 0x3C, 0x7E, 0xDB, 0x18, 0x18, 0x18, 0x00},
    [HUD_ICON_SLINGSHOT] = {0x42, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x00},
    [HUD_ICON_OCARINA]   = {0x00, 0x3E, 0x7F, 0xED, 0x7F, 0x3E, 0x0C, 0x08},
    [HUD_ICON_BOMBCHU]   = {0x24, 0x5A, 0x7E, 0xDB, 0xFF, 0x7E, 0x24, 0x42},
    [HUD_ICON_HOOKSHOT]  = {0xE0, 0x70, 0x38, 0x1F, 0x1F, 0x38, 0x70, 0xE0},
    [HUD_ICON_BOOMERANG] = {0xC0, 0x70, 0x1C, 0x07, 0x0E, 0x38, 0x60, 0x00},
    [HUD_ICON_LENS]      = {0x3C, 0x7E, 0xDB, 0xBD, 0xBD, 0xDB, 0x7E, 0x3C},
    [HUD_ICON_BEAN]      = {0x00, 0x38, 0x7C, 0xCE, 0xC6, 0x7C, 0x38, 0x00},
    [HUD_ICON_HAMMER]    = {0x7E, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    [HUD_ICON_BOTTLE]    = {0x18, 0x18, 0x3C, 0x24, 0x42, 0x42, 0x7E, 0x00},
    [HUD_ICON_TRADE]     = {0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x18, 0x00},
    [HUD_ICON_BOOTS]     = {0x30, 0x30, 0x30, 0x38, 0x3E, 0x7F, 0x7F, 0x00},
};

static HudIcon Hud_ItemIcon(u8 item) {
    switch (item) {
        case ITEM_STICK: return HUD_ICON_STICK;
        case ITEM_NUT: return HUD_ICON_NUT;
        case ITEM_BOMB: return HUD_ICON_BOMB;
        case ITEM_BOW: return HUD_ICON_BOW;
        case ITEM_ARROW_FIRE:
        case ITEM_DINS_FIRE:
        case ITEM_ARROW_ICE:
        case ITEM_FARORES_WIND:
        case ITEM_ARROW_LIGHT:
        case ITEM_NAYRUS_LOVE:
        case ITEM_BOW_ARROW_FIRE:
        case ITEM_BOW_ARROW_ICE:
        case ITEM_BOW_ARROW_LIGHT: return HUD_ICON_MAGIC;
        case ITEM_SLINGSHOT: return HUD_ICON_SLINGSHOT;
        case ITEM_OCARINA_FAIRY:
        case ITEM_OCARINA_TIME: return HUD_ICON_OCARINA;
        case ITEM_BOMBCHU: return HUD_ICON_BOMBCHU;
        case ITEM_HOOKSHOT:
        case ITEM_LONGSHOT: return HUD_ICON_HOOKSHOT;
        case ITEM_BOOMERANG: return HUD_ICON_BOOMERANG;
        case ITEM_LENS: return HUD_ICON_LENS;
        case ITEM_BEAN: return HUD_ICON_BEAN;
        case ITEM_HAMMER: return HUD_ICON_HAMMER;
        case ITEM_BOTTLE:
        case ITEM_POTION_RED:
        case ITEM_POTION_GREEN:
        case ITEM_POTION_BLUE:
        case ITEM_FAIRY:
        case ITEM_FISH:
        case ITEM_MILK_BOTTLE:
        case ITEM_LETTER_RUTO:
        case ITEM_BLUE_FIRE:
        case ITEM_BUG:
        case ITEM_BIG_POE:
        case ITEM_MILK_HALF:
        case ITEM_POE: return HUD_ICON_BOTTLE;
        case ITEM_BOOTS_IRON:
        case ITEM_BOOTS_HOVER: return HUD_ICON_BOOTS;
        case ITEM_NONE: return HUD_ICON_NONE;
        default: return HUD_ICON_TRADE;
    }
}

static void Hud_DrawIcon(u32 x, u32 y, u32 color, HudIcon icon) {
    for (u32 row = 0; row < 8; row++) {
        for (u32 col = 0; col < 8; col++) {
            if (sHudIcons[icon][row] & (0x80 >> col)) {
                Draw_DrawPixelTop(x + col, y + row, color);
            }
        }
    }
}

static void Hud_DrawEquippedItem(u32 x, u32 y, u8 item) {
#if defined(HUD_HAS_OOT_ITEM_ASSETS)
    if (item < OOT_ITEM_ICON_COUNT) {
        const u32 stride = OOT_ITEM_ICON_WIDTH * OOT_ITEM_ICON_HEIGHT;
        Draw_DrawSpriteTop(x, y, OOT_ITEM_ICON_WIDTH, OOT_ITEM_ICON_HEIGHT,
                           &OOT_ITEM_ICONS[(u32)item * stride]);
        return;
    }
#endif
    Hud_DrawIcon(x + 1, y + 1, item == ITEM_NONE ? COLOR_MUTED : COLOR_DPAD,
                 Hud_ItemIcon(item));
}

static void __attribute__((unused)) Hud_DrawDiamondButton(u32 centerX, u32 centerY, u32 color, char label) {
    const s32 radius = 9;
    for (s32 y = -radius; y <= radius; y++) {
        const s32 x = radius - (y < 0 ? -y : y);
        Draw_DrawPixelTop(centerX - x, centerY + y, color);
        Draw_DrawPixelTop(centerX + x, centerY + y, color);
    }
    Draw_DrawOverlaidCharacterTop(centerX - 2, centerY - 5, color, label);
}

static void Hud_DrawABXY(void) {
#if defined(HUD_HAS_PROJECT_RESTORATION_ASSETS) && HUD_PROJECT_RESTORATION_TEST_LEVEL >= 2
    const u32 centerX = 370;
    const u32 centerY = 34;
    const u32 offset = 18;
    Draw_DrawSpriteTop(centerX - 10, centerY - offset - 10, PR_BUTTON_BLANK_WIDTH,
                       PR_BUTTON_BLANK_HEIGHT, PR_BUTTON_BLANK);
    Draw_DrawOverlaidCharacterTop(centerX - 2, centerY - offset - 5, COLOR_Y, 'X');
    Draw_DrawSpriteTop(centerX + offset - 10, centerY - 10, PR_BUTTON_A_WIDTH,
                       PR_BUTTON_A_HEIGHT, PR_BUTTON_A);
    Draw_DrawSpriteTop(centerX - 10, centerY + offset - 10, PR_BUTTON_B_WIDTH,
                       PR_BUTTON_B_HEIGHT, PR_BUTTON_B);
    Draw_DrawOverlaidCharacterTop(centerX - 2, centerY + offset - 5, COLOR_Y, 'B');
    Draw_DrawSpriteTop(centerX - offset - 10, centerY - 10, PR_BUTTON_BLANK_WIDTH,
                       PR_BUTTON_BLANK_HEIGHT, PR_BUTTON_BLANK);
    Draw_DrawOverlaidCharacterTop(centerX - offset - 2, centerY - 5, COLOR_Y, 'Y');
#else
    // Nintendo face-button orientation: X top, A right, B bottom, Y left.
    const u32 centerX = 360;
    const u32 centerY = 36;
    const u32 offset = 22;
    Hud_DrawDiamondButton(centerX, centerY - offset, COLOR_X, 'X');
    Hud_DrawDiamondButton(centerX + offset, centerY, COLOR_A, 'A');
    Hud_DrawDiamondButton(centerX, centerY + offset, COLOR_B, 'B');
    Hud_DrawDiamondButton(centerX - offset, centerY, COLOR_Y, 'Y');
#endif
}

static void __attribute__((unused)) Hud_DrawDPadCell(u32 x, u32 y, HudIcon icon, char slotLabel) {
    const u32 color = icon == HUD_ICON_NONE ? COLOR_MUTED : COLOR_DPAD;
    Draw_DrawRectOutlineTop(x, y, 18, 18, color);
    Hud_DrawIcon(x + 5, y + 5, color, icon);
    if (slotLabel) {
        Draw_DrawOverlaidCharacterTop(x + 1, y + 1, color, slotLabel);
    }
}

static void Hud_DrawDPad(void) {
#if defined(HUD_HAS_PROJECT_RESTORATION_ASSETS) && HUD_PROJECT_RESTORATION_TEST_LEVEL >= 3
    // Project Restoration's native D-pad cross anchors the four OoT3D
    // actions. I and II remain driven by the live equipped item IDs.
    Draw_DrawSpriteTop(299, 27, PR_DPAD_WIDTH, PR_DPAD_HEIGHT, PR_DPAD);
    Draw_DrawSpriteTop(303, 10, PR_NAVI_WIDTH, PR_NAVI_HEIGHT, PR_NAVI);
    Hud_DrawEquippedItem(279, 32, gSaveContext.equips.buttonItems[3]);
#if defined(HUD_HAS_OOT_ITEM_ASSETS)
    Hud_DrawEquippedItem(329, 32, ITEM_OCARINA_TIME);
#else
    Hud_DrawIcon(332, 35, COLOR_DPAD, HUD_ICON_OCARINA);
#endif
    Hud_DrawEquippedItem(303, 57, gSaveContext.equips.buttonItems[4]);
    Draw_DrawSpriteTop(272, 33, PR_LABEL_I_WIDTH, PR_LABEL_I_HEIGHT, PR_LABEL_I);
    Draw_DrawSpriteTop(294, 57, PR_LABEL_II_WIDTH, PR_LABEL_II_HEIGHT, PR_LABEL_II);
#else
    const u32 x = 284;
    const u32 y = 18;
    Hud_DrawDPadCell(x + 18, y, HUD_ICON_MAGIC, 0); // Up: Navi
    Hud_DrawDPadCell(x, y + 18, Hud_ItemIcon(gSaveContext.equips.buttonItems[3]), '1');
    Hud_DrawDPadCell(x + 36, y + 18, HUD_ICON_OCARINA, 0);
    Hud_DrawDPadCell(x + 18, y + 36, Hud_ItemIcon(gSaveContext.equips.buttonItems[4]), '2');
#endif
}

static void __attribute__((unused)) Hud_DrawHeart(u32 x, u32 y, u32 color) {
    Draw_DrawRectTop(x + 1, y, 3, 2, color);
    Draw_DrawRectTop(x + 5, y, 3, 2, color);
    Draw_DrawRectTop(x, y + 2, 9, 3, color);
    Draw_DrawRectTop(x + 1, y + 5, 7, 1, color);
    Draw_DrawRectTop(x + 2, y + 6, 5, 1, color);
    Draw_DrawRectTop(x + 3, y + 7, 3, 1, color);
    Draw_DrawPixelTop(x + 4, y + 8, color);
}

static void Hud_DrawVitals(void) {
    // Hearts are native GPU quads in native_hud.c. Drawing them here would
    // reintroduce the low-resolution ARGB2222 copy over the HD texture.
    // Magic is also a native GPU meter; do not overlay the old stretched
    // framebuffer strip or its gray tail will protrude beyond the fill.

#if defined(HUD_HAS_PROJECT_RESTORATION_ASSETS) && HUD_PROJECT_RESTORATION_TEST_LEVEL >= 4
    Draw_DrawSpriteTop(345, 215, PR_RUPEE_GREEN_WIDTH,
                       PR_RUPEE_GREEN_HEIGHT, PR_RUPEE_GREEN);
    u32 rupees = gSaveContext.rupees < 0 ? 0 : (u32)gSaveContext.rupees;
    if (rupees > 999) rupees = 999;
    u32 x = 359;
    if (rupees >= 100) {
        Draw_DrawOverlaidCharacterTop(x, 216, COLOR_WHITE, '0' + rupees / 100);
        x += 6;
    }
    if (rupees >= 10) {
        Draw_DrawOverlaidCharacterTop(x, 216, COLOR_WHITE, '0' + (rupees / 10) % 10);
        x += 6;
    }
    Draw_DrawOverlaidCharacterTop(x, 216, COLOR_WHITE, '0' + rupees % 10);
#else
    u32 maxHearts = gSaveContext.healthCapacity / 16;
    Hud_DrawIcon(11, maxHearts > 10 ? 46 : 35, COLOR_RUPEE, HUD_ICON_TRADE);
    Draw_DrawFormattedStringTop(23, maxHearts > 10 ? 45 : 34, COLOR_WHITE, "%d", gSaveContext.rupees);
#endif
}

void Hud_Draw(GlobalContext* globalCtx) {
    if (!globalCtx || !IsInGameOrBossChallenge()) {
        return;
    }

    Hud_DrawVitals();
    // ABXY and the D-pad cross are rendered by OoT3D's native GPU board.
    // Their old framebuffer copies must remain disabled or they produce a
    // soft doubled outline over the full-resolution custom texture.
}
