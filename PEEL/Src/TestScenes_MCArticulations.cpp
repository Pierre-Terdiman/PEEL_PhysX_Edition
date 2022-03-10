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
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "Loader_Bin.h"
#include "ProceduralTrack.h"
#include "MyConvex.h"
#include "GUI_Helpers.h"

///////////////////////////////////////////////////////////////////////////////

	struct ConfigurableArticulationDefault
	{
				ConfigurableArticulationDefault() :
				mNbSimCallsPerRenderFrame	(1),
				mNbLinks					(0),
				mCapsuleRadius				(0.0f),
				mCapsuleMass				(0.0f),
				mCapsuleMassForInertia		(0.0f),
				mBoxMass					(0.0f),
				mInitialAngularVelocity		(0.0f),
				mUseArticulations			(false),
//				mUseRCArticulations			(false),
				mUseExtraDistanceConstraints(false)
				{}
		udword	mNbSimCallsPerRenderFrame;
		udword	mNbLinks;
		float	mCapsuleRadius;
		float	mCapsuleMass;
		float	mCapsuleMassForInertia;
		float	mBoxMass;
		float	mInitialAngularVelocity;
		bool	mUseArticulations;
//		bool	mUseRCArticulations;
		bool	mUseExtraDistanceConstraints;
	};

	class ConfigurableArticulationTest : public TestBase
	{
		protected:
				ConfigurableArticulationDefault	mValues;
				CheckBoxPtr		mCheckBox_Articulations;
//				CheckBoxPtr		mCheckBox_RCArticulations;
				CheckBoxPtr		mCheckBox_DistanceConstraints;
				CheckBoxPtr		mCheckBox_OverlappingLinks;
				EditBoxPtr		mEditBox_NbSimCalls;
				EditBoxPtr		mEditBox_NbLinks;
				EditBoxPtr		mEditBox_CapsuleMass;
				EditBoxPtr		mEditBox_CapsuleMassForInertia;
				EditBoxPtr		mEditBox_CapsuleRadius;
				EditBoxPtr		mEditBox_BoxMass;
				EditBoxPtr		mEditBox_InitialAngularVel;
		public:
								ConfigurableArticulationTest()	{}
		virtual					~ConfigurableArticulationTest()	{}

		virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
		{
			TestBase::GetSceneParams(desc);
			desc.mNbSimulateCallsPerFrame = GetInt(mValues.mNbSimCallsPerRenderFrame, mEditBox_NbSimCalls);
		}

		virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
		{
			WindowDesc WD;
			WD.mParent	= null;
			WD.mX		= 50;
			WD.mY		= 50;
			WD.mWidth	= 300;
			WD.mHeight	= 400;
			WD.mLabel	= GetName();
			WD.mType	= WINDOW_DIALOG;
			IceWindow* UI = ICE_NEW(IceWindow)(WD);
			RegisterUIElement(UI);
			UI->SetVisible(true);

			Widgets& UIElems = GetUIElements();

			const sdword EditBoxWidth = 60;
			const sdword LabelWidth = 140;
			const sdword OffsetX = LabelWidth + 10;
			const sdword LabelOffsetY = 2;
			const sdword YStep = 20;
			sdword y = 10;
			{
				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb sim calls/frame:", &UIElems);
				mEditBox_NbSimCalls = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, _F("%d", mValues.mNbSimCallsPerRenderFrame), &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
				y += YStep;

				mCheckBox_Articulations = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use articulations", &UIElems, mValues.mUseArticulations, null, null);
				y += YStep;

//				mCheckBox_RCArticulations = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use RC articulations", &UIElems, mValues.mUseRCArticulations, null, null);
//				y += YStep;

				mCheckBox_DistanceConstraints = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use extra distance constraints", &UIElems, mValues.mUseExtraDistanceConstraints, null, null);
				y += YStep;

	//			mCheckBox_OverlappingLinks = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Create overlapping capsules", &UIElems, false, null, null);
	//			y += YStep;

				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb links:", &UIElems);
				mEditBox_NbLinks = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, _F("%d", mValues.mNbLinks), &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
				y += YStep;

				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Capsule radius:", &UIElems);
				mEditBox_CapsuleRadius = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, _F("%.2f", mValues.mCapsuleRadius), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;

				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Capsule mass:", &UIElems);
				mEditBox_CapsuleMass = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, _F("%.2f", mValues.mCapsuleMass), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;

				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Capsule mass for inertia:", &UIElems);
				mEditBox_CapsuleMassForInertia = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, _F("%.2f", mValues.mCapsuleMassForInertia), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;

				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Box mass:", &UIElems);
				mEditBox_BoxMass = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, _F("%.2f", mValues.mBoxMass), &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
				y += YStep;

				helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Initial angular velocity:", &UIElems);
				mEditBox_InitialAngularVel = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, _F("%.2f", mValues.mInitialAngularVelocity), &UIElems, EDITBOX_FLOAT, null, null);
				y += YStep;
			}

			y += YStep;
			AddResetButton(UI, 4, y, 300-16);

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
	};

///////////////////////////////////////////////////////////////////////////////

class ArticulatedChain : public ConfigurableArticulationTest
{
	public:
							ArticulatedChain(bool use_articulation)
							{
								mValues.mNbSimCallsPerRenderFrame		= 1;
								mValues.mNbLinks						= 30;
								mValues.mCapsuleRadius					= 0.5f;
								mValues.mCapsuleMass					= 1.0f;
								mValues.mCapsuleMassForInertia			= 1.0;
								mValues.mBoxMass						= 100.0;
								mValues.mInitialAngularVelocity			= 0.0;
								mValues.mUseArticulations				= use_articulation;
								mValues.mUseExtraDistanceConstraints	= false;
							}

	virtual					~ArticulatedChain()			{									}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		ConfigurableArticulationTest::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(6.46f, 27.81f, 16.09f), Point(0.00f, -0.39f, -0.92f));
		SetDefEnv(desc, false);
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		IceTabControl* TabControl = ConfigurableArticulationTest::InitUI(helper);
		mEditBox_InitialAngularVel->SetEnabled(false);
		mEditBox_NbLinks->SetEnabled(false);
		mCheckBox_Articulations->SetEnabled(false);
		return TabControl;
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const bool UseFiltering = true;
		if(UseFiltering)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			const PintDisabledGroups DG[2] = { PintDisabledGroups(1, 2), PintDisabledGroups(3, 3)	};
			pint.SetDisabledGroups(2, DG);
		}

		const bool UseArticulations = mCheckBox_Articulations ? mCheckBox_Articulations->IsChecked() : mValues.mUseArticulations;
		bool UseRCArticulations = false;
//		if(!UseArticulations)
//			UseRCArticulations = mCheckBox_RCArticulations ? mCheckBox_RCArticulations->IsChecked() : mValues.mUseRCArticulations;

		const bool UseDistanceConstraints = mCheckBox_DistanceConstraints ? mCheckBox_DistanceConstraints->IsChecked() : mValues.mUseExtraDistanceConstraints;
//		const bool UseOverlappingLinks = mCheckBox_OverlappingLinks ? mCheckBox_OverlappingLinks->IsChecked() : false;
//		const float AngularVel = GetFromEditBox(mValues.mInitialAngularVelocity, mEditBox_InitialAngularVel, MIN_FLOAT, MAX_FLOAT);
		const float BoxMass = GetFloat(mValues.mBoxMass, mEditBox_BoxMass);
		const float CapsuleMass = GetFloat(mValues.mCapsuleMass, mEditBox_CapsuleMass);
		const float CapsuleMassForInertia = GetFloat(mValues.mCapsuleMassForInertia, mEditBox_CapsuleMassForInertia);
		const float CapsuleRadius = GetFloat(mValues.mCapsuleRadius, mEditBox_CapsuleRadius);
//		const udword NbLinks = GetFromEditBox(mValues.mNbLinks, mEditBox_NbLinks);

		if(UseArticulations && !caps.mSupportMCArticulations)
//			return false;
			UseRCArticulations = true;

		if(UseRCArticulations && !caps.mSupportRCArticulations)
			return false;

		if(UseDistanceConstraints && !caps.mSupportDistanceJoints)
			return false;

		if(CapsuleMass!=CapsuleMassForInertia && !caps.mSupportMassForInertia)
			printf(_F("WARNING: %s doesn't support 'mass for inertia', feature is ignored.\n", pint.GetName()));

		// TODO: revisit this
		PintArticHandle Articulation = UseArticulations ? pint.CreateArticulation(PINT_ARTICULATION_CREATE()) : null;
		PintArticHandle RCArticulation = UseRCArticulations ? pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(true)) : null;

		const float Scale = 0.25f*0.5f;
		const float Radius = CapsuleRadius*Scale;
		const float HalfHeight = 2.0f*Scale;
		const udword NbCapsules = 30;
		const Point Dir(1.0f, 0.0f, 0.0f);
		const Point Extents = Dir * (Radius + HalfHeight);
//		const Point Extents = Dir * (HalfHeight);
		const Point PosOffset = Dir * 0.0f;

		Matrix3x3 m;
		m.RotZ(degToRad(90.0f));

		PINT_CAPSULE_CREATE CapsuleDesc(Radius, HalfHeight);
		CapsuleDesc.mLocalRot	= m;
		CapsuleDesc.mRenderer	= CreateCapsuleRenderer(Radius, HalfHeight*2.0f);

		PintActorHandle Handles[NbCapsules];
		Point Positions[NbCapsules];

		const Point BoxExtents(10.0f*Scale, 10.0f*Scale, 10.0f*Scale);
//		const float InitY = (5.0f + float(NbCapsules)*(Radius+HalfHeight)*2.0f + BoxExtents.y)*Scale;
		const float InitY = 185.0f*Scale;
		Point Pos(0.0f, InitY, 0.0f);

		Positions[0] = Pos;
		udword GroupBit = 0;

		// Creates a spherical attachment point for the rope, to make the scene more visually pleasing
		if(1)
		{
			PINT_SPHERE_CREATE SphereDesc(2.0f*Scale);
			SphereDesc.mRenderer = CreateSphereRenderer(SphereDesc.mRadius);

			PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
			ObjectDesc.mMass			= 0.0f;
			ObjectDesc.mPosition		= Pos - Point(2.0f*Scale, 0.0f, 0.0f);
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
			
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);
		}

		// Creates a static box obstacle. The rope will collide with it and rotate around it.
		if(1)
		{
			PINT_BOX_CREATE BoxDesc(10.0f*Scale, 3.0f*Scale, 20.0f*Scale);
			BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Point(50.0f*Scale, InitY-10.0f*Scale, 0.0f);

			PintActorHandle h = CreatePintObject(pint, ObjectDesc);
		}

		{
			PINT_OBJECT_CREATE ObjectDesc(&CapsuleDesc);
			ObjectDesc.mMass			= CapsuleMass;
			ObjectDesc.mMassForInertia	= CapsuleMassForInertia;
			ObjectDesc.mPosition		= Pos;
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;

			if(Articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				Handles[0] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
//				Handles[0] = pint.CreateArticulatedObject(articulation, null, *this, ObjectDesc);
			}
			else if(RCArticulation)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				Handles[0] = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCArticulation);
//				Handles[0] = pint.CreateArticulatedObject(articulation, null, *this, ObjectDesc);
			}
			else
			{
				Handles[0] = CreatePintObject(pint, ObjectDesc);
			}

			if(!RCArticulation)
			{
				if(0)
				{
					// Make it "static"...
	//				PINT_SPHERICAL_JOINT_CREATE Desc;
					PINT_HINGE_JOINT_CREATE Desc;
					Desc.mLocalAxis0 = Point(0.0f, 0.0f, 1.0f);
					Desc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);
					Desc.mObject0		= null;
					Desc.mObject1		= Handles[0];
					Desc.mLocalPivot0	= Pos + Point(0.0f, 0.0f, 0.0f);
	//				Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
					Desc.mLocalPivot1	= Pos + Point(0.0f, 0.0f, 0.0f);
					Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
				else
				{
					PINT_BOX_CREATE BoxDesc(0.1f*Scale, 0.1f*Scale, 0.1f*Scale);
					BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

					ObjectDesc.SetShape(&BoxDesc);
					ObjectDesc.mMass			= 0.0f;
					ObjectDesc.mCollisionGroup	= 1 + GroupBit;
					PintActorHandle h = CreatePintObject(pint, ObjectDesc);

					PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(h, Handles[0], Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f)));
					ASSERT(JointHandle);
				}
			}
		}
		Pos += (PosOffset + Extents)*2.0f;

		for(udword i=1;i<NbCapsules-1;i++)
		{
			Positions[i] = Pos;
			{
				PINT_OBJECT_CREATE ObjectDesc(&CapsuleDesc);
				ObjectDesc.mMass			= CapsuleMass;
				ObjectDesc.mMassForInertia	= CapsuleMassForInertia;
				ObjectDesc.mPosition		= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
//				Handles[i] = CreateArticulationLink(articulation, (PxArticulationLink*)Handles[i-1], *this, ObjectDesc);
				if(Articulation)
				{
					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = Handles[i-1];
					ArticulatedDesc.mLocalPivot0 = Extents + PosOffset;
					ArticulatedDesc.mLocalPivot1 = -Extents - PosOffset;
					Handles[i] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
				}
				else if(RCArticulation)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = Handles[i-1];
					ArticulatedDesc.mLocalPivot0.mPos = Extents + PosOffset;
					ArticulatedDesc.mLocalPivot1.mPos = -Extents - PosOffset;
					Handles[i] = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCArticulation);
				}
				else
				{
					Handles[i] = CreatePintObject(pint, ObjectDesc);

/*					PINT_HINGE_JOINT_CREATE Desc;
					Desc.mLocalAxis0 = Point(0.0f, 0.0f, 1.0f);
					Desc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);
					Desc.mObject0		= Handles[i-1];
					Desc.mObject1		= Handles[i];
					Desc.mLocalPivot0	= Extents + PosOffset;
					Desc.mLocalPivot1	= -Extents - PosOffset;*/

					const Point Offset = Extents + PosOffset;
					PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[i-1], Handles[i], Offset, -Offset));
					ASSERT(JointHandle);
				}
				if(0)
				{
					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0			= Handles[i-1];
					Desc.mObject1			= Handles[i];
					Desc.mLimits.mMaxValue	= Positions[i-1].Distance(Positions[i]);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
			Pos += (PosOffset + Extents)*2.0f;
		}

		{
			const udword i=NbCapsules-1;
			PINT_BOX_CREATE BoxDesc(BoxExtents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxExtents);

			//###
			Pos -= (PosOffset + Extents);
			Pos.x += BoxExtents.x;
//			Pos.x += BoxExtents.x - Radius;

			Positions[i] = Pos;
//			Handles[i] = CreateDynamicObject(pint, &SphereDesc, Pos);
			{
				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= BoxMass;
				ObjectDesc.mPosition		= Pos;
//				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
//				Handles[i] = CreateArticulationLink(articulation, (PxArticulationLink*)Handles[i-1], *this, ObjectDesc);
				if(Articulation)
				{
					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = Handles[i-1];
					ArticulatedDesc.mLocalPivot0 = Extents + PosOffset;
					ArticulatedDesc.mLocalPivot1 = Point(-BoxExtents.x, 0.0f, 0.0f);
					Handles[i] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
//					printf("Big Mass: %f\n", ((PxArticulationLink*)Handles[i])->getMass());

/*				PxArticulationJoint* joint = ((PxArticulationLink*)Handles[i])->getInboundJoint();
				if(joint)
				{
					joint->setParentPose(PxTransform(ToPxVec3(Extents + PosOffset)));
					joint->setChildPose(PxTransform(PxVec3(-BoxExtents.x, 0.0f, 0.0f)));
					setupJoint(joint);
				}*/
				}
				else if(RCArticulation)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = Handles[i-1];
					ArticulatedDesc.mLocalPivot0.mPos = Extents + PosOffset;
					ArticulatedDesc.mLocalPivot1.mPos = Point(-BoxExtents.x, 0.0f, 0.0f);
					Handles[i] = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCArticulation);
//					printf("Big Mass: %f\n", ((PxArticulationLink*)Handles[i])->getMass());

/*				PxArticulationJoint* joint = ((PxArticulationLink*)Handles[i])->getInboundJoint();
				if(joint)
				{
					joint->setParentPose(PxTransform(ToPxVec3(Extents + PosOffset)));
					joint->setChildPose(PxTransform(PxVec3(-BoxExtents.x, 0.0f, 0.0f)));
					setupJoint(joint);
				}*/
				}
				else
				{
					Handles[i] = CreatePintObject(pint, ObjectDesc);

					PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[i-1], Handles[i], Extents + PosOffset, Point(-BoxExtents.x, 0.0f, 0.0f)));
					ASSERT(JointHandle);
				}

				if(0)
				{
					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0			= Handles[0];
					Desc.mObject1			= Handles[i];
					Desc.mLimits.mMaxValue	= Positions[i].Distance(Positions[0]);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}

			}
			Pos += (PosOffset + Extents)*2.0f;
		}
		if(Articulation)
			pint.AddArticulationToScene(Articulation);
		if(RCArticulation)
			pint.AddRCArticulationToScene(RCArticulation);

		// Distance constraints
		if(UseDistanceConstraints)
		{
			const float Slop = 1.0f;
//			const float Slop = 1.025f;
			if(0)
			{
				udword i=NbCapsules-2;
				PINT_DISTANCE_JOINT_CREATE Desc;
				Desc.mObject0			= Handles[0];
				Desc.mObject1			= Handles[i];
				Desc.mLimits.mMaxValue	= Positions[i].Distance(Positions[0])*Slop;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}

			if(1)
			{
				for(udword i=0;i<NbCapsules;i++)
				{
					if(i+2<NbCapsules)
					{
						PINT_DISTANCE_JOINT_CREATE Desc;
						Desc.mObject0			= Handles[i];
						Desc.mObject1			= Handles[i+2];
						Desc.mLimits.mMaxValue	= Positions[i].Distance(Positions[i+2])*Slop;
						PintJointHandle JointHandle = pint.CreateJoint(Desc);
						ASSERT(JointHandle);
					}
				}
			}

			if(0)
			{
				for(udword i=1;i<NbCapsules;i++)
				{
					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0			= Handles[0];
					Desc.mObject1			= Handles[i];
					Desc.mLimits.mMaxValue	= Positions[0].Distance(Positions[i])*Slop;
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}

		// Rotating plank
		if(0)
		{
			PINT_BOX_CREATE BoxDesc(2.0f*Scale, 4.0f*Scale, 20.0f*Scale);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
//			ObjectDesc.mMass			= 10000.0f;
//			ObjectDesc.mPosition		= Point(0.0f*Scale, -50.0f*Scale, -15.0f*Scale);
			ObjectDesc.mPosition		= Point(0.0f*Scale, InitY - float(NbCapsules)*(Radius+HalfHeight)*2.0f - BoxExtents.y, -15.0f*Scale);
			
			ObjectDesc.mCollisionGroup	= 3;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			if(0)
			{
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
				Desc.mObject0		= null;
				Desc.mObject1		= h;
				Desc.mLocalPivot0	= ObjectDesc.mPosition;
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			else
			{
				PINT_BOX_CREATE BoxDesc(0.1f*Scale, 0.1f*Scale, 0.1f*Scale);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
				ObjectDesc.SetShape(&BoxDesc);
				ObjectDesc.mMass	= 0.0f;
				PintActorHandle h2 = CreatePintObject(pint, ObjectDesc);

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mLocalAxis0 = Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1 = Point(0.0f, 1.0f, 0.0f);
				Desc.mObject0		= h2;
				Desc.mObject1		= h;
				Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ArticulatedChain_Regular = "Articulated chain. The chain is made of 30 capsules. Each capsule has a mass of 1. \
The large box at the end of the chain has a mass of 100. This kind of scene is challenging for iterative solvers (that most physics engines use by default). \
On the other hand some engines also support dedicated solutions/solvers for articulated bodies, which can handle this test case out of the box. \
The PhysX 4.0 TGS solver also manages to handle this one pretty well, as demonstrated here.";

class RegularJoints_ArticulatedChain : public ArticulatedChain
{
public:
	RegularJoints_ArticulatedChain() : ArticulatedChain(false)	{}
	virtual	const char*		GetName()			const	{ return "ArticulatedChain_RegularJoints";	}
	virtual	const char*		GetDescription()	const	{ return gDesc_ArticulatedChain_Regular;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;					}
}RegularJoints_ArticulatedChain;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ArticulatedChain_MCA = "Same setup as ArticulatedChain_RegularJoints, this time using a MC articulation (as initially included in PEEL 1.1).";

class MCArticulatedChain : public ArticulatedChain
{
public:
	MCArticulatedChain() : ArticulatedChain(true)	{}
	virtual	const char*		GetName()			const	{ return "ArticulatedChain_MCArticulation";	}
	virtual	const char*		GetDescription()	const	{ return gDesc_ArticulatedChain_MCA;		}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_MCARTICULATIONS;			}
}MCArticulatedChain;

///////////////////////////////////////////////////////////////////////////////

static void AttachLink(PintActorHandle h, Pint& pint, const Point& pos, float radius, float half_height)
{
	PINT_BOX_CREATE BoxDesc(0.01f, 0.01f, 0.01f);
	BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mPosition	= pos;
	ObjectDesc.mMass		= 0.0f;
//	ObjectDesc.mCollisionGroup	= 1 + GroupBit;

	const PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(CreatePintObject(pint, ObjectDesc), h, Point(0.0f, 0.0f, 0.0f), Point(0.0f, -half_height - radius, 0.0f)));
	ASSERT(JointHandle);
}

static void CreateChain(Pint& pint, bool use_articulations, bool use_overlapping_links, bool use_distance_constraints,
						const Point& p0, const Point& p1, udword nb_links, float radius, float mass, float mass_for_inertia,
						PintActorHandle heavy_object, const Point& local_pivot0)
{
	PintActorHandle Handles[MAX_LINKS];
	float HalfHeight;
	{
		PintArticHandle Articulation = use_articulations ? pint.CreateArticulation(PINT_ARTICULATION_CREATE()) : null;
		CreateCapsuleRope2(pint, HalfHeight, Articulation, null, p0, p1, nb_links, radius, mass, mass_for_inertia, use_overlapping_links, use_distance_constraints, Handles);
		if(Articulation)
			pint.AddArticulationToScene(Articulation);
	}
	AttachLink(Handles[0], pint, p0, radius, HalfHeight);

	PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(heavy_object, Handles[nb_links-1], local_pivot0, Point(0.0f, HalfHeight+radius, 0.0f)));
	ASSERT(JointHandle);
}

class SwingSet : public ConfigurableArticulationTest
{
	public:
							SwingSet(bool use_articulation)
							{
								mValues.mNbSimCallsPerRenderFrame		= 4;
								mValues.mNbLinks						= 32;
								mValues.mCapsuleRadius					= 0.25f;
								mValues.mCapsuleMass					= 1.0f;
								mValues.mCapsuleMassForInertia			= 10.0;
								mValues.mBoxMass						= 2000.0;
								mValues.mInitialAngularVelocity			= 2.0;
								mValues.mUseArticulations				= use_articulation;
								mValues.mUseExtraDistanceConstraints	= false;
							}
	virtual					~SwingSet()			{	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		IceTabControl* TabControl = ConfigurableArticulationTest::InitUI(helper);
		mCheckBox_Articulations->SetEnabled(false);
		return TabControl;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		ConfigurableArticulationTest::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-28.59f, 36.09f, 99.17f), Point(0.42f, -0.02f, -0.91f));
		SetDefEnv(desc, false);
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const bool UseArticulations = mCheckBox_Articulations ? mCheckBox_Articulations->IsChecked() : mValues.mUseArticulations;
		const bool UseDistanceConstraints = mCheckBox_DistanceConstraints ? mCheckBox_DistanceConstraints->IsChecked() : mValues.mUseExtraDistanceConstraints;
		const bool UseOverlappingLinks = mCheckBox_OverlappingLinks ? mCheckBox_OverlappingLinks->IsChecked() : false;
		const float AngularVel = GetFloat(mValues.mInitialAngularVelocity, mEditBox_InitialAngularVel);
		const float BoxMass = GetFloat(mValues.mBoxMass, mEditBox_BoxMass);
		const float CapsuleMass = GetFloat(mValues.mCapsuleMass, mEditBox_CapsuleMass);
		const float CapsuleMassForInertia = GetFloat(mValues.mCapsuleMassForInertia, mEditBox_CapsuleMassForInertia);
		const float CapsuleRadius = GetFloat(mValues.mCapsuleRadius, mEditBox_CapsuleRadius);
		const udword NbLinks = GetInt(mValues.mNbLinks, mEditBox_NbLinks);
		if(UseArticulations && !caps.mSupportMCArticulations)
			return false;
		if(UseDistanceConstraints && !caps.mSupportDistanceJoints)
			return false;

		if(CapsuleMass!=CapsuleMassForInertia && !caps.mSupportMassForInertia)
			printf(_F("WARNING: %s doesn't support 'mass for inertia', feature is ignored.\n", pint.GetName()));

//		const float Scale = 0.2f;
		const float Scale = 1.0f;
		const float x = 0.0f;
		const float y = 0.0f;
		const float z = 0.0f;
		const float h = 100.0f * Scale;
		const float ex = 10.0f * Scale;
		const float ey = 4.0f * Scale;
		const float ez = 4.0f * Scale;
		const float Radius = CapsuleRadius * Scale;

		PINT_BOX_CREATE BoxDesc(ex, ey, ez);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mPosition		= Point(0.0f, y - BoxDesc.mExtents.y, 0.0f);
		ObjectDesc.mMass			= BoxMass;
	//	ObjectDesc.mCollisionGroup	= 1 + GroupBit;
		ObjectDesc.mAngularVelocity	= Point(0.0f, AngularVel, 0.0f);
		const PintActorHandle HeavyObject = CreatePintObject(pint, ObjectDesc);

		const bool b0 = UseArticulations;
		const bool b1 = UseOverlappingLinks;
		const bool b2 = UseDistanceConstraints;

		CreateChain(pint, b0, b1, b2, Point(x-ex, y+h, z-ez), Point(x-ex, y, z-ez), NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, HeavyObject, Point(-ex, BoxDesc.mExtents.y, -ez));
		CreateChain(pint, b0, b1, b2, Point(x+ex, y+h, z-ez), Point(x+ex, y, z-ez), NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, HeavyObject, Point(ex, BoxDesc.mExtents.y, -ez));
		CreateChain(pint, b0, b1, b2, Point(x-ex, y+h, z+ez), Point(x-ex, y, z+ez), NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, HeavyObject, Point(-ex, BoxDesc.mExtents.y, ez));
		CreateChain(pint, b0, b1, b2, Point(x+ex, y+h, z+ez), Point(x+ex, y, z+ez), NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, HeavyObject, Point(ex, BoxDesc.mExtents.y, ez));
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SwingSet_Regular = "A test inspired by a swing set. We all did that on the playground as kids!... This is not \
really a swing set though: the capsules each have a mass of 1 and the box has a mass of 2000. This variation uses regular joints and the TGS solver in PhysX 4.0, \
but it remains challenging without articulations. Running it side-by-side with PhysX 3.x shows clear improvements though.";

class RegularJoints_SwingSet : public SwingSet
{
public:
	RegularJoints_SwingSet() : SwingSet(false)			{}
	virtual	const char*		GetName()			const	{ return "SwingSet_RegularJoints";	}
	virtual	const char*		GetDescription()	const	{ return gDesc_SwingSet_Regular;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;			}
}RegularJoints_SwingSet;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_SwingSet_MCA = "Same setup as SwingSet_RegularJoints, this time using a MC articulation (as initially included in PEEL 1.1).";

class MCSwingSet : public SwingSet
{
public:
	MCSwingSet() : SwingSet(true)						{}
	virtual	const char*		GetName()			const	{ return "SwingSet_MCArticulation";	}
	virtual	const char*		GetDescription()	const	{ return gDesc_SwingSet_MCA;		}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_MCARTICULATIONS;	}
}MCSwingSet;

///////////////////////////////////////////////////////////////////////////////

class Crane : public ConfigurableArticulationTest
{
	public:
							Crane(bool use_articulation)
							{
								mValues.mNbSimCallsPerRenderFrame		= 1;
								mValues.mNbLinks						= 11;
								mValues.mCapsuleRadius					= 0.1f;
								mValues.mCapsuleMass					= 1.0f;
								mValues.mCapsuleMassForInertia			= 10.0;
								mValues.mBoxMass						= 1000.0;
								mValues.mInitialAngularVelocity			= 0.0;
								mValues.mUseArticulations				= use_articulation;
								mValues.mUseExtraDistanceConstraints	= false;
							}
	virtual					~Crane()					{	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		IceTabControl* TabControl = ConfigurableArticulationTest::InitUI(helper);
		mCheckBox_Articulations->SetEnabled(false);
		return TabControl;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		ConfigurableArticulationTest::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-13.92f, 32.40f, 87.53f), Point(0.20f, 0.01f, -0.98f));
		SetDefEnv(desc, false);
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float x = 0.0f;
		const float y = 0.0f;
		const float z = 0.0f;
		const float h = 50.0f;
		const float ex = 10.0f;
		const float ey = 4.0f;
		const float ez = 4.0f;
		const float ex2 = 0.0f;
		const float ez2 = 0.0f;

		PintActorHandle Handles[MAX_LINKS];
		PintCollisionGroup CollisionGroups[MAX_LINKS];
		float HalfHeight;

		const bool UseArticulations = mCheckBox_Articulations ? mCheckBox_Articulations->IsChecked() : mValues.mUseArticulations;
		const bool UseDistanceConstraints = mCheckBox_DistanceConstraints ? mCheckBox_DistanceConstraints->IsChecked() : mValues.mUseExtraDistanceConstraints;
		const bool UseOverlappingLinks = mCheckBox_OverlappingLinks ? mCheckBox_OverlappingLinks->IsChecked() : false;
		const float AngularVel = GetFloat(mValues.mInitialAngularVelocity, mEditBox_InitialAngularVel);
		const float BoxMass = GetFloat(mValues.mBoxMass, mEditBox_BoxMass);
		const float CapsuleMass = GetFloat(mValues.mCapsuleMass, mEditBox_CapsuleMass);
		const float CapsuleMassForInertia = GetFloat(mValues.mCapsuleMassForInertia, mEditBox_CapsuleMassForInertia);
		const float Radius = GetFloat(mValues.mCapsuleRadius, mEditBox_CapsuleRadius);
		const udword NbLinks = GetInt(mValues.mNbLinks, mEditBox_NbLinks);

		if(UseArticulations && !caps.mSupportMCArticulations)
			return false;

		if(UseDistanceConstraints && !caps.mSupportDistanceJoints)
			return false;

		if(CapsuleMass!=CapsuleMassForInertia && !caps.mSupportMassForInertia)
			printf(_F("WARNING: %s doesn't support 'mass for inertia', feature is ignored.\n", pint.GetName()));

		if(1)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			if(0)
			{
				// We need to do two separate things here:
				//
				// 1) disable collisions between elements N and N+1. That's because some engines
				// do not disable collisions between jointed objects automatically, so we have to
				// do it explicitly (as in other PEEL scenes).
				//
				// 2) we need to disable collisions at the point where the 5 different chains are
				// connected. The elements there are not all connected by joints, but they will be
				// overlapping eachother constantly. And contacts there will make things jitter.
				//
				// We're going to use the following collision groups:
				//
				//            31212121212
				// 21212121213
				//            31212121212
				//
				// - the regular chain elements have groups 1 and 2
				// - the elements at the junction are in group 3
				// - CD between 1 and 2 is disabled, as usual
				// - CD within group 3 is disabled
				// - CD between 1/2 and 3 is disabled
			}
			else
			{
				// However since the ropes cannot actually touch eachother in this test (contrary to
				// the swing set scene), we can do something simpler: just disable CD between all links...
				for(udword i=0;i<NbLinks;i++)
					CollisionGroups[i] = 1;
				const PintDisabledGroups DG(1, 1);
				pint.SetDisabledGroups(1, &DG);
			}
		}

		PINT_BOX_CREATE BoxDesc(ex, ey, ez);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mPosition		= Point(0.0f, y - BoxDesc.mExtents.y, 0.0f);
		ObjectDesc.mMass			= BoxMass;
		ObjectDesc.mCollisionGroup	= 1;
		ObjectDesc.mAngularVelocity = Point(0.0f, AngularVel, 0.0f);
		PintActorHandle HeavyObject = CreatePintObject(pint, ObjectDesc);

		PintArticHandle Articulation = UseArticulations ? pint.CreateArticulation(PINT_ARTICULATION_CREATE()) : null;

		const Point BasePos(x, y+h, z);
		const Point TopPos(x, y+h+h, z);
		CreateCapsuleRope2(pint, HalfHeight, Articulation, null, TopPos, BasePos, NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, false, UseDistanceConstraints, Handles, CollisionGroups);
		AttachLink(Handles[0], pint, TopPos, Radius, HalfHeight);

		PintActorHandle Parent = Handles[NbLinks-1];

		const float Offset = 1.0f;

		if(1)
		{
			Point RootPos[4];
			PintActorHandle Root[4];
			{
				RootPos[0] = Point(x-ex2, y+h, z-ez2);
				CreateCapsuleRope2(pint, HalfHeight, Articulation, Parent, RootPos[0], Point(x-ex, y, z-ez), NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, false, UseDistanceConstraints, Handles, CollisionGroups);
				Root[0] = Handles[0];

				PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(HeavyObject, Handles[NbLinks-1], Point(-ex*Offset, BoxDesc.mExtents.y, -ez*Offset), Point(0.0f, HalfHeight+Radius, 0.0f)));
				ASSERT(JointHandle);
			}

			{
				RootPos[1] = Point(x+ex2, y+h, z-ez2);
				CreateCapsuleRope2(pint, HalfHeight, Articulation, Parent, RootPos[1], Point(x+ex, y, z-ez), NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, false, UseDistanceConstraints, Handles, CollisionGroups);
				Root[1] = Handles[0];

				PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(HeavyObject, Handles[NbLinks-1], Point(ex*Offset, BoxDesc.mExtents.y, -ez*Offset), Point(0.0f, HalfHeight+Radius, 0.0f)));
				ASSERT(JointHandle);
			}

			{
				RootPos[2] = Point(x-ex2, y+h, z+ez2);
				CreateCapsuleRope2(pint, HalfHeight, Articulation, Parent, RootPos[2], Point(x-ex, y, z+ez), NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, false, UseDistanceConstraints, Handles, CollisionGroups);
				Root[2] = Handles[0];

				PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(HeavyObject, Handles[NbLinks-1], Point(-ex*Offset, BoxDesc.mExtents.y, ez*Offset), Point(0.0f, HalfHeight+Radius, 0.0f)));
				ASSERT(JointHandle);
			}

			{
				RootPos[3] = Point(x+ex2, y+h, z+ez2);
				CreateCapsuleRope2(pint, HalfHeight, Articulation, Parent, RootPos[3], Point(x+ex, y, z+ez), NbLinks, Radius, CapsuleMass, CapsuleMassForInertia, false, UseDistanceConstraints, Handles, CollisionGroups);
				Root[3] = Handles[0];

				PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(HeavyObject, Handles[NbLinks-1], Point(ex*Offset, BoxDesc.mExtents.y, ez*Offset), Point(0.0f, HalfHeight+Radius, 0.0f)));
				ASSERT(JointHandle);
			}
		}

/*		for(udword j=0;j<4;j++)
		{
			for(udword i=j+1;i<4;i++)
			{
				PINT_DISTANCE_JOINT_CREATE Desc;
				Desc.mObject0	= Root[j];
				Desc.mObject1	= Root[i];
				Desc.mDistance	= RootPos[j].Distance(RootPos[i]);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}*/
		if(Articulation)
			pint.AddArticulationToScene(Articulation);
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Crane_Regular = "Crane test. This variation uses regular joints and the TGS solver in PhysX 4.0, \
but it remains challenging without articulations. Running it side-by-side with PhysX 3.x shows clear improvements though.";

class RegularJoints_Crane : public Crane
{
public:
	RegularJoints_Crane() : Crane(false)				{}
	virtual	const char*		GetName()			const	{ return "Crane_RegularJoints";	}
	virtual	const char*		GetDescription()	const	{ return gDesc_Crane_Regular;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_JOINTS;		}
}RegularJoints_Crane;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Crane_MCA = "Same setup as Crane_RegularJoints, this time using a MC articulation (as initially included in PEEL 1.1).";

class MCCrane : public Crane
{
public:
	MCCrane() : Crane(true)								{}
	virtual	const char*		GetName()			const	{ return "Crane_MCArticulation";	}
	virtual	const char*		GetDescription()	const	{ return gDesc_Crane_MCA;			}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_MCARTICULATIONS;	}
}MCCrane;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ConnectedMCArticulations = "The PhysX MC articulations are limited to 64 links per articulation. This test shows how to \
bypass that limit for a rope, by connecting multiple articulations (3 in this case) with regular joints. This test also shows that using overlapping capsules \
for the chain is better for collision detection. You should be able to pick the end of the rope with the mouse and with some practice actually make a knot.";

START_TEST(ConnectedMCArticulations, CATEGORY_MCARTICULATIONS, gDesc_ConnectedMCArticulations)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("ConnectedMCArticulations", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.72f, 10.81f, 6.42f), Point(0.77f, -0.34f, -0.54f));
		desc.mCamera[1] = PintCameraPose(Point(45.47f, 4.24f, 4.65f), Point(-0.84f, -0.23f, -0.49f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

/*		const bool UseFiltering = true;
		if(UseFiltering)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			const PintDisabledGroups DG[2] = { PintDisabledGroups(1, 2), PintDisabledGroups(3, 3)	};
			pint.SetDisabledGroups(2, DG);
		}*/

		const udword NbArticulations = 3;
		PintArticHandle Articulations[64];
		for(udword i=0;i<NbArticulations;i++)
		{
			Articulations[i] = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
			// This test doesn't work without articulations
			if(!Articulations[i])
				return false;
		}

		const float Scale = 0.25f*0.5f*0.5f;
//		const float Scale = 0.25f*0.5f*0.1f;
//		const float Scale = 0.25f*0.5f*0.25f;

//		const float Radius = 0.5f*Scale;
		const float Radius = 1.0f*Scale;
//		const float Radius = 2.0f*Scale;
		const float HalfHeight = 2.0f*Scale;
//		const float HalfHeight = 1.0f*Scale;
//		const udword NbCapsules = 20;
//		const udword NbCapsules = 40;
		const udword NbCapsules = 64;
//		const udword NbCapsules = 100;
//		const udword NbCapsules = 200;
//		const udword NbCapsules = 500;
		const Point Dir(1.0f, 0.0f, 0.0f);
//		const Point Extents = Dir * (Radius + HalfHeight);
		const Point Extents = Dir * (HalfHeight);
		const Point PosOffset = Dir * 0.0f;
		const bool AttachToWorld = true;

		Matrix3x3 m;
		m.RotZ(degToRad(90.0f));

		PINT_CAPSULE_CREATE CapsuleDesc(Radius, HalfHeight);
		CapsuleDesc.mLocalRot	= m;
		CapsuleDesc.mRenderer	= CreateCapsuleRenderer(Radius, HalfHeight*2.0f);

		PintActorHandle Handles[NbCapsules*NbArticulations];
		Point Positions[NbCapsules*NbArticulations];

		const Point BoxExtents(10.0f*Scale, 10.0f*Scale, 10.0f*Scale);
//		const float InitY = (5.0f + float(NbCapsules)*(Radius+HalfHeight)*2.0f + BoxExtents.y)*Scale;
		const float InitY = 185.0f*Scale;
		Point Pos(0.0f, InitY, 0.0f);

		for(udword j=0;j<NbArticulations;j++)
		{
			PintActorHandle* H = Handles + j*NbCapsules;
			Point* P = Positions + j*NbCapsules;

			const PintArticHandle Articulation = Articulations[j];
			for(udword i=0;i<NbCapsules;i++)
			{
				{
					PINT_OBJECT_CREATE ObjectDesc(&CapsuleDesc);
					ObjectDesc.mMass		= 1.0f;
					ObjectDesc.mPosition	= Pos;

					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					if(i)
						ArticulatedDesc.mParent = H[i-1];
					ArticulatedDesc.mLocalPivot0 = Extents + PosOffset;
					ArticulatedDesc.mLocalPivot1 = -Extents - PosOffset;
					H[i] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
					P[i] = Pos;
				}
				Pos += (PosOffset + Extents)*2.0f;
			}
			pint.AddArticulationToScene(Articulation);
		}

		for(udword j=0;j<NbArticulations-1;j++)
		{
			const PintActorHandle* H = Handles + j*NbCapsules;
			const Point* P = Positions + j*NbCapsules;

			const Point Offset = Extents + PosOffset;
			const PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(H[NbCapsules-1], H[NbCapsules], Offset, -Offset));
			ASSERT(JointHandle);

			if(1)
			{
//				for(udword i=0;i<NbCapsules;i++)
				for(udword i=0;i<4;i++)
				{
					PINT_DISTANCE_JOINT_CREATE Desc;
					const udword Index0 = NbCapsules-i-1;
					const udword Index1 = NbCapsules+i;
					Desc.mObject0			= H[Index0];
					Desc.mObject1			= H[Index1];
					Desc.mLimits.mMaxValue	= P[Index0].Distance(P[Index1]);
					const PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}

		if(AttachToWorld)
		{
			PINT_BOX_CREATE BoxDesc(0.1f*Scale, 0.1f*Scale, 0.1f*Scale);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Point(0.0f, InitY, 0.0f);
			const PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			if(0)
			{
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= h;
				Desc.mObject1		= Handles[0];
				Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
				const PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			else
			{
				const PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(h, Handles[0], Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f)));
				ASSERT(JointHandle);
			}
		}
		return true;
	}

END_TEST(ConnectedMCArticulations)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_Winch = "Revisited PEEL 1.1 winch test, using MC articulations.";

START_TEST(Winch, CATEGORY_MCARTICULATIONS, gDesc_Winch)

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("Winch", 20);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(2.68f, 12.05f, 2.63f), Point(-0.85f, -0.08f, -0.51f));
		desc.mCamera[1] = PintCameraPose(Point(4.11f, 12.58f, 9.86f), Point(-0.46f, -0.26f, -0.85f));
		desc.mCamera[2] = PintCameraPose(Point(-3.80f, 12.55f, 11.81f), Point(0.37f, -0.29f, -0.88f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

/*		const bool UseFiltering = true;
		if(UseFiltering)
		{
			if(!caps.mSupportCollisionGroups)
				return false;

			const PintDisabledGroups DG[2] = { PintDisabledGroups(1, 2), PintDisabledGroups(3, 3)	};
			pint.SetDisabledGroups(2, DG);
		}*/

		const udword NbArticulations = 3;
		PintArticHandle Articulations[64];
		for(udword i=0;i<NbArticulations;i++)
		{
			Articulations[i] = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
			// This test doesn't work without articulations
			if(!Articulations[i])
				return false;
		}

		const float Scale = 0.25f*0.5f*0.5f;
//		const float Scale = 0.25f*0.5f*0.1f;
//		const float Scale = 0.25f*0.5f*0.25f;

//		const float Radius = 0.5f*Scale;
		const float Radius = 1.0f*Scale;
//		const float Radius = 2.0f*Scale;
		const float HalfHeight = 2.0f*Scale;
		const udword NbCapsules = 64;
		const Point Dir(1.0f, 0.0f, 0.0f);
//		const Point Extents = Dir * (Radius + HalfHeight);
		const Point Extents = Dir * (HalfHeight);
		const Point PosOffset = Dir * 0.0f;
		const bool AttachToWorld = true;

		const Point BoxExtents(10.0f*Scale, 10.0f*Scale, 10.0f*Scale);
//		const float InitY = (5.0f + float(NbCapsules)*(Radius+HalfHeight)*2.0f + BoxExtents.y)*Scale;
		const float InitY = 185.0f*Scale;
		Point Pos(0.0f, InitY, 0.0f);

		Matrix3x3 m;
		m.RotZ(degToRad(90.0f));

//		{
//			PINT_CAPSULE_CREATE CapsuleDesc2(HalfHeight*2.0f, 4.0f);
			PINT_CAPSULE_CREATE CapsuleDesc2(HalfHeight, 4.0f);
			CapsuleDesc2.mRenderer = CreateCapsuleRenderer(CapsuleDesc2.mRadius, CapsuleDesc2.mHalfHeight*2.0f);

			Matrix3x3 m2;
			m2.RotX(degToRad(90.0f));
			CapsuleDesc2.mLocalRot	= m2;


			PINT_CAPSULE_CREATE CapsuleDesc3(HalfHeight, 2.0f);
			CapsuleDesc3.mRenderer	= CreateCapsuleRenderer(CapsuleDesc3.mRadius, CapsuleDesc3.mHalfHeight*2.0f);
			CapsuleDesc3.mLocalPos	= Point(0.0f, 0.0f, CapsuleDesc2.mHalfHeight);
			CapsuleDesc3.mLocalRot	= m;
			CapsuleDesc2.SetNext(&CapsuleDesc3);

/*
			PINT_OBJECT_CREATE ObjectDesc(&CapsuleDesc2);
			ObjectDesc.mMass		= 10.0f;
			ObjectDesc.mPosition	= Pos;
//			ObjectDesc.mRotation	= m2;
//			PintActorHandle h = CreatePintObject(pint, ObjectDesc);*/
//		}

		PINT_CAPSULE_CREATE CapsuleDesc(Radius, HalfHeight);
		CapsuleDesc.mLocalRot	= m;
		CapsuleDesc.mRenderer	= CreateCapsuleRenderer(Radius, HalfHeight*2.0f);

		PintActorHandle Handles[NbCapsules*NbArticulations];
		Point Positions[NbCapsules*NbArticulations];

		for(udword j=0;j<NbArticulations;j++)
		{
			PintActorHandle* H = Handles + j*NbCapsules;
			Point* P = Positions + j*NbCapsules;

			PintArticHandle Articulation = Articulations[j];
			for(udword i=0;i<NbCapsules;i++)
			{
				{
					PINT_OBJECT_CREATE ObjectDesc;
					if(i || j)
					{
						ObjectDesc.SetShape(&CapsuleDesc);
						ObjectDesc.mMass		= 1.0f;
						ObjectDesc.mPosition	= Pos;
					}
					else
					{
						ObjectDesc.SetShape(&CapsuleDesc2);
//						ObjectDesc.mMass		= 20.0f;
						ObjectDesc.mMass		= 200.0f;
						ObjectDesc.mPosition	= Pos;
//						ObjectDesc.mRotation	= m2;
//						ObjectDesc.mAngularVelocity	= Point(0.0f, 0.0f, 100.0f);
					}

					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					if(i)
						ArticulatedDesc.mParent = H[i-1];
//					if(i||j)
					ArticulatedDesc.mLocalPivot0 = Extents + PosOffset;
//					else
//					ArticulatedDesc.mLocalPivot0 = (Dir * CapsuleDesc2.mRadius) + PosOffset;
					ArticulatedDesc.mLocalPivot1 = -Extents - PosOffset;
					H[i] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
					P[i] = Pos;
				}
//				if(i||j)
				Pos += (PosOffset + Extents)*2.0f;
//				else
//				Pos += (PosOffset + (Dir * (CapsuleDesc2.mRadius)))*2.0f;
			}
			pint.AddArticulationToScene(Articulation);
		}

		for(udword j=0;j<NbArticulations-1;j++)
		{
			const PintActorHandle* H = Handles + j*NbCapsules;
			const Point* P = Positions + j*NbCapsules;

			const Point Offset = Extents + PosOffset;
			PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(H[NbCapsules-1], H[NbCapsules], Offset, -Offset));
			ASSERT(JointHandle);

			if(1)
			{
//				for(udword i=0;i<NbCapsules;i++)
				for(udword i=0;i<4;i++)
				{
					PINT_DISTANCE_JOINT_CREATE Desc;
					const udword Index0 = NbCapsules-i-1;
					const udword Index1 = NbCapsules+i;
					Desc.mObject0			= H[Index0];
					Desc.mObject1			= H[Index1];
					Desc.mLimits.mMaxValue	= P[Index0].Distance(P[Index1]);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}

		if(AttachToWorld)
		{
			PINT_BOX_CREATE BoxDesc(0.1f*Scale, 0.1f*Scale, 0.1f*Scale);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Point(0.0f, InitY, 0.0f);
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			if(1)
			{
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= h;
				Desc.mObject1		= Handles[0];
				Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			else
			{
				PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(h, Handles[0], Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f)));
				ASSERT(JointHandle);
			}

//			pint.SetAngularVelocity(Handles[0], Point(0.0f, 0.0f, 100.0f));
			pint.mUserData = Handles[0];
		}
		return true;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		PintActorHandle h = PintActorHandle(pint.mUserData);
		if(h)
			pint.SetAngularVelocity(h, Point(0.0f, 0.0f, 8.0f));
		return TestBase::Update(pint, dt);
	}

END_TEST(Winch)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ArticulationDriveTest = "Articulation drive test. This one is mainly to get a feeling for the MC articulation drive \
parameters in PhysX. Pick and drag the rotating object with the mouse to see how it reacts to external forces. This test also shows how to setup the MC articulation \
to emulate a hinge joint.";

	enum ArticulationDriveType
	{
		DRIVE_VELOCITY,
		DRIVE_POSITION,
	};

	struct WheelTestData
	{
		WheelTestData() :
			mDriveType			(DRIVE_VELOCITY)
			{}
		EditBoxPtr				mEditBox_TargetVelocity;
		EditBoxPtr				mEditBox_TargetPosition;
		EditBoxPtr				mEditBox_Stiffness;
		EditBoxPtr				mEditBox_Damping;
		EditBoxPtr				mEditBox_InternalCompliance;
		EditBoxPtr				mEditBox_ExternalCompliance;
		ComboBoxPtr				mComboBox_DriveType;
		ArticulationDriveType	mDriveType;
	};

	class DriveComboBox : public IceComboBox
	{
		public:
						DriveComboBox(const ComboBoxDesc& desc, WheelTestData* data) : IceComboBox(desc), mData(data)	{}

		virtual	void	OnComboBoxEvent(ComboBoxEvent event)
						{
							if(event==CBE_SELECTION_CHANGED)
							{
								mData->mDriveType = ArticulationDriveType(GetSelectedIndex());
								if(mData->mDriveType==DRIVE_VELOCITY)
								{
									mData->mEditBox_TargetVelocity->SetEnabled(true);
									mData->mEditBox_TargetPosition->SetEnabled(false);
									mData->mEditBox_TargetVelocity->SetLabel("2");
									mData->mEditBox_TargetPosition->SetLabel("0");
									mData->mEditBox_Stiffness->SetLabel("0");
									mData->mEditBox_Damping->SetLabel("1000");
									mData->mEditBox_InternalCompliance->SetLabel("1");
									mData->mEditBox_ExternalCompliance->SetLabel("1");
								}
								else
								{
									mData->mEditBox_TargetVelocity->SetEnabled(false);
									mData->mEditBox_TargetPosition->SetEnabled(true);
									mData->mEditBox_TargetVelocity->SetLabel("0");
									mData->mEditBox_TargetPosition->SetLabel("45");
									mData->mEditBox_Stiffness->SetLabel("1000");
									mData->mEditBox_Damping->SetLabel("200");
									mData->mEditBox_InternalCompliance->SetLabel("1");
									mData->mEditBox_ExternalCompliance->SetLabel("1");
								}
							}
						}

				WheelTestData*	mData;
	};

START_TEST(ArticulationDriveTest, CATEGORY_MCARTICULATIONS, gDesc_ArticulationDriveTest)

	WheelTestData	mData;

	virtual	IceTabControl*		InitUI(PintGUIHelper& helper)
	{
		udword ID = 0;

		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 250;
		WD.mLabel	= "Articulation drive";
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
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Drive type:", &UIElems);

			ComboBoxDesc CBBD;
			CBBD.mID		= ID++;
			CBBD.mParent	= UI;
			CBBD.mX			= 4+OffsetX;
			CBBD.mY			= y;
			CBBD.mWidth		= 150;
			CBBD.mHeight	= 20;
			CBBD.mLabel		= "Drive type";
			mData.mComboBox_DriveType = ICE_NEW(DriveComboBox)(CBBD, &mData);
			RegisterUIElement(mData.mComboBox_DriveType);
			mData.mComboBox_DriveType->Add("Velocity");
			mData.mComboBox_DriveType->Add("Position");
			mData.mComboBox_DriveType->Select(mData.mDriveType);
			mData.mComboBox_DriveType->SetVisible(true);
			y += YStep;
		}

		{
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Target velocity:", &UIElems);
			mData.mEditBox_TargetVelocity = helper.CreateEditBox(UI, ID++, 4+OffsetX, y, EditBoxWidth, 20, "2", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Target position:", &UIElems);
			mData.mEditBox_TargetPosition = helper.CreateEditBox(UI, ID++, 4+OffsetX, y, EditBoxWidth, 20, "0", &UIElems, EDITBOX_FLOAT, null, null);
			mData.mEditBox_TargetPosition->SetEnabled(false);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Stiffness:", &UIElems);
			mData.mEditBox_Stiffness = helper.CreateEditBox(UI, ID++, 4+OffsetX, y, EditBoxWidth, 20, "0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Damping:", &UIElems);
			mData.mEditBox_Damping = helper.CreateEditBox(UI, ID++, 4+OffsetX, y, EditBoxWidth, 20, "1000", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Internal compliance:", &UIElems);
			mData.mEditBox_InternalCompliance = helper.CreateEditBox(UI, ID++, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "External compliance:", &UIElems);
			mData.mEditBox_ExternalCompliance = helper.CreateEditBox(UI, ID++, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(2.73f, 10.47f, 9.51f), Point(-0.36f, -0.43f, -0.83f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		// This test doesn't work without articulation
		const PintArticHandle Articulation = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
		if(!Articulation)
			return false;

		const float Altitude = 5.0f;
		const Point Extents(1.0f, 1.0f, 1.0f);
		const Point ArticulationPos(0.0f, Altitude + Extents.y, 0.0f);

		//

		const float TargetVelocity = GetFloat(0.0f, mData.mEditBox_TargetVelocity);
		const float TargetPosition = GetFloat(0.0f, mData.mEditBox_TargetPosition);
		const float Stiffness = GetFloat(0.0f, mData.mEditBox_Stiffness);
		const float Damping = GetFloat(0.0f, mData.mEditBox_Damping);
		const float InternalCompliance = GetFloat(0.0f, mData.mEditBox_InternalCompliance);
		const float ExternalCompliance = GetFloat(0.0f, mData.mEditBox_ExternalCompliance);

		//
		PintActorHandle ArticulatedObjectRoot;
		{
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= ArticulationPos;

			//
			PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;

			ArticulatedObjectRoot = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
		}

		//

		//
		if(1)
		{
			PINT_FIXED_JOINT_CREATE fjc;
			fjc.mObject0 = null;
			fjc.mObject1 = ArticulatedObjectRoot;
			fjc.mLocalPivot0 = ArticulationPos;
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
			PintJointHandle j1 = pint.CreateJoint(fjc);
		}
		if(0)
		{
			PintJointHandle j1 = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(null, ArticulatedObjectRoot, ArticulationPos, Point(0.0f, 0.0f, 0.0f)));
		}
		if(0)
		{
			PINT_HINGE_JOINT_CREATE fjc;
			fjc.mObject0 = null;
			fjc.mObject1 = ArticulatedObjectRoot;
			fjc.mLocalPivot0 = ArticulationPos;
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
			fjc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
			fjc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
			PintJointHandle j1 = pint.CreateJoint(fjc);
		}
		//

			PINT_BOX_CREATE BoxDesc(2.0f, 2.0f, 0.5f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= ArticulationPos;

		ObjectDesc.mPosition = ArticulationPos;
		PintActorHandle Parent = ArticulatedObjectRoot;
		{
			PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mParent = Parent;
			ArticulatedDesc.mLocalPivot0 = Point(0.0f, 0.0f, 0.0f);
			ArticulatedDesc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
			// We want a hinge. We will use the twist axis as the hinge axis. The twist axis is
			// defined by the "X" axis in PhysX, and we can remap it here.
			ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
//			ArticulatedDesc.mX = Point(1.0f, 0.0f, 0.0f);
//			ArticulatedDesc.mEnableTwistLimit = true;
			ArticulatedDesc.mEnableTwistLimit = false;
			ArticulatedDesc.mTwistLowerLimit = -0.001f;
			ArticulatedDesc.mTwistUpperLimit = 0.001f;
			ArticulatedDesc.mEnableSwingLimit = true;
			ArticulatedDesc.mSwingYLimit = 0.001f;
			ArticulatedDesc.mSwingZLimit = 0.001f;
//			ArticulatedDesc.mSwingZLimit = FLT_MAX;

			ArticulatedDesc.mUseMotor = true;
			ArticulatedDesc.mMotor.mExternalCompliance = ExternalCompliance;
			ArticulatedDesc.mMotor.mInternalCompliance = InternalCompliance;
			ArticulatedDesc.mMotor.mDamping = Damping;
			ArticulatedDesc.mMotor.mStiffness = Stiffness;
			// So this is a bit confusing here. The target velocity is defined in "parent constraint frame". In the
			// PEEL plugin we use the same constraint frame for the two bodies linked by an articulation joint. So
			// in our case this velocity is defined in the rotating object's frame. We remapped its X axis to Z above,
			// so the desired angular velocity is now around X. Another way to see it is just that we want to rotate
			// around the twist axis, and the twist axis is always X from the joint's local point of view.
			// I suspect the Y and Z values of the target vel would just be around the Y and Z swing axes.
			ArticulatedDesc.mMotor.mTargetVelocity = Point(TargetVelocity, 0.0f, 0.0f);
//			ArticulatedDesc.mMotor.mTargetVelocity = Point(TargetVelocity, TargetVelocity, TargetVelocity);

			const AngleAxis AA(TargetPosition*DEGTORAD, 1.0f, 0.0f, 0.0f);
			Quat Rot = AA;
			Rot.Normalize();

			ArticulatedDesc.mMotor.mTargetOrientation = Rot;

			Parent = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
		}
		pint.AddArticulationToScene(Articulation);

		return true;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
//		PintActorHandle Obj = pint.mUserData;
//		pint.SetAngularVelocity(Obj, Point(0.0f, 0.0f, 0.0f));

		return TestBase::Update(pint, dt);
	}

END_TEST(ArticulationDriveTest)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ArticulationDriveVsObstacle = "Articulation drive blocked by a static obstacle. This is a case where speculative contacts (angular CCD) or just an increased \
contact offset visually improves the behavior, as demonstrated in this test.";

START_TEST(ArticulationDriveVsStaticObstacle, CATEGORY_MCARTICULATIONS, gDesc_ArticulationDriveVsObstacle)

	// We don't expose the shape's contact offset in the PINT API because it's a PhysX setting not supported by all engines,
	// so we'll customize this in the per-test UI instead.
	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("ArticulationDriveVsStaticObstacle", 0);
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-13.52f, 11.39f, 11.78f), Point(0.56f, -0.36f, -0.74f));
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		// This test doesn't work without articulation
		const PintArticHandle Articulation = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
		if(!Articulation)
			return false;

		const float Altitude = 5.0f;
		const Point Extents(1.0f, 1.0f, 1.0f);
		const Point ArticulationPos(0.0f, Altitude + Extents.y, 0.0f);

		//

		const float TargetVelocity = 0.0f;
		const float TargetPosition = 0.0f;
		const float Stiffness = 0.0f;
//		const float Damping = 10000000.0f;
		const float Damping = 10000.0f;
		const float InternalCompliance = 1.0f;
		const float ExternalCompliance = 1.0f;
//		const float ExternalCompliance = 0.1f;

		//
		PintActorHandle ArticulatedObjectRoot;
		{
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= ArticulationPos;

			//
			PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;

			ArticulatedObjectRoot = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);


			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	-= Point(10.0f, 0.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}

		//

		//
		if(1)
		{
			PINT_FIXED_JOINT_CREATE fjc;
			fjc.mObject0 = null;
			fjc.mObject1 = ArticulatedObjectRoot;
			fjc.mLocalPivot0 = ArticulationPos;
			fjc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
			PintJointHandle j1 = pint.CreateJoint(fjc);
		}
		//

			PINT_BOX_CREATE BoxDesc(2.0f, 10.0f, 0.5f);
//			PINT_BOX_CREATE BoxDesc(1.0f, 1.0f, 1.0f);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= ArticulationPos;

		ObjectDesc.mPosition = ArticulationPos;
		PintActorHandle Parent = ArticulatedObjectRoot;

		{
			PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mParent = Parent;
			ArticulatedDesc.mLocalPivot0 = Point(0.0f, 0.0f, 0.0f);
			ArticulatedDesc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
			// We want a hinge. We will use the twist axis as the hinge axis. The twist axis is
			// defined by the "X" axis in PhysX, and we can remap it here.
			ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
//			ArticulatedDesc.mX = Point(1.0f, 0.0f, 0.0f);
//			ArticulatedDesc.mEnableTwistLimit = true;
			ArticulatedDesc.mEnableTwistLimit = false;
			ArticulatedDesc.mTwistLowerLimit = -0.001f;
			ArticulatedDesc.mTwistUpperLimit = 0.001f;
			ArticulatedDesc.mEnableSwingLimit = true;
			ArticulatedDesc.mSwingYLimit = 0.001f;
			ArticulatedDesc.mSwingZLimit = 0.001f;
//			ArticulatedDesc.mSwingZLimit = FLT_MAX;

			ArticulatedDesc.mUseMotor = true;
			ArticulatedDesc.mMotor.mExternalCompliance = ExternalCompliance;
			ArticulatedDesc.mMotor.mInternalCompliance = InternalCompliance;
			ArticulatedDesc.mMotor.mDamping = Damping;
			ArticulatedDesc.mMotor.mStiffness = Stiffness;
			// So this is a bit confusing here. The target velocity is defined in "parent constraint frame". In the
			// PEEL plugin we use the same constraint frame for the two bodies linked by an articulation joint. So
			// in our case this velocity is defined in the rotating object's frame. We remapped its X axis to Z above,
			// so the desired angular velocity is now around X. Another way to see it is just that we want to rotate
			// around the twist axis, and the twist axis is always X from the joint's local point of view.
			// I suspect the Y and Z values of the target vel would just be around the Y and Z swing axes.
			ArticulatedDesc.mMotor.mTargetVelocity = Point(TargetVelocity, 0.0f, 0.0f);
			ArticulatedDesc.mMotor.mTargetVelocity = Point(1.0f, 0.0f, 0.0f);
//			ArticulatedDesc.mMotor.mTargetVelocity = Point(TargetVelocity, TargetVelocity, TargetVelocity);

			const AngleAxis AA(TargetPosition*DEGTORAD, 1.0f, 0.0f, 0.0f);
			Quat Rot = AA;
			Rot.Normalize();

			ArticulatedDesc.mMotor.mTargetOrientation = Rot;

			Parent = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
		}

		pint.AddArticulationToScene(Articulation);

		return true;
	}

END_TEST(ArticulationDriveVsStaticObstacle)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ArticulatedCaterpillarTrack = "Articulated caterpillar track. This test demonstrates how to overcome two \
apparent limits of the PhysX MC articulations, namely how to do a closed loop and hinge joints with them.";
START_TEST(ArticulatedCaterpillarTrack, CATEGORY_MCARTICULATIONS, gDesc_ArticulatedCaterpillarTrack )

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(0.37f, 8.80f, 11.93f), Point(-0.03f, -0.38f, -0.93f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		// This test doesn't work without articulation
		const PintArticHandle Articulation = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
		if(!Articulation)
			return false;

		const float Radius = 4.0f;
		const udword NbLinks = 64;
		const float D = PI*Radius/float(NbLinks);
		const float Altitude = 5.0f;
//		const Point Extents(D*0.8f, 0.1f, 1.0f);
		const Point Extents(D, 0.1f, 1.0f);
		const Point ArticulationPos(0.0f, Altitude + Extents.y, 0.0f);

		// Main plank
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			// Smaller inside box to catch the gear
			PINT_BOX_CREATE BoxDesc2(Extents.x*0.1f, Extents.y, Extents.z*0.5f);
			BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
			BoxDesc2.mLocalPos	= Point(0.0f, -Extents.y - BoxDesc2.mExtents.y, 0.0f);
			BoxDesc.SetNext(&BoxDesc2);

			// Left & right upper boxes (will touch the ground)
			PINT_BOX_CREATE BoxDesc3(Extents.x, 0.2f, 0.1f);
			BoxDesc3.mRenderer	= CreateBoxRenderer(BoxDesc3.mExtents);
			BoxDesc3.mLocalPos	= Point(0.0f, (BoxDesc3.mExtents.y - Extents.y), Extents.z + BoxDesc3.mExtents.z);
			BoxDesc2.SetNext(&BoxDesc3);

			PINT_BOX_CREATE BoxDesc4(BoxDesc3.mExtents);
			BoxDesc4.mRenderer	= BoxDesc3.mRenderer;
			BoxDesc4.mLocalPos	= Point(0.0f, BoxDesc3.mLocalPos.y, -BoxDesc3.mLocalPos.z);
			BoxDesc3.SetNext(&BoxDesc4);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mPosition	= ArticulationPos;

		//
		PintActorHandle Objects[128];
		PintActorHandle Parent = null;
		for(udword i=0;i<NbLinks;i++)
		{
			const float Coeff = float(i)/float(NbLinks);
			const float Angle = Coeff*TWOPI;
			const float x = sinf(Angle) * Radius;
			const float y = cosf(Angle) * Radius;

			ObjectDesc.mPosition = Point(x, y+Altitude, 0.0f);

			Matrix3x3 Rot;
			Rot.RotZ(-Angle);
			ObjectDesc.mRotation = Rot;

			PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
			if(Parent)
			{
				ArticulatedDesc.mParent = Parent;
				ArticulatedDesc.mLocalPivot0 = Point(D, 0.0f, 0.0f);
				ArticulatedDesc.mLocalPivot1 = Point(-D, 0.0f, 0.0f);
			}

			if(0)
			{
				// This way is similar to what we did in the ArticulationDrive test.
				ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit = false;
				ArticulatedDesc.mTwistLowerLimit = -0.001f;
				ArticulatedDesc.mTwistUpperLimit = 0.001f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
			}
			else
			{
				const float Epsilon = 0.0001f;
				// We simply emulate a hinge joint using the joints limits.
				ArticulatedDesc.mEnableTwistLimit = true;
				ArticulatedDesc.mTwistLowerLimit = -Epsilon;
				ArticulatedDesc.mTwistUpperLimit = Epsilon;
				ArticulatedDesc.mEnableSwingLimit = true;
//				ArticulatedDesc.mSwingYLimit = Epsilon;
				ArticulatedDesc.mSwingZLimit = Epsilon;
//				ArticulatedDesc.mSwingYLimit = FLT_MAX;
//				ArticulatedDesc.mSwingYLimit = PI - Epsilon;
				ArticulatedDesc.mSwingYLimit = PI/2.0f;
			}
			Parent = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
			Objects[i] = Parent;
		}
		pint.AddArticulationToScene(Articulation);

		// We use a regular hinge joint to close the loop. It could be made more robust by using
		// extra distance constraints on top of this, as seen in the ConnectedArticulations test.
		// But this test shows that the basic & obvious approach also works.
		if(1)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= Objects[0];
			Desc.mObject1		= Objects[NbLinks-1];
			Desc.mLocalPivot0	= Point(-D, 0.0f, 0.0f);
			Desc.mLocalPivot1	= Point(D, 0.0f, 0.0f);
			Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
			Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
		return true;
	}

END_TEST(ArticulatedCaterpillarTrack)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ArticulatedVehicle = "A basic articulated vehicle with large mass ratios between the jointed parts. \
This test uses a MC articulation for the car, showing that it can do more than just ropes. It is a basic vehicle without suspension or anything, \
just to show that this setup does work in PhysX - provided you use the right feature. You can also make it work a lot better with regular joints, \
by using more homogeneous masses for the car parts and/or increasing the solver iteration counts and/or creating the constraints several times, etc. \
Since this test uses regular rigid bodies (as opposed to the dedicated PhysX vehicle SDK), the behavior also depends on which friction model is \
selected in the PhysX UI panel.";
class ArticulatedVehicle : public TestBase
{
			CheckBoxPtr			mCheckBox_Articulations;
			EditBoxPtr			mEditBox_Multiplier;
			EditBoxPtr			mEditBox_GroundFriction;
			EditBoxPtr			mEditBox_WheelFriction;
			EditBoxPtr			mEditBox_ChassisMass;
			EditBoxPtr			mEditBox_WheelMass;
			EditBoxPtr			mEditBox_StructMass;
			EditBoxPtr			mEditBox_WheelRadius;
			EditBoxPtr			mEditBox_WheelWidth;
			EditBoxPtr			mEditBox_WheelTess;
			EditBoxPtr			mEditBox_ChassisX;
			EditBoxPtr			mEditBox_ChassisY;
			EditBoxPtr			mEditBox_ChassisZ;
			EditBoxPtr			mEditBox_COMLocalOffset;
			EditBoxPtr			mEditBox_AxleRadius;
			EditBoxPtr			mEditBox_AxleWidth;
	public:
							ArticulatedVehicle()		{									}
	virtual					~ArticulatedVehicle()		{									}
	virtual	const char*		GetName()			const	{ return "ArticulatedVehicle";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_ArticulatedVehicle;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_MCARTICULATIONS;	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(6.13f, 4.43f, -7.21f), Point(-0.35f, -0.55f, 0.76f));
		desc.mCamera[1] = PintCameraPose(Point(6.34f, 3.77f, 5.14f), Point(-0.69f, -0.41f, -0.60f));
		SetDefEnv(desc, false);
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 400;
		WD.mLabel	= "ArticulatedVehicle";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 120;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			mCheckBox_Articulations = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use articulations", &UIElems, true, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Constraint multiplier:", &UIElems);
			mEditBox_Multiplier = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Ground friction:", &UIElems);
			mEditBox_GroundFriction = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Wheel friction:", &UIElems);
			mEditBox_WheelFriction = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Chassis mass:", &UIElems);
			mEditBox_ChassisMass = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "40.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Wheel mass:", &UIElems);
			mEditBox_WheelMass = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "15.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Struct mass:", &UIElems);
			mEditBox_StructMass = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Wheel radius:", &UIElems);
			mEditBox_WheelRadius = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Wheel width:", &UIElems);
			mEditBox_WheelWidth = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.3", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Wheel tessellation:", &UIElems);
			mEditBox_WheelTess = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "60", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Chassis size X:", &UIElems);
			mEditBox_ChassisX = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Chassis size Y:", &UIElems);
			mEditBox_ChassisY = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Chassis size Z:", &UIElems);
			mEditBox_ChassisZ = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.7", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "COM local offset (Y):", &UIElems);
			mEditBox_COMLocalOffset = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "-1.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Axle radius:", &UIElems);
			mEditBox_AxleRadius = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Axle width:", &UIElems);
			mEditBox_AxleWidth = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual	bool			CommonSetup()
	{
		const udword Nb = 100;
		const float Scale = 0.1f;
		IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
		bool status = IS->MakePlane(Nb, Nb);
		ASSERT(status);
		IS->Scale(Point(Scale, 1.0f, Scale));
		IS->Flip();
		RegisterSurface(IS);
		return true;
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportMeshes)
			return false;

		const bool UseArticulations = mCheckBox_Articulations ? mCheckBox_Articulations->IsChecked() : true;
		if(UseArticulations && !caps.mSupportMCArticulations)
			return false;

		if(!UseArticulations && !caps.mSupportHingeJoints)
			return false;

		if(!caps.mSupportCollisionGroups)
			return false;

		const float GroundFriction = GetFloat(0.1f, mEditBox_GroundFriction);
		PINT_MATERIAL_CREATE GroundMaterial(GroundFriction, GroundFriction, 0.0f);

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps, &GroundMaterial))
			return false;

		// We just disable collisions between all parts of the car for now. That's because some engines don't automatically disable
		// collisions between jointed objects. We could do that better but the focus of this test is the stability of the vehicle,
		// and we don't really need proper collision filtering setup to evaluate this.
		const PintDisabledGroups DG(1, 1);
		pint.SetDisabledGroups(1, &DG);
		PintCollisionGroup GroupChassis = 1;
		PintCollisionGroup GroupWheel = 1;
		PintCollisionGroup GroupStruct = 1;

		// The following code will create the vehicle with or without articulations
		const PintArticHandle Articulation = UseArticulations ? pint.CreateArticulation(PINT_ARTICULATION_CREATE()) : null;

		const udword NbPts = GetInt(60, mEditBox_WheelTess);
		const udword Multiplier = GetInt(1, mEditBox_Multiplier);
		const float WheelFriction = GetFloat(1.0f, mEditBox_WheelFriction);
		const float ChassisMass = GetFloat(40.0f, mEditBox_ChassisMass);
		const float WheelMass = GetFloat(15.0f, mEditBox_WheelMass);
		const float StructMass = GetFloat(1.0f, mEditBox_StructMass);
		const float WheelRadius = GetFloat(0.5f, mEditBox_WheelRadius);
		const float WheelWidth = GetFloat(0.3f, mEditBox_WheelWidth);
		const float ChassisX = GetFloat(1.5f, mEditBox_ChassisX);
		const float ChassisY = GetFloat(0.5f, mEditBox_ChassisY);
		const float ChassisZ = GetFloat(1.0f, mEditBox_ChassisZ);
		const float COMLocalOffset = GetFloat(-1.0f, mEditBox_COMLocalOffset);
		const float AxleRadius = GetFloat(0.1f, mEditBox_AxleRadius);
		const float AxleWidth = GetFloat(1.0f, mEditBox_AxleWidth);

		const PINT_MATERIAL_CREATE WheelMaterial(WheelFriction, WheelFriction, 0.0f);

//		const float y = 4.0f;	// ###
		const float y = ChassisY + WheelRadius;

		// When not using articulations we'll batch the joints to create them all at once in the end,
		// which allows us to easily create them multiple times for improved stability.
		PINT_HINGE_JOINT_CREATE Hinges[8];
		udword NbHinges = 0;

		// Chassis
		const Point Chassis(ChassisX, ChassisY, ChassisZ);
		const Point ChassisPos(0.0f, y + Chassis.y, 0.0f);
		PintActorHandle ChassisObject;
		{
			// ### Note that the chassis is going to use the scene's default material & friction value
			PINT_BOX_CREATE BoxDesc(Chassis);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mPosition		= ChassisPos;
			ObjectDesc.mMass			= ChassisMass;
			ObjectDesc.mCollisionGroup	= GroupChassis;
			ObjectDesc.mCOMLocalOffset	= Point(0.0f, COMLocalOffset, 0.0f);

			if(Articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ChassisObject = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
			}
			else
				ChassisObject = CreatePintObject(pint, ObjectDesc);

/*			if(0)
			{
				BoxDesc.mExtents	= Point(Chassis.x, 0.5f, Chassis.z*0.6f);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= CarPos + Point(0.0f, Chassis.y+BoxDesc.mExtents.y, 0.0f);
				ObjectDesc.mMass		= 4.0f;
				PintActorHandle h = CreatePintObject(pint, ObjectDesc);

				PINT_PRISMATIC_JOINT_CREATE Desc;
				Desc.mObject0		= h;
				Desc.mObject1		= ChassisObject;
				Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
			}*/
		}

		// Rear wheels + rear axle. We put them all in one rigid compound object, which means they will all be
		// connected without the need for joints, and the collision between these pieces will be naturally
		// filtered out.
		//###TODO: share this data and the wheel renderer
		const CylinderMesh Cylinder(NbPts, WheelRadius, WheelWidth*0.5f);
		const udword NbWheelVerts = Cylinder.mNbVerts;
		const Point* WheelVerts = Cylinder.mVerts;
		PintShapeRenderer* WheelRenderer = CreateConvexRenderer(NbWheelVerts, WheelVerts);

		const Point Axle(AxleRadius, AxleRadius, AxleWidth);
		{
			const Point RearAxleOffset(-Chassis.x, -Chassis.y - Axle.y, 0.0f);	// delta between chassis center and rear axle center.
			const float RearWheelMass = StructMass+WheelMass*2.0f;	// 1 axle + 2 wheels all put in the same compound object
			const bool UseSphereWheels = false;

			PINT_BOX_CREATE BoxDesc(Axle);
			BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

			const Point AxlePos = ChassisPos + RearAxleOffset;
			PINT_OBJECT_CREATE AxleDesc(&BoxDesc);
			AxleDesc.mPosition			= AxlePos;
			AxleDesc.mMass				= RearWheelMass;
			AxleDesc.mCollisionGroup	= GroupStruct;	//### Technically we'd need 3 groups here

			PINT_CONVEX_CREATE WheelDesc;
			PINT_CONVEX_CREATE WheelDesc2;
			PINT_SPHERE_CREATE SphereWheelDesc;
			PINT_SPHERE_CREATE SphereWheelDesc2;
			//###TODO: this uses the per-shape material feature (and it shows why it's useful) but some engines
			// don't support this. Check what happens for them.
			WheelDesc.mMaterial			= &WheelMaterial;
			WheelDesc2.mMaterial		= &WheelMaterial;
			SphereWheelDesc.mMaterial	= &WheelMaterial;
			SphereWheelDesc2.mMaterial	= &WheelMaterial;

			// The wheels will be located relative to the compound's position, i.e. AxlePos.
			// We put the wheel centers exactly on the left & right sides of the axle box.
			const Point RearWheelOffset(0.0f, 0.0f, Axle.z);

			if(UseSphereWheels)
			{
				SphereWheelDesc.mRadius		= WheelRadius;
				SphereWheelDesc.mRenderer	= CreateSphereRenderer(WheelRadius);
				SphereWheelDesc.mLocalPos	= RearWheelOffset;
				BoxDesc.SetNext(&SphereWheelDesc);

				SphereWheelDesc2.mRadius	= WheelRadius;
				SphereWheelDesc2.mRenderer	= CreateSphereRenderer(WheelRadius);
				SphereWheelDesc2.mLocalPos	= -RearWheelOffset;
				SphereWheelDesc.SetNext(&SphereWheelDesc2);
			}
			else
			{
				WheelDesc.mNbVerts		= NbWheelVerts;
				WheelDesc.mVerts		= WheelVerts;
				WheelDesc.mRenderer		= WheelRenderer;
				WheelDesc.mLocalPos		= RearWheelOffset;
				BoxDesc.SetNext(&WheelDesc);

				WheelDesc2.mNbVerts		= NbWheelVerts;
				WheelDesc2.mVerts		= WheelVerts;
				WheelDesc2.mRenderer	= WheelRenderer;
				WheelDesc2.mLocalPos	= -RearWheelOffset;
				WheelDesc.SetNext(&WheelDesc2);
			}

			// Now we attach the compound object to the chassis. We want a hinge between the two objects.
			// Collisions between the chassis and the rear compound will be disabled automatically in engines
			// that disable collision between jointed objects by default. For other engines we'd need to use
			// explicit collision filters here.
			PintActorHandle RearAxleObject;
			if(Articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent				= ChassisObject;
				ArticulatedDesc.mLocalPivot0		= RearAxleOffset;
				ArticulatedDesc.mLocalPivot1		= Point(0.0f, 0.0f, 0.0f);
				ArticulatedDesc.mX					= Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit	= false;
				ArticulatedDesc.mTwistLowerLimit	= -0.01f;
				ArticulatedDesc.mTwistUpperLimit	= 0.01f;
				ArticulatedDesc.mEnableSwingLimit	= true;
				ArticulatedDesc.mSwingYLimit		= 0.001f;//PI/6.0f;
				ArticulatedDesc.mSwingZLimit		= 0.001f;//PI/6.0f;
				RearAxleObject = pint.CreateArticulatedObject(AxleDesc, ArticulatedDesc, Articulation);
			}
			else
			{
				RearAxleObject = CreatePintObject(pint, AxleDesc);

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= ChassisObject;
				Desc.mObject1		= RearAxleObject;
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalPivot0	= RearAxleOffset;
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
//				PintJointHandle JointHandle = pint.CreateJoint(Desc);
//				ASSERT(JointHandle);
				Hinges[NbHinges++] = Desc;
			}
		}

		// Front wheels + front axle. The "FrontAxleObject" here will be cut into two smaller parts, but otherwise it is the
		// same as the rear axle (same size & location compared to the chassis).
		Point WheelPt[2];
		Point WheelGlobalPt[2];
		PintActorHandle FrontAxleObject[2];
		// We want Chassis.z + FrontAxle.z = AxleWidth to make sure the front & rear wheels are aligned.
		// i.e. FrontAxle.z = AxleWidth - Chassis.z
		const Point FrontAxle = Point(AxleRadius, AxleRadius, AxleWidth - Chassis.z);	//### Object to whom the wheel is directly attached

		const float z2 = FrontAxle.z*0.25f;
		for(udword i=0;i<2;i++)
		{
			const float Sign = i ? -1.0f : 1.0f;
			// delta between chassis center and front axle(s) center:
			// *  +Chassis.x to put the objects' centers exactly at the front of the chassis (similar to the rear axle, which was -Chassis.x)
			// *  "-Chassis.y - Axle.y" is exactly the same as for the rear axle
			// *  Now things are different for Z, i.e. along the axle. For the rear wheels we only had one compound object for the whole thing
			//    so the Z offset was 0.0f. But for the front wheels we have 2 compounds, created left & right in such a way that the centers
			//    of the object are exactly located on the sides of the chassis (hence +/- Chassis.z). Note that the compound's position will be
			//    the center of the "FrontAxle" box.
			const Point FrontAxleOffset = Point(Chassis.x, -Chassis.y - Axle.y, Sign*Chassis.z);
			const Point AxlePos = ChassisPos + FrontAxleOffset;

			{
				// So that's the "front axle" box, i.e. the object similar to the rear axle but which got cut in two pieces.
				PINT_BOX_CREATE BoxDesc(FrontAxle);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				// And then that one is the extra (new) box that doesn't exist for the rear axle. It goes back along -x towards
				// the rear axle, and we'll attach some kind of sway bar to these guys later.
				PINT_BOX_CREATE BoxDesc2(0.3f, FrontAxle.y*0.5f, z2);
				// 0.3 is the length of the piece towards the rear axle
				// 0.5 is some coeff<1 so that the sway bar apparatus doesn't touch the chassis
				// z2 is arbitrary, it has to be <1 and there's some restriction with the wheel width here
				BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
				// We move it so that it aligns perfectly with the front axle object.
				// "-BoxDesc.mExtents.x - BoxDesc2.mExtents.x" aligns it perfectly along x, and as we said, towards the rear axle.
				// "FrontAxle.z*0.5f" = z2 = BoxDesc.mExtents.z so we align it with the front axle but let the objects overlap on that axis.
				BoxDesc2.mLocalPos	= Point(-BoxDesc.mExtents.x - BoxDesc2.mExtents.x, 0.0f, -Sign*z2);
				BoxDesc.SetNext(&BoxDesc2);

				// So far these two boxes are in the same rigid body compound so collisions will be disabled between them.
				// This time we cannot put the wheels in the same compound, since they will need to rotate freely around Z,
				// and the front axles + sway bar thing cannot do so. So we'll attach the wheels to these compounds separately.
				// For now we add these compounds to our articulation. We want them to rotate around Y (i.e. to steer!). That's
				// another hinge.

/*				PINT_CONVEX_CREATE WheelDesc;
				WheelDesc.mNbVerts	= TotalNbVerts;
				WheelDesc.mVerts	= Verts;
				WheelDesc.mRenderer	= CreateConvexRenderer(WheelDesc.mNbVerts, WheelDesc.mVerts);
				WheelDesc.mLocalPos	= Point(0.0f, 0.0f, Coeff*FrontAxle.z);
				BoxDesc.mNext = &WheelDesc;*/

				PINT_OBJECT_CREATE AxleDesc(&BoxDesc);
				AxleDesc.mPosition			= AxlePos;
				AxleDesc.mMass				= StructMass;
				AxleDesc.mCollisionGroup	= GroupStruct;

				// We attach these to the chassis, so collisions between the chassis and the compound will be disabled by
				// default in most engines.
				if(Articulation)
				{
//				const float Limit = PI/4.0f;
				const float Limit = PI/6.0f;
//				const float Limit = 0.0f;
					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = ChassisObject;
					ArticulatedDesc.mLocalPivot0 = FrontAxleOffset;
					ArticulatedDesc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
/*					ArticulatedDesc.mX = Point(0.0f, 1.0f, 0.0f);
					ArticulatedDesc.mEnableTwistLimit = true;
					ArticulatedDesc.mTwistLowerLimit = -Limit;
					ArticulatedDesc.mTwistUpperLimit = Limit;
					ArticulatedDesc.mEnableSwingLimit = true;
					ArticulatedDesc.mSwingYLimit = 0.001f;//PI/6.0f;
					ArticulatedDesc.mSwingZLimit = 0.001f;//PI/6.0f;*/
					ArticulatedDesc.mX = Point(1.0f, 0.0f, 0.0f);
					ArticulatedDesc.mEnableTwistLimit = true;
					ArticulatedDesc.mTwistLowerLimit = -0.001f;
					ArticulatedDesc.mTwistUpperLimit = 0.001f;
					ArticulatedDesc.mEnableSwingLimit = true;
					ArticulatedDesc.mSwingYLimit = 0.001f;
					ArticulatedDesc.mSwingZLimit = Limit;
					FrontAxleObject[i] = pint.CreateArticulatedObject(AxleDesc, ArticulatedDesc, Articulation);
				}
				else
				{
					FrontAxleObject[i] = CreatePintObject(pint, AxleDesc);

					//### Creating this before or after the wheel joint will produce different artefacts/failures
					if(1 && !Articulation)
					{
						const float Limit = PI/6.0f;
		//				const float Limit = 0.0f;
						PINT_HINGE_JOINT_CREATE Desc;
						Desc.mObject0		= ChassisObject;
						Desc.mObject1		= FrontAxleObject[i];
						Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
						Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
						Desc.mLocalPivot0	= FrontAxleOffset;
						Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
						Desc.mLimits.Set(-Limit, Limit);
						Hinges[NbHinges++] = Desc;
					}
				}

				WheelPt[i] = Point(-BoxDesc.mExtents.x - BoxDesc2.mExtents.x*2.0f, 0.0f, -Sign*z2);
				WheelGlobalPt[i] = AxlePos + WheelPt[i];
			}

			// Create & attach front wheels to the previous compound
			if(1)
			{
				PINT_CONVEX_CREATE WheelDesc(NbWheelVerts, WheelVerts);
				WheelDesc.mRenderer	= WheelRenderer;
				WheelDesc.mMaterial	= &WheelMaterial;

				// AxlePos is the center of the "FrontAxle" object, so we just put the wheel's center on the
				// left or right side of it.
				PINT_OBJECT_CREATE ObjectDesc(&WheelDesc);
				ObjectDesc.mPosition		= AxlePos + Point(0.0f, 0.0f, Sign*FrontAxle.z);
				ObjectDesc.mMass			= WheelMass;
				ObjectDesc.mCollisionGroup	= GroupWheel;
//				ObjectDesc.mAngularVelocity	= Point(0.0, 0.0f, -100.0f);

				// We simply attach this to the previous front-axle compound, using an hinge to let the wheel rotate freely.
				if(Articulation)
				{
					PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = FrontAxleObject[i];
					ArticulatedDesc.mLocalPivot0 = Point(0.0f, 0.0f, Sign*FrontAxle.z);
					ArticulatedDesc.mLocalPivot1 = Point(0.0f, 0.0f, 0.0f);
					ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
					ArticulatedDesc.mEnableTwistLimit = false;
					ArticulatedDesc.mTwistLowerLimit = -0.01f;
					ArticulatedDesc.mTwistUpperLimit = 0.01f;
					ArticulatedDesc.mEnableSwingLimit = true;
					ArticulatedDesc.mSwingYLimit = 0.001f;//PI/6.0f;
					ArticulatedDesc.mSwingZLimit = 0.001f;//PI/6.0f;

					ArticulatedDesc.mUseMotor					= true;
					ArticulatedDesc.mMotor.mExternalCompliance	= 1.0f;
					ArticulatedDesc.mMotor.mInternalCompliance	= 1.0f;
					ArticulatedDesc.mMotor.mDamping				= 10000.0f;
					ArticulatedDesc.mMotor.mStiffness			= 0.0f;
					ArticulatedDesc.mMotor.mTargetVelocity		= Point(-10.0f, 0.0f, 0.0f);

					PintActorHandle WheelObject = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
				}
				else
				{
					PintActorHandle WheelObject = CreatePintObject(pint, ObjectDesc);

					PINT_HINGE_JOINT_CREATE Desc;
					Desc.mObject0		= WheelObject;
					Desc.mObject1		= FrontAxleObject[i];
					Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
					Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
					Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
					Desc.mLocalPivot1	= Point(0.0f, 0.0f, Sign*FrontAxle.z);
//					Desc.mDriveVelocity	= 20.0f;
//					Desc.mUseMotor		= true;
					Hinges[NbHinges++] = Desc;
				}
			}

			if(0 && !Articulation)
			{
				const float Limit = PI/6.0f;
//				const float Limit = 0.0f;
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= ChassisObject;
				Desc.mObject1		= FrontAxleObject[i];
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalPivot0	= FrontAxleOffset;
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLimits.Set(-Limit, Limit);
				Hinges[NbHinges++] = Desc;
			}
		}

		// Finally, we just connect the two "front axle" compounds with a rod, to make sure the front wheels
		// always have the same orientation.
		if(1)
		{
			const float Length = WheelGlobalPt[0].Distance(WheelGlobalPt[1]);
			if(0)
			{
				PINT_DISTANCE_JOINT_CREATE Desc;
				Desc.mObject0			= FrontAxleObject[0];
				Desc.mObject1			= FrontAxleObject[1];
				Desc.mLocalPivot0		= WheelPt[0];
				Desc.mLocalPivot1		= WheelPt[1];
				Desc.mLimits.mMinValue	= Length;
				Desc.mLimits.mMaxValue	= Length;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			else
			{
				PINT_BOX_CREATE BoxDesc(0.05f, 0.05f, Length*0.5f/* - z2*/);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition		= (WheelGlobalPt[0] + WheelGlobalPt[1])*0.5f;
				ObjectDesc.mMass			= StructMass;
				ObjectDesc.mCollisionGroup	= GroupStruct;
				PintActorHandle RodObject = CreatePintObject(pint, ObjectDesc);

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
				Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);

				Desc.mObject0		= FrontAxleObject[0];
				Desc.mObject1		= RodObject;
				Desc.mLocalPivot0	= WheelPt[0];
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, BoxDesc.mExtents.z);
				Hinges[NbHinges++] = Desc;

				Desc.mObject0		= FrontAxleObject[1];
				Desc.mObject1		= RodObject;
				Desc.mLocalPivot0	= WheelPt[1];
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, -BoxDesc.mExtents.z);
				Hinges[NbHinges++] = Desc;
			}
		}

		ASSERT(NbHinges<8);
		for(udword j=0;j<Multiplier;j++)
		{
			for(udword i=0;i<NbHinges;i++)
			{
				PintJointHandle h = pint.CreateJoint(Hinges[i]);
				ASSERT(h);
			}
		}

		if(Articulation)
			pint.AddArticulationToScene(Articulation);
		return true;
	}

END_TEST(ArticulatedVehicle)

///////////////////////////////////////////////////////////////////////////////
