///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to compute LODs. (progressive meshes)
 *	\file		IceLODRunner.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Notes	:	Based on:
//				- code by Stan Melax in Game Developer
//				- article by Jan Svarovsky in GDConf'99 ("Extreme Detail Graphics")
//				- SIGGRAPH papers by Hoppe & al
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

  - The edge-collapse cost formula comes from Stan Melax. Many other ones may be used, from the simplest (edge-length only) to the most difficult (Hoppe's energy function).

  - Many issues have been chosen to match DX7's vertex buffers easily. (No geomorphs for example) For better or for worse.

  - The PM is computed independently for each submesh of a topochannel, i.e. the way we use meshes in Irion *DO* have drawbacks in the algorithm. E.g. along a texture seam
  I could have collapsed two vertices the same way, not to create holes in th mesh: since this implies a relation from one topo to another, my solution was just to mark those
  frontier edges as uncollapsable. For better or for worse, indeed.

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICELODRUNNER_H
#define ICELODRUNNER_H

#ifdef OBSOLETE_LOD
	// Result structure
	struct MESHMERIZER_API LOD_RESULT{
				LOD_RESULT()			{ ZeroMemory(this, SIZEOFOBJECT); }
				udword*					Permutation;
				udword*					Map;
				udword					OffsetToRelative;
				udword					MinimumLOD;
	};

	struct MESHMERIZER_API LOD_Vertex{
				LOD_Vertex()			{ ZeroMemory(this, SIZEOFOBJECT); }
				~LOD_Vertex()			{ DELETEARRAY(mSharingFaces); DELETEARRAY(mNeighbors); }

				LOD_Vertex&				AddSharingFace(udword nb);		// tells the system current vertex is referenced in face 'nb'
				LOD_Vertex&				DeleteSharingFace(udword nb);	// tells the system current vertex is no more referenced in face 'nb'

				LOD_Vertex&				AddNeighbor(LOD_Vertex& vertex);
				LOD_Vertex&				DeleteNeighbor(LOD_Vertex& vertex);

				// Faces sharing this vertex
				udword					mNbAllocatedFaces;				// current allocated ram = max #faces we can store without resizing
				udword					mNbSharingFaces;				// current #stored faces
				udword*					mSharingFaces;					// stored faces (refs)

				// Neighbor vertices
				udword					mNbAllocatedNeighbors;			// current allocated ram = max #vertices we can store without resizing
				udword					mNbNeighbors;					// #neighbor vertices for current vertex
				udword*					mNeighbors;						// neighbors list

				// Vertex parameters
				udword					mID;							// vertex-ID = index in original list
				Point					mPt;

				// Collapse data
				float					mCost;							// Cached cost of collapsing edge
				LOD_Vertex*				mCollapsed;
				bool					mDiscarded;
				bool					mConstrained;
	};

	class MESHMERIZER_API LOD_Face : public IndexedTriangle
	{
		public:
										LOD_Face()	{}
										~LOD_Face()	{}
				LOD_Face&				ComputeNormal(LOD_Vertex* v);
				Point					Normal;
	};

	class MESHMERIZER_API LODRunner
	{
		public:
											LODRunner();
											~LODRunner();
		// Creation methods
						bool				Init(IndexedSurface& surface);
		// Computation method
						bool				ComputeProgressiveMesh(LOD_RESULT& result);
						bool				ReorderVertexList(udword nbvertices, Point* vertices, udword* permutation);
						bool				ReorderFaceList(udword nbfaces, uword* wfaces, udword* dfaces, udword* permutation);
						udword				ComputeTopology(udword nbfaces, uword* wfaces, udword* dfaces, udword minref, udword finalnbvtx, udword* crossref, CustomArray& newmesh);
		private:
						udword				mNbFaces;						// #faces in original mesh
						udword				mNbVerts;						// #vertices in original mesh
						LOD_Face*			mFaces;
						LOD_Vertex*			mVerts;

						udword				mMinRef;						// Offset from absolute to relative references

						udword*				mPermutation;
						udword*				mMap;

		// Edge stuff
//						udword				mNbEdges;
//						LOD_EdgesRefs*		mEdgesRef;
//						LOD_Edge*			mEdges;
//						LOD_Pair*			mEdgeToTriangles;
//						udword*				mFacesByEdges;

						udword				mNbConstrainedVertices;

		// Methods
						float				ComputeEdgeCollapseCost(LOD_Vertex& u, LOD_Vertex& v);
						void				ComputeEdgeCostAtVertex(LOD_Vertex& v);
						LOD_Vertex*			MinimumCostEdge();
						void				Collapse(LOD_Vertex* u, LOD_Vertex* v);
						bool				ComputeConstrainedVertices();
	};
#endif

#endif // ICELODRUNNER_H
