///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Zcb2_RenderData.h"
#include "PintShapeRenderer.h"
#include "./LZ4/lz4.h"

// We decouple the shape renderers from the exported render data because of the render models. A simple box
// can be interpreted either as a unique OpenGL cube captured in a display list, or as a convex object part
// of a batched/instanced stream of similar objects. This all depends on selected render model, so we need
// to export the source render data used to create the shape renderers, rather that the shape renderers
// themselves.
//
// These render-data-related chunks extend the physics-related chunks (see FourCCType) which extended the
// ICE native chunks.
//
// We need to export render-data because the collision/physics shapes aren't always the same as the render
// shapes.

// This is currently experimental/hacky so no proper API there, we keep that one under the hood
float gZCB2_RenderScale = 1.0f;

CHECK_CONTAINER_ITEM(ZCB2FactoryEx::TextureData);

///////////////////////////////////////////////////////////////////////////////

#define RENDER_DATA_VERSION	1

RenderDataChunk::RenderDataChunk()
{
	mChunkType					= RenderDataType;
	mRenderDataCore.mVersion	= RENDER_DATA_VERSION;
}

RenderDataChunk::~RenderDataChunk()
{
}

const char* RenderDataChunk::Validate()
{
	return null;
}

bool RenderDataChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mRenderDataCore.Export(array);
	return true;
}

bool RenderDataChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mRenderDataCore.Import(file);
	return true;
}

/*	void RenderDataChunk::RenderExport(ZCB2Exporter& database, udword& render_id)
	{
		if(GetID()==INVALID_ID)
		{
			SetID(render_id++);
			database.RegisterUserDefinedChunk(this, FALSE);
		}
	}*/

///////////////////////////////////////////////////////////////////////////////

#define SPHERE_RENDER_DATA_VERSION	1

SphereRenderDataChunk::SphereRenderDataChunk() :
	mRadius(0.0f)
{
	mChunkType						= SphereRenderDataType;
	mSphereRenderDataCore.mVersion	= SPHERE_RENDER_DATA_VERSION;
}

SphereRenderDataChunk::~SphereRenderDataChunk()
{
}

const char* SphereRenderDataChunk::Validate()
{
	return null;
}

SphereRenderDataChunk::SphereRenderDataChunk(float radius) :
	mRadius(radius)
{
	mChunkType						= SphereRenderDataType;
	mSphereRenderDataCore.mVersion	= SPHERE_RENDER_DATA_VERSION;
}

bool SphereRenderDataChunk::Export(CustomArray& array)
{
	RenderDataChunk::Export(array);
	mSphereRenderDataCore.Export(array);
	array.Store(mRadius);
	return true;
}

bool SphereRenderDataChunk::Import(const VirtualFile& file)
{
	RenderDataChunk::Import(file);
	mSphereRenderDataCore.Import(file);
	mRadius = file.ReadFloat();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define MESH_RENDER_DATA_VERSION	2

MeshRenderDataChunk::MeshRenderDataChunk()
{
	mChunkType						= MeshRenderDataType;
	mMeshRenderDataCore.mVersion	= MESH_RENDER_DATA_VERSION;
}

MeshRenderDataChunk::~MeshRenderDataChunk()
{
}

const char* MeshRenderDataChunk::Validate()
{
	return null;
}

MeshRenderDataChunk::MeshRenderDataChunk(const MultiSurface& multi_surface, bool active_edges, bool direct_data)
{
	mChunkType						= MeshRenderDataType;
	mMeshRenderDataCore.mVersion	= MESH_RENDER_DATA_VERSION;

	SetActiveEdges(active_edges);
	SetDirectData(direct_data);
//		MESHChunk	mMeshData;

	const MultiSurface* CurMsh = &multi_surface;
	GetFaces().GetCore().Enable(ZCB2_FACE_NO_DELTA);
	GetVertices().SetPts(CurMsh->GetNbVerts(), CurMsh->GetVerts());
	GetFaces().SetFaces(CurMsh->GetNbFaces(), (const udword*)CurMsh->GetFaces(), null);

	IndexedSurface* IS = CurMsh->GetExtraSurface(SURFACE_UVS);
	if(IS)
	{
		GetTFaces().GetCore().Enable(ZCB2_FACE_NO_DELTA);
		GetUVs().SetPts(IS->GetNbVerts(), IS->GetVerts());
		GetTFaces().SetFaces(IS->GetNbFaces(), (const udword*)IS->GetFaces(), null);
	}
}

MeshRenderDataChunk::MeshRenderDataChunk(const SurfaceInterface& surface, bool active_edges, bool direct_data)
{
	mChunkType						= MeshRenderDataType;
	mMeshRenderDataCore.mVersion	= MESH_RENDER_DATA_VERSION;

	SetActiveEdges(active_edges);
	SetDirectData(direct_data);
//		MESHChunk	mMeshData;

	GetFaces().GetCore().Enable(ZCB2_FACE_NO_DELTA);
	GetVertices().SetPts(surface.mNbVerts, surface.mVerts);
	GetFaces().SetFaces(surface.mNbFaces, surface.mDFaces, surface.mWFaces);
}

bool MeshRenderDataChunk::Export(CustomArray& array)
{
	RenderDataChunk::Export(array);
	mMeshRenderDataCore.Export(array);
//	mMeshData.Export(array);
	mVertices.Export(array);
	mUVs.Export(array);
	mFaces.Export(array);
	mTFaces.Export(array);
	return true;
}

bool MeshRenderDataChunk::Import(const VirtualFile& file)
{
	RenderDataChunk::Import(file);
	mMeshRenderDataCore.Import(file);
//	mMeshData.Import(file);
	mVertices.Import(file);
	mUVs.Import(file);
	mFaces.Import(file);
	mTFaces.Import(file);

	if(mMeshRenderDataCore.mVersion==1)
	{
		if(mTFaces.GetDFaces())
		{
			udword* Indices32 = const_cast<udword*>(mTFaces.GetDFaces());
			*Indices32=0;
		}
		if(mTFaces.GetWFaces())
		{
			uword* Indices16 = const_cast<uword*>(mTFaces.GetWFaces());
			*Indices16=0;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define COLOR_RENDER_DATA_VERSION	1

ColorRenderDataChunk::ColorRenderDataChunk() :
	mSource		(null),
	mTexture	(null),
	mColor		(0.0f, 0.0f, 0.0f, 1.0f),
	mSourceID	(INVALID_ID),
	mTextureID	(INVALID_ID)
{
	mChunkType						= ColorRenderDataType;
	mColorRenderDataCore.mVersion	= COLOR_RENDER_DATA_VERSION;
}

ColorRenderDataChunk::~ColorRenderDataChunk()
{
}

const char* ColorRenderDataChunk::Validate()
{
	return null;
}

ColorRenderDataChunk::ColorRenderDataChunk(RenderDataChunk* source, const RGBAColor& color, const ManagedTexture* texture) :
	mSource		(source),
	mTexture	(texture),
	mColor		(color),
	mSourceID	(INVALID_ID),
	mTextureID	(INVALID_ID)
{
//	if(texture)
//		mTexture = *texture;
	mChunkType						= ColorRenderDataType;
	mColorRenderDataCore.mVersion	= COLOR_RENDER_DATA_VERSION;
}

bool ColorRenderDataChunk::Export(CustomArray& array)
{
	RenderDataChunk::Export(array);
	mColorRenderDataCore.Export(array);
	array.Store(mColor.R).Store(mColor.G).Store(mColor.B).Store(mColor.A);
	array.Store(mSource ? mSource->GetID() : INVALID_ID);
//	array.Store(mTexture.mExportID);
	array.Store(mTexture ? mTexture->mExportID : INVALID_ID);
	return true;
}

bool ColorRenderDataChunk::Import(const VirtualFile& file)
{
	RenderDataChunk::Import(file);
	mColorRenderDataCore.Import(file);
	mColor.R = file.ReadFloat();
	mColor.G = file.ReadFloat();
	mColor.B = file.ReadFloat();
	mColor.A = file.ReadFloat();
	mSourceID = file.ReadDword();
//	mTexture.mExportID = file.ReadDword();
	mTextureID = file.ReadDword();
	return true;
}

/*	void ColorRenderDataChunk::RenderExport(ZCB2Exporter& database, udword& render_id)
	{
		const ManagedTexture* MT = mTexture;
		if(MT)
		{
			if(MT->mExportID == INVALID_ID)
			{
				if(MT->mSource)	//#### temporary
				{
					MT->mExportID = texture_id++;
					TXMPChunk* txmp = static_cast<TXMPChunk*>(database.CreateChunk('TXMP'));
					// TODO: support external files (filename/URL)
					txmp->SetSourceBitmap(MT->mSource, null, null);
					const char* error = txmp->Validate();
					ASSERT(!error);
				}
			}
		}

		// Takes care of ColorRenderDataChunk. Only deals with one child here.
		RenderDataChunk* ChildChunk = RenderChunk->GetChildRenderChunk();
		if(ChildChunk)
			Export::Chunk(database, ChildChunk, render_id);

		Export::Chunk(database, RenderChunk, render_id);
	}*/

///////////////////////////////////////////////////////////////////////////////

#define RENDER_DATA_COLLECTION_VERSION	1

RenderDataChunkCollection::RenderDataChunkCollection()// : mCollection(null)
{
	mChunkType					= RenderCollectionType;
	mCollectionCore.mVersion	= RENDER_DATA_COLLECTION_VERSION;
}

RenderDataChunkCollection::~RenderDataChunkCollection()
{
}

const char* RenderDataChunkCollection::Validate()
{
	return null;
}

bool RenderDataChunkCollection::Export(CustomArray& array)
{
	RenderDataChunk::Export(array);
	mCollectionCore.Export(array);

/*	ASSERT(mCollection);
	const udword Nb = mCollection->GetNbRenderData();

	udword NbRenderSources = 0;
	for(udword i=0;i<Nb;i++)
	{
		const PintRendererCollection::RenderData* rd = mCollection->GetRenderData(i);
		if(rd && rd->mRenderer->mRenderSource)
			NbRenderSources++;
	}
	array.Store(NbRenderSources);

	for(udword i=0;i<Nb;i++)
	{
		const PintRendererCollection::RenderData* rd = mCollection->GetRenderData(i);
		if(rd && rd->mRenderer->mRenderSource)
		{
			array.Store(rd->mRenderer->mRenderSource->GetID());
			array.Store(rd->mPose.mPos.x);
			array.Store(rd->mPose.mPos.y);
			array.Store(rd->mPose.mPos.z);
			array.Store(rd->mPose.mRot.p.x);
			array.Store(rd->mPose.mRot.p.y);
			array.Store(rd->mPose.mRot.p.z);
			array.Store(rd->mPose.mRot.w);
		}
	}*/

	const udword NbChunks = mRenderDataChunks.GetNbEntries();
	array.Store(NbChunks);
	for(udword i=0;i<NbChunks;i++)
	{
		RenderDataChunk* Chunk = (RenderDataChunk*)mRenderDataChunks[i];
		array.Store(Chunk->GetID());
	}

	array.Store(mLocalPoses.GetEntries(), mLocalPoses.GetNbEntries()*sizeof(udword));

	return true;
}

bool RenderDataChunkCollection::Import(const VirtualFile& file)
{
	RenderDataChunk::Import(file);
	mCollectionCore.Import(file);

//	const udword NbRenderSources = file.ReadDword();
//	udword* dst = mImportedData.Reserve(NbRenderSources*8);
//	file.ReadBuffer(dst, NbRenderSources*8*sizeof(udword));

	ASSERT(!mRenderDataChunks.GetNbEntries());
	const udword NbChunks = file.ReadDword();
	for(udword i=0;i<NbChunks;i++)
	{
		const udword ID = file.ReadDword();
		mRenderDataChunks.AddPtr(reinterpret_cast<void*>(size_t(ID)));
	}

	udword* dst = mLocalPoses.Reserve(NbChunks*7);
	file.ReadBuffer(dst, NbChunks*7*sizeof(udword));

	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define MANAGED_TEXTURE_CHUNK_VERSION	1

ManagedTextureChunk::ManagedTextureChunk() : mBitmap(null), mCompressed(null), mWidth(0), mHeight(0), mCompressedSize(0)
{
	mChunkType						= ManagedTextureType;
	mManagedTextureCore.mVersion	= MANAGED_TEXTURE_CHUNK_VERSION;
}

ManagedTextureChunk::~ManagedTextureChunk()
{
	ICE_FREE(mCompressed);
	DELETESINGLE(mBitmap);
}

bool ManagedTextureChunk::SetSourceBitmap(const Picture& pic)
{
	const udword Width = pic.GetWidth();
	const udword Height = pic.GetHeight();
	const RGBAPixel* Pixels = pic.GetPixels();
	const bool ValidBitmapData = Pixels && Width && Height;
	if(!ValidBitmapData)
		return false;

	if(0)
	{
		GetCore().Disable(MANAGED_TEXTURE_CHUNK_FLAG_COMPRESSED);

		DELETESINGLE(mBitmap);
		mBitmap = ICE_NEW(Picture);
		CHECKALLOC(mBitmap);
		mBitmap->Copy(pic);
	}
	else
	{
		GetCore().Enable(MANAGED_TEXTURE_CHUNK_FLAG_COMPRESSED);	// We always compress textures now

		const udword inputSize = Width*Height*sizeof(RGBAPixel);
		const int dstCapacity = LZ4_compressBound(inputSize);
		void* dst = ICE_ALLOC(dstCapacity);
		const int compressedSize = LZ4_compress_default((const char*)Pixels, (char*)dst, inputSize, dstCapacity);
		//printf("srcSize: %d | compressedSize: %d\n", inputSize, compressedSize);

		ICE_FREE(mCompressed);
		mCompressed = (ubyte*)ICE_ALLOC(compressedSize);
		CopyMemory(mCompressed, dst, compressedSize);

		ICE_FREE(dst);

		mWidth = Width;
		mHeight = Height;
		mCompressedSize = compressedSize;
	}
	return true;
}

const char* ManagedTextureChunk::Validate()
{
	return BaseChunk::Validate();
}

bool ManagedTextureChunk::Export(CustomArray& array)
{
	BaseChunk::Export(array);
	mManagedTextureCore.Export(array);

	if(IsTextureCompressed())
	{
		array.Store(mWidth).Store(mHeight).Store(mCompressedSize);
		array.Store(mCompressed, mCompressedSize);
	}
	else
	{
		udword Width = 0;
		udword Height = 0;
		if(mBitmap)
		{
			Width	= mBitmap->GetWidth();
			Height	= mBitmap->GetHeight();
		}

		array.Store(Width).Store(Height);

		const bool ValidBitmapData = mBitmap && mBitmap->GetPixels() && Width && Height;
		if(ValidBitmapData)
			array.Store(mBitmap->GetPixels(), sizeof(RGBAPixel)*Width*Height);
	}
	return true;
}

bool ManagedTextureChunk::Import(const VirtualFile& file)
{
	BaseChunk::Import(file);
	mManagedTextureCore.Import(file);

	if(IsTextureCompressed())
	{
		mWidth	= file.ReadDword();
		mHeight	= file.ReadDword();
		mCompressedSize	= file.ReadDword();

		if(mWidth && mHeight && mCompressedSize)
		{
			mCompressed = (ubyte*)ICE_ALLOC(mCompressedSize);
			file.ReadBuffer(mCompressed, mCompressedSize);

			// #### temp
			mBitmap = ICE_NEW(Picture)(ToWord(mWidth), ToWord(mHeight));
			int decompressedSize = LZ4_decompress_safe((const char*)mCompressed, (char*)mBitmap->GetPixels(), mCompressedSize, mWidth*mHeight*sizeof(RGBAPixel));
			ASSERT(decompressedSize==mWidth*mHeight*sizeof(RGBAPixel));
		}
	}
	else
	{
		const udword Width	= file.ReadDword();
		const udword Height	= file.ReadDword();

		if(Width && Height)
		{
			mBitmap = ICE_NEW(Picture)(ToWord(Width), ToWord(Height));
			file.ReadBuffer(mBitmap->GetPixels(), Width*Height*sizeof(RGBAPixel));
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

ZCB2FactoryEx::ZCB2FactoryEx()// : mGlobalScale(1.0f)
{
}

bool ZCB2FactoryEx::NewChunk(const BaseChunk* chunk)
{
	if(!chunk)
		return false;

	switch(chunk->GetChunkType())
	{
		case 'TXMP':
		{
			const TXMPChunk* txmp = static_cast<const TXMPChunk*>(chunk);
			const Picture* B = txmp->GetBitmap();
			const ManagedTexture* MT = CreateManagedTexture(B->GetWidth(), B->GetHeight(), B->GetPixels(), txmp->GetFilename());
			//mImportHelper_Textures.AddPtr(MT);
			ZCB2FactoryEx::TextureData* TD = ICE_RESERVE(ZCB2FactoryEx::TextureData, mImportHelper_TextureData);
			TD->mTexture = MT;
			// The chunk ID is only needed if we try to import old ZB2 files into PEEL, i.e. when the TXMP chunk is used as originally intended.
			TD->mZB2ID = chunk->GetID();
			return true;
		}
		break;
	}
	return NewLegacyChunk(chunk);
}

// TODO: revisit signatures & design, esp for names
void ZCB2FactoryEx::NewUnknownChunk(udword type, const char* name, const VirtualFile& file)
{
	switch(type)
	{
		case SphereRenderDataType:
		{
			SphereRenderDataChunk Chunk;
			Chunk.SetName(name);	//######
			Chunk.Import(file);
			ASSERT(Chunk.GetID()==mImportHelper_Renderers.GetNbEntries());

			PintShapeRenderer* Renderer = CreateSphereRenderer(Chunk.mRadius);
			mImportHelper_Renderers.AddPtr(Renderer);
			return;
		}
		break;

		case MeshRenderDataType:
		{
			MeshRenderDataChunk Chunk;
			Chunk.SetName(name);	//######
			Chunk.Import(file);
			ASSERT(Chunk.GetID()==mImportHelper_Renderers.GetNbEntries());

			const bool UseDirectData = Chunk.IsSetDirectData();
			if(0)
				OutputConsoleInfo(_F("UseDirectData = %d\n", UseDirectData));
			//UseDirectData = true;
			const bool UseActiveEdges = Chunk.IsSetActiveEdges();

			PintShapeRenderer* Renderer;
			if(Chunk.GetUVs().GetNbPts() && Chunk.GetTFaces().GetNbFaces())
			{
				MultiSurface MS;
				MS.IndexedSurface::Init(Chunk.GetFaces().GetNbFaces(), Chunk.GetVertices().GetNbPts(), Chunk.GetVertices().GetPts(), (const IndexedTriangle*)Chunk.GetFaces().GetDFaces());
				if(gZCB2_RenderScale!=1.0f)
					MS.IndexedSurface::Scale(Point(gZCB2_RenderScale, gZCB2_RenderScale, gZCB2_RenderScale));
				{
					IndexedSurface* UVSurface = MS.AddExtraSurface(SURFACE_UVS);
					UVSurface->Init(Chunk.GetTFaces().GetNbFaces(), Chunk.GetUVs().GetNbPts());

					IndexedTriangle* TFaces = (IndexedTriangle*)UVSurface->GetFaces();
					CopyMemory(TFaces, Chunk.GetTFaces().GetDFaces(), Chunk.GetTFaces().GetNbFaces()*sizeof(IndexedTriangle));

					Point* uvs = (Point*)UVSurface->GetVerts();
					CopyMemory(uvs, Chunk.GetUVs().GetPts(), Chunk.GetUVs().GetNbPts()*sizeof(Point));
				}
				Renderer = CreateMeshRenderer(MS, UseActiveEdges, UseDirectData);
			}
			else
			{
				SurfaceInterface SI;
				SI.mNbVerts	= Chunk.GetVertices().GetNbPts();
				SI.mVerts	= Chunk.GetVertices().GetPts();
				SI.mNbFaces	= Chunk.GetFaces().GetNbFaces();
				SI.mDFaces	= Chunk.GetFaces().GetDFaces();
				SI.mWFaces	= Chunk.GetFaces().GetWFaces();

				if(gZCB2_RenderScale!=1.0f)
				{
					const float Scale = gZCB2_RenderScale;
					Point* v0 = const_cast<Point*>(SI.mVerts);
					for(udword i=0;i<SI.mNbVerts;i++)
						v0[i] *= Scale;
				}

				// ### TODO: store & export the CRCs so that we don't have to recompute them here

				Renderer = CreateMeshRenderer(PintSurfaceInterface(SI), null, UseActiveEdges, UseDirectData);
			}

			mImportHelper_Renderers.AddPtr(Renderer);
			return;
		}
		break;

		case ColorRenderDataType:
		{
			ColorRenderDataChunk Chunk;
			Chunk.SetName(name);	//######
			Chunk.Import(file);
			ASSERT(Chunk.GetID()==mImportHelper_Renderers.GetNbEntries());

			ASSERT(Chunk.mSourceID<mImportHelper_Renderers.GetNbEntries());
			if(Chunk.mSourceID<mImportHelper_Renderers.GetNbEntries())
			{
				PintShapeRenderer* SourceRenderer = (PintShapeRenderer*)mImportHelper_Renderers.GetEntry(Chunk.mSourceID);

//				const ManagedTexture* Texture = null;
				const ZCB2FactoryEx::TextureData* Texture = null;
//				if(Chunk.mTexture.mExportID!=INVALID_ID)
				if(Chunk.mTextureID!=INVALID_ID)
				{
					const udword NbTextures = mImportHelper_TextureData.GetNbEntries()/(sizeof(ZCB2FactoryEx::TextureData)/sizeof(udword));
//					const udword NbTextures = mImportHelper_Textures.GetNbEntries();
					const ZCB2FactoryEx::TextureData* Textures = (const ZCB2FactoryEx::TextureData*)mImportHelper_TextureData.GetEntries();
//					const ManagedTexture** Textures = (const ManagedTexture**)mImportHelper_Textures.GetEntries();
//					ASSERT(Chunk.mTexture.mExportID<NbTextures);
					ASSERT(Chunk.mTextureID<NbTextures);
//					Texture = Textures + Chunk.mTexture.mExportID;
//					Texture = Textures[Chunk.mTextureID];
					Texture = Textures + Chunk.mTextureID;
				}

//				PintShapeRenderer* Renderer = CreateColorShapeRenderer(SourceRenderer, Chunk.mColor, Texture);
				PintShapeRenderer* Renderer = CreateColorShapeRenderer(SourceRenderer, Chunk.mColor, Texture ? Texture->mTexture : null);
				mImportHelper_Renderers.AddPtr(Renderer);
			}
			return;
		}
		break;

		case RenderCollectionType:
		{
			RenderDataChunkCollection Chunk;
			Chunk.SetName(name);	//######
			Chunk.Import(file);
			ASSERT(Chunk.GetID()==mImportHelper_Renderers.GetNbEntries());

			PintRendererCollection* RC = static_cast<PintRendererCollection*>(CreateRendererCollection());
			mImportHelper_Renderers.AddPtr(RC);

			const udword Nb = Chunk.mRenderDataChunks.GetNbEntries();
			const PR* LocalPoses = (const PR*)Chunk.mLocalPoses.GetEntries();

			if(gZCB2_RenderScale!=1.0f)
			{
				const float Scale = gZCB2_RenderScale;
				PR* poses = const_cast<PR*>(LocalPoses);
				for(udword i=0;i<Nb;i++)
					poses[i].mPos *= Scale;
			}

			for(udword i=0;i<Nb;i++)
			{
				const udword ID = udword(size_t(Chunk.mRenderDataChunks[i]));
				PintShapeRenderer* r = GetShapeRenderer(ID);

				RC->AddRenderer(r, LocalPoses[i]);
			}
			return;
		}
		break;

		case ManagedTextureType:
		{
			ManagedTextureChunk Chunk;
			Chunk.SetName(name);	//######
			Chunk.Import(file);

			const Picture* B = Chunk.GetBitmap();
			const ManagedTexture* MT = CreateManagedTexture(B->GetWidth(), B->GetHeight(), B->GetPixels(), null/*Chunk.GetFilename()*/);
			//mImportHelper_Textures.AddPtr(MT);
			ZCB2FactoryEx::TextureData* TD = ICE_RESERVE(ZCB2FactoryEx::TextureData, mImportHelper_TextureData);
			TD->mTexture = MT;
			// The chunk ID is only needed if we try to import old ZB2 files into PEEL, i.e. when the TXMP chunk is used as originally intended.
			TD->mZB2ID = Chunk.GetID();
			return;
		}
		break;
	}

	ZCB2Factory::NewUnknownChunk(type, name, file);
}

PintShapeRenderer* ZCB2FactoryEx::GetShapeRenderer(udword id)
{
	ASSERT(id<mImportHelper_Renderers.GetNbEntries());
	if(id<mImportHelper_Renderers.GetNbEntries())
		return (PintShapeRenderer*)mImportHelper_Renderers.GetEntry(id);
	else
		return null;
}

void ZCB2FactoryEx::ReleasePintIndependentData()
{
	mImportHelper_MaterialData.Empty();
//	mImportHelper_Textures.Empty();
	mImportHelper_TextureData.Empty();
	mImportHelper_Renderers.Empty();
	ZCB2Factory::ReleasePintIndependentData();
}

