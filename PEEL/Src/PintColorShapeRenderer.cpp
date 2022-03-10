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

	if(0)
	{
	//	printf("%f %f %f\n", color.R, color.G, color.B);
		const udword Hex = color;
	//	printf("%x\n", Hex);

		udword Color0;
		//GetColorByName("FloralWhite", Color0);
		//GetColorByName("DarkOrange", Color0);
		//GetColorByName("Firebrick", Color0);
		//GetColorByName("Khaki4", Color0);
		//GetColorByName("DarkOliveGreen4", Color0);
		//GetColorByName("PaleGreen4", Color0);
		//GetColorByName("DarkSeaGreen4", Color0);
		GetColorByName("DeepSkyBlue4", Color0);
		//GetColorByName("SteelBlue4", Color0);
		//GetColorByName("DodgerBlue4", Color0);

	//		Peru
	//		Burlywood
	//		Wheat
	//		Tan

		udword Color1;
		//if(GetColorByName("Bisque", Color1))
		GetColorByName("Ivory2", Color1);
		//GetColorByName("Tan", Color1);
		//GetColorByName("Peru", Color1);

		if(Hex==0xffd40023)
		{
			RGBAColor& c = const_cast<RGBAColor&>(mData.mColor);
			//c.R = 0.0f;
			//c.G = 0.0f;
			//c.B = 1.0f;
			c.B = float(Color0 & 0xff)/255.0f;
			c.G = float((Color0>>8) & 0xff)/255.0f;
			c.R = float((Color0>>16) & 0xff)/255.0f;
		}

		if(Hex==0xff2a53a5)
		{
			RGBAColor& c = const_cast<RGBAColor&>(mData.mColor);
			//c.R = 1.0f;
			//c.G = 1.0f;
			//c.B = 1.0f;
			c.B = float(Color1 & 0xff)/255.0f;
			c.G = float((Color1>>8) & 0xff)/255.0f;
			c.R = float((Color1>>16) & 0xff)/255.0f;
		}

	/*ffd40023
	ff4e5156
	ff2a53a5
	ff171717
	ffa1a0a2
	ff887453
	ffbebebe
	fff7c51e
	ffd3ba7b
	65ededed*/
	}
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

