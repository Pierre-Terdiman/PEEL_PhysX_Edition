///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_EditBox.h"

PEEL_EditBox::PEEL_EditBox(const EditBoxDesc& desc, EditBoxInterface& owner, udword label_width, const char* label) :
	IceEditBox			(desc),
	mOwner				(owner),
	mSomethingChanged	(false)
{
	if(label_width)
		mLabel = GUI_CreateLabel(desc.mParent, desc.mX - label_width, desc.mY+2, label_width, 20, label, null);
}

PEEL_EditBox::~PEEL_EditBox()
{
	DELETESINGLE(mLabel);
}

void PEEL_EditBox::SetVisible(bool b)
{
	IceEditBox::SetVisible(b);
	if(mLabel)
		mLabel->SetVisible(b);
}

bool PEEL_EditBox::FilterKey(udword key) const
{
	//printf("Key: %d\n", key);
	mOwner.ChangeNotification();

	mSomethingChanged = true;
	return IceEditBox::FilterKey(key);
}

static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
{
	if(param!=IceEditBox::CB_PARAM_RETURN)	
		return;

	EditBoxInterface* EBI = reinterpret_cast<EditBoxInterface*>(user_data);
	if(EBI)
		EBI->ChangeNotification();

	const PEEL_EditBox& PEB = static_cast<const PEEL_EditBox&>(edit_box);
	PEB.mSomethingChanged = true;
}

PEEL_EditBox* CreateTextEditBox(EditBoxInterface& owner, IceWidget* parent, sdword x, sdword y, const char* label, udword label_width, udword edit_box_width, udword edit_box_height, udword id, const char* default_name)
{
	EditBoxDesc EBD;
	EBD.mParent		= parent;
	EBD.mX			= x + label_width;
	EBD.mY			= y;
	EBD.mWidth		= edit_box_width;
	EBD.mHeight		= edit_box_height;
	EBD.mLabel		= default_name;
	EBD.mID			= id;
	EBD.mType		= EDITBOX_NORMAL;
	EBD.mFilter		= EDITBOX_TEXT;
	EBD.mCallback	= gEBCallback;
	EBD.mUserData	= &owner;

	return ICE_NEW(PEEL_EditBox)(EBD, owner, label_width, label);
}

PEEL_EditBox* CreateFloatEditBox(EditBoxInterface& owner, IceWidget* parent, sdword x, sdword y, const char* label, udword label_width, udword edit_box_width, udword edit_box_height, udword id, float default_value)
{
	EditBoxDesc EBD;
	EBD.mParent		= parent;
	EBD.mX			= x + label_width;
	EBD.mY			= y;
	EBD.mWidth		= edit_box_width;
	EBD.mHeight		= edit_box_height;
	EBD.mLabel		= _F("%.4f", default_value);
	EBD.mID			= id;
	EBD.mType		= EDITBOX_NORMAL;
	EBD.mFilter		= EDITBOX_FLOAT;
	EBD.mCallback	= gEBCallback;
	EBD.mUserData	= &owner;

	return ICE_NEW(PEEL_EditBox)(EBD, owner, label_width, label);
}
