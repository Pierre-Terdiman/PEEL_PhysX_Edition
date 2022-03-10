///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tool.h"
#include "ToolCreateJoint.h"
#include "PintShapeRenderer.h"
#include "PintObjectsManager.h"
#include "Camera.h"
#include "GLRenderHelpers.h"
#include "GUI_PosEdit.h"
#include "GUI_Helpers.h"

///////////////////////////////////////////////////////////////////////////////

// Cannot use helper because we need a BUTTON_PUSHLIKE style
static IceButton* GUI_CreatePushButton(IceWidget* parent, udword id, sdword x, sdword y, sdword width, sdword height, const char* label, Widgets* owner, BCallback callback, void* user_data, const char* tooltip)
{
	ButtonDesc BD;
	BD.mStyle		= BUTTON_PUSHLIKE;
	BD.mCallback	= callback;
	BD.mUserData	= user_data;
	BD.mID			= id;
	BD.mParent		= parent;
	BD.mX			= x;
	BD.mY			= y;
	BD.mWidth		= width;
	BD.mHeight		= height;
	BD.mLabel		= label;
	IceButton* B = ICE_NEW(IceButton)(BD);
	B->SetVisible(true);

	if(owner)
		owner->Register(B);

	if(tooltip)
		B->AddToolTip(tooltip);
	return B;
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	enum InternalJointType
	{
		IJT_SPHERICAL_X,
		IJT_SPHERICAL_Y,
		IJT_SPHERICAL_Z,
		IJT_HINGE_X,
		IJT_HINGE_Y,
		IJT_HINGE_Z,
		IJT_PRISMATIC_X,
		IJT_PRISMATIC_Y,
		IJT_PRISMATIC_Z,
		IJT_FIXED,
		IJT_DISTANCE,
		IJT_D6,
		IJT_GEAR,
		IJT_RACK,
	};
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class JointListBoxes
	{
		public:
								JointListBoxes()	{}
								~JointListBoxes()	{}

				void			Reset();
				void			Create(sdword x, sdword y, IceWidget* parent, Widgets& owner);
				bool			FillListBox(Pint& pint, PintActorHandle actor, udword id, PintJoint jt0, PintJoint jt1, PintJoint jt2);
				PintJointHandle	GetSelectedJoint(udword id, Pint_Joint* joint_api, PintActorHandle actor, PR& pose);

				ListBoxPtr		mJointsFromActor0;
				ListBoxPtr		mJointsFromActor1;
	};

void JointListBoxes::Reset()
{
	if(mJointsFromActor0)
		mJointsFromActor0->RemoveAll();
	if(mJointsFromActor1)
		mJointsFromActor1->RemoveAll();
}

void JointListBoxes::Create(sdword x, sdword y, IceWidget* parent, Widgets& owner)
{
	ListBoxDesc LBD;
	LBD.mParent			= parent;
	LBD.mX				= x;
	LBD.mY				= y;
	LBD.mWidth			= 160;
	LBD.mHeight			= 100;
	LBD.mLabel			= "List box";
	mJointsFromActor0	= ICE_NEW(IceListBox)(LBD);
	owner.Register(mJointsFromActor0);

	LBD.mX				= x + 160 + 10;
	LBD.mLabel			= "List box";
	mJointsFromActor1	= ICE_NEW(IceListBox)(LBD);
	owner.Register(mJointsFromActor1);
}

bool JointListBoxes::FillListBox(Pint& pint, PintActorHandle actor, udword id, PintJoint jt0, PintJoint jt1, PintJoint jt2)
{
	Pint_Actor* ActorAPI = pint.GetActorAPI();
	if(!ActorAPI)
		return false;

	Pint_Joint* JointAPI = pint.GetJointAPI();
	if(!JointAPI)
		return false;

	IceListBox* LB = id ? mJointsFromActor1 : mJointsFromActor0;
	if(!LB)
		return false;
	LB->RemoveAll();

	const udword NbJoints = ActorAPI->GetNbJoints(actor);
	udword Index=0;
	for(udword i=0;i<NbJoints;i++)
	{
		const PintJointHandle jh = ActorAPI->GetJoint(actor, i);
		if(jh)
		{
			const PintJoint jt = JointAPI->GetType(jh);
			if(jt==jt0 || jt==jt1 || jt==jt2)
			{
				const char* JointName = JointAPI->GetName(jh);
				if(!JointName)
					JointName = "(unnamed joint)";

				LB->Add(JointName);
				if(!Index)
					LB->Select(0);
				LB->SetItemData(Index++, jh);
			}
		}
	}
	return true;
}

PintJointHandle JointListBoxes::GetSelectedJoint(udword id, Pint_Joint* joint_api, PintActorHandle actor, PR& pose)
{
	IceListBox* LB = id ? mJointsFromActor1 : mJointsFromActor0;
	if(!LB)
		return null;

	const int Selected = LB->GetSelectedIndex();
	//ASSERT(Selected!=-1);
	if(Selected==-1)
		return null;
	const PintJointHandle jh = PintJointHandle(LB->GetItemData(Selected));

	PintActorHandle a0, a1;
	joint_api->GetActors(jh, a0, a1);

	PR pose0, pose1;
	joint_api->GetFrames(jh, &pose0, &pose1);

	if(actor==a0)
		pose = pose0;
	else if(actor==a1)
		pose = pose1;
	return jh;
}



	class GearJointWindow : public IceWindow
	{
		public:
								GearJointWindow(const WindowDesc& desc);
		virtual					~GearJointWindow();

				void			Reset();
				void			Create(PintGUIHelper& helper, Widgets& owner);
				bool			ListHinges(Pint& pint, PintActorHandle actor, udword id);

				EditBoxPtr		mEditBox_NbTeeth0;
				EditBoxPtr		mEditBox_NbTeeth1;
//				IceComboBox*	mComboBox_ErrorSign;
				JointListBoxes	mListBoxes;
	};

GearJointWindow::GearJointWindow(const WindowDesc& desc) :
	IceWindow			(desc)
//	mComboBox_ErrorSign	(null)
{
}

GearJointWindow::~GearJointWindow()
{
}

void GearJointWindow::Reset()
{
	if(mEditBox_NbTeeth0)
		mEditBox_NbTeeth0->SetText("0");
	if(mEditBox_NbTeeth1)
		mEditBox_NbTeeth1->SetText("0");
	mListBoxes.Reset();
}

void GearJointWindow::Create(PintGUIHelper& helper, Widgets& owner)
{
	sdword y = 4;
	sdword x = 4;

	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword OffsetX = 80;
	const sdword LabelWidth = 80;
	helper.CreateLabel(this, x, y+LabelOffsetY, LabelWidth, 20, "Nb teeth0:", &owner);
	mEditBox_NbTeeth0 = helper.CreateEditBox(this, 0, x+OffsetX, y, EditBoxWidth, 20, "0", &owner, EDITBOX_INTEGER_POSITIVE, null, null);

	y += 20;
	helper.CreateLabel(this, x, y+LabelOffsetY, LabelWidth, 20, "Nb teeth1:", &owner);
	mEditBox_NbTeeth1 = helper.CreateEditBox(this, 0, x+OffsetX, y, EditBoxWidth, 20, "0", &owner, EDITBOX_INTEGER_POSITIVE, null, null);

	y += 20;
/*	helper.CreateLabel(this, x, y+LabelOffsetY, LabelWidth, 20, "Error sign:", &owner);
	mComboBox_ErrorSign = CreateComboBox<IceComboBox>(this, 0, x+OffsetX, y, 150, 20, "Error sign", &owner, null);
	mComboBox_ErrorSign->Add("No error correction");
	mComboBox_ErrorSign->Add("+1");
	mComboBox_ErrorSign->Add("-1");
	mComboBox_ErrorSign->Select(0);*/

	mListBoxes.Create(x, y+25, this, owner);
}

bool GearJointWindow::ListHinges(Pint& pint, PintActorHandle actor, udword id)
{
	return mListBoxes.FillListBox(pint, actor, id, PINT_JOINT_HINGE, PINT_JOINT_HINGE2, PINT_JOINT_D6);
}



	class RackJointWindow : public IceWindow
	{
		public:
								RackJointWindow(const WindowDesc& desc);
		virtual					~RackJointWindow();

				void			Reset();
				void			Create(PintGUIHelper& helper, Widgets& owner);
				bool			ListHinges(Pint& pint, PintActorHandle actor);
				bool			ListPrismatic(Pint& pint, PintActorHandle actor);

				EditBoxPtr		mEditBox_NbTeethPinion;
				EditBoxPtr		mEditBox_NbTeethRack;
				EditBoxPtr		mEditBox_RackLength;
				CheckBoxPtr		mCheckBox_SelectShape;
//				IceComboBox*	mComboBox_ErrorSign;
				JointListBoxes	mListBoxes;
				AABB			mRackBounds;
	};

RackJointWindow::RackJointWindow(const WindowDesc& desc) :
	IceWindow				(desc)
//	mComboBox_ErrorSign		(null)
{
	mRackBounds.SetEmpty();
}

RackJointWindow::~RackJointWindow()
{
}

void RackJointWindow::Reset()
{
	if(mEditBox_NbTeethPinion)
		mEditBox_NbTeethPinion->SetText("0");
	if(mEditBox_NbTeethRack)
		mEditBox_NbTeethRack->SetText("0");
	if(mEditBox_RackLength)
		mEditBox_RackLength->SetText("1.0");
	if(mCheckBox_SelectShape)
		mCheckBox_SelectShape->SetChecked(false);
	mListBoxes.Reset();
	mRackBounds.SetEmpty();
}

void RackJointWindow::Create(PintGUIHelper& helper, Widgets& owner)
{
	sdword y = 4;
	sdword x = 4;

	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword OffsetX = 80;
	const sdword LabelWidth = 80;
	helper.CreateLabel(this, x, y+LabelOffsetY, LabelWidth, 20, "Nb teeth pinion:", &owner);
	mEditBox_NbTeethPinion = helper.CreateEditBox(this, 0, x+OffsetX, y, EditBoxWidth, 20, "0", &owner, EDITBOX_INTEGER_POSITIVE, null, null);

	y += 20;
	helper.CreateLabel(this, x, y+LabelOffsetY, LabelWidth, 20, "Nb teeth rack:", &owner);
	mEditBox_NbTeethRack = helper.CreateEditBox(this, 0, x+OffsetX, y, EditBoxWidth, 20, "0", &owner, EDITBOX_INTEGER_POSITIVE, null, null);

	y += 20;
	helper.CreateLabel(this, x, y+LabelOffsetY, LabelWidth, 20, "Rack length:", &owner);
	mEditBox_RackLength = helper.CreateEditBox(this, 0, x+OffsetX, y, EditBoxWidth, 20, "1.0", &owner, EDITBOX_FLOAT_POSITIVE, null, null);

	mCheckBox_SelectShape = helper.CreateCheckBox(this, 0, x+150, y, 100, 20, "Select shape", &owner, false, null);

/*	y += 20;
	helper.CreateLabel(this, x, y+LabelOffsetY, LabelWidth, 20, "Error sign:", &owner);
	mComboBox_ErrorSign = CreateComboBox<IceComboBox>(this, 0, x+OffsetX, y, 150, 20, "Error sign", &owner, null);
	mComboBox_ErrorSign->Add("No error correction");
	mComboBox_ErrorSign->Add("+1");
	mComboBox_ErrorSign->Add("-1");
	mComboBox_ErrorSign->Select(0);*/

	mListBoxes.Create(x, y+25, this, owner);
}

bool RackJointWindow::ListHinges(Pint& pint, PintActorHandle actor)
{
	return mListBoxes.FillListBox(pint, actor, 0, PINT_JOINT_HINGE, PINT_JOINT_HINGE2, PINT_JOINT_D6);
}

bool RackJointWindow::ListPrismatic(Pint& pint, PintActorHandle actor)
{
	return mListBoxes.FillListBox(pint, actor, 1, PINT_JOINT_PRISMATIC, PINT_JOINT_PRISMATIC, PINT_JOINT_D6);
}

/*float GetErrorSign(IceComboBox* cb)
{
	const int Index = cb->GetSelectedIndex();
	if(Index==1)
		return 1.0f;
	else if(Index==2)
		return -1.0f;
	return 0.0f;
}*/

	class JointTypeComboBox : public IceComboBox
	{
		public:
						JointTypeComboBox(const ComboBoxDesc& desc) : IceComboBox(desc){}

		virtual	void	OnComboBoxEvent(ComboBoxEvent event)
		{
			if(event==CBE_SELECTION_CHANGED)
			{
				ToolCreateJointBase* tcjb = reinterpret_cast<ToolCreateJointBase*>(GetUserData());
				tcjb->OnComboBoxChange();
			}
		}
	};
}

///////////////////////////////////////////////////////////////////////////////

ToolCreateJointBase::ToolCreateJointBase() :
	mPint			(null),
	mEditRot0		(Idt),
	mEditRot1		(Idt),
	mGearJointWindow(null),
	mRackJointWindow(null),
	mPendingAction	(NONE)
{
}

ToolCreateJointBase::~ToolCreateJointBase()
{
}

void ToolCreateJointBase::OnComboBoxChange()
{
	mEditPos0->SetVisible(false);
	mEditPos1->SetVisible(false);
	mSnap0->SetVisible(false);
	mSnap1->SetVisible(false);
	mFromShape0->SetVisible(false);
	mFromShape1->SetVisible(false);
	mGearJointWindow->SetVisible(false);
	mRackJointWindow->SetVisible(false);

//	ResetButtonNames(mActor0==null, mActor1==null);

	ASSERT(mJointType.mWidget);
	const udword SelectedIndex = mJointType->GetSelectedIndex();
	if(SelectedIndex>=IJT_SPHERICAL_X && SelectedIndex<=IJT_SPHERICAL_Z)
	{
		mEditPos0->SetVisible(true);
		mEditPos1->SetVisible(true);
		mSnap0->SetVisible(true);
		mSnap1->SetVisible(true);
		mFromShape0->SetVisible(true);
		mFromShape1->SetVisible(true);
	}
	else if(SelectedIndex>=IJT_HINGE_X && SelectedIndex<=IJT_HINGE_Z)
	{
		mEditPos0->SetVisible(true);
		mEditPos1->SetVisible(true);
		mSnap0->SetVisible(true);
		mSnap1->SetVisible(true);
		mFromShape0->SetVisible(true);
		mFromShape1->SetVisible(true);
	}
	else if(SelectedIndex>=IJT_PRISMATIC_X && SelectedIndex<=IJT_PRISMATIC_Z)
	{
		mEditPos0->SetVisible(true);
		mEditPos1->SetVisible(true);
		mSnap0->SetVisible(true);
		mSnap1->SetVisible(true);
		mFromShape0->SetVisible(true);
		mFromShape1->SetVisible(true);
	}
	else if(SelectedIndex==IJT_FIXED)
	{
		mEditPos0->SetVisible(true);
		mEditPos1->SetVisible(true);
		mSnap0->SetVisible(true);
		mSnap1->SetVisible(true);
		mFromShape0->SetVisible(true);
		mFromShape1->SetVisible(true);
	}
	else if(SelectedIndex==IJT_GEAR)
	{
		mGearJointWindow->SetVisible(true);
	}
	else if(SelectedIndex==IJT_RACK)
	{
		mRackJointWindow->SetVisible(true);
	}
}

void ToolCreateJointBase::Reset(udword pint_index)
{
	if(mJointType)
		mJointType->Select(IJT_SPHERICAL_X);
	OnComboBoxChange();

	mPint = null;
	mPendingAction = NONE;

	if(mEditPos0)
		mEditPos0->Reset();
	if(mEditPos1)
		mEditPos1->Reset();

	mEditRot0.Identity();
	mEditRot1.Identity();

	if(mGearJointWindow)
		static_cast<GearJointWindow*>(mGearJointWindow)->Reset();
	if(mRackJointWindow)
		static_cast<RackJointWindow*>(mRackJointWindow)->Reset();
}

void ToolCreateJointBase::PostRenderCallback()
{
	mPendingAction=NONE;
}

void ToolCreateJointBase::CreateJointComboBox(PintGUIHelper& helper, IceWidget* parent, Widgets& owner, sdword x, sdword y)
{
	helper.CreateLabel(parent, x, y, 90, 20, "Joint type:", &owner);
	mJointType = CreateComboBox<JointTypeComboBox>(parent, 0, x+60, y, 150, 20, "Joint type", &owner, null/*gTooltip_CurrentTool*/);
	mJointType->SetUserData(this);
	mJointType->Add("Spherical X");		// IJT_SPHERICAL_X
	mJointType->Add("Spherical Y");		// IJT_SPHERICAL_Y
	mJointType->Add("Spherical Z");		// IJT_SPHERICAL_Z
	mJointType->Add("Hinge X");			// IJT_HINGE_X
	mJointType->Add("Hinge Y");			// IJT_HINGE_Y
	mJointType->Add("Hinge Z");			// IJT_HINGE_Z
	mJointType->Add("Prismatic X");		// IJT_PRISMATIC_X
	mJointType->Add("Prismatic Y");		// IJT_PRISMATIC_Y
	mJointType->Add("Prismatic Z");		// IJT_PRISMATIC_Z
	mJointType->Add("Fixed");			// IJT_FIXED
	mJointType->Add("Distance");		// IJT_DISTANCE
	mJointType->Add("D6");				// IJT_D6
	mJointType->Add("Gear");			// IJT_GEAR
	mJointType->Add("Rack & pinion");	// IJT_RACK
	mJointType->Select(IJT_SPHERICAL_X);
}

void ToolCreateJointBase::CreateJointWindows(PintGUIHelper& helper, IceWidget* parent, Widgets& owner, sdword x, sdword y)
{
	struct Callbacks
	{
		static void Snap(IceButton& button, void* user_data)
		{
			ToolCreateJointBase* tcj = reinterpret_cast<ToolCreateJointBase*>(user_data);
			tcj->OnSnapButton(button);
		}

		static void FromShape(IceButton& button, void* user_data)
		{
			ToolCreateJointBase* tcj = reinterpret_cast<ToolCreateJointBase*>(user_data);
			tcj->OnFromShape(button);
		}
	};

	const sdword YStep = 20;

	sdword SavedY;
	{
		y += YStep;
		y += YStep;
SavedY = y;

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

		mSnap0 = helper.CreateButton(parent, 0, x+Spacing*0, y, 90, 20, "Snap to 1", &owner, Callbacks::Snap, null);	mSnap0->SetUserData(this);
		mSnap1 = helper.CreateButton(parent, 1, x+Spacing*1, y, 90, 20, "Snap to 0", &owner, Callbacks::Snap, null);	mSnap1->SetUserData(this);
		if(0)
		{
			IceButton* B2 = helper.CreateButton(parent, 2, x+Spacing*2, y, 90, 20, "Snap to 1", &owner, Callbacks::Snap, null);	B2->SetUserData(this);
			IceButton* B3 = helper.CreateButton(parent, 3, x+Spacing*3, y, 90, 20, "Snap to 0", &owner, Callbacks::Snap, null);	B3->SetUserData(this);
		}

		y += YStep;
		mFromShape0 = helper.CreateButton(parent, 0, x+Spacing*0, y, 90, 20, "From shape", &owner, Callbacks::FromShape, null);	mFromShape0->SetUserData(this);
		mFromShape1 = helper.CreateButton(parent, 1, x+Spacing*1, y, 90, 20, "From shape", &owner, Callbacks::FromShape, null);	mFromShape1->SetUserData(this);
	}

	// Gear joint
	{
		y = SavedY;

		WindowDesc WD;
		WD.mParent	= parent;
		WD.mX		= x;
		WD.mY		= y;
		WD.mWidth	= 420;
		WD.mHeight	= 165;
		WD.mType	= WINDOW_NORMAL;
		//WD.mStyle	= WSTYLE_BORDER;
//		WD.mStyle	= WSTYLE_CLIENT_EDGES;
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
//		mGearJointWindow = ICE_NEW(IceWindow)(WD);
		GearJointWindow* GJW = ICE_NEW(GearJointWindow)(WD);
		GJW->Create(helper, owner);
		mGearJointWindow = GJW;
		owner.Register(mGearJointWindow);
	}

	// Rack joint
	{
		y = SavedY;

		WindowDesc WD;
		WD.mParent	= parent;
		WD.mX		= x;
		WD.mY		= y;
		WD.mWidth	= 420;
		WD.mHeight	= 165;
		WD.mType	= WINDOW_NORMAL;
		//WD.mStyle	= WSTYLE_BORDER;
//		WD.mStyle	= WSTYLE_CLIENT_EDGES;
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
		RackJointWindow* RJW = ICE_NEW(RackJointWindow)(WD);
		RJW->Create(helper, owner);
		mRackJointWindow = RJW;
		owner.Register(mRackJointWindow);
	}
}

///////////////////////////////////////////////////////////////////////////////

ToolCreateJoint::ToolCreateJoint() :
	mActor0	(null),
	mShape0	(null),
	mActor1	(null),
	mShape1	(null)
{
}

ToolCreateJoint::~ToolCreateJoint()
{
}

void ToolCreateJoint::Reset(udword pint_index)
{
	ToolCreateJointBase::Reset(pint_index);

	ResetButtonNames(true, true);

	if(mButton0)
		mButton0->SetButtonDown(false);
	if(mButton1)
		mButton1->SetButtonDown(false);
	mActiveButton = null;
	mActor0 = null;
	mShape0 = null;
	mActor1 = null;
	mShape1 = null;
	if(mEditBoxName)
		mEditBoxName->SetText("JointName");

//	if(mEditBox_NbTeeth0)
//		mEditBox_NbTeeth0->Set
//	mEditBox_NbTeeth1	(null),
}

void ToolCreateJoint::OnObjectReleased(Pint& pint, PintActorHandle removed_object)
{
	if(mPint==&pint)
	{
		if(removed_object==mActor0 || removed_object==mActor1)
		{
			mButton0->SetButtonDown(false);
			mButton1->SetButtonDown(false);
			ResetButtonNames(true, true);
			mActor0 = null;
			mShape0 = null;
			mActor1 = null;
			mShape1 = null;
			mActiveButton = null;
		}
	}
}

void ToolCreateJoint::MouseMoveCallback(const MouseInfo& mouse)
{
}

void ToolCreateJoint::PreRenderCallback()
{
}

// Workaround for super weird joint bug
static void workaround(Pint& pint, PintActorHandle h)
{
	if(!h)
		return;
//		const PR pose = pint.GetWorldTransform(mActor0);
//		pint.SetWorldTransform(mActor0, pose);

//	const bool isKine = pint.IsKinematic(h);
//	if(
	pint.EnableKinematic(h, true);
	pint.EnableKinematic(h, false);

	// This part we'll need to keep
	Pint_Actor* API = pint.GetActorAPI();
	if(API)
		API->WakeUp(h);
}

static void RenderJointFrames(Pint& pint, PintActorHandle actor, const Point& jointFramePos, const Quat& jointFrameRot)
{
	if(!actor)
		return;

	const PR ActorPose = pint.GetWorldTransform(actor);
	//GLRenderHelpers::DrawFrame(ActorPose.mPos);

//	GLRenderHelpers::DrawFrame(Pose.mPos + epw->mEdited, GetFrameSize());

	PR JointPose;
	if(0)
	{
		JointPose.mPos = jointFramePos * Matrix4x4(ActorPose);
//		JointPose.mRot = Quat(Identity);	// ### wrong
		JointPose.mRot = ActorPose.mRot;
	}
	else
	{
		Matrix4x4 JointFrameM = jointFrameRot;
		JointFrameM.SetTrans(jointFramePos);

		JointFrameM *= Matrix4x4(ActorPose);

		JointPose.mPos = JointFrameM.GetTrans();
		JointPose.mRot = JointFrameM;
	}

	GLRenderHelpers::DrawFrame(JointPose, GetFrameSize(), GetSymmetricFrames());
}

static void RenderActorData(Pint& pint, PintActorHandle actor, EditPosWindow* epw, const Quat& jointFrameRot)
{
	if(!actor)
		return;

	DrawBoxCorners(pint, actor, TOOLS_BOX_CORNERS_COLOR);

	if(epw)
	{
/*		const PR ActorPose = pint.GetWorldTransform(actor);
		//GLRenderHelpers::DrawFrame(ActorPose.mPos);

//		GLRenderHelpers::DrawFrame(Pose.mPos + epw->mEdited, GetFrameSize());

		PR JointPose;
		if(0)
		{
			JointPose.mPos = epw->mEdited * Matrix4x4(ActorPose);
//			JointPose.mRot = Quat(Identity);	// ### wrong
			JointPose.mRot = ActorPose.mRot;
		}
		else
		{
			Matrix4x4 JointFrameM = jointFrameRot;
			JointFrameM.SetTrans(epw->mEdited);

			JointFrameM *= Matrix4x4(ActorPose);

			JointPose.mPos = JointFrameM.GetTrans();
			JointPose.mRot = JointFrameM;
		}

		GLRenderHelpers::DrawFrame(JointPose, GetFrameSize(), GetSymmetricFrames());*/

		RenderJointFrames(pint, actor, epw->mEdited, jointFrameRot);
	}
}

static void SnapPos(EditPosWindow* mEditPos0, const EditPosWindow* mEditPos1, const PR& Pose0, const PR& Pose1)
{
	const Point LocalPivot1 = mEditPos1->mEdited;

		// ### super clunky, to rewrite
		Matrix4x4 M0 = Pose0;
		Matrix4x4 M1 = Pose1;

		//const Point wp0 = LocalPivot0 * M0;
		//const Point wp1 = LocalPivot1 * M1;
		//    LocalPivot0 * M0 = LocalPivot1 * M1
		//<=> LocalPivot0 = (LocalPivot1 * M1)*Inverse(M0)
		Matrix4x4 InvM0;
		InvertPRMatrix(InvM0, M0);

		mEditPos0->SetPos((LocalPivot1 * M1)*InvM0);
}

static void SnapRot(Quat& edit_rot0, const Quat& rot0, const Quat& rot1, const Quat& edit_rot1)
{
	Matrix3x3 am = rot0;
	am.Invert();

	Matrix3x3 jm = Matrix3x3(edit_rot1) * Matrix3x3(rot1) * am;
	edit_rot0 = jm;
}

static void Snap0(Pint& pint, PintActorHandle actor0, PintActorHandle actor1, EditPosWindow* edit_pos0, const EditPosWindow* edit_pos1, Quat& edit_rot0, const Quat& edit_rot1)
{
	//ASSERT(actor0);
	//ASSERT(actor1);
	ASSERT(edit_pos0);
	ASSERT(edit_pos1);

	const PR Pose0 = actor0 ? pint.GetWorldTransform(actor0) : PR(Idt);
	const PR Pose1 = actor1 ? pint.GetWorldTransform(actor1) : PR(Idt);

/*		//const Point LocalPivot0 = mEditPos0->mEdited;
	const Point LocalPivot1 = mEditPos1->mEdited;

		// ### super clunky, to rewrite
		Matrix4x4 M0 = Pose0;
		Matrix4x4 M1 = Pose1;

		//const Point wp0 = LocalPivot0 * M0;
		//const Point wp1 = LocalPivot1 * M1;
		//    LocalPivot0 * M0 = LocalPivot1 * M1
		//<=> LocalPivot0 = (LocalPivot1 * M1)*Inverse(M0)
		Matrix4x4 InvM0;
		InvertPRMatrix(InvM0, M0);

		mEditPos0->SetPos((LocalPivot1 * M1)*InvM0);*/
		SnapPos(edit_pos0, edit_pos1, Pose0, Pose1);

			//mEditRot0 = Pose1.mRot;

/*				Matrix3x3 am0 = Pose0.mRot;
			am0.Invert();

			Matrix3x3 jm0 = Matrix3x3(mEditRot1) * Matrix3x3(Pose1.mRot) * am0;
			mEditRot0 = jm0;*/
			SnapRot(edit_rot0, Pose0.mRot, Pose1.mRot, edit_rot1);
}

static void Snap1(Pint& pint, PintActorHandle actor0, PintActorHandle actor1, const EditPosWindow* edit_pos0, EditPosWindow* edit_pos1, const Quat& edit_rot0, Quat& edit_rot1)
{
//	ASSERT(actor0);
//	ASSERT(actor1);
	ASSERT(edit_pos0);
	ASSERT(edit_pos1);

	const PR Pose0 = actor0 ? pint.GetWorldTransform(actor0) : PR(Idt);
	const PR Pose1 = actor1 ? pint.GetWorldTransform(actor1) : PR(Idt);

/*		const Point LocalPivot0 = mEditPos0->mEdited;
	//const Point LocalPivot1 = mEditPos1->mEdited;

		// ### super clunky, to rewrite
		Matrix4x4 M0 = Pose0;
		Matrix4x4 M1 = Pose1;

		//const Point wp0 = LocalPivot0 * M0;
		//const Point wp1 = LocalPivot1 * M1;
		//    LocalPivot0 * M0 = LocalPivot1 * M1
		//<=> LocalPivot0 = (LocalPivot1 * M1)*Inverse(M0)
		Matrix4x4 InvM1;
		InvertPRMatrix(InvM1, M1);

		mEditPos1->SetPos((LocalPivot0 * M0)*InvM1);*/
		SnapPos(edit_pos1, edit_pos0, Pose1, Pose0);

//				mEditRot1 = Pose0.mRot;

			// jm1 * am1 = jm0 * am0
			// jm1 = (jm0 * am0)*inv(am1)

/*				Matrix3x3 am1 = Pose1.mRot;
			am1.Invert();
//				Quat am1 = Pose1.mRot;
//				am1.Invert();

			Matrix3x3 jm1 = Matrix3x3(mEditRot0) * Matrix3x3(Pose0.mRot) * am1;
//				Quat jm1 = Pose0.mRot * am1;
			mEditRot1 = jm1;*/
			SnapRot(edit_rot1, Pose1.mRot, Pose0.mRot, edit_rot0);
}


template<class T>
void ToolCreateJoint::SetupPivot(T& desc, Pint& pint, const Quat& q)
{
	if(mActor0)
	{
		desc.mLocalPivot0.mPos = mEditPos0->mEdited;
		desc.mLocalPivot0.mRot = mEditRot0 * q;
	}
	else
	{
		const PR pose1 = pint.GetWorldTransform(mActor1);
			// ###  missing mEditPos0 in here
		//desc.mLocalPivot0.mPos = pose1.mPos;
			Snap0(pint, mActor0, mActor1, mEditPos0, mEditPos1, mEditRot0, mEditRot1);
			desc.mLocalPivot0.mPos = mEditPos0->mEdited;

		desc.mLocalPivot0.mRot = pose1.mRot * q;
	}

	if(mActor1)
	{
		desc.mLocalPivot1.mPos = mEditPos1->mEdited;
		desc.mLocalPivot1.mRot = mEditRot1 * q;
	}
	else
	{
		const PR pose0 = pint.GetWorldTransform(mActor0);
			// ###  missing mEditPos1 in here
		//desc.mLocalPivot1.mPos = pose0.mPos;
			Snap1(pint, mActor0, mActor1, mEditPos0, mEditPos1, mEditRot0, mEditRot1);
			desc.mLocalPivot1.mPos = mEditPos1->mEdited;

		desc.mLocalPivot1.mRot = pose0.mRot * q;
	}
}

template<class T>
void ToolCreateJoint::SetupPivot(T& desc, Pint& pint)
{
	if(mActor0)
	{
		desc.mLocalPivot0 = mEditPos0->mEdited;
//		desc.mLocalPivot0.mRot = mEditRot0;
	}
	else
	{
		Snap0(pint, mActor0, mActor1, mEditPos0, mEditPos1, mEditRot0, mEditRot1);
		desc.mLocalPivot0 = mEditPos0->mEdited;

		if(0)
		{
			const PR pose1 = pint.GetWorldTransform(mActor1);
			desc.mLocalPivot0 = pose1.mPos;
//			desc.mLocalPivot0.mRot = pose1.mRot;
		}
	}

	if(mActor1)
	{
		desc.mLocalPivot1 = mEditPos1->mEdited;
//		desc.mLocalPivot1.mRot = mEditRot1;
	}
	else
	{
		Snap1(pint, mActor0, mActor1, mEditPos0, mEditPos1, mEditRot0, mEditRot1);
		desc.mLocalPivot1 = mEditPos1->mEdited;

		if(0)
		{
			const PR pose0 = pint.GetWorldTransform(mActor0);
			desc.mLocalPivot1 = pose0.mPos;
//			desc.mLocalPivot1.mRot = pose0.mRot;
		}
	}
}

void ToolCreateJoint::CreateJoint(Pint& pint)
{
	struct Local
	{
		static void setupDesc(PINT_JOINT_CREATE& desc, ToolCreateJoint& tool, String& jointName)
		{
			if(tool.mEditBoxName)
			{
				tool.mEditBoxName->GetTextAsString(jointName);
				desc.mName	= jointName;
			}
			desc.mObject0	= tool.mActor0;
			desc.mObject1	= tool.mActor1;
		}

/*			static PintJointHandle FindJoint(Pint_Actor* actor_api, Pint_Joint* joint_api, PintActorHandle actor, PR& pose, PintJoint type0, PintJoint type1)
		{
			const udword NbJoints = actor_api->GetNbJoints(actor);
			for(udword i=0;i<NbJoints;i++)
			{
				const PintJointHandle jh = actor_api->GetJoint(actor, NbJoints-i-1);
//					const PintJointHandle jh = actor_api->GetJoint(actor, i);
				if(jh)
				{
					const PintJoint jt = joint_api->GetType(jh);
//						if(jt==PINT_JOINT_HINGE || jt==PINT_JOINT_HINGE2)	// ### or D6 I guess?
					if(jt==type0 || jt==type1)	// ### or D6 I guess?
					{
						PintActorHandle a0, a1;
						joint_api->GetActors(jh, a0, a1);

						PR pose0, pose1;
						joint_api->GetFrames(jh, &pose0, &pose1);

						if(actor==a0)
							pose = pose0;
						else if(actor==a1)
							pose = pose1;
						return jh;
					}
				}
			}
			return null;
		}*/
	};

	FlushErrors();

	PintJointHandle j = null;

	String JointName;

	const Quat XtoY = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
	const Quat XtoZ = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

	const int JT = mJointType->GetSelectedIndex();
	switch(JT)
	{
		case IJT_SPHERICAL_X:
		case IJT_SPHERICAL_Y:
		case IJT_SPHERICAL_Z:
		{
			PINT_SPHERICAL_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);
			//
/*				if(mEditPos0)
				Desc.mLocalPivot0 = mEditPos0->mEdited;
			if(mEditPos1)
				Desc.mLocalPivot1 = mEditPos1->mEdited;*/

			//SetupPivot(Desc, pint);
				if(JT==IJT_SPHERICAL_X)
				{
					SetupPivot(Desc, pint, Quat(Idt));
				}
				else if(JT==IJT_SPHERICAL_Y)
				{
					SetupPivot(Desc, pint, XtoY);
				}
				else if(JT==IJT_SPHERICAL_Z)
				{
					SetupPivot(Desc, pint, XtoZ);
				}

			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_HINGE_X:
		case IJT_HINGE_Y:
		case IJT_HINGE_Z:
		{
			if(1)
			{
				PINT_HINGE2_JOINT_CREATE Desc;
				Local::setupDesc(Desc, *this, JointName);

/*					if(mEditPos0)
					Desc.mLocalPivot0.mPos = mEditPos0->mEdited;
				if(mEditPos1)
					Desc.mLocalPivot1.mPos = mEditPos1->mEdited;*/

				if(JT==IJT_HINGE_X)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*Quat(Idt);
//						Desc.mLocalPivot1.mRot = mEditRot1*Quat(Idt);
					SetupPivot(Desc, pint, Quat(Idt));
				}
				else if(JT==IJT_HINGE_Y)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*YtoX;
//						Desc.mLocalPivot1.mRot = mEditRot1*YtoX;
					SetupPivot(Desc, pint, XtoY);
				}
				else if(JT==IJT_HINGE_Z)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*ZtoX;
//						Desc.mLocalPivot1.mRot = mEditRot1*ZtoX;
					SetupPivot(Desc, pint, XtoZ);
				}

				j = pint.CreateJoint(Desc);
			}
			else
			{
				PINT_HINGE_JOINT_CREATE Desc;
				Local::setupDesc(Desc, *this, JointName);

				if(mEditPos0)
					Desc.mLocalPivot0 = mEditPos0->mEdited;
				if(mEditPos1)
					Desc.mLocalPivot1 = mEditPos1->mEdited;

				if(JT==IJT_HINGE_X)
					Desc.mLocalAxis0 = Desc.mLocalAxis1 = Point(1.0f, 0.0f, 0.0f);
				else if(JT==IJT_HINGE_Y)
					Desc.mLocalAxis0 = Desc.mLocalAxis1 = Point(0.0f, 1.0f, 0.0f);
				else if(JT==IJT_HINGE_Z)
					Desc.mLocalAxis0 = Desc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);

				j = pint.CreateJoint(Desc);
			}
		}
		break;

		case IJT_PRISMATIC_X:
		case IJT_PRISMATIC_Y:
		case IJT_PRISMATIC_Z:
		{
			PINT_PRISMATIC_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);

/*					if(mEditPos0)
					Desc.mLocalPivot0.mPos = mEditPos0->mEdited;
				if(mEditPos1)
					Desc.mLocalPivot1.mPos = mEditPos1->mEdited;*/

				if(JT==IJT_PRISMATIC_X)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*Quat(Idt);
//						Desc.mLocalPivot1.mRot = mEditRot1*Quat(Idt);
					SetupPivot(Desc, pint, Quat(Idt));
				}
				else if(JT==IJT_PRISMATIC_Y)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*YtoX;
//						Desc.mLocalPivot1.mRot = mEditRot1*YtoX;
					SetupPivot(Desc, pint, XtoY);
				}
				else if(JT==IJT_PRISMATIC_Z)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*ZtoX;
//						Desc.mLocalPivot1.mRot = mEditRot1*ZtoX;
					SetupPivot(Desc, pint, XtoZ);
				}

			//## hardcoded test
			Desc.mLimits.Set(0.0f, 0.01f);
			Desc.mSpring.mStiffness		= 100.0f;
			Desc.mSpring.mDamping		= 10.0f;

			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_FIXED:
		{
			PINT_FIXED_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);

			SetupPivot(Desc, pint);

			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_DISTANCE:
		{
			PINT_DISTANCE_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);

			SetupPivot(Desc, pint);

			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_D6:
		{
			PINT_D6_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);
			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_GEAR:
		{
			bool ok = true;
			if(!mActor0 || !mActor1)
			{
				IceCore::MessageBox(0, "Need two actors to create gear joint", "Cannot create gear joint", MB_OK);
				ok = false;
			}

			Pint_Actor* ActorAPI = pint.GetActorAPI();
			if(!ActorAPI)
			{
				IceCore::MessageBox(0, "Actor API not available", "Cannot create gear joint", MB_OK);
				ok  = false;
			}

			Pint_Joint* JointAPI = pint.GetJointAPI();
			if(!JointAPI)
			{
				IceCore::MessageBox(0, "Joint API not available", "Cannot create gear joint", MB_OK);
				ok  = false;
			}

			if(ok)
			{
				PINT_GEAR_JOINT_CREATE Desc;
				Local::setupDesc(Desc, *this, JointName);

				GearJointWindow* GJW = static_cast<GearJointWindow*>(mGearJointWindow);

//					Desc.mHinge0 = Local::FindJoint(ActorAPI, JointAPI, mActor0, Desc.mLocalPivot0, PINT_JOINT_HINGE, PINT_JOINT_HINGE2);
//					Desc.mHinge1 = Local::FindJoint(ActorAPI, JointAPI, mActor1, Desc.mLocalPivot1, PINT_JOINT_HINGE, PINT_JOINT_HINGE2);
				Desc.mHinge0 = GJW->mListBoxes.GetSelectedJoint(0, JointAPI, mActor0, Desc.mLocalPivot0);
				Desc.mHinge1 = GJW->mListBoxes.GetSelectedJoint(1, JointAPI, mActor1, Desc.mLocalPivot1);

				if(!Desc.mHinge0 || !Desc.mHinge1)
				{
					IceCore::MessageBox(0, "Selected actors must have a hinge joint", "Cannot create gear joint", MB_OK);
				}
				else
				{
					const udword NbTeeth0 = GetInt(0, GJW->mEditBox_NbTeeth0);
					const udword NbTeeth1 = GetInt(0, GJW->mEditBox_NbTeeth1);
					if(!NbTeeth0 || !NbTeeth1)
					{
						IceCore::MessageBox(0, "Nb teeths cannot be zero", "Cannot create gear joint", MB_OK);
					}
					else
					{
						Desc.mGearRatio = float(NbTeeth0)/float(NbTeeth1);
						//Desc.mRadius0 = 1.0f;
						//Desc.mRadius1 = 1.0f;

/*							if(mActor0)
							Desc.mLocalPivot0.mRot = Desc.mLocalPivot0.mRot.GetConjugate();
						if(mActor1)
							Desc.mLocalPivot1.mRot = Desc.mLocalPivot1.mRot.GetConjugate();*/

/*			Matrix3x3 Rot;
		Rot.FromTo(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		Desc.mLocalPivot0.mRot = Rot;
		Desc.mLocalPivot1.mRot = Rot;*/

/*					PR pr0 = pint.GetWorldTransform(Desc.mObject0);
				PR pr1 = pint.GetWorldTransform(Desc.mObject1);

				Matrix3x3 rot0 = pr0.mRot;
				Matrix3x3 rot1 = pr1.mRot;

				//cA2w = bA2w.transform(data.c2b[0]);
				//cB2w = bB2w.transform(data.c2b[1]);

				rot0 *= Matrix3x3(Desc.mLocalPivot0.mRot);
				rot1 *= Matrix3x3(Desc.mLocalPivot1.mRot);*/

						j = pint.CreateJoint(Desc);
					}
				}
			}
		}
		break;

		case IJT_RACK:
		{
			bool ok = true;
			if(!mActor0 || !mActor1)
			{
				IceCore::MessageBox(0, "Need two actors to create rack joint", "Cannot create rack joint", MB_OK);
				ok = false;
			}

			Pint_Actor* ActorAPI = pint.GetActorAPI();
			if(!ActorAPI)
			{
				IceCore::MessageBox(0, "Actor API not available", "Cannot create rack joint", MB_OK);
				ok  = false;
			}

			Pint_Joint* JointAPI = pint.GetJointAPI();
			if(!JointAPI)
			{
				IceCore::MessageBox(0, "Joint API not available", "Cannot create rack joint", MB_OK);
				ok  = false;
			}

			if(ok)
			{
				PINT_RACK_AND_PINION_JOINT_CREATE Desc;
				Local::setupDesc(Desc, *this, JointName);

				RackJointWindow* RJW = static_cast<RackJointWindow*>(mRackJointWindow);

//					Desc.mHinge = Local::FindJoint(ActorAPI, JointAPI, mActor0, Desc.mLocalPivot0, PINT_JOINT_HINGE, PINT_JOINT_HINGE2);
//					Desc.mPrismatic = Local::FindJoint(ActorAPI, JointAPI, mActor1, Desc.mLocalPivot1, PINT_JOINT_PRISMATIC, PINT_JOINT_PRISMATIC);
				Desc.mHinge = RJW->mListBoxes.GetSelectedJoint(0, JointAPI, mActor0, Desc.mLocalPivot0);
				Desc.mPrismatic = RJW->mListBoxes.GetSelectedJoint(1, JointAPI, mActor1, Desc.mLocalPivot1);

				if(!Desc.mHinge || !Desc.mPrismatic)
				{
					IceCore::MessageBox(0, "Selected actors must have a hinge & prismatic joint respectively", "Cannot create rack joint", MB_OK);
				}
				else
				{
					const udword NbTeethPinion = GetInt(0, RJW->mEditBox_NbTeethPinion);
					const udword NbTeethRack = GetInt(0, RJW->mEditBox_NbTeethRack);
					if(!NbTeethPinion || !NbTeethRack)
					{
						IceCore::MessageBox(0, "Nb teeths cannot be zero", "Cannot create gear joint", MB_OK);
					}
					else
					{
						// ### not the same API as the gear...
						Desc.mNbPinionTeeth = NbTeethPinion;
						Desc.mNbRackTeeth = NbTeethRack;
						Desc.mRackLength = GetFloat(1.0f, RJW->mEditBox_RackLength);

/*							if(mActor0)
							Desc.mLocalPivot0.mRot = Desc.mLocalPivot0.mRot.GetConjugate();
						if(mActor1)
							Desc.mLocalPivot1.mRot = Desc.mLocalPivot1.mRot.GetConjugate();*/

						j = pint.CreateJoint(Desc);
					}
				}
			}
		}
		break;
	}

	if(j)
	{
		workaround(pint, mActor0);
		workaround(pint, mActor1);
	}
	else
	{
		const IceError* LastError = GetLastIceError();
		if(LastError)
			IceCore::MessageBox(0, LastError->mErrorText, "Error", MB_OK);
	}
}

void ToolCreateJoint::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	RenderActorData(pint, mActor0, mEditPos0, mEditRot0);
	RenderActorData(pint, mActor1, mEditPos1, mEditRot1);

	if(mJointType)
	{
		const udword SelectedIndex = mJointType->GetSelectedIndex();
		if(SelectedIndex==IJT_RACK)
		{
			const RackJointWindow* RJW = static_cast<const RackJointWindow*>(mRackJointWindow);
			GLRenderHelpers::DrawBoxCorners(RJW->mRackBounds, Point(1.0f, 0.0f, 1.0f), 0.4f);
		}
	}

	if(mPendingAction==SNAP_POS_0)
	{
		Snap0(pint, mActor0, mActor1, mEditPos0, mEditPos1, mEditRot0, mEditRot1);
	}
	else if(mPendingAction==SNAP_POS_1)
	{
		Snap1(pint, mActor0, mActor1, mEditPos0, mEditPos1, mEditRot0, mEditRot1);
	}
	else if(mPendingAction==CREATE_JOINT && mJointType)
	{
		CreateJoint(pint);
	}
}

bool ToolCreateJoint::ValidateTouchedActor(Pint& pint, IceButton* button, PintActorHandle& dstActor, PintShapeHandle& dstShape, const PintRaycastHit& hit, PintActorHandle other_actor, bool& validate)
{
	if(mActiveButton==button)
	{
		if(hit.mTouchedActor==other_actor)
		{
			IceCore::MessageBox(0, "You cannot join an actor to itself", "Error", MB_OK);
			validate = false;
		}
		else
		{
			dstActor = hit.mTouchedActor;
			dstShape = hit.mTouchedShape;
			mPint = &pint;
			OnActorSelected();
		}
		return true;
	}
	return false;
}

void ToolCreateJoint::SelectActor(Pint& pint, const PintRaycastHit& hit)
{
	bool ValidateHit = true;
	if(mPint && mPint!=&pint)
	{
		IceCore::MessageBox(0, "You cannot join actors from different engines!", "Nice try", MB_OK);
		ValidateHit = false;
	}
	else
	{
		ValidateTouchedActor(pint, mButton0, mActor0, mShape0, hit, mActor1, ValidateHit);
		ValidateTouchedActor(pint, mButton1, mActor1, mShape1, hit, mActor0, ValidateHit);
	}

	if(ValidateHit)
	{
		//printf("Picked object: 0x%x (%s)\n", size_t(Hit.mObject), pint.GetName());

		Pint_Actor* API = pint.GetActorAPI();
		if(API)
		{
			const char* Name = API->GetName(hit.mTouchedActor);
			if(Name)
				mActiveButton->SetLabel(_F("%s - 0x%x", Name, size_t(hit.mTouchedActor)));
			else
				mActiveButton->SetLabel(_F("0x%x", size_t(hit.mTouchedActor)));
		}
		else
			mActiveButton->SetLabel(_F("0x%x", size_t(hit.mTouchedActor)));

		mActiveButton->SetButtonDown(false);
		mActiveButton = null;
	}
}

void ToolCreateJoint::RightDownCallback(Pint& pint, udword pint_index)
{
	// This callback is only used to select actors. It only happens if we have an active (pushed) button.
	if(!mActiveButton || !mActiveButton->GetButtonDown())
		return;

	PintRaycastHit Hit;
	if(Raycast(pint, Hit, mOrigin, mDir) && !IsDefaultEnv(pint, Hit.mTouchedActor))
	{
		SelectActor(pint, Hit);

/*		bool ValidateHit = true;
		if(mPint && mPint!=&pint)
		{
			IceCore::MessageBox(0, "You cannot join actors from different engines!", "Nice try", MB_OK);
			ValidateHit = false;
		}
		else
		{
			ValidateTouchedActor(pint, mButton0, mActor0, mShape0, Hit, mActor1, ValidateHit);
			ValidateTouchedActor(pint, mButton1, mActor1, mShape1, Hit, mActor0, ValidateHit);
		}

		if(ValidateHit)
		{
			//printf("Picked object: 0x%x (%s)\n", size_t(Hit.mObject), pint.GetName());

			Pint_Actor* API = pint.GetActorAPI();
			if(API)
			{
				const char* Name = API->GetName(Hit.mTouchedActor);
				if(Name)
					mActiveButton->SetLabel(_F("%s - 0x%x", Name, size_t(Hit.mTouchedActor)));
				else
					mActiveButton->SetLabel(_F("0x%x", size_t(Hit.mTouchedActor)));
			}
			else
				mActiveButton->SetLabel(_F("0x%x", size_t(Hit.mTouchedActor)));

			mActiveButton->SetButtonDown(false);
			mActiveButton = null;
		}*/
	}
	else
	{
		if(mActiveButton==mButton0)
		{
			ResetButtonNames(true, false);
			mActor0 = null;
			mShape0 = null;
		}
		else if(mActiveButton==mButton1)
		{
			ResetButtonNames(false, true);
			mActor1 = null;
			mShape1 = null;
		}

		mActiveButton->SetButtonDown(false);
		mActiveButton = null;
	}
}

void ToolCreateJoint::OnSelectedActor(Pint* pint, PintActorHandle h, void* user_data)
{
	const IceButton* B = reinterpret_cast<const IceButton*>(user_data);
	const udword ID = B->GetID();
	if(ID==0)
	{
		mActiveButton = mButton0;
		mButton1->SetButtonDown(false);
	}
	else if(ID==1)
	{
		mActiveButton = mButton1;
		mButton0->SetButtonDown(false);
	}
	else ASSERT(0);

	PintRaycastHit Hit;
	Hit.mTouchedActor	= h;
	Hit.mTouchedShape	= null;

	SelectActor(*pint, Hit);

/*	// TODO: refactor this
	{
		bool ValidateHit = true;
		if(mPint && mPint!=pint)
		{
			IceCore::MessageBox(0, "You cannot join actors from different engines!", "Nice try", MB_OK);
			ValidateHit = false;
		}
		else
		{
			ValidateTouchedActor(*pint, mButton0, mActor0, mShape0, Hit, mActor1, ValidateHit);
			ValidateTouchedActor(*pint, mButton1, mActor1, mShape1, Hit, mActor0, ValidateHit);
		}

		if(ValidateHit)
		{
			//printf("Picked object: 0x%x (%s)\n", size_t(Hit.mObject), pint.GetName());

			Pint_Actor* API = pint->GetActorAPI();
			if(API)
			{
				const char* Name = API->GetName(Hit.mTouchedActor);
				if(Name)
					mActiveButton->SetLabel(_F("%s - 0x%x", Name, size_t(Hit.mTouchedActor)));
				else
					mActiveButton->SetLabel(_F("0x%x", size_t(Hit.mTouchedActor)));
			}
			else
				mActiveButton->SetLabel(_F("0x%x", size_t(Hit.mTouchedActor)));

			mActiveButton->SetButtonDown(false);
			mActiveButton = null;
		}
	}*/


	SetFocusActor();
}

///////////////////////////////////////////////////////////////////////////////

static const char* gButtonLabel0 = "Select 1st actor";
static const char* gButtonLabel1 = "Select 2nd actor";

void ToolCreateJoint::OnSnapButton(IceButton& button)
{
	const udword ID = button.GetID();
	if(ID>=2)
	{
		IceCore::MessageBox(null, "Not implemented yet :)", "Nope", MB_OK);
	}
	else if(ID==0)
	{
		if(!mActor1)
			IceCore::MessageBox(null, "Actor1 not defined yet", "Error", MB_OK);
		else
			mPendingAction=SNAP_POS_0;
	}
	else if(ID==1)
	{
		if(!mActor0)
			IceCore::MessageBox(null, "Actor0 not defined yet", "Error", MB_OK);
		else
			mPendingAction=SNAP_POS_1;
	}
}

void ToolCreateJoint::OnSelectedShape(PintShapeHandle sh, void* user_data)
{
	ASSERT(mPint);
//	Pint_Actor* Actor_API = mPint->GetActorAPI();
//	ASSERT(Actor_API);
//	if(!Actor_API)
//		return;
	Pint_Shape* Shape_API = mPint->GetShapeAPI();
	ASSERT(Shape_API);
	if(!Shape_API)
		return;

	const PintActorHandle ah = PintActorHandle(user_data);
	ASSERT(ah==mActor0 || ah==mActor1);

	const PR ActorPose = mPint->GetWorldTransform(ah);
	const PR ShapePose = Shape_API->GetWorldTransform(ah, sh);

	// EditPos * ActorPose = ShapePose.mPos
	// EditPos = ShapePose.mPos * Inverse(ActorPose)

	// TODO: don't go to matrices?

	Matrix4x4 InvActorPose;
	InvertPRMatrix(InvActorPose, ActorPose);

	const Point LocalPivot = ShapePose.mPos * InvActorPose;

	if(user_data==mActor0)
		mEditPos0->SetPos(LocalPivot);
	else if(user_data==mActor1)
		mEditPos1->SetPos(LocalPivot);
	else
		ASSERT(0);

	SetFocusShape();
}

void ToolCreateJoint::OnFromShape(IceButton& button)
{
	const udword ID = button.GetID();
	if(ID==0 && mActor0)
	{
		ASSERT(mPint);
		ShowShapeSelectionWindow(*mPint, mActor0, this, mActor0);
	}
	else if(ID==1 && mActor1)
	{
		ASSERT(mPint);
		ShowShapeSelectionWindow(*mPint, mActor1, this, mActor1);
	}
	else ASSERT(0);
}


static const char* gTooltip_SelectActor_Picking = "Select actor by picking it with right mouse button in render window";
static const char* gTooltip_SelectActor_List = "Select actor from a list";

void ToolCreateJoint::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword x = 4;
	sdword y = 4;
	const sdword YStep = 20;

	struct Callbacks
	{
		// This is called when we press one of the two "select actor" buttons
		static void SelectActor(IceButton& button, void* user_data)
		{
			ToolCreateJoint* tcj = reinterpret_cast<ToolCreateJoint*>(user_data);
			// If the other button is already active/pushed, make it inactive
			if(tcj->mActiveButton && tcj->mActiveButton!=&button)
			{
				tcj->mActiveButton->SetButtonDown(false);
				tcj->mActiveButton = null;
			}

			if(!button.GetButtonDown())
			{
				// We pushed an inactive button => mark it as pushed/active.
				button.SetButtonDown(true);
				tcj->mActiveButton = &button;
				//button.SetBitmapColor(0,0,0);
				//button.EnableBitmap();
			}
			else
			{
				// We pushed an active button => mark it as inactive.
				button.SetButtonDown(false);
				tcj->mActiveButton = null;
			}
		}

		// This is called when we press one of the two "select actor from list" buttons
		static void SelectActorFromList(IceButton& button, void* user_data)
		{
			ToolCreateJoint* tcj = reinterpret_cast<ToolCreateJoint*>(user_data);
			ShowActorSelectionWindow(tcj, &button);
		}

		// This is called when we press the "create joint" button
		static void CreateJoint(IceButton& button, void* user_data)
		{
			ToolCreateJoint* tcj = reinterpret_cast<ToolCreateJoint*>(user_data);
			if(!tcj->mActor0 && !tcj->mActor1)
			{
				IceCore::MessageBox(0, "No actors defined", "Error", MB_OK);
				return;
			}

			ASSERT(tcj->mPint);
			Pint_Actor* API = tcj->mPint->GetActorAPI();
			if(API)
			{
				if(tcj->mActor0 && API->GetMass(tcj->mActor0)==0.0f)
				{
					if(tcj->mActor1 && API->GetMass(tcj->mActor1)==0.0f)
					{
						IceCore::MessageBox(0, "Cannot create joint between two static actors", "Error", MB_OK);
						return;
					}
				}
			}

			tcj->mPendingAction=CREATE_JOINT;
		}
	};

	{
		mButton0 = GUI_CreatePushButton(parent, 0, x, y, 200, 20, gButtonLabel0, &owner, Callbacks::SelectActor, this, gTooltip_SelectActor_Picking);
		helper.CreateButton(parent, 0, x+204, y, 16, 20, "--", &owner, Callbacks::SelectActorFromList, this, gTooltip_SelectActor_List);
		y += YStep;

		mButton1 = GUI_CreatePushButton(parent, 0, x, y, 200, 20, gButtonLabel1, &owner, Callbacks::SelectActor, this, gTooltip_SelectActor_Picking);
		helper.CreateButton(parent, 1, x+204, y, 16, 20, "--", &owner, Callbacks::SelectActorFromList, this, gTooltip_SelectActor_List);
		y += YStep;

		IceButton* B = helper.CreateButton(parent, 0, 260, 4, 200, 20, "Create joint", &owner, Callbacks::CreateJoint, this, null);
	}

	{
		const sdword NameX = 220;
		const sdword NameY = 4+YStep+2+8+30;
		helper.CreateLabel(parent, NameX, NameY+2, 40, 20, "Name:", &owner);
		mEditBoxName = helper.CreateEditBox(parent, 0, NameX+40, NameY, 200, 20, "JointName", &owner, EDITBOX_TEXT, null);
	}

	y += YStep;
	CreateJointComboBox(helper, parent, owner, x, y);
//	y += YStep;

	CreateJointWindows(helper, parent, owner, x, y);

	OnComboBoxChange();
}

void ToolCreateJoint::ResetButtonNames(bool reset0, bool reset1)
{
	if(!mButton0)
		reset0 = false;
	if(!mButton1)
		reset1 = false;

	ASSERT(mJointType.mWidget);
	const udword SelectedIndex = mJointType->GetSelectedIndex();
	if(SelectedIndex==IJT_GEAR)
	{
		if(reset0)
			mButton0->SetLabel("Select 1st gear");
		if(reset1)
			mButton1->SetLabel("Select 2nd gear");
	}
	else if(SelectedIndex==IJT_RACK)
	{
		if(reset0)
			mButton0->SetLabel("Select pinion");
		if(reset1)
			mButton1->SetLabel("Select rack");
	}
	else
	{
		if(reset0)
			mButton0->SetLabel(gButtonLabel0);
		if(reset1)
			mButton1->SetLabel(gButtonLabel1);
	}
}

void ToolCreateJoint::OnComboBoxChange()
{
	ToolCreateJointBase::OnComboBoxChange();

	ResetButtonNames(mActor0==null, mActor1==null);
}

void ToolCreateJoint::OnActorSelected()
{
	const udword SelectedIndex = mJointType->GetSelectedIndex();
	if(SelectedIndex==IJT_GEAR)
	{
		GearJointWindow* GJW = static_cast<GearJointWindow*>(mGearJointWindow);
		if(mActiveButton==mButton0)
		{
			GJW->ListHinges(*mPint, mActor0, 0);
		}
		else if(mActiveButton==mButton1)
		{
			GJW->ListHinges(*mPint, mActor1, 1);
		}
	}
	else if(SelectedIndex==IJT_RACK)
	{
		RackJointWindow* RJW = static_cast<RackJointWindow*>(mRackJointWindow);
		if(mActiveButton==mButton0)
		{
			// Pinion
			RJW->ListHinges(*mPint, mActor0);
		}
		else if(mActiveButton==mButton1)
		{
			// Rack
			RJW->ListPrismatic(*mPint, mActor1);

			AABB Bounds;
			bool ValidBounds = false;

			if(RJW->mCheckBox_SelectShape && RJW->mCheckBox_SelectShape->IsChecked())
			{
				Pint_Shape* API = mPint->GetShapeAPI();
				if(API)
					ValidBounds = API->GetWorldBounds(mActor1, mShape1, Bounds);
			}
			else
			{
				Pint_Actor* API = mPint->GetActorAPI();
				if(API)
					ValidBounds = API->GetWorldBounds(mActor1, Bounds);
			}

			if(ValidBounds)
			{
				RJW->mEditBox_RackLength->SetText(_F("%f", Bounds.GetSize()*2.0f));
				RJW->mRackBounds = Bounds;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

LinkActors::LinkActors() : mDirty(false)
{
}

LinkActors::~LinkActors()
{
}

void LinkActors::Sort(Pint& pint)
{
	if(!mDirty)
		return;

	const udword NbLinkActors = GetNbLinkActors();
	if(!NbLinkActors)
	{
		mDirty = false;
		return;
	}

	const LinkActor* Actors = GetLinkActors();

	// ### to refactor / optimize
	Point Center(0.0f, 0.0f, 0.0f);
	const float Coeff = 1.0f/float(NbLinkActors);
	Point* Positions = reinterpret_cast<Point*>(StackAlloc(sizeof(Point)*NbLinkActors));
	for(udword i=0;i<NbLinkActors;i++)
	{
		const PR ActorPose = pint.GetWorldTransform(Actors[i].mActor);
		Center += ActorPose.mPos * Coeff;
		Positions[i] = ActorPose.mPos;
	}

	Plane ChainPlane;
	ComputeNewellPlane(ChainPlane, NbLinkActors, null, Positions);

	float* Keys = (float*)StackAlloc(NbLinkActors*sizeof(float));
	Point Right, Up;
	ComputeBasis2(ChainPlane.n, Right, Up);

	const Plane SinPlane(Center, Up);

	for(udword i=0;i<NbLinkActors;i++)
	{
		const Point Dir = Positions[i] - Center;

		Keys[i] = Angle(Right, Dir);

		if(SinPlane.Distance(Positions[i])<0.0f)
			Keys[i] = -Keys[i];
	}
	RadixSort RS;
	const udword* Sorted = RS.Sort(Keys, NbLinkActors).GetRanks();

	LinkActor* Tmp = (LinkActor*)StackAlloc(NbLinkActors*sizeof(LinkActor));
	for(udword i=0;i<NbLinkActors;i++)
		Tmp[i] = Actors[Sorted[i]];

	CopyMemory(const_cast<LinkActor*>(Actors), Tmp, NbLinkActors*sizeof(LinkActor));

	mDirty = false;
}

///////////////////////////////////////////////////////////////////////////////

#include "DefaultControlInterface.h"

static const char* gButtonLabel2 = "Select link actors";

ToolCreateChain::ToolCreateChain()
{
}

ToolCreateChain::~ToolCreateChain()
{
}

void ToolCreateChain::Reset(udword pint_index)
{
	ToolCreateJointBase::Reset(pint_index);

/*
	ResetButtonNames(true, true);
*/
	if(mButton0)
		mButton0->SetButtonDown(false);
/*	if(mButton1)
		mButton1->SetButtonDown(false);*/
	mActiveButton = null;
/*
	mActor0 = null;
	mShape0 = null;
	mActor1 = null;
	mShape1 = null;
	if(mEditBoxName)
		mEditBoxName->SetText("JointName");
*/
//	if(mEditBox_NbTeeth0)
//		mEditBox_NbTeeth0->Set
//	mEditBox_NbTeeth1	(null),

	mLinkActors.Empty();
}

bool ToolCreateChain::ValidateActor(PintActorHandle actor) const
{
	const udword NbLinkActors = mLinkActors.GetNbLinkActors();
	const LinkActor* Actors = mLinkActors.GetLinkActors();
	for(udword i=0;i<NbLinkActors;i++)
	{
		if(Actors[i].mActor==actor)
			return false;
	}
	return true;
}

void ToolCreateChain::RightDownCallback(Pint& pint, udword pint_index)
{
	// This callback is only used to select actors. It only happens if we have an active (pushed) button.
	if(!mActiveButton || !mActiveButton->GetButtonDown())
		return;

	PintRaycastHit Hit;
	if(Raycast(pint, Hit, mOrigin, mDir) && !IsDefaultEnv(pint, Hit.mTouchedActor))
	{
		bool ValidateHit = true;
		if(mPint && mPint!=&pint)
		{
			IceCore::MessageBox(0, "You cannot join actors from different engines!", "Nice try", MB_OK);
			ValidateHit = false;
		}
		else
		{
			ValidateHit = ValidateActor(Hit.mTouchedActor);

			if(ValidateHit)
			{
				mPint = &pint;

				LinkActor* Data = mLinkActors.ReserveSlot();
				Data->mActor = Hit.mTouchedActor;
				//Data->mShape = Hit.mTouchedShape;

				//OnActorSelected();
				// ### hack to make selected links more visible
				DefaultControlInterface& DCI = GetDefaultControlInterface();
				SelectedObject* Selected = DCI.mSelMan.AddToSelection(mPint, Hit.mTouchedActor, pint_index);
			}
			else
				IceCore::MessageBox(0, "You cannot select the same actor multiple times", "Error", MB_OK);
		}

/*		if(ValidateHit)
		{
			//printf("Picked object: 0x%x (%s)\n", size_t(Hit.mObject), pint.GetName());

			Pint_Actor* API = pint.GetActorAPI();
			if(API)
			{
				const char* Name = API->GetName(Hit.mTouchedActor);
				if(Name)
					mActiveButton->SetLabel(_F("%s - 0x%x", Name, size_t(Hit.mTouchedActor)));
				else
					mActiveButton->SetLabel(_F("0x%x", size_t(Hit.mTouchedActor)));
			}
			else
				mActiveButton->SetLabel(_F("0x%x", size_t(Hit.mTouchedActor)));

			mActiveButton->SetButtonDown(false);
			mActiveButton = null;
		}*/
	}
	else
	{
/*		if(mActiveButton==mButton0)
		{
			ResetButtonNames(true, false);
			mActor0 = null;
			mShape0 = null;
		}
		else if(mActiveButton==mButton1)
		{
			ResetButtonNames(false, true);
			mActor1 = null;
			mShape1 = null;
		}

		mActiveButton->SetButtonDown(false);
		mActiveButton = null;*/
	}
}

void ToolCreateChain::CreateJoints(Pint& pint)
{
#ifdef REMOVED
	struct Local
	{
		static void setupDesc(PINT_JOINT_CREATE& desc, ToolCreateJoint& tool, String& jointName)
		{
			if(tool.mEditBoxName)
			{
				tool.mEditBoxName->GetTextAsString(jointName);
				desc.mName	= jointName;
			}
			desc.mObject0	= tool.mActor0;
			desc.mObject1	= tool.mActor1;
		}

/*			static PintJointHandle FindJoint(Pint_Actor* actor_api, Pint_Joint* joint_api, PintActorHandle actor, PR& pose, PintJoint type0, PintJoint type1)
		{
			const udword NbJoints = actor_api->GetNbJoints(actor);
			for(udword i=0;i<NbJoints;i++)
			{
				const PintJointHandle jh = actor_api->GetJoint(actor, NbJoints-i-1);
//					const PintJointHandle jh = actor_api->GetJoint(actor, i);
				if(jh)
				{
					const PintJoint jt = joint_api->GetType(jh);
//						if(jt==PINT_JOINT_HINGE || jt==PINT_JOINT_HINGE2)	// ### or D6 I guess?
					if(jt==type0 || jt==type1)	// ### or D6 I guess?
					{
						PintActorHandle a0, a1;
						joint_api->GetActors(jh, a0, a1);

						PR pose0, pose1;
						joint_api->GetFrames(jh, &pose0, &pose1);

						if(actor==a0)
							pose = pose0;
						else if(actor==a1)
							pose = pose1;
						return jh;
					}
				}
			}
			return null;
		}*/
	};
#endif

	FlushErrors();

	PintJointHandle j = null;

	String JointName;

	const Quat XtoY = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
	const Quat XtoZ = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

#ifdef REMOVED
	const int JT = mJointType->GetSelectedIndex();
	switch(JT)
	{
		case IJT_SPHERICAL:
		{
			PINT_SPHERICAL_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);
			//
/*				if(mEditPos0)
				Desc.mLocalPivot0 = mEditPos0->mEdited;
			if(mEditPos1)
				Desc.mLocalPivot1 = mEditPos1->mEdited;*/

			SetupPivot(Desc, pint);

			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_HINGE_X:
		case IJT_HINGE_Y:
		case IJT_HINGE_Z:
		{
			if(1)
			{
				PINT_HINGE2_JOINT_CREATE Desc;
				Local::setupDesc(Desc, *this, JointName);

/*					if(mEditPos0)
					Desc.mLocalPivot0.mPos = mEditPos0->mEdited;
				if(mEditPos1)
					Desc.mLocalPivot1.mPos = mEditPos1->mEdited;*/

				if(JT==IJT_HINGE_X)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*Quat(Idt);
//						Desc.mLocalPivot1.mRot = mEditRot1*Quat(Idt);
					SetupPivot(Desc, pint, Quat(Idt));
				}
				else if(JT==IJT_HINGE_Y)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*YtoX;
//						Desc.mLocalPivot1.mRot = mEditRot1*YtoX;
					SetupPivot(Desc, pint, XtoY);
				}
				else if(JT==IJT_HINGE_Z)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*ZtoX;
//						Desc.mLocalPivot1.mRot = mEditRot1*ZtoX;
					SetupPivot(Desc, pint, XtoZ);
				}

				j = pint.CreateJoint(Desc);
			}
			else
			{
				PINT_HINGE_JOINT_CREATE Desc;
				Local::setupDesc(Desc, *this, JointName);

				if(mEditPos0)
					Desc.mLocalPivot0 = mEditPos0->mEdited;
				if(mEditPos1)
					Desc.mLocalPivot1 = mEditPos1->mEdited;

				if(JT==IJT_HINGE_X)
					Desc.mLocalAxis0 = Desc.mLocalAxis1 = Point(1.0f, 0.0f, 0.0f);
				else if(JT==IJT_HINGE_Y)
					Desc.mLocalAxis0 = Desc.mLocalAxis1 = Point(0.0f, 1.0f, 0.0f);
				else if(JT==IJT_HINGE_Z)
					Desc.mLocalAxis0 = Desc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);

				j = pint.CreateJoint(Desc);
			}
		}
		break;

		case IJT_PRISMATIC_X:
		case IJT_PRISMATIC_Y:
		case IJT_PRISMATIC_Z:
		{
			PINT_PRISMATIC_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);

/*					if(mEditPos0)
					Desc.mLocalPivot0.mPos = mEditPos0->mEdited;
				if(mEditPos1)
					Desc.mLocalPivot1.mPos = mEditPos1->mEdited;*/

				if(JT==IJT_PRISMATIC_X)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*Quat(Idt);
//						Desc.mLocalPivot1.mRot = mEditRot1*Quat(Idt);
					SetupPivot(Desc, pint, Quat(Idt));
				}
				else if(JT==IJT_PRISMATIC_Y)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*YtoX;
//						Desc.mLocalPivot1.mRot = mEditRot1*YtoX;
					SetupPivot(Desc, pint, XtoY);
				}
				else if(JT==IJT_PRISMATIC_Z)
				{
//						Desc.mLocalPivot0.mRot = mEditRot0*ZtoX;
//						Desc.mLocalPivot1.mRot = mEditRot1*ZtoX;
					SetupPivot(Desc, pint, XtoZ);
				}

			//## hardcoded test
			Desc.mMaxLimit				= 0.01f;
			Desc.mMinLimit				= 0.0f;
			Desc.mSpringStiffness		= 100.0f;
			Desc.mSpringDamping			= 10.0f;

			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_FIXED:
		{
			PINT_FIXED_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);

			SetupPivot(Desc, pint);

			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_DISTANCE:
		{
			PINT_DISTANCE_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);

			SetupPivot(Desc, pint);

			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_D6:
		{
			PINT_D6_JOINT_CREATE Desc;
			Local::setupDesc(Desc, *this, JointName);
			j = pint.CreateJoint(Desc);
		}
		break;

		case IJT_GEAR:
		{
			bool ok = true;
			if(!mActor0 || !mActor1)
			{
				IceCore::MessageBox(0, "Need two actors to create gear joint", "Cannot create gear joint", MB_OK);
				ok = false;
			}

			Pint_Actor* ActorAPI = pint.GetActorAPI();
			if(!ActorAPI)
			{
				IceCore::MessageBox(0, "Actor API not available", "Cannot create gear joint", MB_OK);
				ok  = false;
			}

			Pint_Joint* JointAPI = pint.GetJointAPI();
			if(!JointAPI)
			{
				IceCore::MessageBox(0, "Joint API not available", "Cannot create gear joint", MB_OK);
				ok  = false;
			}

			if(ok)
			{
				PINT_GEAR_JOINT_CREATE Desc;
				Local::setupDesc(Desc, *this, JointName);

				GearJointWindow* GJW = static_cast<GearJointWindow*>(mGearJointWindow);

//					Desc.mHinge0 = Local::FindJoint(ActorAPI, JointAPI, mActor0, Desc.mLocalPivot0, PINT_JOINT_HINGE, PINT_JOINT_HINGE2);
//					Desc.mHinge1 = Local::FindJoint(ActorAPI, JointAPI, mActor1, Desc.mLocalPivot1, PINT_JOINT_HINGE, PINT_JOINT_HINGE2);
				Desc.mHinge0 = GJW->mListBoxes.GetSelectedJoint(0, JointAPI, mActor0, Desc.mLocalPivot0);
				Desc.mHinge1 = GJW->mListBoxes.GetSelectedJoint(1, JointAPI, mActor1, Desc.mLocalPivot1);

				if(!Desc.mHinge0 || !Desc.mHinge1)
				{
					IceCore::MessageBox(0, "Selected actors must have a hinge joint", "Cannot create gear joint", MB_OK);
				}
				else
				{
					const udword NbTeeth0 = GetInt(0, GJW->mEditBox_NbTeeth0);
					const udword NbTeeth1 = GetInt(0, GJW->mEditBox_NbTeeth1);
					if(!NbTeeth0 || !NbTeeth1)
					{
						IceCore::MessageBox(0, "Nb teeths cannot be zero", "Cannot create gear joint", MB_OK);
					}
					else
					{
						Desc.mGearRatio = float(NbTeeth0)/float(NbTeeth1);
						//Desc.mRadius0 = 1.0f;
						//Desc.mRadius1 = 1.0f;

/*							if(mActor0)
							Desc.mLocalPivot0.mRot = Desc.mLocalPivot0.mRot.GetConjugate();
						if(mActor1)
							Desc.mLocalPivot1.mRot = Desc.mLocalPivot1.mRot.GetConjugate();*/

/*			Matrix3x3 Rot;
		Rot.FromTo(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		Desc.mLocalPivot0.mRot = Rot;
		Desc.mLocalPivot1.mRot = Rot;*/

/*					PR pr0 = pint.GetWorldTransform(Desc.mObject0);
				PR pr1 = pint.GetWorldTransform(Desc.mObject1);

				Matrix3x3 rot0 = pr0.mRot;
				Matrix3x3 rot1 = pr1.mRot;

				//cA2w = bA2w.transform(data.c2b[0]);
				//cB2w = bB2w.transform(data.c2b[1]);

				rot0 *= Matrix3x3(Desc.mLocalPivot0.mRot);
				rot1 *= Matrix3x3(Desc.mLocalPivot1.mRot);*/

						j = pint.CreateJoint(Desc);
					}
				}
			}
		}
		break;

		case IJT_RACK:
		{
			bool ok = true;
			if(!mActor0 || !mActor1)
			{
				IceCore::MessageBox(0, "Need two actors to create rack joint", "Cannot create rack joint", MB_OK);
				ok = false;
			}

			Pint_Actor* ActorAPI = pint.GetActorAPI();
			if(!ActorAPI)
			{
				IceCore::MessageBox(0, "Actor API not available", "Cannot create rack joint", MB_OK);
				ok  = false;
			}

			Pint_Joint* JointAPI = pint.GetJointAPI();
			if(!JointAPI)
			{
				IceCore::MessageBox(0, "Joint API not available", "Cannot create rack joint", MB_OK);
				ok  = false;
			}

			if(ok)
			{
				PINT_RACK_AND_PINION_JOINT_CREATE Desc;
				Local::setupDesc(Desc, *this, JointName);

				RackJointWindow* RJW = static_cast<RackJointWindow*>(mRackJointWindow);

//					Desc.mHinge = Local::FindJoint(ActorAPI, JointAPI, mActor0, Desc.mLocalPivot0, PINT_JOINT_HINGE, PINT_JOINT_HINGE2);
//					Desc.mPrismatic = Local::FindJoint(ActorAPI, JointAPI, mActor1, Desc.mLocalPivot1, PINT_JOINT_PRISMATIC, PINT_JOINT_PRISMATIC);
				Desc.mHinge = RJW->mListBoxes.GetSelectedJoint(0, JointAPI, mActor0, Desc.mLocalPivot0);
				Desc.mPrismatic = RJW->mListBoxes.GetSelectedJoint(1, JointAPI, mActor1, Desc.mLocalPivot1);

				if(!Desc.mHinge || !Desc.mPrismatic)
				{
					IceCore::MessageBox(0, "Selected actors must have a hinge & prismatic joint respectively", "Cannot create rack joint", MB_OK);
				}
				else
				{
					const udword NbTeethPinion = GetInt(0, RJW->mEditBox_NbTeethPinion);
					const udword NbTeethRack = GetInt(0, RJW->mEditBox_NbTeethRack);
					if(!NbTeethPinion || !NbTeethRack)
					{
						IceCore::MessageBox(0, "Nb teeths cannot be zero", "Cannot create gear joint", MB_OK);
					}
					else
					{
						// ### not the same API as the gear...
						Desc.mNbPinionTeeth = NbTeethPinion;
						Desc.mNbRackTeeth = NbTeethRack;
						Desc.mRackLength = GetFloat(1.0f, RJW->mEditBox_RackLength);

/*							if(mActor0)
							Desc.mLocalPivot0.mRot = Desc.mLocalPivot0.mRot.GetConjugate();
						if(mActor1)
							Desc.mLocalPivot1.mRot = Desc.mLocalPivot1.mRot.GetConjugate();*/

						j = pint.CreateJoint(Desc);
					}
				}
			}
		}
		break;
	}
#endif

	if(j)
	{
#ifdef REMOVED
		workaround(pint, mActor0);
		workaround(pint, mActor1);
#endif
	}
	else
	{
		//### this is wrong anyway
#ifdef REMOVED
		const IceError* LastError = GetLastIceError();
		if(LastError)
			IceCore::MessageBox(0, LastError->mErrorText, "Error", MB_OK);
#endif
	}
}

void ToolCreateChain::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	if(mPint==&pint)
	{
		const udword NbLinkActors = mLinkActors.GetNbLinkActors();
		if(NbLinkActors)
		{
			const Quat Idt(Idt);
			const LinkActor* Actors = mLinkActors.GetLinkActors();
			for(udword i=0;i<NbLinkActors;i++)
			{
				RenderActorData(*mPint, Actors[i].mActor, null, Idt);
			}

			if(!mActiveButton)
			{
/*				// ### to refactor / optimize
				Point Center(0.0f, 0.0f, 0.0f);
				const float Coeff = 1.0f/float(NbLinkActors);
				Point* Positions = reinterpret_cast<Point*>(StackAlloc(sizeof(Point)*NbLinkActors));
				for(udword i=0;i<NbLinkActors;i++)
				{
					const PR ActorPose = pint.GetWorldTransform(Actors[i].mActor);
					Center += ActorPose.mPos * Coeff;
					Positions[i] = ActorPose.mPos;
				}

				Plane ChainPlane;
				ComputeNewellPlane(ChainPlane, NbLinkActors, null, Positions);

				float* Keys = (float*)StackAlloc(NbLinkActors*sizeof(float));
				Point Right, Up;
				ComputeBasis2(ChainPlane.n, Right, Up);

				const Plane SinPlane(Center, Up);

				for(udword i=0;i<NbLinkActors;i++)
				{
					const Point Dir = Positions[i] - Center;

					Keys[i] = Angle(Right, Dir);

					if(SinPlane.Distance(Positions[i])<0.0f)
						Keys[i] = -Keys[i];
				}
				RadixSort RS;
				const udword* Sorted = RS.Sort(Keys, NbLinkActors).GetRanks();

				for(udword i=0;i<NbLinkActors;i++)
				{
					const float ColorCoeff = float(i)/float(NbLinkActors-1);

					const Point& CurrentPos = Positions[Sorted[i]];
					GLRenderHelpers::DrawLine(CurrentPos, Center, Point(ColorCoeff, ColorCoeff, ColorCoeff));

					const udword j = (i+1)%NbLinkActors;
					const Point& NextPos = Positions[Sorted[j]];
					GLRenderHelpers::DrawLine(CurrentPos, NextPos, Point(0.0f, 0.0f, 1.0f));


					PintActorHandle CurrentActor = Actors[Sorted[i]].mActor;
					PintActorHandle NextActor = Actors[Sorted[j]].mActor;

					RenderJointFrames(pint, CurrentActor, mEditPos0->mEdited, mEditRot0);
					RenderJointFrames(pint, NextActor, mEditPos1->mEdited, mEditRot1);
				}*/

				// ### this won't work for multiple pints
				mLinkActors.SortActors(pint);

				Point Center(0.0f, 0.0f, 0.0f);
				const float Coeff = 1.0f/float(NbLinkActors);
				Point* Positions = reinterpret_cast<Point*>(StackAlloc(sizeof(Point)*NbLinkActors));
				for(udword i=0;i<NbLinkActors;i++)
				{
					const PR ActorPose = pint.GetWorldTransform(Actors[i].mActor);
					Center += ActorPose.mPos * Coeff;
					Positions[i] = ActorPose.mPos;
				}

				//for(udword i=0;i<NbLinkActors;i++)
				udword i=0;
				{
					const float ColorCoeff = float(i)/float(NbLinkActors-1);

					const Point& CurrentPos = Positions[i];
					GLRenderHelpers::DrawLine(CurrentPos, Center, Point(ColorCoeff, ColorCoeff, ColorCoeff));

					const udword j = (i+1)%NbLinkActors;
					const Point& NextPos = Positions[j];
					GLRenderHelpers::DrawLine(CurrentPos, NextPos, Point(0.0f, 0.0f, 1.0f));

					PintActorHandle CurrentActor = Actors[i].mActor;
					PintActorHandle NextActor = Actors[j].mActor;

					//if(i==0)
					{
						RenderJointFrames(pint, CurrentActor, mEditPos0->mEdited, mEditRot0);
						RenderJointFrames(pint, NextActor, mEditPos1->mEdited, mEditRot1);
					}
				}

			}
		}
	}

	// TODO: move this to base class?

	if(mJointType)
	{
		const udword SelectedIndex = mJointType->GetSelectedIndex();
		if(SelectedIndex==IJT_RACK)
		{
			const RackJointWindow* RJW = static_cast<const RackJointWindow*>(mRackJointWindow);
			GLRenderHelpers::DrawBoxCorners(RJW->mRackBounds, Point(1.0f, 0.0f, 1.0f), 0.4f);
		}
	}

	if(mPendingAction==SNAP_POS_0)
	{
		const udword NbLinkActors = mLinkActors.GetNbLinkActors();
		if(NbLinkActors>1)
		{
			// ### this won't work for multiple pints
			mLinkActors.SortActors(pint);

			const LinkActor* Actors = mLinkActors.GetLinkActors();

			PintActorHandle CurrentActor = Actors[0].mActor;
			PintActorHandle NextActor = Actors[1].mActor;

			//#######
			Snap0(pint, CurrentActor, NextActor, mEditPos0, mEditPos1, mEditRot0, mEditRot1);
		}
	}
	else if(mPendingAction==SNAP_POS_1)
	{
		const udword NbLinkActors = mLinkActors.GetNbLinkActors();
		if(NbLinkActors>1)
		{
			// ### this won't work for multiple pints
			mLinkActors.SortActors(pint);

			const LinkActor* Actors = mLinkActors.GetLinkActors();

			PintActorHandle CurrentActor = Actors[0].mActor;
			PintActorHandle NextActor = Actors[1].mActor;

			//#######
			Snap1(pint, CurrentActor, NextActor, mEditPos0, mEditPos1, mEditRot0, mEditRot1);
		}
	}
	else if(mPendingAction==CREATE_JOINT && mJointType)
	{
		//CreateJoints(pint);

		const udword NbLinkActors = mLinkActors.GetNbLinkActors();
		if(NbLinkActors)
		{
			// ### this won't work for multiple pints
			mLinkActors.SortActors(pint);

			const LinkActor* Actors = mLinkActors.GetLinkActors();

			for(udword i=0;i<NbLinkActors;i++)
			{
				const udword j = (i+1)%NbLinkActors;
				PintActorHandle CurrentActor = Actors[i].mActor;
				PintActorHandle NextActor = Actors[j].mActor;

//### hardcoded test

				PINT_HINGE2_JOINT_CREATE desc;
				//Local::setupDesc(Desc, *this, JointName);
				//desc.mName	= jointName;
				desc.mObject0	= CurrentActor;
				desc.mObject1	= NextActor;

				{
					desc.mLocalPivot0.mPos = mEditPos0->mEdited;
					//desc.mLocalPivot0.mRot = mEditRot0 * q;
				}

				{
					desc.mLocalPivot1.mPos = mEditPos1->mEdited;
					//desc.mLocalPivot1.mRot = mEditRot1 * q;
				}

				pint.CreateJoint(desc);

			}
		}
	}
}

void ToolCreateChain::OnSnapButton(IceButton& button)
{
	const udword ID = button.GetID();
	if(ID>=2)
	{
		IceCore::MessageBox(null, "Not implemented yet :)", "Nope", MB_OK);
	}
	else if(ID==0)
	{
/*		if(!mActor1)
			IceCore::MessageBox(null, "Actor1 not defined yet", "Error", MB_OK);
		else*/
			mPendingAction=SNAP_POS_0;
	}
	else if(ID==1)
	{
/*		if(!mActor0)
			IceCore::MessageBox(null, "Actor0 not defined yet", "Error", MB_OK);
		else*/
			mPendingAction=SNAP_POS_1;
	}
}

void ToolCreateChain::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword x = 4;
	sdword y = 4;
	const sdword YStep = 20;

	struct Callbacks
	{
		static void SelectLinkActors(IceButton& button, void* user_data)
		{
			ToolCreateChain* tcc = reinterpret_cast<ToolCreateChain*>(user_data);

			// TODO: enable/disable selection
			if(!button.GetButtonDown())
			{
				button.SetButtonDown(true);
				tcc->mActiveButton = &button;

				// If there's already something selected, use this!
				DefaultControlInterface& DCI = GetDefaultControlInterface();
				const udword NbCurrentlySelected = DCI.mSelMan.GetNbSelected();
				if(NbCurrentlySelected)
				{
					button.SetButtonDown(false);
					tcc->mActiveButton = null;

					const SelectedObject* CurrentlySelected = DCI.mSelMan.GetSelection();
					for(udword i=0;i<NbCurrentlySelected;i++)
					{
						// TODO: refactor with MB callback
						Pint* pint = CurrentlySelected[i].mEngine;

						bool ValidateHit = true;
						if(tcc->mPint && tcc->mPint!=pint)
						{
							//IceCore::MessageBox(0, "You cannot join actors from different engines!", "Nice try", MB_OK);
							ValidateHit = false;
						}
						else
						{
							//ValidateHit = ValidateActor(Hit.mTouchedActor);
							ValidateHit = tcc->ValidateActor(CurrentlySelected[i].mActor);

							if(ValidateHit)
							{
								tcc->mPint = pint;

								LinkActor* Data = tcc->mLinkActors.ReserveSlot();
								Data->mActor = CurrentlySelected[i].mActor;
								//Data->mActor = Hit.mTouchedActor;
								//Data->mShape = Hit.mTouchedShape;

								//OnActorSelected();
								// ### hack to make selected links more visible
								//DefaultControlInterface& DCI = GetDefaultControlInterface();
								//SelectedObject* Selected = DCI.mSelMan.AddToSelection(mPint, Hit.mTouchedActor, pint_index);
							}
							//else
							//	IceCore::MessageBox(0, "You cannot select the same actor multiple times", "Error", MB_OK);
						}


					}
				}
			}
			else
			{
				button.SetButtonDown(false);
				tcc->mActiveButton = null;
			}
		}

		// This is called when we press the "create joints" button
		static void CreateJoints(IceButton& button, void* user_data)
		{
			ToolCreateChain* tcc = reinterpret_cast<ToolCreateChain*>(user_data);

			// TODO: add checks here
			/*if(!tcj->mActor0 && !tcj->mActor1)
			{
				IceCore::MessageBox(0, "No actors defined", "Error", MB_OK);
				return;
			}*/

/*			ASSERT(tcc->mPint);
			Pint_Actor* API = tcj->mPint->GetActorAPI();
			if(API)
			{
				if(tcj->mActor0 && API->GetMass(tcj->mActor0)==0.0f)
				{
					if(tcj->mActor1 && API->GetMass(tcj->mActor1)==0.0f)
					{
						IceCore::MessageBox(0, "Cannot create joint between two static actors", "Error", MB_OK);
						return;
					}
				}
			}*/

			tcc->mPendingAction=CREATE_JOINT;
		}
	};

	mButton0 = GUI_CreatePushButton(parent, 0, x, y, 200, 20, gButtonLabel2, &owner, Callbacks::SelectLinkActors, this, null);

	IceButton* B = helper.CreateButton(parent, 0, 260, 4, 200, 20, "Create joints", &owner, Callbacks::CreateJoints, this, null);

	y += YStep;
	y += YStep;
	CreateJointComboBox(helper, parent, owner, x, y);

	CreateJointWindows(helper, parent, owner, x, y);

	OnComboBoxChange();
}

