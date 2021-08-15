///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCEXTRUDE_H
#define CTCEXTRUDE_H

	CONTACT_API	udword	ExtrudeMesh(udword nb_tris, const Triangle* triangles, const udword* edge_flags, const Point& extrusion_dir, Triangle* tris, udword* ids, const Point& dir);
	CONTACT_API	udword	ExtrudeMesh(udword nb_tris, const Triangle* triangles, const udword* edge_flags, const Point& extrusion_dir, Triangle* tris, udword* ids, const Point& dir, const OBB& tbv);
	CONTACT_API	udword	ExtrudeBox(const AABB& local_box, const Matrix4x4* world, const Point& extrusion_dir, Triangle* tris);
	CONTACT_API	udword	ExtrudeBox2(const AABB& local_box, const Matrix4x4* world, const Point& extrusion_dir, Triangle* tris, const Point& dir);

#endif // CTCEXTRUDE_H

