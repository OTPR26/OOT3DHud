#include "memory_probe.h"
#include "common.h"

/* Read-only USA prototype diagnostics. Use the native heap's locking/statistics
 * routine; never walk a potentially changing allocator list ourselves. */
void MemoryProbe_Log(const char* phase, int detail) {
#if UNIFIED_MENU_PROTOTYPE && (defined(Version_USA) || defined(Version_EUR) || defined(Version_JP))
    u32 largest=0, freeBytes=0, usedBytes=0;
    if (!*(volatile u32*)0x00565500) return;
    ((void(*)(u32*,u32*,u32*))ADDR(0x00301954))(&largest,&freeBytes,&usedBytes);
    CitraPrint("MEM147 %s n=%d free=%u largest=%u used=%u\n",
               phase,detail,freeBytes,largest,usedBytes);
#else
    (void)phase; (void)detail;
#endif
}

void MemoryProbe_Update(GlobalContext* globalCtx) {
#if UNIFIED_MENU_PROTOTYPE && (defined(Version_USA) || defined(Version_EUR) || defined(Version_JP))
    static u32 frames;
    static s16 lastScene=-1;
    if (!globalCtx) return;
    if (globalCtx->sceneNum!=lastScene || ++frames>=60) {
        frames=0; lastScene=globalCtx->sceneNum;
        MemoryProbe_Log("scene",lastScene);
    }
#else
    (void)globalCtx;
#endif
}
