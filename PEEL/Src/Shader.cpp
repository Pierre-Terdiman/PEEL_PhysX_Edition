///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Shader.h"
#include <assert.h>

#include "Shader.h"
#include "PintRenderState.h"
#include "PintShapeRenderer.h"

// Hack for editor / selection, not sure how to do this in a cleaner way
bool gExternalColorControl = false;

PEEL::Shader* gCurrentShader = null;

PEEL::Shader::Shader() : mActive(FALSE)
{
}

PEEL::Shader::~Shader()
{
	DeleteShaders();
}

bool PEEL::Shader::LoadShaderCode(const char* vertexShaderCode, const char* fragmentShaderCode)
{
	return LoadShaderCode(vertexShaderCode, NULL, fragmentShaderCode);
}

bool PEEL::Shader::LoadShaderCode(const char* vertexShaderCode, const char* geometryShaderCode,  const char* fragmentShaderCode)
{
	mData = GLShader::CompileProgram(vertexShaderCode, fragmentShaderCode, geometryShaderCode);
	return mData.mProgram!=0;
}

void PEEL::Shader::DeleteShaders()
{
	GLShader::ReleaseProgram(mData);
}

void PEEL::Shader::Activate(const ShaderProps& mat)
{
	if(mData.mProgram)
	{
		glUseProgram(mData.mProgram);
		mActive = TRUE;
		gCurrentShader = this;
	}
}

void PEEL::Shader::Deactivate()
{
	if(mData.mProgram)
	{
#ifdef _DEBUG
		GLint curProg;
		glGetIntegerv(GL_CURRENT_PROGRAM, &curProg);
		ASSERT(curProg == mData.mProgram);
#endif
		glUseProgram(0);
		mActive = FALSE;
		gCurrentShader = null;
	}
}

void PEEL::Shader::SetupGeometry(const PintShapeRenderer* renderer)
{
}

void PEEL::Shader::SetupMaterial(const PintShapeRenderer* renderer)
{
	// ### optimize this - maybe cache last set color
	if(1 && !gExternalColorControl)
	{
		const RGBAColor* Color = renderer->GetColor();
		if(Color)
			SetMainColor(*Color);
		else
			ResetMainColor();
	}
}

void PEEL::Shader::SetWorldMatrix(const float* m)
{
	if(mActive)
	{
		const udword loc = glGetUniformLocation(mData.mProgram, "worldMatrix");
		glUniformMatrix4fv(loc, 1, GL_FALSE, m);
	}
}

#include "PintShapeRenderer.h"

extern	bool	gWireframePass;
extern	bool	gShadowPass;

void SetupMaterial(const PintShapeRenderer* renderer)
{
	if(gCurrentShader)
	{
		//if(!(renderer->mBaseFlags & SHAPE_RENDERER_USES_BATCHING))
			gCurrentShader->SetupMaterial(renderer);
	}
}

void RenderShape(const PintShapeRenderer* renderer, const PR& pose)
{
	if(gCurrentShader)
	{
		ASSERT(!gWireframePass);
		ASSERT(!gShadowPass);
		//if(!(renderer->mBaseFlags & SHAPE_RENDERER_USES_BATCHING))
			gCurrentShader->SetupGeometry(renderer);
	}

	renderer->_Render(pose);
}

// ### this should change with batching
/*void SetupMaterialAndRenderShape(const PintShapeRenderer* renderer, const PR& pose)
{
	if(gCurrentShader)
	{
		ASSERT(!gWireframePass);
		ASSERT(!gShadowPass);
		//if(!(renderer->mBaseFlags & SHAPE_RENDERER_USES_BATCHING))
		{
			gCurrentShader->SetupMaterial(renderer);
			gCurrentShader->SetupGeometry(renderer);
		}
	}

	renderer->_Render(pose);
}*/
