///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QPCTime.h"

static signed __int64 getTimeTicks()
{
	LARGE_INTEGER a;
	QueryPerformanceCounter (&a);
	return a.QuadPart;
}

static double getTickDuration()
{
	LARGE_INTEGER a;
	QueryPerformanceFrequency (&a);
	return 1.0f / double(a.QuadPart);
}

static double sTickDuration = getTickDuration();

static const CounterFrequencyToTensOfNanos gCounterFreq = QPCTime::getCounterFrequency();

const CounterFrequencyToTensOfNanos& QPCTime::getBootCounterFrequency()
{
	return gCounterFreq;
}

CounterFrequencyToTensOfNanos QPCTime::getCounterFrequency()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency (&freq);
	return CounterFrequencyToTensOfNanos( QPCTime::sNumTensOfNanoSecondsInASecond, freq.QuadPart );
}


unsigned __int64 QPCTime::getCurrentCounterValue()
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter (&ticks);
	return ticks.QuadPart;
}

QPCTime::QPCTime(): mTickCount(0)
{
	getElapsedSeconds();
}

QPCTime::Second QPCTime::getElapsedSeconds()
{
	signed __int64 lastTickCount = mTickCount;
	mTickCount = getTimeTicks();
	return (mTickCount - lastTickCount) * sTickDuration;
}

QPCTime::Second QPCTime::peekElapsedSeconds()
{
	return (getTimeTicks() - mTickCount) * sTickDuration;
}

QPCTime::Second QPCTime::getLastTime() const
{
	return mTickCount * sTickDuration;
}
