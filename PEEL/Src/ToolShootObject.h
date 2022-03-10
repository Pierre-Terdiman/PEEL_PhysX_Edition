///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_SHOOT_OBJECT_H
#define TOOL_SHOOT_OBJECT_H

#include "ToolRayBased.h"

	class ToolShootObject : public ToolRayBased
	{
		public:
								ToolShootObject();
		virtual					~ToolShootObject();

		virtual	void			CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void			RightDownCallback	(Pint& pint, udword pint_index);

		private:
				ComboBoxPtr		mShapeComboBox;

				void			CreateShotObject(Pint& pint, const PINT_SHAPE_CREATE& create);
	};

#endif
