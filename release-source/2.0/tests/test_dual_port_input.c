/* Host regression test of the actual generated Dual Screen input adapter. */
#define _Static_assert(...)
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "input_remap.h"
static u32 native_state[32], native_input[16];
static int game_mode;
#define DUAL_OPTIONS_STATE native_state
#define DUAL_OPTIONS_MODE game_mode
#define DUAL_OPTIONS_INPUT native_input
#include "input_remap.c"

InputContext rInputCtx;
static int begins, ends, updates, exits, fail_save, decision=-1;
static u32 last_pressed;
u32 irrstKeysHeld(void) { return 0; }
void NativeHud_CycleScale(void) {}
void Camera_ApplyControlAction(ControlAction action) { (void)action; }
void MenuOptions_Begin(void) { ++begins; }
void MenuOptions_End(void) { ++ends; }
int MenuOptions_Update(u32 pressed) { ++updates; last_pressed=pressed; return decision; }
void MenuOptions_ExitNative(int accept) { ++exits; if (!(accept && fail_save)) native_state[15]=3; }
u32 MenuOptions_NativeButtons(void) { return 0; }

int main(void) {
    static GlobalContext context;
    native_input[3]=BUTTON_A; native_input[9]=BUTTON_DOWN;
    assert(UnifiedMenu_GetNativeMenuButtons()==BUTTON_A);
    assert(UnifiedMenu_GetNativeMenuRepeat()==BUTTON_DOWN);
    native_state[10]=3; native_state[15]=0;
    rInputCtx.cur.val=BUTTON_A; rInputCtx.pressed.val=BUTTON_A;
    InputRemap_Update(&context);
    assert(begins==1 && updates==0 && UnifiedMenu_IsOptionsDialog());
    native_state[15]=2;
    InputRemap_Update(&context);
    assert(begins==1 && updates==0); /* Opening press cannot edit a setting. */
    assert(UnifiedMenu_GetNativeMenuButtons()==0);
    assert(UnifiedMenu_GetNativeMenuRepeat()==0);
    rInputCtx.cur.val=0; rInputCtx.pressed.val=0;
    InputRemap_Update(&context);
    assert(updates==1);
    rInputCtx.pressed.val=BUTTON_RIGHT;
    InputRemap_Update(&context);
    assert(last_pressed==BUTTON_RIGHT);
    u8 sample[12]; memset(sample,0xa5,sizeof(sample)); s32 count=1;
    InputRemap_FilterTouch(sample,&count);
    assert(sample[4]==0 && count==1);
    for (unsigned i=5;i<sizeof(sample);++i) assert(sample[i]==0xa5);
    fail_save=1; decision=1;
    InputRemap_Update(&context);
    assert(exits==1 && ends==0 && UnifiedMenu_IsOptionsDialog());
    fail_save=0;
    decision=0; rInputCtx.cur.val=rInputCtx.pressed.val=BUTTON_B;
    InputRemap_Update(&context);
    assert(exits==2 && ends==1 && !UnifiedMenu_IsOptionsDialog());
    InputRemap_Update(&context);
    assert(ends==1 && !UnifiedMenu_IsOptionsDialog());
    assert(UnifiedMenu_GetNativeMenuButtons()==0); /* No exit edge leaks to Save. */
    InputRemap_Update(&context);
    assert(ends==1 && exits==2);
    native_state[10]=2;
    rInputCtx.cur.val=rInputCtx.pressed.val=0;
    InputRemap_Update(&context);
    assert(UnifiedMenu_GetNativeMenuButtons()==BUTTON_A);
    decision=-1; native_state[10]=3; native_state[15]=2;
    InputRemap_Update(&context); assert(begins==2);
    game_mode=2;
    InputRemap_Update(&context); assert(ends==2);
    InputRemap_Update(&context); assert(ends==2);
    assert(!sDualOptions && !sDualExitHeld);

    union { u32 aligned[3]; u8 bytes[12]; } out;
    memset(&out,0xa5,sizeof(out)); count=0;
    InputRemap_ApplyVanillaAction(CONTROL_ACTION_ITEM_I);
    InputRemap_FilterTouch(out.bytes,&count);
    assert(count==1 && *(s16*)out.bytes==316 && *(s16*)(out.bytes+2)==3 && out.bytes[4]==1);
    for (unsigned i=5;i<sizeof(out);++i) assert(out.bytes[i]==0xa5);
    InputRemap_EndUpdate(); count=0;
    InputRemap_FilterTouch(out.bytes,&count); assert(count==1 && out.bytes[4]==0);
    memset(&out,0x33,sizeof(out)); count=1;
    rInputCtx.touchHeld=1;
    InputRemap_ApplyVanillaAction(CONTROL_ACTION_OCARINA);
    InputRemap_FilterTouch(out.bytes,&count); assert(out.bytes[4]==0x33);
    puts("Dual Options lifecycle, entry/exit latches, native input passthrough, touch suppression and shortcut priority passed");
}
