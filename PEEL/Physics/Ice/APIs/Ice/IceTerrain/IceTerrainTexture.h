///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains terrain texture synthesis code.
 *	\file		IceTerrainTexture.h
 *	\author		Pierre Terdiman
 *	\date		March, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETERRAINTEXTURE_H
#define ICETERRAINTEXTURE_H

	struct ICETERRAIN_API TerrainSun
	{
		Point				mPos;
		float				mAmbient;
		float				mDiffuse;
		float				mIntensity;
		float				mSpecular;
		float				mShininess;
	};

	struct ICETERRAIN_API TERRAINTEXTURECREATE
	{
							TERRAINTEXTURECREATE();

		const float*		mField;			//!< Heightfield, size is mWidth * mHeight
		const float*		mMinMax;		//!< Forced min/max value for heightfield. Null to auto-compute.
		const Point*		mNormals;		//!< "vertex normals", or null
		const RGBPalette*	mColorRamp;		//!< Color palette, or null
		const TerrainSun*	mSun;			//!< Sun data, or null
		udword				mWidth;			//!< Width of heightfield
		udword				mHeight;		//!< Height of heightfield
		float				mSeaLevel;		//!< Altitude of sea level
		bool				mRenderWater;	//!< Render water of not
	};

	class ICETERRAIN_API TerrainTexture : public Picture
	{
		public:
										TerrainTexture();
										~TerrainTexture();

						bool			Create(const TERRAINTEXTURECREATE& create);
						bool			Create2(const TERRAINTEXTURECREATE& create, const Picture& base_texture, const Picture& base_texture2, float length, const Point& pos);

		inline_	const	RGBPalette&		GetColorRamp()	const	{ return mColorRamp;	}

						Picture			mBase;
		private:
						RGBPalette		mColorRamp;
	};

#endif // ICETERRAINTEXTURE_H
