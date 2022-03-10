///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_DEPRECATED_H
#define PINT_COMMON_PHYSX3_DEPRECATED_H

// Copy of deprecated 3.3 stuff
#define PxSceneQueryFlag PxHitFlag
#define PxSceneQueryFlags PxHitFlags
#define PxSceneQueryHit PxQueryHit
#define PxSceneQueryFilterData PxQueryFilterData
#define PxSceneQueryFilterFlag PxQueryFlag
#define PxSceneQueryFilterFlags PxQueryFlags
#define PxSceneQueryFilterCallback PxQueryFilterCallback
#define PxSceneQueryCache PxQueryCache

PX_INLINE bool raycastAny(PxScene* scene,
const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
PxSceneQueryHit& hit, const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
PxClientID queryClient = PX_DEFAULT_CLIENT)
{
	PxSceneQueryFilterData fdAny = filterData;
	fdAny.flags |= PxQueryFlag::eANY_HIT;
#ifndef PHYSX_REMOVED_CLIENT_ID
	fdAny.clientId = queryClient;
#endif
	PxRaycastBuffer buf;
	scene->raycast(origin, unitDir, distance, buf, PxHitFlag::eMESH_ANY, fdAny, filterCall, cache);
	hit = buf.block;
	return buf.hasBlock;
}

PX_INLINE bool raycastSingle(PxScene* scene,
const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
PxSceneQueryFlags outputFlags, PxRaycastHit& hit,
const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
PxClientID queryClient = PX_DEFAULT_CLIENT)
{
	PxRaycastBuffer buf;
	PxQueryFilterData fd1 = filterData;
#ifndef PHYSX_REMOVED_CLIENT_ID
	fd1.clientId = queryClient;
#endif
	scene->raycast(origin, unitDir, distance, buf, outputFlags, fd1, filterCall, cache);
	hit = buf.block;
	return buf.hasBlock;
}

PX_INLINE PxI32 raycastMultiple(PxScene* scene,
const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
PxSceneQueryFlags outputFlags,
PxRaycastHit* hitBuffer, PxU32 hitBufferSize, bool& blockingHit,
const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
PxClientID queryClient = PX_DEFAULT_CLIENT)
{
	PxRaycastBuffer buf(hitBuffer, hitBufferSize);
	PxQueryFilterData fd1 = filterData;
#ifndef PHYSX_REMOVED_CLIENT_ID
	fd1.clientId = queryClient;
#endif
	scene->raycast(origin, unitDir, distance, buf, outputFlags, fd1, filterCall, cache);
	blockingHit = buf.hasBlock;
	if (blockingHit)
	{
		if (buf.nbTouches < hitBufferSize)
		{
			hitBuffer[buf.nbTouches] = buf.block;
			return buf.nbTouches+1;
		}
		else // overflow, drop the last touch
		{
			hitBuffer[hitBufferSize-1] = buf.block;
			return -1;
		}
	} else
		// no block
		return buf.nbTouches;
}

PX_INLINE bool sweepAny(PxScene* scene,
const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
PxSceneQueryFlags queryFlags,
PxSceneQueryHit& hit,
const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
PxSceneQueryFilterCallback* filterCall = NULL,
const PxSceneQueryCache* cache = NULL,
PxClientID queryClient = PX_DEFAULT_CLIENT,
const PxReal inflation = 0.f)
{
	PxSceneQueryFilterData fdAny = filterData;
	fdAny.flags |= PxQueryFlag::eANY_HIT;
#ifndef PHYSX_REMOVED_CLIENT_ID
	fdAny.clientId = queryClient;
#endif
	PxSweepBuffer buf;
	scene->sweep(geometry, pose, unitDir, distance,
		buf, queryFlags, fdAny, filterCall, cache, inflation);
	hit = buf.block;
	return buf.hasBlock;
}

PX_INLINE bool sweepSingle(PxScene* scene,
const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
PxSceneQueryFlags outputFlags,
PxSweepHit& hit,
const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
PxSceneQueryFilterCallback* filterCall = NULL,
const PxSceneQueryCache* cache = NULL,
PxClientID queryClient = PX_DEFAULT_CLIENT, const PxReal inflation=0.f)
{
	PxSweepBuffer buf;
	PxQueryFilterData fd1 = filterData;
#ifndef PHYSX_REMOVED_CLIENT_ID
	fd1.clientId = queryClient;
#endif
	scene->sweep(geometry, pose, unitDir, distance, buf,
		outputFlags, fd1, filterCall, cache, inflation);
	hit = buf.block;
	return buf.hasBlock;
}

PX_INLINE PxI32 sweepMultiple(PxScene* scene,
const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
PxSceneQueryFlags outputFlags, PxSweepHit* hitBuffer, PxU32 hitBufferSize, bool& blockingHit,
const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
PxClientID queryClient = PX_DEFAULT_CLIENT, const PxReal inflation = 0.f)
{
	PxQueryFilterData fd1 = filterData;
#ifndef PHYSX_REMOVED_CLIENT_ID
	fd1.clientId = queryClient;
#endif
	PxSweepBuffer buf(hitBuffer, hitBufferSize);
	scene->sweep(
		geometry, pose, unitDir, distance, buf, outputFlags, fd1, filterCall,
		cache, inflation);
	blockingHit = buf.hasBlock;
	if (blockingHit)
	{
		if (buf.nbTouches < hitBufferSize)
		{
			hitBuffer[buf.nbTouches] = buf.block;
			return buf.nbTouches+1;
		}
		else // overflow, drop the last touch
		{
			hitBuffer[hitBufferSize-1] = buf.block;
			return -1;
		}
	} else
		// no block
		return buf.nbTouches;
}

PX_INLINE PxI32 overlapMultiple(PxScene* scene,
const PxGeometry& geometry, const PxTransform& pose,
PxOverlapHit* hitBuffer, PxU32 hitBufferSize,
const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
PxSceneQueryFilterCallback* filterCall = NULL,
PxClientID queryClient = PX_DEFAULT_CLIENT)
{
	PxQueryFilterData fd1 = filterData;
#ifndef PHYSX_REMOVED_CLIENT_ID
	fd1.clientId = queryClient;
#endif
	fd1.flags |= PxQueryFlag::eNO_BLOCK;
	PxOverlapBuffer buf(hitBuffer, hitBufferSize);
	scene->overlap(geometry, pose, buf, fd1, filterCall);
	if (buf.hasBlock)
	{
		if (buf.nbTouches < hitBufferSize)
		{
			hitBuffer[buf.nbTouches] = buf.block;
			return buf.nbTouches+1;
		}
		else // overflow, drop the last touch
		{
			hitBuffer[hitBufferSize-1] = buf.block;
			return -1;
		}
	} else
		// no block
		return buf.nbTouches;
}

PX_INLINE bool overlapAny(PxScene* scene,
	const PxGeometry& geometry, const PxTransform& pose,
	PxOverlapHit& hit,
	const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
	PxSceneQueryFilterCallback* filterCall = NULL,
	PxClientID queryClient = PX_DEFAULT_CLIENT)
{
	PxSceneQueryFilterData fdAny = filterData;
	fdAny.flags |= (PxQueryFlag::eANY_HIT | PxQueryFlag::eNO_BLOCK);
#ifndef PHYSX_REMOVED_CLIENT_ID
	fdAny.clientId = queryClient;
#endif
	PxOverlapBuffer buf;
	scene->overlap(geometry, pose, buf, fdAny, filterCall);
	hit = buf.block;
	return buf.hasBlock;
}
//~Copy of deprecated 3.3 stuff

#endif