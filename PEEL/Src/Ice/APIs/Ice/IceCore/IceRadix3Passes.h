///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A 3-passes radix-sort using a 11-bit wide radix.
 *	\file		IceRadix3Passes.h
 *	\author		Pierre Terdiman
 *	\date		December, 20, 2006
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERADIX3PASSES_H
#define ICERADIX3PASSES_H

	class ICECORE_API RadixSort3 : public Allocateable
	{
		public:
								RadixSort3();
								~RadixSort3();
		// Sorting methods
				RadixSort3&		Sort(const udword* input, udword nb, RadixHint hint=RADIX_SIGNED);
				RadixSort3&		Sort(const float* input, udword nb);

		//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
		inline_	const udword*	GetRanks()			const	{ return mRanks;		}

		//! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
		inline_	udword*			GetRecyclable()		const	{ return mRanks2;		}

		// Stats
		//! Returns the total number of calls to the radix sorter.
		inline_	udword			GetNbTotalCalls()	const	{ return mTotalCalls;	}
		//! Returns the number of eraly exits due to temporal coherence.
		inline_	udword			GetNbHits()			const	{ return mNbHits;		}

				bool			SetRankBuffers(udword* ranks0, udword* ranks1);

								PREVENT_COPY(RadixSort3)
		private:
				udword			mCurrentSize;		//!< Current size of the indices list
				udword*			mRanks;				//!< Two lists, swapped each pass
				udword*			mRanks2;
		// Stats
				udword			mTotalCalls;		//!< Total number of calls to the sort routine
				udword			mNbHits;			//!< Number of early exits due to coherence
		// Stack-radix
				bool			mDeleteRanks;		//!<
		// Internal methods
				void			CheckResize(udword nb);
				bool			Resize(udword nb);
	};

	#define StackRadixSort3(name, ranks0, ranks1)	\
		RadixSort3 name;							\
		name.SetRankBuffers(ranks0, ranks1);

#endif // ICERADIX3PASSES_H
