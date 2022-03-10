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
#include "Pint.h"
#include "GUI_EditBox.h"
#include "GUI_RenderInterface.h"
#include "GUI_JointParams.h"

	class Pint;

	class EditJointSubWindow : public IceWindow
	{
		public:
											EditJointSubWindow(const WindowDesc& desc, EditBoxInterface& owner);
		virtual								~EditJointSubWindow();

				void						InitFrom(Pint* pint, PintJointHandle handle, PintActorHandle owner);
				void						ReleaseJoint();
				void						SwapActors();

				void						SaveParams();
				void						Cancel();

				void						Render();

				PEEL_EditBox*				mJointNameUI;
				LabelPtr					mTypeLabel;
				LabelPtr					mActor0Label;
				LabelPtr					mActor1Label;
				EditPosWindowPtr			mEditPos0;
				EditPosWindowPtr			mEditPos1;

				ButtonPtr					mButton0;
				ButtonPtr					mButton1;

				GUI_JointParams*			mJointParams[PINT_JOINT_NB];

				bool						SomethingChanged()	const;
		private:
				Pint*						mPint;
				PintJointHandle				mHandle;

				void						AskUserForSave();
	};

	class EditActorWindow : public IceWindow, public EditBoxInterface, public GUI_RenderInterface
	{
		public:
									EditActorWindow(const WindowDesc& desc);
		virtual						~EditActorWindow();

		virtual int					handleEvent(IceGUIEvent* event);

		// EditBoxInterface
		virtual	void				ChangeNotification()	override;
		//~EditBoxInterface

		// GUI_RenderInterface
		virtual	void				PreRenderCallback()	override;
		//~GUI_RenderInterface

				void				InitFrom(Pint* pint, PintActorHandle handle);

				void				SaveParams();
				void				Cancel();

				void				JointNameHasChanged(PintJointHandle handle);
		//private:
				PEEL_EditBox*		mActorNameUI;
				PEEL_EditBox*		mActorMassUI;
				PEEL_EditBox*		mActorLinearDampingUI;
				PEEL_EditBox*		mActorAngularDampingUI;

				ButtonPtr			mOKButton;
				ButtonPtr			mCancelButton;

				LabelPtr			mShapesAndJointsLabel;
				LabelPtr			mInfoLabel;
				ListBoxPtr			mShapesAndJoints;

				Pint*				mPint;
				PintActorHandle		mHandle;

				EditJointSubWindow*	mJointEditWindow;

		private:
				bool				SomethingChanged()	const;
				void				AskUserForSave();

		friend EditActorWindow* CreateActorEditGUI(IceWidget* parent, sdword x, sdword y);
	};

	EditActorWindow*	CreateActorEditGUI(IceWidget* parent, sdword x, sdword y);
	void				CloseActorEditGUI();

	void				ShowEditActorWindow(Pint* pint, PintActorHandle handle);
	void				HideEditActorWindow();
	EditActorWindow*	GetVisibleActorEditWindow();

#endif
