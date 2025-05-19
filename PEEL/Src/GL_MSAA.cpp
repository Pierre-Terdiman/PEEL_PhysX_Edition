///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GL_MSAA.h"
#include "GLTexture.h"

#if PEEL_USE_MSAA
static int gMSAA_NbSamples = 0;
/*static*/ udword gMSAA_Index = 0;
/*static*/ GLuint g_msaaFbo = 0;
static GLuint g_msaaColorBuf = 0;
static GLuint g_msaaDepthBuf = 0;
extern udword gScreenWidth;
extern udword gScreenHeight;
#endif

void StartFrame_MSAA()
{
	SPY_ZONE("StartFrame MSAA")

#if PEEL_USE_MSAA
	if(gMSAA_NbSamples)
	{
		glEnable(GL_MULTISAMPLE);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, g_msaaFbo);
	}
#endif
}

void EndFrame_MSAA()
{
	SPY_ZONE("EndFrame MSAA")

#if PEEL_USE_MSAA
	if(g_msaaFbo && gMSAA_NbSamples)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, g_msaaFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
		glBlitFramebuffer(0, 0, gScreenWidth, gScreenHeight, 0, 0, gScreenWidth, gScreenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glDisable(GL_MULTISAMPLE);
	}
#endif
}

void Release_MSAA()
{
#if PEEL_USE_MSAA
	GLTexture::ReleaseFramebuffer(g_msaaFbo);
	GLTexture::ReleaseRenderbuffer(g_msaaColorBuf);
	GLTexture::ReleaseRenderbuffer(g_msaaDepthBuf);
#endif
}

void Resize_MSAA()
{
#if PEEL_USE_MSAA
	if(gMSAA_NbSamples)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		Release_MSAA();

		int samples;
//		glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
//		printf("%d max samples for MSAA\n");
		samples = gMSAA_NbSamples;

		glGenFramebuffers(1, &g_msaaFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFbo);

		glGenRenderbuffers(1, &g_msaaColorBuf);
		glBindRenderbuffer(GL_RENDERBUFFER, g_msaaColorBuf);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, gScreenWidth, gScreenHeight);

		glGenRenderbuffers(1, &g_msaaDepthBuf);
		glBindRenderbuffer(GL_RENDERBUFFER, g_msaaDepthBuf);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, gScreenWidth, gScreenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_msaaDepthBuf);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, g_msaaColorBuf);

		glCheckFramebufferStatus(GL_FRAMEBUFFER);

//		glEnable(GL_MULTISAMPLE);
	}
//	else
//		glDisable(GL_MULTISAMPLE);
#endif
}

void Select_MSAA(udword index)
{
#if PEEL_USE_MSAA
	if(index==gMSAA_Index)
		return;
	gMSAA_Index = index;
	Release_MSAA();
	if(index==0)
		gMSAA_NbSamples = 0;
	else if(index==1)
		gMSAA_NbSamples = 2;
	else if(index==2)
		gMSAA_NbSamples = 4;
	else if(index==3)
		gMSAA_NbSamples = 8;
	else if(index==4)
		gMSAA_NbSamples = 16;
	Resize_MSAA();
#endif
}
