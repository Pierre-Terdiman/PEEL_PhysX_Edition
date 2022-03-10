///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code to limit the FPS.
 *	\file		IceVSync.h
 *	\author		Pierre Terdiman
 *	\date		July, 11, 2007
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVSYNC_H
#define ICEVSYNC_H

	class ICECORE_API VSync
	{
		public:
							VSync(sdword fps)	{ SetTargetFPS(fps);	}
							~VSync()			{}

		inline_	void		SetTargetFPS(sdword fps)
							{
								// Can't go < 1 Ms, i.e. can't go beyond 1000 fps
								mTargetMS = 1000 / fps;
								mError = 0;
							}
		inline_	void		Start()
							{
								mTime = TimeGetTime();
							}
		inline_	void		End()
							{
								const udword CurrentTime = TimeGetTime();

								mTime = CurrentTime - mTime;

								// Target FPS = f == frequency
								// => target milliseconds = 1000/target fps
								sdword Delta = mTargetMS - mTime;
								if(Delta>0)
								{
									// Account for error from last frame
									if(Delta>=mError)	Delta -= mError;

									// The desired delay is "Delta". Compute actual delay and record the error (used next frame)
									udword RealDelay = CurrentTime;
									Delay(Delta);
									RealDelay = TimeGetTime() - RealDelay;
									mError = RealDelay - Delta;
								}
							}
		private:
				sdword		mTargetMS;
				sdword		mTime;
				sdword		mError;
	};

#endif // ICEVSYNC_H
