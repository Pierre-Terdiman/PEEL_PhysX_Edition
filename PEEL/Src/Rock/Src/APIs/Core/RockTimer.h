#ifndef ROCK_TIMER_H
#define ROCK_TIMER_H

#include <Core/RockCommon.h>
#include <Core/RockQPC.h>

namespace Rock
{
	// Granularity: cycles
	ROCK_FUNCTION ROCK_API void StartProfile_RDTSC(u64& val);
	ROCK_FUNCTION ROCK_API void EndProfile_RDTSC(u64& val);

	// Granularity: us
	ROCK_FUNCTION ROCK_API void StartProfile_QPC(QPCTime& timer);
	ROCK_FUNCTION ROCK_API u32 EndProfile_QPC(QPCTime& timer);

	// Granularity: ms
	ROCK_FUNCTION ROCK_API void StartProfile_TimeGetTime(u32& val);
	ROCK_FUNCTION ROCK_API void EndProfile_TimeGetTime(u32& val);
}

#endif
