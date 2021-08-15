///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for segment-segment distance
 *	\file		CTC_SegmentSegmentDistance.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCSEGMENTSEGMENTDISTANCE_H
#define CTCSEGMENTSEGMENTDISTANCE_H

	CONTACT_API float SegmentSegmentSqrDist(const Segment& rkSeg0, const Segment& rkSeg1, float* s=null, float* t=null);

	inline_ float SegmentSegmentSqrDist(const Point& seg0_p0, const Point& seg0_p1,
										const Point& seg1_p0, const Point& seg1_p1,
										float* s=null, float* t=null)
	{
		return SegmentSegmentSqrDist(Segment(seg0_p0, seg0_p1), Segment(seg1_p0, seg1_p1), s, t);
	}

	inline_ float SegmentRaySqrDist(const Segment& rkSeg0, const Ray& rkSeg1, float* s=null, float* t=null)
	{
		return SegmentSegmentSqrDist(rkSeg0, Segment(rkSeg1.mOrig, rkSeg1.mOrig + rkSeg1.mDir), s, t);
	}

	CONTACT_API void IntersectLineSegments(const float A1x, const float A1y, const float A1z,
							   const float A2x, const float A2y, const float A2z, 
							   const float B1x, const float B1y, const float B1z,
							   const float B2x, const float B2y, const float B2z,
							   bool infinite_lines, float epsilon, float &PointOnSegAx,
							   float &PointOnSegAy, float &PointOnSegAz, float &PointOnSegBx,
							   float &PointOnSegBy, float &PointOnSegBz, float &NearestPointX,
							   float &NearestPointY, float &NearestPointZ, float &NearestVectorX,
							   float &NearestVectorY, float &NearestVectorZ, bool &true_intersection);

	CONTACT_API void FindNearestPointOnLineSegment(const float A1x, const float A1y, const float A1z,
									   const float Lx, const float Ly, const float Lz,
									   const float Bx, const float By, const float Bz,
									   bool infinite_line, float epsilon_squared, float &NearestPointX,
									   float &NearestPointY, float &NearestPointZ,
									   float &parameter);

	CONTACT_API void FindNearestPointOfParallelLineSegments(float A1x, float A1y, float A1z,
												float A2x, float A2y, float A2z,
												float Lax, float Lay, float Laz,
												float B1x, float B1y, float B1z,
												float B2x, float B2y, float B2z,
												float Lbx, float Lby, float Lbz,
												bool infinite_lines, float epsilon_squared,
												float &PointOnSegAx, float &PointOnSegAy, float &PointOnSegAz,
												float &PointOnSegBx, float &PointOnSegBy, float &PointOnSegBz);

	CONTACT_API void AdjustNearestPoints(float A1x, float A1y, float A1z,
							 float Lax, float Lay, float Laz,
							 float B1x, float B1y, float B1z,
							 float Lbx, float Lby, float Lbz,
							 float epsilon_squared, float s, float t,
							 float &PointOnSegAx, float &PointOnSegAy, float &PointOnSegAz,
							 float &PointOnSegBx, float &PointOnSegBy, float &PointOnSegBz);

#endif // CTCSEGMENTSEGMENTDISTANCE_H
