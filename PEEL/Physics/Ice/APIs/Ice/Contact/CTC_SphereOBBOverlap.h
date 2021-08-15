///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for sphere-OBB intersection
 *	\file		CTC_SphereOBBOverlap.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCSPHEREOBBOVERLAP_H
#define CTCSPHEREOBBOVERLAP_H

	// Sphere-OBB intersection
	CONTACT_API bool SphereOBB_(const Point& center, float radius, const Point& ocenter, const Point& extents, const Matrix3x3& rot, Point& p, Point& normal, float& depth);

	CONTACT_API bool SphereOBB(const Point& center, float radius, const Point& ocenter, const Point& extents, const Matrix3x3& rot, Point& p, Point& normal, float& depth);

	inline_ bool SphereOBB_(const Point& center, float radius, const OBB& obb, Point& p, Point& normal, float& depth)
	{
		return Ctc::SphereOBB_(center, radius, obb.mCenter, obb.mExtents, obb.mRot, p, normal, depth);
	}

	inline_ bool SphereOBB(const Point& center, float radius, const OBB& obb, Point& p, Point& normal, float& depth)
	{
		return Ctc::SphereOBB(center, radius, obb.mCenter, obb.mExtents, obb.mRot, p, normal, depth);
	}

#endif // CTCSPHEREOBBOVERLAP_H
