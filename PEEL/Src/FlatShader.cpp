///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FlatShader.h"
#include "PxMat44.h"
#include "GLShader.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

static const char* simpleVertexProgram = "#version 130\n" STRINGIFY(

void main()
{
	gl_Position = ftransform();
}
);

///////////////////////////////////////////////////////////////////////////////

static const char* simpleFragmentProgram = "#version 130\n" STRINGIFY(

void main()
{
	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
);

///////////////////////////////////////////////////////////////////////////////

FlatShader::FlatShader()
{
	for (int i = 0; i < 16; i++)
	{
		mCamModelView[i] = i%5 == 0 ? 1.0f : 0.0f;
		mCamProj[i]      = i%5 == 0 ? 1.0f : 0.0f;
	}
}

FlatShader::~FlatShader()
{
}

bool FlatShader::Init()
{
	const char* vsProg = simpleVertexProgram;
	const char* psProg = simpleFragmentProgram;
	return LoadShaderCode(vsProg, psProg);
}

void FlatShader::__UpdateCamera(const float* modelView, const float* proj)
{
	for (int i = 0; i < 16; i++)
	{
		mCamModelView[i] = modelView[i];
		mCamProj[i] = proj[i];
	}
}

