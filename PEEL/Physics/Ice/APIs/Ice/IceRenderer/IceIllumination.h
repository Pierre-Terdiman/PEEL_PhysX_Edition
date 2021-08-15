///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains illumination code.
 *	\file		IceIllumination.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEILLUMINATION_H
#define ICEILLUMINATION_H

	typedef bool (*SHADOW_CALLBACK)	(const Ray& ray, float maxdist, void* user_data);

	class ICERENDERER_API IlluminationModel
	{
		public:
										IlluminationModel();
										~IlluminationModel();

				bool					Compute(const Point& p, const Point& normal, RGBAColor& color)	const;
		// Settings
		inline_	void					SetAmbientOcclusion(bool flag)				{ mAmbientOcclusion = flag;							}
		inline_	void					SetAmbientColor(const RGBAColor& color)		{ mAmbientColor = color; mValidateSpecular = true;	}
		inline_	void					SetMaterial(const MaterialProps& material)	{ mMaterial = material;								}
		inline_	void					SetViewPoint(const Point& view)				{ mViewPoint = view;								}
		inline_	void					SetSelfShadowEpsilon(float epsilon)			{ mSelfShadowEpsilon = epsilon;						}
		inline_	void					SetAmbientOcclusionRadius(float radius)		{ mAmbientOcclusionRadius = radius;					}
		inline_	void					SetColorMultiplier(float multiplier)		{ mColorMultiplier = multiplier;					}

				bool					AddLight(const LightProps& light);
		// Data access
		inline_	const RGBAColor*		GetAmbientColor()					const	{ return &mAmbientColor;							}
		inline_	const MaterialProps*	GetMaterial()						const	{ return &mMaterial;								}
		inline_	const Point*			GetViewPoint()						const	{ return &mViewPoint;								}

		// Callback control
		inline_	void					SetUserData(void* data)						{ mUserData	= data;									}
		inline_	void					SetShadowCallback(SHADOW_CALLBACK callback)	{ mCallback	= callback;								}

		private:
		// User callback
				void*					mUserData;			//!< User-defined data sent to callbacks
				SHADOW_CALLBACK			mCallback;
				float					mSelfShadowEpsilon;
				float					mAmbientOcclusionRadius;
				float					mColorMultiplier;

				RGBAColor				mAmbientColor;		//!< Global ambient color
				MaterialProps			mMaterial;			//!< Material properties
				Point					mViewPoint;
				PtrContainer			mLights;
				bool					mValidateSpecular;
				bool					mAmbientOcclusion;
	};

#endif // ICEILLUMINATION_H
