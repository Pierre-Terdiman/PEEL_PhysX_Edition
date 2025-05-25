#ifndef ROCK_BITMAP_H
#define ROCK_BITMAP_H

#include <Core/RockCommon.h>
#include <Core/RockAllocator.h>
#include <Core/RockMemory.h>
#include <Core/RockBitUtils.h>

namespace Rock
{
	class Bitmap : public UserAllocated
	{
		ROCK_NOCOPY(Bitmap)

		public:

		inline_				Bitmap() : mMap(null), mWordCount(0) {}

		inline_				~Bitmap()
							{
								Release();
							}

		inline_	void		Release()
							{
								ROCK_FREE(mMap);
							}

		inline_	void		GrowAndSet(u32 index)
							{
								Extend(index + 1);
								mMap[index >> 5] |= 1 << (index & 31);
							}

		inline_	void		GrowAndReset(u32 index)
							{
								Extend(index + 1);
								mMap[index >> 5] &= ~(1 << (index & 31));
							}

		inline_	b32			BoundedTest(u32 index) const
							{
								return b32(index >> 5 >= GetWordCount() ? 0 : (mMap[index >> 5] & (1 << (index & 31))));
							}

		inline_	void		BoundedReset(u32 index)
							{
								if((index >> 5) < GetWordCount())
									mMap[index >> 5] &= ~(1 << (index & 31));
							}

		// Special optimized versions, when you _know_ your index is in range
		inline_	void		Set(u32 index)
							{
								ROCK_ASSERT(index<GetWordCount() * 32);
								mMap[index >> 5] |= 1 << (index & 31);
							}

		inline_	void		Reset(u32 index)
							{
								ROCK_ASSERT(index<GetWordCount() * 32);
								mMap[index >> 5] &= ~(1 << (index & 31));
							}

		inline_	b32			Test(u32 index) const
							{
								ROCK_ASSERT(index<GetWordCount() * 32);
								return b32(mMap[index >> 5] & (1 << (index & 31)));
							}

		inline_	void		Clear()
							{
								ZeroMemory(mMap, GetWordCount() * sizeof(u32));
							}

				void		ResizeAndClear(u32 newBitCount)
							{
								ExtendUninitialized(newBitCount);
								ZeroMemory(mMap, GetWordCount() * sizeof(u32));
							}

				void		SetEmpty()
							{
								mMap = null;
								mWordCount = 0;
							}

		inline_	u32			Size()			const	{ return GetWordCount() * 32; }

		inline_	const u32*	GetWords()		const	{ return mMap;		}
		inline_	u32*		GetWords()				{ return mMap;		}
		inline_	u32			GetWordCount()	const	{ return mWordCount; }

		//! returns 0 if no bits set (!!!)
				u32			FindLast() const
							{
								const u32 wordCount = GetWordCount();
								for(u32 i = wordCount; i-- > 0;)
								{
									if(mMap[i])
										return (i << 5) + HighestSetBit(mMap[i]);
								}
								return u32(0);
							}
		protected:
				u32*		mMap;			//one bit per index
				u32			mWordCount;

				void		Extend(u32 size);
				void		ExtendUninitialized(u32 size);
	};

}

#endif
