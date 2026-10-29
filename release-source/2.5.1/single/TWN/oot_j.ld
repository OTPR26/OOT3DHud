OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(loader_patch)
SECTIONS
{
	. = 0x100000;
	.patch_JapanTop 0x300074 : { *(.patch_JapanTop) }
	.patch_JapanCallback 0x419848 : { *(.patch_JapanCallback) }
	.patch_JapanFileBefore 0x2FFF1C : { *(.patch_JapanFileBefore) }
	.patch_JapanFileAfter 0x2FFF80 : { *(.patch_JapanFileAfter) }
	.patch_JapanFileText 0x300030 : { *(.patch_JapanFileText) }
	.patch_JapanFileBackdrop 0x2FFFE8 : { *(.patch_JapanFileBackdrop) }
	.patch_JapanFileHeading 0x4337C0 : { *(.patch_JapanFileHeading) }
	.patch_JapanPresentation 0x2FFFF4 : { *(.patch_JapanPresentation) }
	.patch_JapanOverlay 0x300038 : { *(.patch_JapanOverlay) }
	.patch_JapanTitle 0x42BE64 : { *(.patch_JapanTitle) }
	.patch_JapanLeather 0x41EE04 : { *(.patch_JapanLeather) }
	.patch_JapanItems 0x2F854C : { *(.patch_JapanItems) }
	.patch_JapanGear 0x2EFF5C : { *(.patch_JapanGear) }
	.patch_JapanMap 0x2F8220 : { *(.patch_JapanMap) }
	.patch_loader 0x100000 : {
		*(.patch_loader)
	}

	.patch_CameraUpdate 0x2D7FE0 : {
	    * (.patch_CameraUpdate)
	}

	.patch_NativeActionHudSync 0x41E228 : {
		*(.patch_NativeActionHudSync)
	}

	.patch_before_GlobalContext_Update 0x452368 : {
		*(.patch_before_GlobalContext_Update)
	}

	.patch_after_GlobalContext_Update 0x45240C : {
		*(.patch_after_GlobalContext_Update)
	}

	.patch_SelectItemsMenu 0x42EBD8 : {
		*(.patch_SelectItemsMenu)
	}

	.patch_SyncNativeMenu 0x42F248 : { *(.patch_SyncNativeMenu) }

	.patch_NativeHudTextureSlot 0x469600 : {
		*(.patch_NativeHudTextureSlot)
	}

	.patch_NativeHudIgnoreMotionDisable 0x2DE878 : {
		*(.patch_NativeHudIgnoreMotionDisable)
	}

	.patch_NativeHudIgnoreMotionSettingGate1 0x42BC80 : {
		*(.patch_NativeHudIgnoreMotionSettingGate1)
	}

	.patch_NativeHudIgnoreMotionSettingGate2 0x42BE20 : {
		*(.patch_NativeHudIgnoreMotionSettingGate2)
	}

	.patch_NativeHudKeepPositionUVPointers 0x4695C8 : {
		*(.patch_NativeHudKeepPositionUVPointers)
	}

	.patch_NativeHudKeepIndexPointer 0x4695D0 : {
		*(.patch_NativeHudKeepIndexPointer)
	}

	.patch_NativeHudDescriptor 0x4D30A4 : {
		*(.patch_NativeHudDescriptor)
	}

	. = 0x4C99A8;
	. = ALIGN(4);
	.loader : {
		*(.loader*)
	}

	. = 0x005C7000;
	.text : {
        __text_start = . ;
        *(.text)
        *(.text.*)
        __text_end  = . ;
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
