///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for triangle-point distance
 *	\file		CTC_PointTriangleDistance.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCPOINTTRIANGLEDISTANCE_H
#define CTCPOINTTRIANGLEDISTANCE_H

	// Point-Triangle squared distance
	CONTACT_API float PointTriangleSqrDist(const Point& point, const Point& p0, const Point& p1, const Point& p2, float* u=null, float* v=null);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Computes the distance from a point to a triangle. Original code by Dave Eberly.
	 *	\param		point		[in] the point
	 *	\param		triangle	[in] the indexed triangle
	 *	\param		verts		[in] the vertices
	 *	\param		u			[out] closest point barycentric coordinates
	 *	\param		v			[out] closest point barycentric coordinates
	 *	\return		the distance from the point to the triangle
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline_ float PointTriangleDist(const Point& point, const Point& p0, const Point& p1, const Point& p2, float* u=null, float* v=null)
	{
		return sqrtf(PointTriangleSqrDist(point, p0, p1, p2, u, v));
	}

	inline_ float PointTriangleSqrDist(const Point& point, const IndexedTriangle& triangle, const Point* verts, float* u=null, float* v=null)
	{
		return Ctc::PointTriangleSqrDist(point, verts[triangle.mRef[0]], verts[triangle.mRef[1]], verts[triangle.mRef[2]], u, v);
	}

	inline_ float PointTriangleDist(const Point& point, const IndexedTriangle& triangle, const Point* verts, float* u=null, float* v=null)
	{
		return Ctc::PointTriangleDist(point, verts[triangle.mRef[0]], verts[triangle.mRef[1]], verts[triangle.mRef[2]], u, v);
	}

#endif // CTCPOINTTRIANGLEDISTANCE_H
