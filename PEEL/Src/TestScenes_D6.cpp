///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "GUI_Helpers.h"
#include "GLFontRenderer.h"

///////////////////////////////////////////////////////////////////////////////

static bool CheckD6Caps(const PintCaps& caps)
{
	if(!caps.mSupportRigidBodySimulation || !caps.mSupportD6Joints)
		return false;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool GetD6DynamicData(Pint& pint, PintJointHandle handle, PintD6DynamicData& data)
{
	Pint_Joint* API = pint.GetJointAPI();
	if(!API)
		return false;

	return API->GetD6DynamicData(handle, data);
}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_Prismatic = "D6 joint - prismatic joint with asymmetric limits.";

START_TEST(D6Joint_Prismatic, CATEGORY_JOINTS, gDesc_D6Joint_Prismatic)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-9.24f, 15.44f, 21.01f), Point(0.63f, -0.34f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		{
			const Point StaticPos(0.0f, 0.0f, 0.0f);
			const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
//			PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, StaticPos);
			PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= StaticObject;
			Desc.mObject1				= DynamicObject;
			Desc.mLocalPivot0.mPos		= Disp*0.5f;
			Desc.mLocalPivot1.mPos		= -Disp*0.5f;
			Desc.mLinearLimits.mMin.x	= -BoxSize;
			Desc.mLinearLimits.mMax.x	= BoxSize*2.0f;
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}

		{
			const Point StaticPos(5.0f, 5.0f, 5.0f);
			const Point Disp(0.0f, BoxSize*2.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= StaticObject;
			Desc.mObject1				= DynamicObject;
			Desc.mLocalPivot0.mPos		= Disp*0.5f;
			Desc.mLocalPivot1.mPos		= -Disp*0.5f;
			Desc.mLinearLimits.mMin.y	= -BoxSize;
			Desc.mLinearLimits.mMax.y	= BoxSize*2.0f;
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}

		{
			const Point StaticPos(10.0f, 10.0f, 10.0f);
			const Point Disp(0.0f, 0.0f, BoxSize*2.0f);
			const Point DynamicPos = StaticPos + Disp;

			PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= StaticObject;
			Desc.mObject1				= DynamicObject;
			Desc.mLocalPivot0.mPos		= Disp*0.5f;
			Desc.mLocalPivot1.mPos		= -Disp*0.5f;
			Desc.mLinearLimits.mMin.z	= -BoxSize;
			Desc.mLinearLimits.mMax.z	= BoxSize*2.0f;
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return true;
	}

END_TEST(D6Joint_Prismatic)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_PrismaticDrive = "D6 joint - prismatic drive.";

/*#define NB_PRESETS	7
static const udword	gPreset_GridSize[NB_PRESETS]   = { 1,     1,    10,    10,   16,    16,   1 };
static const udword	gPreset_Multiplier[NB_PRESETS] = { 1,     1,    1,     1,    1,     1,    4 };
static const bool	gPreset_Aggregates[NB_PRESETS] = { false, true, false, true, false, true, false };*/

class D6Joint_PrismaticDrive : public TestBase
{
	struct MyData : Allocateable
	{
		PintActorHandle	mDynamicObject;
		PintJointHandle	mJointHandle;
	};

//			IceComboBox*	mComboBox_Preset;
			EditBoxPtr		mEditBox_Stiffness;
			EditBoxPtr		mEditBox_Damping;
			EditBoxPtr		mEditBox_TargetVelocity;
			EditBoxPtr		mEditBox_ObstacleMass;
			CheckBoxPtr		mCheckBox_UsePositionDrive;

			float			mTargetVelocity;
			bool			mUsePosDrive;
	public:
							D6Joint_PrismaticDrive() :
/*								mComboBox_Preset(null),*/
								mTargetVelocity	(0.0f),
								mUsePosDrive	(false)	{										}
	virtual					~D6Joint_PrismaticDrive()	{										}
	virtual	const char*		GetName()			const	{ return "D6Joint_PrismaticDrive";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_D6Joint_PrismaticDrive;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;				}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 280;
		WD.mHeight	= 200;
		WD.mLabel	= "Prismatic Drive";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 100;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		const bool InitialEnabled = true;
		sdword y = 0;
		{
			mCheckBox_UsePositionDrive = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use position drive (else velocity drive)", &UIElems, false, null, null);
			mCheckBox_UsePositionDrive->SetEnabled(InitialEnabled);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Stiffness:", &UIElems);
			mEditBox_Stiffness = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_Stiffness->SetEnabled(InitialEnabled);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Damping:", &UIElems);
			mEditBox_Damping = helper.CreateEditBox(UI, 2, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_Damping->SetEnabled(InitialEnabled);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Target velocity:", &UIElems);
			mEditBox_TargetVelocity = helper.CreateEditBox(UI, 2, 4+OffsetX, y, EditBoxWidth, 20, "2.0", &UIElems, EDITBOX_FLOAT, null, null);
			mEditBox_TargetVelocity->SetEnabled(InitialEnabled);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Obstacle mass:", &UIElems);
			mEditBox_ObstacleMass = helper.CreateEditBox(UI, 2, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			mEditBox_ObstacleMass->SetEnabled(InitialEnabled);
			y += YStep;
		}
/*		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Presets:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				D6Joint_PrismaticDrive&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, D6Joint_PrismaticDrive& test) :
									IceComboBox(desc),
									mTest(test)			{}
				virtual			~MyComboBox()			{}
				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
						const udword SelectedIndex = GetSelectedIndex();
						const bool Enabled = SelectedIndex==GetItemCount()-1;
						mTest.mEditBox_Size->SetEnabled(Enabled);
						mTest.mEditBox_Multiplier->SetEnabled(Enabled);
						mTest.mCheckBox_Aggregates->SetEnabled(Enabled);

						if(!Enabled && SelectedIndex<NB_PRESETS)
						{
							mTest.mEditBox_Size->SetText(_F("%d", gPreset_GridSize[SelectedIndex]));
							mTest.mEditBox_Multiplier->SetText(_F("%d", gPreset_Multiplier[SelectedIndex]));
							mTest.mCheckBox_Aggregates->SetChecked(gPreset_Aggregates[SelectedIndex]);
						}
					}
				}
			};

			ComboBoxDesc CBBD;
			CBBD.mID		= 0;
			CBBD.mParent	= UI;
			CBBD.mX			= 4+OffsetX;
			CBBD.mY			= y;
			CBBD.mWidth		= 150;
			CBBD.mHeight	= 20;
			CBBD.mLabel		= "Presets";
			mComboBox_Preset = ICE_NEW(MyComboBox)(CBBD, *this);
			RegisterUIElement(mComboBox_Preset);
			mComboBox_Preset->Add("1 radgoll - regular");
			mComboBox_Preset->Add("1 radgoll - aggregate");
			mComboBox_Preset->Add("100 radgolls - regular");
			mComboBox_Preset->Add("100 radgolls - aggregates");
			mComboBox_Preset->Add("256 radgolls - regular");
			mComboBox_Preset->Add("256 radgolls - aggregates");
			mComboBox_Preset->Add("1 radgoll - constraints * 4");
			mComboBox_Preset->Add("User-defined");
			mComboBox_Preset->Select(0);
			mComboBox_Preset->SetVisible(true);
			mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
			y += YStep;

		}*/
		y += YStep;
		AddResetButton(UI, 4, y, 264);

		return null;
	}

/*	virtual	const char*		GetSubName()	const
	{
		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==0)
				return "Ragdoll";
			else if(SelectedIndex==1)
				return "Ragdoll_Aggregate";
			else if(SelectedIndex==2)
				return "Ragdolls_100";
			else if(SelectedIndex==3)
				return "Ragdolls_100_Aggregate";
			else if(SelectedIndex==4)
				return "Ragdolls_256";
			else if(SelectedIndex==5)
				return "Ragdolls_256_Aggregate";
			else if(SelectedIndex==6)
				return "Ragdoll_MultipleConstraints";
		}
		return null;
	}*/

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.32f, 4.48f, 12.28f), Point(-0.03f, -0.26f, -0.96f));
		SetDefEnv(desc, true);
	}

	virtual void		Close(Pint& pint)
	{
		MyData* UserData = reinterpret_cast<MyData*>(pint.mUserData);
		DELETESINGLE(UserData);
		pint.mUserData = null;

		TestBase::Close(pint);
	}

	virtual	bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		mUsePosDrive = mCheckBox_UsePositionDrive ? mCheckBox_UsePositionDrive->IsChecked() : false;

		const float Stiffness = GetFloat(0.0f, mEditBox_Stiffness);
		const float Damping = GetFloat(0.0f, mEditBox_Damping);
		const float ObstacleMass = GetFloat(0.0f, mEditBox_ObstacleMass);
		mTargetVelocity = GetFloat(0.0f, mEditBox_TargetVelocity);

		MyData* UserData = ICE_NEW(MyData);
		pint.mUserData = UserData;

//		mCreateDefaultEnvironment = false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);
			const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;
			const Point ObstaclePos = StaticPos + Point(BoxSize*6.0f-0.5f, 0.0f, 0.0f);

			PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);
			PintActorHandle Obstacle = null;
			if(ObstacleMass==0.0f)
				Obstacle = CreateStaticObject(pint, &BoxDesc, ObstaclePos);
			else if(ObstacleMass>=0.0f)
				Obstacle = CreateSimpleObject(pint, &BoxDesc, ObstacleMass, ObstaclePos);

			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= StaticObject;
			Desc.mObject1				= DynamicObject;
			Desc.mLocalPivot0.mPos		= Disp*0.5f;
			Desc.mLocalPivot1.mPos		= -Disp*0.5f;
			Desc.mLinearLimits.mMin.x	= -FLT_MAX;
			Desc.mLinearLimits.mMax.x	= FLT_MAX;
//			Desc.mLinearLimits.mMin.x	= -FLT_MAX;
//			Desc.mLinearLimits.mMax.x	= 1.0f;

			Desc.mMotorFlags			= PINT_D6_MOTOR_DRIVE_X;
/*			if(mUsePosDrive)
			{
				// Position drive
				Desc.mMotorStiffness		= 100000.0f;
				Desc.mMotorDamping			= 0.0f;

	//			Desc.mMotorStiffness		= 10000.0f;
	//			Desc.mMotorDamping			= 100.0f;

	//			Desc.mMotorStiffness		= 100.0f;
	//			Desc.mMotorDamping			= 10.0f;
			}
			else
			{
				// Velocity drive
				Desc.mMotorStiffness		= 0.0f;
				Desc.mMotorDamping			= 100000.0f;
			}*/
			Desc.mMotorStiffness		= Stiffness;
			Desc.mMotorDamping			= Damping;

			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);

			UserData->mDynamicObject = DynamicObject;
			UserData->mJointHandle = JointHandle;

			if(0)
			{
				PR Pose;
				Pose.Identity();
				Pose.mPos.x = 2.0f;
				pint.SetDrivePosition(JointHandle, Pose);
			}
		}
		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
		const MyData* UserData = reinterpret_cast<const MyData*>(pint.mUserData);
		if(UserData->mDynamicObject)
		{
			const PintJointHandle JointHandle = UserData->mJointHandle;

			if(mUsePosDrive)
			{
				const PR CurrentPose = pint.GetWorldTransform(UserData->mDynamicObject);
	//			const PR CurrentPose = pint.GetDrivePosition(JointHandle);
				const float TargetPos = (CurrentPose.mPos.x - 2.0f) + (1.0f/60.0f)*mTargetVelocity;

				PR Pose;
				Pose.Identity();
	//			Pose.mPos.x = mCurrentTime*mTargetVelocity;
				Pose.mPos.x = TargetPos;
				pint.SetDrivePosition(JointHandle, Pose);
			}
			else
			{
				pint.SetDriveVelocity(JointHandle, Point(mTargetVelocity, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f));
	//			pint.SetLinearVelocity(UserData->mDynamicObject, Point(mTargetVelocity, 0.0f, 0.0f));
			}
		}
		return TestBase::Update(pint, dt);
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const MyData* UserData = reinterpret_cast<const MyData*>(pint.mUserData);
		const PintActorHandle Handle = UserData->mDynamicObject;
		if(Handle)
		{
			const Point LinVel = pint.GetLinearVelocity(Handle);
			renderer.print(0.0f, y, text_scale, _F("Linear velocity: %.3f | %.3f | %.3f\n", LinVel.x, LinVel.y, LinVel.z));
			y -= text_scale;
		}
		return y;
	}

	virtual	void		OnObjectReleased(Pint& pint, PintActorHandle removed_object)	
	{
		MyData* UserData = reinterpret_cast<MyData*>(pint.mUserData);
		if(UserData && UserData->mDynamicObject==removed_object)
		{
			UserData->mDynamicObject = null;
		}
	}

}D6Joint_PrismaticDrive;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_PrismaticPosDrive = "D6 joint - prismatic position drive.";

class D6Joint_PrismaticPosDrive : public TestBase
{
			EditBoxPtr		mEditBox_Stiffness;
			EditBoxPtr		mEditBox_Damping;
			SliderPtr		mSlider;
	public:
							D6Joint_PrismaticPosDrive()	{											}
	virtual					~D6Joint_PrismaticPosDrive(){											}
	virtual	const char*		GetName()			const	{ return "D6Joint_PrismaticPosDrive";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_D6Joint_PrismaticPosDrive;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;					}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 280;
		WD.mHeight	= 200;
		WD.mLabel	= "Prismatic Drive";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 100;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		const bool InitialEnabled = true;
		sdword y = 0;
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Stiffness:", &UIElems);
			mEditBox_Stiffness = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_Stiffness->SetEnabled(InitialEnabled);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Damping:", &UIElems);
			mEditBox_Damping = helper.CreateEditBox(UI, 2, 4+OffsetX, y, EditBoxWidth, 20, "100.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_Damping->SetEnabled(InitialEnabled);
			y += YStep;

			y += YStep;
			y += YStep;
			SliderDesc SD;
			SD.mStyle	= SLIDER_HORIZONTAL;
			SD.mID		= 0;
			SD.mParent	= UI;
			SD.mX		= 0;
			SD.mY		= y;
			SD.mWidth		= 200;
			SD.mHeight		= 20;
			SD.mLabel		= "test";
			mSlider = ICE_NEW(IceSlider)(SD);
			mSlider->SetRange(0.0f, 1.0f, 100);
			mSlider->SetValue(0.5f);

//			mSlider->SetIntRange(0, 100);
//			mSlider->SetIntValue(50);

//			mSlider->SetSteps(int line, int page);
			UIElems.Register(mSlider);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 264);

		return null;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(0.18f, 2.63f, 8.56f), Point(-0.05f, -0.23f, -0.97f));
//		mCreateDefaultEnvironment = false;
		SetDefEnv(desc, true);
	}

	virtual	bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float Length = 8.0f;
		const Point RodExtents(Length*0.5f, 0.2f, 0.2f);
		const Point CursorExtents(0.2f, 0.2f, 0.2f);

		const Point RodPos(0.0f, RodExtents.y, 0.0f);
		const Point CursorPos(0.0f, RodExtents.y+CursorExtents.y, 0.0f);

		PintActorHandle Rod;
		{
			PINT_BOX_CREATE BoxDesc(RodExtents);
			BoxDesc.mRenderer = CreateBoxRenderer(RodExtents);

//			Rod = CreateStaticObject(pint, &BoxDesc, RodPos);
//			Rod = CreateDynamicObject(pint, &BoxDesc, RodPos);
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 10.0f;
			ObjectDesc.mPosition	= RodPos;
			Rod = CreatePintObject(pint, ObjectDesc);
		}

		PintActorHandle Cursor;
		{
			PINT_BOX_CREATE BoxDesc(CursorExtents);
			BoxDesc.mRenderer = CreateBoxRenderer(CursorExtents);

			Cursor = CreateDynamicObject(pint, &BoxDesc, CursorPos);
		}

		{
			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= Rod;
			Desc.mObject1				= Cursor;
			Desc.mLocalPivot0.mPos		= Point(0.0f, RodExtents.y, 0.0f);
			Desc.mLocalPivot1.mPos		= Point(0.0f, -CursorExtents.y, 0.0f);
			Desc.mLinearLimits.mMin.x	= -Length*0.5f + CursorExtents.x;
			Desc.mLinearLimits.mMax.x	= Length*0.5f - CursorExtents.x;
			Desc.mMotorFlags			= PINT_D6_MOTOR_DRIVE_X;
			Desc.mMotorStiffness		= GetFloat(0.0f, mEditBox_Stiffness);
			Desc.mMotorDamping			= GetFloat(0.0f, mEditBox_Damping);

			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}
		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
		if(mSlider)
		{
			float V = mSlider->GetValue();
//			printf("%f\n", V);
			const float TargetX = (V-0.5f)*8.0f;

			PintJointHandle JointHandle = PintJointHandle(pint.mUserData);

			PR Pose;
			Pose.Identity();
			Pose.mPos.x = TargetX;
			pint.SetDrivePosition(JointHandle, Pose);
		}
		return TestBase::Update(pint, dt);
	}

}D6Joint_PrismaticPosDrive;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_PointInPlane = "D6 joint - point-in-plane joint with asymmetric limits.";

START_TEST(D6Joint_PointInPlane, CATEGORY_JOINTS, gDesc_D6Joint_PointInPlane)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-9.24f, 15.44f, 21.01f), Point(0.63f, -0.34f, -0.70f));

		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);
			const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
//			PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, StaticPos);
			PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= StaticObject;
			Desc.mObject1				= DynamicObject;
			Desc.mLocalPivot0.mPos		= Disp*0.5f;
			Desc.mLocalPivot1.mPos		= -Disp*0.5f;
			Desc.mLinearLimits.mMin.x	= -BoxSize;
			Desc.mLinearLimits.mMax.x	= BoxSize*2.0f;
			Desc.mLinearLimits.mMin.y	= -BoxSize;
			Desc.mLinearLimits.mMax.y	= BoxSize*2.0f;
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}

		{
			const Point StaticPos(5.0f, 5.0f, 5.0f);
			const Point Disp(0.0f, BoxSize*2.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
//			PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, StaticPos);
			PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= StaticObject;
			Desc.mObject1				= DynamicObject;
			Desc.mLocalPivot0.mPos		= Disp*0.5f;
			Desc.mLocalPivot1.mPos		= -Disp*0.5f;
			Desc.mLinearLimits.mMin.y	= -BoxSize;
			Desc.mLinearLimits.mMax.y	= BoxSize*2.0f;
			Desc.mLinearLimits.mMin.z	= -BoxSize;
			Desc.mLinearLimits.mMax.z	= BoxSize*2.0f;
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}

		{
			const Point StaticPos(10.0f, 10.0f, 10.0f);
			const Point Disp(0.0f, 0.0f, BoxSize*2.0f);
			const Point DynamicPos = StaticPos + Disp;

			PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
//			PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, StaticPos);
			PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= StaticObject;
			Desc.mObject1				= DynamicObject;
			Desc.mLocalPivot0.mPos		= Disp*0.5f;
			Desc.mLocalPivot1.mPos		= -Disp*0.5f;
			Desc.mLinearLimits.mMin.z	= -BoxSize;
			Desc.mLinearLimits.mMax.z	= BoxSize*2.0f;
			Desc.mLinearLimits.mMin.x	= -BoxSize;
			Desc.mLinearLimits.mMax.x	= BoxSize*2.0f;
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return true;
	}

END_TEST(D6Joint_PointInPlane)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_PointInBox = "D6 joint - point-in-box joint with asymmetric limits.";

START_TEST(D6Joint_PointInBox, CATEGORY_JOINTS, gDesc_D6Joint_PointInBox)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-5.84f, 8.10f, 9.45f), Point(0.66f, -0.42f, -0.62f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);
			const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
//			PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, StaticPos);
			PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_D6_JOINT_CREATE Desc;
			Desc.mObject0				= StaticObject;
			Desc.mObject1				= DynamicObject;
			Desc.mLocalPivot0.mPos		= Disp*0.5f;
			Desc.mLocalPivot1.mPos		= -Disp*0.5f;
			Desc.mLinearLimits.mMin.x	= -BoxSize;
			Desc.mLinearLimits.mMax.x	= BoxSize*2.0f;
			Desc.mLinearLimits.mMin.y	= -BoxSize;
			Desc.mLinearLimits.mMax.y	= BoxSize*2.0f;
			Desc.mLinearLimits.mMin.z	= -BoxSize*4.0f;
			Desc.mLinearLimits.mMax.z	= BoxSize*4.0f;
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return true;
	}

END_TEST(D6Joint_PointInBox)

///////////////////////////////////////////////////////////////////////////////

static PintJointHandle CreateD6Hinge(Pint& pint, const PINT_BOX_CREATE& box_desc, const Point& pos, const Matrix3x3& rot, float box_size, float min_angle, float max_angle)
{
	const Point Disp(box_size*2.0f, 0.0f, 0.0f);
	const Point DynamicPos = pos + Disp;

	PintActorHandle StaticObject = CreateStaticObject(pint, &box_desc, pos);
//	PintActorHandle StaticObject = CreateDynamicObject(pint, &box_desc, pos);
	PintActorHandle DynamicObject = CreateDynamicObject(pint, &box_desc, DynamicPos);

	PINT_D6_JOINT_CREATE Desc;
	Desc.mObject0				= StaticObject;
	Desc.mObject1				= DynamicObject;
	Desc.mLocalPivot0.mPos		= Disp*0.5f;
	Desc.mLocalPivot1.mPos		= -Disp*0.5f;
	Desc.mLocalPivot0.mRot		= rot;
	Desc.mLocalPivot1.mRot		= rot;
//	Desc.mAngularLimitsMin.x	= min_angle*DEGTORAD;
//	Desc.mAngularLimitsMax.x	= max_angle*DEGTORAD;
	Desc.mMinTwist				= min_angle*DEGTORAD;
	Desc.mMaxTwist				= max_angle*DEGTORAD;
	PintJointHandle JointHandle = pint.CreateJoint(Desc);
	ASSERT(JointHandle);
	return JointHandle;
}

static const char* gDesc_D6Joint_Hinge = "D6 joint - hinge joint with asymmetric limits around the twist axis.";

START_TEST(D6Joint_HingeUsingTwist, CATEGORY_JOINTS, gDesc_D6Joint_Hinge)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mCamera[1] = PintCameraPose(Point(4.63f, 2.89f, -1.95f), Point(-0.64f, -0.47f, -0.61f));
		desc.mCamera[2] = PintCameraPose(Point(4.29f, 2.65f, -6.86f), Point(-0.64f, -0.32f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.5f, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		const float DistBetweenJoints = 5.0f;
		const float MinAngle = -45.0f * 0.5f;
		const float MaxAngle = 45.0f;

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);
			PintJointHandle h = CreateD6Hinge(pint, BoxDesc, StaticPos, Matrix3x3(Idt), BoxSize, MinAngle, MaxAngle);
			pint.mUserData = h;
		}

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, -DistBetweenJoints);
			// Rotate joint frame so that X (twist axis) == Y
			Matrix3x3 M;
			M.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
			PintJointHandle h = CreateD6Hinge(pint, BoxDesc, StaticPos, M, BoxSize, MinAngle, MaxAngle);
			(void)h;
		}

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, -DistBetweenJoints*2.0f);
			// Rotate joint frame so that X (twist axis) == Z
			Matrix3x3 M;
			M.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
			PintJointHandle h = CreateD6Hinge(pint, BoxDesc, StaticPos, M, BoxSize, MinAngle, MaxAngle);
			(void)h;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Twist angle: %.5f\n", Data.mTwistAngle * RADTODEG));
		return y - text_scale;
	}

END_TEST(D6Joint_HingeUsingTwist)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_Hinge2 = "D6 joint - hinge joint with PI limits around the twist axis.";

START_TEST(D6Joint_HingeUsingTwist2, CATEGORY_JOINTS, gDesc_D6Joint_Hinge2)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.5f, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		if(1)
		{
			Matrix3x3 M;
			M.RotX(45.0f * DEGTORAD);
			BoxDesc.mLocalRot = M;
		}

//		const float DistBetweenJoints = 5.0f;
		const float MinAngle = -180.0f;
		const float MaxAngle = 180.0f;

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);
			PintJointHandle h = CreateD6Hinge(pint, BoxDesc, StaticPos, Matrix3x3(Idt), BoxSize, MinAngle, MaxAngle);
			pint.mUserData = h;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Twist angle: %.5f\n", Data.mTwistAngle * RADTODEG));
		return y - text_scale;
	}

END_TEST(D6Joint_HingeUsingTwist2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_Hinge3 = "D6 joint - hinge joint with PI limits around the twist axis, and a rotated joint frame.";

START_TEST(D6Joint_HingeUsingTwist3, CATEGORY_JOINTS, gDesc_D6Joint_Hinge3)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.5f, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;
		const float MinAngle = -180.0f;
		const float MaxAngle = 180.0f;

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);
			Matrix3x3 M;
			M.RotX(45.0f*DEGTORAD);
			PintJointHandle h = CreateD6Hinge(pint, BoxDesc, StaticPos, M, BoxSize, MinAngle, MaxAngle);
			pint.mUserData = h;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Twist angle: %.5f\n", Data.mTwistAngle * RADTODEG));
		return y - text_scale;
	}

END_TEST(D6Joint_HingeUsingTwist3)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_PrismaticTwist = "D6 joint - prismatic twist";

START_TEST(D6Joint_PrismaticTwist, CATEGORY_JOINTS, gDesc_D6Joint_PrismaticTwist)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.5f, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);

				const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
				const Point DynamicPos = StaticPos + Disp;

				PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			//	PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, pos);
				PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= StaticObject;
				Desc.mObject1				= DynamicObject;
				Desc.mLocalPivot0.mPos		= Disp*0.5f;
				Desc.mLocalPivot1.mPos		= -Disp*0.5f;
				Desc.mMinTwist				= -180*DEGTORAD;
				Desc.mMaxTwist				= 180*DEGTORAD;
//				Desc.mMaxSwingY				= 45*DEGTORAD;
//				Desc.mMaxSwingZ				= 45*DEGTORAD;
				Desc.mLinearLimits.mMin.x	= 0.0f;
				Desc.mLinearLimits.mMax.x	= 2.0f;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}

		return true;
	}

END_TEST(D6Joint_PrismaticTwist)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_SwingY = "D6 joint - swing Y";

START_TEST(D6Joint_SwingY, CATEGORY_JOINTS, gDesc_D6Joint_SwingY)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.5f, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);

				const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
				const Point DynamicPos = StaticPos + Disp;

				PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			//	PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, pos);
				PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= StaticObject;
				Desc.mObject1				= DynamicObject;
				Desc.mLocalPivot0.mPos		= Disp*0.5f;
				Desc.mLocalPivot1.mPos		= -Disp*0.5f;
				Desc.mMaxSwingY				= 45.0f*DEGTORAD;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Swing Y angle: %.5f\n", Data.mSwingYAngle * RADTODEG));
		return y - text_scale;
	}

END_TEST(D6Joint_SwingY)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_SwingZ = "D6 joint - swing Z";

START_TEST(D6Joint_SwingZ, CATEGORY_JOINTS, gDesc_D6Joint_SwingZ)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.5f, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);

				const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
				const Point DynamicPos = StaticPos + Disp;

				PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			//	PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, pos);
				PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= StaticObject;
				Desc.mObject1				= DynamicObject;
				Desc.mLocalPivot0.mPos		= Disp*0.5f;
				Desc.mLocalPivot1.mPos		= -Disp*0.5f;
				Desc.mMaxSwingZ				= 90.0f*DEGTORAD;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Swing Z angle: %.5f\n", Data.mSwingZAngle * RADTODEG));
		return y - text_scale;
	}

END_TEST(D6Joint_SwingZ)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_SwingCone = "D6 joint - swing YZ cone";

START_TEST(D6Joint_SwingYZCone, CATEGORY_JOINTS, gDesc_D6Joint_SwingCone)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.15f, BoxSize*0.15f);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;
		const Point LinVel(0.0f, 0.0f, 0.0f);
//		const Point LinVel(0.0f, 1.0f, 0.0f);

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);

				const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
				const Point DynamicPos = StaticPos + Disp;

				PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			//	PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, pos);
				PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos, NULL, &LinVel);

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= StaticObject;
				Desc.mObject1				= DynamicObject;
				Desc.mLocalPivot0.mPos		= Disp*0.5f;
				Desc.mLocalPivot1.mPos		= -Disp*0.5f;
				Desc.mMaxSwingY				= 20.0f*DEGTORAD;
				Desc.mMaxSwingZ				= 30.0f*DEGTORAD;
//				Desc.mMaxSwingZ				= 120.0f*DEGTORAD;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Swing Y angle: %.5f\n", Data.mSwingYAngle * RADTODEG));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Swing Z angle: %.5f\n", Data.mSwingZAngle * RADTODEG));	y -= text_scale;
		return y;
	}

END_TEST(D6Joint_SwingYZCone)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_SwingCone2 = "D6 joint - swing YZ cone";

START_TEST(D6Joint_SwingYZCone2, CATEGORY_JOINTS, gDesc_D6Joint_SwingCone2)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.15f, BoxSize*0.15f);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;
		const Point LinVel(0.0f, 1.0f, 0.0f);

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);

				const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
				const Point DynamicPos = StaticPos + Disp;

				PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			//	PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, pos);
				PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos, NULL, &LinVel);

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= StaticObject;
				Desc.mObject1				= DynamicObject;
				Desc.mLocalPivot0.mPos		= Disp*0.5f;
				Desc.mLocalPivot1.mPos		= -Disp*0.5f;
				Desc.mMaxSwingY				= 20.0f*DEGTORAD;
				Desc.mMaxSwingZ				= 120.0f*DEGTORAD;
//				Desc.mMaxSwingZ				= 90.0f*DEGTORAD;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Twist angle: %.5f\n", Data.mTwistAngle * RADTODEG));		y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Swing Y angle: %.5f\n", Data.mSwingYAngle * RADTODEG));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Swing Z angle: %.5f\n", Data.mSwingZAngle * RADTODEG));	y -= text_scale;
		return y;
	}

END_TEST(D6Joint_SwingYZCone2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_SwingY_ZFree = "D6 joint - swing Y, Z free";

START_TEST(D6Joint_SwingYZFree, CATEGORY_JOINTS, gDesc_D6Joint_SwingY_ZFree)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.5f, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;
		const Point LinVel(0.0f, 0.0f, 1.0f);

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);

				const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
				const Point DynamicPos = StaticPos + Disp;

				PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			//	PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, pos);
				PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos, null, &LinVel);

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= StaticObject;
				Desc.mObject1				= DynamicObject;
				Desc.mLocalPivot0.mPos		= Disp*0.5f;
				Desc.mLocalPivot1.mPos		= -Disp*0.5f;
				Desc.mMaxSwingY				= 45.0f*DEGTORAD;
				Desc.mMaxSwingZ				= -1.0f;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Swing Y angle: %.5f\n", Data.mSwingYAngle * RADTODEG));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Swing Z angle: %.5f\n", Data.mSwingZAngle * RADTODEG));	y -= text_scale;
		return y;
	}

END_TEST(D6Joint_SwingYZFree)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_SphericalNoLimits = "D6 joint - spherical, no limits";

START_TEST(D6Joint_SphericalNoLimits, CATEGORY_JOINTS, gDesc_D6Joint_SphericalNoLimits)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.15f, BoxSize*0.15f);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);

				const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
				const Point DynamicPos = StaticPos + Disp;
//				const Point LinVel(0.0f, 1.0f, 0.0f);
//				const Point LinVel(0.0f, 0.0f, 1.0f);
				const Point LinVel(0.0f, 1.0f, 0.1f);

				PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			//	PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, pos);
				PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos, null, &LinVel);

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= StaticObject;
				Desc.mObject1				= DynamicObject;
				Desc.mLocalPivot0.mPos		= Disp*0.5f;
				Desc.mLocalPivot1.mPos		= -Disp*0.5f;
//				Desc.mMinTwist				= 1.0f;
//				Desc.mMaxTwist				= -1.0f;
				Desc.mMaxSwingY				= -1.0f;
				Desc.mMaxSwingZ				= -1.0f;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Twist angle: %.5f\n", Data.mTwistAngle * RADTODEG));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Swing Y angle: %.5f\n", Data.mSwingYAngle * RADTODEG));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Swing Z angle: %.5f\n", Data.mSwingZAngle * RADTODEG));	y -= text_scale;
		return y;
	}

END_TEST(D6Joint_SphericalNoLimits)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_D6Joint_AsymmetricSwingCone = "D6 joint - asymmetric swing YZ cone";

START_TEST(D6Joint_AsymmetricSwingYZCone, CATEGORY_JOINTS, gDesc_D6Joint_AsymmetricSwingCone)

	static	const float DesiredMinSwingY;
	static	const float DesiredMaxSwingY;
	static	const float DesiredMinSwingZ;
	static	const float DesiredMaxSwingZ;
	static	const float APIMaxY;
	static	const float APIMaxZ;
	static	const float APIRotY;
	static	const float APIRotZ;

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.97f, 2.93f, 4.01f), Point(-0.61f, -0.36f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CheckD6Caps(caps))
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize*0.15f, BoxSize*0.15f);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//		const float DistBetweenJoints = 5.0f;
		const Point LinVel(0.0f, 0.0f, 0.0f);
//		const Point LinVel(0.0f, 1.0f, 0.0f);

		Matrix3x3 RotY;	RotY.RotY(APIRotY*DEGTORAD);
		Matrix3x3 RotZ;	RotZ.RotZ(APIRotZ*DEGTORAD);
		Matrix3x3 Rot = RotY * RotZ;

		if(1)
		{
			const Point StaticPos(0.0f, BoxSize, 0.0f);

				const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
				const Point DynamicPos = StaticPos + Disp;

				PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			//	PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, pos);
				PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos, NULL, &LinVel);

				PINT_D6_JOINT_CREATE Desc;
				Desc.mObject0				= StaticObject;
				Desc.mObject1				= DynamicObject;
				Desc.mLocalPivot0.mPos		= Disp*0.5f;
				Desc.mLocalPivot1.mPos		= -Disp*0.5f;
				Desc.mLocalPivot0.mRot		= Rot;
				Desc.mMaxSwingY				= APIMaxY*DEGTORAD;
				Desc.mMaxSwingZ				= APIMaxZ*DEGTORAD;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

			pint.mUserData = JointHandle;
		}

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintJointHandle Handle = PintJointHandle(pint.mUserData);

		PintD6DynamicData Data;
		if(!GetD6DynamicData(pint, Handle, Data))
			return y;

		renderer.print(0.0f, y, text_scale, _F("Swing Y angle: %.5f\n", Data.mSwingYAngle * RADTODEG + APIRotY));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Swing Z angle: %.5f\n", Data.mSwingZAngle * RADTODEG + APIRotZ));	y -= text_scale;
		return y;
	}

END_TEST(D6Joint_AsymmetricSwingYZCone)

const float D6Joint_AsymmetricSwingYZCone::DesiredMinSwingY = -45.0f;
const float D6Joint_AsymmetricSwingYZCone::DesiredMaxSwingY = 45.0f;
const float D6Joint_AsymmetricSwingYZCone::DesiredMinSwingZ = -5.0f;
const float D6Joint_AsymmetricSwingYZCone::DesiredMaxSwingZ = 65.0f;
const float D6Joint_AsymmetricSwingYZCone::APIMaxY = (DesiredMaxSwingY - DesiredMinSwingY)*0.5f;
const float D6Joint_AsymmetricSwingYZCone::APIMaxZ = (DesiredMaxSwingZ - DesiredMinSwingZ)*0.5f;
const float D6Joint_AsymmetricSwingYZCone::APIRotY = (DesiredMaxSwingY + DesiredMinSwingY)*0.5f;
const float D6Joint_AsymmetricSwingYZCone::APIRotZ = (DesiredMaxSwingZ + DesiredMinSwingZ)*0.5f;

///////////////////////////////////////////////////////////////////////////////

