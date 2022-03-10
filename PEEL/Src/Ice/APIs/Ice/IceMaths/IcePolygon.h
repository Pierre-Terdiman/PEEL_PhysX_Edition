///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a polygon class.
 *	\file		IcePolygon.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPOLYGON_H
#define ICEPOLYGON_H

	class ICEMATHS_API Polygon : public Allocateable
	{
		public:
								Polygon();
								Polygon(udword nb_verts);
								Polygon(const Polygon& polygon);
								~Polygon();

				bool			SetVerts(udword nb_verts, const Point* verts=null, const Matrix4x4* world=null);

		// Data access
		inline_	udword			GetNbVerts()	const	{ return mNbVerts;	}
		inline_	Point*			GetVerts()				{ return mVerts;	}
		inline_	const Point*	GetVerts()		const	{ return mVerts;	}

		protected:
				udword			mNbVerts;
				Point*			mVerts;
	};

	class ICEMATHS_API IndexedPolygon : public Allocateable
	{
		public:
								IndexedPolygon();
								IndexedPolygon(udword nb_verts, udword* indices);
								IndexedPolygon(const IndexedPolygon& polygon);
								~IndexedPolygon();

		// Data access
		inline_	udword			GetNbVerts()	const	{ return mNbVerts;	}
		inline_	const udword*	GetIndices()	const	{ return mIndices;	}

		protected:
				udword			mNbVerts;
				udword*			mIndices;
	};

	enum Orientation
	{
		ORIENTATION_XY,
		ORIENTATION_XZ,
		ORIENTATION_YZ,

		ORIENTATION_FORCE_DWORD = 0x7fffffff
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Generates a planar polygon. The vertices are generated on a circle of origin = O(0,0,0) and radius = amplitude.
	 *	\fn			GeneratePolygon(udword nb_verts, void* verts, udword stride, Orientation orientation, float amplitude, float phase)
	 *	\relates	Polygon
	 *	\param		nb_verts	[in] number of vertices
	 *	\param		verts		[out] generated vertices
	 *	\param		stride		[in] stride between two vertices, in bytes
	 *	\param		orientation	[in] orientation of polygon
	 *	\param		amplitude	[in] amplitude/scale of polygon
	 *	\param		phase		[in] phase for vertex generation
	 *	\return		true if success
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ICEMATHS_API	bool	GeneratePolygon(udword nb_verts, void* verts, udword stride, Orientation orientation, float amplitude, float phase=0.0f);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Finds the plane equation of a polygon.
	 *	\fn			CreatePolygonPlane(Plane& plane, udword nb_verts, const udword* indices, const Point* verts)
	 *	\relates	Polygon
	 *	\param		plane		[out] plane equation
	 *	\param		nb_verts	[in] number of vertices
	 *	\param		indices		[in] array of vertex indices
	 *	\param		verts		[in] array of vertices
	 *	\return		true if success
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ICEMATHS_API	bool	CreatePolygonPlane(Plane& plane, udword nb_verts, const udword* indices, const Point* verts);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Computes the plane equation of a polygon using Newell's method
	 *	\fn			ComputeNewellPlane(Plane& plane, udword nb_verts, const udword* indices, const Point* verts)
	 *	\relates	Polygon
	 *	\param		plane		[out] plane equation
	 *	\param		nb_verts	[in] number of vertices
	 *	\param		indices		[in] array of vertex indices (or null)
	 *	\param		verts		[in] array of vertices
	 *	\return		true if success
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ICEMATHS_API	bool	ComputeNewellPlane(Plane& plane, udword nb_verts, const udword* indices, const Point* verts);
	ICEMATHS_API	bool	ComputeNewellPlane2(Plane& plane, udword nb_verts, const ubyte* indices, const Point* verts);

#endif // ICEPOLYGON_H
