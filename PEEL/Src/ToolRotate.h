///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_ROTATE_H
#define TOOL_ROTATE_H

#include "ToolRayBased.h"
#include "ToolTransform.h"

#define SUPPORT_ROTATE_TOOL_UI

	class ToolRotate : public ToolRayBased
	{
		public:
						ToolRotate();
		virtual			~ToolRotate();

#ifdef SUPPORT_ROTATE_TOOL_UI
		virtual	void	CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);
#endif
		virtual	void	Select				();
		virtual	void	Deselect			();
		virtual	void	Reset				(udword pint_index);

		virtual	void	OnObjectReleased	(Pint& pint, PintActorHandle removed_object);

		virtual	void	KeyboardCallback	(Pint& pint, udword pint_index, unsigned char key, bool down);

		virtual	void	RightDownCallback	(Pint& pint, udword pint_index);
		virtual	void	RightDragCallback	(Pint& pint, udword pint_index);
		virtual	void	RightUpCallback		(Pint& pint, udword pint_index);

		// GUI_RenderInterface
		virtual	void	RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		virtual	void	PostRenderCallback	()													override;
		//~GUI_RenderInterface

#ifdef SUPPORT_ROTATE_TOOL_UI
				EditPosWindowPtr	mEditPos;
#endif
		struct RotateData
		{
			RotateData()
			{
				Reset();
			}

			inline	void	Reset()
			{
				mObject = null;
				mTransformer.Reset();
			}

			PintActorHandle	mObject;
			Transformer		mTransformer;
		};

		RotateData	mData[MAX_NB_ENGINES];
	};

#endif
