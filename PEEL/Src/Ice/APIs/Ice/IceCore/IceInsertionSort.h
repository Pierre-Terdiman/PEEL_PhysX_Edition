///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for insertion sort.
 *	\file		IceInsertionSort.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEINSERTIONSORT_H
#define ICEINSERTIONSORT_H

	class ICECORE_API InsertionSort : public Allocateable
	{
		public:
								InsertionSort();
								~InsertionSort();
		// Sorting methods
				InsertionSort&	Sort(const sdword* input, udword nb);
				InsertionSort&	Sort(const udword* input, udword nb);
				InsertionSort&	Sort(const float* input, udword nb);

		//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
		inline_	const udword*	GetRanks()			const	{ return mRanks;	}

								PREVENT_COPY(InsertionSort)
		private:
				udword			mCurrentSize;		//!< Current size of the indices list
				udword*			mRanks;				//!< Ranks
		// Internal methods
				bool			CheckResize(udword nb);
	};

#endif // ICEINSERTIONSORT_H
