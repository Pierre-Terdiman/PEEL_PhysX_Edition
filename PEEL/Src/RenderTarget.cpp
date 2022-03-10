#include "stdafx.h"
#include "RenderTarget.h"
#include "TextureManager.h"

PEEL::RenderTarget::RenderTarget(udword width, udword height) : mFBO(null), mColorTexId(0), mDepthTexId(0), mWidth(width), mHeight(height), mColorTexture(null), mDepthTexture(null)
{
	Resize(width, height);
}

void PEEL::RenderTarget::Clear()
{
	if(mColorTexId > 0)
	{
		glDeleteTextures(1, &mColorTexId);
		mColorTexId = 0;
	}
	if(mDepthTexId > 0)
	{
		glDeleteTextures(1, &mDepthTexId);
		mDepthTexId = 0;
	}
	DELETESINGLE(mFBO);
}

PEEL::RenderTarget::~RenderTarget()
{
	Clear();
}

//static GLenum gTexTarget = GL_TEXTURE_RECTANGLE_ARB;
static GLenum gTexTarget = GL_TEXTURE_2D;

static GLuint CreateTexture(GLenum target, int width, int height, GLint internalFormat, GLenum format)
{
	GLuint texid;
    glGenTextures(1, &texid);
    glBindTexture(target, texid);

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(target, 0, internalFormat, width, height, 0, format, GL_FLOAT, 0);
    //glTexImage2D(target, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, OGL_FONT_TEXTURE_WIDTH, OGL_FONT_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, P);

    return texid;
}

void PEEL::RenderTarget::Resize(udword width, udword height)
{
	Clear();

//	width/=2;
//	height/=2;

	mFBO = new FrameBufferObject();
	mColorTexId = ::CreateTexture(gTexTarget, width, height, GL_RGBA8, GL_RGBA);
	mDepthTexId = ::CreateTexture(gTexTarget, width, height, GL_DEPTH_COMPONENT32_ARB, GL_DEPTH_COMPONENT);

	if(!mColorTexture)
		mColorTexture = CreateSystemTexture(width, height, mColorTexId, "RenderTarget_Color");
	else
	{
		mColorTexture->mGLID = mColorTexId;
		mColorTexture->mWidth = width;
		mColorTexture->mHeight = height;
	}

	if(!mDepthTexture)
		mDepthTexture = CreateSystemTexture(width, height, mDepthTexId, "RenderTarget_Depth");
	else
	{
		mDepthTexture->mGLID = mDepthTexId;
		mDepthTexture->mWidth = width;
		mDepthTexture->mHeight = height;
	}

	mWidth	= width;
	mHeight	= height;
}

void PEEL::RenderTarget::BeginCapture()
{
	mFBO->Bind();
    mFBO->AttachTexture(gTexTarget, mColorTexId, GL_COLOR_ATTACHMENT0_EXT);
    mFBO->AttachTexture(gTexTarget, mDepthTexId, GL_DEPTH_ATTACHMENT_EXT);
    mFBO->IsValid();
}

void PEEL::RenderTarget::EndCapture()
{
    mFBO->AttachTexture(gTexTarget, 0, GL_COLOR_ATTACHMENT0_EXT);
    mFBO->AttachTexture(gTexTarget, 0, GL_DEPTH_ATTACHMENT_EXT);
    mFBO->Disable();
}



