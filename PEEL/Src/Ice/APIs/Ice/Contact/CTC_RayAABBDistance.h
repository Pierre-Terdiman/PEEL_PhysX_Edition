///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for ray-AABB intersection
 *	\file		CTC_RayAABBDistance.h
 *	\author		Pierre Terdiman - original code by Andrew Woo
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCRAYAABBDISTANCE_H
#define CTCRAYAABBDISTANCE_H

	// Original Ray-AABB intersection
	CONTACT_API bool	RayAABB(const Point& min, const Point& max, const Point& origin, const Point& dir, Point& coord);
	// Optimized Ray-AABB intersection
	CONTACT_API bool	RayAABB2(const Point& min, const Point& max, const Point& origin, const Point& dir, Point& coord);

	// Ray-AABB intersection by Tim Schröder / Volition
	CONTACT_API bool	RayAABB3(const Point& min, const Point& max, const Point& p1, const Point& p2, Point& intercept);

	// My new version
	CONTACT_API bool	RayAABB4(const Point& min, const Point& max, const Point& p1, const Point& p2, Point& intercept);

	inline_ bool RayAABB(const AABB& aabb, const Point& origin, const Point& dir, Point& coord)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::RayAABB(Box->mMin, Box->mMax, origin, dir, coord);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::RayAABB(Min, Max, origin, dir, coord);
#endif
	}
	inline_ bool RayAABB2(const AABB& aabb, const Point& origin, const Point& dir, Point& coord)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::RayAABB2(Box->mMin, Box->mMax, origin, dir, coord);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::RayAABB2(Min, Max, origin, dir, coord);
#endif
	}
	inline_ bool RayAABB3(const AABB& aabb, const Point& p1, const Point& p2, Point& intercept)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::RayAABB3(Box->mMin, Box->mMax, p1, p2, intercept);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::RayAABB3(Min, Max, p1, p2, intercept);
#endif
	}
	inline_ bool RayAABB4(const AABB& aabb, const Point& p1, const Point& p2, Point& intercept)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::RayAABB4(Box->mMin, Box->mMax, p1, p2, intercept);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::RayAABB4(Min, Max, p1, p2, intercept);
#endif
	}

	CONTACT_API	udword	RayAABBIntersectAdam(const Point& min, const Point& max, const Point& origin, const Point& dir, Point& coord, float& t);

#endif // CTCRAYAABBDISTANCE_H

