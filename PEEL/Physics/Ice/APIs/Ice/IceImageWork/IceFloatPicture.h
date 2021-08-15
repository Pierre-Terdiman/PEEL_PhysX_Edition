///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a float picture.
 *	\file		IceFloatPicture.h
 *	\author		Pierre Terdiman
 *	\date		November, 14, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFLOATPICTURE_H
#define ICEFLOATPICTURE_H

	//! A generic float-pixel class
	class ICEIMAGEWORK_API FloatPixel : public Allocateable
	{
		public:
		inline_						FloatPixel()													{}
		inline_						FloatPixel(float _r, float _g, float _b) : r(_r), g(_g), b(_b)	{}
		inline_						~FloatPixel()													{}

				float				r;		//!< Red
				float				g;		//!< Green
				float				b;		//!< Blue
	};

	class ICEIMAGEWORK_API FloatPicture : public Allocateable
	{
		public:
									FloatPicture();
									FloatPicture(sdword width, sdword height);
									FloatPicture(const Picture* source);
									FloatPicture(const Picture& source);
									~FloatPicture();

		// Data access
		inline_	sdword				GetWidth()						const	{ return mWidth;			}	//!< Returns the picture's width
		inline_	sdword				GetHeight()						const	{ return mHeight;			}	//!< Returns the picture's height
		inline_	udword				GetNbPixels()					const	{ return mWidth*mHeight;	}	//!< Returns the number of pixels
		inline_	FloatPixel*			GetPixels()						const	{ return mData;				}	//!< Returns the picture's pixels
		inline_	const FloatPixel&	GetPixel(sdword x, sdword y)	const	{ return mData[x+y*mWidth];	}

			// element access
			// x,y,z are indices for row, column, and color, respectively
/*			inline	float&	operator()	(const sdword x, const sdword y, const sdword z)
			{
				if(z==0)	return mData[x+y*mWidth].R;
				if(z==1)	return mData[x+y*mWidth].G;
				if(z==2)	return mData[x+y*mWidth].B;
				return 0.0f;
			}
*/
		// Image ops
				FloatPicture&		Clear(float r=0.0f, float g=0.0f, float b=0.0f);						// Clear a picture
				bool				Spread(const FloatPixel& default_color);

		//! Operator for "FloatPicture A = Picture B"
		inline_	void				operator = (const Picture& source);

		private:
				sdword				mWidth;
				sdword				mHeight;
				FloatPixel*			mData;
	};

#endif // ICEFLOATPICTURE_H
