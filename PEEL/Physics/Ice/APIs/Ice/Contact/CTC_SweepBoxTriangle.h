///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCSWEEPBOXTRIANGLE_H
#define CTCSWEEPBOXTRIANGLE_H

	CONTACT_API	bool	SweepBoxTriangle(const Triangle& tri, const Triangle* edge_triangle, udword edge_flags,
								const AABB& box, const Point* box_vertices, const Point& motion,
								Point& hit, Point& normal, float& d);

	CONTACT_API	bool	SweepBoxTriangles(udword nb_tris, const Triangle* triangles, const Triangle* _edge_triangles,
								const udword* edge_flags, const AABB& box, const Point& dir, float length,
								Point& _hit, Point& _normal, float& _d, udword& _index, udword* cachedIndex);

#endif // CTCSWEEPBOXTRIANGLE_H
