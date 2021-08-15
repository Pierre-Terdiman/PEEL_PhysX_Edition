///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a multi surface class. This is a complex tri-surface, completed with multiple rendering properties.
 *	\file		IceRenderSurface.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERSURFACE_H
#define ICERENDERSURFACE_H

	enum SurfaceType
	{
		SURFACE_UVS			= 0x1000,		//!< Texture surface
		SURFACE_RGB			= 0x2000,		//!< Color surface
		SURFACE_ALPHA		= 0x4000,		//!< Alpha surface
		SURFACE_LIGHT		= 0x8000,		//!< Lightmap surface
		SURFACE_DETAIL		= 0x8001,		//!< Detail surface

		SURFACE_FORCE_DWORD	= 0x7fffffff
	};

	typedef	udword	SmoothingGroups;

	// A complex render surface
	class MESHMERIZER_API MultiSurface : public IndexedSurface
	{
		public:
														MultiSurface();
		virtual											~MultiSurface();
		// Surface init
									bool				Init(udword nb_faces, udword nb_verts, const Point* verts=null, bool matid=true, bool smg=true);
		virtual						IndexedSurface*		AddExtraSurface(SurfaceType type, const IndexedSurface* source=null);
		virtual						IndexedSurface*		AddConstantAlphaSurface(float opacity);
		virtual						bool				SetDefaultFaceProperties();
		virtual						bool				SetAttributes(KID matid=INVALID_KID, SmoothingGroups smg=1);
		// Surface destruction
		virtual						bool				Clean();
		virtual						bool				DiscardExtraSurface(SurfaceType type);
		virtual						bool				DiscardExtraSurfaces();
		virtual						bool				DiscardFaceProperties();
			// Resets the surface
		override(VertexCloud)		bool				Reset();

		// Surface operations
			// Keeps the face! (french humour)
		override(IndexedSurface)	bool				KeepFaces(const Container& faces);
			// Optimization
		override(IndexedSurface)	bool				Optimize();
		override(IndexedSurface)	bool				WeldVertices(float threshold=0.1f);
			// Subdivision
		override(IndexedSurface)	bool				Subdivide(SubdivScheme& scheme, bool use_cache=true, SubdivHistory* history=null);
			// Merge a surface with another (not a boolean operation)
									bool				Merge(const MultiSurface* surface, const Matrix4x4* world=null, bool extra_surfaces=true);

			// Flips the surface
		override(IndexedSurface)	bool				Flip();

			// Smoothing groups
		virtual						bool				SetUniqueSmoothingGroup();
		virtual						bool				SetNullSmoothingGroup();
#ifdef OBSOLETE
		// Surface compression
		virtual						bool				Compress(const char* filename);
		virtual						bool				Decompress(const char* filename);
#endif
		// Default surfaces
		virtual						bool				MakePlane(udword nbu=2, udword nbv=2, bool uvs=false, Quad* fit=null);

		// Data access
									IndexedSurface*		GetExtraSurface(SurfaceType type)	const;
		inline_						udword				GetNbSurfaces()						const	{ return mNbSurfaces;		}
		inline_						IndexedSurface**	GetExtraSurfaces()					const	{ return mExtraSurfaces;	}
		inline_						KID*				GetMaterialIDs()					const	{ return mMatID;			}
		inline_						SmoothingGroups*	GetSmoothingGroups()				const	{ return mSmoothingGroups;	}

		// Stats
				udword				GetObjectSize()		const;
				udword				GetOwnedSize()		const;
				udword				GetUsedRam()		const;

		private:
		// Extra surfaces (UVs, second UVs, vertex colors...)
									udword				mNbSurfaces;		//!< Number of extra surfaces
									IndexedSurface**	mExtraSurfaces;		//!< List of extra surfaces
									SurfaceType*		mSTypes;			//!< List of surface types
		protected:
		// Face properties
									KID*				mMatID;				//!< List of material IDs
									SmoothingGroups*	mSmoothingGroups;	//!< List of smoothing groups
		private:
		// Internal methods
									bool				FixFaceProperties(udword nb_faces);
	};

#endif // ICERENDERSURFACE_H
