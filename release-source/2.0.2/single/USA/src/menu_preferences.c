#include "menu_preferences.h"
#include "menu_preferences_format.h"
#include "camera.h"
#include "native_hud.h"
#include "movement_options.h"
#include "common.h"
#include "3ds/srv.h"
#include "3ds/svc.h"
#include "3ds/ipc.h"

#if UNIFIED_MENU_PROTOTYPE && defined(Version_USA)
/* Dedicated SDMC sidecars, never the game's save archive. IPC layouts
 * cross-checked against devkitPro/libctru services/fs.c. Two checksummed
 * generations retain the previous settings across interrupted writes. */
static int sSlot = -1;
static u32 sIdentity;
static Handle sFs;
static Result Request(Handle handle) {
    Result result = svcSendSyncRequest(handle);
    return result < 0 ? result : (Result)getThreadCommandBuffer()[1];
}
static int Open(Handle* file, u32 flags) {
    u32* c = getThreadCommandBuffer();
    if (!sFs) {
        if (srvGetServiceHandle(&sFs, "fs:USER") < 0) {
            sFs = 0;
            return 0;
        }
        c[0] = IPC_MakeHeader(0x801, 0, 2);
        c[1] = IPC_Desc_CurProcessId();
        c[2] = 0;
        if (Request(sFs) < 0) {
            svcCloseHandle(sFs);
            sFs = 0;
            return 0;
        }
    }
    char path[34];
    PrefPath(path, sSlot);
    c[0] = IPC_MakeHeader(0x803, 8, 4);
    c[1] = 0;
    c[2] = 9;
    c[3] = 1;
    c[4] = 1;
    c[5] = 3;
    c[6] = sizeof(path);
    c[7] = flags;
    c[8] = 0;
    c[9] = IPC_Desc_StaticBuffer(1, 2);
    c[10] = (u32) "";
    c[11] = IPC_Desc_StaticBuffer(sizeof(path), 0);
    c[12] = (u32)path;
    Result result = Request(sFs);
    if (result < 0) {
        CitraPrint("PREF open slot=%d flags=%u result=%08x\n", sSlot, flags, (u32)result);
        return 0;
    }
    *file = c[3];
    return 1;
}
static void Close(Handle file) {
    getThreadCommandBuffer()[0] = IPC_MakeHeader(0x808, 0, 0);
    Request(file);
    svcCloseHandle(file);
}
static int Transfer(Handle file, u8* bytes, u32 offset, int write) {
    u32* c = getThreadCommandBuffer();
    c[0] = IPC_MakeHeader(write ? 0x803 : 0x802, write ? 4 : 3, 2);
    c[1] = offset;
    c[2] = 0;
    c[3] = PREF_SIZE;
    unsigned desc = write ? 5 : 4;
    if (write)
        c[4] = 1; /* flush this record before reporting success */
    c[desc] = IPC_Desc_Buffer(PREF_SIZE, write ? IPC_BUFFER_R : IPC_BUFFER_W);
    c[desc + 1] = (u32)bytes;
    return Request(file) >= 0 && c[2] == PREF_SIZE;
}
static int ReadRecords(u8 records[2][PREF_SIZE]) {
    memset(records, 0, 2 * PREF_SIZE);
    Handle file;
    if (!Open(&file, 1))
        return -1;
    for (u32 i = 0; i < 2; ++i)
        if (!Transfer(file, records[i], i * PREF_SIZE, 0))
            memset(records[i], 0, PREF_SIZE);
    Close(file);
    return PrefNewest(records[0], records[1], sSlot, sIdentity);
}
static void Apply(const u8* v) {
    volatile u8* save = (volatile u8*)&gSaveContext;
    save[0x2d] = v[0];
    save[0x13d8] = v[1];
    save[0xf] = v[2];
    static const u8 sizes[] = {0, 75, 100, 125};
    NativeHud_SetScalePercent(sizes[v[3]]);
    Camera_SetDistanceOption(v[4]);
    Camera_SetControlMode(v[5]);
    for (unsigned i = 0; i < 3; ++i)
        MovementOptions_Set(i, v[6 + i]);
}
void MenuPreferences_Update(GlobalContext* context) {
    if (!context || gSaveContext.gameMode != 0) {
        sSlot = -1;
        return;
    }
    const u8* save = (const u8*)&gSaveContext;
    const s32 file = *(const s32*)(save + 0x14dc);
    if (file < 0 || file > 2 || gSaveContext.masterQuestFlag > 1 || !context->mainCamera.player) {
        sSlot = -1;
        return;
    }
    const int slot = file + 3 * gSaveContext.masterQuestFlag;
    const u32 identity = PrefChecksum(save + 0x1c, 17); /* UTF-16 name and length */
    if (sSlot == slot && sIdentity == identity)
        return;
    sSlot = slot;
    sIdentity = identity;
    /* Missing/invalid preferences must never inherit another slot's HUD. */
    NativeHud_SetScalePercent(100);
    Camera_SetDistanceOption(2);
    Camera_SetControlMode(0);
    for (unsigned i = 0; i < 3; ++i)
        MovementOptions_Set(i, 0);
    u8 records[2][PREF_SIZE];
    int latest = ReadRecords(records);
    if (latest >= 0) {
        u8 values[PREF_VALUES];
        memcpy(values, records[latest] + 6, 6);
        memcpy(values + 6, records[latest] + 20, 3);
        Apply(values);
    }
    CitraPrint("PREF load slot=%d identity=%08x generation=%d\n", sSlot, sIdentity, latest);
}
int MenuPreferences_Save(const u8 values[9]) {
    if (sSlot < 0 || sSlot > 5 || !PrefValuesValid(values))
        return 0;
    u8 records[2][PREF_SIZE], encoded[PREF_SIZE], verify[PREF_SIZE];
    int latest = ReadRecords(records), target = latest == 0 ? 1 : 0;
    u32 generation = latest < 0 ? 1 : PrefGet32(records[latest] + 16) + 1;
    PrefEncode(encoded, sSlot, sIdentity, generation, values);
    Handle file;
    if (!Open(&file, 7))
        return 0;
    int success = Transfer(file, encoded, target * PREF_SIZE, 1);
    Close(file);
    if (!success || !Open(&file, 1))
        return 0;
    success =
        Transfer(file, verify, target * PREF_SIZE, 0) && PrefEqual(encoded, verify, PREF_SIZE);
    Close(file);
    CitraPrint("PREF saved slot=%d generation=%u verified=%d\n", sSlot, generation, success);
    return success;
}
#else
void MenuPreferences_Update(GlobalContext* context) {
    (void)context;
}
int MenuPreferences_Save(const u8 values[9]) {
    (void)values;
    return 0;
}
#endif
