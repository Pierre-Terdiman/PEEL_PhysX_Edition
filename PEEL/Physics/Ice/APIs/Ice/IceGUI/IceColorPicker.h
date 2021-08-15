///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECOLORPICKER_H
#define ICECOLORPICKER_H

	enum PickerEvent
	{
		PICKER_PALETTE_UPDATE,
	};

	struct PickerEventContext
	{
	};

	struct PickerEventContext_PaletteUpdate : public PickerEventContext
	{
		const RGBAPalette*	mPalette;
	};

	typedef udword (*ColorPickerCB)(PickerEvent picker_event_code, void* context, void* user_data);

	FUNCTION ICEGUI_API IceWindow* CreateColorPicker(IceWidget* parent, sdword x, sdword y, sdword width, sdword height, ColorPickerCB cb, void* user_data);

#endif	// ICECOLORPICKER_H