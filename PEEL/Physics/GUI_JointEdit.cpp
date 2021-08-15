///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_JointEdit.h"

EditJointWindow::EditJointWindow(const WindowDesc& desc) :
	IceWindow			(desc)
{
}

EditJointWindow::~EditJointWindow()
{
}

///////////////////////////////////////////////////////////////////////////////

static EditJointWindow* gEditJointWindow = null;

EditJointWindow* CreateJointEditGUI(IceWidget* parent, sdword x, sdword y)
{
	const udword Width = 300;
	const udword Height = 300;

	WindowDesc WD;
	WD.mParent	= parent;
	WD.mX		= x;
	WD.mY		= y;
	WD.mWidth	= Width;
	WD.mHeight	= Height;
	WD.mLabel	= "Edit joint";
	WD.mType	= WINDOW_DIALOG;
	WD.mStyle	= WSTYLE_HIDDEN;

	EditJointWindow* EJW = ICE_NEW(EditJointWindow)(WD);
//	EAW->SetVisible(true);

	gEditJointWindow = EJW;

	return EJW;
}

void CloseJointEditGUI()
{
	DELETESINGLE(gEditJointWindow);
}

void HideEditJointWindow()
{
	if(!gEditJointWindow)
		return;
	gEditJointWindow->SetVisible(false);
//	gEditJointWindow->InitFrom(null, null);	// Make sure we don't keep dangling pointers in there
}

void ShowEditJointWindow(/*Pint* pint, PintActorHandle handle*/)
{
	if(!gEditJointWindow)
		return;

//	gEditJointWindow->InitFrom(pint, handle);

	POINT pt;
	IceCore::GetCursorPos(&pt);
	gEditJointWindow->SetPosition(pt.x, pt.y);

	gEditJointWindow->SetVisible(true);
}
