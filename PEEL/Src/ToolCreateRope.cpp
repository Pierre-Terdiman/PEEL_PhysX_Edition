///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tool.h"
#include "ToolCreateRope.h"
#include "PintShapeRenderer.h"
#include "PintObjectsManager.h"
#include "Camera.h"
#include "GLRenderHelpers.h"
#include "GUI_PosEdit.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gButtonLabel0 = "Define 1st anchor";
static const char* gButtonLabel1 = "Define 2nd anchor";

///////////////////////////////////////////////////////////////////////////////

ToolCreateRope::ToolCreateRope() :
	mPendingAction	(NONE)
{
}

ToolCreateRope::~ToolCreateRope()
{
}

void ToolCreateRope::Reset(udword pint_index)
{
	if(mButton0)
	{
		mButton0->SetLabel(gButtonLabel0);
		mButton0->SetButtonDown(false);
	}
	if(mButton1)
	{
		mButton1->SetLabel(gButtonLabel1);
		mButton1->SetButtonDown(false);
	}
	mActiveButton = null;
	if(mEditPos0)
		mEditPos0->Reset();
	if(mEditPos1)
		mEditPos1->Reset();
	mPendingAction = NONE;
}

void ToolCreateRope::MouseMoveCallback(const MouseInfo& mouse)
{
}

void ToolCreateRope::PreRenderCallback()
{
}

// Workaround for super weird joint bug
static void workaround(Pint& pint, PintActorHandle h)
{
	if(!h)
		return;
//					const PR pose = pint.GetWorldTransform(mActor0);
//					pint.SetWorldTransform(mActor0, pose);

//	const bool isKine = pint.IsKinematic(h);
//	if(
	pint.EnableKinematic(h, true);
	pint.EnableKinematic(h, false);

	// This part we'll need to keep
	Pint_Actor* API = pint.GetActorAPI();
	if(API)
		API->WakeUp(h);
}

void ToolCreateRope::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	GLRenderHelpers::DrawFrame(mEditPos0->mEdited, GetFrameSize());
	GLRenderHelpers::DrawFrame(mEditPos1->mEdited, GetFrameSize());

	GLRenderHelpers::DrawLine(mEditPos0->mEdited, mEditPos1->mEdited, Point(1.0f, 1.0f, 0.0f));

	if(mPendingAction==CREATE_ROPE)
		CreateRope(pint);
}

void ToolCreateRope::PostRenderCallback()
{
	mPendingAction=NONE;
}

void ToolCreateRope::RightDownCallback(Pint& pint, udword pint_index)
{
	if(mActiveButton && mActiveButton->GetButtonDown())
	{
		PintRaycastHit Hit;
		if(Raycast(pint, Hit, mOrigin, mDir) && !IsDefaultEnv(pint, Hit.mTouchedActor))
		{
			if(mActiveButton==mButton0)
			{
				mEditPos0->SetPos(Hit.mImpact);
			}
			else if(mActiveButton==mButton1)
			{
				mEditPos1->SetPos(Hit.mImpact);
			}

			mActiveButton->SetButtonDown(false);
		}
	}
}

bool ToolCreateRope::CreateRope(Pint& pint)
{
	PintCaps caps;
	pint.GetCaps(caps);

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

	Point p0 = mEditPos0->mEdited;
	Point p1 = mEditPos1->mEdited;
	Point Dir = p1 - p0;
	Dir.Normalize();

	const float RopeLength = p0.Distance(p1);

	const float Radius = 0.1f;
	const udword NbSpheres = udword(RopeLength/(Radius*2.0f));
	float Remain = RopeLength - float(NbSpheres)*Radius*2.0f;
	if(Remain<0.0f)
		Remain = 0.0f;

	p0 += Dir*(Radius+Remain*0.5f);
	p1 -= Dir*(Radius+Remain*0.5f);

	const float Mass = 1.0f;

	const Point InitPos = p0;
	Point Pos = InitPos;
	PintActorHandle* Handles = new PintActorHandle[NbSpheres];
	Point* Positions = ICE_NEW(Point)[NbSpheres];

	udword GroupBit = 0;
	{
		PINT_SPHERE_CREATE SphereDesc(Radius);
		SphereDesc.mRenderer = CreateSphereRenderer(Radius);

		const Point Offset = Dir * (Radius * 2.0f);
		const Point LocalPivot0	= Point(0.0f, 0.0f, 0.0f);
		const Point LocalPivot1	= - Dir * (Radius * 2.0f);

		for(udword i=0;i<NbSpheres;i++)
		{
			Positions[i] = Pos;

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.SetShape(&SphereDesc);
			ObjectDesc.mMass			= Mass;
//			if(i==0)
//				ObjectDesc.mMass		= 0.0f;
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
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "GUI_Helpers.h"

void ToolCreateRope::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword x = 4;
	sdword y = 4;
	const sdword YStep = 20;

	struct Callbacks
	{
		static void DefineAnchor(IceButton& button, void* user_data)
		{
			ToolCreateRope* tcj = reinterpret_cast<ToolCreateRope*>(user_data);
			if(tcj->mActiveButton && tcj->mActiveButton!=&button)
			{
				tcj->mActiveButton->SetButtonDown(false);
				tcj->mActiveButton = null;
			}

			if(!button.GetButtonDown())
			{
				button.SetButtonDown(true);
				tcj->mActiveButton = &button;
				//button.SetBitmapColor(0,0,0);
				//button.EnableBitmap();
			}
			else
			{
				button.SetButtonDown(false);
				tcj->mActiveButton = null;
			}
		}

		static void CreateRope(IceButton& button, void* user_data)
		{
			ToolCreateRope* tcj = reinterpret_cast<ToolCreateRope*>(user_data);
			tcj->mPendingAction = CREATE_ROPE;
		}
	};

//	IceButton* B = helper.CreateButton(parent, 0, x, y, 100, 20, "Button", &owner, Local::ButtonCB, null);
	{
		ButtonDesc BD;
		BD.mParent	= parent;
		BD.mX		= x;
		BD.mY		= y;
		BD.mWidth	= 150;
		BD.mHeight	= 20;
		BD.mLabel	= gButtonLabel0;
		BD.mStyle	= BUTTON_PUSHLIKE;
		{
			IceButton* B = ICE_NEW(IceButton)(BD);
			B->SetCallback(Callbacks::DefineAnchor);
			B->SetUserData(this);
			//B->EnableBitmap();
			//B->SetBitmapColor(255,0,0);
			owner.Register(B);
			mButton0 = B;
		}

		y += YStep;

		BD.mY		= y;
		BD.mLabel	= gButtonLabel1;
		{
			IceButton* B = ICE_NEW(IceButton)(BD);
			B->SetCallback(Callbacks::DefineAnchor);
			B->SetUserData(this);
			//B->EnableBitmap();
			//B->SetBitmapColor(0,255,0);
			owner.Register(B);
			mButton1 = B;
		}

		y += YStep;

		BD.mX		= 200;
		BD.mY		= 4;
		BD.mLabel	= "Create joint";
		{
			IceButton* B = ICE_NEW(IceButton)(BD);
			B->SetCallback(Callbacks::CreateRope);
			B->SetUserData(this);
			owner.Register(B);
		}
	}

	{
		y += YStep;
		y += YStep;

		WindowDesc WD;
		WD.mParent	= parent;
		WD.mX		= x;
		WD.mY		= y;
		WD.mWidth	= 90;
		WD.mHeight	= 70;
		WD.mType	= WINDOW_NORMAL;
		WD.mStyle	= WSTYLE_BORDER;
//		WD.mStyle	= WSTYLE_CLIENT_EDGES;
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
		mEditPos0 = ICE_NEW(EditPosWindow)(WD);
		owner.Register(mEditPos0);

		const sdword Spacing = 110;

		WD.mX		= x + Spacing;
		mEditPos1 = ICE_NEW(EditPosWindow)(WD);
		owner.Register(mEditPos1);

		if(0)
		{
			WD.mX		= x + Spacing*2;
			EditPosWindow* mEditPos2 = ICE_NEW(EditPosWindow)(WD);
			owner.Register(mEditPos2);

			WD.mX		= x + Spacing*3;
			EditPosWindow* mEditPos3 = ICE_NEW(EditPosWindow)(WD);
			owner.Register(mEditPos3);
		}

		y += 74;
	}
}

