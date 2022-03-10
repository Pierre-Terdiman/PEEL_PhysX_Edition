///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintTiming.h"

PintTiming::PintTiming() :
	mNbCalls			(0),
	mCurrentTestResult	(INVALID_ID),
	mCurrentMemory		(0),
	mCurrentTime		(0),
	mAvgTime			(0),
	mWorstTime			(0)
{
	ZeroMemory(mRecorded, sizeof(PintRecord)*MAX_NB_RECORDED_FRAMES);
}

PintTiming::~PintTiming()
{
}

