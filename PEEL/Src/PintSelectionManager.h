///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_SELECTION_MANAGER_H
#define PINT_SELECTION_MANAGER_H

#include "PintDef.h"
#include "PsHashSet.h"
namespace ps = physx::shdfnd;

	class Pint;

	struct SelectedObject
	{	
		Pint*				mEngine;
		PintActorHandle		mActor;
		udword				mEngineIndex;
	};

	class ModifyInterface
	{
		public:
		virtual	PintActorHandle	ModifySelected(PintActorHandle h) = 0;
	};

	class SelectionManager : public Allocateable
	{
		public:
												SelectionManager();
												~SelectionManager();

				void							Release(bool from_dtor=false);
				void							CancelSelection();

		inline_	bool							HasSelection()						const
												{
													return mSelection.GetNbEntries()!=0;
												}

		inline_	udword							GetNbSelected()	const
												{
													const udword N = sizeof(SelectedObject)/sizeof(udword);
													return mSelection.GetNbEntries()/N;
												}

		inline_	const SelectedObject*			GetSelection()	const
												{
													return (const SelectedObject*)mSelection.GetEntries();
												}

		inline_	SelectedObject*					GetSelection()
												{
													return (SelectedObject*)mSelection.GetEntries();
												}

				SelectedObject*					AddToSelection(Pint* pint, PintActorHandle object, udword index);
				bool							RemoveFromSelection(Pint* pint, PintActorHandle actor);

				void							DeleteSelected();

				void							ModifySelected(ModifyInterface& modify);

				bool							IsSelectedNative(Pint*, PintActorHandle native_handle)	const;

//		inline_	bool							IsSelectionLocked()	const	{ return mLockSelection;	}
//		inline_	void							LockSelection(bool b)		{ mLockSelection = b;		}

		private:
				Container						mSelection;
				ps::HashSet<PintActorHandle>*	mSelectedNative;	// Native handles in there
//				bool							mLockSelection;
	};

#endif