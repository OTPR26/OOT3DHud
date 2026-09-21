OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(loader_patch)
SECTIONS
{
	. = 0x100000;
 .patch_SyncNativeMenu 0x42F294 : { *(.patch_SyncNativeMenu) }
 .patch_ItemsPageActivated 0x2F8A34 : { *(.patch_ItemsPageActivated) }
 .patch_GearPageActivated 0x2F0444 : { *(.patch_GearPageActivated) }
 .patch_MapPageActivated 0x2F8708 : { *(.patch_MapPageActivated) }
 .patch_SingleScreenAfterTopPass 0x30055C : { *(.patch_SingleScreenAfterTopPass) }
 .patch_SingleScreenBeforeTopOverlay 0x300520 : { *(.patch_SingleScreenBeforeTopOverlay) }
 .patch_SingleScreenBeforeTopPresentation 0x3004DC : { *(.patch_SingleScreenBeforeTopPresentation) }
 .patch_FileSelectBeforeScene 0x300404 : { *(.patch_FileSelectBeforeScene) }
 .patch_FileSelectAfterScene 0x300468 : { *(.patch_FileSelectAfterScene) }
 .patch_FileSelectText 0x300518 : { *(.patch_FileSelectText) }
 .patch_FileSelectBackdrop 0x3004D0 : { *(.patch_FileSelectBackdrop) }
 .patch_FileSelectLeather 0x41EE50 : { *(.patch_FileSelectLeather) }
 .patch_FileSelectHeading 0x43380C : { *(.patch_FileSelectHeading) }
 .patch_SingleScreenMenuTitle 0x42BEB0 : { *(.patch_SingleScreenMenuTitle) }
 .patch_SingleScreenSecondaryCallback 0x419848 : { *(.patch_SingleScreenSecondaryCallback) }
	.patch_loader 0x100000 : {
		*(.patch_loader)
	}

	.patch_CameraUpdate 0x2D84C8 : {
	    * (.patch_CameraUpdate)
	}

	.patch_NativeActionHudSync 0x41E274 : {
		*(.patch_NativeActionHudSync)
	}

	.patch_before_GlobalContext_Update 0x4523B0 : {
		*(.patch_before_GlobalContext_Update)
	}

	.patch_after_GlobalContext_Update 0x452454 : {
		*(.patch_after_GlobalContext_Update)
	}

	.patch_SelectItemsMenu 0x42EC24 : {
		*(.patch_SelectItemsMenu)
	}

	.patch_NativeHudTextureSlot 0x469648 : {
		*(.patch_NativeHudTextureSlot)
	}

	.patch_NativeHudIgnoreMotionDisable 0x2DED60 : {
		*(.patch_NativeHudIgnoreMotionDisable)
	}

	.patch_NativeHudIgnoreMotionSettingGate1 0x42BCCC : {
		*(.patch_NativeHudIgnoreMotionSettingGate1)
	}

	.patch_NativeHudIgnoreMotionSettingGate2 0x42BE6C : {
		*(.patch_NativeHudIgnoreMotionSettingGate2)
	}

	.patch_NativeHudKeepPositionUVPointers 0x469610 : {
		*(.patch_NativeHudKeepPositionUVPointers)
	}

	.patch_NativeHudKeepIndexPointer 0x469618 : {
		*(.patch_NativeHudKeepIndexPointer)
	}

	.patch_NativeHudDescriptor 0x4D30A4 : {
		*(.patch_NativeHudDescriptor)
	}

	.patch_MessageFontAtlasDimensions 0x44CF08 : {
		*(.patch_MessageFontAtlasDimensions)
	}

	.patch_MessageFontCommandCapacity 0x44CF14 : {
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

	.patch_MessageFontMeasureBounds0 0x48C3BC : {
		*(.patch_MessageFontMeasureBounds0)
	}

	.patch_MessageFontMeasureBounds1 0x48C3F0 : {
		*(.patch_MessageFontMeasureBounds1)
	}

	.patch_MessageFontMeasureBounds2 0x48C408 : {
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
