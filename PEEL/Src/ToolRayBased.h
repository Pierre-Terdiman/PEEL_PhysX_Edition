///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_RAY_BASED_H
#define TOOL_RAY_BASED_H

#include "ToolInterface.h"
#include "PintSQ.h"
#include "Pint.h"
#include "PEEL.h"

	inline_ bool Raycast(Pint& pint, PintRaycastHit& hit, const Point& origin, const Point& dir)
	{
		PintRaycastData tmp;
		tmp.mOrigin		= origin;
		tmp.mDir		= dir;
		tmp.mMaxDist	= GetPickingDistance();
		return pint.BatchRaycasts(pint.mSQHelper->GetThreadContext(), 1, &hit, &tmp)!=0;
	}

	class ToolRayBased : public ToolInterface
	{
		public:
						ToolRayBased();
		virtual			~ToolRayBased();

		virtual	void	SetMouseData(const MouseInfo& mouse);

				Point	mDir;
				Point	mOrigin;
				sdword	mX,mY;
				sdword	mOldX,mOldY;
	};

#endif
