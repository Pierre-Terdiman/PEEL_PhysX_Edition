///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_CREATE_ROPE_H
#define TOOL_CREATE_ROPE_H

//#include "ToolInterface.h"
#include "ToolRayBased.h"

	class EditPosWindow;

	class ToolCreateRope : public ToolRayBased
	{
		public:
								ToolCreateRope();
		virtual					~ToolCreateRope();

		virtual	void			CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void			Reset				(udword pint_index);
		virtual	void			Deselect			()						{ Reset(INVALID_ID);	}

		virtual	void			RightDownCallback	(Pint& pint, udword pint_index);
//		virtual	void			SetMouseData		(const MouseInfo& mouse);
		virtual	void			MouseMoveCallback	(const MouseInfo& mouse);

		virtual	void			PreRenderCallback	();
		virtual	void			RenderCallback		(Pint& pint, udword pint_index);
		virtual	void			PostRenderCallback	();

				void			OnComboBoxChange();
		private:
				IceButton*		mButton0;
				IceButton*		mButton1;
				IceButton*		mActiveButton;
				EditPosWindow*	mEditPos0;
				EditPosWindow*	mEditPos1;

				enum DelayedAction
				{
					NONE,
					CREATE_ROPE,
				};
				DelayedAction	mPendingAction;

				bool			CreateRope(Pint& pint);
	};

#endif
