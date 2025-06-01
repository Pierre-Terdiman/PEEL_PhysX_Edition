///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RockInit.h"
#include <Core/RockAllocator.h>

void SetupAllocator(Rock::Allocator* allocator);

namespace
{
	class RockAllocatorWrapper : public Rock::Allocator
	{
		public:
						RockAllocatorWrapper()	{}
		virtual			~RockAllocatorWrapper()	{}

		virtual void*	malloc(size_t size, Rock::MemoryType /*type*/)	override
		{
			//return _aligned_malloc(size, 16);
			return ICE_ALLOC(size);
		}

		virtual void*	mallocDebug(size_t size, const char* filename, u32 line, const char* class_name, Rock::MemoryType /*type*/, bool /*from_new*/)	override
		{
//			return _aligned_malloc(size, 16);
			return ICE_ALLOC(size);
		}

		virtual void	free(void* memory, bool /*from_new*/)	override
		{
//			_aligned_free(memory);
			ICE_FREE(memory);
		}
	};
	static RockAllocatorWrapper gWrapper;
}
void InitRock()
{
	SetupAllocator(&gWrapper);
}

void ReleaseRock()
{
}

