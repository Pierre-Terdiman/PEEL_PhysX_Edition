///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains the reference tracking system.
 *	\file		IceRefTracker.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEREFTRACKER_H
#define ICEREFTRACKER_H

	// Tracker flags
/*	enum RefTrackFlag
	{
		REFTRACK_INVALID_OFFSETS	=	(1<<0),		//!< Offsets need recomputing (lazy evaluation)

		REFTRACK_FORCE_DWORD		=	0x7fffffff
	};
*/
	typedef udword	RTYPE;		//!< Relationship-type (!) between owners and references

	// RTYPE-related macros
	#define RTYPE_NBBITS			16
	#define RTYPE_OWNER(x)			(x&0xffff)
	#define RTYPE_REF(x)			((x>>16)&0xffff)
	#define RTYPE_SETMASK			0x0000ffff
	#define RTYPE_CLEARMASK			0xffff0000
	#define RTYPE_MAKE(owner, ref)	(((ref&0xffff)<<16)|(owner&0xffff))

	enum RTYPEOP
	{
		RTYPEOP_REPLACE				= 0,
		RTYPEOP_AND					= 1,
		RTYPEOP_OR					= 2,

		RTYPEOP_FORCE_DWORD			= 0x7fffffff
	};

	class ICECORE_API RefTracker
	{
		public:
							RefTracker();
							~RefTracker();

				bool		AddRef(uword owner, uword ref, RTYPE type=0);
				bool		RemoveRef(uword owner, uword ref, Container* marked_indices=null);
				bool		RemoveIndex(uword id, Container* marked_indices=null);

		inline_	udword*		GetOwnersArray()		const		{ return mOwners;		}
		inline_	udword*		GetRefsArray()			const		{ return mRefs;			}
		inline_	udword		GetNbPairs()			const		{ return mCurNbPairs;	}

				udword		GetNbRefs(uword owner)	const;
				udword		GetNbOwners(uword ref)	const;

		inline_	udword		GetOwner(uword ref, udword i)	const
							{
								// Lazy evaluation: create valid offsets
//								if(mFlags&REFTRACK_INVALID_OFFSETS)	const_cast<RefTracker* const>(this)->CreateOffsets();	// "mutable method"
								if(!mRefsOffsets)	const_cast<RefTracker* const>(this)->CreateOffsets();	// "mutable method"

								return (mOwners[mSortedRefs[mRefsOffsets[ref-mMinRef]+i]]>>RTYPE_NBBITS);
							}

		inline_	udword		GetRef(uword owner, udword i)	const
							{
								// Lazy evaluation: create valid offsets
//								if(mFlags&REFTRACK_INVALID_OFFSETS)	const_cast<RefTracker* const>(this)->CreateOffsets();	// "mutable method"
								if(!mOwnersOffsets)	const_cast<RefTracker* const>(this)->CreateOffsets();	// "mutable method"

								return (mRefs[mSortedOwners[mOwnersOffsets[owner-mMinOwner]+i]]>>RTYPE_NBBITS);
							}

				bool		PairExists(uword owner, uword ref)	const	{ return GetPairIndex(owner, ref)!=INVALID_ID;	}
				bool		SetType(uword owner, uword ref, RTYPE type, RTYPEOP op);

				udword		GetUsedRam();

		// Dump
				void		Dump();

							PREVENT_COPY(RefTracker)
		private:
//				udword		mFlags;					//!< Tracker flags

				udword		mMaxNbPairs;			//!< Number of dwords allocated in mOwners & mRefs
				udword		mCurNbPairs;			//!< Number of used entries
				udword*		mOwners;				//!< Array of mCurNbPairs owners
				udword*		mRefs;					//!< Array of mCurNbPairs references

				udword*		mSortedOwners;			//!< Array of sorted owners
				udword*		mSortedRefs;			//!< Array of sorted references

				udword*		mOwnersOffsets;			//!< Radix-like offsets for owners
				udword*		mRefsOffsets;			//!< Radix-like offsets for references

				udword		mMinOwner;				//!< Smallest owner index
				udword		mMinRef;				//!< Smallest reference index
				udword		mMaxOwner;				//!< Largest owner index
				udword		mMaxRef;				//!< Largest reference index

		// Internal methods
				bool		CreateOffsets();
				void		ReleaseOffsets();
				bool		PurgeArray(const Container& marked_links);
				udword		GetPairIndex(uword owner, uword ref)													const;
				udword		GetPairIndex2(uword owner, uword ref, Container& marked_links)							const;
				bool		MarkLink(uword owner, uword ref, Container& marked_links, Container& marked_indices)	const;
				bool		MarkIndex(uword id, Container& marked_links, Container& marked_indices)					const;
	};

#endif // ICEREFTRACKER_H
