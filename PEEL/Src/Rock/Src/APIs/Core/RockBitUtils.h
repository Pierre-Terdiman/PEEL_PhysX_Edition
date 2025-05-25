#ifndef ROCK_BIT_UTILS_H
#define ROCK_BIT_UTILS_H

#include <Core/RockCommon.h>
#include <Core/RockPlatform.h>

namespace Rock
{
	inline_	u32 BitCount(u32 v)
	{
		// from http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
		u32 const w = v - ((v >> 1) & 0x55555555);
		u32 const x = (w & 0x33333333) + ((w >> 2) & 0x33333333);
		return (((x + (x >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
	}

	inline_ bool IsPowerOfTwo(u32 x)
	{
		return x != 0 && (x & (x - 1)) == 0;
	}

	// "Next Largest Power of 2
	// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
	// that recursively "folds" the upper bits into the lower bits. This process yields a bit vector with
	// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
	// largest power of 2. For a 32-bit value:"
	inline_ u32 NextPowerOfTwo(u32 x)
	{
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		return x + 1;
	}

	inline_ u32 LowestSetBit(u32 x)
	{
		ROCK_ASSERT(x);
		return LowestSetBitUnsafe(x);
	}

	inline_ u32 HighestSetBit(u32 x)
	{
		ROCK_ASSERT(x);
		return HighestSetBitUnsafe(x);
	}

}

#endif

