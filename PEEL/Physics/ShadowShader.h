///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SHADOW_SHADER_DIFFUSE_H
#define SHADOW_SHADER_DIFFUSE_H

#include "Shader.h"
#include "PxVec3.h"
using namespace physx;
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

		udword texId;
		udword envTexId;
		float environmentMappingCoeff;
//		float color[4];
//		float ambientCoeff;
//		float diffuseCoeff;
		float specularCoeff;
		float reflectionCoeff;
//		float refractionCoeff;
		float shadowAmbient;
//		float shadowCoeff;
	};

	class ShadowShader : public PEEL::Shader
	{
		public:
							ShadowShader();
		virtual				~ShadowShader(){}

				bool		init();

				void		setSpotLight(int nr, const PxVec3& pos, PxVec3& dir, float decayBegin = 0.98f, float decayEnd = 0.997f);
				void		setBackLightDir(const PxVec3& dir) { mBackLightDir = dir; }

				void		updateCamera(float* modelView, float* proj);

				void		setShadowMap(int nr, ShadowMap *shadowMap) { mShadowMaps[nr] = shadowMap; }
				void		setNumShadows(int num) { mNumShadows = num; }
				void		setShowReflection(bool show)	{ 	mShowReflection = show;}
				bool		getShowReflection() const {return mShowReflection;}

				void		setReflectionTexId(udword texId) { mReflectionTexId = texId; }

		//	Shader
		virtual void		activate(const ShaderProps& mat)					override;
		virtual void		deactivate()										override;
		virtual	void		SetupGeometry(const PintShapeRenderer* renderer)	override;
		virtual	void		SetupMaterial(const PintShapeRenderer* renderer)	override;
		//~Shader

				void		setShadowAmbient(float sa);
		
		private:
		static const int	mNumLights = 3;

				ShadowMap*	mShadowMaps[mNumLights];

				PxVec3		mSpotLightPos[mNumLights];
				PxVec3		mSpotLightDir[mNumLights];

				float		mSpotLightCosineDecayBegin[mNumLights];
				float		mSpotLightCosineDecayEnd[mNumLights];

				GLuint		mReflectionTexId;

				PxVec3		mBackLightDir;
				int			mNumShadows;
				bool		mShowReflection;

				float		mCamModelView[16];
				float		mCamProj[16];
				float		shadowAmbient;
	};

#endif