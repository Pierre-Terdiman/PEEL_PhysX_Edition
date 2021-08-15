///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains obsolete memory manager. Kept for compatibility with very old code.
 *	\file		IceOldMM.h
 *	\author		Pierre Terdiman
 *	\date		February, 25, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEOLDMM_H__
#define __ICEOLDMM_H__

	struct MemoryCell;

	class ICECORE_API MemoryManager
	{
		private:

		MemoryCell*			mCurrentCell;				//!< Active cell
		MemoryCell*			mInitCell;					//!< Initial cell
		long				mTotal;						//!< Total allocated ram
		bool				mIsAligned;					//!< Is alignment on/off ?

		public:

		//! Constructor
							MemoryManager();
		//! Destructor
							~MemoryManager();

				void*		Malloc(size_t n);								//!< Allocates a buffer and keep track of it.
				bool		Free(void* buffer);								//!< Frees a previously allocated buffer.
		inline_	long		RamUsed()	const	{ return mTotal;		}	//!< Returns total ram used
		inline_	void		AlignOn()			{ mIsAligned = true;	}	//!< Set the alignment on
		inline_	void		AlignOff()			{ mIsAligned = false;	}	//!< Set the alignment off
	};

#endif // __ICEOLDMM_H__
