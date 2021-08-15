
#ifdef TEMPLATED_MANAGER
#ifdef TEMPLATED_OBJECT

//! Initial list size
#define DEFAULT_HANDLEMANAGER_SIZE	2

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEMPLATED_MANAGER::TEMPLATED_MANAGER() : mCurrentNbObjects(0), mNbFreeIndices(0)
{
	mMaxNbObjects	= DEFAULT_HANDLEMANAGER_SIZE;
	mObjects		= (TEMPLATED_OBJECT*)	ICE_ALLOC(sizeof(TEMPLATED_OBJECT)*mMaxNbObjects);
	mOutToIn		= (uword*)				ICE_ALLOC(sizeof(uword)*mMaxNbObjects);
	mInToOut		= (uword*)				ICE_ALLOC(sizeof(uword)*mMaxNbObjects);
	mStamps			= (uword*)				ICE_ALLOC(sizeof(uword)*mMaxNbObjects);
	FillMemory(mOutToIn, mMaxNbObjects*sizeof(uword), 0xff);
	FillMemory(mInToOut, mMaxNbObjects*sizeof(uword), 0xff);
	ZeroMemory(mStamps, mMaxNbObjects*sizeof(uword));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEMPLATED_MANAGER::~TEMPLATED_MANAGER()
{
	SetupLists();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases old arrays and assign new ones.
 *	\param		objects	[in] new objects address
 *	\param		oti		[in] new out-to-in address
 *	\param		ito		[in] new in-to-out address
 *	\param		stamps	[in] new stamps address
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TEMPLATED_MANAGER::SetupLists(TEMPLATED_OBJECT* objects, uword* oti, uword* ito, uword* stamps)
{
	// Release old data
	ICE_FREE(mStamps);
	ICE_FREE(mInToOut);
	ICE_FREE(mOutToIn);
	ICE_FREE(mObjects);
	// Assign new data
	mObjects	= objects;
	mOutToIn	= oti;
	mInToOut	= ito;
	mStamps		= stamps;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Adds an object to the manager.
 *	\param		object	[in] the new object to be added.
 *	\return		The object's handle. Use it as further reference for the remove method.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Handle TEMPLATED_MANAGER::Add(const TEMPLATED_OBJECT& object)
{
	// Are there any free indices I should recycle?
	if(mNbFreeIndices)
	{
		const uword FreeIndex = mInToOut[mCurrentNbObjects];	// Get the recycled virtual index
		mObjects[mCurrentNbObjects]	= object;					// The physical location is always at the end of the list (it never has holes).
		mOutToIn[FreeIndex]			= mCurrentNbObjects++;		// Update virtual-to-physical remapping table.
		mNbFreeIndices--;
		return (mStamps[FreeIndex]<<16)|FreeIndex;				// Return virtual index (handle) to the client app
	}
	else
	{
		ASSERT(mCurrentNbObjects<0xffff && "Internal error - 64K objects in HandleManager!");

		// Is the array large enough for another entry?
		if(mCurrentNbObjects==mMaxNbObjects)
		{
			// Nope! Resize all arrays (could be avoided with linked lists... one day)
			mMaxNbObjects<<=1;													// The more you eat, the more you're given
			if(mMaxNbObjects>0xffff)	mMaxNbObjects = 0xffff;					// Clamp to 64K
			TEMPLATED_OBJECT* NewList	= (TEMPLATED_OBJECT*)ICE_ALLOC(sizeof(TEMPLATED_OBJECT)*mMaxNbObjects);		// New physical list
			uword* NewOTI				= (uword*)ICE_ALLOC(sizeof(uword)*mMaxNbObjects);		// New remapping table
			uword* NewITO				= (uword*)ICE_ALLOC(sizeof(uword)*mMaxNbObjects);		// New remapping table
			uword* NewStamps			= (uword*)ICE_ALLOC(sizeof(uword)*mMaxNbObjects);		// New stamps
			CopyMemory(NewList, mObjects,	mCurrentNbObjects*sizeof(TEMPLATED_OBJECT));	// Copy old data
			CopyMemory(NewOTI, mOutToIn,	mCurrentNbObjects*sizeof(uword));				// Copy old data
			CopyMemory(NewITO, mInToOut,	mCurrentNbObjects*sizeof(uword));				// Copy old data
			CopyMemory(NewStamps, mStamps,	mCurrentNbObjects*sizeof(uword));				// Copy old data
			FillMemory(NewOTI+mCurrentNbObjects, (mMaxNbObjects-mCurrentNbObjects)*sizeof(uword), 0xff);
			FillMemory(NewITO+mCurrentNbObjects, (mMaxNbObjects-mCurrentNbObjects)*sizeof(uword), 0xff);
			ZeroMemory(NewStamps+mCurrentNbObjects, (mMaxNbObjects-mCurrentNbObjects)*sizeof(uword));
			SetupLists(NewList, NewOTI, NewITO, NewStamps);
		}

		mObjects[mCurrentNbObjects]	= object;						// Store object at mCurrentNbObjects = physical index = virtual index
		mOutToIn[mCurrentNbObjects]	= ToWord(mCurrentNbObjects);	// Update virtual-to-physical remapping table.
		mInToOut[mCurrentNbObjects]	= ToWord(mCurrentNbObjects);	// Update physical-to-virtual remapping table.
		const udword tmp = mCurrentNbObjects++;
		return (mStamps[tmp]<<16)|tmp;								// Return virtual index (handle) to the client app
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Removes an object from the manager.
 *	\param		handle	[in] the handle returned from the Add() method.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TEMPLATED_MANAGER::Remove(Handle handle)
{
	const uword VirtualIndex = uword(handle);
	if(VirtualIndex>=mMaxNbObjects)		return;			// Invalid handle
	const uword PhysicalIndex = mOutToIn[VirtualIndex];	// Get the physical index
	if(PhysicalIndex==0xffff)			return;			// Value has already been deleted
	if(PhysicalIndex>=mMaxNbObjects)	return;			// Invalid index

	// There must be at least one valid entry.
	if(mCurrentNbObjects)
	{
		if(mStamps[VirtualIndex]!=handle>>16)	return;							// Stamp mismatch => index has been recycled

		// Update list so that there's no hole
		mObjects[PhysicalIndex]					= mObjects[--mCurrentNbObjects];// Move the real object so that the array has no holes.
		mOutToIn[mInToOut[mCurrentNbObjects]]	= PhysicalIndex;				// Update virtual-to-physical remapping table.
		mInToOut[PhysicalIndex]					= mInToOut[mCurrentNbObjects];	// Update physical-to-virtual remapping table.
		// Keep track of the recyclable virtual index
		mInToOut[mCurrentNbObjects]				= VirtualIndex;					// Store the free virtual index/handle at the end of mInToOut
		mOutToIn[VirtualIndex]					= 0xffff;						// Invalidate the entry
		mNbFreeIndices++;														// One more free index
		mStamps[VirtualIndex]++;												// Update stamp
	}
}

// Experimental
void TEMPLATED_MANAGER::RemovePhysicalIndex(udword physical_index)
{
	// There must be at least one valid entry.
	if(!mCurrentNbObjects)	return;

//	uword VirtualIndex = uword(handle);
	const uword VirtualIndex = mInToOut[physical_index];
//	if(VirtualIndex>=mMaxNbObjects)		return;		// Invalid handle

//	uword PhysicalIndex = mOutToIn[VirtualIndex];	// Get the physical index
//	if(PhysicalIndex==0xffff)			return;		// Value has already been deleted
	if(physical_index>=mMaxNbObjects)	return;		// Invalid index

	{
//		if(mStamps[VirtualIndex]!=handle>>16)	return;							// Stamp mismatch => index has been recycled

		// Update list so that there's no hole
		mObjects[physical_index]				= mObjects[--mCurrentNbObjects];// Move the real object so that the array has no holes.
		mOutToIn[mInToOut[mCurrentNbObjects]]	= ToWord(physical_index);		// Update virtual-to-physical remapping table.
		mInToOut[physical_index]				= mInToOut[mCurrentNbObjects];	// Update physical-to-virtual remapping table.
		// Keep track of the recyclable virtual index
		mInToOut[mCurrentNbObjects]				= VirtualIndex;					// Store the free virtual index/handle at the end of mInToOut
		mOutToIn[VirtualIndex]					= 0xffff;						// Invalidate the entry
		mNbFreeIndices++;														// One more free index
		mStamps[VirtualIndex]++;												// Update stamp
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets an object from the list. Returns real pointer according to handle.
 *	\param		handle	[in] the handle returned from the Add() method.
 *	\return		the corresponding object
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEMPLATED_OBJECT* TEMPLATED_MANAGER::GetObject(Handle handle) const
{
	uword VirtualIndex = uword(handle);
	if(VirtualIndex>=mMaxNbObjects)			return null;	// Invalid handle
	uword PhysicalIndex = mOutToIn[VirtualIndex];			// Get physical index
	if(PhysicalIndex==0xffff)				return null;	// Object has been deleted
	if(PhysicalIndex>=mMaxNbObjects)		return null;	// Index is invalid
	if(mStamps[VirtualIndex]!=handle>>16)	return null;	// Index has been recycled
	return &mObjects[PhysicalIndex];						// Returns stored pointer
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Updates an object.
 *	\param		handle	[in] the handle returned from the Add() method.
 *	\param		object	[in] the object's new address
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TEMPLATED_MANAGER::UpdateObject(Handle handle, const TEMPLATED_OBJECT& object)
{
	uword VirtualIndex = uword(handle);
	if(VirtualIndex>=mMaxNbObjects)			return false;	// Invalid handle
	uword PhysicalIndex = mOutToIn[VirtualIndex];			// Get physical index
	if(PhysicalIndex==0xffff)				return false;	// Object has been deleted
	if(PhysicalIndex>=mMaxNbObjects)		return false;	// Index is invalid
	if(mStamps[VirtualIndex]!=handle>>16)	return false;	// Index has been recycled
	mObjects[PhysicalIndex] = object;						// Updates stored pointer
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Remaps the inner array in an app-friendly order. Of course all handles remain valid.
 *	\param		ranks	[in] remapping table => ranks[i] = index of new object i in old list
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TEMPLATED_MANAGER::Remap(const udword* ranks)
{
	// Checking
	if(!ranks)	return false;

	// Get some bytes
	TEMPLATED_OBJECT* NewList	= (TEMPLATED_OBJECT*)ICE_ALLOC(sizeof(TEMPLATED_OBJECT)*mMaxNbObjects);	CHECKALLOC(NewList);
	uword* NewOTI				= (uword*)ICE_ALLOC(sizeof(uword)*mMaxNbObjects);						CHECKALLOC(NewOTI);
	uword* NewITO				= (uword*)ICE_ALLOC(sizeof(uword)*mMaxNbObjects);						CHECKALLOC(NewITO);
	FillMemory(NewOTI,	mMaxNbObjects*sizeof(uword), 0xff);
	FillMemory(NewITO,	mMaxNbObjects*sizeof(uword), 0xff);
	// Stamps are not modified since they're handle-indexed

	// Rebuild internal lists
	for(udword i=0;i<mCurrentNbObjects;i++)
	{
		// Current physical index in mObjects is i. New index is ranks[i].
		const udword NewIndex = ranks[i];

		// Check 0 <= NewIndex < mCurrentNbObjects
		if(NewIndex>=mCurrentNbObjects)
		{
			ICE_FREE(NewList);
			ICE_FREE(NewOTI);
			ICE_FREE(NewITO);
			return false;
		}

		NewList[i] = mObjects[NewIndex];			// Update new list
		const uword Handle = mInToOut[NewIndex];	// Catch current handle (which is not modified by the remapping process)
		NewITO[i] = Handle;							// Update ITO internal remapping table
		NewOTI[Handle] = ToWord(i);					// Update OTI internal remapping table
	}

	// Fix free indices
	for(udword i=0;i<mNbFreeIndices;i++)
	{
		// Copy the recycled handle (still the same) in the new list
		NewITO[mCurrentNbObjects+i] = mInToOut[mCurrentNbObjects+i];
	}

	// Swap lists

	// Release old data
	ICE_FREE(mInToOut);
	ICE_FREE(mOutToIn);
	ICE_FREE(mObjects);
	// Assign new data
	mObjects	= NewList;
	mOutToIn	= NewOTI;
	mInToOut	= NewITO;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets used ram.
 *	\return		number of bytes used
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
udword TEMPLATED_MANAGER::GetUsedRam() const
{
	udword Ram = sizeof(TEMPLATED_MANAGER);
	Ram += mMaxNbObjects * sizeof(uword);	// mOutToIn
	Ram += mMaxNbObjects * sizeof(uword);	// mInToOut
	Ram += mMaxNbObjects * sizeof(uword);	// mStamps
	Ram += mMaxNbObjects * sizeof(TEMPLATED_OBJECT);	// mObjects
	return Ram;
}

#endif
#endif

#undef TEMPLATED_EXPORT
#undef TEMPLATED_MANAGER
#undef TEMPLATED_OBJECT
