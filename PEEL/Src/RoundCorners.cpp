///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoundCorners.h"
#include "GLRenderHelpers.h"
#include "GLTexture.h"
#include "TextureManager.h"

static GLuint mTexID = 0;

static void Init()
{
	const udword w = 256;
	const udword h = 256;
	Picture P(w, h);
//	P.MakeMarble2();
//	P.AlphaLuminance();
//	P.BinaryAlpha();
//	P.MakeAlphaGrid(4);

	RGBAPixel* Pixels = P.GetPixels();
	for(udword j=0;j<h;j++)
	{
		for(udword i=0;i<w;i++)
		{
			RGBAPixel& p = Pixels[i+j*w];

			const float y = float(j)/float(h-1);
			const float x = float(i)/float(w-1);
			const float r = sqrtf(x*x+y*y);
			if(r>1.0f)
			{
				p.R = p.G = p.B = 0;
				p.A = 255;//(r-1.0f)*255.0f*2.0f;
//				p.A = (r-1.0f)*255.0f*2.0f;
			}
			else
			{
				p.R = p.G = p.B = 0;
				p.A = 0;
			}
		}
	}
	for(udword i=0;i<32;i++)
		P.Smooth(PIXEL_A);
/*
//	for(udword i=0;i<16;i++)
//		P.SpreadAlpha();

	for(udword j=0;j<h;j++)
	{
		for(udword i=0;i<w;i++)
		{
			RGBAPixel& p = Pixels[i+j*w];
			p.R = p.G = p.B = 0;
		}
	}*/

	mTexID = GLTexture::CreateTexture(256, 256, P.GetPixels(), true);
	SystemTexture* ST = CreateSystemTexture(256, 256, mTexID, "Round corners");
}

void DrawRoundCorners(udword screen_width, udword screen_height, udword corner_size)
{
	SPY_ZONE("DrawRoundCorners")

	if(!mTexID)
		Init();

	glBindTexture(GL_TEXTURE_2D, mTexID);
//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glDisable(GL_CULL_FACE);
	const Point color(1.0f, 1.0f, 1.0f);
	const float Alpha = 0.999f;
	const udword PixelSize = corner_size;

	const float x = float(screen_width-PixelSize)/float(screen_width);
	const float y = float(screen_height-PixelSize)/float(screen_height);

	GLRenderHelpers::DrawRectangle(	x,    1.0f,     y, 1.0f, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING);
	GLRenderHelpers::DrawRectangle(	0.0f, 1.0f - x, y, 1.0f, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING|GLRenderHelpers::SQT_FLIP_U);
	GLRenderHelpers::DrawRectangle(	x,    1.0f,     0.0f, 1.0f - y, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING|GLRenderHelpers::SQT_FLIP_V);
	GLRenderHelpers::DrawRectangle(	0.0f, 1.0f - x, 0.0f, 1.0f - y, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING|GLRenderHelpers::SQT_FLIP_U|GLRenderHelpers::SQT_FLIP_V);

	glDisable(GL_TEXTURE_2D);
}
