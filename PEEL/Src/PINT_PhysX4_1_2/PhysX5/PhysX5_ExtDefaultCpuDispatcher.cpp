// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2021 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "PhysX5_ExtDefaultCpuDispatcher.h"
#include "PhysX5_ExtCpuWorkerThread.h"
#include "extensions/ExtTaskQueueHelper.h"
#include "PsString.h"

using namespace physx;
using namespace physx5;

#define PX_ALLOCATE(type, count, name)	reinterpret_cast<type*>(PX_ALLOC(count*sizeof(type), name))

physx5::PxDefaultCpuDispatcher* physx5::PxDefaultCpuDispatcherCreate(PxU32 numThreads, PxU32* affinityMasks, PxDefaultCpuDispatcherWaitForWorkMode::Enum mode, PxU32 yieldProcessorCount)
{
	return PX_NEW(Ext::DefaultCpuDispatcher)(numThreads, affinityMasks, mode, yieldProcessorCount);
}

#if !PX_PS4 && !PX_XBOXONE && !PX_SWITCH && !PX_XBOX_SERIES_X
void physx5::Ext::DefaultCpuDispatcher::getAffinityMasks(PxU32* affinityMasks, PxU32 threadCount)
{
	for(PxU32 i=0; i < threadCount; i++)
	{
		affinityMasks[i] = 0;
	}
}
#endif

physx5::Ext::DefaultCpuDispatcher::DefaultCpuDispatcher(PxU32 numThreads, PxU32* affinityMasks, PxDefaultCpuDispatcherWaitForWorkMode::Enum mode, PxU32 yieldProcessorCount)
	: mQueueEntryPool(EXT_TASK_QUEUE_ENTRY_POOL_SIZE, "QueueEntryPool"), mNumThreads(numThreads), mShuttingDown(false)
#if PX_PROFILE
	,mRunProfiled(true)
#else
	,mRunProfiled(false)
#endif
	, mWaitForWorkMode(mode)
	, mYieldProcessorCount(yieldProcessorCount)
{
	PX_CHECK_MSG((((PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_PROCESSOR == mWaitForWorkMode) && (mYieldProcessorCount > 0)) ||
					(((PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_THREAD == mWaitForWorkMode) || (PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK == mWaitForWorkMode)) && (0 == mYieldProcessorCount))), "Illegal yield processor count for chosen execute mode");

	PxU32* defaultAffinityMasks = NULL;

	if(!affinityMasks)
	{
		defaultAffinityMasks = PX_ALLOCATE(PxU32, numThreads, "ThreadAffinityMasks");
		getAffinityMasks(defaultAffinityMasks, numThreads);
		affinityMasks = defaultAffinityMasks;
	}
	 
	// initialize threads first, then start

	mWorkerThreads = PX_ALLOCATE(CpuWorkerThread, numThreads, "CpuWorkerThread");
	const PxU32 nameLength = 32;
	mThreadNames = PX_ALLOCATE(PxU8, nameLength * numThreads, "CpuWorkerThreadName");

	if (mWorkerThreads)
	{
		for(PxU32 i = 0; i < numThreads; ++i)
		{
			PX_PLACEMENT_NEW(mWorkerThreads+i, CpuWorkerThread)();
			mWorkerThreads[i].initialize(this);
		}

		for(PxU32 i = 0; i < numThreads; ++i)
		{
			if (mThreadNames)
			{
				char* threadName = reinterpret_cast<char*>(mThreadNames + (i*nameLength));
				Ps::snprintf(threadName, nameLength, "PxWorker%02d", i);
				mWorkerThreads[i].setName(threadName);
			}

			mWorkerThreads[i].setAffinityMask(affinityMasks[i]);
			mWorkerThreads[i].start(Ps::Thread::getDefaultStackSize());
		}

		PX_FREE(defaultAffinityMasks);
	}
	else
	{
		mNumThreads = 0;
	}
}

physx5::Ext::DefaultCpuDispatcher::~DefaultCpuDispatcher()
{
	for(PxU32 i = 0; i < mNumThreads; ++i)
		mWorkerThreads[i].signalQuit();

	mShuttingDown = true;
	if(PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK == mWaitForWorkMode)
		mWorkReady.set();
	for(PxU32 i = 0; i < mNumThreads; ++i)
		mWorkerThreads[i].waitForQuit();

	for(PxU32 i = 0; i < mNumThreads; ++i)
		mWorkerThreads[i].~CpuWorkerThread();

	PX_FREE(mWorkerThreads);
	PX_FREE(mThreadNames);
}

void physx5::Ext::DefaultCpuDispatcher::submitTask(PxBaseTask& task)
{
	if(!mNumThreads)
	{
		// no worker threads, run directly
		runTask(task);
		task.release();
		return;
	}	

	// TODO: Could use TLS to make this more efficient
	const Ps::Thread::Id currentThread = Ps::Thread::getId();
	for(PxU32 i = 0; i < mNumThreads; ++i)
	{
		if(mWorkerThreads[i].tryAcceptJobToLocalQueue(task, currentThread))
		{
			if(PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK == mWaitForWorkMode)
			{
				return mWorkReady.set();
			}
			else
			{
				PX_ASSERT(PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_PROCESSOR == mWaitForWorkMode || PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_THREAD == mWaitForWorkMode);
				return;
			}
		}
	}

	physx::Ext::SharedQueueEntry* entry = mQueueEntryPool.getEntry(&task);
	if (entry)
	{
		mJobList.push(*entry);
		if(PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK == mWaitForWorkMode)
		{
			mWorkReady.set();
		}
	}
}

PxBaseTask* physx5::Ext::DefaultCpuDispatcher::fetchNextTask()
{
	PxBaseTask* task = getJob();

	if(!task)
		task = stealJob();

	return task;
}

void physx5::Ext::DefaultCpuDispatcher::release()
{
	PX_DELETE(this);
}

PxBaseTask* physx5::Ext::DefaultCpuDispatcher::getJob(void)
{
	return physx::Ext::TaskQueueHelper::fetchTask(mJobList, mQueueEntryPool);
}

PxBaseTask* physx5::Ext::DefaultCpuDispatcher::stealJob()
{
	PxBaseTask* ret = NULL;

	for(PxU32 i = 0; i < mNumThreads; ++i)
	{
		ret = mWorkerThreads[i].giveUpJob();

		if(ret != NULL)
			break;
	}

	return ret;
}

void physx5::Ext::DefaultCpuDispatcher::resetWakeSignal()
{
	PX_ASSERT(PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK == mWaitForWorkMode);
	mWorkReady.reset();
	
	// The code below is necessary to avoid deadlocks on shut down.
	// A thread usually loops as follows:
	// while quit is not signaled
	// 1)  reset wake signal
	// 2)  fetch work
	// 3)  if work -> process
	// 4)  else -> wait for wake signal
	//
	// If a thread reaches 1) after the thread pool signaled wake up,
	// the wake up sync gets reset and all other threads which have not
	// passed 4) already will wait forever.
	// The code below makes sure that on shutdown, the wake up signal gets
	// sent again after it was reset
	//
	if (mShuttingDown)
		mWorkReady.set();
}
