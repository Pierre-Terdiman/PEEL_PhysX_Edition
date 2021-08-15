///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a pixelformat class.
 *	\file		IcePixelFormat.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPIXELFORMAT_H
#define ICEPIXELFORMAT_H

	// Pixel Format flags
	#define	PIXFMT_RGB			0x0001		//!< Pixel is RGB format
	#define	PIXFMT_YUV			0x0002		//!< Pixel is YUV format
	#define	PIXFMT_ALPHA		0x0004		//!< Pixel contain an alpha channel
	#define	PIXFMT_PAL1			0x0010		//!< Palettized of 1 entry
	#define	PIXFMT_PAL2			0x0020		//!< Palettized of 2 entries
	#define	PIXFMT_PAL4			0x0040		//!< Palettized of 4 entries
	#define	PIXFMT_PAL8			0x0080		//!< Palettized of 8 entries
	#define	PIXFMT_PALTO8		0x0100		//!< 1, 2, 4 bit color indexed into a 8 bits palette
	#define	PIXFMT_COMPRESSED	0x0200		//!< Compressed texture

	enum FourCC
	{
		FOURCC_UNCOMPRESSED = 0,
		FOURCC_DXT1			= 1,
		FOURCC_DXT2			= 2,
		FOURCC_DXT3			= 3,
		FOURCC_DXT4			= 4,
		FOURCC_DXT5			= 5,

		FOURCC_UNKNOWN		= 0x7fffffff
	};

	class ICERENDERER_API PixelFormat
	{
		public:
		inline_			PixelFormat()	{}
		inline_			~PixelFormat()	{}

		PixelFormat&	Set(udword flags, udword bitcount, udword rmask, udword gmask, udword bmask, udword amask)
		{
			mFlags			= flags;
			mRGBBitCount	= bitcount;
			mRedMask		= rmask;
			mGreenMask		= gmask;
			mBlueMask		= bmask;
			mAlphaMask		= amask;
			return *this;
		}

		udword			mFlags;
		udword			mRGBBitCount;
		udword			mRedMask;
		udword			mGreenMask;
		udword			mBlueMask;
		udword			mAlphaMask;
	};

	FUNCTION ICERENDERER_API FourCC TranslateFourCC(udword fourcc, String* str=null);

/*
	udword Flag = 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_RGB)				? PIXFMT_RGB		: 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_YUV)				? PIXFMT_YUV		: 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_ALPHAPIXELS)		? PIXFMT_ALPHA		: 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXED1)	? PIXFMT_PAL1		: 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXED2)	? PIXFMT_PAL2		: 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXED4)	? PIXFMT_PAL4		: 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXED8)	? PIXFMT_PAL8		: 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_PALETTEINDEXEDTO8)? PIXFMT_PALTO8		: 0;
	Flag |= (lpDDPixFmt->dwFlags&DDPF_COMPRESSED)		? PIXFMT_COMPRESSED	: 0;
*/

#endif // ICEPIXELFORMAT_H
