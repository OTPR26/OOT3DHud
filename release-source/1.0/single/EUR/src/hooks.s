.arm
.text

.global hook_before_GlobalContext_Update
hook_before_GlobalContext_Update:
    push {r0-r12, lr}
    bl before_GlobalContext_Update
    pop {r0-r12, lr}
.if (_TWN_==1) || (_KOR_==1)
    cpy r8,r0
.else
    cpy r7,r0
.endif
    bx lr

.global hook_after_GlobalContext_Update
hook_after_GlobalContext_Update:
    push {r0-r12, lr}
    bl after_GlobalContext_Update
    pop {r0-r12, lr}
.if _TWN_==1
    b 0x2FC1A0
.elseif _KOR_==1
    b 0x2FC0A0
.elseif _JP_==1
    b 0x2E2108
.else
# both USA and EUR
    b 0x2E25F0
.endif

.section .loader
.global hook_into_loader
hook_into_loader:
    push {r0-r12, lr}
    bl loader_main
    pop {r0-r12, lr}
.if (_TWN_==1) || (_KOR_==1)
    bl 0x100024
.else
    bl 0x100028
.endif
    b  0x100004

.global hook_CameraUpdate
hook_CameraUpdate:
    push {r0-r12, lr}
    cpy r0,r1
    bl Camera_FreeCamEnabled
    cmp r0,#0x0
    pop {r0-r12, lr}
.if (_TWN_==1) || (_KOR_==1)
    cpyeq r4,r1
    bxeq lr
    bl Camera_FreeCamUpdate
    ldmia sp!,{r0,r1,r4-r11,pc}
.else
    cpyeq r6,r0
    bxeq lr
    bl Camera_FreeCamUpdate
    ldmia sp!,{r4-r11,pc}
.endif

.global hook_NativeActionHudSync
hook_NativeActionHudSync:
    push {r0-r12, lr}
    bl NativeHud_PrepareActionPrompt
    ldmia sp, {r0-r3}
.if _TWN_==1
    bl 0x115438
.elseif _KOR_==1
    bl 0x115360
.elseif _JP_==1
    bl 0x42B820
.elseif _EUR_==1
    bl 0x42B86C
.else
    bl 0x42B848
.endif
    bl NativeHud_RestoreActionPrompt
    pop {r0-r12, lr}
    bx lr

.global hook_SelectItemsMenu
hook_SelectItemsMenu:
    push {r0-r12, lr}
.if (_TWN_==1) || (_KOR_==1)
    cpy r0,r10
.else
    cpy r0,r5
.endif
    bl InputRemap_TryOpenItemsMenu
    cmp r0,#0x1
    pop {r0-r12, lr}
    // The replaced instruction is a NOP. If Select was handled, return at the
    // existing BEQ that skips the vanilla save-menu activation.
    addeq lr,lr,#0xC
    bx lr

.global hook_ItemsPageActivated
.if (_USA_==1) || ((_EUR_==1) && (_EUR_SINGLE_SCREEN_PROBE_==1))
.global hook_SyncNativeMenu
hook_SyncNativeMenu:
    push {r0-r12, lr}
    mrs r4, cpsr
    mov r0, r5
    bl InputRemap_SyncNativeMenu
    msr cpsr_f, r4
    pop {r0-r12, lr}
    bx lr
.endif

hook_ItemsPageActivated:
    push {r0-r12, lr}
    mov r0, #1
    bl InputRemap_OnPausePageActivated
    pop {r0-r12, lr}
    push {r4, r5, r6, lr}
    b 0x002F8A38

.global hook_GearPageActivated
hook_GearPageActivated:
    push {r0-r12, lr}
    mov r0, #2
    bl InputRemap_OnPausePageActivated
    pop {r0-r12, lr}
    push {r4-r11, lr}
    b 0x002F0448

.global hook_MapPageActivated
hook_MapPageActivated:
    push {r0-r12, lr}
    mov r0, #3
    bl InputRemap_OnPausePageActivated
    pop {r0-r12, lr}
    push {r4, r5, r6, lr}
    b 0x002F870C

.if (_JP_==1) && (_JP_SINGLE_SCREEN_PROBE_==1)
.global hook_JapanFileBefore
hook_JapanFileBefore:
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_BeforeFileScene
    vpop {d0-d7}
    pop {r0-r12, lr}
    b 0x00422460
.global hook_JapanFileBackdrop
hook_JapanFileBackdrop:
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_BeforeFileBackdrop
    vpop {d0-d7}
    pop {r0-r12, lr}
    b 0x00422890
.global hook_JapanFileAfter
hook_JapanFileAfter:
    push {r4, lr}
    bl 0x002FA834
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_AfterFileScene
    vpop {d0-d7}
    pop {r0-r12, lr}
    pop {r4, pc}
.macro japan_page_hook name, page, target, gear=0
.global \name
\name:
    push {r0-r12, lr}
    vpush {d0-d7}
    mov r0, #\page
    bl InputRemap_OnPausePageActivated
    vpop {d0-d7}
    pop {r0-r12, lr}
.if \gear
    push {r4-r11, lr}
.else
    push {r4-r6, lr}
.endif
    b \target
.endm
japan_page_hook hook_JapanItems, 1, 0x002F8550
japan_page_hook hook_JapanGear, 2, 0x002EFF60, 1
japan_page_hook hook_JapanMap, 3, 0x002F8224
.global hook_JapanPresentation
hook_JapanPresentation:
    push {r0-r12, lr}
    vpush {d0-d7}
    bl JapanProbe_Position
    vpop {d0-d7}
    pop {r0-r12, lr}
    b 0x0042B9A8
.endif
.if (_TWN_==1) && (_TWN_SINGLE_SCREEN_PROBE_==1)
.global hook_TaiwanFileBefore
hook_TaiwanFileBefore:
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_BeforeFileScene
    vpop {d0-d7}
    pop {r0-r12, lr}
    b 0x0010C314
.global hook_TaiwanFileBackdrop
hook_TaiwanFileBackdrop:
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_BeforeFileBackdrop
    vpop {d0-d7}
    pop {r0-r12, lr}
    b 0x0010C74C
.global hook_TaiwanFileAfter
hook_TaiwanFileAfter:
    push {r4, lr}
    bl 0x003194B4
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_AfterFileScene
    vpop {d0-d7}
    pop {r0-r12, lr}
    pop {r4, pc}
.macro taiwan_page_hook name, page, target, gear=0
.global \name
\name:
    push {r0-r12, lr}
    vpush {d0-d7}
    mov r0, #\page
    bl InputRemap_OnPausePageActivated
    vpop {d0-d7}
    pop {r0-r12, lr}
.if \gear
    push {r4-r11, lr}
.else
    push {r4-r6, lr}
.endif
    b \target
.endm
taiwan_page_hook hook_TaiwanProbeItems, 1, 0x00315F70
taiwan_page_hook hook_TaiwanProbeGear, 2, 0x0030D5E4, 1
taiwan_page_hook hook_TaiwanProbeMap, 3, 0x00315C30

.global hook_TaiwanProbePresentation
hook_TaiwanProbePresentation:
    push {r0-r12, lr}
    vpush {d0-d7}
    bl TaiwanProbe_PositionPresentation
    vpop {d0-d7}
    pop {r0-r12, lr}
    b 0x00115588
.endif

.global hook_SingleScreenSecondaryCallback
hook_SingleScreenSecondaryCallback:
    b SingleScreen_DrawSecondaryCallback

.if (_USA_==1) || ((_EUR_==1) && (_EUR_SINGLE_SCREEN_PROBE_==1))
.global hook_FileSelectBeforeScene
hook_FileSelectBeforeScene:
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_BeforeFileScene
    vpop {d0-d7}
    pop {r0-r12, lr}
 .if _EUR_==1
    b 0x004224AC
 .else
    b 0x00422488
 .endif

.global hook_FileSelectText
hook_FileSelectText:
    b SingleScreen_DrawFileText

.global hook_FileSelectLeather
hook_FileSelectLeather:
    b SingleScreen_DrawSecondaryBackdrop

.global hook_FileSelectHeading
hook_FileSelectHeading:
    b SingleScreen_UpdateFileBoard

.global hook_FileSelectBackdrop
hook_FileSelectBackdrop:
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_BeforeFileBackdrop
    vpop {d0-d7}
    pop {r0-r12, lr}
 .if _EUR_==1
    b 0x004228DC
 .else
    b 0x004228B8
 .endif

.global hook_FileSelectAfterScene
hook_FileSelectAfterScene:
    push {r4, lr}
    bl 0x002FAD1C
    push {r0-r12, lr}
    vpush {d0-d7}
    bl SingleScreen_AfterFileScene
    vpop {d0-d7}
    pop {r0-r12, lr}
    pop {r4, pc}

.endif

.global hook_SingleScreenBeforeTopOverlay
hook_SingleScreenBeforeTopOverlay:
    // Preserve the original call at 0x300520, then insert the viewport change
    // at the boundary immediately before the final top-screen overlay group.
    push {r0-r12, lr}
.if (_USA_==1) || ((_EUR_==1) && (_EUR_SINGLE_SCREEN_PROBE_==1))
    vpush {d0-d7}
    bl SingleScreen_DrawFileLogoPass
    vpop {d0-d7}
    pop {r0-r12, lr}
    push {r0-r12, lr}
.else
    bl 0x004228C0
.endif
    bl SingleScreen_BeforeTopOverlay
    pop {r0-r12, lr}
    bx lr

.global hook_SingleScreenBeforeTopPresentation
hook_SingleScreenBeforeTopPresentation:
    // Move the native top-HUD group before entering its original renderer.
    push {r0-r12, lr}
    bl SingleScreen_BeforeTopPresentation
    pop {r0-r12, lr}
 .if _EUR_==1
    b 0x0042B9F4
 .else
    b 0x0042B9D0
 .endif

.if (_USA_==1) || ((_EUR_==1) && (_EUR_SINGLE_SCREEN_PROBE_==1))
.global hook_SingleScreenMenuTitle
hook_SingleScreenMenuTitle:
    // The title is one board inside the moved HUD group. Draw it at the full
    // viewport, then draw the repurposed motion-control render node in this
    // same final pass. Calling the node here keeps both boards out of the
    // compact presentation viewport and places the controls above the menu.
    push {r0-r12, lr}
    bl SingleScreen_BeforeMenuTitle
    pop {r0-r12, lr}
    push {r4-r12, lr}
    blx r1
    bl SingleScreen_AfterMenuTitleBoard
    ldr r0, =0x004FC6C0
    ldr r0, [r0]
    cmp r0, #0
    beq 1f
    ldr r1, [r0]
    ldr r1, [r1, #12]
    blx r1
1:
    bl SingleScreen_AfterMenuTitle
    pop {r4-r12, pc}

.endif

.global hook_ItemAssignXDestination
hook_ItemAssignXDestination:
    ldr r0, =gItemAssignmentDestination
    ldrb r0, [r0]
    cmp r0, #5
    movne r0, #0xB
    bx lr

.global hook_ItemAssignYDestination
hook_ItemAssignYDestination:
    ldr r0, =gItemAssignmentDestination
    ldrb r0, [r0]
    cmp r0, #0x17
    movne r0, #0x11
    bx lr

.global hook_ItemAssignXPositionR0
hook_ItemAssignXPositionR0:
    cmp r0, #5
    moveq r0, #0x100
    addeq r0, r0, #0xF
    addne r0, r0, #0x108
    bx lr

.global hook_ItemAssignIPositionR0
hook_ItemAssignIPositionR0:
    ldr r0, =gItemAssignmentDestination
    ldrb r0, [r0]
    cmp r0, #5
    moveq r0, #7
    movne r0, #0x49
    bx lr

.global hook_ItemAssignXPositionR1
hook_ItemAssignXPositionR1:
    cmp r0, #5
    moveq r1, #0x100
    addeq r1, r1, #0xF
    addne r1, r0, #0x108
    bx lr

.global hook_ItemAssignIPositionR2
hook_ItemAssignIPositionR2:
    ldr r2, =gItemAssignmentDestination
    ldrb r2, [r2]
    cmp r2, #5
    moveq r2, #7
    movne r2, #0x49
    bx lr

.global hook_ItemAssignYPositionR0
hook_ItemAssignYPositionR0:
    cmp r0, #0x17
    moveq r0, #0x100
    addeq r0, r0, #0xF
    addne r0, r0, #0xF6
    bx lr

.global hook_ItemAssignIIPositionR0
hook_ItemAssignIIPositionR0:
    ldr r0, =gItemAssignmentDestination
    ldrb r0, [r0]
    cmp r0, #0x17
    moveq r0, #0xBF
    movne r0, #0x7D
    bx lr

.global hook_ItemAssignYPositionR1
hook_ItemAssignYPositionR1:
    cmp r0, #0x17
    moveq r1, #0x100
    addeq r1, r1, #0xF
    addne r1, r0, #0xF6
    bx lr

.global hook_ItemAssignIIPositionR2
hook_ItemAssignIIPositionR2:
    ldr r2, =gItemAssignmentDestination
    ldrb r2, [r2]
    cmp r2, #0x17
    moveq r2, #0xBF
    movne r2, #0x7D
    bx lr
