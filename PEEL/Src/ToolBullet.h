///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_BULLET_H
#define TOOL_BULLET_H

#include "ToolRayBased.h"

	class ToolBullet : public ToolRayBased
	{
		public:
						ToolBullet();
		virtual			~ToolBullet();

		virtual	void	CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void	Reset				(udword pint_index);

		virtual	void	RightDownCallback	(Pint& pint, udword pint_index);
		virtual	void	RightUpCallback		(Pint& pint, udword pint_index);

		// GUI_RenderInterface
		virtual	void	PreRenderCallback	()													override;
		virtual	void	RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		//~GUI_RenderInterface

		private:

				udword	mDelay;
				bool	mIsFiring;
	};

#endif
