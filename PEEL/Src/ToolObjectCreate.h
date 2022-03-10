///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_CREATE_OBJECT_H
#define TOOL_CREATE_OBJECT_H

//#include "ToolRayBased.h"
#include "ToolInterface.h"

//	class ToolCreateObject : public ToolRayBased
	class ToolCreateObject : public ToolInterface
	{
		public:
						ToolCreateObject();
		virtual			~ToolCreateObject();

		virtual	void	CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void	RightDownCallback	(Pint& pint, udword pint_index);
//		virtual	void	SetMouseData		(const MouseInfo& mouse);
		virtual	void	MouseMoveCallback	(const MouseInfo& mouse);

		// GUI_RenderInterface
		virtual	void	RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		//~GUI_RenderInterface

				Point	mDir;
				Point	mOrigin;
				Point	mImpact;
				sdword	mX,mY;
	};

#endif
