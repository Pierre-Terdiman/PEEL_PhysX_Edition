///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an indexed surface class.
 *	\file		IceIndexedSurface.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEINDEXEDSURFACE_H
#define ICEINDEXEDSURFACE_H

	// Forward declarations
	class SubdivScheme;
	class SubdivHistory;
	struct PolygonNormalMask;
	struct EdgeDesc;
	class Valencies;
	class Adjacencies;
	class StabbedFace;

	//! Indexed surface flags
	enum SurfaceFlag
	{
		SF_GEOM_CENTER		= (1<<0),		//!< Geometric center is valid
		SF_GEOM_CENTER_MODE	= (1<<1),		//!< Geometric center computation mode
		SF_AABB				= (1<<2),		//!< AABB is valid
		SF_OBB				= (1<<3),		//!< OBB is valid
		SF_BSPHERE			= (1<<4),		//!< BoundingSphere is valid
		SF_VALENCIES		= (1<<5),		//!< Valencies are valid
		SF_VALENCIES_MODE	= (1<<6),		//!< Valencies computation mode
		SF_ADJACENCY		= (1<<7),		//!< Adjacencies are valid
		SF_NORMAL_MASKS		= (1<<8),		//!< Normal masks are valid
		SF_EDGE_LIST		= (1<<9),		//!< Normal masks are valid

		SF_GEOMETRY			= SF_GEOM_CENTER|SF_AABB|SF_OBB|SF_BSPHERE|SF_EDGE_LIST,	//!< All geometry-related flags
		SF_TOPOLOGY			= SF_VALENCIES|SF_ADJACENCY|SF_NORMAL_MASKS|SF_EDGE_LIST,	//!< All topology-related flags

		SF_FORCE_DWORD		= 0x7fffffff
	};

	//! Shooting methods for in-out tests
	enum ShootingMethod
	{
		SHOOT_X,			//!< Shoot along X
		SHOOT_Y,			//!< Shoot along Y
		SHOOT_Z,			//!< Shoot along Z
		SHOOT_RANDOM,		//!< Shoot along a random direction
		SHOOT_STATISTICS,	//!< Shoot multiple random rays and keep most probable answer

		SHOOT_FORCE_DWORD	= 0x7fffffff,
	};

	// An indexed triangle surface
	class MESHMERIZER_API IndexedSurface : public VertexCloud
	{
		public:
										IndexedSurface();
										IndexedSurface(const IndexedSurface& surface);
		virtual							~IndexedSurface();
		// Surface initialization
				bool					Init(udword nb_faces, udword nb_verts, const Point* verts=null, const IndexedTriangle* faces=null, bool zero_vertices=true);

		// Surface construction

			// Adds a face to the surface
				bool					SetFace(udword ref0, udword ref1, udword ref2, udword i);
				bool					CreateDefaultTopology();
				bool					HasDefaultTopology();

			// Sets new vertices - all at once
		inline_	void					SetVertices(const Point* verts)
										{
											CopyMemory(mVerts, verts, mNbVerts*sizeof(Point));	// Copy new vertex cloud
											InvalidateGeometry();								// Invalidate old data
										}

			// Sets new AABB - ex: can be useful for skinning, when an external process is able
			// to provide a new set of vertices and the new AABB as well.
		inline_	void					SetLocalAABB(const AABB& aabb)
										{
											mLocalAABB = aabb;	// Copy new box
											mFlags|=SF_AABB;	// Mark it as ok
										}
		// Surface destruction

			// Releases everything
				IndexedSurface&			Clean();
			// Resets the surface
		override(VertexCloud)	bool	Reset();
			// Releases the adjacency graph
				bool					DeleteAdjacencies();
			// Releases the edge list
				bool					DeleteEdgeList();
			// Releases the valencies
				bool					DeleteValencies();
			// Releases the surface
				bool					DeleteSurface();
			// Invalidates the surface
		inline_	void					InvalidateGeometry()	{ mFlags&=~SF_GEOMETRY;	}
		inline_	void					InvalidateTopology()	{ mFlags&=~SF_TOPOLOGY;	}

		// Surface description

			// Computes the supporting vertex using hill-climbing
		virtual	udword					ComputeConvexSupportingVertex(const Point& dir, udword* id=null, udword* time=null, udword* stamps=null);
			// Helper-function returning a supporting vertex
		inline_	const Point*			Support(const Point& dir, udword* id=null)
										{
											udword CSV = ComputeConvexSupportingVertex(dir, id);
											return GetVertex(CSV);
										}
			// Gets vertices boundary status
				bool					GetBoundaryVertices(bool* bound_status);
			// Extreme edge lengths
				float					ComputeMinEdgeLength()	const;
				float					ComputeMaxEdgeLength()	const;
			// Convex edges
				bool					ComputeConvexEdges(Pairs& active_pairs, float epsilon=0.001f);

		// Surface operations

			// Translates the surface
		override(VertexCloud)	bool	Translate(const Point& translation);
			// Scales the surface
		override(VertexCloud)	bool	Scale(const Point& scale);
			// Transforms vertex cloud
		override(VertexCloud)	bool	Transform(const Matrix4x4& mat);

			// Spherifies the surface
		virtual	bool					Spherify(float radius=0.0f);
			// Smoothes the surface
		virtual	bool					Smooth(bool lockboundaries=false, SubdivHistory* history=null, const Valencies* valencies=null);
			// Remaps faces
		virtual	bool					Remap(udword* xref);
			// Finds invalid faces
				bool					FindInvalidFaces(bool kill_zero_area, Container* killed_faces=null);
			// Keeps the face! (french humour)
		virtual	bool					KeepFaces(const Container& faces);
			// Discards provided faces
				bool					RemoveFaces(const Container& faces);
			// Discards useless vertices
				bool					RemoveUnusedVertices(udword* nb_removed_vertices=null);
			// Makes the mesh manifold
				bool					RemoveSingularEdges();
			// Optimizes the surface
		virtual	bool					Optimize();
			// Welds nearby vertices
		virtual	bool					WeldVertices(float threshold=0.1f);
			// Subdivides the surface
		virtual	bool					Subdivide(SubdivScheme& scheme, bool use_cache=true, SubdivHistory* history=null);
			// Unfolds the surface
				bool					Unfold();
			// Merges a surface with another (not a boolean operation)
				bool					Merge(const IndexedSurface* surface, const Matrix4x4* world=null);

		virtual	bool					UnifyNormals(bool test_without_fix=false);

			// Flips the surface
		virtual	bool					Flip();

#ifdef OBSOLETE
		// Surface compression

			// Compresses the surface
//				bool					Compress();
		virtual	bool					Compress(CustomArray* array, Adjacencies* adj);
		virtual	bool					Decompress(CustomArray* array);
//				void					CompressVertex(udword link, udword prevface, udword prevedge, PackInfo& info);
//				void					DecompressVertex(udword prevface, udword prevedge, DepackInfo& info);
#endif
		// Surface interaction

			// Stabs & counts
		virtual	udword					CountIntersections(const Point& p, const Point& dir)					const;
			// Checks a point is inside the surface
				bool					Contains(const Point& p, ShootingMethod method=SHOOT_RANDOM)			const;
			// Returns a list of sampled points within the surface
				udword					GetInsideDots(udword density, ShootingMethod method, Vertices& list)	const;
			// Returns a list of sampled random points on the surface's hull
				bool					GetRandomHullPoints(udword nb_points, Vertices& list)					const;
			// Returns the minimum distance from a point to the surface
		virtual	bool					ComputeDistance(const Point& local_pt, StabbedFace& face)				const;

		// Default surfaces

			// Makes a plane
		virtual	bool					MakePlane(udword nbu=2, udword nbv=2, Quad* fit=null);

		// Data access
		inline_	udword					GetNbFaces()		const	{ return mNbFaces;				}
		inline_	const IndexedTriangle*	GetFaces()			const	{ return mFaces;				}
		inline_	IndexedTriangle*		GetFaces()					{ return mFaces;				}
		inline_	const IndexedTriangle*	GetFace(udword i)	const	{ return &mFaces[i];			}
		inline_	EdgeDesc*				GetNormalMasksMap()	const	{ return mPNMap;				}

		inline_	void					GetTriangle(udword index, Triangle& triangle, const Matrix4x4* transform=null)	const
										{
											const IndexedTriangle& Face = mFaces[index];
											triangle.mVerts[0] = mVerts[Face.mRef[0]];
											triangle.mVerts[1] = mVerts[Face.mRef[1]];
											triangle.mVerts[2] = mVerts[Face.mRef[2]];
											if(transform)
											{
												triangle.mVerts[0] *= *transform;
												triangle.mVerts[1] *= *transform;
												triangle.mVerts[2] *= *transform;
											}
										}

		inline_	void					GetTriangle(udword index, Triangle& triangle, const Matrix4x4& transform)	const
										{
											const IndexedTriangle& Face = mFaces[index];
											triangle.mVerts[0] = mVerts[Face.mRef[0]] * transform;
											triangle.mVerts[1] = mVerts[Face.mRef[1]] * transform;
											triangle.mVerts[2] = mVerts[Face.mRef[2]] * transform;
										}

		// Data access with lazy evaluation
		inline_	const AABB&				GetSurfaceLocalAABB()					const
										{
											if(!(mFlags&SF_AABB))	const_cast<IndexedSurface* const>(this)->ComputeLocalAABB();	// "mutable method"
											return mLocalAABB;
										}
		inline_	const OBB&				GetSurfaceLocalOBB()					const
										{
											if(!(mFlags&SF_OBB))	const_cast<IndexedSurface* const>(this)->ComputeLocalOBB();	// "mutable method"
											return mLocalOBB;
										}
		inline_	const Sphere&			GetSurfaceLocalBSphere()				const
										{
											if(!(mFlags&SF_BSPHERE))	const_cast<IndexedSurface* const>(this)->ComputeLocalBSphere();	// "mutable method"
											return mLocalBSphere;
										}
		inline_	const Adjacencies*		GetAdjacencies()						const
										{
											if(!(mFlags&SF_ADJACENCY))	const_cast<IndexedSurface* const>(this)->ComputeAdjacencies();	// "mutable method"
											return mAdj;
										}
		inline_	PolygonNormalMask*		GetNormalMasks(udword nbsubdiv)			const
										{
											if(!(mFlags&SF_NORMAL_MASKS))	const_cast<IndexedSurface* const>(this)->ComputeNormalMasks(nbsubdiv);	// "mutable method"
											return mPNM;
										}
		inline_	const Point&			GetLocalGeomCenter(bool method=true)	const
										{
											if(!(mFlags&SF_GEOM_CENTER) || method!=((mFlags&SF_GEOM_CENTER_MODE)!=0))	const_cast<IndexedSurface* const>(this)->ComputeLocalGeomCenter();	// "mutable method"
											return mLocalGeomCenter;
										}
		inline_	const Valencies*		GetValencies(bool adjlist=true)			const
										{
											if(!(mFlags&SF_VALENCIES) || adjlist!=((mFlags&SF_VALENCIES_MODE)!=0))	const_cast<IndexedSurface* const>(this)->ComputeValencies();	// "mutable method"
											return mValencies;
										}
		inline_	const EdgeList*			GetEdgeList()							const
										{
											if(!(mFlags&SF_EDGE_LIST))	const_cast<IndexedSurface* const>(this)->ComputeEdgeList();	// "mutable method"
											return mEdgeList;
										}
		// Stats
				udword					GetObjectSize()		const;
				udword					GetOwnedSize()		const;
				udword					GetUsedRam()		const;

		// Interface
		inline_	SurfaceInterface		GetSurfaceInterface()	const	{ return SurfaceInterface(mNbVerts, mVerts, mNbFaces, (const udword*)mFaces, null);	}

		protected:
		// Topology
				udword					mNbFaces;			//!< Number of triangles
				IndexedTriangle*		mFaces;				//!< List of indexed triangles
		// Surface description
				udword					mFlags;				//!< Surface flags
		private:
		// Lazy evaluated surface attributes must be kept private
			// Geometry-related
				Point					mLocalGeomCenter;	//!< Geometric center or centroid, in local space
				AABB					mLocalAABB;			//!< Axis-aligned bounding box in local space
				OBB						mLocalOBB;			//!< Oriented bounding box in local space
				Sphere					mLocalBSphere;		//!< Minimal bounding sphere in local space
			// Topology-related
				Valencies*				mValencies;			//!< Surface valencies
				Adjacencies*			mAdj;				//!< Surface adjacencies
				EdgeList*				mEdgeList;			//!< Edge list

				PolygonNormalMask*		mPNM;				//!< Polygon Normal Masks
				EdgeDesc*				mPNMap;				// .........

		// Internal methods
		private:
			// Computes the geometric center
				bool					ComputeLocalGeomCenter(bool method=true);
			// Computes the AABB
				bool					ComputeLocalAABB();
			// Computes the OBB
				bool					ComputeLocalOBB();
			// Computes the bounding sphere
				bool					ComputeLocalBSphere();
			// Computes the valencies
				bool					ComputeValencies(bool adjlist=true);
			// Computes the adjacency graph
				bool					ComputeAdjacencies();
			// Computes the edge list
				bool					ComputeEdgeList();
			// Computes the normal masks
				bool					ComputeNormalMasks(udword nbsubdiv);
			// Recursive methods
				bool					_WeldFace(udword curface, udword prevface, udword crossededge, udword prevedge, Point* newverts, IndexedTriangle* newfaces, bool* markers);
	};

#endif // ICEINDEXEDSURFACE_H
