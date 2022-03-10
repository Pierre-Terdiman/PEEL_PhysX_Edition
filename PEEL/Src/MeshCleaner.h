///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef MESH_CLEANER_H
#define MESH_CLEANER_H

	class MeshCleaner
	{
		public:
			MeshCleaner(udword nbVerts, const Point* verts, udword nbTris, const udword* indices, float meshWeldTolerance=0.01f);
			~MeshCleaner();

			udword	mNbVerts;
			udword	mNbTris;
			Point*	mVerts;
			udword*	mIndices;
			udword*	mRemap;
	};

#endif
