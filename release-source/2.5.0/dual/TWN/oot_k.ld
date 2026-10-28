OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(loader_patch)
SECTIONS
{
    .patch_FilterTouch 0x120694 : { *(.patch_FilterTouch) }
    .patch_DialogueTextScale 0x31FDFC : { *(.patch_DialogueTextScale) }
    .patch_MenuOptionsTextLookup 0x302268 : { *(.patch_MenuOptionsTextLookup) }
    .patch_MenuOptionsTextLookupFallback 0x30229C : { *(.patch_MenuOptionsTextLookupFallback) }
    .patch_UnifiedMenuRepeat 0x313B64 : { *(.patch_UnifiedMenuRepeat) }
    .patch_UnifiedMenuButtons 0x3169B0 : { *(.patch_UnifiedMenuButtons) }
    .patch_MovementSpeed 0x3C2D2C : { *(.patch_MovementSpeed) }
    .patch_MovementAnimation 0x2BAFB8 : { *(.patch_MovementAnimation) }
    .patch_RollRecovery 0x17C22C : { *(.patch_RollRecovery) }
	.patch_loader 0x100000 : { *(.patch_loader) }
	.patch_CameraUpdate 0x2ED1C8 : { *(.patch_CameraUpdate) }
	.patch_NativeActionHudSync 0x108080 : { *(.patch_NativeActionHudSync) }
	.patch_before_GlobalContext_Update 0x13A1F0 : { *(.patch_before_GlobalContext_Update) }
	.patch_after_GlobalContext_Update 0x13A248 : { *(.patch_after_GlobalContext_Update) }
	.patch_SelectItemsMenu 0x1188FC : { *(.patch_SelectItemsMenu) }
	.patch_NativeHudTextureSlot 0x150B30 : { *(.patch_NativeHudTextureSlot) }
	.patch_NativeHudIgnoreMotionDisable 0x2F80DC : { *(.patch_NativeHudIgnoreMotionDisable) }
	.patch_NativeHudIgnoreMotionSettingGate1 0x115794 : { *(.patch_NativeHudIgnoreMotionSettingGate1) }
	.patch_NativeHudIgnoreMotionSettingGate2 0x115930 : { *(.patch_NativeHudIgnoreMotionSettingGate2) }
	.patch_NativeHudKeepPositionUVPointers 0x150AF0 : { *(.patch_NativeHudKeepPositionUVPointers) }
	.patch_NativeHudKeepIndexPointer 0x150B00 : { *(.patch_NativeHudKeepIndexPointer) }
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
