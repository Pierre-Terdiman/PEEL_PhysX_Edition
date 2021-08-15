///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for vertex clouds.
 *	\file		IceVertexCloud.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVERTEXCLOUD_H
#define ICEVERTEXCLOUD_H

	// A generic cloud of vertices.
	class MESHMERIZER_API VertexCloud
	{
		public:
									VertexCloud();
									VertexCloud(udword nb_verts, const Point* verts=null);
									VertexCloud(const VertexCloud& cloud);
		virtual						~VertexCloud();
		// Vertex cloud init
				bool				Init(udword nb_verts, const Point* verts=null, bool zero_vertices=true);
		// Vertex cloud construction
				bool				SetVertex(const Point& p, udword i);
				bool				SetVertex(float x, float y, float z, udword i);
		// Vertex cloud destruction
				VertexCloud&		Clean();
		virtual	bool				Reset();
		// Vertex cloud description
				VertexCloud&		ComputeLocalGeomCenter(Point& center);
				VertexCloud&		ComputeLocalAABB(AABB& aabb);
				bool				ComputeLocalBSphere(Sphere& sphere);
#ifdef REMOVED
		virtual	udword				ComputeSupportingVertex(const Point& dir);
				udword				ComputeSupportingVertex(const Point& dir, const Matrix4x4& mat);
#endif
//				VertexCloud&		ComputeMaxVector(Point& max);
//				VertexCloud&		ComputeMinVector(Point& max);

				bool				ComputeVariance(Matrix3x3& result);

		// Vertex cloud operations
		virtual	bool				Translate(const Point& translation);
		virtual	bool				Scale(const Point& scale);
		inline_	bool				Scale(float factor)			{ return Scale(Point(factor, factor, factor));	}
		virtual	bool				Transform(const Matrix4x4& mat);

				VertexCloud&		Spherify(const Point& center, float radius);
				bool				Quantize(udword nb, sword* quantized, float& dequant_coeff_x, float& dequant_coeff_y, float& dequant_coeff_z);
				bool				Reduce(udword* xref_copy=null);
				bool				Weld(float threshold, udword* xref_copy=null);
		virtual	bool				Optimize(udword* xref_copy=null);

		// Data access
		inline_	udword				GetNbVerts()		const	{ return mNbVerts;		}
		inline_	const Point*		GetVerts()			const	{ return mVerts;		}
		inline_	Point*				GetVerts()					{ return mVerts;		}
		inline_	const Point*		GetVertex(udword i)	const	{ return &mVerts[i];	}

		// Stats
				udword				GetObjectSize()		const;
				udword				GetOwnedSize()		const;
				udword				GetUsedRam()		const;

		protected:
		// The vertex cloud
				udword				mNbVerts;	//!< Number of vertices
				Point*				mVerts;		//!< List of vertices
		private:
		//
				bool				FindWeldedVertices(float* axislist, Point* centers, float commonradius, udword nbspheres, Container& welded);
	};



	//! Vertex cloud reduction result structure
	struct MESHMERIZER_API REDUCEDCLOUD
	{
				// Out
				Point*				RVerts;		//!< Reduced list
				udword				NbRVerts;	//!< Reduced number of vertices
				udword*				CrossRef;	//!< nb_verts remapped indices
	};

	class MESHMERIZER_API ReducedVertexCloud : public Allocateable
	{
		public:
		// Constructors/destructor
									ReducedVertexCloud(const VertexCloud* vcloud);
									ReducedVertexCloud(const Point* verts, udword nb_verts);
									ReducedVertexCloud(const Vertices& v);
									~ReducedVertexCloud();
		// Free used bytes
				ReducedVertexCloud&	Clean();
		// Cloud reduction
				bool				Reduce(REDUCEDCLOUD* rc=null);
				bool				Reduce2(REDUCEDCLOUD* rc=null);
		// Data access
		inline_	udword				GetNbVerts()				const	{ return mNbVerts;		}
		inline_	udword				GetNbReducedVerts()			const	{ return mNbRVerts;		}
		inline_	const Point*		GetReducedVerts()			const	{ return mRVerts;		}
		inline_	const Point&		GetReducedVertex(udword i)	const	{ return mRVerts[i];	}
		inline_	const udword*		GetCrossRefTable()			const	{ return mXRef;			}

		private:
		// Original vertex cloud
				udword				mNbVerts;	//!< Number of vertices
				const Point*		mVerts;		//!< List of vertices (pointer copy)

		// Reduced vertex cloud
				udword				mNbRVerts;	//!< Reduced number of vertices
				Point*				mRVerts;	//!< Reduced list of vertices
				udword*				mXRef;		//!< Cross-reference table (used to remap topologies)
	};

#endif // ICEVERTEXCLOUD_H

