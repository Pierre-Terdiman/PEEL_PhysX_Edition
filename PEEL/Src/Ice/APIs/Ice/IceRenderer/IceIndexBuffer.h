///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains Index-Buffer base code.
 *	\file		IceIndexBuffer.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEINDEXBUFFER_H
#define ICEINDEXBUFFER_H

	enum IndexSize
	{
		IB_UNDEFINED	= 0,							//!< Undefined
		IB_INDEX16		= 101,							//!< Indices are 16 bits each.
		IB_INDEX32		= 102,							//!< Indices are 32 bits each.

		IB_FORCE_DWORD	= 0x7fffffff
	};

	//! Index Buffer creation structure
	struct ICERENDERER_API INDEXBUFFERCREATE
	{
								INDEXBUFFERCREATE();

				udword			mNbIndices;				//!< Number of indices
				IndexSize		mIdxSize;				//!< Word or dwords
				const void*		mIndices;				//!< Source indices, or null
				IceFile*		mFileIndices;			//!< Source indices, or null
				PrimType		mPType;					//!< Type of the primitive we're indexing into
				// Subsets
				uword			mNbSubsets;				//!< Number of subsets
				const uword*	mWSubsets;				//!< List of number of word-indices for each subset
				const udword*	mDSubsets;				//!< List of number of dword-indices for each subset
				bool			mComputePrimCount;		//!< Store primitive counts (else number of indices)
				bool			mMakeRelative;			//!<
	};

	class ICERENDERER_API Subset : public Allocateable
	{
		public:
					Subset();
					~Subset();

		// Subset description
		udword		mCount;		//!< Number of indices or number of primitives in the subset
		udword		mOffset;	//!< Offset of the first triangle in the index buffer
		udword		mMinIndex;	//!< Min index
		// Subset sorting
		RadixSort*	mSorter;	//!< Possibly embedded radix sorter (for temporal coherence)
		Point		mDirCache;	//!< Possibly cached direction vector
	};

	//! Index buffer descriptor
	struct ICERENDERER_API IBDescriptor{
	};

	//! Index buffer class
	class ICERENDERER_API IndexBuffer : public Hardwired, public Allocateable
	{
		protected:
									IndexBuffer();
		virtual						~IndexBuffer();

		public:

		// Creation
		virtual				bool	Create(const INDEXBUFFERCREATE& create);
		virtual				bool	Release();

		// Lock/Unlock
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Locks the hardwired surface and get a pointer to it.
		 *	\param		flags	[in] LockFlags combination
		 *	\param		offset	[in] Offset into the vertex data to lock, in bytes
		 *	\param		size	[in] Size of the vertex data to lock, in bytes
		 *	\return		pointer to locked memory, or null
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Hardwired)	void*	Lock(udword flags, udword offset = 0, udword size = 0)
									{
//										if(size)	*size = GetBufferSize();
										mLock = mIndices;
										return mIndices;
									}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Unlocks a previously locked surface.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		override(Hardwired)	void	Unlock()					{ mLock = null;							}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Copies a system memory buffer to its hardware counterpart. Destination is always the result of Lock().
		 *	\param		source		[in] system memory source buffer
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							bool	CopyLogicToPhysic(const void* source)
									{
										// Checkings
										if(!source)	return false;
										// Get the hardware pointer (our destination buffer)
			//							udword Size;
			//							void* Dest = Lock(LOCK_FLUSH, &Size);
										void* Dest = Lock(LOCK_NOSYSLOCK | LOCK_WRITEONLY);
 										// The copy is always relevant when the result of the lock isn't the already existing system memory.
										// It happens when a device doesn't expose a hardware counterpart of the data. Ex : index buffers in DX7
										// don't exist at hardware level.
										if(Dest && source!=Dest)	CopyMemory(Dest, source, GetBufferSize());
										// Finished!
										Unlock();
										return true;
									}

		// Management
					IndexBuffer&	FreeData();
		inline_		bool			Refresh()					{ return CopyLogicToPhysic(mIndices);	}

		// Data access
		inline_		udword			GetNbIndices()		const	{ return mNbIndices;					}
		inline_		udword			GetIndexSize()		const	{ return mIdxSize;						}
		inline_		void*			GetIndices()		const	{ return mIndices;						}
		inline_		PrimType		GetPrimType()		const	{ return mPType;						}
		inline_		udword			GetPrimCount()		const	{ return mPrimCount;					}
		inline_		uword			GetNbSubsets()		const	{ return mNbSubsets;					}
		inline_		Subset*			GetSubset(udword i)	const	{ return &mSubsets[i];					}
		inline_		udword			GetBufferSize()		const	{ return mNbIndices*mIdxSize;			}

		protected:
					udword			mNbIndices;		//!< Number of indices
					udword			mIdxSize;		//!< Size of a single index in bytes
					void*			mIndices;		//!< Source indices. This is a system memory copy in any case.
					PrimType		mPType;			//!< Type of the primitive we're indexing into
					udword			mPrimCount;		//!< Primitive count
					uword			mNbSubsets;		//!< Number of subsets
					Subset*			mSubsets;		//!< List of subsets

		friend		class			Renderer;
	};

#endif // ICEINDEXBUFFER_H
