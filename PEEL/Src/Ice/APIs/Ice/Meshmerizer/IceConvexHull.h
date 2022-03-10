///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to compute a convex hull in 3D.
 *	\file		IceConvexHull.h
 *	\author		Pierre Terdiman, original code by Dave Eberly.
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECONVEXHULL_H
#define ICECONVEXHULL_H

	// Forward declarations
	struct SurfaceInterface;

	//! Convex hull creation structure
	struct MESHMERIZER_API CONVEXHULLCREATE
	{
		//! Constructor
						CONVEXHULLCREATE()	{ ZeroMemory(this, SIZEOFOBJECT); }

		udword			NbVerts;		//!< Number of source vertices
		const Point*	Vertices;		//!< List of source vertices
		float			Scale;			//!< Artificial hull scale
		bool			UnifyNormals;	//!< true if hull faces orientation must be unified
		bool			PolygonData;	//!< true to extract polygon data
	};

	class MESHMERIZER_API HullPolygon : public Allocateable
	{
		public:
		//! Constructor
		inline_			HullPolygon() : mNbVerts(0), mVRef(null), mERef(null)	{}
		inline_			~HullPolygon()											{}

		udword			mNbVerts;		//!< Number of vertices/edges in the polygon
		const udword*	mVRef;			//!< Vertex references
		const udword*	mERef;			//!< Edge references
		Plane			mPlane;			//!< Plane equation for this polygon
#ifdef USE_PRECOMPUTED_HULL_PROJECTION
		float			mMin, mMax;
#endif
	};

	class MESHMERIZER_API ConvexHull : public Allocateable
	{
		public:
											ConvexHull();
		virtual								~ConvexHull();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the convex hull in 3D.
		 *	After the call, get results with the appropriate accessors.
		 *	\param		create	[in] creation structure
		 *	\return		true if success.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				Compute(const CONVEXHULLCREATE& create);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the hull with already existing data.
		 *	\param		surface		[in] surface interface
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			bool				Init(const SurfaceInterface& surface);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks a point is inside the hull.
		 *	\param		p	[in] point in local space
		 *	\return		true if the hull contains the point
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				Contains(const Point& p)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks a sphere is inside the hull.
		 *	\param		sphere	[in] sphere in local space
		 *	\return		true if the hull contains the sphere
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				Contains(const Sphere& sphere)	const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the center of the hull. It should be inside it !
		 *	\param		center	[out] hull center
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool				ComputeGeomCenter(Point& center) const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the polygon whose normal is closest to given witness vector.
		 *	\param		world_witness	[in] witness vector in world space
		 *	\param		world			[in] possible world matrix for the hull, or null
		 *	\return		index of closest polygon
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						udword				SelectClosestPolygon(const Point& world_witness, const Matrix4x4* world=null)	const;
						udword				SelectClosestEdge(const Point& world_witness, const Matrix4x4* world=null, udword* code=null)		const;

						udword				DumpPolyNeighbors(udword poly_index, udword* indices) const;

		// Data access
		inline_			udword				GetNbFaces()			const	{ return mNbHullFaces;		}
		inline_	const	IndexedTriangle16*	GetWordFaces()			const	{ return mWordFaces;		}

		inline_			udword				GetNbVerts()			const	{ return mNbHullVertices;	}
		inline_	const	Point*				GetVerts()				const	{ return mHullVertices;		}
		inline_	const	Point&				GetCenter()				const	{ return mCenter;			}

		inline_			udword				GetNbPolygons()			const
											{
												if(!mNbPolygons)	const_cast<ConvexHull* const>(this)->CreatePolygonData();	// "mutable method"
												return mNbPolygons;
											}
		inline_	const	HullPolygon&		GetPolygon(udword i)	const
											{
												if(!mPolygons)	const_cast<ConvexHull* const>(this)->CreatePolygonData();	// "mutable method"
												return mPolygons[i];
											}
/*		inline_	const	udword*				GetPolygonData()		const
											{
												if(!mPolygonData)	const_cast<ConvexHull* const>(this)->CreatePolygonData();	// "mutable method"
												return mPolygonData;
											}*/

		inline_	const	Vertices*			GetEdgeDirections()		const
											{
												if(!mEdgeDirections)	const_cast<ConvexHull* const>(this)->CreateEdgeDirections();	// "mutable method"
												return mEdgeDirections;
											}

		inline_	udword						GetNbEdges()			const
											{
												if(!mNbEdges)	const_cast<ConvexHull* const>(this)->CreateEdgeList();	// "mutable method"
												return mNbEdges;
											}
		inline_	const Edge*					GetEdges()				const
											{
												if(!mEdges)	const_cast<ConvexHull* const>(this)->CreateEdgeList();	// "mutable method"
												return mEdges;
											}
		inline_	const Point*				GetEdgeNormals()		const
											{
												if(!mEdgeNormals)	const_cast<ConvexHull* const>(this)->CreateEdgeList();	// "mutable method"
												return mEdgeNormals;
											}
		inline_	const EdgeDesc*				GetEdgeToTriangles()	const
											{
												if(!mEdgeToTriangles)	const_cast<ConvexHull* const>(this)->CreateEdgeList();	// "mutable method"
												return mEdgeToTriangles;
											}
		inline_	const udword*				GetFacesByEdges()		const
											{
												if(!mFacesByEdges)	const_cast<ConvexHull* const>(this)->CreateEdgeList();	// "mutable method"
												return mFacesByEdges;
											}
		inline_	const Point*				GetNormals()			const
											{
												if(!mHullNormals)	const_cast<ConvexHull* const>(this)->CreateNormals();	// "mutable method"
												return mHullNormals;
											}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes the number of bytes used by the convex hull.
		 *	\return		number of used bytes
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual			udword				GetUsedRam()		const;

		protected:
		// Triangle data
						udword				mNbHullFaces;			//!< Number of faces in the convex hull
						IndexedTriangle16*	mWordFaces;				//!< Convex hull faces
		// Reduced hull vertices (else use original cloud)
						udword				mNbHullVertices;		//!< Number of vertices in the convex hull
						Point*				mHullVertices;			//!< Convex hull vertices
						Point*				mHullNormals;			//!< Convex hull normals
						Point				mCenter;				//!< Precomputed hull center
		// Polygon data
						udword				mNbPolygons;			//!< Number of planar polygons composing the hull
						HullPolygon*		mPolygons;				//!< Array of mNbPolygons structures
						udword*				mVertexData;			//!< Vertex indices indexed by hull polygons
						udword*				mEdgeData;				//!< Edge indices indexed by hull polygons
		// Edge data
						Vertices*			mEdgeDirections;		//!< Edge directions
						udword				mNbEdges;
						Edge*				mEdges;
						Point*				mEdgeNormals;
						EdgeDesc*			mEdgeToTriangles;
						udword*				mFacesByEdges;
		// Internal methods
						bool				CreatePolygonData();
						bool				CreateEdgeDirections();
						bool				CreateEdgeList();
						bool				CreateNormals();
						bool				CheckHullPolygons()	const;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Analyses a convex hull made of triangles and extracts polygon data out of it.
	 *	\relates	ConvexHull
	 *	\fn			ExtractHullPolygons(Container& polygon_data, const ConvexHull& hull)
	 *	\param		nb_polygons		[out] number of extracted polygons
	 *	\param		polygon_data	[out] polygon data: (Nb indices, index 0, index 1... index N)(Nb indices, index 0, index 1... index N)(...)
	 *	\param		hull			[in] convex hull
	 *	\return		true if success
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION MESHMERIZER_API bool ExtractHullPolygons(udword& nb_polygons, Container& polygon_data, const ConvexHull& hull);

#endif	// ICECONVEXHULL_H

