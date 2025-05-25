#ifndef ROCK_ASSERT_H
#define ROCK_ASSERT_H

#include <Core/RockPreprocessor.h>

namespace Rock
{
	ROCK_FUNCTION ROCK_API bool CustomAssert(size_t, const char*, int, const char*, bool&);
}

	// Custom assert function. Various usages:
	// ROCK_ASSERT(condition)
	// ROCK_ASSERT(!"Not implemented")
	// ROCK_ASSERT(condition && "error text")
	#define ROCK_ASSERT_IMPL(exp)																		\
	{																									\
		static bool IgnoreAlways = false;																\
		if(!IgnoreAlways && Rock::CustomAssert(size_t(exp), #exp, __LINE__, __FILE__, IgnoreAlways))	\
			__debugbreak();																				\
	}

	// Call ROCK_ASSERT_IMPL to assert even in Release builds.
	// Call ROCK_ASSERT to assert only in Debug builds.

#if defined(_DEBUG)
	#define ROCK_ASSERT(exp)	ROCK_ASSERT_IMPL(exp)
#else
	#define ROCK_ASSERT(exp)	{}
#endif

	#define ROCK_COMPILE_TIME_ASSERT(exp)	extern char ROCK_Dummy[ (exp) ? 1 : -1 ]

#endif
