///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SHADOW_SHADER_H
#define SHADOW_SHADER_H

#include "Shader.h"
//#define USE_FOG

	class ShadowMap;

	struct ShadowShaderProps : ShaderProps
	{
		void init()
		{
			texId = 0;
			envTexId = 0;
			environmentMappingCoeff = 0.0f;
//			ambientCoeff = 0.0f;
//			diffuseCoeff = 1.0f;
			specularCoeff = 0.0f;
			reflectionCoeff = 0.0f;
//			refractionCoeff = 0.0f;
//			shadowCoeff = 0.0f;
			shadowAmbient = 0.1f;
			shadowOffset = 0.0f;
//			color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f; color[3] = 1.0f;
		}

/*		void setColor(float r, float g, float b, float a = 1.0f)
		{
			color[0] = r; color[1] = g; color[2] = b; color[3] = a;
		}

		bool operator == (const ShaderProps& m) const
		{
			if (texId != m.texId) return false;
			if (color[0] != m.color[0] || color[1] != m.color[1] || color[2] != m.color[2] || color[3] != m.color[3]) return false;
			return true;
		}*/

		udword	texId;
		udword	envTexId;
		float	environmentMappingCoeff;
//		float	color[4];
//		float	ambientCoeff;
//		float	diffuseCoeff;
		float	specularCoeff;
		float	reflectionCoeff;
//		float	refractionCoeff;
		float	shadowAmbient;
//		float	shadowCoeff;
		float	shadowOffset;
	};

	class ShadowShader : public PEEL::Shader
	{
		public:
							ShadowShader();
		virtual				~ShadowShader(){}

				bool		Init();

				void		SetSpotLight(int nr, const Point& pos, const Point& dir, float decayBegin = 0.98f, float decayEnd = 0.997f);
		inline_	void		SetBackLightDir(const Point& dir)			{ mBackLightDir = dir;			}

				void		__UpdateCamera(const float* modelView, const float* proj);

		inline_	void		SetShadowMap(int nr, ShadowMap* shadowMap)	{ mShadowMaps[nr] = shadowMap;	}
		inline_	void		SetNumShadows(int num)						{ mNumShadows = num;			}

		inline_	void		SetShowReflection(bool show)				{ mShowReflection = show;		}
		inline_	bool		GetShowReflection()					const	{ return mShowReflection;		}

		inline_	void		SetReflectionTexId(udword texId)			{ mReflectionTexId = texId;		}
		inline_	void		SetReflectionWidth(udword width)			{ mReflectionWidth = width;		}
		inline_	void		SetReflectionHeight(udword width)			{ mReflectionHeight = width;	}

		//	Shader
		virtual void		Activate(const ShaderProps& mat)					override;
		virtual void		Deactivate()										override;
		virtual	void		SetupGeometry(const PintShapeRenderer* renderer)	override;
		virtual	void		SetupMaterial(const PintShapeRenderer* renderer)	override;
		//~Shader

		private:
		static const int	mNumLights = 3;

				ShadowMap*	mShadowMaps[mNumLights];

				Point		mSpotLightPos[mNumLights];
				Point		mSpotLightDir[mNumLights];

				float		mSpotLightCosineDecayBegin[mNumLights];
				float		mSpotLightCosineDecayEnd[mNumLights];

				GLuint		mReflectionTexId;
				udword		mReflectionWidth;
				udword		mReflectionHeight;

				Point		mBackLightDir;
				int			mNumShadows;
				bool		mShowReflection;

				float		mCamModelView[16];
				float		mCamProj[16];
	};

#endif