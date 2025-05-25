#include <Core/RockAssert.h>
#include <Core/RockFormat.h>
#include <Core/RockPlatform.h>
#include <assert.h>

bool Rock::CustomAssert(size_t exp, const char* desc, int line, const char* file, bool& ignore)
{
	if(ignore || exp)
		return false;

	const MbReturnCode Ret = Rock::MessageBoxAbortRetryIgnore(_F("In %s\nat line %d\n\n%s", file, line, desc), "Custom assert");
	if(Ret==ROCK_MB_RETRY)
		return false;
	if(Ret==ROCK_MB_ABORT)
		return true;
	assert(Ret==ROCK_MB_IGNORE);
	ignore = true;
	return false;
}

