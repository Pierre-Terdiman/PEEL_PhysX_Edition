///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DisplayTexture.h"
#include "GLRenderHelpers.h"
#include "GLTexture.h"

static GLuint mTexID = 0;

static void Init()
{
	const udword w = 256*2;
	const udword h = 256*2;
	Picture P(w, h);
//	P.MakeVoronoi();
//	P.MakeMarble2();
//	P.AlphaLuminance();
//	P.BinaryAlpha();
//	P.MakeAlphaGrid(4);
//	P.AlphaToColor();

//	for(udword i=0;i<32;i++)
//		P.Smooth(PIXEL_A);

//	mTexID = GLTexture::createTexture(w, h, P.GetPixels(), true);
//	return;


/*	const float Length = 256.0f;
	for(sdword y=0;y<sy;y++)
	{
		for(sdword x=0;x<sx;x++)
		{
//			mChunks[x+y*nbx].Init(64, 64, 64.0f, Point(float(x-sx/2)*65.0f, 0.0f, float(y-sy/2)*65.0f));
//			mChunks[x+y*nbx].Init(64, 64, 64.0f, Point((float(x)-hx)*64.0f, 0.0f, (float(y)-hy)*64.0f));
//			mChunks[x+y*nbx].Init(32, 32, Length, Point((float(x)-hx)*Length, 0.0f, (float(y)-hy)*Length));
			mChunks[x+y*nbx].Init(32, 32, Length, Point(float(x)*Length, 0.0f, float(y)*Length));
		}
	}*/

udword nbx = 32;
udword nby = 32;
//float length = 10.0f*256.0f;
float length = 256.0f*4.0f;
const Point pos(0.0f, 0.0f, 0.0f);


	const float Amplitude = 400.0f;
//	const float Amplitude = 1200.0f;
	const float FractalIncrement = 1.0f;
	const float Lacunarity = 2.17f;
	const float Octaves = 12.0f;
	FractalBrownianMotion fbm(FractalIncrement, Lacunarity, Octaves);
//	PerlinNoise PN;
	const float rf_FractalIncrement	= 1.3f;
	const float rf_Octaves			= 8.0f;
	const float rf_Offset			= 10.0f;
	const float rf_Gain				= 2.0f;
	const float rf_Amplitude		= 100.0f;
//	RidgedFractal RF(rf_FractalIncrement, Lacunarity, rf_Octaves, rf_Offset, rf_Gain);
//	const float rf_Offset2 = fabsf(rf_Amplitude * 100.0f);
//	const float rf_Offset2 = -10000.0f;
	const float rf_Offset2 = 15560.0f;

//	const udword TextureFactor = 2;
//	const udword TextureFactor = 4;
//	const udword TextureFactor = 6;
//	const udword TextureFactor = 8;
	const udword TextureFactor = 16;

	// Generate larger field for texture gen
	const udword TNbX = nbx * TextureFactor;
	const udword TNbY = nby * TextureFactor;
	const udword NbVerts = TNbX * TNbY;

	Point* TexVerts = ICE_NEW(Point)[NbVerts];
	{
		float MinY = MAX_FLOAT;
		float MaxY = MIN_FLOAT;

		float* TexHeights = new float[NbVerts];
		const float OneOverNbX = 1.0f/float(TNbX-1);
		const float OneOverNbY = 1.0f/float(TNbY-1);
		for(udword y=0;y<TNbY;y++)
		{
			const float YCoeff = float(y)*OneOverNbY;
			for(udword x=0;x<TNbX;x++)
			{
				const float XCoeff = float(x)*OneOverNbX;

				Point wp = pos + Point((XCoeff-0.5f)*length, 0.0f, (YCoeff-0.5f)*length);
//				wp.y = Amplitude * fbm.Compute(wp*0.002f);
				wp.y = Amplitude * 5.0f * fbm.Compute(wp*0.002f) * fbm.Compute(wp*0.001f);	// This one is pretty cool
//				wp.y = Amplitude * fbm.Compute(wp*0.001f);
//				wp.y = -Amplitude * PN.Turbulence(wp*0.001f, 256.0f);
//				wp.y = -rf_Offset2 + rf_Amplitude * RF.Compute(wp*0.0005f);
				MinY = MIN(wp.y, MinY);
				MaxY = MAX(wp.y, MaxY);

				TexHeights	[x+y*TNbX] = wp.y;
				TexVerts	[x+y*TNbX] = wp;
			}
		}
		if(0)
		{
			const float Middle = (MaxY+MinY)*0.5f;
			for(udword i=0;i<TNbX*TNbY;i++)
			{
				TexHeights[i] -= Middle;
				TexVerts[i].y -= Middle;
			}
		}

//		const float MinMax[2] = { -100.0f, 100.0f };
//		const float MinMax[2] = { -200.0f, 100.0f };
		const float MinMax[2] = { -200.0f, 200.0f };
//		const float MinMax[2] = { -1000.0f, 100.0f };	// RF

		const bool RenderWater = false;

		TerrainTexture TT;

		TERRAINTEXTURECREATE ttc;
		ttc.mField			= TexHeights;
		ttc.mWidth			= TNbX;
		ttc.mHeight			= TNbY;
		ttc.mRenderWater	= RenderWater;
		ttc.mSeaLevel		= 0.0f;
		ttc.mMinMax			= MinMax;

		TT.Create(ttc);
P = TT;
//P.FlipHorizontal();
	}

	mTexID = GLTexture::CreateTexture(w, h, P.GetPixels(), true);
}

#include "RenderTarget.h"
extern PEEL::RenderTarget* gReflectedSceneTarget;
extern udword gScreenWidth;
extern udword gScreenHeight;

void DisplayTexture(udword screen_width, udword screen_height)
{
	if(screen_width==INVALID_ID)
		screen_width = gScreenWidth;
	if(screen_height==INVALID_ID)
		screen_height = gScreenHeight;

	if(0)
	{
		if(!gReflectedSceneTarget)
			return;
		mTexID = gReflectedSceneTarget->GetColorTexId();
	}
	else
	{
		if(!mTexID)
			Init();
	}

	glBindTexture(GL_TEXTURE_2D, mTexID);
//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glDisable(GL_CULL_FACE);
	const Point color(1.0f, 1.0f, 1.0f);
	const float Alpha = 0.999f;
	const udword PixelSize = 256*2;

	const float x = float(screen_width-PixelSize)/float(screen_width);
	const float y = float(screen_height-PixelSize)/float(screen_height);

	GLRenderHelpers::DrawRectangle(	x,    1.0f,     y, 1.0f, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING);
//	GLRenderHelpers::DrawRectangle(	0.0f, 1.0f - x, y, 1.0f, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING|GLRenderHelpers::SQT_FLIP_U);
//	GLRenderHelpers::DrawRectangle(	x,    1.0f,     0.0f, 1.0f - y, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING|GLRenderHelpers::SQT_FLIP_V);
//	GLRenderHelpers::DrawRectangle(	0.0f, 1.0f - x, 0.0f, 1.0f - y, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING|GLRenderHelpers::SQT_FLIP_U|GLRenderHelpers::SQT_FLIP_V);

	glDisable(GL_TEXTURE_2D);
}
