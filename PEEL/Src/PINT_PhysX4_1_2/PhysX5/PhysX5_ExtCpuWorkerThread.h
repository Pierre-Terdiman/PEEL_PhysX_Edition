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

#ifndef PHYSX5_EXT_CPU_WORKER_THREAD_H
#define PHYSX5_EXT_CPU_WORKER_THREAD_H

#include "CmPhysXCommon.h"
#include "PsThread.h"
#include "PhysX5_PxDefaultCpuDispatcher.h"
#include "extensions/ExtSharedQueueEntryPool.h"

namespace physx5
{
namespace Ext
{
class DefaultCpuDispatcher;

#if PX_VC
#pragma warning(push)
#pragma warning(disable:4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
#endif							// Because of the SList member I assume

	class CpuWorkerThread : public physx::shdfnd::Thread
	{
	public:
										CpuWorkerThread();
										~CpuWorkerThread();
		
		void							initialize(DefaultCpuDispatcher* ownerDispatcher);
		void							execute();
		bool							tryAcceptJobToLocalQueue(physx::PxBaseTask& task, physx::shdfnd::Thread::Id taskSubmitionThread);
		physx::PxBaseTask*				giveUpJob();
		physx::shdfnd::Thread::Id		getWorkerThreadId() const { return mThreadId; }

	protected:
	physx::Ext::SharedQueueEntryPool<>	mQueueEntryPool;
		DefaultCpuDispatcher*			mOwner;
		physx::shdfnd::SList			mLocalJobList;
		physx::shdfnd::Thread::Id		mThreadId;
	};

#if PX_VC
#pragma warning(pop)
#endif

} // namespace Ext

}

#endif
