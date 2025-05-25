#include <Core/RockTimer.h>
#include <Core/RockQPC.h>
#include <Core/RockPlatform.h>

#include <intrin.h>
#pragma intrinsic(__rdtsc)

void Rock::StartProfile_RDTSC(u64& val)
{
	val = __rdtsc();
}

void Rock::EndProfile_RDTSC(u64& val)
{
	val = __rdtsc() - val;
}

// We must pass a QPCTime instance to the functions below, because "getElapsedSeconds" updates a
// local time value in the class. So we cannot safely use a global timer in multi-threaded code.

void Rock::StartProfile_QPC(QPCTime& timer)
{
	timer.getElapsedSeconds();
}

u32 Rock::EndProfile_QPC(QPCTime& timer)
{
	const QPCTime::Second s = timer.getElapsedSeconds();
	return u32(s*1000000.0);
}

void Rock::StartProfile_TimeGetTime(u32& val)
{
	val = TimeGetTime();
}

void Rock::EndProfile_TimeGetTime(u32& val)
{
	val = TimeGetTime() - val;
}
