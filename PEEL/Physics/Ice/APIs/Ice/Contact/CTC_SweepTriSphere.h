///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCSWEEPTRISPHERE_H
#define CTCSWEEPTRISPHERE_H

#define gExperimentalCulling	1
	//	dpc0 = center|dir;
	inline_ bool CullTriangle(const Triangle& CurrentTri, const Point& dir, float radius, float t, const float dpc0)
	{
		const float dp0 = CurrentTri.mVerts[0]|dir;
		const float dp1 = CurrentTri.mVerts[1]|dir;
		const float dp2 = CurrentTri.mVerts[2]|dir;
		float dp = dp0;
		if(dp1<dp)	dp = dp1;
		if(dp2<dp)	dp = dp2;

		if(dp>dpc0 + t + radius)
			return false;

		if(gExperimentalCulling)
		{
			if(dp0<dpc0 && dp1<dpc0 && dp2<dpc0)
				return false;
		}
		return true;
	}

	CONTACT_API	bool SweepSphereTriangles(	udword nb_tris, const Triangle* triangles,
								const Point& center, const float radius,
								const Point& dir, float length,
								Point& hit, Point& normal, float& t, udword& index);

	CONTACT_API	bool SweepSphereTriangles2(	udword nb_tris, const Triangle* triangles, const udword* edge_flags,
								const Point& center, const float radius,
								const Point& dir, float length,
								Point& hit, Point& normal, float& t, udword& index);

#ifdef OLDIES
	CONTACT_API	bool	SweepTriSphere(const Triangle& tri, const Sphere& sphere, const Point& dir, float& min_dist, Point& normal);
	CONTACT_API	bool	SweepSphereTriangles(	udword nb_tris, const Triangle* triangles,
							const Point& center, const float radius,
							const Point& dir, float length,
							Point& hit, Point& normal, float& t, udword& index);
	CONTACT_API	bool	SweepSphereTriangles2(	udword nb_tris, const Triangle* triangles,
							const Point& center, const float radius,
							const Point& dir, float length,
							Point& hit, Point& normal, float& t, udword& index);
#endif

#endif // CTCSWEEPTRISPHERE_H
