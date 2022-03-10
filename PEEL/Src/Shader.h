///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SHADER_H
#define SHADER_H

#include "GLShader.h"

	class PintShapeRenderer;

	struct ShaderProps
	{
	};

	struct ShaderMaterial
	{
	};

namespace PEEL	// To avoid conflicts with the ICE Shader class
{
	class Shader : public Allocateable
	{
	public:
								Shader();
		virtual					~Shader();

		virtual	void			Activate(const ShaderProps& mat);
		virtual	void			Deactivate();
		virtual	void			SetupGeometry(const PintShapeRenderer* renderer);
		virtual	void			SetupMaterial(const PintShapeRenderer* renderer);

		operator GLuint ()
		{
			return mData.mProgram;
		}

				bool			LoadShaderCode(const char* vertexShaderCode, const char* fragmentShaderCode);
				bool			LoadShaderCode(const char* vertexShaderCode, const char* geometryShaderCode,  const char* fragmentShaderCode);
				void			DeleteShaders();

				void			SetWorldMatrix(const float* m);

				GLShader::Data	mData;
				BOOL			mActive;
	};
}

	void	SetupMaterial(const PintShapeRenderer* renderer);
	void	RenderShape(const PintShapeRenderer* renderer, const PR& pose);
//	void	SetupMaterialAndRenderShape(const PintShapeRenderer* renderer, const PR& pose);

#endif