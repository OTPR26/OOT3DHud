OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(loader_patch)
SECTIONS
{
	.patch_loader 0x100000 : { *(.patch_loader) }
	.patch_CameraUpdate 0x2ED2C8 : { *(.patch_CameraUpdate) }
	.patch_NativeActionHudSync 0x108158 : { *(.patch_NativeActionHudSync) }
	.patch_before_GlobalContext_Update 0x13A2C8 : { *(.patch_before_GlobalContext_Update) }
	.patch_after_GlobalContext_Update 0x13A320 : { *(.patch_after_GlobalContext_Update) }
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
