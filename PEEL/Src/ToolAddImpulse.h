///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_ADD_IMPULSE_H
#define TOOL_ADD_IMPULSE_H

#include "ToolRayBased.h"

	class ToolAddImpulse : public ToolRayBased
	{
		public:
						ToolAddImpulse();
		virtual			~ToolAddImpulse();

		virtual	void	CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void	RightDownCallback	(Pint& pint, udword pint_index)	override;
	};

#endif
