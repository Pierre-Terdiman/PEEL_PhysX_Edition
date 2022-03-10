///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for decals.
 *	\file		IceDecal.h
 *	\author		Pierre Terdiman
 *	\date		January, 20, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Copyright (C) Eric Lengyel, 2001. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Eric Lengyel, 2001"
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEDECAL_H
#define ICEDECAL_H

	const long gMaxDecalVertices = 256;

	struct MESHMERIZER_API UVs
	{
		float u,v;
	};

	class MESHMERIZER_API Decal
	{
		public:
								Decal(const Point& center, const Point& normal, const Point& tangent, float width, float height, float depth);
								~Decal();

				bool			ClipMesh(udword nb_faces, const IndexedTriangle* faces, const Point* verts, const Point* vertex_normals);
				void			ComputeUVs(const Point& center, const Point& normal, const Point& tangent, float width, float height);
		public:
				Point			mDecalCenter;
				Point			mDecalNormal;

				HPoint			mLeftPlane;
				HPoint			mRightPlane;
				HPoint			mBottomPlane;
				HPoint			mTopPlane;
				HPoint			mFrontPlane;
				HPoint			mBackPlane;

				long			mDecalVertexCount;
				long			mDecalTriangleCount;

				Point			mVertexArray[gMaxDecalVertices];
				UVs				mTexcoordArray[gMaxDecalVertices];
				HPoint			mColorArray[gMaxDecalVertices];
				IndexedTriangle	mTriangleArray[gMaxDecalVertices];
		private:
		// Internal methods
				bool			AddPolygon(long vertex_count, const Point* vertex, const Point* normal);
				long			ClipPolygon(long vertex_count, const Point* vertex, const Point* normal, Point* new_vertex, Point* new_normal) const;
	};

#endif // ICEDECAL_H
