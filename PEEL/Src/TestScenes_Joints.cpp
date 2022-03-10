///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintShapeRenderer.h"
#include "TestScenes_VehiclesBase.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "Loader_Bin.h"
#include "GUI_Helpers.h"
#include "Cylinder.h"
#include "GLFontRenderer.h"
#include "ZB2Import.h"
#include "MyConvex.h"
#include "PintSQ.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_GearJoint = "Gear joints.";
//static const udword gNbTeeth = 13*2+1;
//static const udword gNbTeeth = 33;
static const udword gNbTeeth = 65;
#define CREATE_GEAR_TEETH
static const bool gCreateGearJoint = true;
static const bool gUseGearMotor = false;
static const float gHandleSize = 0.025f;

//START_TEST(GearJoint, CATEGORY_JOINTS, gDesc_GearJoint)
class GearJoint : public TestBase
{
			ComboBoxPtr			mComboBox_Preset;
			PintJointHandle		mHingeJoint0;
			PintJointHandle		mHingeJoint1;
			PintJointHandle		mGearJoint;
			PintActorHandle	mGearObject0;
			PintActorHandle	mGearObject1;
			CylinderMesh		mCylinder;
			CylinderMesh		mCylinder2;
			float				mGearRatio;
			float				mGearRadius0;
			float				mGearRadius1;
			float				mGearThickness;
			float				mTeethZ;
			float				mTeethWidth;
	public:
							GearJoint()	:
								mHingeJoint0	(null),
								mHingeJoint1	(null),
								mGearJoint		(null),
								mGearObject0	(null),
								mGearObject1	(null),
								mGearRatio		(0.0f),
								mGearRadius0	(0.0f),
								mGearRadius1	(0.0f),
								mGearThickness	(0.0f),
								mTeethZ			(0.0f),
								mTeethWidth		(0.0f)
														{							}
	virtual					~GearJoint()				{							}
	virtual	const char*		GetName()			const	{ return "GearJoints";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_GearJoint;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;	}

/*	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 100;
		WD.mLabel	= "Gear joints";
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
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Scene:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				GearJoint&		mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, GearJoint& test) : IceComboBox(desc), mTest(test)	{}
				virtual			~MyComboBox()																			{}

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
			mComboBox_Preset->Select(0);
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
		}
		return null;
	}*/

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(2.44f, 3.52f, 5.36f), Point(-0.37f, -0.42f, -0.83f));
		SetDefEnv(desc, true);
	}

	virtual	bool	CommonSetup()
	{
		mGearRatio = 0.5f;	// Gear ratio decoupled from gear radius for tests
		mGearRadius0 = 0.5f;
		mGearRadius1 = 1.0f;
		mGearThickness = 0.1f;
//		mTeethZ = 0.04f;
//		mTeethZ = 0.03f;
		mTeethZ = 0.01f;
//		mTeethWidth = 0.04f;
		mTeethWidth = 0.02f;

		const float HalfHeight = 0.1f;
		const float Radius0 = mGearRadius0 - 0.05f;
		const udword NbCirclePts = 64;
		mCylinder.Generate(NbCirclePts, Radius0, HalfHeight, ORIENTATION_XZ);
		RegisterRenderer(CreateConvexRenderer(mCylinder.mNbVerts, mCylinder.mVerts));

		const float Radius1 = mGearRadius1 - 0.05f;
		mCylinder2.Generate(NbCirclePts, Radius1, HalfHeight, ORIENTATION_XZ);
		RegisterRenderer(CreateConvexRenderer(mCylinder2.mNbVerts, mCylinder2.mVerts));

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		mCylinder2.Reset();
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
//		if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation)
//			return false;

		if(!caps.mSupportGearJoints)
			return false;

		const float Altitude = mGearThickness*2.0f;
		PintActorHandle SupportObject;
		{
			const float MaxRadius = MAX(mGearRadius0, mGearRadius1);
			const Point Extents(MaxRadius*2.0f, mGearThickness, MaxRadius);

			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			SupportObject = CreateDynamicObject(pint, &BoxDesc, Point(0.0f, Extents.y+Altitude, 0.0f));
//			SupportObject = CreateStaticObject(pint, &BoxDesc, Point(0.0f, Extents.y+Altitude, 0.0f));
		}

		PintActorHandle Gear0 = null;
		{
			PINT_CONVEX_CREATE ConvexDesc(mCylinder.mNbVerts, mCylinder.mVerts);
			ConvexDesc.mRenderer	= GetRegisteredRenderers()[0];
//				const Point Extents(mGearRadius0, mGearThickness, mGearRadius0);
//				PINT_BOX_CREATE ConvexDesc(Extents);
//				ConvexDesc.mRenderer	= GetRegisteredRenderers()[0];

			PINT_SHAPE_CREATE* Current = &ConvexDesc;

#ifdef CREATE_GEAR_TEETH
			const udword NbTeeth = gNbTeeth/2;
			PINT_BOX_CREATE Teeth[NbTeeth];
			for(udword i=0;i<NbTeeth;i++)
			{
				const float Coeff = float(i)/float(NbTeeth);

				Matrix3x3 Rot;
				Rot.RotY(Coeff*PI);

				Teeth[i].mExtents = Point(mGearRadius0+mTeethWidth, mGearThickness*0.75f, mTeethZ);
				Teeth[i].mRenderer	= CreateBoxRenderer(Teeth[i].mExtents);
				Teeth[i].mLocalRot	= Rot;

				Current->SetNext(&Teeth[i]);
				Current = &Teeth[i];
			}
#endif

			PINT_BOX_CREATE BoxDesc2(Point(gHandleSize, gHandleSize, gHandleSize));
			BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
			BoxDesc2.mLocalPos	= Point(mGearRadius0-BoxDesc2.mExtents.x-0.05f, mGearThickness+BoxDesc2.mExtents.y, 0.0f);
			Current->SetNext(&BoxDesc2);

			Gear0 = CreateDynamicObject(pint, &ConvexDesc, Point(-mGearRadius0, Altitude+mGearThickness*3.0f, 0.0f));
//			Gear0 = CreateStaticObject(pint, &ConvexDesc, Point(-mGearRadius0, Altitude+mGearThickness*3.0f, 0.0f));
			mGearObject0 = Gear0;
		}

		PintActorHandle Gear1 = null;
		if(1)
		{
/*			const Point Extents(GearRadius, GearThickness, GearRadius);

			PINT_BOX_CREATE BoxDesc(Extents);
//			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);
			BoxDesc.mRenderer	= GetRegisteredRenderers()[0];

			PINT_BOX_CREATE BoxDesc2(Point(0.1f, 0.1f, 0.1f));
			BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
			BoxDesc2.mLocalPos	= Point(-GearRadius+BoxDesc2.mExtents.x+0.05f, GearThickness+BoxDesc2.mExtents.y, 0.0f);
			BoxDesc.mNext = &BoxDesc2;*/

			PINT_CONVEX_CREATE ConvexDesc(mCylinder2.mNbVerts, mCylinder2.mVerts);
			ConvexDesc.mRenderer	= GetRegisteredRenderers()[1];
//				const Point Extents(mGearRadius1, mGearThickness, mGearRadius1);
//				PINT_BOX_CREATE ConvexDesc(Extents);
//				ConvexDesc.mRenderer	= GetRegisteredRenderers()[1];
			PINT_SHAPE_CREATE* Current = &ConvexDesc;

#ifdef CREATE_GEAR_TEETH
			const udword NbTeeth = gNbTeeth;
			PINT_BOX_CREATE Teeth[NbTeeth];
			const float Phase = 0.5f/float(NbTeeth);
			for(udword i=0;i<NbTeeth;i++)
			{
				const float Coeff = Phase+float(i)/float(NbTeeth-1);

				Matrix3x3 Rot;
				Rot.RotY(Coeff*PI);

				Teeth[i].mExtents = Point(mGearRadius1+mTeethWidth, mGearThickness*0.75f, mTeethZ);
				Teeth[i].mRenderer	= CreateBoxRenderer(Teeth[i].mExtents);
				Teeth[i].mLocalRot	= Rot;

				Current->SetNext(&Teeth[i]);
				Current = &Teeth[i];
			}
#endif
			PINT_BOX_CREATE BoxDesc2(Point(gHandleSize, gHandleSize, gHandleSize));
			BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
			BoxDesc2.mLocalPos	= Point(-mGearRadius1+BoxDesc2.mExtents.x+0.05f, mGearThickness+BoxDesc2.mExtents.y, 0.0f);
			Current->SetNext(&BoxDesc2);

			Gear1 = CreateDynamicObject(pint, &ConvexDesc, Point(mGearRadius1, Altitude+mGearThickness*3.0f, 0.0f));
			mGearObject1 = Gear1;
		}

		if(Gear0)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= SupportObject;
			Desc.mObject1		= Gear0;
			Desc.mLocalPivot0	= Point(-mGearRadius0, mGearThickness, 0.0f);
			Desc.mLocalPivot1	= Point(0.0f, -mGearThickness, 0.0f);
			Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
			Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
			if(gUseGearMotor)
			{
				Desc.mUseMotor		= true;
				Desc.mDriveVelocity	= 1.0f;
			}
			mHingeJoint0		= pint.CreateJoint(Desc);
		}
/*		else
		{
			PINT_PRISMATIC_JOINT_CREATE Desc;
			Desc.mObject0		= SupportObject;
			Desc.mObject1		= Gear0;
			Desc.mLocalPivot0	= Point(-GearRadius, GearThickness, 0.0f);
			Desc.mLocalPivot1	= Point(0.0f, -GearThickness, 0.0f);
			Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
			Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
			mHingeJoint0		= pint.CreateJoint(Desc);
		}*/

		if(Gear1)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= SupportObject;
			Desc.mObject1		= Gear1;
			Desc.mLocalPivot0	= Point(mGearRadius1, mGearThickness, 0.0f);
			Desc.mLocalPivot1	= Point(0.0f, -mGearThickness, 0.0f);
			Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
			Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
			mHingeJoint1		= pint.CreateJoint(Desc);
		}

		if(gCreateGearJoint && Gear0 && Gear1)
		{
			Matrix3x3 Rot;
			Rot.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));

			PINT_GEAR_JOINT_CREATE Desc;
			Desc.mHinge0			= mHingeJoint0;
			Desc.mHinge1			= mHingeJoint1;
			Desc.mObject0			= Gear0;
			Desc.mObject1			= Gear1;
//			Desc.mLocalPivot0.mPos	= Point(0.0f, -GearThickness, 0.0f);
//			Desc.mLocalPivot0.mPos	= Point(GearRadius, 0.0f, 0.0f);
			Desc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalPivot0.mRot	= Rot;
//			Desc.mLocalPivot1.mPos	= Point(0.0f, -GearThickness, 0.0f);
//			Desc.mLocalPivot1.mPos	= Point(-GearRadius, 0.0f, 0.0f);
			Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalPivot1.mRot	= Rot;
			Desc.mGearRatio			= mGearRatio;
			//Desc.mRadius0			= mGearRadius0;
			//Desc.mRadius1			= mGearRadius1;
			mGearJoint				= pint.CreateJoint(Desc);
		}

		return true;
	}

/*	virtual	udword		Update(Pint& pint, float dt)
	{
		PintHingeInfo info;
		if(pint.GetHingeInfo(gGear0, info))


		return TestBase::Update(pint, dt);
	}*/

	virtual	float	DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
//		return PrintTwistAngle(pint, renderer, y, text_scale);

		float Angle0 = 0.0f;
		float Angle0b = 0.0f;
		if(mHingeJoint0)
		{
			float TwistAngle;
			if(!GetHingeTwistAngle(pint, mHingeJoint0, TwistAngle))
				return y;

			Angle0 = Angle0b = TwistAngle;
		}

		float Angle1 = 0.0f;
		float Angle1b = 0.0f;
		if(mHingeJoint1)
		{
			float TwistAngle;
			if(!GetHingeTwistAngle(pint, mHingeJoint1, TwistAngle))
				return y;

			Angle1 = Angle1b = -TwistAngle;
		}

		Angle0 = fmodf(Angle0, TWOPI);
		Angle1 = fmodf(Angle1, TWOPI);
		if(mGearRatio>1.0f)
		{
			while(Angle0<0.0f)
				Angle0 += TWOPI;
			while(Angle1<Angle0)
				Angle1 += TWOPI;
		}
		else if(mGearRatio<1.0f)
		{
			while(Angle1<0.0f)
				Angle1 += TWOPI;
			while(Angle0<Angle1)
				Angle0 += TWOPI;
		}
//		Angle0 = fmodf(Angle0, TWOPI);
//		Angle1 = fmodf(Angle1, TWOPI);
		renderer.print(0.0f, y, text_scale, _F("Angle0: %.5f\n", Angle0 * RADTODEG));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Angle1: %.5f\n", Angle1 * RADTODEG));	y -= text_scale;

		renderer.print(0.0f, y, text_scale, _F("Ratio: %.5f\n", Angle1/Angle0));	y -= text_scale;
		const float Error0 = fmodf(Angle1*2.0f-Angle0, TWOPI);
		const float Error1 = fmodf(Angle1b*2.0f-Angle0b, TWOPI);
		float Error;
		if(fabsf(Error0)<fabsf(Error1))
			Error = Error0;
		else
			Error = Error1;

		renderer.print(0.0f, y, text_scale, _F("Diff: %.5f\n", Error));	y -= text_scale;

/*		if(0 && mGearJoint)
		{
			pint.SetGearJointError(mGearJoint, Error);
		}*/

		const Point AngVel0 = mGearObject0 ? pint.GetAngularVelocity(mGearObject0) : Point(0.0f, 0.0f, 0.0f);
		const Point AngVel1 = mGearObject1 ? pint.GetAngularVelocity(mGearObject1) : Point(0.0f, 0.0f, 0.0f);
		renderer.print(0.0f, y, text_scale, _F("Vel0: %.5f\n", AngVel0.y));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Vel1: %.5f\n", AngVel1.y));	y -= text_scale;
		renderer.print(0.0f, y, text_scale, _F("Ratio: %.5f\n", AngVel1.y/AngVel0.y));	y -= text_scale;

		return y;
	}

END_TEST(GearJoint)

#include "ZCB\PINT_ZCB2.h"

static const bool gUseActiveEdges = false;

static const char* gDesc_TwoLegoGears = "Two Lego gears.";

//START_TEST(TwoLegoGears, CATEGORY_JOINTS, gDesc_TwoLegoGears)
class TwoLegoGears : public TestBase
{
			CylinderMesh		mCylinder;
			CylinderMesh		mCylinder2;
			PintJointHandle		mHingeJoint0;
			PintJointHandle		mHingeJoint1;
			CheckBoxPtr			mCheckBox_Dynamic;
			ComboBoxPtr			mComboBox_Preset;
	public:
							TwoLegoGears() :
								mHingeJoint0	(null),
								mHingeJoint1	(null)
							{
							}
	virtual					~TwoLegoGears()				{								}
	virtual	const char*		GetName()			const	{ return "TwoLegoGears";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_TwoLegoGears;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;		}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 300;
		WD.mLabel	= "Two Lego Gears";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

//		const sdword EditBoxWidth = 100;
		const sdword LabelWidth = 120;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Scenario:", &UIElems);

		class MyComboBox : public IceComboBox
		{
			TwoLegoGears&	mTest;
			public:
							MyComboBox(const ComboBoxDesc& desc, TwoLegoGears& test) : IceComboBox(desc), mTest(test)	{}
			virtual			~MyComboBox()																				{}

			virtual	void	OnComboBoxEvent(ComboBoxEvent event)
			{
				if(event==CBE_SELECTION_CHANGED)
				{
//					const udword SelectedIndex = GetSelectedIndex();
//					const bool Enabled = SelectedIndex==GetItemCount()-1;
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
		mComboBox_Preset->Add("Source static meshes");
		mComboBox_Preset->Add("Simple gear joint");
		mComboBox_Preset->Select(1);
		mComboBox_Preset->SetVisible(true);
		mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
		y += YStep;

		mCheckBox_Dynamic = helper.CreateCheckBox(UI, 0, 4, y, 100, 20, "Dynamic", &UIElems, false, null, null);
		y += YStep;

		y += YStep;
		AddResetButton(UI, 4, y, WD.mWidth-4*2*2);

		return null;
	}

/*	virtual	const char*		GetSubName()	const
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
	}*/

	virtual	float	GetRenderData(Point& center)	const	{ return 50.0f;	}

	virtual	bool	CommonSetup()
	{
		mHingeJoint0 = mHingeJoint1 = null;

		// TODO: derive from loaded data
		const float Radius0 = 1.29f;
		const float Radius1 = 2.08f;
		const float HalfHeight = 0.39f;

		const udword NbCirclePts = 64;
		mCylinder.Generate(NbCirclePts, Radius0, HalfHeight, ORIENTATION_XY);
		mCylinder2.Generate(NbCirclePts, Radius1, HalfHeight, ORIENTATION_XY);

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		mCylinder2.Reset();
		TestBase::CommonRelease();
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		const char* Filename = FindPEELFile("TwoGears2.zb2");
//		const char* Filename = FindPEELFile("TwoGears.zb2");
		if(Filename)
		{
			bool status = ImportZB2File(desc, Filename);
			(void)status;

			desc.mCamera[0] = PintCameraPose(Point(1.46f, 4.29f, 6.12f), Point(0.01f, -0.37f, -0.93f));
			SetDefEnv(desc, true);
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportGearJoints)
			return false;

		if(!mZB2Factory)
			return false;

		const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
		if(SelectedIndex==0)
			return CreateZB2Scene(pint, caps);

		if(SelectedIndex==1)
		{
			if(!caps.mSupportConvexes)
				return false;

			const udword NbActors = mZB2Factory->GetNbActors();
			const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

			ASSERT(!mZB2Factory->mNbDisabledGroups);
			ASSERT(NbActors==2);
			ASSERT(!mZB2Factory->mJoints.GetNbEntries());

			const bool UseFiltering = true;
			if(UseFiltering)
			{
				if(!caps.mSupportCollisionGroups)
					return false;

				const PintDisabledGroups DG(1, 1);
				pint.SetDisabledGroups(1, &DG);
			}

			// By default the gear axis is Z (that's the way the meshes are oriented in the file).
			const bool UseGearAxisX = false;

			const PINT_OBJECT_CREATE* Create0 = Actors[0].mCreate;
			const PINT_OBJECT_CREATE* Create1 = Actors[1].mCreate;
			const PINT_SHAPE_CREATE* Shape0 = Create0->GetFirstShape();
			const PINT_SHAPE_CREATE* Shape1 = Create1->GetFirstShape();

			PINT_OBJECT_CREATE Copy0 = *Create0;
			PINT_OBJECT_CREATE Copy1 = *Create1;

			PINT_CONVEX_CREATE Box0(mCylinder.mNbVerts, mCylinder.mVerts);
			Box0.mLocalPos	= Shape0->mLocalPos;
			Box0.mLocalRot	= Shape0->mLocalRot;
			Box0.mMaterial	= Shape0->mMaterial;
			Box0.mRenderer	= Shape0->mRenderer;	// Keep the Lego triangle mesh renderer
			ASSERT(!Shape0->_GetNext());

			PINT_CONVEX_CREATE Box1(mCylinder2.mNbVerts, mCylinder2.mVerts);
			Box1.mLocalPos	= Shape1->mLocalPos;
			Box1.mLocalRot	= Shape1->mLocalRot;
			Box1.mMaterial	= Shape1->mMaterial;
			Box1.mRenderer	= Shape1->mRenderer;	// Keep the Lego triangle mesh renderer
			ASSERT(!Shape1->_GetNext());

			// Transform source triangle meshes to convex meshes
			Copy0.SetShape(&Box0);
			Copy1.SetShape(&Box1);
			// Transform from static to dynamic
			Copy0.mMass = 1.0f;
			Copy1.mMass = 1.0f;
			// We need to move the scene up a bit, because by default the gears overlap the ground plane.
			const float OffsetY = 1.0f;
			Copy0.mPosition.y += OffsetY;
			Copy1.mPosition.y += OffsetY;

			// By default one of the gears has been rotated a bit around Z, so that the teeth don't overlap.
			// But the rotation isn't perfect and they do touch a bit.
			// TODO: use editor to find proper angle
//			Copy0.mRotation.Identity();
//			Copy1.mRotation.Identity();

			// Clumsy test to rotate the scene and transform the gear axis from Z to X (to test the joint shader works with arbitrary axes)
			if(UseGearAxisX)
			{
				Matrix4x4 SceneRot;
//				SceneRot.RotY(HALFPI);
				SceneRot.RotZ(0.8f);

				Matrix4x4 M0(Matrix3x3(Copy0.mRotation), Copy0.mPosition);
				Matrix4x4 M1(Matrix3x3(Copy1.mRotation), Copy1.mPosition);

				M0 *= SceneRot;
				M1 *= SceneRot;

				Copy0.mRotation = M0;
				Copy0.mPosition = M0.GetTrans();

				Copy1.mRotation = M1;
				Copy1.mPosition = M1.GetTrans();
			}

			if(UseFiltering)
			{
				Copy0.mCollisionGroup = 1;
				Copy1.mCollisionGroup = 1;
			}

			const PintActorHandle Gear0 = CreatePintObject(pint, Copy0);
			const PintActorHandle Gear1 = CreatePintObject(pint, Copy1);

//			return true;

			PintActorHandle SupportObject = null;
			if(mCheckBox_Dynamic && mCheckBox_Dynamic->IsChecked())
			{
				const Point SupportExtents(5.0f, 2.5f, 0.5f);
				PINT_BOX_CREATE BoxDesc(SupportExtents);
				BoxDesc.mRenderer	= CreateBoxRenderer(SupportExtents);

				SupportObject = CreateDynamicObject(pint, &BoxDesc, Point(2.0f, SupportExtents.y, -1.0f));
			}

			struct Local
			{
				static PintJointHandle CreateHinge(Pint& pint, PintActorHandle gear, PintActorHandle support, const Point& pos, const Point& axis, bool limits)
				{
					if(!gear)
						return null;

					if(0)
					{
						PINT_HINGE_JOINT_CREATE Desc;
						Desc.mObject0		= support;
						Desc.mObject1		= gear;
						Desc.mLocalAxis0	= axis;
						Desc.mLocalAxis1	= axis;
						
						if(support)
							Desc.mLocalPivot0	= Point(pos.x - 2.0f, 0.0f, 1.0f + pos.z);
						else
							Desc.mLocalPivot0	= pos;
						Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
						return pint.CreateJoint(Desc);
					}
					else
					{
						if(0)
						{
							PINT_SPHERICAL_JOINT_CREATE Desc;
							Desc.mObject0			= support;
							Desc.mObject1			= gear;
							Desc.mLocalPivot0.mPos	= pos;
							Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
							return pint.CreateJoint(Desc);
						}
						else
						{


						Matrix3x3 FromTo;
						FromTo.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));


						PINT_HINGE2_JOINT_CREATE Desc;

						// Setup gear
						Desc.mObject1			= gear;
						Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
						Desc.mLocalPivot1.mRot	= FromTo;

/*						{
							const PR pose1 = pint.GetWorldTransform(gear);
							const Matrix3x3 M1 = pose1.mRot;
							const Point& CurrentWorldAxisX1 = M1[2];
							Matrix3x3 FromTo;
							FromTo.FromTo(CurrentWorldAxisX1, axis);
							Desc.mLocalPivot1.mRot	= FromTo;
						}*/

						// Setup support
						Desc.mObject0			= support;
						if(support)
							Desc.mLocalPivot0.mPos	= Point(pos.x - 2.0f, 0.0f, 1.0f + pos.z);
						else
						{
							Desc.mLocalPivot0.mPos	= pos;	// pos = pose0.pos
//							Desc.mLocalPivot0.mRot	= FromTo;
/*							{
								const Point CurrentWorldAxisX0(1.0f, 0.0f, 0.0f);
								Matrix3x3 FromTo;
								FromTo.FromTo(CurrentWorldAxisX0, axis);
								Desc.mLocalPivot0.mRot	= FromTo;
							}*/

							const PR pose0 = pint.GetWorldTransform(gear);
Desc.mLocalPivot0.mPos = pose0.mPos;
/*							const Matrix3x3 M0 = pose0.mRot;
							const Point& CurrentWorldAxisX0 = M0[0];
							Matrix3x3 FromTo;
							FromTo.FromTo(CurrentWorldAxisX0, Point(0.0f, 0.0f, 1.0f));
							Desc.mLocalPivot0.mRot	= FromTo;*/

							Desc.mLocalPivot0.mRot = pose0.mRot * FromTo;
						}

						if(limits)
							Desc.mLimits.Set(-HALFPI/2.0f, HALFPI/2.0f);

						return pint.CreateJoint(Desc);
						}
					}
				}
			};

			const Point DesiredWorldAxis = UseGearAxisX ? Point(1.0f, 0.0f, 0.0f) : Point(0.0f, 0.0f, 1.0f);
			mHingeJoint0 = Local::CreateHinge(pint, Gear0, SupportObject, Copy0.mPosition, DesiredWorldAxis, true);
			mHingeJoint1 = Local::CreateHinge(pint, Gear1, SupportObject, Copy1.mPosition, DesiredWorldAxis, false);

			if(1 && Gear0 && Gear1)
			{
				Matrix3x3 Rot;
				Rot.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
//					Rot.Identity();

				PINT_GEAR_JOINT_CREATE Desc;
				Desc.mHinge0			= mHingeJoint0;
				Desc.mHinge1			= mHingeJoint1;
				Desc.mObject0			= Gear0;
				Desc.mObject1			= Gear1;
				Desc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot0.mRot	= Rot;
				Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1.mRot	= Rot;
				const udword NbTeeth0 = 24;
				const udword NbTeeth1 = 40;
				const float Radius0 = float(NbTeeth0)*0.05f;
				const float Radius1 = float(NbTeeth1)*0.05f;
				Desc.mGearRatio			= Radius0/Radius1;
				//Desc.mRadius0			= Radius0;
				//Desc.mRadius1			= Radius1;
	//				Desc.mLocalPivot0.mPos	= Point(Radius0, 0.0f, 0.0f);
	//				Desc.mLocalPivot1.mPos	= Point(-Radius1, 0.0f, 0.0f);
				PintJointHandle mGearJoint				= pint.CreateJoint(Desc);
				(void)mGearJoint;
			}
		}
		return true;
	}

	virtual	float	DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		if(mHingeJoint0)
		{
			float TwistAngle;
			if(!GetHingeTwistAngle(pint, mHingeJoint0, TwistAngle))
				return y;

			renderer.print(0.0f, y, text_scale, _F("Angle0: %.5f\n", TwistAngle * RADTODEG));	y -= text_scale;
		}
		return y;
	}

END_TEST(TwoLegoGears)

static const char* gDesc_ThreeLegoGears = "Three Lego gears.";

START_TEST(ThreeLegoGears, CATEGORY_JOINTS, gDesc_ThreeLegoGears)

			CylinderMesh		mCylinder;

	virtual	bool	CommonSetup()
	{
		const float Radius = 1.02f;
		const float HalfHeight = 0.396f;

		const udword NbCirclePts = 64;
		mCylinder.Generate(NbCirclePts, Radius, HalfHeight, ORIENTATION_XY);

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		TestBase::CommonRelease();
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		SetDefEnv(desc, false);

		const char* Filename = FindPEELFile("ThreeGears.zb2");
		if(Filename)
		{
			bool status = ImportZB2File(desc, Filename);
			(void)status;

			desc.mCamera[0] = PintCameraPose(Point(0.01f, 2.05f, 3.04f), Point(-0.00f, -0.67f, -0.74f));
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportGearJoints)
			return false;

//		return CreateZB2Scene(pint, caps);

		if(!mZB2Factory)
			return false;

		const udword NbActors = mZB2Factory->GetNbActors();
		const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

		ASSERT(!mZB2Factory->mNbDisabledGroups);
		ASSERT(NbActors==3);
		ASSERT(!mZB2Factory->mJoints.GetNbEntries());

		const PINT_OBJECT_CREATE* Create0 = Actors[0].mCreate;
		const PINT_OBJECT_CREATE* Create1 = Actors[1].mCreate;
		const PINT_OBJECT_CREATE* Create2 = Actors[2].mCreate;
		const PINT_SHAPE_CREATE* Shape0 = Create0->GetFirstShape();
		const PINT_SHAPE_CREATE* Shape1 = Create1->GetFirstShape();
		const PINT_SHAPE_CREATE* Shape2 = Create2->GetFirstShape();

		PINT_OBJECT_CREATE Copy0 = *Create0;
		PINT_OBJECT_CREATE Copy1 = *Create1;
		PINT_OBJECT_CREATE Copy2 = *Create2;

		PINT_CONVEX_CREATE Box0(mCylinder.mNbVerts, mCylinder.mVerts);
		Box0.mLocalPos	= Shape0->mLocalPos;
		Box0.mLocalRot	= Shape0->mLocalRot;
		Box0.mMaterial	= Shape0->mMaterial;
		Box0.mRenderer	= Shape0->mRenderer;
		ASSERT(!Shape0->_GetNext());

		PINT_CONVEX_CREATE Box1(mCylinder.mNbVerts, mCylinder.mVerts);
		Box1.mLocalPos	= Shape1->mLocalPos;
		Box1.mLocalRot	= Shape1->mLocalRot;
		Box1.mMaterial	= Shape1->mMaterial;
		Box1.mRenderer	= Shape1->mRenderer;
		ASSERT(!Shape1->_GetNext());

		PINT_CONVEX_CREATE Box2(mCylinder.mNbVerts, mCylinder.mVerts);
		Box2.mLocalPos	= Shape2->mLocalPos;
		Box2.mLocalRot	= Shape2->mLocalRot;
		Box2.mMaterial	= Shape2->mMaterial;
		Box2.mRenderer	= Shape2->mRenderer;
		ASSERT(!Shape2->_GetNext());

		Copy0.SetShape(&Box0);
		Copy1.SetShape(&Box1);
		Copy2.SetShape(&Box2);
		Copy0.mMass = 1.0f;
		Copy1.mMass = 1.0f;
		Copy2.mMass = 1.0f;
/*
Matrix3x3 Rot;
Rot.RotY(HALFPI);
//Rot.FromTo(Point(), Point());
//Copy1.mRotation = Rot;
Box1.mLocalRot = Rot;
*/
		const PintActorHandle Gear0 = CreatePintObject(pint, Copy0);
		const PintActorHandle Gear1 = CreatePintObject(pint, Copy1);
		const PintActorHandle Gear2 = CreatePintObject(pint, Copy2);

		PintJointHandle HingeJoint0=null;
		if(Gear0)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= null;
			Desc.mObject1		= Gear0;
			Desc.mLocalPivot0	= Copy0.mPosition;
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
			Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
			HingeJoint0			= pint.CreateJoint(Desc);
		}

		PintJointHandle HingeJoint1=null;
		if(Gear1)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= null;
			Desc.mObject1		= Gear1;
			Desc.mLocalPivot0	= Copy1.mPosition;
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalAxis0	= Point(1.0f, 0.0f, 0.0f);
			Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
			HingeJoint1			= pint.CreateJoint(Desc);
		}

		PintJointHandle HingeJoint2=null;
		if(Gear2)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= null;
			Desc.mObject1		= Gear2;
			Desc.mLocalPivot0	= Copy2.mPosition;
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalAxis0	= Point(-1.0f, 0.0f, 0.0f);
			Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
			HingeJoint2			= pint.CreateJoint(Desc);
		}

		if(1 && Gear0 && Gear1)
		{
			Matrix3x3 Rot;
			Rot.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

			PINT_GEAR_JOINT_CREATE Desc;
			Desc.mObject0			= Gear0;
			Desc.mObject1			= Gear1;
			Desc.mHinge0			= HingeJoint0;
			Desc.mHinge1			= HingeJoint1;
			Desc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalPivot0.mRot	= Rot;
			Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalPivot1.mRot	= Rot;
			const float Radius0 = 0.5f;
			const float Radius1 = 0.5f;
			Desc.mGearRatio			= Radius0/Radius1;
			//Desc.mRadius0			= Radius0;
			//Desc.mRadius1			= Radius1;
			PintJointHandle mGearJoint				= pint.CreateJoint(Desc);
			(void)mGearJoint;
		}

		if(1 && Gear0 && Gear2)
		{
			Matrix3x3 Rot;
			Rot.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

			PINT_GEAR_JOINT_CREATE Desc;
			Desc.mObject0			= Gear0;
			Desc.mObject1			= Gear2;
			Desc.mHinge0			= HingeJoint0;
			Desc.mHinge1			= HingeJoint2;
			Desc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalPivot0.mRot	= Rot;
			Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalPivot1.mRot	= Rot;
			const float Radius0 = 0.5f;
			const float Radius1 = 0.5f;
			Desc.mGearRatio			= Radius0/Radius1;
			//Desc.mRadius0			= Radius0;
			//Desc.mRadius1			= Radius1;
			PintJointHandle mGearJoint				= pint.CreateJoint(Desc);
			(void)mGearJoint;
		}

		return true;
	}

END_TEST(ThreeLegoGears)

///////////////////////////////////////////////////////////////////////////////

static void PreprocessMesh(Point& center, Point& extents, const PINT_OBJECT_CREATE* create)
{
	const PINT_SHAPE_CREATE* Shape0 = create->GetFirstShape();
	ASSERT(Shape0 && Shape0->mType==PINT_SHAPE_MESH);
	const PINT_MESH_CREATE* MeshShape = static_cast<const PINT_MESH_CREATE*>(Shape0);

	const udword NbVerts = MeshShape->GetSurface().mNbVerts;
	const Point* Verts = MeshShape->GetSurface().mVerts;

	AABB MeshBounds;
	ComputeAABB(MeshBounds, Verts, NbVerts);
	MeshBounds.GetCenter(center);
	MeshBounds.GetExtents(extents);

	if(1)
	{
		PINT_MESH_CREATE* MeshShapeMut = const_cast<PINT_MESH_CREATE*>(MeshShape);
		Point* V = const_cast<Point*>(Verts);
		for(udword i=0;i<NbVerts;i++)			
			V[i] -= center;

		MeshShapeMut->RecomputeCRC32_Verts();

		MeshShapeMut->mRenderer = CreateMeshRenderer(MeshShapeMut->GetSurface(), gUseActiveEdges);
	}
}

static const char* gDesc_RackAndPinion = "Rack and pinion.";

static const float gRackAndPinionScale = 1.0f;

START_TEST(RackAndPinion, CATEGORY_JOINTS, gDesc_RackAndPinion)

			CylinderMesh		mCylinder;
			Point				mExtents;
			Point				mCenter;

	virtual	bool	CommonSetup()
	{
		const float NbRackTeeths = 5.0f;
		const float NbPinionTeeths = 16.0f;
		const float PinionTravel = TWOPI;
		const float RackTravel = 1.5956f;
		const float dx = RackTravel / NbRackTeeths;
//		(2.0*3.14159)/(16.0*dx)	1.2305676938556522
		const float r = TWOPI/(NbPinionTeeths*dx);
		const float r0 = TWOPI/(NbPinionTeeths*(RackTravel / NbRackTeeths));
		const float r1 = (TWOPI/RackTravel)*(NbRackTeeths/NbPinionTeeths);
		const float r2 = (TWOPI*NbRackTeeths)/(RackTravel*NbPinionTeeths);
		(void)r0;
		(void)r1;
		(void)r2;

		const float Radius = 0.8716f * gRackAndPinionScale;
//		const float Radius = 0.7f;
//		const float Radius = 0.68f;
//		const float Radius = 0.81f;
		const float HalfHeight = 0.396f * gRackAndPinionScale;

		const udword NbCirclePts = 64;
		mCylinder.Generate(NbCirclePts, Radius, HalfHeight, ORIENTATION_XY);

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		TestBase::CommonRelease();
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		SetDefEnv(desc, false);

		const char* Filename = FindPEELFile("RackAndPinion.zb2");
		if(Filename)
		{
			bool status = ImportZB2File(desc, Filename);

			desc.mCamera[0] = PintCameraPose(Point(2.82f, 0.82f, 0.03f), Point(-1.00f, 0.06f, -0.01f));
			desc.mCamera[1] = PintCameraPose(Point(7.70f, 2.43f, -0.31f), Point(-0.99f, -0.09f, 0.06f));

			if(mZB2Factory)
			{
				const udword NbActors = mZB2Factory->GetNbActors();
				const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

				ASSERT(!mZB2Factory->mNbDisabledGroups);
				ASSERT(NbActors==4);
				ASSERT(!mZB2Factory->mJoints.GetNbEntries());

				const PINT_OBJECT_CREATE* Create0 = Actors[0].mCreate;

				PreprocessMesh(mCenter, mExtents, Create0);
			}
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRackJoints)
			return false;

		if(0)
			return CreateZB2Scene(pint, caps);

		if(!mZB2Factory)
			return false;

		const bool UseFiltering = true;
		if(UseFiltering)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			const PintDisabledGroups DG(1, 1);
			pint.SetDisabledGroups(1, &DG);
		}

//		Matrix3x3 SceneRot;
////		SceneRot.RotY(HALFPI);
//		SceneRot.Identity();

		// PhysX doesn't support dynamic meshes so we must replace the shapes with convexes/boxes

		const udword NbActors = mZB2Factory->GetNbActors();
		const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

		// There are 4 meshes in the file, 1 gear and 3 rack parts
		const PINT_OBJECT_CREATE* Create0 = Actors[0].mCreate;
		const PINT_OBJECT_CREATE* Create1 = Actors[1].mCreate;
		const PINT_OBJECT_CREATE* Create2 = Actors[2].mCreate;
		const PINT_OBJECT_CREATE* Create3 = Actors[3].mCreate;
		const PINT_SHAPE_CREATE* Shape0 = Create0->GetFirstShape();
		const PINT_SHAPE_CREATE* Shape1 = Create1->GetFirstShape();
		const PINT_SHAPE_CREATE* Shape2 = Create2->GetFirstShape();
		const PINT_SHAPE_CREATE* Shape3 = Create3->GetFirstShape();

		// Gear
		//{
			PINT_OBJECT_CREATE Copy1 = *Create1;

			PINT_CONVEX_CREATE Convex(mCylinder.mNbVerts, mCylinder.mVerts);
			Convex.mLocalPos	= Shape1->mLocalPos;
			Convex.mLocalRot	= Shape1->mLocalRot;
			Convex.mMaterial	= Shape1->mMaterial;
			Convex.mRenderer	= Shape1->mRenderer;
			ASSERT(!Shape1->_GetNext());

			Copy1.SetShape(&Convex);
			Copy1.mMass = 1.0f;
			Copy1.mPosition *= gRackAndPinionScale;
		//}

		// Rack
		PINT_OBJECT_CREATE Copy0 = *Create0;

		Point Extents = mExtents;
		Point Center = mCenter;
		//PreprocessMesh(Center, Extents, Create0);

		PINT_BOX_CREATE Box0(Extents);
		Box0.mLocalPos	= Shape0->mLocalPos;
//		Box0.mLocalPos -= Center;
		Box0.mLocalRot	= Shape0->mLocalRot;
		Box0.mMaterial	= Shape0->mMaterial;
		Box0.mRenderer	= Shape0->mRenderer;
		ASSERT(!Shape0->_GetNext());

			PINT_BOX_CREATE Box1 = Box0;
			PINT_BOX_CREATE Box2 = Box0;
			PINT_BOX_CREATE Box3 = Box0;
			PINT_BOX_CREATE Box4 = Box0;

			Box1.mLocalPos.z	+= Box0.mExtents.z*2.0f;
			Box2.mLocalPos.z	-= Box0.mExtents.z*2.0f;
			Box3.mLocalPos.z	+= Box0.mExtents.z*4.0f;
			Box4.mLocalPos.z	-= Box0.mExtents.z*4.0f;

			Box0.SetNext(&Box1);
			Box1.SetNext(&Box2);
			Box2.SetNext(&Box3);
			Box3.SetNext(&Box4);

//		Box0.mLocalPos.y += 0.29f*10.0f;
		Copy0.SetShape(&Box0);
		Copy0.mMass = 1.0f;
		Copy0.mPosition += Center;
		Copy0.mPosition *= gRackAndPinionScale;

//		Copy0.mRotation *= SceneRot;
//		Copy1.mRotation *= SceneRot;

			if(0/*UseGearAxisX*/)
			{
				Matrix4x4 SceneRot;
				SceneRot.RotY(HALFPI*0.5f);
//				SceneRot.RotZ(0.8f);

				Matrix4x4 M0(Matrix3x3(Copy0.mRotation), Copy0.mPosition);
				Matrix4x4 M1(Matrix3x3(Copy1.mRotation), Copy1.mPosition);

				M0 *= SceneRot;
				M1 *= SceneRot;

				Copy0.mRotation = M0;
				Copy0.mPosition = M0.GetTrans();

				Copy1.mRotation = M1;
				Copy1.mPosition = M1.GetTrans();
			}

		if(UseFiltering)
		{
			Copy0.mCollisionGroup = 1;
			Copy1.mCollisionGroup = 1;
		}

		const PintActorHandle Rack = CreatePintObject(pint, Copy0);
		const PintActorHandle Gear = CreatePintObject(pint, Copy1);
//		const PintActorHandle Gear2 = CreatePintObject(pint, Copy2);
//		const PintActorHandle Gear3 = CreatePintObject(pint, Copy3);

		if(1)
		{
			PintJointHandle HingeJoint = null;
			if(1 && Gear)
			{
				struct Local
				{
					// Cleaned up version of what we did for gear joint
					// Version with support!=null not touched, probably not working
					// This is hardcoded for the hinge & prismatic axes needed in the file (FromTo)
					static PintJointHandle CreateHinge(Pint& pint, PintActorHandle gear, PintActorHandle support, const Point& pos)
					{
						if(!gear)
							return null;

						Matrix3x3 FromTo;
						FromTo.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

						PINT_HINGE2_JOINT_CREATE Desc;

						// Setup gear
						Desc.mObject1			= gear;
						Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
						Desc.mLocalPivot1.mRot	= FromTo;

						// Setup support
						Desc.mObject0			= support;
						if(support)
							Desc.mLocalPivot0.mPos	= Point(pos.x - 2.0f, 0.0f, 1.0f + pos.z);
						else
						{
							Desc.mLocalPivot0.mPos	= pos;	// pos = pose0.pos

							const PR pose0 = pint.GetWorldTransform(gear);
							Desc.mLocalPivot0.mPos = pose0.mPos;
							Desc.mLocalPivot0.mRot = pose0.mRot * FromTo;
						}
						return pint.CreateJoint(Desc);
					}
				};

/*
				PINT_HINGE2_JOINT_CREATE Desc;
				Desc.mObject0		= null;
				Desc.mObject1		= Gear;
	//			Desc.mLocalPivot0	= Copy1.mPosition;
	//			Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
	//			Desc.mLocalAxis0	= Point(1.0f, 0.0f, 0.0f);
	//			Desc.mLocalAxis1	= Point(1.0f, 0.0f, 0.0f);
				Desc.mLocalPivot0.mPos	= Copy1.mPosition;
//				Desc.mLocalPivot0.mRot	= SceneRot;
				Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
				Matrix3x3 Rot;
				Rot.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
				Desc.mLocalPivot1.mRot	= Rot;
				HingeJoint		= pint.CreateJoint(Desc);*/
				HingeJoint		= Local::CreateHinge(pint, Gear, null, Copy1.mPosition);
			}

			PintJointHandle PrismaticJoint = null;
			if(1 && Rack)
			{
/*				PINT_PRISMATIC_JOINT_CREATE Desc;
				Desc.mObject0			= null;
				Desc.mObject1			= Rack;
				//###PRISMATIC2
				Desc.mLocalPivot0.mPos	= Copy0.mPosition;
				Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalAxis0		= Point(0.0f, 0.0f, 1.0f);//*SceneRot;
				Desc.mLocalAxis1		= Point(0.0f, 0.0f, 1.0f);
				PrismaticJoint			= pint.CreateJoint(Desc);*/

				struct Local
				{
					static PintJointHandle CreatePrismatic(Pint& pint, PintActorHandle rack, PintActorHandle support, const Point& pos)
					{
						if(!rack)
							return null;

						Matrix3x3 FromTo;
						FromTo.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

						PINT_PRISMATIC_JOINT_CREATE Desc;

						Desc.mLimits.Set(-7.0f, 7.0f);

						// Setup gear
						Desc.mObject1			= rack;
						Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
						Desc.mLocalPivot1.mRot	= FromTo;

						// Setup support
						Desc.mObject0			= support;
						if(support)
							Desc.mLocalPivot0.mPos	= Point(pos.x - 2.0f, 0.0f, 1.0f + pos.z);
						else
						{
							Desc.mLocalPivot0.mPos	= pos;	// pos = pose0.pos

							const PR pose0 = pint.GetWorldTransform(rack);
							Desc.mLocalPivot0.mPos = pose0.mPos;
							Desc.mLocalPivot0.mRot = pose0.mRot * FromTo;
						}
						return pint.CreateJoint(Desc);
					}
				};

				PrismaticJoint			= Local::CreatePrismatic(pint, Rack, null, Copy0.mPosition);
			}

			if(1 && Gear && Rack)
			{
				// gear axis = Z
				// prismatic axis = Z
				Matrix3x3 Rot;
				Rot.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

				PINT_RACK_AND_PINION_JOINT_CREATE Desc;
				Desc.mHinge				= HingeJoint;
				Desc.mPrismatic			= PrismaticJoint;
				Desc.mObject0			= Gear;
				Desc.mObject1			= Rack;
				Desc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot0.mRot	= Rot;
				Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1.mRot	= Rot;
				Desc.mNbRackTeeth		= 5;
				Desc.mNbPinionTeeth		= 16;
				Desc.mRackLength		= 1.5956f;
				PintJointHandle RackAndPinionJoint		= pint.CreateJoint(Desc);
			}
		}
		return true;
	}

END_TEST(RackAndPinion)

///////////////////////////////////////////////////////////////////////////////

static PintActorHandle CreateMeshAsBox(Pint& pint, const PINT_OBJECT_CREATE* create, const Point& center, const Point& extents, const Matrix3x3& rot, float mass, const char* name=null)
{
	const PINT_SHAPE_CREATE* Shape = create->GetFirstShape();

	PINT_BOX_CREATE Box0(extents);
	Box0.mLocalPos	= Shape->mLocalPos;
	Box0.mLocalPos += center;
	Box0.mLocalRot	= Shape->mLocalRot;
	Box0.mMaterial	= Shape->mMaterial;
	Box0.mRenderer	= Shape->mRenderer;
	ASSERT(!Shape->_GetNext());

	PINT_OBJECT_CREATE Copy0 = *create;
	Copy0.SetShape(&Box0);
	Copy0.mMass = mass;
//	Copy0.mPosition += Center;

	Copy0.mRotation *= rot;
	Copy0.mCollisionGroup	= 1;
	Copy0.mName = name;

	return CreatePintObject(pint, Copy0);
}

static const char* gDesc_UniversalJoint = "Universal joint.";

START_TEST(UniversalJoint, CATEGORY_JOINTS, gDesc_UniversalJoint)

	Point	mCenter[3];
	Point	mExtents[3];

	virtual	bool	CommonSetup()
	{
		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		TestBase::CommonRelease();
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		SetDefEnv(desc, false);

		const char* Filename = FindPEELFile("UniversalJoint.zb2");
		if(Filename)
		{
			bool status = ImportZB2File(desc, Filename);

			desc.mCamera[0] = PintCameraPose(Point(2.31f, 1.24f, -0.09f), Point(-0.92f, -0.40f, 0.05f));
			desc.mCamera[1] = PintCameraPose(Point(0.02f, 0.84f, 0.86f), Point(0.13f, -0.49f, -0.86f));
			desc.mCamera[2] = PintCameraPose(Point(0.85f, 1.41f, 0.65f), Point(-0.56f, -0.75f, -0.34f));
			//desc.mCamera[0] = PintCameraPose(Point(2.58f, 0.74f, -1.01f), Point(-0.91f, -0.25f, 0.35f));

			if(mZB2Factory)
			{
				const udword NbActors = mZB2Factory->GetNbActors();
				const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

				ASSERT(!mZB2Factory->mNbDisabledGroups);
				ASSERT(NbActors==3);
				ASSERT(!mZB2Factory->mJoints.GetNbEntries());

				for(udword i=0;i<3;i++)
					PreprocessMesh(mCenter[i], mExtents[i], Actors[i].mCreate);
			}
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportHingeJoints || !caps.mSupportCollisionGroups)
			return false;

		if(0)
			return CreateZB2Scene(pint, caps);

		if(!mZB2Factory)
			return false;

		const udword NbActors = mZB2Factory->GetNbActors();
		const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

		ASSERT(!mZB2Factory->mNbDisabledGroups);
		ASSERT(NbActors==3);
		ASSERT(!mZB2Factory->mJoints.GetNbEntries());

		const PintDisabledGroups DG(1, 1);
		pint.SetDisabledGroups(1, &DG);

		Matrix3x3 SceneRot;
//		SceneRot.RotY(HALFPI);
		SceneRot.Identity();

		const PINT_OBJECT_CREATE* Create0 = Actors[0].mCreate;
		const PINT_OBJECT_CREATE* Create1 = Actors[1].mCreate;
		const PINT_OBJECT_CREATE* Create2 = Actors[2].mCreate;

		// Rotate the pieces a bit to make it clear this isn't just a revolute joint
		Matrix3x3 RotY;	RotY.RotY(-HALFPI/2.0f);
		Matrix3x3 RotX;	RotX.RotX(HALFPI/2.0f);

		const float Mass = 1.0f;
		const PintActorHandle Part0 = CreateMeshAsBox(pint, Create0, mCenter[0], mExtents[0], Matrix3x3(Idt)/*SceneRot*/, Mass, "Part0");
		const PintActorHandle Part1 = CreateMeshAsBox(pint, Create1, mCenter[1], mExtents[1], RotX/*SceneRot*/, Mass, "Part1");
		const PintActorHandle Part2 = CreateMeshAsBox(pint, Create2, mCenter[2], mExtents[2], RotY/*SceneRot*/, Mass, "Part2");

		{
			const PR pose0 = pint.GetWorldTransform(Part0);

			const Quat q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

			PINT_HINGE2_JOINT_CREATE Desc;
			Desc.mObject0			= null;
			Desc.mLocalPivot0.mPos	= pose0.mPos;
			Desc.mLocalPivot0.mRot	= pose0.mRot * q;

			Desc.mObject1			= Part0;
			Desc.mLocalPivot1.mRot	= q;

			//Desc.mUseMotor	= true;
			//Desc.mDriveVelocity	= 1.0f;

			pint.CreateJoint(Desc);
		}

		{
			PINT_HINGE2_JOINT_CREATE Desc;
			Desc.mObject0			= Part0;
			Desc.mObject1			= Part1;
			pint.CreateJoint(Desc);
		}

		{
			const Quat q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
			//Matrix3x3 FromTo;
			//FromTo.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
			//const Quat q = FromTo;

			PINT_HINGE2_JOINT_CREATE Desc;
//			PINT_SPHERICAL_JOINT_CREATE Desc;
			Desc.mObject0			= Part1;
			Desc.mObject1			= Part2;
			Desc.mLocalPivot0.mRot	= q.GetConjugate();	// ### ouch
//			Desc.mLocalPivot1.mRot	= q;
			pint.CreateJoint(Desc);
		}

		{
			PR pose2 = pint.GetWorldTransform(Part2);

			const Quat q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

			PINT_HINGE2_JOINT_CREATE Desc;
			Desc.mObject0			= null;
			Desc.mLocalPivot0.mPos	= pose2.mPos;
			Desc.mLocalPivot0.mRot	= pose2.mRot * q;

			Desc.mObject1			= Part2;
			Desc.mLocalPivot1.mRot	= q;

			Desc.mUseMotor			= true;
			Desc.mDriveVelocity		= 5.0f;

			pint.CreateJoint(Desc);
		}

		pint.mUserData = Part0;

		return true;
	}

//		virtual	void			DrawDebugInfo(Pint& pint, PintRender& render)	{}

/*	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		return PrintAngularVelocity(pint, renderer, PintActorHandle(pint.mUserData), y, text_scale);
	}*/

/*	virtual	udword		Update(Pint& pint, float dt)
	{
		if(pint.mUserData)
		{
			Point AngVel = pint.GetAngularVelocity(PintActorHandle(pint.mUserData));
//			printf("%f %f %f\n", AngVel.x, AngVel.y, AngVel.z);
			printf("%f\n", fabsf(AngVel.z)*1000.0f);
		}
		return 0;
	}*/

END_TEST(UniversalJoint)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_JacobsLadder = "Jacob's Ladder";

class JacobsLadder : public TestBase
{
	class LocalTestData : public TestData
	{
		public:
			LocalTestData() : mDynamicObject(null), mJoint(null)	{}
			PintActorHandle	mDynamicObject;
			PintJointHandle		mJoint;
	};

			CheckBoxPtr		mCheckBox_EnableMotor;
			CheckBoxPtr		mCheckBox_TweakInertia;
			EditBoxPtr		mEditBox_TargetVel;
			ComboBoxPtr		mComboBox_JointType;
//			SliderPtr		mSlider;
	public:
							JacobsLadder()				{								}
	virtual					~JacobsLadder()				{								}
	virtual	const char*		GetName()			const	{ return "JacobsLadder";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_JacobsLadder;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;		}

	virtual	float	GetRenderData(Point& center)const	{ return 20.0f;					}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 280;
		WD.mHeight	= 600;
		WD.mLabel	= "JacobsLadder";
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

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Target vel.:", &UIElems);
			mEditBox_TargetVel = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "-2.0", &UIElems, EDITBOX_FLOAT, null, null);
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
			mCheckBox_TweakInertia = helper.CreateCheckBox(UI, 0, 4, y, 100, 20, "Tweak inertia", &UIElems, false, null, null);
			y += YStep;
		}

/*		{
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
		}*/

		y += YStep;
		AddResetButton(UI, 4, y, 264);

		IceTabControl* TabControl;
		{
			TabControlDesc TCD;
			TCD.mParent	= UI;
			TCD.mX		= 4;
			TCD.mY		= y + 30;
			TCD.mWidth	= WD.mWidth - 16;
			TCD.mHeight	= 120;
			TabControl = ICE_NEW(IceTabControl)(TCD);
			RegisterUIElement(TabControl);
		}
		return TabControl;
	}

/*	virtual bool	Setup(Pint& pint, const PintCaps& caps)
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
			const float TargetVel = GetFromEditBox(1.0f, mEditBox_TargetVel, -FLT_MAX, FLT_MAX);

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

	float	mScale;
	float	mTargetVel;

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		mScale = 0.25f;
		mTargetVel = -2.0f;
//		mTargetVel = GetFloat(-2.0f, mEditBox_TargetVel);

		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(3.82f, 2.01f, 3.73f), Point(-0.72f, -0.02f, -0.70f));
		desc.mCamera[0] = PintCameraPose(Point(1.84f, 2.05f, 3.94f), Point(-0.41f, 0.02f, -0.91f));
		desc.mCamera[1] = PintCameraPose(Point(2.29f, 4.37f, 2.59f), Point(-0.59f, -0.49f, -0.65f));
		desc.mCreateDefaultEnvironment	= true;
//		mCreateDefaultEnvironment = false;
//		desc.mGravity.Zero();
//		desc.mGravity = Point(0.0f, -100.0f, 0.0f);
	}

	PintActorHandle CreateObject(Pint& pint, const PINT_SHAPE_CREATE* shape, float mass, const Point& pos, const Quat* rot, PintCollisionGroup group)
	{
		PINT_OBJECT_CREATE ObjectDesc(shape);
		ObjectDesc.mMass			= mass;
		if(mCheckBox_TweakInertia && mCheckBox_TweakInertia->IsChecked())
			ObjectDesc.mMassForInertia	= mass*10.0f;
//			ObjectDesc.mMass	= 1.0f;
		ObjectDesc.mPosition		= pos;
		ObjectDesc.mCollisionGroup	= group;
		if(rot)
			ObjectDesc.mRotation	= *rot;
		return CreatePintObject(pint, ObjectDesc);
	}

	void	Connect(Pint& pint, PINT_HINGE2_JOINT_CREATE& Desc,
		PINT_BOX_CREATE& SmallLinkDesc,
		PINT_BOX_CREATE& BigLinkDesc,
		float SideY,
		PintActorHandle object0, PintActorHandle object1,
		const Point& Object0Pos, const Point& Object1Pos)
	{
//			Desc.mMaxLimitAngle		= -SideY*HALFPI;
//			Desc.mMinLimitAngle		= SideY*HALFPI;

		const float BoxSizeX = 1.5f*mScale;
		const float BoxSizeY = 0.2f*mScale;
		const float BoxSizeZ = 1.0f*mScale;
		const Point Extents(BoxSizeX, BoxSizeY, BoxSizeZ);

		//////////////

		const float SmallLinkSizeX = BoxSizeY;
		const float SmallLinkSizeY = 0.001f;
		const float SmallLinkSizeZ = 0.3f*mScale;
		const Point SmallLinkExtents(SmallLinkSizeX, SmallLinkSizeY, SmallLinkSizeZ);

		//////////////

		const float BigLinkSizeX = BoxSizeX;
		const float BigLinkSizeY = SmallLinkSizeY;
		const float BigLinkSizeZ = SmallLinkSizeZ;
		const Point BigLinkExtents(BigLinkSizeX, BigLinkSizeY, BigLinkSizeZ);

		const float Mass = 1.0f*mScale;

		{
			const Point SmallLink0Pos = Object0Pos + Point(BoxSizeX+SmallLinkSizeX, SideY*BoxSizeY, 0.0f);

				Matrix3x3 Rot;
				Rot.RotZ(-HALFPI*SideY);
				const Quat Q = Rot;
				const Point SmallLink0Pos2 = Object0Pos + Point(BoxSizeX, 0.0f, 0.0f);

			PintActorHandle SmallLink0 = CreateObject(pint, &SmallLinkDesc, Mass, SmallLink0Pos2, &Q, 1);

			Desc.mObject0			= object0;
			Desc.mObject1			= SmallLink0;
			Desc.mLocalPivot0.mPos	= Point(BoxSizeX, SideY*BoxSizeY, 0.0f);
			Desc.mLocalPivot1.mPos	= Point(-SmallLinkSizeX, 0.0f, 0.0f);
			Desc.mLimits.Set(-HALFPI, HALFPI);
			pint.CreateJoint(Desc);
				Desc.mLimits.Set(MIN_FLOAT, MAX_FLOAT);

			/////////

				Matrix3x3 Rot2;
				Rot2.RotZ(HALFPI*SideY);
				const Quat Q2 = Rot2;

			PintActorHandle SmallLink0b[2];
			for(udword j=0;j<2;j++)
			{
				const float ZPos = BoxSizeZ - SmallLinkSizeZ;
				const float OffsetZ = j ? ZPos : -ZPos;
				const Point SmallLink0bPos2 = Object0Pos + Point(-BoxSizeX, 0.0f, OffsetZ);
				SmallLink0b[j] = CreateObject(pint, &SmallLinkDesc, Mass, SmallLink0bPos2, &Q2, 1);

				Desc.mObject0			= object0;
				Desc.mObject1			= SmallLink0b[j];
				Desc.mLocalPivot0.mPos	= Point(-BoxSizeX, SideY*BoxSizeY, OffsetZ);
				Desc.mLocalPivot1.mPos	= Point(SmallLinkSizeX, 0.0f, 0.0f);
				Desc.mLimits.Set(-HALFPI, HALFPI);
				pint.CreateJoint(Desc);
					Desc.mLimits.Set(MIN_FLOAT, MAX_FLOAT);
			}

/*			Desc.mObject0			= Object1;
			Desc.mObject1			= Link0;
			Desc.mLocalPivot0.mPos	= Point(-BoxSizeX, -SideY*BoxSizeY, 0.0f);
			Desc.mLocalPivot1.mPos	= Point(LinkSizeX, 0.0f, 0.0f);
			Desc.mMaxLimitAngle		= HALFPI;
			Desc.mMinLimitAngle		= -HALFPI;
			const PintJointHandle JointHandle1 = pint.CreateJoint(Desc);
			ASSERT(JointHandle1);*/

			/////////

			const Point SmallLink1Pos = Object1Pos + Point(BoxSizeX+SmallLinkSizeX, SideY*BoxSizeY, 0.0f);

				const Point SmallLink1Pos2 = Object1Pos + Point(BoxSizeX, 0.0f, 0.0f);

			PintActorHandle SmallLink1 = CreateObject(pint, &SmallLinkDesc, Mass, SmallLink1Pos2, &Q, 1);

			Desc.mObject0			= object1;
			Desc.mObject1			= SmallLink1;
			Desc.mLocalPivot0.mPos	= Point(BoxSizeX, SideY*BoxSizeY, 0.0f);
			Desc.mLocalPivot1.mPos	= Point(-SmallLinkSizeX, 0.0f, 0.0f);
			Desc.mLimits.Set(-HALFPI, HALFPI);
			pint.CreateJoint(Desc);
					Desc.mLimits.Set(MIN_FLOAT, MAX_FLOAT);

			/////////

			PintActorHandle SmallLink1b[2];
			for(udword j=0;j<2;j++)
			{
				const float ZPos = BoxSizeZ - SmallLinkSizeZ;
				const float OffsetZ = j ? ZPos : -ZPos;
				const Point SmallLink1bPos2 = Object1Pos + Point(-BoxSizeX, 0.0f, OffsetZ);
				SmallLink1b[j] = CreateObject(pint, &SmallLinkDesc, Mass, SmallLink1bPos2, &Q2, 1);

				Desc.mObject0			= object1;
				Desc.mObject1			= SmallLink1b[j];
				Desc.mLocalPivot0.mPos	= Point(-BoxSizeX, SideY*BoxSizeY, OffsetZ);
				Desc.mLocalPivot1.mPos	= Point(SmallLinkSizeX, 0.0f, 0.0f);
				Desc.mLimits.Set(-HALFPI, HALFPI);
				pint.CreateJoint(Desc);
					Desc.mLimits.Set(MIN_FLOAT, MAX_FLOAT);
			}

			/////////

			const Point BigLink1Pos = Object0Pos + Point(BoxSizeX+BigLinkSizeX, -SideY*BoxSizeY, 0.0f);
			PintActorHandle BigLink1 = CreateObject(pint, &BigLinkDesc, Mass, BigLink1Pos, null, 1);

			Desc.mObject0			= BigLink1;
			Desc.mObject1			= SmallLink0;
			Desc.mLocalPivot0.mPos	= Point(-BigLinkSizeX, 0.0f, 0.0f);
			Desc.mLocalPivot1.mPos	= Point(SmallLinkSizeX, 0.0f, 0.0f);
//			Desc.mMaxLimitAngle		= HALFPI;
//			Desc.mMinLimitAngle		= -HALFPI;
			pint.CreateJoint(Desc);

			Desc.mObject0			= BigLink1;
			Desc.mObject1			= SmallLink1;
			Desc.mLocalPivot0.mPos	= Point(BigLinkSizeX, 0.0f, 0.0f);
			Desc.mLocalPivot1.mPos	= Point(SmallLinkSizeX, 0.0f, 0.0f);
//			Desc.mMaxLimitAngle		= HALFPI;
//			Desc.mMinLimitAngle		= -HALFPI;
			pint.CreateJoint(Desc);

			/////////

			for(udword j=0;j<2;j++)
			{
				const float ZPos = BoxSizeZ - SmallLinkSizeZ;
				const float OffsetZ = j ? ZPos : -ZPos;

				const Point BigLink0bPos = Object0Pos + Point(0.0f, -SideY*BoxSizeY, OffsetZ);
				PintActorHandle BigLink0b = CreateObject(pint, &BigLinkDesc, Mass, BigLink0bPos, null, 1);

				Desc.mObject0			= BigLink0b;
				Desc.mObject1			= SmallLink0b[j];
				Desc.mLocalPivot0.mPos	= Point(-BigLinkSizeX, 0.0f, 0.0f);
				Desc.mLocalPivot1.mPos	= Point(-SmallLinkSizeX, 0.0f, 0.0f);
	//			Desc.mMaxLimitAngle		= HALFPI;
	//			Desc.mMinLimitAngle		= -HALFPI;
				pint.CreateJoint(Desc);

				Desc.mObject0			= BigLink0b;
				Desc.mObject1			= SmallLink1b[j];
				Desc.mLocalPivot0.mPos	= Point(BigLinkSizeX, 0.0f, 0.0f);
				Desc.mLocalPivot1.mPos	= Point(-SmallLinkSizeX, 0.0f, 0.0f);
	//			Desc.mMaxLimitAngle		= HALFPI;
	//			Desc.mMinLimitAngle		= -HALFPI;
				pint.CreateJoint(Desc);
			}
		}
	}

	PintShapeRenderer*	__CreateMeshRenderer(const Point& extents)	const
	{
		AABB Box;
		Box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), extents);

		Point Pts[8];
		Box.ComputePoints(Pts);

		SurfaceInterface SI;
		SI.mNbVerts	= 8;
		SI.mVerts	= Pts;
		SI.mDFaces	= Box.GetTriangles();
		SI.mNbFaces	= 12;

		return CreateMeshRenderer(PintSurfaceInterface(SI));
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

			const PintDisabledGroups DG(1, 1);
			pint.SetDisabledGroups(1, &DG);
		}

//		PINT_HINGE_JOINT_CREATE Desc;
//		Desc.mLocalAxis0 = Point(0.0f, 0.0f, 1.0f);
//		Desc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);
		PINT_HINGE2_JOINT_CREATE Desc;
		{
			const Quat Q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
			Desc.mLocalPivot0.mRot = Q;
			Desc.mLocalPivot1.mRot = Q;
		}

		//////////////

		const float BoxSizeX = 1.5f*mScale;
		const float BoxSizeY = 0.2f*mScale;
		const float BoxSizeZ = 1.0f*mScale;
		const Point Extents(BoxSizeX, BoxSizeY, BoxSizeZ);
		PINT_BOX_CREATE ObjectDesc(Extents);
		ObjectDesc.mRenderer = CreateBoxRenderer(Extents);
		if(0)
		{
			ObjectDesc.mRenderer = __CreateMeshRenderer(Extents);
			ObjectDesc.mRenderer = CreateColorShapeRenderer(ObjectDesc.mRenderer, RGBAColor(248.0f/255.0f, 232.0f/255.0f, 199.0f/255.0f));
		}

		//////////////

		const float ZFightingTweak = 0.01f;

		const float SmallLinkSizeX = BoxSizeY;
		const float SmallLinkSizeY = 0.001f;
		const float SmallLinkSizeZ = 0.3f*mScale;
		const Point SmallLinkExtents(SmallLinkSizeX, SmallLinkSizeY, SmallLinkSizeZ);
		PINT_BOX_CREATE SmallLinkDesc(SmallLinkExtents);
		if(1)
		{
			SmallLinkDesc.mRenderer = __CreateMeshRenderer(SmallLinkExtents + Point(0.0f, ZFightingTweak, 0.0f));
			SmallLinkDesc.mRenderer = CreateColorShapeRenderer(SmallLinkDesc.mRenderer, RGBAColor(0.0f, 0.0f, 1.0f));
		}
		else
		{
			SmallLinkDesc.mRenderer = CreateBoxRenderer(SmallLinkExtents + Point(0.0f, ZFightingTweak, 0.0f));
			SmallLinkDesc.mRenderer = CreateColorShapeRenderer(SmallLinkDesc.mRenderer, RGBAColor(0.0f, 0.0f, 1.0f));
		}

		//////////////

		const float BigLinkSizeX = BoxSizeX;
		const float BigLinkSizeY = SmallLinkSizeY;
		const float BigLinkSizeZ = SmallLinkSizeZ;
		const Point BigLinkExtents(BigLinkSizeX, BigLinkSizeY, BigLinkSizeZ);
		PINT_BOX_CREATE BigLinkDesc(BigLinkExtents);
		if(1)
		{
			BigLinkDesc.mRenderer = __CreateMeshRenderer(BigLinkExtents + Point(0.0f, ZFightingTweak, 0.0f));
			BigLinkDesc.mRenderer = CreateColorShapeRenderer(BigLinkDesc.mRenderer, RGBAColor(0.0f, 0.0f, 1.0f));
		}
		else
		{
			BigLinkDesc.mRenderer = CreateBoxRenderer(BigLinkExtents + Point(0.0f, ZFightingTweak, 0.0f));
			BigLinkDesc.mRenderer = CreateColorShapeRenderer(BigLinkDesc.mRenderer, RGBAColor(0.0f, 0.0f, 1.0f));
		}

		//////////////

//#define DEBUG_EXPORT

#ifdef DEBUG_EXPORT
	#define NB_OBJECTS	1
#else
	#define NB_OBJECTS	6
#endif
		PintActorHandle Objects[NB_OBJECTS];
		Point ObjectPos[NB_OBJECTS];

		const Point Disp(BoxSizeX*2.0f, 0.0f, 0.0f);
		const Point Object0Pos(0.0f, 4.0f, 0.0f);

		if(1)
		{
			const float Mass = 10.0f*mScale;
			ObjectPos[0] = Object0Pos;
			Objects[0] = CreateObject(pint, &ObjectDesc, Mass, Object0Pos, null, 2);
			{
				// TODO: revisit this API
				PINT_HINGE_JOINT_CREATE fjc;
				fjc.mLocalAxis0 = Point(0.0f, 0.0f, 1.0f);
				fjc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);
				fjc.mObject0 = null;
				fjc.mObject1 = Objects[0];
				fjc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
				fjc.mLocalPivot0 = Object0Pos;
					fjc.mLimits.Set(-270.0f*DEGTORAD, 0.0f*DEGTORAD);
					//fjc.mLimits.Set(-270.0f*DEGTORAD, -90.0f*DEGTORAD);

#ifndef DEBUG_EXPORT
				if(true)
				{
					fjc.mUseMotor		= true;
					fjc.mDriveVelocity	= 0.0f;
				}
#endif

				PintJointHandle j0 = pint.CreateJoint(fjc);

				LocalTestData* LTD = ICE_NEW(LocalTestData);
				RegisterTestData(LTD);
				LTD->mDynamicObject = Objects[0];
				LTD->mJoint			= j0;
				pint.mUserData		= LTD;
			}
/*			Objects[1] = CreateObject(pint, &ObjectDesc, Mass, Object1Pos, null, 2);
			Objects[2] = CreateObject(pint, &ObjectDesc, Mass, Object2Pos, null, 2);
			Objects[3] = CreateObject(pint, &ObjectDesc, Mass, Object3Pos, null, 2);
			Objects[4] = CreateObject(pint, &ObjectDesc, Mass, Object4Pos, null, 2);
			Objects[5] = CreateObject(pint, &ObjectDesc, Mass, Object5Pos, null, 2);*/
			for(udword i=0;i<NB_OBJECTS-1;i++)
			{
				ObjectPos[i+1] = ObjectPos[i] + Disp;
				Objects[i+1] = CreateObject(pint, &ObjectDesc, Mass, ObjectPos[i+1], null, 2);
			}
		}

/*		Connect(pint, Desc, SmallLinkDesc, BigLinkDesc, -1.0f, Objects[0], Objects[1], Object0Pos, Object1Pos);
		Connect(pint, Desc, SmallLinkDesc, BigLinkDesc, 1.0f, Objects[1], Objects[2], Object1Pos, Object2Pos);
		Connect(pint, Desc, SmallLinkDesc, BigLinkDesc, -1.0f, Objects[2], Objects[3], Object2Pos, Object3Pos);
		Connect(pint, Desc, SmallLinkDesc, BigLinkDesc, 1.0f, Objects[3], Objects[4], Object3Pos, Object4Pos);
		Connect(pint, Desc, SmallLinkDesc, BigLinkDesc, -1.0f, Objects[4], Objects[5], Object4Pos, Object5Pos);*/
		for(udword i=0;i<NB_OBJECTS-1;i++)
			Connect(pint, Desc, SmallLinkDesc, BigLinkDesc, (i&1) ? 1.0f : -1.0f, Objects[i], Objects[i+1], ObjectPos[i], ObjectPos[i+1]);

		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
#ifndef DEBUG_EXPORT
		const bool EnableMotor = mCheckBox_EnableMotor ? mCheckBox_EnableMotor->IsChecked() : true;
		const LocalTestData* LTD = (const LocalTestData*)pint.mUserData;
		if(LTD)
		{
//			const float Coeff = mSlider ? mSlider->GetValue() : 1.0f;
//			const float TargetVel = Coeff * GetFromEditBox(1.0f, mEditBox_TargetVel, -FLT_MAX, FLT_MAX);
//			printf("TargetVel: %f\n", TargetVel);
//			static float TargetVel = -2.0f;

			if(mCurrentTime>4.0f)
			{
				mCurrentTime = 0.0f;
				mTargetVel = -mTargetVel;
			}

			if(LTD->mJoint)
			{
				pint.SetDriveEnabled(LTD->mJoint, EnableMotor);
				if(EnableMotor)
					pint.SetDriveVelocity(LTD->mJoint, Point(0.0f, 0.0f, 0.0f), Point(mTargetVel, 0.0f, 0.0f));
			}
			else
			{
				pint.SetRCADriveEnabled(LTD->mDynamicObject, EnableMotor);
				if(EnableMotor)
					pint.SetRCADriveVelocity(LTD->mDynamicObject, mTargetVel);
			}
		}
#endif
		return 0;
	}

END_TEST(JacobsLadder)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_HingeJointMotors = "Motorized hinges test scene from APE.";

START_TEST(HingeJointMotors, CATEGORY_JOINTS, gDesc_HingeJointMotors)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.25f, 12.43f, 41.98f), Point(0.01f, -0.37f, -0.93f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual void		Close(Pint& pint)
	{
		PintActorHandle* UserData = (PintActorHandle*)pint.mUserData;
		ICE_FREE(UserData);
		pint.mUserData = null;

		TestBase::Close(pint);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const float ex = 1.0f;
		const float ey = 1.0f;
		float ez = 1.0f;

		float x = 0.0f;
		float y = 1.0f;
		float z = 0.0f;

		PintActorHandle box0;
		{
			const Point Extents(ex, ey, ez);
			const Point StaticPos(x, y, z);

			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateBoxRenderer(Extents);

			box0 = CreateStaticObject(pint, &BoxDesc, StaticPos);
		}

		const float growth = 1.5f;
	//	float velocity = 0.25f;
		float velocity = 1.0f;

		const udword Nb = 5;
		PintActorHandle* Handles = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*Nb);
		pint.mUserData = Handles;

		for(udword i=0;i<Nb;i++)
		{
			x += ex*2.0f;
			z += ez;

			const Point localAnchor0(ex, 0.0f, ez);

			ez *= growth;
			z += ez;
			PintActorHandle box1;
			{
				const Point Extents(ex, ey, ez);
				const Point DynamicPos(x, y, z);

				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//				box1 = CreateDynamicObject(pint, &BoxDesc, DynamicPos);
				box1 = CreateSimpleObject(pint, &BoxDesc, float(i)+1.0f, DynamicPos);
				Handles[i] = box1;
			}

			{
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= box0;
				Desc.mObject1		= box1;
				Desc.mLocalPivot0	= localAnchor0;
				Desc.mLocalPivot1	= Point(-ex, 0.0f, -ez);
				Desc.mLocalAxis0	= Point(1.0f, 0.0f, 0.0f);
				Desc.mLocalAxis1	= Point(1.0f, 0.0f, 0.0f);

				if(1)
				{
					Desc.mUseMotor		= true;
					Desc.mDriveVelocity	= velocity;
				}

				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}

			box0 = box1;

//			velocity *= 2.0f;
		}
		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
		if(0)
		{
			const PintActorHandle* Handles = (const PintActorHandle*)pint.mUserData;
			for(udword i=0;i<5;i++)
			{
				pint.SetAngularVelocity(Handles[i], Point(float(i+1), 0.0f, 0.0f));
			}
		}
		return 0;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintActorHandle* Handles = (const PintActorHandle*)pint.mUserData;
		for(udword i=0;i<5;i++)
			y = PrintAngularVelocity(pint, renderer, Handles[i], y, text_scale);
		return y;
	}

END_TEST(HingeJointMotors)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_FixedJointCantilever = "Cantilever beam.";

START_TEST(FixedJointCantilever, CATEGORY_JOINTS, gDesc_FixedJointCantilever)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("FixedJointCantilever", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(13.65f, 25.41f, 15.88f), Point(-0.25f, -0.44f, -0.86f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportFixedJoints || !caps.mSupportRigidBodySimulation)
			return false;

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		Point Pos(0.0f, 20.0f, 0.0f);
		PintActorHandle PrevObject = CreateStaticObject(pint, &BoxDesc, Pos);

		const Point Disp(BoxSize*2.0f, 0.0f, 0.0f);

		for(udword i=0;i<8;i++)
		{
			Pos += Disp;
			PintActorHandle NewObject = CreateDynamicObject(pint, &BoxDesc, Pos);

			PINT_FIXED_JOINT_CREATE Desc;
			Desc.mObject0		= PrevObject;
			Desc.mObject1		= NewObject;
			Desc.mLocalPivot0	= Disp*0.5f;
			Desc.mLocalPivot1	= -Disp*0.5f;

			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);

			PrevObject = NewObject;
		}
		return true;
	}

END_TEST(FixedJointCantilever)

///////////////////////////////////////////////////////////////////////////////

static void GenerateFixedJointsTorus(Pint& pint, const Point& torus_pos, const Quat& torus_rot, float mass, udword nb_loops, bool use_rca)
{
	// Generate torus. We generate a torus by sweeping a small circle along a large circle, taking N samples (slices) along the way.
	const float BigRadius = 3.0f;
	const float SmallRadius = 1.0f;
//	const float SmallRadius = 0.75f;
//	const udword NbPtsSmallCircle = 8;
//	const udword NbSlices = 16;
	const udword NbPtsSmallCircle = 16;
	const udword NbSlices = 32;
//	const udword NbSlices = 8;

	// First we generate a small vertical template circle, oriented in the XY plane.
	Point SmallCirclePts[NbPtsSmallCircle];
	GeneratePolygon(NbPtsSmallCircle, SmallCirclePts, sizeof(Point), ORIENTATION_XY, SmallRadius);

	// We'll be sweeping this initial circle along a curve (a larger circle), taking N slices along the way.
	// The final torus will use these vertices exclusively so the total #verts is:
	const udword TotalNbVerts = NbPtsSmallCircle * NbSlices;
	Point* Verts = ICE_NEW(Point)[TotalNbVerts];

	// Now we do the sweep along the larger circle.
	Point SliceCenters[NbSlices];
	{
		const Matrix3x3 TRot = torus_rot;

		udword Index = 0;
		for(udword j=0;j<NbSlices;j++)
		{
			const float Coeff = float(j)/float(NbSlices);

			// We rotate and translate the template circle to position it along the larger circle.
			Matrix3x3 Rot;
			Rot.RotX(Coeff * TWOPI);

			const Point Trans = Rot[1]*BigRadius;
			for(udword i=0;i<NbPtsSmallCircle;i++)
				Verts[Index++] = (Trans + SmallCirclePts[i]*Rot)*TRot;

			SliceCenters[j] = Trans*TRot;
		}
		ASSERT(Index==TotalNbVerts);
	}
	// Here we have generated all the vertices.

	PintArticHandle Articulation = null;
	if(0 && mass!=0.0f)
		Articulation = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
	PintArticHandle RCArticulation = null;
	if(use_rca)
		RCArticulation = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(mass==0.0f));

	// Next, we generate a convex object for each part of the torus. A part is a section connecting two of the previous slices.
	PintActorHandle Handles[NbSlices];
	Point ObjectCenters[NbSlices];

	for(udword s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;
	}

	PintActorHandle RCA_Root = null;
	if(RCArticulation)
	{
		if(mass==0.0f)
			mass = 1.0f;
		// Dummy root object in the center of the torus
		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mMass		= mass;
		ObjectDesc.mPosition	= torus_pos;

		PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
		ArticulatedDesc.mFrictionCoeff	= 0.0f;
		RCA_Root = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCArticulation);
	}

	udword GroupBit = 0;
	for(int s=0;s<NbSlices;s++)
	{
		const udword SliceIndex0 = s;
		const udword SliceIndex1 = (s+1)%NbSlices;
		// V0 and V1 point to the slices' vertices.
		const Point* V0 = Verts + SliceIndex0*NbPtsSmallCircle;
		const Point* V1 = Verts + SliceIndex1*NbPtsSmallCircle;

		// Each convex connects two slices and thus contains twice the amount of vertices in a single slice.
		Point ConvexPts[NbPtsSmallCircle*2];
		for(udword i=0;i<NbPtsSmallCircle;i++)
		{
			ConvexPts[i] = V0[i];
			ConvexPts[i+NbPtsSmallCircle] = V1[i];
		}

		// Recenter vertices
		Point Center(0.0f, 0.0f, 0.0f);
		{
			const float Coeff = 1.0f / float(NbPtsSmallCircle*2);
			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				Center += ConvexPts[i] * Coeff;

			for(udword i=0;i<NbPtsSmallCircle*2;i++)
				ConvexPts[i] -= Center;
		}
		ObjectCenters[s] = Center;

		// Now we create the convex object itself
		PINT_CONVEX_CREATE ConvexCreate(NbPtsSmallCircle*2, ConvexPts);
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexPts);

//		Handles[s] = CreateDynamicObject(pint, &ConvexCreate, pos+Center);
		{
			PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
			ObjectDesc.mMass			= mass;
			ObjectDesc.mPosition		= torus_pos+Center;
			// Each convex has been computed from rotated small circles, and we only re-centered the verts but
			// didn't cancel the rotation. In theory we'd only need one convex object that we could translate/rotate,
			// but it was easier to create different parts with different rotations. As a result we don't need to set
			// a rotation for the object itself, it's captured in the vertices.
//			ObjectDesc.mRotation		= torus_rot;
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;

			if(Articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = s ? Handles[s-1] : null;

				const udword SliceIndex0 = (s-1)%NbSlices;
				const udword SliceIndex1 = (s)%NbSlices;
				ArticulatedDesc.mLocalPivot0	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
				ArticulatedDesc.mLocalPivot1	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];

				if(1)
				{
//					ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
					ArticulatedDesc.mEnableTwistLimit = true;
					ArticulatedDesc.mTwistLowerLimit = -0.0001f;
					ArticulatedDesc.mTwistUpperLimit = 0.0001f;
					ArticulatedDesc.mEnableSwingLimit = true;
					ArticulatedDesc.mSwingYLimit = 0.001f;
					ArticulatedDesc.mSwingZLimit = 0.001f;
				}

				Handles[s] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
			}
			else if(RCArticulation)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType	= PINT_JOINT_FIXED;
				ArticulatedDesc.mParent = s ? Handles[s-1] : RCA_Root;
				ArticulatedDesc.mFrictionCoeff	= 0.0f;

				if(s)
				{
					const udword SliceIndex0 = (s-1)%NbSlices;
					const udword SliceIndex1 = (s)%NbSlices;
					ArticulatedDesc.mLocalPivot0.mPos	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
					ArticulatedDesc.mLocalPivot1.mPos	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];
				}
				else
				{
					ArticulatedDesc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
					ArticulatedDesc.mLocalPivot1.mPos	= -ObjectCenters[0];
				}

				Handles[s] = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCArticulation);
			}
			else
				Handles[s] = CreatePintObject(pint, ObjectDesc);
		}
		ASSERT(Handles[s]);
	}
	DELETEARRAY(Verts);

	if(Articulation)
		pint.AddArticulationToScene(Articulation);
	if(RCArticulation)
		pint.AddRCArticulationToScene(RCArticulation);

	if(!Articulation && !RCArticulation && mass!=0.0f)
	{
		for(udword i=0;i<nb_loops;i++)
		{
			for(udword s=0;s<NbSlices;s++)
			{
				const udword SliceIndex0 = s;
				const udword SliceIndex1 = (s+1)%NbSlices;

				PINT_FIXED_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[SliceIndex0];
				Desc.mObject1		= Handles[SliceIndex1];
				Desc.mLocalPivot0	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex0];
				Desc.mLocalPivot1	= SliceCenters[SliceIndex1] - ObjectCenters[SliceIndex1];
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_FixedJointsTorusXP = "Fixed-joints / articulation torus experiments.";

static const char* gDesc_FixedJointsTorus_Regular =
"A torus made of 32 convexes connected by fixed joints.\n\n\
Ideally the torus should behave like a rigid body compound, but\n\
iterative solvers make it look like a soft body. Increasing the number\n\
of solver iterations typically makes it look more rigid - the more\n\
iterations the closer to the ideal. Different engines give a different\n\
softness by default, and have a different cost for each additional\n\
solver iteration. The perceived rigidity can also vary depending on\n\
selected solver.\n\n\
You can pick & drag the torus to check each engine's stability.\n\
There could also be a per-test UI to change the number of iterations\n\
at runtime.";

static const char* gDesc_FixedJointsTorus_Multiple =
"Increasing the number of solver iterations can be costly, especially\n\
in engines that do not support customizing it per-actor.\n\n\
Instead, one can sometimes create the same fixed joints constraints\n\
multiple times (8 in this example). This can have the same effect\n\
overall (increasing the torus' rigidity) but with a more controllable\n\
cost. The cost here is localized to the jointed object, while the cost\n\
of increasing the number of solver iterations can spread to a full\n\
simulation island (or to the whole scene in some engines).\n\n\
On the other hand this uses more memory than adding extra solver\n\
iterations and it does not work well in all engines. So this is not\n\
a recommended technique overall.";

static const char* gDesc_FixedJointsTorus_Articulation =
"Another option is to use an articulation instead of regular joints.\n\
They can provide perfect rigidity out-of-the-box.\n\n\
Of course in this case it would be even simpler and better to just use\n\
a rigid body compound (without joints).";

struct FixedJointsTorusPreset
{
	const char*	mDesc;
	udword		mNbLoops;
};

#define NB_FIXED_JOINTS_TORUS_PRESETS	3
static const FixedJointsTorusPreset gPreset_FixedJointsTorus[NB_FIXED_JOINTS_TORUS_PRESETS] = {
	{gDesc_FixedJointsTorus_Regular},
	{gDesc_FixedJointsTorus_Multiple},
	{gDesc_FixedJointsTorus_Articulation},
};

class FixedJointsTorus : public TestBase
{
			EditBoxPtr		mEditBox_Desc;
			ComboBoxPtr		mComboBox_Preset;
			CheckBoxPtr		mCheckBox_StressTest;
	public:
							FixedJointsTorus()			{									}
	virtual					~FixedJointsTorus()			{									}
	virtual	const char*		GetName()			const	{ return "FixedJointsTorus";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_FixedJointsTorusXP;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;			}

	virtual	float	GetRenderData(Point& center)const
	{
		if(mCheckBox_StressTest && mCheckBox_StressTest->IsChecked())
			return 800.0f;
		else
			return 200.0f;
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 350;
		WD.mHeight	= 550;
		WD.mLabel	= "FixedJointsTorus";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 80;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Presets:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				FixedJointsTorus&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, FixedJointsTorus& test) :
									IceComboBox(desc),
									mTest(test)	{}
				virtual			~MyComboBox()	{}
				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
						const udword SelectedIndex = GetSelectedIndex();
						const bool Enabled = SelectedIndex==GetItemCount()-1;
						if(SelectedIndex<NB_FIXED_JOINTS_TORUS_PRESETS)
							mTest.mEditBox_Desc->SetMultilineText(gPreset_FixedJointsTorus[SelectedIndex].mDesc);
						else
							mTest.mEditBox_Desc->SetMultilineText("User-defined");
						mTest.mMustResetTest = true;
					}
				}
			};

			ComboBoxDesc CBBD;
			CBBD.mID		= 0;
			CBBD.mParent	= UI;
			CBBD.mX			= 4+OffsetX;
			CBBD.mY			= y;
			CBBD.mWidth		= 250;
			CBBD.mHeight	= 20;
			CBBD.mLabel		= "Presets";
			mComboBox_Preset = ICE_NEW(MyComboBox)(CBBD, *this);
			RegisterUIElement(mComboBox_Preset);
			mComboBox_Preset->Add("FixedJointsTorus - PEEL 1.1 test");
			mComboBox_Preset->Add("FixedJointsTorusMultipleConstraints - PEEL 1.1 test");
			mComboBox_Preset->Add("FixedJointsTorus articulation");
			mComboBox_Preset->Select(0);
			mComboBox_Preset->SetVisible(true);
			y += YStep;

			y += YStep;
			mEditBox_Desc = helper.CreateEditBox(UI, 0, 4, y, 350-16, 200, "", &UIElems, EDITBOX_TEXT, null);
			mEditBox_Desc->SetReadOnly(true);
			y += 200;

			mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
		}

		struct Local{ static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data) { *((bool*)user_data) = true;	}};
		mCheckBox_StressTest = helper.CreateCheckBox(UI, 0, 4, y, 200, 20, "Use more complex 'stress test' scene", &UIElems, false, Local::gCheckBoxCallback, null);
		mCheckBox_StressTest->SetUserData(&mMustResetTest);
		y += YStep;

		y += YStep;
		AddResetButton(UI, 4, y, 350-16);

		y += YStep;
		y += YStep;
		IceTabControl* TabControl;
		{
			TabControlDesc TCD;
			TCD.mParent	= UI;
			TCD.mX		= 4;
			TCD.mY		= y;
			TCD.mWidth	= WD.mWidth - 16;
			TCD.mHeight	= 140;
			TabControl = ICE_NEW(IceTabControl)(TCD);
			RegisterUIElement(TabControl);
		}
		return TabControl;
	}

	virtual	const char*		GetSubName()	const
	{
		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==0)
				return "Regular";
			else if(SelectedIndex==1)
				return "MultipleConstraints";
			else if(SelectedIndex==2)
				return "Articulation";
		}
		return null;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		if(mCheckBox_StressTest->IsChecked())
			desc.mCamera[0] = PintCameraPose(Point(55.35f, 49.71f, 55.94f), Point(-0.66f, -0.01f, -0.75f));
		else
			desc.mCamera[0] = PintCameraPose(Point(15.66f, 7.75f, 14.53f), Point(-0.71f, -0.18f, -0.68f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes || !caps.mSupportCollisionGroups)
			return false;

		bool UseRCA = false;
		if(mComboBox_Preset->GetSelectedIndex()==2)
		{
			if(!caps.mSupportRCArticulations)
				return false;
			UseRCA = true;
		}

		udword NbLoops = 1;
		if(mComboBox_Preset->GetSelectedIndex()==1)
			NbLoops = 8;

		const PintDisabledGroups DG(1, 2);
		pint.SetDisabledGroups(1, &DG);

		if(!mCheckBox_StressTest->IsChecked())
		{
			const Point pos(0.0f, 10.0f, 0.0f);
			const Quat Q(1.0f, 0.0f, 0.0f, 0.0f);
			GenerateFixedJointsTorus(pint, pos, Q, 1.0f, NbLoops, UseRCA);
		}
		else
		{
			Matrix3x3 Rot;	Rot.RotY(degToRad(90.0f));
			const Quat RQ = Rot;
			const udword NbTorus = 10;
			for(udword i=0;i<NbTorus;i++)
			{
				const Point pos(0.0f, 50.0f+float(i)*4.0f, 0.0f);
				const Quat Q(1.0f, 0.0f, 0.0f, 0.0f);
				GenerateFixedJointsTorus(pint, pos, i&1 ? Q : RQ, i==(NbTorus-1) ? 0.0f : 1.0f, NbLoops, UseRCA);
			}
		}
		return true;
	}

}FixedJointsTorus;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_BridgeUsingHinges = "Bridges made of 20 planks connected by hinge joints. Pick & drag objects to check each engine's stability.";

START_TEST(BridgeUsingHinges, CATEGORY_JOINTS, gDesc_BridgeUsingHinges)

	virtual	float	GetRenderData(Point& center)const
	{
		return 800.0f;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(42.34f, 49.89f, 132.99f), Point(-0.72f, -0.50f, -0.48f));
		desc.mCamera[1] = PintCameraPose(Point(18.33f, 41.38f, 155.88f), Point(-0.03f, -0.48f, -0.88f));
		SetDefEnv(desc, true);
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

		const Point Extents(1.0f, 0.1f, 2.0f);
		const udword NbBoxes = 20;
		const udword NbRows = 20;
		const Point Dir(1.0f, 0.0f, 0.0f);
	//	const Point PosOffset = Dir*(Extents.x + 0.1f);
		const Point PosOffset = Dir*Extents.x;

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

		for(udword j=0;j<NbRows;j++)
		{
			PintActorHandle Handles[NbBoxes];
			Point Positions[NbBoxes];

			Point Pos(0.0f, 40.0f, float(j)*Extents.z*4.0f);
			Positions[0] = Pos;

			udword GroupBit = 0;
	//		Handles[0] = CreateStaticObject(pint, &BoxDesc, Pos);
			{
				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 0.0f;
				ObjectDesc.mPosition		= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
				Handles[0] = CreatePintObject(pint, ObjectDesc);
			}
			Pos += PosOffset*2.0f;

			for(udword i=1;i<NbBoxes-1;i++)
			{
				Positions[i] = Pos;
	//			Handles[i] = CreateDynamicObject(pint, &BoxDesc, Pos);
				{
					PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
					ObjectDesc.mMass			= 1.0f;
					ObjectDesc.mPosition		= Pos;
					ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
					Handles[i] = CreatePintObject(pint, ObjectDesc);
				}
				Pos += PosOffset*2.0f;
				// you must allow a wiggle room otherwise the bridge the mass matrix is singular leading to a explosion of engine with force base solvers
//				Pos += (PosOffset - Point (Extents.x * 0.01f, 0.0f, 0.0f)) *2.0f;
			}
			Positions[NbBoxes-1] = Pos;
			{
				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 0.0f;
				ObjectDesc.mPosition		= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
				Handles[NbBoxes-1] = CreatePintObject(pint, ObjectDesc);
			}
			Pos += PosOffset*2.0f;

			for(udword i=0;i<NbBoxes-1;i++)
			{
				if(1)
				{
					PINT_HINGE_JOINT_CREATE Desc;
					Desc.mObject0		= Handles[i];
					Desc.mObject1		= Handles[i+1];
					Desc.mLocalPivot0	= PosOffset;
					Desc.mLocalPivot1	= -PosOffset;
		//			Desc.mLocalAxis0	= Point(1.0f, 0.0f, 0.0f);
		//			Desc.mLocalAxis1	= Point(1.0f, 0.0f, 0.0f);
		//			Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
		//			Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
					Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
					Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
				else
				{
					PINT_FIXED_JOINT_CREATE Desc;
					Desc.mObject0		= Handles[i];
					Desc.mObject1		= Handles[i+1];
					Desc.mLocalPivot0	= PosOffset;
					Desc.mLocalPivot1	= -PosOffset;
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}
		return true;
	}

END_TEST(BridgeUsingHinges)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CatenaryBridge = "Catenary bridges.";

START_TEST(CatenaryBridge, CATEGORY_JOINTS, gDesc_CatenaryBridge)

	virtual	float	GetRenderData(Point& center)const
	{
		return 800.0f;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.43f, 40.32f, 95.38f), Point(0.06f, -0.04f, -1.00f));
		desc.mCamera[1] = PintCameraPose(Point(0.77f, 56.76f, -20.68f), Point(0.45f, -0.36f, 0.82f));
//		desc.mCamera[0] = PintCameraPose(Point(-0.04f, 49.89f, 26.35f), Point(0.46f, -0.22f, -0.86f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		BRIDGES_CREATE Create;
		Create.mNbBridges		= 10;
		Create.mNbPlanks		= 20;
		Create.mPlankExtents	= Point(1.0f, 0.1f, 2.0f);
		Create.mOrigin			= Point(0.0f, 40.0f, 0.0f);
		if(!CreateBridges(pint, caps, Create))
			return false;

		// Create spheres
		const float Radius = 1.0f;
		PintShapeRenderer* SphereRenderer = CreateSphereRenderer(Radius);

		for(udword j=0;j<Create.mNbBridges;j++)
		{
			const Point Pos(0.0f, 40.0f, float(j)*Create.mPlankExtents.z*4.0f);

			if(1)
			{
				PINT_SPHERE_CREATE SphereDesc(Radius);
				SphereDesc.mRenderer	= SphereRenderer;

				PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
				ObjectDesc.mMass		= 10.0f;
				ObjectDesc.mPosition	= Pos;
				ObjectDesc.mPosition.y	+= Radius*3.0f;
	//			ObjectDesc.mPosition	+= Point(12.0f, 10.0f, 0.0f);
				CreatePintObject(pint, ObjectDesc);
			}
		}
		return true;
	}

END_TEST(CatenaryBridge)

///////////////////////////////////////////////////////////////////////////////

static const float gNetScale = 1.0f;

static const char* gDesc_SphericalJointNet = "Stress test for spherical joints. The net is made of 40*40 spheres, all connected by spherical joints.";

START_TEST(SphericalJointNet, CATEGORY_JOINTS, gDesc_SphericalJointNet)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		const float Size = 50.0f * gNetScale;
		desc.mGlobalBounds.SetMinMax(Point(-Size, -Size, -Size), Point(Size, Size, Size));
		desc.mCamera[0] = PintCameraPose(Point(Size, Size, Size), Point(-0.59f, -0.47f, -0.66f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphericalJoints || !caps.mSupportRigidBodySimulation)
			return false;

		{
			PINT_SPHERE_CREATE ShapeDesc(16.0f*gNetScale);
			ShapeDesc.mRenderer	= CreateSphereRenderer(ShapeDesc.mRadius);

			CreateStaticObject(pint, &ShapeDesc, Point(0.0f, 10.0f*gNetScale, 0.0f));
		}

		const udword NbX = 40;
		const udword NbY = 40;
		const float Scale = 40.0f*gNetScale;
		const float Altitude = 30.0f*gNetScale;

		PINT_SPHERE_CREATE ShapeDesc(1.0f*gNetScale);
//		PINT_SPHERE_CREATE ShapeDesc(0.5f*gNetScale);
		ShapeDesc.mRenderer	= CreateSphereRenderer(ShapeDesc.mRadius);

		PintActorHandle* Handles = (PintActorHandle*)StackAlloc(NbX*NbY*sizeof(PintActorHandle*));
		udword Index = 0;
		for(udword y=0;y<NbY;y++)
		{
			const float CoeffY = 2.0f * ((float(y)/float(NbY-1)) - 0.5f);
			for(udword x=0;x<NbX;x++)
			{
				const float CoeffX = 2.0f * ((float(x)/float(NbX-1)) - 0.5f);

				PINT_OBJECT_CREATE ObjectDesc(&ShapeDesc);
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mPosition.x	= CoeffX * Scale;
				ObjectDesc.mPosition.y	= Altitude;
				ObjectDesc.mPosition.z	= CoeffY * Scale;
				Handles[Index++] = CreatePintObject(pint, ObjectDesc);
			}
		}

		const float CoeffX0 = 2.0f * ((0.0f/float(NbX-1)) - 0.5f);
		const float CoeffX1 = 2.0f * ((1.0f/float(NbX-1)) - 0.5f);
		const float CenterX = (CoeffX1 - CoeffX0)*0.5f*Scale;

		for(udword y=0;y<NbY;y++)
		{
			const udword Base = y*NbX;
			for(udword x=0;x<NbX-1;x++)
			{
//				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[Base+x], Handles[Base+x+1], Point(CenterX, 0.0f, 0.0f), Point(-CenterX, 0.0f, 0.0f)));
				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[Base+x], Handles[Base+x+1], Point(CenterX*2.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f)));
			}
		}

		const float CoeffY0 = 2.0f * ((0.0f/float(NbY-1)) - 0.5f);
		const float CoeffY1 = 2.0f * ((1.0f/float(NbY-1)) - 0.5f);
		const float CenterY = (CoeffY1 - CoeffY0)*0.5f*Scale;

		for(udword x=0;x<NbX;x++)
		{
			const udword Base = x;
			for(udword y=0;y<NbY-1;y++)
			{
//				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[x+(y*NbX)], Handles[x+(y+1)*NbX], Point(0.0f, 0.0f, CenterY), Point(0.0f, 0.0f, -CenterY)));
				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[x+(y*NbX)], Handles[x+(y+1)*NbX], Point(0.0f, 0.0f, CenterY*2.0f), Point(0.0f, 0.0f, 0.0f)));
			}
		}
		return true;
	}

END_TEST(SphericalJointNet)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SphericalJointNet2 = "Stress test for spherical joints. The net is made of 40*40 spheres, all connected by spherical joints.";

START_TEST(SphericalJointNet2, CATEGORY_JOINTS, gDesc_SphericalJointNet2)

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		const float Size = 50.0f * gNetScale;
		desc.mGlobalBounds.SetMinMax(Point(-Size, -Size, -Size), Point(Size, Size, Size));
		desc.mCamera[0] = PintCameraPose(Point(60.07f*gNetScale, 35.87f*gNetScale, 60.52f*gNetScale), Point(-0.64f, -0.40f, -0.65f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportSphericalJoints || !caps.mSupportRigidBodySimulation)
			return false;

		{
			const float BoxSide = 20.0f*gNetScale;
			const float BoxDepth = 20.0f*gNetScale;

			const float SphereRadius = 3.0f*gNetScale;
			const float CapsuleRadius = 3.0f*gNetScale;
			const float HalfHeight = 3.0f*gNetScale;
			const udword NbLayers = 2;
			const udword NbX = 4;
			const udword NbY = 4;
			float yy = 10.0f*gNetScale;
			BasicRandom Rnd(42);

			PINT_SPHERE_CREATE SphereDesc(SphereRadius);
			SphereDesc.mRenderer = CreateSphereRenderer(SphereRadius);

			PINT_BOX_CREATE BoxDesc(CapsuleRadius, CapsuleRadius, CapsuleRadius);
			BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

			PINT_CAPSULE_CREATE CapsuleDesc(CapsuleRadius, HalfHeight);
			CapsuleDesc.mRenderer = CreateCapsuleRenderer(CapsuleRadius, HalfHeight*2.0f);

			for(udword k=0;k<NbLayers;k++)
			{
				for(udword y=0;y<NbY;y++)
				{
					const float CoeffY = 2.0f * ((float(y)/float(NbY-1)) - 0.5f);
					for(udword x=0;x<NbX;x++)
					{
						const float CoeffX = 2.0f * ((float(x)/float(NbX-1)) - 0.5f);

						const float RandomX = 1.0f * Rnd.RandomFloat() * gNetScale;
						const float RandomY = 1.0f * Rnd.RandomFloat() * gNetScale;

						const udword Index = Rnd.Randomize() % 3;

						PINT_OBJECT_CREATE ObjectDesc;
						if(Index==0)
							ObjectDesc.SetShape(&SphereDesc);
						else if(Index==1)
							ObjectDesc.SetShape(&BoxDesc);
						else if(Index==2)
							ObjectDesc.SetShape(&CapsuleDesc);
						ObjectDesc.mMass		= 1.0f;
						ObjectDesc.mPosition.x	= RandomX + CoeffX * (BoxDepth - SphereRadius - BoxSide*2.0f);
						ObjectDesc.mPosition.y	= yy;
						ObjectDesc.mPosition.z	= RandomY + CoeffY * (BoxDepth - SphereRadius - BoxSide*2.0f);

						UnitRandomQuat(ObjectDesc.mRotation, Rnd);

						CreatePintObject(pint, ObjectDesc);
					}
				}
				yy += HalfHeight*5.0f;
			}
		}


		const udword NbX = 40;
		const udword NbY = 40;
		const float Scale = 40.0f * gNetScale;
		const float Altitude = 0.0f * gNetScale;

		PINT_SPHERE_CREATE ShapeDesc(1.0f * gNetScale);
		ShapeDesc.mRenderer	= CreateSphereRenderer(ShapeDesc.mRadius);

		PintActorHandle* Handles = (PintActorHandle*)StackAlloc(NbX*NbY*sizeof(PintActorHandle*));
		udword Index = 0;
		for(udword y=0;y<NbY;y++)
		{
			const float CoeffY = 2.0f * ((float(y)/float(NbY-1)) - 0.5f);
			for(udword x=0;x<NbX;x++)
			{
				const float CoeffX = 2.0f * ((float(x)/float(NbX-1)) - 0.5f);

				PINT_OBJECT_CREATE ObjectDesc(&ShapeDesc);
				ObjectDesc.mMass			= 1.0f;
//				ObjectDesc.mMassForInertia	= 10.0f;
				if(x==0 && y==0)
					ObjectDesc.mMass		= 0.0f;
				if(x==0 && y==NbY-1)
					ObjectDesc.mMass		= 0.0f;
				if(x==NbX-1 && y==0)
					ObjectDesc.mMass		= 0.0f;
				if(x==NbX-1 && y==NbY-1)
					ObjectDesc.mMass		= 0.0f;
				ObjectDesc.mPosition.x		= CoeffX * Scale;
				ObjectDesc.mPosition.y		= Altitude;
				ObjectDesc.mPosition.z		= CoeffY * Scale;
				Handles[Index++] = CreatePintObject(pint, ObjectDesc);
			}
		}

		const float CoeffX0 = 2.0f * ((0.0f/float(NbX-1)) - 0.5f);
		const float CoeffX1 = 2.0f * ((1.0f/float(NbX-1)) - 0.5f);
		const float CenterX = (CoeffX1 - CoeffX0)*0.5f*Scale;

		for(udword y=0;y<NbY;y++)
		{
			const udword Base = y*NbX;
			for(udword x=0;x<NbX-1;x++)
			{
//				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[Base+x], Handles[Base+x+1], Point(CenterX, 0.0f, 0.0f), Point(-CenterX, 0.0f, 0.0f)));
				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[Base+x], Handles[Base+x+1], Point(CenterX*2.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f)));
			}
		}

		const float CoeffY0 = 2.0f * ((0.0f/float(NbY-1)) - 0.5f);
		const float CoeffY1 = 2.0f * ((1.0f/float(NbY-1)) - 0.5f);
		const float CenterY = (CoeffY1 - CoeffY0)*0.5f*Scale;

		for(udword x=0;x<NbX;x++)
		{
			const udword Base = x;
			for(udword y=0;y<NbY-1;y++)
			{
//				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[x+(y*NbX)], Handles[x+(y+1)*NbX], Point(0.0f, 0.0f, CenterY), Point(0.0f, 0.0f, -CenterY)));
				pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[x+(y*NbX)], Handles[x+(y+1)*NbX], Point(0.0f, 0.0f, CenterY*2.0f), Point(0.0f, 0.0f, 0.0f)));
			}
		}
		return true;
	}

END_TEST(SphericalJointNet2)

///////////////////////////////////////////////////////////////////////////////

class BinReader
{
public:
	BinReader(const char* data) : mData(data)	{}

	int readDword()
	{
		const int* tmp = (const int*)mData;
		int Value = *tmp;
		mData += sizeof(int);
		return Value;
	}

	short readWord()
	{
		const short* tmp = (const short*)mData;
		short Value = *tmp;
		mData += sizeof(short);
		return Value;
	}

	float readFloat()
	{
		const float* tmp = (const float*)mData;
		float Value = *tmp;
		mData += sizeof(float);
		return Value;
	}
	const char*	mData;
};

struct BoneData
{
	udword				mID;
	Point				mPivot;
	PintActorHandle	mBody;
	PR					mPose;
};

class TestRagdoll
{
public:
	enum { NUM_BONES = 19 };

	TestRagdoll();
	~TestRagdoll();

	bool				Init(Pint& pint, const Point& offset, PintCollisionGroup group, bool use_compound, udword constraint_multiplier);

	BoneData			mBones[NUM_BONES];
	PintJointHandle*	mJoints[NUM_BONES-1];

	const BoneData*		FindBoneByName(int name)	const;
};

static /*const*/ float gRagdollScale = 0.1f;

static const udword gRagdollData[]={
	0x00000013, 
	0x01c7d2f8, 0x41afcf8e, 0x40f61aee, 0xc3173a20, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41afcdbc, 0x41031a76, 0xc3171df7, 0x3c5c4318, 0x3f4df85f, 0xbf17fcea, 0x3f7aaa38, 0x3de0bf0b, 
	0x3e2efcb6, 0x3e4f816e, 0xbf1568a0, 0xbf494cd1, 0, 0x3f962963, 0x3f82a747, 0x3facfaae, 0x41200000, 0x01c211d0, 0x41b19fcc, 0x4115b5da, 0xc317093f, 0x3f000000, 0x3f000000, 0x41200000, 
	0x0000001e, 0x41b2e15d, 0x411ead21, 0xc31710d4, 0x3dc82982, 0x3f538bba, 0xbf0dface, 0x3f7e8ea1, 0xbd6deaec, 0x3db59f18, 0x3d283688, 0xbf0f6602, 0xbf53ce8b, 0, 0x3f85e78f, 0x3f3fbd05, 
	0x3f72ad19, 0x41200000, 0x00cb87d8, 0x41b2a164, 0x412a6224, 0xc316fb60, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41b46871, 0x4132a8ec, 0xc3171ea4, 0x3de9e76c, 0x3f41b1eb, 0xbf24d011, 
	0x3f7da826, 0xbe08e43e, 0x3c98e5c0, 0xbd93578c, 0xbf23d9a2, 0xbf43d4a7, 0, 0x3fff4dd5, 0x3fa9a6c6, 0x3fd64ffc, 0x41200000, 0x02e2f720, 0x41b44d4e, 0x41477671, 0xc3171d0b, 0x3f000000, 
	0x3f000000, 0x41200000, 0x0000001e, 0x41b60d03, 0x414fb8e9, 0xc317482b, 0x3ebb5b9a, 0x3f2c083e, 0xbf24d17f, 0x3f62aa31, 0xbeedcb3d, 0x3c975790, 0xbe92bdb3, 0xbf13a99b, 0xbf43d3c3, 0, 
	0x3f47be8b, 0x3f0e353f, 0x3f0259e7, 0x41200000, 0x01c0ab78, 0x41b71239, 0x4154dd09, 0xc3176272, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41b75e37, 0x415ff130, 0xc3174207, 0x3cb748e0, 
	0x3d863664, 0xbf7f6299, 0x3f7a33e7, 0x3e559708, 0x3d11e698, 0x3e57775c, 0xbf79ce60, 0xbd733a30, 0, 0x3fa45e84, 0x3fac9056, 0x3fa12de9, 0x41200000, 0x02da42c0, 0x41b08d5b, 0x4146748c, 
	0xc31790f4, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41acf2e7, 0x413f6bf5, 0xc317f575, 0xbf33bfaf, 0xbf32a077, 0xbe113d9a, 0xbea38ede, 0x3e05cffc, 0x3f7044b0, 0xbf22e7c5, 0x3f344d8b, 
	0xbea119dc, 0, 0x3f4ef6b5, 0x3f3d4831, 0x3f647526, 0x41200000, 0x02d7d660, 0x41aa6215, 0x4140d788, 0xc31843dd, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41ab042b, 0x413810f2, 
	0xc3198628, 0x3d835938, 0xbf297a1e, 0xbf3f2a06, 0xbec3b797, 0xbf351e95, 0x3f182af1, 0xbf6bfcb1, 0x3e7d431f, 0xbe98ccaf, 0, 0x3fe367b7, 0x3f1b7cb7, 0x3ee6f860, 0x41200000, 0x01bb7670, 
	0x41aba7c0, 0x4131ad4c, 0xc31a8cfe, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41aecc44, 0x4133f9f3, 0xc31b538e, 0x3f07a8b7, 0xbecdcdb6, 0xbf3f2a01, 0x3e856301, 0xbf42c337, 0x3f182aee, 
	0xbf4e9a2a, 0xbf02704b, 0xbe98cc99, 0, 0x3fa20510, 0x3f2bd0a1, 0x3f10131b, 0x41200000, 0x02e5a990, 0x41b3fe97, 0x4139e064, 0xc31c2367, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 
	0x41b89ee4, 0x413b9ea2, 0xc31c4fb8, 0x3f55e05b, 0x3ef57505, 0xbe898ec6, 0x3e4a0551, 0xbf37c9ba, 0xbf2ae648, 0xbf034f1b, 0x3f0135aa, 0xbf31c251, 0, 0x3f314730, 0x3edcf402, 0x3ef8df37, 
	0x41200000, 0x01c78ae0, 0x41b7377a, 0x414612a0, 0xc3169391, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41b9e44c, 0x413ed265, 0xc31609a9, 0x3f008136, 0xbf4f596a, 0xbe9b49fe, 0xbeb0b327, 
	0x3e09301d, 0xbf6dcf74, 0x3f4b04d8, 0x3f122b98, 0xbe595fc4, 0, 0x3f4ef6ae, 0x3f3d482e, 0x3f64752b, 0x41200000, 0x02e7b820, 0x41bba084, 0x41400226, 0xc315b49a, 0x3f000000, 0x3f000000, 
	0x41200000, 0x0000001e, 0x41c2385f, 0x4131763f, 0xc31653e7, 0x3f14f045, 0x3e37f8c2, 0xbf4b122e, 0xbf29d990, 0xbeeb2794, 0xbf1733d8, 0xbef0ddec, 0x3f5eb382, 0xbe178f33, 0, 0x3fe367b4, 
	0x3f1b7cb8, 0x3ee6f85b, 0x41200000, 0x01b03a10, 0x41c72ad7, 0x4125afe2, 0xc316df37, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41c65566, 0x412c54a1, 0xc317a4bc, 0xbe7b440c, 0x3f0ea8f2, 
	0xbf4b1224, 0x3f085505, 0xbf1b32fd, 0xbf1733d2, 0xbf4f5f41, 0xbf113f4b, 0xbe178f06, 0, 0x3fa204a7, 0x3f2bcfe7, 0x3f101362, 0x41200000, 0x02dc4670, 0x41c34e4b, 0x413672c3, 0xc3187723, 
	0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41bfa089, 0x413b660c, 0xc318b84e, 0xbf1de048, 0xbf3f81d9, 0xbe7aee4c, 0x3f011ba6, 0xbf1d4f24, 0x3f1b4e90, 0xbf1abb03, 0x3e804849, 0x3f419924, 
	0, 0x3f314730, 0x3edcf40b, 0x3ef8df46, 0x41200000, 0x01c91298, 0x41ac2296, 0x40fa5649, 0xc317d5e8, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41b022a8, 0x40db32fd, 0xc318fead, 
	0x3eb5712d, 0x3f011740, 0x3f4998b1, 0xbf1a161f, 0x3f447ab1, 0xbe61e495, 0xbf3733b2, 0xbecaa80b, 0x3f1353fd, 0, 0x4013a5a4, 0x3f909b1c, 0x3f654bde, 0x41200000, 0x01b72a48, 0x41b73cbe, 
	0x40aee909, 0xc31aa357, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41b3ca9f, 0x406a6229, 0xc31ae39c, 0xbe3edbd8, 0x3f16647e, 0x3f4998a7, 0xbf79ab09, 0xbc60bb00, 0xbe61e49a, 0xbdf3496c, 
	0xbf4f22e9, 0x3f135402, 0, 0x40243acf, 0x3f930583, 0x3f64436a, 0x41200000, 0x01add448, 0x41b17700, 0x3fd86010, 0xc31b1912, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41b2a2eb, 
	0x3f7a46de, 0xc31b9041, 0xbe14e0d0, 0x3f0325dd, 0x3f58ae48, 0xbf77220d, 0xbe858450, 0xbc02da80, 0x3e5dd530, 0xbf517935, 0x3f085002, 0, 0x3f797f50, 0x3fa81d25, 0x3efa2532, 0x41200000, 
	0x00c49c28, 0x41b37c86, 0x40f1df94, 0xc3169e57, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41bb0a32, 0x40c8d2bd, 0xc3168f52, 0x3f19a53a, 0x3f409f96, 0x3e8aed8f, 0xbf4cc351, 0x3f10fcb4, 
	0x3e4b712c, 0xbb8937e0, 0xbeac2c4c, 0x3f7116ce, 0, 0x4013a5a3, 0x3f909b1c, 0x3f654bd5, 0x41200000, 0x02dc3ad0, 0x41c64a04, 0x408da3d2, 0xc316a28a, 0x3f000000, 0x3f000000, 0x41200000, 
	0x0000001e, 0x41bc3f79, 0x404dfd9c, 0xc315f40f, 0xbf2bcdb6, 0x3f309e32, 0x3e8aed87, 0xbf298d7b, 0xbf38ef2e, 0x3e4b711c, 0x3eaa8a68, 0xbd3dfc4c, 0x3f7116d0, 0, 0x40243acf, 0x3f930585, 
	0x3f644368, 0x41200000, 0x00c06480, 0x41b1813a, 0x3fee5e20, 0xc315586f, 0x3f000000, 0x3f000000, 0x41200000, 0x0000001e, 0x41b5b2c6, 0x3f99035b, 0xc315303d, 0xbe4c3cf4, 0x3f74dbed, 0x3e5a161b, 
	0xbf711426, 0xbe7d1d44, 0x3e69aaba, 0x3e8ab3f9, 0xbe1ec503, 0x3f7335a2, 0, 0x3f797f52, 0x3fa81d27, 0x3efa2570, 0x41200000, 0x02e2f720, 0x01c0ab78, 0x41b71239, 0x4154dd09, 0xc3176272, 
	0xbf24d17f, 0x3c975790, 0xbf43d3c3, 0x00cb87d8, 0x02e2f720, 0x41b44d4e, 0x41477671, 0xc3171d0b, 0xbf24d011, 0x3c98e5c0, 0xbf43d4a7, 0x01c211d0, 0x00cb87d8, 0x41b2a164, 0x412a6224, 0xc316fb60, 
	0xbf0dface, 0x3db59f18, 0xbf53ce8b, 0x01c7d2f8, 0x01c211d0, 0x41b19fcc, 0x4115b5da, 0xc317093f, 0xbf17fcea, 0x3e2efcb6, 0xbf494cd1, 0x01c91298, 0x01c7d2f8, 0x41ac2296, 0x40fa5649, 0xc317d5e8, 
	0x3f4998b1, 0xbe61e495, 0x3f1353fd, 0x01b72a48, 0x01c91298, 0x41b73cbe, 0x40aee909, 0xc31aa357, 0x3f4998a7, 0xbe61e49a, 0x3f135402, 0x01add448, 0x01b72a48, 0x41b17700, 0x3fd86010, 0xc31b1912, 
	0x3f58ae48, 0xbc02da80, 0x3f085002, 0x00c49c28, 0x01c7d2f8, 0x41b37c86, 0x40f1df94, 0xc3169e57, 0x3e8aed8f, 0x3e4b712c, 0x3f7116ce, 0x02dc3ad0, 0x00c49c28, 0x41c64a04, 0x408da3d2, 0xc316a28a, 
	0x3e8aed87, 0x3e4b711c, 0x3f7116d0, 0x00c06480, 0x02dc3ad0, 0x41b1813a, 0x3fee5e20, 0xc315586f, 0x3e5a161b, 0x3e69aaba, 0x3f7335a2, 0x02da42c0, 0x00cb87d8, 0x41b08d5b, 0x4146748c, 0xc31790f4, 
	0xbe113d9a, 0x3f7044b0, 0xbea119dc, 0x02d7d660, 0x02da42c0, 0x41aa6215, 0x4140d788, 0xc31843dd, 0xbf3f2a06, 0x3f182af1, 0xbe98ccaf, 0x01bb7670, 0x02d7d660, 0x41aba7c0, 0x4131ad4c, 0xc31a8cfe, 
	0xbf3f2a01, 0x3f182aee, 0xbe98cc99, 0x02e5a990, 0x01bb7670, 0x41b3fe97, 0x4139e064, 0xc31c2367, 0xbe898ec6, 0xbf2ae648, 0xbf31c251, 0x01c78ae0, 0x00cb87d8, 0x41b7377a, 0x414612a0, 0xc3169391, 
	0xbe9b49fe, 0xbf6dcf74, 0xbe595fc4, 0x02e7b820, 0x01c78ae0, 0x41bba084, 0x41400226, 0xc315b49a, 0xbf4b122e, 0xbf1733d8, 0xbe178f33, 0x01b03a10, 0x02e7b820, 0x41c72ad7, 0x4125afe2, 0xc316df37, 
	0xbf4b1224, 0xbf1733d2, 0xbe178f06, 0x02dc4670, 0x01b03a10, 0x41c34e4b, 0x413672c3, 0xc3187723, 0xbe7aee4c, 0x3f1b4e90, 0x3f419924, 0xdeadbabe, 0x01c0ab78, 0x02e2f720, 0x01c0ab78, 0x00cb87d8, 
	0x01c0ab78, 0x01c211d0, 0x01c0ab78, 0x01c7d2f8, 0x01c0ab78, 0x01c78ae0, 0x01c0ab78, 0x02e7b820, 0x01c0ab78, 0x01b03a10, 0x01c0ab78, 0x02dc4670, 0x01c0ab78, 0x00c49c28, 0x01c0ab78, 0x02dc3ad0, 
	0x01c0ab78, 0x00c06480, 0x01c0ab78, 0x02da42c0, 0x01c0ab78, 0x02d7d660, 0x01c0ab78, 0x01bb7670, 0x01c0ab78, 0x02e5a990, 0x01c0ab78, 0x01c91298, 0x01c0ab78, 0x01b72a48, 0x01c0ab78, 0x01add448, 
	0x02e2f720, 0x00cb87d8, 0x02e2f720, 0x01c211d0, 0x02e2f720, 0x01c7d2f8, 0x02e2f720, 0x01c78ae0, 0x02e2f720, 0x02e7b820, 0x02e2f720, 0x01b03a10, 0x02e2f720, 0x02dc4670, 0x02e2f720, 0x00c49c28, 
	0x02e2f720, 0x02dc3ad0, 0x02e2f720, 0x00c06480, 0x02e2f720, 0x02da42c0, 0x02e2f720, 0x02d7d660, 0x02e2f720, 0x01bb7670, 0x02e2f720, 0x02e5a990, 0x02e2f720, 0x01c91298, 0x02e2f720, 0x01b72a48, 
	0x02e2f720, 0x01add448, 0x00cb87d8, 0x01c211d0, 0x00cb87d8, 0x01c7d2f8, 0x00cb87d8, 0x01c78ae0, 0x00cb87d8, 0x02e7b820, 0x00cb87d8, 0x01b03a10, 0x00cb87d8, 0x02dc4670, 0x00cb87d8, 0x00c49c28, 
	0x00cb87d8, 0x02dc3ad0, 0x00cb87d8, 0x00c06480, 0x00cb87d8, 0x02da42c0, 0x00cb87d8, 0x02d7d660, 0x00cb87d8, 0x01bb7670, 0x00cb87d8, 0x02e5a990, 0x00cb87d8, 0x01c91298, 0x00cb87d8, 0x01b72a48, 
	0x00cb87d8, 0x01add448, 0x01c211d0, 0x01c7d2f8, 0x01c211d0, 0x01c78ae0, 0x01c211d0, 0x02e7b820, 0x01c211d0, 0x01b03a10, 0x01c211d0, 0x02dc4670, 0x01c211d0, 0x00c49c28, 0x01c211d0, 0x02dc3ad0, 
	0x01c211d0, 0x00c06480, 0x01c211d0, 0x02da42c0, 0x01c211d0, 0x02d7d660, 0x01c211d0, 0x01bb7670, 0x01c211d0, 0x02e5a990, 0x01c211d0, 0x01c91298, 0x01c211d0, 0x01b72a48, 0x01c211d0, 0x01add448, 
	0x01c7d2f8, 0x01c78ae0, 0x01c7d2f8, 0x02e7b820, 0x01c7d2f8, 0x01b03a10, 0x01c7d2f8, 0x02dc4670, 0x01c7d2f8, 0x00c49c28, 0x01c7d2f8, 0x02dc3ad0, 0x01c7d2f8, 0x00c06480, 0x01c7d2f8, 0x02da42c0, 
	0x01c7d2f8, 0x02d7d660, 0x01c7d2f8, 0x01bb7670, 0x01c7d2f8, 0x02e5a990, 0x01c7d2f8, 0x01c91298, 0x01c7d2f8, 0x01b72a48, 0x01c7d2f8, 0x01add448, 0x01c78ae0, 0x02e7b820, 0x01c78ae0, 0x01b03a10, 
	0x01c78ae0, 0x02dc4670, 0x01c78ae0, 0x00c49c28, 0x01c78ae0, 0x02dc3ad0, 0x01c78ae0, 0x00c06480, 0x01c78ae0, 0x02da42c0, 0x01c78ae0, 0x02d7d660, 0x01c78ae0, 0x01bb7670, 0x01c78ae0, 0x02e5a990, 
	0x01c78ae0, 0x01c91298, 0x01c78ae0, 0x01b72a48, 0x01c78ae0, 0x01add448, 0x02e7b820, 0x01b03a10, 0x02e7b820, 0x02dc4670, 0x02e7b820, 0x00c49c28, 0x02e7b820, 0x02dc3ad0, 0x02e7b820, 0x00c06480, 
	0x02e7b820, 0x02da42c0, 0x02e7b820, 0x02d7d660, 0x02e7b820, 0x01bb7670, 0x02e7b820, 0x02e5a990, 0x02e7b820, 0x01c91298, 0x02e7b820, 0x01b72a48, 0x02e7b820, 0x01add448, 0x01b03a10, 0x02dc4670, 
	0x01b03a10, 0x00c49c28, 0x01b03a10, 0x02dc3ad0, 0x01b03a10, 0x00c06480, 0x01b03a10, 0x02da42c0, 0x01b03a10, 0x02d7d660, 0x01b03a10, 0x01bb7670, 0x01b03a10, 0x02e5a990, 0x01b03a10, 0x01c91298, 
	0x01b03a10, 0x01b72a48, 0x01b03a10, 0x01add448, 0x02dc4670, 0x00c49c28, 0x02dc4670, 0x02dc3ad0, 0x02dc4670, 0x00c06480, 0x02dc4670, 0x02da42c0, 0x02dc4670, 0x02d7d660, 0x02dc4670, 0x01bb7670, 
	0x02dc4670, 0x02e5a990, 0x02dc4670, 0x01c91298, 0x02dc4670, 0x01b72a48, 0x02dc4670, 0x01add448, 0x00c49c28, 0x02dc3ad0, 0x00c49c28, 0x00c06480, 0x00c49c28, 0x02da42c0, 0x00c49c28, 0x02d7d660, 
	0x00c49c28, 0x01bb7670, 0x00c49c28, 0x02e5a990, 0x00c49c28, 0x01c91298, 0x00c49c28, 0x01b72a48, 0x00c49c28, 0x01add448, 0x02dc3ad0, 0x00c06480, 0x02dc3ad0, 0x02da42c0, 0x02dc3ad0, 0x02d7d660, 
	0x02dc3ad0, 0x01bb7670, 0x02dc3ad0, 0x02e5a990, 0x02dc3ad0, 0x01c91298, 0x02dc3ad0, 0x01b72a48, 0x02dc3ad0, 0x01add448, 0x00c06480, 0x02da42c0, 0x00c06480, 0x02d7d660, 0x00c06480, 0x01bb7670, 
	0x00c06480, 0x02e5a990, 0x00c06480, 0x01c91298, 0x00c06480, 0x01b72a48, 0x00c06480, 0x01add448, 0x02da42c0, 0x02d7d660, 0x02da42c0, 0x01bb7670, 0x02da42c0, 0x02e5a990, 0x02da42c0, 0x01c91298, 
	0x02da42c0, 0x01b72a48, 0x02da42c0, 0x01add448, 0x02d7d660, 0x01bb7670, 0x02d7d660, 0x02e5a990, 0x02d7d660, 0x01c91298, 0x02d7d660, 0x01b72a48, 0x02d7d660, 0x01add448, 0x01bb7670, 0x02e5a990, 
	0x01bb7670, 0x01c91298, 0x01bb7670, 0x01b72a48, 0x01bb7670, 0x01add448, 0x02e5a990, 0x01c91298, 0x02e5a990, 0x01b72a48, 0x02e5a990, 0x01add448, 0x01c91298, 0x01b72a48, 0x01c91298, 0x01add448, 
	0x01b72a48, 0x01add448
};

static udword LocalVectors_Data[]={
0x3d13b29c, 
0xb9e269c0, 0x39752680, 0xbd90392d, 0xbb18aa20, 0x3b023760, 0xb4a80000, 0xb5300000, 0x3f7fffff, 0xbe2115c1, 0x3f354685, 0x3f303843, 0x3e03b7f2, 0xbc99df24, 0x3b37ad60, 0xbd6fc276, 0xb9e22a00, 
0x39760a80, 0xb42c0000, 0xb5500000, 0x3f7ffffe, 0x393c6800, 0xb88b3000, 0x3f800000, 0x3d9515f5, 0xbc3cd7a0, 0x3aa8b690, 0xbd606487, 0xbc994d22, 0x3b379740, 0x33500000, 0xb5080000, 0x3f800000, 
0x3dac2459, 0x3dc8d6f8, 0x3f7ddb0e, 0x3ded30f6, 0x3cd7d820, 0xb5ca8000, 0xbd6a05b2, 0xbc3a6fe4, 0x3aa8cf80, 0x34100000, 0xb4c00000, 0x3f7fffff, 0x3da307f7, 0xbd760d18, 0x3f7eb94b, 0xbe23122b, 
0x3b6ac3ec, 0x3bc07081, 0xbd52ee09, 0x3a7e28c0, 0x3d9e7cea, 0x35100000, 0xb4b80000, 0x3f7ffffc, 0xbdb5d9e2, 0x3e8c088e, 0xbf753001, 0xbe3f67ef, 0x3b24fd48, 0x3c0c891c, 0x3e6def63, 0x3b6ac1f8, 
0x3bc06aee, 0x36270000, 0x34000000, 0x3f7ffffd, 0x34000000, 0xb5600000, 0x3f7ffffd, 0xbd67d0cb, 0xbd837370, 0x3c41c128, 0x3e4d02c2, 0x3b24e630, 0x3c0c8410, 0x36560000, 0x35d40000, 0x3f7ffffe, 
0xbe5a56ea, 0x3d883a24, 0x3f798856, 0xbe231251, 0x3b6ad032, 0xbbb96bfc, 0xbd52edd6, 0x3a7e1a00, 0xbd9e7ede, 0x34728000, 0x33000000, 0x3f7ffffc, 0x3ec72db4, 0xbe9e78f6, 0xbf5e2008, 0xbe3f6816, 
0x3b24d7cb, 0xbbc910b0, 0x3e6def24, 0x3b6acda8, 0xbbb96fcc, 0x35180000, 0xb40c0000, 0x3f7ffffd, 0x34910000, 0xb4a00000, 0x3f7ffffc, 0xbd67d067, 0xbd83743e, 0xbc320030, 0x3e4d02b4, 0x3b24dc7e, 
0xbbc90e40, 0x34f00000, 0x355c0000, 0x3f7fffff, 0x3cb75a20, 0xbd7ddb24, 0x3f7f718f, 0xbd90b19f, 0x3b00de90, 0x3cb87512, 0x3df65e49, 0xbcc81a6c, 0x3d8a3c19, 0xb5a20000, 0x35a80000, 0x3f7ffffe, 
0x3f6fb7d9, 0xbd009a5c, 0x3eb2f584, 0xbe0cd413, 0xbb1f25e0, 0x3a767780, 0x3d2073c5, 0x3b00e936, 0x3cb874fd, 0xb5e40000, 0x34d00000, 0x3f7fffff, 0x3f063b90, 0x3ec6ecec, 0x3f41f893, 0xbdb27fb1, 
0xbc51a90f, 0xbb1a6ba0, 0x3de224b3, 0xbb1f2480, 0x3a767480, 0xb5f80000, 0xb5980000, 0x3f7ffffc, 0xb6160000, 0x34f80000, 0x3f7ffffe, 0xbd730f3d, 0xbc36e99f, 0x3c30c3e1, 0x3de05d99, 0xbc51a821, 
0xbb1a6460, 0xb59c0000, 0x35b40000, 0x3f800001, 0x3e79f4da, 0x3f783d72, 0x3c34def0, 0xbd99e60f, 0x3b1d1530, 0xbca81be7, 0x3df66b16, 0xbcc81a50, 0xbd7d86e8, 0xb5d20000, 0xb65a0000, 0x3f7fffff, 
0xbf7097b6, 0x3cfa6f6c, 0x3eae3d85, 0xbe0cd812, 0xbb1f3f00, 0x3b21b1c0, 0x3d0e0ad5, 0x3b1d11dc, 0xbca81b81, 0xb56e0000, 0x35f80000, 0x3f800000, 0xbe9f9ad3, 0x3ef52cda, 0x3f521848, 0xbdb2864b, 
0xbc51a470, 0x3b9e83c0, 0x3de21c74, 0xbb1f3ab8, 0x3b21b240, 0xb6100000, 0xb5da0000, 0x3f7ffffb, 0xb5930000, 0x36200000, 0x3f7fffff, 0xbd7317ed, 0xbc139c52, 0xbc30bd61, 0x3de05658, 0xbc51a540, 
0x3b9e8330, 0xb60c0000, 0x35540000, 0x3f800000, 0xbe6ae3ae, 0xbf6ef4e1, 0xbe8d3e51
};

TestRagdoll::TestRagdoll()
{
	for(udword i=0;i<18;i++)
		mJoints[i] = null;
}

TestRagdoll::~TestRagdoll()
{
}

const BoneData* TestRagdoll::FindBoneByName(int name) const
{
	for(udword i=0;i<19;i++)
		if(mBones[i].mID==name)
			return &mBones[i];
	return NULL;
}

static PintActorHandle createBodyPart(Pint& pint, const Point& extents, const PR& pose, float mass, PintCollisionGroup group, bool use_compound)
{
	PINT_BOX_CREATE BoxDesc(extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(extents);

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mPosition		= pose.mPos;
	ObjectDesc.mRotation		= pose.mRot;
	ObjectDesc.mMass			= mass;
	ObjectDesc.mCollisionGroup	= group;
	if(use_compound)
		ObjectDesc.mAddToWorld	= false;
	PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
	ASSERT(Handle);
	return Handle;
}

static void ComputeLocalAnchor(Point& local, const Point& global, const PR* pose)
{
	if(pose)
	{
		Matrix4x4 M;
		InvertPRMatrix(M, Matrix4x4(*pose));

		local = global * M;
	}
	else local = global;
}

static void ComputeLocalAxis(Point& local, const Point& global, const PR* pose)
{
	if(pose)
	{
		Matrix4x4 M;
		InvertPRMatrix(M, Matrix4x4(*pose));

		local = global * Matrix3x3(M);
	}
	else local = global;
}

bool TestRagdoll::Init(Pint& pint, const Point& offset, PintCollisionGroup group, bool use_compound, udword constraint_multiplier)
{
	BinReader Data((const char*)gRagdollData);
	const int NbBones = Data.readDword();
	ASSERT(NbBones==19);

	PintAggregateHandle Aggregate = null;
	if(use_compound)
		Aggregate = pint.CreateAggregate(NbBones, false);

	for(int i=0;i<NbBones;i++)
	{
		mBones[i].mID = Data.readDword();
		mBones[i].mPivot.x = Data.readFloat();
		mBones[i].mPivot.y = Data.readFloat();
		mBones[i].mPivot.z = Data.readFloat();
		mBones[i].mPivot *= gRagdollScale;

		const float linearDamping = Data.readFloat();
		const float angularDamping = Data.readFloat();
		const float maxAngularVelocity = Data.readFloat();
		const int solverIterationCount = Data.readDword();

		PR massLocalPose;
		massLocalPose.mPos.x = Data.readFloat();
		massLocalPose.mPos.y = Data.readFloat();
		massLocalPose.mPos.z = Data.readFloat();
		massLocalPose.mPos *= gRagdollScale;
		massLocalPose.mPos += offset;

		Matrix3x3 m;
		for(int x=0;x<3;x++)
		{
			for(int y=0;y<3;y++)
			{
				m.m[y][x] = Data.readFloat();
			}
		}
		massLocalPose.mRot = m;
		massLocalPose.mRot.Normalize();
/*		if(massLocalPose.mRot.w<0.0f)
		{
//			printf("Negating quat...\n");
			massLocalPose.mRot = -massLocalPose.mRot;
		}*/

		int collisionGroup = Data.readDword();
		Point dimensions;
		dimensions.x = Data.readFloat();
		dimensions.y = Data.readFloat();
		dimensions.z = Data.readFloat();
dimensions *= gRagdollScale;

		float density = Data.readFloat();
density *= gRagdollScale;
		mBones[i].mBody = createBodyPart(pint, dimensions, massLocalPose, density, group, use_compound);
		mBones[i].mPose = massLocalPose;

		if(Aggregate)
			pint.AddToAggregate(mBones[i].mBody, Aggregate);
	}

	const char* SavedPtr = Data.mData;
	for(udword j=0;j<constraint_multiplier;j++)
	{
		Data.mData = SavedPtr;

		const Point* LocalVectors = reinterpret_cast<const Point*>(LocalVectors_Data);
		for(udword i=0;i<18;i++)
		{
			int bodyID0 = Data.readDword();
			int bodyID1 = Data.readDword();
			const BoneData* b0 = FindBoneByName(bodyID0);
			const BoneData* b1 = FindBoneByName(bodyID1);
			ASSERT(b0);
			ASSERT(b1);

			Point globalAnchor;
			globalAnchor.x = Data.readFloat();
			globalAnchor.y = Data.readFloat();
			globalAnchor.z = Data.readFloat();
	globalAnchor *= gRagdollScale;
			globalAnchor += offset;

			Point globalAxis;
			globalAxis.x = Data.readFloat();
			globalAxis.y = Data.readFloat();
			globalAxis.z = Data.readFloat();

	/*		Matrix4x4 Mat0 = b0->mPose;
			Mat0.Invert();
			Matrix4x4 Mat1 = b1->mPose;
			Mat1.Invert();*/

			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= b0->mBody;
			Desc.mObject1		= b1->mBody;
	/*		Desc.mLocalPivot0	= globalAnchor * Mat0;
			Desc.mLocalPivot1	= globalAnchor * Mat1;
			Desc.mLocalAxis0	= globalAxis * Matrix3x3(Mat0);
			Desc.mLocalAxis1	= globalAxis * Matrix3x3(Mat1);*/

			if(1)
			{
	// ### test
	ComputeLocalAnchor(Desc.mLocalPivot0, globalAnchor, &b0->mPose);
	ComputeLocalAnchor(Desc.mLocalPivot1, globalAnchor, &b1->mPose);
	ComputeLocalAxis(Desc.mLocalAxis0, globalAxis, &b0->mPose);
	ComputeLocalAxis(Desc.mLocalAxis1, globalAxis, &b1->mPose);

			Desc.mGlobalAnchor	= globalAnchor;
			Desc.mGlobalAxis	= globalAxis;
			}
			else
			{
				// ### this path doesn't work yet with ragdoll scale != 0.1
				Desc.mLocalPivot0 = LocalVectors[i*4+0];
				Desc.mLocalPivot1 = LocalVectors[i*4+1];
				Desc.mLocalAxis0 = LocalVectors[i*4+2];
				Desc.mLocalAxis1 = LocalVectors[i*4+3];
			}
			Desc.mLimits.Set(-0.2f, 0.2f);
	//		Desc.mMinLimitAngle	= -0.02f;
	//		Desc.mMaxLimitAngle	= 0.02f;
	//		Desc.mMinLimitAngle	= degToRad(-45.0f);
	//		Desc.mMaxLimitAngle	= degToRad(45.0f);
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
	}

	if(Aggregate)
		pint.AddAggregateToScene(Aggregate);

	return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool GenerateArrayOfRagdolls(Pint& pint, const PintCaps& caps, udword NbX, udword NbY, float Scale, Point* offset, bool use_aggregates, udword constraint_multiplier)
{
	if(!caps.mSupportHingeJoints || !caps.mSupportCollisionGroups || !caps.mSupportRigidBodySimulation)
		return false;

	if(use_aggregates && !caps.mSupportAggregates)
		return false;

	const PintDisabledGroups DG(1, 1);
	pint.SetDisabledGroups(1, &DG);

	for(udword i=0;i<NbX;i++)
	{
		const float CoeffX = NbX>1 ? (float(i)/float(NbX-1))-0.5f : 0.0f;
		for(udword j=0;j<NbY;j++)
		{
			const float CoeffY = NbY>1 ? (float(j)/float(NbY-1))-0.5f : 0.0f;
			TestRagdoll RD;

			Point Offset(CoeffX*Scale, 1.0f, CoeffY*Scale);
			if(offset)
				Offset += *offset;

			RD.Init(pint, Offset, 1, use_aggregates, constraint_multiplier);
		}
	}
	return true;
}

static bool GenerateColumnOfRagdolls(Pint& pint, const PintCaps& caps, udword nb, udword constraint_multiplier)
{
	if(!caps.mSupportHingeJoints || !caps.mSupportCollisionGroups || !caps.mSupportRigidBodySimulation)
		return false;

	// Disable each ragdoll's inner collisions, but let ragdolls collide with each other.
	PintDisabledGroups* DisabledGroups = (PintDisabledGroups*)StackAlloc(nb*sizeof(PintDisabledGroups));
	for(udword i=0;i<nb;i++)
	{
		const PintCollisionGroup DisabledGroup = PintCollisionGroup(i+1);
		DisabledGroups[i] = PintDisabledGroups(DisabledGroup, DisabledGroup);
	}
	pint.SetDisabledGroups(nb, DisabledGroups);

	const float Inc = 2.0;
	Point Offset(0.0f, Inc, 0.0f);
	for(udword i=0;i<nb;i++)
	{
		TestRagdoll RD;
		RD.Init(pint, Offset, i+1, false, constraint_multiplier);
		Offset.y += Inc;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_RagdollConfigurable = "Each ragdoll is made of 19 bones connected by 18 hinge joints. Ragdoll-ragdoll collisions are disabled.";

#define NB_PRESETS	7
static const udword	gPreset_GridSize[NB_PRESETS]   = { 1,     1,    10,    10,   16,    16,   1 };
static const udword	gPreset_Multiplier[NB_PRESETS] = { 1,     1,    1,     1,    1,     1,    4 };
static const bool	gPreset_Aggregates[NB_PRESETS] = { false, true, false, true, false, true, false };

class RagdollConfigurable : public TestBase
{
			ComboBoxPtr		mComboBox_Preset;
			EditBoxPtr		mEditBox_Size;
			EditBoxPtr		mEditBox_Multiplier;
			CheckBoxPtr		mCheckBox_Aggregates;
	public:
							RagdollConfigurable()		{									}
	virtual					~RagdollConfigurable()		{									}
	virtual	const char*		GetName()			const	{ return "Ragdolls";				}
	virtual	const char*		GetDescription()	const	{ return gDesc_RagdollConfigurable;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;			}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 280;
		WD.mHeight	= 160;
		WD.mLabel	= "Ragdolls";
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
		sdword y = 0;
		{
			mCheckBox_Aggregates = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use aggregates", &UIElems, false, null, null);
			mCheckBox_Aggregates->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Ragdoll array size:", &UIElems);
			mEditBox_Size = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_Size->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Constraint multiplier:", &UIElems);
			mEditBox_Multiplier = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_Multiplier->SetEnabled(false);
			y += YStep;
		}
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Presets:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				RagdollConfigurable&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, RagdollConfigurable& test) : IceComboBox(desc), mTest(test)	{}
				virtual			~MyComboBox()																						{}

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

			y += YStep;
			AddResetButton(UI, 4, y, 264);
		}

		return null;
	}

	virtual	const char*		GetSubName()	const
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
	}

	virtual	void			SetDefaultSubscene(const char* sub_scene)
	{
		if(!sub_scene || !mComboBox_Preset)
			return;
		udword SelectedIndex = INVALID_ID;
		if(strcmp(sub_scene, "Ragdoll")==0)
			SelectedIndex = 0;
		else if(strcmp(sub_scene, "Ragdoll_Aggregate")==0)
			SelectedIndex = 1;
		else if(strcmp(sub_scene, "Ragdolls_100")==0)
			SelectedIndex = 2;
		else if(strcmp(sub_scene, "Ragdolls_100_Aggregate")==0)
			SelectedIndex = 3;
		else if(strcmp(sub_scene, "Ragdolls_256")==0)
			SelectedIndex = 4;
		else if(strcmp(sub_scene, "Ragdolls_256_Aggregate")==0)
			SelectedIndex = 5;
		else if(strcmp(sub_scene, "Ragdoll_MultipleConstraints")==0)
			SelectedIndex = 6;
		if(SelectedIndex!=INVALID_ID)
		{
			mComboBox_Preset->Select(SelectedIndex);
			mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
		}
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(6.58f, 1.32f, -14.27f), Point(-0.98f, -0.02f, -0.21f));
		desc.mCamera[1] = PintCameraPose(Point(15.68f, 2.85f, -4.21f), Point(-0.73f, -0.30f, -0.62f));
		desc.mCamera[2] = PintCameraPose(Point(17.08f, 2.98f, -1.78f), Point(-0.75f, -0.15f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual	bool			Setup(Pint& pint, const PintCaps& caps)
	{
		const udword Size = GetInt(1, mEditBox_Size);
		const udword Multiplier = GetInt(1, mEditBox_Multiplier);
		const bool UseAggregates = mCheckBox_Aggregates ? mCheckBox_Aggregates->IsChecked() : false;
		return GenerateArrayOfRagdolls(pint, caps, Size, Size, 2.0f * float(Size), null, UseAggregates, Multiplier);
	}

}RagdollConfigurable;

///////////////////////////////////////////////////////////////////////////////

// TODO: fix this. It doesn't work when multiple engines are selected
#ifdef REMOVED
static const char* gDesc_ExplodingRagdoll = "Exploding ragdoll.";

START_TEST(ExplodingRagdoll, CATEGORY_WIP, gDesc_ExplodingRagdoll)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(6.58f, 1.32f, -14.27f), Point(-0.98f, -0.02f, -0.21f));
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportHingeJoints || !caps.mSupportCollisionGroups || !caps.mSupportRigidBodySimulation)
			return false;

		const PintDisabledGroups DG(1, 1);
		pint.SetDisabledGroups(1, &DG);

		TestRagdoll RD;
		const Point Offset(0.0f, 0.0f, 0.0f);
		RD.Init(pint, Offset, 1, false, 1);

//		pint.mUserData = RD.mBones[0].mBody;
		pint.mUserData = RD.mBones[1].mBody;

		return true;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		PintActorHandle h = (PintActorHandle)pint.mUserData;
		if(h)
		{
			if(mCurrentTime>4.0f)
			{
				mCurrentTime = -FLT_MAX;

				const PR Pose = pint.GetWorldTransform(h);

				pint.AddWorldImpulseAtWorldPos(h, Point(0.0f, 500.0f, 0.0f), Pose.mPos);
			}
		}
		return TestBase::Update(pint, dt);
	}

END_TEST(ExplodingRagdoll)
#endif

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Ragdoll256_OnTerrain = "256 ragdolls on a terrain. Ragdoll-ragdoll collisions are disabled.";

START_TEST(Ragdolls_256_OnTerrain, CATEGORY_JOINTS, gDesc_Ragdoll256_OnTerrain)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
//		desc.mCamera[0] = PintCameraPose(Point(17.08f, 2.98f, -1.78f), Point(-0.75f, -0.15f, -0.65f));
		desc.mCamera[0] = PintCameraPose(Point(2271.26f, 326.41f, 1416.32f), Point(-0.63f, -0.44f, 0.64f));
		desc.mCamera[1] = PintCameraPose(Point(2055.45f, 20.57f, 1983.79f), Point(-0.96f, -0.23f, 0.13f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		LoadMeshesFromFile_(*this, "terrain.bin");

		if(0)
		{
			const float Scale = 2000.0f;
			const udword NbX = 16;
			const udword NbY = 16;
			Point Center, Extents;
			GetGlobalBounds(Center, Extents);
			Center.y += Extents.y;

			for(udword i=0;i<NbX;i++)
			{
				const float CoeffX = NbX>1 ? (float(i)/float(NbX-1))-0.5f : 0.0f;
				for(udword j=0;j<NbY;j++)
				{
					const float CoeffY = NbY>1 ? (float(j)/float(NbY-1))-0.5f : 0.0f;

					Point Offset(CoeffX*Scale, 1.0f, CoeffY*Scale);
					Offset += Center;

					RegisterRaycast(Offset, Point(0.0f, -1.0f, 0.0f), 5000.0f);
				}
			}
		}
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
//		if(!caps.mSupportRaycasts)
//			return false;
		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		Point Center, Extents;
		GetGlobalBounds(Center, Extents);
		Center.y += Extents.y*0.2f;

		const float Saved = gRagdollScale;
		gRagdollScale = 1.0f;
//		bool Status = GenerateArrayOfRagdolls(pint, caps, 16, 16, 2000.0f, &Center);
//		static bool GenerateArrayOfRagdolls(Pint& pint, const PintCaps& caps, udword NbX, udword NbY, float Scale, Point* offset=null)
		{
			udword NbX = 16;
			udword NbY = 16;
			float Scale = 2000.0f;
			Point* offset = &Center;
			if(!caps.mSupportHingeJoints || !caps.mSupportCollisionGroups || !caps.mSupportRigidBodySimulation)
				return false;

			const PintDisabledGroups DG(1, 1);
			pint.SetDisabledGroups(1, &DG);

			SRand(42);

			for(udword i=0;i<NbX;i++)
			{
				const float CoeffX = NbX>1 ? (float(i)/float(NbX-1))-0.5f : 0.0f;
				for(udword j=0;j<NbY;j++)
				{
					const float CoeffY = NbY>1 ? (float(j)/float(NbY-1))-0.5f : 0.0f;
					TestRagdoll RD;

					Point Offset(CoeffX*Scale, 1.0f, CoeffY*Scale);
					if(offset)
						Offset += *offset;

					Offset.y += UnitRandomFloat()*100.0f;

					RD.Init(pint, Offset, 1, false, 1);
				}
			}
		}
		gRagdollScale = Saved;
		return true;
//		return Status;
	}

/*	virtual udword	Update(Pint& pint, float dt)
	{
		return DoBatchRaycasts(*this, pint, false);
	}*/

END_TEST(Ragdolls_256_OnTerrain)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_PileOfRagdolls16 = "A pile of 16 ragdolls. Ragdoll-ragdoll collisions are enabled.";

START_TEST(PileOfRagdolls_16, CATEGORY_JOINTS, gDesc_PileOfRagdolls16)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(6.88f, 2.47f, -11.72f), Point(-0.78f, -0.26f, -0.56f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return GenerateColumnOfRagdolls(pint, caps, 16, 1);
	}

END_TEST(PileOfRagdolls_16)

///////////////////////////////////////////////////////////////////////////////

	enum ChainDataUIIndex
	{
		CHAIN_DATA_NB_SPHERES,
		CHAIN_DATA_RADIUS,
		CHAIN_DATA_GAP,
		CHAIN_DATA_MASS,
		CHAIN_DATA_CENTERED_PIVOTS,
		CHAIN_DATA_USE_DISTANCE_CONSTAINTS,
		CHAIN_DATA_USE_LARGER_INERTIA,
		CHAIN_DATA_ATTACH_HEAVY_BOX,
		CHAIN_DATA_USE_ARTICULATION,
		CHAIN_DATA_FIRST_SPHERE_IS_STATIC,
	};

	struct ChainData
	{
		ChainData() :
			mCenteredPivots				(false),
			mUseExtraDistanceConstraints(false),
			mUseLargerInertia			(false),
			mAttachHeavyBox				(false),
			mUseArticulation			(false),
			mFirstSphereIsStatic		(false)
			{}
		EditBoxPtr		mEditBox_NbSimCalls;
		EditBoxPtr		mEditBox_NbSpheres;
		EditBoxPtr		mEditBox_Radius;
		EditBoxPtr		mEditBox_Gap;
		EditBoxPtr		mEditBox_Mass;
		bool			mCenteredPivots;
		bool			mUseExtraDistanceConstraints;
		bool			mUseLargerInertia;
		bool			mAttachHeavyBox;
		bool			mUseArticulation;
		bool			mFirstSphereIsStatic;
	};

static const char* gDesc_ConfigurableSphericalChain = "Spherical chain. This tutorial demonstrates various ways to make \
a rope/chain more robust, and lets you play with the settings to see the effects on stability. It also supports MC articulations. \
There are some additional options like the 'inverse inertia scale' settings in PhysX 3.x or the TGS solver in PhysX 4.x that are not exposed \
in the test UI itself, and must/can be tweaked in the per-engine UI.";

START_TEST(ConfigurableSphericalChain, CATEGORY_JOINTS, gDesc_ConfigurableSphericalChain)

	ChainData	mData;

	static void CheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		ChainData* CD = (ChainData*)user_data;

		switch(check_box.GetID())
		{
			case CHAIN_DATA_CENTERED_PIVOTS:
				CD->mCenteredPivots = checked;
				break;
			case CHAIN_DATA_USE_DISTANCE_CONSTAINTS:
				CD->mUseExtraDistanceConstraints = checked;
				break;
			case CHAIN_DATA_USE_LARGER_INERTIA:
				CD->mUseLargerInertia = checked;
				break;
			case CHAIN_DATA_ATTACH_HEAVY_BOX:
				CD->mAttachHeavyBox = checked;
				break;
			case CHAIN_DATA_USE_ARTICULATION:
				CD->mUseArticulation = checked;
				break;
			case CHAIN_DATA_FIRST_SPHERE_IS_STATIC:
				CD->mFirstSphereIsStatic = checked;
				break;
		}
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 450;
		WD.mHeight	= 500;
		WD.mLabel	= "ConfigurableSphericalChain";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		const sdword EditBoxWidth = 60;
		const sdword LabelSize = 240;

		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelSize, 20, "Nb sim calls/frame:", &UIElems);
			mData.mEditBox_NbSimCalls = helper.CreateEditBox(UI, 1, 4+LabelSize, y, EditBoxWidth, 20, "4", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelSize, 20, "Nb spheres (more spheres is less stable):", &UIElems);
			mData.mEditBox_NbSpheres = helper.CreateEditBox(UI, CHAIN_DATA_NB_SPHERES, 4+LabelSize, y, EditBoxWidth, 20, "64", &UIElems, EDITBOX_INTEGER_POSITIVE, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelSize, 20, "Sphere radius (larger radius is more stable):", &UIElems);
			mData.mEditBox_Radius = helper.CreateEditBox(UI, CHAIN_DATA_RADIUS, 4+LabelSize, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelSize, 20, "Sphere mass:", &UIElems);
			mData.mEditBox_Mass = helper.CreateEditBox(UI, CHAIN_DATA_MASS, 4+LabelSize, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelSize, 20, "Gap between spheres:", &UIElems);
			mData.mEditBox_Gap = helper.CreateEditBox(UI, CHAIN_DATA_GAP, 4+LabelSize, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null);
			y += YStep;
		}

		{
			y += YStep;

			IceCheckBox* CB;

			CB = helper.CreateCheckBox(UI, CHAIN_DATA_CENTERED_PIVOTS, 4, y, 400, 20, "Place pivots at sphere centers (more stable).", &UIElems, mData.mCenteredPivots, ConfigurableSphericalChain::CheckBoxCallback, null);
			CB->SetUserData(&mData);
			y += YStep;

			CB = helper.CreateCheckBox(UI, CHAIN_DATA_USE_DISTANCE_CONSTAINTS, 4, y, 400, 20, "Use extra distance constraints between spheres (more stable)", &UIElems, mData.mUseExtraDistanceConstraints, ConfigurableSphericalChain::CheckBoxCallback, null);
			CB->SetUserData(&mData);
			y += YStep;

			CB = helper.CreateCheckBox(UI, CHAIN_DATA_USE_LARGER_INERTIA, 4, y, 400, 20, "Make the inertia tensor artificially larger (can be more stable)", &UIElems, mData.mUseLargerInertia, ConfigurableSphericalChain::CheckBoxCallback, null);
			CB->SetUserData(&mData);
			y += YStep;

			CB = helper.CreateCheckBox(UI, CHAIN_DATA_ATTACH_HEAVY_BOX, 4, y, 400, 20, "Attach heavy box (100X sphere mass) to last sphere (less stable)", &UIElems, mData.mAttachHeavyBox, ConfigurableSphericalChain::CheckBoxCallback, null);
			CB->SetUserData(&mData);
			y += YStep;

			CB = helper.CreateCheckBox(UI, CHAIN_DATA_USE_ARTICULATION, 4, y, 400, 20, "Use dedicated MC articulation feature (more stable)", &UIElems, mData.mUseArticulation, ConfigurableSphericalChain::CheckBoxCallback, null);
			CB->SetUserData(&mData);
			y += YStep;

			CB = helper.CreateCheckBox(UI, CHAIN_DATA_FIRST_SPHERE_IS_STATIC, 4, y, 400, 20, "Make first sphere static (else attach it to static anchor)", &UIElems, mData.mFirstSphereIsStatic, ConfigurableSphericalChain::CheckBoxCallback, null);
			CB->SetUserData(&mData);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 450-16);

		IceTabControl* TabControl;
		{
			TabControlDesc TCD;
			TCD.mParent	= UI;
			TCD.mX		= 4;
			TCD.mY		= y + 30;
			TCD.mWidth	= WD.mWidth - 16;
			TCD.mHeight	= 120;
			TabControl = ICE_NEW(IceTabControl)(TCD);
			RegisterUIElement(TabControl);
		}
		return TabControl;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(11.92f, -37.17f, 88.77f), Point(-0.13f, 0.07f, -0.99f));
		desc.mCamera[1] = PintCameraPose(Point(1.04f, -1.68f, 6.24f), Point(-0.20f, -0.09f, -0.98f));
		desc.mNbSimulateCallsPerFrame = GetInt(4, mData.mEditBox_NbSimCalls);
//		desc.mTimestep = (1.0f/60.0f)/desc.mNbSimulateCallsPerFrame;
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportSphericalJoints)
			return false;
		if(mData.mUseExtraDistanceConstraints && !caps.mSupportDistanceJoints)
			return false;
		if(mData.mUseArticulation && !caps.mSupportMCArticulations)
			return false;
		if(mData.mUseLargerInertia && !caps.mSupportMassForInertia)
			return false;

		// Filtering is used to disable collisions between two jointed objects.
		const bool UseFiltering = true;
		if(UseFiltering)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			const PintDisabledGroups DG(1, 2);
			pint.SetDisabledGroups(1, &DG);
		}

		const udword NbSpheres = GetInt(0, mData.mEditBox_NbSpheres);
		if(!NbSpheres)
			return true;

		const float Radius = GetFloat(0.5f, mData.mEditBox_Radius);
		const float Mass = GetFloat(1.0f, mData.mEditBox_Mass);
		const float Gap = GetFloat(0.5f, mData.mEditBox_Gap);

		const Point InitPos(0.0f, 0.0f, 0.0f);
		Point Pos = InitPos;
		PintActorHandle* Handles = new PintActorHandle[NbSpheres];
		Point* Positions = ICE_NEW(Point)[NbSpheres];

		PintArticHandle Articulation = mData.mUseArticulation ? pint.CreateArticulation(PINT_ARTICULATION_CREATE()) : null;

		udword GroupBit = 0;
		{
			PINT_SPHERE_CREATE SphereDesc(Radius);
			SphereDesc.mRenderer	= CreateSphereRenderer(Radius);

			const Point Offset((Radius+Gap)*2.0f, 0.0f, 0.0f);

			Point LocalPivot0, LocalPivot1;
			if(mData.mCenteredPivots)
			{
				LocalPivot0	= Point(0.0f, 0.0f, 0.0f);
				LocalPivot1	= Point(-(Radius+Gap)*2.0f, 0.0f, 0.0f);
			}
			else
			{
				LocalPivot0	= Point(Radius+Gap, 0.0f, 0.0f);
				LocalPivot1	= Point(-Radius-Gap, 0.0f, 0.0f);
			}

			for(udword i=0;i<NbSpheres;i++)
			{
				Positions[i] = Pos;

				PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
				ObjectDesc.mMass		= Mass;
				if(mData.mFirstSphereIsStatic && i==0)
					ObjectDesc.mMass	= 0.0f;

				if(mData.mUseLargerInertia)
					ObjectDesc.mMassForInertia = Mass*10.0f;
				ObjectDesc.mPosition	= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;

				if(Articulation)
				{
					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = i ? Handles[i-1] : null;
					ArticulatedDesc.mLocalPivot0 = LocalPivot0;
					ArticulatedDesc.mLocalPivot1 = LocalPivot1;
					Handles[i] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
				}
				else
					Handles[i] = CreatePintObject(pint, ObjectDesc);

				Pos += Offset;
			}

			if(!Articulation)
			{
				for(udword i=0;i<NbSpheres-1;i++)
				{
					PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[i], Handles[i+1], LocalPivot0, LocalPivot1));
					ASSERT(JointHandle);
				}
			}

			if(mData.mUseExtraDistanceConstraints)
			{
				for(udword i=0;i<NbSpheres;i++)
				{
					if(i+2<NbSpheres)
					{
						PINT_DISTANCE_JOINT_CREATE Desc;
						Desc.mObject0			= Handles[i];
						Desc.mObject1			= Handles[i+2];
						Desc.mLimits.mMaxValue	= Positions[i].Distance(Positions[i+2]);
						PintJointHandle JointHandle = pint.CreateJoint(Desc);
						ASSERT(JointHandle);
					}
				}
/*				for(udword i=0;i<NbSpheres-1;i++)
				{
					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0		= Handles[i];
					Desc.mObject1		= Handles[i+1];
					Desc.mMaxDistance	= Positions[i].Distance(Positions[i+1]);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}*/
			}
		}
		if(Articulation)
			pint.AddArticulationToScene(Articulation);

		// Attach first sphere to static world.
		if(!mData.mFirstSphereIsStatic)
		{
			PINT_BOX_CREATE BoxDesc(0.1f, 0.1f, 0.1f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 0.0f;
			ObjectDesc.mPosition		= InitPos;
			ObjectDesc.mCollisionGroup	= 2;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(h, Handles[0], Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f)));
			ASSERT(JointHandle);
		}

		// Attach heavy box to last sphere
		if(mData.mAttachHeavyBox)
		{
			PintActorHandle HeavyBox;
			const Point BoxExtents(Radius*10.0f, Radius*10.0f, Radius*10.0f);
			{
				PINT_BOX_CREATE BoxDesc(BoxExtents);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxExtents);

				Pos.x += BoxExtents.x - Radius;

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= Mass * 100.0f;
				ObjectDesc.mPosition		= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
				HeavyBox = CreatePintObject(pint, ObjectDesc);
			}

			PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[NbSpheres-1], HeavyBox, Point(Radius, 0.0f, 0.0f), Point(-BoxExtents.x, 0.0f, 0.0f)));
			ASSERT(JointHandle);
		}

		DELETEARRAY(Positions);
		DELETEARRAY(Handles);
		return true;
	}

END_TEST(ConfigurableSphericalChain)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_StableSphericalChain = "Demonstrates how to make a long, stable spherical chain with regular joints (no articulations). \
This PEEL 1.1 test is similar to the previous ConfigurableSphericalChain test, but hardcoded for a rope made of 256 spheres of mass 1, \
at the end of which we attach a box of mass 100. Ideally the rope should not stretch and the box should not go below the edge of the render window. \
You can increase the number of solver iteration counts in each engine to reduce stretching if needed. Tricks used in this specific test are extra \
distance constraints and inertia tensor tweaks. This worked fairly well even back in PhysX 2.x.";

START_TEST(StableSphericalChain, CATEGORY_JOINTS, gDesc_StableSphericalChain)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(57.35f, -242.04f, 493.93f), Point(-0.06f, -0.05f, -1.00f));
		desc.mNbSimulateCallsPerFrame = 4;
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportSphericalJoints)
			return false;

		bool UseDistanceConstraints = true;
		if(!caps.mSupportDistanceJoints)
		{
			printf(_F("WARNING: %s doesn't support distance joints, feature is disabled.\n", pint.GetName()));
			UseDistanceConstraints = false;
		}

		if(!caps.mSupportMassForInertia)
		{
			printf(_F("WARNING: %s doesn't support 'mass for inertia', feature is ignored.\n", pint.GetName()));
		}

		// Filtering is used to disable collisions between two jointed objects.
		const bool UseFiltering = true;
		if(UseFiltering)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			const PintDisabledGroups DG(1, 2);
			pint.SetDisabledGroups(1, &DG);
		}

		const udword NbSpheres = 256;
		const float Radius = 1.0f;
		const float Mass = 1.0f;

		const Point InitPos(0.0f, 0.0f, 0.0f);
		Point Pos = InitPos;
		PintActorHandle* Handles = new PintActorHandle[NbSpheres];
		Point* Positions = ICE_NEW(Point)[NbSpheres];

		udword GroupBit = 0;
		{
			PINT_SPHERE_CREATE SphereDesc(Radius);
			SphereDesc.mRenderer = CreateSphereRenderer(Radius);

			const Point Offset(Radius*2.0f, 0.0f, 0.0f);
			const Point LocalPivot0	= Point(0.0f, 0.0f, 0.0f);
			const Point LocalPivot1	= Point(-Radius*2.0f, 0.0f, 0.0f);

			for(udword i=0;i<NbSpheres;i++)
			{
				Positions[i] = Pos;

				PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
				ObjectDesc.mMass			= Mass;
				if(i==0)
					ObjectDesc.mMass		= 0.0f;
				ObjectDesc.mMassForInertia	= Mass*10.0f;
				ObjectDesc.mPosition		= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
				Handles[i] = CreatePintObject(pint, ObjectDesc);

				Pos += Offset;
			}

			for(udword i=0;i<NbSpheres-1;i++)
			{
				PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[i], Handles[i+1], LocalPivot0, LocalPivot1));
				ASSERT(JointHandle);
			}

			if(UseDistanceConstraints)
			{
				for(udword i=0;i<NbSpheres;i++)
				{
					if(i+2<NbSpheres)
					{
						PINT_DISTANCE_JOINT_CREATE Desc;
						Desc.mObject0			= Handles[i];
						Desc.mObject1			= Handles[i+2];
						Desc.mLimits.mMaxValue	= Positions[i].Distance(Positions[i+2]);
						PintJointHandle JointHandle = pint.CreateJoint(Desc);
						ASSERT(JointHandle);
					}
				}
			}
		}

		// Attach heavy box to last sphere
		{
			PintActorHandle HeavyBox;
			const Point BoxExtents(Radius*10.0f, Radius*10.0f, Radius*10.0f);
			{
				PINT_BOX_CREATE BoxDesc(BoxExtents);
				BoxDesc.mRenderer = CreateBoxRenderer(BoxExtents);

				Pos.x += BoxExtents.x - Radius;

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= Mass * 100.0f;
				ObjectDesc.mPosition		= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
				HeavyBox = CreatePintObject(pint, ObjectDesc);
			}

			PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[NbSpheres-1], HeavyBox, Point(Radius, 0.0f, 0.0f), Point(-BoxExtents.x, 0.0f, 0.0f)));
			ASSERT(JointHandle);
		}

		DELETEARRAY(Positions);
		DELETEARRAY(Handles);
		return true;
	}

END_TEST(StableSphericalChain)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CaterpillarTrack = "Rough/experimental caterpillar track using regular joints. Not properly tuned or optimized.";

START_TEST(CaterpillarTrack, CATEGORY_JOINTS, gDesc_CaterpillarTrack)

	CylinderMesh	mCylinder;
	CylinderMesh	mCylinder2;
	PtrContainer	mTrackObjects;
	EditBoxPtr		mEditBox_Multiplier;

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 200;
		WD.mLabel	= "Caterpillar track";
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
		sdword y = 10;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Constraint multiplier:", &UIElems);
		mEditBox_Multiplier = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "32", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
		y += YStep;

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(21.58f, 2.71f, 10.92f), Point(-0.62f, 0.06f, -0.78f));
		desc.mCamera[1] = PintCameraPose(Point(-0.09f, 2.56f, 10.53f), Point(-0.01f, 0.04f, -1.00f));
		SetDefEnv(desc, true);
	}

	virtual	bool	CommonSetup()
	{
		const float HalfHeight = 0.5f;
		const float Radius = 0.6f;
		const udword NbCirclePts = 16;
		mCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mCylinder.mNbVerts, mCylinder.mVerts));

		const float HalfHeight2 = 0.15f;
		const float Radius2 = 1.0f;
		mCylinder2.Generate(NbCirclePts, Radius2, HalfHeight2);
		RegisterRenderer(CreateConvexRenderer(mCylinder2.mNbVerts, mCylinder2.mVerts));

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		const udword Size = mTrackObjects.GetNbEntries();
		for(udword i=0;i<Size;i++)
		{
			CaterpillarTrackObjects* CTO = (CaterpillarTrackObjects*)mTrackObjects[i];
			DELETESINGLE(CTO);
		}
		mTrackObjects.Empty();

		mCylinder.Reset();
		mCylinder2.Reset();
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes || !caps.mSupportCompounds || !caps.mSupportHingeJoints)
			return false;

		const udword Multiplier = GetInt(32, mEditBox_Multiplier);

		CaterpillarTrackObjects* TO = ICE_NEW(CaterpillarTrackObjects);
		CreateCaterpillarTrack_OLD(pint, *TO, &mHighFrictionMaterial, Point(0.0f, 2.0f, 0.0f), mCylinder, GetRegisteredRenderers()[0], mCylinder2, GetRegisteredRenderers()[1], Multiplier);

		pint.mUserData = TO;
		mTrackObjects.AddPtr(TO);

		return true;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		// Make the object move automatically for the demo
		if(1)
		{
			CaterpillarTrackObjects* TO = reinterpret_cast<CaterpillarTrackObjects*>(pint.mUserData);
			for(udword i=0;i<TO->mNbGears;i++)
			{
				if(TO->mGears[i])
					pint.SetAngularVelocity(TO->mGears[i], Point(0.0f, 0.0f, -5.0f));
			}
		}
		return TestBase::Update(pint, dt);
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)
	{
		CaterpillarTrackObjects* TO = reinterpret_cast<CaterpillarTrackObjects*>(pint.mUserData);
		TO->OnObjectReleased(removed_object);
	}

END_TEST(CaterpillarTrack)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CaterpillarTrackOnBridge = "Caterpillar track on bridge. Artificial benchmark scene involving a lot of joints.";

START_TEST(CaterpillarTrackOnBridge, CATEGORY_JOINTS, gDesc_CaterpillarTrackOnBridge)

	CylinderMesh	mCylinder;
	CylinderMesh	mCylinder2;
	EditBoxPtr		mEditBox_Multiplier;
	EditBoxPtr		mEditBox_NbX;
	EditBoxPtr		mEditBox_NbZ;

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 200;
		WD.mLabel	= "Caterpillar track";
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
		sdword y = 10;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Constraint multiplier:", &UIElems);
		mEditBox_Multiplier = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "32", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "NbX:", &UIElems);
		mEditBox_NbX = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "2", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "NbZ:", &UIElems);
		mEditBox_NbZ = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "8", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
		y += YStep;

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual	float	GetRenderData(Point& center)const
	{
		return 800.0f;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(25.09f, 15.86f, -16.50f), Point(-0.11f, -0.56f, 0.82f));
		desc.mCamera[1] = PintCameraPose(Point(21.58f, 2.71f, 10.92f), Point(-0.62f, 0.06f, -0.78f));
		desc.mCamera[2] = PintCameraPose(Point(-33.26f, 93.85f, 138.49f), Point(0.65f, -0.54f, -0.54f));
		SetDefEnv(desc, false);
	}

	virtual	bool	CommonSetup()
	{
		const float HalfHeight = 0.5f;
		const float Radius = 0.6f;
		const udword NbCirclePts = 16;
		mCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mCylinder.mNbVerts, mCylinder.mVerts));

		const float HalfHeight2 = 0.15f;
		const float Radius2 = 1.0f;
		mCylinder2.Generate(NbCirclePts, Radius2, HalfHeight2);
		RegisterRenderer(CreateConvexRenderer(mCylinder2.mNbVerts, mCylinder2.mVerts));

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		mCylinder2.Reset();
		TestBase::CommonRelease();
	}

	struct PintData : public Allocateable
	{
		PtrContainer	mGears;
	};

	void	CreateScene(Pint& pint, const Point& pos)
	{
		const udword Multiplier = GetInt(32, mEditBox_Multiplier);

		CaterpillarTrackObjects TO;
		CreateCaterpillarTrack_OLD(pint, TO, &mHighFrictionMaterial, pos+Point(3.0f, 2.0f, 0.0f), mCylinder, GetRegisteredRenderers()[0], mCylinder2, GetRegisteredRenderers()[1], Multiplier);

		PintData* UserData;
		if(pint.mUserData)
		{
			UserData = reinterpret_cast<PintData*>(pint.mUserData);
		}
		else
		{
			UserData = ICE_NEW(PintData);
			pint.mUserData = UserData;
		}

		for(udword i=0;i<TO.mNbGears;i++)
			UserData->mGears.AddPtr(TO.mGears[i]);

		if(1)
		{
			const Point Extents(1.0f, 0.3f, 4.0f);
			const udword NbBoxes = 20;
			const udword NbRows = 10;
			const Point Dir(1.0f, 0.0f, 0.0f);
		//	const Point PosOffset = Dir*(Extents.x + 0.1f);
			const Point PosOffset = Dir*Extents.x;

			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);
	//		BoxDesc.mExtents	= Point(2.0f, 0.1f, 2.0f);

			float Chainette[NbBoxes+1];
			float Dy[NbBoxes+1];
			const float Coeff = 20.0f;
			for(udword i=0;i<NbBoxes+1;i++)
			{
				const float f0 = float(i) - float(NbBoxes/2);
				const float f1 = float(i+1) - float(NbBoxes/2);

				Chainette[i] = Coeff*coshf(f0/Coeff);

				Dy[i] = Coeff*(coshf(f1/Coeff) - coshf(f0/Coeff));
			}

			//for(udword j=0;j<NbRows;j++)
			udword j=0;
			{
				const float RowCoeff = float(j)/float(NbRows-1);
				PintActorHandle Handles[NbBoxes];

				Point Pos(0.0f, 0.0f, float(j)*Extents.z*4.0f);
				Pos += pos;

				Point RightPos(0.0f, 0.0f, 0.0f);
				udword GroupBit = 0;
				for(udword i=0;i<NbBoxes;i++)
				{
					float Mass = 1.0f;
					if(i==0 || i==NbBoxes-1)
						Mass = 0.0f;

					Matrix3x3 Rot;
					const float Alpha = atanf(Dy[i]);
					const float CurveCoeff = 0.5f + RowCoeff * 2.5f;
					Rot.RotZ(Alpha*CurveCoeff);

					Point R;
					Rot.GetRow(0, R);

					{
						PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
						ObjectDesc.mMass			= Mass;
						ObjectDesc.mPosition		= Pos;
	//					ObjectDesc.mPosition.y		= (Chainette[i] + Chainette[i+1])*0.5f;
	//					ObjectDesc.mPosition.y		+= Chainette[i] - 10.0f;

						ObjectDesc.mRotation = Rot;

						const Point LeftPos = Pos - R*Extents.x;
						if(i)
						{
							Pos += RightPos - LeftPos;
							//ObjectDesc.mPosition += RightPos - LeftPos;
							ObjectDesc.mPosition		= Pos;
						}

						ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
						Handles[i] = CreatePintObject(pint, ObjectDesc);
					}

					RightPos = Pos + R*Extents.x;

	//				Pos += PosOffset*2.0f;
					Pos += R*2.0f*Extents.x;
				}

				//for(udword j=0;j<16;j++)
				{
				for(udword i=0;i<NbBoxes-1;i++)
				{
					PINT_HINGE_JOINT_CREATE Desc;
					Desc.mObject0		= Handles[i];
					Desc.mObject1		= Handles[i+1];
					Desc.mLocalPivot0	= PosOffset;
					Desc.mLocalPivot1	= -PosOffset;
					Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
					Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
				}
			}
		}
	}

	virtual void	Close(Pint& pint)
	{
		PintData* UserData = reinterpret_cast<PintData*>(pint.mUserData);
		DELETESINGLE(UserData);
		pint.mUserData = null;

		TestBase::Close(pint);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes || !caps.mSupportCompounds || !caps.mSupportHingeJoints)
			return false;

		const bool UseFiltering = true;
		if(UseFiltering)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			const PintDisabledGroups DG(1, 2);
			pint.SetDisabledGroups(1, &DG);
		}

		const udword NbX = GetInt(2, mEditBox_NbX);
		const udword NbZ = GetInt(8, mEditBox_NbZ);
		for(udword j=0;j<NbX;j++)
		for(udword k=0;k<NbZ;k++)
		{
			CreateScene(pint, Point(float(j)*40.0f, 0.0f, float(k)*10.0f));
		}

		return true;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		if(1)
		{
			const PintData* UserData = reinterpret_cast<const PintData*>(pint.mUserData);

			const PtrContainer& Gears = UserData->mGears;

			for(udword i=0;i<Gears.GetNbEntries();i++)
			{
				if(Gears.GetEntry(i))
					pint.SetAngularVelocity(PintActorHandle(Gears.GetEntry(i)), Point(0.0f, 0.0f, -5.0f));
			}
		}
		return TestBase::Update(pint, dt);
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)
	{
		PintData* UserData = reinterpret_cast<PintData*>(pint.mUserData);
		PtrContainer& Gears = UserData->mGears;
		for(udword i=0;i<Gears.GetNbEntries();i++)
		{
			if(PintActorHandle(Gears.GetEntry(i))==removed_object)
				Gears[i] = null;
		}
	}

END_TEST(CaterpillarTrackOnBridge)

///////////////////////////////////////////////////////////////////////////////

#include "Camera.h"

namespace
{
static const bool gDriveVehicle = true;

	struct WheelData : public Allocateable
	{
		WheelData() : mParent(null), mWheel(null)
		{
		}

		PintActorHandle	mParent;
		PintActorHandle	mWheel;
	};

	struct ChassisData : public Allocateable
	{
		ChassisData() : mChassis(null)//, mNbWheels(0)
		{
			for(udword i=0;i<2;i++)
				mFrontAxleObject[i] = null;
		}
		PintActorHandle	mChassis;
		WheelData			mFront[2];
		WheelData			mRear[2];
		PintActorHandle	mFrontAxleObject[2];
	};

	class VehicleBase : public VehicleInput
	{
		public:
									VehicleBase();
		virtual						~VehicleBase();

		virtual	void				Close(Pint& pint);
		virtual	bool				CommonSetup();

				float				mAcceleration;
				float				mMaxAngularVelocity;
				float				mSteeringForce;
				//
				bool				mClampAngularVelocity;
				bool				mLoadTerrain;

				bool				ClampAngularVelocity(Pint& pint, const ChassisData& vehicle_data);
	};

VehicleBase::VehicleBase() :
	mAcceleration			(0.0f),
	mMaxAngularVelocity		(0.0f),
	mSteeringForce			(0.0f),
	mClampAngularVelocity	(false),
	mLoadTerrain			(false)
{
}

VehicleBase::~VehicleBase()
{
}

void VehicleBase::Close(Pint& pint)
{
	ChassisData* UserData = (ChassisData*)pint.mUserData;
	DELETESINGLE(UserData);
	pint.mUserData = null;

	VehicleInput::Close(pint);
}

bool VehicleBase::CommonSetup()
{
	if(mLoadTerrain)
	{
//		LoadMeshesFromFile_(*this, "Terrain.bin", null, false, 0);
			const Point Scale(1.0f, 0.2f, 1.0f);
			LoadMeshesFromFile_(*this, "Terrain.bin", &Scale, false, 0);
	}
	return VehicleInput::CommonSetup();
}

bool VehicleBase::ClampAngularVelocity(Pint& pint, const ChassisData& vehicle_data)
{
	bool CanAccelerate = true;
	if(mClampAngularVelocity)
	{
		const float MaxAngularVelocity = mMaxAngularVelocity;
		for(udword i=0;i<2;i++)
		{
			if(vehicle_data.mFront[i].mWheel && vehicle_data.mFront[i].mParent)
			{
//				Point AngularVelocity = pint.GetAngularVelocity(vehicle_data.mFront[i].mWheel);
//				printf("Angular velocity %d: %f %f %f\n", i, AngularVelocity.x, AngularVelocity.y, AngularVelocity.z);

				if(::ClampAngularVelocity(pint, vehicle_data.mFront[i].mParent, vehicle_data.mFront[i].mWheel, MaxAngularVelocity))
					CanAccelerate = false;
			}
		}
		for(udword i=0;i<2;i++)
		{
			if(vehicle_data.mRear[i].mWheel && vehicle_data.mRear[i].mParent)
			{
//				Point AngularVelocity = pint.GetAngularVelocity(vehicle_data.mRear[i].mWheel);
//				printf("Angular velocity %d: %f %f %f\n", i, AngularVelocity.x, AngularVelocity.y, AngularVelocity.z);

				if(::ClampAngularVelocity(pint, vehicle_data.mRear[i].mParent, vehicle_data.mRear[i].mWheel, MaxAngularVelocity))
					CanAccelerate = false;
			}
		}
	}
	return CanAccelerate;
}

}

#define START_VEHICLE_TEST(name, category, desc)										\
	class name : public VehicleBase														\
	{																					\
		public:																			\
								name()						{						}	\
		virtual					~name()						{						}	\
		virtual	const char*		GetName()			const	{ return #name;			}	\
		virtual	const char*		GetDescription()	const	{ return desc;			}	\
		virtual	TestCategory	GetCategory()		const	{ return category;		}

static const char* gDesc_Tank = "Tank test. This uses regular joints (no articulation). This is an experimental, work-in-progress test. It has not been properly tuned or \
optimized, and it has not been tested in engines other than PhysX 3.4. Use the arrow keys to control the tank. Use the PageUp and PageDown keys to switch between different \
camera views. Controls are experimental.";

START_VEHICLE_TEST(Tank, CATEGORY_WIP, gDesc_Tank)

	CylinderMesh mCylinder;
	CylinderMesh mCylinder2;

	virtual	float	GetRenderData(Point& center)	const
	{
		center = GetCameraPos();
		return 1000.0f;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleBase::GetSceneParams(desc);
//		desc.mCamera[0] = PintCameraPose(Point(10.76f, 11.91f, 14.07f), Point(-0.54f, -0.49f, -0.68f));
		desc.mCamera[0] = PintCameraPose(Point(-27.42f, 10.88f, 79.49f), Point(0.97f, -0.26f, 0.01f));

		mFilteredCameraPos = desc.mCamera[0].mPos;
		mFilteredCameraDir = desc.mCamera[0].mDir;
		SetDefEnv(desc, false);
	}

	virtual	bool	CommonSetup()
	{
		mSteeringForce			= 100.0f;
		mAcceleration			= 100.0f;
		mMaxAngularVelocity		= 10.0f;
//		mMaxAngularVelocity		= 5.0f;
		mCamera.mUpOffset		= 4.0f;
		mCamera.mDistToTarget	= 15.0f;
		mClampAngularVelocity	= true;
		mControlCamera			= gDriveVehicle;
		mLoadTerrain			= true;

		const float HalfHeight = 0.5f;
		const float Radius = 0.6f;
		const udword NbCirclePts = 16;
		mCylinder.Generate(NbCirclePts, Radius, HalfHeight);
		RegisterRenderer(CreateConvexRenderer(mCylinder.mNbVerts, mCylinder.mVerts));

		const float HalfHeight2 = 0.15f;
		const float Radius2 = 1.0f;
//		const float Radius2 = 1.1f;
		mCylinder2.Generate(NbCirclePts, Radius2, HalfHeight2);
		RegisterRenderer(CreateConvexRenderer(mCylinder2.mNbVerts, mCylinder2.mVerts));

		return VehicleBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mCylinder.Reset();
		mCylinder2.Reset();
		VehicleBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportConvexes || !caps.mSupportCompounds || !caps.mSupportHingeJoints)
			return false;

		CreateMeshesFromRegisteredSurfaces(pint, caps, &mHighFrictionMaterial);

		const float Altitude = 4.0f;
		const float D = 6.0f;

		Point RefPoint(0.0f, 4.0f+Altitude, D*0.5f);
		Point Pos0(0.0f, 2.0f+Altitude, 0.0f);
		Point Pos1(0.0f, 2.0f+Altitude, D);
		const Point Offset0 = RefPoint - Pos0;
		const Point Offset1 = RefPoint - Pos1;

		const Point StartPosPoint(-13.69f, 10.50f, 79.74f);
//		const Point StartPosPoint(11.05f, 7.94f, 63.99f);
		RefPoint = StartPosPoint;
		Pos0 = StartPosPoint - Offset0;
		Pos1 = StartPosPoint - Offset1;

		//
		CaterpillarTrackObjects TO0;
		CreateCaterpillarTrack_OLD(pint, TO0, &mHighFrictionMaterial, Pos0, mCylinder, GetRegisteredRenderers()[0], mCylinder2, GetRegisteredRenderers()[1]);

		CaterpillarTrackObjects TO1;
		CreateCaterpillarTrack_OLD(pint, TO1, &mHighFrictionMaterial, Pos1, mCylinder, GetRegisteredRenderers()[0], mCylinder2, GetRegisteredRenderers()[1]);

		ChassisData* UserData = ICE_NEW(ChassisData);
		pint.mUserData = UserData;
//		UserData->mChassis = TO0.mChassis;
		UserData->mFront[0].mWheel = TO0.mGears[0];
		UserData->mFront[1].mWheel = TO1.mGears[0];
		UserData->mFront[0].mParent = TO0.mChassis;
		UserData->mFront[1].mParent = TO0.mChassis;
		UserData->mRear[0].mWheel = TO0.mGears[1];
		UserData->mRear[1].mWheel = TO1.mGears[1];
		UserData->mRear[0].mParent = TO0.mChassis;
		UserData->mRear[1].mParent = TO0.mChassis;
//		UserData->mFrontAxleObject[0] = TO.mChassis;
//		UserData->mFrontAxleObject[1] = TO.mChassis;
		{
			PINT_FIXED_JOINT_CREATE fjc;
			fjc.mObject0 = TO0.mChassis;
			fjc.mObject1 = TO1.mChassis;
			fjc.mLocalPivot0 = Point(0.0f, 0.0f, D*0.5f);
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, -D*0.5f);
			pint.CreateJoint(fjc);
		}
		{
			PINT_BOX_CREATE BoxDesc(3.0f, 2.0f, D*0.2f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition		= RefPoint;
			ObjectDesc.mMass			= 1.0f;
//			ObjectDesc.mCollisionGroup	= WheelGroup;
			PintActorHandle Object = CreatePintObject(pint, ObjectDesc);
			UserData->mChassis = Object;

			PINT_FIXED_JOINT_CREATE fjc;
			fjc.mObject0 = Object;
			fjc.mObject1 = TO0.mChassis;
			fjc.mLocalPivot0 = Point(0.0f, -2.0f, -BoxDesc.mExtents.x);
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, D*0.5f-BoxDesc.mExtents.x);
			pint.CreateJoint(fjc);

			fjc.mObject0 = Object;
			fjc.mObject1 = TO1.mChassis;
			fjc.mLocalPivot0 = Point(0.0f, -2.0f, BoxDesc.mExtents.x);
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, -D*0.5f+BoxDesc.mExtents.x);
			pint.CreateJoint(fjc);
		}
		return true;
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		if(!gDriveVehicle)
			return 0;

		ChassisData* UserData = (ChassisData*)pint.mUserData;
		if(!UserData)
			return 0;

		bool CanAccelerate = ClampAngularVelocity(pint, *UserData);

		{
			const float Coeff = mSteeringForce;
			if(mInput.mKeyboard_Right)
			{
				pint.AddLocalTorque(UserData->mFront[0].mWheel, Point(0.0f, 0.0f, -Coeff));
				pint.AddLocalTorque(UserData->mRear[0].mWheel, Point(0.0f, 0.0f, -Coeff));
				pint.AddLocalTorque(UserData->mFront[1].mWheel, Point(0.0f, 0.0f, Coeff));
				pint.AddLocalTorque(UserData->mRear[1].mWheel, Point(0.0f, 0.0f, Coeff));
			}
			if(mInput.mKeyboard_Left)
			{
				pint.AddLocalTorque(UserData->mFront[1].mWheel, Point(0.0f, 0.0f, -Coeff));
				pint.AddLocalTorque(UserData->mRear[1].mWheel, Point(0.0f, 0.0f, -Coeff));
				pint.AddLocalTorque(UserData->mFront[0].mWheel, Point(0.0f, 0.0f, Coeff));
				pint.AddLocalTorque(UserData->mRear[0].mWheel, Point(0.0f, 0.0f, Coeff));
			}
		}


		const bool FWD = true;
		const bool RWD = true;
		{
			const float Coeff = mAcceleration;
			if(mInput.mKeyboard_Accelerate /*&& CanAccelerate*/)
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							pint.AddLocalTorque(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, -Coeff));
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							pint.AddLocalTorque(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, -Coeff));
					}
				}
			}
			if(mInput.mKeyboard_Brake)
			{
				if(FWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mFront[i].mWheel)
							pint.AddLocalTorque(UserData->mFront[i].mWheel, Point(0.0f, 0.0f, Coeff));
					}
				}
				if(RWD)
				{
					for(udword i=0;i<2;i++)
					{
						if(UserData->mRear[i].mWheel)
							pint.AddLocalTorque(UserData->mRear[i].mWheel, Point(0.0f, 0.0f, Coeff));
					}
				}
			}
		}

		// Camera
		UpdateCamera(pint, UserData->mChassis);

		return 0;
	}

END_TEST(Tank)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SteeringTest4b = "Lego Technic steering test 4b";

START_TEST(SteeringTest4b, CATEGORY_JOINTS, gDesc_SteeringTest4b)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		desc.mCamera[0] = PintCameraPose(Point(3.89f, 9.77f, 12.64f), Point(-0.42f, -0.36f, -0.83f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		const char* Filename = FindPEELFile("SteeringTest4c.zb2");
		if(Filename)
		{
			PINT_WORLD_CREATE desc;
			ImportZB2File(desc, Filename);
		}

		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateZB2Scene(pint, caps);
	}

END_TEST(SteeringTest4b)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DuneBuggySteeringTest = "Lego Technic Dune Buggy steering test. This one exposed two modelling issues in the original Mecabricks file: some of the \
gears' teeth overlap, and the orientation of the steering column is slightly wrong. It's invisible in Mecabricks but it becomes obvious when trying to simulate the model...";

START_TEST(DuneBuggySteeringTest, CATEGORY_JOINTS, gDesc_DuneBuggySteeringTest)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		desc.mCamera[0] = PintCameraPose(Point(2.53f, 6.62f, 14.48f), Point(-0.30f, -0.32f, -0.90f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		const char* Filename = FindPEELFile("DuneBuggy_SteeringTest2.zb2");
		if(Filename)
		{
			PINT_WORLD_CREATE desc;
			ImportZB2File(desc, Filename);
		}

		return TestBase::CommonSetup();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return CreateZB2Scene(pint, caps);
	}

END_TEST(DuneBuggySteeringTest)

///////////////////////////////////////////////////////////////////////////////

class LegoTechnicVehicle : public VehicleInput
{
	protected:
	float	mLightRange;
	public:

	struct PintData	: public Allocateable
	{
		PintData() : mChassis(null), mSteeringJoint(null)
		{
			for(udword i=0;i<4;i++)
				mWheels[i] = null;
		}

		PintActorHandle	mChassis;
		PintJointHandle	mSteeringJoint;
		PintJointHandle	mWheels[4];
	};

							LegoTechnicVehicle() : mLightRange(50.0f)	{							}
	virtual					~LegoTechnicVehicle()						{							}
	virtual	TestCategory	GetCategory()						const	{ return CATEGORY_JOINTS;	}

	virtual	void			Close(Pint& pint)
	{
		PintData* PD = reinterpret_cast<PintData*>(pint.mUserData);
		DELETESINGLE(PD);

		VehicleInput::Close(pint);
	}

	virtual	float			GetRenderData(Point& center)	const
	{
		center = mCameraTarget;
		return mLightRange;
	}
};

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_LegoTechnicKart = "Lego Technic kart. TODO: this scene needs to be scaled down";

extern float gZCB2_RenderScale;

static const float gScaleCoeff = 1.0f;

static const float gSteerLimit = 0.5f;
static const float gSteerStiffness = 1500.0f;
static const float gSteerDamping = 500.0f;

//static const float gSpeed = 5.0f;
static const float gSpeed = 10.0f;
static const bool gEnableFrontWheels = true;
static const bool gEnableBackWheels = false;

class LegoTechnicKart : public LegoTechnicVehicle
{
	public:
							LegoTechnicKart()			{								}
	virtual					~LegoTechnicKart()			{								}
	virtual	const char*		GetName()			const	{ return "LegoTechnicKart";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_LegoTechnicKart;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("Lego Technic Kart", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleInput::GetSceneParams(desc);

		desc.mCamera[0] = PintCameraPose(Point(14.53f, 15.69f, 11.22f), Point(-0.64f, -0.62f, -0.45f));
		SetDefEnv(desc, true);
		//desc.mGravity = Point(0.0f, -100.0f, 0.0f);
	}

	class MyCustomizeCallback : public ZB2CustomizeCallback
	{
		public:
						MyCustomizeCallback()	{}
		virtual			~MyCustomizeCallback()	{}

		virtual	void	CustomizeMaterial(PINT_MATERIAL_CREATE&)
		{
//			printf("...CustomizeMaterial\n");
		}

		virtual	void	CustomizeMesh(PINT_MESH_DATA_CREATE&)
		{
//			printf("...CustomizeMesh\n");
		}

		virtual	void	CustomizeShape(PINT_SHAPE_CREATE& create)
		{
//			printf("...CustomizeShape\n");

			// ######### todo better
			static PINT_MATERIAL_CREATE m(1.0f, 1.0f, 0.0f);

			create.mMaterial = &m;
		}

		virtual	void	CustomizeActor(PINT_OBJECT_CREATE& create)
		{
//			printf("...CustomizeActor\n");
			//create.mMass = 10.0f;
			//create.mMassForInertia = 10.0f;
			if(create.mName)
			{
//				if(strcmp(create.mName, "RearWheel")==0 || strcmp(create.mName, "FrontWheel")==0)
//					create.mMass = 10.0f;
//				if(strcmp(create.mName, "pipo")==0)
//					create.mMass = 0.1f;
			}
		}

		virtual	void	CustomizeJoint(PINT_JOINT_CREATE& create)
		{
//			printf("...CustomizeJoint\n");
			if(create.mType==PINT_JOINT_D6)
			{
				PINT_D6_JOINT_CREATE& d6 = static_cast<PINT_D6_JOINT_CREATE&>(create);
				d6.mMotorFlags		= PINT_D6_MOTOR_DRIVE_X;
				d6.mMotorStiffness	= gSteerStiffness;
				d6.mMotorDamping	= gSteerDamping;
			}
		}
	};

	virtual bool	CommonSetup()
	{
		mCameraStyle = true;
		gZCB2_RenderScale = 0.35f * gScaleCoeff;

		const char* Filename = FindPEELFile("LegoTechnic_Kart2c.zb2");
		if(Filename)
		{
			MyCustomizeCallback CB;

			PINT_WORLD_CREATE desc;
			ImportZB2File(desc, Filename, &CB);

			if(0)
			{
				const udword NbActors = mZB2Factory->GetNbActors();
				const ZCB2Factory::ActorCreate* Actors = mZB2Factory->GetActors();

				for(udword i=0;i<NbActors;i++)
				{
					if(Actors[i].mCreate)
					{
						const udword NbShapes = Actors[i].mCreate->GetNbShapes();
						printf("Actor %d: (%d shapes) %s\n", i, NbShapes, Actors[i].mCreate->mName);
					}
				}
			}

			if(1)
			{
				const udword NbJoints = mZB2Factory->mJoints.GetNbEntries();
				for(udword i=0;i<NbJoints;i++)
				{
					PINT_JOINT_CREATE* Create = reinterpret_cast<PINT_JOINT_CREATE*>(mZB2Factory->mJoints[i]);
					if(Create)
					{
//						printf("Joint %d (type %d): %s\n", i, Create->mType, Create->mName);

						if(Create->mType==PINT_JOINT_HINGE2)
						{
							PINT_HINGE2_JOINT_CREATE* hjc = static_cast<PINT_HINGE2_JOINT_CREATE*>(Create);

							const float Speed = 0.0f;
							if(gEnableFrontWheels)
							{
								if(i==0)
								{
									hjc->mUseMotor		= true;
									hjc->mDriveVelocity	= Speed;
								}
								else if(i==1)
								{
									hjc->mUseMotor		= true;
									hjc->mDriveVelocity	= -Speed;
								}
							}
							if(gEnableBackWheels)
							{
								if(i==14 || i==15)
								{
									hjc->mUseMotor		= true;
									hjc->mDriveVelocity	= Speed;
								}
							}
						}
					}
				}
			}
		}

		if(0)
			mPlanarMeshHelper.Generate(200, 0.5f);

		return VehicleInput::CommonSetup();
	}

	class MyCreationCallback : public ZB2CreationCallback
	{
		PintData&	mData;

		public:

		MyCreationCallback(PintData& data) : mData(data)
		{
		}

		virtual	void	NotifyCreatedObjects(Pint& pint, const ZB2CreatedObjects& objects, ZCB2Factory*)	override
		{
//			printf("%d mesh objects\n", objects.mNbMeshObjects);
//			printf("%d actors\n", objects.mNbActors);
//			printf("%d joints\n", objects.mNbJoints);

			{
				Pint_Actor* ActorAPI = pint.GetActorAPI();
				if(!ActorAPI)
					return;

				PintActorHandle MainActor = null;
				udword MaxNbShapes = 0;
				for(udword i=0;i<objects.mNbActors;i++)
				{
					const PintActorHandle CurrentActor = objects.mActors[i].mHandle;

					udword NbShapes = ActorAPI->GetNbShapes(CurrentActor);
					if(NbShapes>MaxNbShapes)
					{
						MaxNbShapes = NbShapes;
						MainActor = CurrentActor;
					}
				}

				mData.mChassis = MainActor;
			}

			{
				Pint_Joint* JointAPI = pint.GetJointAPI();
				if(!JointAPI)
					return;

				for(udword i=0;i<objects.mNbJoints;i++)
				{
					PintJointHandle CurrentJoint = objects.mJoints[i];

					if(JointAPI->GetType(CurrentJoint)==PINT_JOINT_D6)
					{
						mData.mSteeringJoint = CurrentJoint;
					}
				}

				// ### hardcoded
				if(gEnableFrontWheels)
				{
					mData.mWheels[0] = objects.mJoints[0];
					mData.mWheels[1] = objects.mJoints[1];
				}
				if(gEnableBackWheels)
				{
					mData.mWheels[2] = objects.mJoints[14];
					mData.mWheels[3] = objects.mJoints[15];
				}
			}
		}
	};

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		gZCB2_RenderScale = 1.0f;

		if(0)
		{
			const float Altitude = 0.0f;
			mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, null);
		}

		PintData* PD = ICE_NEW(PintData);
		pint.mUserData = PD;

		MyCreationCallback cb(*PD);
		return CreateZB2Scene(pint, caps, &cb);
	}

	virtual	void	CommonUpdate(float dt)
	{
		VehicleInput::CommonUpdate(dt);

/*		Point CameraPos = mCameraTarget + Point(0.0f, 10.0f, 10.0f);
		Point Dir = mCameraTarget - CameraPos;
		Dir.Normalize();

		SetCamera(CameraPos, Dir);*/
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		const PintData* PD = reinterpret_cast<const PintData*>(pint.mUserData);

		if(PD && PD->mChassis)
		{
			const PR Pose = pint.GetWorldTransform(PD->mChassis);
			mCameraTarget = Pose.mPos;

			mCamera.mUpOffset = 0.5f * gScaleCoeff;
			mCamera.mDistToTarget = 8.0f * gScaleCoeff;
			mCamera.mSharpnessPos = 0.2f;
			mCamera.mSharpnessRot = 0.2f;

			UpdateCamera(pint, PD->mChassis);
		}

		if(PD && PD->mSteeringJoint)
		{
			PR Pose;
			Pose.Identity();

			const float Limit = gSteerLimit;
			if(mInput.mKeyboard_Right)
			{
				Pose.mPos.x = -Limit;
			}
			if(mInput.mKeyboard_Left)
			{
				Pose.mPos.x = Limit;
			}

			pint.SetDrivePosition(PD->mSteeringJoint, Pose);
		}

		if(PD)
		{
			float TargetSpeed = 0.0f;
			if(mInput.mKeyboard_Accelerate)
				TargetSpeed = gSpeed;
			else if(mInput.mKeyboard_Brake)
				TargetSpeed = -gSpeed;

			for(udword i=0;i<4;i++)
			{
				if(PD->mWheels[i])
				{
					// #### clumsy api
					float Speed;
					if(i!=1)
						Speed = TargetSpeed;
					else
						Speed = -TargetSpeed;

					if(TargetSpeed==0.0f)
					{
						pint.SetDriveEnabled(PD->mWheels[i], false);
					}
					else
					{
						pint.SetDriveEnabled(PD->mWheels[i], true);
						pint.SetDriveVelocity(PD->mWheels[i], Point(0.0f, 0.0f, 0.0f), Point(Speed, 0.0f, 0.0f));
					}
				}
			}
		}

		return VehicleInput::Update(pint, dt);
	}

END_TEST(LegoTechnicKart)

///////////////////////////////////////////////////////////////////////////////

// - tolerances
// - speed??

static const bool gOnlyTerrain = false;

static const char* gDesc_LegoTechnicBuggy = "Lego Technic Buggy 8048. Use cursor keys to drive, num keypad keys to control the camera.";

class LegoTechnicBuggy : public LegoTechnicVehicle
{
			EditBoxPtr		mEditBox_Gravity;
			EditBoxPtr		mEditBox_Scale;
			EditBoxPtr		mEditBox_Mass;
			EditBoxPtr		mEditBox_Stiffness;
			EditBoxPtr		mEditBox_Damping;
			EditBoxPtr		mEditBox_SteerStiffness;
			EditBoxPtr		mEditBox_SteerDamping;
			EditBoxPtr		mEditBox_SteerLimit;
			EditBoxPtr		mEditBox_MaxSpeed;
			CheckBoxPtr		mCheckBox_AddMotors;
			CheckBoxPtr		mCheckBox_AutoDrive;
			CheckBoxPtr		mCheckBox_UserSteer;
			CheckBoxPtr		mCheckBox_FreeCamera;

	public:
							LegoTechnicBuggy()			{ mLightRange = 1000.0f;			}
	virtual					~LegoTechnicBuggy()			{									}
	virtual	const char*		GetName()			const	{ return "LegoTechnicBuggy";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_LegoTechnicBuggy;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 500;
		WD.mLabel	= "Lego Technic Buggy";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 80;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 10;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Gravity:", &UIElems);
		mEditBox_Gravity = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "-981.0", &UIElems, EDITBOX_FLOAT, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Scale:", &UIElems);
		//mEditBox_Scale = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.35", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		// Real world scale is about 26*15 cm
		//mEditBox_Scale = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.02", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		mEditBox_Scale = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "2.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Mass:", &UIElems);
		//mEditBox_Mass = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		mEditBox_Mass = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.01", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Spring stiffness:", &UIElems);
		//mEditBox_Stiffness = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		mEditBox_Stiffness = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "200.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Spring damping:", &UIElems);
		//mEditBox_Damping = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "100.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		mEditBox_Damping = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "10.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Steer stiffness:", &UIElems);
		//mEditBox_SteerStiffness = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "2000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		mEditBox_SteerStiffness = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "200000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Steer damping:", &UIElems);
		mEditBox_SteerDamping = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "10000.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Steer limit:", &UIElems);
		//mEditBox_SteerLimit = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.75", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		mEditBox_SteerLimit = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Max speed:", &UIElems);
		//mEditBox_MaxSpeed = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "15.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		//mEditBox_MaxSpeed = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "50.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		mEditBox_MaxSpeed = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "30.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
		y += YStep;

		mCheckBox_AddMotors = helper.CreateCheckBox(UI, 0, 4, y, 100, 20, "Add motors", &UIElems, true, null, null);
		y += YStep;

		mCheckBox_AutoDrive = helper.CreateCheckBox(UI, 0, 4, y, 100, 20, "Auto-drive", &UIElems, false, null, null);
		y += YStep;

		mCheckBox_UserSteer = helper.CreateCheckBox(UI, 0, 4, y, 100, 20, "User-steer", &UIElems, true, null, null);
		y += YStep;

		mCheckBox_FreeCamera = helper.CreateCheckBox(UI, 0, 4, y, 100, 20, "Free camera", &UIElems, false, null, null);
		y += YStep;


		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

//		return CreateOverrideTabControl("Lego Technic Buggy", 20);
		{
			IceTabControl* TabControl;
			{
				TabControlDesc TCD;
				TCD.mParent	= UI;
				TCD.mX		= 4;
				TCD.mY		= y + 30;
				TCD.mWidth	= WD.mWidth - 16;
				TCD.mHeight	= 120 + 20;
				TabControl = ICE_NEW(IceTabControl)(TCD);
				RegisterUIElement(TabControl);
			}
			return TabControl;
		}
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		VehicleInput::GetSceneParams(desc);

		desc.mCamera[0] = PintCameraPose(Point(14.53f, 15.69f, 11.22f), Point(-0.64f, -0.62f, -0.45f));
		desc.mGravity.y = GetFloat(0.0f, mEditBox_Gravity);
		SetDefEnv(desc, false);
	}

	class MyCustomizeCallback : public ZB2CustomizeCallback
	{
		public:
						MyCustomizeCallback(float mass, float stiffness, float damping, float steer_stiffness, float steer_damping, bool user_steering) :
							mNbActors(0),
							mNbSprings(0),
							mDesiredMass(mass),
							mDesiredStiffness(stiffness),
							mDesiredDamping(damping),
							mDesiredSteerStiffness(steer_stiffness),
							mDesiredSteerDamping(steer_damping),
							mUserSteering(user_steering)
						{
						}
		virtual			~MyCustomizeCallback()
						{
							printf("%d actors found\n", mNbActors);
							printf("%d springs found\n", mNbSprings);
						}

		virtual	void	CustomizeMaterial(PINT_MATERIAL_CREATE&)
		{
		}

		virtual	void	CustomizeMesh(PINT_MESH_DATA_CREATE&)
		{
		}

		virtual	void	CustomizeShape(PINT_SHAPE_CREATE& create)
		{
			//static PINT_MATERIAL_CREATE m(1.0f, 1.0f, 0.0f);
			//create.mMaterial = &m;
		}

		virtual	void	CustomizeActor(PINT_OBJECT_CREATE& create)
		{
			//printf("Mass: %f\n", create.mMass);
			create.mMass = mDesiredMass;
			mNbActors++;
		}

		virtual	void	CustomizeJoint(PINT_JOINT_CREATE& create)
		{
			if(create.mType==PINT_JOINT_PRISMATIC)
			{
				PINT_PRISMATIC_JOINT_CREATE& PC = static_cast<PINT_PRISMATIC_JOINT_CREATE&>(create);
				if(PC.mSpring.mStiffness!=0.0f && PC.mSpring.mDamping!=0.0f)
				{
					//printf("Spring: %s\n", PC.mName);
					//printf("--Stiffness: %f\n", PC.mSpring.mStiffness);
					//printf("--Damping: %f\n", PC.mSpring.mDamping);
					mNbSprings++;
					if(strcmp(PC.mName, "Spring")==0)
					{
						//printf("CHECKPOINT %f %f\n", mDesiredStiffness, mDesiredDamping);
						PC.mSpring.mStiffness = mDesiredStiffness;
						PC.mSpring.mDamping = mDesiredDamping;
					}
				}
			}

			if(mUserSteering && create.mType==PINT_JOINT_D6)
			{
				PINT_D6_JOINT_CREATE& d6 = static_cast<PINT_D6_JOINT_CREATE&>(create);
				d6.mMotorFlags		= PINT_D6_MOTOR_DRIVE_X;
				d6.mMotorStiffness	= mDesiredSteerStiffness;
				d6.mMotorDamping	= mDesiredSteerDamping;
			}
		}

		udword	mNbActors;
		udword	mNbSprings;
		const float	mDesiredMass;
		const float	mDesiredStiffness;
		const float	mDesiredDamping;
		const float	mDesiredSteerStiffness;
		const float	mDesiredSteerDamping;
		const bool	mUserSteering;
	};

	virtual bool	CommonSetup()
	{
		const float Scale = GetFloat(1.0f, mEditBox_Scale);

		mPlanarMeshHelper.Generate(200, 0.5f * Scale, 0.5f, 10.0f * Scale);
		//mPlanarMeshHelper.Generate(200, 0.5f, 0.5f, 0.0f);
		if(gOnlyTerrain)
			return true;

		mCameraStyle = true;

		gZCB2_RenderScale = Scale;

		//const char* Filename = FindPEELFile("__Final5.zb2");
		//const char* Filename = FindPEELFile("LegoTechnic_Buggy.zb2");		// Regular
		//const char* Filename = FindPEELFile("LegoTechnic_Buggy2.zb2");	// Scaled down + automatic drive + terrain
		//const char* Filename = FindPEELFile("LegoTechnic_Buggy2b.zb2");		// + Blue scheme
		//const char* Filename = FindPEELFile("LegoTechnic_Buggy2c.zb2");	// Without render sources
		const char* Filename = FindPEELFile("LegoTechnic_Buggy3.zb2");	// Non scaled, default color scheme, no terrain, D6 prismatic rack
		if(Filename)
		{
			const float Mass = GetFloat(1.0f, mEditBox_Mass);
			const float Stiffness = GetFloat(1.0f, mEditBox_Stiffness);
			const float Damping = GetFloat(1.0f, mEditBox_Damping);
			const float SteerStiffness = GetFloat(1.0f, mEditBox_SteerStiffness);
			const float SteerDamping = GetFloat(1.0f, mEditBox_SteerDamping);
			const bool UserSteering = mCheckBox_UserSteer->IsChecked();
			MyCustomizeCallback CB(Mass, Stiffness, Damping, SteerStiffness, SteerDamping, UserSteering);
			PINT_WORLD_CREATE desc;
			ImportZB2File(desc, Filename, &CB);

			// Add motor to wheels
			if(mCheckBox_AddMotors->IsChecked())
			{
				const bool AutoDrive = mCheckBox_AutoDrive->IsChecked();
				const float Speed = GetFloat(0.0f, mEditBox_MaxSpeed);

				const udword NbJoints = mZB2Factory->mJoints.GetNbEntries();
				for(udword i=0;i<NbJoints;i++)
				{
					PINT_JOINT_CREATE* Create = reinterpret_cast<PINT_JOINT_CREATE*>(mZB2Factory->mJoints[i]);
					if(Create && Create->mName)
					{
						if(Create->mType==PINT_JOINT_HINGE2)
						{
							PINT_HINGE2_JOINT_CREATE* hjc = static_cast<PINT_HINGE2_JOINT_CREATE*>(Create);

							const bool IsFreeRearWheel = strcmp(Create->mName, "Free rear wheel hinge")==0;
							const bool IsConnectedRearWheel = strcmp(Create->mName, "Connected rear wheel hinge")==0;

							if(IsFreeRearWheel || IsConnectedRearWheel)
							{
								if(AutoDrive)
								{
									hjc->mUseMotor		= true;
									hjc->mDriveVelocity	= IsFreeRearWheel ? Speed : -Speed;							
								}
								else
								{
									hjc->mUseMotor		= true;
									hjc->mDriveVelocity	= IsFreeRearWheel ? Speed : -Speed;

									hjc->mDriveVelocity	= 0.0f;
								}
							}
						}
					}
				}
			}

			const bool SwitchToD6 = false;
			const bool SwitchToPrismatic = false;
			if(SwitchToD6 || SwitchToPrismatic)
			{
				// Surgery to switch the prismatic rack to a d6 rack
				const udword NbJoints = mZB2Factory->mJoints.GetNbEntries();
				for(udword i=0;i<NbJoints;i++)
				{
					PINT_JOINT_CREATE* Create = reinterpret_cast<PINT_JOINT_CREATE*>(mZB2Factory->mJoints[i]);
					if(Create && Create->mName && strcmp(Create->mName, "Rack prismatic")==0)
					{
						if(SwitchToD6 && Create->mType==PINT_JOINT_PRISMATIC)
						{
							// Switch from prismatic to d6
							const PINT_PRISMATIC_JOINT_CREATE* pjc = static_cast<const PINT_PRISMATIC_JOINT_CREATE*>(Create);

							PINT_D6_JOINT_CREATE* NewJoint = ICE_NEW(PINT_D6_JOINT_CREATE);
							NewJoint->mName		= Create->mName;
							NewJoint->mObject0	= Create->mObject0;
							NewJoint->mObject1	= Create->mObject1;

							NewJoint->mLocalPivot0	= pjc->mLocalPivot0;
							NewJoint->mLocalPivot1	= pjc->mLocalPivot1;

							//AABB	mLinearLimits;

							DELETESINGLE(Create);
							mZB2Factory->mJoints[i] = NewJoint;
						}
						else if(SwitchToPrismatic && Create->mType==PINT_JOINT_D6)
						{
							// Switch from d6 to prismatic
							const PINT_D6_JOINT_CREATE* d6jc = static_cast<const PINT_D6_JOINT_CREATE*>(Create);

							PINT_PRISMATIC_JOINT_CREATE* NewJoint = ICE_NEW(PINT_PRISMATIC_JOINT_CREATE);
							NewJoint->mName		= Create->mName;
							NewJoint->mObject0	= Create->mObject0;
							NewJoint->mObject1	= Create->mObject1;

							NewJoint->mLocalPivot0	= d6jc->mLocalPivot0;
							NewJoint->mLocalPivot1	= d6jc->mLocalPivot1;

							NewJoint->mLimits.mMinValue = d6jc->mLinearLimits.mMin.x;
							NewJoint->mLimits.mMaxValue = d6jc->mLinearLimits.mMax.x;

							DELETESINGLE(Create);
							mZB2Factory->mJoints[i] = NewJoint;
						}
					}
				}
			}
		}

		return VehicleInput::CommonSetup();
	}

	class MyCreationCallback : public ZB2CreationCallback
	{
		PintData&	mData;

		public:

		MyCreationCallback(PintData& data) : mData(data)
		{
		}

		virtual	void	NotifyCreatedObjects(Pint& pint, const ZB2CreatedObjects& objects, ZCB2Factory*)	override
		{
			{
				Pint_Actor* ActorAPI = pint.GetActorAPI();
				if(!ActorAPI)
					return;

				//printf("%d created actors\n", objects.mNbActors);

				AABB GlobalBounds;
				GlobalBounds.SetEmpty();
				for(udword i=0;i<objects.mNbActors;i++)
				{
					const PintActorHandle CurrentActor = objects.mActors[i].mHandle;

					AABB Bounds;
					ActorAPI->GetWorldBounds(CurrentActor, Bounds);
					GlobalBounds.Add(Bounds);

					const char* ActorName = ActorAPI->GetName(CurrentActor);
					if(ActorName && strcmp(ActorName, "Chassis")==0)
					{
						mData.mChassis = CurrentActor;
						//break;
					}
				}

				Point E;
				GlobalBounds.GetExtents(E);
				printf("Car bounds: %f %f %f\n", E.x, E.y, E.z);
			}

			{
				Pint_Joint* JointAPI = pint.GetJointAPI();
				if(!JointAPI)
					return;

				for(udword i=0;i<objects.mNbJoints;i++)
				{
					const PintJointHandle CurrentJoint = objects.mJoints[i];

					const PintJoint JT = JointAPI->GetType(CurrentJoint);

					const char* Name = JointAPI->GetName(CurrentJoint);
					if(Name)
					{
						// Cannot use the type, all joints are d6 if we select "use d6" in UI
						//if(JT==PINT_JOINT_D6)
						if(strcmp(Name, "Rack prismatic")==0)
						{
							mData.mSteeringJoint = CurrentJoint;
						}
						else //if(JT==PINT_JOINT_HINGE2)
						{
							const bool IsFreeRearWheel = strcmp(Name, "Free rear wheel hinge")==0;
							const bool IsConnectedRearWheel = strcmp(Name, "Connected rear wheel hinge")==0;
							if(IsFreeRearWheel)
								mData.mWheels[2] = CurrentJoint;
							if(IsConnectedRearWheel)
								mData.mWheels[3] = CurrentJoint;
						}
					}
				}

				// ### hardcoded
/*				if(gEnableFrontWheels)
				{
					mData.mWheels[0] = objects.mJoints[0];
					mData.mWheels[1] = objects.mJoints[1];
				}
				if(gEnableBackWheels)
				{
					mData.mWheels[2] = objects.mJoints[14];
					mData.mWheels[3] = objects.mJoints[15];
				}*/
			}
		}
	};

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		const float Altitude = 0.0f;
		//mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, &mHighFrictionMaterial);
		mPlanarMeshHelper.CreatePlanarMesh(pint, Altitude, null);


		if(0 && caps.mSupportConvexes)
		{
			ConvexIndex Index = CONVEX_INDEX_12;

			const udword Size = 256;

			const udword i = Index;
			const udword nb_x = Size;
			const udword nb_y = Size;
			{
				const float Altitude = 10.0f;

				MyConvex C;
				C.LoadFile(i);
C.Scale(2.0f);

				// 36 fps
//				PINT_CONVEX_CREATE Create(C.mNbVerts, C.mVerts);
//				Create.mRenderer	= CreateConvexRenderer(Create.mNbVerts, Create.mVerts);

				// 20 fps
				Container Indices;
				udword TotalNbTris = 0;
				for(int j=0;j<C.mNbPolys;j++)
				{
					const udword NbVerts = C.mPolys[j].mNbVerts;
					const udword NbTris = NbVerts - 2;
					TotalNbTris += NbTris;
					for(udword k=0;k<NbTris;k++)
					{
						const udword VRef0 = C.mPolys[j].mIndices[0];
						const udword VRef1 = C.mPolys[j].mIndices[(k+1)%NbVerts];
						const udword VRef2 = C.mPolys[j].mIndices[(k+2)%NbVerts];
						Indices.Add(VRef0).Add(VRef1).Add(VRef2);
					}
				}

				//PINT_MESH_CREATE Create;
				//Create.SetSurfaceData(C.mNbVerts, C.mVerts, TotalNbTris, Indices.GetEntries(), null);
				//Create.mRenderer	= CreateMeshRenderer(Create.GetSurface());

				Vertices MergedVerts;
				Container MergedTris;

				const float Amplitude = 1.5f;
				const udword NbX = nb_x;
				const udword NbY = nb_y;

				Matrix3x3 MX;
				MX.RotX(-HALFPI);
				const Quat Rot = MX;

				udword Offset = 0;
				//const float Scale = 10.0f;
				const float Scale = 2.5f;
				//const float Scale = 5.0f;
//const float Scale = 7.5f;
//const float Scale = 10.0f;
				for(udword y=0;y<NbY;y++)
				{
					for(udword x=0;x<NbX;x++)
					{
						const float xf = (float(x)-float(NbX)*0.5f)*Scale;
						const float yf = (float(y)-float(NbY)*0.5f)*Scale;

						const float RndX = UnitRandomFloat() * Scale * 0.5f;
						const float RndY = UnitRandomFloat() * Scale * 0.5f;

						const Point pos = Point(xf+RndX, Altitude + 300.0f, yf+RndY);

						PintRaycastHit Hit;
						PintRaycastData Data;
						Data.mOrigin = pos;
						Data.mDir = Point(0.0f, -1.0f, 0.0f);
						Data.mMaxDist = 5000.0f;
						if(pint.BatchRaycasts(pint.mSQHelper->GetThreadContext(), 1, &Hit, &Data))
						{
							Matrix3x3 MY;
							MY.RotY(UnitRandomFloat()*TWOPI);

							Matrix3x3 MZ;
							MZ.RotZ(UnitRandomFloat()*TWOPI);

							const Quat Rot2 = Rot * Quat(MZ) * Quat(MY);

							const Point Pos = Hit.mImpact - Point(0.0f, 0.1f, 0.0f);

							//PintActorHandle Handle = CreateStaticObject(pint, &Create, Pos, &Rot2);
							//ASSERT(Handle);

							Matrix4x4 M44 = Rot2;
							M44.SetTrans(Pos);

							for(int v=0;v<C.mNbVerts;v++)
								MergedVerts.AddVertex(C.mVerts[v] * M44);

							for(udword v=0;v<TotalNbTris*3;v++)
								MergedTris.Add(Offset+Indices[v]);

							Offset += C.mNbVerts;
						}
					}
				}

				// ~500 fps
				PINT_MESH_CREATE Create;
				Create.SetSurfaceData(MergedVerts.GetNbVertices(), MergedVerts.GetVertices(), MergedTris.GetNbEntries()/3, MergedTris.GetEntries(), null);
				Create.mRenderer	= CreateMeshRenderer(Create.GetSurface());
				PintActorHandle Handle = CreateStaticObject(pint, &Create, Point(0.0f, 0.0f, 0.0f));
				ASSERT(Handle);
			}
		}

		if(gOnlyTerrain)
			return true;

		if(0)
		{
			ConvexIndex Index = CONVEX_INDEX_13;
//			if(mComboBox_ConvexIndex)
//				Index = ConvexIndex(mComboBox_ConvexIndex->GetSelectedIndex());

			//const udword Size = GetInt(32, mEditBox_Size);
			const udword Size = 32;

			const udword i = Index;
			const udword nb_x = Size;
			const udword nb_y = Size;
			{
				const float Altitude = -0.8f;

				MyConvex C;
				C.LoadFile(i);

				PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
				ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

				const float Amplitude = 1.5f;
				const udword NbX = nb_x;
				const udword NbY = nb_y;

				const float Scale = 2.50f;
				for(udword y=0;y<NbY;y++)
				{
					for(udword x=0;x<NbX;x++)
					{
						const float xf = (float(x)-float(NbX)*0.5f)*Scale;
						const float yf = (float(y)-float(NbY)*0.5f)*Scale;

						const Point pos = Point(xf, Altitude, yf);

						PintActorHandle Handle = CreateStaticObject(pint, &ConvexCreate, pos);
						ASSERT(Handle);
					}
				}
			}
		}

		gZCB2_RenderScale = 1.0f;

		PintData* PD = ICE_NEW(PintData);
		pint.mUserData = PD;

		MyCreationCallback cb(*PD);
		return CreateZB2Scene(pint, caps, &cb);
	}

/*	virtual	void	CommonUpdate(float dt)
	{
		VehicleInput::CommonUpdate(dt);
	}*/

	virtual udword	Update(Pint& pint, float dt)
	{
		const PintData* PD = reinterpret_cast<const PintData*>(pint.mUserData);

		const bool FreeCamera = mCheckBox_FreeCamera->IsChecked();

		if(PD && PD->mChassis && !FreeCamera)
		{
			const PR Pose = pint.GetWorldTransform(PD->mChassis);
			mCameraTarget = Pose.mPos;

			const float Scale = GetFloat(1.0f, mEditBox_Scale);

			mCamera.mUpOffset = 0.5f * Scale;
			mCamera.mDistToTarget = 40.0f * Scale;
			mCamera.mSharpnessPos = 0.2f;
			mCamera.mSharpnessRot = 0.2f;
			mCamera.mSharpnessPos = 1000.0f;
			mCamera.mSharpnessRot = 1000.0f;

			mExtraDistanceScale = Scale;

			UpdateCamera(pint, PD->mChassis);
		}

		const bool UserSteering = mCheckBox_UserSteer->IsChecked();

		if(PD && PD->mSteeringJoint && UserSteering)
		{
			PR Pose;
			Pose.Identity();

			const float Limit = GetFloat(1.0f, mEditBox_SteerLimit);
			if(mInput.mKeyboard_Right)
				Pose.mPos.x = -Limit;
			else if(mInput.mKeyboard_Left)
				Pose.mPos.x = Limit;
			else
				Pose.mPos.x = 0.0f;

			pint.SetDrivePosition(PD->mSteeringJoint, Pose);
		}

		if(PD && mCheckBox_AddMotors->IsChecked() && !mCheckBox_AutoDrive->IsChecked())
		{
			float TargetSpeed = 0.0f;
			const float Speed = GetFloat(0.0f, mEditBox_MaxSpeed);
			if(mInput.mKeyboard_Accelerate)
				TargetSpeed = Speed;
			else if(mInput.mKeyboard_Brake)
				TargetSpeed = -Speed;

			static float Memory = 0.0f;
			//FeedbackFilter(TargetSpeed, Memory, 0.01f);
			FeedbackFilter(TargetSpeed, Memory, 0.005f);

			for(udword i=0;i<4;i++)
			{
				if(PD->mWheels[i])
				{
					// #### clumsy api
					const bool IsFreeRearWheel = i==2;
					const float Sign = IsFreeRearWheel ? 1.0f : -1.0f;
					const float Speed = Memory * Sign;

					if(TargetSpeed==0.0f)
					{
						pint.SetDriveEnabled(PD->mWheels[i], false);
					}
					else
					{
						pint.SetDriveEnabled(PD->mWheels[i], true);
						pint.SetDriveVelocity(PD->mWheels[i], Point(0.0f, 0.0f, 0.0f), Point(Speed, 0.0f, 0.0f));
					}
				}
			}
		}

		return VehicleInput::Update(pint, dt);
	}

	virtual	bool	SpecialKeyCallback(int key, int x, int y, bool down)
	{
		if(mCheckBox_FreeCamera->IsChecked())
			return TestBase::SpecialKeyCallback(key, x, y, down);
		else
			return LegoTechnicVehicle::SpecialKeyCallback(key, x, y, down);
	}

	virtual	bool	KeyboardCallback(unsigned char key, int x, int y, bool down)
	{
		if(mCheckBox_FreeCamera->IsChecked())
			return TestBase::KeyboardCallback(key, x, y, down);
		else
			return LegoTechnicVehicle::KeyboardCallback(key, x, y, down);
	}

END_TEST(LegoTechnicBuggy)

///////////////////////////////////////////////////////////////////////////////
