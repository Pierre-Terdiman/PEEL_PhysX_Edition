#ifndef ROCK_SORT_H
#define ROCK_SORT_H

#include <Core/RockCommon.h>
#include <Core/RockCoreTemplates.h>

namespace Rock
{
	namespace SortInternals
	{
		template <class T, class Predicate>
		inline void Median3(T* elements, i32 first, i32 last, Predicate& compare)
		{
			// This creates sentinels because we know there is an element at the start minimum(or equal)
			// than the pivot and an element at the end greater(or equal) than the pivot. Plus the
			// median of 3 reduces the chance of degenerate behavour.

			const i32 mid = (first + last) / 2;

			if(compare(elements[mid], elements[first]))
				SwapT(elements[first], elements[mid]);

			if(compare(elements[last], elements[first]))
				SwapT(elements[first], elements[last]);

			if(compare(elements[last], elements[mid]))
				SwapT(elements[mid], elements[last]);

			// keep the pivot at last-1
			SwapT(elements[mid], elements[last - 1]);
		}

		template <class T, class Predicate>
		inline i32 Partition(T* elements, i32 first, i32 last, Predicate& compare)
		{
			Median3(elements, first, last, compare);

			i32 i = first;    // we know first is less than pivot(but i gets pre incremented)
			i32 j = last - 1; // pivot is in last-1 (but j gets pre decremented)

			for(;;)
			{
				while(compare(elements[++i], elements[last - 1]));
				while(compare(elements[last - 1], elements[--j]));
				if(i >= j)
					break;

				ROCK_ASSERT(i <= last && j >= first);
				SwapT(elements[i], elements[j]);
			}
			// put the pivot in place

			ROCK_ASSERT(i <= last && first <= (last - 1));
			SwapT(elements[i], elements[last - 1]);
			return i;
		}

		template <class T, class Predicate>
		inline void SmallSort(T* elements, i32 first, i32 last, Predicate& compare)
		{
			// selection sort - could reduce to fsel on 360 with floats.

			for(i32 i=first; i<last; i++)
			{
				i32 m = i;
				for(i32 j = i + 1; j <= last; j++)
					if(compare(elements[j], elements[m]))
						m = j;

				if(m != i)
					SwapT(elements[m], elements[i]);
			}
		}

		class Stack
		{
			u32 mSize, mCapacity;
			i32* mMemory;
			bool mRealloc;

			public:
			Stack(i32* memory, u32 capacity) : mSize(0), mCapacity(capacity), mMemory(memory), mRealloc(false)	{}
			~Stack();

			void Grow();

			inline_ void Push(i32 start, i32 end)
			{
				if(mSize >= mCapacity - 1)
					Grow();
				mMemory[mSize++] = start;
				mMemory[mSize++] = end;
			}

			inline_ void Pop(i32& start, i32& end)
			{
				ROCK_ASSERT(!Empty());
				end = mMemory[--mSize];
				start = mMemory[--mSize];
			}

			inline_ bool Empty()	const
			{
				return mSize == 0;
			}
		};
	}

	template <class T, class Predicate>
	void SortT(T* elements, u32 count, const Predicate& compare, u32 initialStackSize = 1024)
	{
		static const u32 SMALL_SORT_CUTOFF = 5; // must be >= 3 since we need 3 for median

		ROCK_ALLOCA(stackMem, i32, initialStackSize);
		SortInternals::Stack stack(stackMem, initialStackSize);

		i32 first = 0, last = i32(count - 1);
		if(last > first)
		{
			for(;;)
			{
				while(last > first)
				{
					ROCK_ASSERT(first >= 0 && last < i32(count));
					if(u32(last - first) < SMALL_SORT_CUTOFF)
					{
						SortInternals::SmallSort(elements, first, last, compare);
						break;
					}
					else
					{
						const i32 partIndex = SortInternals::Partition(elements, first, last, compare);

						// push smaller sublist to minimize stack usage
						if((partIndex - first) < (last - partIndex))
						{
							stack.Push(first, partIndex - 1);
							first = partIndex + 1;
						}
						else
						{
							stack.Push(partIndex + 1, last);
							last = partIndex - 1;
						}
					}
				}

				if(stack.Empty())
					break;

				stack.Pop(first, last);
			}
		}
#if _DEBUG
		if(1)
		{
			for(u32 i=1; i<count; i++)
				ROCK_ASSERT(!compare(elements[i], elements[i - 1]));
		}
#endif
	}

	template <class T, class Predicate>
	void Sort(T* elements, u32 count, const Predicate& compare)
	{
		SortT(elements, count, compare);
	}

	template <class T>
	void Sort(T* elements, u32 count)
	{
		SortT(elements, count, Less<T>());
	}
}

#endif
