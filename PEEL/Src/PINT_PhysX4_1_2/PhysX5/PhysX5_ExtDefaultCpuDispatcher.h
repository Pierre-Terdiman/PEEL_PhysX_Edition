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

#ifndef PHYSX5_EXT_DEFAULT_CPU_DISPATCHER_H
#define PHYSX5_EXT_DEFAULT_CPU_DISPATCHER_H

#include "common/PxProfileZone.h"
#include "task/PxTask.h"
#include "PhysX5_PxDefaultCpuDispatcher.h"

#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PsSync.h"
#include "PsSList.h"
#include "extensions/ExtSharedQueueEntryPool.h"


namespace physx5
{
namespace Ext
{
	class CpuWorkerThread;

#if PX_VC
#pragma warning(push)
#pragma warning(disable:4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
#endif							// Because of the SList member I assume

	class DefaultCpuDispatcher : public PxDefaultCpuDispatcher, public physx::Ps::UserAllocated
	{
		friend class TaskQueueHelper;

	private:
												DefaultCpuDispatcher() : mQueueEntryPool(0) {}
												~DefaultCpuDispatcher();
	public:
												DefaultCpuDispatcher(physx::PxU32 numThreads, physx::PxU32* affinityMasks, PxDefaultCpuDispatcherWaitForWorkMode::Enum mode = PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK, physx::PxU32 yieldProcessorCount = 0);

		//---------------------------------------------------------------------------------
		// PxCpuDispatcher implementation
		//---------------------------------------------------------------------------------
		virtual			void					submitTask(physx::PxBaseTask& task);
		virtual			physx::PxU32			getWorkerCount()	const	{ return mNumThreads;	}

		//---------------------------------------------------------------------------------
		// PxDefaultCpuDispatcher implementation
		//---------------------------------------------------------------------------------
		virtual			void					release();

		virtual			void					setRunProfiled(bool runProfiled) { mRunProfiled = runProfiled; }

		virtual			bool					getRunProfiled() const { return mRunProfiled; }

		//---------------------------------------------------------------------------------
		// DefaultCpuDispatcher
		//---------------------------------------------------------------------------------
						physx::PxBaseTask*		getJob();
						physx::PxBaseTask*		stealJob();
						physx::PxBaseTask*		fetchNextTask();

		PX_FORCE_INLINE	void					runTask(physx::PxBaseTask& task)
												{
#if PX_SUPPORT_PXTASK_PROFILING
													if(mRunProfiled)
													{
														PX_PROFILE_ZONE(task.getName(), task.getContextId());
														task.run();
													}
													else
#endif
														task.run();
												}

    					void					waitForWork() { PX_ASSERT(PxDefaultCpuDispatcherWaitForWorkMode::eWAIT_FOR_WORK == mWaitForWorkMode); mWorkReady.wait(); }
						void					resetWakeSignal();

		static			void					getAffinityMasks(physx::PxU32* affinityMasks, physx::PxU32 threadCount);

		PX_FORCE_INLINE	PxDefaultCpuDispatcherWaitForWorkMode::Enum		getWaitForWorkMode() const {return mWaitForWorkMode;}
		PX_FORCE_INLINE	physx::PxU32			getYieldProcessorCount() const {return mYieldProcessorCount;}

	protected:
						CpuWorkerThread*		mWorkerThreads;
			physx::Ext::SharedQueueEntryPool<>	mQueueEntryPool;
						physx::shdfnd::SList	mJobList;
						physx::shdfnd::Sync		mWorkReady;
						physx::PxU8*			mThreadNames;
						physx::PxU32			mNumThreads;
						bool					mShuttingDown;
						bool					mRunProfiled;
						PxDefaultCpuDispatcherWaitForWorkMode::Enum		mWaitForWorkMode;
						physx::PxU32			mYieldProcessorCount;
	};

#if PX_VC
#pragma warning(pop)
#endif

} // namespace Ext
}

#endif
