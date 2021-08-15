///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShadowShader.h"
#include "ShadowMap.h"
#include "PxMat44.h"
#include "GLShader.h"
#include "Camera.h"
#include "PEEL.h"
#include "PintShapeRenderer.h"
#include "PintRenderState.h"
#include "TextureManager.h"
#include "Scattering.h"

extern	bool	gExternalColorControl;

///////////////////////////////////////////////////////////////////////////////

ShadowShader::ShadowShader()
{
	for(int i = 0; i < mNumLights; i++)
		mShadowMaps[i] = NULL;
	mReflectionTexId = 0;

	for(int i = 0; i < 16; i++)
	{
		mCamModelView[i] = i%5 == 0 ? 1.0f : 0.0f;
		mCamProj[i]      = i%5 == 0 ? 1.0f : 0.0f;
	}
	mNumShadows = mNumLights;
	mShowReflection = true;
	shadowAmbient = 0.1f;
}

bool ShadowShader::init()
{
	for(int i = 0; i < mNumLights; i++)
		mShadowMaps[i] = NULL;

	// some defaults
	for(int i = 0; i < mNumLights; i++)
	{
		mSpotLightCosineDecayBegin[i] = 0.98f;
		mSpotLightCosineDecayEnd[i] = 0.997f;
		mSpotLightPos[i] = PxVec3(10.0f, 10.0f, 10.0f);
		mSpotLightDir[i] = PxVec3(0.0f, 0.0f, 0.0f) - mSpotLightPos[i];
		mSpotLightDir[i].normalize();
	}

	mBackLightDir = PxVec3(0, 20.0f, 20.0f);

	const GLShader::Loader vs(FindPEELFile("RenderModel_SimpleShadows_vs.h"));
	const GLShader::Loader ps(FindPEELFile("RenderModel_SimpleShadows_ps.h"));
	return loadShaderCode(vs.mData, ps.mData);
}

void ShadowShader::setSpotLight(int nr, const PxVec3& pos, PxVec3& dir, float decayBegin, float decayEnd)
{
	if(nr < 0 || nr >= mNumLights)
		return;

	mSpotLightPos[nr] = pos;
	mSpotLightDir[nr] = dir;

	mSpotLightCosineDecayBegin[nr] = decayBegin;
	mSpotLightCosineDecayEnd[nr] = decayEnd;
}

void ShadowShader::updateCamera(float* modelView, float* proj)
{
	for(int i = 0; i < 16; i++)
	{
		mCamModelView[i] = modelView[i];
		mCamProj[i] = proj[i];
	}
}

void ShadowShader::setShadowAmbient(float sa)
{
	shadowAmbient = sa;
}

void ShadowShader::activate(const ShaderProps& mat)
{
	const ShadowShaderProps& Mtl = static_cast<const ShadowShaderProps&>(mat);

	const GLuint mShaderProgram = mData.mProgram;

	Shader::activate(mat);

	{
		const GLuint mShaderProgram = mData.mProgram;

		// surface texture
		GLShader::SetUniform1i(mShaderProgram, "texture", 0);
		if(Mtl.texId > 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, Mtl.texId);
			GLShader::SetUniform1i(mShaderProgram, "useTexture", 1);
		}
		else
			GLShader::SetUniform1i(mShaderProgram, "useTexture", 0);

		GLShader::SetUniform1i(mShaderProgram, "envTexture", 1);
		GLShader::SetUniform1f(mShaderProgram, "envMapping", Mtl.environmentMappingCoeff);
		if(Mtl.envTexId > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, Mtl.envTexId);
		}

	//	GLShader::SetUniform1(mShaderProgram, "specularCoeff", mat.specularCoeff);

		//glColor4fv(Mtl.color);

		GLShader::SetUniform1f(mShaderProgram, "reflectionCoeff", mShowReflection ? Mtl.reflectionCoeff : 0.0f);

	//	printf("shadow ambient: %f\n", mat.shadowAmbient);
	//	GLShader::SetUniform1(mShaderProgram, "shadowAmbient", mat.shadowAmbient);
	}

	for(int i=0; i<mNumShadows; i++)
	{
		if(!mShadowMaps[i])
			return;
	}

	GLShader::SetUniform1i(mShaderProgram, "numShadows", mNumShadows);

	// the three shadow maps
	if(mShadowMaps[0])
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, mShadowMaps[0]->getDepthTexArray());
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

/*		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
*/
		const int size = mShadowMaps[0]->getTextureSize();
		GLShader::SetUniform2f(mShaderProgram, "texSize", float(size), 1.0f/float(size));
		//printf("size: %d\n", size);
	}
	GLShader::SetUniform1i(mShaderProgram, "stex", 2);

	if(mShadowMaps[1])
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, mShadowMaps[1]->getDepthTexArray());
		glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	}
	GLShader::SetUniform1i(mShaderProgram, "stex2", 3);

	if(mShadowMaps[2])
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, mShadowMaps[2]->getDepthTexArray());
		glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	}
	GLShader::SetUniform1i(mShaderProgram, "stex3", 4);

	glActiveTexture(GL_TEXTURE0);

	GLShader::SetUniform1f(mShaderProgram, "shadowAdd", -0.00001f);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, mReflectionTexId);
	GLShader::SetUniform1i(mShaderProgram, "reflectionTex", 5);

	PxMat44 camTrans(mCamModelView);
	PxVec3 eyePos, eyeDir;

	eyePos = camTrans.transform(mSpotLightPos[0]);
	eyeDir = camTrans.rotate(mSpotLightDir[0]); eyeDir.normalize();
	GLShader::SetUniform3f(mShaderProgram, "spotLightDir", eyeDir.x, eyeDir.y, eyeDir.z);
	GLShader::SetUniform3f(mShaderProgram, "spotLightPos", eyePos.x, eyePos.y, eyePos.z);
	GLShader::SetUniform1f(mShaderProgram, "spotLightCosineDecayBegin", mSpotLightCosineDecayBegin[0]);
	GLShader::SetUniform1f(mShaderProgram, "spotLightCosineDecayEnd", mSpotLightCosineDecayEnd[0]);

	eyePos = camTrans.transform(mSpotLightPos[1]);
	eyeDir = camTrans.rotate(mSpotLightDir[1]); eyeDir.normalize();
	GLShader::SetUniform3f(mShaderProgram, "spotLightDir2", eyeDir.x, eyeDir.y, eyeDir.z);
	GLShader::SetUniform3f(mShaderProgram, "spotLightPos2", eyePos.x, eyePos.y, eyePos.z);
	GLShader::SetUniform1f(mShaderProgram, "spotLightCosineDecayBegin2", mSpotLightCosineDecayBegin[1]);
	GLShader::SetUniform1f(mShaderProgram, "spotLightCosineDecayEnd2", mSpotLightCosineDecayEnd[1]);

	eyePos = camTrans.transform(mSpotLightPos[2]);
	eyeDir = camTrans.rotate(mSpotLightDir[2]); eyeDir.normalize();
	GLShader::SetUniform3f(mShaderProgram, "spotLightDir3", eyeDir.x, eyeDir.y, eyeDir.z);
	GLShader::SetUniform3f(mShaderProgram, "spotLightPos3", eyePos.x, eyePos.y, eyePos.z);
	GLShader::SetUniform1f(mShaderProgram, "spotLightCosineDecayBegin3", mSpotLightCosineDecayBegin[2]);
	GLShader::SetUniform1f(mShaderProgram, "spotLightCosineDecayEnd3", mSpotLightCosineDecayEnd[2]);

	eyeDir = camTrans.rotate(mBackLightDir); eyeDir.normalize();
	GLShader::SetUniform3f(mShaderProgram, "parallelLightDir", eyeDir.x, eyeDir.y, eyeDir.z);

	if(mShadowMaps[0])
	{
		GLShader::SetUniform4f(mShaderProgram, "far_d", 
			mShadowMaps[0]->getFarBound(0), 
			mShadowMaps[0]->getFarBound(1), 
			mShadowMaps[0]->getFarBound(2), 
			mShadowMaps[0]->getFarBound(3));
	}

	for(int i=0; i<mNumLights; i++)
	{
		if(mShadowMaps[i])
			mShadowMaps[i]->prepareForRender(mCamModelView, mCamProj);
	}

	shadowAmbient = Mtl.shadowAmbient;
	//printf("shadow ambient: %f\n", shadowAmbient);
	GLShader::SetUniform1f(mShaderProgram, "shadowAmbient", shadowAmbient);

	const Point CamPos = GetCameraPos();
	GLShader::SetUniform3f(mShaderProgram, "camPos", CamPos.x, CamPos.y, CamPos.z);

	const Point SunDir = GetSunDir();
	GLShader::SetUniform3f(mShaderProgram, "sunDir", SunDir.x, SunDir.y, SunDir.z);

	const Matrix4x4 Identity(Idt);
	setWorldMatrix(&Identity.m[0][0]);
}

void ShadowShader::deactivate()
{
	Shader::deactivate();

	for(int i=0; i<mNumLights; i++)
	{
		if(mShadowMaps[i])
			mShadowMaps[i]->doneRender();
	}

	// ### 'doneRender' above changes the matrix mode
	glMatrixMode(GL_MODELVIEW);

	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE4);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
}

void ShadowShader::SetupGeometry(const PintShapeRenderer* renderer)
{
	Shader::SetupGeometry(renderer);

	// ### optimize SetUniform1i

	GLShader::SetUniform1i(mData.mProgram, "useExplicitVertexNormals", renderer->_UseExplicitVertexNormals());
}

void ShadowShader::SetupMaterial(const PintShapeRenderer* renderer)
{
	Shader::SetupMaterial(renderer);

	// Reuse that bool (which was for the main color) for textures. Problem is that the shader doesn't blend the
	// current color with the texture so textured selected shapes don't work yet
	if(!gExternalColorControl)
	{
		// ### all these vcalls aren't great. Maybe we could use a double-dispatch between Shaders & Shapes here
		// This only affects our shader so we don't need to reset the FFP OpenGL stuff?
		const ManagedTexture* MT = renderer->GetTexture();
		if(MT && MT->mGLID)
		{
			glActiveTexture(GL_TEXTURE0);
	//		glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, MT->mGLID);
			GLShader::SetUniform1i(mData.mProgram, "useTexture", 1);
		}
		else
		{
			GLShader::SetUniform1i(mData.mProgram, "useTexture", 0);
		}
	}
}
