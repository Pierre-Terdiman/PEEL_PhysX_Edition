///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for triangle-triangle intersection
 *	\file		CTC_TriangleTriangleOverlap.h
 *	\author		Tomas Möller
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCTRIANGLETRIANGLEOVERLAP_H
#define CTCTRIANGLETRIANGLEOVERLAP_H

	// Triangle-triangle
	CONTACT_API bool TriTri(const Point& V0, const Point& V1, const Point& V2, const Point& U0, const Point& U1, const Point& U2);

	CONTACT_API int TriTri(const Point& v0, const Point& v1, const Point& v2, const Point& u0, const Point& u1, const Point& u2, int& coplanar, Point& isectpt1, Point& isectpt2);


#ifdef SUPPORT_NEW_TRI_TRI_CODE	// Usually disabled, too bloody slow to compile
CONTACT_API int tri_tri_overlap_test_3d(float p1[3], float q1[3], float r1[3], 
			    float p2[3], float q2[3], float r2[3]);

CONTACT_API int tri_tri_intersection_test_3d(float p1[3], float q1[3], float r1[3], 
				 float p2[3], float q2[3], float r2[3],
				 int * coplanar, 
				 float source[3],float target[3]);
#endif


#endif // CTCTRIANGLETRIANGLEOVERLAP_H

