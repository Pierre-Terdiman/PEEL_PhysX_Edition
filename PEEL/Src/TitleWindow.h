///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TITLE_WINDOW_H
#define TITLE_WINDOW_H

#include "Common.h"

	#define	TITLE_WIDTH		400
	#define	TITLE_HEIGHT	195

	class TitleBitmapWindow;

	class TitleWindow : public IceWindow
	{
		public:
						TitleWindow(const WindowDesc& desc);
		virtual			~TitleWindow();

		virtual void	redraw();
		void			CreatePic();

		TitleBitmapWindow*	mRGBWindow;
//		IceGroupBox*	mRGBGroup;
		Picture			mPic;
	};

	class TitleBitmapWindow: public IceWindow
	{
		public:
						TitleBitmapWindow(const WindowDesc& desc);
		virtual			~TitleBitmapWindow();

		virtual void	redraw();

		TitleWindow*	mMainW;
	};

	class Widgets;
	TitleWindow* CreateTitleWindow(Widgets& owner, IceWidget* parent, sdword x, sdword y);

#endif