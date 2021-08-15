///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintGUIHelper.h"

class GUIHelper : public PintGUIHelper
{
	public:
	virtual	const char*		Convert(float value);

	virtual	IceWindow*		CreateMainWindow(Widgets*& gui, IceWidget* parent, udword id, const char* label);
	virtual	IceLabel*		CreateLabel		(IceWidget* parent,				sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner);
	virtual	IceCheckBox*	CreateCheckBox	(IceWidget* parent, udword id,	sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, bool state, CBCallback callback, const char* tooltip=null);
	virtual	IceEditBox*		CreateEditBox	(IceWidget* parent, udword id,	sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, EditBoxFilter filter, EBCallback callback, const char* tooltip=null);
	virtual	IceButton*		CreateButton	(IceWidget* parent, udword id,	sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, BCallback callback, void* user_data, const char* tooltip=null);
};

static GUIHelper gGUIHelper;

PintGUIHelper* GetGUIHelper()
{
	return &gGUIHelper;
}

const char* GUIHelper::Convert(float value)
{
	static char ConvertBuffer[256];
//	ConvertAndRemoveTrailingZeros(ConvertBuffer, value);
	sprintf(ConvertBuffer, "%g", value);
	return ConvertBuffer;
}

IceWindow* GUIHelper::CreateMainWindow(Widgets*& gui, IceWidget* parent, udword id, const char* label)
{
	ASSERT(!gui);
	gui = ICE_NEW(Widgets);
	ASSERT(gui);

	WindowDesc WD;
	WD.mID		= id;
	WD.mParent	= parent;
	WD.mX		= 100;
	WD.mY		= 100;
	WD.mWidth	= 256;
	WD.mHeight	= 300;
	WD.mLabel	= label;
//	WD.mType	= WINDOW_NORMAL;
	WD.mType	= WINDOW_DIALOG;
//	WD.mType	= WINDOW_MODALDIALOG;
//	WD.mStyle	= WSTYLE_DLGFRAME;
	IceWindow* Main = ICE_NEW(IceWindow)(WD);
	gui->Register(Main);
	Main->SetVisible(true);
	return Main;
}

IceLabel* GUI_CreateLabel(IceWidget* parent, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner)
{
	LabelDesc LD;
	LD.mParent		= parent;
	LD.mX			= x;
	LD.mY			= y;
	LD.mWidth		= width;
	LD.mHeight		= height;
	LD.mLabel		= label;
	IceLabel* Label = ICE_NEW(IceLabel)(LD);
	Label->SetVisible(true);
	if(owner)
		owner->Register(Label);
	return Label;
}

IceCheckBox* GUI_CreateCheckBox(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, bool state, CBCallback callback, const char* tooltip)
{
	CheckBoxDesc CBD;
	CBD.mID			= id;
	CBD.mParent		= parent;
	CBD.mX			= x;
	CBD.mY			= y;
	CBD.mWidth		= width;
	CBD.mHeight		= height;
	CBD.mLabel		= label;
	CBD.mChecked	= state;
	CBD.mCallback	= callback;
	IceCheckBox* CB = ICE_NEW(IceCheckBox)(CBD);
	CB->SetVisible(true);
	if(owner)
		owner->Register(CB);

	if(tooltip)
		CB->AddToolTip(tooltip);

	return CB;
}

IceEditBox* GUI_CreateEditBox(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, EditBoxFilter filter, EBCallback callback, const char* tooltip)
{
	EditBoxDesc EBD;
	EBD.mID			= id;
	EBD.mParent		= parent;
	EBD.mX			= x;
	EBD.mY			= y;
	EBD.mWidth		= width;
	EBD.mHeight		= height;
	EBD.mLabel		= label;
	EBD.mFilter		= filter;
	EBD.mCallback	= callback;
	IceEditBox* EB = ICE_NEW(IceEditBox)(EBD);
	EB->SetVisible(true);
	if(owner)
		owner->Register(EB);

	if(tooltip)
		EB->AddToolTip(tooltip);

	return EB;
}

IceButton* GUI_CreateButton(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, BCallback callback, void* user_data, const char* tooltip)
{
	ButtonDesc BD;
//	BD.mStyle		= ;
	BD.mCallback	= callback;
	BD.mUserData	= user_data;
	BD.mID			= id;
	BD.mParent		= parent;
	BD.mX			= x;
	BD.mY			= y;
	BD.mWidth		= width;
	BD.mHeight		= height;
	BD.mLabel		= label;
	IceButton* B = ICE_NEW(IceButton)(BD);
	B->SetVisible(true);

	if(owner)
		owner->Register(B);

	if(tooltip)
		B->AddToolTip(tooltip);
	return B;
}

IceLabel* GUIHelper::CreateLabel(IceWidget* parent, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner)
{
	return GUI_CreateLabel(parent, x, y, width, height, label, owner);
}

IceCheckBox* GUIHelper::CreateCheckBox(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, bool state, CBCallback callback, const char* tooltip)
{
	return GUI_CreateCheckBox(parent, id, x, y, width, height, label, owner, state, callback, tooltip);
}

IceEditBox* GUIHelper::CreateEditBox(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, EditBoxFilter filter, EBCallback callback, const char* tooltip)
{
	return GUI_CreateEditBox(parent, id, x, y, width, height, label, owner, filter, callback, tooltip);
}

IceButton* GUIHelper::CreateButton(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, BCallback callback, void* user_data, const char* tooltip)
{
	return GUI_CreateButton(parent, id, x, y, width, height, label, owner, callback, user_data, tooltip);
}
