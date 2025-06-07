#ifndef ROCK_ARRAY_H
#define ROCK_ARRAY_H

#include <Core/RockAllocator.h>

namespace Rock
{
	template <class T>
	class Array : protected UserAllocated
	{
		public:

		inline_	explicit Array() : mData(null), mSize(0), mCapacity(0)
		{
		}

		inline_ explicit Array(u32 size, const T& a = T()) : mData(null), mSize(0), mCapacity(0)
		{
			Resize(size, a);
		}

		template <class T>
		inline_ explicit Array(const Array<T>& other)
		{
			Copy(other);
		}

		inline_ Array(const Array& other)
		{
			Copy(other);
		}

		inline_ explicit Array(const T* first, const T* last) : mSize(last < first ? 0 : u32(last - first)), mCapacity(mSize)
		{
			mData = Allocate(mSize);
			Copy(mData, mData + mSize, first);
		}

		inline_ ~Array()
		{
			Destroy(mData, mData + mSize);

			if(Capacity())
				Deallocate(mData);
		}

		//template <class T>
		//inline_ Array& operator=(const Array<T>& rhs)
		inline_ Array& operator=(const Array& rhs)
		{
			if(&rhs == this)
				return *this;

			Clear();
			Reserve(rhs.mSize);
			Copy(mData, mData + rhs.mSize, rhs.mData);

			mSize = rhs.mSize;
			return *this;
		}

		/*inline_ Array& operator=(const Array& t)
		{
			return operator=(t);
		}*/

		inline_ const T& operator[](u32 i) const
		{
			ROCK_ASSERT(i < mSize);
			return mData[i];
		}

		inline_ T& operator[](u32 i)
		{
			ROCK_ASSERT(i < mSize);
			return mData[i];
		}

		inline_ const T* Begin() const
		{
			return mData;
		}

		inline_ T* Begin()
		{
			return mData;
		}

		inline_ const T* End() const
		{
			return mData + mSize;
		}

		inline_ T* End()
		{
			return mData + mSize;
		}

		inline_ const T& Front() const
		{
			ROCK_ASSERT(mSize);
			return mData[0];
		}

		inline_ T& Front()
		{
			ROCK_ASSERT(mSize);
			return mData[0];
		}

		inline_ const T& Back() const
		{
			ROCK_ASSERT(mSize);
			return mData[mSize - 1];
		}

		inline_ T& Back()
		{
			ROCK_ASSERT(mSize);
			return mData[mSize - 1];
		}

		inline_ u32 Size() const
		{
			return mSize;
		}

		inline_ void Clear()
		{
			Destroy(mData, mData + mSize);
			mSize = 0;
		}

		inline_ bool Empty() const
		{
			return mSize == 0;
		}

		inline_ T& PushBack(const T& a)
		{
			if(Capacity() <= mSize)
				return GrowAndPushBack(a);

			ROCK_PLACEMENT_NEW(reinterpret_cast<void*>(mData + mSize), T)(a);

			return mData[mSize++];
		}

		inline_ T PopBack()
		{
			ROCK_ASSERT(mSize);
			T t = mData[mSize - 1];

			mData[--mSize].~T();

			return t;
		}

		inline_ T& Insert()
		{
			if(Capacity() <= mSize)
				Grow(CapacityIncrement());

			T* ptr = mData + mSize++;
			ROCK_PLACEMENT_NEW(ptr, T); // not 'T()' because PODs should not get default-initialized.
			return *ptr;
		}

		inline_ void ReplaceWithLast(u32 i)
		{
			ROCK_ASSERT(i < mSize);
			mData[i] = mData[--mSize];

			mData[mSize].~T();
		}

		inline_ bool FindAndReplaceWithLast(const T& a)
		{
			u32 index = 0;
			while(index < mSize && mData[index] != a)
				++index;
			if(index == mSize)
				return false;
			ReplaceWithLast(index);
			return true;
		}

		inline_ void Remove(u32 i)
		{
			ROCK_ASSERT(i < mSize);

			T* it = mData + i;
			it->~T();
			while (++i < mSize)
			{								
				ROCK_PLACEMENT_NEW(it, T(mData[i]));
				++it;
				it->~T();
			} 
			--mSize;
		}

		noinline_ void Resize(u32 size, const T& a = T());

		noinline_ void ResizeUninitialized(u32 size);

		inline_ void Shrink()
		{
			Recreate(mSize);
		}

		inline_ void Reset()
		{
			Resize(0);
			Shrink();
		}

		inline_ void ResetOrClear()
		{
			const u32 c = Capacity();
			const u32 s = Size();
			if(s>=c/2)
				Clear();
			else
				Reset();
		}

		inline_ void Reserve(u32 capacity)
		{
			if(capacity > this->Capacity())
				Grow(capacity);
		}

		inline_ u32 Capacity() const
		{
			return mCapacity;
		}

		inline_ void ForceSize_Unsafe(u32 size)
		{
			ROCK_ASSERT(size <= mCapacity);
			mSize = size;
		}

		private:

		noinline_ void Copy(const Array<T>& other);

		inline_ T* Allocate(u32 size)
		{
			if(size > 0)
			{
				T* p = ROCK_ALLOCATE(T, size, "Array");
				return p;
			}
			return 0;
		}

		inline_ void Deallocate(void* mem)
		{
			ROCK_FREE(mem);
		}

		static inline_ void Create(T* first, T* last, const T& a)
		{
			for(; first < last; ++first)
				::ROCK_PLACEMENT_NEW(first, T(a));
		}

		static inline_ void Copy(T* first, T* last, const T* src)
		{
			if(last <= first)
				return;

			for(; first < last; ++first, ++src)
				::ROCK_PLACEMENT_NEW(first, T(*src));
		}

		static inline_ void Destroy(T* first, T* last)
		{
			for(; first < last; ++first)
				first->~T();
		}

		noinline_ T& GrowAndPushBack(const T& a);

		inline_ void Grow(u32 capacity)
		{
			ROCK_ASSERT(this->Capacity() < capacity);
			Recreate(capacity);
		}

		noinline_ void Recreate(u32 capacity);

		inline_ u32 CapacityIncrement() const
		{
			const u32 capacity = this->Capacity();
			return capacity == 0 ? 1 : capacity * 2;
		}

		T*	mData;
		u32	mSize;
		u32	mCapacity;
	};

	template <class T>
	noinline_ void Array<T>::Resize(u32 size, const T& a)
	{
		Reserve(size);
		Create(mData + mSize, mData + size, a);
		Destroy(mData + size, mData + mSize);
		mSize = size;
	}

	template <class T>
	noinline_ void Array<T>::Copy(const Array<T>& other)
	{
		if(!other.Empty())
		{
			mData = Allocate(mSize = mCapacity = other.Size());
			Copy(mData, mData + mSize, other.Begin());
		}
		else
		{
			mData = null;
			mSize = 0;
			mCapacity = 0;
		}
	}

	template <class T>
	noinline_ void Array<T>::ResizeUninitialized(u32 size)
	{
		Reserve(size);
		mSize = size;
	}

	template <class T>
	noinline_ T& Array<T>::GrowAndPushBack(const T& a)
	{
		const u32 capacity = CapacityIncrement();

		T* newData = Allocate(capacity);
		ROCK_ASSERT((!capacity) || (newData && (newData != mData)));
		Copy(newData, newData + mSize, mData);

		// inserting element before destroying old array
		// avoids referencing destroyed object when duplicating array element.
		ROCK_PLACEMENT_NEW(reinterpret_cast<void*>(newData + mSize), T)(a);

		Destroy(mData, mData + mSize);
		Deallocate(mData);

		mData = newData;
		mCapacity = capacity;

		return mData[mSize++];
	}

	template <class T>
	noinline_ void Array<T>::Recreate(u32 capacity)
	{
		T* newData = Allocate(capacity);
		ROCK_ASSERT((!capacity) || (newData && (newData != mData)));

		Copy(newData, newData + mSize, mData);
		Destroy(mData, mData + mSize);
		Deallocate(mData);

		mData = newData;
		mCapacity = capacity;
	}

	template<class T>
	inline_ T* reserveContainerMemory(Array<T>& container, u32 nb)
	{
		const u32 maxNbEntries = container.Capacity();
		const u32 requiredSize = container.Size() + nb;

		if(requiredSize>maxNbEntries)
		{
			const u32 naturalGrowthSize = maxNbEntries ? maxNbEntries*2 : 2;
			const u32 newSize = PxMax(requiredSize, naturalGrowthSize);
			container.Reserve(newSize);
		}

		T* buf = container.End();
		container.ForceSize_Unsafe(requiredSize);
		return buf;
	}
}

#endif

