///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SimpleShader.h"
#include "PxMat44.h"
#include "GLShader.h"
#include "PEEL.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

SimpleShader::SimpleShader(bool needs_vertex_normals) :
	mNeedsVertexNormals(needs_vertex_normals)
{
	for (int i = 0; i < 16; i++)
	{
		mCamModelView[i] = i%5 == 0 ? 1.0f : 0.0f;
		mCamProj[i]      = i%5 == 0 ? 1.0f : 0.0f;
	}
}

SimpleShader::~SimpleShader()
{
}

bool SimpleShader::Init()
{
	const char* vsProg = mNeedsVertexNormals ? "RenderModel_SimpleShader1_vs.h" : "RenderModel_SimpleShader2_vs.h";
	const char* psProg = mNeedsVertexNormals ? "RenderModel_SimpleShader1_ps.h" : "RenderModel_SimpleShader2_ps.h";

	const GLShader::Loader vs(FindPEELFile(vsProg));
	const GLShader::Loader ps(FindPEELFile(psProg));
	return LoadShaderCode(vs.mData, ps.mData);
}

void SimpleShader::__UpdateCamera(const float* modelView, const float* proj)
{
	for (int i = 0; i < 16; i++)
	{
		mCamModelView[i] = modelView[i];
		mCamProj[i] = proj[i];
	}
}

void SimpleShader::Activate(const ShaderProps& mat)
{
	const SimpleShaderProps& Mtl = static_cast<const SimpleShaderProps&>(mat);

	Shader::Activate(mat);

	const PxMat44 camTrans(mCamModelView);
	PxVec3 eyeDir = camTrans.rotate((const PxVec3&)Mtl.mBackLightDir); eyeDir.normalize();

	const GLuint mShaderProgram = mData.mProgram;
	GLShader::SetUniform3f(mShaderProgram, "parallelLightDir", eyeDir.x, eyeDir.y, eyeDir.z);
}
