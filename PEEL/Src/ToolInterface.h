///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_INTERFACE_H
#define TOOL_INTERFACE_H

#include "PintDef.h"
#include "GUI_RenderInterface.h"

	#define MAX_NB_ENGINES	32

	class Pint;
	class PintGUIHelper;
	class Widgets;

	class ToolInterface : public GUI_RenderInterface, public Allocateable
	{
		public:

						ToolInterface		()																{}
		virtual			~ToolInterface		()																{}

		virtual	void	CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)		{}

		virtual	void	Select				()																{}
		virtual	void	Deselect			()																{}
		virtual	void	Reset				(udword pint_index)												{}

		virtual	void	OnObjectReleased	(Pint& pint, PintActorHandle removed_object)					{}

		virtual	void	KeyboardCallback	(Pint& pint, udword pint_index, unsigned char key, bool down)	{}

		virtual	void	SetMouseData		(const MouseInfo& mouse)										{}
		virtual	void	MouseMoveCallback	(const MouseInfo& mouse)										{}

		virtual	void	RightDownCallback	(Pint& pint, udword pint_index)									{}
		virtual	void	RightDragCallback	(Pint& pint, udword pint_index)									{}
		virtual	void	RightUpCallback		(Pint& pint, udword pint_index)									{}
		virtual	void	RightDblClkCallback	(Pint& pint, udword pint_index)									{}

		virtual	bool	IsControllingCamera	()		const													{ return false;	}
	};

#endif
