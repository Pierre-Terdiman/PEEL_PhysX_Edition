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

#include "task/PxTask.h"
#include "PhysX5_ExtCpuWorkerThread.h"
#include "PhysX5_ExtDefaultCpuDispatcher.h"
#include "Extensions/ExtTaskQueueHelper.h"
#include "PsFPU.h"

using namespace physx;
using namespace physx5;

physx5::Ext::CpuWorkerThread::CpuWorkerThread()
:	mQueueEntryPool(EXT_TASK_QUEUE_ENTRY_POOL_SIZE),
	mThreadId(0)
{
}

physx5::Ext::CpuWorkerThread::~CpuWorkerThread()
{
}

void physx5::Ext::CpuWorkerThread::initialize(DefaultCpuDispatcher* ownerDispatcher)
{
	mOwner = ownerDispatcher;
}

bool physx5::Ext::CpuWorkerThread::tryAcceptJobToLocalQueue(PxBaseTask& task, physx::shdfnd::Thread::Id taskSubmitionThread)
{
	if(taskSubmitionThread == mThreadId)
	{
		physx::Ext::SharedQueueEntry* entry = mQueueEntryPool.getEntry(&task);
		if (entry)
		{
			mLocalJobList.push(*entry);
			return true;
		}
		else
			return false;
	}

	return false;
}

PxBaseTask* physx5::Ext::CpuWorkerThread::giveUpJob()
{
	return physx::Ext::TaskQueueHelper::fetchTask(mLocalJobList, mQueueEntryPool);
}

void physx5::Ext::CpuWorkerThread::execute()
{
	mThreadId = getId();

	while (!quitIsSignalled())
    {
		const PxDefaultCpuDispatcherWaitForWorkMode::Enum ownerWaitForWorkMode = mOwner->getWaitForWorkMode();
		if(PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK == ownerWaitForWorkMode)
		{
			mOwner->resetWakeSignal();
		}

		PxBaseTask* task = physx::Ext::TaskQueueHelper::fetchTask(mLocalJobList, mQueueEntryPool);

		if(!task)
			task = mOwner->fetchNextTask();
		
		if (task)
		{
			mOwner->runTask(*task);
			task->release();
		}
		else if(PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_THREAD == ownerWaitForWorkMode)
		{
			physx::shdfnd::Thread::yield();
		}
/*		else if(PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_PROCESSOR == ownerWaitForWorkMode)
		{
			const PxU32 pauseCounter = mOwner->getYieldProcessorCount();
			for (PxU32 j = 0; j < pauseCounter; j++)
			{
				physx::shdfnd::Thread::yieldProcesor();
			}
		}*/
		else
		{
			PX_ASSERT(PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK == ownerWaitForWorkMode);
			mOwner->waitForWork();
		}
	}

	quit();
}
