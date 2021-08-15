///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains color classes.
 *	\file		IceColor.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECOLOR_H
#define ICECOLOR_H

	// Forward declarations
	class RGBAColor;

	//! Turns a normalized vector into RGBA form. Used to encode vectors into a height map. Comes from NVIDIA.
	inline_ udword VectorToRGBA(const Point& v, udword a=0xff)
	{
		const udword r = (udword)(127.0f * v.x + 128.0f);
		const udword g = (udword)(127.0f * v.y + 128.0f);
		const udword b = (udword)(127.0f * v.z + 128.0f);

		return((a<<24L) + (r<<16L) + (g<<8L) + (b<<0L));
	}

	//! Helper: returns a given dword color mixed with an alpha mask
	inline_ udword ColorWithAlphaMask(udword color, udword alpha_mask)
	{
		const udword RGB = color & 0x00ffffff;	// Remove current alpha
		return alpha_mask | RGB;
	}

	//! Helper: returns a given dword color mixed with a [0;1) float alpha
	inline_ udword ColorWithAlpha(udword color, float alpha)
	{
		const udword AlphaMask = udword(alpha * 255.0f)<<24;
		return ColorWithAlphaMask(color, AlphaMask);
	}

	#define	PIXEL_OPAQUE		0xff
	#define	PIXEL_INVISIBLE		0

	//! A generic RGB pixel class
	class ICEIMAGEWORK_API RGBPixel : public Allocateable
	{
		public:
		//! Constructor
		inline_					RGBPixel()												{}
		//! Constructor
		inline_					RGBPixel(ubyte r, ubyte g, ubyte b) : R(r), G(g), B(b)	{}
		//! Destructor
		inline_					~RGBPixel()												{}

		//! Operator for "if(RGBPixel==RGBPixel)"
		inline_		bool		operator==(const RGBPixel& obj)	const
								{
									if(R!=obj.R || G!=obj.G || B!=obj.B)	return false;
									return true;
								}

		//! Operator for "if(RGBPixel!=RGBPixel)"
		inline_		bool		operator!=(const RGBPixel& obj)	const	{ return !(*this==obj);	}

		//! Computes luminance between 0.0f and 255.0f
		inline_		float		Luminance()						const	{ return 0.299f * float(R) + 0.587f * float(G) + 0.114f * float(B);	}

					ubyte		R;		//!< Red
					ubyte		G;		//!< Green
					ubyte		B;		//!< Blue
	};

	//! A generic RGBA pixel class
	class ICEIMAGEWORK_API RGBAPixel : public RGBPixel
	{
		public:
		//! Constructor
		inline_					RGBAPixel()													{}
		//! Constructor
		inline_					RGBAPixel(ubyte r, ubyte g, ubyte b, ubyte a=PIXEL_OPAQUE) : RGBPixel(r,g,b), A(a)	{}
		//! Constructor
		inline_					RGBAPixel(udword argb_value)								{ SetColor(argb_value);		}
		//! Constructor
								RGBAPixel(const RGBAColor& color);
		//! Destructor
		inline_					~RGBAPixel()												{}

		inline_		void		SetColor(const RGBAPixel& col)								{ B = col.B;	G = col.G;  R = col.R;	A = col.A;  }
		inline_		void		SetColor(ubyte r, ubyte g, ubyte b, ubyte a=PIXEL_OPAQUE)	{ R = r; G = g; B = b; A = a;	}

		inline_		void		SetColor(udword argb)
								{
									B = (ubyte)(argb & 0xFF);
									G = (ubyte)((argb & 0xFF00) >> 8);
									R = (ubyte)((argb & 0xFF0000) >> 16);
									A = (ubyte)((argb & 0xFF000000) >> 24);
								}

		inline_		void		SetColor24(udword rgb)
								{
									B = (ubyte)(rgb & 0xFF);
									G = (ubyte)((rgb & 0xFF00) >> 8);
									R = (ubyte)((rgb & 0xFF0000) >> 16);
									A = PIXEL_OPAQUE;
								}

		inline_		void		SetColor16(uword rgb)
								{
									B = (ubyte)(rgb & 0x1F);
									G = (ubyte)((rgb & 0x7E0) >> 5);
									R = (ubyte)((rgb & 0xF800) >> 11);
									A = PIXEL_OPAQUE;
								}

		//! Operator for "if(RGBAPixel==RGBAPixel)"
		inline_		bool		operator==(const RGBAPixel& obj)	const
								{
									if(R!=obj.R || G!=obj.G || B!=obj.B || A!=obj.A)	return false;
									return true;
								}

		//! Operator for "if(RGBAPixel!=RGBAPixel)"
		inline_		bool		operator!=(const RGBAPixel& obj)	const	{ return !(*this==obj);	}

					// a is 0..256
					// ================================================================================================
					// 'a' is base-256, not 255, s alpha is ignored
					// ================================================================================================
		inline_		void		Lerp(udword a, const RGBAPixel& p)
								{
									udword& d = (udword&)R;
									const udword& s = (const udword&)p.R;

									udword dstrb = d      & 0xFF00FF;
									udword dstag = d >> 8 & 0xFF00FF;

									udword srcrb = s      & 0xFF00FF;
									udword srcag = s >> 8 & 0xFF00FF;

									udword drb = srcrb - dstrb;
									udword dag = srcag - dstag;

									drb *= a;  
									dag *= a;  
									drb >>= 8;
									dag >>= 8;

									const udword rb  = (drb + dstrb)      & 0x00FF00FF;
									const udword ag  = (dag + dstag) << 8 & 0xFF00FF00;

									d = rb | ag;
								}

					ubyte		A;		//!< Alpha
	};

	class ICEIMAGEWORK_API RGBAColor : public Allocateable
	{
		public:
		//! Constructor
		inline_					RGBAColor()												{}
		//! Constructor
		inline_					RGBAColor(float r, float g, float b, float a=1.0f) : R(r), G(g), B(b), A(a)	{}
		//! Constructor
		inline_					RGBAColor(const RGBAPixel& pixel)						{ *this = pixel;			}
		//! Destructor
		inline_					~RGBAColor()											{}

		//! Operator for "RGBAColor = RGBAPixel"
		inline_		void		operator = (const RGBAPixel& pixel)
								{
									const float OneOver255 = 1.0f / 255.0f;
									R = float(pixel.R)*OneOver255;
									G = float(pixel.G)*OneOver255;
									B = float(pixel.B)*OneOver255;
									A = float(pixel.A)*OneOver255;
								}

		//! Operator for "if(RGBAColor==RGBAColor)"
		inline_		bool		operator==(const RGBAColor& obj)	const
								{
									if(SIR(R)!=SIR(obj.R))	return false;
									if(SIR(G)!=SIR(obj.G))	return false;
									if(SIR(B)!=SIR(obj.B))	return false;
									if(SIR(A)!=SIR(obj.A))	return false;
									return true;
								}

		//! Operator for "if(RGBAColor!=RGBAColor)"
		inline_		bool		operator!=(const RGBAColor& obj)	const	{ return !(*this==obj);	}

	// This operator first saturates the color, then converts it into udword
		inline_					operator	udword()				const
								{
									udword Color=0;

									if(SIR(A)>=IEEE_1_0)	Color = 0xFF<<24;
									else if(SIR(A)>0)		Color = ((udword)(A*255.0f)) << 24;

									if(SIR(R)>=IEEE_1_0)	Color |= 0xFF<<16;
									else if(SIR(R)>0)		Color |= ((udword)(R*255.0f)) << 16;

									if(SIR(G)>=IEEE_1_0)	Color |= 0xFF<<8;
									else if(SIR(G)>0)		Color |= ((udword)(G*255.0f)) << 8;

									if(SIR(B)>=IEEE_1_0)	Color |= 0xFF;
									else if(SIR(B)>0)		Color |= (udword)(B*255.0f);

									return	Color;
								}

		//! Computes luminance between 0.0f and 1.0f
		inline_		float		Luminance()							const	{ return 0.299f * R + 0.587f * G + 0.114f * B;	}

					float		R;		//!< Red
					float		G;		//!< Green
					float		B;		//!< Blue
					float		A;		//!< Alpha
	};

#endif // ICECOLOR_H
