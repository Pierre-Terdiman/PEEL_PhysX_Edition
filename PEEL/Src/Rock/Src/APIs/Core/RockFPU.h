#ifndef ROCK_FPU_H
#define ROCK_FPU_H

#include <Core/RockCommon.h>
#include <xmmintrin.h>

namespace Rock
{
/*	template<u32 valueT, u32 maskT>
	class FPUGuard
	{
		ROCK_NOCOPY(FPUGuard)

		public:
		FPUGuard()
		{
			const u32 PreviousState = _mm_getcsr();
			mSavedState = PreviousState;
			_mm_setcsr((PreviousState & ~maskT) | valueT);
		}

		~FPUGuard()
		{
			_mm_setcsr((_mm_getcsr() & ~maskT) | (mSavedState & maskT));
		}

		private:
		u32		mSavedState;	
	};*/


	class SIMDGuard
	{
		public:
		inline_	SIMDGuard(bool enable = true) : mEnabled(enable)
		{
			if(enable)
			{
				mControlWord = _mm_getcsr();
				_mm_setcsr(_MM_MASK_MASK | _MM_FLUSH_ZERO_ON | (1 << 6));
			}
			else
			{
				ROCK_ASSERT(_mm_getcsr() & _MM_FLUSH_ZERO_ON);
				ROCK_ASSERT(_mm_getcsr() & (1 << 6));
				ROCK_ASSERT(_mm_getcsr() & _MM_MASK_MASK);
			}
		}

		inline_	~SIMDGuard()
		{
			if(mEnabled)
				_mm_setcsr(mControlWord & ~_MM_EXCEPT_MASK);
		}

		private:
		u32		mControlWord;
		bool	mEnabled;
	};

	#define ROCK_SIMD_GUARD			SIMDGuard ScopedFpGuard;
	#define ROCK_SIMD_GUARD_CNDT(x)	SIMDGuard ScopedFpGuard(x);
}

#endif
