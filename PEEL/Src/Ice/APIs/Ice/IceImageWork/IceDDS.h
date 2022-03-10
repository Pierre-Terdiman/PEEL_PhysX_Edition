///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to handle DDS files.
 *	\file		IceDDS.h
 *	\author		Pierre Terdiman
 *	\date		August, 2, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEDDS_H
#define ICEDDS_H

#ifndef _WIN64
	enum DXTCCode
	{
		DXTC_NONE			= 0,
		DXTC_DXT1			= 1,	//!< Compressed, 1 bit alpha
		DXTC_DXT2			= 2,	//!< Compressed, 4 bit premultiplied alpha
		DXTC_DXT3			= 3,	//!< Compressed, 4 bit nonpremultiplied alpha
		DXTC_DXT4			= 4,	//!< Compressed, 4 bit interpolated premultiplied alpha
		DXTC_DXT5			= 5,	//!< Compressed, 4 bit interpolated nonpremultiplied alpha
		DXTC_FORCE_DWORD	= 0x7fffffff
	};

	struct DXTCCREATE
	{
		Picture*		mSource;
		DXTCCode		mFormat;
	};

	class ICEIMAGEWORK_API DXTCPicture : public Allocateable
	{
		public:
									DXTCPicture();
									~DXTCPicture();
		// Creation
						bool		Create(const DXTCCREATE& create);
		// Destruction
						void		Release();
		// Data access
		inline_			udword		GetWidth()		const	{ return mWidth;	}
		inline_			udword		GetHeight()		const	{ return mHeight;	}
		inline_			DXTCCode	GetFormat()		const	{ return mFormat;	}
		inline_			ubyte*		GetData()		const	{ return mData;		}
		inline_			udword		GetDataSize()	const	{ return mDataSize;	}

		private:
						udword		mWidth;
						udword		mHeight;
						DXTCCode	mFormat;
						ubyte*		mData;
						udword		mDataSize;
	};

	enum DDSCode
	{
		DDS_NONE,
		DDS_DXT1,
		DDS_DXT2,
		DDS_DXT3,
		DDS_DXT4,
		DDS_DXT5,
		DDS_A8R8G8B8,
		DDS_A1R5G5B5,
		DDS_A4R4G4B4,
		DDS_R8G8B8,
		DDS_R5G6B5,
	};

	struct DDSCREATE
	{
		Picture*		mSource;
		const char*		mFilename;
		DDSCode			mFormat;
	};

	FUNCTION ICEIMAGEWORK_API bool SaveDDS(const DDSCREATE& create);
#endif

#endif // ICEDDS_H
