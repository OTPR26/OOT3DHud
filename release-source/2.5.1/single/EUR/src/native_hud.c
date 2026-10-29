#include "manual_jump.h"
#include "manual_jump_hud.h"
#include "native_hud.h"

#include "common.h"
#include "draw.h"
#include "input_remap.h"
#include "z3D/z3Ditem.h"

// The native motion-control board is converted into a multi-quad triangle
// strip. These buffers stay resident for the lifetime of the game because the
// board's descriptor retains their addresses while uploading GPU data.
#define NATIVE_HUD_QUAD_COUNT 106
#define NATIVE_HUD_VERTEX_COUNT (NATIVE_HUD_QUAD_COUNT * 4)
#define NATIVE_HUD_INDEX_COUNT (4 + (NATIVE_HUD_QUAD_COUNT - 1) * 6)

#ifndef HUD_SCALE_PERCENT
#define HUD_SCALE_PERCENT 100
#endif

#if HUD_SCALE_PERCENT != 0 && HUD_SCALE_PERCENT != 75 && \
    HUD_SCALE_PERCENT != 100 && HUD_SCALE_PERCENT != 125
#error "HUD_SCALE_PERCENT must be 0, 75, 100, or 125"
#endif

// Vertex order per quad: lower-left, lower-right, upper-left, upper-right.
// The face-button size follows the visual scale of Project Restoration's
// widescreen HUD.
float gNativeHudPositions[NATIVE_HUD_VERTEX_COUNT * 3]
    __attribute__((aligned(16), used)) = {
    // Native bottom-right rupee. This old A-placeholder quad is drawn early,
    // but the rupee occupies an unrelated screen area and needs no overlap.
    345.0f, 227.0f, 0.0f, 355.0f, 227.0f, 0.0f,
    345.0f, 215.0f, 0.0f, 355.0f, 215.0f, 0.0f,
    // B base (bottom)
    357.0f, 62.0f, 0.0f, 373.0f, 62.0f, 0.0f,
    357.0f, 46.0f, 0.0f, 373.0f, 46.0f, 0.0f,
    // X base (top)
    357.0f, 28.0f, 0.0f, 373.0f, 28.0f, 0.0f,
    357.0f, 12.0f, 0.0f, 373.0f, 12.0f, 0.0f,
    // Y base (left)
    340.0f, 45.0f, 0.0f, 356.0f, 45.0f, 0.0f,
    340.0f, 29.0f, 0.0f, 356.0f, 29.0f, 0.0f,
    // D-pad cross (upper-left, beneath the vitals)
    30.0f, 89.0f, 0.0f, 50.0f, 89.0f, 0.0f,
    30.0f, 68.0f, 0.0f, 50.0f, 68.0f, 0.0f,
    // Live B item, centered inside the bottom face-button base.
    359.0f, 60.0f, 0.0f, 371.0f, 60.0f, 0.0f,
    359.0f, 48.0f, 0.0f, 371.0f, 48.0f, 0.0f,
    // Live X item, centered inside the top face-button base.
    359.0f, 26.0f, 0.0f, 371.0f, 26.0f, 0.0f,
    359.0f, 14.0f, 0.0f, 371.0f, 14.0f, 0.0f,
    // Live Y item, centered inside the left face-button base.
    342.0f, 43.0f, 0.0f, 354.0f, 43.0f, 0.0f,
    342.0f, 31.0f, 0.0f, 354.0f, 31.0f, 0.0f,
    // Ocarina (D-pad right), moved onto the obsolete static A-label quad.
    50.0f, 85.0f, 0.0f, 64.0f, 85.0f, 0.0f,
    50.0f, 71.0f, 0.0f, 64.0f, 71.0f, 0.0f,
    // Navi marker (D-pad up)
    34.0f, 64.0f, 0.0f, 46.0f, 64.0f, 0.0f,
    34.0f, 52.0f, 0.0f, 46.0f, 52.0f, 0.0f,
    // I-slot equipped item (D-pad left)
    16.0f, 85.0f, 0.0f, 30.0f, 85.0f, 0.0f,
    16.0f, 71.0f, 0.0f, 30.0f, 71.0f, 0.0f,
    // II-slot equipped item (D-pad down)
    33.0f, 105.0f, 0.0f, 47.0f, 105.0f, 0.0f,
    33.0f, 91.0f, 0.0f, 47.0f, 91.0f, 0.0f,
    // I ammo count. Quad 12 is drawn after both D-pad item quads, so the
    // digits remain visibly layered over the equipped item artwork.
    23.0f, 85.0f, 0.0f, 30.0f, 85.0f, 0.0f,
    23.0f, 80.0f, 0.0f, 30.0f, 80.0f, 0.0f,
    // Ten native pair cells cover all twenty hearts while staying below the
    // board's proven 33-quad upload boundary. Runtime UV updates select one
    // of fifteen lossless two-heart states from the HD atlas.
#define NATIVE_HEART_PAIR_POS(x, y) \
    (x), (y) + 11.0f, 0.0f, (x) + 22.0f, (y) + 11.0f, 0.0f, \
    (x), (y), 0.0f, (x) + 22.0f, (y), 0.0f
    NATIVE_HEART_PAIR_POS(8.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(30.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(52.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(74.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(96.0f, 8.0f),
    NATIVE_HEART_PAIR_POS(8.0f, 19.0f),
    NATIVE_HEART_PAIR_POS(30.0f, 19.0f),
    NATIVE_HEART_PAIR_POS(52.0f, 19.0f),
    NATIVE_HEART_PAIR_POS(74.0f, 19.0f),
    NATIVE_HEART_PAIR_POS(96.0f, 19.0f),
    // II ammo count plus three live PR rupee-number glyphs. Quad 23 is
    // safely inside the proven upload range and renders after the item quad.
    40.0f, 105.0f, 0.0f, 47.0f, 105.0f, 0.0f,
    40.0f, 100.0f, 0.0f, 47.0f, 100.0f, 0.0f,
    359.0f, 226.0f, 0.0f, 365.0f, 226.0f, 0.0f,
    359.0f, 216.0f, 0.0f, 365.0f, 216.0f, 0.0f,
    366.0f, 226.0f, 0.0f, 372.0f, 226.0f, 0.0f,
    366.0f, 216.0f, 0.0f, 372.0f, 216.0f, 0.0f,
    373.0f, 226.0f, 0.0f, 379.0f, 226.0f, 0.0f,
    373.0f, 216.0f, 0.0f, 379.0f, 216.0f, 0.0f,
    // The board only uploads its first 33 quads reliably across supported
    // emulator renderers.
    // Magic needs two cells (frame and fill); the other four reliable cells
    // carry live ammo counts for X, Y, I, and II.
    8.0f, 42.0f, 0.0f, 118.0f, 42.0f, 0.0f,
    8.0f, 33.0f, 0.0f, 118.0f, 33.0f, 0.0f,
    // X ammo count
    366.0f, 27.0f, 0.0f, 373.0f, 27.0f, 0.0f,
    366.0f, 22.0f, 0.0f, 373.0f, 22.0f, 0.0f,
    // Y ammo count
    349.0f, 44.0f, 0.0f, 356.0f, 44.0f, 0.0f,
    349.0f, 39.0f, 0.0f, 356.0f, 39.0f, 0.0f,
    10.0f, 40.0f, 0.0f, 116.0f, 40.0f, 0.0f,
    10.0f, 35.0f, 0.0f, 116.0f, 35.0f, 0.0f,
    // Quads 31 and 32 cannot safely carry visible geometry on every Azahar
    // renderer. Keep them degenerate; I/II use safe quads 12 and 23 instead.
    -100.0f, -100.0f, 0.0f, -100.0f, -100.0f, 0.0f,
    -100.0f, -100.0f, 0.0f, -100.0f, -100.0f, 0.0f,
    -100.0f, -100.0f, 0.0f, -100.0f, -100.0f, 0.0f,
    -100.0f, -100.0f, 0.0f, -100.0f, -100.0f, 0.0f,
#undef NATIVE_HEART_PAIR_POS
    // Quads 33-105 are compatibility padding. Their implicit zero
    // initialization keeps every triangle degenerate while preserving the
    // descriptor layout used by the supported regional builds.
};

// UVs address Project Restoration's logical 512x256 atlas. V is flipped
// because the native board uses a bottom-left texture origin.
float gNativeHudUVs[NATIVE_HUD_VERTEX_COUNT * 2]
    __attribute__((aligned(16), used)) = {
    // Rupee starts transparent until the first live state upload.
    0.99609375f, 0.0078125f, 0.99609375f, 0.0078125f,
    0.99609375f, 0.0078125f, 0.99609375f, 0.0078125f,
    // Blank circular base: (48,0)-(85,38), used by B/X/Y.
    0.09375f, 0.8515625f, 0.166015625f, 0.8515625f,
    0.09375f, 1.0f,       0.166015625f, 1.0f,
    0.09375f, 0.8515625f, 0.166015625f, 0.8515625f,
    0.09375f, 1.0f,       0.166015625f, 1.0f,
    0.09375f, 0.8515625f, 0.166015625f, 0.8515625f,
    0.09375f, 1.0f,       0.166015625f, 1.0f,
    // D-pad: (40,39)-(67,68)
    0.078125f, 0.734375f, 0.130859375f, 0.734375f,
    0.078125f, 0.84765625f, 0.130859375f, 0.84765625f,
    // Current-save B/Master Sword, X/Bow, and Y/Bottle defaults. Runtime
    // rebinding below keeps these synchronized with the second screen.
    0.2475585938f, 0.25f,         0.2683919271f, 0.25f,
    0.2475585938f, 0.2916666667f, 0.2683919271f, 0.2916666667f,
    0.3100585938f, 0.4583333333f, 0.3308919271f, 0.4583333333f,
    0.3100585938f, 0.5f,          0.3308919271f, 0.5f,
    0.4142252604f, 0.4166666667f, 0.4350585938f, 0.4166666667f,
    0.4142252604f, 0.4583333333f, 0.4350585938f, 0.4583333333f,
    // Ocarina of Time (item-atlas cell 8).
    0.4142252604f, 0.4583333333f, 0.4350585938f, 0.4583333333f,
    0.4142252604f, 0.5f,          0.4350585938f, 0.5f,
    // Navi: (88,0)-(116,26)
    0.171875f, 0.8984375f, 0.2265625f, 0.8984375f,
    0.171875f, 1.0f,       0.2265625f, 1.0f,
    // Active-save constructor defaults from the OoT atlas's 12x12 grid:
    // I/Bombchu (cell 9), II/Longshot (cell 11).
    0.4350585938f, 0.4583333333f, 0.4558919271f, 0.4583333333f,
    0.4350585938f, 0.5f,          0.4558919271f, 0.5f,
    0.4767252604f, 0.4583333333f, 0.4975585938f, 0.4583333333f,
    0.4767252604f, 0.5f,          0.4975585938f, 0.5f,
    // I count starts transparent until the first live state upload.
    0.99609375f, 0.0078125f, 0.99609375f, 0.0078125f,
    0.99609375f, 0.0078125f, 0.99609375f, 0.0078125f,
    // Default to atlas tile 14: two full hearts. Runtime replaces all ten
    // pair cells from live capacity and health before the board is displayed.
#define NATIVE_FULL_HEART_PAIR_UV \
    0.75f, 0.3125f, 0.8125f, 0.3125f, \
    0.75f, 0.375f,  0.8125f, 0.375f
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
    NATIVE_FULL_HEART_PAIR_UV, NATIVE_FULL_HEART_PAIR_UV,
#undef NATIVE_FULL_HEART_PAIR_UV
    // Quads 23-32 are overwritten with live rupee/magic UVs. Initialize them
    // transparently so no placeholder art can flash during startup.
#define NATIVE_TRANSPARENT_UV \
    0.802734375f, 0.953125f, 0.802734375f, 0.953125f, \
    0.802734375f, 0.953125f, 0.802734375f, 0.953125f
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
    NATIVE_TRANSPARENT_UV, NATIVE_TRANSPARENT_UV,
#undef NATIVE_TRANSPARENT_UV
    // Compatibility padding is implicitly zeroed to match its degenerate
    // position data above.
};

// Join independent four-vertex strips with repeated degenerate vertices.
#define NATIVE_HUD_JOIN(n) \
    (4 * (n) - 1), (4 * (n)), (4 * (n)), (4 * (n) + 1), \
    (4 * (n) + 2), (4 * (n) + 3)
u16 gNativeHudIndices[NATIVE_HUD_INDEX_COUNT]
    __attribute__((aligned(16), used)) = {
    0, 1, 2, 3, 3, 4, 4, 5, 6, 7, 7, 8, 8, 9, 10, 11,
    11, 12, 12, 13, 14, 15, 15, 16, 16, 17, 18, 19, 19, 132, 132, 133,
    134, 135, 135, 136, 136, 137, 138, 139, 139, 140, 140, 141, 142, 143, 143, 144,
    144, 145, 146, 147, 147, 148, 148, 149, 150, 151, 151, 152, 152, 153, 154, 155,
    155, 156, 156, 157, 158, 159, 159, 160, 160, 161, 162, 163, 163, 164, 164, 165,
    166, 167, 167, 168, 168, 169, 170, 171, 171, 172, 172, 173, 174, 175, 175, 176,
    176, 177, 178, 179, 179, 180, 180, 181, 182, 183, 183, 184, 184, 185, 186, 187,
    187, 188, 188, 189, 190, 191, 191, 192, 192, 193, 194, 195, 195, 196, 196, 197,
    198, 199, 199, 200, 200, 201, 202, 203, 203, 204, 204, 205, 206, 207, 207, 208,
    208, 209, 210, 211, 211, 212, 212, 213, 214, 215, 215, 216, 216, 217, 218, 219,
    219, 220, 220, 221, 222, 223, 223, 224, 224, 225, 226, 227, 227, 228, 228, 229,
    230, 231, 231, 232, 232, 233, 234, 235, 235, 236, 236, 237, 238, 239, 239, 240,
    240, 241, 242, 243, 243, 244, 244, 245, 246, 247, 247, 248, 248, 249, 250, 251,
    251, 252, 252, 253, 254, 255, 255, 256, 256, 257, 258, 259, 259, 260, 260, 261,
    262, 263, 263, 264, 264, 265, 266, 267, 267, 268, 268, 269, 270, 271, 271, 272,
    272, 273, 274, 275, 275, 276, 276, 277, 278, 279, 279, 280, 280, 281, 282, 283,
    283, 284, 284, 285, 286, 287, 287, 288, 288, 289, 290, 291, 291, 292, 292, 293,
    294, 295, 295, 296, 296, 297, 298, 299, 299, 300, 300, 301, 302, 303, 303, 304,
    304, 305, 306, 307, 307, 308, 308, 309, 310, 311, 311, 312, 312, 313, 314, 315,
    315, 316, 316, 317, 318, 319, 319, 320, 320, 321, 322, 323, 323, 324, 324, 325,
    326, 327, 327, 328, 328, 329, 330, 331, 331, 332, 332, 333, 334, 335, 335, 336,
    336, 337, 338, 339, 339, 340, 340, 341, 342, 343, 343, 344, 344, 345, 346, 347,
    347, 348, 348, 349, 350, 351, 351, 352, 352, 353, 354, 355, 355, 356, 356, 357,
    358, 359, 359, 360, 360, 361, 362, 363, 363, 364, 364, 365, 366, 367, 367, 368,
    368, 369, 370, 371, 371, 372, 372, 373, 374, 375, 375, 376, 376, 377, 378, 379,
    379, 380, 380, 381, 382, 383, 383, 384, 384, 385, 386, 387, 387, 20, 20, 21,
    22, 23, 23, 24, 24, 25, 26, 27, 27, 28, 28, 29, 30, 31, 31, 32,
    32, 33, 34, 35, 35, 36, 36, 37, 38, 39, 39, 40, 40, 41, 42, 43,
    43, 44, 44, 45, 46, 47, 47, 48, 48, 49, 50, 51, 51, 52, 52, 53,
    54, 55, 55, 56, 56, 57, 58, 59, 59, 60, 60, 61, 62, 63, 63, 64,
    64, 65, 66, 67, 67, 68, 68, 69, 70, 71, 71, 72, 72, 73, 74, 75,
    75, 76, 76, 77, 78, 79, 79, 80, 80, 81, 82, 83, 83, 84, 84, 85,
    86, 87, 87, 88, 88, 89, 90, 91, 91, 92, 92, 93, 94, 95, 95, 96,
    96, 97, 98, 99, 99, 100, 100, 101, 102, 103, 103, 104, 104, 105, 106, 107,
    107, 108, 108, 109, 110, 111, 111, 112, 112, 113, 114, 115, 115, 116, 116, 117,
    118, 119, 119, 120, 120, 121, 122, 123, 123, 124, 124, 125, 126, 127, 127, 128,
    128, 129, 130, 131, 131, 388, 388, 389, 390, 391, 391, 392, 392, 393, 394, 395,
    395, 396, 396, 397, 398, 399, 399, 400, 400, 401, 402, 403, 403, 404, 404, 405,
    406, 407, 407, 408, 408, 409, 410, 411, 411, 412, 412, 413, 414, 415, 415, 416,
    416, 417, 418, 419, 419, 420, 420, 421, 422, 423,
};
#undef NATIVE_HUD_JOIN

// OoT3D's motion-control HUD flag. The standalone free camera does not use
// the icon, so the HUD repurposes its existing stereoscopic quad.
#if defined(Version_TWN)
    #define NATIVE_HUD_BASE 0x00508A30
    #define NATIVE_HUD_SOURCE_POSITIONS_ADDR 0x0031B0A4
    #define NATIVE_HUD_UPLOAD_UVS_ADDR 0x0031701C
    #define NATIVE_HUD_UPLOAD_POSITIONS_ADDR 0x003170C8
#elif defined(Version_KOR)
    #define NATIVE_HUD_BASE 0x00508A30
    #define NATIVE_HUD_SOURCE_POSITIONS_ADDR 0x0031AFA4
    #define NATIVE_HUD_UPLOAD_UVS_ADDR 0x00316F1C
    #define NATIVE_HUD_UPLOAD_POSITIONS_ADDR 0x00316FC8
#else
    #define NATIVE_HUD_BASE ADDR(0x004FC648)
    #define NATIVE_HUD_SOURCE_POSITIONS_ADDR ADDR(0x002FC3FC)
    #define NATIVE_HUD_UPLOAD_UVS_ADDR ADDR(0x00317D1C)
    #define NATIVE_HUD_UPLOAD_POSITIONS_ADDR ADDR(0x0036759C)
#endif

#define NATIVE_HUD_VISIBLE (*(volatile u8*)NATIVE_HUD_BASE)
#define NATIVE_HUD_BOARD (*(void**)(NATIVE_HUD_BASE + 0x74))

typedef float* (*NativeHudMappedBufferFn)(void* board, u32 layer);
#define NATIVE_HUD_SOURCE_POSITIONS \
    ((NativeHudMappedBufferFn)NATIVE_HUD_SOURCE_POSITIONS_ADDR)

typedef void (*NativeHudUploadBufferFn)(void* board, u32 byteCount, const void* source);
#define NATIVE_HUD_UPLOAD_UVS ((NativeHudUploadBufferFn)NATIVE_HUD_UPLOAD_UVS_ADDR)
#define NATIVE_HUD_UPLOAD_POSITIONS \
    ((NativeHudUploadBufferFn)NATIVE_HUD_UPLOAD_POSITIONS_ADDR)

#define NATIVE_ACTION_VERTEX_COUNT 8
static float sNativeActionOriginal[NATIVE_ACTION_VERTEX_COUNT * 3];
static float* sNativeActionSource;
static u32 sNativeHudUploadedHash;
static u8 sNativeHudHasUploaded;
static void* sNativeHudUploadedBoard;
static float sNativeHudBasePositions[NATIVE_HUD_VERTEX_COUNT * 3];
static void* sNativeHudPositionBoard;
static u8 sNativeHudScalePercent = HUD_SCALE_PERCENT;
static u8 sNativeHudScaleInitialized;
static u8 sNativeHudPositionsDirty;
// 0 = gameplay HUD, 1 = Items controller diagram, 2 = Gear/Map D-pad only.
static u8 sNativeHudPauseLayout;
static u8 sNativeHudOcarinaLayout;

static float NativeHud_GetScale(void) {
    return (float)sNativeHudScalePercent / 100.0f;
}

static void NativeHud_ScaleQuad(u32 quad, float anchorX, float anchorY,
                                float scale) {
    float* positions = gNativeHudPositions + quad * 4 * 3;
    for (u32 vertex = 0; vertex < 4; vertex++) {
        float* position = positions + vertex * 3;
        position[0] = anchorX + (position[0] - anchorX) * scale;
        position[1] = anchorY + (position[1] - anchorY) * scale;
    }
}

static void NativeHud_ScaleQuadRange(u32 first, u32 last,
                                     float anchorX, float anchorY,
                                     float scale) {
    for (u32 quad = first; quad <= last; quad++) {
        NativeHud_ScaleQuad(quad, anchorX, anchorY, scale);
    }
}

static void NativeHud_CollapseQuad(u32 quad) {
    float* positions = gNativeHudPositions + quad * 4 * 3;
    for (u32 vertex = 0; vertex < 4; vertex++) {
        positions[vertex * 3] = -100.0f;
        positions[vertex * 3 + 1] = -100.0f;
    }
}

static void NativeHud_TranslateQuad(u32 quad, float x, float y) {
    float* positions = gNativeHudPositions + quad * 4 * 3;
    for (u32 vertex = 0; vertex < 4; vertex++) {
        positions[vertex * 3] += x;
        positions[vertex * 3 + 1] += y;
    }
}

static void NativeHud_SetQuadRect(u32 quad, float left, float bottom,
                                  float right, float top) {
    float* positions = gNativeHudPositions + quad * 4 * 3;
    positions[0] = left;  positions[1] = bottom;
    positions[3] = right; positions[4] = bottom;
    positions[6] = left;  positions[7] = top;
    positions[9] = right; positions[10] = top;
}

#if UNIFIED_MENU_PROTOTYPE && (defined(Version_USA) || defined(Version_EUR))
static void NativeHud_PositionPauseDpad(void) {
    /* Balance the controller clusters around the preview lane: D-pad center
     * 252, diamond center 348 (midpoint 300). Preserve height and scale.
     * Translate the cross and all four assignments as one unit. */
    const u32 quads[] = {4, 8, 9, 10, 11};
    for (u32 i = 0; i < sizeof(quads) / sizeof(quads[0]); ++i)
        NativeHud_TranslateQuad(quads[i], -48.0f, 0.0f);
    /* Include the blank East base and all live face-button assignments. */
    const u32 diamondQuads[] = {0, 1, 2, 3, 5, 6, 7};
    for (u32 i = 0; i < sizeof(diamondQuads) / sizeof(diamondQuads[0]); ++i)
        NativeHud_TranslateQuad(diamondQuads[i], -16.0f, 0.0f);
}
#endif

static u8 sManualJumpBumper;
static void NativeHud_PositionJumpBumper(void) {
    if(!sManualJumpBumper)return;
    NativeHud_CollapseQuad(2);
    /* Keep the remaining Y/A/B triangle together, above and to the left.
     * The remapped item occupies the compact shoulder beside its top row. */
    NativeHud_TranslateQuad(1,-30.0f,-17.0f);
    NativeHud_TranslateQuad(3,-30.0f,-17.0f);
    NativeHud_TranslateQuad(5,-30.0f,-17.0f);
    NativeHud_TranslateQuad(7,-30.0f,-17.0f);
    NativeHud_TranslateQuad(29,-30.0f,-17.0f);
    NativeHud_SetQuadRect(6,377.0f,24.0f,385.0f,16.0f);
    NativeHud_SetQuadRect(28,385.0f,25.0f,392.0f,21.0f);
    for(u32 i=0;i<64;++i) {
        const float* p=ManualJump_BumperPatches[i];
        float* out=gNativeHudPositions+(33+i)*12;
        for(u32 vertex=0;vertex<4;++vertex) {
            out[vertex*3]=p[vertex*2];out[vertex*3+1]=p[vertex*2+1];out[vertex*3+2]=0;
        }
    }
}
static void NativeHud_RebuildPositions(void) {
    for (u32 i = 0; i < NATIVE_HUD_VERTEX_COUNT * 3; i++) {
        gNativeHudPositions[i] = sNativeHudBasePositions[i];
    }

    if (sNativeHudPauseLayout == 1) {
        // The menu diagram uses only four face-button bases, three face-button
        // items, and the D-pad with its four live assignments. Everything
        // else stays hidden so health, magic, ammo, and rupees cannot cover
        // the native item-description presentation below.
        for (u32 quad = 0; quad <= 30; quad++) {
            const u8 keep = quad <= 11 && quad != 0 ? 1 : 0;
            if (!keep) {
                NativeHud_CollapseQuad(quad);
            }
        }

        // Quad 0 normally carries the rupee icon. In the Items menu it becomes
        // the otherwise-missing blank East face-button base.
        NativeHud_SetQuadRect(0, 374.0f, 45.0f, 390.0f, 29.0f);

        // Move the complete D-pad cluster beside the face-button diamond in
        // the upper-right column. Face-button geometry already occupies the
        // desired top-right positions.
        NativeHud_TranslateQuad(4, 260.0f, -35.0f);
        NativeHud_TranslateQuad(8, 260.0f, -35.0f);
        NativeHud_TranslateQuad(9, 260.0f, -35.0f);
        NativeHud_TranslateQuad(10, 260.0f, -35.0f);
        NativeHud_TranslateQuad(11, 260.0f, -35.0f);
#if UNIFIED_MENU_PROTOTYPE && (defined(Version_USA) || defined(Version_EUR))
        // Leave space above the assignment display for the future tab band.
        // Rebuilt from immutable base geometry; never accumulate per-frame.
        for (u32 quad = 0; quad <= 11; ++quad)
            NativeHud_TranslateQuad(quad, 0.0f, 32.0f);
        NativeHud_PositionPauseDpad();
#endif
        sNativeHudPositionsDirty = 1;
        return;
    }

    if (sNativeHudPauseLayout == 2) {
#if UNIFIED_MENU_PROTOTYPE && (defined(Version_USA) || defined(Version_EUR))
        /* Gear/Map are informational pages, not assignment screens. */
        for (u32 quad = 0; quad < NATIVE_HUD_VERTEX_COUNT / 4; ++quad)
            NativeHud_CollapseQuad(quad);
        sNativeHudPositionsDirty = 1;
        return;
#endif
        // Gear and Map retain the same universal controller presentation as
        // Items. Keep the full face-button diamond and align the live D-pad
        // immediately beside it while hiding gameplay-only HUD elements.
        for (u32 quad = 0; quad <= 30; quad++) {
            const u8 keep = quad <= 11 && quad != 0 ? 1 : 0;
            if (!keep) {
                NativeHud_CollapseQuad(quad);
            }
        }
        NativeHud_SetQuadRect(0, 374.0f, 45.0f, 390.0f, 29.0f);
        NativeHud_TranslateQuad(4, 260.0f, -35.0f);
        NativeHud_TranslateQuad(8, 260.0f, -35.0f);
        NativeHud_TranslateQuad(9, 260.0f, -35.0f);
        NativeHud_TranslateQuad(10, 260.0f, -35.0f);
        NativeHud_TranslateQuad(11, 260.0f, -35.0f);
#if UNIFIED_MENU_PROTOTYPE && (defined(Version_USA) || defined(Version_EUR))
        for (u32 quad = 0; quad <= 11; ++quad)
            NativeHud_TranslateQuad(quad, 0.0f, 32.0f);
        NativeHud_PositionPauseDpad();
#endif
        sNativeHudPositionsDirty = 1;
        return;
    }

    NativeHud_PositionJumpBumper();
    const float scale = NativeHud_GetScale();
    if (sNativeHudOcarinaLayout) {
        // Instrument mode hides the native action prompt. Borrow the I-slot
        // ammo cell (not actionable while playing) for its blank East base.
        // Do not use quads 31/32, which are unreliable on some renderers.
        NativeHud_SetQuadRect(12, 400.0f - 26.0f * scale, 45.0f * scale,
                              400.0f - 10.0f * scale, 29.0f * scale);
    }
    if (sNativeHudScalePercent == 100) {
        sNativeHudPositionsDirty = 1;
        return;
    }

    // Keep each HUD cluster attached to its natural screen corner. Scaling
    // positions and quad dimensions together preserves the source texture at
    // full resolution and avoids the filtering damage of resized assets.
    NativeHud_ScaleQuad(0, 400.0f, 240.0f, scale); // Rupee icon
    NativeHud_ScaleQuadRange(33, 96, 400.0f, 0.0f, scale);
    NativeHud_ScaleQuadRange(1, 3, 400.0f, 0.0f, scale); // B/X/Y bases
    NativeHud_ScaleQuad(4, 0.0f, 0.0f, scale); // D-pad cross
    NativeHud_ScaleQuadRange(5, 7, 400.0f, 0.0f, scale); // B/X/Y items
    NativeHud_ScaleQuadRange(8, 11, 0.0f, 0.0f, scale); // D-pad items
    if (!sNativeHudOcarinaLayout)
        NativeHud_ScaleQuad(12, 0.0f, 0.0f, scale); // I ammo
    NativeHud_ScaleQuadRange(13, 23, 0.0f, 0.0f, scale); // Hearts and II ammo
    NativeHud_ScaleQuadRange(24, 26, 400.0f, 240.0f, scale); // Rupee digits
    NativeHud_ScaleQuad(27, 0.0f, 0.0f, scale); // Magic frame
    NativeHud_ScaleQuadRange(28, 29, 400.0f, 0.0f, scale); // X/Y ammo
    NativeHud_ScaleQuad(30, 0.0f, 0.0f, scale); // Magic fill
    sNativeHudPositionsDirty = 1;
}

void NativeHud_InitializeScale(void) {
    if (!sNativeHudScaleInitialized) {
        for (u32 i = 0; i < NATIVE_HUD_VERTEX_COUNT * 3; i++) {
            sNativeHudBasePositions[i] = gNativeHudPositions[i];
        }
        sNativeHudScaleInitialized = 1;
    }
    NativeHud_RebuildPositions();
}

void NativeHud_CycleScale(void) {
    if (sNativeHudScalePercent == 75) {
        sNativeHudScalePercent = 100;
    } else if (sNativeHudScalePercent == 100) {
        sNativeHudScalePercent = 125;
    } else if (sNativeHudScalePercent == 125) {
        sNativeHudScalePercent = 0;
    } else {
        sNativeHudScalePercent = 75;
    }
    NativeHud_RebuildPositions();
}

u8 NativeHud_GetScalePercent(void) {
    return sNativeHudScalePercent;
}

void NativeHud_SetScalePercent(u8 percent) {
    if (percent != 0 && percent != 75 && percent != 100 && percent != 125) {
        return;
    }
    sNativeHudScalePercent = percent;
    NativeHud_RebuildPositions();
}

static u32 NativeHud_UvHash(void) {
    const u32* words = (const u32*)gNativeHudUVs;
    u32 hash = 2166136261u;
    for (u32 i = 0; i < sizeof(gNativeHudUVs) / sizeof(u32); i++) {
        hash ^= words[i];
        hash *= 16777619u;
    }
    return hash;
}

void NativeHud_PrepareActionPrompt(void) {
    // Quads 0 and 1 are alternate live A states. One may be collapsed while
    // the other is active, so transform them independently instead of using a
    // combined bound (which made the visible artwork tiny and malformed).
    void* actionSource = *(void**)(NATIVE_HUD_BASE + 0x18);
    float* source = actionSource != 0 ? NATIVE_HUD_SOURCE_POSITIONS(actionSource, 0) : 0;
    sNativeActionSource = 0;
    if (source == 0) {
        return;
    }
    for (u32 i = 0; i < NATIVE_ACTION_VERTEX_COUNT * 3; i++) {
        sNativeActionOriginal[i] = source[i];
    }
    for (u32 quad = 0; quad < 2; quad++) {
        float* first = source + quad * 4 * 3;
        float minX = first[0];
        float maxX = first[0];
        float minY = first[1];
        float maxY = first[1];
        for (u32 i = 1; i < 4; i++) {
            float x = first[i * 3];
            float y = first[i * 3 + 1];
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
        float width = maxX - minX;
        float height = maxY - minY;
        if (width < 1.0f || height < 1.0f) {
            continue;
        }
        float sourceCenterX = (minX + maxX) * 0.5f;
        float sourceCenterY = (minY + maxY) * 0.5f;
        // The source contains separate geometry for the nearly-square A
        // circle and the wide, changing action verb. They need independent
        // scale and placement: Project Restoration keeps the circle in the
        // diamond and puts the smaller verb immediately below it.
        bool isActionText = width > height * 1.35f;
        const float hudScale = NativeHud_GetScale();
        float scale = (isActionText ? 9.0f : 16.0f) * hudScale / height;
        if (isActionText && width * scale > 48.0f * hudScale) {
            scale = 48.0f * hudScale / width;
        }
        float scaledWidth = width * scale;
        float targetCenterX = isActionText ?
            400.0f - 8.0f * hudScale - scaledWidth * 0.5f :
            400.0f - 18.0f * hudScale;
        float targetCenterY = (isActionText ? 40.0f : 37.0f) * hudScale;
        if(sManualJumpBumper){targetCenterX-=30.0f*hudScale;targetCenterY-=17.0f*hudScale;}

        for (u32 i = 0; i < 4; i++) {
            float* vertex = first + i * 3;
            vertex[0] = targetCenterX + (vertex[0] - sourceCenterX) * scale;
            vertex[1] = targetCenterY + (vertex[1] - sourceCenterY) * scale;
        }
    }
    sNativeActionSource = source;
}

void NativeHud_RestoreActionPrompt(void) {
    float* source = sNativeActionSource;
    if (source == 0) {
        return;
    }
    for (u32 i = 0; i < NATIVE_ACTION_VERTEX_COUNT * 3; i++) {
        source[i] = sNativeActionOriginal[i];
    }
    sNativeActionSource = 0;
}

static void NativeHud_WriteItemUV(float* uv, u32 quad, u8 item) {
    u32 index = item <= ITEM_HEART_PIECE_2 ? item : 0xFF;
    // The 512x512 OoT item atlas is embedded unscaled at (512,512) in the
    // 2048x1024 Project Restoration replacement. The working HUD mapping
    // addresses it as a 12x12 grid and shifts the sampling window five source
    // pixels left to align the live item artwork.
    u32 column = index % 12;
    u32 row = index / 12;
    float u0 = 0.25f + (float)column / 48.0f - (5.0f / 2048.0f);
    float u1 = u0 + (1.0f / 48.0f);
    float v1 = 0.5f - (float)row / 24.0f;
    float v0 = v1 - (1.0f / 24.0f);
#if UNIFIED_MENU_PROTOTYPE
    /* Keep linear filtering inside the item cell, away from adjacent art. */
    if (quad == 8 || quad == 10 || quad == 11) {
        u0 += 0.5f / 2048.0f;
        u1 -= 0.5f / 2048.0f;
        v0 += 0.5f / 1024.0f;
        v1 -= 0.5f / 1024.0f;
    }
#endif
    uv += quad * 8;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1;
    uv[6] = u1; uv[7] = v1;
}

static u32 NativeHud_HeartProgress(s32 health) {
    if (health <= 0) return 0;
    if (health <= 4) return 1;
    if (health <= 8) return 2;
    if (health < 16) return 3;
    return 4;
}

static void NativeHud_WriteHeartPairUV(float* uv, u32 quad,
                                       u32 capacity, s32 health) {
    uv += quad * 8;
    if (capacity > 2) capacity = 2;
    s32 maxHealth = (s32)capacity * 16;
    if (health < 0) health = 0;
    if (health > maxHealth) health = maxHealth;

    u32 tile = 0;
    if (capacity == 1) {
        tile = 1 + NativeHud_HeartProgress(health);
    } else if (capacity == 2) {
        tile = health <= 16 ? 6 + NativeHud_HeartProgress(health) :
               10 + NativeHud_HeartProgress(health - 16);
    }

    const u32 column = tile % 5;
    const u32 row = tile / 5;
    // Half-pixel insets prevent bilinear filtering from sampling an adjacent
    // pair tile at the bank boundaries.
    float u0 = (1024.5f + (float)column * 128.0f) / 2048.0f;
    float u1 = u0 + 127.0f / 2048.0f;
    float v1 = 1.0f - (514.0f + (float)row * 64.0f) / 1024.0f;
    float v0 = v1 - 60.0f / 1024.0f;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1;
    uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteSolidUV(float* uv, u32 quad, float pixelX, float pixelY) {
    const float u = pixelX / 2048.0f;
    const float v = 1.0f - pixelY / 1024.0f;
    uv += quad * 8;
    for (u32 i = 0; i < 4; i++) {
        uv[i * 2] = u;
        uv[i * 2 + 1] = v;
    }
}

static void NativeHud_WriteMagicFrameUV(float* uv, u32 quad, bool doubleMagic) {
    uv += quad * 8;
    const float u0 = 1024.5f / 2048.0f;
    const float u1 = u0 + 439.0f / 2048.0f;
    const float y = doubleMagic ? 756.5f : 720.5f;
    const float v1 = 1.0f - y / 1024.0f;
    const float v0 = v1 - 35.0f / 1024.0f;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1;
    uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteMagicFillUV(float* uv, u32 quad, u32 width) {
    if (width > 106) width = 106;
    uv += quad * 8;
    const float u0 = (1024.5f + (float)(106 - width) * 4.0f) / 2048.0f;
    const float u1 = u0 + 423.0f / 2048.0f;
    const float v1 = 1.0f - 800.5f / 1024.0f;
    const float v0 = v1 - 19.0f / 1024.0f;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1;
    uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteRupeeUV(float* uv, u32 quad) {
    uv += quad * 8;
    const float u0 = 456.0f / 512.0f;
    const float u1 = 474.0f / 512.0f;
    const float v0 = 1.0f - 78.0f / 256.0f;
    const float v1 = 1.0f - 60.0f / 256.0f;
    uv[0] = u0; uv[1] = v0; uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1; uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteDigitUV(float* uv, u32 quad, u32 digit) {
    uv += quad * 8;
    // PR's 0-9 strip uses fixed 8x16 logical cells at y=72.
    const float u0 = (float)(digit * 8) / 512.0f;
    const float u1 = (float)(digit * 8 + 8) / 512.0f;
    const float v0 = 1.0f - 88.0f / 256.0f;
    const float v1 = 1.0f - 72.0f / 256.0f;
    uv[0] = u0; uv[1] = v0; uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1; uv[6] = u1; uv[7] = v1;
}

static u8 NativeHud_AmmoSlotForItem(u8 item) {
    switch (item) {
        case ITEM_STICK: return SLOT_STICK;
        case ITEM_NUT: return SLOT_NUT;
        case ITEM_BOMB: return SLOT_BOMB;
        case ITEM_BOW:
        case ITEM_ARROW_FIRE:
        case ITEM_ARROW_ICE:
        case ITEM_ARROW_LIGHT:
        case ITEM_BOW_ARROW_FIRE:
        case ITEM_BOW_ARROW_ICE:
        case ITEM_BOW_ARROW_LIGHT: return SLOT_BOW;
        case ITEM_SLINGSHOT: return SLOT_SLINGSHOT;
        case ITEM_BOMBCHU: return SLOT_BOMBCHU;
        case ITEM_BEAN: return SLOT_BEAN;
        default: return SLOT_NONE;
    }
}

static void NativeHud_WriteAmmoUV(float* uv, u32 quad, u8 item) {
    const u8 slot = NativeHud_AmmoSlotForItem(item);
    if (slot == SLOT_NONE) {
        NativeHud_WriteSolidUV(uv, quad, 2040.0f, 1016.0f);
        return;
    }

    s32 ammo = gSaveContext.ammo[slot];
    if (ammo < 0) ammo = 0;
    if (ammo > 50) ammo = 50;

    // The atlas stores 0-50 as lossless two-digit PR tiles in an 18-column
    // bank. Single-digit values are right-aligned with a transparent tens
    // cell, matching the native OoT3D counter treatment.
    const u32 column = (u32)ammo % 18;
    const u32 row = (u32)ammo / 18;
    const float u0 = (1024.5f + (float)column * 56.0f) / 2048.0f;
    const float u1 = u0 + 55.0f / 2048.0f;
    const float v1 = 1.0f - (832.5f + (float)row * 64.0f) / 1024.0f;
    const float v0 = v1 - 63.0f / 1024.0f;
    uv += quad * 8;
    uv[0] = u0; uv[1] = v0; uv[2] = u1; uv[3] = v0;
    uv[4] = u0; uv[5] = v1; uv[6] = u1; uv[7] = v1;
}

static void NativeHud_WriteBlankButtonUV(float* uv, u32 quad) {
    uv += quad * 8;
    uv[0] = 0.09375f;    uv[1] = 0.8515625f;
    uv[2] = 0.166015625f; uv[3] = 0.8515625f;
    uv[4] = 0.09375f;    uv[5] = 1.0f;
    uv[6] = 0.166015625f; uv[7] = 1.0f;
}

static void NativeHud_UpdateMappedItems(u8 visible) {
    void* board = NATIVE_HUD_BOARD;
    if (!visible || board == 0) {
        return;
    }

    for(u32 i=0;i<64;++i) {
        float* uv=gNativeHudUVs+(33+i)*8;
        const float u=66.5f/512.0f;
        const float v=1.0f-3.5f/256.0f;
        for(u32 vertex=0;vertex<4;++vertex){uv[vertex*2]=u;uv[vertex*2+1]=v;}
    }
    // These are the live values used by the current second-screen I/II
    // buttons. Avoid reconstructing them through inventory-slot indices.
    u8 itemI = gSaveContext.equips.buttonItems[3];
    u8 itemII = gSaveContext.equips.buttonItems[4];
    // Some touchless layouts leave the cached I/II button-item bytes at
    // ITEM_NONE while retaining the equipped inventory slots. Resolve those
    // two slots from the live inventory so the native HUD mirrors the action
    // that the corner touch targets actually trigger.
    ItemEquips* ageEquips = gSaveContext.linkAge == 0 ?
        &gSaveContext.adultEquips : &gSaveContext.childEquips;
    if (itemI > ITEM_HEART_PIECE_2) itemI = ageEquips->buttonItems[3];
    if (itemII > ITEM_HEART_PIECE_2) itemII = ageEquips->buttonItems[4];
    u8 slotI = gSaveContext.equips.buttonSlots[2];
    u8 slotII = gSaveContext.equips.buttonSlots[3];
    if (slotI == SLOT_NONE) slotI = ageEquips->buttonSlots[2];
    if (slotII == SLOT_NONE) slotII = ageEquips->buttonSlots[3];
    if (itemI > ITEM_HEART_PIECE_2 && slotI < 26) itemI = gSaveContext.items[slotI];
    if (itemII > ITEM_HEART_PIECE_2 && slotII < 26) itemII = gSaveContext.items[slotII];
    NativeHud_WriteItemUV(gNativeHudUVs, 5, gSaveContext.equips.buttonItems[0]);
    NativeHud_WriteItemUV(gNativeHudUVs, 6, gSaveContext.equips.buttonItems[2]);
    NativeHud_WriteItemUV(gNativeHudUVs, 7, gSaveContext.equips.buttonItems[1]);
    NativeHud_WriteItemUV(gNativeHudUVs, 10, itemI);
    NativeHud_WriteItemUV(gNativeHudUVs, 11, itemII);
    NativeHud_WriteItemUV(gNativeHudUVs, 8, ITEM_OCARINA_TIME);

    s32 maxHearts = gSaveContext.healthCapacity > 0 ? gSaveContext.healthCapacity / 16 : 0;
    if (maxHearts > 20) maxHearts = 20;
    s32 health = gSaveContext.health > 0 ? gSaveContext.health : 0;
    for (u32 i = 0; i < 10; i++) {
        s32 remainingCapacity = maxHearts - (s32)i * 2;
        u32 pairCapacity = remainingCapacity <= 0 ? 0 :
                           remainingCapacity == 1 ? 1 : 2;
        NativeHud_WriteHeartPairUV(gNativeHudUVs, 13 + i, pairCapacity,
                                   health - (s32)i * 32);
    }

    u32 rupees = gSaveContext.rupees > 0 ? (u32)gSaveContext.rupees : 0;
    if (rupees > 999) rupees = 999;
    if (sNativeHudPauseLayout != 0) {
        NativeHud_WriteBlankButtonUV(gNativeHudUVs, 0);
    } else {
        NativeHud_WriteRupeeUV(gNativeHudUVs, 0);
    }
    if (rupees >= 100) {
        NativeHud_WriteDigitUV(gNativeHudUVs, 24, rupees / 100);
    } else {
        NativeHud_WriteSolidUV(gNativeHudUVs, 24, 1644.0f, 48.0f);
    }
    if (rupees >= 10) {
        NativeHud_WriteDigitUV(gNativeHudUVs, 25, (rupees / 10) % 10);
    } else {
        NativeHud_WriteSolidUV(gNativeHudUVs, 25, 1644.0f, 48.0f);
    }
    NativeHud_WriteDigitUV(gNativeHudUVs, 26, rupees % 10);

    NativeHud_WriteAmmoUV(gNativeHudUVs, 28,
                          gSaveContext.equips.buttonItems[2]);
    NativeHud_WriteAmmoUV(gNativeHudUVs, 29,
                          gSaveContext.equips.buttonItems[1]);
    NativeHud_WriteAmmoUV(gNativeHudUVs, 12, itemI);
    if (sNativeHudOcarinaLayout)
        NativeHud_WriteBlankButtonUV(gNativeHudUVs, 12);
    NativeHud_WriteAmmoUV(gNativeHudUVs, 23, itemII);

    // The upgrade state lives in dedicated save flags. magicLevel is not a
    // reliable indication of double magic in a naturally progressed save.
    const bool hasMagic = gSaveContext.magicAcquired != 0 || gSaveContext.magicLevel > 0;
    const bool doubleMagic = gSaveContext.doubleMagic != 0;
    if (hasMagic) {
        NativeHud_WriteMagicFrameUV(gNativeHudUVs, 27, doubleMagic);
    } else {
        NativeHud_WriteSolidUV(gNativeHudUVs, 27, 1644.0f, 48.0f);
    }
    u32 magicWidth = 0;
    if (hasMagic) {
        const u32 maxMagic = doubleMagic ? 0x60 : 0x30;
        u32 magic = gSaveContext.magic > 0 ? (u32)gSaveContext.magic : 0;
        if (magic > maxMagic) magic = maxMagic;
        const u32 displayWidth = doubleMagic ? 106 : 70;
        magicWidth = (magic * displayWidth) / maxMagic;
    }
    if (hasMagic) {
        NativeHud_WriteMagicFillUV(gNativeHudUVs, 30, magicWidth);
    } else {
        NativeHud_WriteSolidUV(gNativeHudUVs, 30, 1644.0f, 48.0f);
    }

    // Upload the complete UV stream through the board's native setter.
    // 0x4546F4, used previously, is an internal address calculator rather
    // than a UV getter and caused the correct values to land in unrelated
    // GPU memory.
    const u32 uvHash = NativeHud_UvHash();
    if (!sNativeHudHasUploaded || board != sNativeHudUploadedBoard ||
        uvHash != sNativeHudUploadedHash) {
        NATIVE_HUD_UPLOAD_UVS(board, sizeof(gNativeHudUVs), gNativeHudUVs);
        sNativeHudUploadedHash = uvHash;
        sNativeHudUploadedBoard = board;
        sNativeHudHasUploaded = 1;
    }
}

static void NativeHud_UploadPositions(u8 visible) {
    void* board = NATIVE_HUD_BOARD;
    if (!visible || board == 0) {
        return;
    }
    if (!sNativeHudPositionsDirty && board == sNativeHudPositionBoard) {
        return;
    }

    // The board keeps its CPU geometry and GPU vertex buffer separately. Use
    // its native setter to keep both copies in sync.
    NATIVE_HUD_UPLOAD_POSITIONS(board, sizeof(gNativeHudPositions),
                                gNativeHudPositions);
    sNativeHudPositionBoard = board;
    sNativeHudPositionsDirty = 0;
}

void NativeHud_Update(GlobalContext* globalCtx) {
    const u8 bumper=ManualJump_Get()==2 && ManualJump_IsReady();
    if(bumper!=sManualJumpBumper){sManualJumpBumper=bumper;NativeHud_RebuildPositions();}
    const u8 pauseLayout = InputRemap_IsItemsPageActive() ? 1 :
                           InputRemap_IsItemsMenuOpen() ? 2 : 0;
    const u8 ocarinaLayout = pauseLayout == 0 && InputRemap_IsOcarinaUiOpen();
    if (pauseLayout != sNativeHudPauseLayout ||
        ocarinaLayout != sNativeHudOcarinaLayout) {
        sNativeHudPauseLayout = pauseLayout;
        sNativeHudOcarinaLayout = ocarinaLayout;
        NativeHud_RebuildPositions();
    }
    const u8 visible = globalCtx != 0 && IsInGameOrBossChallenge() &&
                       (sNativeHudScalePercent != 0 || pauseLayout != 0);
    NativeHud_UploadPositions(visible);
    NativeHud_UpdateMappedItems(visible);
    NATIVE_HUD_VISIBLE = visible ? 1 : 0;
}
