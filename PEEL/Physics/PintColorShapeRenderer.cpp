///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintColorShapeRenderer.h"
#include "GLShader.h"
#include "PintRenderState.h"
#include "Shader.h"
#include "Common.h"

#ifdef REMOVED
extern	bool	gWireframePass;
extern	bool	gShadowPass;

namespace
{
	struct TranparentData
	{
		const PintColorShapeRenderer*	mRenderer;
		PR								mPose;
	};
}

static Container* gTransparent = null;
#endif

void ReleaseTransparent()
{
#ifdef REMOVED
	DELETESINGLE(gTransparent);
#endif
}

///////////////////////////////////////////////////////////////////////////////

PintColorShapeRenderer::PintColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture) : mData(renderer, color, texture)
	//PintShapeRenderer	(SHAPE_RENDERER_COLOR),
{
	if(color.A!=1.0f)
	{
		mSortKey = 1;
		mBaseFlags |= SHAPE_RENDERER_TRANSPARENT;
	}

	if(renderer->_UseExplicitVertexNormals())
		mBaseFlags |= SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS;
}

PintColorShapeRenderer::~PintColorShapeRenderer()
{
}

///////////////////////////////////////////////////////////////////////////////

// TODO: the material setup is done before calling this function, sometimes in vain for delayed/transparent shapes
extern	bool	gExternalColorControl;

void PintColorShapeRenderer::_Render(const PR& pose) const
{
#ifdef REMOVED
	const bool SetupRS = !gWireframePass && !gShadowPass;
	if(0 && SetupRS)
	{
		if(!gExternalColorControl && mData.mColor.A!=1.0f)
		{
			if(!gTransparent)
				gTransparent = ICE_NEW(Container);

			TranparentData* Data = ICE_RESERVE(TranparentData, *gTransparent);

			Data->mRenderer = this;
			Data->mPose	= pose;
			return;
		}
	}
#endif
	mData.mRenderer->_Render(pose);
}

void RenderTransparent()
{
#ifdef REMOVED
	if(!gTransparent)
		return;

	SPY_ZONE("RenderTransparent")

	// TODO: sort meshes & faces
	udword NbMeshes = gTransparent->GetNbEntries()/(sizeof(TranparentData)/sizeof(udword));
	if(NbMeshes)
	{
		glDepthMask(FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		const TranparentData* Data = (const TranparentData*)gTransparent->GetEntries();

		while(NbMeshes--)
		{
			const PintColorShapeRenderer* M = Data->mRenderer;
			SetupMaterial(M);
			RenderShape(M->mData.mRenderer, Data->mPose);
			Data++;
		}

		glDisable(GL_BLEND);
		glDepthMask(TRUE);
	}
	gTransparent->Reset();
#endif
}

///////////////////////////////////////////////////////////////////////////////

