///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DefaultControlInterface.h"
#include "TestScenes.h"
#include "Tool.h"
#include "ToolInterface.h"
#include "Camera.h"
#include "PintSQ.h"
#include "GUI_ActorEdit.h"
#include "GUI_JointEdit.h"
#include "GUI_CompoundEdit.h"
#include "GUI_Helpers.h"
#include "PINT_Editor.h"
#include "PEEL.h"
#include "PintVisibilityManager.h"
#include "Script.h"

extern	bool			gMenuIsVisible;
extern	PhysicsTest*	gRunningTest;
extern	bool			gUseEditor;

extern EditActorWindow* gEditActorWindow;

String gVehicleFilename;

CHECK_CONTAINER_ITEM(SelectedObject);

static SelectedObject* ReplaceSelectionWith(SelectionManager& manager, Pint* pint, udword index, PintActorHandle h)
{
	manager.DeleteSelected();
	// Add merged object to selection to give an immediate visual feedback to users (they "see" that the merge succeeded)
	return manager.AddToSelection(pint, h, index);
}

DefaultControlInterface::DefaultControlInterface() :
	mMouseX				(-1),
	mMouseY				(-1),
	mMouseX2			(-1),
	mMouseY2			(-1),
	mLeftDrag			(false),
	mRectangleSelection	(false),
	mValidateLeftDownCB	(false)
{
}

void DefaultControlInterface::Reset()
{
	mSelMan.Release();
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class ReportActorCallback : public Reporter
	{
									PREVENT_COPY(ReportActorCallback)
		public:
									ReportActorCallback(SelectionManager& manager, Pint* engine, udword index, bool ignore_hidden_objects);
		virtual						~ReportActorCallback();
		virtual	bool				ReportObject(PintActorHandle h);
				SelectionManager&	mSelMan;
				Pint*				mEngine;
				const udword		mEngineIndex;
				const bool			mIgnoreHiddenObjects;
				bool				mHideActorWindow;
	};

	ReportActorCallback::ReportActorCallback(SelectionManager& manager, Pint* engine, udword index, bool ignore_hidden_objects) :
		mSelMan				(manager),
		mEngine				(engine),
		mEngineIndex		(index),
		mIgnoreHiddenObjects(ignore_hidden_objects),
		mHideActorWindow	(false)
	{
	}

	ReportActorCallback::~ReportActorCallback()
	{
		if(mHideActorWindow)
			HideEditActorWindow();
	}

	bool ReportActorCallback::ReportObject(PintActorHandle h)
	{
		ASSERT(mEngine);
		// Ignore hidden objects
		if(mIgnoreHiddenObjects && mEngine->mVisHelper && !mEngine->mVisHelper->IsRenderable2(h))
			return true;

		SelectedObject* Selected = mSelMan.AddToSelection(mEngine, h, mEngineIndex);

		mHideActorWindow = true;
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////

bool DefaultControlInterface::LeftDownCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return true;
	mLeftDrag = false;
	mRectangleSelection = false;
	mMouseX = mouse.mMouseX;
	mMouseY = mouse.mMouseY;

	mValidateLeftDownCB = gRunningTest->LeftDownCallback(mouse);
	return true;
}

void DefaultControlInterface::LeftDragCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible || !mValidateLeftDownCB)
		return;
	mLeftDrag = true;

	const bool RectangleSelection = IsKeyPressed(VK_CONTROL);
	if(RectangleSelection)
	{
		mRectangleSelection = true;
		mMouseX2 = mouse.mMouseX;
		mMouseY2 = mouse.mMouseY;
	}
	else
	{
		gRunningTest->LeftDragCallback(mouse);

		// We can rotate the camera only if current tool doesn't already control it.
		ToolInterface* CurrentTool = GetCurrentTool();
		if(!CurrentTool || !CurrentTool->IsControllingCamera())
			RotateCamera(mouse.mRelMouseX, mouse.mRelMouseY);
	}
}

//static udword gNastyGlobal = 0;
void DefaultControlInterface::LeftUpCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible || !mValidateLeftDownCB)
		return;
//	printf("gLeftDrag: %d\n", gLeftDrag);
	gRunningTest->LeftUpCallback(mouse);

	if(mRectangleSelection)
	{
		mRectangleSelection = false;

		// This stuff taken directly from ICE
		{
			// Create final selection rectangle
			ScreenRect rect;
			rect.mLeft		= MIN(mMouseX, mMouseX2);
			rect.mRight		= MAX(mMouseX, mMouseX2);
			rect.mBottom	= MIN(mMouseY, mMouseY2);
			rect.mTop		= MAX(mMouseY, mMouseY2);

			if(!rect.IsEmpty())
			{
				//   (left, top)  --------------- (right, top)
				//        p0      |             |       p2
				//                |             |
				//                |             |
				//                |             |
				// (left, bottom) --------------- (right, bottom)
				//      p1                              p3

				const Point Origin = GetCameraPos();
				const Point p0 = ComputeWorldRay(rect.mLeft, rect.mTop);
				const Point p1 = ComputeWorldRay(rect.mLeft, rect.mBottom);
				const Point p2 = ComputeWorldRay(rect.mRight, rect.mTop);
				const Point p3 = ComputeWorldRay(rect.mRight, rect.mBottom);

				Plane PL[4];
				PL[0].Set(Origin, Origin + p1, Origin + p0);
				PL[1].Set(Origin, Origin + p0, Origin + p2);
				PL[2].Set(Origin, Origin + p2, Origin + p3);
				PL[3].Set(Origin, Origin + p3, Origin + p1);

				mSelMan.CancelSelection();

				const udword NbEngines = GetNbEngines();
				for(udword i=0;i<NbEngines;i++)
				{
					if(ValidateEngine(i))
					{
						//gNastyGlobal = i;
						Pint* P = GetEngine(i);
						Pint_Scene* API = P->GetSceneAPI();
						if(API)
							API->Cull(4, PL, ReportActorCallback(mSelMan, P, i, true));
						else
							ASSERT(0);
					}
				}
			}
		}
	}
	else
	{
		if(!mLeftDrag)
		{
			const int x = mouse.mMouseX;
			const int y = mouse.mMouseY;
			const Point Dir = ComputeWorldRay(x, y);
			const Point Origin = GetCameraPos();

			const bool MultiSelection = IsKeyPressed(VK_CONTROL);
			if(!MultiSelection)
			{
				mSelMan.CancelSelection();
	//			HideEditActorWindow();
			}
			else
				HideEditActorWindow();

			const udword NbEngines = GetNbEngines();
			for(udword i=0;i<NbEngines;i++)
			{
				if(ValidateEngine(i))
				{
					Pint* P = GetEngine(i);

					PintRaycastHit h;

					PintRaycastData tmp;
					tmp.mOrigin		= Origin;
					tmp.mDir		= Dir;
					tmp.mMaxDist	= 5000.0f;
					if(P->BatchRaycasts(P->mSQHelper->GetThreadContext(), 1, &h, &tmp))
					{
	//					printf("Selected object: 0x%x (%s)\n", size_t(h.mObject), P->GetName());
	//					mSelection.AddUnique(udword(h.mObject));

						// If picked object is already in selection, remove it from the selection.
/*						bool Found = false;
						const udword NbSelected = mSelMan.GetNbSelected();
						SelectedObject* Selected = mSelMan.GetSelection();
						for(udword j=0;j<NbSelected;j++)
						{
							if(Selected[j].mEngine==P && Selected[j].mActor==h.mTouchedActor)
							{
								Found = true;
								if(1)
								{
									mSelMan.RemoveSelected(j);
								}
								break;
							}
						}*/
						if(!mSelMan.RemoveFromSelection(P, h.mTouchedActor))
						// Else add it to current selection
//						if(!Found)
						{
							SelectedObject* Selected = mSelMan.AddToSelection(P, h.mTouchedActor, i);

	//						HideEditActorWindow();

							if(Selected)
							{
								if(gEditActorWindow && gEditActorWindow->IsVisible())
									gEditActorWindow->InitFrom(Selected->mEngine, Selected->mActor);
							}
							else
								HideEditActorWindow();
						}
					}
					else
					{
						HideEditActorWindow();
					}
				}
			}
		}
	}
}

#ifdef REMOVED
bool DefaultControlInterface::ReportObject(PintActorHandle h)
{
	Pint* P = GetEngine(gNastyGlobal);

//	P->mvi

	// TODO: refactor this bit with the regular picking path
	SelectedObject* Selected = AddToSelection(P, h, gNastyGlobal);

	HideEditActorWindow();
/*	if(Selected && gEditActorWindow && gEditActorWindow->IsVisible())
	{
		gEditActorWindow->InitFrom(Selected->mEngine, Selected->mActor);
	}*/
	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool DefaultControlInterface::MiddleDownCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return true;
	mMouseX = mouse.mMouseX;
	mMouseY = mouse.mMouseY;
//	if(IceCore::GetFocus()==gWindowHandle)
//		printf("We have focus\n");

//	if(IsKeyPressed(VK_CONTROL)/* && gMouseX==mouse.mMouseX && gMouseY==mouse.mMouseY*/)
//		DoPopupMenu();
//	else
		gRunningTest->MiddleDownCallback(mouse);
	return true;
}

void DefaultControlInterface::MiddleDragCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return;
	gRunningTest->MiddleDragCallback(mouse);
}

void DefaultControlInterface::MiddleUpCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return;

	if(0 && mMouseX==mouse.mMouseX && mMouseY==mouse.mMouseY)
		DoPopupMenu(mouse);

	gRunningTest->MiddleUpCallback(mouse);
}

///////////////////////////////////////////////////////////////////////////////

bool DefaultControlInterface::RightDownCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return true;
	mMouseX = mouse.mMouseX;
	mMouseY = mouse.mMouseY;

	if(IsKeyPressed(VK_CONTROL)/* && gMouseX==mouse.mMouseX && gMouseY==mouse.mMouseY*/)
		DoPopupMenu(mouse);
	else
	{
		gRunningTest->RightDownCallback(mouse);

		ToolInterface* CurrentTool = GetCurrentTool();
		if(CurrentTool)
		{
			CurrentTool->SetMouseData(mouse);

			const udword NbEngines = GetNbEngines();
			for(udword i=0;i<NbEngines;i++)
			{
				if(ValidateEngine(i))
					CurrentTool->RightDownCallback(*GetEngine(i), i);
			}
		}
	}
	return true;
}

void DefaultControlInterface::RightDragCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return;
	gRunningTest->RightDragCallback(mouse);

	ToolInterface* CurrentTool = GetCurrentTool();
	if(CurrentTool)
	{
		CurrentTool->SetMouseData(mouse);

		const udword NbEngines = GetNbEngines();
		for(udword i=0;i<NbEngines;i++)
		{
			if(ValidateEngine(i))
				CurrentTool->RightDragCallback(*GetEngine(i), i);
		}
	}
}

void DefaultControlInterface::RightUpCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return;
	gRunningTest->RightUpCallback(mouse);

	ToolInterface* CurrentTool = GetCurrentTool();
	if(CurrentTool)
	{
//		CurrentTool->SetMouseData(mouse);

		const udword NbEngines = GetNbEngines();
		for(udword i=0;i<NbEngines;i++)
		{
			if(ValidateEngine(i))
				CurrentTool->RightUpCallback(*GetEngine(i), i);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::LeftDblClkCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible || !mValidateLeftDownCB)
		return;

	ShowEditActorWindow(/*mouse*/);
}

void DefaultControlInterface::MiddleDblClkCallback(const MouseInfo& mouse)
{
}

void DefaultControlInterface::RightDblClkCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return;
	gRunningTest->RightDblClkCallback(mouse);

	ToolInterface* CurrentTool = GetCurrentTool();
	if(CurrentTool)
	{
		const udword NbEngines = GetNbEngines();
		for(udword i=0;i<NbEngines;i++)
		{
			if(ValidateEngine(i))
				CurrentTool->RightDblClkCallback(*GetEngine(i), i);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::MouseMoveCallback(const MouseInfo& mouse)
{
	if(gMenuIsVisible)
		return;
	gRunningTest->MouseMoveCallback(mouse);

	{
		ToolInterface* CurrentTool = GetCurrentTool();
		if(CurrentTool)
			CurrentTool->MouseMoveCallback(mouse);
	}
}

///////////////////////////////////////////////////////////////////////////////

bool DefaultControlInterface::KeyboardCallback(unsigned char key, int x, int y, bool down)
{
//	printf("%d\n", key);

	if(key==127)
		mSelMan.DeleteSelected();

	//if(gUseEditor)
	{
//		printf("%d\n", IsKeyPressed(VK_CONTROL));
		//### this doesn't work, looks like a bug in Glut, the modifier is correct but they key isn't anymore!
//		const int Modifiers = glutGetModifiers();
//		if(Modifiers & GLUT_ACTIVE_CTRL)
		//### this doesn't work for the same reason, IsKeyPressed returns true but the key isn't 'a' anymore (it's 1 for some reason)
		if(IsKeyPressed(VK_CONTROL))
		{
			//if(key=='a' || key=='A')
			if(key==1)
			{
				SelectAll(true);
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::OnButtonEvent(udword button_id, bool down)
{
	if(gMenuIsVisible)
		return;
	if(gRunningTest)
		gRunningTest->OnButtonEvent(button_id, down);
}

void DefaultControlInterface::OnAnalogButtonEvent(udword button_id, ubyte old_value, ubyte new_value)
{
	if(gMenuIsVisible)
		return;
	if(gRunningTest)
		gRunningTest->OnAnalogButtonEvent(button_id, old_value, new_value);
}

void DefaultControlInterface::OnAxisEvent(udword axis_id, float value)
{
	if(gMenuIsVisible)
	{
		if(0)
		{
			const udword Key = value>0.0f ? GLUTX_KEY_UP : GLUTX_KEY_DOWN;
			const udword Nb = udword(fabsf(value));

			void MoveTestSelection(udword key, udword nb);
			MoveTestSelection(Key, Nb);
		}
		return;
	}
	if(gRunningTest)
		gRunningTest->OnAxisEvent(axis_id, value);
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::RenderCallback()
{
	if(gMenuIsVisible)
		return;
	gRunningTest->RenderCallback();

	// TODO: keep a bool copy of gMouseInfo.mRightMouseButton to early exit
//	if(gMouseInfo.mRightMouseButton)
	{
		ToolInterface* CurrentTool = GetCurrentTool();
		if(CurrentTool)
		{
			CurrentTool->PreRenderCallback();
			const udword NbEngines = GetNbEngines();
			for(udword i=0;i<NbEngines;i++)
			{
				if(ValidateEngine(i))
					CurrentTool->RenderCallback(*GetEngine(i), i);
			}
			CurrentTool->PostRenderCallback();
		}
	}

	{
		const Point Color(1.0f, 1.0f, 0.0f);
		const udword NbSelected = mSelMan.GetNbSelected();
		const SelectedObject* Selected = mSelMan.GetSelection();
		for(udword i=0;i<NbSelected;i++)
		{
			if(ValidateEngine(Selected->mEngineIndex))
				DrawBoxCorners(*Selected->mEngine, Selected->mActor, Color);

			Selected++;
		}
	}

	if(mRectangleSelection)
		SetRectangleSelection(mMouseX, mMouseY, mMouseX2, mMouseY2);
	else
		SetRectangleSelection(0, 0, 0, 0);

	CompoundCreateWindow* CCW = GetVisibleCompoundCreateWindow();
	if(CCW)
		CCW->Render();
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::SelectAll(bool ignore_hidden_objects)
{
	mSelMan.CancelSelection();

	if(1)
	{
		const udword NbEngines = GetNbEngines();
		for(udword i=0;i<NbEngines;i++)
		{
			if(ValidateEngine(i))
			{
				Pint* P = GetEngine(i);
				Pint_Scene* API = P->GetSceneAPI();
				if(API)
					API->GetActors(ReportActorCallback(mSelMan, P, i, ignore_hidden_objects));
				else
					ASSERT(0);
			}
		}
	}
/*	else
	{
		// This codepath only works with editor
		PtrContainer AllActors;
		Editor_GetAllActors(AllActors);

		const udword NbEngines = GetNbEngines();
		for(udword i=0;i<NbEngines;i++)
		{
			if(ValidateEngine(i))
			{
				Pint* P = GetEngine(i);
				if(P==GetEditorPlugin()->GetPint())
				{
					for(udword j=0;j<AllActors.GetNbEntries();j++)
					{
						SelectedObject* Selected = AddToSelection(P, PintActorHandle(AllActors.GetEntry(j)), i);
					}
				}
			}
		}
	}*/

	HideEditActorWindow();
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class HideUnselectedCallback : public Reporter
	{
									PREVENT_COPY(HideUnselectedCallback)
		public:
									HideUnselectedCallback(SelectionManager& manager, Pint* engine);
		virtual						~HideUnselectedCallback();
		virtual	bool				ReportObject(PintActorHandle h);
				SelectionManager&	mSelMan;
				Pint*				mEngine;
				bool				mNeedsTranslation;
	};

	HideUnselectedCallback::HideUnselectedCallback(SelectionManager& manager, Pint* engine) :
		mSelMan				(manager),
		mEngine				(engine)
	{
		mNeedsTranslation = IsEditor(engine);
	}

	HideUnselectedCallback::~HideUnselectedCallback()
	{
	}

	bool HideUnselectedCallback::ReportObject(PintActorHandle h)
	{
		ASSERT(mEngine);
		ASSERT(mEngine->mVisHelper);

		PintActorHandle h0 = h;

		if(mNeedsTranslation)
			h = Editor_GetNativeHandle(h);

		const bool IsSelected = mSelMan.IsSelectedNative(mEngine, h);
		if(!IsSelected)
		{
			mEngine->mVisHelper->SetRenderable(h0, false);
		}
		return true;
	}
}

void DefaultControlInterface::HideAll()
{
	SelectAll(false);
	SetSelectionVisibility(false);
}

void DefaultControlInterface::HideUnselected()
{
	const udword NbEngines = GetNbEngines();
	for(udword i=0;i<NbEngines;i++)
	{
		if(ValidateEngine(i))
		{
			Pint* P = GetEngine(i);
			Pint_Scene* API = P->GetSceneAPI();
			if(API)
				API->GetActors(HideUnselectedCallback(mSelMan, P));
			else
				ASSERT(0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	// TODO: refactor with ReportActorCallback ?
	class SelectHiddenCallback : public Reporter
	{
									PREVENT_COPY(SelectHiddenCallback)
		public:
									SelectHiddenCallback(SelectionManager& manager, Pint* engine, udword index);
		virtual						~SelectHiddenCallback();
		virtual	bool				ReportObject(PintActorHandle h);
				SelectionManager&	mSelMan;
				Pint*				mEngine;
				const udword		mEngineIndex;
				bool				mHideActorWindow;
	};

	SelectHiddenCallback::SelectHiddenCallback(SelectionManager& manager, Pint* engine, udword index) :
		mSelMan				(manager),
		mEngine				(engine),
		mEngineIndex		(index),
		mHideActorWindow	(false)
	{
	}

	SelectHiddenCallback::~SelectHiddenCallback()
	{
		if(mHideActorWindow)
			HideEditActorWindow();
	}

	bool SelectHiddenCallback::ReportObject(PintActorHandle h)
	{
		ASSERT(mEngine);
		// Ignore visible objects
		if(mEngine->mVisHelper && mEngine->mVisHelper->IsRenderable2(h))
			return true;

		SelectedObject* Selected = mSelMan.AddToSelection(mEngine, h, mEngineIndex);

		mHideActorWindow = true;
		return true;
	}
}

void DefaultControlInterface::SelectHidden()
{
	mSelMan.CancelSelection();

	// TODO: refactor this loop
	const udword NbEngines = GetNbEngines();
	for(udword i=0;i<NbEngines;i++)
	{
		if(ValidateEngine(i))
		{
			Pint* P = GetEngine(i);
			Pint_Scene* API = P->GetSceneAPI();
			if(API)
				API->GetActors(SelectHiddenCallback(mSelMan, P, i));
			else
				ASSERT(0);
		}
	}

	HideEditActorWindow();
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class ShowAllCallback : public Reporter
	{
		public:
									ShowAllCallback(Pint* engine);
		virtual						~ShowAllCallback();
		virtual	bool				ReportObject(PintActorHandle h);
				Pint*				mEngine;
	};

	ShowAllCallback::ShowAllCallback(Pint* engine) :
		mEngine				(engine)
	{
	}

	ShowAllCallback::~ShowAllCallback()
	{
	}

	bool ShowAllCallback::ReportObject(PintActorHandle h)
	{
		ASSERT(mEngine);
		ASSERT(mEngine->mVisHelper);
		mEngine->mVisHelper->SetRenderable(h, true);
		return true;
	}
}

void DefaultControlInterface::ShowAll()
{
	const udword NbEngines = GetNbEngines();
	for(udword i=0;i<NbEngines;i++)
	{
		if(ValidateEngine(i))
		{
			Pint* P = GetEngine(i);
			Pint_Scene* API = P->GetSceneAPI();
			if(API)
				API->GetActors(ShowAllCallback(P));
			else
				ASSERT(0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::GetSelectedEditorObjects(PtrContainer& objects)
{
	udword NbSelected = mSelMan.GetNbSelected();
	SelectedObject* Selected = mSelMan.GetSelection();
	while(NbSelected--)
	{
		ASSERT(IsEditor(Selected->mEngine));
		objects.AddPtr(Selected->mActor);
		Selected++;
	}
}

///////////////////////////////////////////////////////////////////////////////

Pint* DefaultControlInterface::CanMergeSelected(udword& index) const
{
	Pint* P = null;
	udword Index = INVALID_ID;

	udword NbSelected = mSelMan.GetNbSelected();
	const SelectedObject* Selected = mSelMan.GetSelection();
	while(NbSelected--)
	{
		if(!P)
		{
			P = Selected->mEngine;
			Index = Selected->mEngineIndex;
		}
		else
		{
			if(P!=Selected->mEngine || Index!=Selected->mEngineIndex)
				return null;
		}
		Selected++;
	}
	index = Index;
	return P;
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::ShowEditActorWindow(/*const MouseInfo& mouse*/)
{
	if(gEditActorWindow)
	{
		const udword NbSelected = mSelMan.GetNbSelected();
		SelectedObject* Selected = mSelMan.GetSelection();
		if(NbSelected==1 && Selected)
		{
/*			gEditActorWindow->InitFrom(Selected->mEngine, Selected->mActor);

			// Passed mouse coords are local to the render window so we do this instead
			POINT pt;
			IceCore::GetCursorPos(&pt);
			gEditActorWindow->SetPosition(pt.x, pt.y);
			//gEditActorWindow->SetPosition(mouse.mMouseX, mouse.mMouseY);

			gEditActorWindow->SetVisible(true);*/
			::ShowEditActorWindow(Selected->mEngine, Selected->mActor);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::SetSelectionVisibility(bool vis)
{
	const udword NbSelected = mSelMan.GetNbSelected();
	const SelectedObject* Selected = mSelMan.GetSelection();
	for(udword i=0;i<NbSelected;i++)
	{
//		VisibilityManager* VisMan = GetVisHelper(Selected->mEngineIndex);
		VisibilityManager* VisMan = Selected->mEngine->mVisHelper;
		ASSERT(VisMan);
		ASSERT(VisMan->GetOwner()==Selected->mEngine);
		VisMan->SetRenderable(Selected->mActor, vis);
		Selected++;
	}
	// ###
//	mSelMan.CancelSelection();
}

void DefaultControlInterface::SetSelectionEnabled(bool enabled)
{
	const udword NbSelected = mSelMan.GetNbSelected();
	const SelectedObject* Selected = mSelMan.GetSelection();
	for(udword i=0;i<NbSelected;i++)
	{
		Pint_Actor* API = Selected->mEngine->GetActorAPI();
		if(API)
			API->SetSimulationFlag(Selected->mActor, enabled);
		Selected++;
	}
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::CreateCompound(CompoundCreateWindow& ccw)
{
	String CompoundName;
	if(ccw.mNameEditBox)
		ccw.mNameEditBox->GetTextAsString(CompoundName);

	const float Mass = GetFloat(-1.0f, ccw.mMassEditBox);

	PintActorHandle h = Editor_CreateCompound(ccw.mHandles.GetNbEntries(), (PintActorHandle*)ccw.mHandles.GetEntries(), MergeParams(CompoundName, Mass, &ccw.mCompoundPR));
/*	if(1)
	{
		mSelMan.DeleteSelected();
		// Add merged object to selection to give an immediate visual feedback to users (they "see" that the merge succeeded)
		SelectedObject* Selected = mSelMan.AddToSelection(ccw.mPint, h, ccw.mIndex);
	}*/
	ReplaceSelectionWith(mSelMan, ccw.mPint, ccw.mIndex, h);
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::CreateJoints()
{
	ShowEditJointWindow();
}

///////////////////////////////////////////////////////////////////////////////

void DefaultControlInterface::DoPopupMenu(const MouseInfo& mouse)
{
	if(0)
	{
		HMENU Menu = StartPopupMenu();
		const udword NbTools = GetNbTools();
		const ToolIndex CurrentToolIndex = GetCurrentToolIndex();
		for(udword i=0;i<NbTools;i++)
			AddPopupMenuEntry(Menu, GetToolName(i), i, i==CurrentToolIndex, false);	
		AddPopupMenuEntry(Menu, "Cancel", NbTools, false, false);
		const udword Choice = DisplayPopupMenu(GetWindowHandle(), Menu, mMouseX, mMouseY) - 1;
//		printf("Choice: %d\n", Choice);
		DestroyPopupMenu(Menu);
		if(Choice!=CurrentToolIndex)
		{
			SelectTool(Choice);
			UpdateToolComboBox();
		}
	}
	else
	{
		enum Action
		{
			SELECTION_ALL				= 128,
			SELECTION_DELETE			= 129,
			SELECTION_EDIT				= 130,
			SELECTION_STATIC			= 131,
			SELECTION_DYNAMIC			= 132,
			SELECTION_RESET_POSES		= 133,
			SELECTION_MERGE_COMPOUND	= 134,
			SELECTION_MERGE_COMPOUND2	= 135,
			SELECTION_MERGE_TO_CONVEX	= 136,
			SELECTION_CANCEL			= 137,
			SELECTION_HIDE				= 138,
			SELECTION_SHOW				= 139,
			SELECTION_DISABLE			= 140,
			SELECTION_ENABLE			= 141,

			COLLISION_SPHERE			= 150,
			COLLISION_BOX				= 151,
			COLLISION_CONVEX			= 152,
			COLLISION_MESH				= 153,
//#ifdef TOSEE
			COLLISION_CONVEX_DECOMP		= COLLISION_MESH,
//			COLLISION_CONVEX_DECOMP		= 154,
//#endif

			HIDE_ALL					= 200,
			HIDE_UNSELECTED				= 201,
			SELECT_HIDDEN				= 202,
			SHOW_ALL					= 203,

			CREATE_JOINTS				= 204,

			TEST_VEHICLE				= 205,

			CANCEL						= 256
		};

		const udword NbTools = GetNbTools();
		const ToolIndex CurrentToolIndex = GetCurrentToolIndex();

		const bool NothingSelected = !mSelMan.HasSelection();
		const udword NbSelected = mSelMan.GetNbSelected();

		HMENU Menu = StartPopupMenu();

		HMENU ToolSubMenu = StartPopupMenu();
		{
			for(udword i=0;i<NbTools;i++)
				AddPopupMenuEntry(ToolSubMenu, GetToolName(i), i, i==CurrentToolIndex, false);

			IceCore::AppendMenu(Menu, MF_POPUP, (UINT_PTR)ToolSubMenu, "Tool");
		}

		HMENU CollisionSubMenu = StartPopupMenu();
		{
			const bool Disabled = NothingSelected || !gUseEditor;
//			const bool Disabled = !gUseEditor;
			AddPopupMenuEntry(CollisionSubMenu, "Sphere", COLLISION_SPHERE, false, Disabled);
			AddPopupMenuEntry(CollisionSubMenu, "Box", COLLISION_BOX, false, Disabled);
			AddPopupMenuEntry(CollisionSubMenu, "Convex", COLLISION_CONVEX, false, Disabled);
			AddPopupMenuEntry(CollisionSubMenu, "Mesh", COLLISION_MESH, false, Disabled);
#ifdef TOSEE
			AddPopupMenuEntry(CollisionSubMenu, "Convex decomposition", COLLISION_CONVEX_DECOMP, false, Disabled);
#endif
			//IceCore::AppendMenu(Menu, MF_POPUP, (UINT_PTR)CollisionSubMenu, "Set collision shape");
		}

		HMENU SelectionSubMenu = StartPopupMenu();
		{
			IceCore::AppendMenu(SelectionSubMenu, MF_POPUP, (UINT_PTR)CollisionSubMenu, "Set collision shape");
			AddPopupMenuEntry(SelectionSubMenu, "Make static", SELECTION_STATIC, false, !gUseEditor || !NbSelected);
			AddPopupMenuEntry(SelectionSubMenu, "Make dynamic", SELECTION_DYNAMIC, false, !gUseEditor || !NbSelected);
			AddPopupMenuEntry(SelectionSubMenu, "Reset poses", SELECTION_RESET_POSES, false, !gUseEditor || !NbSelected);
			AddPopupMenuEntry(SelectionSubMenu, "Merge to compound", SELECTION_MERGE_COMPOUND, false, !gUseEditor || NbSelected<2);
			AddPopupMenuEntry(SelectionSubMenu, "Merge to compound Ex", SELECTION_MERGE_COMPOUND2, false, !gUseEditor || NbSelected<2);
			AddPopupMenuEntry(SelectionSubMenu, "Merge to single convex", SELECTION_MERGE_TO_CONVEX, false, !gUseEditor || NbSelected<2);
			IceCore::AppendMenu(Menu, MF_POPUP, (UINT_PTR)SelectionSubMenu, "Editor");
		}

		IceCore::AppendMenu(Menu, MF_SEPARATOR, 0, 0);
		AddPopupMenuEntry(Menu, "Hide all", HIDE_ALL, false, false);
		AddPopupMenuEntry(Menu, "Show all", SHOW_ALL, false, false);
		AddPopupMenuEntry(Menu, "Select all   (Ctrl+A)", SELECTION_ALL, false, false);
		IceCore::AppendMenu(Menu, MF_SEPARATOR, 0, 0);
		AddPopupMenuEntry(Menu, "Hide selected (H)", SELECTION_HIDE, false, NothingSelected);
		AddPopupMenuEntry(Menu, "Show selected (J)", SELECTION_SHOW, false, NothingSelected);
		AddPopupMenuEntry(Menu, "Hide unselected", HIDE_UNSELECTED, false, NothingSelected);
		AddPopupMenuEntry(Menu, "Select hidden", SELECT_HIDDEN, false, false/*NothingHidden*/);		
		AddPopupMenuEntry(Menu, "Disable selected", SELECTION_DISABLE, false, NothingSelected);
		AddPopupMenuEntry(Menu, "Enable selected", SELECTION_ENABLE, false, NothingSelected);
		AddPopupMenuEntry(Menu, _F("Delete %d selected object(s) (Del)", NbSelected), SELECTION_DELETE, false, NothingSelected);
		AddPopupMenuEntry(Menu, "Edit selected (Left dbl-clk on object)", SELECTION_EDIT, false, NbSelected!=1);
		AddPopupMenuEntry(Menu, "Cancel selection", SELECTION_CANCEL, false, NothingSelected);

//		AddPopupMenuEntry(Menu, "Reset poses", 131, false, false);
//			IceCore::AppendMenu(Menu, MF_SEPARATOR, 0, 0);
//			AddPopupMenuEntry(Menu, "Create joints", CREATE_JOINTS, false, false);

		IceCore::AppendMenu(Menu, MF_SEPARATOR, 0, 0);
		AddPopupMenuEntry(Menu, "Test vehicle", TEST_VEHICLE, false, !gUseEditor);

		IceCore::AppendMenu(Menu, MF_SEPARATOR, 0, 0);
		AddPopupMenuEntry(Menu, "Cancel", CANCEL, false, false);

		const udword Choice = DisplayPopupMenu(GetWindowHandle(), Menu, mMouseX, mMouseY) - 1;
//		printf("Choice: %d\n", Choice);
		DestroyPopupMenu(SelectionSubMenu);
		DestroyPopupMenu(CollisionSubMenu);
		DestroyPopupMenu(ToolSubMenu);
		DestroyPopupMenu(Menu);
		if(Choice<NbTools)
		{
			if(Choice!=CurrentToolIndex)
			{
				SelectTool(Choice);
				UpdateToolComboBox();
			}
		}
		else if(Choice==SELECTION_CANCEL)
		{
			mSelMan.CancelSelection();
			HideEditActorWindow();
		}
		else if(Choice==SELECTION_ALL)
		{
			SelectAll(true);
		}
		else if(Choice==SELECTION_DELETE)
		{
			mSelMan.DeleteSelected();
		}
		else if(Choice==SELECTION_EDIT)
		{
			ShowEditActorWindow(/*mouse*/);
		}
		else if(Choice==SELECTION_HIDE)
		{
			SetSelectionVisibility(false);
		}
		else if(Choice==SELECTION_SHOW)
		{
			SetSelectionVisibility(true);
		}
		else if(Choice==SELECTION_DISABLE)
		{
			SetSelectionEnabled(false);
		}
		else if(Choice==SELECTION_ENABLE)
		{
			SetSelectionEnabled(true);
		}
		else if(Choice==HIDE_UNSELECTED)
		{
			HideUnselected();
		}
		else if(Choice==SELECT_HIDDEN)
		{
			SelectHidden();
		}		
		else if(Choice==HIDE_ALL)
		{
			HideAll();

/*			if(0)
			{
				PtrContainer EditorObjects;
				GetSelectedEditorObjects(EditorObjects);
				Editor_HideActors(EditorObjects.GetNbEntries(), (PintActorHandle*)EditorObjects.GetEntries());
			}
			else
			{
				const udword NbSelected = GetNbSelected();
				const SelectedObject* Selected = GetSelection();
				for(udword i=0;i<NbSelected;i++)
				{
					VisibilityManager* VisMan = GetVisHelper(Selected->mEngineIndex);
					ASSERT(VisMan);
					ASSERT(VisMan->GetOwner()==Selected->mEngine);
					VisMan->SetRenderable(Selected->mActor, false);
					Selected++;
				}
				//mSelMan.CancelSelection();
				//HideEditActorWindow();
			}*/

//			mSelMan.CancelSelection();
//			HideEditActorWindow();
		}
		else if(Choice==SHOW_ALL)
		{
//			SelectAll(false);
//			SetSelectionVisibility(true);
			ShowAll();
		}
		else if(Choice==CREATE_JOINTS)
		{
			CreateJoints();
		}
		else if(Choice==SELECTION_RESET_POSES)
		{
			PtrContainer EditorObjects;
			GetSelectedEditorObjects(EditorObjects);
			Editor_ResetPoses(EditorObjects.GetNbEntries(), (PintActorHandle*)EditorObjects.GetEntries());

//			mSelMan.CancelSelection();
//			HideEditActorWindow();
		}
		else if(Choice==SELECTION_MERGE_COMPOUND)
		{
			udword Index;
			Pint* P = CanMergeSelected(Index);
			if(P)
			{
				PtrContainer EditorObjects;
				GetSelectedEditorObjects(EditorObjects);
				PintActorHandle h = Editor_CreateCompound(EditorObjects.GetNbEntries(), (PintActorHandle*)EditorObjects.GetEntries(), MergeParams());
/*				if(1)
				{
				mSelMan.DeleteSelected();
				// Add merged object to selection to give an immediate visual feedback to users (they "see" that the merge succeeded)
				SelectedObject* Selected = mSelMan.AddToSelection(P, h, Index);
				}*/
				ReplaceSelectionWith(mSelMan, P, Index, h);
			}
			else
				IceCore::MessageBoxA(0, "Cannot merge objects from different engines.", "Error", MB_OK);
		}		
		else if(Choice==SELECTION_MERGE_COMPOUND2)
		{
			udword Index;
			Pint* P = CanMergeSelected(Index);
			if(P)
			{
				PtrContainer EditorObjects;
				GetSelectedEditorObjects(EditorObjects);
				ShowCompoundCreateWindow(P, Index, EditorObjects.GetNbEntries(), (PintActorHandle*)EditorObjects.GetEntries(), this);
			}
			else
				IceCore::MessageBoxA(0, "Cannot merge objects from different engines.", "Error", MB_OK);
		}		
		else if(Choice==SELECTION_MERGE_TO_CONVEX)
		{
			udword Index;
			Pint* P = CanMergeSelected(Index);
			if(P)
			{
				PtrContainer EditorObjects;
				GetSelectedEditorObjects(EditorObjects);
				PintActorHandle h = Editor_CreateSingle(EditorObjects.GetNbEntries(), (PintActorHandle*)EditorObjects.GetEntries());
/*				if(1)
				{
				mSelMan.DeleteSelected();
				// Add merged object to selection to give an immediate visual feedback to users (they "see" that the merge succeeded)
				SelectedObject* Selected = mSelMan.AddToSelection(P, h, Index);
				}*/
				ReplaceSelectionWith(mSelMan, P, Index, h);
			}
			else
				IceCore::MessageBoxA(0, "Cannot merge objects from different engines.", "Error", MB_OK);
		}
		else if(Choice==SELECTION_STATIC)
		{
			class MakeStatic : public ModifyInterface
			{
				public:
				virtual	PintActorHandle	ModifySelected(PintActorHandle h)	{ return Editor_MakeStatic(h);	}
			}MI;

			mSelMan.ModifySelected(MI);
		}
		else if(Choice==SELECTION_DYNAMIC)
		{
			class MakeDynamic : public ModifyInterface
			{
				public:
				virtual	PintActorHandle	ModifySelected(PintActorHandle h)	{ return Editor_MakeDynamic(h);	}
			}MI;

			mSelMan.ModifySelected(MI);
		}
		else if(Choice>=COLLISION_SPHERE && Choice<=COLLISION_CONVEX_DECOMP)
		{
			class MakeCollision : public ModifyInterface
			{
				udword	mChoice;
				public:
					MakeCollision(udword choice) : mChoice(choice)	{}
				virtual	PintActorHandle	ModifySelected(PintActorHandle h)
				{
					if(mChoice==COLLISION_SPHERE)
						return Editor_MakeSphere(h);
					else if(mChoice==COLLISION_BOX)
						return Editor_MakeBox(h);
					else if(mChoice==COLLISION_CONVEX)
						return Editor_MakeConvex(h);
					else if(mChoice==COLLISION_MESH)
						return Editor_MakeMesh(h);
#ifdef TOSEE
					else if(mChoice==COLLISION_CONVEX_DECOMP)
						return Editor_MakeConvexDecomp(h);
#endif
					ASSERT(0);
					return null;
				}
			}MI(Choice);

			mSelMan.ModifySelected(MI);
		}
/*		else if(Choice==131)
		{
			PintPlugin*	GetEditorPlugin();
			PintPlugin*	Editor = GetEditorPlugin();
			const udword NbSelected = GetNbSelected();
			SelectedObject* Selected = GetSelection();
			for(udword i=0;i<NbSelected;i++)
			{
				//ReleaseObject(*Selected->mEngine, Selected->mActor);
				Selected++;
			}
		}*/
		else if(Choice==TEST_VEHICLE)
		{
			if(GetEditor())
			{
				ExportScene(&gVehicleFilename);

				StartTest("LegoVehicleFactory");
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
