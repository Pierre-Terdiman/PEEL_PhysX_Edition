#ifndef ROCK_PLATFORM_H
#define ROCK_PLATFORM_H

#include <Core/RockCommon.h>
#include <windows.h>
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

	void	SetupTimeBeginPeriod();
	u32		TimeGetTime();

namespace Rock
{
	ROCK_FUNCTION ROCK_API	void	OpenDebugConsole();

	enum ConsoleTextColor
	{
		CONSOLE_TEXT_COLOR_RED		= (1<<0),
		CONSOLE_TEXT_COLOR_GREEN	= (1<<1),
		CONSOLE_TEXT_COLOR_BLUE		= (1<<2),
		CONSOLE_TEXT_COLOR_YELLOW	= CONSOLE_TEXT_COLOR_RED|CONSOLE_TEXT_COLOR_GREEN,
		CONSOLE_TEXT_COLOR_MAGENTA	= CONSOLE_TEXT_COLOR_RED|CONSOLE_TEXT_COLOR_BLUE,
		CONSOLE_TEXT_COLOR_CYAN		= CONSOLE_TEXT_COLOR_GREEN|CONSOLE_TEXT_COLOR_BLUE,
		CONSOLE_TEXT_COLOR_WHITE	= CONSOLE_TEXT_COLOR_RED|CONSOLE_TEXT_COLOR_GREEN|CONSOLE_TEXT_COLOR_BLUE
	};

	ROCK_FUNCTION ROCK_API	void	SetConsoleTextColor(ConsoleTextColor console_text_color);

	enum MbReturnCode
	{
		ROCK_MB_ABORT,
		ROCK_MB_RETRY,
		ROCK_MB_IGNORE,
	};

	ROCK_FUNCTION ROCK_API	MbReturnCode	MessageBoxAbortRetryIgnore(const char* msg, const char* title);

	inline_ u32 HighestSetBitUnsafe(u32 v)
	{
		unsigned long retval;
		_BitScanReverse(&retval, v);
		return retval;
	}

	inline_ u32 LowestSetBitUnsafe(u32 v)
	{
		unsigned long retval;
		_BitScanForward(&retval, v);
		return retval;
	}

	inline_ u32 CountLeadingZeros(u32 v)
	{
		if(v)
		{
			unsigned long bsr = (unsigned long)-1;
			_BitScanReverse(&bsr, v);
			return 31 - bsr;
		}
		else
			return 32;
	}

}

#endif
