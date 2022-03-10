///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a consolidated surface.
 *	\file		IceConsolidatedSurface.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECONSOLIDATEDSURFACE_H
#define ICECONSOLIDATEDSURFACE_H

	//! Consolidated surface creation structure
	struct ICERENDERER_API RENDERABLESURFACECREATE
	{
								RENDERABLESURFACECREATE();
			// Surfaces
			IndexedSurface*		mTopoSurface;		//!< Input surface [compulsory]
			IndexedSurface*		mUVSurface;			//!< Input surface, or null. Must have the same number of faces as the toposurface.
			IndexedSurface*		mColorSurface;		//!< Input surface, or null. Must have the same number of faces as the toposurface.
			IndexedSurface*		mAlphaSurface;		//!< Input surface, or null. Must have the same number of faces as the toposurface.
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
			IndexedSurface*		mUVSurface2;		//!< Input surface, or null. Must have the same number of faces as the toposurface.
#endif
			// Face properties
			udword*				mDMatID;			//!< List of material IDs, or null. One udword / face.
			uword*				mWMatID;			//!< List of material IDs, or null. One uword / face.
			udword*				mSMG;				//!< List of smoothing groups, or null. One udword / face.
			IndexSize			mIndexSize;			//!< 16 or 32-bits indices. In doubt, don't initialize this.
			// Misc
			Renderer*			mRenderer;			//!< The renderer
#ifdef SUPPORT_COLORS
			udword*				mColors;			//!< Precomputed lighting, or null
			udword				mNbColors;			//!< Number of colors if precomputed lighting is used
#endif
			// Consolidation settings
			bool				mVertexNormals;		//!< Compute vertex normals
			bool				mNormalInfo;		//!< Compute normal information
			bool				mIndexed;			//!< Use indexed data
			bool				mIsSkin;			//!< Override some settings for skins
			bool				mParity;			//!< Surface parity
			// Renderable settings
			bool				mIsStatic;			//!< Override some settings for static meshes
			bool				mUseStrips;			//!< Use strips if possible
	};

	//! A consolidated surface.
	class ICERENDERER_API ConsolidatedSurface
	{
		public:
										ConsolidatedSurface();
		virtual							~ConsolidatedSurface();

		// Data access
		inline_	const Consolidation*	GetRenderInfos() const
										{
											if(!mData)
											{
												// Lazy-consolidation
												ConsolidatedSurface* const FakeThis = const_cast<ConsolidatedSurface* const>(this);	// "mutable method"
												RENDERABLESURFACECREATE params;
												// Catch parameters
												if(!FakeThis->GetRenderSource(params))	return null;
												// Perform mesh conversion
												FakeThis->Consolidate(params);
											}
											return mData;
										}
		// Creation
		virtual	void					GetTextureTransform(TextureTransform& tf, udword mat_id, udword channel)	= 0;
		virtual	bool					GetRenderSource(RENDERABLESURFACECREATE& params)							= 0;

				void					FreeRenderInfos();

		// Stats
				udword					GetObjectSize()		const;
				udword					GetOwnedSize()		const;
				udword					GetUsedRam()		const;

		// Shared data
		static	Renderer*				mRd;			//!< Shared shortcut to the renderer

		protected:

		// Creation
		virtual	bool					Consolidate(const RENDERABLESURFACECREATE& create);

		private:
		// Consolidation data
				Consolidation*			mData;
		// Internal methods
				bool					FillVertexBuffer(const MBResult& consolidation, udword code);
				bool					SetupRenderer(Renderer* renderer);
	};

#endif // ICECONSOLIDATEDSURFACE_H
