///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SPHERE_H
#define SPHERE_H

	class SphereMesh
	{
		public:
							SphereMesh();
							SphereMesh(udword nb_circle_pts, udword nb_rotations, float radius, bool half_sphere=false);
							~SphereMesh();

		void				Generate(udword nb_circle_pts, udword nb_rotations, float radius, bool half_sphere=false);
		void				Reset();

		float				mRadius;
		udword				mNbVerts;
		Point*				mVerts;
		udword				mNbTris;
		IndexedTriangle*	mTris;
	};

	class GeoSphereMesh
	{
		public:
								GeoSphereMesh(float radius);
								~GeoSphereMesh();

		float					mRadius;
		udword					mNbVerts;
		Point*					mVerts;
		udword					mNbTris;
		const IndexedTriangle*	mTris;
	};

#endif
