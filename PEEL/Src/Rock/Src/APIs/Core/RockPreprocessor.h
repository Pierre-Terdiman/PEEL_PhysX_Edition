#ifndef ROCK_PREPROCESSOR_H
#define ROCK_PREPROCESSOR_H

	#include <stdlib.h>
	#include <string.h>

	#pragma warning( disable : 4996 ) // This function or variable may be unsafe.

	#ifndef ROCK_NO_DLL
		#ifdef ROCK_EXPORTS
			#define ROCK_API	__declspec(dllexport)
		#else
			#define ROCK_API	__declspec(dllimport)
		#endif
		#define ROCK_FUNCTION	extern "C"
	#else
			#define ROCK_API
			#define ROCK_FUNCTION	// Must be defined to nothing to avoid conflicts with ICE
	#endif

	#define inline_		__forceinline
	#define noinline_	__declspec(noinline)
	#define restrict_	__restrict

#ifndef null
	#define null		0
#endif

	#pragma inline_depth(255)

	#pragma intrinsic(memcmp)
	#pragma intrinsic(memcpy)
	#pragma intrinsic(memset)
	#pragma intrinsic(strcat)
	#pragma intrinsic(strcmp)
	#pragma intrinsic(strcpy)
	#pragma intrinsic(strlen)
	#pragma intrinsic(abs)
	#pragma intrinsic(labs)

	// Macro for avoiding default assignment and copy. Doing this with inheritance can increase class size on some platforms.
	#define ROCK_NOCOPY(Class)	protected:	Class(const Class&);	Class& operator=(const Class&);

	#define __FL__	__FILE__, __LINE__

#endif
