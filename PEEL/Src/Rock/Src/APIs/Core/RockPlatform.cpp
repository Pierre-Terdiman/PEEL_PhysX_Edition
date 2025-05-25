#include <Core/RockPlatform.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <stdio.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")

#define	ROCK_WIN32_CALL(x)	x

using namespace Rock;

BOOL APIENTRY DllMain( HMODULE /*hModule*/,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void Rock::SetConsoleTextColor(ConsoleTextColor console_text_color)
{
	WORD Color = 0;
	if(console_text_color & CONSOLE_TEXT_COLOR_RED)
		Color |= FOREGROUND_RED;
	if(console_text_color & CONSOLE_TEXT_COLOR_GREEN)
		Color |= FOREGROUND_GREEN;
	if(console_text_color & CONSOLE_TEXT_COLOR_BLUE)
		Color |= FOREGROUND_BLUE;

	ROCK_WIN32_CALL(SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color|FOREGROUND_INTENSITY));
}

void Rock::OpenDebugConsole()
{
	if(ROCK_WIN32_CALL(AllocConsole()))
	{
		freopen("CONOUT$", "wt", stdout);
		ROCK_WIN32_CALL(SetConsoleTitleA("ROCK Debug Console"));
		ROCK_WIN32_CALL(SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_TEXT_COLOR_WHITE|FOREGROUND_INTENSITY));
	}
}

void SetupTimeBeginPeriod()
{
	// This needs to be called before you use "Sleep". Without this, a call like Sleep(1) can take
	// much longer than 1ms (typically 16ms by default). The same call with the same parameter must
	// be done at the start & end of the app so we use a wrapper to make sure the parameter matches.
	// Even with this calls, "Sleep" is not terribly accurate.
	ROCK_WIN32_CALL(timeBeginPeriod(1));
}

u32 TimeGetTime()
{
	return ROCK_WIN32_CALL(timeGetTime());
}

MbReturnCode Rock::MessageBoxAbortRetryIgnore(const char* msg, const char* title)
{
	const int Ret = ROCK_WIN32_CALL(MessageBoxA(null, msg, title, MB_ABORTRETRYIGNORE|MB_ICONWARNING));
	if(Ret==IDRETRY)
		return ROCK_MB_RETRY;
	if(Ret==IDIGNORE)
		return ROCK_MB_IGNORE;
	return ROCK_MB_ABORT;
}
