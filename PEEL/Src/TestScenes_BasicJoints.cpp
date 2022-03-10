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
//#include "TestScenes_VehiclesBase.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
//#include "Loader_Bin.h"
#include "GUI_Helpers.h"
//#include "Cylinder.h"
#include "GLFontRenderer.h"
//#include "ZB2Import.h"

///////////////////////////////////////////////////////////////////////////////

static float PrintTwistAngle(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
{
	const PintJointHandle Handle = PintJointHandle(pint.mUserData);

	float TwistAngle;
	if(!GetHingeTwistAngle(pint, Handle, TwistAngle))
		return y;

	renderer.print(0.0f, y, text_scale, _F("Angle: %.5f\n", TwistAngle * RADTODEG));
	return y - text_scale;
}

///////////////////////////////////////////////////////////////////////////////

static bool CreateJointedCubes(Pint& pint, const PintCaps& caps, const Point& static_pos, const Point& local_axis, float limit)
{
	if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation || !caps.mSupportCollisionGroups)
		return false;

	const PintDisabledGroups DG(1, 2);
	pint.SetDisabledGroups(1, &DG);

	const float BoxSize = 1.0f;
	const Point Extents(BoxSize, BoxSize, BoxSize);

	PINT_BOX_CREATE BoxDesc(Extents);
	BoxDesc.mRenderer = CreateBoxRenderer(Extents);

	const Point Disp(BoxSize*2.0f, -BoxSize*2.0f, 0.0f);
	const Point DynamicPos = static_pos + Disp;

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mMass			= 0.0f;
	ObjectDesc.mPosition		= static_pos;
	ObjectDesc.mCollisionGroup	= 1;
	const PintActorHandle StaticObject = CreatePintObject(pint, ObjectDesc);
	ObjectDesc.mMass			= 1.0f;
	ObjectDesc.mPosition		= DynamicPos;
	ObjectDesc.mCollisionGroup	= 2;
	const PintActorHandle DynamicObject = CreatePintObject(pint, ObjectDesc);

	PINT_HINGE_JOINT_CREATE Desc;
	Desc.mObject0		= StaticObject;
	Desc.mObject1		= DynamicObject;
	Desc.mLocalPivot0	= Disp*0.5f;
	Desc.mLocalPivot1	= -Disp*0.5f;
	Desc.mLocalAxis0	= local_axis;
	Desc.mLocalAxis1	= local_axis;
	if(limit>0.0f)
		Desc.mLimits.Set(degToRad(-limit), degToRad(limit));

	PintJointHandle JointHandle = pint.CreateJoint(Desc);
	ASSERT(JointHandle);
	pint.mUserData = JointHandle;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BasicJointAPI = "Test for basic joint API. Creates a single joint of a chosen type. Each test has one dynamic object linked to one \
static object. You can manipulate the dynamic object with the mouse.";

class BasicJointAPI : public TestBase
{
			ComboBoxPtr		mComboBox_Preset;
	public:
							BasicJointAPI()				{									}
	virtual					~BasicJointAPI()			{									}
	virtual	const char*		GetName()			const	{ return "BasicJointAPI";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_BasicJointAPI;		}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS_BASICS;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 100;
		WD.mLabel	= "Basic joint API";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		//const sdword EditBoxWidth = 100;
		const sdword LabelWidth = 120;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Joint type:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				BasicJointAPI&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, BasicJointAPI& test) : IceComboBox(desc), mTest(test)	{}
				virtual			~MyComboBox()																				{}

				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
//						const udword SelectedIndex = GetSelectedIndex();
//						const bool Enabled = SelectedIndex==GetItemCount()-1;
						mTest.mMustResetTest = true;
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
			mComboBox_Preset->Add("Fixed joint");
			mComboBox_Preset->Add("Distance joint");
			mComboBox_Preset->Add("Spherical joint");
			mComboBox_Preset->Add("Hinge joint");
			mComboBox_Preset->Add("Prismatic joint");
			mComboBox_Preset->Select(3);
			mComboBox_Preset->SetVisible(true);
			mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
			y += YStep;

			y += YStep;
			AddResetButton(UI, 4, y, WD.mWidth-4*2*2);
		}
		return null;
	}

	virtual	const char*		GetSubName()	const
	{
		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==0)
				return "FixedJoint";
			if(SelectedIndex==1)
				return "DistanceJoint";
			if(SelectedIndex==2)
				return "SphericalJoint";
			if(SelectedIndex==3)
				return "HingeJoint";
			if(SelectedIndex==4)
				return "PrismaticJoint";
		}
		return null;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		if(!mComboBox_Preset)
			return;
		const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();

		if(SelectedIndex==0)
			desc.mCamera[0] = PintCameraPose(Point(7.10f, 22.91f, -10.82f), Point(-0.63f, -0.29f, 0.72f));
		else if(SelectedIndex==1)
			desc.mCamera[0] = PintCameraPose(Point(-0.61f, 20.57f, 6.71f), Point(0.12f, -0.20f, -0.97f));
		else if(SelectedIndex==2)
			desc.mCamera[0] = PintCameraPose(Point(0.10f, 14.65f, 8.24f), Point(-0.09f, 0.39f, -0.92f));
		else if(SelectedIndex==3)
			desc.mCamera[0] = PintCameraPose(Point(5.93f, 20.16f, 5.70f), Point(-0.67f, -0.28f, -0.68f));
		else if(SelectedIndex==4)
			desc.mCamera[0] = PintCameraPose(Point(4.13f, 19.60f, 8.74f), Point(-0.17f, 0.09f, -0.98f));

		SetDefEnv(desc, false);
	}

	virtual	bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		if(!mComboBox_Preset)
			return false;
		const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();

		if(SelectedIndex==0)
		{
			if(!caps.mSupportFixedJoints)
				return false;

			const float BoxSize = 1.0f;
			const Point Extents(BoxSize, BoxSize, BoxSize);

			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateBoxRenderer(Extents);

			const Point StaticPos(0.0f, 20.0f, 0.0f);
			const Point DynamicPos = StaticPos - Extents*2.0f;

			const PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			const PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_FIXED_JOINT_CREATE Desc;
			Desc.mObject0		= StaticObject;
			Desc.mObject1		= DynamicObject;
			Desc.mLocalPivot0	= -Extents;
			Desc.mLocalPivot1	= Extents;
			const PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		else if(SelectedIndex==1)
		{
			if(!caps.mSupportDistanceJoints)
				return false;

			const float Radius = 1.0f;
			PINT_SPHERE_CREATE SphereDesc(Radius);
			SphereDesc.mRenderer = CreateSphereRenderer(Radius);

			const Point StaticPos(0.0f, 20.0f, 0.0f);
			const Point DynamicPos = StaticPos + Point(Radius*2.0f, 0.0f, 0.0f);

			const PintActorHandle StaticObject = CreateStaticObject(pint, &SphereDesc, StaticPos);
	//		const PintActorHandle StaticObject = CreateDynamicObject(pint, &SphereDesc, StaticPos);
	//		SphereDesc.mRadius		= Radius/2.0f;
			const PintActorHandle DynamicObject = CreateDynamicObject(pint, &SphereDesc, DynamicPos);

			PINT_DISTANCE_JOINT_CREATE Desc;
			Desc.mObject0			= StaticObject;
			Desc.mObject1			= DynamicObject;
			Desc.mLimits.mMaxValue	= 2.0f * Radius;
			const PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		else if(SelectedIndex==2)
		{
			if(!caps.mSupportSphericalJoints)
				return false;

			const float BoxSize = 1.0f;
			const Point Extents(BoxSize, BoxSize, BoxSize);

			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateBoxRenderer(Extents);

			const Point StaticPos(0.0f, 20.0f, 0.0f);
			const Point DynamicPos = StaticPos - Extents*2.0f;

//			const PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
//			const PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);
			const PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);
			const PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);

			const PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(StaticObject, DynamicObject, -Extents, Extents));
			ASSERT(JointHandle);
		}
		else if(SelectedIndex==3)
		{
			if(!caps.mSupportHingeJoints)
				return false;
			return CreateJointedCubes(pint, caps, Point(0.0f, 20.0f, 0.0f), Point(0.0f, 0.0f, 1.0f), -1.0f);
		}
		else if(SelectedIndex==4)
		{
			if(!caps.mSupportPrismaticJoints)
				return false;

			const float BoxSize = 1.0f;
			const Point Extents(BoxSize, BoxSize, BoxSize);

			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateBoxRenderer(Extents);

			const Point StaticPos(0.0f, 20.0f, 0.0f);
			const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			const PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			const PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_PRISMATIC_JOINT_CREATE Desc;
			Desc.mObject0			= StaticObject;
			Desc.mObject1			= DynamicObject;
			//###PRISMATIC2
			Desc.mLocalPivot0.mPos	= Disp*0.5f;
			Desc.mLocalPivot1.mPos	= -Disp*0.5f;
			Desc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
			Desc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);
			const PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return true;
	}

}BasicJointAPI;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CheckCollisionBetweenJointed = "Test to check if collision is enabled or disabled by default, between objects \
connected by a joint. All engines do not have the same default behavior here. In some engines explicit collision filtering between jointed objects is needed...";

START_TEST(CheckCollisionBetweenJointed, CATEGORY_JOINTS_BASICS, gDesc_CheckCollisionBetweenJointed)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(8.17f, 22.24f, 8.67f), Point(-0.57f, -0.33f, -0.75f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		const Point StaticPos(0.0f, 20.0f, 0.0f);
		const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
		const Point DynamicPos0 = StaticPos + Disp;
		const Point DynamicPos1 = DynamicPos0 + Disp;

		PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
		PintActorHandle DynamicObject0 = CreateDynamicObject(pint, &BoxDesc, DynamicPos0);
		PintActorHandle DynamicObject1 = CreateDynamicObject(pint, &BoxDesc, DynamicPos1);

		PINT_HINGE_JOINT_CREATE Desc;
		Desc.mObject0		= StaticObject;
		Desc.mObject1		= DynamicObject0;
		Desc.mLocalPivot0	= Disp*0.5f;
		Desc.mLocalPivot1	= -Disp*0.5f;
		Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
		Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
		PintJointHandle JointHandle0 = pint.CreateJoint(Desc);
		ASSERT(JointHandle0);

		Desc.mObject0		= DynamicObject0;
		Desc.mObject1		= DynamicObject1;
		PintJointHandle JointHandle1 = pint.CreateJoint(Desc);
		ASSERT(JointHandle1);
		return true;
	}

END_TEST(CheckCollisionBetweenJointed)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_LimitedHingeJoint = "Simple test scene for hinge joints with (angular) limits. Filtering is used to disable collisions between jointed objects.";

START_TEST(LimitedHingeJoint, CATEGORY_JOINTS_BASICS, gDesc_LimitedHingeJoint)

/*	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("LimitedHingeJoint");
	}*/

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(19.00f, 22.95f, 13.39f), Point(-0.48f, -0.16f, -0.86f));
		desc.mCamera[1] = PintCameraPose(Point(3.57f, 19.59f, 5.08f), Point(-0.48f, -0.15f, -0.86f));
		desc.mCamera[2] = PintCameraPose(Point(9.60f, 23.57f, 3.19f), Point(-0.14f, -0.79f, -0.59f));
		desc.mCamera[3] = PintCameraPose(Point(17.59f, 21.81f, 4.68f), Point(-0.13f, -0.50f, -0.85f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const bool UseFiltering = true;
		if(UseFiltering)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			const PintDisabledGroups DG(1, 2);
			pint.SetDisabledGroups(1, &DG);
		}

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);

		udword GroupBit = 0;
//		udword i=0;
		for(udword i=0;i<3;i++)
		{
			const Point StaticPos(float(i)*BoxSize*8.0f, 20.0f, 0.0f);
//			const Point StaticPos(float(i)*BoxSize*8.0f, 2.0f, 0.0f);
			const Point Disp(BoxSize*2.0f, -BoxSize*2.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			ObjectDesc.mMass			= 0.0f;
			ObjectDesc.mPosition		= StaticPos;
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
			PintActorHandle StaticObject = CreatePintObject(pint, ObjectDesc);

			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= DynamicPos;
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
			PintActorHandle DynamicObject = CreatePintObject(pint, ObjectDesc);

			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= StaticObject;
			Desc.mObject1		= DynamicObject;
			Desc.mLocalPivot0	= Disp*0.5f;
			Desc.mLocalPivot1	= -Disp*0.5f;
			if(i==0)
			{
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
			}
			else if(i==1)
			{
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
			}
			else if(i==2)
			{
				Desc.mLocalAxis0	= Point(1.0f, 0.0f, 0.0f);
				Desc.mLocalAxis1	= Point(1.0f, 0.0f, 0.0f);
			}
			Desc.mLimits.Set(degToRad(-45.0f), degToRad(45.0f));

//				Desc.mMaxLimitAngle	= 0.0f;
//				Desc.mMinLimitAngle	= -PI/4.0f;

			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return true;
	}

END_TEST(LimitedHingeJoint)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_LimitedHingeJoint2 = "Simple test scene for hinge joints with (angular) limits, this time between two dynamic objects.";

START_TEST(LimitedHingeJoint2, CATEGORY_JOINTS_BASICS, gDesc_LimitedHingeJoint2)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(3.62f, 4.58f, 8.59f), Point(-0.31f, -0.27f, -0.91f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		const Point DynamicPos0(0.0f, 4.0f, 0.0f);
		const Point Disp(BoxSize*2.0f, -BoxSize*2.0f, 0.0f);
		const Point DynamicPos1 = DynamicPos0 + Disp;

		const PintActorHandle DynamicObject0 = CreateDynamicObject(pint, &BoxDesc, DynamicPos0);
		const PintActorHandle DynamicObject1 = CreateDynamicObject(pint, &BoxDesc, DynamicPos1);

		PINT_HINGE_JOINT_CREATE Desc;
		Desc.mObject0		= DynamicObject0;
		Desc.mObject1		= DynamicObject1;
		Desc.mLocalPivot0	= Disp*0.5f;
		Desc.mLocalPivot1	= -Disp*0.5f;
		Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
		Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
		Desc.mLimits.Set(degToRad(-45.0f), degToRad(45.0f));
		PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
		return true;
	}

END_TEST(LimitedHingeJoint2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_LimitedHingeJoint3 = "Same as before with new hinge API.";

START_TEST(LimitedHingeJoint3, CATEGORY_JOINTS_BASICS, gDesc_LimitedHingeJoint3)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(3.62f, 4.58f, 8.59f), Point(-0.31f, -0.27f, -0.91f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		const Point DynamicPos0(0.0f, 4.0f, 0.0f);
		const Point Disp(BoxSize*2.0f, -BoxSize*2.0f, 0.0f);
		const Point DynamicPos1 = DynamicPos0 + Disp;

		const PintActorHandle DynamicObject0 = CreateDynamicObject(pint, &BoxDesc, DynamicPos0);
		const PintActorHandle DynamicObject1 = CreateDynamicObject(pint, &BoxDesc, DynamicPos1);

		PINT_HINGE2_JOINT_CREATE Desc;
		Desc.mObject0			= DynamicObject0;
		Desc.mObject1			= DynamicObject1;
		Desc.mLocalPivot0.mPos	= Disp*0.5f;
		Desc.mLocalPivot1.mPos	= -Disp*0.5f;
		Matrix3x3 xToZ;
		xToZ.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
		const Quat q = xToZ;
		Desc.mLocalPivot0.mRot	= q;
		Desc.mLocalPivot1.mRot	= q;
		Desc.mLimits.Set(degToRad(-45.0f), degToRad(45.0f));
		PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
		return true;
	}

END_TEST(LimitedHingeJoint3)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConfigurableLimitedHingeJoint = "Hinge joints with configurable limits. Different engines support different limit ranges. Some only support -PI to +PI.";

class ConfigurableLimitedHingeJoint : public TestBase
{
			EditBoxPtr		mEditBox_LimitAngle;
	public:
							ConfigurableLimitedHingeJoint()	{												}
	virtual					~ConfigurableLimitedHingeJoint(){												}
	virtual	const char*		GetName()				const	{ return "ConfigurableLimitedHingeJoint";		}
	virtual	const char*		GetDescription()		const	{ return gDesc_ConfigurableLimitedHingeJoint;	}
	virtual	TestCategory	GetCategory()			const	{ return CATEGORY_JOINTS_BASICS;				}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 100;
		WD.mLabel	= "ConfigurableLimitedHingeJoint";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 100;
		const sdword LabelWidth = 120;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Limit angle (degrees):", &UIElems);
			mEditBox_LimitAngle = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "90.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, WD.mWidth-4*2*2);

		return null;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.44f, 3.94f, 3.88f), Point(-0.69f, -0.19f, -0.70f));
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		const float LimitAngle = GetFloat(0.0f, mEditBox_LimitAngle);
		return CreateJointedCubes(pint, caps, Point(0.0f, 4.0f, 0.0f), Point(1.0f, 0.0f, 0.0f), LimitAngle);
	}

	virtual	float	DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		return PrintTwistAngle(pint, renderer, y, text_scale);
	}

END_TEST(ConfigurableLimitedHingeJoint)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_LimitedPrismaticJoint = "Simple test scene for prismatic joints with (linear) limits.";

START_TEST(LimitedPrismaticJoint, CATEGORY_JOINTS_BASICS, gDesc_LimitedPrismaticJoint)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-9.24f, 15.44f, 21.01f), Point(0.63f, -0.34f, -0.70f));

		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportPrismaticJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		{
			const Point StaticPos(0.0f, 0.0f, 0.0f);
			const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			const PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
//			const PintActorHandle StaticObject = CreateDynamicObject(pint, &BoxDesc, StaticPos);
			const PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_PRISMATIC_JOINT_CREATE Desc;
			Desc.mObject0			= StaticObject;
			Desc.mObject1			= DynamicObject;
			//###PRISMATIC2
			Desc.mLocalPivot0.mPos	= Disp*0.5f;
			Desc.mLocalPivot1.mPos	= -Disp*0.5f;
			Desc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
			Desc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);
			Desc.mLimits.Set(-BoxSize, BoxSize*2.0f);
			const PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}

		{
			const Point StaticPos(5.0f, 5.0f, 5.0f);
			const Point Disp(0.0f, BoxSize*2.0f, 0.0f);
			const Point DynamicPos = StaticPos + Disp;

			const PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			const PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_PRISMATIC_JOINT_CREATE Desc;
			Desc.mObject0			= StaticObject;
			Desc.mObject1			= DynamicObject;
			//###PRISMATIC2
			Desc.mLocalPivot0.mPos	= Disp*0.5f;
			Desc.mLocalPivot1.mPos	= -Disp*0.5f;
			Desc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
			Desc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
			Desc.mLimits.Set(-BoxSize, BoxSize*2.0f);
			const PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}

		{
			const Point StaticPos(10.0f, 10.0f, 10.0f);
			const Point Disp(0.0f, 0.0f, BoxSize*2.0f);
			const Point DynamicPos = StaticPos + Disp;

			const PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, StaticPos);
			const PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc, DynamicPos);

			PINT_PRISMATIC_JOINT_CREATE Desc;
			Desc.mObject0			= StaticObject;
			Desc.mObject1			= DynamicObject;
			//###PRISMATIC2
			Desc.mLocalPivot0.mPos	= Disp*0.5f;
			Desc.mLocalPivot1.mPos	= -Disp*0.5f;
			Desc.mLocalAxis0		= Point(0.0f, 0.0f, 1.0f);
			Desc.mLocalAxis1		= Point(0.0f, 0.0f, 1.0f);
			Desc.mLimits.Set(-BoxSize, BoxSize*2.0f);
			const PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return true;
	}

END_TEST(LimitedPrismaticJoint)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_LimitedPrismaticJoint2 = "Simple test scene for prismatic joints with (linear) limits, this time between two dynamic objects. \
The top cube should rest on top of the bottom cube.";

START_TEST(LimitedPrismaticJoint2, CATEGORY_JOINTS_BASICS, gDesc_LimitedPrismaticJoint2)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-2.55f, 5.55f, 10.57f), Point(0.29f, -0.27f, -0.92f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportPrismaticJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		const Point DynamicPos0(0.0f, BoxSize, 0.0f);
		const Point Disp(0.0f, BoxSize*2.0f, 0.0f);
		const Point DynamicPos1 = DynamicPos0 + Disp;

		const PintActorHandle DynamicObject0 = CreateDynamicObject(pint, &BoxDesc, DynamicPos0);
		const PintActorHandle DynamicObject1 = CreateDynamicObject(pint, &BoxDesc, DynamicPos1);

		PINT_PRISMATIC_JOINT_CREATE Desc;
		Desc.mObject0			= DynamicObject0;
		Desc.mObject1			= DynamicObject1;
		//###PRISMATIC2
		Desc.mLocalPivot0.mPos	= Disp*0.5f;
		Desc.mLocalPivot1.mPos	= -Disp*0.5f;
		Desc.mLocalAxis0		= Point(0.0f, 1.0f, 0.0f);
		Desc.mLocalAxis1		= Point(0.0f, 1.0f, 0.0f);
		Desc.mLimits.Set(0.0f, BoxSize*2.0f);
		const PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
		return true;
	}

END_TEST(LimitedPrismaticJoint2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PrismaticSpring = "Simple test scene for prismatic springs. Some engines have support for either soft or hard limits. \
The soft limits can be used to implement a spring behavior, which could then be used to model e.g. a suspension.";

START_TEST(PrismaticSpring, CATEGORY_JOINTS_BASICS, gDesc_PrismaticSpring)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-8.31f, 7.13f, 13.49f), Point(0.48f, -0.33f, -0.82f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportPrismaticJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const Point Extents(4.0f, 2.0f, 4.0f);

		PintActorHandle Chassis;
		const Point ChassisPos(0.0f, Extents.y*4.0f, 0.0f);

		{
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
//			Chassis = CreateDynamicObject(pint, &BoxDesc, ChassisPos);
			Chassis = CreateSimpleObject(pint, &BoxDesc, 10.0f, ChassisPos);
		}

		const Point ChildExtents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(ChildExtents);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
		for(udword j=0;j<2;j++)
		{
			const float z = j ? 1.0f : -1.0f;
			for(udword i=0;i<2;i++)
			{
				const float x = i ? 1.0f : -1.0f;

				const Point Offset(x*(Extents.x-ChildExtents.x), -Extents.y-ChildExtents.y, z*(Extents.z-ChildExtents.z));
				const Point ChildPos = ChassisPos + Offset;

				PintActorHandle Child = CreateDynamicObject(pint, &BoxDesc, ChildPos);

				PINT_PRISMATIC_JOINT_CREATE Desc;
				Desc.mObject0				= Chassis;
				Desc.mObject1				= Child;
				//###PRISMATIC2
				Desc.mLocalPivot0.mPos		= Offset;
				Desc.mLocalPivot1.mPos		= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalAxis0			= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1			= Point(0.0f, 1.0f, 0.0f);
				Desc.mLimits.Set(0.0f, 0.01f);
				Desc.mSpring.mStiffness		= 100.0f;
				Desc.mSpring.mDamping		= 10.0f;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
		return true;
	}

END_TEST(PrismaticSpring)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_HingeJointMotor = "Hinge joints motor test (velocity drive). Also tests RCA drives.";

class HingeJointMotor : public TestBase
{
	class LocalTestData : public TestData
	{
		public:
			LocalTestData() : mDynamicObject(null), mJoint(null)	{}
			PintActorHandle	mDynamicObject;
			PintJointHandle	mJoint;
	};

			CheckBoxPtr		mCheckBox_EnableMotor;
			CheckBoxPtr		mCheckBox_EnableUpdate;
			EditBoxPtr		mEditBox_TargetVel;
			ComboBoxPtr		mComboBox_JointType;
			SliderPtr		mSlider;

	public:
							HingeJointMotor()			{									}
	virtual					~HingeJointMotor()			{									}
	virtual	const char*		GetName()			const	{ return "HingeJointMotor";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_HingeJointMotor;		}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS_BASICS;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 280;
		WD.mHeight	= 220;
		WD.mLabel	= "HingeJointMotor";
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
			mCheckBox_EnableMotor = helper.CreateCheckBox(UI, 0, 4, y, 100, 20, "Enable motor", &UIElems, true, null, null);
			y += YStep;

			mCheckBox_EnableUpdate = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Enable motor update", &UIElems, true, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Target vel.:", &UIElems);
			mEditBox_TargetVel = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT, null, null);
			mEditBox_TargetVel->SetEnabled(InitialEnabled);
			y += YStep;
		}

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Joint type:", &UIElems);
		{
			IceComboBox* CB = CreateComboBox<IceComboBox>(UI, 0, 4+OffsetX, y, 140, 20, "Joint type", &UIElems, null);
			CB->Add("Revolute");
			CB->Add("RCA");
			CB->Select(0);
			mComboBox_JointType = CB;
		}
		y += YStep;

		{
			y += YStep;
			y += YStep;
			SliderDesc SD;
			SD.mStyle	= SLIDER_HORIZONTAL;
			SD.mID		= 0;
			SD.mParent	= UI;
			SD.mX		= 4;
			SD.mY		= y;
			SD.mWidth	= WD.mWidth - SD.mX*2;
			SD.mHeight	= 20;
			SD.mLabel	= "test";
			mSlider		= ICE_NEW(IceSlider)(SD);
			mSlider->SetRange(0.0f, 1.0f, 100);
			mSlider->SetValue(1.0f);
			UIElems.Register(mSlider);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 264);

		return null;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.93f, 20.16f, 5.70f), Point(-0.65f, -0.09f, -0.75f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const Point static_pos(0.0f, 20.0f, 0.0f);
		const Point local_axis(0.0f, 0.0f, 1.0f);

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		const Point Disp(BoxSize*2.0f, -BoxSize*2.0f, 0.0f);
//		const Point Disp(0.0f, 0.0f, 0.0f);
		const Point DynamicPos = static_pos + Disp;

		udword Type = 0;
		if(mComboBox_JointType)
			Type = mComboBox_JointType->GetSelectedIndex();

		PintArticHandle RCA = null;
		if(Type==0)
		{
			if(!caps.mSupportHingeJoints)
				return false;
		}
		else
		{
			if(!caps.mSupportRCArticulations)
				return false;
			RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(true));
		}

		PintActorHandle StaticObject;
		{
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= RCA ? 1.0f : 0.0f;
			ObjectDesc.mPosition		= static_pos;
//			ObjectDesc.mCollisionGroup	= 1;

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mFrictionCoeff = 0.0f;
				StaticObject = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
			{
				StaticObject = CreatePintObject(pint, ObjectDesc);
			}
		}

		const bool EnableMotor = mCheckBox_EnableMotor ? mCheckBox_EnableMotor->IsChecked() : true;

		PintActorHandle DynamicObject;
		{
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= DynamicPos;
//			ObjectDesc.mCollisionGroup	= 1;

			const Point Pivot0	= Disp*0.5f;
			const Point Pivot1	= -Disp*0.5f;
			const float TargetVel = GetFloat(1.0f, mEditBox_TargetVel);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE Desc;

				Desc.mJointType			= PINT_JOINT_HINGE;
				Desc.mParent			= StaticObject;
				Desc.mAxisIndex			= Z_;
				Desc.mLocalPivot0.mPos	= Pivot0;
				Desc.mLocalPivot1.mPos	= Pivot1;
				Desc.mFrictionCoeff		= 0.0f;

				if(EnableMotor)
				{
					Desc.mUseMotor			= true;
					//TODO: revisit these
					Desc.mMotor.mStiffness	= 0.0f;
					Desc.mMotor.mDamping	= 1000.0f;
					Desc.mTargetVel			= TargetVel;
				}

				DynamicObject = pint.CreateRCArticulatedObject(ObjectDesc, Desc, RCA);

				LocalTestData* LTD = ICE_NEW(LocalTestData);
				RegisterTestData(LTD);
				LTD->mDynamicObject = DynamicObject;
//				LTD->mJoint			= JointHandle;
				pint.mUserData		= LTD;
			}
			else
			{
				DynamicObject = CreatePintObject(pint, ObjectDesc);

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= StaticObject;
				Desc.mObject1		= DynamicObject;
				Desc.mLocalPivot0	= Pivot0;
				Desc.mLocalPivot1	= Pivot1;
				Desc.mLocalAxis0	= local_axis;
				Desc.mLocalAxis1	= local_axis;

				if(EnableMotor)
				{
					Desc.mUseMotor		= true;
					Desc.mDriveVelocity	= TargetVel;
				}

				const PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

				LocalTestData* LTD = ICE_NEW(LocalTestData);
				RegisterTestData(LTD);
				LTD->mDynamicObject = DynamicObject;
				LTD->mJoint			= JointHandle;
				pint.mUserData		= LTD;
			}
		}

		if(RCA)
			pint.AddRCArticulationToScene(RCA);

		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
		const bool EnableUpdate = mCheckBox_EnableUpdate ? mCheckBox_EnableUpdate->IsChecked() : true;
		if(EnableUpdate)
		{
			const bool EnableMotor = mCheckBox_EnableMotor ? mCheckBox_EnableMotor->IsChecked() : true;
			const LocalTestData* LTD = (const LocalTestData*)pint.mUserData;
			if(LTD)
			{
				const float Coeff = mSlider ? mSlider->GetValue() : 1.0f;
				const float TargetVel = Coeff * GetFloat(1.0f, mEditBox_TargetVel);
	//			printf("TargetVel: %f\n", TargetVel);
				if(LTD->mJoint)
				{
					pint.SetDriveEnabled(LTD->mJoint, EnableMotor);
					if(EnableMotor)
						pint.SetDriveVelocity(LTD->mJoint, Point(0.0f, 0.0f, 0.0f), Point(TargetVel, 0.0f, 0.0f));
				}
				else
				{
					pint.SetRCADriveEnabled(LTD->mDynamicObject, EnableMotor);
					if(EnableMotor)
						pint.SetRCADriveVelocity(LTD->mDynamicObject, TargetVel);
				}
			}
		}
		return 0;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const LocalTestData* LTD = (const LocalTestData*)pint.mUserData;
		if(LTD && LTD->mDynamicObject)
		{
			const Point AngVel = pint.GetAngularVelocity(LTD->mDynamicObject);
	//		renderer.print(0.0f, y, text_scale, _F("Angular velocity: %.3f | %.3f | %.3f\n", Float(AngVel.x), Float(AngVel.y), Float(AngVel.z)));
			renderer.print(0.0f, y, text_scale, _F("Angular velocity: %f\n", AngVel.z));
			y -= text_scale;
	//		return PrintTwistAngle(pint, renderer, y, text_scale);
		}
		return y;
	}

END_TEST(HingeJointMotor)

///////////////////////////////////////////////////////////////////////////////

// TODO: disable CD between connected pair using filtering
static const char* gDesc_HingeJointMotorVsObstacle = "Hinge joints motor vs static obstacle. This is a case where increasing the shape's contact offset \
or enabling angular CCD could improve the behavior on impact.";

START_TEST(HingeJointMotorVsObstacle, CATEGORY_JOINTS_BASICS, gDesc_HingeJointMotorVsObstacle)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("HingeJointMotorVsObstacle", 0);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-11.66f, 21.81f, 4.42f), Point(0.82f, -0.06f, -0.57f));
//		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		const Point static_pos(0.0f, 20.0f, 0.0f);
		const Point local_axis(0.0f, 0.0f, 1.0f);

		if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		const Point Extents2(BoxSize, BoxSize*8.0f, BoxSize);
		PINT_BOX_CREATE BoxDesc2(Extents2);
		BoxDesc2.mRenderer = CreateBoxRenderer(Extents2);

		const Point Disp(0.0f, 0.0f, 0.0f);
		const Point DynamicPos = static_pos + Disp;

		PintActorHandle StaticObject = CreateStaticObject(pint, &BoxDesc, static_pos);

		PintActorHandle Obstacle = CreateStaticObject(pint, &BoxDesc, static_pos + Point(-8.0f, 0.0f, 0.0f));
//		PintActorHandle Obstacle = CreateSimpleObject(pint, &BoxDesc, 1000000.0f, static_pos + Point(-8.0f, 0.0f, 0.0f));
//		PintActorHandle Obstacle = CreateSimpleObject(pint, &BoxDesc, 10.0f, static_pos + Point(-8.0f, 0.0f, 0.0f));

		PintActorHandle DynamicObject = CreateDynamicObject(pint, &BoxDesc2, DynamicPos);

//		pint.SetAngularVelocity(DynamicObject, Point(0.0f, 0.0f, 1.0f));

		PINT_HINGE_JOINT_CREATE Desc;
		Desc.mObject0		= StaticObject;
		Desc.mObject1		= DynamicObject;
		Desc.mLocalPivot0	= Disp*0.5f;
		Desc.mLocalPivot1	= -Disp*0.5f;
		Desc.mLocalAxis0	= local_axis;
		Desc.mLocalAxis1	= local_axis;

		if(1)
		{
			Desc.mUseMotor		= true;
			Desc.mDriveVelocity	= 1.0f;
		}

		PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
//		pint.mUserData = JointHandle;
		pint.mUserData = DynamicObject;
		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
//		pint.SetAngularVelocity(pint.mUserData, Point(0.0f, 0.0f, 1.0f));
		return 0;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		return PrintAngularVelocity(pint, renderer, PintActorHandle(pint.mUserData), y, text_scale);
//		return PrintTwistAngle(pint, renderer, y, text_scale);
	}

END_TEST(HingeJointMotorVsObstacle)

///////////////////////////////////////////////////////////////////////////////

#ifdef REMOVED
static const char* gDesc_HingeJointPosDrive = "Hinge joints motor test (position drive).";

class HingeJointPosDrive : public TestBase
{
	class LocalTestData : public TestData
	{
		public:
			LocalTestData() : mDynamicObject(null), mJoint(null)	{}
			PintActorHandle	mDynamicObject;
			PintJointHandle	mJoint;
	};

//			CheckBoxPtr		mCheckBox_EnableMotor;
//			EditBoxPtr		mEditBox_TargetVel;
//			SliderPtr		mSlider;

	public:
							HingeJointPosDrive()// :
//								mComboBox_JointType		(null),
														{									}
	virtual					~HingeJointPosDrive()		{									}
	virtual	const char*		GetName()			const	{ return "HingeJointPosDrive";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_HingeJointPosDrive;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS_BASICS;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.93f, 20.16f, 5.70f), Point(-0.65f, -0.09f, -0.75f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const Point static_pos(0.0f, 20.0f, 0.0f);
		const Point local_axis(0.0f, 0.0f, 1.0f);

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		const Point Disp(BoxSize*2.0f, -BoxSize*2.0f, 0.0f);
//		const Point Disp(0.0f, 0.0f, 0.0f);
		const Point DynamicPos = static_pos + Disp;

		{
			if(!caps.mSupportHingeJoints)
				return false;
		}

		PintActorHandle StaticObject;
		{
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 0.0f;
			ObjectDesc.mPosition		= static_pos;
//			ObjectDesc.mCollisionGroup	= 1;

			StaticObject = CreatePintObject(pint, ObjectDesc);
		}

		const bool EnableMotor =true;

		PintActorHandle DynamicObject;
		{
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= DynamicPos;
//			ObjectDesc.mCollisionGroup	= 1;

			const Point Pivot0	= Disp*0.5f;
			const Point Pivot1	= -Disp*0.5f;
			const float TargetVel = 1.0f;

			{
				DynamicObject = CreatePintObject(pint, ObjectDesc);

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= StaticObject;
				Desc.mObject1		= DynamicObject;
				Desc.mLocalPivot0	= Pivot0;
				Desc.mLocalPivot1	= Pivot1;
				Desc.mLocalAxis0	= local_axis;
				Desc.mLocalAxis1	= local_axis;

				if(EnableMotor)
				{
					Desc.mUseMotor		= true;
					Desc.mDriveVelocity	= TargetVel;
				}

				const PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);

				LocalTestData* LTD = ICE_NEW(LocalTestData);
				RegisterTestData(LTD);
				LTD->mDynamicObject = DynamicObject;
				LTD->mJoint			= JointHandle;
				pint.mUserData		= LTD;
			}
		}

		return true;
	}

/*	virtual	udword		Update(Pint& pint, float dt)
	{
		const bool EnableMotor = mCheckBox_EnableMotor ? mCheckBox_EnableMotor->IsChecked() : true;
		const LocalTestData* LTD = (const LocalTestData*)pint.mUserData;
		if(LTD)
		{
			const float Coeff = mSlider ? mSlider->GetValue() : 1.0f;
			const float TargetVel = Coeff * GetFloat(1.0f, mEditBox_TargetVel);
//			printf("TargetVel: %f\n", TargetVel);
			if(LTD->mJoint)
			{
				pint.SetDriveEnabled(LTD->mJoint, EnableMotor);
				if(EnableMotor)
					pint.SetDriveVelocity(LTD->mJoint, Point(0.0f, 0.0f, 0.0f), Point(TargetVel, 0.0f, 0.0f));
			}
			else
			{
				pint.SetRCADriveEnabled(LTD->mDynamicObject, EnableMotor);
				if(EnableMotor)
					pint.SetRCADriveVelocity(LTD->mDynamicObject, TargetVel);
			}
		}
		return 0;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const LocalTestData* LTD = (const LocalTestData*)pint.mUserData;
		if(LTD && LTD->mDynamicObject)
		{
			const Point AngVel = pint.GetAngularVelocity(LTD->mDynamicObject);
	//		renderer.print(0.0f, y, text_scale, _F("Angular velocity: %.3f | %.3f | %.3f\n", Float(AngVel.x), Float(AngVel.y), Float(AngVel.z)));
			renderer.print(0.0f, y, text_scale, _F("Angular velocity: %f\n", AngVel.z));
			y -= text_scale;
	//		return PrintTwistAngle(pint, renderer, y, text_scale);
		}
		return y;
	}*/

END_TEST(HingeJointPosDrive)
#endif

///////////////////////////////////////////////////////////////////////////////
