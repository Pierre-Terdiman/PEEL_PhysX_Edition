///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H

#include "Shader.h"

	struct SimpleShaderProps : ShaderProps
	{
		Point	mBackLightDir;
	};

	class SimpleShader : public PEEL::Shader
	{
						PREVENT_COPY(SimpleShader)
		public:
						SimpleShader(bool needs_vertex_normals);
		virtual			~SimpleShader();

				bool	init();

				void	updateCamera(float* modelView, float* proj);

		// Shader
		virtual	void	activate(const ShaderProps& mat)	override;
		//~Shader
		
		private:
				float	mCamModelView[16];
				float	mCamProj[16];
		const	bool	mNeedsVertexNormals;
	};

#endif