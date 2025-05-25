#ifndef ROCK_LOG_H
#define ROCK_LOG_H

#include <Core/RockCommon.h>

namespace Rock
{
	enum LogType
	{
		LOG_INFO,
		LOG_WARNING,
		LOG_ERROR,
	};

	class ROCK_API Log
	{
		public:
						Log()	{}
		virtual			~Log()	{}

		virtual void	SendText(LogType type, const char* text, const void* user_data=null)	= 0;
	};

	ROCK_FUNCTION ROCK_API	Log&	GetLog();

	#define ROCK_INFO(x)	GetLog().SendText(LOG_INFO, x);
	#define ROCK_WARNING(x)	GetLog().SendText(LOG_WARNING, x);
	#define ROCK_ERROR(x)	GetLog().SendText(LOG_ERROR, x);
}

#endif
