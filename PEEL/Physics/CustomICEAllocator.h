///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef CUSTOM_ICE_ALLOCATOR_H
#define CUSTOM_ICE_ALLOCATOR_H

#include "Common.h"

	Allocator* CreatePEELDefaultAllocator();

	class CustomIceAllocator : public Allocator
	{
		public:
		CustomIceAllocator() : mCurrentNbAllocs(0)
		{
		}
		virtual ~CustomIceAllocator()
		{
			if(mCurrentNbAllocs)
				OutputConsoleError(_F("MyIceAllocator: %d memory leaks detected...\n", mCurrentNbAllocs));
			else
				printf("MyIceAllocator: no memory leaks detected.\n");
		}
		virtual void*	malloc(size_t size, MemoryType type)
		{
			mCurrentNbAllocs++;
//			return ::malloc(size);
			return _aligned_malloc(size, 16);
		}
		virtual void*	mallocDebug(size_t size, const char* filename, udword line, const char* class_name, MemoryType type, bool from_new)
		{
			mCurrentNbAllocs++;
//			return ::malloc(size);
			return _aligned_malloc(size, 16);
		}
		virtual void*	realloc(void* memory, size_t size)
		{
			return ::realloc(memory, size);
		}
		virtual void*	shrink(void* memory, size_t size)
		{
			return memory;
		}
		virtual void	free(void* memory, bool from_new)
		{
			mCurrentNbAllocs--;
//			::free(memory);
			_aligned_free(memory);
		}
		udword	mCurrentNbAllocs;
	};

#endif
