///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SCREEN_QUAD_COLOR_SPHERE_H
#define SCREEN_QUAD_COLOR_SPHERE_H

#include "ScreenQuad.h"
#include "GLShader.h"

	class ScreenQuadColorSphere : public ShaderBasedScreenQuad, public ColorPickerCallback
	{
		public:
							ScreenQuadColorSphere();
		virtual				~ScreenQuadColorSphere();

		virtual	const char*	GetUIName()		const		{ return "Color sphere";	}
		virtual	void		Apply(udword screen_width, udword screen_height);
		virtual	void		CreateUI(PintGUIHelper& helper, Widgets* widgets, IceWindow* parent);

		// ColorPickerCallback
		virtual	void		OnNewColorSelected(ubyte r, ubyte g, ubyte b);

				Point		mTopColor;
				Point		mMiddleColor;
				Point		mBottomColor;
				udword		mID;
				float		mSpread;
	};

#endif
