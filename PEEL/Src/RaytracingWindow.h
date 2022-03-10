///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef RAYTRACING_WINDOW_H
#define RAYTRACING_WINDOW_H

#include "Common.h"

	#define	RAYTRACING_MAX_RENDER_SIZE	768
	#define	RAYTRACING_DISPLAY_WIDTH	256
	#define	RAYTRACING_DISPLAY_HEIGHT	256

	class BitmapWindow;

	class RaytracingWindow : public IceWindow
	{
		public:
						RaytracingWindow(const WindowDesc& desc);
		virtual			~RaytracingWindow();

		virtual void	redraw();
		void			CreatePic();

		BitmapWindow*	mRGBWindow;
		IceGroupBox*	mRGBGroup;
		Picture			mTarget;
		Picture			mPic;
	};

	class BitmapWindow: public IceWindow
	{
		public:
						BitmapWindow(const WindowDesc& desc);
		virtual			~BitmapWindow();

		virtual void	redraw();

		RaytracingWindow*	mMainW;
	};

	class Widgets;
	RaytracingWindow* CreateRaytracingWindow(Widgets& owner, IceWidget* parent, sdword x, sdword y, sdword width, sdword height);

#endif