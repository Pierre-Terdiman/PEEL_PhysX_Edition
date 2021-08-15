///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TitleWindow.h"
#include "PintGUIHelper.h"

#define	BORDER_SIZE_X	4
#define	BORDER_SIZE_Y	4

TitleWindow::TitleWindow(const WindowDesc& desc) : IceWindow(desc)
{
	mRGBWindow = null;
//	mRGBGroup = null;
	mFlags |= WF_MOVABLE_CLIENT_AREA;

	mPic.Init(TITLE_WIDTH, TITLE_HEIGHT);
	mPic.Clear();

	CreatePic();
}

TitleWindow::~TitleWindow()
{
}

extern udword gPictureData_Data[];

void TitleWindow::CreatePic()
{
	mPic.Init(TITLE_WIDTH, TITLE_HEIGHT);
	RGBAPixel* Pixels = mPic.GetPixels();
	CopyMemory(Pixels, gPictureData_Data, mPic.GetDataSize());
}

void TitleWindow::redraw()
{
//	CreatePic();
	mRGBWindow->redraw();
}

TitleBitmapWindow::TitleBitmapWindow(const WindowDesc& desc) : IceWindow(desc)
{
	mMainW = null;
}

TitleBitmapWindow::~TitleBitmapWindow()
{
}

void TitleBitmapWindow::redraw()
{
	CopyPictureToWindow(GetHandle(), mMainW->mPic, false);
}

static void CreateBitmapWindow(Widgets& owner, TitleBitmapWindow*& bitmap, /*IceGroupBox*& group,*/ TitleWindow* parent, sdword x, sdword y, udword width, udword height, const char* label)
{
	// Create bitmap window
	WindowDesc WD;
	WD.mParent	= parent;
	WD.mX		= x;
	WD.mY		= y;
	WD.mWidth	= width;
	WD.mHeight	= height;
	WD.mLabel	= "BitmapWindow";
	WD.mType	= WINDOW_POPUP;
	WD.mStyle	= WSTYLE_NORMAL;
	bitmap = ICE_NEW(TitleBitmapWindow)(WD);
	bitmap->SetVisible(true);
	bitmap->mMainW = parent;
	owner.Register(bitmap);
}

TitleWindow* CreateTitleWindow(Widgets& owner, IceWidget* parent, sdword x, sdword y)
{
	const udword StyleWidth = 6;
	const udword StyleHeight = 6;

	const udword TitleWindowWidth = TITLE_WIDTH + BORDER_SIZE_X*2 + StyleWidth;
	const udword TitleWindowHeight = TITLE_HEIGHT + BORDER_SIZE_Y*2 + StyleHeight;

	// Create main window
	TitleWindow* TTWindow = null;
	if(1)
	{
		WindowDesc WD;
		WD.mParent	= parent;
		WD.mX		= x;
		WD.mY		= y;
		WD.mWidth	= TitleWindowWidth;
		WD.mHeight	= TitleWindowHeight;
		WD.mLabel	= "Title window";
		WD.mType	= WINDOW_NORMAL;
//		WD.mType	= WINDOW_POPUP;
//		WD.mStyle	= WSTYLE_CLIENT_EDGES;
		//WD.mStyle	= WSTYLE_NORMAL;
		//WD.mStyle	= WSTYLE_BORDER;
		WD.mStyle	= WSTYLE_CLIENT_EDGES|WSTYLE_STATIC_EDGES;
		TTWindow = ICE_NEW(TitleWindow)(WD);
		TTWindow->SetVisible(true);
		owner.Register(TTWindow);
	}

	CreateBitmapWindow(owner, TTWindow->mRGBWindow, /*TTWindow->mRGBGroup,*/ TTWindow, BORDER_SIZE_X, BORDER_SIZE_Y, TITLE_WIDTH, TITLE_HEIGHT, "");

	return TTWindow;
}
