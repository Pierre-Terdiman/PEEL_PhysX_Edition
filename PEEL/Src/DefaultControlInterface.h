///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef DEFAULT_CONTROL_INTERFACE_H
#define DEFAULT_CONTROL_INTERFACE_H

#include "ControlInterface.h"
#include "PintDef.h"
#include "Pint.h"
#include "GUI_CompoundEdit.h"
#include "GUI_ActorSelection.h"
#include "PintSelectionManager.h"
#include "Gamepad.h"

	class Pint;
	class GUI_RenderInterface;

	struct SelectionAction
	{
		bool	mHideAll;
		bool	mShowAll;
		bool	mSelectAll;
		bool	mHideSelection;
		bool	mShowSelection;
		bool	mHideUnselected;
		bool	mSelectHidden;
		bool	mDisableSelection;
		bool	mEnableSelection;
		bool	mDeleteSelection;
		bool	mEditSelection;
		bool	mCancelSelection;
		bool	mShowSelectionDialog;
	};

	class DefaultControlInterface : public PEEL::ControlInterface, public CreateCompoundCallback, public GamepadInterface, public ActorSelectionCallback//, public Reporter
	{
		public:
										DefaultControlInterface();
		virtual							~DefaultControlInterface(){}

		virtual	bool					LeftDownCallback	(const MouseInfo& mouse)	override;
		virtual	bool					MiddleDownCallback	(const MouseInfo& mouse)	override;
		virtual	bool					RightDownCallback	(const MouseInfo& mouse)	override;

		virtual	void					LeftDragCallback	(const MouseInfo& mouse)	override;
		virtual	void					MiddleDragCallback	(const MouseInfo& mouse)	override;
		virtual	void					RightDragCallback	(const MouseInfo& mouse)	override;

		virtual	void					LeftUpCallback		(const MouseInfo& mouse)	override;
		virtual	void					MiddleUpCallback	(const MouseInfo& mouse)	override;
		virtual	void					RightUpCallback		(const MouseInfo& mouse)	override;

		virtual	void					MouseMoveCallback	(const MouseInfo& mouse)	override;

		virtual	void					LeftDblClkCallback	(const MouseInfo& mouse)	override;
		virtual	void					MiddleDblClkCallback(const MouseInfo& mouse)	override;
		virtual	void					RightDblClkCallback	(const MouseInfo& mouse)	override;

		virtual	bool					KeyboardCallback	(unsigned char key, int x, int y, bool down)	override;
		virtual	void					RenderCallback		(PintRender&)	override;
		virtual	void					Reset				()				override;

		// CreateCompoundCallback
		virtual	void					CreateCompound(CompoundCreateWindow& ccw)	override;
		//~CreateCompoundCallback

		// GamepadInterface
		virtual	void					OnButtonEvent(udword button_id, bool down)	override;
		virtual	void					OnAnalogButtonEvent(udword button_id, ubyte old_value, ubyte new_value)	override;
		virtual	void					OnAxisEvent(udword axis_id, float value)	override;
		//~GamepadInterface

		// ActorSelectionCallback
		virtual	void					OnSelectedActor(Pint* pint, PintActorHandle h, void* user_data)	override;
		//~ActorSelectionCallback

		// Reporter
//		virtual	bool					ReportObject(PintActorHandle);
		//~Reporter

				SelectionManager		mSelMan;

				void					HideAll();
				void					ShowAll();
				void					SelectAll(bool ignore_hidden_objects);
				void					SetSelectionVisibility(bool vis);
				void					SetSelectionEnabled(bool enabled);
				void					HideUnselected();
				udword					GetNbHidden()	const;
				void					SelectHidden();
				void					ShowEditActorWindow(/*const MouseInfo& mouse*/);
				void					CreateJoints();

				void					GetSelectionActionAvailability(SelectionAction& actions)	const;

		inline_	bool					IsSelectionLocked()	const	{ return mLockSelection;	}
				void					LockSelection(bool b);

				void					PickObject(Pint* pint, PintActorHandle touched_actor, udword engine_index);

		private:
				sdword					mMouseX;
				sdword					mMouseY;
				sdword					mMouseX2;
				sdword					mMouseY2;
				bool					mLeftDrag;
				bool					mRectangleSelection;
				bool					mValidateLeftDownCB;
				bool					mLockSelection;

				void					GetSelectedEditorObjects(PtrContainer& objects);
				Pint*					CanMergeSelected(udword& index)	const;
				void					DoPopupMenu(const MouseInfo& mouse);
				void					CallRenderCallbacks(GUI_RenderInterface& ri, PintRender& render)	const;
	};

#endif
