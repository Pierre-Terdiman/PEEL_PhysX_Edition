/*
 *  Progressive Mesh type Polygon Reduction Algorithm
 *  by Stan Melax (c) 1998
 *
 *  The function ProgressiveMesh() takes a model in an "indexed face 
 *  set" sort of way.  i.e. list of vertices and list of triangles.
 *  The function then does the polygon reduction algorithm
 *  internally and reduces the model all the way down to 0
 *  vertices and then returns the order in which the
 *  vertices are collapsed and to which neighbor each vertex
 *  is collapsed to.  More specifically the returned "permutation"
 *  indicates how to reorder your vertices so you can render
 *  an object by using the first n vertices (for the n 
 *  vertex version).  After permuting your vertices, the
 *  map list indicates to which vertex each vertex is collapsed to.
 */

#ifndef PROGRESSIVE_MESH_H
#define PROGRESSIVE_MESH_H

	class PMVertex
	{
		public:
		inline_					PMVertex()		{}
		inline_					~PMVertex()		{}

				Point			mPos;			//!< Position in world space
				udword			mID;			//!< Place of vertex in original list
				Container		mNeighbors;		//!< Adjacent vertices
				Container		mFaces;			//!< Adjacent triangles
				float			mCost;			//!< Cached cost of collapsing edge
				PMVertex*		mCollapse;		//!< Candidate vertex for collapse
				bool			mBoundary;		//!< true for boundary vertices

				void			RemoveIfNonNeighbor(PMVertex *n);
	};

	class PMTriangle
	{
		public:
		inline_					PMTriangle()	{}
		inline_					~PMTriangle()	{}

				PMVertex*		mVertex[3];		//!< Pointers to vertices
				udword			mVRefs[3];		//!< Vertex references
				Point			mNormal;		//!< Unit face normal

				void			ComputeNormal();
				void			ReplaceVertex(PMVertex* vold, PMVertex* vnew);
				bool			HasVertex(PMVertex* v);
	};


	class MESHMERIZER_API LODRunner2
	{
		public:
								LODRunner2();
								~LODRunner2();

				bool			ProgressiveMesh(udword nbverts, const Point* verts, udword nbfaces, const IndexedTriangle* faces, Container& map, Container& permutation, udword& minref);

		inline_	udword			GetNbConstrainedVertices()	const	{ return mNbConstrainedVertices; }

		private:
				udword			mNbVerts;
				udword			mNbFaces;
				PMVertex*		mVerts;
				PMTriangle*		mFaces;
				Container		mActiveVerts;
				Container		mActiveFaces;
				udword			mNbConstrainedVertices;
		// Internal methods
				LODRunner2&		FreeUsedRam();
				bool			ComputeConstrainedVertices();

				void			Collapse(PMVertex* u, PMVertex* v);
				PMVertex*		MinimumCostEdge();
				float			ComputeEdgeCollapseCost(PMVertex *u,PMVertex *v);
				void			ComputeEdgeCostAtVertex(PMVertex *v);
				void			ComputeAllEdgeCollapseCosts();

				bool			DeleteVertex(PMVertex* v);
				bool			DeleteTriangle(PMTriangle* t);
	};

	MESHMERIZER_API	bool	PermuteVertices(udword nbverts, const Container& permutation, ubyte* verts, udword vertexsize, udword nbfaces, udword* dtri, uword* wtri, udword minref);
	MESHMERIZER_API udword	BuildPM(udword render_num, const udword* collapse_map, udword nbfaces, const udword* dfaces, const uword* wfaces, Container& mesh, udword minref);

#endif
