///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains z-curve related code.
 *	\file		IceZCurve.h
 *	\author		Pierre Terdiman
 *	\date		January, 12, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEZCURVE_H
#define ICEZCURVE_H

	inline_ udword SpaceBits_16bit_Crister_Jukka(udword n)
	{
		udword t;
		t = n & 0xff00ff00; n = (n ^ t) + (t << 8);
		t = n & 0xf0f0f0f0; n = (n ^ t) + (t << 4);
		t = n & 0x0c0c0c0c; n = (n ^ t) + (t << 2);
		t = n & 0x22222222; n = (n ^ t) + (t << 1);
		return n;
	}

	inline_ udword SpaceBits_16bit_CristerOptimized(udword n)
	{
		n = (n ^ (n << 8)) & 0x00ff00ff;
		n = (n ^ (n << 4)) & 0x0f0f0f0f;
		n = (n ^ (n << 2)) & 0x33333333;
		n = (n ^ (n << 1)) & 0x55555555;
		return n;
	}

	ICEMATHS_API udword ComputeZValue(udword x, udword y, udword z);
	ICEMATHS_API udword ComputeZValue(udword x, udword y);

	FUNCTION ICEMATHS_API udword GetMortonNumber_16bit_Stephans(udword x, udword y);
	FUNCTION ICEMATHS_API udword GetMortonNumber_16bit_Crister_Jukka(udword x, udword y);
	FUNCTION ICEMATHS_API udword GetMortonNumber_16bit_CristerOptimized(udword x, udword y);

	// Only valid for 8bit imput values
	FUNCTION ICEMATHS_API udword GetMortonNumber_16bit_CristerOptimizedReturn(udword x, udword y);
	FUNCTION ICEMATHS_API udword GetMortonNumber_16bit_Lasses(udword a, udword b);
	FUNCTION ICEMATHS_API udword GetMortonNumber_16bit_Lasses_Jukka(udword a, udword b);
	FUNCTION ICEMATHS_API udword GetMortonNumber_16bit_LassesOptimized(udword a, udword b);

	inline_ udword ComputeMortonIndex(udword i, udword j, udword k, udword level)
	{
		udword Index = 0;
		for(udword n=0;n<level;n++)
		{
			udword Mask = 1<<n;

			udword ib = (i&Mask)>>n;
			udword jb = (j&Mask)>>n;
			udword kb = (k&Mask)>>n;

			udword Bits = ((kb)<<(n*3+0)) | ((jb)<<(n*3+1)) | ((ib)<<(n*3+2));

			Index|=Bits;
		}
		return Index;
	}

#endif	// ICEZCURVE_H

