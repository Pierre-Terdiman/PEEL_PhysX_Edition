///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	TXMP chunk for ZB2 format.
 *	\file		ChunkTXMP.h
 *	\author		Pierre Terdiman
 *	\date		August, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKTXMP_H
#define CHUNKTXMP_H

	#define TXMP_VERSION	1

	enum ZCB2_TXMP_Flag
	{
		ZCB2_TXMP_BITMAP_INCLUDED	= (1<<0),
		ZCB2_TXMP_URL				= (1<<1),
	};

	enum ZCB2_TXMP_Format
	{
		ZCB2_TXMP_8888			= 0,		//!< RGBA 32 bits (default)
		ZCB2_TXMP_888			= 1,		//!< RGB 24 bits
		ZCB2_TXMP_565			= 2,		//!< RGB 16 bits
		ZCB2_TXMP_4444			= 3,		//!< RGBA 16 bits
		ZCB2_TXMP_1555			= 4,		//!< RGBA 16 bits
		ZCB2_TXMP_555			= 5,		//!< RGBA 15 bits
		ZCB2_TXMP_PALETTE		= 6,		//!< Paletted
		ZCB2_TXMP_DXT1			= 7,		//!< DXT1 compressed
		ZCB2_TXMP_DXT2			= 8,		//!< DXT2 compressed
		ZCB2_TXMP_DXT3			= 9,		//!< DXT3 compressed
		ZCB2_TXMP_DXT4			= 10,		//!< DXT4 compressed
		ZCB2_TXMP_DXT5			= 11,		//!< DXT5 compressed

		ZCB2_TXMP_FORCE_DWORD	= 0x7fffffff
	};

	class ZCB2_API TXMPChunk : public ROBJChunk
	{
									DECLARE_CHUNK(TXMPChunk, mTXMPCore)
		// Data access
		inline_	bool				IsBitmapIncluded()		const	{ return (mTXMPCore.mFlags&ZCB2_TXMP_BITMAP_INCLUDED)!=0;	}
		inline_	bool				MustBeDownloaded()		const	{ return (mTXMPCore.mFlags&ZCB2_TXMP_URL)!=0;				}
		inline_	const Picture*		GetBitmap()				const	{ return mBitmap;											}
		inline_	const char*			GetFilename()			const	{ return mFileName;											}
		inline_	const char*			GetURL()				const	{ return mURL;												}
		inline_	udword				GetWidth()				const	{ return mWidth;											}
		inline_	udword				GetHeight()				const	{ return mHeight;											}

		// Chunk definition

		// Bitmap
				bool				SetFormat(ZCB2_TXMP_Format format);
				bool				SetSize(udword width, udword height);
				bool				SetSourceBitmap(const Picture* pic, const char* filename, const char* url);
		private:
		// Chunk data
		// Attributes
				ZCB2_TXMP_Format	mFormat;
				udword				mWidth, mHeight;
		// Possible sources
				Picture*			mBitmap;
				String				mFileName;
				String				mURL;
	};

#endif // CHUNKTXMP_H
