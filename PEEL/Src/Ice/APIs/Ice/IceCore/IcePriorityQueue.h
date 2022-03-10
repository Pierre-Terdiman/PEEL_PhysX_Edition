///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple priority queue.
 *	\file		IcePriorityQueue.h
 *	\author		Pierre Terdiman
 *	\date		February, 5, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPRIORITYQUEUE_H
#define ICEPRIORITYQUEUE_H

	enum PriorityMode
	{
		PM_SMALL_FIRST,		//!< Small priorities should be served first
		PM_BIG_FIRST,		//!< Big priorities should be served first
	};

	class ICECORE_API PriorityQueue : public Pairs
	{
		public:
		//! Constructor
								PriorityQueue()	: mPriorityMode(PM_BIG_FIRST), mMaxNbAllowed(MAX_UDWORD)	{}
		//! Destructor
								~PriorityQueue()															{}

				void			Push(udword value, udword priority);
				bool			Pop(Pair& next_pair);

		inline_	const Pair*		NextPair()
								{
									if(!HasPairs()) return null;
									return (const Pair*)GetPairs();
								}

		inline_	PriorityMode	GetPriorityMode()						const	{ return mPriorityMode;			}
		inline_	void			SetPriorityMode(PriorityMode mode)				{ mPriorityMode = mode;			}

		inline_	udword			GetMaxNbAllowed()						const	{ return mMaxNbAllowed;			}
		inline_	void			SetMaxNbAllowed(udword max_allowed)				{ mMaxNbAllowed = max_allowed;	}

		protected:

				PriorityMode	mPriorityMode;		//!< Management mode
				udword			mMaxNbAllowed;		//!< Maximum number of entries allowed (use this to limit used ram)
	};

#endif // ICEPRIORITYQUEUE_H
