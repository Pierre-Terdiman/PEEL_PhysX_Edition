///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SCREEN_QUAD_COLOR_GRADIENT_H
#define SCREEN_QUAD_COLOR_GRADIENT_H

#include "ScreenQuad.h"

	class ScreenQuadColorGradient : public ScreenQuadMode, public ColorPickerCallback
	{
		public:
							ScreenQuadColorGradient(const Point& top_color, const Point& bottom_color);
		virtual				~ScreenQuadColorGradient();

		virtual	const char*	GetUIName()		const		{ return "Gradient";	}
		virtual	void		Apply(udword screen_width, udword screen_height);
		virtual	void		CreateUI(PintGUIHelper& helper, Widgets* widgets, IceWindow* parent);

		// ColorPickerCallback
		virtual	void		OnNewColorSelected(ubyte r, ubyte g, ubyte b);

				Point		mTopColor;
				Point		mBottomColor;
				udword		mID;
	};

#endif
