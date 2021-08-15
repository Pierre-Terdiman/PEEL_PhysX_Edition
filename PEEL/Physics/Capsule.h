///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef CAPSULE_H
#define CAPSULE_H

	class CapsuleMesh
	{
		public:
							CapsuleMesh();
							CapsuleMesh(udword nb_circle_pts, float radius, float half_height, bool generate_normals);
							~CapsuleMesh();

		void				Generate(udword nb_circle_pts, float radius, float half_height, bool generate_normals);
		void				Reset();

		Orientation			mOrientation;
		float				mRadius;
		float				mHalfHeight;
		udword				mNbVerts;
		Point*				mVerts;
		Point*				mNormals;
		udword				mNbTris;
		IndexedTriangle*	mTris;
	};

#endif
