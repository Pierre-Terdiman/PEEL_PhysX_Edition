///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCSWEEPCAPSULETRIANGLES_H
#define CTCSWEEPCAPSULETRIANGLES_H

	CONTACT_API	bool	SweepCapsuleTriangles(udword nb_tris, const Triangle* triangles, const udword* edge_flags,
								const LSS& capsule, const Point& dir, float max_dist, float* t, Point* normal, Point* hit, udword* hit_index);

	CONTACT_API	bool	SweepCapsuleTriangles(udword up_direction,
								udword nb_tris, const Triangle* triangles, const udword* edge_flags,
								const Point& center, const float radius, const float height,
								const Point& dir, float length,
								Point& hit, Point& normal, float& d, udword& index, udword* cached_index=null);

	CONTACT_API	bool	SweepCapsuleTriangles(	udword nb_tris, const Triangle* triangles,
								const udword* edge_flags, const Point& extrusion_dir,
								const Point& center, const float radius, const Point& dir,
								float length,
								Point& hit, Point& normal, float& t, udword& index);

#ifdef OLDIES
	CONTACT_API	bool	SweepCapsuleTriangles(	udword nb_tris, const Triangle* triangles,
								const udword* edge_flags, const Point& extrusion_dir,
								const Point& center, const float radius, const Point& dir,
								float length,
								Point& hit, Point& normal, float& t, udword& index);

	CONTACT_API	bool	SweepCapsuleTriangles(udword up_direction,
								udword nb_tris, const Triangle* triangles, const udword* edge_flags,
								const Point& center, const float radius, const float height,
								const Point& dir, float length,
								Point& hit, Point& normal, float& d, udword& index);
#endif

#endif // CTCSWEEPCAPSULETRIANGLES_H
