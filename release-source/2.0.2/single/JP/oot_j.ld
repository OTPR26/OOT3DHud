OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(loader_patch)
SECTIONS
{
	. = 0x100000;
	.patch_MovementSpeed 0x251D34 : { *(.patch_MovementSpeed) }
	.patch_MovementAnimation 0x2BE224 : { *(.patch_MovementAnimation) }
	.patch_RollRecovery 0x492CEC : { *(.patch_RollRecovery) }
	.patch_MenuOptionsTextLookup 0x344290 : {
		*(.patch_MenuOptionsTextLookup)
	}
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

	.patch_ItemsPageActivated 0x2F854C : {
		*(.patch_ItemsPageActivated)
	}

	.patch_SyncNativeMenu 0x42F248 : {
		*(.patch_SyncNativeMenu)
	}

	.patch_UnifiedMenuRepeat 0x33B0E8 : {
		*(.patch_UnifiedMenuRepeat)
	}
	.patch_UnifiedMenuButtons 0x33B104 : {
		*(.patch_UnifiedMenuButtons)
	}

	.patch_UnifiedMenuCursor 0x2FB44C : {
		*(.patch_UnifiedMenuCursor)
	}

	.patch_GearPageActivated 0x2EFF5C : {
		*(.patch_GearPageActivated)
	}

	.patch_MapPageActivated 0x2F8220 : {
		*(.patch_MapPageActivated)
	}

	.patch_ItemAssignXDestination1 0x2EC0A4 : {
		*(.patch_ItemAssignXDestination1)
	}

	.patch_ItemAssignXPosition1 0x2EC0B0 : {
		*(.patch_ItemAssignXPosition1)
	}

	.patch_ItemAssignIPosition1 0x2EC0BC : {
		*(.patch_ItemAssignIPosition1)
	}

	.patch_ItemAssignYDestination1 0x2EC2EC : {
		*(.patch_ItemAssignYDestination1)
	}

	.patch_ItemAssignYPosition1 0x2EC2F8 : {
		*(.patch_ItemAssignYPosition1)
	}

	.patch_ItemAssignIIPosition1 0x2EC304 : {
		*(.patch_ItemAssignIIPosition1)
	}

	.patch_ItemAssignXDestination2 0x2EC900 : {
		*(.patch_ItemAssignXDestination2)
	}

	.patch_ItemAssignXPosition2 0x2EC90C : {
		*(.patch_ItemAssignXPosition2)
	}

	.patch_ItemAssignIPosition2 0x2EC914 : {
		*(.patch_ItemAssignIPosition2)
	}

	.patch_ItemAssignYDestination2 0x2ECA74 : {
		*(.patch_ItemAssignYDestination2)
	}

	.patch_ItemAssignYPosition2 0x2ECA80 : {
		*(.patch_ItemAssignYPosition2)
	}

	.patch_ItemAssignIIPosition2 0x2ECA88 : {
		*(.patch_ItemAssignIIPosition2)
	}

	.patch_SingleScreenAfterTopPass 0x300074 : {
		*(.patch_SingleScreenAfterTopPass)
	}

	.patch_SingleScreenBeforeTopOverlay 0x300038 : {
		*(.patch_SingleScreenBeforeTopOverlay)
	}

	.patch_SingleScreenBeforeTopPresentation 0x2FFFF4 : {
		*(.patch_SingleScreenBeforeTopPresentation)
	}

	.patch_UnifiedPauseDim 0x41EBC0 : {
		*(.patch_UnifiedPauseDim)
	}
	.patch_DialogueTextScale 0x300068 : {
		*(.patch_DialogueTextScale)
	}
	.patch_ItemsShelfColor 0x46B5AC : { *(.patch_ItemsShelfColor) }
	.patch_GearShelfColor 0x467158 : { *(.patch_GearShelfColor) }
	.patch_GearPreviewCamera 0x30000C : {
		*(.patch_GearPreviewCamera)
	}

	.patch_FileSelectBeforeScene 0x2FFF1C : {
		*(.patch_FileSelectBeforeScene)
	}
	.patch_FileSelectAfterScene 0x2FFF80 : {
		*(.patch_FileSelectAfterScene)
	}
	.patch_FileSelectText 0x300030 : {
		*(.patch_FileSelectText)
	}
	.patch_FileSelectBackdrop 0x2FFFE8 : {
		*(.patch_FileSelectBackdrop)
	}
	.patch_FileSelectLeather 0x41EE04 : {
		*(.patch_FileSelectLeather)
	}
	.patch_FileSelectHeading 0x4337C0 : {
		*(.patch_FileSelectHeading)
	}

	.patch_SingleScreenMenuTitle 0x42BE64 : {
		*(.patch_SingleScreenMenuTitle)
	}

	.patch_SingleScreenSecondaryCallback 0x419848 : {
		*(.patch_SingleScreenSecondaryCallback)
	}

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

	.patch_MessageFontAtlasDimensions 0x44CEC0 : {
		*(.patch_MessageFontAtlasDimensions)
	}

	.patch_MessageFontCommandCapacity 0x44CECC : {
		*(.patch_MessageFontCommandCapacity)
	}

	.patch_MessageFontQuadWidth 0x2B6DCC : {
		*(.patch_MessageFontQuadWidth)
	}

	.patch_MessageFontQuadBaseline 0x2B6DF4 : {
		*(.patch_MessageFontQuadBaseline)
	}

	.patch_MessageFontLineAdvancePrimary 0x4C0D90 : {
		*(.patch_MessageFontLineAdvancePrimary)
	}

	.patch_MessageFontLineAdvanceSecondary 0x4C0DB0 : {
		*(.patch_MessageFontLineAdvanceSecondary)
	}

	.patch_MessageFontMeasureSecondary 0x2BAB00 : {
		*(.patch_MessageFontMeasureSecondary)
	}

	.patch_MessageFontMeasureBlock0 0x2C232C : {
		*(.patch_MessageFontMeasureBlock0)
	}

	.patch_MessageFontMeasureBlock1 0x2C2344 : {
		*(.patch_MessageFontMeasureBlock1)
	}

	.patch_MessageFontMeasureBlock2 0x2C23A8 : {
		*(.patch_MessageFontMeasureBlock2)
	}

	.patch_MessageFontMeasureBlock3 0x2C2410 : {
		*(.patch_MessageFontMeasureBlock3)
	}

	.patch_MessageFontMeasureBounds0 0x48C374 : {
		*(.patch_MessageFontMeasureBounds0)
	}

	.patch_MessageFontMeasureBounds1 0x48C3A8 : {
		*(.patch_MessageFontMeasureBounds1)
	}

	.patch_MessageFontMeasureBounds2 0x48C3C0 : {
		*(.patch_MessageFontMeasureBounds2)
	}

	. = 0x4C99A8;
	. = ALIGN(4);
	.loader : {
		*(.loader*)
	}

	. = 0x5C7000;
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
