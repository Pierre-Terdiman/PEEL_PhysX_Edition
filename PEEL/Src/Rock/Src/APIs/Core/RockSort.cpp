#include <Core/RockSort.h>
#include <Core/RockAllocator.h>
#include <Core/RockMemory.h>

using namespace Rock;
using namespace SortInternals;

Stack::~Stack()
{
	if(mRealloc)
		ROCK_FREE(mMemory);
}

void Stack::Grow()
{
	mCapacity *= 2;
	i32* newMem = ROCK_ALLOCATE(i32, mCapacity, "Stack");
	CopyMemory_(newMem, mMemory, mSize * sizeof(i32));
	if(mRealloc)
		ROCK_FREE(mMemory);
	mRealloc = true;
	mMemory = newMem;
}
