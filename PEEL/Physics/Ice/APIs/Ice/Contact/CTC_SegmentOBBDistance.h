///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for segment-box distance
 *	\file		CTC_SegmentOBBDistance.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCSEGMENTOBBDISTANCE_H
#define CTCSEGMENTOBBDISTANCE_H

	// Segment-OBB squared distance
	CONTACT_API float SegmentOBBSqrDist(const Segment& segment, const Point& c0, const Point& e0, const Matrix3x3& r0, float* t=null, Point* p=null);

	// Segment-OBB distance
	inline_ float SegmentOBBDist(const Segment& segment, const Point& c0, const Point& e0, const Matrix3x3& r0, float* t=null, Point* p=null)
	{
		return sqrtf(SegmentOBBSqrDist(segment, c0, e0, r0, t, p));
	}

	inline_ float SegmentOBBSqrDist(const Segment& segment, const OBB& obb, float* t=null, Point* p=null)
	{
		return Ctc::SegmentOBBSqrDist(segment, obb.mCenter, obb.mExtents, obb.mRot, t, p);
	}

	inline_ float SegmentOBBDist(const Segment& segment, const OBB& obb, float* t=null, Point* p=null)
	{
		return Ctc::SegmentOBBDist(segment, obb.mCenter, obb.mExtents, obb.mRot, t, p);
	}

	inline_ bool CapsuleBoxOverlap(const LSS& capsule, const AABB& box)
	{
		float r2 = capsule.mRadius*capsule.mRadius;

		Point Center, Extents;
		box.GetCenter(Center);
		box.GetExtents(Extents);

		Matrix3x3 Idt;
		Idt.Identity();

		return SegmentOBBSqrDist(capsule, Center, Extents, Idt) < r2;
	}

#endif // CTCSEGMENTOBBDISTANCE_H
