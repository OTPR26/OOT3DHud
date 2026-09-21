.arm

.if _KOR_==1
.section .patch_KoreaDualInstrument
    bl KoreaDual_AfterTopPass
.endif

.if _TWN_==1
.section .patch_TaiwanDualInstrument
    bl TaiwanDual_AfterTopPass
.endif

.section .patch_before_GlobalContext_Update
.global before_GlobalContext_Update_patch
before_GlobalContext_Update_patch:
    bl hook_before_GlobalContext_Update

.section .patch_after_GlobalContext_Update
.global after_GlobalContext_Update_patch
after_GlobalContext_Update_patch:
    b hook_after_GlobalContext_Update

.section .patch_SelectItemsMenu
.global SelectItemsMenu_patch
SelectItemsMenu_patch:
    bl hook_SelectItemsMenu

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

// USA and Europe use the same Latin message font. Their HD QBF stores
// 32x32 source cells in a 512x512 atlas, while the original renderer expects
// 16x16 cells in a 256x256 atlas. Keep the high-resolution source cells but
// draw and measure them at the game's original visual scale.
.if (_USA_==1) || (_EUR_==1)
.section .patch_MessageFontAtlasDimensions
.global MessageFontAtlasDimensions_patch
MessageFontAtlasDimensions_patch:
    mov r3, #0x200

.section .patch_MessageFontCommandCapacity
.global MessageFontCommandCapacity_patch
MessageFontCommandCapacity_patch:
    mov r2, #0x100

.section .patch_MessageFontQuadWidth
.global MessageFontQuadWidth_patch
MessageFontQuadWidth_patch:
    b MessageFontQuadWidth_hook

.section .text.MessageFontQuadWidth, "ax", %progbits
.align 2
.global MessageFontQuadWidth_hook
MessageFontQuadWidth_hook:
    // Shared renderer also draws native small date glyphs. Scale only 32px cells.
    push {r1, r2}
    mrs r2, cpsr
    ldr r1, [sp, #24]
    cmp r1, #32
    rsbeq ip, ip, ip, lsl #3
    addeq ip, sl, ip, asr #4
    addne ip, ip, sl
    msr cpsr_f, r2
    pop {r1, r2}
    ldr pc, =0x002B72B8

// Lower the complete glyph quad by one native pixel without clipping it.
.section .patch_MessageFontQuadBaseline
.global MessageFontQuadBaseline_patch
MessageFontQuadBaseline_patch:
    b MessageFontQuadBaseline_hook

.section .text.MessageFontQuadBaseline, "ax", %progbits
.align 2
.global MessageFontQuadBaseline_hook
MessageFontQuadBaseline_hook:
    ldr r9, [sp, #52]
    // Preserve the native loop's flags and baseline for non-message glyphs.
    push {r1, r2}
    mrs r2, cpsr
    ldr r1, [sp, #28]
    cmp r1, #32
    addeq r9, r9, #1
    rsbeq ip, ip, ip, lsl #3
    addeq ip, r9, ip, asr #4
    addne ip, ip, r9
    msr cpsr_f, r2
    pop {r1, r2}
    ldr pc, =0x002B72E4

// Preserve the original 16-pixel line advance even though the replacement
// QBF reports a 32-pixel cell height.
.section .patch_MessageFontLineAdvancePrimary
.global MessageFontLineAdvancePrimary_patch
MessageFontLineAdvancePrimary_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontLineAdvanceSecondary
.global MessageFontLineAdvanceSecondary_patch
MessageFontLineAdvanceSecondary_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontMeasureSecondary
.global MessageFontMeasureSecondary_patch
MessageFontMeasureSecondary_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontMeasureBlock0
.global MessageFontMeasureBlock0_patch
MessageFontMeasureBlock0_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontMeasureBlock1
.global MessageFontMeasureBlock1_patch
MessageFontMeasureBlock1_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontMeasureBlock2
.global MessageFontMeasureBlock2_patch
MessageFontMeasureBlock2_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontMeasureBlock3
.global MessageFontMeasureBlock3_patch
MessageFontMeasureBlock3_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontMeasureBounds0
.global MessageFontMeasureBounds0_patch
MessageFontMeasureBounds0_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontMeasureBounds1
.global MessageFontMeasureBounds1_patch
MessageFontMeasureBounds1_patch:
    bl MessageFontLayoutHeight_hook

.section .patch_MessageFontMeasureBounds2
.global MessageFontMeasureBounds2_patch
MessageFontMeasureBounds2_patch:
    bl MessageFontLayoutHeight_hook

.section .text.MessageFontLayoutHeight, "ax", %progbits
.align 2
.global MessageFontLayoutHeight_hook
MessageFontLayoutHeight_hook:
    push {lr}
    ldr ip, =0x002DA7C8
    blx ip
    mov r0, r0, asr #1
    pop {pc}
.endif
