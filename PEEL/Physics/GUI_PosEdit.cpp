///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_PosEdit.h"
#include "PintGUIHelper.h"
#include "GUI_Helpers.h"

EditPosWindow::EditPosWindow(const WindowDesc& desc) :
	IceWindow	(desc),
	mLabelX		(null),
	mLabelY		(null),
	mLabelZ		(null),
	mEditBoxX	(null),
	mEditBoxY	(null),
	mEditBoxZ	(null),
	mEdited		(Point(0.0f, 0.0f, 0.0f)),
	mHasEdit	(false),
	mEnabled	(false)
{
	const sdword OffsetX = 20;
	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	const sdword x = 4;

	struct Local
	{
		static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			if(param!=IceEditBox::CB_PARAM_RETURN)	
				return;

			EditPosWindow* EPW = reinterpret_cast<EditPosWindow*>(user_data);
			ASSERT(EPW);

			//printf("gEBCallback\n");
			const udword ID = edit_box.GetID();
			if(ID==X_)
				EPW->mEdited.x = GetFloat(EPW->mEdited.x, &edit_box);
			else if(ID==Y_)
				EPW->mEdited.y = GetFloat(EPW->mEdited.y, &edit_box);
			else if(ID==Z_)
				EPW->mEdited.z = GetFloat(EPW->mEdited.z, &edit_box);
			EPW->mHasEdit = true;
		}
	};

	sdword y = 4;

	mLabelX = GUI_CreateLabel(this, x, y+LabelOffsetY, 90, 20, "X:", null);
//	mEditBoxX = GUI_CreateEditBox(this, X_, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(mEdited.x), null, EDITBOX_FLOAT, Local::gEBCallback, null);
	mEditBoxX = GUI_CreateEditBox(this, X_, x+OffsetX, y, EditBoxWidth, 20, "0", null, EDITBOX_FLOAT, Local::gEBCallback, null);
	mEditBoxX->SetUserData(this);
	y += YStep;

	mLabelY = GUI_CreateLabel(this, x, y+LabelOffsetY, 90, 20, "Y:", null);
//	mEditBoxY = GUI_CreateEditBox(this, Y_, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(mEdited.y), null, EDITBOX_FLOAT, Local::gEBCallback, null);
	mEditBoxY = GUI_CreateEditBox(this, Y_, x+OffsetX, y, EditBoxWidth, 20, "0", null, EDITBOX_FLOAT, Local::gEBCallback, null);
	mEditBoxY->SetUserData(this);
	y += YStep;

	mLabelZ = GUI_CreateLabel(this, x, y+LabelOffsetY, 90, 20, "Z:", null);
//	mEditBoxZ = GUI_CreateEditBox(this, Z_, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(mEdited.z), null, EDITBOX_FLOAT, Local::gEBCallback, null);
	mEditBoxZ = GUI_CreateEditBox(this, Z_, x+OffsetX, y, EditBoxWidth, 20, "0", null, EDITBOX_FLOAT, Local::gEBCallback, null);
	mEditBoxZ->SetUserData(this);
	y += YStep;

	mEnabled = true;
}

EditPosWindow::~EditPosWindow()
{
	DELETESINGLE(mEditBoxZ);
	DELETESINGLE(mLabelZ);
	DELETESINGLE(mEditBoxY);
	DELETESINGLE(mLabelY);
	DELETESINGLE(mEditBoxX);
	DELETESINGLE(mLabelX);
	mEnabled = false;
}

void EditPosWindow::SetPos(const Point& pos)
{
	if(mEditBoxX)
		mEditBoxX->SetText(_F("%f", pos.x));
	if(mEditBoxY)
		mEditBoxY->SetText(_F("%f", pos.y));
	if(mEditBoxZ)
		mEditBoxZ->SetText(_F("%f", pos.z));
	mEdited = pos;
}

Point EditPosWindow::GetPos() const
{
	mEdited.x = GetFloat(mEdited.x, mEditBoxX);
	mEdited.y = GetFloat(mEdited.y, mEditBoxY);
	mEdited.z = GetFloat(mEdited.z, mEditBoxZ);
	return mEdited;
}

void EditPosWindow::SetEulerAngles(const Quat& rot)
{
	float Angles[3];
	QuatToEuler(rot, Angles);

	SetPos(Point(Angles[0]*RADTODEG, Angles[1]*RADTODEG, Angles[2]*RADTODEG));
}

void EditPosWindow::GetEulerAngles(Quat& rot) const
{
	float Angles[3];
	Angles[0] = mEdited.x * DEGTORAD;
	Angles[1] = mEdited.y * DEGTORAD;
	Angles[2] = mEdited.z * DEGTORAD;

	EulerToQuat(Angles, rot);
}

void EditPosWindow::Reset()
{
	mHasEdit = false;
	mEdited.Zero();
	if(mEditBoxX)
		mEditBoxX->SetText("0");
	if(mEditBoxY)
		mEditBoxY->SetText("0");
	if(mEditBoxZ)
		mEditBoxZ->SetText("0");
}

void EditPosWindow::SetEnabled(bool b)
{
	if(mEnabled==b)
		return;
	mEnabled = b;
	
	if(mEditBoxX)
		mEditBoxX->SetEnabled(b);
	if(mEditBoxY)
		mEditBoxY->SetEnabled(b);
	if(mEditBoxZ)
		mEditBoxZ->SetEnabled(b);
}
