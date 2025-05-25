#include <Core/RockLog.h>
#include <Core/RockPlatform.h>
#include <stdio.h>

using namespace Rock;

namespace
{
	class BasicLog : public Log
	{
		public:
						BasicLog()	{}
		virtual			~BasicLog()	{}

		virtual void	SendText(LogType type, const char* text, const void* /*user_data*/)	override
		{
			if(type==LOG_INFO)
			{
				SetConsoleTextColor(CONSOLE_TEXT_COLOR_WHITE);
				printf(text);
			}
			else
			{
				if(type==LOG_WARNING)
					SetConsoleTextColor(CONSOLE_TEXT_COLOR_YELLOW);
				else if(type==LOG_ERROR)
					SetConsoleTextColor(CONSOLE_TEXT_COLOR_RED);
				printf(text);
				SetConsoleTextColor(CONSOLE_TEXT_COLOR_WHITE);
			}
		}
	};
}

static BasicLog gBasicLog;
static Log* gCurrentLog = null;

void SetupLog(Log* log)
{
	gCurrentLog = log ? log : &gBasicLog;
}

Log& Rock::GetLog()
{
	ROCK_ASSERT(gCurrentLog);
	return *gCurrentLog;
}

