///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains misc timer related code.
 *	\file		IceTimer.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETIMER_H
#define ICETIMER_H

	//! Timer commands
	enum TimerCommand
	{
		TIMER_RESET				= 0,			//!< Reset the timer
		TIMER_START				= 1,			//!< Start the timer
		TIMER_STOP				= 2,			//!< Stop (or pause) the timer
		TIMER_ADVANCE			= 3,			//!< Advance the timer by 0.1 seconds
		TIMER_GETABSOLUTETIME	= 4,			//!< Get the absolute system time
		TIMER_GETAPPTIME		= 5,			//!< Get the current time
		TIMER_GETELAPSEDTIME	= 6,			//!< Get the time that elapsed between TIMER_GETELAPSEDTIME calls

		TIMER_FORCE_DWORD		= 0x7fffffff
	};

	enum TimeFormat
	{
		TF_H_M_S,	//!< Hours::Minutes::Seconds
		TF_M_S_MS,	//!< Minutes::Seconds::Milliseconds
	};

	FUNCTION ICECORE_API bool ConvertTimeToString(float time, char* buffer, TimeFormat format);
	FUNCTION ICECORE_API void Delay(udword ms);
	FUNCTION ICECORE_API float SetupTimer(TimerCommand command);

	class ICECORE_API FPS
	{
		public:
								FPS();
								~FPS();

				void			Update();
		inline_	float			GetFPS()		const	{ return mFPS;			}
		inline_	float			GetInstantFPS()	const	{ return mInstantFPS;	}

		private:
				float			mLastTime;
				udword			mFrames;
				float			mLastTime2;
				udword			mFrames2;
				float			mFPS;
				float			mInstantFPS;
	};

	class ICECORE_API Timer
	{
		public:
								Timer();
								~Timer();

				int				Refresh();

				double			mTime;				// Time since windows started
				double			mLastTime;			// Last recorded time
				double			mFrameTime;			// Time elapsed in the last frame
				int				mFrames;			// Number of frames

				LARGE_INTEGER	mFrequency;
				double			mResolution;
		private:
				udword			mOldTime;
				int				mSameTimeCount;		// Counter for frames with the same time.

				int				mLowShift;
				bool			mPerformanceTimerEnabled;
	};

#endif // ICETIMER_H
