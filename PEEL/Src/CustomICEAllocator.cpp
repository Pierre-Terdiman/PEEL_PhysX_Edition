///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomICEAllocator.h"

static inline_ void _IceTrace(const char* buffer)
{
/*	va_list args;
	va_start(args, lpszFormat);

	sdword nBuf;
	ubyte szBuffer[512];

	nBuf = _vsnprintf((LPSTR)szBuffer, lstrlen((LPCSTR)szBuffer), (LPCSTR)lpszFormat, args);

	_RPT0(_CRT_WARN, szBuffer);

	va_end(args);*/
	if(buffer)
	{
		_RPT0(_CRT_WARN, buffer);
		printf("%s\n", buffer);
	}
}


//#define ZERO_OVERHEAD_RELEASE
#define NEW_CODE
// For some reason dmalloc seems a lot slower than the system malloc?
//#define USE_DMALLOC
#define SIMD_ALLOC

#ifdef USE_DMALLOC
	#include "dmalloc.h"
	#define	LOCAL_MALLOC	dlmalloc
	#define	LOCAL_FREE		dlfree
#else
	#define	LOCAL_MALLOC	::malloc
	#define	LOCAL_FREE		::free
#endif


// WARNING: this makes allocations a lot slower. Only use when tracking memory leaks.
//#define ALLOC_STRINGS

// ### doesn't seem that useful
//#define	FAST_BUFFER_SIZE	256*1024

#define	DEBUG_IDENTIFIER	0xBeefBabe
#define	DEBUG_DEALLOCATED	0xDeadDead

#ifdef ALLOC_STRINGS
static const char* AllocString(const char* str)
{
	GUARD("AllocString")
	if(!str)	return null;
	char* mem = (char*)LOCAL_MALLOC(strlen(str)+1);
	strcpy(mem, str);
	return mem;
}

static void FreeString(const char* str)
{
	GUARD("FreeString")
	if(str)	LOCAL_FREE((void*)str);
}

#endif

	class DefaultAllocator : public Allocator
	{
		public:
										DefaultAllocator();
		virtual							~DefaultAllocator();

							void		reset();

		override(Allocator)	void*		malloc(size_t size, MemoryType type);
		override(Allocator)	void*		mallocDebug(size_t size, const char* filename, udword line, const char* class_name, MemoryType type, bool from_new);
		override(Allocator)	void*		realloc(void* memory, size_t size);
		override(Allocator)	void*		shrink(void* memory, size_t size);
		override(Allocator)	void		free(void* memory, bool from_new);

		override(Allocator)	void		FrameAnalysis();

							void		DumpCurrentMemoryState() const;
							void		Release();
		private:
							void**		mMemBlockList;
							udword		mMemBlockListSize;
#ifdef NEW_CODE
							udword		mFirstFree;
#else
							udword		mMemBlockFirstFree;
#endif
							udword		mMemBlockUsed;

							size_t		mNbAllocatedBytes;
							size_t		mHighWaterMark;
							sdword		mTotalNbAllocs;
							sdword		mNbAllocs;
							sdword		mNbReallocs;
#ifdef FAST_BUFFER_SIZE
							udword		mNbFastBytes;
							udword		mFastBufferOffset;
							ubyte*		mFastBuffer;
#endif
							sdword		mPreviousTotalNbAllocs;
							sdword		mPreviousNbAllocs;
							udword		mFrameCount;
							bool		mNoLeak;
	};

#define MEMBLOCKSTART		64

	struct DebugBlock
	{
		udword			mCheckValue;
#ifdef FAST_BUFFER_SIZE
		MemoryType		mType;
#endif
		udword			mSize;
		const char*		mFilename;
		udword			mLine;
		udword			mSlotIndex;
		const char*		mClassName;
		BOOL			mFromNew;
	};

#ifndef FAST_BUFFER_SIZE
//	ICE_COMPILE_TIME_ASSERT(sizeof(DebugBlock)==28);	// Prevents surprises.....
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DefaultAllocator::DefaultAllocator() : mNbAllocatedBytes(0), mHighWaterMark(0), mTotalNbAllocs(0), mNbAllocs(0), mNbReallocs(0)
{
	GUARD("DefaultAllocator::DefaultAllocator()")

	mMemBlockList = null;

#ifdef _DEBUG
	// Initialize the Memory blocks list (DEBUG mode only)
	mMemBlockList = (void**)LOCAL_MALLOC(MEMBLOCKSTART*sizeof(void*));
	ZeroMemory(mMemBlockList, MEMBLOCKSTART*sizeof(void*));
	mMemBlockListSize	= MEMBLOCKSTART;
#ifdef NEW_CODE
	mFirstFree			= INVALID_ID;
#else
	mMemBlockFirstFree	= 0;
#endif
	mMemBlockUsed		= 0;
#endif

#ifdef FAST_BUFFER_SIZE
	mNbFastBytes		= 0;
	mFastBufferOffset	= 0;
	mFastBuffer			= (ubyte*)LOCAL_MALLOC(FAST_BUFFER_SIZE);
#endif

	mPreviousTotalNbAllocs	= 0;
	mPreviousNbAllocs		= 0;
	mFrameCount				= 0;
	mNoLeak					= true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DefaultAllocator::~DefaultAllocator()
{
	Release();
}

void DefaultAllocator::Release()
{
	GUARD("DefaultAllocator::~DefaultAllocator()")

#ifdef FAST_BUFFER_SIZE
	mNbFastBytes = 0;
	mFastBufferOffset = 0;
	if(mFastBuffer)	LOCAL_FREE(mFastBuffer);
	mFastBuffer = null;
#endif

#ifdef _DEBUG

	// Ok, it is a bad idea to use _F() here, because it internally uses the allocator (for the log string). So let's use good old C style here.
	char Buffer[4096];

	if(mNbAllocatedBytes)
	{
		sprintf(Buffer, "Memory leak detected: %d bytes non released\n", mNbAllocatedBytes);
		_IceTrace(Buffer);
//		_IceTrace(_F("Memory leak detected: %d bytes non released\n", mNbAllocatedBytes));
	}
	if(mNbAllocs)
	{
		sprintf(Buffer, "Remaining allocs: %d\n", mNbAllocs);
		_IceTrace(Buffer);
//		_IceTrace(_F("Remaining allocs: %d\n", mNbAllocs));
	}
//	_IceTrace(_F("Nb alloc: %d\n", mTotalNbAllocs));
	sprintf(Buffer, "Total nb alloc: %d\n", mTotalNbAllocs);
	_IceTrace(Buffer);

//	_IceTrace(_F("Nb realloc: %d\n", mNbReallocs));
	sprintf(Buffer, "Nb realloc: %d\n", mNbReallocs);
	_IceTrace(Buffer);

//	_IceTrace(_F("High water mark: %d Kb\n", mHighWaterMark/1024));
	sprintf(Buffer, "High water mark: %d Kb\n", mHighWaterMark/1024);
	_IceTrace(Buffer);

	// Scanning for memory leaks
	if(mMemBlockList && mNbAllocs)
	{
		udword NbLeaks = 0;
		_IceTrace("\n\n  ICE Message Memory leaks detected :\n\n");

#ifdef NEW_CODE
		for(udword i=0; i<mMemBlockUsed; i++)
		{
			if(size_t(mMemBlockList[i])&1)
				continue;

			const DebugBlock* DB = (const DebugBlock*)mMemBlockList[i];
//			_IceTrace(_F(" Address 0x%.8X, %d bytes (%s), allocated in: %s(%d):\n\n", cur+6, cur[1], (const char*)cur[5], (const char*)cur[2], cur[3]));
//			_IceTrace(_F(" Address 0x%.8X, %d bytes (%s), allocated in: %s(%d):\n\n", DB+1, DB->mSize, DB->mClassName, DB->mFilename, DB->mLine));
			sprintf(Buffer, " Address 0x%.8X, %d bytes (%s), allocated in: %s(%d):\n\n", DB+1, DB->mSize, DB->mClassName, DB->mFilename, DB->mLine);
			_IceTrace(Buffer);

			NbLeaks++;
//			Free(cur+4);
		}
#else
		for(udword i=0, j=0; i<mMemBlockUsed; i++, j++)
		{
			// Skip empty slots
			while(!mMemBlockList[j]) j++;

			const DebugBlock* DB = (const DebugBlock*)mMemBlockList[j];
//			_IceTrace(_F(" Address 0x%.8X, %d bytes (%s), allocated in: %s(%d):\n\n", cur+6, cur[1], (const char*)cur[5], (const char*)cur[2], cur[3]));
//			_IceTrace(_F(" Address 0x%.8X, %d bytes (%s), allocated in: %s(%d):\n\n", DB+1, DB->mSize, DB->mClassName, DB->mFilename, DB->mLine));
			sprintf(Buffer, " Address 0x%.8X, %d bytes (%s), allocated in: %s(%d):\n\n", DB+1, DB->mSize, DB->mClassName, DB->mFilename, DB->mLine);
			_IceTrace(Buffer);

			NbLeaks++;
//			Free(cur+4);
		}
#endif
//		_IceTrace(_F("\n  Dump complete (%d leaks)\n\n", NbLeaks));
		sprintf(Buffer, "\n  Dump complete (%d leaks)\n\n", NbLeaks);
		_IceTrace(Buffer);
	}
	// Free the Memory Block list
	if(mMemBlockList)	LOCAL_FREE(mMemBlockList);
	mMemBlockList = null;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DefaultAllocator::reset()
{
	GUARD("DefaultAllocator::reset")

	mNbAllocatedBytes	= 0;
	mHighWaterMark		= 0;
	mNbAllocs			= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* DefaultAllocator::malloc(size_t size, MemoryType type)
{
	GUARD("DefaultAllocator::malloc")

//	return ::malloc(size);

#ifdef _DEBUG
	return mallocDebug(size, null, 0, "Undefined", type, false);
#endif

	if(!size)
	{
#ifdef _DEBUG
		_IceTrace("Warning: trying to allocate 0 bytes\n");
#endif
		return null;
	}

	mTotalNbAllocs++;
	mNbAllocs++;

	mNbAllocatedBytes+=size;
	if(mNbAllocatedBytes>mHighWaterMark)	mHighWaterMark = mNbAllocatedBytes;

#ifdef ZERO_OVERHEAD_RELEASE
	return LOCAL_MALLOC(size);
#else
	#ifdef SIMD_ALLOC
	void* ptr = _aligned_malloc(size+16, 16);
	#else
	void* ptr = (void*)LOCAL_MALLOC(size+8);
	#endif
	udword* blockStart = (udword*)ptr;
	blockStart[0] = DEBUG_IDENTIFIER;
	blockStart[1] = udword(size);
	#ifdef SIMD_ALLOC
	return ((udword*)ptr)+4;
	#else
	return ((udword*)ptr)+2;
	#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* DefaultAllocator::mallocDebug(size_t size, const char* filename, udword line, const char* class_name, MemoryType type, bool from_new)
{
	GUARD("DefaultAllocator::mallocDebug")

//	printf("%s (%d)\n", filename, line);

#ifdef _DEBUG
	if(!size)
	{
		_IceTrace("Warning: trying to allocate 0 bytes\n");
		return null;
	}

	// Catch improper use of alloc macro...
	if(0 && class_name)
	{
		const char* c = class_name;
		while(*c)
		{
			if(*c==']' || *c=='[')
			{
				int stop=0;
			}
			c++;
		}
	}

	// Make sure size is even
	if(size&1)	size++;

#ifdef FAST_BUFFER_SIZE
	// Allocate one debug block in front of each real allocation
	void* ptr = null;
	if(type==MEMORY_TEMP)
	{
		udword NeededSize = size + sizeof(DebugBlock);
		if(mFastBufferOffset + NeededSize <= FAST_BUFFER_SIZE)
		{
			ptr = mFastBuffer + mFastBufferOffset;
			mFastBufferOffset += NeededSize;
			mNbFastBytes += NeededSize;
		}
	}

	if(!ptr)
	{
		ptr = (void*)LOCAL_MALLOC(size + sizeof(DebugBlock));
		type = MEMORY_PERSISTENT;
	}
#else
	// Allocate one debug block in front of each real allocation
	void* ptr = (void*)LOCAL_MALLOC(size + sizeof(DebugBlock));
#endif
	ASSERT(IS_ALIGNED_2(size_t(ptr)));

	// Fill debug block
	DebugBlock* DB = (DebugBlock*)ptr;
	DB->mCheckValue	= DEBUG_IDENTIFIER;
	DB->mFromNew	= from_new;
#ifdef FAST_BUFFER_SIZE
	DB->mType		= type;
#endif
	DB->mSize		= udword(size);
	DB->mLine		= line;
	DB->mSlotIndex	= INVALID_ID;
#ifdef ALLOC_STRINGS
	DB->mFilename	= AllocString(filename);
	DB->mClassName	= AllocString(class_name);
#else
	DB->mFilename	= filename;
	DB->mClassName	= class_name;
#endif

	// Update global stats
	mTotalNbAllocs++;
	mNbAllocs++;
	mNbAllocatedBytes += size;
	if(mNbAllocatedBytes>mHighWaterMark)
		mHighWaterMark = mNbAllocatedBytes;

	// Insert the allocated block in the debug memory block list
	if(mMemBlockList)
	{
#ifdef NEW_CODE
		if(mFirstFree!=INVALID_ID)
		{
			// Recycle old location

			udword NextFree = udword(mMemBlockList[mFirstFree]);
			if(NextFree!=INVALID_ID)	NextFree>>=1;

			mMemBlockList[mFirstFree] = ptr;
			DB->mSlotIndex = mFirstFree;

			mFirstFree = NextFree;
		}
		else
		{
			if(mMemBlockUsed==mMemBlockListSize)
			{
				// Allocate a bigger block
				void** tps = (void**)LOCAL_MALLOC((mMemBlockListSize+MEMBLOCKSTART)*sizeof(void*));
				// Copy already used part
				CopyMemory(tps, mMemBlockList, mMemBlockListSize*sizeof(void*));
				// Initialize remaining part
				void* Next = tps + mMemBlockListSize;
				ZeroMemory(Next, MEMBLOCKSTART*sizeof(void*));

				// Free previous memory, setup new pointer
				LOCAL_FREE(mMemBlockList);
				mMemBlockList = tps;
				// Setup new size
				mMemBlockListSize += MEMBLOCKSTART;
			}

			mMemBlockList[mMemBlockUsed] = ptr;
			DB->mSlotIndex = mMemBlockUsed++;
		}
#else
		// Store allocated pointer in first free slot
		mMemBlockList[mMemBlockFirstFree] = ptr;
		DB->mSlotIndex = mMemBlockFirstFree;

		// Count number of used slots
		mMemBlockUsed++;

		// Resize if needed
		if(mMemBlockUsed==mMemBlockListSize)
		{
			// Allocate a bigger block
			void** tps = (void**)LOCAL_MALLOC((mMemBlockListSize+MEMBLOCKSTART)*sizeof(void*));
			// Copy already used part
			CopyMemory(tps, mMemBlockList, mMemBlockListSize*sizeof(void*));
			// Initialize remaining part
			void* Next = tps + mMemBlockListSize;
			ZeroMemory(Next, MEMBLOCKSTART*sizeof(void*));

			// Free previous memory, setup new pointer
			LOCAL_FREE(mMemBlockList);
			mMemBlockList = tps;
			// -1 because we'll do a ++ just afterwards
			mMemBlockFirstFree = mMemBlockListSize-1;
			// Setup new size
			mMemBlockListSize += MEMBLOCKSTART;
		}

		// Look for first free ### recode this ugly thing
		while(mMemBlockList[++mMemBlockFirstFree] && (mMemBlockFirstFree<mMemBlockListSize));
		if(mMemBlockFirstFree==mMemBlockListSize)
		{
			mMemBlockFirstFree = (udword)-1;
			while(mMemBlockList[++mMemBlockFirstFree] && (mMemBlockFirstFree<mMemBlockListSize));
		}
#endif
	}

	return ((ubyte*)ptr) + sizeof(DebugBlock);
#else
	Log("Error: mallocDebug has been called in release!\n");
	ASSERT(0);//Don't use debug malloc for release mode code!
	return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* DefaultAllocator::shrink(void* memory, size_t size)
{
	GUARD("DefaultAllocator::shrink")

	return null;	// #ifdef ZERO_OVERHEAD_RELEASE

	if(!memory)	return null;
#ifdef _DEBUG
	// Debug codepath

	ubyte* SystemPointer = ((ubyte*)memory) - sizeof(DebugBlock);

	DebugBlock* DB = (DebugBlock*)SystemPointer;
	if(DB->mCheckValue!=DEBUG_IDENTIFIER)
	{
		// Not a valid memory block
		return null;
	}
	if(size>DB->mSize)
	{
		// New size should be smaller!
		return null;
	}

	//  Try to shrink the block
	void* Reduced = _expand(SystemPointer, size + sizeof(DebugBlock));
	if(!Reduced)	return null;

	if(Reduced!=SystemPointer)
	{
		// Should not be possible?!
	}

	// Update stats
	mNbAllocatedBytes -= DB->mSize;
	mNbAllocatedBytes += size;
	// Setup new size
	DB->mSize = udword(size);

	return memory;	// The pointer should not have changed!
#else
	// Release codepath
	udword* SystemPointer = ((udword*)memory)-2;
	if(SystemPointer[0]!=DEBUG_IDENTIFIER)
	{
		// Not a valid memory block
		return null;
	}
	if(size>SystemPointer[1])
	{
		// New size should be smaller!
		return null;
	}

	//  Try to shrink the block
	void* Reduced = _expand(SystemPointer, size+8);
	if(!Reduced)	return null;

	if(Reduced!=SystemPointer)
	{
		// Should not be possible?!
	}

	// Update stats
	mNbAllocatedBytes -= SystemPointer[1];
	mNbAllocatedBytes += size;
	// Setup new size
	SystemPointer[1] = size;

	return memory;	// The pointer should not have changed!
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* DefaultAllocator::realloc(void* memory, size_t size)
{
	GUARD("DefaultAllocator::realloc")

//	return ::realloc(memory, size);

	ASSERT(0);
	return null;

/*

	if(!memory)
	{
#ifdef _DEBUG
		_IceTrace("Warning: trying to realloc null pointer\n");
#endif
		return null;
	}

	if(!size)
	{
#ifdef _DEBUG
		_IceTrace("Warning: trying to realloc 0 bytes\n");
#endif
	}

#ifdef _DEBUG

	udword* ptr = ((udword*)memory)-6;
	if(ptr[0]!=DEBUG_IDENTIFIER)
	{
		_IceTrace("Error: realloc unknown memory!!\n");
	}
	mNbAllocatedBytes -= ptr[1];
	mNbAllocatedBytes += size;

	if(mNbAllocatedBytes>mHighWaterMark)	mHighWaterMark = mNbAllocatedBytes;

	void* ptr2 = ::realloc(ptr, size+24);
	mNbReallocs++;
	*(((udword*)ptr2)+1) = size;
	if(ptr==ptr2)
	{
		return memory;
	}

	*(((udword*)ptr2)) = DEBUG_IDENTIFIER;
	*(((udword*)ptr2)+1) = size;
	*(((udword*)ptr2)+2) = 0;	// File
	*(((udword*)ptr2)+3) = 0;	// Line

	udword* blockStart = (udword*)ptr2;

	// Insert the allocated block in the debug memory block list
	if(mMemBlockList)
	{
		mMemBlockList[mMemBlockFirstFree] = ptr;
		blockStart[4] = mMemBlockFirstFree;
		mMemBlockUsed++;
		if(mMemBlockUsed==mMemBlockListSize)
		{
			void** tps = (void**)::malloc((mMemBlockListSize+MEMBLOCKSTART)*sizeof(void*));
			CopyMemory(tps, mMemBlockList, mMemBlockListSize*sizeof(void*));
			void* Next = tps + mMemBlockListSize;
			ZeroMemory(Next, MEMBLOCKSTART*sizeof(void*));

			::free(mMemBlockList);
			mMemBlockList = tps;
			mMemBlockFirstFree = mMemBlockListSize-1;
			mMemBlockListSize += MEMBLOCKSTART;
		}
		while (mMemBlockList[++mMemBlockFirstFree] && (mMemBlockFirstFree<mMemBlockListSize));
		if(mMemBlockFirstFree==mMemBlockListSize)
		{
			mMemBlockFirstFree = (udword)-1;
			while(mMemBlockList[++mMemBlockFirstFree] && (mMemBlockFirstFree<mMemBlockListSize));
		}
	}
	else
	{
		blockStart[4] = 0;
	}
	blockStart[5] = 0;	// Classname

	return ((udword*)ptr2)+6;
#else
	udword* ptr = ((udword*)memory)-2;
	if(ptr[0]!=DEBUG_IDENTIFIER)
	{
#ifdef _DEBUG
		_IceTrace("Error: realloc unknown memory!!\n");
#endif
	}
	mNbAllocatedBytes -= ptr[1];
	mNbAllocatedBytes += size;

	if(mNbAllocatedBytes>mHighWaterMark)	mHighWaterMark = mNbAllocatedBytes;

	void* ptr2 = ::realloc(ptr, size+8);
	mNbReallocs++;
	*(((udword*)ptr2)+1) = size;
	if(ptr==ptr2)	return memory;

	*(((udword*)ptr2)) = DEBUG_IDENTIFIER;
	return ((udword*)ptr2)+2;
#endif
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DefaultAllocator::free(void* memory, bool from_new)
{
	GUARD("DefaultAllocator::free")

	if(!memory)
	{
#ifdef _DEBUG
		_IceTrace("Warning: trying to free null pointer\n");
#endif
		return;
	}

#ifdef _DEBUG
	DebugBlock* DB = ((DebugBlock*)memory)-1;

//	DebugBlock TmpDB = *DB;	// Keep a local copy to have readable data when ::free() fails!

	// Check we allocated it
	if(DB->mCheckValue!=DEBUG_IDENTIFIER)
	{
		_IceTrace("Error: free unknown memory!!\n");
		// ### should we really continue??
		return;
	}

	ASSERT(udword(from_new)==DB->mFromNew);

	// Update global stats
	mNbAllocatedBytes -= DB->mSize;
	mNbAllocs--;

#ifdef NEW_CODE
	// Remove the block from the Memory block list
	if(mMemBlockList)
	{
		udword FreeSlot = DB->mSlotIndex;
		ASSERT(mMemBlockList[FreeSlot]==DB);

		udword NextFree = mFirstFree;
		if(NextFree!=INVALID_ID)
		{
			NextFree<<=1;
			NextFree|=1;
		}

		mMemBlockList[FreeSlot] = (void*)NextFree;
		mFirstFree = FreeSlot;
	}
#else
	udword MemBlockFirstFree = DB->mSlotIndex;	// The slot we are in
	udword Line = DB->mLine;
	const char* File = DB->mFilename;

	// Remove the block from the Memory block list
	if(mMemBlockList)
	{
		ASSERT(mMemBlockList[MemBlockFirstFree]==DB);
		mMemBlockList[MemBlockFirstFree] = null;
		mMemBlockUsed--;
	}
#endif

#ifdef ALLOC_STRINGS
	FreeString(DB->mClassName);
	FreeString(DB->mFilename);
#endif

#ifdef FAST_BUFFER_SIZE
	if(DB->mType==MEMORY_TEMP)
	{
		mNbFastBytes -= DB->mSize + sizeof(DebugBlock);
		if(mNbFastBytes==0)
		{
			mFastBufferOffset = 0;
		}
		return;
	}
#endif

	// ### should be useless since we'll release the memory just afterwards
	DB->mCheckValue = DEBUG_DEALLOCATED;
	DB->mSize		= 0;
	DB->mClassName	= null;
	DB->mFilename	= null;
	DB->mSlotIndex	= INVALID_ID;
	DB->mLine		= INVALID_ID;

	LOCAL_FREE(DB);
#else
	// Release codepath
	#ifdef ZERO_OVERHEAD_RELEASE

//	mNbAllocatedBytes -= ptr[1];	// ### use _msize() ?
	mNbAllocs--;
	LOCAL_FREE(memory);

	#else

	#ifdef SIMD_ALLOC
	udword* ptr = ((udword*)memory)-4;
	#else
	udword* ptr = ((udword*)memory)-2;
	#endif

	if(ptr[0]!=DEBUG_IDENTIFIER)
	{
	#ifdef _DEBUG
		_IceTrace("Error: free unknown memory!!\n");
	#endif
	}
	mNbAllocatedBytes -= ptr[1];
	if(mNbAllocatedBytes<0)
	{
	#ifdef _DEBUG
		_IceTrace(_F("Oops (%d)\n", ptr[1]));
	#endif
	}
	mNbAllocs--;
	ptr[0]=DEBUG_DEALLOCATED;
	ptr[1]=0;

	#ifdef SIMD_ALLOC
	_aligned_free(ptr);
	#else
	LOCAL_FREE(ptr);
	#endif

	#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DefaultAllocator::FrameAnalysis()
{
	const sdword NbAllocsPerFrame = mTotalNbAllocs - mPreviousTotalNbAllocs;
	(void)NbAllocsPerFrame;
//	printf("%d allocs/frame\n", NbAllocsPerFrame);

	if(mNbAllocs<=mPreviousNbAllocs)
		mNoLeak = true;

	mFrameCount++;
	if(mFrameCount==200)
	{
		if(!mNoLeak)
		{
//			IceCore::MessageBox(0, "Memory leak detected!", "Oops", MB_OK);
		}
		mFrameCount=0;
		mNoLeak = false;
	}

	mPreviousTotalNbAllocs = mTotalNbAllocs;
	mPreviousNbAllocs = mNbAllocs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline_ bool ValidAddress(const void* addy)
{
#ifdef NEW_CODE
	return (addy && !(size_t(addy)&1));
#else
	return addy!=null;
#endif
}

void DefaultAllocator::DumpCurrentMemoryState() const
{
	GUARD("DefaultAllocator::DumpCurrentMemoryState")

#ifdef _DEBUG
	// Scanning for memory leaks
	if(mMemBlockList && mMemBlockUsed)
	{
		_IceTrace("\n\n----ALLOCATOR MEMORY DUMP:\n\n");

		// We can't just use the "const char*" stored in the debug blocks because they're not guaranteed to
		// be unique for similar strings. For example if a Container is allocated from two different DLLs,
		// the "Container" character string will be duplicated (one per DLL). So we need to group similar
		// strings together using the actual characters, not just the string address. We also have to do this
		// without allocating any new memory, since it would add new entries to the memory debug structure.
		//
		// The good news is that we don't care about speed too much in this function, since it's not supposed
		// to be called all the time.

		struct Local
		{
			struct TmpStruct
			{
				const char*	mName;
				udword		mSize;
			};
			static int SortCB(const void* elem1, const void* elem2)
			{
				const TmpStruct* s1 = (const TmpStruct*)elem1;
				const TmpStruct* s2 = (const TmpStruct*)elem2;
				return strcmp(s1->mName, s2->mName);
			}
		};

		Local::TmpStruct* SortedStrings = (Local::TmpStruct*)LOCAL_MALLOC(sizeof(Local::TmpStruct)*mMemBlockListSize);
		udword NbStrings = 0;
		udword TotalSize = 0;
		for(udword i=0;i<mMemBlockListSize;i++)
		{
			if(ValidAddress(mMemBlockList[i]))
			{
				const DebugBlock* DB = (const DebugBlock*)mMemBlockList[i];
				if(DB->mClassName)
				{
					SortedStrings[NbStrings].mName = DB->mClassName;	// Memory by class
//					SortedStrings[NbStrings].mName = DB->mFilename;		// Memory by file
					SortedStrings[NbStrings].mSize = DB->mSize;
					TotalSize += DB->mSize;
					NbStrings++;
				}
			}
		}
		qsort(SortedStrings, NbStrings, sizeof(Local::TmpStruct), Local::SortCB);

		// Strings are now sorted. They might still be duplicated, i.e. we may have two strings for the same
		// class. So now we parse the list and collect used memory for all classes. Then we sort this again,
		// to report results in order of increasing memory.

		udword NbClasses=0;
		udword* Classes = (udword*)LOCAL_MALLOC(sizeof(udword)*NbStrings);
		udword* Sizes = (udword*)LOCAL_MALLOC(sizeof(udword)*NbStrings);

		udword CurrentSize = SortedStrings[0].mSize;
		const char* CurrentClass = SortedStrings[0].mName;
		for(udword i=1;i<=NbStrings;i++)	// One more time on purpose
		{
			const char* Current = null;
			if(i!=NbStrings)
			{
				Current = SortedStrings[i].mName;
			}

			if(Current && strcmp(Current, CurrentClass)==0)
			{
				// Same class
				CurrentSize += SortedStrings[i].mSize;
			}
			else
			{
				// New class

				// Store previous class
				if(CurrentClass)
				{
					Classes[NbClasses] = (udword)CurrentClass;	// We can store this pointer now because it's unique in our new array
					Sizes[NbClasses++] = CurrentSize;
				}

				// Next one
				if(Current)
				{
					CurrentClass = Current;
					CurrentSize = SortedStrings[i].mSize;
				}
			}
		}

		udword* Ranks0 = (udword*)LOCAL_MALLOC(sizeof(udword)*NbClasses);
		udword* Ranks1 = (udword*)LOCAL_MALLOC(sizeof(udword)*NbClasses);

		StackRadixSort(RS, Ranks0, Ranks1);
		const udword* Sorted = RS.Sort(Sizes, NbClasses).GetRanks();
		for(udword i=0;i<NbClasses;i++)
		{
			udword Index = Sorted[i];
			char Buffer[4096];
			sprintf(Buffer, "%s : %d\n", (const char*)Classes[Index], Sizes[Index]);
			_IceTrace(Buffer);
		}
		char Buffer[4096];
		sprintf(Buffer, "Total size: %d\n", TotalSize);
		_IceTrace(Buffer);

		LOCAL_FREE(Ranks1);
		LOCAL_FREE(Ranks0);

		LOCAL_FREE(Sizes);
		LOCAL_FREE(Classes);

		LOCAL_FREE(SortedStrings);



/*
		udword* Ranks0 = (udword*)::malloc(sizeof(udword)*mMemBlockListSize);
		udword* Ranks1 = (udword*)::malloc(sizeof(udword)*mMemBlockListSize);
		udword* Keys = (udword*)::malloc(sizeof(udword)*mMemBlockListSize);

		udword NbClasses=0;
		udword* Classes = (udword*)::malloc(sizeof(udword)*mMemBlockListSize);
		udword* Sizes = (udword*)::malloc(sizeof(udword)*mMemBlockListSize);

		for(udword i=0;i<mMemBlockListSize;i++)
		{
			if(ValidAddress(mMemBlockList[i]))
			{
				const DebugBlock* DB = (const DebugBlock*)mMemBlockList[i];
				Keys[i] = (udword)DB->mClassName;
			}
			else Keys[i] = 0;
		}

		StackRadixSort(RS, Ranks0, Ranks1);
		const udword* Sorted = RS.Sort(Keys, mMemBlockListSize).GetRanks();

		const DebugBlock* DB0 = (const DebugBlock*)mMemBlockList[Sorted[0]];
		udword Previous = 0;
		udword CurrentSize = 0;
		const char* CurrentClass = null;
		if(ValidAddress(DB0))
		{
			Previous = (udword)DB0->mClassName;
			CurrentSize = DB0->mSize;
			CurrentClass = DB0->mClassName;
		}
		for(udword i=1;i<=mMemBlockListSize;i++)
		{
			udword Current;
			const DebugBlock* DB;
			if(i==mMemBlockListSize)
			{
				DB = null;
				Current = INVALID_ID;
			}
			else
			{
				DB = (const DebugBlock*)mMemBlockList[Sorted[i]];
#ifdef NEW_CODE
				if(!ValidAddress(DB))	DB = null;
#endif
				Current = DB ? (udword)DB->mClassName : null;
			}

			if(Current!=Previous)
			{
				if(CurrentClass)
				{
//					char Buffer[4096];
//					sprintf(Buffer, "%s - %d\n", CurrentClass, CurrentSize);
//					_IceTrace(Buffer);
					Classes[NbClasses] = (udword)CurrentClass;
					Sizes[NbClasses++] = CurrentSize;
				}

				// Next one
				Previous = Current;
				if(DB)
				{
					CurrentSize = DB->mSize;
					CurrentClass = DB->mClassName;
				}
			}
			else CurrentSize += DB ? DB->mSize : 0;
		}

		StackRadixSort(RS2, Ranks0, Ranks1);
		Sorted = RS2.Sort(Sizes, NbClasses).GetRanks();
		for(udword i=0;i<NbClasses;i++)
		{
			udword Index = Sorted[i];
			char Buffer[4096];
			sprintf(Buffer, "%s - %d\n", (const char*)Classes[Index], Sizes[Index]);
			_IceTrace(Buffer);
		}


		::free(Sizes);
		::free(Classes);

		::free(Keys);
		::free(Ranks1);
		::free(Ranks0);*/
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Allocator* CreatePEELDefaultAllocator()
{
	return ::new DefaultAllocator;
}
