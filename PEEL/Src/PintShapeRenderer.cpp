///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RenderModel.h"
#include "PintNullRenderer.h"
#include "Zcb2_RenderData.h"
#include "Shader.h"

#include "PintDLSphereShapeRenderer.h"
#include "PintDLCapsuleShapeRenderer.h"
#include "PintDLCylinderShapeRenderer.h"
#include "PintDLBoxShapeRenderer.h"
#include "PintDLMeshShapeRenderer.h"
#include "PintColorShapeRenderer.h"

static const bool gShareShapeRenderers = true;
static const bool gDiscardUVs = false;
static const bool gExportRenderSources = true;	// TODO: don't export all of them
static const bool gUseNullRenderer = false;
//#define USE_DISPLAY_LISTS_FOR_CONVEXES

extern	RenderModel*	gCurrentRenderModel;
extern	bool			gUseEditor;

///////////////////////////////////////////////////////////////////////////////

static PtrContainer* gRenderSources = null;
udword GetNbRenderSources()
{
	return gRenderSources ? gRenderSources->GetNbEntries() : 0;
}

RenderDataChunk* GetRenderSource(udword i)
{
	if(!gRenderSources)
		return null;
	const udword Size = gRenderSources->GetNbEntries();
	if(i>=Size)
		return null;
	return (RenderDataChunk*)gRenderSources->GetEntry(i);
}

static void RegisterRenderSource(PintShapeRenderer* renderer)
{
	ASSERT(renderer);
	if(renderer->mRenderSource)
	{
		if(!gRenderSources)
			gRenderSources = ICE_NEW(PtrContainer);
		ASSERT(gRenderSources);
		gRenderSources->AddPtr(renderer->mRenderSource);
	}
}

static void ReleaseAllRenderSources()
{
	if(gRenderSources)
	{
		const udword Size = gRenderSources->GetNbEntries();
		for(udword i=0;i<Size;i++)
		{
			RenderDataChunk* chunk = (RenderDataChunk*)gRenderSources->GetEntry(i);
			DELETESINGLE(chunk);
		}
		DELETESINGLE(gRenderSources);
	}
}

///////////////////////////////////////////////////////////////////////////////

static PtrContainer* gShapeRenderers = null;
/*static PtrContainer* gSphereShapeRenderers = null;
static PtrContainer* gCapsuleShapeRenderers = null;
static PtrContainer* gCylinderShapeRenderers = null;
static PtrContainer* gConvexShapeRenderers = null;
static PtrContainer* gBoxShapeRenderers = null;*/

udword GetNbShapeRenderers()
{
	udword Nb = 0;
	if(gShapeRenderers)
		Nb += gShapeRenderers->GetNbEntries();
/*	if(gSphereShapeRenderers)
		Nb += gSphereShapeRenderers->GetNbEntries();
	if(gCapsuleShapeRenderers)
		Nb += gCapsuleShapeRenderers->GetNbEntries();
	if(gCylinderShapeRenderers)
		Nb += gCylinderShapeRenderers->GetNbEntries();
	if(gConvexShapeRenderers)
		Nb += gConvexShapeRenderers->GetNbEntries();
	if(gBoxShapeRenderers)
		Nb += gBoxShapeRenderers->GetNbEntries();*/
	return Nb;
}

PintShapeRenderer* GetShapeRenderer(udword i)
{
	if(!gShapeRenderers)
		return null;
	const udword Size = gShapeRenderers->GetNbEntries();
	if(i>=Size)
		return null;
	return (PintShapeRenderer*)gShapeRenderers->GetEntry(i);
}

// Only called from TerrainRegionManager::RemoveRegion for now
bool ReleaseShapeRenderer(PintShapeRenderer* renderer)
{
	bool Found = false;
	// ### TODO: optimize these O(n) calls
	if(gShapeRenderers && gShapeRenderers->Delete(renderer))
		Found = true;
/*	else if(gSphereShapeRenderers && gSphereShapeRenderers->Delete(renderer))
		Found = true;
	else if(gCapsuleShapeRenderers && gCapsuleShapeRenderers->Delete(renderer))
		Found = true;
	else if(gCylinderShapeRenderers && gCylinderShapeRenderers->Delete(renderer))
		Found = true;
	else if(gConvexShapeRenderers && gConvexShapeRenderers->Delete(renderer))
		Found = true;
	else if(gBoxShapeRenderers && gBoxShapeRenderers->Delete(renderer))
		Found = true;*/

	// TODO: take refcount into account, update gPintColorShapeRenderers

	if(Found)
		ICE_SAFE_RELEASE(renderer);
	return Found;
}

static PintShapeRenderer* _RegisterShapeRenderer(PtrContainer*& renderers, PintShapeRenderer* renderer)
{
	if(renderer)
	{
		ASSERT(!(renderer->mBaseFlags & SHAPE_RENDERER_IS_REGISTERED));
		if(!renderers)
			renderers = ICE_NEW(PtrContainer);
		ASSERT(renderers);
		renderers->AddPtr(renderer);
		renderer->mBaseFlags |= SHAPE_RENDERER_IS_REGISTERED;

		RegisterRenderSource(renderer);
	}
	return renderer;
}

// TODO: merge with above once all renderers are created by factories
static inline_ PintShapeRenderer* _RegisterNewShapeRenderer(PintShapeRenderer* new_renderer)
{
	if(new_renderer && !(new_renderer->mBaseFlags & SHAPE_RENDERER_IS_REGISTERED))
		_RegisterShapeRenderer(gShapeRenderers, new_renderer);
	return new_renderer;
}

///////////////////////////////////////////////////////////////////////////////

static NullRenderer gNullRenderer;

PintShapeRenderer* CreateNullRenderer()
{
	return &gNullRenderer;
}

///////////////////////////////////////////////////////////////////////////////

// New management of shape renderers:
// 1) make private ctor/dtor for each shape renderer class you want to track.  Now you cannot allocate them all over the place.
// 2) add a factory function (e.g. CreatePintDLSphereShapeRenderer), make it a friend of the shape renderer class.
// 3) redirect non-compiling shape renderer creation calls to factory function.
// 4) move the render source creation to new factory function. We'll have to skip that step for shared shape renderers.
// 5) call _RegisterNewShapeRenderer() instead of _RegisterShapeRenderer().
// 6) add a container for newly created shape renderers, store new ptr in it (KeepTrackOfRendererForSharing).
// 7) delete that container in ReleaseAllShapeRenderers()
// 8) iterate that container at the start of factory function, looking for existing shareable renderer. Might need to store previously unstored creation param in the renderer there.
// 9) add a customized GetOwnerContainer() function that returns the new container.

///////////////////////////////////////////////////////////////////////////////

template<class RendererT, class ParamsT>
static RendererT* FindRenderer(PtrContainer*& renderers, const ParamsT& params)
{
	if(gShareShapeRenderers && renderers)
	{
		const udword Nb = renderers->GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			RendererT* Current = reinterpret_cast<RendererT*>(renderers->GetEntry(i));
			if(Current->Equal(params))
			{
				//OutputConsoleInfo("Sharing PintDLSphereShapeRenderer...\n");
				Current->mRefCount++;
				return Current;
			}
		}
	}
	return null;
}

template<class RendererT>
static inline_ RendererT* KeepTrackOfRendererForSharing(PtrContainer*& renderers, RendererT* renderer)
{
	if(renderer && gShareShapeRenderers)
	{
		if(!renderers)
			renderers = ICE_NEW(PtrContainer);
		renderers->AddPtr(renderer);
	}
	return renderer;
}

///////////////////////////////////////////////////////////////////////////////

bool PintShapeRenderer::Release()
{
	mRefCount--;

	if(mRefCount)
		return false;

	PtrContainer* Owner = GetOwnerContainer();
	if(Owner)
	{
		bool Status = Owner->Delete(this);
		ASSERT(Status);
	}

	delete this;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

PintShapeRenderer* CreateSphereRenderer(float radius, bool geo_sphere)
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	// ### No need to export a render source, we can recreate this one implicitly on import
	//	if(gUseEditor && gExportRenderSources)
	//	{
	//		ASSERT(!NewRenderer->mRenderSource);
	//		NewRenderer->mRenderSource = ICE_NEW(SphereRenderDataChunk)(radius);
	//	}

	return _RegisterNewShapeRenderer(gCurrentRenderModel->_CreateSphereRenderer(radius, geo_sphere));
}

static PtrContainer* gPintDLSphereShapeRenderers = null;

PintDLSphereShapeRenderer* CreatePintDLSphereShapeRenderer(float radius, bool uses_vertex_normals, bool geo_sphere)
{
	PintDLSphereShapeRenderer* Shared = FindRenderer<PintDLSphereShapeRenderer>(gPintDLSphereShapeRenderers, PintDLSphereShapeRenderer::Data(radius, uses_vertex_normals, geo_sphere));
	if(Shared)
		return Shared;

	PintDLSphereShapeRenderer* NewRenderer = ICE_NEW(PintDLSphereShapeRenderer)(radius, uses_vertex_normals, geo_sphere);
	return KeepTrackOfRendererForSharing(gPintDLSphereShapeRenderers, NewRenderer);
}

PtrContainer* PintDLSphereShapeRenderer::GetOwnerContainer() const
{
	return gPintDLSphereShapeRenderers;
}

///////////////////////////////////////////////////////////////////////////////

PintShapeRenderer* CreateCapsuleRenderer(float radius, float height)
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	// ### No need to export a render source, we can recreate this one implicitly on import
/*	if(gUseEditor && renderer && gExportRenderSources)
	{
		ASSERT(0);
	}*/

	return _RegisterNewShapeRenderer(gCurrentRenderModel->_CreateCapsuleRenderer(radius, height));
}

static PtrContainer* gPintDLCapsuleShapeRenderers = null;

PintDLCapsuleShapeRenderer* CreatePintDLCapsuleShapeRenderer(float radius, float height, bool uses_vertex_normals)
{
	PintDLCapsuleShapeRenderer* Shared = FindRenderer<PintDLCapsuleShapeRenderer>(gPintDLCapsuleShapeRenderers, PintDLCapsuleShapeRenderer::Data(radius, height, uses_vertex_normals));
	if(Shared)
		return Shared;

	PintDLCapsuleShapeRenderer* NewRenderer = ICE_NEW(PintDLCapsuleShapeRenderer)(radius, height, uses_vertex_normals);
	return KeepTrackOfRendererForSharing(gPintDLCapsuleShapeRenderers, NewRenderer);
}

PtrContainer* PintDLCapsuleShapeRenderer::GetOwnerContainer() const
{
	return gPintDLCapsuleShapeRenderers;
}

///////////////////////////////////////////////////////////////////////////////

PintShapeRenderer* CreateCylinderRenderer(float radius, float height)
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	// ### No need to export a render source, we can recreate this one implicitly on import
/*	if(gUseEditor && renderer && gExportRenderSources)
	{
		ASSERT(0);
	}*/

	return _RegisterNewShapeRenderer(gCurrentRenderModel->_CreateCylinderRenderer(radius, height));
}

static PtrContainer* gPintDLCylinderShapeRenderers = null;

PintDLCylinderShapeRenderer* CreatePintDLCylinderShapeRenderer(float radius, float height)
{
	PintDLCylinderShapeRenderer* Shared = FindRenderer<PintDLCylinderShapeRenderer>(gPintDLCylinderShapeRenderers, PintDLCylinderShapeRenderer::Data(radius, height));
	if(Shared)
		return Shared;

	PintDLCylinderShapeRenderer* NewRenderer = ICE_NEW(PintDLCylinderShapeRenderer)(radius, height);
	return KeepTrackOfRendererForSharing(gPintDLCylinderShapeRenderers, NewRenderer);
}

PtrContainer* PintDLCylinderShapeRenderer::GetOwnerContainer() const
{
	return gPintDLCapsuleShapeRenderers;
}

///////////////////////////////////////////////////////////////////////////////

PintShapeRenderer* CreateBoxRenderer(const Point& extents)
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	// ### No need to export a render source, we can recreate this one implicitly on import
/*	if(gUseEditor && renderer && gExportRenderSources)
	{
		ASSERT(0);
	}*/

	return _RegisterNewShapeRenderer(gCurrentRenderModel->_CreateBoxRenderer(extents));
/*#else
	if(1)
	{
		return RegisterShapeRenderer(ICE_NEW(PintDLBoxShapeRenderer)(extents, gCurrentRenderModel->NeedsVertexNormals()));
	}
	else
	{
		AABB box;
		box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), extents);

		Point Pts[8];
		box.ComputePoints(Pts);

		return CreateConvexRenderer(8, Pts);
	}
#endif*/
}

static PtrContainer* gPintDLBoxShapeRenderers = null;

PintDLBoxShapeRenderer* CreatePintDLBoxShapeRenderer(const Point& extents, bool uses_vertex_normals)
{
	PintDLBoxShapeRenderer* Shared = FindRenderer<PintDLBoxShapeRenderer>(gPintDLBoxShapeRenderers, PintDLBoxShapeRenderer::Data(extents, uses_vertex_normals));
	if(Shared)
		return Shared;

	PintDLBoxShapeRenderer* NewRenderer = ICE_NEW(PintDLBoxShapeRenderer)(extents, uses_vertex_normals);
	return KeepTrackOfRendererForSharing(gPintDLBoxShapeRenderers, NewRenderer);
}

PtrContainer* PintDLBoxShapeRenderer::GetOwnerContainer() const
{
	return gPintDLBoxShapeRenderers;
}

///////////////////////////////////////////////////////////////////////////////

// #### TODO: convex container

PintShapeRenderer* CreateConvexRenderer(udword nb_verts, const Point* verts)
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	// ### No need to export a render source, we can recreate this one implicitly on import
/*	if(gUseEditor && renderer && gExportRenderSources)
	{
		ASSERT(0);
	}*/

	PintShapeRenderer* renderer = gCurrentRenderModel->_CreateConvexRenderer(nb_verts, verts);
	return _RegisterShapeRenderer(gShapeRenderers, renderer);
/*#else
	#ifdef USE_DISPLAY_LISTS_FOR_CONVEXES
	return RegisterShapeRenderer(ICE_NEW(PintConvexShapeDLRenderer)(nb_verts, verts));
	#else

	if(gCurrentRenderModel->NeedsVertexNormals())
//		return RegisterShapeRenderer(ICE_NEW(PintConvexShapeDLRenderer)(nb_verts, verts));
		return RegisterShapeRenderer(ICE_NEW(PintBatchConvexShapeRenderer)(nb_verts, verts));
	else
//		return RegisterShapeRenderer(ICE_NEW(PintConvexBatchRendererCPUTransformNoNormals)(nb_verts, verts));
		return RegisterShapeRenderer(ICE_NEW(PintConvexInstanceRenderer)(nb_verts, verts));
	#endif
#endif*/
}

///////////////////////////////////////////////////////////////////////////////

PintShapeRenderer* CreateMeshRenderer(const PintSurfaceInterface& surface, const Point* normals, bool active_edges, bool direct_data)
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	PintShapeRenderer* NewRenderer = gCurrentRenderModel->_CreateMeshRenderer(surface, normals, active_edges, direct_data);

	if(gUseEditor && gExportRenderSources && NewRenderer && !NewRenderer->mRenderSource)	// mRenderSource will already exist for shared renderers
		NewRenderer->mRenderSource = ICE_NEW(MeshRenderDataChunk)(surface, active_edges, direct_data);

	return _RegisterNewShapeRenderer(NewRenderer);
}

PintShapeRenderer* CreateMeshRenderer(const MultiSurface& multi_surface, bool active_edges, bool direct_data)
{
//	return CreateMeshRenderer(PintSurfaceInterface(multi_surface.GetSurfaceInterface()), null, false, true);

	if(gDiscardUVs)
		return CreateMeshRenderer(PintSurfaceInterface(multi_surface.GetSurfaceInterface()), null, active_edges, direct_data);

	if(gUseNullRenderer)
		return &gNullRenderer;

	PintShapeRenderer* NewRenderer = gCurrentRenderModel->_CreateMeshRenderer(multi_surface, active_edges, direct_data);

	if(gUseEditor && gExportRenderSources && NewRenderer && !NewRenderer->mRenderSource)	// mRenderSource will already exist for shared renderers
		NewRenderer->mRenderSource = ICE_NEW(MeshRenderDataChunk)(multi_surface, active_edges, direct_data);

	return _RegisterNewShapeRenderer(NewRenderer);
}

static PtrContainer* gPintDLMeshShapeRenderers = null;

PintDLMeshShapeRenderer* CreatePintDLMeshShapeRenderer(const PintSurfaceInterface& surface, udword flags)
{
	PintDLMeshShapeRenderer* Shared = FindRenderer<PintDLMeshShapeRenderer>(gPintDLMeshShapeRenderers, PintDLMeshShapeRenderer::Data(surface, flags));
	if(Shared)
		return Shared;

	PintDLMeshShapeRenderer* NewRenderer = ICE_NEW(PintDLMeshShapeRenderer)(surface, flags);
	return KeepTrackOfRendererForSharing(gPintDLMeshShapeRenderers, NewRenderer);
}

PintDLMeshShapeRenderer* CreatePintDLMeshShapeRenderer(const MultiSurface& multi_surface, udword flags)
{
//	PintDLMeshShapeRenderer* Shared = FindRenderer<PintDLMeshShapeRenderer>(gPintDLMeshShapeRenderers, PintDLMeshShapeRenderer::Data(flags));
//	if(Shared)
//		return Shared;

	PintDLMeshShapeRenderer* NewRenderer = ICE_NEW(PintDLMeshShapeRenderer)(multi_surface, flags);
	return KeepTrackOfRendererForSharing(gPintDLMeshShapeRenderers, NewRenderer);
}

PtrContainer* PintDLMeshShapeRenderer::GetOwnerContainer() const
{
	return gPintDLMeshShapeRenderers;
}

///////////////////////////////////////////////////////////////////////////////

PintShapeRenderer* CreateColorShapeRenderer(PintShapeRenderer* source_renderer, const RGBAColor& color, const ManagedTexture* texture)
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	PintShapeRenderer* NewRenderer = gCurrentRenderModel->_CreateColorShapeRenderer(source_renderer, color, texture);

	if(gUseEditor && gExportRenderSources && NewRenderer && !NewRenderer->mRenderSource)	// mRenderSource will already exist for shared renderers
	{
//		const RGBAColor* AdjustedColor = NewRenderer->GetColor();
//		if(AdjustedColor)
//			NewRenderer->mRenderSource = ICE_NEW(ColorRenderDataChunk)(source_renderer->mRenderSource, *AdjustedColor, texture);
//		else
			NewRenderer->mRenderSource = ICE_NEW(ColorRenderDataChunk)(source_renderer->mRenderSource, color, texture);
	}

	return _RegisterNewShapeRenderer(NewRenderer);
}

static PtrContainer* gPintColorShapeRenderers = null;

PintColorShapeRenderer* CreatePintColorShapeRenderer(PintShapeRenderer* source_renderer, const RGBAColor& color, const ManagedTexture* texture)
{
	PintColorShapeRenderer* Shared = FindRenderer<PintColorShapeRenderer>(gPintColorShapeRenderers, PintColorShapeRenderer::Data(source_renderer, color, texture));
	if(Shared)
		return Shared;

	PintColorShapeRenderer* NewRenderer = ICE_NEW(PintColorShapeRenderer)(source_renderer, color, texture);
	return KeepTrackOfRendererForSharing(gPintColorShapeRenderers, NewRenderer);
}

PtrContainer* PintColorShapeRenderer::GetOwnerContainer() const
{
	return gPintColorShapeRenderers;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef RETIRED
	class PintCustomShapeRenderer : public PintShapeRenderer
	{
		public:

							PintCustomShapeRenderer(PintShapeRenderer* renderer) : mRenderer(renderer)			{}
		virtual				~PintCustomShapeRenderer()	{}

		virtual	void		Render(const PR& pose)						{ mRenderer->Render(pose);				}

		PintShapeRenderer*	mRenderer;
	};

PintShapeRenderer* CreateCustomRenderer(PintShapeRenderer* renderer)
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	return RegisterShapeRenderer(ICE_NEW(PintCustomShapeRenderer)(renderer));
}
#endif

///////////////////////////////////////////////////////////////////////////////

// TODO: this is starting to be a lot of indirections, we'll need to optimize all this

PintRendererCollection::PintRendererCollection()// : PintShapeRenderer(SHAPE_RENDERER_COLLECTION)
{
}

PintRendererCollection::~PintRendererCollection()
{
}

void PintRendererCollection::AddRenderer(PintShapeRenderer* renderer, const PR& pose)
{
	ASSERT(renderer);
	if(!renderer)
		return;

	PintRendererCollection::RenderData* RD = ICE_RESERVE(PintRendererCollection::RenderData, mRenderData);

	RD->mRenderer	= renderer;
	RD->mPose		= pose;

	if(mRenderSource && renderer->mRenderSource)
	{
		RenderDataChunkCollection* Collection = static_cast<RenderDataChunkCollection*>(mRenderSource);
		Collection->mRenderDataChunks.AddPtr(renderer->mRenderSource);
		Collection->mLocalPoses.Add(&pose.mPos.x, 7);
//		Collection->mCollection = this;
	}
}

void PintRendererCollection::_Render(const PR& pose) const
{
	//ASSERT(0);
	// This is still used by the codepath that renders selected shapes.....

	udword NbRenderers = mRenderData.GetNbEntries()/(sizeof(PintRendererCollection::RenderData)/sizeof(udword));
	const PintRendererCollection::RenderData* RD = (const PintRendererCollection::RenderData*)mRenderData.GetEntries();
	while(NbRenderers--)
	{
		// TODO: this PR stuff is very old and lame.... to revisit
		PR GlobalPose = RD->mPose;
		GlobalPose *= pose;

		SetupMaterial(RD->mRenderer);
		RenderShape(RD->mRenderer, GlobalPose);
		RD++;
	}
}

// #### TODO: share collections?

PintShapeRenderer* CreateRendererCollection()
{
	if(gUseNullRenderer)
		return &gNullRenderer;

	PintShapeRenderer* renderer = ICE_NEW(PintRendererCollection);

	if(gUseEditor && renderer && gExportRenderSources)
		renderer->mRenderSource = ICE_NEW(RenderDataChunkCollection);

	return _RegisterShapeRenderer(gShapeRenderers, renderer);
}

///////////////////////////////////////////////////////////////////////////////

static void _ReleaseAllShapeRenderers(PtrContainer*& renderers)
{
	if(renderers)
	{
		const udword Size = renderers->GetNbEntries();
		for(udword i=0;i<Size;i++)
		{
			PintShapeRenderer* renderer = (PintShapeRenderer*)renderers->GetEntry(i);
			while(!renderer->Release());
		}
		DELETESINGLE(renderers);
	}
}

void ReleaseAllShapeRenderers()
{
	DELETESINGLE(gPintDLMeshShapeRenderers);
	DELETESINGLE(gPintDLBoxShapeRenderers);
	DELETESINGLE(gPintDLCylinderShapeRenderers);
	DELETESINGLE(gPintDLCapsuleShapeRenderers);
	DELETESINGLE(gPintDLSphereShapeRenderers);
	DELETESINGLE(gPintColorShapeRenderers);

	_ReleaseAllShapeRenderers(gShapeRenderers);
/*	_ReleaseAllShapeRenderers(gSphereShapeRenderers);
	_ReleaseAllShapeRenderers(gCapsuleShapeRenderers);
	_ReleaseAllShapeRenderers(gCylinderShapeRenderers);
	_ReleaseAllShapeRenderers(gConvexShapeRenderers);
	_ReleaseAllShapeRenderers(gBoxShapeRenderers);*/


	ReleaseAllRenderSources();
}
