///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a dynamic cross-list class.
 *	\file		IceXList.h
 *	\author		Pierre Terdiman
 *	\date		December, 12, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEXLIST_H
#define ICEXLIST_H

    typedef uword  VirtualIndex;

//#define XLIST_STATS

#ifdef XLIST_STATS
	DECLARE_COUNTER(ICECORE_API, XListCounter)
#endif

	class ICECORE_API XList
#ifdef XLIST_STATS
		: public XListCounter
#endif
	{
		public:
								XList();
								~XList();
		// Basic usage
				VirtualIndex	Add(udword value);
				void			Remove(VirtualIndex handle);

		// Advanced usage
				bool			Remap(const udword* ranks);

		// Physical data access
		inline_	udword			GetMaxNbEntries()				const	{ return mMaxNbEntries;		}	//!< Returns max number of entries
		inline_	udword			GetNbEntries()					const	{ return mCurrentNbEntries;	}	//!< Returns current number of entries
		inline_	udword*			GetList()						const	{ return mList;				}	//!< Gets the complete list
				udword			GetValue(VirtualIndex handle)	const;	// Returns real value according to handle.
				void*			GetPointer(VirtualIndex handle)	const;	// Returns real value according to handle.

		//! High-speed access - same as GetValue without any checkings - handle with care.
		inline_	udword			PickValue(VirtualIndex handle)	const	{ return mList[mOutToIn[handle]]; }

		// Stats
				udword			GetUsedRam()					const;

								PREVENT_COPY(XList)
		private:
		// Physical list
				udword*			mList;				//!< Physical list, with no holes but unsorted.
				udword			mCurrentNbEntries;	//!< Current number of values in the physical list.
				udword			mMaxNbEntries;		//!< Maximum possible number of values in the physical list.

		// Cross-references
				uword*			mOutToIn;			//!< Maps virtual indices (handles) to real ones.
				uword*			mInToOut;			//!< Maps real indices to virtual ones (handles).

		// Recycled locations
				udword			mNbFreeIndices;		//!< Current number of free indices

		// Internal methods
				bool			SetupLists(udword* list=null, uword* oti=null, uword* ito=null);
	};

#endif // ICEXLIST_H


