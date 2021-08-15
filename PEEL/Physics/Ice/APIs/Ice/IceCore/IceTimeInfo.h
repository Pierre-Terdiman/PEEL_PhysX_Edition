///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a time information class.
 *	\file		IceTimeInfo.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETIMEINFO_H
#define ICETIMEINFO_H

	// Cycle limit
	inline_	udword ClampFrameIndex(udword frame_index, udword limit, BOOL cycle)
	{
		if(cycle)	return frame_index % limit;
		else		return frame_index < limit ? frame_index : limit - 1;
	}

	class ICECORE_API TimeInfo
	{
		public:
						TimeInfo();
						~TimeInfo();

		// Time management
				void	Reset();

		inline_	udword	GetGlobalFrameCounter()					const	{ return mGlobalFrameCounter;						}
		inline_	void	SetGlobalFrameCounter(udword counter)			{ mGlobalFrameCounter = counter;					}

		inline_	float	GetInterpolant()						const	{ return mInterpolant;								}
		inline_	void	SetInterpolant(float interpolant)				{ mInterpolant = interpolant;						}
//		inline_	float	Lerp(float f0, float f1)				const	{ return (1.0f - mInterpolant)*f0 + mInterpolant*f1;}
		inline_	float	Lerp(float f0, float f1)				const	{ return f0 + mInterpolant * (f1 - f0);				}

#ifdef ICECORE_TIMEINFO_INTEGER
				void	UpdateGlobalTime(udword relative_time);

		inline_	float	ComputePreviousGlobalTime()				const	{ return float(mGlobalTime - mRelativeTime)*0.001f; }
		inline_	float	GetDeltaTM()							const	{ return 1.0f / float(mDeltaTM);					}
		inline_	void	SetDeltaTM(udword delta_tm)						{ mDeltaTM = delta_tm;								}

		inline_	float	GetRelativeTime()						const	{ return float(mRelativeTime)*0.001f;				}
		inline_	float	GetGlobalTime()							const	{ return float(mGlobalTime)*0.001f;					}

		private:
		// Static time information
				udword	mDeltaTM;				//!< Time between two frames (say 1.0f/30.0f)

		// Dynamic time information
				udword	mRelativeTime;			//!< Elapsed time since last frame (in milli-seconds)
				udword	mGlobalTime;			//!< Global time reference (in milli-seconds)
#else
				void	UpdateGlobalTime(float relative_time);

		inline_	float	ComputePreviousGlobalTime()				const	{ return mGlobalTime - mRelativeTime;				}
		inline_	float	GetDeltaTM()							const	{ return mDeltaTM;									}
		inline_	void	SetDeltaTM(float delta_tm)						{ mDeltaTM = delta_tm;								}

		inline_	float	GetRelativeTime()						const	{ return mRelativeTime;								}
		inline_	void	SetRelativeTime(float relative_time)			{ mRelativeTime = relative_time;					}

		inline_	float	GetGlobalTime()							const	{ return mGlobalTime;								}
		inline_	void	SetGlobalTime(float global_time)				{ mGlobalTime = global_time;						}
		inline_	void	ResetGlobalTime(udword base_frame=0)			{ mGlobalTime = mDeltaTM * float(base_frame);		}

		private:
		// Static time information
				float	mDeltaTM;				//!< Time between two frames (say 1.0f/30.0f)
		// Dynamic time information
				float	mRelativeTime;			//!< Elapsed time since last frame
				float	mGlobalTime;			//!< Global time reference
#endif
				udword	mGlobalFrameCounter;	//!< Current global frame number (a.k.a. time stamp)
				float	mInterpolant;			//!< Interpolation coeff between two frames (0.0f => 1.0f)
	};

#endif // ICETIMEINFO_H
