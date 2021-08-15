///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Foundation.h"

#include "PxFoundation.h"
#include "PxPhysicsVersion.h"
#include "foundation/PxErrorCallback.h"
#include "foundation/PxAllocatorCallback.h"
using namespace physx;

static PxFoundation* gFoundation = null;

namespace
{
	class MyAllocatorCallback : public PxAllocatorCallback
	{
		public:

		virtual void* allocate(size_t size, const char* /*typeName*/, const char* /*filename*/, int /*line*/)
		{
//			return _aligned_malloc(size, 16);
			return ICE_ALLOC(size);
		}

		virtual void deallocate(void* ptr)
		{
//			_aligned_free(ptr);
			ICE_FREE(ptr);
		}

	}gMyAllocatorCallback;

	class MyErrorCallback : public PxErrorCallback
	{
		public:

		virtual void reportError(PxErrorCode::Enum /*code*/, const char* message, const char* /*file*/, int /*line*/)
		{
			printf("NV message: %s", message);
		}

	}gMyErrorCallback;
}

void initFoundation()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gMyAllocatorCallback, gMyErrorCallback);
}

void releaseFoundation()
{
	if(gFoundation)
		gFoundation->release();
	gFoundation = null;
}

bool isFoundationInitialized()
{
	return gFoundation!=null;
}

