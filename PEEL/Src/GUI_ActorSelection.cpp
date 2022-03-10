///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_ActorSelection.h"
#include "Pint.h"
#include "PintObjectsManager.h"
#include "PintShapeRenderer.h"
#include "PEEL.h"
#include "DefaultControlInterface.h"

static const char* gNoName = "(null)";

namespace
{
	class ASWButton : public IceButton
	{
		public:
										ASWButton(ItemSelectionWindow& owner, const ButtonDesc& desc) : IceButton(desc), mOwner(owner)	{}
		virtual							~ASWButton()																					{}

		virtual	void					OnClick()
										{
											if(GetID()==0)
												mOwner.OnOK();

											mOwner.Reset();
											mOwner.SetVisible(false);
										}

				ItemSelectionWindow&	mOwner;
	};

	struct ExtraItemData
	{
		Pint*	mPint;
		udword	mEngineIndex;
	};
	CHECK_CONTAINER_ITEM(ExtraItemData)

	class ASWListBox : public IceListBox
	{
		ItemSelectionWindow&	mOwner;
		Container				mExtraItemData;
		public:
		PintActorHandle			mSelectedActor;
		PintShapeHandle			mSelectedShape;
		Pint*					mSelectedPint;

								ASWListBox(const ListBoxDesc& desc, ItemSelectionWindow& owner) : IceListBox(desc), mOwner(owner), mSelectedActor(null), mSelectedShape(null), mSelectedPint(null)	{}
		virtual					~ASWListBox()																																						{}

		void					Reset()
		{
			mSelectedActor	= null;
			mSelectedShape	= null;
			mSelectedPint	= null;
			mExtraItemData.Reset();
		}

		void					AddActor(Pint* pint, const char* actor_name, PintActorHandle h, udword id, udword engine_index)
		{
			ASSERT(mOwner.mActorCallback);

			Add(_F("Actor %d - %s (%x)", id, actor_name, h));
			SetItemData(id, h);

			ExtraItemData* EID = ICE_RESERVE(ExtraItemData, mExtraItemData);
			EID->mPint = pint;
			EID->mEngineIndex = engine_index;
		}

		virtual	void			OnListboxEvent(ListBoxEvent event)
		{
			if(event==LBE_DBL_CLK)
			{
				mOwner.OnOK();
				mOwner.Reset();
				mOwner.SetVisible(false);
			}
			else
			{
				if(mOwner.mActorCallback)
				{
					mSelectedShape = null;
					if(event==LBE_SELECTION_CHANGED)
					{
						const udword Selected = GetSelectedIndex();

						const ExtraItemData* Entries = reinterpret_cast<const ExtraItemData*>(mExtraItemData.GetEntries());
						const ExtraItemData& EID = Entries[Selected];

						const PintActorHandle ah = PintActorHandle(GetItemData(Selected));
						if(1)
						{
							SetFocusActor(ah);
							mSelectedActor = ah;
							mSelectedPint = EID.mPint;
						}
						else
						{
							GetDefaultControlInterface().mSelMan.CancelSelection();
							GetDefaultControlInterface().PickObject(EID.mPint, ah, EID.mEngineIndex);
						}
					}
				}
				else if(mOwner.mShapeCallback)
				{
					mSelectedActor = null;
					// TODO: refactor with actor edit code?
					if(event==LBE_SELECTION_CHANGED)
					{
						const udword Selected = GetSelectedIndex();
						{
							//printf("Selected shape\n");
							const PintShapeHandle sh = PintShapeHandle(GetItemData(Selected));
							SetFocusActor();
							SetFocusShape(mOwner.mActorOwner, sh);
							mSelectedShape = sh;
						}
					}
				}
			}
		}
	};
}

ItemSelectionWindow::ItemSelectionWindow(const WindowDesc& desc) :
	IceWindow		(desc),
	mActorCallback	(null),
	mShapeCallback	(null),
	mUserData		(null),
	mActorOwner		(null)
{
}

ItemSelectionWindow::~ItemSelectionWindow()
{
	DELETESINGLE(mItems);
	DELETESINGLE(mCancelButton);
	DELETESINGLE(mOKButton);
}

int ItemSelectionWindow::handleEvent(IceGUIEvent* event)
{
//	if(event->mType==EVENT_CLOSE)
//		AskUserForSave();
	return 0;
}

void ItemSelectionWindow::Reset()
{
	if(mItems)
	{
		IceListBox* LB = mItems;
		ASWListBox* MyListBox = static_cast<ASWListBox*>(LB);
		MyListBox->Reset();

		mItems->RemoveAll();
	}
}

void ItemSelectionWindow::OnOK()
{
		IceListBox* LB = mItems;
	const ASWListBox* MyListBox = static_cast<const ASWListBox*>(LB);

	if(mActorCallback)
		mActorCallback->OnSelectedActor(MyListBox->mSelectedPint, MyListBox->mSelectedActor, mUserData);

	if(mShapeCallback)
		mShapeCallback->OnSelectedShape(MyListBox->mSelectedShape, mUserData);
}


namespace
{
	class ReportActorCallback : public Reporter
	{
								PREVENT_COPY(ReportActorCallback)
		public:
								ReportActorCallback(Pint& pint, ASWListBox& list_box, udword engine_index);
		virtual					~ReportActorCallback();
		virtual	bool			ReportObject(PintActorHandle h);

				Pint&			mPint;
				ASWListBox&		mListBox;
				Pint_Actor*		mActorAPI;
				const udword	mEngineIndex;
				udword			mID;
	};

	ReportActorCallback::ReportActorCallback(Pint& pint, ASWListBox& list_box, udword engine_index) : mPint(pint), mListBox(list_box), mEngineIndex(engine_index), mID(0)
	{
		mActorAPI = pint.GetActorAPI();
	}

	ReportActorCallback::~ReportActorCallback()
	{
	}

	bool ReportActorCallback::ReportObject(PintActorHandle h)
	{
		// Filter out "default env" object
		if(IsDefaultEnv(mPint, h))
			return true;

		const char* ActorName = null;
		if(mActorAPI)
			ActorName = mActorAPI->GetName(h);

		if(!ActorName)
			ActorName = gNoName;

		mListBox.AddActor(&mPint, ActorName, h, mID++, mEngineIndex);

		return true;
	}
}

void ItemSelectionWindow::Populate()
{
	if(!mItems)
		return;

	Reset();

	const udword NbEngines = GetNbEngines();
	for(udword i=0;i<NbEngines;i++)
	{
		if(!ValidateEngine(i))
			continue;

		Pint* P = GetEngine(i);
		if(P)
		{
			Pint_Scene* API = P->GetSceneAPI();
			if(API)
			{
				IceListBox* LB = mItems;
				API->GetActors(ReportActorCallback(*P, *static_cast<ASWListBox*>(LB), i));
			}
		}
	}
}

void ItemSelectionWindow::Populate(Pint& pint, PintActorHandle handle)
{
	if(!mItems)
		return;

	Reset();

	// TODO: refactor with actor edit window

		Pint_Actor* ActorAPI = pint.GetActorAPI();
		ASSERT(ActorAPI);
		if(!ActorAPI)
			return;

		const udword NbShapes = ActorAPI->GetNbShapes(handle);

		//list_box->mNbShapes = NbShapes;

		if(NbShapes!=INVALID_ID)
		{
			Pint_Shape* ShapeAPI = pint.GetShapeAPI();
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

					mItems->Add(_F("Shape %d (0x%x) - %s - %s - %s (%d)", i, size_t(sh), ShapeTypeName, ShapeName, RendererName, NbRenderers));
					// ### hardcoded IDs here aren't nice
					mItems->SetItemData(i, sh);
				}
			}
		}
}

///////////////////////////////////////////////////////////////////////////////

/*static*/ ItemSelectionWindow*	gItemSelectionWindow = null;

ItemSelectionWindow* CreateItemSelectionWindow(IceWidget* parent, sdword x, sdword y)
{
	const udword Width = 500;
	const udword ActorListAreaSize = 200;
	const udword Height = 100 + ActorListAreaSize;

	ItemSelectionWindow* ASW;
	{
		WindowDesc WD;
		WD.mParent	= parent;
		WD.mX		= x;
		WD.mY		= y;
		WD.mWidth	= Width;
		WD.mHeight	= Height;
		WD.mLabel	= "Select item";
		WD.mType	= WINDOW_DIALOG;
		WD.mStyle	= WSTYLE_HIDDEN;

		ASW = ICE_NEW(ItemSelectionWindow)(WD);
	}

	sdword YOffset = 4;
	{
		const udword OffsetX = 4;

		const udword ButtonWidth = Width-OffsetX*2-8;

		{
			ListBoxDesc LBD;
//			LBD.mType		= LISTBOX_MULTISELECTION;
			LBD.mParent		= ASW;
			LBD.mX			= OffsetX;
			LBD.mY			= YOffset;
			LBD.mWidth		= Width - LBD.mX*2 - 8;
			LBD.mHeight		= ActorListAreaSize;
			LBD.mLabel		= "List box";
			ASW->mItems		= ICE_NEW(ASWListBox)(LBD, *ASW);

			YOffset += LBD.mHeight + 4;
		}

		ButtonDesc BD;
		BD.mID			= 0;
		BD.mParent		= ASW;
		BD.mX			= OffsetX;
		BD.mY			= YOffset;
		BD.mWidth		= ButtonWidth;
		BD.mHeight		= 20;
		BD.mLabel		= "OK";
		ASW->mOKButton	= ICE_NEW(ASWButton)(*ASW, BD);

		YOffset += BD.mHeight;
		BD.mID				= 1;
		BD.mY				= YOffset;
		BD.mLabel			= "Cancel";
		ASW->mCancelButton	= ICE_NEW(ASWButton)(*ASW, BD);
	}

	gItemSelectionWindow = ASW;

	return ASW;
}

void CloseItemSelectionWindow()
{
	DELETESINGLE(gItemSelectionWindow);
}

void HideItemSelectionWindow()
{
	if(!gItemSelectionWindow)
		return;
	gItemSelectionWindow->SetVisible(false);
	gItemSelectionWindow->Reset();
}

void ShowActorSelectionWindow(ActorSelectionCallback* callback, void* user_data)
{
	if(!gItemSelectionWindow)
		return;

	ASSERT(callback);
	gItemSelectionWindow->SetLabel("Select actor");
	gItemSelectionWindow->mActorCallback = callback;
	gItemSelectionWindow->mShapeCallback = null;
	gItemSelectionWindow->mUserData = user_data;
	gItemSelectionWindow->mActorOwner = null;
	gItemSelectionWindow->Populate();

	// Passed mouse coords are local to the render window so we do this instead
	POINT pt;
	IceCore::GetCursorPos(&pt);
	gItemSelectionWindow->SetPosition(pt.x, pt.y);
	//gEditActorWindow->SetPosition(mouse.mMouseX, mouse.mMouseY);

	gItemSelectionWindow->SetVisible(true);
}

void ShowShapeSelectionWindow(Pint& pint, PintActorHandle actor, ShapeSelectionCallback* callback, void* user_data)
{
	if(!gItemSelectionWindow)
		return;

	ASSERT(callback);
	gItemSelectionWindow->SetLabel("Select shape");
	gItemSelectionWindow->mActorCallback = null;
	gItemSelectionWindow->mShapeCallback = callback;
	gItemSelectionWindow->mUserData = user_data;
	gItemSelectionWindow->mActorOwner = actor;
	gItemSelectionWindow->Populate(pint, actor);

	// Passed mouse coords are local to the render window so we do this instead
	POINT pt;
	IceCore::GetCursorPos(&pt);
	gItemSelectionWindow->SetPosition(pt.x, pt.y);
	//gEditActorWindow->SetPosition(mouse.mMouseX, mouse.mMouseY);

	gItemSelectionWindow->SetVisible(true);
}

ItemSelectionWindow* GetVisibleItemSelectionWindow()
{
	if(!gItemSelectionWindow || !gItemSelectionWindow->IsVisible())
		return null;
	return gItemSelectionWindow;
}
