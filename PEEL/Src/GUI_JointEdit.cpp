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

///////////////////////////////////////////////////////////////////////////////

/*#include "PintGUIHelper.h"

JointTypeComboBox* CreateJointComboBox(IceWidget* parent, sdword x, sdword y, void* user_data, EditJointTypeCallback& callback)
{
	ComboBoxDesc CBD;
	CBD.mID		= 0;
	CBD.mParent	= parent;
	CBD.mX		= x;
	CBD.mY		= y;
	CBD.mWidth	= 150;
	CBD.mHeight	= 20;
	CBD.mLabel	= "Joint type";
	JointTypeComboBox* JTCB = ICE_NEW(JointTypeComboBox)(CBD, callback);
	JTCB->SetVisible(true);

	JTCB->SetUserData(user_data);
	JTCB->Add("Spherical");		// IJT_SPHERICAL
	JTCB->Add("Hinge X");		// IJT_HINGE_X
	JTCB->Add("Hinge Y");		// IJT_HINGE_Y
	JTCB->Add("Hinge Z");		// IJT_HINGE_Z
	JTCB->Add("Prismatic X");	// IJT_PRISMATIC_X
	JTCB->Add("Prismatic Y");	// IJT_PRISMATIC_Y
	JTCB->Add("Prismatic Z");	// IJT_PRISMATIC_Z
	JTCB->Add("Fixed");			// IJT_FIXED
	JTCB->Add("Distance");		// IJT_DISTANCE
	JTCB->Add("D6");			// IJT_D6
	JTCB->Add("Gear");			// IJT_GEAR
	JTCB->Add("Rack & pinion");	// IJT_RACK
	return JTCB;
}
*/