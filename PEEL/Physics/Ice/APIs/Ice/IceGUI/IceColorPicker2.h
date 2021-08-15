///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECOLORPICKER2_H
#define ICECOLORPICKER2_H

	class ICEGUI_API ColorPickerCallback
	{
		public:
			virtual	void	OnNewColorSelected(ubyte r, ubyte g, ubyte b)	= 0;
	};

	FUNCTION ICEGUI_API void		CreateColorPic(Picture& picture, udword w, udword h, float z);
	FUNCTION ICEGUI_API IceWindow*	CreateColorPicker2(IceWidget* parent, sdword x, sdword y, ColorPickerCallback* callback);

#endif	// ICECOLORPICKER2_H