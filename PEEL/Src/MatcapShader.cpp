///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MatcapShader.h"
#include "PxMat44.h"
#include "GLShader.h"
#include "PEEL.h"
#include "PintShapeRenderer.h"
#include "PintRenderState.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

MatcapShader::MatcapShader()
{
	for (int i = 0; i < 16; i++)
	{
		mCamModelView[i] = i%5 == 0 ? 1.0f : 0.0f;
		mCamProj[i]      = i%5 == 0 ? 1.0f : 0.0f;
	}
}

MatcapShader::~MatcapShader()
{
}

bool MatcapShader::Init()
{
//	mBackLightDir = PxVec3(0.0f, 20.0f, 20.0f);

	const GLShader::Loader vs(FindPEELFile("RenderModel_Mapcap_vs.h"));
	const GLShader::Loader ps(FindPEELFile("RenderModel_Mapcap_ps.h"));
	return LoadShaderCode(vs.mData, ps.mData);
}

void MatcapShader::__UpdateCamera(const float* modelView, const float* proj)
{
	for (int i = 0; i < 16; i++)
	{
		mCamModelView[i] = modelView[i];
		mCamProj[i] = proj[i];
	}
}

void MatcapShader::Activate(const ShaderProps& mat)
{
	const MatcapShaderProps& Mtl = static_cast<const MatcapShaderProps&>(mat);

	Shader::Activate(mat);

	{
//		glColor4fv(mat.color);

		const GLuint mShaderProgram = mData.mProgram;
		GLShader::SetUniform1i(mShaderProgram, "envTexture", 1);
		GLShader::SetUniform1f(mShaderProgram, "envMapping", Mtl.mEnvironmentMappingCoeff);
		if(Mtl.mEnvTexId > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, Mtl.mEnvTexId);
		}
	}

	const PxMat44 camTrans(mCamModelView);
	PxVec3 eyeDir = camTrans.rotate((const PxVec3&)Mtl.mBackLightDir); eyeDir.normalize();

	const GLuint mShaderProgram = mData.mProgram;
	GLShader::SetUniform3f(mShaderProgram, "parallelLightDir", eyeDir.x, eyeDir.y, eyeDir.z);
}

void MatcapShader::Deactivate()
{
	Shader::Deactivate();

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
}

void MatcapShader::SetupGeometry(const PintShapeRenderer* renderer)
{
	Shader::SetupGeometry(renderer);

	// ### optimize SetUniform1i

	GLShader::SetUniform1i(mData.mProgram, "useExplicitVertexNormals", renderer->_UseExplicitVertexNormals());
}
