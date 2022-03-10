///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_ACTOR_SELECTION_H
#define GUI_ACTOR_SELECTION_H

#include "PintDef.h"
#include "PintGUIHelper.h"

	class Pint;

	class ActorSelectionCallback
	{
		public:
		virtual	void	OnSelectedActor(Pint* pint, PintActorHandle h, void* user_data)	= 0;
	};

	class ShapeSelectionCallback
	{
		public:
		virtual	void	OnSelectedShape(PintShapeHandle h, void* user_data)	= 0;
	};

	class ItemSelectionWindow : public IceWindow
	{
		public:
										ItemSelectionWindow(const WindowDesc& desc);
		virtual							~ItemSelectionWindow();

		virtual int						handleEvent(IceGUIEvent* event);

				void					Populate();
				void					Populate(Pint& pint, PintActorHandle actor);
				void					Reset();
				void					OnOK();

				ButtonPtr				mOKButton;
				ButtonPtr				mCancelButton;
				ListBoxPtr				mItems;

				ActorSelectionCallback*	mActorCallback;
				ShapeSelectionCallback*	mShapeCallback;

				void*					mUserData;

				PintActorHandle			mActorOwner;	// For shape selector
	};

	ItemSelectionWindow*	CreateItemSelectionWindow(IceWidget* parent, sdword x, sdword y);
	void					CloseItemSelectionWindow();

	void					ShowActorSelectionWindow(ActorSelectionCallback*, void* user_data=null);
	void					ShowShapeSelectionWindow(Pint& pint, PintActorHandle actor, ShapeSelectionCallback*, void* user_data=null);
	void					HideItemSelectionWindow();
	ItemSelectionWindow*	GetVisibleItemSelectionWindow();

#endif
