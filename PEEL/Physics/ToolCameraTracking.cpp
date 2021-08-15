///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolCameraTracking.h"
#include "Camera.h"

#ifdef REMOVED

///////////////////////////////////////////////////////////////////////////////

ToolCameraTracking::ToolCameraTracking()
{
}

ToolCameraTracking::~ToolCameraTracking()
{
}

void ToolCameraTracking::Reset(udword pint_index)
{
	if(pint_index==INVALID_ID)
	{
		for(udword i=0;i<MAX_NB_ENGINES;i++)
			mTrackedData[i].Reset();
	}
	else
	{
		ASSERT(pint_index<MAX_NB_ENGINES)
		mTrackedData[pint_index].Reset();
	}
}

void ToolCameraTracking::RightDownCallback(Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	TrackingData& TD = mTrackedData[pint_index];

	PintRaycastHit Hit;
	if(Raycast(pint, Hit, mOrigin, mDir))
	{
		printf("Picked object: %d\n", size_t(Hit.mObject));
		TD.mObject = Hit.mObject;
//		gTrackedEngine = gEngines[i].mEngine;
	}
	else
	{
		TD.mObject = null;
//		gTrackedEngine = null;
	}
}

void ToolCameraTracking::RenderCallback(Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	TrackingData& TD = mTrackedData[pint_index];

//	if(gTrackedObject/* && gTrackedEngine*/)
	if(TD.mObject/* && gTrackedEngine*/)
	{
		const PR Pose = pint.GetWorldTransform(TD.mObject);
		const Point CamPos = GetCameraPos();
		const Point Dir = (Pose.mPos - CamPos).Normalize();
		SetCamera(CamPos, Dir);
	}
}

///////////////////////////////////////////////////////////////////////////////

#endif