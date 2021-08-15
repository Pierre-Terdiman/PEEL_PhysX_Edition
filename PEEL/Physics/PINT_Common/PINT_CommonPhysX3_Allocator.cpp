///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_CommonPhysX3_Allocator.h"

///////////////////////////////////////////////////////////////////////////////

PEEL_PhysX3_AllocatorCallback::PEEL_PhysX3_AllocatorCallback() :
	mTotalNbAllocs	(0),
	mNbAllocs		(0),
	mCurrentMemory	(0),
	mLog			(false)
{
}

PEEL_PhysX3_AllocatorCallback::~PEEL_PhysX3_AllocatorCallback()
{
	if(mNbAllocs)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_INTENSITY);
		printf("PhysX: %d leaks found (%d bytes)\n", mNbAllocs, mCurrentMemory);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
	}
}

static int atomicIncrement(volatile int* val)
{
	return (int)InterlockedIncrement((volatile LONG*)val);
}

static int atomicDecrement(volatile int* val)
{
	return (int)InterlockedDecrement((volatile LONG*)val);
}

static int atomicAdd(volatile int* val, int delta)
{
	LONG newValue, oldValue;
	do
	{
		oldValue = *val;
		newValue = oldValue + delta;
	} while(InterlockedCompareExchange((volatile LONG*)val, newValue, oldValue) != oldValue);

	return newValue;
}

PX_COMPILE_TIME_ASSERT(sizeof(PEEL_PhysX3_AllocatorCallback::Header)<=32);
void* PEEL_PhysX3_AllocatorCallback::allocate(size_t size, const char* typeName, const char* filename, int line)
{
	char* memory = (char*)_aligned_malloc(size+32, 16);
	Header* H = (Header*)memory;
	H->mMagic		= 0x12345678;
	H->mSize		= size;
	H->mType		= typeName;
	H->mFilename	= filename;
	H->mLine		= line;

	atomicIncrement((int*)&mTotalNbAllocs);
//	mTotalNbAllocs++;

	atomicIncrement((int*)&mNbAllocs);
//	mNbAllocs++;

	atomicAdd((int*)&mCurrentMemory, size);
//	mCurrentMemory+=size;

	return memory + 32;
}

void PEEL_PhysX3_AllocatorCallback::deallocate(void* ptr)
{
	if(!ptr)
		return;

	char* bptr = (char*)ptr;
	Header* H = (Header*)(bptr - 32);
	ASSERT(H->mMagic==0x12345678);
	const udword Size = H->mSize;
	_aligned_free(H);

	atomicDecrement((int*)&mNbAllocs);
//	mNbAllocs--;

	atomicAdd((int*)&mCurrentMemory, -(int)Size);
//	mCurrentMemory-=Size;
}

