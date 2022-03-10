///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintSelectionManager.h"
#include "PintObjectsManager.h"
#include "GUI_ActorEdit.h"
#include "PINT_Editor.h"
#include "PEEL_MenuBar.h"
#include "PEEL.h"

///////////////////////////////////////////////////////////////////////////////

SelectionManager::SelectionManager() : mSelectedNative(null)//, mLockSelection(false)
{
}

SelectionManager::~SelectionManager()
{
	Release(true);
}

void SelectionManager::Release(bool from_dtor)
{
	SetFocusActor();
	SetFocusShape();

	mSelection.Empty();
	DELETESINGLE(mSelectedNative);
	if(!from_dtor)
		UpdateSelectionItems();
}

void SelectionManager::CancelSelection()
{
	SetFocusActor();
	SetFocusShape();

	mSelection.Reset();
	if(mSelectedNative)
		mSelectedNative->clear();
	UpdateSelectionItems();
}

///////////////////////////////////////////////////////////////////////////////

SelectedObject* SelectionManager::AddToSelection(Pint* pint, PintActorHandle actor, udword index)
{
	// Filter out "default env" object, we don't want to let users edit/change this one
	if(IsDefaultEnv(*pint, actor))
		return null;

	{
		if(!mSelectedNative)
			mSelectedNative = new ps::HashSet<PintActorHandle>;

		PintActorHandle NativeActor = actor;
		if(IsEditor(pint))
			NativeActor = Editor_GetNativeHandle(NativeActor);

		const bool status = mSelectedNative->insert(NativeActor);
		ASSERT(status);
	}

	SelectedObject* Selected = ICE_RESERVE(SelectedObject, mSelection);

	Selected->mEngine		= pint;
	Selected->mActor		= actor;
	Selected->mEngineIndex	= index;
	UpdateSelectionItems();
	return Selected;
}

///////////////////////////////////////////////////////////////////////////////

bool SelectionManager::RemoveFromSelection(Pint* pint, PintActorHandle actor)
{
	bool Found = false;
	udword NbSelected = GetNbSelected();
	SelectedObject* Selected = GetSelection();
	for(udword i=0;i<NbSelected;i++)
	{
		if(Selected[i].mEngine==pint && Selected[i].mActor==actor)
		{
			{
				ASSERT(mSelectedNative);

				PintActorHandle NativeActor = actor;
				if(IsEditor(pint))
					NativeActor = Editor_GetNativeHandle(NativeActor);

				const bool status = mSelectedNative->erase(NativeActor);
				ASSERT(status);
			}

			Selected[i] = Selected[--NbSelected];
			const udword N = sizeof(SelectedObject)/sizeof(udword);
			mSelection.ForceSize(NbSelected*N);
			UpdateSelectionItems();
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void SelectionManager::DeleteSelected()
{
	const udword NbSelected = GetNbSelected();
	SelectedObject* Selected = GetSelection();
	for(udword i=0;i<NbSelected;i++)
	{
		ReleasePintObject(*Selected->mEngine, Selected->mActor, false);
		Selected++;
	}
	CancelSelection();
	UI_HideEditWindows();
}

///////////////////////////////////////////////////////////////////////////////

void SelectionManager::ModifySelected(ModifyInterface& modify)
{
	if(mSelectedNative)
		mSelectedNative->clear();

	udword NbSelected = GetNbSelected();
	SelectedObject* Selected = GetSelection();
	while(NbSelected--)
	{
		const PintActorHandle h = modify.ModifySelected(Selected->mActor);
		if(h)
		{
			ReleasePintObject(*Selected->mEngine, Selected->mActor, false);
			Selected->mActor = h;

			{
				if(!mSelectedNative)
					mSelectedNative = new ps::HashSet<PintActorHandle>;

				PintActorHandle NativeActor = h;
				if(IsEditor(Selected->mEngine))
					NativeActor = Editor_GetNativeHandle(NativeActor);

				const bool status = mSelectedNative->insert(NativeActor);
				ASSERT(status);
			}

		}
		Selected++;
	}
	UI_HideEditWindows();
}

///////////////////////////////////////////////////////////////////////////////

bool SelectionManager::IsSelectedNative(Pint* pint, PintActorHandle h) const
{
	// Warning, we're getting a native handle here.

//	udword Time;
//	StartProfile(Time);
	if(mSelectedNative)
		return mSelectedNative->contains(h);
//	EndProfile(Time);
//	printf("Time: %d\n", Time);

	udword Nb = GetNbSelected();
	const SelectedObject* Selection = GetSelection();
	while(Nb--)
	{
		if(Selection->mEngine==pint)
		{
			PintActorHandle CurrentSelectedActor = Selection->mActor;

			if(IsEditor(Selection->mEngine))
				CurrentSelectedActor = Editor_GetNativeHandle(CurrentSelectedActor);

			if(CurrentSelectedActor==h)
				return true;
		}
		Selection++;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
