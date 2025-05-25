#include <Core/RockFormat.h>
#include <stdio.h>
#include <stdarg.h>

using namespace Rock;

/*
	Here's a safe version of sprintf called format.  Unlike sprintf, it
	won't produce buffer overflows and doesn't require the caller to
	allocate and deallocate the output buffer.  The return value is an
	std::string that will be deallocated automatically when the last
	reference to it is dropped.  The function is optimized for short
	strings but will work correctly for strings up to 1e6 bytes in length.

	This code is part of the <A HREF="http://graphics3d.com/cpp">Graphics3D
	library</A> and may be used under the terms of that license (which is
	similar to the MIT/Berkley "don't sue me" license).

	Example usage:
	  int month = 7, day = 26, year = 76;
	  std::string name = "Birthday";
	  std::string result =
			 format("%s = %d-%02d-%02d\n", f.c_str(), month, day, year);

	Morgan McGuire
	matrix@graphics3d.com
*/
static void vformat(const char* fmt, va_list argPtr, String& str)
{
	// We draw the line at a 1MB string.
	const int maxSize = 1000000;

	// If the string is less than 161 characters,
	// allocate it on the stack because this saves
	// the malloc/free time.
	//const int bufSize = 161;
	const int bufSize = 1025;
	char stackBuffer[bufSize];

	int attemptedSize = bufSize - 1;

	int numChars = _vsnprintf(stackBuffer, attemptedSize, fmt, argPtr);

	if(numChars>=0)
	{
		// Got it on the first try.
		str.Set(stackBuffer);
	}
	else
	{
		// Now use the heap.
		char* heapBuffer = null;

		while((numChars == -1) && (attemptedSize < maxSize))
		{
			// Try a bigger size
			attemptedSize *= 2;
			heapBuffer = (char*)realloc(heapBuffer, attemptedSize + 1);
			numChars = _vsnprintf(heapBuffer, attemptedSize, fmt, argPtr);
		}

		str.Set(heapBuffer);
		free(heapBuffer);
	}
}

String Rock::_F(const char* format, ...)
{
	String Str;

	va_list argList;
    va_start(argList, format);

		vformat(format, argList, Str);

    va_end(argList);

	// TODO: we could probably save an allocation here
    return Str;
}

