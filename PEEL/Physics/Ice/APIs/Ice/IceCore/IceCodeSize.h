///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains various "code size" functions.
 *	\file		IceCodeSize.h
 *	\author		Pierre Terdiman + Steve Baker
 *	\date		September, 1, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECODESIZE_H
#define ICECODESIZE_H

// Because some versions of math.h make log2 a macro.
/*#ifdef log2
#undef log2
#endif*/

	inline_ float	ice_log2(float f)
	{
	  return logf(f) * INVLN2;
	}

	// The original loop
	inline_ udword CodeSize1(udword value)
	{
		udword NbBits = 0;
		for(udword i=0;i<32;i++)
		{
			udword Limit = 1<<i;
			if(value>Limit)	NbBits++;
			else break;
		}
		return NbBits;
	}

	// The FPU trick
	inline_ udword CodeSize2(udword value)
	{
		int n;
		*((float*)&n) = (float)value;
		n = (n>>23)-126;
		return n;
	}

	// "which will beat yours for speed I think"
	inline_ udword CodeSize3(udword value)
	{
		int i = 0;
		while (value) { i++ ; value >>= 1 ; }
		return i ;
	}

	// "If your numbers *do* tend to be on the large side"
	inline_ udword CodeSize4(udword value)
	{
		int i = 31;
		while ( value < 0x80000000 ) { i-- ; value <<= 1 ; }
		return i+1;	// Off-by-one indeed
	}

	// "That would be slower than the loop for small numbers - but faster for large ones"
	inline_ udword CodeSize5(udword value)
	{
		return	(value>=0x80000000) + (value>=0x40000000) + (value>=0x20000000) + (value>= 0x10000000) +
				(value>=0x8000000) + (value>=0x4000000) + (value>=0x2000000) + (value>= 0x1000000) +
				(value>=0x800000) + (value>=0x400000) + (value>=0x200000) + (value>= 0x100000) +
				(value>=0x80000) + (value>=0x40000) + (value>=0x20000) + (value>= 0x10000) +
				(value>=0x8000) + (value>=0x4000) + (value>=0x2000) + (value>= 0x1000) +
				(value>=0x800) + (value>=0x400) + (value>=0x200) + (value>= 0x100) +
				(value>=0x80) + (value>=0x40) + (value>=0x20) + (value>= 0x10) +
				(value>=0x8) + (value>=0x4) + (value>=0x2) + (value>= 0x1) ;
	}

	// "Fastest of all (I'm pretty sure)"
	inline_ udword CodeSize6(udword value)
	{
		static int nbits = 0;
		static unsigned int mask = 1;

		if(value<mask)	return nbits;

		mask <<= 1 ;
		return ++nbits;
	}

	// "I'd stake money on that being faster than the 'trick' version below"
	inline_ udword CodeSize7(udword value)
	{
		static int nbits = 0;
		static unsigned int mask = 1;

		if(value<mask) return nbits;

		do
		{
			mask <<= 1;
			nbits++;
		}while(value>=mask);

		return nbits ;
	}

#endif // ICECODESIZE_H
