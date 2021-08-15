///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef MATCAP_SHADER_H
#define MATCAP_SHADER_H

#include "Shader.h"

	struct MatcapShaderProps : ShaderProps
	{
		Point	mBackLightDir;
		udword	mEnvTexId;
		float	mEnvironmentMappingCoeff;
	};

	class MatcapShader : public PEEL::Shader
	{
						PREVENT_COPY(MatcapShader)
		public:
						MatcapShader();
		virtual			~MatcapShader();

				bool	init();

				void	updateCamera(float* modelView, float* proj);

		// Shader
		virtual	void	activate(const ShaderProps& mat)					override;
		virtual void	deactivate()										override;
		virtual	void	SetupGeometry(const PintShapeRenderer* renderer)	override;
		//~Shader
		
		private:
				float	mCamModelView[16];
				float	mCamProj[16];
	};

#endif