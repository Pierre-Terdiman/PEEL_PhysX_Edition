///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains profiling code.
 *	\file		IceProfiler.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPROFILER_H
#define ICEPROFILER_H

	// Forward declarations
	class Profiler;

	FUNCTION ICECORE_API void		SetBaseTime(udword time);
	FUNCTION ICECORE_API udword		GetBaseTime();
	FUNCTION ICECORE_API Profiler&	GetProfiler();

	#define PROFILE_APP		// It's been tested with various scenes that profiling the app doesn't slow it down at all

#ifdef PROFILE_APP
	#define __PROFILE_START(label)	GetProfiler().StartProfile(label);
	#define __PROFILE_END(label)	GetProfiler().EndProfile(label);
#else
	#define __PROFILE_START(label)
	#define __PROFILE_END(label)
#endif

#ifdef _WIN64
	inline_ void InitProfiler()
	{
	}
	inline_ void	StartProfile(udword& val)
	{
		val = udword(__rdtsc());
	}
	inline_ void	EndProfile(udword& val)
	{
		val = udword(__rdtsc()) - val;
	}
#else
	//! This function initializes the profiler by counting the cpuid overhead.
	//! This is done 3 times on purpose, since cpuid takes a longer time to execute the first times it's called.
	//! "cpuid" is used before rdtsc to prevent out-of-sequence execution from producing wrong results.
	//! For more details, read Intel's application notes "Using the RDTSC instruction for performance monitoring".
	//!	\see		StartProfile
	//!	\see		EndProfile
	inline_ void InitProfiler()
	{
		udword cyc, Base;
#if defined(WIN32)
		_asm{
			cpuid
			rdtsc
			mov		cyc, eax
			cpuid
			rdtsc
			sub		eax, cyc
			mov		Base, eax

			cpuid
			rdtsc
			mov		cyc, eax
			cpuid
			rdtsc
			sub		eax, cyc
			mov		Base, eax

			cpuid
			rdtsc
			mov		cyc, eax
			cpuid
			rdtsc
			sub		eax, cyc
			mov		Base, eax
		}
#elif defined(LINUX)
      //__asm__("cpuid\n\t"
			   //     "rdtsc\n\t"
			   //     "mov      %0,   eax\n\t"
			   //     "cpuid\n\t"
			   //     "rdtsc\n\t"
			   //     "sub      eax,  %0\n\t"
			   //     "mov      %1,   eax\n\t"

			   //     "cpuid\n\t"
			   //     "rdtsc\n\t"
			   //     "mov      %0,   eax\n\t"
			   //     "cpuid\n\t"
			   //     "rdtsc\n\t"
			   //     "sub      eax,  %0\n\t"
			   //     "mov      %1,   eax\n\t"

			   //     "cpuid\n\t"
			   //     "rdtsc\n\t"
			   //     "mov      %0,   eax\n\t"
			   //     "cpuid\n\t"
			   //     "rdtsc\n\t"
			   //     "sub      eax,  %0\n\t"
			   //     "mov      %1,   eax"
			   //     : :"0"(cyc), "1"(Base)
			   //     :"%eax");

#elif defined(__APPLE__)
    #error TODO!
#else
    #error No platform defined!
#endif
		SetBaseTime(Base);
	}

	//!	This function starts recording the number of cycles elapsed.
	//!	\param		val		[out] address of a 32 bits value where the system should store the result.
	//!	\see		EndProfile
	//!	\see		InitProfiler
	inline_ void	StartProfile(udword& val)
	{
#if defined(WIN32)
		__asm{
			cpuid
			rdtsc
			mov		ebx, val
			mov		[ebx], eax
		}
#elif defined(LINUX)
  //asm("cpuid\n\t"
		//  "rdtsc\n\t"
		//	"mov    ebx,    val\n\t"
		//	"mov    (%ebx), eax");
#elif defined(__APPLE__)
  #error TODO!
#else
    #error No platform defined!
#endif
	}

	//!	This function ends recording the number of cycles elapsed.
	//!	\param		val		[out] address to store the number of cycles elapsed since the last StartProfile.
	//!	\see		StartProfile
	//!	\see		InitProfiler
	inline_ void	EndProfile(udword& val)
	{
#if defined(WIN32)
		__asm{
			cpuid
			rdtsc
			mov		ebx, val
			sub		eax, [ebx]
			mov		[ebx], eax
		}
#elif defined(LINUX)
		//asm("cpuid\n\t"
		//    "rdtsc\n\t"
		//	  "mov    ebx,    val\n\t"
		//	  "sub    eax,    (%ebx)\n\t"
		//	  "mov    (%ebx), eax");
#elif defined(__APPLE__)
		#error TODO!!!
#else
    #error No platform defined!
#endif
		val-=GetBaseTime();
		if(sdword(val)<0)	val=0;
	}
#endif

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	User-callback, called by the profiler on starting/ending profiling a given process.
	 *	\param		bool		[in] true when profile starts, false when profile ends
	 *	\param		user_data	[in] user-defined data
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef void	(*PROFILE_CALLBACK)	(bool begin, void* user_data);

	struct ICECORE_API ProfileData
	{
		udword		mOrder;				//!< Insertion order
		const char*	mLabel;				//!< User-defined label
		udword		mNbCycles;			//!< Number of elapsed cycles
		float		mPercents;			//!< NbCycles * 100 / TotalNbCycles
		udword		mRecursionLevel;	//!< Level of recursion
	};
	CHECK_CONTAINER_ITEM(ProfileData)

	class ICECORE_API Profiler : public Allocateable
	{
		public:
									Profiler();
									~Profiler();
		// Start of frame call
				void				Reset();
		// Profiling
		inline_	bool				StartProfile(const char* label=null)
									{
										// Check the user has called Reset() beforehand.
										if(!mIsReady)	return false;
										return _StartProfile(label);
									}

		inline_	bool				EndProfile(const char* label=null)
									{
										// Check the user has called Reset() beforehand.
										if(!mIsReady)	return false;
										return _EndProfile(label);
									}

		// End of frame call
				ProfileData*		GetResults(udword& nb_records, udword& total_nb_cycles);

		// Callback control - used to pick out one particular event
		inline_	void				SetUserData(void* data)					{ mUserData	= data;		}
		inline_	void				SetCallback(PROFILE_CALLBACK callback)	{ mCallback	= callback;	}
		inline_	void				SetCallbackEvent(const char* label)		{ mEvent	= label;	}

		private:
		// User callback
				void*				mUserData;			//!< User-defined data sent to the possible callback
				PROFILE_CALLBACK	mCallback;			//!< User-defined callback
				const char*			mEvent;				//!< Event triggering off the callback

				LIFOStack			mCyclesCounters;	//!< Elapsed cycles
				LIFOStack			mOrder;				//!< Insertion orders
				Container			mRecorder;			//!< Data records
				udword				mNbRecords;			//!< Current number of recorded events
				udword				mRecursionLevel;	//!< Current recursion level
				bool				mIsReady;			//!< true if Reset() has been called
				bool				mEndingPending;		//!< true if the ending callback is pending
		// Internal methods
				bool				_StartProfile(const char* label);
				bool				_EndProfile(const char* label);
	};

	class ProfileEntry
	{
		public:
		inline_				ProfileEntry(const char* label) : mLabel(label)	{ __PROFILE_START(mLabel)	}
		inline_				~ProfileEntry()									{ __PROFILE_END(mLabel)		}

				const char*	mLabel;
	};

	#undef PROFILE_MARK
	#define PROFILE_MARK(x)	ProfileEntry PE(x);

#endif // ICEPROFILER_H
