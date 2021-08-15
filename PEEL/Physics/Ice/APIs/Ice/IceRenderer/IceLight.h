///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a hardware light class.
 *	\file		IceLight.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICELIGHT_H
#define ICELIGHT_H

	//! Possible light types
	enum LightType
	{
		LIGHT_POINT			= 0x1,				//!< Light is a point source. The light has a position in space and radiates light in all directions.
		LIGHT_SPOT			= 0x2,				//!< Light is a spotlight source. This light is like a point light, except that the illumination is limited to a cone.
												//!< This light type has a direction and several other parameters that determine the shape of the cone it produces.
		LIGHT_DIRECT		= 0x3,				//!< Light is a directional source. This is equivalent to using a point light source at an infinite distance.

		LIGHT_FORCE_DWORD	= 0x7FFFFFFF,
	};

	//! Light properties
	struct ICERENDERER_API LightProps : public Allocateable
	{
		// Nothing virtual here to map hardware structure
							LightProps();

				LightType	mType;				//!< Type of light source
				RGBAColor	mDiffuseColor;		//!< Diffuse color of light
				RGBAColor	mSpecularColor;		//!< Specular color of light
				RGBAColor	mAmbientColor;		//!< Ambient color of light
				Point		mPosition;			//!< Position in world space
				Point		mDirection;			//!< Direction in world space (make sure the magnitude is not null there)
				float		mRange;				//!< Cutoff range
				float		mFalloff;			//!< Falloff
				float		mAttenuation0;		//!< Constant attenuation
				float		mAttenuation1;		//!< Linear attenuation
				float		mAttenuation2;		//!< Quadratic attenuation
				float		mTheta;				//!< Inner angle of spotlight cone (in radians)
				float		mPhi;				//!< Outer angle of spotlight cone (in radians)
	};

	class ICERENDERER_API Light
	{
		public:
							Light() : mVI(0xffff), mUsed(false)	{}
		virtual				~Light()							{}

				LightProps	mProps;				//!< Light properties
				uword		mVI;				//!< Virtual index = light identifier
				bool		mUsed;				//!< True if the light is on at hardware level
	};

#endif // ICELIGHT_H
