///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to handle crop values.
 *	\file		IceCropValues.h
 *	\author		Pierre Terdiman
 *	\date		August, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECROPVALUES_H
#define ICECROPVALUES_H

	struct ICEIMAGEWORK_API TexCoords : public Allocateable
	{
				float	u;
				float	v;
	};

	struct ICEIMAGEWORK_API CropValues : public Allocateable
	{
						CropValues() : mOffsetU(0.0f), mOffsetV(0.0f), mScaleU(1.0f), mScaleV(1.0f)	{}

		inline_	void	Reset()
						{
							mOffsetU	= 0.0f;
							mOffsetV	= 0.0f;
							mScaleU		= 1.0f;
							mScaleV		= 1.0f;
						}

		// tu= (u*ScaleU) + OffsetU;
		// tv= ((v-1.0f)*ScaleV) - OffsetV;
		inline_	void	Apply(float& u, float& v)
						{
											u*=mScaleU;	u+=mOffsetU;
								v-=1.0f;	v*=mScaleV;	v-=mOffsetV;	v+=1.0f;
						}

				float	mOffsetU;	//!< Offset for U
				float	mOffsetV;	//!< Offset for V
				float	mScaleU;	//!< Scale for U
				float	mScaleV;	//!< Scale for V
	};

#endif // ICECROPVALUES_H
