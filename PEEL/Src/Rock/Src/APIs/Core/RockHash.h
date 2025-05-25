#ifndef ROCK_HASH_H
#define ROCK_HASH_H

#include <Core/RockCommon.h>

namespace Rock
{
	// Thomas Wang's 32 bit mix
	inline_ u32 ComputeHash32(u32 key)
	{
		u32 k = key;
		k += ~(k << 15);
		k ^= (k >> 10);
		k += (k << 3);
		k ^= (k >> 6);
		k += ~(k << 11);
		k ^= (k >> 16);
		return u32(k);
	}

	inline_ u32 ComputeHash32(i32 key)
	{
		return ComputeHash32(u32(key));
	}

	// Thomas Wang's 64 bit mix
	inline_ u32 ComputeHash64(u64 key)
	{
		u64 k = key;
		k += ~(k << 32);
		k ^= (k >> 22);
		k += ~(k << 13);
		k ^= (k >> 8);
		k += (k << 3);
		k ^= (k >> 15);
		k += ~(k << 27);
		k ^= (k >> 31);
		return u32(k);
	}

	// Hash function for pointers
	inline_ u32 ComputeHashPtr(const void* ptr)
	{
#ifdef WIN64
		return ComputeHash64(u64(ptr));
#else
		return ComputeHash32(u32(size_t(ptr)));
#endif
	}

	// hash object for hash map template parameter
	template <class Key>
	struct Hash
	{
		inline_	u32 operator()(const Key& k) const
		{
			return ComputeHash(k);
		}

		inline_	bool equal(const Key& k0, const Key& k1) const
		{
			return k0 == k1;
		}
	};
}

#endif
