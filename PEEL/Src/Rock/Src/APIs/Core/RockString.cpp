#include <Core/RockString.h>
#include <Core/RockMemory.h>
#include <Core/RockAllocator.h>

using namespace Rock;

String::String(const char* string) : mText(null)
{
	if(string)
		Set(string);
}

String::String(const String& string) : mText(null)
{
	const char* Str = string;
	if(Str)
		Set(Str);
}

String::~String()
{
	Reset();
}

void String::Reset()
{
	ROCK_FREE(mText);
}

bool String::Set(const char* string)
{
	Reset();
	if(string)
	{
		const size_t Length = strlen(string);
		if(Length)
		{
			mText = ROCK_ALLOCATE(char, (Length+1), "String::mText");
			ROCK_CHECKALLOC(mText);
			CopyMemory_(mText, string, Length);
			mText[Length] = 0;
		}
	}
	return true;
}
