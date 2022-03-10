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

	class ToolCreateRope : public ToolRayBased
	{
		public:
									ToolCreateRope();
		virtual						~ToolCreateRope();

		virtual	void				CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void				Reset				(udword pint_index);
		virtual	void				Deselect			()						{ Reset(INVALID_ID);	}

		virtual	void				RightDownCallback	(Pint& pint, udword pint_index);
//		virtual	void				SetMouseData		(const MouseInfo& mouse);
		virtual	void				MouseMoveCallback	(const MouseInfo& mouse);

		// GUI_RenderInterface
		virtual	void				PreRenderCallback	()													override;
		virtual	void				RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		virtual	void				PostRenderCallback	()													override;
		//~GUI_RenderInterface

				void				OnComboBoxChange();
		private:
				ButtonPtr			mButton0;
				ButtonPtr			mButton1;
				ButtonPtr			mActiveButton;
				EditPosWindowPtr	mEditPos0;
				EditPosWindowPtr	mEditPos1;

				enum DelayedAction
				{
					NONE,
					CREATE_ROPE,
				};
				DelayedAction		mPendingAction;

				bool				CreateRope(Pint& pint);
	};

#endif
