#include "native_mods_menu.h"

#include "z3D/z3D.h"

#if CONTROLLER_OPTIONS_ENABLED && defined(Version_USA)

#define SAVE_MENU_MANAGER_ADDR 0x0050A508
#define SAVE_MENU_ROOT_STATE_OFFSET 0x24
#define SAVE_MENU_MODE_OFFSET 0x28
#define SAVE_MENU_BOTTOM_ROW_OFFSET 0x38

#define SAVE_MENU_MODE_ROOT 2
#define SAVE_MENU_ROOT_IDLE 1

#define MODS_BUTTON_X_SHIFT 108.0f
#define MODS_ATLAS_U_SHIFT (224.0f / 512.0f)
#define SAVE_MENU_QUAD_COUNT 43

#define OPTIONS_BUTTON_WIDTH 96.0f
#define OPTIONS_BUTTON_HEIGHT 32.0f
#define POSITION_TOLERANCE 3.0f
#define MODS_BUTTON_QUAD 40

typedef float* (*NativeBoardBufferFn)(void* board, u32 quad);
#define NATIVE_BOARD_POSITIONS ((NativeBoardBufferFn)0x002FC3FC)
#define NATIVE_BOARD_UVS ((NativeBoardBufferFn)0x002FC3F0)
#define NATIVE_BOARD_COLORS ((NativeBoardBufferFn)0x002FC3E4)

static bool sSelected;
static s32 sOptionsQuad = -1;
static s32 sModsQuad = -1;

static volatile u8* NativeModsMenu_Manager(void) {
    return (volatile u8*)SAVE_MENU_MANAGER_ADDR;
}

void NativeModsMenu_Reset(void) {
    sSelected = false;
    sOptionsQuad = -1;
    sModsQuad = -1;
}

void NativeModsMenu_SetSelected(bool selected) {
    sSelected = selected;
}

bool NativeModsMenu_IsSelected(void) {
    return sSelected;
}

bool NativeModsMenu_IsVisible(void) {
    volatile u8* manager = NativeModsMenu_Manager();
    const u32 mode = *(volatile u32*)(manager + SAVE_MENU_MODE_OFFSET);
    const u32 rootState = *(volatile u32*)(manager + SAVE_MENU_ROOT_STATE_OFFSET);
    return mode == SAVE_MENU_MODE_ROOT && rootState == SAVE_MENU_ROOT_IDLE;
}

static void NativeModsMenu_CopyWords(float* destination, const float* source,
                                     u32 count) {
    if (destination == 0 || source == 0) {
        return;
    }
    for (u32 i = 0; i < count; ++i) {
        destination[i] = source[i];
    }
}

static float NativeModsMenu_Abs(float value) {
    return value < 0.0f ? -value : value;
}

static void NativeModsMenu_Bounds(const float* positions, float* minX,
                                  float* minY, float* maxX, float* maxY) {
    *minX = *maxX = positions[0];
    *minY = *maxY = positions[1];
    for (u32 vertex = 1; vertex < 4; ++vertex) {
        const float x = positions[vertex * 3];
        const float y = positions[vertex * 3 + 1];
        if (x < *minX) *minX = x;
        if (x > *maxX) *maxX = x;
        if (y < *minY) *minY = y;
        if (y > *maxY) *maxY = y;
    }
}

static bool NativeModsMenu_Close(float left, float right) {
    return NativeModsMenu_Abs(left - right) <= POSITION_TOLERANCE;
}

static void NativeModsMenu_FindQuads(void* board) {
    if (sOptionsQuad >= 0 && sModsQuad >= 0) {
        return;
    }

    for (s32 quad = 0; quad < SAVE_MENU_QUAD_COUNT; ++quad) {
        float* positions = NATIVE_BOARD_POSITIONS(board, (u32)quad);
        if (positions == 0) {
            continue;
        }

        float minX, minY, maxX, maxY;
        NativeModsMenu_Bounds(positions, &minX, &minY, &maxX, &maxY);
        const float width = maxX - minX;
        const float height = maxY - minY;

        if (NativeModsMenu_Close(width, OPTIONS_BUTTON_WIDTH) &&
            NativeModsMenu_Close(height, OPTIONS_BUTTON_HEIGHT)) {
            sOptionsQuad = quad;
            break;
        }
    }

    if (sOptionsQuad < 0) {
        return;
    }

    // Quad 40 is transition-only on the root Save prompt. The earlier test
    // copied an empty source into it without disturbing any visible element,
    // confirming it is safe to reuse here.
    sModsQuad = MODS_BUTTON_QUAD;
}

void NativeModsMenu_Update(void) {
    void* board = *(void**)(NativeModsMenu_Manager() + 0x08);
    if (board == 0) {
        return;
    }

    if (!NativeModsMenu_IsVisible()) {
        return;
    }

    NativeModsMenu_FindQuads(board);
    if (sOptionsQuad < 0 || sModsQuad < 0) {
        return;
    }

    float* sourcePositions = NATIVE_BOARD_POSITIONS(board, (u32)sOptionsQuad);
    float* destinationPositions = NATIVE_BOARD_POSITIONS(board, (u32)sModsQuad);
    float* sourceUvs = NATIVE_BOARD_UVS(board, (u32)sOptionsQuad);
    float* destinationUvs = NATIVE_BOARD_UVS(board, (u32)sModsQuad);
    float* sourceColors = NATIVE_BOARD_COLORS(board, (u32)sOptionsQuad);
    float* destinationColors = NATIVE_BOARD_COLORS(board, (u32)sModsQuad);

    NativeModsMenu_CopyWords(destinationPositions, sourcePositions, 12);
    NativeModsMenu_CopyWords(destinationUvs, sourceUvs, 8);
    NativeModsMenu_CopyWords(destinationColors, sourceColors, 16);

    if (destinationPositions != 0) {
        for (u32 vertex = 0; vertex < 4; ++vertex) {
            destinationPositions[vertex * 3] += MODS_BUTTON_X_SHIFT;
        }
    }
    if (destinationUvs != 0) {
        for (u32 vertex = 0; vertex < 4; ++vertex) {
            destinationUvs[vertex * 2] += MODS_ATLAS_U_SHIFT;
        }
    }

    const bool bottomRow = *(volatile u32*)(NativeModsMenu_Manager() +
                                            SAVE_MENU_BOTTOM_ROW_OFFSET) != 0;
    if (destinationColors != 0 && bottomRow && sSelected) {
        for (u32 vertex = 0; vertex < 4; ++vertex) {
            destinationColors[vertex * 4] = 1.0f;
            destinationColors[vertex * 4 + 1] = 0.82f;
            destinationColors[vertex * 4 + 2] = 0.35f;
        }
    }
}

#else

void NativeModsMenu_Reset(void) {}
void NativeModsMenu_SetSelected(bool selected) { (void)selected; }
bool NativeModsMenu_IsSelected(void) { return false; }
bool NativeModsMenu_IsVisible(void) { return false; }
void NativeModsMenu_Update(void) {}

#endif
