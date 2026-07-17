.arm

.section .patch_before_GlobalContext_Update
.global before_GlobalContext_Update_patch
before_GlobalContext_Update_patch:
    bl hook_before_GlobalContext_Update

.section .patch_after_GlobalContext_Update
.global after_GlobalContext_Update_patch
after_GlobalContext_Update_patch:
    b hook_after_GlobalContext_Update

.section .patch_loader
.global loader_patch
loader_patch:
    b hook_into_loader

.section .patch_CameraUpdate
.global CameraUpdate_patch
CameraUpdate_patch:
    bl hook_CameraUpdate

#ifndef PLUS_CONTROLS_ONLY
.section .patch_NativeActionHudSync
.global NativeActionHudSync_patch
NativeActionHudSync_patch:
    bl hook_NativeActionHudSync

// Native HD renderer proof. OoT3D's unused motion-control quad already draws
// stereoscopically on the top screen. Point it at cam_interface00 (slot 15),
// whose 2:1 texture is replaced losslessly by the official PR HD atlas.
.section .patch_NativeHudTextureSlot
.global NativeHudTextureSlot_patch
NativeHudTextureSlot_patch:
    mov r0, #15

// Keep the descriptor's external buffer pointers instead of replacing them
// with the constructor's four-vertex stack copies.
.section .patch_NativeHudKeepPositionUVPointers
.global NativeHudKeepPositionUVPointers_patch
NativeHudKeepPositionUVPointers_patch:
    nop

.section .patch_NativeHudKeepIndexPointer
.global NativeHudKeepIndexPointer_patch
NativeHudKeepIndexPointer_patch:
    nop

.section .patch_NativeHudDescriptor
.global NativeHudDescriptor_patch
NativeHudDescriptor_patch:
    .word gNativeHudPositions
    .word gNativeHudUVs
    .word 0
#ifdef PLUS_MINIMAL_HUD
    .word 52
#elif defined(PLUS_HEARTS_ONLY)
    .word 132
#else
    .word 424
#endif
    .word gNativeHudIndices
#ifdef PLUS_MINIMAL_HUD
    .word 76
#elif defined(PLUS_HEARTS_ONLY)
    .word 196
#else
    .word 634
#endif
    .word 2
    .word 0x0C
#endif
