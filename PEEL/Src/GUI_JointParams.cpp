///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_JointParams.h"
#include "GUI_EditBox.h"
#include "Pint.h"

///////////////////////////////////////////////////////////////////////////////

static void EditBox_SetFloatValue(PEEL_EditBox* edit_box, float value, bool enabled, const char* label)
{
	if(edit_box)
	{
		edit_box->SetText(_F("%f", value));
		edit_box->SetEnabled(enabled);
		if(edit_box->mLabel && label)
			edit_box->mLabel->SetLabel(label);
	}
}

///////////////////////////////////////////////////////////////////////////////

GUI_JointParams::GUI_JointParams()
{
}

GUI_JointParams::~GUI_JointParams()
{
}

///////////////////////////////////////////////////////////////////////////////

GUI_1DLimitJointParams::GUI_1DLimitJointParams() :
	mEditBox_MinLimit	(null),
	mEditBox_MaxLimit	(null)
{
}

GUI_1DLimitJointParams::~GUI_1DLimitJointParams()
{
	DELETESINGLE(mEditBox_MaxLimit);
	DELETESINGLE(mEditBox_MinLimit);
}

udword GUI_1DLimitJointParams::Init(EditBoxInterface& owner, IceWidget* parent, udword y)
{
	ASSERT(!mEditBox_MinLimit);
	ASSERT(!mEditBox_MaxLimit);

	const udword ParamsLabelWidth = 80;
	const udword EditBoxWidth = 100;
	const udword EditBoxHeight = 20;
	udword YOffset = y;

	const udword x0 = 4;
	const udword x1 = x0+ParamsLabelWidth+EditBoxWidth+10;

	mEditBox_MinLimit = CreateFloatEditBox(owner, parent, x0, YOffset, "Min limit:", ParamsLabelWidth, EditBoxWidth, EditBoxHeight, 0, 0.0f);
	mEditBox_MaxLimit = CreateFloatEditBox(owner, parent, x1, YOffset, "Max limit:", ParamsLabelWidth, EditBoxWidth, EditBoxHeight, 0, 0.0f);
	YOffset += EditBoxHeight + 4;

	return YOffset;
}

void GUI_1DLimitJointParams::SetVisible(bool is_visible)
{
	if(mEditBox_MinLimit)
		mEditBox_MinLimit->SetVisible(is_visible);
	if(mEditBox_MaxLimit)
		mEditBox_MaxLimit->SetVisible(is_visible);
}

void GUI_1DLimitJointParams::InitFrom(Pint_Joint* joint_api, PintJointHandle handle, const char* min_label, const char* max_label, udword index)
{
	if(!joint_api)	// Might not be implemented by all plugins
	{
		if(mEditBox_MinLimit)
			mEditBox_MinLimit->SetEnabled(false);
		if(mEditBox_MaxLimit)
			mEditBox_MaxLimit->SetEnabled(false);
	}
	else
	{
		PintLimits Limits;
		const bool EnableLimits = joint_api->GetLimits(handle, Limits, index);
		if(!EnableLimits)
			Limits.Set(0.0f, 0.0f);

		EditBox_SetFloatValue(mEditBox_MinLimit, Limits.mMinValue, EnableLimits, min_label);
		EditBox_SetFloatValue(mEditBox_MaxLimit, Limits.mMaxValue, EnableLimits, max_label);
	}
}

void GUI_1DLimitJointParams::Cancel()
{
	if(mEditBox_MinLimit)
		mEditBox_MinLimit->mSomethingChanged = false;
	if(mEditBox_MaxLimit)
		mEditBox_MaxLimit->mSomethingChanged = false;
}

bool GUI_1DLimitJointParams::SomethingChanged() const
{
	if(mEditBox_MinLimit && mEditBox_MinLimit->mSomethingChanged)
		return true;
	if(mEditBox_MaxLimit && mEditBox_MaxLimit->mSomethingChanged)
		return true;
	return false;
}

void GUI_1DLimitJointParams::SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors, udword index)
{
	if(mEditBox_MinLimit && mEditBox_MaxLimit)
	{
		if(mEditBox_MinLimit->mSomethingChanged || mEditBox_MaxLimit->mSomethingChanged)
		{
			mEditBox_MinLimit->mSomethingChanged = false;
			mEditBox_MaxLimit->mSomethingChanged = false;

			if(joint_api)
			{
				const PintLimits Limits(mEditBox_MinLimit->GetFloat(), mEditBox_MaxLimit->GetFloat());

				joint_api->SetLimits(handle, Limits, index);
				wake_up_actors = true;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

static const char* gSphericalMinLimitLabel = "Cone limit Y:";
static const char* gSphericalMaxLimitLabel = "Cone limit Z:";

GUI_SphericalJointParams::GUI_SphericalJointParams()
{
}

GUI_SphericalJointParams::~GUI_SphericalJointParams()
{
}

udword GUI_SphericalJointParams::Init(EditBoxInterface& owner, IceWidget* parent, udword y)
{
	return mLimit.Init(owner, parent, y);
}

void GUI_SphericalJointParams::SetVisible(bool is_visible)
{
	mLimit.SetVisible(is_visible);
}

void GUI_SphericalJointParams::InitFrom(Pint_Joint* joint_api, PintJointHandle handle)
{
	mLimit.InitFrom(joint_api, handle, gSphericalMinLimitLabel, gSphericalMaxLimitLabel, 0);
}

void GUI_SphericalJointParams::Cancel()
{
	mLimit.Cancel();
}

bool GUI_SphericalJointParams::SomethingChanged() const
{
	if(mLimit.SomethingChanged())
		return true;
	return false;
}

void GUI_SphericalJointParams::SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)
{
	mLimit.SaveParams(joint_api, handle, wake_up_actors, 0);
}

///////////////////////////////////////////////////////////////////////////////

static const char* gHingeMinLimitLabel = "Min limit:";
static const char* gHingeMaxLimitLabel = "Max limit:";

GUI_HingeJointParams::GUI_HingeJointParams()
{
}

GUI_HingeJointParams::~GUI_HingeJointParams()
{
}

udword GUI_HingeJointParams::Init(EditBoxInterface& owner, IceWidget* parent, udword y)
{
	return mLimit.Init(owner, parent, y);
}

void GUI_HingeJointParams::SetVisible(bool is_visible)
{
	mLimit.SetVisible(is_visible);
}

void GUI_HingeJointParams::InitFrom(Pint_Joint* joint_api, PintJointHandle handle)
{
	mLimit.InitFrom(joint_api, handle, gHingeMinLimitLabel, gHingeMaxLimitLabel, 0);
}

void GUI_HingeJointParams::Cancel()
{
	mLimit.Cancel();
}

bool GUI_HingeJointParams::SomethingChanged() const
{
	if(mLimit.SomethingChanged())
		return true;
	return false;
}

void GUI_HingeJointParams::SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)
{
	mLimit.SaveParams(joint_api, handle, wake_up_actors, 0);
}

///////////////////////////////////////////////////////////////////////////////

static const char* gPrismaticMinLimitLabel = "Min limit:";
static const char* gPrismaticMaxLimitLabel = "Max limit:";

GUI_PrismaticJointParams::GUI_PrismaticJointParams() :
	mEditBox_SpringStiffness(null),
	mEditBox_SpringDamping	(null)
{
}

GUI_PrismaticJointParams::~GUI_PrismaticJointParams()
{
	DELETESINGLE(mEditBox_SpringDamping);
	DELETESINGLE(mEditBox_SpringStiffness);
}

udword GUI_PrismaticJointParams::Init(EditBoxInterface& owner, IceWidget* parent, udword y)
{
	ASSERT(!mEditBox_SpringStiffness);
	ASSERT(!mEditBox_SpringDamping);

	const udword ParamsLabelWidth = 80;
	const udword EditBoxWidth = 100;
	const udword EditBoxHeight = 20;

	udword YOffset = mLimit.Init(owner, parent, y);

	const udword x0 = 4;
	const udword x1 = x0+ParamsLabelWidth+EditBoxWidth+10;

	mEditBox_SpringStiffness = CreateFloatEditBox(owner, parent, x0, YOffset, "Spring stiffness:", ParamsLabelWidth, EditBoxWidth, EditBoxHeight, 0, 0.0f);
	mEditBox_SpringDamping = CreateFloatEditBox(owner, parent, x1, YOffset, "Spring damping:", ParamsLabelWidth, EditBoxWidth, EditBoxHeight, 0, 0.0f);
	YOffset += EditBoxHeight + 4;

	return YOffset;
}

void GUI_PrismaticJointParams::SetVisible(bool is_visible)
{
	mLimit.SetVisible(is_visible);
	if(mEditBox_SpringStiffness)
		mEditBox_SpringStiffness->SetVisible(is_visible);
	if(mEditBox_SpringDamping)
		mEditBox_SpringDamping->SetVisible(is_visible);
}

void GUI_PrismaticJointParams::InitFrom(Pint_Joint* joint_api, PintJointHandle handle)
{
	mLimit.InitFrom(joint_api, handle, gPrismaticMinLimitLabel, gPrismaticMaxLimitLabel, 0);

	if(!joint_api)	// Might not be implemented by all plugins
	{
		if(mEditBox_SpringStiffness)
			mEditBox_SpringStiffness->SetEnabled(false);
		if(mEditBox_SpringDamping)
			mEditBox_SpringDamping->SetEnabled(false);
	}
	else
	{
		{
			PintSpring Spring;
			const bool EnableSpring = joint_api->GetSpring(handle, Spring);
			if(!EnableSpring)
				Spring.mStiffness = Spring.mDamping = 0.0f;

			EditBox_SetFloatValue(mEditBox_SpringStiffness, Spring.mStiffness, EnableSpring, null);
			EditBox_SetFloatValue(mEditBox_SpringDamping, Spring.mDamping, EnableSpring, null);
		}
	}
}

void GUI_PrismaticJointParams::Cancel()
{
	mLimit.Cancel();

	if(mEditBox_SpringStiffness)
		mEditBox_SpringStiffness->mSomethingChanged = false;
	if(mEditBox_SpringDamping)
		mEditBox_SpringDamping->mSomethingChanged = false;
}

bool GUI_PrismaticJointParams::SomethingChanged() const
{
	if(mLimit.SomethingChanged())
		return true;

	if(mEditBox_SpringStiffness && mEditBox_SpringStiffness->mSomethingChanged)
		return true;
	if(mEditBox_SpringDamping && mEditBox_SpringDamping->mSomethingChanged)
		return true;
	return false;
}

void GUI_PrismaticJointParams::SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)
{
	mLimit.SaveParams(joint_api, handle, wake_up_actors, 0);

	if(mEditBox_SpringStiffness && mEditBox_SpringDamping)
	{
		if(mEditBox_SpringStiffness->mSomethingChanged || mEditBox_SpringDamping->mSomethingChanged)
		{
			mEditBox_SpringStiffness->mSomethingChanged = false;
			mEditBox_SpringDamping->mSomethingChanged = false;

			if(joint_api)
			{
				PintSpring Spring;
				Spring.mStiffness = mEditBox_SpringStiffness->GetFloat();
				Spring.mDamping = mEditBox_SpringDamping->GetFloat();

				joint_api->SetSpring(handle, Spring);
				wake_up_actors = true;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

static const char* gDistanceMinLimitLabel = "Min limit:";
static const char* gDistanceMaxLimitLabel = "Max limit:";

GUI_DistanceJointParams::GUI_DistanceJointParams()
{
}

GUI_DistanceJointParams::~GUI_DistanceJointParams()
{
}

udword GUI_DistanceJointParams::Init(EditBoxInterface& owner, IceWidget* parent, udword y)
{
	return mLimit.Init(owner, parent, y);
}

void GUI_DistanceJointParams::SetVisible(bool is_visible)
{
	mLimit.SetVisible(is_visible);
}

void GUI_DistanceJointParams::InitFrom(Pint_Joint* joint_api, PintJointHandle handle)
{
	mLimit.InitFrom(joint_api, handle, gDistanceMinLimitLabel, gDistanceMaxLimitLabel, 0);
}

void GUI_DistanceJointParams::Cancel()
{
	mLimit.Cancel();
}

bool GUI_DistanceJointParams::SomethingChanged() const
{
	if(mLimit.SomethingChanged())
		return true;
	return false;
}

void GUI_DistanceJointParams::SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)
{
	mLimit.SaveParams(joint_api, handle, wake_up_actors, 0);
}

///////////////////////////////////////////////////////////////////////////////

static const char* gD6MinLimitLabel0 = "Min limit X:";
static const char* gD6MaxLimitLabel0 = "Max limit X:";
static const char* gD6MinLimitLabel1 = "Min limit Y:";
static const char* gD6MaxLimitLabel1 = "Max limit Y:";
static const char* gD6MinLimitLabel2 = "Min limit Z:";
static const char* gD6MaxLimitLabel2 = "Max limit Z:";

GUI_D6JointParams::GUI_D6JointParams()
{
}

GUI_D6JointParams::~GUI_D6JointParams()
{
}

udword GUI_D6JointParams::Init(EditBoxInterface& owner, IceWidget* parent, udword y)
{
	y = mLimitX.Init(owner, parent, y);
	y = mLimitY.Init(owner, parent, y);
	y = mLimitZ.Init(owner, parent, y);
	return y;
}

void GUI_D6JointParams::SetVisible(bool is_visible)
{
	mLimitX.SetVisible(is_visible);
	mLimitY.SetVisible(is_visible);
	mLimitZ.SetVisible(is_visible);
}

void GUI_D6JointParams::InitFrom(Pint_Joint* joint_api, PintJointHandle handle)
{
	mLimitX.InitFrom(joint_api, handle, gD6MinLimitLabel0, gD6MaxLimitLabel0, 0);
	mLimitY.InitFrom(joint_api, handle, gD6MinLimitLabel1, gD6MaxLimitLabel1, 1);
	mLimitZ.InitFrom(joint_api, handle, gD6MinLimitLabel2, gD6MaxLimitLabel2, 2);
}

void GUI_D6JointParams::Cancel()
{
	mLimitX.Cancel();
	mLimitY.Cancel();
	mLimitZ.Cancel();
}

bool GUI_D6JointParams::SomethingChanged() const
{
	if(mLimitX.SomethingChanged())
		return true;
	if(mLimitY.SomethingChanged())
		return true;
	if(mLimitZ.SomethingChanged())
		return true;
	return false;
}

void GUI_D6JointParams::SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)
{
	mLimitX.SaveParams(joint_api, handle, wake_up_actors, 0);
	mLimitY.SaveParams(joint_api, handle, wake_up_actors, 1);
	mLimitZ.SaveParams(joint_api, handle, wake_up_actors, 2);
}

///////////////////////////////////////////////////////////////////////////////

GUI_GearJointParams::GUI_GearJointParams() :
	mEditBox_GearRatio	(null)
{
}

GUI_GearJointParams::~GUI_GearJointParams()
{
	DELETESINGLE(mEditBox_GearRatio);
}

udword GUI_GearJointParams::Init(EditBoxInterface& owner, IceWidget* parent, udword y)
{
	ASSERT(!mEditBox_GearRatio);

	const udword ParamsLabelWidth = 100;
	const udword EditBoxHeight = 20;
	udword YOffset = y;

	mEditBox_GearRatio = CreateFloatEditBox(owner, parent, 4, YOffset, "Gear ratio:", ParamsLabelWidth, 100, EditBoxHeight, 0, 0.0f);
	YOffset += EditBoxHeight + 4;

	return YOffset;
}

void GUI_GearJointParams::SetVisible(bool is_visible)
{
	if(mEditBox_GearRatio)
		mEditBox_GearRatio->SetVisible(is_visible);
}

void GUI_GearJointParams::InitFrom(Pint_Joint* joint_api, PintJointHandle handle)
{
	if(mEditBox_GearRatio)
	{
		if(!joint_api)	// Might not be implemented by all plugins
		{
			mEditBox_GearRatio->SetEnabled(false);
		}
		else
		{
			float GearRatio;
			const bool EnableGearRatio = joint_api->GetGearRatio(handle, GearRatio);
			if(!EnableGearRatio)
				GearRatio = 0.0f;

			EditBox_SetFloatValue(mEditBox_GearRatio, GearRatio, EnableGearRatio, null);
		}
	}
}

void GUI_GearJointParams::Cancel()
{
	if(mEditBox_GearRatio)
		mEditBox_GearRatio->mSomethingChanged = false;
}

bool GUI_GearJointParams::SomethingChanged() const
{
	if(mEditBox_GearRatio && mEditBox_GearRatio->mSomethingChanged)
		return true;
	return false;
}

void GUI_GearJointParams::SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)
{
	if(mEditBox_GearRatio)
	{
		if(mEditBox_GearRatio->mSomethingChanged)
		{
			mEditBox_GearRatio->mSomethingChanged = false;

			if(joint_api)
			{
				const float GearRatio = mEditBox_GearRatio->GetFloat();
				joint_api->SetGearRatio(handle, GearRatio);
				wake_up_actors = true;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

GUI_RackJointParams::GUI_RackJointParams() :
	mEditBox_GearRatio	(null)
{
}

GUI_RackJointParams::~GUI_RackJointParams()
{
	DELETESINGLE(mEditBox_GearRatio);
}

udword GUI_RackJointParams::Init(EditBoxInterface& owner, IceWidget* parent, udword y)
{
	ASSERT(!mEditBox_GearRatio);

	const udword ParamsLabelWidth = 100;
	const udword EditBoxHeight = 20;
	udword YOffset = y;

	mEditBox_GearRatio = CreateFloatEditBox(owner, parent, 4, YOffset, "Gear ratio:", ParamsLabelWidth, 100, EditBoxHeight, 0, 0.0f);
	YOffset += EditBoxHeight + 4;

	return YOffset;
}

void GUI_RackJointParams::SetVisible(bool is_visible)
{
	if(mEditBox_GearRatio)
		mEditBox_GearRatio->SetVisible(is_visible);
}

void GUI_RackJointParams::InitFrom(Pint_Joint* joint_api, PintJointHandle handle)
{
	if(mEditBox_GearRatio)
	{
		if(!joint_api)	// Might not be implemented by all plugins
		{
			mEditBox_GearRatio->SetEnabled(false);
		}
		else
		{
			float GearRatio;
			const bool EnableGearRatio = joint_api->GetGearRatio(handle, GearRatio);
			if(!EnableGearRatio)
				GearRatio = 0.0f;

			EditBox_SetFloatValue(mEditBox_GearRatio, GearRatio, EnableGearRatio, null);
		}
	}
}

void GUI_RackJointParams::Cancel()
{
	if(mEditBox_GearRatio)
		mEditBox_GearRatio->mSomethingChanged = false;
}

bool GUI_RackJointParams::SomethingChanged() const
{
	if(mEditBox_GearRatio && mEditBox_GearRatio->mSomethingChanged)
		return true;
	return false;
}

void GUI_RackJointParams::SaveParams(Pint_Joint* joint_api, PintJointHandle handle, bool& wake_up_actors)
{
	if(mEditBox_GearRatio)
	{
		if(mEditBox_GearRatio->mSomethingChanged)
		{
			mEditBox_GearRatio->mSomethingChanged = false;

			if(joint_api)
			{
				const float GearRatio = mEditBox_GearRatio->GetFloat();
				joint_api->SetGearRatio(handle, GearRatio);
				wake_up_actors = true;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

