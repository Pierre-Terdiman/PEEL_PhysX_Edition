///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a timestamp manager.
 *	\file		IceTimestampManager.h
 *	\author		Pierre Terdiman
 *	\date		May, 9, 2004
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETIMESTAMPMANAGER_H
#define ICETIMESTAMPMANAGER_H

	class ICECORE_API TimestampsManager
	{
		public:
							TimestampsManager();
							~TimestampsManager();
		// Explicit API
				bool		Reserve(udword size);
				udword		IncreaseTime();
		inline_	udword		GetSize()										const	{ return mSize;						}
		inline_	udword		GetCurrentTime()								const	{ return mCurrentTime;				}
		inline_	udword*		GetStamps()												{ return mStamps;					}
		//
		inline_	BOOL		MustProcess(udword index, udword time_stamp)	const	{ return mStamps[index]-time_stamp;	}
		inline_	void		Tag(udword index, udword time_stamp)			const	{ mStamps[index]=time_stamp;		}

		inline_	BOOL		Update(udword index, udword time_stamp)			const //returns true if it was not yet tagged.
							{
								if(mStamps[index]==time_stamp)	return FALSE;	// Already processed
								mStamps[index]=time_stamp;		return TRUE;
							}
		private:
				udword		mSize;
				udword		mCurrentTime;
				udword*		mStamps;
	};

#endif // ICETIMESTAMPMANAGER_H
