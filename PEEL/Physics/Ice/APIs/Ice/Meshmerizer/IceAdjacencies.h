///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to create adjacency structures.
 *	\file		IceAdjacencies.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEADJACENCIES_H
#define ICEADJACENCIES_H

#ifdef MSH_ADJACENCIES_INCLUDE_CONVEX_BITS
	#define ADJ_TRIREF_MASK		0x1fffffff				//!< Masks 3 bits
	#define IS_CONVEX_EDGE(x)	(x & 0x20000000)		//!< Returns true for convex edges
#else
	#define ADJ_TRIREF_MASK		0x3fffffff				//!< Masks 2 bits
#endif

	#define MAKE_ADJ_TRI(x)		(x & ADJ_TRIREF_MASK)	//!< Transforms a link into a triangle reference.
	#define GET_EDGE_NB(x)		(x>>30)					//!< Transforms a link into a counterpart edge ID.
//	#define IS_BOUNDARY(x)		(x==INVALID_ID)			//!< Returns true for boundary edges.
	#define IS_BOUNDARY(x)		((x & ADJ_TRIREF_MASK)==ADJ_TRIREF_MASK)		//!< Returns true for boundary edges.

	// Forward declarations
	class Adjacencies;

	enum SharedEdgeIndex
	{
		EDGE01	= 0,
		EDGE02	= 1,
		EDGE12	= 2
	};

	inline_ void GetEdgeIndices(SharedEdgeIndex edge_index, udword& id0, udword& id1)
	{
		if(edge_index==0)
		{
			id0 = 0;
			id1 = 1;
		}
		else if(edge_index==1)
		{
			id0 = 0;
			id1 = 2;
		}
		else if(edge_index==2)
		{
			id0 = 1;
			id1 = 2;
		}
	}

	struct CommonWalkParams
	{
		Adjacencies*	Adj;			//!< Surface adjacencies
		udword			SubmeshIndex;	//!< Current submesh index
		void*			UserData;		//!< User-provided data
	};

	typedef void (*WALK_CALLBACK) (const CommonWalkParams& cwp , udword cur_face, udword prev_face, udword crossed_edge, udword prev_edge);

	//! Sets a new edge code
	#define SET_EDGE_NB(link, code)	\
		link&=ADJ_TRIREF_MASK;		\
		link|=code<<30;				\

	//! A triangle class used to compute the adjacency structures.
	class MESHMERIZER_API AdjTriangle
#ifdef MSH_ADJACENCIES_INCLUDE_TOPOLOGY
		: public IndexedTriangle
#else
		: public Allocateable
#endif
	{
		public:
		//! Constructor
		inline_						AdjTriangle()	{}
		//! Destructor
		inline_						~AdjTriangle()	{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the number of boundary edges in a triangle.
		 *	\return		the number of boundary edges. (0 => 3)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				udword				ComputeNbBoundaryEdges()						const;
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the number of valid neighbors.
		 *	\return		the number of neighbors. (0 => 3)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				udword				ComputeNbNeighbors()							const;
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks whether the triangle has a particular neighbor or not.
		 *	\param		tref	[in] the triangle reference to look for
		 *	\param		index	[out] the corresponding index in the triangle (null if not needed)
		 *	\return		true if the triangle has the given neighbor
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				HasNeighbor(udword tref, udword* index=null)	const;
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Flips the winding.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void				Flip();

		// Data access
		inline_	udword				GetLink(SharedEdgeIndex edge_index)				const	{ return mATri[edge_index];					}
		inline_	udword				GetAdjTri(SharedEdgeIndex edge_index)			const	{ return MAKE_ADJ_TRI(mATri[edge_index]);	}
		inline_	udword				GetAdjEdge(SharedEdgeIndex edge_index)			const	{ return GET_EDGE_NB(mATri[edge_index]);	}
		inline_	BOOL				IsBoundaryEdge(SharedEdgeIndex edge_index)		const	{ return IS_BOUNDARY(mATri[edge_index]);	}
#ifdef MSH_ADJACENCIES_INCLUDE_CONVEX_BITS
		inline_	BOOL				HasActiveEdge01()								const	{ return IS_CONVEX_EDGE(mATri[EDGE01]);		}
		inline_	BOOL				HasActiveEdge20()								const	{ return IS_CONVEX_EDGE(mATri[EDGE02]);		}
		inline_	BOOL				HasActiveEdge12()								const	{ return IS_CONVEX_EDGE(mATri[EDGE12]);		}
		inline_	BOOL				HasActiveEdge(udword i)							const	{ return IS_CONVEX_EDGE(mATri[i]);			}
#endif
//		private:
		//! Links/References of adjacent triangles. The 2 most significant bits contains the counterpart edge in the adjacent triangle.
		//! mATri[0] refers to edge 0-1
		//! mATri[1] refers to edge 0-2
		//! mATri[2] refers to edge 1-2
				udword				mATri[3];
	};

	//! The adjacencies creation structure.
	struct MESHMERIZER_API ADJACENCIESCREATE
	{
		//! Constructor
									ADJACENCIESCREATE()	: NbFaces(0), DFaces(null), WFaces(null)
									{
#ifdef MSH_ADJACENCIES_INCLUDE_CONVEX_BITS
									Verts	= null;
									Epsilon	= 0.001f;
#endif
									}
				
				udword				NbFaces;		//!< Number of faces in source topo
				const udword*		DFaces;			//!< List of faces (dwords) or null
				const uword*		WFaces;			//!< List of faces (words) or null
#ifdef MSH_ADJACENCIES_INCLUDE_CONVEX_BITS
				const Point*		Verts;
				float				Epsilon;
#endif
	};

	class MESHMERIZER_API Adjacencies : public Allocateable
	{
		public:
									Adjacencies();
									~Adjacencies();

				udword				mNbFaces;		//!< Number of faces involved in the computation.
				AdjTriangle*		mFaces;			//!< A list of AdjTriangles (one/face)

				bool				Init(const ADJACENCIESCREATE& create);

		// Basic mesh walking
		inline_	const AdjTriangle*	GetAdjacentFace(const AdjTriangle& current_tri, SharedEdgeIndex edge_nb) const
									{
										// No checkings here, make sure mFaces has been created

										// Catch the link
										udword Link = current_tri.GetLink(edge_nb);

										// Returns null for boundary edges
										if(IS_BOUNDARY(Link))	return null;

										// Else transform into face index
										udword Id = MAKE_ADJ_TRI(Link);

										// Possible counterpart edge is:
										// udword Edge = GET_EDGE_NB(Link);

										// And returns adjacent triangle
										return &mFaces[Id];
									}
		// Complete mesh walking
				bool				Walk(WALK_CALLBACK callback, void* user_data=null);
		// Helpers
				udword				ComputeNbBoundaryEdges() const;
#ifdef MSH_ADJACENCIES_INCLUDE_TOPOLOGY
				bool				GetBoundaryVertices(udword nb_verts, bool* bound_status) const;
#else
				bool				GetBoundaryVertices(udword nb_verts, bool* bound_status, const IndexedTriangle* faces) const;
#endif
		// 
#ifdef MSH_ADJACENCIES_INCLUDE_TOPOLOGY
				bool				MakeLastRef(AdjTriangle& cur_tri, udword vref);
#else
				bool				MakeLastRef(AdjTriangle& cur_tri, udword vref, IndexedTriangle* cur_topo);
#endif
		// Stats
				udword				GetUsedRam()	const;
		private:
		// New edge codes assignment
				void				AssignNewEdgeCode(udword link, ubyte edge_nb);
	};

#endif // ICEADJACENCIES_H
