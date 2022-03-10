///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Cylinder.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "GUI_Helpers.h"

///////////////////////////////////////////////////////////////////////////////

class CylinderTest : public TestBase
{
	protected:
	CheckBoxPtr		mCheckBox_UseConvexesForAll;
	EditBoxPtr		mEditBox_CylinderTess;

	public:
					CylinderTest()	{}
	virtual			~CylinderTest()	{}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 160;
		WD.mLabel	= "Cylinder";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword OffsetX = 120;
		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 120;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;

		{
			y += YStep;

			mCheckBox_UseConvexesForAll = helper.CreateCheckBox(UI, 0, 4, y, 200, 20, "Use convexes for all", &UIElems, false, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Cylinder tessellation:", &UIElems);
			mEditBox_CylinderTess = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "60", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		if(!caps.mSupportCylinders && !caps.mSupportConvexes)
			return false;

		return true;
	}
};

#define START_CYLINDER_TEST(name, category, desc)									\
	class name : public CylinderTest												\
	{																				\
		public:																		\
								name()						{					}	\
		virtual					~name()						{					}	\
		virtual	const char*		GetName()			const	{ return #name;		}	\
		virtual	const char*		GetDescription()	const	{ return desc;		}	\
		virtual	TestCategory	GetCategory()		const	{ return category;	}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_RollingCylinder = "Rolling cylinder. Some engines do not support proper cylinder shapes. This test investigates the rolling behavior of \
cylinders approximated by a highly tessellated convex. With a high-enough tessellation the motion is very smooth, virtually the same as for a real cylinder. \
A side effect of this approach is that it gives you some rolling friction, even if the engine does not support it. In other words, using a convex may give \
a better behavior overall, eventually stopping the motion even when linear damping is fully disabled.";

START_CYLINDER_TEST(RollingCylinder, CATEGORY_BEHAVIOR, gDesc_RollingCylinder)

	CylinderMesh mCylinder;

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(12.52f, 6.76f, 20.21f), Point(-0.35f, -0.27f, -0.90f));
		SetDefEnv(desc, true);
	}

	virtual	bool	CommonSetup()
	{
		const float HalfHeight = 3.0f;
		const float Radius = 3.0f;
		const udword NbCirclePts = GetInt(60, mEditBox_CylinderTess);

		mCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mCylinder.mNbVerts, mCylinder.mVerts));

		RegisterRenderer(CreateCylinderRenderer(Radius, HalfHeight*2.0f));

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CylinderTest::Setup(pint, caps))
			return false;

		// Create ramp
		{
			Matrix3x3 M;
			M.RotZ(-0.2f);
			const Quat R = M;

			PINT_BOX_CREATE BoxDesc(10.0f, 1.0f, 10.0f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
			PintActorHandle Handle = CreateStaticObject(pint, &BoxDesc, Point(0.0f, 0.0f, 0.0f), &R);
			ASSERT(Handle);
		}

		const bool UseConvexes = mCheckBox_UseConvexesForAll ? mCheckBox_UseConvexesForAll->IsChecked() : false;

		PintActorHandle Handle;
		if(!caps.mSupportCylinders || UseConvexes)
		{
			// Create cylinder as a convex object
			PINT_CONVEX_CREATE ConvexCreate(mCylinder.mNbVerts, mCylinder.mVerts);
			ConvexCreate.mRenderer	= GetRegisteredRenderers()[0];
			Handle = CreateDynamicObject(pint, &ConvexCreate, Point(0.0f, mCylinder.mRadius+2.0f, 0.0f));
		}
		else
		{
			PINT_CYLINDER_CREATE CylinderCreate(mCylinder.mRadius, mCylinder.mHalfHeight);
			CylinderCreate.mRenderer	= GetRegisteredRenderers()[1];
			const Quat Q = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
			Handle = CreateDynamicObject(pint, &CylinderCreate, Point(0.0f, mCylinder.mRadius+2.0f, 0.0f), &Q);
		}
		ASSERT(Handle);
		pint.mUserData = Handle;
		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
		if(0)
		{
			const PintActorHandle Handle = PintActorHandle(pint.mUserData);
			const PR WorldPose = pint.GetWorldTransform(Handle);

			const Matrix3x3 Rot = WorldPose.mRot;
			const Point Axis = Rot[2];
			printf("Error: %f (%s)\n", fabsf(Axis.y)*100.0f, pint.GetName());

	/*		const float Error = Angle(Axis, Point(0.0f, 0.0f, 1.0f));
	//		printf("%f %f %f\n", Axis.x, Axis.y, Axis.z);
			printf("Error: %f (%s)\n", Error, pint.GetName());*/

	//		EulerAngles EA;
	//		EA.Set(WorldPose.mRot);
	//		printf("%f %f %f\n", EA.GetYaw(), EA.GetPitch(), EA.GetRoll());
		}
		return CylinderTest::Update(pint, dt);
	}

END_TEST(RollingCylinder)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CylinderStack = "Cylinder stack. The problem with using a tessellated convex instead of a proper cylinder shape is of course performance. \
This test investigates this performance impact using a stack of cylinders.";

START_CYLINDER_TEST(CylinderStack, CATEGORY_BEHAVIOR, gDesc_CylinderStack)

	CylinderMesh mCylinder;

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(35.97f, 15.73f, 25.50f), Point(-0.58f, -0.10f, -0.81f));
		SetDefEnv(desc, true);
	}

	virtual	bool	CommonSetup()
	{
		const float HalfHeight = 1.0f;
		const float Radius = 1.0f;
		const udword NbCirclePts = GetInt(60, mEditBox_CylinderTess);
		mCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mCylinder.mNbVerts, mCylinder.mVerts));

		RegisterRenderer(CreateCylinderRenderer(Radius, HalfHeight*2.0f));

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CylinderTest::Setup(pint, caps))
			return false;

		const bool UseConvexes = mCheckBox_UseConvexesForAll ? mCheckBox_UseConvexesForAll->IsChecked() : false;

		Matrix3x3 M;
		M.RotX(HALFPI);
		const Quat R = M;

		udword Nb = 14;

		const float Radius = mCylinder.mRadius;
		const float HalfHeight = mCylinder.mHalfHeight;
		float Altitude = mCylinder.mRadius;
		float OffsetX = 0.0f;
		while(Nb)
		{
			for(udword i=0;i<Nb;i++)
			{
				if(!caps.mSupportCylinders || UseConvexes)
				{
					PINT_CONVEX_CREATE ConvexCreate(mCylinder.mNbVerts, mCylinder.mVerts);
					ConvexCreate.mRenderer	= GetRegisteredRenderers()[0];
					PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, Point(OffsetX + float(i)*Radius*2.2f, Altitude, 0.0f), &R);
					ASSERT(Handle);
				}
				else
				{
					PINT_CYLINDER_CREATE CylinderCreate(mCylinder.mRadius, mCylinder.mHalfHeight);
					CylinderCreate.mRenderer	= GetRegisteredRenderers()[1];
//					const Quat Q = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
					PintActorHandle Handle = CreateDynamicObject(pint, &CylinderCreate, Point(OffsetX + float(i)*Radius*2.2f, Altitude, 0.0f)/*, &R*/);
					ASSERT(Handle);
				}
			}
			Nb--;
			Altitude += HalfHeight*2.0f+0.01f;
			OffsetX += Radius*1.1f;
		}
		return true;
	}

END_TEST(CylinderStack)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CDStack = "CD stack. Stack of 'thin' cylinders. (TODO: scene scale is wrong)";

START_CYLINDER_TEST(CDStack, CATEGORY_BEHAVIOR, gDesc_CDStack)

	CylinderMesh mCylinder;

//	virtual	float	GetRenderData(Point& center)	const	{ return 5.0f;	}
	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		CylinderTest::InitUI(helper);
		return CreateOverrideTabControl("CDStack", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(1.23f, 1.26f, 3.06f), Point(-0.35f, -0.18f, -0.92f));
		//desc.mCamera[0] = PintCameraPose(Point(0.38f, 0.46f, 1.04f), Point(-0.29f, -0.34f, -0.89f));
		SetDefEnv(desc, true);
	}

	virtual	bool	CommonSetup()
	{
		//const float HalfHeight = 0.002f;
		//const float Radius = 0.1f;
		const float HalfHeight = 0.02f;
		const float Radius = 1.0f;
		const udword NbCirclePts = GetInt(60, mEditBox_CylinderTess);
		mCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mCylinder.mNbVerts, mCylinder.mVerts));

		RegisterRenderer(CreateCylinderRenderer(Radius, HalfHeight*2.0f));

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!CylinderTest::Setup(pint, caps))
			return false;

		const bool UseConvexes = mCheckBox_UseConvexesForAll ? mCheckBox_UseConvexesForAll->IsChecked() : false;

		Matrix3x3 M;
		M.RotX(HALFPI);
		const Quat R = M;

		udword Nb = 14;

//		const float Radius = mCylinder.mRadius;
		const float HalfHeight = mCylinder.mHalfHeight;
		float Altitude = mCylinder.mRadius;
		while(Nb--)
		{
			if(!caps.mSupportCylinders || UseConvexes)
			{
				PINT_CONVEX_CREATE ConvexCreate(mCylinder.mNbVerts, mCylinder.mVerts);
				ConvexCreate.mRenderer	= GetRegisteredRenderers()[0];
				PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, Point(0.0f, Altitude, 0.0f), &R);
				ASSERT(Handle);
			}
			else
			{
				PINT_CYLINDER_CREATE CylinderCreate(mCylinder.mRadius, mCylinder.mHalfHeight);
				CylinderCreate.mRenderer	= GetRegisteredRenderers()[1];
				PintActorHandle Handle = CreateDynamicObject(pint, &CylinderCreate, Point(0.0f, Altitude, 0.0f));
				ASSERT(Handle);
			}
			Altitude += HalfHeight*2.0f+0.01f;
		}
		return true;
	}

END_TEST(CDStack)

///////////////////////////////////////////////////////////////////////////////
