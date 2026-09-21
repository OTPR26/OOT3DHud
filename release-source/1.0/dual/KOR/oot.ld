OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(loader_patch)
SECTIONS
{
	. = 0x100000;
	.patch_loader 0x100000 : {
		*(.patch_loader)
	}

	.patch_CameraUpdate 0x2D84C8 : {
	    * (.patch_CameraUpdate)
	}

	.patch_NativeActionHudSync 0x41E250 : {
		*(.patch_NativeActionHudSync)
	}

	.patch_before_GlobalContext_Update 0x452390 : {
		*(.patch_before_GlobalContext_Update)
	}

	.patch_after_GlobalContext_Update 0x452434 : {
		*(.patch_after_GlobalContext_Update)
	}

	.patch_SelectItemsMenu 0x42EC00 : {
		*(.patch_SelectItemsMenu)
	}

	.patch_NativeHudTextureSlot 0x469628 : {
		*(.patch_NativeHudTextureSlot)
	}

	.patch_NativeHudIgnoreMotionDisable 0x2DED60 : {
		*(.patch_NativeHudIgnoreMotionDisable)
	}

	.patch_NativeHudIgnoreMotionSettingGate1 0x42BCA8 : {
		*(.patch_NativeHudIgnoreMotionSettingGate1)
	}

	.patch_NativeHudIgnoreMotionSettingGate2 0x42BE48 : {
		*(.patch_NativeHudIgnoreMotionSettingGate2)
	}

	.patch_NativeHudKeepPositionUVPointers 0x4695F0 : {
		*(.patch_NativeHudKeepPositionUVPointers)
	}

	.patch_NativeHudKeepIndexPointer 0x4695F8 : {
		*(.patch_NativeHudKeepIndexPointer)
	}

	.patch_NativeHudDescriptor 0x4D30A4 : {
		*(.patch_NativeHudDescriptor)
	}

	.patch_MessageFontAtlasDimensions 0x44CEE8 : {
		*(.patch_MessageFontAtlasDimensions)
	}

	.patch_MessageFontCommandCapacity 0x44CEF4 : {
		*(.patch_MessageFontCommandCapacity)
	}

	.patch_MessageFontQuadWidth 0x2B72B4 : {
		*(.patch_MessageFontQuadWidth)
	}

	.patch_MessageFontQuadBaseline 0x2B72DC : {
		*(.patch_MessageFontQuadBaseline)
	}

	.patch_MessageFontLineAdvancePrimary 0x4C0D90 : {
		*(.patch_MessageFontLineAdvancePrimary)
	}

	.patch_MessageFontLineAdvanceSecondary 0x4C0DB0 : {
		*(.patch_MessageFontLineAdvanceSecondary)
	}

	.patch_MessageFontMeasureSecondary 0x2BAFE8 : {
		*(.patch_MessageFontMeasureSecondary)
	}

	.patch_MessageFontMeasureBlock0 0x2C2814 : {
		*(.patch_MessageFontMeasureBlock0)
	}

	.patch_MessageFontMeasureBlock1 0x2C282C : {
		*(.patch_MessageFontMeasureBlock1)
	}

	.patch_MessageFontMeasureBlock2 0x2C2890 : {
		*(.patch_MessageFontMeasureBlock2)
	}

	.patch_MessageFontMeasureBlock3 0x2C28F8 : {
		*(.patch_MessageFontMeasureBlock3)
	}

	.patch_MessageFontMeasureBounds0 0x48C39C : {
		*(.patch_MessageFontMeasureBounds0)
	}

	.patch_MessageFontMeasureBounds1 0x48C3D0 : {
		*(.patch_MessageFontMeasureBounds1)
	}

	.patch_MessageFontMeasureBounds2 0x48C3E8 : {
		*(.patch_MessageFontMeasureBounds2)
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
		*(.data)
		*(.bss)
		*(COMMON)
		*(.rodata)
		*(.rodata.*)
	}

	/DISCARD/ : {
		*(.eh_frame*)
		*(.ARM.exidx*)
		*(.ARM.extab*)
	}
}
