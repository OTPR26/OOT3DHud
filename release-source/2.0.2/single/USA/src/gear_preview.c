#include "gear_preview.h"
#include "unified_menu_layout.h"
#include "common.h"
#include "input_remap.h"
#include "unified_menu.h"
#include "input.h"
#include "gear_preview_rotation.h"

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
static SkeletonAnimationModel* sPreview;
static const SkeletonAnimationModel* sLive;
static u8 sRetired;
static u8 sDrawReady;
static const Player* sOwner;
static u32 sResource;
static s16 sScene;
static u16 sIdlePhase;
static u16 sUserYaw;
/* Existing bounded trigonometry implementation in camera.c. */
extern f32 sins(u16 angle);
extern f32 coss(u16 angle);
extern f32 sqrtf(f32 value);

/* Rotate a preview joint around portrait-world Z, expressed in that
 * joint's own coordinates. The two leg chains have different rest axes;
 * derive the axis from the evaluated pose instead of assuming a sign. */
static void GearPreview_SpreadJoint(float* local, const float* world, float degrees) {
    float x = world[8], y = world[9], z = world[10];
    const float length = sqrtf(x * x + y * y + z * z);
    if (length < 0.000001f)
        return;
    x /= length;
    y /= length;
    z /= length;
    const u16 angle = (u16)(s16)(degrees * (65536.0f / 360.0f));
    const float s = sins(angle), c = coss(angle), t = 1.0f - c;
    const float rotation[9] = {t * x * x + c,     t * x * y - s * z, t * x * z + s * y,
                               t * x * y + s * z, t * y * y + c,     t * y * z - s * x,
                               t * x * z - s * y, t * y * z + s * x, t * z * z + c};
    for (u32 row = 0; row < 3; ++row) {
        const float a = local[row * 4], b = local[row * 4 + 1], d = local[row * 4 + 2];
        for (u32 col = 0; col < 3; ++col)
            local[row * 4 + col] =
                a * rotation[col] + b * rotation[3 + col] + d * rotation[6 + col];
    }
}
/* Post-rotate a preview-local joint around its own Z axis. Translation
 * stays unchanged, and each frame starts from the preview's rest values. */
static void GearPreview_Bend(float* m, float degrees) {
    const u16 angle = (u16)(s16)(degrees * (65536.0f / 360.0f));
    const float s = sins(angle), c = coss(angle);
    for (u32 row = 0; row < 3; ++row) {
        const float x = m[row * 4], y = m[row * 4 + 1];
        m[row * 4] = x * c + y * s;
        m[row * 4 + 1] = y * c - x * s;
    }
}

static void GearPreview_LowerArm(float* m, float degrees) {
    const u16 angle = (u16)(s16)(degrees * (65536.0f / 360.0f));
    const float s = sins(angle), c = coss(angle);
    for (u32 row = 0; row < 3; ++row) {
        const float x = m[row * 4], z = m[row * 4 + 2];
        m[row * 4] = x * c - z * s;
        m[row * 4 + 2] = x * s + z * c;
    }
}

static void GearPreview_AnimateJoints(u32* skeleton, u32 bones, float* matrix,
                                      const float* neutralHead) {
    static u8 reported;
    /* Verified from the local adult link_v2 CMB skeleton, not assumed from
     * N64 limb indices. Reject other hierarchies rather than bend equipment. */
    static const s16 parents[25] = {-1, 0,  1,  2,  3,  4,  2,  6,  7,  1, 9,  10, 11,
                                    10, 13, 14, 15, 10, 17, 18, 19, 10, 0, 22, 22};
    if (bones != 25) {
        if (!reported) {
            CitraPrint("GEAR-IDLE unsupported bones=%u\n", bones);
            reported = 1;
        }
        return;
    }
    const u8* descriptors = (const u8*)((const u32*)skeleton[1])[1];
    if (!descriptors || !skeleton[5])
        return;
    for (u32 i = 0; i < 25; ++i)
        if (*(const s16*)(descriptors + i * 40 + 2) != parents[i]) {
            if (!reported) {
                CitraPrint("GEAR-IDLE hierarchy mismatch bone=%u\n", i);
                reported = 1;
            }
            return;
        }
    if (!reported) {
        CitraPrint("GEAR-IDLE joint animation active bones=%u\n", bones);
        reported = 1;
    }
    float* local = (float*)skeleton[4];
    float* world = (float*)skeleton[5];
    float feet[3];
    for (u32 row = 0; row < 3; ++row)
        feet[row] = (world[5 * 12 + row * 4 + 3] + world[8 * 12 + row * 4 + 3]) * 0.5f;
    const float breath = sins((u16)(sIdlePhase * 2));
    const float sway = sins(sIdlePhase);
    const float bend = 7.0f + 3.0f * breath;
    /* Relax the feet-together rest stance. Rotate each thigh outward by
     * eight degrees and counter-rotate its ankle, keeping boots level.
     * Existing knee flex, floating motion and foot-center compensation
     * remain intact; only the independently owned Gear skeleton changes. */
    const float leftSign = world[5 * 12 + 3] < world[8 * 12 + 3] ? -1.0f : 1.0f;
    const float spread = 8.0f;
    GearPreview_SpreadJoint(local + 3 * 12, world + 3 * 12, leftSign * spread);
    GearPreview_SpreadJoint(local + 5 * 12, world + 5 * 12, -leftSign * spread);
    GearPreview_SpreadJoint(local + 6 * 12, world + 6 * 12, -leftSign * spread);
    GearPreview_SpreadJoint(local + 8 * 12, world + 8 * 12, leftSign * spread);
    /* CMB rest upper arms point diagonally outward/down. Bring them
     * alongside the body, starting from fresh preview-owned rest matrices. */
    GearPreview_LowerArm(local + 14 * 12, 22.0f + 5.0f * sway);
    GearPreview_LowerArm(local + 18 * 12, -25.0f + 5.0f * sway);
    GearPreview_Bend(local + 3 * 12, -bend);
    GearPreview_Bend(local + 4 * 12, 2 * bend);
    GearPreview_Bend(local + 5 * 12, -bend);
    GearPreview_Bend(local + 6 * 12, -bend);
    GearPreview_Bend(local + 7 * 12, 2 * bend);
    GearPreview_Bend(local + 8 * 12, -bend);
    GearPreview_Bend(local + 10 * 12, 2.0f + 3.5f * breath);
    GearPreview_Bend(local + 13 * 12, 3.0f * sway);
    GearPreview_Bend(local + 17 * 12, -3.0f * sway);
    GearPreview_Bend(local + 15 * 12, 20.0f + 7.0f * breath);
    GearPreview_Bend(local + 19 * 12, -17.0f - 6.0f * sway);
    /* Do not preserve gameplay's look-at direction. Restore the model's
     * neutral head orientation, then turn gently about the torso's vertical
     * (parent-local X) axis. Keep the joint attachment translation intact. */
    float* head = local + 11 * 12;
    for (u32 row = 0; row < 3; ++row)
        for (u32 col = 0; col < 3; ++col)
            head[row * 4 + col] = neutralHead[row * 4 + col];
    const u16 headAngle = (u16)(s16)(364.0f * sway);
    const float hs = sins(headAngle), hc = coss(headAngle);
    for (u32 col = 0; col < 3; ++col) {
        const float y = head[4 + col], z = head[8 + col];
        head[4 + col] = hc * y - hs * z;
        head[8 + col] = hs * y + hc * z;
    }
    ((void (*)(void*, const float*))0x0030478C)(skeleton, matrix);
    /* Compensate the average foot position in portrait world space after
     * bending; all corrections belong solely to the preview transform. */
    for (u32 row = 0; row < 3; ++row)
        matrix[row * 4 + 3] +=
            feet[row] - (world[5 * 12 + row * 4 + 3] + world[8 * 12 + row * 4 + 3]) * 0.5f;
    ((void (*)(void*, const float*))0x0030478C)(skeleton, matrix);
}
/* One independent preview allocation per boot. USA 3FF53C owns
 * these model components; flag 0 creates default render state instead of
 * borrowing the gameplay actor callback. Never modify the global factory. */
static void GearPreview_TestOwnership(const Player* player, const SkeletonAnimationModel* live) {
    static u8 attempted;
    if (attempted)
        return;
    attempted = 1;
    u32* factory = *(u32**)0x005BE734;
    if (!factory || !factory[0])
        return;
    const u32 create = ((const u32*)factory[0])[2];
    const u32 resource = *(const u32*)((const u8*)player + 0x24dc);
    CitraPrint("GEAR-LINK factory=%08x create=%08x resource=%08x\n", (u32)factory, create,
               resource);
    if (create != 0x003FF53C || !resource || ((const u32*)live->vtbl)[1] != 0x003FEFC0) {
        CitraPrint("GEAR-LINK ownership test skipped: identity mismatch\n");
        return;
    }
    const u32* original = (const u32*)live;
    u32 before[8];
    for (u32 i = 0; i < 8; ++i)
        before[i] = original[i];
    SkeletonAnimationModel* preview =
        ((SkeletonAnimationModel * (*)(void*, u32, u32)) create)(factory, resource, 0);
    if (!preview) {
        CitraPrint("GEAR-LINK ownership test: allocation failed\n");
        return;
    }
    const u32* owned = (const u32*)preview;
    u8 independent = preview != live;
    /* +18 may legitimately be null. All other populated owned pointers
     * must differ; compare across slots as well as corresponding slots. */
    for (u32 i = 1; i < 8; ++i) {
        if (!owned[i])
            continue;
        for (u32 j = 1; j < 8; ++j)
            if (owned[i] == before[j])
                independent = 0;
    }
    CitraPrint("GEAR-LINK allocated=%08x independent=%u skeleton=%08x raw=%08x state=%08x\n",
               (u32)preview, independent, owned[1], owned[5], owned[7]);
    if (!independent || !preview->vtbl || ((const u32*)preview->vtbl)[1] != 0x003FEFC0) {
        /* Do not risk destroying shared gameplay state on a failed probe.
         * This test is bounded to one allocation and never queues a draw. */
        CitraPrint("GEAR-LINK ownership mismatch: cleanup withheld; restart test\n");
        return;
    }
    /* Exercise the independent skeleton's default pose without queuing it.
     * This must succeed before its matrices can be used by a render pass. */
    ((void (*)(SkeletonAnimationModel*))((const u32*)preview->vtbl)[2])(preview);
    CitraPrint("GEAR-LINK independent pose update returned\n");
    /* Retain one allocation across menu visits, without destroying resources
     * that may still be queued by the GPU. Retire on owner/resource/scene
     * change; cross-scene reconstruction remains a separate lifecycle step. */
    sPreview = preview;
    sLive = live;
    sOwner = player;
    sResource = resource;
    u8 unchanged = 1;
    for (u32 i = 0; i < 8; ++i)
        if (original[i] != before[i])
            unchanged = 0;
    CitraPrint("GEAR-LINK retained live-ownership-unchanged=%u\n", unchanged);
}
#endif

void GearPreview_DrawCamera(void* queue) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    void (*nativeDraw)(void*) = (void (*)(void*))0x004224E8;
    const u8 active = gSaveContext.gameMode == 0 && InputRemap_IsGearPageActive() &&
                      !UnifiedMenu_IsOpening() && !UnifiedMenu_IsClosing() &&
                      !UnifiedMenu_GetDialogPage();
    if (!queue || !active || !sDrawReady || !sPreview || sRetired) {
        nativeDraw(queue);
        return;
    }
    u32* count = (u32*)((u8*)queue + 0x212c);
    u32* record = (u32*)((u8*)queue + 0x2130);
    const u32 savedCount = *count;
    const u32 saved = record[0];
    const u32 savedKind = record[1];
    /* Gear's portrait does not depend on which collectible is highlighted.
     * The native selected-item slot may be empty or use another model kind.
     * Temporarily supply our own kind-0 skeleton, restoring the whole slot. */
    /* Collectibles such as Skulltula tokens queue their glow separately.
     * Replace the complete preview list for this draw, not only lists with
     * one entry. The remaining records are untouched and count is restored. */
    if (savedCount > 32) {
        nativeDraw(queue);
        return;
    }
    /* Copy values only, never renderer pointers or ownership. Native
     * 2B9BF8 uses raw+68/count and raw+6C/visibility bytes. */
    const u8* liveRaw = (const u8*)((const u32*)sLive)[5];
    u8* raw = (u8*)((u32*)sPreview)[5];
    u32 meshCount = *(u32*)(raw + 0x68);
    if (meshCount && meshCount <= 256 && meshCount == *(const u32*)(liveRaw + 0x68)) {
        u8* dst = *(u8**)(raw + 0x6c);
        const u8* src = *(const u8* const*)(liveRaw + 0x6c);
        if (dst && src && dst != src)
            for (u32 i = 0; i < meshCount; ++i)
                dst[i] = src[i];
    }
    /* Preview-owned portrait lights. 3130A4 uploads the two RGBA values
     * at +88/+98 when the direction's W at +D4 is exactly 1.0.
     * Keep all pointers, callbacks and renderer ownership unchanged. */
    u32* state = (u32*)((u32*)sPreview)[7];
    const u32* liveState = (const u32*)((const u32*)sLive)[7];
    if (state) {
        for (u32 light = 0; light < 3; ++light) {
            float* values = (float*)((u8*)state + 0x88 + light * 0x60);
            for (u32 channel = 0; channel < 3; ++channel) {
                values[channel] = light == 0 ? 0.55f : 0.12f;
                values[4 + channel] = 0.18f;
            }
            values[3] = values[7] = 1.0f;
            values[16] = light == 1 ? -1.0f : 1.0f;
            values[17] = 1.0f;
            values[18] = -1.0f;
            values[19] = 1.0f;
        }
    }
    /* Inline constant-color enables (six bytes) and six RGBA values.
     * Do not copy the renderer pointer, fog fields, or callbacks. */
    if (state && liveState && state != liveState) {
        for (u32 i = 4; i < 10; ++i)
            ((u8*)state)[i] = ((const u8*)liveState)[i];
        for (u32 i = 0x0c / 4; i < 0x6c / 4; ++i)
            state[i] = liveState[i];
    }
    /* Keep a small portrait turn; joint motion below replaces the previous
     * whole-model breathing scale. Never change the gameplay animation. */
    const u16 yaw = (u16)(s16)(364.0f * sins(sIdlePhase));
    const float sy = sins(yaw), cy = coss(yaw);
    float matrix[12] = {0.0025f * cy,  0, 0.0025f * sy, 0,     0, 0.0025f, 0, -2.5f,
                        -0.0025f * sy, 0, 0.0025f * cy, -36.0f};
    /* Preview-only slow weight shift and buoyant idle; no scene-actor writes.
     * Facing stays within two degrees of front instead of inheriting a turn. */
    matrix[3] = 0.05f * sins(sIdlePhase);
    matrix[7] += 0.16f * sins((u16)(sIdlePhase * 2));
    ((void (*)(void*, const float*))0x003721E0)(sPreview, matrix);
    ((u8*)sPreview)[0xac] = 1;
    ((u8*)sPreview)[0xad] = 1;
    ((void (*)(void*))((u32*)sPreview->vtbl)[2])(sPreview);
    /* Use the preview's newly evaluated rest pose, never gameplay joints.
     * Pausing during a run/turn must not become the portrait's base pose.
     * Appearance is still synchronized independently below. */
    u32* skeleton = (u32*)((u32*)sPreview)[1];
    const u32* liveSkeleton = (const u32*)((const u32*)sLive)[1];
    u32 bones = *(const u32*)(*(const u32*)skeleton[1] + 8);
    if (bones && bones <= 128 && skeleton[1] == liveSkeleton[1] && skeleton[4] && liveSkeleton[4] &&
        skeleton[4] != liveSkeleton[4] && skeleton[12] == 0) {
        float neutralHead[12] = {0};
        if (bones > 11)
            for (u32 i = 0; i < 12; ++i)
                neutralHead[i] = ((const float*)skeleton[4])[11 * 12 + i];
        ((void (*)(void*, const float*))0x0030478C)(skeleton, matrix);
        GearPreview_AnimateJoints(skeleton, bones, matrix, neutralHead);
        /* Turn the completed portrait pose, not its rest-space joints.
         * This keeps the leg-spread axes/signs independent of user yaw.
         * Rotate about the portrait origin, never the camera origin. */
        const float turnSin = sins(sUserYaw), turnCos = coss(sUserYaw);
        for (u32 col = 0; col < 4; ++col) {
            const float x = matrix[col];
            const float z = matrix[8 + col] + (col == 3 ? 36.0f : 0.0f);
            matrix[col] = turnCos * x + turnSin * z;
            matrix[8 + col] = -turnSin * x + turnCos * z - (col == 3 ? 36.0f : 0.0f);
        }
        ((void (*)(void*, const float*))0x003721E0)(sPreview, matrix);
        ((void (*)(void*, const float*))0x0030478C)(skeleton, matrix);
    }
    /* Sync inline appearance values, including tunic/face texture selection.
     * 46B974/46B9A0 read the live texture lookup tables; borrow those read-only
     * only while this synchronous draw builds commands. Restore all three
     * ownership fields afterward, before any update/destruction can run.
     * The same validated live resource remains alive for this draw. */
    u32 savedTextureTables[128][3];
    u32 syncedMaterials = 0;
    u32* mapper = (u32*)((u32*)sPreview)[4];
    const u32* liveMapper = (const u32*)((const u32*)sLive)[4];
    if (mapper && liveMapper && mapper[0] == liveMapper[0] && mapper[1] != liveMapper[1]) {
        const u32* materials = *(const u32**)(mapper[0] + 8);
        u32 materialCount = *(const u32*)(materials[0] + 8);
        if (materialCount <= 128 && mapper[1] && liveMapper[1]) {
            syncedMaterials = materialCount;
            for (u32 m = 0; m < materialCount; ++m) {
                u8* dst = (u8*)mapper[1] + m * 0x124;
                const u8* src = (const u8*)liveMapper[1] + m * 0x124;
                for (u32 i = 0; i < 3; ++i)
                    savedTextureTables[m][i] = ((u32*)(dst + 0x118))[i];
                for (u32 i = 0; i < 0x118; ++i)
                    dst[i] = src[i];
                const u32* table = (const u32*)(src + 0x118);
                if (table[0] <= 256 && (!table[0] || (table[1] && table[2]))) {
                    for (u32 i = 0; i < 3; ++i)
                        ((u32*)(dst + 0x118))[i] = table[i];
                } else {
                    dst[7] = dst[8] = dst[9] = 0;
                }
            }
        }
    }
    record[0] = (u32)sPreview;
    record[1] = 0;
    *count = 1;
    /* Enlarge only the portrait camera, not Gear's text presentation.
     * 40% larger than {214,76,180,120}, centered on the same portrait
     * lane with its feet anchored. Native rotated viewport allows -30. */
    ((void (*)(unsigned, int, unsigned, unsigned))0x002FEABC)(48, -30 - MENU_GEAR_PORTRAIT_SHIFT,
                                                              336, 252);
    nativeDraw(queue);
    for (u32 m = 0; m < syncedMaterials; ++m)
        for (u32 i = 0; i < 3; ++i)
            ((u32*)((u8*)mapper[1] + m * 0x124 + 0x118))[i] = savedTextureTables[m][i];
    ((void (*)(unsigned, int, unsigned, unsigned))0x002FEABC)(88, 6, 240, 180);
    *count = savedCount;
    record[0] = saved;
    record[1] = savedKind;
    static u8 reported;
    if (!reported) {
        CitraPrint("GEAR-LINK independent camera draw returned\n");
        reported = 1;
    }
#else
    (void)queue;
#endif
}

void GearPreview_TracePresentation(void* pass) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    static u8 traced;
    static u8 settledFrames;
    if (traced || !InputRemap_IsGearPageActive() || UnifiedMenu_IsOpening() ||
        UnifiedMenu_IsClosing() || UnifiedMenu_GetDialogPage())
        return;
    if (!pass)
        return;
    if (settledFrames++ < 30)
        return;
    traced = 1;
    /* 300328 passes root+25F0 here. Camera-specific model queue 4224E8
     * receives root+180, count +212C, 8-byte records starting +2130. */
    const u8* cameraQueue = (const u8*)pass - 0x2470;
    const u32 cameraCount = *(const u32*)(cameraQueue + 0x212c);
    CitraPrint("GEAR-LINK camera-models=%u\n", cameraCount);
    if (cameraCount <= 32) {
        for (u32 i = 0; i < cameraCount; ++i) {
            const u32* record = (const u32*)(cameraQueue + 0x2130 + i * 8);
            const u32* model = (const u32*)record[0];
            CitraPrint("GEAR-LINK camera-index=%u model=%08x kind=%u\n", i, record[0],
                       record[1] & 255);
            if (!model || (record[1] & 255) != 0)
                continue;
            CitraPrint("GEAR-LINK camera-vtable=%08x raw=%08x\n", model[0], model[5]);
            for (u32 row = 0; row < 3; ++row) {
                const u32* matrix = model + 0x7c / 4 + row * 4;
                CitraPrint("GEAR-LINK camera-matrix=%u %08x %08x %08x %08x\n", row, matrix[0],
                           matrix[1], matrix[2], matrix[3]);
            }
        }
    }
    /* Verified ARM at 2FEA30: model counts at +754+4*stage;
     * model lists at +770+96*stage. Snapshot only, before native pass 5. */
    const u8* manager = pass;
    for (u32 stage = 0; stage < 6; ++stage) {
        u32 count = *(const u32*)(manager + 0x754 + stage * 4);
        CitraPrint("GEAR-LINK pass=%u models=%u boards=%u\n", stage, count,
                   *(const u32*)(manager + 0x14 + stage * 4));
        if (count > 24)
            continue;
        for (u32 i = 0; i < count; ++i) {
            const u32* model = *(const u32**)(manager + 0x770 + stage * 96 + i * 4);
            if (!model)
                continue;
            CitraPrint("GEAR-LINK pass=%u index=%u model=%08x raw=%08x override=%u\n", stage, i,
                       (u32)model, model[5], ((const u8*)model)[0xac]);
            for (u32 row = 0; row < 3; ++row) {
                const u32* matrix = model + 0x7c / 4 + row * 4;
                CitraPrint("GEAR-LINK matrix row=%u %08x %08x %08x %08x\n", row, matrix[0],
                           matrix[1], matrix[2], matrix[3]);
            }
        }
    }
#else
    (void)pass;
#endif
}

void GearPreview_Probe(GlobalContext* globalCtx) {
#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
    static u8 started;
    if (!started) {
        CitraPrint("GEAR-LINK probe active\n");
        started = 1;
    }
    static const Player* previousPlayer;
    static u16 previousEquipment;
    static u8 previousActive;
    sDrawReady = 0;
    /* Inspect only the current context/player, never a previously saved
     * actor after a transition. Menu close itself is not retirement. */
    const Player* player =
        globalCtx && gSaveContext.gameMode == 0 ? globalCtx->mainCamera.player : 0;
    if (sPreview && !sRetired) {
        if (!player || player != sOwner || globalCtx->sceneNum != sScene ||
            player->skelAnime.unk_28 != sLive ||
            *(const u32*)((const u8*)player + 0x24dc) != sResource)
            sRetired = 1;
    }
    const u8 active = globalCtx && gSaveContext.gameMode == 0 && InputRemap_IsGearPageActive() &&
                      !UnifiedMenu_IsOpening() && !UnifiedMenu_IsClosing() &&
                      !UnifiedMenu_GetDialogPage();
    sUserYaw = GearPreview_UpdateYaw(sUserYaw, rInputCtx.cStick.dx, active, previousActive);
    if (!active) {
        previousActive = 0;
        return;
    }
    /* Advance once per active menu update, not once per render pass. */
    sIdlePhase += 384;
    if (!player)
        return;
    if (sPreview && !sRetired)
        sDrawReady = 1;
    const u16 equipment = gSaveContext.equips.equipment;
    if (previousActive && previousPlayer == player && previousEquipment == equipment)
        return;
    previousActive = 1;
    previousPlayer = player;
    previousEquipment = equipment;
    /* USA 34913C reads live equipment at SaveContext+8A; mask/shift
     * tables 53CB0C/53CB08 are 000F,00F0,0F00,F000 and 0,4,8,12.
     * Never fall back to the inactive age's saved equipment. */
    CitraPrint("GEAR-LINK age=%d equip=%04x sword=%u shield=%u tunic=%u boots=%u\n",
               (int)gSaveContext.linkAge, equipment, equipment & 15, (equipment >> 4) & 15,
               (equipment >> 8) & 15, equipment >> 12);
    const SkeletonAnimationModel* model = player->skelAnime.unk_28;
    if (!model || !model->vtbl)
        return;
    const u32* vtable = (const u32*)model->vtbl;
    CitraPrint("GEAR-LINK model=%08x update=%08x destroy=%08x raw=%08x\n", (u32)model, vtable[2],
               vtable[1], (u32)model->unk_draw_struct_14);
    /* Native player rendering uses actor+1A4 (zero-based tunic),
     * +1A6 (shield) and +1A7 (zero-based boots). Compare without writing. */
    const u8* actor = (const u8*)player;
    CitraPrint("GEAR-LINK actor-tunic=%u shield=%u boots=%u resource=%08x\n", actor[0x1a4],
               actor[0x1a6], actor[0x1a7], *(const u32*)(actor + 0x24dc));
    if (!sPreview)
        sScene = globalCtx->sceneNum;
    GearPreview_TestOwnership(player, model);
    sDrawReady = sPreview && !sRetired;
#else
    (void)globalCtx;
#endif
}
