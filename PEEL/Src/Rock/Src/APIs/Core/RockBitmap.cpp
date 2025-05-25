#include <Core/RockBitmap.h>

using namespace Rock;

void Bitmap::Extend(u32 size)
{
	const u32 newWordCount = (size + 31) >> 5;
	if(newWordCount > GetWordCount())
	{
		u32* newMap = ROCK_ALLOCATE(u32, newWordCount, "Bitmap");
		if(mMap)
		{
			CopyMemory(newMap, mMap, GetWordCount() * sizeof(u32));
			ROCK_FREE(mMap);
		}
		ZeroMemory(newMap + GetWordCount(), (newWordCount - GetWordCount()) * sizeof(u32));
		mMap = newMap;
		mWordCount = newWordCount;
	}
}

void Bitmap::ExtendUninitialized(u32 size)
{
	const u32 newWordCount = (size + 31) >> 5;
	if(newWordCount > GetWordCount())
	{
		ROCK_FREE(mMap);
		mWordCount = newWordCount;
		mMap = ROCK_ALLOCATE(u32, newWordCount, "Bitmap");
	}
}
