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

	.patch_ItemsPageActivated 0x2F8A34 : {
		*(.patch_ItemsPageActivated)
	}

	.patch_SyncNativeMenu 0x42F270 : {
		*(.patch_SyncNativeMenu)
	}

	.patch_GearPageActivated 0x2F0444 : {
		*(.patch_GearPageActivated)
	}

	.patch_MapPageActivated 0x2F8708 : {
		*(.patch_MapPageActivated)
	}

	.patch_ItemAssignXDestination1 0x2EC58C : {
		*(.patch_ItemAssignXDestination1)
	}

	.patch_ItemAssignXPosition1 0x2EC598 : {
		*(.patch_ItemAssignXPosition1)
	}

	.patch_ItemAssignIPosition1 0x2EC5A4 : {
		*(.patch_ItemAssignIPosition1)
	}

	.patch_ItemAssignYDestination1 0x2EC7D4 : {
		*(.patch_ItemAssignYDestination1)
	}

	.patch_ItemAssignYPosition1 0x2EC7E0 : {
		*(.patch_ItemAssignYPosition1)
	}

	.patch_ItemAssignIIPosition1 0x2EC7EC : {
		*(.patch_ItemAssignIIPosition1)
	}

	.patch_ItemAssignXDestination2 0x2ECDE8 : {
		*(.patch_ItemAssignXDestination2)
	}

	.patch_ItemAssignXPosition2 0x2ECDF4 : {
		*(.patch_ItemAssignXPosition2)
	}

	.patch_ItemAssignIPosition2 0x2ECDFC : {
		*(.patch_ItemAssignIPosition2)
	}

	.patch_ItemAssignYDestination2 0x2ECF5C : {
		*(.patch_ItemAssignYDestination2)
	}

	.patch_ItemAssignYPosition2 0x2ECF68 : {
		*(.patch_ItemAssignYPosition2)
	}

	.patch_ItemAssignIIPosition2 0x2ECF70 : {
		*(.patch_ItemAssignIIPosition2)
	}

	.patch_SingleScreenAfterTopPass 0x30055C : {
		*(.patch_SingleScreenAfterTopPass)
	}

	.patch_SingleScreenBeforeTopOverlay 0x300520 : {
		*(.patch_SingleScreenBeforeTopOverlay)
	}

	.patch_SingleScreenBeforeTopPresentation 0x3004DC : {
		*(.patch_SingleScreenBeforeTopPresentation)
	}

	.patch_FileSelectBeforeScene 0x300404 : {
		*(.patch_FileSelectBeforeScene)
	}
	.patch_FileSelectAfterScene 0x300468 : {
		*(.patch_FileSelectAfterScene)
	}
	.patch_FileSelectText 0x300518 : {
		*(.patch_FileSelectText)
	}
	.patch_FileSelectBackdrop 0x3004D0 : {
		*(.patch_FileSelectBackdrop)
	}
	.patch_FileSelectLeather 0x41EE2C : {
		*(.patch_FileSelectLeather)
	}
	.patch_FileSelectHeading 0x4337E8 : {
		*(.patch_FileSelectHeading)
	}

	.patch_SingleScreenMenuTitle 0x42BE8C : {
		*(.patch_SingleScreenMenuTitle)
	}

	.patch_SingleScreenSecondaryCallback 0x419848 : {
		*(.patch_SingleScreenSecondaryCallback)
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
