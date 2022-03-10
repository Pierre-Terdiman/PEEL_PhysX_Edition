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

EditPosWindow::EditPosWindow(const WindowDesc& desc, EditBoxInterface* owner) :
	IceWindow	(desc),
	mOwner		(owner),
	mEditBoxX	(null),
	mEditBoxY	(null),
	mEditBoxZ	(null),
	mEdited		(Point(0.0f, 0.0f, 0.0f)),
	mHasEdit	(false),
	mEnabled	(false)
{
	const sdword OffsetX = 20;
	const sdword EditBoxWidth = 60;
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

			if(EPW->mOwner)
				EPW->mOwner->ChangeNotification();
		}
	};

	sdword y = 4;

//	const float DefaultValueX = helper.Convert(mEdited.x);
//	const float DefaultValueY = helper.Convert(mEdited.y);
//	const float DefaultValueZ = helper.Convert(mEdited.z);

	mEditBoxX = CreateFloatEditBox(*this, this, x, y, "X:", OffsetX, EditBoxWidth, 20, X_, 0.0f);
	mEditBoxX->SetCallback(Local::gEBCallback);
	mEditBoxX->SetUserData(this);
	y += YStep;

	mEditBoxY = CreateFloatEditBox(*this, this, x, y, "Y:", OffsetX, EditBoxWidth, 20, Y_, 0.0f);
	mEditBoxY->SetCallback(Local::gEBCallback);
	mEditBoxY->SetUserData(this);
	y += YStep;

	mEditBoxZ = CreateFloatEditBox(*this, this, x, y, "Z:", OffsetX, EditBoxWidth, 20, Z_, 0.0f);
	mEditBoxZ->SetCallback(Local::gEBCallback);
	mEditBoxZ->SetUserData(this);
	y += YStep;

	mEnabled = true;
}

EditPosWindow::~EditPosWindow()
{
	DELETESINGLE(mEditBoxZ);
	DELETESINGLE(mEditBoxY);
	DELETESINGLE(mEditBoxX);
	mEnabled = false;
}

void EditPosWindow::ChangeNotification()
{
	if(mOwner)
		mOwner->ChangeNotification();
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
	// mEdited is not always up to date. It's updated when:
	// - the value is changed by scrolling with the mouse
	// - the value is manually entered & validated by pressing Return
	// But if users manually enter the value and DO NOT press return, then the above callback
	// is not called and mEdited not updated. Calling GetPos instead of reading mEdited
	// directly will then make a final read of the edit box data and use the correct value.

	// Related issue: mHasEdit will still be false in that case. This is why mSomethingChanged
	// in the edit box class itself is more reliable, since it's updated in the FilterKey function
	// as soon as a single key is pressed (return or not)

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

bool EditPosWindow::SomethingChanged() const
{
	if(mEditBoxX && mEditBoxX->mSomethingChanged)
		return true;
	if(mEditBoxY && mEditBoxY->mSomethingChanged)
		return true;
	if(mEditBoxZ && mEditBoxZ->mSomethingChanged)
		return true;
	return mHasEdit;
}

void EditPosWindow::ResetChangedState()
{
	if(mEditBoxX)
		mEditBoxX->mSomethingChanged = false;
	if(mEditBoxY)
		mEditBoxY->mSomethingChanged = false;
	if(mEditBoxZ)
		mEditBoxZ->mSomethingChanged = false;
	mHasEdit = false;
}
