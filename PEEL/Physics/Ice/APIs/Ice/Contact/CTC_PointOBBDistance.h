///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for box-point distance
 *	\file		CTC_PointOBBDistance.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCPOINTOBBDISTANCE_H
#define CTCPOINTOBBDISTANCE_H

	// Point-OBB squared distance
	CONTACT_API float PointOBBSqrDist(const Point& point, const Point& center, const Point& extents, const Matrix3x3& rot, Point* params=null);

	// Point-OBB distance
	inline_ float PointOBBDist(const Point& point, const Point& center, const Point& extents, const Matrix3x3& rot, Point* params=null)
	{
		return sqrtf(PointOBBSqrDist(point, center, extents, rot, params));
	}

	inline_ float PointOBBSqrDist(const Point& point, const OBB& obb, Point* params=null)
	{
		return Ctc::PointOBBSqrDist(point, obb.mCenter, obb.mExtents, obb.mRot, params);
	}

	inline_ float PointOBBDist(const Point& point, const OBB& obb, Point* params=null)
	{
		return Ctc::PointOBBDist(point, obb.mCenter, obb.mExtents, obb.mRot, params);
	}

#endif // CTCPOINTOBBDISTANCE_H
