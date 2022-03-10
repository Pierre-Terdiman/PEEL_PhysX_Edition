///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEEL_Threads.h"

#include "PsSync.h"
#include "PsThread.h"
using namespace physx;
using namespace shdfnd;

namespace
{
	struct UtilAllocator
	{
		void*	allocate(size_t size,const char* file, PxU32 line)	{ return _aligned_malloc(size, 16);	}
		void	deallocate(void* ptr)								{ _aligned_free(ptr);				}
	};

	struct Sync : public SyncT<UtilAllocator>
	{
	};

	Sync* syncCreate()				{ return new(_aligned_malloc(sizeof(Sync), 16)) Sync();	}
	void syncWait(Sync* sync)		{ sync->wait();		}
	void syncSet(Sync* sync)		{ sync->set();		}
	void syncReset(Sync* sync)		{ sync->reset();	}
	void syncRelease(Sync* sync)	{ sync->~Sync();	_aligned_free(sync);	}

	typedef void (*ThreadEntryPoint)(void*);

	struct Thread: public ThreadT<UtilAllocator>
	{
		Thread(ThreadEntryPoint entryPoint, void* data): 
			ThreadT<UtilAllocator>(),
			mEntryPoint(entryPoint),
			mData(data)
		{
		}

		virtual void execute(void)											
		{ 
			mEntryPoint(mData);
		}

		ThreadEntryPoint mEntryPoint;
		void* mData;
	};

	Thread* threadCreate(ThreadEntryPoint entryPoint, void* data)
	{
		Thread* createThread = static_cast<Thread*>(_aligned_malloc(sizeof(Thread), 16));
		PX_PLACEMENT_NEW(createThread, Thread(entryPoint, data));
		createThread->start();
		return createThread;
	}

	void threadQuit(Thread* thread)				{ thread->quit();					}
	void threadSignalQuit(Thread* thread)		{ thread->signalQuit();				}
	bool threadWaitForQuit(Thread* thread)		{ return thread->waitForQuit();		}
	bool threadQuitIsSignalled(Thread* thread)	{ return thread->quitIsSignalled();	}
	void threadRelease(Thread* thread)			{ thread->~Thread();	_aligned_free(thread);	}

	struct PEEL_Thread
	{
		inline_	PEEL_Thread() : mWorkReadySyncHandle(null), mThreadHandle(null), mWorkDoneSyncHandle(null), mCallback(null), mUserData(null)
		{
		}

		inline_	void	Run()
		{
			if(mCallback)
				(mCallback)(mUserData);
		}

		Sync*	mWorkReadySyncHandle;
		Thread*	mThreadHandle;
		Sync*	mWorkDoneSyncHandle;

		PEEL_ThreadCallback	mCallback;
		void*				mUserData;
	};
}

static	const PxU32	gNumThreads = 16;
static	PEEL_Thread	gThreads[gNumThreads];

static void threadExecute(void* data)
{
	PEEL_Thread* testThread = static_cast<PEEL_Thread*>(data);

	for(;;)
	{
		::syncWait(testThread->mWorkReadySyncHandle);
		::syncReset(testThread->mWorkReadySyncHandle);

		if(::threadQuitIsSignalled(testThread->mThreadHandle))
			break;

		testThread->Run();
		::syncSet(testThread->mWorkDoneSyncHandle);
	}

	::threadQuit(testThread->mThreadHandle);
}

void PEEL_InitThreads()
{
	for(PxU32 i=0; i<gNumThreads; i++)
	{
		gThreads[i].mWorkReadySyncHandle = ::syncCreate();
		gThreads[i].mWorkDoneSyncHandle = ::syncCreate();
		gThreads[i].mThreadHandle = ::threadCreate(threadExecute, &gThreads[i]);
	}
}

void PEEL_ReleaseThreads()
{
	for(PxU32 i=0; i<gNumThreads; i++)
	{
		::threadSignalQuit(gThreads[i].mThreadHandle);
		::syncSet(gThreads[i].mWorkReadySyncHandle);
	}

	for(PxU32 i=0; i<gNumThreads; i++)
	{
		::threadWaitForQuit(gThreads[i].mThreadHandle);
		::threadRelease(gThreads[i].mThreadHandle);
		::syncRelease(gThreads[i].mWorkReadySyncHandle);
	}

	for(PxU32 i=0; i<gNumThreads; i++)
		::syncRelease(gThreads[i].mWorkDoneSyncHandle);
}

void PEEL_AddThreadWork(udword index, PEEL_ThreadCallback callback, void* user_data)
{
	ASSERT(index<gNumThreads);
	gThreads[index].mCallback = callback;
	gThreads[index].mUserData = user_data;
}

void PEEL_StartThreadWork(udword nb_threads)
{
	ASSERT(nb_threads<=gNumThreads);
	for(udword i=0; i<nb_threads; i++)
		::syncSet(gThreads[i].mWorkReadySyncHandle);
}

void PEEL_EndThreadWork(udword nb_threads)
{
	ASSERT(nb_threads<=gNumThreads);
	for(udword i=0; i<nb_threads; i++)
	{
		::syncWait(gThreads[i].mWorkDoneSyncHandle);
		::syncReset(gThreads[i].mWorkDoneSyncHandle);
	}
}


/*

	if(gUseMultipleThreads)
	{
		PxU32 index = 0;
		for(PxU32 j=0;j<4;j++)
		{
			for(PxU32 i=0;i<4;i++)
			{
				PxU32 offset = (RAYTRACING_RENDER_WIDTH/4)*i;
				offset += (RAYTRACING_RENDER_HEIGHT/4)*j*RAYTRACING_RENDER_WIDTH;
				gThreads[index++].Setup(this, &rp, buffer + offset, camPos, fScreenWidth, fScreenHeight, i, j);
			}
		}

		{
			for (PxU32 i=0; i<gNumThreads; i++)
				::syncSet(gThreads[i].mWorkReadySyncHandle);

			for (PxU32 i=0; i<gNumThreads; i++)
			{
				::syncWait(gThreads[i].mWorkDoneSyncHandle);
				::syncReset(gThreads[i].mWorkDoneSyncHandle);
			}
		}
	}

*/


