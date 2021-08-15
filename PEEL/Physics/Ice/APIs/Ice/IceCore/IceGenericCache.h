///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for a generic cache.
 *	\file		IceGenericCache.h
 *	\author		Pierre Terdiman
 *	\date		September, 13, 2004
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEGENERICCACHE_H
#define ICEGENERICCACHE_H

	typedef udword	CacheIndex;

	class ICECORE_API GenericCache : public Allocateable
	{
		public:
								GenericCache();
								~GenericCache();

				bool			Init(udword cache_size);

				CacheIndex		CacheData(udword nb_bytes, const void* data=null, void** cache=null);
				bool			ShrinkCache(CacheIndex cookie, udword used_size);
				void*			GetData(CacheIndex cookie)	const;

		inline_	void			CacheBegin()			{ mMarker = mWritePtr;						}
		inline_	udword			CacheEnd()		const	{ return ::abs(int(mWritePtr - mMarker));	}
		inline_	udword			getSize()		const	{ return mCacheSize;						}
		private:
				udword			mCacheSize;
				CacheIndex		mWritePtr;		//!< Virtual running index
				CacheIndex		mPtrOwner;		//!< Owner of write pointer
				ubyte*			mPool;			//!< Cyclic-array of mCacheSize bytes
				CacheIndex		mMarker;		//!< 

		// Internal methods
				void			Release();
				void*			AllocFromCache(udword nb_bytes, CacheIndex& cookie);
		inline_	udword			ComputeRealIndex(udword virtual_index)	const	{ return mCacheSize ? virtual_index % mCacheSize : 0;	}
	};

#endif // ICEGENERICCACHE_H
