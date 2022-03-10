///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains texture base code.
 *	\file		IceTexture.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETEXTURE_H
#define ICETEXTURE_H

	//! Supported texture formats
	enum TextureFormat
	{
		TF_16_555		= 1,			//!< 16 bits, no alpha
		TF_16_1555		= 2,			//!< 16 bits, alpha
		TF_16_4444		= 3,			//!< 16 bits, alpha
		TF_16_565		= 4,			//!< 16 bits, no alpha
		TF_32_888		= 5,			//!< 32 bits, no alpha
		TF_32_8888		= 6,			//!< 32 bits, alpha
		//
		TF_DXT1			= 7,			//!< DXT1 compressed
		TF_DXT2			= 8,			//!< DXT2 compressed
		TF_DXT3			= 9,			//!< DXT3 compressed
		TF_DXT4			= 10,			//!< DXT4 compressed
		TF_DXT5			= 11,			//!< DXT5 compressed
		//
		TF_CHOOSE_BEST	= 0x7fffffff
	};

	//! Cropping values & texture matrix
	struct ICERENDERER_API TextureTransform
	{
		TextureTransform()	{ Reset(); }

		void		Reset()
		{
			mCV.Reset();
			mTexMat.Identity();
		}

		void		Apply(Point& texture_vector)
		{
			// Apply texture matrix
			texture_vector *= mTexMat;

			// Apply crop values
			mCV.Apply(texture_vector.x, texture_vector.y);
		}

		CropValues	mCV;
		Matrix4x4	mTexMat;
	};

	//! Texture creation structure
	struct ICERENDERER_API TEXTURECREATE
	{
		TEXTURECREATE()
		{
			mWidth			= 256;
			mHeight			= 256;
			mStage			= 0;
			mIsDynamic		= false;
			mIsRenderTarget	= false;
			mIsCubemap		= false;
			mMipMap			= true;
			mAlpha			= true;
			mTF				= TF_32_8888;
		}

		udword			mWidth;				//!< Requested width
		udword			mHeight;			//!< Requested height
		udword			mStage;				//!< Texture stage
											//!< Caution: the mStage member doesn't matter on a lot of HW, but needs to be set explicitly on HW where
											//!< textures are stored in separate memory spaces depending on stage (3Dfx  Voodoo 2)for example)
		//udword		mMipmapLevel;
		bool			mIsDynamic;			//!< True for dynamic textures
		bool			mIsRenderTarget;	//!< True for render targets
		bool			mIsCubemap;			//!< True for cubemaps
		bool			mMipMap;			//!< True to enable mipmaps
		bool			mAlpha;				//!< Request alpha channel
		TextureFormat	mTF;				//!< Requested pixel format (or TF_CHOOSE_BEST if doesn't matter)
	};

	FUNCTION ICERENDERER_API bool Blit(const Picture& source, ubyte* dest, udword width, udword height, udword pitch, TextureFormat tf);

	//! Texture base class
	class ICERENDERER_API Texture : public RenderTarget
	{
		protected:
								Texture();
		virtual					~Texture();

		public:

		virtual	bool			Release()	= 0;

		// Setup picture

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Setups the source picture
		 *	\param		pic		[in] the source picture
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool			Load(const Picture& pic);

		// Lock / unlock

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Locks the texture surface.
		 *	\param		pitch		[out] the returned surface pitch
		 *	\param		level		[in] the mipmap level (unused in DX7)
		 *	\return		pointer to the physical surface, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	void*			Lock(udword& pitch, udword level=0)		= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Unlocks the texture surface.
		 *	\param		level		[in] the mipmap level (unused in DX7)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	void			Unlock(udword level=0)					= 0;

		// Settings
		inline_	void			SetNbMipMaps(udword nb)				{ mNbMipMap = nb;	}
		inline_	void			SetPixelFormat(TextureFormat tf)	{ mTF = tf;			}
		inline_	void			SetWidth(udword width)				{ mWidth = width;	}
		inline_	void			SetHeight(udword height)			{ mHeight = height;	}

		// Data access
		inline_	udword			GetWidth()					const	{ return mWidth;	}
		inline_	udword			GetHeight()					const	{ return mHeight;	}
		inline_	TextureFormat	GetPixelFormat()			const	{ return mTF;		}	//!< Returns the hardwired pixel format

		protected:

				udword			mWidth;		//!< Hardwired width
				udword			mHeight;	//!< Hardwired height
				udword			mNbMipMap;	//!< Number of mipmaps
				TextureFormat	mTF;		//!< Format of hardwired surface
	};

#endif // ICETEXTURE_H
