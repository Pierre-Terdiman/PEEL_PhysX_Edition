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
#include "PintSelectionManager.h"
#include "Gamepad.h"

	class Pint;

	class DefaultControlInterface : public PEEL::ControlInterface, public CreateCompoundCallback, public GamepadInterface//, public Reporter
	{
		public:
										DefaultControlInterface();
		virtual							~DefaultControlInterface(){}

		virtual	bool					LeftDownCallback	(const MouseInfo& mouse);
		virtual	bool					MiddleDownCallback	(const MouseInfo& mouse);
		virtual	bool					RightDownCallback	(const MouseInfo& mouse);

		virtual	void					LeftDragCallback	(const MouseInfo& mouse);
		virtual	void					MiddleDragCallback	(const MouseInfo& mouse);
		virtual	void					RightDragCallback	(const MouseInfo& mouse);

		virtual	void					LeftUpCallback		(const MouseInfo& mouse);
		virtual	void					MiddleUpCallback	(const MouseInfo& mouse);
		virtual	void					RightUpCallback		(const MouseInfo& mouse);

		virtual	void					MouseMoveCallback	(const MouseInfo& mouse);

		virtual	void					LeftDblClkCallback	(const MouseInfo& mouse);
		virtual	void					MiddleDblClkCallback(const MouseInfo& mouse);
		virtual	void					RightDblClkCallback	(const MouseInfo& mouse);

		virtual	bool					KeyboardCallback	(unsigned char key, int x, int y, bool down);
		virtual	void					RenderCallback		();
		virtual	void					Reset				();

		// CreateCompoundCallback
		virtual	void					CreateCompound(CompoundCreateWindow& ccw);
		//~CreateCompoundCallback

		// GamepadInterface
		virtual	void					OnButtonEvent(udword button_id, bool down);
		virtual	void					OnAnalogButtonEvent(udword button_id, ubyte old_value, ubyte new_value);
		virtual	void					OnAxisEvent(udword axis_id, float value);
		//~GamepadInterface

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
				void					SelectHidden();
				void					ShowEditActorWindow(/*const MouseInfo& mouse*/);
				void					CreateJoints();

		private:
				sdword					mMouseX;
				sdword					mMouseY;
				sdword					mMouseX2;
				sdword					mMouseY2;
				bool					mLeftDrag;
				bool					mRectangleSelection;
				bool					mValidateLeftDownCB;

				void					GetSelectedEditorObjects(PtrContainer& objects);
				Pint*					CanMergeSelected(udword& index)	const;
				void					DoPopupMenu(const MouseInfo& mouse);
	};

#endif
