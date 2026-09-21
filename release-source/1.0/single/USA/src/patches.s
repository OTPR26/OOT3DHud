.arm

.if (_JP_==1) && (_JP_SINGLE_SCREEN_PROBE_==1)
.section .patch_JapanFileBefore
    bl hook_JapanFileBefore
.section .patch_JapanFileAfter
    bl hook_JapanFileAfter
.section .patch_JapanFileText
    blne SingleScreen_DrawFileText
.section .patch_JapanFileBackdrop
    bl hook_JapanFileBackdrop
.section .patch_JapanFileHeading
    bl SingleScreen_UpdateFileBoard
.section .patch_JapanTop
    bl JapanProbe_AfterTop
.section .patch_JapanCallback
    blne JapanProbe_Callback
.section .patch_JapanPresentation
    bl hook_JapanPresentation
.section .patch_JapanOverlay
    bl JapanProbe_Overlay
.section .patch_JapanTitle
    bl JapanProbe_Title
.section .patch_JapanLeather
    bl JapanProbe_Backdrop
.section .patch_JapanItems
    b hook_JapanItems
.section .patch_JapanGear
    b hook_JapanGear
.section .patch_JapanMap
    b hook_JapanMap
.endif
.if (_TWN_==1) && (_TWN_SINGLE_SCREEN_PROBE_==1)
.section .patch_TaiwanFileBefore
    bl hook_TaiwanFileBefore
.section .patch_TaiwanFileAfter
    bl hook_TaiwanFileAfter
.section .patch_TaiwanFileText
    blne SingleScreen_DrawFileText
.section .patch_TaiwanFileBackdrop
    bl hook_TaiwanFileBackdrop
.section .patch_TaiwanFileHeading
    bl SingleScreen_UpdateFileBoard
.section .patch_TaiwanProbeTopPass
    bl TaiwanProbe_AfterTopPass
.section .patch_TaiwanProbePresentation
    bl hook_TaiwanProbePresentation
.section .patch_TaiwanProbeOverlay
    bl TaiwanProbe_Overlay
.section .patch_TaiwanProbeTitle
    bl TaiwanProbe_Title
.section .patch_TaiwanProbeLeather
    bl TaiwanProbe_Backdrop
.section .patch_TaiwanProbeItems
    b hook_TaiwanProbeItems
.section .patch_TaiwanProbeGear
    b hook_TaiwanProbeGear
.section .patch_TaiwanProbeMap
    b hook_TaiwanProbeMap
.section .patch_TaiwanProbeCallback
    blne TaiwanProbe_SecondaryCallback
.endif

.if _DISABLE_BEFORE_GLOBAL_HOOK_==0
    .section .patch_before_GlobalContext_Update
    .global before_GlobalContext_Update_patch
before_GlobalContext_Update_patch:
    bl hook_before_GlobalContext_Update
.endif

.if _DISABLE_AFTER_GLOBAL_HOOK_==0
    .section .patch_after_GlobalContext_Update
    .global after_GlobalContext_Update_patch
after_GlobalContext_Update_patch:
    b hook_after_GlobalContext_Update
.endif

.if _DISABLE_SELECT_PATCH_==0
    .section .patch_SelectItemsMenu
    .global SelectItemsMenu_patch
SelectItemsMenu_patch:
    bl hook_SelectItemsMenu
.endif

.if (_USA_==1) || (_EUR_==1) || (_JP_==1)
.section .patch_ItemAssignXDestination1
    bl hook_ItemAssignXDestination

.section .patch_ItemAssignXPosition1
    bl hook_ItemAssignXPositionR0

.section .patch_ItemAssignIPosition1
    bl hook_ItemAssignIPositionR0

.section .patch_ItemAssignYDestination1
    bl hook_ItemAssignYDestination

.section .patch_ItemAssignYPosition1
    bl hook_ItemAssignYPositionR0

.section .patch_ItemAssignIIPosition1
    bl hook_ItemAssignIIPositionR0

.section .patch_ItemAssignXDestination2
    bl hook_ItemAssignXDestination

.section .patch_ItemAssignXPosition2
    bl hook_ItemAssignXPositionR1

.section .patch_ItemAssignIPosition2
    bl hook_ItemAssignIPositionR2

.section .patch_ItemAssignYDestination2
    bl hook_ItemAssignYDestination

.section .patch_ItemAssignYPosition2
    bl hook_ItemAssignYPositionR1

.section .patch_ItemAssignIIPosition2
    bl hook_ItemAssignIIPositionR2
.endif

.if _USA_==1
.section .patch_ItemsPageActivated
.if _USA_==1
.section .patch_SyncNativeMenu
    bl hook_SyncNativeMenu
.section .patch_ItemsPageActivated
.endif
    b hook_ItemsPageActivated

.section .patch_GearPageActivated
    b hook_GearPageActivated

.section .patch_MapPageActivated
    b hook_MapPageActivated
.endif

.if (_USA_==1) || (_EUR_==1) || (_JP_==1)
.section .patch_SingleScreenAfterTopPass
.global SingleScreenAfterTopPass_patch
SingleScreenAfterTopPass_patch:
    bl SingleScreen_AfterTopPass

.section .patch_SingleScreenBeforeTopOverlay
.global SingleScreenBeforeTopOverlay_patch
SingleScreenBeforeTopOverlay_patch:
    bl hook_SingleScreenBeforeTopOverlay

.if _USA_==1
.section .patch_FileSelectBeforeScene
    bl hook_FileSelectBeforeScene
.section .patch_FileSelectAfterScene
    bl hook_FileSelectAfterScene
.section .patch_FileSelectText
    blne hook_FileSelectText
.section .patch_FileSelectBackdrop
    bl hook_FileSelectBackdrop
.section .patch_FileSelectLeather
    bl hook_FileSelectLeather
.section .patch_FileSelectHeading
    bl hook_FileSelectHeading

.section .patch_SingleScreenBeforeTopPresentation
.global SingleScreenBeforeTopPresentation_patch
SingleScreenBeforeTopPresentation_patch:
    bl hook_SingleScreenBeforeTopPresentation

.section .patch_SingleScreenMenuTitle
.global SingleScreenMenuTitle_patch
SingleScreenMenuTitle_patch:
    bl hook_SingleScreenMenuTitle

.endif

.section .patch_SingleScreenSecondaryCallback
.global SingleScreenSecondaryCallback_patch
SingleScreenSecondaryCallback_patch:
    bl hook_SingleScreenSecondaryCallback
.endif

.section .patch_loader
.global loader_patch
loader_patch:
    b hook_into_loader

.if _DISABLE_CAMERA_PATCH_==0
    .section .patch_CameraUpdate
    .global CameraUpdate_patch
CameraUpdate_patch:
    bl hook_CameraUpdate
.endif

.if _DISABLE_NATIVE_HUD_PATCHES_==0
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
.endif

// The message font normally renders into a 256x256 atlas made from 16x16
// cells. The HD QBF uses 32x32 cells, so give the atlas 512x512 dimensions
// while retaining the original 256-entry draw-command capacity.
.if (_USA_==1) || (_EUR_==1) || (_JP_==1)
.section .patch_MessageFontAtlasDimensions
.global MessageFontAtlasDimensions_patch
MessageFontAtlasDimensions_patch:
    mov r3, #0x200

.section .patch_MessageFontCommandCapacity
.global MessageFontCommandCapacity_patch
MessageFontCommandCapacity_patch:
    mov r2, #0x100

// Keep the valid 32x32 source cells while drawing a 14x14 quad. The hook
// computes 7/16 of each 32-pixel vertex offset; UV calculations still consume
// the complete source cell.
.section .patch_MessageFontQuadWidth
.global MessageFontQuadWidth_patch
MessageFontQuadWidth_patch:
    b MessageFontQuadWidth_hook

.section .text.MessageFontQuadWidth, "ax", %progbits
.align 2
.global MessageFontQuadWidth_hook
MessageFontQuadWidth_hook:
 .if _USA_==1
    // Preserve native small date glyphs; only resize replacement 32px cells.
    push {r1, r2}
    mrs r2, cpsr
    ldr r1, [sp, #24]
    cmp r1, #32
    rsbeq ip, ip, ip, lsl #3
    addeq ip, sl, ip, asr #4
    addne ip, ip, sl
    msr cpsr_f, r2
    pop {r1, r2}
.else
    rsb ip, ip, ip, lsl #3
    add ip, sl, ip, asr #4
.endif
.if _JP_==1
    ldr pc, =0x002B6DD0
.else
    ldr pc, =0x002B72B8
.endif

// Lower the complete glyph quad by one native pixel. This adjusts geometry,
// rather than moving pixels within the QBF cell, so the glyph is not clipped.
.section .patch_MessageFontQuadBaseline
.global MessageFontQuadBaseline_patch
MessageFontQuadBaseline_patch:
    b MessageFontQuadBaseline_hook

.section .text.MessageFontQuadBaseline, "ax", %progbits
.align 2
.global MessageFontQuadBaseline_hook
MessageFontQuadBaseline_hook:
    ldr r9, [sp, #52]
.if _USA_==1
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
.else
    add r9, r9, #1
    rsb ip, ip, ip, lsl #3
    add ip, r9, ip, asr #4
.endif
.if _JP_==1
    ldr pc, =0x002B6DFC
.else
    ldr pc, =0x002B72E4
.endif

// QBF cell height is 32 for the HD atlas, but message layout should retain
// the original 16-pixel line advance. Only redirect the calls that advance
// the text cursor; atlas construction continues to receive the full height.
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
.if _JP_==1
    ldr ip, =0x002DA2E0
.else
    ldr ip, =0x002DA7C8
.endif
    blx ip
    mov r0, r0, asr #1
    pop {pc}
.endif
