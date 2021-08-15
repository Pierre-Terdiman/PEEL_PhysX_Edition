///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GUI_ActorEdit.h"
#include "Pint.h"
#include "PintShapeRenderer.h"
#include "PEEL.h"

namespace
{
	// We use a custom edit box to enable/disable other items when the edited field changes
	class NameEditBox : public IceEditBox
	{
		public:
									NameEditBox(EditActorWindow& owner, const EditBoxDesc& desc) : IceEditBox(desc), mOwner(owner)	{}
		virtual						~NameEditBox()																					{}

		virtual	bool				FilterKey(udword key)			const
									{
										//printf("Key: %d\n", key);
										
										if(mOwner.mOKButton)
											mOwner.mOKButton->SetEnabled(true);

										mOwner.mSomethingChanged = true;

										return IceEditBox::FilterKey(key);
									}

				EditActorWindow&	mOwner;
	};

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
											mOwner.mSomethingChanged = false;

										mOwner.SetVisible(false);
									}

				EditActorWindow&	mOwner;
//				bool				mSaveOnClick;
	};
}

EditActorWindow::EditActorWindow(const WindowDesc& desc) :
	IceWindow			(desc),
	mNameEditBox		(null),
	mOKButton			(null),
	mCancelButton		(null),
	mNameLabel			(null),
	mShapesLabel		(null),
	mMassLabel			(null),
	mInfoLabel			(null),
	mShapes				(null),
	mPint				(null),
	mHandle				(null),
	mSomethingChanged	(false)
{
}

EditActorWindow::~EditActorWindow()
{
	DELETESINGLE(mShapes);
	DELETESINGLE(mInfoLabel);
	DELETESINGLE(mMassLabel);
	DELETESINGLE(mShapesLabel);
	DELETESINGLE(mNameLabel);
	DELETESINGLE(mCancelButton);
	DELETESINGLE(mOKButton);
	DELETESINGLE(mNameEditBox);
}

void EditActorWindow::AskUserForSave()
{
	if(mSomethingChanged)
	{
		mSomethingChanged = false;

		int ret = IceCore::MessageBoxA(null, "Do you want to save your changes?", "Something changed in Edit Actor dialog", MB_OKCANCEL);
		if(ret==IDOK)
			SaveParams();
	}
}

static const char* gShapeTypeName[] = 
{
	"Undefined",
	"Sphere",
	"Capsule",
	"Cylinder",
	"Box",
	"Convex",
	"Mesh",
	"Mesh2",
};

void EditActorWindow::InitFrom(Pint* pint, PintActorHandle handle)
{
	AskUserForSave();

	Pint_Actor* ActorAPI = pint ? pint->GetActorAPI() : null;

	const char* ActorName = null;
	if(pint)
	{
		//ActorName = pint->GetName(handle);
		ActorName = ActorAPI ? ActorAPI->GetName(handle) : "(not available)";

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

	if(mNameEditBox)
		mNameEditBox->SetText(ActorName);

	///

	if(mMassLabel)
	{
		if(ActorAPI)
			mMassLabel->SetLabel(_F("Mass: %f\n", ActorAPI->GetMass(handle)));
		else
			mMassLabel->SetLabel(null);
	}

	///

	if(mShapes)
		mShapes->RemoveAll();

	if(mShapesLabel)
	{
		if(pint)
		{
			const udword NbShapes = ActorAPI ? ActorAPI->GetNbShapes(handle) : INVALID_ID;

			if(NbShapes!=INVALID_ID)
			{
				if(NbShapes==1)
					mShapesLabel->SetLabel(_F("Single actor (0x%x)", size_t(handle)));
				else
					mShapesLabel->SetLabel(_F("Compound actor - %d shapes (0x%x)", NbShapes, size_t(handle)));
			}
			else
				mShapesLabel->SetLabel("Nb shapes: (not available)");

			if(mShapes && NbShapes!=INVALID_ID)
			{
				Pint_Shape* ShapeAPI = pint->GetShapeAPI();
				if(ShapeAPI)
				{
					for(udword i=0;i<NbShapes;i++)
					{
						const PintShapeHandle sh = ActorAPI->GetShape(handle, i);
						const char* ShapeName = ShapeAPI->GetName(sh);
						if(!ShapeName)
							ShapeName = "(unnamed)";
						
						const PintShape ShapeType = ShapeAPI->GetType(sh);
						const char* ShapeTypeName = gShapeTypeName[ShapeType];

						PintShapeRenderer* PSR = ShapeAPI->GetShapeRenderer(sh);
						const char* RendererName = PSR ? PSR->GetClassName() : "(unnamed)";
						const udword NbRenderers = PSR ? PSR->GetNbRenderers() : 0;

						mShapes->Add(_F("Shape %d - %s - %s - %s (%d)", i, ShapeTypeName, ShapeName, RendererName, NbRenderers));
						// ### hardcoded IDs here aren't nice
						mShapes->SetItemData(i, sh);
					}
				}
			}
		}
		else
		{
			mShapesLabel->SetLabel(null);
		}
	}
}

void EditActorWindow::SaveParams()
{
	if(mPint && mNameEditBox)
	{
		String Buffer;
//		char Buffer[1024];
//		mNameEditBox->GetText(Buffer, 1024);
		mNameEditBox->GetTextAsString(Buffer);
		//mPint->SetName(mHandle, Buffer);
		Pint_Actor* API = mPint->GetActorAPI();
		if(API)
		{
			API->SetName(mHandle, Buffer);
//			API->SetMass(mHandle, 0.01f);
		}
		mSomethingChanged = false;
	}
}

int EditActorWindow::handleEvent(IceGUIEvent* event)
{
	if(event->mType==EVENT_CLOSE)
		AskUserForSave();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class MyListBox : public IceListBox
	{
		EditActorWindow&	mOwner;
		public:
							MyListBox(const ListBoxDesc& desc, EditActorWindow& owner) : IceListBox(desc), mOwner(owner)	{}
		virtual				~MyListBox()																					{}

		virtual	void	OnListboxEvent(ListBoxEvent event)
		{
			if(event==LBE_SELECTION_CHANGED)
			{
				const udword Selected = GetSelectedIndex();

				const PintShapeHandle sh = (const PintShapeHandle)GetItemData(Selected);

				SetFocusShape(sh);
			}
			else if(event==LBE_DBL_CLK)
			{
				SetFocusShape(null);
			}
		}
	};
}

/*static*/ EditActorWindow*	gEditActorWindow = null;

EditActorWindow* CreateActorEditGUI(IceWidget* parent, sdword x, sdword y)
{
	const udword Width = 500;
	const udword Height = 400;

	WindowDesc WD;
	WD.mParent	= parent;
	WD.mX		= x;
	WD.mY		= y;
	WD.mWidth	= Width;
	WD.mHeight	= Height;
	WD.mLabel	= "Edit actor";
	WD.mType	= WINDOW_DIALOG;
	WD.mStyle	= WSTYLE_HIDDEN;

	EditActorWindow* EAW = ICE_NEW(EditActorWindow)(WD);
//	EAW->SetVisible(true);

	sdword YOffset = 4;
	{
		const udword OffsetX = 4;

		const udword NameLabelWidth = 40;
		const udword ButtonWidth = Width-OffsetX*2-8;
		const udword EditBoxWidth = Width-OffsetX*2-8-NameLabelWidth;
		const udword EditBoxHeight = 20;

		EAW->mNameLabel = GUI_CreateLabel(EAW, OffsetX, YOffset+2, NameLabelWidth, 20, "Name:", null);

		EditBoxDesc EBD;
		EBD.mParent	= EAW;
		EBD.mX		= OffsetX + NameLabelWidth;
		EBD.mY		= YOffset;
		EBD.mWidth	= EditBoxWidth;
		EBD.mHeight	= EditBoxHeight;
		EBD.mLabel	= "Default name";
//		EBD.mID		= id;
		EBD.mType	= EDITBOX_NORMAL;
		EBD.mFilter	= EDITBOX_TEXT;

		EAW->mNameEditBox = ICE_NEW(NameEditBox)(*EAW, EBD);
		YOffset += EditBoxHeight;

		EAW->mShapesLabel = GUI_CreateLabel(EAW, OffsetX, YOffset+2, Width - OffsetX*2 - 8, 20, "Nb shapes:", null);
		YOffset += 20;

		EAW->mMassLabel = GUI_CreateLabel(EAW, OffsetX, YOffset+2, Width - OffsetX*2 - 8, 20, "Mass:", null);
		YOffset += 20;

		EAW->mInfoLabel = GUI_CreateLabel(EAW, OffsetX, YOffset+2, Width - OffsetX*2 - 8, 20, "Shape ID  -  Shape type  -  Shape name  -  Renderer name  -  Nb renderers:", null);
		YOffset += 22;

		{
			ListBoxDesc LBD;
//			LBD.mType		= LISTBOX_MULTISELECTION;
			LBD.mParent		= EAW;
			LBD.mX			= OffsetX;
			LBD.mY			= YOffset;
			LBD.mWidth		= Width - LBD.mX*2 - 8;
			LBD.mHeight		= Height - 200;
			LBD.mLabel		= "List box";
			EAW->mShapes	= ICE_NEW(MyListBox)(LBD, *EAW);

			YOffset += LBD.mHeight + 4;
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
