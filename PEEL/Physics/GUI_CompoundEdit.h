///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_COMPOUND_EDIT_H
#define GUI_COMPOUND_EDIT_H

#include "PintDef.h"
	class Pint;
	class CompoundCreateWindow;
	class EditPosWindow;

	class CreateCompoundCallback
	{
		public:
		virtual	void	CreateCompound(CompoundCreateWindow&) =	0;
	};

	class CompoundCreateWindow : public IceWindow
	{
		public:
								CompoundCreateWindow(const WindowDesc& desc);
		virtual					~CompoundCreateWindow();

		virtual int				handleEvent(IceGUIEvent* event);

				void			InitFrom(Pint* pint, udword index, udword nb, const PintActorHandle* handles, CreateCompoundCallback* callback);
				void			Render();

				void			CreateCompound();
				void			SetCompoundPos(const Point& pos);
				void			SetCompoundRot(const Quat& rot);
		//private:
		CreateCompoundCallback*	mCallback;
				EditPosWindow*	mEditPos;
				EditPosWindow*	mEditRot;
				IceEditBox*		mNameEditBox;
				IceEditBox*		mMassEditBox;
				IceButton*		mOKButton;
				IceButton*		mCancelButton;
				IceLabel*		mNameLabel;
				IceLabel*		mActorsLabel;
				IceLabel*		mMassLabel;
				IceListBox*		mActors;
				IceButton*		mButton0;
				IceButton*		mButton1;
				Pint*			mPint;
				udword			mIndex;
				PtrContainer	mHandles;	// TODO: replace this with listbox's item's userdata
				PintActorHandle	mSelected;
				PR				mCompoundPR;
		private:
				void			AskUserForSave();

		friend CompoundCreateWindow* CreateCompoundCreateGUI(IceWidget* parent, sdword x, sdword y);
	};

	CompoundCreateWindow*	CreateCompoundCreateGUI(IceWidget* parent, sdword x, sdword y);
	void					CloseCompoundCreateGUI();

	void					ShowCompoundCreateWindow(Pint* pint, udword index, udword nb, const PintActorHandle* handles, CreateCompoundCallback* callback);
	void					HideCompoundCreateWindow();
	CompoundCreateWindow*	GetVisibleCompoundCreateWindow();

#endif
