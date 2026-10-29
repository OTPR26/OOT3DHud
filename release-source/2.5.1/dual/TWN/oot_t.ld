OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(loader_patch)
SECTIONS
{
    .patch_FilterTouch 0x12076C : { *(.patch_FilterTouch) }
    .patch_DialogueTextScale 0x31FEFC : { *(.patch_DialogueTextScale) }
    .patch_MenuOptionsTextLookup 0x302368 : { *(.patch_MenuOptionsTextLookup) }
    .patch_MenuOptionsTextLookupFallback 0x30239C : { *(.patch_MenuOptionsTextLookupFallback) }
    .patch_UnifiedMenuRepeat 0x313C64 : { *(.patch_UnifiedMenuRepeat) }
    .patch_UnifiedMenuButtons 0x316AB0 : { *(.patch_UnifiedMenuButtons) }
    .patch_MovementSpeed 0x3C2E2C : { *(.patch_MovementSpeed) }
    .patch_MovementAnimation 0x2BB0B8 : { *(.patch_MovementAnimation) }
    .patch_RollRecovery 0x17C304 : { *(.patch_RollRecovery) }
    .patch_TaiwanDualInstrument 0x31FF08 : { *(.patch_TaiwanDualInstrument) }
	.patch_loader 0x100000 : { *(.patch_loader) }
	.patch_CameraUpdate 0x2ED2C8 : { *(.patch_CameraUpdate) }
	.patch_NativeActionHudSync 0x108158 : { *(.patch_NativeActionHudSync) }
	.patch_before_GlobalContext_Update 0x13A2C8 : { *(.patch_before_GlobalContext_Update) }
	.patch_after_GlobalContext_Update 0x13A320 : { *(.patch_after_GlobalContext_Update) }
	.patch_SelectItemsMenu 0x1189D4 : { *(.patch_SelectItemsMenu) }
	.patch_NativeHudTextureSlot 0x150C08 : { *(.patch_NativeHudTextureSlot) }
	.patch_NativeHudIgnoreMotionDisable 0x2F81DC : { *(.patch_NativeHudIgnoreMotionDisable) }
	.patch_NativeHudIgnoreMotionSettingGate1 0x11586C : { *(.patch_NativeHudIgnoreMotionSettingGate1) }
	.patch_NativeHudIgnoreMotionSettingGate2 0x115A08 : { *(.patch_NativeHudIgnoreMotionSettingGate2) }
	.patch_NativeHudKeepPositionUVPointers 0x150BC8 : { *(.patch_NativeHudKeepPositionUVPointers) }
	.patch_NativeHudKeepIndexPointer 0x150BD8 : { *(.patch_NativeHudKeepIndexPointer) }
	.patch_NativeHudDescriptor 0x4DB08C : { *(.patch_NativeHudDescriptor) }

	. = 0x4E93FC;
	. = ALIGN(4);
	.loader : { *(.loader*) }

	. = 0x005CE000;
	.text : {
		__text_start = .;
		*(.text)
		*(.text.*)
		__text_end = .;
		*(.rodata)
		*(.rodata.*)
		*(.data)
		*(.bss)
		*(COMMON)
	}

	/DISCARD/ : {
		*(.eh_frame*)
		*(.ARM.exidx*)
		*(.ARM.extab*)
	}
}
