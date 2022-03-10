///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_CompoundEdit.h"
#include "GUI_PosEdit.h"
#include "Pint.h"
#include "PEEL.h"
#include "GLRenderHelpers.h"

namespace
{
	class CCWButton : public IceButton
	{
		public:
									CCWButton(CompoundCreateWindow& owner, const ButtonDesc& desc) : IceButton(desc), mOwner(owner)	{}
		virtual						~CCWButton()																					{}

		virtual	void				OnClick()
									{
										if(GetID()==0)
											mOwner.CreateCompound();

										mOwner.SetVisible(false);
									}

				CompoundCreateWindow&	mOwner;
	};
}

CompoundCreateWindow::CompoundCreateWindow(const WindowDesc& desc) :
	IceWindow	(desc),
	mCallback	(null),
	mEditPos	(null),
	mEditRot	(null),
	mPint		(null),
	mIndex		(INVALID_ID),
	mSelected	(null)
{
	mCompoundPR.Identity();
}

CompoundCreateWindow::~CompoundCreateWindow()
{
	mHandles.Empty();
	DELETESINGLE(mActors);
	DELETESINGLE(mMassLabel);
	DELETESINGLE(mActorsLabel);
	DELETESINGLE(mNameLabel);
	DELETESINGLE(mButton0);
	DELETESINGLE(mButton1);
	DELETESINGLE(mCancelButton);
	DELETESINGLE(mOKButton);
	DELETESINGLE(mMassEditBox);
	DELETESINGLE(mNameEditBox);
	DELETESINGLE(mEditPos);
	DELETESINGLE(mEditRot);
}

void CompoundCreateWindow::SetCompoundPos(const Point& pos)
{
	mCompoundPR.mPos = pos;
	if(mEditPos)
		mEditPos->SetPos(pos);
}

void CompoundCreateWindow::SetCompoundRot(const Quat& rot)
{
	mCompoundPR.mRot = rot;
	if(mEditRot)
		mEditRot->SetEulerAngles(rot);
}

void CompoundCreateWindow::InitFrom(Pint* pint, udword index, udword nb, const PintActorHandle* handles, CreateCompoundCallback* callback)
{
	mSelected = null;
	mCallback = callback;
	mPint = pint;
	mIndex = index;
	mHandles.Empty();
	for(udword i=0;i<nb;i++)
		mHandles.AddPtr(handles[i]);

	Pint_Actor* API = pint ? pint->GetActorAPI() : null;

	if(mNameEditBox)
	{
		const char* CompoundName;
		if(nb==1 && API)
			CompoundName = API->GetName(*handles);
		else
			CompoundName = "(Merged actors)";
		mNameEditBox->SetText(CompoundName);
	}

	{
		float Mass = 0.0f;
		Point CompoundPos(0.0f, 0.0f, 0.0f);
		Quat CompoundRot(1.0f, 0.0f, 0.0f, 0.0f);
		if(nb && API)
		{
			const float Coeff = 1.0f/float(nb);
			for(udword i=0;i<nb;i++)
			{
				Mass += API->GetMass(handles[i])*Coeff;

				const PR GlobalPose = pint->GetWorldTransform(handles[i]);
				CompoundPos += GlobalPose.mPos*Coeff;
				
				if(i==0)
					CompoundRot = GlobalPose.mRot;
			}
		}
		SetCompoundPos(CompoundPos);
		SetCompoundRot(CompoundRot);

		if(mMassEditBox)
			mMassEditBox->SetText(_F("%f", Mass));
	}

	if(mActorsLabel)
		mActorsLabel->SetLabel(_F("Nb actors to merge: %d", nb));

	if(mActors)
	{
		mActors->RemoveAll();

		for(udword i=0;i<nb;i++)
		{
			const char* ActorName = API ? API->GetName(handles[i]) : null;
			if(!ActorName)
				ActorName = "(unnamed)";
			mActors->Add(_F("Actor %i: %s (0x%x)", i, ActorName, size_t(handles[i])));
		}
	}
}

void CompoundCreateWindow::CreateCompound()
{
	if(mCallback)
		mCallback->CreateCompound(*this);
}

void CompoundCreateWindow::PreRenderCallback()
{
	if(mEditPos && mEditPos->mHasEdit)
	{
		mEditPos->mHasEdit = false;
		mCompoundPR.mPos = mEditPos->mEdited;
	}

	if(mEditRot && mEditRot->mHasEdit)
	{
		mEditRot->mHasEdit = false;
		mEditRot->GetEulerAngles(mCompoundPR.mRot);
	}

	const float Scale = GetFrameSize();
	const bool Symmetric = GetSymmetricFrames();
	GLRenderHelpers::DrawFrame(mCompoundPR, Scale, Symmetric);
	if(mSelected)
	{
		const PR GlobalPose = mPint->GetWorldTransform(mSelected);
		GLRenderHelpers::DrawFrame(GlobalPose, Scale, Symmetric);
	}
}

int CompoundCreateWindow::handleEvent(IceGUIEvent* event)
{
	return 0;
}

namespace
{
	class MyListBox : public IceListBox
	{
		CompoundCreateWindow&	mOwner;
		public:
						MyListBox(const ListBoxDesc& desc, CompoundCreateWindow& owner) : IceListBox(desc), mOwner(owner)	{}
		virtual			~MyListBox()																						{}

/*		virtual int		handleEvent(IceGUIEvent* event)
		{
			return 0;
		}*/

		virtual	void	OnListboxEvent(ListBoxEvent event)
		{
			if(event==LBE_SELECTION_CHANGED)
			{
				const udword Selected = GetSelectedIndex();
				ASSERT(Selected<mOwner.mHandles.GetNbEntries());
				if(Selected<mOwner.mHandles.GetNbEntries())
				{
					mOwner.mSelected = PintActorHandle(mOwner.mHandles[Selected]);
					if(mOwner.mButton0)
						mOwner.mButton0->SetEnabled(true);
					if(mOwner.mButton1)
						mOwner.mButton1->SetEnabled(true);
				}
			}
			else if(event==LBE_DBL_CLK)
			{
				Select(-1);
				mOwner.mSelected = null;
				if(mOwner.mButton0)
					mOwner.mButton0->SetEnabled(false);
				if(mOwner.mButton1)
					mOwner.mButton1->SetEnabled(false);
			}

			SetFocusActor(mOwner.mSelected);
		}
	};
}

static CompoundCreateWindow* gCompoundCreateWindow = null;

CompoundCreateWindow* CreateCompoundCreateGUI(IceWidget* parent, sdword x, sdword y)
{
	const udword Width = 300;
	const udword Height = 500;

	WindowDesc WD;
	WD.mParent	= parent;
	WD.mX		= x;
	WD.mY		= y;
	WD.mWidth	= Width;
	WD.mHeight	= Height;
	WD.mLabel	= "Create compound";
	WD.mType	= WINDOW_DIALOG;
	WD.mStyle	= WSTYLE_HIDDEN;

	CompoundCreateWindow* CCW = ICE_NEW(CompoundCreateWindow)(WD);
	ASSERT(CCW);
//	CCW->SetVisible(true);

	sdword YOffset = 4;
	{
		const udword OffsetX = 4;

		const udword NameLabelWidth = 40;
		const udword ButtonWidth = Width-OffsetX*2-8;
		const udword EditBoxWidth = Width-OffsetX*2-8-NameLabelWidth;
		const udword EditBoxHeight = 20;

		CCW->mNameLabel = GUI_CreateLabel(CCW, OffsetX, YOffset+2, NameLabelWidth, 20, "Name:", null);
		CCW->mNameEditBox = GUI_CreateEditBox(CCW, 0, OffsetX + NameLabelWidth, YOffset, EditBoxWidth, EditBoxHeight, "Default name", null, EDITBOX_TEXT, null, null);
		YOffset += EditBoxHeight;

		CCW->mMassLabel = GUI_CreateLabel(CCW, OffsetX, YOffset+2, NameLabelWidth, 20, "Mass:", null);
		CCW->mMassEditBox = GUI_CreateEditBox(CCW, 0, OffsetX + NameLabelWidth, YOffset, EditBoxWidth, EditBoxHeight, "-1.0", null, EDITBOX_FLOAT_POSITIVE, null, null);
		YOffset += EditBoxHeight;

		YOffset += 10;
		{
			WindowDesc WD;
			WD.mParent	= CCW;
			WD.mX		= OffsetX;
			WD.mY		= YOffset;
			WD.mWidth	= 100;
			WD.mHeight	= 80;
			WD.mType	= WINDOW_NORMAL;
			WD.mStyle	= WSTYLE_BORDER;
			WD.mStyle	= WSTYLE_CLIENT_EDGES;
			WD.mStyle	= WSTYLE_STATIC_EDGES;
			CCW->mEditPos = ICE_NEW(EditPosWindow)(WD);

			WD.mX		= OffsetX + 100;
			CCW->mEditRot = ICE_NEW(EditPosWindow)(WD);

			YOffset += 74;
		}

		YOffset += 20;
		CCW->mActorsLabel = GUI_CreateLabel(CCW, OffsetX, YOffset+2, Width - OffsetX*2 - 8, 20, "Nb actors:", null);
		YOffset += 20;

		{
			ListBoxDesc LBD;
//			LBD.mType		= LISTBOX_MULTISELECTION;
			LBD.mParent		= CCW;
			LBD.mX			= OffsetX;
			LBD.mY			= YOffset;
			LBD.mWidth		= Width - LBD.mX*2 - 8;
			LBD.mHeight		= 140;
			LBD.mLabel		= "List box";
			CCW->mActors	= ICE_NEW(MyListBox)(LBD, *CCW);

			YOffset += LBD.mHeight + 4;
		}

		YOffset += 10;
		{
			struct Callbacks
			{
				static void CopyPose(IceButton& button, void* user_data)
				{
					CompoundCreateWindow* ccw = reinterpret_cast<CompoundCreateWindow*>(user_data);
					if(ccw->mSelected)
					{
						const PR GlobalPose = ccw->mPint->GetWorldTransform(ccw->mSelected);
						if(button.GetID()==0)
							ccw->SetCompoundPos(GlobalPose.mPos);
						else if(button.GetID()==1)
							ccw->SetCompoundRot(GlobalPose.mRot);
						else
							ASSERT(0);
					}
				}
			};

			ButtonDesc BD;
			BD.mID		= 0;
			BD.mParent	= CCW;
			BD.mX		= OffsetX;
			BD.mY		= YOffset;
			BD.mWidth	= 150;
			BD.mHeight	= 20;
			BD.mLabel	= "Copy pos";
			{
				IceButton* B = ICE_NEW(IceButton)(BD);
				B->SetCallback(Callbacks::CopyPose);
				B->SetUserData(CCW);
				B->SetEnabled(false);
				CCW->mButton0 = B;
			}
			YOffset += BD.mHeight;

			BD.mID		= 1;
			BD.mY		= YOffset;
			BD.mLabel	= "Copy rot";
			{
				IceButton* B = ICE_NEW(IceButton)(BD);
				B->SetCallback(Callbacks::CopyPose);
				B->SetUserData(CCW);
				B->SetEnabled(false);
				CCW->mButton1 = B;
			}
			YOffset += BD.mHeight;
		}

		YOffset += 10;

		ButtonDesc BD;
		BD.mID			= 0;
		BD.mParent		= CCW;
		BD.mX			= OffsetX;
		BD.mY			= YOffset;
		BD.mWidth		= ButtonWidth;
		BD.mHeight		= 20;
		BD.mLabel		= "OK";
		CCW->mOKButton	= ICE_NEW(CCWButton)(*CCW, BD);

		YOffset += BD.mHeight;
		BD.mID				= 1;
		BD.mY				= YOffset;
		BD.mLabel			= "Cancel";
		CCW->mCancelButton	= ICE_NEW(CCWButton)(*CCW, BD);
	}

	gCompoundCreateWindow = CCW;

	return CCW;
}

void CloseCompoundCreateGUI()
{
	DELETESINGLE(gCompoundCreateWindow);
}

void HideCompoundCreateWindow()
{
	if(!gCompoundCreateWindow)
		return;
	gCompoundCreateWindow->SetVisible(false);
	gCompoundCreateWindow->InitFrom(null, INVALID_ID, 0, null, null);	// Make sure we don't keep dangling pointers in there
}

void ShowCompoundCreateWindow(Pint* pint, udword index, udword nb, const PintActorHandle* handles, CreateCompoundCallback* callback)
{
	if(!gCompoundCreateWindow)
		return;

	gCompoundCreateWindow->InitFrom(pint, index, nb, handles, callback);

	POINT pt;
	IceCore::GetCursorPos(&pt);
	gCompoundCreateWindow->SetPosition(pt.x, pt.y);

	gCompoundCreateWindow->SetVisible(true);
}

CompoundCreateWindow* GetVisibleCompoundCreateWindow()
{
	if(!gCompoundCreateWindow || !gCompoundCreateWindow->IsVisible())
		return null;
	return gCompoundCreateWindow;
}

