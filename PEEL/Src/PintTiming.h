///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_TIMING_H
#define PINT_TIMING_H

	#define MAX_NB_RECORDED_FRAMES	1024*8

	struct PintRecord
	{
		udword	mTime;
		udword	mUsedMemory;
	};

	class PintTiming : public Allocateable
	{
		public:
							PintTiming();
							~PintTiming();

		inline_	void		ResetTimings()			{ mNbCalls = mCurrentMemory = mCurrentTime = mAvgTime = mWorstTime = 0;	}
		inline_	udword		GetAvgTime()	const	{ return mNbCalls ? udword(mAvgTime/mNbCalls) : 0;						}

		inline_	void		RecordTimeAndMemory(udword time, udword memory, udword frame_nb)
							{
								mNbCalls++;
								mAvgTime += time;
								mCurrentTime = time;
								mCurrentMemory = memory;
								if(time>mWorstTime)
									mWorstTime = time;
								if(frame_nb<MAX_NB_RECORDED_FRAMES)
								{
									mRecorded[frame_nb].mTime = time;
									mRecorded[frame_nb].mUsedMemory = memory;
								}
							}

		inline_	void		UpdateRecordedTime(udword time, udword frame_nb)
							{
								mAvgTime += time;
								mCurrentTime += time;
								if(mCurrentTime>mWorstTime)
									mWorstTime = mCurrentTime;
								if(frame_nb<MAX_NB_RECORDED_FRAMES)
									mRecorded[frame_nb].mTime += time;
							}

				udword		mNbCalls;
				udword		mCurrentTestResult;
				udword		mCurrentMemory;
				udword		mCurrentTime;
				udword		mAvgTime;
				udword		mWorstTime;
				PintRecord	mRecorded[MAX_NB_RECORDED_FRAMES];
	};

#endif
