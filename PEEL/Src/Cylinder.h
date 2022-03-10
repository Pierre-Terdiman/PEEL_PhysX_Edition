///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef CYLINDER_H
#define CYLINDER_H

	class CylinderMesh
	{
		public:
					CylinderMesh();
					CylinderMesh(udword nb_circle_pts, float radius, float half_height, Orientation orientation=ORIENTATION_XY);
					~CylinderMesh();

		void		Generate(udword nb_circle_pts, float radius, float half_height, Orientation orientation=ORIENTATION_XY);
		void		Reset();

		Orientation	mOrientation;
		float		mRadius;
		float		mHalfHeight;
		udword		mNbVerts;
		Point*		mVerts;
	};

#endif
