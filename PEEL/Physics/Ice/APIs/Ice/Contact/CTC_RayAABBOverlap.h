///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for ray-AABB intersection
 *	\file		CTC_RayAABBOverlap.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCRAYAABBOVERLAP_H
#define CTCRAYAABBOVERLAP_H

	CONTACT_API bool	SegmentOBB(const Segment& segment, const Point& center, const Point& extents, const Matrix3x3& rot);
	CONTACT_API bool	RayOBB(const Ray& ray, const Point& center, const Point& extents, const Matrix3x3& rot);

	CONTACT_API bool	SegmentAABB(const Segment& segment, const Point& min, const Point& max);
	CONTACT_API bool	RayAABB(const Ray& ray, const Point& min, const Point& max);

	inline_ bool SegmentOBB(const Segment& segment, const OBB& obb)
	{
		return Ctc::SegmentOBB(segment, obb.mCenter, obb.mExtents, obb.mRot);
	}
	inline_ bool RayOBB(const Ray& ray, const OBB& obb)
	{
		return Ctc::RayOBB(ray, obb.mCenter, obb.mExtents, obb.mRot);
	}
	inline_ bool SegmentAABB(const Segment& segment, const AABB& aabb)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::SegmentAABB(segment, Box->mMin, Box->mMax);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::SegmentAABB(segment, Min, Max);
#endif
	}
	inline_ bool RayAABB(const Ray& ray, const AABB& aabb)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::RayAABB(ray, Box->mMin, Box->mMax);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::RayAABB(ray, Min, Max);
#endif
	}

	CONTACT_API int ray_intersect(const Point& min, const Point& max, const Point& ro, const Point& rd, float& tnear, float& tfar);

	enum CLASSIFICATION
	{
		MMM, MMP, MPM, MPP, PMM, PMP, PPM, PPP, POO, MOO, OPO, OMO, OOP, OOM,
		OMM,OMP,OPM,OPP,MOM,MOP,POM,POP,MMO,MPO,PMO,PPO
	};


	struct CONTACT_API aabox
	{
		float x0, y0, z0, x1, y1, z1;
	};

	struct CONTACT_API PrecompRay
	{	
		//common variables
		float x, y, z;		// ray origin	
		float i, j, k;		// ray direction	
		float ii, ij, ik;	// inverses of direction components
		
		// ray slope
		int classification;
		float ibyj, jbyi, kbyj, jbyk, ibyk, kbyi; //slope
		float c_xy, c_xz, c_yx, c_yz, c_zx, c_zy;	
	};

	CONTACT_API void PrecomputeRay(float x, float y, float z, float i, float j, float k, PrecompRay& r);
	CONTACT_API bool slope(const PrecompRay& r, const aabox& b);
	CONTACT_API bool slopeint_div(const PrecompRay& r, const aabox& b, float* t);
	CONTACT_API bool slopeint_mul(const PrecompRay& r, const aabox& b, float* t);

#endif // CTCRAYAABBOVERLAP_H

