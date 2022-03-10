///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_ActorEdit.h"
#include "GUI_PosEdit.h"
//#include "Pint.h"
#include "PintShapeRenderer.h"
#include "PEEL.h"
#include "GLRenderHelpers.h"

/*static*/ EditActorWindow*	gEditActorWindow = null;

static const char* gNoName = "(null)";
static const char* gNotAvailable = "(not available - API not implemented)";

EditJointSubWindow::EditJointSubWindow(const WindowDesc& desc, EditBoxInterface& owner) :
	IceWindow	(desc),
	//
	mPint		(null),
	mHandle		(null)
{
	for(udword i=0;i<PINT_JOINT_NB;i++)
		mJointParams[i] = null;

	const udword OffsetX = 4;
	const udword EditBoxHeight = 20;
	const udword Width = 500;
	const udword JointNameLabelWidth = 60;
	const udword JointNameEditBoxWidth = Width-OffsetX*2-8-JointNameLabelWidth-16;

	udword YOffset = 4;
	mJointNameUI = CreateTextEditBox(owner, this, 4, YOffset, "Joint name:", JointNameLabelWidth, JointNameEditBoxWidth, EditBoxHeight, 1, "Default name");
	YOffset += EditBoxHeight + 4;

	mTypeLabel = GUI_CreateLabel(this, 4, YOffset, 200, 20, "Joint type:", null);

	if(1)
	{
		struct Callbacks
		{
			static void OnDeleteButton(IceButton& button, void* user_data)
			{
				EditJointSubWindow* EJSW = reinterpret_cast<EditJointSubWindow*>(user_data);
				EJSW->ReleaseJoint();
			}
			static void OnSwapButton(IceButton& button, void* user_data)
			{
				EditJointSubWindow* EJSW = reinterpret_cast<EditJointSubWindow*>(user_data);
				EJSW->SwapActors();
			}
		};

		const udword ButtonWidth = 80;
		const udword ButtonHeight = 20;
		mButton0 = GUI_CreateButton(this, 0, 300, YOffset, ButtonWidth, ButtonHeight, "Swap actors", null, Callbacks::OnSwapButton, this, null);
		mButton1 = GUI_CreateButton(this, 1, 400, YOffset, ButtonWidth, ButtonHeight, "Delete", null, Callbacks::OnDeleteButton, this, null);
	}

	YOffset += 20;

	mActor0Label = GUI_CreateLabel(this, 4, YOffset, JointNameEditBoxWidth, 20, "Actor0 name:", null);
	YOffset += 20;
	mActor1Label = GUI_CreateLabel(this, 4, YOffset, JointNameEditBoxWidth, 20, "Actor1 name:", null);
	YOffset += 20;

	udword SavedY = YOffset;


	// Copied from joint create tool - refactor?
	{
		WindowDesc WD;
		WD.mParent	= this;
		WD.mX		= 4;
		WD.mY		= YOffset;
		WD.mWidth	= 90;
		WD.mHeight	= 70;
		WD.mType	= WINDOW_NORMAL;
		WD.mStyle	= WSTYLE_BORDER;
//		WD.mStyle	= WSTYLE_CLIENT_EDGES;
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
		mEditPos0 = ICE_NEW(EditPosWindow)(WD, &owner);
		//owner.Register(mEditPos0);
		//mEditPos0->SetEnabled(false);

		const sdword Spacing = 110;

		WD.mX		= 4 + Spacing;
		mEditPos1 = ICE_NEW(EditPosWindow)(WD, &owner);
		//owner.Register(mEditPos1);
		//mEditPos1->SetEnabled(false);
	}

	YOffset += 80;

	mJointParams[PINT_JOINT_SPHERICAL] = ICE_NEW(GUI_SphericalJointParams);
	mJointParams[PINT_JOINT_HINGE] = ICE_NEW(GUI_HingeJointParams);
	mJointParams[PINT_JOINT_HINGE2] = ICE_NEW(GUI_HingeJointParams);
	mJointParams[PINT_JOINT_PRISMATIC] = ICE_NEW(GUI_PrismaticJointParams);
	//	PINT_JOINT_FIXED,
	mJointParams[PINT_JOINT_DISTANCE] = ICE_NEW(GUI_DistanceJointParams);
	mJointParams[PINT_JOINT_D6] = ICE_NEW(GUI_D6JointParams);
	mJointParams[PINT_JOINT_GEAR] = ICE_NEW(GUI_GearJointParams);
	mJointParams[PINT_JOINT_RACK_AND_PINION] = ICE_NEW(GUI_RackJointParams);
	//	PINT_JOINT_CHAIN,
	//	PINT_JOINT_PORTAL,

	const udword YStart[] = {
		YOffset,	// PINT_JOINT_UNDEFINED,
		YOffset,	// PINT_JOINT_SPHERICAL,
		YOffset,	// PINT_JOINT_HINGE,
		YOffset,	// PINT_JOINT_HINGE2,
		YOffset,	// PINT_JOINT_PRISMATIC,
		YOffset,	// PINT_JOINT_FIXED,
		YOffset,	// PINT_JOINT_DISTANCE,
		YOffset,	// PINT_JOINT_D6,
		SavedY,		// PINT_JOINT_GEAR,
		SavedY,		// PINT_JOINT_RACK_AND_PINION,
		YOffset,	// PINT_JOINT_CHAIN,
		YOffset,	// PINT_JOINT_PORTAL,
	};

	for(udword i=0;i<PINT_JOINT_NB;i++)
	{
		if(mJointParams[i])
			mJointParams[i]->Init(owner, this, YStart[i]);
	}
}

EditJointSubWindow::~EditJointSubWindow()
{
	for(udword i=0;i<PINT_JOINT_NB;i++)
		DELETESINGLE(mJointParams[i]);

	DELETESINGLE(mButton0);
	DELETESINGLE(mButton1);

	DELETESINGLE(mEditPos1);
	DELETESINGLE(mEditPos0);
	DELETESINGLE(mActor1Label);
	DELETESINGLE(mActor0Label);
	DELETESINGLE(mTypeLabel);
	DELETESINGLE(mJointNameUI);
}

void EditJointSubWindow::SaveParams()
{
	if(!mPint)
		return;	// Can be null if we never clicked on a joint

	Pint_Joint* JointAPI = mPint->GetJointAPI();

	if(mJointNameUI && mJointNameUI->mSomethingChanged)
	{
		mJointNameUI->mSomethingChanged = false;

		if(JointAPI)
		{
			String Buffer;
			mJointNameUI->GetTextAsString(Buffer);
			JointAPI->SetName(mHandle, Buffer);

			if(gEditActorWindow)
				gEditActorWindow->JointNameHasChanged(mHandle);
		}
	}

	bool WakeUpActors = false;

	if(mEditPos0 && mEditPos0->SomethingChanged())
	{
		mEditPos0->ResetChangedState();

		if(JointAPI)
		{
			PR LocalFrame;
			JointAPI->GetFrames(mHandle, &LocalFrame, null);

			LocalFrame.mPos = mEditPos0->GetPos();
			//printf("%f %f %f\n", p.x, p.y, p.z);

			JointAPI->SetFrames(mHandle, &LocalFrame, null);
			WakeUpActors = true;
		}
	}

	if(mEditPos1 && mEditPos1->SomethingChanged())
	{
		mEditPos1->ResetChangedState();

		if(JointAPI)
		{
			PR LocalFrame;
			JointAPI->GetFrames(mHandle, null, &LocalFrame);

			LocalFrame.mPos = mEditPos1->GetPos();
			//printf("%f %f %f\n", p.x, p.y, p.z);

			JointAPI->SetFrames(mHandle, null, &LocalFrame);
			WakeUpActors = true;
		}
	}

	// #### hmmm
	for(udword i=0;i<PINT_JOINT_NB;i++)
	{
		if(mJointParams[i])
			mJointParams[i]->SaveParams(JointAPI, mHandle, WakeUpActors);
	}

	if(WakeUpActors && JointAPI)
	{
		Pint_Actor* ActorAPI = mPint->GetActorAPI();
		if(ActorAPI)
		{
			PintActorHandle ah0, ah1;
			JointAPI->GetActors(mHandle, ah0, ah1);

			if(ah0)
				ActorAPI->WakeUp(ah0);
			if(ah1)
				ActorAPI->WakeUp(ah1);
		}
	}
}

void EditJointSubWindow::Cancel()
{
	if(mJointNameUI)
		mJointNameUI->mSomethingChanged = false;
	if(mEditPos0)
		mEditPos0->ResetChangedState();
	if(mEditPos1)
		mEditPos1->ResetChangedState();

	for(udword i=0;i<PINT_JOINT_NB;i++)
	{
		if(mJointParams[i])
			mJointParams[i]->Cancel();
	}
}

bool EditJointSubWindow::SomethingChanged() const
{
	if(mJointNameUI && mJointNameUI->mSomethingChanged)
		return true;
	if(mEditPos0 && mEditPos0->SomethingChanged())
		return true;
	if(mEditPos1 && mEditPos1->SomethingChanged())
		return true;

	for(udword i=0;i<PINT_JOINT_NB;i++)
	{
		if(mJointParams[i])
			if(mJointParams[i]->SomethingChanged())
				return true;
	}

	return false;
}

void EditJointSubWindow::AskUserForSave()
{
	if(SomethingChanged())
	{
		int ret = IceCore::MessageBoxA(null, "Do you want to save your changes?", "Something changed in Edit Joint dialog", MB_OKCANCEL);
		if(ret==IDOK)
			SaveParams();

		Cancel();
	}
}

void EditJointSubWindow::InitFrom(Pint* pint, PintJointHandle handle, PintActorHandle owner)
{
	AskUserForSave();

	mPint = pint;
	mHandle = handle;

	SetFocusActor();

	Pint_Joint* JointAPI = pint ? pint->GetJointAPI() : null;
	Pint_Actor* ActorAPI = pint ? pint->GetActorAPI() : null;

	if(!ActorAPI)	// Might not be implemented by all plugins
	{
		if(mActor0Label)
			mActor0Label->SetLabel("Actor0: (not available)");
		if(mActor1Label)
			mActor1Label->SetLabel("Actor1: (not available)");
	}

	for(udword i=0;i<PINT_JOINT_NB;i++)
	{
		if(mJointParams[i])
		{
			mJointParams[i]->Cancel();
			mJointParams[i]->SetVisible(false);
		}
	}

	bool HasLocalFrameEditBoxes = false;

	if(!JointAPI)	// Might not be implemented by all plugins
	{
		if(mJointNameUI)
			mJointNameUI->SetText(gNotAvailable);
		if(mTypeLabel)
			mTypeLabel->SetLabel("Joint type: (not available)");

		if(mEditPos0)
			mEditPos0->SetEnabled(false);
		if(mEditPos1)
			mEditPos1->SetEnabled(false);
	}
	else
	{
		const char* JointName = JointAPI->GetName(handle);
		const PintJoint Type = JointAPI->GetType(handle);

		PintActorHandle ah0, ah1;
		JointAPI->GetActors(handle, ah0, ah1);

		if(ah0 && owner==ah0)
			SetFocusActor(ah1);
		else if(ah1 && owner==ah1)
			SetFocusActor(ah0);

		if(mJointNameUI)
			mJointNameUI->SetText(JointName ? JointName : gNoName);

		if(mTypeLabel)
			mTypeLabel->SetLabel(_F("Joint type: %s", GetPintJointName(Type)));

		if(ActorAPI)
		{
			const char* Actor0Name = ah0 ? ActorAPI->GetName(ah0) : "(Static world. Joint has a single defined actor.)";
			const char* Actor1Name = ah1 ? ActorAPI->GetName(ah1) : "(Static world. Joint has a single defined actor.)";
			if(mActor0Label)
				mActor0Label->SetLabel(_F("Actor0: %s", Actor0Name ? Actor0Name : gNoName));
			if(mActor1Label)
				mActor1Label->SetLabel(_F("Actor1: %s", Actor1Name ? Actor1Name : gNoName));
		}

		switch(Type)
		{
			case PINT_JOINT_SPHERICAL:
			case PINT_JOINT_HINGE:
			case PINT_JOINT_HINGE2:
			case PINT_JOINT_PRISMATIC:
			case PINT_JOINT_FIXED:
			case PINT_JOINT_DISTANCE:
			case PINT_JOINT_D6:
			//PINT_JOINT_GEAR,
			//PINT_JOINT_RACK_AND_PINION,
			//PINT_JOINT_CHAIN,
			//PINT_JOINT_PORTAL,
				HasLocalFrameEditBoxes = true;
				break;
		}

		if(HasLocalFrameEditBoxes)
		{
			PR LocalFrame0, LocalFrame1;
			JointAPI->GetFrames(handle, &LocalFrame0, &LocalFrame1);

			if(mEditPos0)
			{
				mEditPos0->SetEnabled(true);
				mEditPos0->SetPos(LocalFrame0.mPos);
			}

			if(mEditPos1)
			{
				mEditPos1->SetEnabled(true);
				mEditPos1->SetPos(LocalFrame1.mPos);
			}
		}

		if(mJointParams[Type])
		{
			mJointParams[Type]->InitFrom(JointAPI, handle);
			mJointParams[Type]->SetVisible(true);
		}
	}

	if(mEditPos0)
		mEditPos0->SetVisible(HasLocalFrameEditBoxes);
	if(mEditPos1)
		mEditPos1->SetVisible(HasLocalFrameEditBoxes);
}

void EditJointSubWindow::ReleaseJoint()
{
	if(!mPint || !mHandle)
		return;
	int ret = IceCore::MessageBoxA(null, "Delete the joint?\nThis operation cannot be undone.", "Warning", MB_OKCANCEL);
	if(ret==IDOK)
	{
		mPint->ReleaseJoint(mHandle);

		// Refresh UI
		gEditActorWindow->InitFrom(gEditActorWindow->mPint, gEditActorWindow->mHandle);
	}
}

void EditJointSubWindow::SwapActors()
{
	if(!mPint || !mHandle)
		return;

	Pint_Joint* JointAPI = mPint->GetJointAPI();
	if(!JointAPI)
		return;

	PR LocalFrame0, LocalFrame1;
	JointAPI->GetFrames(mHandle, &LocalFrame0, &LocalFrame1);

	PintActorHandle ah0, ah1;
	JointAPI->GetActors(mHandle, ah0, ah1);

	TSwap(ah0, ah1);
	TSwap(LocalFrame0, LocalFrame1);

	JointAPI->SetFrames(mHandle, &LocalFrame0, &LocalFrame1);
	JointAPI->SetActors(mHandle, ah0, ah1);

	// ### refactor this bit
	Pint_Actor* ActorAPI = mPint->GetActorAPI();
	if(ActorAPI)
	{
		const char* Actor0Name = ah0 ? ActorAPI->GetName(ah0) : "(Static world. Joint has a single defined actor.)";
		const char* Actor1Name = ah1 ? ActorAPI->GetName(ah1) : "(Static world. Joint has a single defined actor.)";
		if(mActor0Label)
			mActor0Label->SetLabel(_F("Actor0: %s", Actor0Name ? Actor0Name : gNoName));
		if(mActor1Label)
			mActor1Label->SetLabel(_F("Actor1: %s", Actor1Name ? Actor1Name : gNoName));
	}

	if(mEditPos0)
		mEditPos0->SetPos(LocalFrame0.mPos);

	if(mEditPos1)
		mEditPos1->SetPos(LocalFrame1.mPos);
}

// Copied from create joint tool - refactor?
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

void EditJointSubWindow::Render()
{
	if(!mPint || !mHandle || !IsVisible())
		return;
	Pint_Joint* JointAPI = mPint->GetJointAPI();
	if(!JointAPI)
		return;

	PR LocalFrame0, LocalFrame1;
	JointAPI->GetFrames(mHandle, &LocalFrame0, &LocalFrame1);

	PintActorHandle Actor0, Actor1;
	JointAPI->GetActors(mHandle, Actor0, Actor1);

	RenderJointFrames(*mPint, Actor0, mEditPos0->GetPos(), LocalFrame0.mRot);
	RenderJointFrames(*mPint, Actor1, mEditPos1->GetPos(), LocalFrame1.mRot);
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class EAWButton : public IceButton
	{
		public:
									EAWButton(EditActorWindow& owner, const ButtonDesc& desc) : IceButton(desc), mOwner(owner)	{}
		virtual						~EAWButton()																				{}

		virtual	void				OnClick()
									{
										if(GetID()==0)
											mOwner.SaveParams();
										else
											mOwner.Cancel();

										mOwner.SetVisible(false);
									}

				EditActorWindow&	mOwner;
//				bool				mSaveOnClick;
	};

	class EAListBox : public IceListBox
	{
		EditActorWindow&	mOwner;
		public:
							EAListBox(const ListBoxDesc& desc, EditActorWindow& owner) : IceListBox(desc), mOwner(owner), mNbShapes(0), mNbJoints(0), mSelectedJointIndex(INVALID_ID)	{}
		virtual				~EAListBox()																																				{}

		virtual	void	OnListboxEvent(ListBoxEvent event)
		{
			if(event==LBE_SELECTION_CHANGED)
			{
				const udword Selected = GetSelectedIndex();
				if(Selected<mNbShapes)
				{
					//printf("Selected shape\n");
					const PintShapeHandle sh = PintShapeHandle(GetItemData(Selected));
					SetFocusShape(mOwner.mHandle, sh);

					if(mOwner.mJointEditWindow)
					{
						mOwner.mJointEditWindow->InitFrom(null, null, null);
						mOwner.mJointEditWindow->SetVisible(false);
					}

					mSelectedJointIndex = INVALID_ID;
				}
				else
				{
					const PintJointHandle jh = PintJointHandle(GetItemData(Selected));
					SetFocusShape();

					//printf("Selected joint\n");
					if(mOwner.mJointEditWindow)
					{
						mOwner.mJointEditWindow->InitFrom(mOwner.mPint, jh, mOwner.mHandle);
						mOwner.mJointEditWindow->SetVisible(true);
					}

					mSelectedJointIndex = Selected;
				}
			}
			else if(event==LBE_DBL_CLK)
			{
				SetFocusShape();
				//mSelectedJointIndex = INVALID_ID;
			}
		}

		udword	mNbShapes;
		udword	mNbJoints;
		udword	mSelectedJointIndex;
	};
}

EditActorWindow::EditActorWindow(const WindowDesc& desc) :
	IceWindow				(desc),
	mActorNameUI			(null),
	mActorMassUI			(null),
	mActorLinearDampingUI	(null),
	mActorAngularDampingUI	(null),
	mPint					(null),
	mHandle					(null),
	mJointEditWindow		(null)
{
}

EditActorWindow::~EditActorWindow()
{
	DELETESINGLE(mJointEditWindow);
	DELETESINGLE(mShapesAndJoints);
	DELETESINGLE(mInfoLabel);
	DELETESINGLE(mShapesAndJointsLabel);
	DELETESINGLE(mCancelButton);
	DELETESINGLE(mOKButton);
	DELETESINGLE(mActorAngularDampingUI);
	DELETESINGLE(mActorLinearDampingUI);
	DELETESINGLE(mActorMassUI);
	DELETESINGLE(mActorNameUI);
}

void EditActorWindow::ChangeNotification()
{
	if(mOKButton)
		mOKButton->SetEnabled(true);
}

void EditActorWindow::Cancel()
{
	if(mActorNameUI)
		mActorNameUI->mSomethingChanged = false;
	if(mActorMassUI)
		mActorMassUI->mSomethingChanged = false;
	if(mActorLinearDampingUI)
		mActorLinearDampingUI->mSomethingChanged = false;
	if(mActorAngularDampingUI)
		mActorAngularDampingUI->mSomethingChanged = false;
	if(mJointEditWindow)
		mJointEditWindow->Cancel();
}

bool EditActorWindow::SomethingChanged() const
{
	if(mActorNameUI && mActorNameUI->mSomethingChanged)
		return true;
	if(mActorMassUI && mActorMassUI->mSomethingChanged)
		return true;
	if(mActorLinearDampingUI && mActorLinearDampingUI->mSomethingChanged)
		return true;
	if(mActorAngularDampingUI && mActorAngularDampingUI->mSomethingChanged)
		return true;
	if(mJointEditWindow && mJointEditWindow->SomethingChanged())
		return true;
	return false;
}

void EditActorWindow::AskUserForSave()
{
	if(SomethingChanged())
	{
		const int ret = IceCore::MessageBoxA(null, "Do you want to save your changes?", "Something changed in Edit Actor dialog", MB_OKCANCEL);
		if(ret==IDOK)
			SaveParams();

		Cancel();
	}
}

static void FillShapesAndJointsListBox(EAListBox* list_box, Pint* pint, PintActorHandle handle)
{
	if(list_box)
	{
		Pint_Actor* ActorAPI = pint ? pint->GetActorAPI() : null;

		const udword NbShapes = ActorAPI ? ActorAPI->GetNbShapes(handle) : INVALID_ID;
		const udword NbJoints = ActorAPI ? ActorAPI->GetNbJoints(handle) : INVALID_ID;

		list_box->mNbShapes = NbShapes;
		list_box->mNbJoints = NbJoints;

		if(NbShapes!=INVALID_ID)
		{
			Pint_Shape* ShapeAPI = pint->GetShapeAPI();
			if(ShapeAPI)
			{
				for(udword i=0;i<NbShapes;i++)
				{
					const PintShapeHandle sh = ActorAPI->GetShape(handle, i);
					const char* ShapeName = ShapeAPI->GetName(sh);
					if(!ShapeName)
						ShapeName = gNoName;
					
					const PintShape ShapeType = ShapeAPI->GetType(sh);
					const char* ShapeTypeName = GetPintShapeName(ShapeType);

					PintShapeRenderer* PSR = ShapeAPI->GetShapeRenderer(sh);
					const char* RendererName = PSR ? PSR->GetClassName() : gNoName;
					const udword NbRenderers = PSR ? PSR->GetNbRenderers() : 0;

					list_box->Add(_F("Shape %d (0x%x) - %s - %s - %s (%d)", i, size_t(sh), ShapeTypeName, ShapeName, RendererName, NbRenderers));
					// ### hardcoded IDs here aren't nice
					list_box->SetItemData(i, sh);
				}
			}
		}

		if(NbJoints!=INVALID_ID)
		{
			Pint_Joint* JointAPI = pint->GetJointAPI();
			if(JointAPI)
			{
				for(udword i=0;i<NbJoints;i++)
				{
					const PintJointHandle jh = ActorAPI->GetJoint(handle, i);
					const char* JointName = JointAPI->GetName(jh);
					if(!JointName)
						JointName = gNoName;
					
					const PintJoint JointType = JointAPI->GetType(jh);
					const char* JointTypeName = GetPintJointName(JointType);

					list_box->Add(_F("Joint %d (0x%x) - %s - %s", i, size_t(jh), JointTypeName, JointName));
					// ### hardcoded IDs here aren't nice
					list_box->SetItemData(i+NbShapes, jh);
				}
			}
		}
	}
}

// Notification from joint's sub-window
void EditActorWindow::JointNameHasChanged(PintJointHandle handle)
{
	// Brute-force refill because "SetItemText" doesn't work
	IceListBox* _LB = mShapesAndJoints;
	EAListBox* LB = static_cast<EAListBox*>(_LB);
	if(LB)
	{
		const int Selected = LB->GetSelectedIndex();
		LB->RemoveAll();
		FillShapesAndJointsListBox(LB, mPint, mHandle);
		//LB->Select(LB->mSelectedJointIndex);
		LB->Select(Selected);
	}

	if(0)
	{
		Pint_Joint* JointAPI = mPint ? mPint->GetJointAPI() : null;
		if(!LB || !JointAPI)
			return;

		const PintJointHandle jh = PintJointHandle(LB->GetItemData(LB->mSelectedJointIndex));
		ASSERT(jh==handle);

		const char* JointName = JointAPI->GetName(jh);
		if(!JointName)
			JointName = gNoName;
		
		const PintJoint JointType = JointAPI->GetType(jh);
		const char* JointTypeName = GetPintJointName(JointType);

		const udword i = LB->mSelectedJointIndex - LB->mNbShapes;
		// TODO: SetItemText doesn't work
		//LB->SetItemText(LB->mSelectedJointIndex, _F("Joint %d - %s - %s", i, JointTypeName, JointName));

		// TODO: and this doesn't preserve the indices. FFS Windows.
		//LB->Remove(LB->mSelectedJointIndex);
		//LB->Add(_F("Joint %d - %s - %s", i, JointTypeName, JointName));
	}
}

void EditActorWindow::InitFrom(Pint* pint, PintActorHandle handle)
{
	AskUserForSave();

	Pint_Actor* ActorAPI = pint ? pint->GetActorAPI() : null;

	const char* ActorName = null;
	if(pint)
	{
		ActorName = ActorAPI ? ActorAPI->GetName(handle) : gNotAvailable;

		mPint	= pint;
		mHandle	= handle;
		if(mOKButton)
			mOKButton->SetEnabled(false);	// Disabled until a change has been made. (User-friendly visual feedback)
	}
	else
	{
		mPint	= null;
		mHandle	= null;
	}

	if(mActorNameUI)
		mActorNameUI->SetText(ActorName);

	///

	if(ActorAPI)
	{
		const float Mass = ActorAPI->GetMass(handle);
		const bool IsDynamic = Mass!=0.0f;

		if(mActorMassUI)
		{
			mActorMassUI->SetText(_F("%f", Mass));
			mActorMassUI->SetEnabled(IsDynamic);
		}

		if(mActorLinearDampingUI)
		{
			mActorLinearDampingUI->SetText(_F("%f", ActorAPI->GetLinearDamping(handle)));
			mActorLinearDampingUI->SetEnabled(IsDynamic);
		}

		if(mActorAngularDampingUI)
		{
			mActorAngularDampingUI->SetText(_F("%f", ActorAPI->GetAngularDamping(handle)));
			mActorAngularDampingUI->SetEnabled(IsDynamic);
		}
	}

	///

	if(mShapesAndJoints)
		mShapesAndJoints->RemoveAll();

	if(mShapesAndJointsLabel)
	{
		if(pint)
		{
			const udword NbShapes = ActorAPI ? ActorAPI->GetNbShapes(handle) : INVALID_ID;
			const udword NbJoints = ActorAPI ? ActorAPI->GetNbJoints(handle) : INVALID_ID;

			if(NbShapes!=INVALID_ID)
			{
				if(NbShapes==1)
					mShapesAndJointsLabel->SetLabel(_F("Single actor (0x%x)", size_t(handle)));
				else
					mShapesAndJointsLabel->SetLabel(_F("Compound actor - %d shapes (0x%x)", NbShapes, size_t(handle)));
			}
			else
				mShapesAndJointsLabel->SetLabel("Nb shapes: (not available)");

			IceListBox* _LB = mShapesAndJoints;
			FillShapesAndJointsListBox(static_cast<EAListBox*>(_LB), pint, handle);
		}
		else
		{
			mShapesAndJointsLabel->SetLabel(null);
		}
	}

	if(mJointEditWindow)
	{
		mJointEditWindow->SetVisible(false);

		if(!pint && !handle)
			mJointEditWindow->InitFrom(null, null, null);
	}
}

void EditActorWindow::SaveParams()
{
	if(mPint)
	{
		if(mActorNameUI && mActorNameUI->mSomethingChanged)
		{
			mActorNameUI->mSomethingChanged = false;

			Pint_Actor* API = mPint->GetActorAPI();
			if(API)
			{
				String Buffer;
				mActorNameUI->GetTextAsString(Buffer);

				API->SetName(mHandle, Buffer);
	//			API->SetMass(mHandle, 0.01f);
			}
		}

		if(mActorMassUI && mActorMassUI->mSomethingChanged)
		{
			mActorMassUI->mSomethingChanged = false;

			Pint_Actor* API = mPint->GetActorAPI();
			if(API)
			{
				const float Value = mActorMassUI->GetFloat();
				API->SetMass(mHandle, Value);
				API->WakeUp(mHandle);
			}
		}

		if(mActorLinearDampingUI && mActorLinearDampingUI->mSomethingChanged)
		{
			mActorLinearDampingUI->mSomethingChanged = false;

			Pint_Actor* API = mPint->GetActorAPI();
			if(API)
			{
				const float Value = mActorLinearDampingUI->GetFloat();
				API->SetLinearDamping(mHandle, Value);
				API->WakeUp(mHandle);
			}
		}

		if(mActorAngularDampingUI && mActorAngularDampingUI->mSomethingChanged)
		{
			mActorAngularDampingUI->mSomethingChanged = false;

			Pint_Actor* API = mPint->GetActorAPI();
			if(API)
			{
				const float Value = mActorAngularDampingUI->GetFloat();
				API->SetAngularDamping(mHandle, Value);
				API->WakeUp(mHandle);
			}
		}
	}

	if(mJointEditWindow)
		mJointEditWindow->SaveParams();
}

int EditActorWindow::handleEvent(IceGUIEvent* event)
{
	if(event->mType==EVENT_CLOSE)
		AskUserForSave();
	return 0;
}

void EditActorWindow::PreRenderCallback()
{
	if(mJointEditWindow)
		mJointEditWindow->Render();
}

///////////////////////////////////////////////////////////////////////////////

EditActorWindow* CreateActorEditGUI(IceWidget* parent, sdword x, sdword y)
{
	const udword ShapeOrJointEditAreaSize = 300;
	const udword ShapeOrJointListAreaSize = 200;

	const udword Width = 500;
	const udword Height = 240 + ShapeOrJointListAreaSize + ShapeOrJointEditAreaSize;

	EditActorWindow* EAW;
	{
		WindowDesc WD;
		WD.mParent	= parent;
		WD.mX		= x;
		WD.mY		= y;
		WD.mWidth	= Width;
		WD.mHeight	= Height;
		WD.mLabel	= "Edit actor";
		WD.mType	= WINDOW_DIALOG;
		WD.mStyle	= WSTYLE_HIDDEN;

		EAW = ICE_NEW(EditActorWindow)(WD);
	}
//	EAW->SetVisible(true);

	sdword YOffset = 4;
	{
		const udword OffsetX = 4;

		const udword NameLabelWidth = 40;
		const udword ButtonWidth = Width-OffsetX*2-8;
		const udword NameEditBoxWidth = Width-OffsetX*2-8-NameLabelWidth;
		const udword FloatEditBoxWidth = 60;
		const udword EditBoxHeight = 20;

		EAW->mActorNameUI = CreateTextEditBox(*EAW, EAW, OffsetX, YOffset, "Name:", NameLabelWidth, NameEditBoxWidth, EditBoxHeight, 0, "Default name");
		YOffset += EditBoxHeight;

		EAW->mShapesAndJointsLabel = GUI_CreateLabel(EAW, OffsetX, YOffset+2, Width - OffsetX*2 - 8, 20, "Nb shapes:", null);
		YOffset += 20;

		const udword NewLabelWidth = 100;
		EAW->mActorMassUI = CreateFloatEditBox(*EAW, EAW, OffsetX, YOffset, "Mass:", NewLabelWidth, FloatEditBoxWidth, EditBoxHeight, 0, 0.0f);
		YOffset += EditBoxHeight;

		EAW->mActorLinearDampingUI = CreateFloatEditBox(*EAW, EAW, OffsetX, YOffset, "Linear damping:", NewLabelWidth, FloatEditBoxWidth, EditBoxHeight, 0, 0.0f);
		YOffset += EditBoxHeight;

		EAW->mActorAngularDampingUI = CreateFloatEditBox(*EAW, EAW, OffsetX, YOffset, "Angular damping:", NewLabelWidth, FloatEditBoxWidth, EditBoxHeight, 0, 0.0f);
		YOffset += EditBoxHeight;

		EAW->mInfoLabel = GUI_CreateLabel(EAW, OffsetX, YOffset+2, Width - OffsetX*2 - 8, 20, "Shape ID  -  Shape type  -  Shape name  -  Renderer name  -  Nb renderers:", null);
		YOffset += 22;

		{
			ListBoxDesc LBD;
//			LBD.mType				= LISTBOX_MULTISELECTION;
			LBD.mParent				= EAW;
			LBD.mX					= OffsetX;
			LBD.mY					= YOffset;
			LBD.mWidth				= Width - LBD.mX*2 - 8;
			LBD.mHeight				= ShapeOrJointListAreaSize;
			LBD.mLabel				= "List box";
			EAW->mShapesAndJoints	= ICE_NEW(EAListBox)(LBD, *EAW);

			YOffset += LBD.mHeight + 4;
		}

		YOffset += 4;
		if(1)
		{
			WindowDesc WD;
			WD.mParent	= EAW;
			WD.mX		= OffsetX;
			WD.mY		= YOffset;
			WD.mWidth	= Width - WD.mX*2 - 8;
			WD.mHeight	= ShapeOrJointEditAreaSize;
			WD.mLabel	= "Edit shape or joint";
			WD.mType	= WINDOW_DIALOG;
			WD.mStyle	= WSTYLE_HIDDEN;
			//WD.mType	= WINDOW_NORMAL;
			WD.mStyle	= WSTYLE_CLIENT_EDGES;
			//WD.mStyle	= WSTYLE_BORDER;
			EAW->mJointEditWindow = ICE_NEW(EditJointSubWindow)(WD, *EAW);
			EAW->mJointEditWindow->SetVisible(false);

			YOffset += WD.mHeight + 4;
		}
		else
		{
			const udword TabControlBorder = 4;

			TabControlDesc TCD;
			TCD.mParent	= EAW;
			TCD.mX		= TabControlBorder;
			TCD.mY		= YOffset;
			TCD.mWidth	= Width - TCD.mX*2 - TabControlBorder*2;
			TCD.mHeight	= ShapeOrJointEditAreaSize;
			IceTabControl* TabControl = ICE_NEW(IceTabControl)(TCD);

			YOffset += TCD.mHeight + 4;

			const udword NbTabs = 3;
			IceWindow* Tabs[NbTabs];
			for(udword i=0;i<NbTabs;i++)
			{
				WindowDesc WD;
				WD.mParent	= EAW;
				WD.mX		= 0;
				WD.mY		= 0;
				WD.mWidth	= TCD.mWidth;
				WD.mHeight	= TCD.mHeight;
				WD.mLabel	= "Tab";
				WD.mType	= WINDOW_DIALOG;
				WD.mStyle	= WSTYLE_CLIENT_EDGES;

				IceWindow* Tab;
				if(i==2)
				{
					EAW->mJointEditWindow = ICE_NEW(EditJointSubWindow)(WD, *EAW);
					Tab = EAW->mJointEditWindow;
				}
				else
				{
					Tab = ICE_NEW(IceWindow)(WD);
				}
				Tab->SetVisible(false);
				Tabs[i] = Tab;
			}
			TabControl->Add(Tabs[0], "Actor");
			TabControl->Add(Tabs[1], "Shapes");
			TabControl->Add(Tabs[2], "Joints");
		}

		YOffset += 20;

		ButtonDesc BD;
		BD.mID			= 0;
		BD.mParent		= EAW;
		BD.mX			= OffsetX;
		BD.mY			= YOffset;
		BD.mWidth		= ButtonWidth;
		BD.mHeight		= 20;
		BD.mLabel		= "OK";
		EAW->mOKButton	= ICE_NEW(EAWButton)(*EAW, BD);

		YOffset += BD.mHeight;
		BD.mID				= 1;
		BD.mY				= YOffset;
		BD.mLabel			= "Cancel";
		EAW->mCancelButton	= ICE_NEW(EAWButton)(*EAW, BD);
	}

	gEditActorWindow = EAW;

	return EAW;
}

void CloseActorEditGUI()
{
	DELETESINGLE(gEditActorWindow);
}

void HideEditActorWindow()
{
	if(!gEditActorWindow)
		return;
	gEditActorWindow->SetVisible(false);
	gEditActorWindow->InitFrom(null, null);	// Make sure we don't keep dangling pointers in there
}

void ShowEditActorWindow(Pint* pint, PintActorHandle handle)
{
	if(!gEditActorWindow)
		return;

	gEditActorWindow->InitFrom(pint, handle);

	// Passed mouse coords are local to the render window so we do this instead
	POINT pt;
	IceCore::GetCursorPos(&pt);
	gEditActorWindow->SetPosition(pt.x, pt.y);
	//gEditActorWindow->SetPosition(mouse.mMouseX, mouse.mMouseY);

	gEditActorWindow->SetVisible(true);
}

EditActorWindow* GetVisibleActorEditWindow()
{
	if(!gEditActorWindow || !gEditActorWindow->IsVisible())
		return null;
	return gEditActorWindow;
}
