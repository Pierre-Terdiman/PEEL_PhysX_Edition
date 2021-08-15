///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains fonts-related code.
 *	\file		IceFnts.h
 *	\author		Pierre Terdiman
 *	\date		December, 19, 2004
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFNTS_H
#define ICEFNTS_H

	struct ICEIMAGEWORK_API FntInfo : public Allocateable
	{
		float		u0;
		float		v0;
		float		u1;
		float		v1;
		udword		dx;
		udword		dy;
	};

	class ICEIMAGEWORK_API FntDataBase : public Allocateable
	{
		public:
								FntDataBase();
								~FntDataBase();

				bool			Init(udword nb_fnts, udword max_dx, udword max_dy, const FntInfo* fnts, const ubyte* xref);
				void			Reset();

		inline_	udword			GetNbFnts()	const	{ return mNbFnts;	}
		inline_	const FntInfo*	GetFnts()	const	{ return mFnts;		}
		inline_	udword			GetMaxDx()	const	{ return mMaxDx;	}
		inline_	udword			GetMaxDy()	const	{ return mMaxDy;	}
		inline_	const ubyte*	GetXRef()	const	{ return mXRef;		}

				udword			ComputeSize(const char* text, float& width, float& height, float scale)	const;

				bool			Save(WriteStream& stream) const;
				bool			Load(const ReadStream& stream);
		protected:
				udword			mNbFnts;
				FntInfo*		mFnts;
				udword			mMaxDx, mMaxDy;
				ubyte			mXRef[256];
	};

	class ICEIMAGEWORK_API FntData : public FntDataBase
	{
		public:
								FntData()	{}
								~FntData()	{}

				bool			Init(Picture& src, udword tex_size, udword space_size, const char* alphabet=null);
	};

	class ICEIMAGEWORK_API FntData2 : public FntDataBase
	{
		public:
								FntData2()	{}
								~FntData2()	{}

				bool			Init(Picture& src, udword tex_size, udword space_size, const char* alphabet=null);
	};

	class ICEIMAGEWORK_API Fonts : public Allocateable
	{
		public:
								Fonts()		{}
								~Fonts()	{}

				bool			Init(const char* alphabet, udword text_size, udword texture_size, udword space_size, const char* fonts_name=null);
				bool			Save(WriteStream& stream, bool save_picture=true) const;
				bool			Load(const ReadStream& stream, bool load_picture=true);

		inline_	const Picture&	GetPicture()	const	{ return mSrc;	}
		inline_	const FntData&	GetFntData()	const	{ return mData;	}

		private:
				Picture			mSrc;
				FntData			mData;
	};

	// Obsolete! Kept for compatibility
	FUNCTION ICEIMAGEWORK_API	bool CreateFntData(Picture& src, Container& fnt_uvs, udword tex_size, udword space_size, udword& fnt_width, udword& fnt_height, ubyte* xref=null, const char* alphabet=null);
	FUNCTION ICEIMAGEWORK_API	bool RenderText(const String& msg, Picture& pic, sdword size, const RGBPixel& bk_color, const RGBPixel& text_color, udword extra_space, const char* fonts_name=null, HFONT* fonts=null);




	class ICEIMAGEWORK_API Fonts2 : public Allocateable
	{
		public:
								Fonts2()	{}
								~Fonts2()	{}

				bool			Init(const char* alphabet, udword text_size, udword texture_size, udword space_size, const char* fonts_name=null);
				bool			Save(WriteStream& stream, bool save_picture=true) const;
				bool			Load(const ReadStream& stream, bool load_picture=true);

		inline_	const Picture&	GetPicture()	const	{ return mSrc;	}
		inline_	const FntData2&	GetFntData()	const	{ return mData;	}

		private:
				Picture			mSrc;
				FntData2		mData;
	};


#endif	// ICEFNTS_H
