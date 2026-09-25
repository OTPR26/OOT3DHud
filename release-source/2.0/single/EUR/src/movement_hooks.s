.arm
.text
.if (_USA_==1) || (_EUR_==1)
.if UNIFIED_MENU_PROTOTYPE==1
/* Preserve all native registers and both flag sets around soft-float C.
 * Only the displaced instruction's destination FP register changes. */
.macro movement_begin
    push {r0-r12, lr}
    mrs r11, cpsr
    vmrs r12, fpscr
    push {r11,r12}
    vpush {d0-d15}
.endm
.macro movement_end
    vpop {d0-d15}
    pop {r11,r12}
    vmsr fpscr,r12
    msr CPSR_f,r11
    pop {r0-r12,pc}
.endm
.global hook_MovementSpeed
hook_MovementSpeed:
    movement_begin
    sub r0,r6,#0x2000
    bl MovementOptions_Multiplier
    vmov s1,r0
    vldr s0,[r6,#0x21c]
    vmul.f32 s0,s0,s1
    vstr s0,[sp]
    movement_end
.global hook_MovementAnimation
hook_MovementAnimation:
    movement_begin
    sub r0,r5,#0x2000
    bl MovementOptions_Multiplier
    vmov s1,r0
    vldr s0,[r5,#0x21c]
    vmul.f32 s0,s0,s1
    vstr s0,[sp,#68]
    movement_end
.global hook_RollRecovery
hook_RollRecovery:
    movement_begin
    mov r0,r4
    bl MovementOptions_RecoveryEnd
    str r0,[sp]
    movement_end
.endif
.endif
