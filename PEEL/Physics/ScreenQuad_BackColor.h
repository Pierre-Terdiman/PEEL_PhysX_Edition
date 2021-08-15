///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SCREEN_QUAD_BACK_COLOR_H
#define SCREEN_QUAD_BACK_COLOR_H

#include "ScreenQuad.h"

	class ScreenQuadBackColor : public ScreenQuadMode, public ColorPickerCallback
	{
		public:
							ScreenQuadBackColor(const Point& back_color);
		virtual				~ScreenQuadBackColor();

		virtual	const char*	GetUIName()		const		{ return "Back color";	}
		virtual	void		Apply(udword screen_width, udword screen_height);
		virtual	void		CreateUI(PintGUIHelper& helper, Widgets* widgets, IceWindow* parent);

		// ColorPickerCallback
		virtual	void		OnNewColorSelected(ubyte r, ubyte g, ubyte b);

				Point		mBackColor;
	};

#endif
