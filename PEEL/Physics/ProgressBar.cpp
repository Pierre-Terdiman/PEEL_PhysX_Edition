///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProgressBar.h"

static IceWindow*		gWindow = null;
static IceProgressBar*	gProgressBar = null;
static String*			gProgressBarTitle = null;

void CreateProgressBar(udword nb, const char* label)
{
	ASSERT(!gProgressBarTitle);
	ASSERT(!gProgressBar);

	gProgressBarTitle = ICE_NEW(String)(label);

	WindowDesc WD;
	WD.mParent	= null;
	WD.mX		= 0;
	WD.mY		= 0;
	WD.mWidth	= 512;
	WD.mHeight	= 80;
	WD.mLabel	= gProgressBarTitle->Get();
	WD.mType	= WINDOW_DIALOG;
//	WD.mType	= WINDOW_POPUP;
	WD.mStyle	= WSTYLE_ALWAYS_ON_TOP|WSTYLE_CLIENT_EDGES;
	WD.Center();
	gWindow = ICE_NEW(IceWindow)(WD);
	gWindow->SetVisible(true);

	ProgressBarDesc PBD;
	PBD.mParent		= gWindow;
	PBD.mX			= 20;
	PBD.mY			= 10;
	PBD.mWidth		= 512-20*2;
	PBD.mHeight		= 20;
	PBD.mStyle		= PROGRESSBAR_NORMAL;
	gProgressBar = ICE_NEW(IceProgressBar)(PBD);
	gProgressBar->SetVisible(true);
	gProgressBar->SetTotalSteps(nb);
	gProgressBar->SetValue(0);
}

void SetProgress(udword i)
{
	gProgressBar->SetValue(i);
}

void ReleaseProgressBar()
{
	if(gWindow)
		gWindow->SetVisible(false);
	DELETESINGLE(gWindow);
	DELETESINGLE(gProgressBar);
	DELETESINGLE(gProgressBarTitle);
}
