#ifndef ROCK_HASH_MAP_H
#define ROCK_HASH_MAP_H

#include <Core/RockCommon.h>
#include <Core/RockAllocator.h>
#include <Core/RockMemory.h>
#include <Core/RockBitUtils.h>
#include <Core/RockCoreTemplates.h>
#include <Core/RockHash.h>

namespace Rock
{
	template <class Entry, class Key, class HashFn, class GetKey, bool compacting>
	class HashBase : public UserAllocated
	{
		void init(u32 initialTableSize, f32 loadFactor)
		{
			mBuffer = null;
			mEntries = null;
			mEntriesNext = null;
			mHash = null;
			mEntriesCapacity = 0;
			mHashSize = 0;
			mLoadFactor = loadFactor;
			mFreeList = u32(EOL);
			mTimestamp = 0;
			mEntriesCount = 0;

			if(initialTableSize)
				reserveInternal(initialTableSize);
		}

	  public:
		typedef Entry EntryType;

		HashBase(u32 initialTableSize = 64, f32 loadFactor = 0.75f)
		{
			init(initialTableSize, loadFactor);
		}

		~HashBase()
		{
			destroy(); // No need to clear()

			ROCK_FREE(mBuffer);
		}

		static const u32 EOL = 0xffffffff;

		inline Entry* create(const Key& k, bool& exists)
		{
			u32 h = 0;
			if(mHashSize)
			{
				h = hash(k);
				u32 index = mHash[h];
				while(index != EOL && !HashFn().equal(GetKey()(mEntries[index]), k))
					index = mEntriesNext[index];
				exists = index != EOL;
				if(exists)
					return mEntries + index;
			}
			else
				exists = false;

			if(freeListEmpty())
			{
				grow();
				h = hash(k);
			}

			const u32 entryIndex = freeListGetNext();

			mEntriesNext[entryIndex] = mHash[h];
			mHash[h] = entryIndex;

			mEntriesCount++;
			mTimestamp++;

			return mEntries + entryIndex;
		}

		inline const Entry* find(const Key& k) const
		{
			if(!mEntriesCount)
				return null;

			const u32 h = hash(k);
			u32 index = mHash[h];
			while(index != EOL && !HashFn().equal(GetKey()(mEntries[index]), k))
				index = mEntriesNext[index];
			return index != EOL ? mEntries + index : null;
		}

		inline bool erase(const Key& k, Entry& e)
		{
			if(!mEntriesCount)
				return false;

			const u32 h = hash(k);
			u32* ptr = mHash + h;
			while(*ptr != EOL && !HashFn().equal(GetKey()(mEntries[*ptr]), k))
				ptr = mEntriesNext + *ptr;

			if(*ptr == EOL)
				return false;

			ROCK_PLACEMENT_NEW(&e, Entry)(mEntries[*ptr]);		

			return eraseInternal(ptr);
		}

		inline bool erase(const Key& k)
		{
			if(!mEntriesCount)
				return false;

			const u32 h = hash(k);
			u32* ptr = mHash + h;
			while(*ptr != EOL && !HashFn().equal(GetKey()(mEntries[*ptr]), k))
				ptr = mEntriesNext + *ptr;

			if(*ptr == EOL)
				return false;		

			return eraseInternal(ptr);
		}

		inline_ u32 size() const
		{
			return mEntriesCount;
		}

		inline_ u32 capacity() const
		{
			return mHashSize;
		}

		void clear()
		{
			if(!mHashSize || mEntriesCount == 0)
				return;

			destroy();

			StoreDwords_(mHash, mHashSize, EOL);

			const u32 sizeMinus1 = mEntriesCapacity - 1;
			for(u32 i = 0; i < sizeMinus1; i++)
				mEntriesNext[i] = i + 1;

			mEntriesNext[mEntriesCapacity - 1] = u32(EOL);
			mFreeList = 0;
			mEntriesCount = 0;
		}

		void reserve(u32 size)
		{
			if(size > mHashSize)
				reserveInternal(size);
		}

		inline_ const Entry* getEntries() const
		{
			return mEntries;
		}

		inline Entry* insertUnique(const Key& k)
		{
			ROCK_ASSERT(find(k) == null);
			u32 h = hash(k);

			u32 entryIndex = freeListGetNext();

			mEntriesNext[entryIndex] = mHash[h];
			mHash[h] = entryIndex;

			mEntriesCount++;
			mTimestamp++;

			return mEntries + entryIndex;
		}

	  private:
		void destroy()
		{
			for(u32 i = 0; i < mHashSize; i++)
			{
				for(u32 j = mHash[i]; j != EOL; j = mEntriesNext[j])
					mEntries[j].~Entry();
			}
		}

		template <typename HK, typename GK, bool comp>
		noinline_ void copy(const HashBase<Entry, Key, HK, GK, comp>& other);

		// free list management - if we're coalescing, then we use mFreeList to hold
		// the top of the free list and it should always be equal to size(). Otherwise,
		// we build a free list in the next() pointers.

		inline void freeListAdd(u32 index)
		{
			if(compacting)
			{
				mFreeList--;
				ROCK_ASSERT(mFreeList == mEntriesCount);
			}
			else
			{
				mEntriesNext[index] = mFreeList;
				mFreeList = index;
			}
		}

		inline void freeListAdd(u32 start, u32 end)
		{
			if(!compacting)
			{
				for(u32 i = start; i < end - 1; i++) // add the new entries to the free list
					mEntriesNext[i] = i + 1;

				// link in old free list
				mEntriesNext[end - 1] = mFreeList;
				ROCK_ASSERT(mFreeList != end - 1);
				mFreeList = start;
			}
			else if(mFreeList == EOL) // don't reset the free ptr for the compacting hash unless it's empty
				mFreeList = start;
		}

		inline u32 freeListGetNext()
		{
			ROCK_ASSERT(!freeListEmpty());
			if(compacting)
			{
				ROCK_ASSERT(mFreeList == mEntriesCount);
				return mFreeList++;
			}
			else
			{
				u32 entryIndex = mFreeList;
				mFreeList = mEntriesNext[mFreeList];
				return entryIndex;
			}
		}

		inline bool freeListEmpty() const
		{
			if(compacting)
				return mEntriesCount == mEntriesCapacity;
			else
				return mFreeList == EOL;
		}

		inline void replaceWithLast(u32 index)
		{
			ROCK_PLACEMENT_NEW(mEntries + index, Entry)(mEntries[mEntriesCount]);
			mEntries[mEntriesCount].~Entry();
			mEntriesNext[index] = mEntriesNext[mEntriesCount];

			u32 h = hash(GetKey()(mEntries[index]));
			u32* ptr;
			for(ptr = mHash + h; *ptr != mEntriesCount; ptr = mEntriesNext + *ptr)
				ROCK_ASSERT(*ptr != EOL);
			*ptr = index;
		}

		inline u32 hash(const Key& k, u32 hashSize) const
		{
			return HashFn()(k) & (hashSize - 1);
		}

		inline u32 hash(const Key& k) const
		{
			return hash(k, mHashSize);
		}

		inline bool eraseInternal(u32* ptr)
		{
			const u32 index = *ptr;

			*ptr = mEntriesNext[index];

			mEntries[index].~Entry();

			mEntriesCount--;
			mTimestamp++;

			if(compacting && index != mEntriesCount)
				replaceWithLast(index);

			freeListAdd(index);
			return true;
		}

		noinline_ void reserveInternal(u32 size)
		{
			if(!IsPowerOfTwo(size))
				size = NextPowerOfTwo(size);

			ROCK_ASSERT(!(size & (size - 1)));

			// decide whether iteration can be done on the entries directly
			bool resizeCompact = compacting || freeListEmpty();

			// define new table sizes
			u32 oldEntriesCapacity = mEntriesCapacity;
			u32 newEntriesCapacity = u32(f32(size) * mLoadFactor);
			u32 newHashSize = size;

			// allocate new common buffer and setup pointers to new tables
			u8* newBuffer;
			u32* newHash;
			u32* newEntriesNext;
			Entry* newEntries;
			{
				u32 newHashByteOffset = 0;
				u32 newEntriesNextBytesOffset = newHashByteOffset + newHashSize * sizeof(u32);
				u32 newEntriesByteOffset = newEntriesNextBytesOffset + newEntriesCapacity * sizeof(u32);
				newEntriesByteOffset += (16 - (newEntriesByteOffset & 15)) & 15;
				u32 newBufferByteSize = newEntriesByteOffset + newEntriesCapacity * sizeof(Entry);

				newBuffer = ROCK_ALLOCATE(u8, newBufferByteSize, "hashMap");
				ROCK_ASSERT(newBuffer);

				newHash = reinterpret_cast<u32*>(newBuffer + newHashByteOffset);
				newEntriesNext = reinterpret_cast<u32*>(newBuffer + newEntriesNextBytesOffset);
				newEntries = reinterpret_cast<Entry*>(newBuffer + newEntriesByteOffset);
			}

			// initialize new hash table
			StoreDwords_(newHash, newHashSize, EOL);

			// iterate over old entries, re-hash and create new entries
			if(resizeCompact)
			{
				// check that old free list is empty - we don't need to copy the next entries
				ROCK_ASSERT(compacting || mFreeList == EOL);

				for(u32 index = 0; index < mEntriesCount; ++index)
				{
					u32 h = hash(GetKey()(mEntries[index]), newHashSize);
					newEntriesNext[index] = newHash[h];
					newHash[h] = index;

					ROCK_PLACEMENT_NEW(newEntries + index, Entry)(mEntries[index]);

					mEntries[index].~Entry();
				}
			}
			else
			{
				// copy old free list, only required for non compact resizing
				CopyMemory_(newEntriesNext, mEntriesNext, mEntriesCapacity * sizeof(u32));

				for(u32 bucket = 0; bucket < mHashSize; bucket++)
				{
					u32 index = mHash[bucket];
					while(index != EOL)
					{
						u32 h = hash(GetKey()(mEntries[index]), newHashSize);
						newEntriesNext[index] = newHash[h];
						ROCK_ASSERT(index != newHash[h]);

						newHash[h] = index;

						ROCK_PLACEMENT_NEW(newEntries + index, Entry)(mEntries[index]);
						mEntries[index].~Entry();

						index = mEntriesNext[index];
					}
				}
			}

			// swap buffer and pointers
			ROCK_FREE(mBuffer);
			mBuffer = newBuffer;
			mHash = newHash;
			mHashSize = newHashSize;
			mEntriesNext = newEntriesNext;
			mEntries = newEntries;
			mEntriesCapacity = newEntriesCapacity;

			freeListAdd(oldEntriesCapacity, newEntriesCapacity);
		}

		void grow()
		{
			ROCK_ASSERT((mFreeList == EOL) || (compacting && (mEntriesCount == mEntriesCapacity)));

			u32 size = mHashSize == 0 ? 16 : mHashSize * 2;
			reserve(size);
		}

		u8* mBuffer;
		Entry* mEntries;
		u32* mEntriesNext; // same size as mEntries
		u32* mHash;
		u32 mEntriesCapacity;
		u32 mHashSize;
		f32 mLoadFactor;
		u32 mFreeList;
		u32 mTimestamp;
		u32 mEntriesCount; // number of entries
	};

	template <class Entry, class Key, class HashFn, class GetKey, bool compacting>
	template <typename HK, typename GK, bool comp>
	noinline_ void HashBase<Entry, Key, HashFn, GetKey, compacting>::copy(const HashBase<Entry, Key, HK, GK, comp>& other)
	{
		reserve(other.mEntriesCount);

		for(u32 i = 0; i < other.mEntriesCount; i++)
		{
			for(u32 j = other.mHash[i]; j != EOL; j = other.mEntriesNext[j])
			{
				const Entry& otherEntry = other.mEntries[j];

				bool exists;
				Entry* newEntry = create(GK()(otherEntry), exists);
				ROCK_ASSERT(!exists);

				ROCK_PLACEMENT_NEW(newEntry, Entry)(otherEntry);
			}
		}
	}

	template <class Key, class HashFn, bool Coalesced = false>
	class HashSetBase
	{
		ROCK_NOCOPY(HashSetBase)
	  public:
		struct GetKey
		{
			inline const Key& operator()(const Key& e)
			{
				return e;
			}
		};

		typedef HashBase<Key, Key, HashFn, GetKey, Coalesced> BaseMap;

		HashSetBase(u32 initialTableSize = 64, f32 loadFactor = 0.75f) : mBase(initialTableSize, loadFactor)
		{
		}

		bool insert(const Key& k)
		{
			bool exists;
			Key* e = mBase.create(k, exists);
			if(!exists)
				ROCK_PLACEMENT_NEW(e, Key)(k);
			return !exists;
		}

		inline bool contains(const Key& k) const
		{
			return mBase.find(k) != 0;
		}

		inline bool erase(const Key& k)
		{
			return mBase.erase(k);
		}

		inline u32 size() const
		{
			return mBase.size();
		}

		inline u32 capacity() const
		{
			return mBase.capacity();
		}

		inline void reserve(u32 size)
		{
			mBase.reserve(size);
		}

		inline void clear()
		{
			mBase.clear();
		}

	  protected:
		BaseMap mBase;
	};

	template <class Key, class Value, class HashFn>
	class HashMapBase
	{
		ROCK_NOCOPY(HashMapBase)
	  public:
		typedef Pair<const Key, Value> Entry;

		struct GetKey
		{
			inline const Key& operator()(const Entry& e)
			{
				return e.mFirst;
			}
		};

		typedef HashBase<Entry, Key, HashFn, GetKey, true> BaseMap;

		HashMapBase(u32 initialTableSize = 64, f32 loadFactor = 0.75f) : mBase(initialTableSize, loadFactor)
		{
		}

		bool insert(const Key /*&*/ k, const Value /*&*/ v)
		{
			bool exists;
			Entry* e = mBase.create(k, exists);
			if(!exists)
				ROCK_PLACEMENT_NEW(e, Entry)(k, v);
			return !exists;
		}

		Value& operator[](const Key& k)
		{
			bool exists;
			Entry* e = mBase.create(k, exists);
			if(!exists)
				ROCK_PLACEMENT_NEW(e, Entry)(k, Value());

			return e->mSecond;
		}

		inline const Entry* find(const Key& k) const
		{
			return mBase.find(k);
		}

		inline bool erase(const Key& k)
		{
			return mBase.erase(k);
		}

		inline bool erase(const Key& k, Entry& e)
		{		
			return mBase.erase(k, e);
		}

		inline u32 size() const
		{
			return mBase.size();
		}

		inline u32 capacity() const
		{
			return mBase.capacity();
		}

		inline void reserve(u32 size)
		{
			mBase.reserve(size);
		}

		inline void clear()
		{
			mBase.clear();
		}

	  protected:
		BaseMap mBase;
	};

	template <class Key, class HashFn = Hash<Key>>
	class HashSet : public HashSetBase<Key, HashFn, false>
	{
	  public:
		typedef HashSetBase<Key, HashFn, false> HashSetBase;

		HashSet(u32 initialTableSize = 64, f32 loadFactor = 0.75f) : HashSetBase(initialTableSize, loadFactor)
		{
		}
	};

	template <class Key, class HashFn = Hash<Key>>
	class CoalescedHashSet : public HashSetBase<Key, HashFn, true>
	{
	  public:
		typedef typename HashSetBase<Key, HashFn, true> HashSetBase;

		CoalescedHashSet(u32 initialTableSize = 64, f32 loadFactor = 0.75f) : HashSetBase(initialTableSize, loadFactor)
		{
		}

		inline_	const Key* getEntries() const
		{
			return HashSetBase::mBase.getEntries();
		}
	};

	template <class Key, class Value, class HashFn = Hash<Key>>
	class HashMap : public HashMapBase<Key, Value, HashFn>
	{
	  public:
		typedef HashMapBase<Key, Value, HashFn> HashMapBase;

		HashMap(u32 initialTableSize = 64, f32 loadFactor = 0.75f) : HashMapBase(initialTableSize, loadFactor)
		{
		}
	};

	template <class Key, class Value, class HashFn = Hash<Key>>
	class CoalescedHashMap : public HashMapBase<Key, Value, HashFn>
	{
	  public:
		typedef HashMapBase<Key, Value, HashFn> HashMapBase;

		CoalescedHashMap(u32 initialTableSize = 64, f32 loadFactor = 0.75f) : HashMapBase(initialTableSize, loadFactor)
		{
		}

		inline_ const Pair<const Key, Value>* getEntries() const
		{
			return HashMapBase::mBase.getEntries();
		}
	};
}

#endif
