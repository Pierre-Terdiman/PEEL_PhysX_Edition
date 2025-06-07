///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Camera.h"
#include "Cylinder.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "Loader_Bin.h"
#include "ProceduralTrack.h"
#include "MyConvex.h"
#include "GUI_Helpers.h"
#include "GLFontRenderer.h"
#include "MuJoCoConverter.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_RCA_Drives = "RCA drives. A test to explore the drive/motor/actuator behavior of different engines. Beware: linear damping in main UI may affect angular velocities here.";

// Velocity drive:
// - a damping value of 0.2 in MuJoCo gives the exact same angular velocity as a linear damping of 0.1 and angular damping of 0.0 in PhysX (for all actuators damping values).

class RCADrives : public TestBase
{
	class LocalTestData : public TestData
	{
		public:
			LocalTestData() : mDynamicObject(null)	{}
			PintActorHandle	mDynamicObject;
	};

			CheckBoxPtr		mCheckBox_EnableMotor;
			CheckBoxPtr		mCheckBox_EnableUpdate;
			CheckBoxPtr		mCheckBox_Velocity;
			CheckBoxPtr		mCheckBox_Position;
			EditBoxPtr		mEditBox_TargetVel;
			EditBoxPtr		mEditBox_TargetPos;
			EditBoxPtr		mEditBox_Stiffness;
			EditBoxPtr		mEditBox_Damping;
			ComboBoxPtr		mComboBox_Preset;
			SliderPtr		mSlider;

	public:
							RCADrives()					{									}
	virtual					~RCADrives()				{									}
	virtual	const char*		GetName()			const	{ return "RCADrives";				}
	virtual	const char*		GetDescription()	const	{ return gDesc_RCA_Drives;			}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_RCARTICULATIONS;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		const sdword Width = 220;
		IceWindow* UI = CreateTestWindow(Width, 320);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 60;
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

			mCheckBox_Velocity = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Velocity drive", &UIElems, true, null, null);
			y += YStep;

			mCheckBox_Position = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Position drive", &UIElems, true, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Target vel.:", &UIElems);
			mEditBox_TargetVel = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			mEditBox_TargetVel->SetEnabled(InitialEnabled);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Target pos.:", &UIElems);
			mEditBox_TargetPos = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			mEditBox_TargetPos->SetEnabled(InitialEnabled);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Stiffness:", &UIElems);
			mEditBox_Stiffness = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			mEditBox_Stiffness->SetEnabled(InitialEnabled);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Damping:", &UIElems);
			mEditBox_Damping = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.0", &UIElems, EDITBOX_FLOAT, null, null);
			mEditBox_Damping->SetEnabled(InitialEnabled);
			y += YStep;

			{
				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Preset:", &UIElems);

				class MyComboBox : public IceComboBox
				{
					RCADrives&		mTest;
					public:
									MyComboBox(const ComboBoxDesc& desc, RCADrives& test) : IceComboBox(desc), mTest(test)	{}
					virtual			~MyComboBox()																			{}

					virtual	void	OnComboBoxEvent(ComboBoxEvent event)
					{
						if(event==CBE_SELECTION_CHANGED)
						{
							const udword SelectedIndex = GetSelectedIndex();
							if(SelectedIndex == 1)
							{
								mTest.mCheckBox_Velocity->SetChecked(true);
								mTest.mCheckBox_Position->SetChecked(false);
								mTest.mEditBox_TargetVel->SetLabel("4.0");
								mTest.mEditBox_TargetPos->SetLabel("0.0");
								mTest.mEditBox_Stiffness->SetLabel("0.0");
								mTest.mEditBox_Damping->SetLabel("10.0");
							}
							else if(SelectedIndex == 2)
							{
								mTest.mCheckBox_Velocity->SetChecked(false);
								mTest.mCheckBox_Position->SetChecked(true);
								mTest.mEditBox_TargetVel->SetLabel("0.0");
								mTest.mEditBox_TargetPos->SetLabel("1.0");
								mTest.mEditBox_Stiffness->SetLabel("1000.0");
								mTest.mEditBox_Damping->SetLabel("100.0");
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
				CBBD.mWidth		= 120;
				CBBD.mHeight	= 20;
				CBBD.mLabel		= "Presets";
				mComboBox_Preset = ICE_NEW(MyComboBox)(CBBD, *this);
				RegisterUIElement(mComboBox_Preset);
				mComboBox_Preset->Add("User-defined");
				mComboBox_Preset->Add("Velocity drive");
				mComboBox_Preset->Add("Position drive");
				mComboBox_Preset->Select(0);
				mComboBox_Preset->SetVisible(true);
				y += YStep;
			}

		}

		{
			y += YStep;
			SliderDesc SD;
			SD.mStyle	= SLIDER_HORIZONTAL;
			SD.mID		= 0;
			SD.mParent	= UI;
			SD.mX		= 4;
			SD.mY		= y;
			SD.mWidth	= Width - 30;
			SD.mHeight	= 20;
			SD.mLabel	= "RCADriveSlider";
			mSlider		= ICE_NEW(IceSlider)(SD);
			mSlider->SetRange(0.0f, 1.0f, 100);
			mSlider->SetValue(1.0f);
			UIElems.Register(mSlider);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, Width);

		return null;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.93f, 20.16f, 5.70f), Point(-0.65f, -0.09f, -0.75f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)	override
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportRCArticulations)
			return false;

		const Point static_pos(0.0f, 20.0f, 0.0f);
		const Point local_axis(0.0f, 0.0f, 1.0f);

		const float BoxSize = 1.0f;
		const Point Extents(BoxSize, BoxSize, BoxSize);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateRenderer(BoxDesc);

		const Point Disp(BoxSize*2.0f, -BoxSize*2.0f, 0.0f);
//		const Point Disp(0.0f, 0.0f, 0.0f);
		const Point DynamicPos = static_pos + Disp;

		PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(true));
		if(!RCA)
			return false;

		PintActorHandle StaticObject;
		{
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= RCA ? 1.0f : 0.0f;
			ObjectDesc.mPosition		= static_pos;
//			ObjectDesc.mCollisionGroup	= 1;

			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mFrictionCoeff = 0.0f;
			StaticObject = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
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
			const float TargetVel = GetFloat(0.0f, mEditBox_TargetVel);
			const float TargetPos = GetFloat(0.0f, mEditBox_TargetPos);
			const float Stiffness = GetFloat(0.0f, mEditBox_Stiffness);
			const float Damping = GetFloat(0.0f, mEditBox_Damping);

			{
				PINT_RC_ARTICULATED_BODY_CREATE Desc;

				Desc.mJointType			= PINT_JOINT_HINGE;
				Desc.mParent			= StaticObject;
				Desc.mAxisIndex			= Z_;
				Desc.mLocalPivot0.mPos	= Pivot0;
				Desc.mLocalPivot1.mPos	= Pivot1;
				Desc.mFrictionCoeff		= 0.0f;

				if(0)
				{
					Desc.mMinLimit	= -PI/4.0f;
					Desc.mMaxLimit	= PI/4.0f;
				}

				if(EnableMotor)
				{
					udword Flags = PINT_MOTOR_NONE;
					if(mCheckBox_Velocity && mCheckBox_Velocity->IsChecked())
						Flags |= PINT_MOTOR_VELOCITY;
					if(mCheckBox_Position && mCheckBox_Position->IsChecked())
						Flags |= PINT_MOTOR_POSITION;

					Desc.mMotorFlags		= PintMotorFlags(Flags);
					Desc.mMotor.mStiffness	= Stiffness;
					Desc.mMotor.mDamping	= Damping;
					Desc.mTargetVel			= TargetVel;
					Desc.mTargetPos			= TargetPos;
				}

				DynamicObject = pint.CreateRCArticulatedObject(ObjectDesc, Desc, RCA);

				LocalTestData* LTD = ICE_NEW(LocalTestData);
				RegisterTestData(LTD);
				LTD->mDynamicObject = DynamicObject;
				pint.mUserData		= LTD;
			}
		}

		pint.AddRCArticulationToScene(RCA);

		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)	override
	{
		const bool EnableUpdate = mCheckBox_EnableUpdate ? mCheckBox_EnableUpdate->IsChecked() : true;
		if(EnableUpdate)
		{
			const bool EnableMotor = mCheckBox_EnableMotor ? mCheckBox_EnableMotor->IsChecked() : true;
			const LocalTestData* LTD = reinterpret_cast<const LocalTestData*>(pint.mUserData);
			if(LTD)
			{
				const float Coeff = mSlider ? mSlider->GetValue() : 1.0f;
				const float TargetVel = Coeff * GetFloat(0.0f, mEditBox_TargetVel);
				const float TargetPos = Coeff * GetFloat(0.0f, mEditBox_TargetPos);
	//			printf("TargetVel: %f\n", TargetVel);
				pint.SetRCADriveEnabled(LTD->mDynamicObject, EnableMotor);
				if(EnableMotor)
				{
					pint.SetRCADriveVelocity(LTD->mDynamicObject, TargetVel);
					pint.SetRCADrivePosition(LTD->mDynamicObject, TargetPos);
				}
			}
		}
		return 0;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)	override
	{
		const LocalTestData* LTD = reinterpret_cast<const LocalTestData*>(pint.mUserData);
		if(LTD && LTD->mDynamicObject)
		{
			const Point AngVel = pint.GetAngularVelocity(LTD->mDynamicObject);
	//		renderer.print(0.0f, y, text_scale, _F("Angular velocity: %.3f | %.3f | %.3f\n", Float(AngVel.x), Float(AngVel.y), Float(AngVel.z)));
			renderer.print(0.0f, y, text_scale, _F("Angular velocity: %f\n", AngVel.z));
			//renderer.print(0.0f, y, text_scale, _F("Angular velocity: %f %f %f\n", AngVel.x, AngVel.y, AngVel.z));
			y -= text_scale;
	//		return PrintTwistAngle(pint, renderer, y, text_scale);
		}
		return y;
	}

END_TEST(RCADrives)

///////////////////////////////////////////////////////////////////////////////

/*static Point Transform(Pint& pint, PintActorHandle actor, const Point& p)
{
	//actor->getGlobalPose().transform(p)
	// TODO: optimize / simplify this
	const PR pr = pint.GetWorldTransform(actor);
	Matrix4x4 M = pr;
	return p * M;
}*/

static Point TransformInv(Pint& pint, PintActorHandle actor, const Point& p)
{
	//actor->getGlobalPose().transformInv(p)
	// TODO: optimize / simplify this
	const PR pr = pint.GetWorldTransform(actor);
	Matrix4x4 M = pr;
	Matrix4x4 InvM;
	InvertPRMatrix(InvM, M);
	return p * InvM;
}

static Quat GetConjugateQ(Pint& pint, PintActorHandle actor)
{
	// TODO: optimize / simplify this
	const PR pr = pint.GetWorldTransform(actor);
	return pr.mRot.GetConjugate();
}

static const char* gDesc_ScissorLift = "Scissor lift. Adapted from the PhysX reduced coordinates articulation snippet. This test will use regular joints if RC articulations are not supported.";

START_TEST(ScissorLift, CATEGORY_RCARTICULATIONS, gDesc_ScissorLift)

	CheckBoxPtr		mCheckBox_RCA;
	CheckBoxPtr		mCheckBox_Wheels;
	EditBoxPtr		mEditBox_LinkHeight;

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		const sdword Width = 300;
		IceWindow* UI = CreateTestWindow(Width, 400);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 120;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			mCheckBox_RCA = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use articulation", &UIElems, true, null, null);
			y += YStep;

			mCheckBox_Wheels = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Add wheels", &UIElems, false, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Link height:", &UIElems);
			mEditBox_LinkHeight = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "3", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, Width);

		return null;
	}

	virtual	float	GetRenderData(Point& center)	const	override	{ return 20.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(3.96f, 2.64f, 3.98f), Point(-0.69f, -0.32f, -0.65f));
		SetDefEnv(desc, true);
	}

	static void CreateLoopJoint(Pint& pint, PintActorHandle object0, PintActorHandle object1, const Point& local_pivot0, const Point& local_pivot1)
	{
		const PINT_SPHERICAL_JOINT_CREATE Desc(object0, object1, local_pivot0, local_pivot1);
		const PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
	}

	static PintActorHandle CreateFixedBodyPart(Pint& pint, PintArticHandle rca, const PINT_OBJECT_CREATE& create, PintActorHandle parent, const Point& local_pivot0, const Point& local_pivot1)
	{
		PintActorHandle h;
		if(rca)
		{
			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mJointType			= PINT_JOINT_FIXED;
			ArticulatedDesc.mParent				= parent;
			ArticulatedDesc.mLocalPivot0.mPos	= local_pivot0;
			ArticulatedDesc.mLocalPivot1.mPos	= local_pivot1;
			h = pint.CreateRCArticulatedObject(create, ArticulatedDesc, rca);
		}
		else
		{
			h = CreatePintObject(pint, create);

			PINT_FIXED_JOINT_CREATE Desc;
			Desc.mObject0		= parent;
			Desc.mObject1		= h;
			Desc.mLocalPivot0	= local_pivot0;
			Desc.mLocalPivot1	= local_pivot1;
			const PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return h;
	}

	static PintActorHandle CreateHingedBodyPart(Pint& pint, PintArticHandle rca, const PINT_OBJECT_CREATE& create, PintActorHandle parent, const PR& local_pivot0, const PR& local_pivot1, bool setup_limits, float min_limit, float max_limit)
	{
		PintActorHandle h;
		if(rca)
		{
			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
			ArticulatedDesc.mAxisIndex			= X_;
			ArticulatedDesc.mParent				= parent;
			ArticulatedDesc.mLocalPivot0		= local_pivot0;
			ArticulatedDesc.mLocalPivot1		= local_pivot1;

			if(setup_limits)
			{
				ArticulatedDesc.mMinLimit	= min_limit;
				ArticulatedDesc.mMaxLimit	= max_limit;
			}
			else
			{
				ArticulatedDesc.mMinLimit	= -PI;
				ArticulatedDesc.mMaxLimit	= PI;
			}

			h = pint.CreateRCArticulatedObject(create, ArticulatedDesc, rca);
		}
		else
		{
			h = CreatePintObject(pint, create);

			PINT_HINGE2_JOINT_CREATE Desc;
			Desc.mObject0		= parent;
			Desc.mObject1		= h;
			Desc.mLocalPivot0	= local_pivot0;
			Desc.mLocalPivot1	= local_pivot1;

			if(setup_limits)
			{
				Desc.mLimits.mMinValue	= min_limit;
				Desc.mLimits.mMaxValue	= max_limit;
			}
			else
			{
				Desc.mLimits.mMinValue	= -PI;
				Desc.mLimits.mMaxValue	= PI;
			}

			const PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return h;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)	override
	{
		if(!caps.mSupportCollisionGroups || !caps.mSupportRigidBodySimulation)
			return false;

		// Group 0 is already used by the default static environment so we start with group 1
		const PintCollisionGroup Layer0_Group = 1;
		const PintDisabledGroups DG(Layer0_Group, Layer0_Group);
		pint.SetDisabledGroups(1, &DG);

		PintArticHandle RCA = null;
		if(caps.mSupportRCArticulations && mCheckBox_RCA && mCheckBox_RCA->IsChecked())
			RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE());

		const bool AddWheels = mCheckBox_Wheels && mCheckBox_Wheels->IsChecked();

		const bool SetupLimits = true;

		const float runnerLength = 2.0f;
		const float placementDistance = 1.8f;

		const float cosAng = (placementDistance) / (runnerLength);

		const float angle = SafeACos(cosAng);

		const float sinAng = sinf(angle);

		const Quat leftRot(AngleAxis(-angle, Point(1.0f, 0.0f, 0.0f)));
		const Quat rightRot(AngleAxis(angle, Point(1.0f, 0.0f, 0.0f)));

		//const Point WheelOffset(0.0f, AddWheels ? 0.1f : 0.0f, 0.0f);
		const Point BasePos(0.0f, 0.25f, 0.0f);

		//(1) Create base...
//		PxArticulationLink* base = gArticulation->createLink(NULL, PxTransform(PxVec3(0.0f, 0.25f, 0.0f)));
//		PxRigidActorExt::createExclusiveShape(*base, PxBoxGeometry(0.5f, 0.25f, 1.5f), *gMaterial);
//		PxRigidBodyExt::updateMassAndInertia(*base, 3.0f);
		PintActorHandle base;
		{
			const Point Extents(0.5f, 0.25f, 1.5f);
//			const Point Extents(0.25f, 0.25f, 1.5f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 3.0f;
			ObjectDesc.mPosition	= BasePos;
ObjectDesc.mCollisionGroup	= Layer0_Group;

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				base = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
			{
				base = CreatePintObject(pint, ObjectDesc);
			}
		}

		if(AddWheels)
		{
			const float WheelRadius = 0.2f;
			PintShapeRenderer* Renderer = CreateSphereRenderer(WheelRadius);

			for(udword i=0;i<4;i++)
			{
				Point Delta;
				Delta.x = i&1 ? -0.5f : 0.5f;
				Delta.y = -0.25f;
				Delta.z = i&2 ? -1.5f : 1.5f;

				PINT_SPHERE_CREATE SphereDesc(WheelRadius);
				SphereDesc.mRenderer	= Renderer;

				PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mPosition	= BasePos + Delta;
ObjectDesc.mCollisionGroup	= Layer0_Group;

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mJointType			= PINT_JOINT_SPHERICAL;
					ArticulatedDesc.mParent				= base;
					ArticulatedDesc.mLocalPivot0.mPos	= Delta;
					ArticulatedDesc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
	//				ArticulatedDesc.mAxisIndex			= X_;
					ArticulatedDesc.mFrictionCoeff		= 0.0f;
	/*
					ArticulatedDesc.mMinTwistLimit		(1.0f),
					ArticulatedDesc.mMaxTwistLimit		(-1.0f),
					ArticulatedDesc.mMinSwing1Limit		(1.0f),
					ArticulatedDesc.mMaxSwing1Limit		(-1.0f),
					ArticulatedDesc.mMinSwing2Limit		(1.0f),
					ArticulatedDesc.mMaxSwing2Limit		(-1.0f),
					ArticulatedDesc.mFrictionCoeff		(0.5f),
	*/
					const PintActorHandle Wheel = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
				{
					const PintActorHandle Wheel = CreatePintObject(pint, ObjectDesc);

					PINT_SPHERICAL_JOINT_CREATE Desc;
					Desc.mObject0			= base;
					Desc.mLocalPivot0.mPos	= Delta;
					Desc.mObject1			= Wheel;
					Desc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, 0.0f);
					const PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}

		//Now create the slider and fixed joints...

//		PxArticulationLink* leftRoot = gArticulation->createLink(base, PxTransform(PxVec3(0.0f, 0.55f, -0.9f)));
//		PxRigidActorExt::createExclusiveShape(*leftRoot, PxBoxGeometry(0.5f, 0.05f, 0.05f), *gMaterial);
//		PxRigidBodyExt::updateMassAndInertia(*leftRoot, 1.0f);
//
//		PxArticulationJointReducedCoordinate* joint = static_cast<PxArticulationJointReducedCoordinate*>(leftRoot->getInboundJoint());
//		joint->setJointType(PxArticulationJointType::eFIX);
//		joint->setParentPose(PxTransform(PxVec3(0.0f, 0.25f, -0.9f)));
//		joint->setChildPose(PxTransform(PxVec3(0.0f, -0.05f, 0.0f)));

		PintActorHandle leftRoot;
		{
			const Point Extents(0.5f, 0.05f, 0.05f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.0f, 0.55f, -0.9f);
ObjectDesc.mCollisionGroup	= Layer0_Group;

			leftRoot = CreateFixedBodyPart(pint, RCA, ObjectDesc, base, Point(0.0f, 0.25f, -0.9f), Point(0.0f, -0.05f, 0.0f));
		}

//		PxArticulationLink* rightRoot = gArticulation->createLink(base, PxTransform(PxVec3(0.0f, 0.55f, 0.9f)));
//		PxRigidActorExt::createExclusiveShape(*rightRoot, PxBoxGeometry(0.5f, 0.05f, 0.05f), *gMaterial);
//		PxRigidBodyExt::updateMassAndInertia(*rightRoot, 1.0f);
		//Set up the drive joint...	
//		gDriveJoint = static_cast<PxArticulationJointReducedCoordinate*>(rightRoot->getInboundJoint());
//		gDriveJoint->setJointType(PxArticulationJointType::ePRISMATIC);
//		gDriveJoint->setMotion(PxArticulationAxis::eZ, PxArticulationMotion::eLIMITED);
//		gDriveJoint->setLimit(PxArticulationAxis::eZ, -1.4f, 0.2f);
//		gDriveJoint->setDrive(PxArticulationAxis::eZ, 100000.0f, 0.0f, PX_MAX_F32);

//		gDriveJoint->setParentPose(PxTransform(PxVec3(0.0f, 0.25f, 0.9f)));
//		gDriveJoint->setChildPose(PxTransform(PxVec3(0.0f, -0.05f, 0.0f)));

		PintActorHandle rightRoot;
		{
			const Point Extents(0.5f, 0.05f, 0.05f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.0f, 0.55f, 0.9f);
ObjectDesc.mCollisionGroup	= Layer0_Group;

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType			= PINT_JOINT_PRISMATIC;
				ArticulatedDesc.mAxisIndex			= Z_;
				ArticulatedDesc.mMinLimit			= -1.4f;
				ArticulatedDesc.mMaxLimit			= 0.2f;
				ArticulatedDesc.mParent				= base;
				ArticulatedDesc.mLocalPivot0.mPos	= Point(0.0f, 0.25f, 0.9f);
				ArticulatedDesc.mLocalPivot1.mPos	= Point(0.0f, -0.05f, 0.0f);
				rightRoot = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
			{
				rightRoot = CreatePintObject(pint, ObjectDesc);

				PINT_PRISMATIC_JOINT_CREATE Desc;
				Desc.mObject0			= base;
				Desc.mLocalPivot0.mPos	= Point(0.0f, 0.25f, 0.9f);
				Desc.mObject1			= rightRoot;
				Desc.mLocalPivot1.mPos	= Point(0.0f, -0.05f, 0.0f);
				Desc.mLocalAxis0		= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1		= Point(0.0f, 0.0f, 1.0f);
				Desc.mLimits.mMinValue	= -1.4f;
				Desc.mLimits.mMaxValue	= 0.2f;
				const PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}

		//

		const udword linkHeight = GetInt(3, mEditBox_LinkHeight);

//		PxArticulationLink* currLeft = leftRoot, *currRight = rightRoot;
		PintActorHandle currLeft = leftRoot;
		PintActorHandle currRight = rightRoot;
		Quat rightParentRot(Idt);
		Quat leftParentRot(Idt);

//		PR LastLeftLinkPose(Idt);
//		PR LastRightLinkPose(Idt);

		for(udword i=0; i<linkHeight; ++i)
		{
			const Point pos(0.5f, 0.55f + 0.1f*(1 + i), 0.0f);

//			PxArticulationLink* leftLink = gArticulation->createLink(currLeft, PxTransform(pos + PxVec3(0.0f, sinAng*(2 * i + 1), 0.0f), leftRot));
//			PxRigidActorExt::createExclusiveShape(*leftLink, PxBoxGeometry(0.05f, 0.05f, 1.0f), *gMaterial);
//			PxRigidBodyExt::updateMassAndInertia(*leftLink, 1.0f);

			const Point leftAnchorLocation = pos + Point(0.0f, sinAng*(2 * i), -0.9f);

//			joint = static_cast<PxArticulationJointReducedCoordinate*>(leftLink->getInboundJoint());
//			joint->setParentPose(PxTransform(currLeft->getGlobalPose().transformInv(leftAnchorLocation), leftParentRot));
//			joint->setChildPose(PxTransform(PxVec3(0.0f, 0.0f, -1.0f), rightRot));
//			joint->setJointType(PxArticulationJointType::eREVOLUTE);
//			joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eLIMITED);
//			joint->setLimit(PxArticulationAxis::eTWIST, -PxPi, angle);

			PintActorHandle leftLink;
			{
				const Point Extents(0.05f, 0.05f, 1.0f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= pos + Point(0.0f, sinAng*(2 * i + 1), 0.0f);
				ObjectDesc.mRotation		= leftRot;
//LastLeftLinkPose = PR(ObjectDesc.mPosition, ObjectDesc.mRotation);

				ObjectDesc.mCollisionGroup	= Layer0_Group;

				leftLink = CreateHingedBodyPart(pint, RCA, ObjectDesc, currLeft,
					PR(TransformInv(pint, currLeft, leftAnchorLocation), leftParentRot),
					PR(Point(0.0f, 0.0f, -1.0f), rightRot),
					SetupLimits, -PI, angle);
			}

			leftParentRot = leftRot;

//			PxArticulationLink* rightLink = gArticulation->createLink(currRight, PxTransform(pos + PxVec3(0.0f, sinAng*(2 * i + 1), 0.0f), rightRot));
//			PxRigidActorExt::createExclusiveShape(*rightLink, PxBoxGeometry(0.05f, 0.05f, 1.0f), *gMaterial);
//			PxRigidBodyExt::updateMassAndInertia(*rightLink, 1.0f);

			const Point rightAnchorLocation = pos + Point(0.0f, sinAng*(2 * i), 0.9f);

//			joint = static_cast<PxArticulationJointReducedCoordinate*>(rightLink->getInboundJoint());
//			joint->setJointType(PxArticulationJointType::eREVOLUTE);
//			joint->setParentPose(PxTransform(currRight->getGlobalPose().transformInv(rightAnchorLocation), rightParentRot));
//			joint->setChildPose(PxTransform(PxVec3(0.0f, 0.0f, 1.0f), leftRot));
//			joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eLIMITED);
//			joint->setLimit(PxArticulationAxis::eTWIST, -angle, PxPi);

			PintActorHandle rightLink;
			{
				const Point Extents(0.05f, 0.05f, 1.0f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= pos + Point(0.0f, sinAng*(2 * i + 1), 0.0f);
				ObjectDesc.mRotation		= rightRot;
//LastRightLinkPose = PR(ObjectDesc.mPosition, ObjectDesc.mRotation);

				ObjectDesc.mCollisionGroup	= Layer0_Group;

				rightLink = CreateHingedBodyPart(pint, RCA, ObjectDesc, currRight,
					PR(TransformInv(pint, currRight, rightAnchorLocation), rightParentRot),
					PR(Point(0.0f, 0.0f, 1.0f), leftRot),
					SetupLimits, -angle, PI);
			}

			rightParentRot = rightRot;

			CreateLoopJoint(pint, leftLink, rightLink, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f));

			currLeft = rightLink;
			currRight = leftLink;
		}
	
#ifdef TOSEE
		PxArticulationLink* leftTop = gArticulation->createLink(currLeft, currLeft->getGlobalPose().transform(PxTransform(PxVec3(-0.5f, 0.0f, -1.0f), leftParentRot)));
		PxRigidActorExt::createExclusiveShape(*leftTop, PxBoxGeometry(0.5f, 0.05f, 0.05f), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*leftTop, 1.0f);

		joint = static_cast<PxArticulationJointReducedCoordinate*>(leftTop->getInboundJoint());
		joint->setParentPose(PxTransform(PxVec3(0.0f, 0.0f, -1.0f), currLeft->getGlobalPose().q.getConjugate()));
		joint->setChildPose(PxTransform(PxVec3(0.5f, 0.0f, 0.0f), leftTop->getGlobalPose().q.getConjugate()));
		joint->setJointType(PxArticulationJointType::eREVOLUTE);
		joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eFREE);
		//joint->setDrive(PxArticulationAxis::eTWIST, 0.0f, 10.0f, PX_MAX_F32);
#endif

		PintActorHandle leftTop;
		{
			const Point Extents(0.5f, 0.05f, 0.05f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

			// ### this pose is wrong for MuJoCo because you cannot call GetWorldTransform() before the model has been compiled :(
			PR tmp0 = pint.GetWorldTransform(currLeft);
//tmp0 = LastRightLinkPose;
			PR tmp1(Point(-0.5f, 0.0f, -1.0f), leftParentRot);
			Matrix4x4 M0 = tmp0;
			Matrix4x4 M1 = tmp1;
			Matrix4x4 M = M1 * M0;
			PR combo = M;

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
//			ObjectDesc.mPosition		= Transform(pint, currLeft, Point(-0.5f, 0.0f, -1.0f));
//			ObjectDesc.mRotation		= leftParentRot;
			ObjectDesc.mPosition		= combo.mPos;
			ObjectDesc.mRotation		= combo.mRot;
			ObjectDesc.mCollisionGroup	= Layer0_Group;

			leftTop = CreateHingedBodyPart(pint, RCA, ObjectDesc, currLeft,
				PR(Point(0.0f, 0.0f, -1.0f), GetConjugateQ(pint, currLeft)),
				PR(Point(0.5f, 0.0f, 0.0f), combo.mRot.GetConjugate()),	//GetConjugateQ(pint, leftTop);
				false, 0.0f, 0.0f);
		}

#ifdef TOSEE
		PxArticulationLink* rightTop = gArticulation->createLink(currRight, currRight->getGlobalPose().transform(PxTransform(PxVec3(-0.5f, 0.0f, 1.0f), rightParentRot)));
		PxRigidActorExt::createExclusiveShape(*rightTop, PxCapsuleGeometry(0.05f, 0.8f), *gMaterial);
		//PxRigidActorExt::createExclusiveShape(*rightTop, PxBoxGeometry(0.5f, 0.05f, 0.05f), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*rightTop, 1.0f);

		joint = static_cast<PxArticulationJointReducedCoordinate*>(rightTop->getInboundJoint());
		joint->setParentPose(PxTransform(PxVec3(0.0f, 0.0f, 1.0f), currRight->getGlobalPose().q.getConjugate()));
		joint->setChildPose(PxTransform(PxVec3(0.5f, 0.0f, 0.0f), rightTop->getGlobalPose().q.getConjugate()));
		joint->setJointType(PxArticulationJointType::eREVOLUTE);
		joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eFREE);
		//joint->setDrive(PxArticulationAxis::eTWIST, 0.0f, 10.0f, PX_MAX_F32);
#endif

//#define FOR_IMM_MODE
		PintActorHandle rightTop;
		{
#ifdef FOR_IMM_MODE
			const Point Extents(0.5f, 0.05f, 0.05f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);
#else
//			PINT_CAPSULE_CREATE CapsuleDesc(0.05f, 0.8f);
			PINT_CAPSULE_CREATE CapsuleDesc(0.05f, 0.5f);
			CapsuleDesc.mRenderer	= CreateRenderer(CapsuleDesc);
			Matrix3x3 CapsuleRot;
//			CapsuleRot.RotX(90.0f * DEGTORAD);
//			CapsuleRot.RotY(90.0f * DEGTORAD);
			CapsuleRot.RotZ(90.0f * DEGTORAD);
			CapsuleDesc.mLocalRot = CapsuleRot;
#endif
			PR tmp0 = pint.GetWorldTransform(currRight);
//tmp0 = LastLeftLinkPose;
			PR tmp1(Point(-0.5f, 0.0f, 1.0f), rightParentRot);
			Matrix4x4 M0 = tmp0;
			Matrix4x4 M1 = tmp1;
			Matrix4x4 M = M1 * M0;
			PR combo = M;

			PINT_OBJECT_CREATE ObjectDesc;
#ifdef FOR_IMM_MODE
			ObjectDesc.SetShape(&BoxDesc);
#else
			ObjectDesc.SetShape(&CapsuleDesc);
#endif
			ObjectDesc.mMass			= 1.0f;
//			ObjectDesc.mPosition		= Transform(pint, currLeft, Point(-0.5f, 0.0f, -1.0f));
//			ObjectDesc.mRotation		= leftParentRot;
			ObjectDesc.mPosition		= combo.mPos;
			ObjectDesc.mRotation		= combo.mRot;
			ObjectDesc.mCollisionGroup	= Layer0_Group;

			rightTop = CreateHingedBodyPart(pint, RCA, ObjectDesc, currRight,
				PR(Point(0.0f, 0.0f, 1.0f), GetConjugateQ(pint, currRight)),
				PR(Point(0.5f, 0.0f, 0.0f), combo.mRot.GetConjugate()),	//GetConjugateQ(pint, rightTop)
				false, 0.0f, 0.0f);
		}

		currLeft = leftRoot;
		currRight = rightRoot;

		rightParentRot.Identity();
		leftParentRot.Identity();

		for(udword i=0; i<linkHeight; ++i)
		{
			const Point pos(-0.5f, 0.55f + 0.1f*(1 + i), 0.0f);

	//		PxArticulationLink* leftLink = gArticulation->createLink(currLeft, PxTransform(pos + PxVec3(0.0f, sinAng*(2 * i + 1), 0.0f), leftRot));
	//		PxRigidActorExt::createExclusiveShape(*leftLink, PxBoxGeometry(0.05f, 0.05f, 1.0f), *gMaterial);
	//		PxRigidBodyExt::updateMassAndInertia(*leftLink, 1.0f);

			const Point leftAnchorLocation = pos + Point(0.0f, sinAng*(2 * i), -0.9f);

	//		joint = static_cast<PxArticulationJointReducedCoordinate*>(leftLink->getInboundJoint());
	//		joint->setJointType(PxArticulationJointType::eREVOLUTE);
	//		joint->setParentPose(PxTransform(currLeft->getGlobalPose().transformInv(leftAnchorLocation), leftParentRot));
	//		joint->setChildPose(PxTransform(PxVec3(0.0f, 0.0f, -1.0f), rightRot));
	//			joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eLIMITED);
	//			joint->setLimit(PxArticulationAxis::eTWIST, -PxPi, angle);

			PintActorHandle leftLink;
			{
				const Point Extents(0.05f, 0.05f, 1.0f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= pos + Point(0.0f, sinAng*(2 * i + 1), 0.0f);
				ObjectDesc.mRotation		= leftRot;
				ObjectDesc.mCollisionGroup	= Layer0_Group;

				leftLink = CreateHingedBodyPart(pint, RCA, ObjectDesc, currLeft,
					PR(TransformInv(pint, currLeft, leftAnchorLocation), leftParentRot),
					PR(Point(0.0f, 0.0f, -1.0f), rightRot),
					SetupLimits, -PI, angle);
			}

			leftParentRot = leftRot;

	//		PxArticulationLink* rightLink = gArticulation->createLink(currRight, PxTransform(pos + PxVec3(0.0f, sinAng*(2 * i + 1), 0.0f), rightRot));
	//		PxRigidActorExt::createExclusiveShape(*rightLink, PxBoxGeometry(0.05f, 0.05f, 1.0f), *gMaterial);
	//		PxRigidBodyExt::updateMassAndInertia(*rightLink, 1.0f);

			const Point rightAnchorLocation = pos + Point(0.0f, sinAng*(2 * i), 0.9f);

	//		joint = static_cast<PxArticulationJointReducedCoordinate*>(rightLink->getInboundJoint());
	//		joint->setParentPose(PxTransform(currRight->getGlobalPose().transformInv(rightAnchorLocation), rightParentRot));
	//		joint->setJointType(PxArticulationJointType::eREVOLUTE);
	//		joint->setChildPose(PxTransform(PxVec3(0.0f, 0.0f, 1.0f), leftRot));
	//		joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eLIMITED);
	//		joint->setLimit(PxArticulationAxis::eTWIST, -angle, PxPi);

			PintActorHandle rightLink;
			{
				const Point Extents(0.05f, 0.05f, 1.0f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= pos + Point(0.0f, sinAng*(2 * i + 1), 0.0f);
				ObjectDesc.mRotation		= rightRot;
				ObjectDesc.mCollisionGroup	= Layer0_Group;

				rightLink = CreateHingedBodyPart(pint, RCA, ObjectDesc, currRight,
					PR(TransformInv(pint, currRight, rightAnchorLocation), rightParentRot),
					PR(Point(0.0f, 0.0f, 1.0f), leftRot),
					SetupLimits, -angle, PI);
			}

			rightParentRot = rightRot;

			CreateLoopJoint(pint, leftLink, rightLink, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f));

			currLeft = rightLink;
			currRight = leftLink;
		}

		CreateLoopJoint(pint, currLeft, leftTop, Point(0.0f, 0.0f, -1.0f), Point(-0.5f, 0.0f, 0.0f));

		// TODO: double-check pivots here
		CreateLoopJoint(pint, currRight, rightTop, Point(0.0f, 0.0f, 1.0f), Point(-0.5f, 0.0f, 0.0f));

#ifdef TOSEE
		const PxTransform topPose(PxVec3(0.0f, leftTop->getGlobalPose().p.y + 0.15f, 0.0f));

		PxArticulationLink* top = gArticulation->createLink(leftTop, topPose);
		PxRigidActorExt::createExclusiveShape(*top, PxBoxGeometry(0.5f, 0.1f, 1.5f), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*top, 1.0f);

		joint = static_cast<PxArticulationJointReducedCoordinate*>(top->getInboundJoint());
		joint->setJointType(PxArticulationJointType::eFIX);
		joint->setParentPose(PxTransform(PxVec3(0.0f, 0.0f, 0.0f)));
		joint->setChildPose(PxTransform(PxVec3(0.0f, -0.15f, -0.9f)));

		gScene->addArticulation(*gArticulation);
#endif

		PintActorHandle top;
		{
			const PR topTransform = pint.GetWorldTransform(leftTop);

			const Point topPose(0.0f, topTransform.mPos.y + 0.15f, 0.0f);

			const Point Extents(0.5f, 0.1f, 1.5f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateRenderer(BoxDesc);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= topPose;
//			ObjectDesc.mCollisionGroup	= Layer0_Group;

			top = CreateFixedBodyPart(pint, RCA, ObjectDesc, leftTop, Point(0.0f, 0.0f, 0.0f), Point(0.0f, -0.15f, -0.9f));
		}

		if(RCA)
			pint.AddRCArticulationToScene(RCA);

		return true;
	}

END_TEST(ScissorLift)

///////////////////////////////////////////////////////////////////////////////

static void createMotorizedArm(Pint& pint, float x, float y, float z, udword nb_links, PintActorHandle* handles, bool use_aggregates)
{
//const PintDisabledGroups DG(1, 1);
//pint.SetDisabledGroups(1, &DG);
//		const float Size = 50.0f;

	const PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(true));
//	const PintArticHandle RCA = 0;

	const float ex = 1.0f;
	const float ey = 1.0f;
	float ez = 1.0f;

	// First we create a static anchor object

	PintActorHandle box0;
	{
		const Point Extents(ex, ey, ez);
//		const Point StaticPos(x+float(xxx-15)*Size, y, z+float(yyy-15)*Size);
		const Point StaticPos(x, y, z);

		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mPosition	= StaticPos;
//		ObjectDesc.mCollisionGroup = 1;
		if(RCA)
		{
			ObjectDesc.mMass	= 1.0f;
			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			box0 = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
		}
		else
		{
			ObjectDesc.mMass	= 0.0f;
			box0 = CreatePintObject(pint, ObjectDesc);
		}
	}

	// Then we create a number of rotating links

	const float growth = 1.5f;
//	float velocity = 0.25f;
	float velocity = 1.0f;

	const udword Nb = nb_links;

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
//			const Point DynamicPos(x+float(xxx-15)*Size, y, z+float(yyy-15)*Size);

			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer = CreateBoxRenderer(Extents);

//				box1 = CreateSimpleObject(pint, &BoxDesc, float(i)+1.0f, DynamicPos);
			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition	= DynamicPos;
			ObjectDesc.mMass		= float(i)+1.0f;
//		ObjectDesc.mCollisionGroup = 1;

			if(RCA)
			{
//				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
//				box0 = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);

				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType	= PINT_JOINT_HINGE;
				ArticulatedDesc.mAxisIndex	= X_;
/*					if(SetupLimits)
				{
					ArticulatedDesc.mMinLimit	= -PI;
					ArticulatedDesc.mMaxLimit	= angle;
				}
				else*/
				{
					ArticulatedDesc.mMinLimit	= PI;
					ArticulatedDesc.mMaxLimit	= -PI;
				}
				ArticulatedDesc.mParent		= box0;

/*					ArticulatedDesc.mLocalPivot0.mPos	= TransformInv(pint, currLeft, leftAnchorLocation);
				ArticulatedDesc.mLocalPivot0.mRot	= leftParentRot;
				ArticulatedDesc.mLocalPivot1.mPos	= Point(0.0f, 0.0f, -1.0f);
				ArticulatedDesc.mLocalPivot1.mRot	= rightRot;*/

				ArticulatedDesc.mLocalPivot0.mPos	= localAnchor0;
				ArticulatedDesc.mLocalPivot1.mPos	= Point(-ex, 0.0f, -ez);
//					ArticulatedDesc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
//					ArticulatedDesc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);

				ArticulatedDesc.mMotorFlags			= PINT_MOTOR_VELOCITY;
//					ArticulatedDesc.mMotor.mStiffness	= 1000.0f;
//					ArticulatedDesc.mMotor.mDamping		= 100.0f;
				ArticulatedDesc.mTargetVel			= velocity;
				//TODO: revisit these
				ArticulatedDesc.mMotor.mStiffness	= 0.0f;
				ArticulatedDesc.mMotor.mDamping		= 10000.0f;
				ArticulatedDesc.mFrictionCoeff		= 0.0f;


				box1 = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
			{
				box1 = CreatePintObject(pint, ObjectDesc);
			}

			if(handles)
				handles[i] = box1;
		}

		if(!RCA)
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

//		velocity *= 2.0f;
	}

	if(RCA)
	{
		if(!use_aggregates)
		{
			pint.AddRCArticulationToScene(RCA);
		}
		else
		{
			PintAggregateHandle	Aggregate = pint.CreateAggregate(128, false);
			pint.AddRCArticulationToAggregate(RCA, Aggregate);
			pint.AddAggregateToScene(Aggregate);
		}
	}
}

static const char* gDesc_MotorizedHinges = "Motorized hinges test scene from APE.";

START_TEST(MotorizedHinges, CATEGORY_RCARTICULATIONS, gDesc_MotorizedHinges)

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.25f, 12.43f, 41.98f), Point(0.01f, -0.37f, -0.93f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual void	Close(Pint& pint)
	{
		PintActorHandle* UserData = (PintActorHandle*)pint.mUserData;
		ICE_FREE(UserData);
		pint.mUserData = null;

		TestBase::Close(pint);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		PintActorHandle* Handles = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*5);
		pint.mUserData = Handles;

		createMotorizedArm(pint, 0.0f, 1.0f, 0.0f, 5, Handles, false);

		return true;
	}

	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintActorHandle* Handles = (const PintActorHandle*)pint.mUserData;
		for(udword i=0;i<5;i++)
			y = PrintAngularVelocity(pint, renderer, Handles[i], y, text_scale);
		return y;
	}

END_TEST(MotorizedHinges)

///////////////////////////////////////////////////////////////////////////////

/*static const char* gDesc_MotorizedHingesExt = "Extended motorized hinges test scene.";

START_TEST(MotorizedHingesExt, CATEGORY_RCARTICULATIONS, gDesc_MotorizedHingesExt)

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.25f, 12.43f, 41.98f), Point(0.01f, -0.37f, -0.93f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		createMotorizedArm(pint, 0.0f, 1.0f, 0.0f, 16, null, false);

		return true;
	}

END_TEST(MotorizedHingesExt)*/

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_MotorizedHingesBenchmark = "Motorized hinges benchmark.";

START_TEST(MotorizedHingesBenchmark, CATEGORY_RCARTICULATIONS, gDesc_MotorizedHingesBenchmark)

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-31.40f, 536.55f, 900.20f), Point(0.41f, -0.80f, -0.45f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		const udword Nb = 64;
		//const udword Nb = 16;

		const float ScaleX = 20.0f;
		const float ScaleZ = 30.0f;
		for(udword j=0;j<Nb;j++)
		{
			for(udword i=0;i<Nb;i++)
			{
				//createMotorizedArm(pint, float(i)*ScaleX, 0.0f, float(j)*ScaleZ, 5, null, false);
				createMotorizedArm(pint, float(i)*ScaleX, 0.0f, float(j)*ScaleZ, 5, null, true);
			}
		}

		return true;
	}

END_TEST(MotorizedHingesBenchmark)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DoubleHinge = "Double hinge.";

START_TEST(DoubleHinge, CATEGORY_RCARTICULATIONS, gDesc_DoubleHinge)

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.11f, 7.16f, 5.43f), Point(-0.65f, -0.26f, -0.71f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		const PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(false));

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mPosition	= Point(0.0f, 3.0f, 0.0f);
		ObjectDesc.mMass		= 1.0f;

		// works
//		ObjectDesc.mLinearVelocity = Point(10000.0f, 10000.0f, 10000.0f);
//		ObjectDesc.mAngularVelocity = Point(1.0f, 0.0f, 0.0f);

		PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
		ArticulatedDesc.mFrictionCoeff = 0.0f;
		ArticulatedDesc.mParent = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);

		ObjectDesc.mPosition	= Point(0.0f, 5.0f, 0.0f);

		// doesn't work
//		ObjectDesc.mLinearVelocity = Point(10000.0f, 10000.0f, 10000.0f);

		ArticulatedDesc.mLocalPivot0.mPos = Point(0.0f, 1.0f, 0.0f);
		ArticulatedDesc.mLocalPivot1.mPos = Point(0.0f, -1.0f, 0.0f);
		ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

		ArticulatedDesc.mMinTwistLimit	= -45.0f * DEGTORAD;
		ArticulatedDesc.mMaxTwistLimit	= 45.0f * DEGTORAD;
		ArticulatedDesc.mMinSwing1Limit	= -75.0f * DEGTORAD;
		ArticulatedDesc.mMaxSwing1Limit	= 30.0f * DEGTORAD;
		ArticulatedDesc.mMinSwing2Limit	= 0.0f;
		ArticulatedDesc.mMaxSwing2Limit	= 0.0f;

//		ObjectDesc.mAngularVelocity = Point(100.0f, 0.0f, 0.0f);
//		ObjectDesc.mAngularVelocity = Point(0.0f, 100.0f, 0.0f);
//		ObjectDesc.mLinearVelocity = Point(0.0f, 0.0f, 10000.0f);
//		ObjectDesc.mAngularVelocity = Point(0.0f, 0.0f, 10000.0f);

		PintActorHandle h = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
		(void)h;

		pint.AddRCArticulationToScene(RCA);

//		pint.SetLinearVelocity(h, Point(10000.0f, 0.0f, 0.0f));
//		pint.SetAngularVelocity(h, Point(10000.0f, 0.0f, 0.0f));

		return true;
	}

END_TEST(DoubleHinge)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DoubleHinge2 = "Double hinge 2.";

START_TEST(DoubleHinge2, CATEGORY_RCARTICULATIONS, gDesc_DoubleHinge2)

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.11f, 7.16f, 5.43f), Point(-0.65f, -0.26f, -0.71f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		const PintDisabledGroups CollisionGroups(1, 1);
		pint.SetDisabledGroups(1, &CollisionGroups);

		const PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(true));

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		const Point Extents2(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc2(Extents2);
		BoxDesc2.mRenderer = CreateNullRenderer();//CreateBoxRenderer(Extents2);

		const Point StartPos(0.0f, 3.0f, 0.0f);

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mPosition	= StartPos;
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mCollisionGroup	= 1;
		PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
		ArticulatedDesc.mFrictionCoeff = 0.02f;
		ObjectDesc.SetShape(&BoxDesc);
		ArticulatedDesc.mParent = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);

		ObjectDesc.mPosition	= StartPos + Point(0.0f, Extents.y/*+Extents2.y*/, 0.0f);

		ArticulatedDesc.mLocalPivot0.mPos = Point(0.0f, Extents.y, 0.0f);
		ArticulatedDesc.mLocalPivot1.mPos = Point(0.0f, 0.0f/*-Extents2.y*/, 0.0f);
		ArticulatedDesc.mJointType = PINT_JOINT_HINGE;

		ArticulatedDesc.mAxisIndex	= X_;
		ArticulatedDesc.mMinLimit	= -45.0f * DEGTORAD;
		ArticulatedDesc.mMaxLimit	= 45.0f * DEGTORAD;

/*		ArticulatedDesc.mMinTwistLimit	= -45.0f * DEGTORAD;
		ArticulatedDesc.mMaxTwistLimit	= 45.0f * DEGTORAD;
		ArticulatedDesc.mMinSwing1Limit	= -75.0f * DEGTORAD;
		ArticulatedDesc.mMaxSwing1Limit	= 30.0f * DEGTORAD;
		ArticulatedDesc.mMinSwing2Limit	= 0.0f;
		ArticulatedDesc.mMaxSwing2Limit	= 0.0f;*/

		ObjectDesc.SetShape(&BoxDesc2);
		ArticulatedDesc.mParent = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);


		ObjectDesc.mPosition	= StartPos + Point(0.0f, (Extents.y/*+Extents2.y*/)*2.0f, 0.0f);

		ArticulatedDesc.mLocalPivot0.mPos = Point(0.0f, 0.0f/*Extents2.y*/, 0.0f);
		ArticulatedDesc.mLocalPivot1.mPos = Point(0.0f, -Extents.y, 0.0f);
		ArticulatedDesc.mJointType = PINT_JOINT_HINGE;

		ArticulatedDesc.mAxisIndex	= Y_;
		ArticulatedDesc.mMinLimit	= -75.0f * DEGTORAD;
		ArticulatedDesc.mMaxLimit	= 30.0f * DEGTORAD;

		ObjectDesc.SetShape(&BoxDesc);
		pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);



		pint.AddRCArticulationToScene(RCA);

		return true;
	}

END_TEST(DoubleHinge2)

///////////////////////////////////////////////////////////////////////////////

// TODO:
// - check collision filters
// - timestep & #iters
// - switch to densities instead of masses
// - try to remap right_hip_xyz to avoid instabilities
// - fix feet axes
// - fix shoulder axes
// - try multiples hinges & invisible links
// - add pos actuators (also sliders in the UI) => are they actually pos actuators?

static const bool gUseFixedJoints = false;
static const bool gAddArms = true;

static float gTimestep = 0.005f;
static float gJointFriction = 0.02f;
static float gMaxJointVelocity = 20.0f;
static bool gUseFakeLimits = false;

static Point MujocoPosition(const Point& p)
{
	return Point(p.x, p.z, p.y);
}

static void SetupMujocoSphere(PINT_SPHERE_CREATE& desc, const Point& pos, float radius)
{
	desc.mRadius	= radius;
	desc.mLocalPos	= MujocoPosition(pos);
	desc.mRenderer	= CreateSphereRenderer(desc.mRadius);
}

static void SetupMujocoCapsule(PINT_CAPSULE_CREATE& desc, const Point& from, const Point& to, float radius)
{
	desc.mRadius = radius;
	desc.mHalfHeight = from.Distance(to)*0.5f;

	Point dir = MujocoPosition(to) - MujocoPosition(from);
	dir.Normalize();

	Matrix3x3 rot;
	rot.FromTo(Point(0.0f, 1.0f, 0.0f), dir);
	desc.mLocalRot = rot;

	const Point center = (from + to)*0.5f;
	desc.mLocalPos = MujocoPosition(center);

	desc.mRenderer	= CreateCapsuleRenderer(desc.mRadius, desc.mHalfHeight*2.0f);
}

static void SetupTwistLimits(PINT_RC_ARTICULATED_BODY_CREATE& desc, float min_limit, float max_limit)
{
	desc.mMinTwistLimit		= min_limit * DEGTORAD;
	desc.mMaxTwistLimit		= max_limit * DEGTORAD;
}

static void SetupSwing1Limits(PINT_RC_ARTICULATED_BODY_CREATE& desc, float min_limit, float max_limit)
{
	desc.mMinSwing1Limit	= min_limit * DEGTORAD;
	desc.mMaxSwing1Limit	= max_limit * DEGTORAD;
}

static void SetupSwing2Limits(PINT_RC_ARTICULATED_BODY_CREATE& desc, float min_limit, float max_limit)
{
	desc.mMinSwing2Limit	= min_limit * DEGTORAD;
	desc.mMaxSwing2Limit	= max_limit * DEGTORAD;
}

static void SetupArticBodyDesc(PINT_RC_ARTICULATED_BODY_CREATE& desc)
{
	desc.mFrictionCoeff = gJointFriction;
	desc.mMaxJointVelocity = gMaxJointVelocity;

	if(0)
	{
		desc.mMotor.mStiffness = 0.0f;
		desc.mMotor.mDamping = 10000.0f;
		desc.mMotor.mMaxForce = 10000.0f;
		desc.mMotor.mAccelerationDrive = false;

		desc.mMotorFlags = PINT_MOTOR_VELOCITY;
		desc.mTargetVel = 0.0f;
	}

	if(gUseFixedJoints)
	{
		if(desc.mParent)
			desc.mJointType = PINT_JOINT_FIXED;
	}

	if(gUseFakeLimits)
	{
		const float epsilon = 5.0f * DEGTORAD;
		desc.mMinLimit = -epsilon;
		desc.mMaxLimit = epsilon;
		SetupTwistLimits(desc, -epsilon, epsilon);
//		desc.mMinSwing1Limit = -epsilon;
//		desc.mMaxSwing1Limit = epsilon;
//		desc.mMinSwing2Limit = -epsilon;
//		desc.mMaxSwing2Limit = epsilon;
		SetupSwing1Limits(desc, -epsilon, epsilon);
		SetupSwing2Limits(desc, -epsilon, epsilon);
	}
}

static void SetupLink(PINT_OBJECT_CREATE& desc, const char* name, const Point& pos, const PINT_SHAPE_CREATE* shapes, float mass)
{
	desc.mName				= name;
	desc.mMass				= mass;
//	desc.mMassForInertia	= mass*10.0f;
	desc.mPosition			= MujocoPosition(pos);
	desc.SetShape(shapes);
}

static const char* gDesc_MujocoHumanoid = "Humanoid model from Mujoco.";

START_TEST(MujocoHumanoid, CATEGORY_RCARTICULATIONS, gDesc_MujocoHumanoid)

	CheckBoxPtr		mCheckBox_FixedBase;
	CheckBoxPtr		mCheckBox_Debug_UseFakeLimits;
	EditBoxPtr		mEditBox_JointFriction;
	EditBoxPtr		mEditBox_MaxJointVelocity;
	EditBoxPtr		mEditBox_Timestep;

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		const sdword Width = 300;
		IceWindow* UI = CreateTestWindow(Width, 200);

		Widgets& UIElems = GetUIElements();

		const sdword OffsetX = 120;
		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 120;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;

		{
//			y += YStep;

			mCheckBox_FixedBase = helper.CreateCheckBox(UI, 0, 4, y, 200, 20, "Fixed base", &UIElems, false, null, null);
			y += YStep;

			mCheckBox_Debug_UseFakeLimits = helper.CreateCheckBox(UI, 0, 4, y, 200, 20, "DEBUG: use small limits", &UIElems, false, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Joint friction:", &UIElems);
//			mEditBox_JointFriction = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.02", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_JointFriction = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Max joint velocity:", &UIElems);
			mEditBox_MaxJointVelocity = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "20.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Timestep:", &UIElems);
			mEditBox_Timestep = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.005", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
	
		}

		y += YStep;
		AddResetButton(UI, 4, y, Width);

		return null;
	}

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(4.25f, 12.43f, 41.98f), Point(0.01f, -0.37f, -0.93f));
		//desc.mCamera[0] = PintCameraPose(Point(-1.20f, 1.54f, 0.65f), Point(0.82f, -0.13f, -0.56f));
		//desc.mCamera[0] = PintCameraPose(Point(1.36f, 1.80f, -0.79f), Point(-0.75f, -0.49f, 0.45f));
		desc.mCamera[0] = PintCameraPose(Point(1.20f, 1.18f, -1.77f), Point(-0.50f, -0.20f, 0.84f));
		//desc.mGravity.Zero();
		SetDefEnv(desc, true);

		gUseFakeLimits = mCheckBox_Debug_UseFakeLimits ? mCheckBox_Debug_UseFakeLimits->IsChecked() : false;
		gJointFriction = GetFloat(0.02f, mEditBox_JointFriction);
		gMaxJointVelocity = GetFloat(20.0f, mEditBox_MaxJointVelocity);
		gTimestep = GetFloat(0.005f, mEditBox_Timestep);

		//<option timestep="0.005" iterations="50" tolerance="1e-10" solver="Newton" jacobian="dense" cone="pyramidal"/>
		desc.mTimestep = gTimestep;
		desc.mNbSimulateCallsPerFrame = udword((1.0f/60.0f)/desc.mTimestep);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;

		return CreateHumanoid(pint, caps, Point(0.0f, 0.0f, 0.0f));
	}

	bool CreateHumanoid(Pint& pint, const PintCaps& caps, const Point& offset)
	{
		const bool FixedBase = mCheckBox_FixedBase ? mCheckBox_FixedBase->IsChecked() : false;
		const PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(FixedBase));

		MatrixStack MS;

		PintActorHandle torso;
Point torsoPos;
		{
			//<geom name="torso1" type="capsule" fromto="0 -.07 0 0 .07 0"  size="0.07"/>
			PINT_CAPSULE_CREATE torso1Desc;
			torso1Desc.mName = "torso1";
			SetupMujocoCapsule(torso1Desc, Point(0.0f, -0.07f, 0.0f), Point(0.0f, 0.07f, 0.0f), 0.07f);

			//<geom name="head" type="sphere" pos="0 0 .19" size=".09"/>
			PINT_SPHERE_CREATE headDesc;
			headDesc.mName = "head";
			SetupMujocoSphere(headDesc, Point(0.0f, 0.0f, 0.19f), 0.09f);

			//<geom name="uwaist" type="capsule" fromto="-.01 -.06 -.12 -.01 .06 -.12" size="0.06"/>
			PINT_CAPSULE_CREATE uwaistDesc;
			uwaistDesc.mName = "uwaist";
			SetupMujocoCapsule(uwaistDesc, Point(-0.01f, -0.06f, -0.12f), Point(-0.01f, 0.06f, -0.12f), 0.06f);

			//<body name="torso" pos="0 0 1.4">
			torso1Desc.SetNext(&headDesc);
			headDesc.SetNext(&uwaistDesc);

			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "torso", offset + Point(0.0f, 0.0f, 1.4f), &torso1Desc, 8.3f);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				SetupArticBodyDesc(ArticulatedDesc);
				torso = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				torso = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
torsoPos = ObjectDesc.mPosition;
		}

		PintActorHandle lwaist;
Point lwaistPos;
		{
			//<geom name="lwaist" type="capsule" fromto="0 -.06 0 0 .06 0"  size="0.06" />
			PINT_CAPSULE_CREATE lwaistDesc;
			SetupMujocoCapsule(lwaistDesc, Point(0.0f, -0.06f, 0.0f), Point(0.0f, 0.06f, 0.0f), 0.06f);

			//<body name="lwaist" pos="-.01 0 -0.260" quat="1.000 0 -0.002 0" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "lwaist", Point(-0.01f, 0.0f, -0.260f), &lwaistDesc, 2.0f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = torso;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.065f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - torsoPos;

/*					// Rel joint pose
					Point JointRelPos = MujocoPosition(Point(0.0f, 0.0f, 0.065f));
					Quat JointRelRot(1.0f, 0.0f, 0.0f, 0.0f);
					// World joint pose
					MS.Transform(JointRelPos, JointRelRot);*/

					// Parent
//					ArticulatedDesc.mLocalPivot0.mPos = TorsoPos - JointRelPos;
					// Child
//					ArticulatedDesc.mLocalPivot1.mPos = ObjectDesc.mPosition - JointRelPos;


//					ArticulatedDesc.mLocalPivot0.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.065f));
					//MS.Transform(ArticulatedDesc.mLocalPivot0.mPos, ArticulatedDesc.mLocalPivot0.mRot);
//					ArticulatedDesc.mLocalPivot0.mPos = -ArticulatedDesc.mLocalPivot1.mPos;

//					ArticulatedDesc.mMinTwistLimit		= 0.0f;
//					ArticulatedDesc.mMaxTwistLimit		= 0.0f;
					SetupTwistLimits(ArticulatedDesc, 0.0f, 0.0f);

					//<joint name="abdomen_z" type="hinge" pos="0 0 0.065" axis="0 0 1" range="-45 45" damping="5" stiffness="20" armature="0.02" />
//					ArticulatedDesc.mMinSwing1Limit		= -45.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 45.0f * DEGTORAD;
					SetupSwing1Limits(ArticulatedDesc, -45.0f, 45.0f);

					//<joint name="abdomen_y" type="hinge" pos="0 0 0.065" axis="0 1 0" range="-75 30" damping="5" stiffness="10" armature="0.02" />
//					ArticulatedDesc.mMinSwing2Limit		= -30.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 75.0f * DEGTORAD;
					SetupSwing2Limits(ArticulatedDesc, -30.0f, 75.0f);

					//LockJoint(ArticulatedDesc);
//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				lwaist = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				lwaist = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
lwaistPos = ObjectDesc.mPosition;
		}

		PintActorHandle pelvis;
Point pelvisPos;
		{
			//<geom name="butt" type="capsule" fromto="-.02 -.07 0 -.02 .07 0"  size="0.09" />
			PINT_CAPSULE_CREATE buttDesc;
			SetupMujocoCapsule(buttDesc, Point(-0.02f, -0.07f, 0.0f), Point(-0.02f, 0.07f, 0.0f), 0.09f);

			//<body name="pelvis" pos="0 0 -0.165" quat="1.000 0 -0.002 0" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "pelvis", Point(0.0f, 0.0f, -0.165f), &buttDesc, 5.9f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = lwaist;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.1f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - lwaistPos;

					//<joint name="abdomen_x" type="hinge" pos="0 0 0.1" axis="1 0 0" range="-35 35" damping="5" stiffness="10" armature="0.02" />
					ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
					ArticulatedDesc.mAxisIndex = X_;
					ArticulatedDesc.mMinLimit = -35.0f * DEGTORAD;
					ArticulatedDesc.mMaxLimit = 35.0f * DEGTORAD;

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				pelvis = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				pelvis = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
pelvisPos = ObjectDesc.mPosition;
		}

		PintActorHandle right_thigh;
Point right_thighPos;
		{
			//<geom name="right_thigh1" type="capsule" fromto="0 0 0 0 0.01 -.34"  size="0.06" />
			PINT_CAPSULE_CREATE right_thigh1Desc;
			SetupMujocoCapsule(right_thigh1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.01f, -0.34f), 0.06f);

			//<body name="right_thigh" pos="0 -0.1 -0.04" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "right_thigh", Point(0.0f, -0.1f, -0.04f), &right_thigh1Desc, 4.5f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = pelvis;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - pelvisPos;

					//<joint name="right_hip_x" type="hinge" pos="0 0 0" axis="1 0 0" range="-25 5"   damping="5" stiffness="10" armature="0.01" />
//					ArticulatedDesc.mMinTwistLimit		= -5.0f * DEGTORAD;
//					ArticulatedDesc.mMaxTwistLimit		= 25.0f * DEGTORAD;
					SetupTwistLimits(ArticulatedDesc, -5.0f, 25.0f);

					//<joint name="right_hip_z" type="hinge" pos="0 0 0" axis="0 0 1" range="-60 35"  damping="5" stiffness="10" armature="0.01" />
//					ArticulatedDesc.mMinSwing1Limit		= -35.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 60.0f * DEGTORAD;
					SetupSwing1Limits(ArticulatedDesc, -35.0f, 60.0f);

					//<joint name="right_hip_y" type="hinge" pos="0 0 0" axis="0 1 0" range="-120 20" damping="5" stiffness="20" armature="0.01" />
//					ArticulatedDesc.mMinSwing2Limit		= -20.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 120.0f * DEGTORAD;
					SetupSwing2Limits(ArticulatedDesc, -20.0f, 120.0f);

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				right_thigh = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				right_thigh = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
right_thighPos = ObjectDesc.mPosition;
		}

		PintActorHandle right_shin;
Point right_shinPos;
		{
			//<geom name="right_shin1" type="capsule" fromto="0 0 0 0 0 -.3"   size="0.049" />
			PINT_CAPSULE_CREATE right_shin1Desc;
			SetupMujocoCapsule(right_shin1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, -0.3f), 0.049f);

			//<body name="right_shin" pos="0 0.01 -0.403" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "right_shin", Point(0.0f, 0.01f, -0.403f), &right_shin1Desc, 2.6f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = right_thigh;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.02f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - right_thighPos;

					//<joint name="right_knee" type="hinge" pos="0 0 .02" axis="0 -1 0" range="-160 -2" stiffness="1" armature="0.0060" />
					ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
					ArticulatedDesc.mAxisIndex = Z_;
					ArticulatedDesc.mMinLimit = -160.0f * DEGTORAD;
					ArticulatedDesc.mMaxLimit = -2.0f * DEGTORAD;

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				right_shin = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				right_shin = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
right_shinPos = ObjectDesc.mPosition;
		}

		PintActorHandle right_foot;
Point right_footPos;
		{
			//<geom name="right_foot_cap1" type="capsule" fromto="-.07 -0.02 0 0.14 -0.04 0"  size="0.027" />
			PINT_CAPSULE_CREATE right_foot_cap1Desc;
			SetupMujocoCapsule(right_foot_cap1Desc, Point(-0.07f, -0.02f, 0.0f), Point(0.14f, -0.04f, 0.0f), 0.027f);

			//<geom name="right_foot_cap2" type="capsule" fromto="-.07 0 0 0.14  0.02 0"  size="0.027" />
			PINT_CAPSULE_CREATE right_foot_cap2Desc;
			SetupMujocoCapsule(right_foot_cap2Desc, Point(-0.07f, 0.0f, 0.0f), Point(0.14f, 0.02f, 0.0f), 0.027f);

			//<body name="right_foot" pos="0 0 -.39" >
			right_foot_cap1Desc.SetNext(&right_foot_cap2Desc);
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "right_foot", Point(0.0f, 0.0f, -0.39f), &right_foot_cap1Desc, 1.1f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = right_shin;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.06f));	//####

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - right_shinPos;

					//<joint name="right_ankle_y" type="hinge" pos="0 0 0.08" axis="0 1 0"   range="-50 50" stiffness="4" armature="0.0008" />
					//<joint name="right_ankle_x" type="hinge" pos="0 0 0.04" axis="1 0 0.5" range="-50 50" stiffness="1"  armature="0.0006" />

//					ArticulatedDesc.mMinTwistLimit		= -50.0f * DEGTORAD;
//					ArticulatedDesc.mMaxTwistLimit		= 50.0f * DEGTORAD;
					SetupTwistLimits(ArticulatedDesc, -50.0f, 50.0f);

//					ArticulatedDesc.mMinSwing1Limit		= 0.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 0.0f * DEGTORAD;
					SetupSwing1Limits(ArticulatedDesc, 0.0f, 0.0f);

//					ArticulatedDesc.mMinSwing2Limit		= -50.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 50.0f * DEGTORAD;
					SetupSwing2Limits(ArticulatedDesc, -50.0f, 50.0f);

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				right_foot = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				right_foot = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
right_footPos = ObjectDesc.mPosition;
		}

		MS.Pop();
		MS.Pop();
		MS.Pop();

		PintActorHandle left_thigh;
Point left_thighPos;
		{
			//<geom name="left_thigh1" type="capsule" fromto="0 0 0 0 -0.01 -.34"  size="0.06" />
			PINT_CAPSULE_CREATE left_thigh1Desc;
			SetupMujocoCapsule(left_thigh1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.0f, -0.01f, -0.34f), 0.06f);

			//<body name="left_thigh" pos="0 0.1 -0.04" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "left_thigh", Point(0.0f, 0.1f, -0.04f), &left_thigh1Desc, 4.5f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = pelvis;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - pelvisPos;

					//<joint name="left_hip_x" type="hinge" pos="0 0 0" axis="-1 0 0" range="-25 5"  damping="5" stiffness="10" armature="0.01" />
//					ArticulatedDesc.mMinTwistLimit		= -25.0f * DEGTORAD;
//					ArticulatedDesc.mMaxTwistLimit		= 5.0f * DEGTORAD;
//						ArticulatedDesc.mMinTwistLimit		= 0.0f;
//						ArticulatedDesc.mMaxTwistLimit		= 0.0f;
					SetupTwistLimits(ArticulatedDesc, -25.0f, 5.0f);

					//<joint name="left_hip_z" type="hinge" pos="0 0 0" axis="0 0 -1" range="-60 35" damping="5" stiffness="10" armature="0.01" />
//					ArticulatedDesc.mMinSwing1Limit		= -60.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 35.0f * DEGTORAD;
//						ArticulatedDesc.mMinSwing1Limit		= 0.0f;
//						ArticulatedDesc.mMaxSwing1Limit		= 0.0f;
					SetupSwing1Limits(ArticulatedDesc, -60.0f, 35.0f);

					//<joint name="left_hip_y" type="hinge" pos="0 0 0" axis="0 1 0" range="-120 20" damping="5" stiffness="20" armature="0.01" />
//					ArticulatedDesc.mMinSwing2Limit		= -20.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 120.0f * DEGTORAD;
//						ArticulatedDesc.mMinSwing2Limit		= 0.0f;
//						ArticulatedDesc.mMaxSwing2Limit		= 0.0f;
					SetupSwing2Limits(ArticulatedDesc, -20.0f, 120.0f);

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				left_thigh = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				left_thigh = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
left_thighPos = ObjectDesc.mPosition;
		}

		PintActorHandle left_shin;
Point left_shinPos;
		{
			//<geom name="left_shin1" type="capsule" fromto="0 0 0 0 0 -.3"   size="0.049" />
			PINT_CAPSULE_CREATE left_shin1Desc;
			SetupMujocoCapsule(left_shin1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, -0.3f), 0.049f);

			//<body name="left_shin" pos="0 -0.01 -0.403" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "left_shin", Point(0.0f, -0.01f, -0.403f), &left_shin1Desc, 2.6f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = left_thigh;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.02f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - left_thighPos;

					//<joint name="left_knee" type="hinge" pos="0 0 .02" axis="0 -1 0" range="-160 -2" stiffness="1" armature="0.0060" />
					ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
					ArticulatedDesc.mAxisIndex = Z_;
					ArticulatedDesc.mMinLimit = -160.0f * DEGTORAD;
					ArticulatedDesc.mMaxLimit = -2.0f * DEGTORAD;

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				left_shin = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				left_shin = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
left_shinPos = ObjectDesc.mPosition;
		}

		PintActorHandle left_foot;
Point left_footPos;
		{
			//<geom name="left_foot_cap1" type="capsule" fromto="-.07 0.02 0 0.14 0.04 0"  size="0.027" />
			PINT_CAPSULE_CREATE left_foot_cap1Desc;
			SetupMujocoCapsule(left_foot_cap1Desc, Point(-0.07f, 0.02f, 0.0f), Point(0.14f, 0.04f, 0.0f), 0.027f);

			//<geom name="left_foot_cap2" type="capsule" fromto="-.07 0 0 0.14  -0.02 0"  size="0.027" />
			PINT_CAPSULE_CREATE left_foot_cap2Desc;
			SetupMujocoCapsule(left_foot_cap2Desc, Point(-0.07f, 0.0f, 0.0f), Point(0.14f, -0.02f, 0.0f), 0.027f);

			//<body name="left_foot" pos="0 0 -.39" >
			left_foot_cap1Desc.SetNext(&left_foot_cap2Desc);
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "left_foot", Point(0.0f, 0.0f, -0.39f), &left_foot_cap1Desc, 1.1f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = left_shin;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.06f));	//####

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - left_shinPos;

					//	<joint name="left_ankle_y" type="hinge" pos="0 0 0.08" axis="0 1 0"   range="-50 50"  stiffness="4" armature="0.0008" />
					//	<joint name="left_ankle_x" type="hinge" pos="0 0 0.04" axis="1 0 0.5" range="-50 50"  stiffness="1"  armature="0.0006" />

//					ArticulatedDesc.mMinTwistLimit		= -50.0f * DEGTORAD;
//					ArticulatedDesc.mMaxTwistLimit		= 50.0f * DEGTORAD;
					SetupTwistLimits(ArticulatedDesc, -50.0f, 50.0f);

//					ArticulatedDesc.mMinSwing1Limit		= 0.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 0.0f * DEGTORAD;
					SetupSwing1Limits(ArticulatedDesc, 0.0f, 0.0f);

//					ArticulatedDesc.mMinSwing2Limit		= -50.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 50.0f * DEGTORAD;
					SetupSwing2Limits(ArticulatedDesc, -50.0f, 50.0f);

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				left_foot = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				left_foot = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
left_footPos = ObjectDesc.mPosition;
		}                        
                        
		MS.Pop();
		MS.Pop();
		MS.Pop();

		MS.Pop();
		MS.Pop();

		if(gAddArms)
		{
			PintActorHandle right_upper_arm;
	Point right_upper_armPos;
			{
				//<geom name="right_uarm1" type="capsule" fromto="0 0 0 .16 -.16 -.16"  size="0.04 0.16" />
				PINT_CAPSULE_CREATE right_uarm1Desc;
				SetupMujocoCapsule(right_uarm1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.16f, -0.16f, -0.16f), 0.04f);

				//<body name="right_upper_arm" pos="0 -0.17 0.06" >
				PINT_OBJECT_CREATE ObjectDesc;
				SetupLink(ObjectDesc, "right_upper_arm", Point(0.0f, -0.17f, 0.06f), &right_uarm1Desc, 1.6f);
				MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = torso;
					if(1)
					{
						ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

						ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

						Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
						ArticulatedDesc.mLocalPivot0.mPos = wp - torsoPos;

						Point axis0(2.0f, 1.0f, 1.0f);	axis0.Normalize();
						Point axis1(0.0f, -1.0f, 1.0f);	axis1.Normalize();
						Point cr = axis0^axis1;

						Matrix3x3 fromTo;
						fromTo.FromTo(Point(1.0f, 0.0f, 0.0f), axis0);
						ArticulatedDesc.mLocalPivot0.mRot = fromTo;
						ArticulatedDesc.mLocalPivot1.mRot = fromTo;

	//                <joint name="right_shoulder1" type="hinge" pos="0 0 0" axis="2 1 1"  range="-85 60" stiffness="1" armature="0.0068" />
	//                <joint name="right_shoulder2" type="hinge" pos="0 0 0" axis="0 -1 1" range="-85 60" stiffness="1"  armature="0.0051" />

//						ArticulatedDesc.mMinTwistLimit		= -85.0f * DEGTORAD;
//						ArticulatedDesc.mMaxTwistLimit		= 60.0f * DEGTORAD;
						SetupTwistLimits(ArticulatedDesc, -85.0f, 60.0f);

//						ArticulatedDesc.mMinSwing1Limit		= 0.0f * DEGTORAD;
//						ArticulatedDesc.mMaxSwing1Limit		= 0.0f * DEGTORAD;
						SetupSwing1Limits(ArticulatedDesc, 0.0f, 0.0f);

//						ArticulatedDesc.mMinSwing2Limit		= -85.0f * DEGTORAD;
//						ArticulatedDesc.mMaxSwing2Limit		= 60.0f * DEGTORAD;
						SetupSwing2Limits(ArticulatedDesc, -85.0f, 60.0f);
					}
					SetupArticBodyDesc(ArticulatedDesc);
					right_upper_arm = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
					right_upper_arm = CreatePintObject(pint, ObjectDesc);

				MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
	right_upper_armPos = ObjectDesc.mPosition;
			}

			PintActorHandle right_lower_arm;
	Point right_lower_armPos;
			{
				//<geom name="right_larm" type="capsule" fromto="0.01 0.01 0.01 .17 .17 .17"  size="0.031" />
				PINT_CAPSULE_CREATE right_larmDesc;
				SetupMujocoCapsule(right_larmDesc, Point(0.01f, 0.01f, 0.01f), Point(0.17f, 0.17f, 0.17f), 0.031f);

				//<geom name="right_hand" type="sphere" pos=".18 .18 .18"  size="0.04"/>
				PINT_SPHERE_CREATE right_handDesc;
				SetupMujocoSphere(right_handDesc, Point(0.18f, 0.18f, 0.18f), 0.04f);

				//<body name="right_lower_arm" pos=".18 -.18 -.18" >
				right_larmDesc.SetNext(&right_handDesc);
				PINT_OBJECT_CREATE ObjectDesc;
				SetupLink(ObjectDesc, "right_lower_arm", Point(0.18f, -0.18f, -0.18f), &right_larmDesc, 1.2f);
				MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = right_upper_arm;
					if(1)
					{
						ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

						ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

						Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
						ArticulatedDesc.mLocalPivot0.mPos = wp - right_upper_armPos;

	//          <joint name="right_elbow" type="hinge" pos="0 0 0" axis="0 -1 1" range="-90 50"  stiffness="0" armature="0.0028" />

						ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
						ArticulatedDesc.mAxisIndex = Z_;
						ArticulatedDesc.mMinLimit = -90.0f * DEGTORAD;
						ArticulatedDesc.mMaxLimit = 50.0f * DEGTORAD;
					}
					SetupArticBodyDesc(ArticulatedDesc);
					right_lower_arm = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
					right_lower_arm = CreatePintObject(pint, ObjectDesc);

				MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
	right_lower_armPos = ObjectDesc.mPosition;
			}        

			MS.Pop();
			MS.Pop();

			PintActorHandle left_upper_arm;
	Point left_upper_armPos;
			{
				//<geom name="left_uarm1" type="capsule" fromto="0 0 0 .16 .16 -.16"  size="0.04 0.16" />
				PINT_CAPSULE_CREATE left_uarm1Desc;
				SetupMujocoCapsule(left_uarm1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.16f, 0.16f, -0.16f), 0.04f);

				//<body name="left_upper_arm" pos="0 0.17 0.06" >
				PINT_OBJECT_CREATE ObjectDesc;
				SetupLink(ObjectDesc, "left_upper_arm", Point(0.0f, 0.17f, 0.06f), &left_uarm1Desc, 1.6f);
				MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = torso;
					if(1)
					{
						ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

						ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

						Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
						ArticulatedDesc.mLocalPivot0.mPos = wp - torsoPos;

						Point axis0(2.0f, -1.0f, 1.0f);	axis0.Normalize();
						Point axis1(0.0f, 1.0f, 1.0f);	axis1.Normalize();
						Point cr = axis0^axis1;

						Matrix3x3 fromTo;
						fromTo.FromTo(Point(1.0f, 0.0f, 0.0f), axis0);
						ArticulatedDesc.mLocalPivot0.mRot = fromTo;
						ArticulatedDesc.mLocalPivot1.mRot = fromTo;

	//                <joint name="left_shoulder1" type="hinge" pos="0 0 0" axis="2 -1 1" range="-60 85" stiffness="1" armature="0.0068" />
	//                <joint name="left_shoulder2" type="hinge" pos="0 0 0" axis="0 1 1" range="-60 85"  stiffness="1" armature="0.0051" />

//						ArticulatedDesc.mMinTwistLimit		= -60.0f * DEGTORAD;
//						ArticulatedDesc.mMaxTwistLimit		= 85.0f * DEGTORAD;
						SetupTwistLimits(ArticulatedDesc, -60.0f, 85.0f);

//						ArticulatedDesc.mMinSwing1Limit		= 0.0f * DEGTORAD;
//						ArticulatedDesc.mMaxSwing1Limit		= 0.0f * DEGTORAD;
						SetupSwing1Limits(ArticulatedDesc, 0.0f, 0.0f);

//						ArticulatedDesc.mMinSwing2Limit		= -60.0f * DEGTORAD;
//						ArticulatedDesc.mMaxSwing2Limit		= 85.0f * DEGTORAD;
						SetupSwing2Limits(ArticulatedDesc, -60.0f, 85.0f);
					}
					SetupArticBodyDesc(ArticulatedDesc);
					left_upper_arm = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
					left_upper_arm = CreatePintObject(pint, ObjectDesc);

				MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
	left_upper_armPos = ObjectDesc.mPosition;


			}

			PintActorHandle left_lower_arm;
	Point left_lower_armPos;
			{
				//<geom name="left_larm" type="capsule" fromto="0.01 -0.01 0.01 .17 -.17 .17"  size="0.031" />
				PINT_CAPSULE_CREATE left_larmDesc;
				SetupMujocoCapsule(left_larmDesc, Point(0.01f, -0.01f, 0.01f), Point(0.17f, -0.17f, 0.17f), 0.031f);

				//<geom name="left_hand" type="sphere" pos=".18 -.18 .18"  size="0.04"/>
				PINT_SPHERE_CREATE left_handDesc;
				SetupMujocoSphere(left_handDesc, Point(0.18f, -0.18f, 0.18f), 0.04f);

				//<body name="left_lower_arm" pos=".18 .18 -.18" >
				left_larmDesc.SetNext(&left_handDesc);
				PINT_OBJECT_CREATE ObjectDesc;
				SetupLink(ObjectDesc, "left_lower_arm", Point(0.18f, 0.18f, -0.18f), &left_larmDesc, 1.2f);
				MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = left_upper_arm;
					if(1)
					{
						ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

						ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

						Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
						ArticulatedDesc.mLocalPivot0.mPos = wp - left_upper_armPos;

	//                    <joint name="left_elbow" type="hinge" pos="0 0 0" axis="0 -1 -1" range="-90 50" stiffness="0" armature="0.0028" />
						ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
						ArticulatedDesc.mAxisIndex = Z_;
						ArticulatedDesc.mMinLimit = -90.0f * DEGTORAD;
						ArticulatedDesc.mMaxLimit = 50.0f * DEGTORAD;
					}
					SetupArticBodyDesc(ArticulatedDesc);
					left_lower_arm = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
					left_lower_arm = CreatePintObject(pint, ObjectDesc);

				MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
	left_lower_armPos = ObjectDesc.mPosition;
			}
		}

		if(RCA)
			pint.AddRCArticulationToScene(RCA);

		return true;
	}

END_TEST(MujocoHumanoid)

///////////////////////////////////////////////////////////////////////////////

// Adapted from https://github.com/google-deepmind/mujoco/issues/172
static const char* gDesc_FourBarLinkage = "4-bar linkage (closed loop RCA using loop joints).";

START_TEST(FourBarLinkage, CATEGORY_RCARTICULATIONS, gDesc_FourBarLinkage)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-6.52f, 3.94f, 4.43f), Point(0.91f, -0.17f, -0.38f));
	    //<option gravity="0 0 0" timestep="0.002" />
		desc.mGravity = Point(0.0f, 0.0f, 0.0f);
		desc.mTimestep	= 0.002f;
		//desc.mNbSimulateCallsPerFrame	= udword((1.0f/60.0f) / desc.mTimestep);
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()	override
	{
		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()	override
	{
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)	override
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation || !caps.mSupportCompounds || !caps.mSupportCylinders)
			return false;

		MuJoCoConverter compiler(pint);
		//compiler.mJointMapping = reinterpret_cast<JointMapping*>(pint.mUserData);
		compiler.mUseRCA = true;
		compiler.mUseRobotAggregate = false;
		compiler.mShowCollisionShapes = true;
		compiler.mShowVisualMeshes  = true;
		//compiler.mSuperAggregate = super_aggregate;
		//compiler.mStiffness = Stiffness;
		//compiler.mDamping = Damping;

		if(compiler.StartModel(true))
		{
			// In MuJoCo's XML files the quats appear in WXYZ order by default (what MuJoCo uses internally).
			// So the identity quat in these files is "1 0 0 0". The ICE quaternion uses the same format, so
			// creating a Quat with the same quat data as in the file (same order) already converts from the
			// MuJoCo ordering to the ICE ordering.
			// Note that I'm talking about the ordering in the Quat ctor, the memory order is different...

			Quat q;

			//<body name="link_1" pos="0 0 0">
			compiler.StartBody("link_1", Point(0.0f, 0.0f, 0.0f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f);

				//<geom type="cylinder" size=".2    2" pos="0 0 2" euler="0 0 0" material="red__" />
				q = compiler.FromEuler(0.0f, 0.0f, 0.0f);
				compiler.AddCylinderShape(0.2f, 2.0f, &Point(0.0f, 0.0f, 2.0f), &q);

				//<geom type="cylinder" size=".25 .25" pos="0 0 4" euler="0 90 0" material="red__" />
				q = compiler.FromEuler(0.0f, 90.0f, 0.0f);
				compiler.AddCylinderShape(0.25f, 0.25f, &Point(0.0f, 0.0f, 4.0f), &q);

				//<geom type="cylinder" size=".25 .25" pos="0 0 0" euler="0 90 0" material="red__" />
				q = compiler.FromEuler(0.0f, 90.0f, 0.0f);
				compiler.AddCylinderShape(0.25f, 0.25f, &Point(0.0f, 0.0f, 0.0f), &q);

				//<body name="link_2" pos="0.5 0 0" euler="0 0 0">
				compiler.StartBody("link_2", Point(0.5f, 0.0f, 0.0f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f);

					//<joint name="hinge_1" pos="0 0 0" axis="1 0 0" />
					compiler.SetHingeJoint("hinge_1", Point(1.0f, 0.0f, 0.0f), PI, -PI);	// ### limits

					//<geom type="cylinder" size=".2    2" pos="0 2 0" euler="90 0 0" material="blue_" />
					q = compiler.FromEuler(90.0f, 0.0f, 0.0f);
					compiler.AddCylinderShape(0.2f, 2.0f, &Point(0.0f, 2.0f, 0.0f), &q);

					//<geom type="cylinder" size=".25 .25" pos="0 4 0" euler="0 90 0" material="blue_" />
					q = compiler.FromEuler(0.0f, 90.0f, 0.0f);
					compiler.AddCylinderShape(0.25f, 0.25f, &Point(0.0f, 4.0f, 0.0f), &q);

					//<geom type="cylinder" size=".25 .25" pos="0 0 0" euler="0 90 0" material="blue_" />
					q = compiler.FromEuler(0.0f, 90.0f, 0.0f);
					compiler.AddCylinderShape(0.25f, 0.25f, &Point(0.0f, 0.0f, 0.0f), &q);

					//<body name="link_3" pos="-0.5 4 0" euler="0 0 0">
					compiler.StartBody("link_3", Point(-0.5f, 4.0f, 0.0f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f);

						//<joint name="hinge_2" pos="0 0 0" axis="1 0 0" />
						compiler.SetHingeJoint("hinge_2", Point(1.0f, 0.0f, 0.0f), PI, -PI);	// ### limits

						//<geom type="cylinder" size=".2    2" pos="0 0 2" euler="0 0 0" material="green" />
						q = compiler.FromEuler(0.0f, 0.0f, 0.0f);
						compiler.AddCylinderShape(0.2f, 2.0f, &Point(0.0f, 0.0f, 2.0f), &q);

						//<geom type="cylinder" size=".25 .25" pos="0 0 0" euler="0 90 0" material="green" />
						q = compiler.FromEuler(0.0f, 90.0f, 0.0f);
						compiler.AddCylinderShape(0.25f, 0.25f, &Point(0.0f, 0.0f, 0.0f), &q);

						//<geom type="cylinder" size=".25 .25" pos="0 0 4" euler="0 90 0" material="green" />
						q = compiler.FromEuler(0.0f, 90.0f, 0.0f);
						compiler.AddCylinderShape(0.25f, 0.25f, &Point(0.0f, 0.0f, 4.0f), &q);

						//<body name="link_4" pos="0.5 0 4" euler="0 0 0">
						compiler.StartBody("link_4", Point(0.5f, 0.0f, 4.0f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f);

							//<joint name="hinge_3" pos="0 0 0" axis="1 0 0" />
							compiler.SetHingeJoint("hinge_3", Point(1.0f, 0.0f, 0.0f), PI, -PI);	// ### limits

							//<geom type="cylinder" size=".2    2" pos="0 -2 0" euler="90 0 0" material="white" />
							q = compiler.FromEuler(90.0f, 0.0f, 0.0f);
							compiler.AddCylinderShape(0.2f, 2.0f, &Point(0.0f, -2.0f, 0.0f), &q);

							//<geom type="cylinder" size=".25 .25" pos="0 0 0" euler="0 90 0" material="white" />
							q = compiler.FromEuler(0.0f, 90.0f, 0.0f);
							compiler.AddCylinderShape(0.25f, 0.25f, &Point(0.0f, 0.0f, 0.0f), &q);

							//<geom type="cylinder" size=".25 .25" pos="0 -4 0" euler="0 90 0" material="white" />
							q = compiler.FromEuler(0.0f, 90.0f, 0.0f);
							compiler.AddCylinderShape(0.25f, 0.25f, &Point(0.0f, -4.0f, 0.0f), &q);

						//</body>
						compiler.EndBody();

					//</body>
					compiler.EndBody();

				//</body>
				compiler.EndBody();

			//</body>
			compiler.EndBody();

			compiler.EndModel();

			//<equality>
			//<connect name="kinematic_link" active="true" body1="link_1" body2="link_4" anchor="0 0 4" />
			compiler.AddEqualityConstraint("kinematic_link", "link_1", "link_4", Point(0.0f, 0.0f, 4.0f));
			//</equality>
		}

		return true;
	}

	virtual void	Close(Pint& pint)	override
	{
		TestBase::Close(pint);
	}

END_TEST(FourBarLinkage)

///////////////////////////////////////////////////////////////////////////////

#include "Loader_OBJ.h"
#include "TextureManager.h"
#include "Devil.h"

static const char* gDesc_AnymalC = "Anybotics Anymal C";

#define NB_ACTUATORS_ANYMALC	12

START_TEST(AnymalC, CATEGORY_RCARTICULATIONS, gDesc_AnymalC)

	WavefrontDatabase	mOBJ;
	CheckBoxPtr			mCheckBox_UseRCA;
	CheckBoxPtr			mCheckBox_UseRobotAggregate;
	//CheckBoxPtr			mCheckBox_UseWorldAggregate;
	CheckBoxPtr			mCheckBox_ApplyTestForces;
	CheckBoxPtr			mCheckBox_AddConvexGround;
	CheckBoxPtr			mCheckBox_ShowCollisionShapes;
	CheckBoxPtr			mCheckBox_ShowVisualMeshes;
	CheckBoxPtr			mCheckBox_ShowTextures;
	EditBoxPtr			mEditBox_NbEnvs;
	EditBoxPtr			mEditBox_Stiffness;
	EditBoxPtr			mEditBox_Damping;
	ComboBoxPtr			mComboBox;
	SliderPtr			mSlider;
	float				mActuatorTarget[NB_ACTUATORS_ANYMALC];
	udword				mCurrentSlider;

	void ResetActuators()
	{
		for(udword i=0; i<NB_ACTUATORS_ANYMALC; i++)
			mActuatorTarget[i] = 0.0f;
	}

	class JointMapping : public JointMapingCallback, public Allocateable
	{
		public:
				JointMapping()	{}
		virtual	~JointMapping()	{}

		virtual	void	RegisterJointMapping(const char* name, PintActorHandle handle)	override
		{
			Mapping* M = ICE_RESERVE(Mapping, mData);
			M->mName	= name;
			M->mHandle	= handle;
		}

		struct Mapping
		{
			const char*		mName;
			PintActorHandle	mHandle;
		};
		Container	mData;
	};

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)	override
	{
		udword extra_size = 350;
		const char* name = "AnymalC";

		const sdword Width = 300;
		IceWindow* UI = CreateTestWindow(Width, 200 + extra_size);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 100;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;

		mCheckBox_UseRCA = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Use RCA", &UIElems, true, null, null);
		y += YStep;

		mCheckBox_UseRobotAggregate = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Use robot aggregate", &UIElems, true, null, null);
		y += YStep;

		//mCheckBox_UseWorldAggregate = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Use world aggregate", &UIElems, false, null, null);
		//y += YStep;

		mCheckBox_ApplyTestForces = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Apply test forces", &UIElems, false, null, null);
		y += YStep;

		mCheckBox_AddConvexGround = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Add convex ground", &UIElems, false, null, null);
		y += YStep;

		mCheckBox_ShowCollisionShapes = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Show collision shapes", &UIElems, false, null, null);
		y += YStep;

		mCheckBox_ShowVisualMeshes = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Show visual meshes", &UIElems, true, null, null);
		y += YStep;

		mCheckBox_ShowTextures = helper.CreateCheckBox(UI, 0, 4, y, 150, 20, "Show textures", &UIElems, true, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "sqrt(nb envs):", &UIElems);
		mEditBox_NbEnvs = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Stiffness:", &UIElems);
		mEditBox_Stiffness = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "500.0", &UIElems, EDITBOX_FLOAT, null, null);
		y += YStep;

		helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Damping:", &UIElems);
		mEditBox_Damping = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "5.0", &UIElems, EDITBOX_FLOAT, null, null);
		y += YStep;

		class LocalComboBox : public IceComboBox
		{
			AnymalC&		mTest;
			public:
							LocalComboBox(const ComboBoxDesc& desc, AnymalC& test) : IceComboBox(desc), mTest(test)	{}
			virtual			~LocalComboBox()																		{}

			virtual	void	OnComboBoxEvent(ComboBoxEvent event)
			{
				if(event==CBE_SELECTION_CHANGED)
				{
					const udword SelectedIndex = GetSelectedIndex();
					if(mTest.mSlider)
					{
						mTest.mSlider->SetValue(mTest.mActuatorTarget[SelectedIndex]);
						mTest.mCurrentSlider = SelectedIndex;
					}
				}
			}
		};

		y += 4;
		ComboBoxDesc CBBD;
		CBBD.mID		= 0;
		CBBD.mParent	= UI;
		CBBD.mX			= 4;
		CBBD.mY			= y;
		CBBD.mWidth		= 100;
		CBBD.mHeight	= 20;
		CBBD.mLabel		= "Presets";
		mComboBox = ICE_NEW(LocalComboBox)(CBBD, *this);
		RegisterUIElement(mComboBox);
		mComboBox->Add("LF_HAA");
		mComboBox->Add("LF_HFE");
		mComboBox->Add("LF_KFE");
		mComboBox->Add("RF_HAA");
		mComboBox->Add("RF_HFE");
		mComboBox->Add("RF_KFE");
		mComboBox->Add("LH_HAA");
		mComboBox->Add("LH_HFE");
		mComboBox->Add("LH_KFE");
		mComboBox->Add("RH_HAA");
		mComboBox->Add("RH_HFE");
		mComboBox->Add("RH_KFE");
		mComboBox->Select(0);
		mComboBox->SetVisible(true);

		mCurrentSlider = 0;
		{
			SliderDesc SD;
			SD.mStyle	= SLIDER_HORIZONTAL;
			SD.mID		= 0;
			SD.mParent	= UI;
			SD.mX		= 110;
			SD.mY		= y;
			SD.mWidth	= Width/2;//Width - 30;
			SD.mHeight	= 30;
			SD.mLabel	= "test";
			mSlider		= ICE_NEW(IceSlider)(SD);
			mSlider->SetRange(-2.0f, 2.0f, 100);
			mSlider->SetValue(0.0f);
			UIElems.Register(mSlider);
			y += YStep;
		}

		y += YStep;

		{
			class LocalButton : public IceButton
			{
				public:
									LocalButton(AnymalC& test, const ButtonDesc& desc) : IceButton(desc), mTest(test)	{}
				virtual				~LocalButton()																		{}

				virtual	void		OnClick()
				{
					mTest.mSlider->SetValue(0.0f);
					mTest.ResetActuators();
				}

						AnymalC&	mTest;
			};

			ButtonDesc BD;
			BD.mID			= 0;
			BD.mParent		= UI;
			BD.mX			= 4;
			BD.mY			= y;
			BD.mWidth		= Width - 24;
			BD.mHeight		= 20;
			BD.mLabel		= "Reset actuators";
			IceButton* IB = ICE_NEW(LocalButton)(*this, BD);
			RegisterUIElement(IB);
			IB->SetVisible(true);
		}

		y += YStep;
		y += YStep;

		return CreateTestTabControlAndResetButton(UI, Width, y, extra_size);

		//return CreateOverrideTabControl("AnymalC", 100);
	}

	virtual	float	GetRenderData(Point& center)	const	override	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-0.43f, 1.60f, 0.42f), Point(-0.52f, -0.41f, -0.75f));
		SetDefEnv(desc, true);
	}

	PintShapeRenderer*	LoadMesh(const char* mesh_name, const char* texture_name, const WavefrontLoaderParams& Params)
	{
		const ManagedTexture* MT = null;
		if(texture_name && mCheckBox_ShowTextures->IsChecked())
		{
			const udword NbToSearch = GetNbManagedTextures();
			for(udword i=0; i<NbToSearch; i++)
			{
				const ManagedTexture* Candidate = GetManagedTexture(i);
				if(Candidate->mFilename == texture_name)
				{
					MT = Candidate;
					printf("Found already existing texture %s ...", texture_name);
					break;
				}
			}

			if(!MT)
			{
				const char* Filename = FindPEELFile(texture_name);
				if(Filename)
				{
					printf("Loading new texture %s ...", texture_name);
					Picture Pic;
					if(LoadWithDevil(Filename, Pic))
						MT = CreateManagedTexture(Pic.GetWidth(), Pic.GetHeight(), Pic.GetPixels(), texture_name);
				}
			}
		}

		LoadObj(mesh_name, Params, mOBJ);

		PintShapeRenderer* renderer = null;

		const udword NbMeshes = mOBJ.mMeshes.GetNbEntries();
		if(NbMeshes)
		{
			const WavefrontMesh* Mesh = (const WavefrontMesh*)mOBJ.mMeshes[0];
			if(MT && Mesh->GetNbTVerts())
			{
				MultiSurface MS;
				MS.IndexedSurface::Init(Mesh->GetNbTris(), Mesh->GetNbVerts(), Mesh->GetVerts(), (const IndexedTriangle*)Mesh->GetIndices());
				{
					IndexedSurface* UVSurface = MS.AddExtraSurface(SURFACE_UVS);
					//UVSurface->Init(Mesh->GetNbTTris(), Mesh->GetNbTVerts());
					UVSurface->Init(Mesh->GetNbTris(), Mesh->GetNbTVerts());

					IndexedTriangle* TFaces = (IndexedTriangle*)UVSurface->GetFaces();
					//CopyMemory(TFaces, Mesh->GetTIndices(), Mesh->GetNbTTris()*sizeof(IndexedTriangle));
					CopyMemory(TFaces, Mesh->GetIndices(), Mesh->GetNbTris()*sizeof(IndexedTriangle));

					Point* uvs = (Point*)UVSurface->GetVerts();
					CopyMemory(uvs, Mesh->GetTVerts(), Mesh->GetNbTVerts()*sizeof(Point));
				}

	//			if(Actor->mMaterial && Actor->mMaterial->mEffect && Actor->mMaterial->mEffect->mImage)
	//				MT = Actor->mMaterial->mEffect->mImage->mTexture;
				renderer = CreateMeshRenderer(MS, true, true);
			}
			else
			{
				const SurfaceInterface SI(Mesh->GetNbVerts(), Mesh->GetVerts(), Mesh->GetNbTris(), Mesh->GetIndices(), null);
				renderer = CreateMeshRenderer(PintSurfaceInterface(SI), null, true, true);
			}
		}

		mOBJ.Release();

		if(renderer)
		{
			const RGBAColor Diffuse(1.0f, 1.0f, 1.0f, 1.0f);
			renderer = CreateColorShapeRenderer(renderer, Diffuse, MT);
		}

		return renderer;
	}

	PintShapeRenderer*	mMeshes[32];	//####

	virtual bool	CommonSetup()	override
	{
		ResetActuators();
		if(mSlider)
			mSlider->SetValue(0.0f);

		WavefrontLoaderParams Params;
		Params.mScale = 1.0f;
		Params.mMergeMeshes = true;
		Params.mRecenter = false;
		Params.mFlipYZ = true;

		if(mCheckBox_ShowVisualMeshes->IsChecked())
		{
			mMeshes[0] = LoadMesh("anybotics_anymal_c/assets/base_0.obj", "anybotics_anymal_c/assets/base.png", Params);
			mMeshes[1] = LoadMesh("anybotics_anymal_c/assets/base_1.obj", "anybotics_anymal_c/assets/base.png", Params);
			mMeshes[2] = LoadMesh("anybotics_anymal_c/assets/base_2.obj", "anybotics_anymal_c/assets/base.png", Params);
			mMeshes[3] = LoadMesh("anybotics_anymal_c/assets/base_3.obj", "anybotics_anymal_c/assets/base.png", Params);
			mMeshes[4] = LoadMesh("anybotics_anymal_c/assets/base_4.obj", "anybotics_anymal_c/assets/base.png", Params);
			mMeshes[5] = LoadMesh("anybotics_anymal_c/assets/base_5.obj", "anybotics_anymal_c/assets/base.png", Params);

			mMeshes[6] = LoadMesh("anybotics_anymal_c/assets/top_shell.obj", "anybotics_anymal_c/assets/top_shell.png", Params);
			mMeshes[7] = LoadMesh("anybotics_anymal_c/assets/bottom_shell.obj", "anybotics_anymal_c/assets/bottom_shell.png", Params);
			mMeshes[8] = LoadMesh("anybotics_anymal_c/assets/remote.obj", "anybotics_anymal_c/assets/remote.png", Params);
			mMeshes[9] = LoadMesh("anybotics_anymal_c/assets/handle.obj", "anybotics_anymal_c/assets/handle.png", Params);
			mMeshes[10] = LoadMesh("anybotics_anymal_c/assets/face.obj", "anybotics_anymal_c/assets/face.png", Params);

			mMeshes[11] = LoadMesh("anybotics_anymal_c/assets/depth_camera.obj", "anybotics_anymal_c/assets/depth_camera.png", Params);
			mMeshes[12] = LoadMesh("anybotics_anymal_c/assets/wide_angle_camera.obj", "anybotics_anymal_c/assets/wide_angle_camera.png", Params);

			mMeshes[13] = LoadMesh("anybotics_anymal_c/assets/battery.obj", "anybotics_anymal_c/assets/battery.png", Params);
			mMeshes[14] = LoadMesh("anybotics_anymal_c/assets/lidar_cage.obj", "anybotics_anymal_c/assets/lidar_cage.png", Params);
			mMeshes[15] = LoadMesh("anybotics_anymal_c/assets/lidar.obj", "anybotics_anymal_c/assets/lidar.png", Params);
			mMeshes[16] = LoadMesh("anybotics_anymal_c/assets/drive.obj", "anybotics_anymal_c/assets/drive.png", Params);
			mMeshes[17] = LoadMesh("anybotics_anymal_c/assets/hatch.obj", "anybotics_anymal_c/assets/hatch.png", Params);

			mMeshes[18] = LoadMesh("anybotics_anymal_c/assets/hip_l.obj", "anybotics_anymal_c/assets/hip_l.png", Params);
			mMeshes[19] = LoadMesh("anybotics_anymal_c/assets/hip_r.obj", "anybotics_anymal_c/assets/hip_r.png", Params);
			mMeshes[20] = LoadMesh("anybotics_anymal_c/assets/thigh.obj", "anybotics_anymal_c/assets/thigh.png", Params);
			mMeshes[21] = LoadMesh("anybotics_anymal_c/assets/shank_l.obj", "anybotics_anymal_c/assets/shank_l.png", Params);
			mMeshes[22] = LoadMesh("anybotics_anymal_c/assets/shank_r.obj", "anybotics_anymal_c/assets/shank_r.png", Params);
			mMeshes[23] = LoadMesh("anybotics_anymal_c/assets/foot.obj", "anybotics_anymal_c/assets/foot.png", Params);
		}
		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()	override
	{
		mOBJ.Release();
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)	override
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation || !caps.mSupportCompounds || !caps.mSupportCylinders)
			return false;

		const udword NbEnvs = GetInt(1, mEditBox_NbEnvs);

		PintAggregateHandle SuperAggregate = null;//pint.CreateAggregate(128*Nb*Nb, false);

		const bool AddConvex = mCheckBox_AddConvexGround->IsChecked() && caps.mSupportConvexes;

		PINT_CONVEX_CREATE ConvexCreate;
		MyConvex C;
		if(AddConvex)
		{
			C.LoadFile(4);
			C.Scale(2.0f, 1.0f, 2.0f);
			ConvexCreate.mNbVerts	= C.mNbVerts;
			ConvexCreate.mVerts		= C.mVerts;
			ConvexCreate.mRenderer	= CreateRenderer(ConvexCreate);
		}

		JointMapping* JP = ICE_NEW(JointMapping);
		pint.mUserData = JP;

		BasicRandom Rnd(42);
		//const float Altitude = AddConvex ? 0.5f : 0.0f;
		const float Altitude = -1.0f;
		//const float Spacing = 2.0f;
		const float Spacing = 3.0f;
		const float Offset = 0.5f * (float(NbEnvs) * Spacing);
		for(udword j=0; j<NbEnvs; j++)
		{
			for(udword i=0; i<NbEnvs; i++)
			{
				const float RndX = Rnd.RandomFloat();
				const float RndY = Rnd.RandomFloat();
				const Point Pos(RndX + float(i)*Spacing - Offset, RndY + float(j)*Spacing - Offset, Altitude);
				CreateRobot(pint, caps, Pos, SuperAggregate);

				if(AddConvex)
					CreateStaticObject(pint, &ConvexCreate, Point(Pos.x, 0.0f + Rnd.RandomFloat() * 0.5f, Pos.y));
			}
		}

		if(SuperAggregate)
			pint.AddAggregateToScene(SuperAggregate);

		return true;
	}

	virtual void	Close(Pint& pint)	override
	{
		JointMapping* JM = reinterpret_cast<JointMapping*>(pint.mUserData);
		DELETESINGLE(JM);
		pint.mUserData = null;

		TestBase::Close(pint);
	}

	bool CreateRobot(Pint& pint, const PintCaps& caps, const Point& offset, PintAggregateHandle super_aggregate = null)
	{
		if(0)
		{
			{
				MuJoCoConverter compiler(pint);
				if(compiler.StartModel(false))
				{
					//<body name="cylinder1" pos="0 0 0.62" childclass="cylinder">
					compiler.StartBody("base", Point(0.0f, 0.0f, 0.62f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f);

						//<geom class="collision" size="0.07 0.06"/>
						compiler.AddCylinderShape(0.07f, 0.06f);
					compiler.EndBody();
					compiler.EndModel();
				}
			}
			{
				MuJoCoConverter compiler(pint);
				if(compiler.StartModel(false))
				{
					//<body name="cylinder2" pos="0.4 0 0.62" quat="0 0 0 1" childclass="cylinder">
					compiler.StartBody("base", Point(0.4f, 0.0f, 0.62f), Quat(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);

						//<geom class="collision" size="0.07 0.06"/>
						compiler.AddCylinderShape(0.07f, 0.06f);
					compiler.EndBody();
					compiler.EndModel();
				}
			}
		}

		if(1)
		{
		MuJoCoConverter compiler(pint);
		compiler.mJointMapping = reinterpret_cast<JointMapping*>(pint.mUserData);
		compiler.mUseRCA = mCheckBox_UseRCA->IsChecked();
		compiler.mUseRobotAggregate = mCheckBox_UseRobotAggregate->IsChecked();
		//compiler.mUseWorldAggregate = mCheckBox_UseWorldAggregate->IsChecked();
		compiler.mShowCollisionShapes = mCheckBox_ShowCollisionShapes->IsChecked();
		compiler.mShowVisualMeshes  = mCheckBox_ShowVisualMeshes->IsChecked();
		compiler.mSuperAggregate = super_aggregate;
		const float Stiffness = GetFloat(0.0f, mEditBox_Stiffness);
		const float Damping = GetFloat(0.0f, mEditBox_Damping);
		compiler.mStiffness = Stiffness;
		compiler.mDamping = Damping;

		if(!compiler.StartModel(false))
			return false;

		// <compiler angle="radian" autolimits="true"/>

/*
  <default>
    <default class="anymal_c">
      <joint damping="1" frictionloss="0.1"/>
 
      <default class="collision">
        <geom group="2" type="cylinder"/>
        <default class="foot">
          <geom type="sphere" size="0.03" pos="0 0 0.02325" priority="1" solimp="0.015 1 0.03" condim="6"
            friction="0.8 0.02 0.01"/>
        </default>
      </default>

      <default class="affine">
		<position kp="100" ctrlrange="-6.28 6.28" forcerange="-80 80"/>
      </default>
    </default>
  </default>
*/
			const Quat visual_zflip(0.0f, 0.0f, 0.0f, 1.0f);

			Matrix3x3 initRot;
			initRot.RotZ(PI/4.0f);
			Quat qq = initRot;
			qq.Identity();

			//<body name="base" pos="0 0 0.62" quat="0 0 0 1" childclass="anymal_c">
			//compiler.StartBody("base", offset + Point(0.0f, 0.0f, 0.62f), Quat(0.0f, 0.0f, 0.0f, 1.0f), 19.2035f);
			compiler.StartBody("base", offset + Point(0.0f, 0.0f, 2.62f), qq, 19.2035f);
			//<freejoint/>
			//<inertial mass="19.2035" pos="0.0025 0 0.0502071" quat="0.5 0.5 0.5 0.5" diaginertia="0.639559 0.624031 0.217374"/>
			compiler.SetInertia(19.2035f, Point(0.0025f, 0.0f, 0.0502071f), Quat(0.5f, 0.5f, 0.5f, 0.5f), Point(0.639559f, 0.624031f, 0.217374f));

				//<geom mesh="base_0" material="green" class="visual"/>
				//<geom mesh="base_1" material="yellow" class="visual"/>
				//<geom mesh="base_2" material="red" class="visual"/>
				//<geom mesh="base_3" material="black_plastic" class="visual"/>
				//<geom mesh="base_4" material="lwl" class="visual"/>
				//<geom mesh="base_5" material="base" class="visual"/>
				compiler.AddVisual(mMeshes[0]);
				compiler.AddVisual(mMeshes[1]);
				compiler.AddVisual(mMeshes[2]);
				compiler.AddVisual(mMeshes[3]);
				compiler.AddVisual(mMeshes[4]);
				compiler.AddVisual(mMeshes[5]);

			//<geom class="collision" size="0.29 0.07 0.09" type="box"/>
			compiler.AddBoxShape(0.29f, 0.07f, 0.09f);
			//<geom class="collision" size="0.09 0.0725" pos="0.2175 0.07 0" quat="1 0 1 0"/>
			compiler.AddCylinderShape(0.09f, 0.0725f, &Point(0.2175f, 0.07f, 0.0f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));
			//<geom class="collision" size="0.09 0.0725" pos="-0.2175 0.07 0" quat="1 0 1 0"/>
			compiler.AddCylinderShape(0.09f, 0.0725f, &Point(-0.2175f, 0.07f, 0.0f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));
			//<geom class="collision" size="0.09 0.0725" pos="0.2175 -0.07 0" quat="1 0 1 0"/>
			compiler.AddCylinderShape(0.09f, 0.0725f, &Point(0.2175f, -0.07f, 0.0f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));
			//<geom class="collision" size="0.09 0.0725" pos="-0.2175 -0.07 0" quat="1 0 1 0"/>
			compiler.AddCylinderShape(0.09f, 0.0725f, &Point(-0.2175f, -0.07f, 0.0f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));

				//<geom material="top_shell" mesh="top_shell" class="visual"/>
				//<geom material="bottom_shell" mesh="bottom_shell" class="visual"/>
				//<geom material="remote" mesh="remote" class="visual"/>
				//<geom material="handle" mesh="handle" class="visual"/>
				//<geom pos="0.4145 0 0" material="face" mesh="face" class="visual"/>
				compiler.AddVisual(mMeshes[6]);
				compiler.AddVisual(mMeshes[7]);
				compiler.AddVisual(mMeshes[8]);
				compiler.AddVisual(mMeshes[9]);
				compiler.AddVisual(mMeshes[10], &Point(0.4145f, 0.0f, 0.0f));

			//<geom class="collision" size="0.055 0.07 0.09" pos="0.4695 0 0" type="box"/>
			compiler.AddBoxShape(0.055f, 0.07f, 0.09f, &Point(0.4695f, 0.0f, 0.0f));
			//<geom class="collision" size="0.09 0.055" pos="0.4695 0.07 0" quat="1 0 1 0"/>
			compiler.AddCylinderShape(0.09f, 0.055f, &Point(0.4695f, 0.07f, 0.0f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));
			//<geom class="collision" size="0.09 0.055" pos="0.4695 -0.07 0" quat="1 0 1 0"/>
			compiler.AddCylinderShape(0.09f, 0.055f, &Point(0.4695f, -0.07f, 0.0f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));

				//<geom pos="0.46165 0 -0.0292" quat="0.965926 0 0.258819 0" material="depth_camera" mesh="depth_camera" class="visual"/>
				compiler.AddVisual(mMeshes[11], &Point(0.46165f, 0.0f, -0.0292f), &Quat(0.965926f, 0.0f, 0.258819f, 0.0f));

				//<geom pos="0.513 0 0.01497" material="wide_angle_camera" mesh="wide_angle_camera" class="visual"/>
				compiler.AddVisual(mMeshes[12], &Point(0.513f, 0.0f, 0.01497f));

				//<geom pos="-0.4145 0 0" material="face" mesh="face" class="visual_zflip"/>
				compiler.AddVisual(mMeshes[10], &Point(-0.4145f, 0.0f, 0.0f), &visual_zflip);

			//<geom class="collision" size="0.055 0.07 0.09" pos="-0.4695 0 0" quat="0 0 0 1" type="box"/>
			compiler.AddBoxShape(0.055f, 0.07f, 0.09f, &Point(-0.4695f, 0.0f, 0.0f), &Quat(0.0f, 0.0f, 0.0f, 1.0f));
			//<geom class="collision" size="0.09 0.055" pos="-0.4695 -0.07 0" quat="0 -1 0 1"/>
			compiler.AddCylinderShape(0.09f, 0.055f, &Point(-0.4695f, -0.07f, 0.0f), &Quat(0.0f, -1.0f, 0.0f, 1.0f));
			//<geom class="collision" size="0.09 0.055" pos="-0.4695 0.07 0" quat="0 -1 0 1"/>
			compiler.AddCylinderShape(0.09f, 0.055f, &Point(-0.4695f, 0.07f, 0.0f), &Quat(0.0f, -1.0f, 0.0f, 1.0f));

				//<geom pos="-0.46165 0 -0.0292" quat="0 -0.258819 0 0.965926" material="depth_camera" mesh="depth_camera" class="visual"/>
				compiler.AddVisual(mMeshes[11], &Point(-0.46165f, 0.0f, -0.0292f), &Quat(0.0f, -0.258819f, 0.0f, 0.965926f));

				//<geom pos="-0.513 0 0.01497" material="wide_angle_camera" mesh="wide_angle_camera" class="visual_zflip"/>
				compiler.AddVisual(mMeshes[12], &Point(-0.513f, 0.0f, 0.01497f), &visual_zflip);

				//<geom material="battery" mesh="battery" class="visual"/>
				compiler.AddVisual(mMeshes[13]);

				//<geom pos="0 0.07646 0.02905" quat="0.683013 -0.183013 0.183013 0.683013" material="depth_camera" mesh="depth_camera" class="visual"/>
				compiler.AddVisual(mMeshes[11], &Point(0.0f, 0.07646f, 0.02905f), &Quat(0.683013f, -0.183013f, 0.183013f, 0.683013f));

				//<geom pos="0 -0.07646 0.02905" quat="0.683013 0.183013 0.183013 -0.683013" material="depth_camera" mesh="depth_camera" class="visual"/>
				compiler.AddVisual(mMeshes[11], &Point(0.0f, -0.07646f, 0.02905f), &Quat(0.683013f, 0.183013f, 0.183013f, -0.683013f));

				//<geom pos="-0.364 0 0.0735" material="lidar_cage" mesh="lidar_cage" class="visual"/>
				compiler.AddVisual(mMeshes[14], &Point(-0.364f, 0.0f, 0.0735f));

			//<geom class="collision" size="0.07 0.06" pos="-0.364 0 0.1335"/>
			compiler.AddCylinderShape(0.07f, 0.06f, &Point(-0.364f, 0.0f, 0.1335f));

				//<geom pos="-0.364 0 0.1422" quat="1 0 0 -1" material="lidar" mesh="lidar" class="visual"/>
				compiler.AddVisual(mMeshes[15], &Point(-0.364f, 0.0f, 0.1422f), &Quat(1.0f, 0.0f, 0.0f, -1.0f));

				//<geom pos="0.2999 0.104 0" quat="0.258819 0.965926 0 0" material="drive" mesh="drive" class="visual"/>
				compiler.AddVisual(mMeshes[16], &Point(0.2999f, 0.104f, 0.0f), &Quat(0.258819f, 0.965926f, 0.0f, 0.0f));

				//<geom pos="0.2999 -0.104 0" quat="0.258819 -0.965926 0 0" material="drive" mesh="drive" class="visual"/>
				compiler.AddVisual(mMeshes[16], &Point(0.2999f, -0.104f, 0.0f), &Quat(0.258819f, -0.965926f, 0.0f, 0.0f));

				//<geom pos="-0.2999 0.104 0" quat="0 0 0.965926 -0.258819" material="drive" mesh="drive" class="visual"/>
				compiler.AddVisual(mMeshes[16], &Point(-0.2999f, 0.104f, 0.0f), &Quat(0.0f, 0.0f, 0.965926f, -0.258819f));

				//<geom pos="-0.2999 -0.104 0" quat="0 0 -0.965926 -0.258819" material="drive" mesh="drive" class="visual"/>
				compiler.AddVisual(mMeshes[16], &Point(-0.2999f, -0.104f, 0.0f), &Quat(0.0f, 0.0f, -0.965926f, -0.258819f));

				//<geom pos="0.116 0 0.073" material="hatch" mesh="hatch" class="visual"/>
				compiler.AddVisual(mMeshes[17], &Point(0.116f, 0.0f, 0.073f));

				//<body name="LF_HIP" pos="0.2999 0.104 0" quat="0.258819 0.965926 0 0">
				compiler.StartBody("LF_HIP", Point(0.2999f, 0.104f, 0.0f), Quat(0.258819f, 0.965926f, 0.0f, 0.0f), 2.781f);
				//compiler.StartBody("LF_HIP", Point(0.2999f, 0.104f, 0.0f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 2.781f);
				//<inertial mass="2.781" pos="0.0566606 -0.015294 -0.00829784" quat="-0.127978 0.709783 -0.135278 0.679359" diaginertia="0.00585729 0.00491868 0.00329081"/>
				compiler.SetInertia(2.781f, Point(0.0566606f, -0.015294f, -0.00829784f), Quat(-0.127978f, 0.709783f, -0.135278f, 0.679359f), Point(0.00585729f, 0.00491868f, 0.00329081f));
				//<joint name="LF_HAA" axis="1 0 0" range="-0.72 0.49"/>
				compiler.SetHingeJoint("LF_HAA", Point(1.0f, 0.0f, 0.0f), -0.72f, 0.49f);
		 
					//<geom quat="0.258819 -0.965926 0 0" material="hip_l" mesh="hip_l" class="visual"/>
					compiler.AddVisual(mMeshes[18], null, &Quat(0.258819f, -0.965926f, 0.0f, 0.0f));
					//<geom pos="0.0599 -0.0725816 -0.041905" quat="0.183013 -0.683013 0.683013 0.183013" material="drive" mesh="drive" class="visual"/>
					compiler.AddVisual(mMeshes[16], &Point(0.0599f, -0.0725816f, -0.041905f), &Quat(0.183013f, -0.683013f, 0.683013f, 0.183013f));

				//<geom class="collision" size="0.05 0.07" pos="0.0599 -0.0119598 -0.006905" quat="-0.353553 -0.612372 0.612372 -0.353553"/>
				compiler.AddCylinderShape(0.05f, 0.07f, &Point(0.0599f, -0.0119598f, -0.006905f), &Quat(-0.353553f, -0.612372f, 0.612372f, -0.353553f));

					//<body name="LF_THIGH" pos="0.0599 -0.0725816 -0.041905" quat="0.183013 -0.683013 0.683013 0.183013">
					compiler.StartBody("LF_THIGH", Point(0.0599f, -0.0725816f, -0.041905f), Quat(0.183013f, -0.683013f, 0.683013f, 0.183013f), 3.071f);
					//<inertial mass="3.071" pos="0.0308147 4.64995e-05 -0.245696" quat="0.993166 -0.00515309 -0.0806592 0.0841972" diaginertia="0.03025 0.0298943 0.00418465"/>
					compiler.SetInertia(3.071f, Point(0.0308147f, 4.64995e-05f, -0.245696f), Quat(0.993166f, -0.00515309f, -0.0806592f, 0.0841972f), Point(0.03025f, 0.0298943f, 0.00418465f));
					//<joint name="LF_HFE" axis="1 0 0" range="-9.42478 9.42478"/>
					compiler.SetHingeJoint("LF_HFE", Point(1.0f, 0.0f, 0.0f), -9.42478f, 9.42478f);
		  
						//<geom quat="1 0 0 -1" material="thigh" mesh="thigh" class="visual"/>
						compiler.AddVisual(mMeshes[20], null, &Quat(1.0f, 0.0f, 0.0f, -1.0f));

					//<geom class="collision" size="0.065 0.04" pos="0.04 0 0" quat="0.5 0.5 -0.5 -0.5"/>
					compiler.AddCylinderShape(0.065f, 0.04f, &Point(0.04f, 0.0f, 0.0f), &Quat(0.5f, 0.5f, -0.5f, -0.5f));
					//<geom class="collision" size="0.0375 0.03 0.141314" pos="0.03 0 -0.141314" quat="1 0 0 -1" type="box"/>
					compiler.AddBoxShape(0.0375f, 0.03f, 0.141314f, &Point(0.03f, 0.0f, -0.141314f), &Quat(1.0f, 0.0f, 0.0f, -1.0f));

						//<geom pos="0.1003 0 -0.285" material="drive" mesh="drive" class="visual"/>
						compiler.AddVisual(mMeshes[16], &Point(0.1003f, 0.0f, -0.285f));

					//<geom class="collision" size="0.05 0.07" pos="0.0303 0 -0.285" quat="1 0 1 0"/>
					compiler.AddCylinderShape(0.05f, 0.07f, &Point(0.0303f, 0.0f, -0.285f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));

						//<body name="LF_SHANK" pos="0.1003 0 -0.285">
						compiler.StartBody("LF_SHANK", Point(0.1003f, 0.0f, -0.285f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 0.58842f);
						//<inertial mass="0.58842" pos="0.005462 -0.0612528 -0.0806598" quat="0.992934 -0.115904 -0.00105487 -0.0254421" diaginertia="0.0101637 0.00923838 0.00111927"/>
						compiler.SetInertia(0.58842f, Point(0.005462f, -0.0612528f, -0.0806598f), Quat(0.992934f, -0.115904f, -0.00105487f, -0.0254421f), Point(0.0101637f, 0.00923838f, 0.00111927f));
						//<joint name="LF_KFE" axis="1 0 0" range="-9.42478 9.42478"/>
						compiler.SetHingeJoint("LF_KFE", Point(1.0f, 0.0f, 0.0f), -9.42478f, 9.42478f);

							//<geom quat="1 0 0 -1" material="shank_l" mesh="shank_l" class="visual"/>
							compiler.AddVisual(mMeshes[21], null, &Quat(1.0f, 0.0f, 0.0f, -1.0f));

						//<geom class="collision" size="0.06 0.02" pos="0.02 0 0" quat="0.5 0.5 -0.5 -0.5"/>
						compiler.AddCylinderShape(0.06f, 0.02f, &Point(0.02f, 0.0f, 0.0f), &Quat(0.5f, 0.5f, -0.5f, -0.5f));
						//<geom class="collision" size="0.057499 0.03375 0.019" pos="0.02 -0.057499 0" quat="0.5 0.5 -0.5 -0.5" type="box"/>
						compiler.AddBoxShape(0.057499f, 0.03375f, 0.019f, &Point(0.02f, -0.057499f, 0.0f), &Quat(0.5f, 0.5f, -0.5f, -0.5f));

							//<geom pos="0.01305 -0.08795 -0.33797" quat="0.382683 0 0 -0.92388" material="foot" mesh="foot" class="visual"/>
							compiler.AddVisual(mMeshes[23], &Point(0.01305f, -0.08795f, -0.33797f), &Quat(0.382683f, 0.0f, 0.0f, -0.92388f));

						//<geom class="collision" size="0.0175 0.141252" pos="0.01305 -0.08795 -0.168985" quat="1 0 0 -1"/>
						compiler.AddCylinderShape(0.0175f, 0.141252f, &Point(0.01305f, -0.08795f, -0.168985f), &Quat(1.0f, 0.0f, 0.0f, -1.0f));
						//<geom class="foot" pos="0.01305 -0.08795 -0.31547" quat="1 0 0 -1"/>
						compiler.AddSphereShape(0.03f, &Point(0.01305f, -0.08795f, -0.31547f), &Quat(1.0f, 0.0f, 0.0f, -1.0f));

						//</body>
						compiler.EndBody();
					//</body>
					compiler.EndBody();
				//</body>
				compiler.EndBody();

				//<body name="RF_HIP" pos="0.2999 -0.104 0" quat="0.258819 -0.965926 0 0">
				compiler.StartBody("RF_HIP", Point(0.2999f, -0.104f, 0.0f), Quat(0.258819f, -0.965926f, 0.0f, 0.0f), 2.781f);
				//<inertial mass="2.781" pos="0.0567633 0.015294 -0.00829784" quat="0.13524 0.679072 0.127985 0.710065" diaginertia="0.00585928 0.0049205 0.00329064"/>
				compiler.SetInertia(2.781f, Point(0.0567633f, 0.015294f, -0.00829784f), Quat(0.13524f, 0.679072f, 0.127985f, 0.710065f), Point(0.00585928f, 0.0049205f, 0.00329064f));
				//<joint name="RF_HAA" axis="1 0 0" range="-0.49 0.72"/>
				compiler.SetHingeJoint("RF_HAA", Point(1.0f, 0.0f, 0.0f), -0.49f, 0.72f);
		
					//<geom quat="0.258819 0.965926 0 0" material="hip_r" mesh="hip_r" class="visual"/>
					compiler.AddVisual(mMeshes[19], null, &Quat(0.258819f, 0.965926f, 0.0f, 0.0f));
					//<geom pos="0.0599 0.0725816 -0.041905" quat="0.183013 0.683013 0.683013 -0.183013" material="drive" mesh="drive" class="visual"/>
					compiler.AddVisual(mMeshes[16], &Point(0.0599f, 0.0725816f, -0.041905f), &Quat(0.183013f, 0.683013f, 0.683013f, -0.183013f));

				//<geom class="collision" size="0.05 0.07" pos="0.0599 0.0119598 -0.006905" quat="-0.353553 0.612372 0.612372 0.353553"/>*/
				compiler.AddCylinderShape(0.05f, 0.07f, &Point(0.0599f, 0.0119598f, -0.006905f), &Quat(-0.353553f, 0.612372f, 0.612372f, 0.353553f));

					//<body name="RF_THIGH" pos="0.0599 0.0725816 -0.041905" quat="0.183013 0.683013 0.683013 -0.183013">
					compiler.StartBody("RF_THIGH", Point(0.0599f, 0.0725816f, -0.041905f), Quat(0.183013f, 0.683013f, 0.683013f, -0.183013f), 3.071f);
					//<inertial mass="3.071" pos="0.0308147 4.64995e-05 -0.245696" quat="0.992775 -0.00512735 -0.0806685 0.0886811" diaginertia="0.0302511 0.0298933 0.0041845"/>
					compiler.SetInertia(3.071f, Point(0.0308147f, 4.64995e-05f, -0.245696f), Quat(0.992775f, -0.00512735f, -0.0806685f, 0.0886811f), Point(0.0302511f, 0.0298933f, 0.0041845f));
					//<joint name="RF_HFE" axis="-1 0 0" range="-9.42478 9.42478"/>
					compiler.SetHingeJoint("RF_HFE", Point(-1.0f, 0.0f, 0.0f), -9.42478f, 9.42478f);

						//<geom quat="1 0 0 -1" material="thigh" mesh="thigh" class="visual"/>
						compiler.AddVisual(mMeshes[20], null, &Quat(1.0f, 0.0f, 0.0f, -1.0f));

					//<geom class="collision" size="0.065 0.04" pos="0.04 0 0" quat="0.5 0.5 0.5 0.5"/>
					compiler.AddCylinderShape(0.065f, 0.04f, &Point(0.04f, 0.0f, 0.0f), &Quat(0.5f, 0.5f, 0.5f, 0.5f));
					//<geom class="collision" size="0.0375 0.03 0.141314" pos="0.03 0 -0.141314" quat="1 0 0 1" type="box"/>
					compiler.AddBoxShape(0.0375f, 0.03f, 0.141314f, &Point(0.03f, 0.0f, -0.141314f), &Quat(1.0f, 0.0f, 0.0f, 1.0f));

						//<geom pos="0.1003 0 -0.285" material="drive" mesh="drive" class="visual"/>
						compiler.AddVisual(mMeshes[16], &Point(0.1003f, 0.0f, -0.285f));

					//<geom class="collision" size="0.05 0.07" pos="0.0303 0 -0.285" quat="1 0 1 0"/>
					compiler.AddCylinderShape(0.05f, 0.07f, &Point(0.0303f, 0.0f, -0.285f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));

						//<body name="RF_SHANK" pos="0.1003 0 -0.285">
						compiler.StartBody("RF_SHANK", Point(0.1003f, 0.0f, -0.285f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 0.58842f);
						//<inertial mass="0.58842" pos="0.005462 0.0612528 -0.0806598" quat="0.992934 0.115904 -0.00105487 0.0254421" diaginertia="0.0101637 0.00923838 0.00111927"/>
						compiler.SetInertia(0.58842f, Point(0.005462f, 0.0612528f, -0.0806598f), Quat(0.992934f, 0.115904f, -0.00105487f, 0.0254421f), Point(0.0101637f, 0.00923838f, 0.00111927f));
						//<joint name="RF_KFE" axis="-1 0 0" range="-9.42478 9.42478"/>
						compiler.SetHingeJoint("RF_KFE", Point(-1.0f, 0.0f, 0.0f), -9.42478f, 9.42478f);

							//<geom quat="1 0 0 1" material="shank" mesh="shank_r" class="visual"/>
							compiler.AddVisual(mMeshes[22], null, &Quat(1.0f, 0.0f, 0.0f, 1.0f));

						//<geom class="collision" size="0.06 0.02" pos="0.02 0 0" quat="0.5 0.5 0.5 0.5"/>
						compiler.AddCylinderShape(0.06f, 0.02f, &Point(0.02f, 0.0f, 0.0f), &Quat(0.5f, 0.5f, 0.5f, 0.5f));
						//<geom class="collision" size="0.057499 0.03375 0.019" pos="0.02 0.057499 0" quat="0.5 0.5 0.5 0.5" type="box"/>
						compiler.AddBoxShape(0.057499f, 0.03375f, 0.019f, &Point(0.02f, 0.057499f, 0.0f), &Quat(0.5f, 0.5f, 0.5f, 0.5f));

							//<geom pos="0.01305 0.08795 -0.33797" quat="0.382683 0 0 0.92388" material="foot" mesh="foot" class="visual"/>
							compiler.AddVisual(mMeshes[23], &Point(0.01305f, 0.08795f, -0.33797f), &Quat(0.382683f, 0.0f, 0.0f, 0.92388f));

						//<geom class="collision" size="0.0175 0.141252" pos="0.01305 0.08795 -0.168985" quat="1 0 0 1"/>
						compiler.AddCylinderShape(0.0175f, 0.141252f, &Point(0.01305f, 0.08795f, -0.168985f), &Quat(1.0f, 0.0f, 0.0f, 1.0f));
						//<geom class="foot" pos="0.01305 0.08795 -0.31547" quat="1 0 0 1"/>
						compiler.AddSphereShape(0.03f, &Point(0.01305f, 0.08795f, -0.31547f), &Quat(1.0f, 0.0f, 0.0f, 1.0f));
						//</body>
						compiler.EndBody();
					//</body>
					compiler.EndBody();
				//</body>
				compiler.EndBody();

				//<body name="LH_HIP" pos="-0.2999 0.104 0" quat="0 0 0.965926 -0.258819">
				compiler.StartBody("LH_HIP", Point(-0.2999f, 0.104f, 0.0f), Quat(0.0f, 0.0f, 0.965926f, -0.258819f), 2.781f);
				//<inertial mass="2.781" pos="0.0567633 0.015294 -0.00829784" quat="0.13524 0.679072 0.127985 0.710065" diaginertia="0.00585928 0.0049205 0.00329064"/>
				compiler.SetInertia(2.781f, Point(0.0567633f, 0.015294f, -0.00829784f), Quat(0.13524f, 0.679072f, 0.127985f, 0.710065f), Point(0.00585928f, 0.0049205f, 0.00329064f));
				//<joint name="LH_HAA" axis="-1 0 0" range="-0.72 0.49"/>
				compiler.SetHingeJoint("LH_HAA", Point(-1.0f, 0.0f, 0.0f), -0.72f, 0.49f);

					//<geom quat="-0.258819 -0.965926 0 0" material="hip_r" mesh="hip_r" class="visual"/>
					compiler.AddVisual(mMeshes[19], null, &Quat(-0.258819f, -0.965926f, 0.0f, 0.0f));
					//<geom pos="0.0599 0.0725816 -0.041905" quat="0.183013 0.683013 0.683013 -0.183013" material="drive" mesh="drive" class="visual"/>
					compiler.AddVisual(mMeshes[16], &Point(0.0599f, 0.0725816f, -0.041905f), &Quat(0.183013f, 0.683013f, 0.683013f, -0.183013f));

				//<geom class="collision" size="0.05 0.07" pos="0.0599 0.0119598 -0.006905" quat="-0.353553 0.612372 0.612372 0.353553"/>
				compiler.AddCylinderShape(0.05f, 0.07f, &Point(0.0599f, 0.0119598f, -0.006905f), &Quat(-0.353553f, 0.612372f, 0.612372f, 0.353553f));

					//<body name="LH_THIGH" pos="0.0599 0.0725816 -0.041905" quat="0.183013 0.683013 0.683013 -0.183013">
					compiler.StartBody("LH_THIGH", Point(0.0599f, 0.0725816f, -0.041905f), Quat(0.183013f, 0.683013f, 0.683013f, -0.183013f), 3.071f);
					//<inertial mass="3.071" pos="0.0308147 4.64995e-05 -0.245696" quat="0.992775 -0.00512735 -0.0806685 0.0886811" diaginertia="0.0302511 0.0298933 0.0041845"/>
					compiler.SetInertia(3.071f, Point(0.0308147f, 4.64995e-05f, -0.245696f), Quat(0.992775f, -0.00512735f, -0.0806685f, 0.0886811f), Point(0.0302511f, 0.0298933f, 0.0041845f));
					//<joint name="LH_HFE" axis="1 0 0" range="-9.42478 9.42478"/>
					compiler.SetHingeJoint("LH_HFE", Point(1.0f, 0.0f, 0.0f), -9.42478f, 9.42478f);

						//<geom quat="1 0 0 -1" material="thigh" mesh="thigh" class="visual"/>
						compiler.AddVisual(mMeshes[20], null, &Quat(1.0f, 0.0f, 0.0f, -1.0f));

					//<geom class="collision" size="0.065 0.04" pos="0.04 0 0" quat="0.5 0.5 -0.5 -0.5"/>
					compiler.AddCylinderShape(0.065f, 0.04f, &Point(0.04f, 0.0f, 0.0f), &Quat(0.5f, 0.5f, -0.5f, -0.5f));
					//<geom class="collision" size="0.0375 0.03 0.141314" pos="0.03 0 -0.141314" quat="1 0 0 -1" type="box"/>
					compiler.AddBoxShape(0.0375f, 0.03f, 0.141314f, &Point(0.03f, 0.0f, -0.141314f), &Quat(1.0f, 0.0f, 0.0f, -1.0f));

						//<geom pos="0.1003 0 -0.285" material="drive" mesh="drive" class="visual"/>
						compiler.AddVisual(mMeshes[16], &Point(0.1003f, 0.0f, -0.285f));

					//<geom class="collision" size="0.05 0.07" pos="0.0303 0 -0.285" quat="1 0 1 0"/>
					compiler.AddCylinderShape(0.05f, 0.07f, &Point(0.0303f, 0.0f, -0.285f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));

						//<body name="LH_SHANK" pos="0.1003 0 -0.285">
						compiler.StartBody("LH_SHANK", Point(0.1003f, 0.0f, -0.285f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 0.58842f);
						//<inertial mass="0.58842" pos="0.005462 0.0612528 -0.0806598" quat="0.992934 0.115904 -0.00105487 0.0254421" diaginertia="0.0101637 0.00923838 0.00111927"/>
						compiler.SetInertia(0.58842f, Point(0.005462f, 0.0612528f, -0.0806598f), Quat(0.992934f, 0.115904f, -0.00105487f, 0.0254421f), Point(0.0101637f, 0.00923838f, 0.00111927f));
						//<joint name="LH_KFE" axis="1 0 0" range="-9.42478 9.42478"/>
						compiler.SetHingeJoint("LH_KFE", Point(1.0f, 0.0f, 0.0f), -9.42478f, 9.42478f);

							//<geom quat="-1 0 0 -1" material="shank" mesh="shank_r" class="visual"/>
							compiler.AddVisual(mMeshes[22], null, &Quat(-1.0f, 0.0f, 0.0f, -1.0f));

						//<geom class="collision" size="0.06 0.02" pos="0.02 0 0" quat="0.5 0.5 -0.5 -0.5"/>
						compiler.AddCylinderShape(0.06f, 0.02f, &Point(0.02f, 0.0f, 0.0f), &Quat(0.5f, 0.5f, -0.5f, -0.5f));
						//<geom class="collision" size="0.057499 0.03375 0.019" pos="0.02 0.057499 0" quat="0.5 0.5 -0.5 -0.5" type="box"/>
						compiler.AddBoxShape(0.057499f, 0.03375f, 0.019f, &Point(0.02f, 0.057499f, 0.0f), &Quat(0.5f, 0.5f, -0.5f, -0.5f));

							//<geom pos="0.01305 0.08795 -0.33797" quat="-0.382683 0 0 -0.92388" material="foot" mesh="foot" class="visual"/>
							compiler.AddVisual(mMeshes[23], &Point(0.01305f, 0.08795f, -0.33797f), &Quat(-0.382683f, 0.0f, 0.0f, -0.92388f));

						//<geom class="collision" size="0.0175 0.141252" pos="0.01305 0.08795 -0.168985" quat="1 0 0 -1"/>
						compiler.AddCylinderShape(0.0175f, 0.141252f, &Point(0.01305f, 0.08795f, -0.168985f), &Quat(1.0f, 0.0f, 0.0f, -1.0f));
						//<geom class="foot" pos="0.01305 0.08795 -0.31547" quat="1 0 0 -1"/>
						compiler.AddSphereShape(0.03f, &Point(0.01305f, 0.08795f, -0.31547f), &Quat(1.0f, 0.0f, 0.0f, -1.0f));
						//</body>
						compiler.EndBody();

					//</body>
					compiler.EndBody();
				//</body>
				compiler.EndBody();

				//<body name="RH_HIP" pos="-0.2999 -0.104 0" quat="0 0 -0.965926 -0.258819">
				compiler.StartBody("RH_HIP", Point(-0.2999f, -0.104f, 0.0f), Quat(0.0f, 0.0f, -0.965926f, -0.258819f), 2.781f);
				//<inertial mass="2.781" pos="0.0566606 -0.015294 -0.00829784" quat="-0.127978 0.709783 -0.135278 0.679359" diaginertia="0.00585729 0.00491868 0.00329081"/>
				compiler.SetInertia(2.781f, Point(0.0566606f, -0.015294f, -0.00829784f), Quat(-0.127978f, 0.709783f, -0.135278f, 0.679359f), Point(0.00585729f, 0.00491868f, 0.00329081f));
				//<joint name="RH_HAA" axis="-1 0 0" range="-0.49 0.72"/>
				compiler.SetHingeJoint("RH_HAA", Point(-1.0f, 0.0f, 0.0f), -0.49f, 0.72f);

					//<geom quat="-0.258819 0.965926 0 0" material="hip_l" mesh="hip_l" class="visual"/>
					compiler.AddVisual(mMeshes[18], null, &Quat(-0.258819f, 0.965926f, 0.0f, 0.0f));
					//<geom pos="0.0599 -0.0725816 -0.041905" quat="-0.183013 0.683013 -0.683013 -0.183013" material="drive" mesh="drive" class="visual"/>
					compiler.AddVisual(mMeshes[16], &Point(0.0599f, -0.0725816f, -0.041905f), &Quat(-0.183013f, 0.683013f, -0.683013f, -0.183013f));

				//<geom class="collision" size="0.05 0.07" pos="0.0599 -0.0119598 -0.006905" quat="0.353553 0.612372 -0.612372 0.353553"/>
				compiler.AddCylinderShape(0.05f, 0.07f, &Point(0.0599f, -0.0119598f, -0.006905f), &Quat(0.353553f, 0.612372f, -0.612372f, 0.353553f));

					//<body name="RH_THIGH" pos="0.0599 -0.0725816 -0.041905" quat="-0.183013 0.683013 -0.683013 -0.183013">
					compiler.StartBody("RH_THIGH", Point(0.0599f, -0.0725816f, -0.041905f), Quat(-0.183013f, 0.683013f, -0.683013f, -0.183013f), 3.071f);
					//<inertial mass="3.071" pos="0.0308147 4.64995e-05 -0.245696" quat="0.993166 -0.00515309 -0.0806592 0.0841972" diaginertia="0.03025 0.0298943 0.00418465"/>
					compiler.SetInertia(3.071f, Point(0.0308147f, 4.64995e-05f, -0.245696f), Quat(0.993166f, -0.00515309f, -0.0806592f, 0.0841972f), Point(0.03025f, 0.0298943f, 0.00418465f));
					//<joint name="RH_HFE" axis="-1 0 0" range="-9.42478 9.42478"/>
					compiler.SetHingeJoint("RH_HFE", Point(-1.0f, 0.0f, 0.0f), -9.42478f, 9.42478f);

						//<geom quat="1 0 0 -1" material="thigh" mesh="thigh" class="visual"/>
						compiler.AddVisual(mMeshes[20], null, &Quat(1.0f, 0.0f, 0.0f, -1.0f));

					//<geom class="collision" size="0.065 0.04" pos="0.04 0 0" quat="0.5 0.5 0.5 0.5"/>
					compiler.AddCylinderShape(0.065f, 0.04f, &Point(0.04f, 0.0f, 0.0f), &Quat(0.5f, 0.5f, 0.5f, 0.5f));
					//<geom class="collision" size="0.0375 0.03 0.141314" pos="0.03 0 -0.141314" quat="1 0 0 1" type="box"/>
					compiler.AddBoxShape(0.0375f, 0.03f, 0.141314f, &Point(0.03f, 0.0f, -0.141314f), &Quat(1.0f, 0.0f, 0.0f, 1.0f));

						//<geom pos="0.1003 0 -0.285" material="drive" mesh="drive" class="visual"/>
						compiler.AddVisual(mMeshes[16], &Point(0.1003f, 0.0f, -0.285f));

					//<geom class="collision" size="0.05 0.07" pos="0.0303 0 -0.285" quat="1 0 1 0"/>
					compiler.AddCylinderShape(0.05f, 0.07f, &Point(0.0303f, 0.0f, -0.285f), &Quat(1.0f, 0.0f, 1.0f, 0.0f));

						//<body name="RH_SHANK" pos="0.1003 0 -0.285">
						compiler.StartBody("RH_SHANK", Point(0.1003f, 0.0f, -0.285f), Quat(1.0f, 0.0f, 0.0f, 0.0f), 0.58842f);
						//<inertial mass="0.58842" pos="0.005462 -0.0612528 -0.0806598" quat="0.992934 -0.115904 -0.00105487 -0.0254421" diaginertia="0.0101637 0.00923838 0.00111927"/>
						compiler.SetInertia(0.58842f, Point(0.005462f, -0.0612528f, -0.0806598f), Quat(0.992934f, -0.115904f, -0.00105487f, -0.0254421f), Point(0.0101637f, 0.00923838f, 0.00111927f));
						//<joint name="RH_KFE" axis="-1 0 0" range="-9.42478 9.42478"/>
						compiler.SetHingeJoint("RH_KFE", Point(-1.0f, 0.0f, 0.0f), -9.42478f, 9.42478f);

							//<geom quat="1 0 0 -1" material="shank_l" mesh="shank_l" class="visual"/>
							compiler.AddVisual(mMeshes[21], null, &Quat(1.0f, 0.0f, 0.0f, -1.0f));

						//<geom class="collision" size="0.06 0.02" pos="0.02 0 0" quat="0.5 0.5 0.5 0.5"/>
						compiler.AddCylinderShape(0.06f, 0.02f, &Point(0.02f, 0.0f, 0.0f), &Quat(0.5f, 0.5f, 0.5f, 0.5f));
						//<geom class="collision" size="0.057499 0.03375 0.019" pos="0.02 -0.057499 0" quat="0.5 0.5 0.5 0.5" type="box"/>
						compiler.AddBoxShape(0.057499f, 0.03375f, 0.019f, &Point(0.02f, -0.057499f, 0.0f), &Quat(0.5f, 0.5f, 0.5f, 0.5f));

							//<geom pos="0.01305 -0.08795 -0.33797" quat="0.382683 0 0 -0.92388" material="foot" mesh="foot" class="visual"/>
							compiler.AddVisual(mMeshes[23], &Point(0.01305f, -0.08795f, -0.33797f), &Quat(0.382683f, 0.0f, 0.0f, -0.92388f));

						//<geom class="collision" size="0.0175 0.141252" pos="0.01305 -0.08795 -0.168985" quat="1 0 0 1"/>
						compiler.AddCylinderShape(0.0175f, 0.141252f, &Point(0.01305f, -0.08795f, -0.168985f), &Quat(1.0f, 0.0f, 0.0f, 1.0f));
						//<geom class="foot" pos="0.01305 -0.08795 -0.31547" quat="1 0 0 1"/>
						compiler.AddSphereShape(0.03f, &Point(0.01305f, -0.08795f, -0.31547f), &Quat(1.0f, 0.0f, 0.0f, 1.0f));

						//</body>
						compiler.EndBody();

					//</body>
					compiler.EndBody();
				//</body>
				compiler.EndBody();

			compiler.EndBody();
		compiler.EndModel();
		}
/*

  <contact>
    <exclude body1="base" body2="LF_THIGH"/>
    <exclude body1="base" body2="RF_THIGH"/>
    <exclude body1="base" body2="LH_THIGH"/>
    <exclude body1="base" body2="RH_THIGH"/>
  </contact>

  <actuator>
    <position class="affine" joint="LF_HAA" name="LF_HAA"/>
    <position class="affine" joint="LF_HFE" name="LF_HFE"/>
    <position class="affine" joint="LF_KFE" name="LF_KFE"/>
    <position class="affine" joint="RF_HAA" name="RF_HAA"/>
    <position class="affine" joint="RF_HFE" name="RF_HFE"/>
    <position class="affine" joint="RF_KFE" name="RF_KFE"/>
    <position class="affine" joint="LH_HAA" name="LH_HAA"/>
    <position class="affine" joint="LH_HFE" name="LH_HFE"/>
    <position class="affine" joint="LH_KFE" name="LH_KFE"/>
    <position class="affine" joint="RH_HAA" name="RH_HAA"/>
    <position class="affine" joint="RH_HFE" name="RH_HFE"/>
    <position class="affine" joint="RH_KFE" name="RH_KFE"/>
  </actuator>
*/
		return true;
	}

	virtual	udword		Update(Pint& pint, float dt)	override
	{
		const JointMapping* JM = reinterpret_cast<const JointMapping*>(pint.mUserData);
		if(JM && mSlider && mComboBox)
		{
			const udword Nb = JM->mData.GetNbEntries() / (sizeof(JointMapping::Mapping)/sizeof(udword));
			const JointMapping::Mapping* Mappings = (const JointMapping::Mapping*)JM->mData.GetEntries();
			//printf("%d\n", Nb);

			if(mCheckBox_ApplyTestForces && mCheckBox_ApplyTestForces->IsChecked())
			{
				const float Coeff = TWOPI / float(NB_ACTUATORS_ANYMALC);
				for(udword i=0; i<Nb; i++)
				{
					const udword ActuatorIndex = i % NB_ACTUATORS_ANYMALC;
					const float Phase = Coeff * float(ActuatorIndex);
					pint.SetRCADrivePosition(Mappings[i].mHandle, sinf(Phase + mCurrentTime * 2.0f) * 0.5f);
				}
			}
			else
			{
				//const udword Selected = mComboBox->GetSelectedIndex();
				const float SliderValue = mSlider->GetValue();
				//printf("Selected: %d (%f)\n", Selected, SliderValue);
				mActuatorTarget[mCurrentSlider] = SliderValue;

				for(udword i=0;i<Nb;i++)
				{
					const udword ActuatorIndex = i % NB_ACTUATORS_ANYMALC;
					//Mappings[i].mName
					pint.SetRCADrivePosition(Mappings[i].mHandle, mActuatorTarget[ActuatorIndex]);
				}
			}
		}


		/*const bool EnableUpdate = mCheckBox_EnableUpdate ? mCheckBox_EnableUpdate->IsChecked() : true;
		if(EnableUpdate)
		{
			const bool EnableMotor = mCheckBox_EnableMotor ? mCheckBox_EnableMotor->IsChecked() : true;
			const LocalTestData* LTD = (const LocalTestData*)pint.mUserData;
			if(LTD)
			{
				const float Coeff = mSlider ? mSlider->GetValue() : 1.0f;
				const float TargetVel = Coeff * GetFloat(0.0f, mEditBox_TargetVel);
				const float TargetPos = Coeff * GetFloat(0.0f, mEditBox_TargetPos);
	//			printf("TargetVel: %f\n", TargetVel);
				pint.SetRCADriveEnabled(LTD->mDynamicObject, EnableMotor);
				if(EnableMotor)
				{
					pint.SetRCADriveVelocity(LTD->mDynamicObject, TargetVel);
					pint.SetRCADrivePosition(LTD->mDynamicObject, TargetPos);
				}
			}
		}*/
		return 0;
	}

END_TEST(AnymalC)

///////////////////////////////////////////////////////////////////////////////
