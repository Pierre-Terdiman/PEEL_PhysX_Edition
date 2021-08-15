///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX5_CUSTOM_SQ_H
#define PINT_COMMON_PHYSX5_CUSTOM_SQ_H

#include "extensions/PxSceneQuerySystemExt.h"

using namespace physx;

	PxSceneQuerySystem* CreatePEELCustomSceneQuerySystem();

	bool PEEL_RaycastClosest(	const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
								PxRaycastHit& hit, PxHitFlags hitFlags,
								const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB);

	bool PEEL_RaycastAny(	const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
							const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB);

	bool PEEL_OverlapAny(	const PxGeometry& geometry, const PxTransform& pose,
							const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB);

	PxU32 PEEL_OverlapAll(	const PxGeometry& geometry, const PxTransform& pose,
							PxOverlapHit* hits, PxU32 nb,
							const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB);

	bool PEEL_SweepClosest(	const PxGeometry& geometry, const PxTransform& pose,
							const PxVec3& unitDir, const PxReal distance,
							PxSweepHit& hit, PxHitFlags hitFlags,
							const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB);

#endif
