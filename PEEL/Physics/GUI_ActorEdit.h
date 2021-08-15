///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_ACTOR_EDIT_H
#define GUI_ACTOR_EDIT_H

#include "PintDef.h"
	class Pint;

	class EditActorWindow : public IceWindow
	{
		public:
								EditActorWindow(const WindowDesc& desc);
		virtual					~EditActorWindow();

		virtual int				handleEvent(IceGUIEvent* event);

				void			InitFrom(Pint* pint, PintActorHandle handle);

				void			SaveParams();
		//private:
				IceEditBox*		mNameEditBox;
				IceButton*		mOKButton;
				IceButton*		mCancelButton;
				IceLabel*		mNameLabel;
				IceLabel*		mShapesLabel;
				IceLabel*		mMassLabel;
				IceLabel*		mInfoLabel;
				IceListBox*		mShapes;
				Pint*			mPint;
				PintActorHandle	mHandle;
				bool			mSomethingChanged;

		private:
				void			AskUserForSave();

		friend EditActorWindow* CreateActorEditGUI(IceWidget* parent, sdword x, sdword y);
	};

	EditActorWindow*	CreateActorEditGUI(IceWidget* parent, sdword x, sdword y);
	void				CloseActorEditGUI();

	void				ShowEditActorWindow(Pint* pint, PintActorHandle handle);
	void				HideEditActorWindow();
	EditActorWindow*	GetVisibleActorEditWindow();

#endif
