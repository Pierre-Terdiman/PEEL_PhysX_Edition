///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for watchers.
 *	\file		IceWatcher.h
 *	\author		Pierre Terdiman
 *	\date		December, 26, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEWATCHER_H
#define ICEWATCHER_H

	// ### Experimental stuff, you probably shouldn't be using it

	class ICECORE_API Watcher
	{
		public:
		inline_				Watcher(const Cell* watched)	{ *this = watched;			}
		inline_				Watcher(const Watcher& object)	{ *this = object.mWatched;	}
							~Watcher();

		//! Operator for "Watcher A = Cell* B"
				void		operator = (const Cell* object);

		//! Cast operator for const Cell* = Watcher
		inline_				operator const Cell*()	const	{ return mWatched;			}
		//! Cast operator for Cell* = Watcher
		inline_				operator Cell*()		const	{ return (Cell*)mWatched;	}
		//! Operator for if(Watcher)
		inline_				operator void*()				{ return (void*)mWatched;	}
		//! Operator for if(Watcher)
		inline_	bool		operator !()					{ return mWatched==null;	}
		//! Operator for Watcher->Method()
		inline_	const Cell*	operator->()			const	{ return mWatched;			}
		//! Operator for Watcher->Method()
		inline_	Cell*		operator->()					{ return (Cell*)mWatched;	}

		inline_	Cell*		GetWatched()					{ return (Cell*)mWatched;	}
		inline_	const Cell*	GetWatched()			const	{ return mWatched;			}

		private:
		const	Cell*		mWatched;						//!< Watched object

		friend	class		WatcherManager;
	};

	class ICECORE_API WatcherManager
	{
		private:
							WatcherManager();
							~WatcherManager();

				bool		AddWatcher(Watcher& watcher);
				bool		RemoveWatcher(Watcher& watcher);

				bool		InvalidateWatched(const Cell* watched);

				udword		mFlags;				//!< Internal flags

				udword		mMaxNbWatchers;		//!< Max. number of watchers
				udword		mCurNbWatchers;		//!< Current number of watchers

				Watcher**	mWatchers;			//!< List of watchers
				udword*		mWatchedKIDs;		//!< List of watched KIDs
				udword*		mSortedKIDs;		//!< List of sorted watched KID indices
				udword*		mOffsets;			//!< Radix-like offsets for KID indices
				udword		mMinKID;			//!< Smallest KID index
				udword		mMaxKID;			//!< Largest KID index
		// Internal methods
				udword		GetNbWatchers(const Cell* watched);
				bool		CreateOffsets();

		friend	class		Kernel;
		friend	class		Watcher;
	};

#endif // ICEWATCHER_H
