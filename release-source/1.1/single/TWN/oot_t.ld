OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(loader_patch)
SECTIONS
{
	.patch_loader 0x100000 : { *(.patch_loader) }
	.patch_TaiwanFileBefore 0x31FF40 : { *(.patch_TaiwanFileBefore) }
	.patch_TaiwanFileAfter 0x31FE10 : { *(.patch_TaiwanFileAfter) }
	.patch_TaiwanFileText 0x31FEC0 : { *(.patch_TaiwanFileText) }
	.patch_TaiwanFileBackdrop 0x31FE78 : { *(.patch_TaiwanFileBackdrop) }
	.patch_TaiwanFileHeading 0x11D598 : { *(.patch_TaiwanFileHeading) }
	.patch_TaiwanProbeTopPass 0x31FF08 : { *(.patch_TaiwanProbeTopPass) }
	.patch_TaiwanProbePresentation 0x31FE84 : { *(.patch_TaiwanProbePresentation) }
	.patch_TaiwanProbeOverlay 0x31FEC8 : { *(.patch_TaiwanProbeOverlay) }
	.patch_TaiwanProbeTitle 0x115A50 : { *(.patch_TaiwanProbeTitle) }
	.patch_TaiwanProbeLeather 0x108B78 : { *(.patch_TaiwanProbeLeather) }
	.patch_TaiwanProbeItems 0x315F6C : { *(.patch_TaiwanProbeItems) }
	.patch_TaiwanProbeGear 0x30D5E0 : { *(.patch_TaiwanProbeGear) }
	.patch_TaiwanProbeMap 0x315C2C : { *(.patch_TaiwanProbeMap) }
	.patch_TaiwanProbeCallback 0x1038E8 : { *(.patch_TaiwanProbeCallback) }
	.patch_CameraUpdate 0x2ED2C8 : { *(.patch_CameraUpdate) }
	.patch_NativeActionHudSync 0x108158 : { *(.patch_NativeActionHudSync) }
	.patch_before_GlobalContext_Update 0x13A2C8 : { *(.patch_before_GlobalContext_Update) }
	.patch_after_GlobalContext_Update 0x13A320 : { *(.patch_after_GlobalContext_Update) }
	.patch_SelectItemsMenu 0x1189D4 : { *(.patch_SelectItemsMenu) }
	.patch_SyncNativeMenu 0x119070 : { *(.patch_SyncNativeMenu) }
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
