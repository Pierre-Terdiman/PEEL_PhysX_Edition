///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a generic history buffer.
 *	\file		IceHistoryBuffer.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEHISTORYBUFFER_H
#define ICEHISTORYBUFFER_H

	template<class T> class HistoryBuffer : public Allocateable
	{
		public:
									HistoryBuffer() : mHistory(null), mHistoryDepth(0), mIndex(0)	{}
									~HistoryBuffer()												{	Release();	}

						void		Release()
									{
										DELETEARRAY(mHistory);
										mHistoryDepth	= 0;
										mIndex			= 0;
									}

						bool		Init(uword depth)
									{
										Release();
										if(depth)
										{
											mIndex			= depth;
											mHistoryDepth	= depth;
											mHistory		= ICE_NEW(T)[mHistoryDepth*2];
											CHECKALLOC(mHistory);
										}
										return true;
									}

						void		Record(const T& data)
									{
										if(!mHistory)	return;
										//### subdivision ?

										// Update index
										if(!mIndex)	mIndex = mHistoryDepth;
										mIndex--;
//										if(mIndex==0xffff)	mIndex = mHistoryDepth-1;

										// Record data
										mHistory[mIndex]				= data;
										mHistory[mIndex+mHistoryDepth]	= data;
									}

		// Data access
		inline_			const T*	GetHistory()	const	{ return &mHistory[mIndex];		}
		inline_			uword		GetDepth()		const	{ return mHistoryDepth;			}

		private:
						T*			mHistory;		//!< Recorded history
						uword		mHistoryDepth;	//!< Size of the history array
						uword		mIndex;			//!< Current index
	};

#endif // ICEHISTORYBUFFER_H
