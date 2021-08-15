///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for ICE kernel.
 *	\file		IceKernel.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEKERNEL_H
#define ICEKERNEL_H

	#define USE_KID_CACHE

	// Returns the V-Table [handle with care]
	FUNCTION ICECORE_API VTABLE			GetVTable(Cell* cell);

	// Create a dynamic identifier
	FUNCTION ICECORE_API DynID			CreateID(const String& desc);
	// Get the dynamic identifier descriptor
	FUNCTION ICECORE_API const String*	GetDescriptor(DynID id);

	// Create & return the global kernel
	FUNCTION ICECORE_API Kernel*		GetKernel();

	// Release the global kernel
	FUNCTION ICECORE_API void			ReleaseKernel();

	// Translation shortcut
	FUNCTION ICECORE_API Cell*			KIDToCell(KID kid);

	// Greasy KID stuff ....
	#define	PTR(kid)	KIDToCell(kid)

	//! ICE dynamic cast
	inline_ Cell* DynamicCast(Cell* cell, const char* class_name)
	{
		// Check type
		return (!cell || !cell->IsKindOf(class_name)) ? null : cell;
	}

	//! ICE dynamic cast
	inline_ const Cell* DynamicCast(const Cell* cell, const char* class_name)
	{
		// Check type
		return (!cell || !cell->IsKindOf(class_name)) ? null : cell;
	}

	//! ICE dynamic cast
	inline_	Cell* DynamicCast(KID kid, const char* class_name)
	{
		// Catch cell pointer
		Cell* Tmp = PTR(kid);
		return DynamicCast(Tmp, class_name);
	}

	//! Kernel flags
	enum KernelFlag
	{
		KF_CELL_CONTROL		=	(1<<0),		//!< Enables high-level cell control (self-destructions, etc)
		KF_GLOBAL_MESSAGES	=	(1<<1),		//!< Enables global messages
		KF_SECURITY			=	(1<<2),		//!< Enables high-level security. Kernel is allowed to automatically disable "explosive" features.
		KF_CELL_NOTIF		=	(1<<3),		//!< Enables Kernel-to-cells notifications
		KF_CELL_MODIF		=	(1<<4),		//!< Allows the kernel to automatically fix bad pointers within cells (handle that one with care)
		KF_CELL_UNIQUE_NAME	=	(1<<5),		//!< Forces unique names. Allows ICCB_DUPLICATENAMEFOUND notifications.

		KF_FORCE_DWORD		=	0x7fffffff
	};

	//! Kernel remapping modes
	enum RemappingMode
	{
		KRM_TYPE			=	0,			//!< Remap by type
		KRM_ADDRESS			=	1,			//!< Remap by address
		KRM_USER			=	2,			//!< Remap by user-defined param

		KRM_FORCE_DWORD		=	0x7fffffff
	};

	class ICECORE_API Kernel : public Allocateable
	{
		private:
									Kernel();
		virtual						~Kernel();

		public:
									DECLARE_ALL_FLAGS(KernelFlag, mFlags)

		// API registration & cell factory

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Registers a class creator. In theory there's one class creator for each API.
		 *	\param		api			[in] API name
		 *	\param		callback	[in] class creation callback
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OBSOLETE
				bool				RegisterAPI		(const char* api, ClassCreator callback);
//				bool				RegisterAPI		(IceAPI* api);
#else
				bool				RegisterClass	(const char* class_name, ClassCreator creator, ClassDestructor destructor);
#endif

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Creates a registered class given a classname.
		 *	\param		class_name	[in] name of the desired class
		 *	\return		the newly created class, or null if failed
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OBSOLETE
				void*				CreateClass		(const char* class_name);
#else
				Cell*				CreateInstance	(const char* class_name);
				bool				DeleteInstance	(Cell* instance);
#endif

		// References machinery
				bool				CreateRef		(Cell* owner, Cell* ref);
				bool				CreateRef		(KID ownerkid, KID refkid);
				bool				DeleteRef		(Cell* owner, Cell* ref);
				bool				DeleteRef		(KID ownerkid, KID refkid);
		// Experimental
				bool				ReplaceRef		(Cell* owner, Cell* oldref, Cell* newref);
		// Data access
		inline_	udword				GetNbRefs		(const Cell* cell)				const	{ return mTracker.GetNbRefs(ToWord(cell->m__KernelID));		}
		inline_	udword				GetNbOwners		(const Cell* cell)				const	{ return mTracker.GetNbOwners(ToWord(cell->m__KernelID));	}
				Cell*				GetOwner		(const Cell* cell, udword i)	const;
				Cell*				GetRef			(const Cell* cell, udword i)	const;

		inline_	bool				CreateTypedRef	(Cell* owner, Cell* ref, RTYPE type)	{ return mTracker.AddRef(ToWord(owner->m__KernelID), ToWord(ref->m__KernelID), type);}

		// Cell searching

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets back a list of cells whose type is given.
		 *	\param		list	[out] list of cells. The container is not reset at first so you can append more and more cells from different types.
		 *	\param		type	[in] wanted type
		 *	\return		the number of cells found
		 *	\see		GetCell(const char* name)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				udword				GetCells		(CellsContainer& list, const char* type);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets back a cell whose name is given.
		 *	\param		name	[in] the cell's name
		 *	\return		the wanted cell, or null if not found
		 *	\see		GetCells(CellsContainer& list, const char* type)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				Cell*				GetCell			(const char* name);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets back a list of cells whose name looks like given one.
		 *	\param		list	[out] list of cells. The container is sorted so that most probable cells are the first ones.
		 *	\param		name	[in] wanted cell's name
		 *	\return		number of cells in returned list
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				udword				FindCell(CellsContainer& list, const String& name);

		// Check kernel integrity
				bool				Checkup			(bool delete_lost_cells=false);
#ifndef _WIN64
		// Kernel remapping
				bool				Remap(RemappingMode mode, udword* user_defined=null);
#endif

		//! Cell access. The first entry is always the kernel itself, which is not even a cell-based component.
		//! Hence the -1 and +1 to return the relevant parts only.
#ifdef USE_HANDLE_MANAGER
		inline_	udword				GetNbEntries	()			const	{ return mTranslator2.GetNbObjects()-1;		}
		inline_	udword				GetMaxNbEntries	()			const	{ return mTranslator2.GetMaxNbObjects()-1;	}
		inline_	Cell**				GetEntries		()			const	{ return ((Cell**)mTranslator2.GetObjects())+1;	}
#else
		inline_	udword				GetNbEntries	()			const	{ return mTranslator.GetNbEntries()-1;		}
		inline_	udword				GetMaxNbEntries	()			const	{ return mTranslator.GetMaxNbEntries()-1;	}
		inline_	Cell**				GetEntries		()			const	{ return ((Cell**)mTranslator.GetList())+1;	}
#endif
		// Stats
		inline_	udword				GetTrackerRam	()					{ return mTracker.GetUsedRam();				}

		//! High-speed KernelID-to-cell translation
#ifdef USE_HANDLE_MANAGER
	#ifdef USE_KID_CACHE
		inline_	Cell*				KIDToCell		(KID kid)	const
									{
										if(kid==mCachedKID)
										{
											mCacheHits++;
										}
										else
										{
											mCachedCell = (Cell*)mTranslator2.GetObject(kid);
											mCachedKID = kid;
											mCacheMiss++;
										}
										return mCachedCell;
									}
	#else
		inline_	Cell*				KIDToCell		(KID kid)	const	{ return (Cell*)mTranslator2.GetObject(kid);}
	#endif
#else
		inline_	Cell*				KIDToCell		(KID kid)	const	{ return (Cell*)mTranslator.GetPointer(kid);}
#endif
		//! Cell self-destruction user-param
		inline_	void*				GetSDUP			()			const	{ return mSDUP;								}
		inline_	void				SetSDUP			(void* param)		{ mSDUP = param;							}

		// Debug
				RefTracker*			GetTracker()						{ return &mTracker; }
				void				Dump();

#ifdef USE_KID_CACHE
		mutable	KID					mCachedKID;
		mutable	Cell*				mCachedCell;
		mutable	udword				mCacheMiss;
		mutable	udword				mCacheHits;
		inline_	void				InvalidateCache()	{ mCachedKID = INVALID_KID;	mCachedCell = null;	}
#endif
									PREVENT_COPY(Kernel)
		private:
				void*				mSDUP;				//!< Self-Destruction user-parameter
				RefTracker			mTracker;			//!< Reference tracker
#ifdef USE_HANDLE_MANAGER
				HandleManager		mTranslator2;		//!< Main translation module
#else
				XList				mTranslator;		//!< Main translation module
#endif
#ifdef OBSOLETE
				Container			mCallbacks;			//!< Class creators
#else
				Constants			mClassDescriptors;	//!< Class descriptors
#endif
				WatcherManager		mWatchers;			//!< Watcher manager

		// Secure cell self-destruction

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Deletes a given cell.
		 *	\param		cell	[in] the cell to delete
		 *	\return		Self-reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				Kernel&				TerminateCell	(Cell* cell);
				void				Terminate(const Container& marked_indices);

		// Cells registration

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Registers a cell. Keeps track of it in the kernel.
		 *	\param		object	[in] the cell to register
		 *	\return		true if success
		 *	\see		Unregister(Cell* object)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				Register		(Cell* object);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Unregisters a cell. Removes it from the kernel.
		 *	\param		object	[in] the cell to unregister
		 *	\return		true if success
		 *	\see		Register(Cell* object)
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				Unregister		(Cell* object);

		// Experimental stuff

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Fixes invalid pointers and invalid KIDs within cell-based classes. This is an advanced ICE feature, possibly quite dangerous.
		 *	The kernel here has full control and can modify all class members, even private ones. Use the appropriate flag if you want to
		 *	ban this behaviour.
		 *	\param		owner			[in] the cell to fix
		 *	\param		bad_ref			[in] the invalid cell pointer (the one we want to fix within the cell)
		 *	\param		new_ref			[in] the new cell pointer or null (will replace the old one)
		 *	\param		allow_modif		[in] true to allow the actual fix, else just look for the bad pointer without fixing it
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				FixInvalidReference(Cell* owner, Cell* bad_ref, Cell* new_ref=null, bool allow_modif=true);

		friend	ICECORE_API	Kernel*	GetKernel();
		friend	ICECORE_API	void	ReleaseKernel();
		friend	class				Cell;
		friend	class				Watcher;
	};

#ifdef ICE_OBSOLETE
	class ICECORE_API SmartKID
	{
		public:
									SmartKID(KID kid=INVALID_KID)	: mKid(kid)		{}
									~SmartKID()										{}

									operator void*()		{ return (void*)(udword(KIDToCell(mKid))-INVALID_ID);	}
				Cell*				operator->()	const	{ return PTR(mKid);										}

//				SmartKID& operator= (const SmartKID& k)	{ return *this; }
//				Cell&		operator*()		const	{ return *PTR(mKid); }

		private:
				KID					mKid;

		friend	class				Kernel;
	};
#endif

#endif // ICEKERNEL_H
