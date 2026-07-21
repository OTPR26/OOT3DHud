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

.section .patch_NativeHudIgnoreMotionDisable
.global NativeHudIgnoreMotionDisable_patch
NativeHudIgnoreMotionDisable_patch:
    // OoT3D normally clears the motion-control board's visibility when gyro
    // aiming is disabled. Ocarina Reframed owns this repurposed board, so let
    // NativeHud_Update manages visibility independently of that setting.
    bx lr

.section .patch_NativeHudIgnoreMotionSettingGate1
.global NativeHudIgnoreMotionSettingGate1_patch
NativeHudIgnoreMotionSettingGate1_patch:
    // The native renderer separately skips this board when the saved gyro
    // option is disabled. Visibility now belongs to Ocarina Reframed.
    nop

.section .patch_NativeHudIgnoreMotionSettingGate2
.global NativeHudIgnoreMotionSettingGate2_patch
NativeHudIgnoreMotionSettingGate2_patch:
    nop

.section .patch_NativeActionHudSync
.global NativeActionHudSync_patch
NativeActionHudSync_patch:
    bl hook_NativeActionHudSync
// Native HD renderer. OoT3D's unused motion-control quad already draws
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
.if (_TWN_==1) || (_KOR_==1)
    // The CJK constructor stores position and UV pointers with two STRs,
    // separated by the ADD that selects the UV stack buffer. Preserve the
    // ADD while suppressing both pointer replacements.
    nop
    add r0, sp, #344
    nop
.else
    nop
.endif

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
    .word 424
    .word gNativeHudIndices
    .word 634
    .word 2
    .word 0x0C
