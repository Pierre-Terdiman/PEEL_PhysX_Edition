///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains permutation code.
 *	\file		IcePermutation.h
 *	\author		Pierre Terdiman
 *	\date		March, 7, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPERMUTATION_H
#define ICEPERMUTATION_H

	class ICECORE_API Permutation
	{
		public:
								Permutation();
								Permutation(udword nb_elements);
								Permutation(udword nb_elements, const uword* old_list, const uword* new_list);
								~Permutation();

				void			Free();
				bool			Init(udword nb_elements);
				bool			Identity();
				bool			Random(udword nb_swaps);
				bool			Reverse();
				bool			Rebuild(udword nb_faces, const uword* old_list, const uword* new_list);
				bool			IsValid()				const;

		inline_	udword			GetSize()				const	{ return mSize; }
		inline_	const udword*	GetList()				const	{ return mList; }

		//! Access as an array
		inline_	udword&			operator[](udword i)	const	{ ASSERT(i<mSize); return mList[i];	}

		private:
				udword			mSize;	//!< Number of elements
				udword*			mList;	//!< List of elements
	};

#endif // ICEPERMUTATION_H
