///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef ZCB2_RENDER_DATA_H
#define ZCB2_RENDER_DATA_H

#include ".\ZCB\PINT_ZCB2.h"
#include "TextureManager.h"

	// "Render data chunks" or "render sources" are exported render-related information, used when
	// the collision shapes and the renderer shapes don't match.
	//
	// Most of the time there is a simple mapping/relation between both: for example we create a
	// render shape (e.g. PintDLSphereShapeRenderer) for a collision sphere, and this can be derived
	// implicitly at load time, so there's no need to save explicit render data in the ZB2 file.
	//
	// Sometimes though a complex object can have a triangle mesh (or several!) on the rendering side
	// and a single convex shape for collisions (e.g. a small Lego car). In this case the convex
	// collision shape alone is not enough: if we wouldn't export additional render data, the system
	// would create a simple convex renderer from the convex collision shape, and it would render one
	// flat-shaded convex on screen (the collision shape itself) instead of a Lego car.
	//
	// Beyond that, there is also a need to decouple the render data chunks below from the PintShapeRenderer
	// classes - we cannot directly export the PintShapeRenderer classes. The reason for that is because
	// the shape renderer classes depend on which render model is selected, and there is no guarantee
	// that a box for example is indeed seen as a box on the rendering side (it could be for example a
	// regular triangle mesh to improve batching, etc). Also some shape renderers don't copy the
	// mesh data, they only capture it in an opaque display list or equivalent.

	// See notes for FourCCType.
	enum RenderDataFourCCType
	{
		RenderDataType			= 'RD00',
		SphereRenderDataType	= 'RD01',
		CapsuleRenderDataType	= 'RD02',
		CylinderRenderDataType	= 'RD03',
		BoxRenderDataType		= 'RD04',
		ConvexRenderDataType	= 'RD05',
		MeshRenderDataType		= 'RD06',
		ColorRenderDataType		= 'RD07',
		RenderCollectionType	= 'RD08',
		ManagedTextureType		= 'RD09',
	};

	///////////////////////////////////////////////////////////////////////////

	class RenderDataChunk : public BaseChunk
	{
		DECLARE_CHUNK(RenderDataChunk, mRenderDataCore)

		virtual	RenderDataChunk*		GetChildRenderChunk()			{ return null;	}
		virtual	const ManagedTexture*	GetManagedTexture()		const	{ return null;	}

//		virtual	void					RenderExport(ZCB2Exporter& database, udword& render_id);
	};

	// Render-data chunks for basic shapes (spheres / capsules / etc) aren't really needed, we don't need to export them,
	// they can be recreated implicitly. Still kept below in case we need them at some point.

	class SphereRenderDataChunk : public RenderDataChunk
	{
		DECLARE_CHUNK(SphereRenderDataChunk, mSphereRenderDataCore)
		SphereRenderDataChunk(float radius);

		float	mRadius;
	};

	class CapsuleRenderDataChunk : public RenderDataChunk
	{
		DECLARE_CHUNK(CapsuleRenderDataChunk, mCapsuleRenderDataCore)
	};

	class CylinderRenderDataChunk : public RenderDataChunk
	{
		DECLARE_CHUNK(CylinderRenderDataChunk, mCylinderRenderDataCore)
	};

	class BoxRenderDataChunk : public RenderDataChunk
	{
		DECLARE_CHUNK(BoxRenderDataChunk, mBoxRenderDataCore)
	};

	class ConvexRenderDataChunk : public RenderDataChunk
	{
		DECLARE_CHUNK(ConvexRenderDataChunk, mConvexRenderDataCore)
	};

	enum MeshRenderDataChunkFlags
	{
		MESH_RENDER_DATA_CHUNK_FLAG_ACTIVE_EDGES	= (1<<0),
		MESH_RENDER_DATA_CHUNK_FLAG_DIRECT_DATA		= (1<<1),
	};

	class MeshRenderDataChunk : public RenderDataChunk
	{
		DECLARE_CHUNK(MeshRenderDataChunk, mMeshRenderDataCore)
		MeshRenderDataChunk(const MultiSurface& multi_surface, bool active_edges, bool direct_data);
		MeshRenderDataChunk(const SurfaceInterface& surface, bool active_edges, bool direct_data);

		DECLARE_STD_FLAG(ActiveEdges,	MESH_RENDER_DATA_CHUNK_FLAG_ACTIVE_EDGES)
		DECLARE_STD_FLAG(DirectData,	MESH_RENDER_DATA_CHUNK_FLAG_DIRECT_DATA)

//		MESHChunk	mMeshData;

		// TODO: consider using a mesh chunk for compatibility with ICE
		DECLARE_SUBCHUNK(Vertices,		PNTSChunk)
		DECLARE_SUBCHUNK(UVs,			PNTSChunk)
		DECLARE_SUBCHUNK(Faces,			FACEChunk)
		DECLARE_SUBCHUNK(TFaces,		FACEChunk)
	};

	class ColorRenderDataChunk : public RenderDataChunk
	{
		DECLARE_CHUNK(ColorRenderDataChunk, mColorRenderDataCore)
		ColorRenderDataChunk(RenderDataChunk* source, const RGBAColor& color, const ManagedTexture* texture);

		virtual	RenderDataChunk*		GetChildRenderChunk()			{ return mSource;	}
//		virtual	const ManagedTexture*	GetManagedTexture()		const	{ return &mTexture;	}
		virtual	const ManagedTexture*	GetManagedTexture()		const	{ return mTexture;	}

//		virtual	void					RenderExport(ZCB2Exporter& database, udword& render_id);

		RenderDataChunk*		mSource;
//		ManagedTexture			mTexture;
		const ManagedTexture*	mTexture;
		RGBAColor				mColor;
		udword					mSourceID;
		udword					mTextureID;
	};

//	class PintRendererCollection;
	class RenderDataChunkCollection : public RenderDataChunk
	{
		DECLARE_CHUNK(RenderDataChunkCollection, mCollectionCore)

		PtrContainer			mRenderDataChunks;
//		PintRendererCollection*	mCollection;
		Container				mLocalPoses;
	};

	enum ManagedTextureChunkFlag
	{
		MANAGED_TEXTURE_CHUNK_FLAG_COMPRESSED	= (1<<0),
	};

	class ManagedTextureChunk : public BaseChunk
	{
									DECLARE_CHUNK(ManagedTextureChunk, mManagedTextureCore)
		// Data access
		inline_	bool				IsTextureCompressed()	const	{ return (mManagedTextureCore.mFlags & MANAGED_TEXTURE_CHUNK_FLAG_COMPRESSED)!=0;	}
		inline_	const Picture*		GetBitmap()				const	{ return mBitmap;																	}

				bool				SetSourceBitmap(const Picture& pic);
		private:
				Picture*			mBitmap;
				ubyte*				mCompressed;
				udword				mWidth, mHeight, mCompressedSize;
	};

	///////////////////////////////////////////////////////////////////////////

	class ZCB2FactoryEx : public ZCB2Factory
	{
		public:
									ZCB2FactoryEx();

		virtual	bool				NewChunk(const BaseChunk* chunk);
		virtual	void				NewUnknownChunk(udword type, const char* name, const VirtualFile& file);
		virtual	PintShapeRenderer*	GetShapeRenderer(udword id);

		virtual	void				ReleasePintIndependentData();

				struct LegacyData
				{
					udword			mZB2ID;
				};

				struct MaterialData : LegacyData
				{
					RGBAColor		mDiffuseColor;
					sdword			mTextureID;
				};
				Container			mImportHelper_MaterialData;

				PtrContainer		mImportHelper_Renderers;
//				PtrContainer		mImportHelper_Textures;
				struct TextureData : LegacyData
				{
					const ManagedTexture*	mTexture;
				};
				Container			mImportHelper_TextureData;
//				float				mGlobalScale;

				bool				NewScene(const SCENChunk* scene);
				bool				NewFreeCamera(const FCAMChunk* camera);
				bool				NewTargetCamera(const TCAMChunk* camera);
#ifndef _WIN64	// "temporary"
				bool				NewMaterial(const MATLChunk* material);
				bool				NewMotion(const MOVEChunk* move);
#endif
				bool				NewMesh(const MESHChunk* mesh);
				bool				NewTextureMap(const TXMPChunk* txmp);
				bool				NewPLight(const PLITChunk* light);
				bool				NewSLight(const SLITChunk* light);
				bool				NewDLight(const DLITChunk* light);
				bool				NewBoxGizmo(const BGIZChunk* gizmo);
				bool				NewSphereGizmo(const SGIZChunk* gizmo);
				bool				NewCylinderGizmo(const CGIZChunk* gizmo);
				bool				NewHelper(const HELPChunk* helper);
				bool				NewSkeleton(const SKELChunk* skeleton);
				bool				NewShape(const SHAPChunk* shape);
				bool				NewSampler(const SAMPChunk* sampler);
				bool				NewRotator(const ROTAChunk* rota);

				bool				NewLegacyChunk(const BaseChunk* chunk);
	};

#endif
