///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple flat renderer.
 *	\file		IceFlat.h
 *	\author		Pierre Terdiman
 *	\date		August, 15, 1998
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFLAT_H
#define ICEFLAT_H

#ifndef _WIN64
	//! Flat renderer creation structure
	struct ICERENDERER_API FLATRENDERERCREATE
	{
					FLATRENDERERCREATE();

		ubyte*		mBuffer;
		udword		mWidth;
		udword		mHeight;
		udword		mPitch;
		udword		mColor;
	};

	struct PixelData
	{
		udword Type;
		float Z;
		float Alpha;
		float Beta;
		float Gamma;
		float dZx;
		float dAlphax;
		float dBetax;
		float dGammax;
	};

	typedef void (*PixelCallback) (RGBAColor& pixel_color, const PixelData& pixel_data, void* user_data);

	class ICERENDERER_API FlatRenderer
	{
		public:
								FlatRenderer();
								~FlatRenderer();

					bool		Init(const FLATRENDERERCREATE& create);
					void		DrawTriangleASM(const Triangle* poly);
					void		DrawTriangleC(const Triangle* poly);
					void		DrawTriangleC2(const Triangle* poly);
					void		DrawTriangleC3(const Triangle* poly);
					void		DrawTriangle(const Triangle* poly, PixelCallback callback, void* user_data);
		// Settings
		inline_		void		SetColor(udword color)		{ mColor = color;	}

		private:
					ubyte*		mBuffer;
					udword		mWidth;
					udword		mHeight;
					udword		mPitch;
					udword		mColor;
					udword*		mMulTable;
					bool		mMustBeReleased;
	};
#endif

#endif // ICEFLAT_H
