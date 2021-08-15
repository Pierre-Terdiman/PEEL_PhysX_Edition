///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a generic array reducer.
 *	\file		IceArrayReducer.h
 *	\author		Pierre Terdiman
 *	\date		April, 23, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEARRAYREDUCER_H
#define ICEARRAYREDUCER_H

	enum ReduceFlag
	{
		REDUCE_RANKS	= (1<<0),
		REDUCE_HASH		= (1<<1)
	};

	class ICECORE_API ArrayReducer
	{
		public:
		// Constructors/destructor
								ArrayReducer();
								~ArrayReducer();
		// Free used bytes
				ArrayReducer&	Clean();
		// Cloud reduction
				bool			Reduce(const udword* array, udword nb_elements, udword element_size, udword flags=0);
		// Data access
		inline_	udword			GetNbReduced()			const	{ return mNbReduced;	}
		inline_	const udword*	GetReduced()			const	{ return mReduced;		}
		inline_	const udword*	GetCrossRefTable()		const	{ return mXRef;			}

		private:
		// Reduced array
				udword			mNbReduced;	//!< Reduced number of elements
				udword*			mReduced;	//!< Reduced array
				udword*			mXRef;		//!< Cross-reference table
	};

#endif // ICEARRAYREDUCER_H

