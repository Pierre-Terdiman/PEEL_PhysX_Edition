///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_QUERIES_H
#define PINT_COMMON_PHYSX3_QUERIES_H

	inline_ PxSceneQueryFlags GetRaycastQueryFlags(const EditableParams& params)
	{
		if(params.mSQBothSides)
			return PxSceneQueryFlag::ePOSITION|PxSceneQueryFlag::eNORMAL|PxSceneQueryFlag::eMESH_BOTH_SIDES;
		else
			return PxSceneQueryFlag::ePOSITION|PxSceneQueryFlag::eNORMAL;
	}

	inline_ PxSceneQueryFlags GetSweepQueryFlags(const EditableParams& params)
	{
		PxSceneQueryFlags flags = PxSceneQueryFlag::ePOSITION|PxSceneQueryFlag::eNORMAL;
		if(!params.mSQInitialOverlap)
	//		flags |= PxSceneQueryFlag::eINITIAL_OVERLAP_DISABLE;
			flags |= PxSceneQueryFlag::eASSUME_NO_INITIAL_OVERLAP;
		if(params.mSQPreciseSweeps)
			flags |= PxSceneQueryFlag::ePRECISE_SWEEP;
//		if(params.mSQBothSides)
//			flags |= PxSceneQueryFlag::eMESH_BOTH_SIDES;
		return flags;
	}

	inline_ void FillResultStruct(PintRaycastHit& hit, const PxRaycastHit& result)
	{
	//	hit.mObject			= CreateHandle(result.shape);
		hit.mTouchedActor	= PintActorHandle(result.actor);
		hit.mTouchedShape	= PintShapeHandle(result.shape);
		hit.mImpact			= ToPoint(result.position);
		hit.mNormal			= ToPoint(result.normal);
		hit.mDistance		= result.distance;
		hit.mTriangleIndex	= result.faceIndex;
	}

	inline_ void FillResultStruct(PintRaycastHit& hit, const PxSweepHit& result)
	{
	//	hit.mObject			= CreateHandle(result.shape);
	//	hit.mObject			= CreateHandle(result.actor);
		hit.mTouchedActor	= PintActorHandle(result.actor);
		hit.mTouchedShape	= PintShapeHandle(result.shape);
		hit.mImpact			= ToPoint(result.position);
		hit.mNormal			= ToPoint(result.normal);
		hit.mDistance		= result.distance;
		hit.mTriangleIndex	= result.faceIndex;
	}

#endif