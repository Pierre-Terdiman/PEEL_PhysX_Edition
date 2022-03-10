///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains convex decomposer code.
 *	\file		IceConvexDecomposer.h
 *	\author		Pierre Terdiman
 *	\date		June, 21, 2005
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECONVEXDECOMPOSER_H
#define ICECONVEXDECOMPOSER_H

	struct MESHMERIZER_API DecompParams
	{
		const IndexedTriangle*	mFaces;
		const Point*			mVerts;
		const EdgeList*			mEdgeList;
		Container*				mConnectedTriangles;
		Container*				mConnectedVertices;
		const bool*				mEdgeFlags;
		udword*					mTags;
		udword					mID;
		udword					mNb;
	};

	enum ConvexDecompDebugFlag
	{
		CDDF_RULE1		= (1<<0),
		CDDF_RULE2		= (1<<1),
		CDDF_RULE2B		= (1<<2),

		CDDF_DEFAULT	= CDDF_RULE1|CDDF_RULE2|CDDF_RULE2B,
	};

	class MESHMERIZER_API ConvexDecomposer
	{
		public:
										ConvexDecomposer();
										~ConvexDecomposer();

				bool					Decompose(udword nb_faces, const IndexedTriangle* faces, udword nb_verts, const Point* verts);

		//! These calls are only valid after ExtractSubMeshes() has been called
		inline_	udword					GetNbConvexSubmeshes()	const	{ return mConvexID;		}
		inline_	const udword*			GetConvexTags()			const	{ return mConvexTags;	}
		inline_	udword					GetNbFlatSubmeshes()	const	{ return mFlatID;		}
		inline_	const udword*			GetFlatTags()			const	{ return mFlatTags;		}

				udword					mFlags;
		private:
				DecompParams			mParams;
//				udword					mNbConnectedFaces;		//!< Number of connected faces for current submesh
				udword					mConvexID;				//!< Submesh ID
				udword*					mConvexTags;			//!< One tag/face, each face is marked with a submesh ID
				udword					mFlatID;				//!< Submesh ID
				udword*					mFlatTags;				//!< One tag/face, each face is marked with a submesh ID
		// Internal methods
				bool					ExtractSubmeshFromFace(/*DecompParams& params,*/udword new_tri, udword nb_faces);
				void					_TrackTriangle(/*DecompParams& params,*/ udword new_tri, udword new_vertex);
				void					ProcessPatch(udword nb_indices, const udword* indices, const IndexedTriangle* faces, const Point* verts);
				void					MakeRelativePatch(udword nb_indices, const udword* indices);

void	NonRecursiveTrack(udword new_tri, udword nb_faces);
void	_TrackTriangle(FIFOStack& stack, udword new_tri, udword new_vertex);

	};

// all 3d triangles should be co-planer.  Doesn't bother to check, you should have done that yourself to begin with!
FUNCTION MESHMERIZER_API unsigned int Triangulate3d(unsigned int pcount,     // number of points in the polygon
                           const double *vertices,   // array of 3d vertices.
                           double *triangles,        // memory to store output triangles
                           unsigned int maxTri,
                           const double *plane);    // maximum triangles we are allowed to output.

FUNCTION MESHMERIZER_API unsigned int TriangulateIndexed3d(unsigned int pcount,     // number of points in the polygon
                           const unsigned int *indices, // polygon points using indices
                           const double *vertices,   // base address for array indexing
                           double *triangles,        // buffer to store output 3d triangles.
                           unsigned int maxTri,
                           const double *plane);    // maximum triangles we can output.

FUNCTION MESHMERIZER_API unsigned int Triangulate2d(unsigned int pcount,     // number of points in the polygon
                           const double *vertices,   // address of input points (2d)
                           double *triangles,        // destination buffer for output triangles.
                           unsigned int maxTri);    // maximum number of triangles we can store.


#endif // ICECONVEXDECOMPOSER_H
