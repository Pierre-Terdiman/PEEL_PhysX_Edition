///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX5_CUSTOM_GEOM_H
#define PINT_COMMON_PHYSX5_CUSTOM_GEOM_H

#include "..\Pint.h"
#include "extensions/PxCustomGeometryExt.h"

	class CylinderCB : public PxCustomGeometryExt::CylinderCallbacks, public PxUserAllocated
	{
		public:
			CylinderCB(float height, float radius, int axis, float margin) : PxCustomGeometryExt::CylinderCallbacks(height, radius, axis, margin)
			{
			}
	};

#endif