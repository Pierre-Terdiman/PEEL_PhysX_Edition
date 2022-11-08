///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_PhysX510.h"
#include "..\PINT_Common\PINT_Common.h"
#include "..\PINT_Common\PINT_CommonPhysX3_Setup.h"

#include "extensions\PxExtensionsAPI.h"
//#include "common/PxIO.h"
#include "common/PxRenderBuffer.h"
//#include "physxprofilesdk/PxProfileZoneManager.h"
//#include "physxprofilesdk/PxProfileSDK.h"
//#include "NvProfileZoneManager.h"

#include "..\PINT_Common\PINT_CommonPhysX5_CustomSQ.h"

//#define MODIFY_CONTACTS

//..\..\..\PEEL_Externals\PhysX-3.4_Trunk\Lib\vc11win32\PhysX3Extensions.lib

//#define USE_LOAD_LIBRARY

//#define USE_FORCE_THRESHOLD

#include "..\PINT_Common\PINT_CommonPhysX5_Fluid.h"
#ifdef TEST_FLUIDS
PxPBDParticleSystem* gParticleSystem = null;
#endif

#define PVD_HOST "127.0.0.1"

#define USE_PVD 0
#ifdef PINT_SUPPORT_PVD
	#define USE_NEW_PVD
#else
//	#define USE_NEW_PVD
#endif

//#define USE_SPY_PROFILER
//#define DEBUG_SPY_PROFILER
#ifdef USE_SPY_PROFILER
	#include "..\Spy\SpyClient.h"

	#ifdef _WIN64
		#ifdef _DEBUG
			#pragma comment(lib, "../../Spy/Win64/SpyClient_D.lib")
		#else
			#pragma comment(lib, "../../Spy/Win64/SpyClient.lib")
		#endif
	#else
		#pragma comment(lib, "../../Spy/Win32/SpyClient.lib")
	#endif

	#define SPY_ZONE(Label)	Spy::Zone __SpyZone(Label);

	class SpyProfilerCallback : public PxProfilerCallback
	{
		public:

		virtual void* zoneStart(const char* eventName, bool detached, uint64_t contextId)	override
		{
			if(!detached)
				return Spy::StartScopedZone(eventName);
			Spy::StartZone(eventName);
			return null;
		}

		virtual void zoneEnd(void* profilerData, const char* eventName, bool detached, uint64_t contextId)	override
		{
			if(profilerData)
			{
				ASSERT(!detached);
				Spy::EndScopedZone(profilerData);
			}
			else
			{
				ASSERT(detached);
				Spy::EndZone(eventName);
			}
		}
	};

static SpyProfilerCallback gSpyProfiler;

#else
	#define SPY_ZONE(Label)
#endif

#ifdef WIN64
	#pragma comment(lib, "../../Ice/Lib64/IceCore64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceMaths64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Contact64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Meshmerizer64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceImageWork64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceGUI64.lib")
//	#pragma comment(lib, "../../Ice/Lib64/IML64.lib")

	#if _MSC_VER >= 1910
		#ifdef PINT_SUPPORT_PVD
			#ifndef USE_LOAD_LIBRARY
				#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/profile/PhysX_64.lib")
				#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/profile/PhysXCommon_64.lib")
				#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/profile/PhysXCooking_64.lib")
				#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/profile/PhysXVehicle_static_64.lib")
				#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/profile/PhysXFoundation_64.lib")
				#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/profile/PhysXExtensions_static_64.lib")
				#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/profile/PhysXCharacterKinematic_static_64.lib")
				//#ifdef USE_NEW_PVD
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/profile/PhysXPvdSDK_static_64.lib")
				//#endif
			#endif
		#else
			#ifdef _DEBUG
				#ifndef USE_LOAD_LIBRARY
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysX_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysXCommon_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysXCooking_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysXVehicle_static_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysXCharacterKinematic_static_64.lib")
					//#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysXGpu_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysXFoundation_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysXExtensions_static_64.lib")
					//#ifdef USE_NEW_PVD
						#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/debug/PhysXPvdSDK_static_64.lib")
					//#endif
				#endif
			#else
				#ifndef USE_LOAD_LIBRARY
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/release/PhysX_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/release/PhysXCommon_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/release/PhysXCooking_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/release/PhysXVehicle_static_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/release/PhysXFoundation_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/release/PhysXExtensions_static_64.lib")
					#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/release/PhysXCharacterKinematic_static_64.lib")
					#ifdef USE_NEW_PVD
						#pragma comment(lib, "../../../../PEEL_Externals/5_1_0/bin/win.x86_64.vc141.mt/release/PhysXPvdSDK_static_64.lib")
					#endif
				#endif
			#endif
		#endif
	#endif
#endif

static PhysX_CQS* gPhysX = null;

///////////////////////////////////////////////////////////////////////////////

static PxPvd* gVisualDebugger = null;

///////////////////////////////////////////////////////////////////////////////

//static PxDefaultAllocator* gDefaultAllocator = null;
//static PxDefaultErrorCallback* gDefaultErrorCallback = null;

/*static*/ PEEL_PhysX3_AllocatorCallback* gDefaultAllocator = null;
//static PxAllocatorCallback* gDefaultAllocator = null;
/*static*/ PxErrorCallback* gDefaultErrorCallback = null;

static PxDefaultCpuDispatcher* gDefaultCPUDispatcher = null;

#ifdef PHYSX_SUPPORT_GPU
static PxCudaContextManager* gCudaContextManager = null;
#endif

static const bool gUseCustomSQ = false;	// ### CCD tests fail with this!!
static PxSceneQuerySystem* gSceneQuerySystem = null;

///////////////////////////////////////////////////////////////////////////////

PhysX::PhysX(const EditableParams& params) :
	SharedPhysX_Vehicles(params),
	mPVD				(null),
	mTransport			(null)
//	mProfileZoneManager	(null),
{
}

PhysX::~PhysX()
{
	ASSERT(!gDefaultCPUDispatcher);
	ASSERT(!gDefaultErrorCallback);
	ASSERT(!gDefaultAllocator);
//	ASSERT(!mProfileZoneManager);
	ASSERT(!mPVD);
	ASSERT(!mTransport);
}

const char* PhysX::GetName() const
{
	const udword Suffix = mParams.ThreadIndexToNbThreads(mParams.mNbThreadsIndex);
	return _F("PhysX 5.1.0 (%dT)", Suffix);
}

const char* PhysX::GetUIName() const
{
	return "PhysX 5.1.0";
}

void PhysX::GetCaps(PintCaps& caps) const
{
	caps.mSupportRigidBodySimulation	= true;
	caps.mSupportCylinders				= true;
	caps.mSupportConvexes				= true;
	caps.mSupportMeshes					= true;
	caps.mSupportDynamicMeshes			= true;
	caps.mSupportDeformableMeshes		= true;
	caps.mSupportHeightfields			= true;
	caps.mSupportContactNotifications	= true;
	caps.mSupportContactModifications	= true;
	caps.mSupportMassForInertia			= true;
	caps.mSupportKinematics				= true;
	caps.mSupportCollisionGroups		= true;
	caps.mSupportCompounds				= true;
	caps.mSupportAggregates				= true;

	caps.mSupportSphericalJoints		= true;
	caps.mSupportHingeJoints			= true;
	caps.mSupportFixedJoints			= true;
	caps.mSupportPrismaticJoints		= true;
	caps.mSupportDistanceJoints			= true;
	caps.mSupportD6Joints				= true;
	caps.mSupportGearJoints				= true;
	caps.mSupportRackJoints				= true;
	caps.mSupportPortalJoints			= true;
	caps.mSupportMCArticulations		= false;
	caps.mSupportRCArticulations		= true;

	caps.mSupportRaycasts				= true;

	caps.mSupportBoxSweeps				= true;
	caps.mSupportSphereSweeps			= true;
	caps.mSupportCapsuleSweeps			= true;
	caps.mSupportConvexSweeps			= true;

	caps.mSupportSphereOverlaps			= true;
	caps.mSupportBoxOverlaps			= true;
	caps.mSupportCapsuleOverlaps		= true;
	caps.mSupportConvexOverlaps			= true;
	caps.mSupportMeshMeshOverlaps		= true;

	caps.mSupportVehicles				= true;
	caps.mSupportCharacters				= true;
}

#ifdef MODIFY_CONTACTS
static PxFilterFlags ContactModifySimulationFilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0, 
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	PxU32 constantBlockSize)
{
	// let triggers through
	if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlags();
	}

	pairFlags = PxPairFlag::eCONTACT_DEFAULT|PxPairFlag::eMODIFY_CONTACTS;
//	pairFlags = PxPairFlag::eCONTACT_DEFAULT|PxPairFlag::eNOTIFY_CONTACT_POINTS|PxPairFlag::eNOTIFY_TOUCH_FOUND|PxPairFlag::eNOTIFY_TOUCH_PERSISTS;

	return PxFilterFlags();
}
#endif

#ifdef USE_LOAD_LIBRARY
//typedef physx::PxPhysics*	(*PxFunction_CreateBasePhysics)	(physx::PxU32 version, physx::PxFoundation& foundation, const physx::PxTolerancesScale& scale, bool trackOutstandingAllocations, physx::PxProfileZoneManager* profileZoneManager);
//typedef void				(*PxFunction_RegisterPCM)		(physx::PxPhysics& physics);
//typedef physx::PxCooking*	(*PxFunction_CreateCooking)		(physx::PxU32 version, physx::PxFoundation& foundation, const physx::PxCookingParams& params);
//typedef physx::PxFoundation*(*PxFunction_CreateFoundation)	(physx::PxU32 version, physx::PxAllocatorCallback& allocator, physx::PxErrorCallback& errorCallback);
//PX_C_EXPORT bool PX_CALL_CONV PxInitExtensions(physx::PxPhysics& physics);
#endif

#ifdef MODIFY_CONTACTS
class MyContactModifyCallback : public PxContactModifyCallback
{
	public:
	virtual void onContactModify(PxContactModifyPair* const pairs, PxU32 count)
	{
//		printf("onContactModify: %d pairs\n", count);
//		const PxReal minS = -0.2f;
		const PxReal minS = -0.1f;
		for(PxU32 i=0;i<count;i++)
		{
			const PxU32 nbContacts = pairs[i].contacts.size();
			for(PxU32 j=0;j<nbContacts;j++)
			{
				const PxTransform Pose = pairs[i].actor[0]->getGlobalPose();
				if(0)
				{
					PxVec3 p = pairs[i].contacts.getPoint(j);
					p.z = Pose.p.z;
					p.x = Pose.p.x;
					pairs[i].contacts.setPoint(j, p);
//					pairs[i].contacts.setPoint(j, Pose.p);
				}

				if(1)
				{
					const PxReal s = pairs[i].contacts.getSeparation(j);
//					printf("%f\n", s);
					if(s<minS)
					{
	//					printf("%f\n", s);
						pairs[i].contacts.setSeparation(j, minS);
					}
				}
			}
		}
	}
}gContactModifyCallback;
#endif

#if PHYSX_SUPPORT_CONTACT_MODIFICATIONS
	#include "..\PINT_Common\PINT_CommonPhysX3_ContactModif.h"
	static PEEL_ContactModifyCallback gNewContactModifyCallback;
#endif

#include "..\PINT_Common\PINT_CommonPhysX3_ContactNotif.h"
static PEEL_SimulationEventCallback gSimulationEventCallback;

void PhysX::Init(const PINT_WORLD_CREATE& desc)
//PintSceneHandle PhysX::Init(const PINT_WORLD_CREATE& desc)
{
#ifdef USE_LOAD_LIBRARY
/*	udword FPUEnv[256];
	FillMemory(FPUEnv, 256*4, 0xff);
	__asm fstenv FPUEnv
		HMODULE handle0 = ::LoadLibraryA("PhysX3_x86.dll");
		HMODULE handle1 = ::LoadLibraryA("PhysX3Cooking_x86.dll");
		HMODULE handle2 = ::LoadLibraryA("PhysX3Common_x86.dll");
	__asm fldenv FPUEnv
	if(!handle0 || !handle1 || !handle2)
		return;
	PxFunction_CreateBasePhysics func0	= (PxFunction_CreateBasePhysics)	GetProcAddress(handle0, "PxCreateBasePhysics");
	PxFunction_RegisterPCM func1		= (PxFunction_RegisterPCM)			GetProcAddress(handle0, "PxRegisterPCM");
	PxFunction_CreateCooking func2		= (PxFunction_CreateCooking)		GetProcAddress(handle0, "PxCreateCooking");
	PxFunction_CreateFoundation func3	= (PxFunction_CreateFoundation)		GetProcAddress(handle0, "PxCreateFoundation");

	if(!func0 || !func1 || !func2 || !func3)
	{
		FreeLibrary(handle2);
		FreeLibrary(handle1);
		FreeLibrary(handle0);
		return;
	}*/
#endif

//	gDefaultAllocator = new PxDefaultAllocator;
//	gDefaultErrorCallback = new PxDefaultErrorCallback;
	gDefaultAllocator = new PEEL_PhysX3_AllocatorCallback;
	gDefaultErrorCallback = new PEEL_PhysX3_ErrorCallback;

	gSimulationEventCallback.Init(*this, desc.mContactNotifyCallback);
	gNewContactModifyCallback.Init(*this, desc.mContactModifyCallback, desc.mContactModifyCallback2);
	mEnableContactNotif = false;
	mContactNotifThreshold = FLT_MAX;

	if(desc.mContactNotifyCallback)
	{
		mEnableContactNotif = true;
		mContactNotifThreshold = desc.mContactNotifyCallback->GetContactThreshold();
	}

	ASSERT(!mFoundation);
#ifdef USE_LOAD_LIBRARY
//	mFoundation = (func3)(PX_PHYSICS_VERSION, *gDefaultAllocator, *gDefaultErrorCallback);
#else
//	mFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, *gDefaultAllocator, *gDefaultErrorCallback);
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *gDefaultAllocator, *gDefaultErrorCallback);
#endif
	ASSERT(mFoundation);

#ifdef USE_SPY_PROFILER
	PxSetProfilerCallback(&gSpyProfiler);
#endif

#if USE_PVD
    gVisualDebugger = PxCreatePvd(*mFoundation);
#endif
#ifdef USE_NEW_PVD
	gVisualDebugger = PxCreatePvd(*mFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	//gVisualDebugger->connect(*transport, PxPvdInstrumentationFlag::eALL);
	bool PVDStatus = gVisualDebugger->connect(*transport, PxPvdInstrumentationFlag::ePROFILE);
	if(!PVDStatus)
		printf("PVD connection failed\n");
#endif

	PxTolerancesScale scale;
	{
		ASSERT(!mPhysics);
	#ifdef USE_LOAD_LIBRARY
//		mPhysics = (func0)(PX_PHYSICS_VERSION, *mFoundation, scale, false, mProfileZoneManager);
	//	PxRegisterArticulations(*mPhysics);
	//	PxRegisterHeightFields(*mPhysics);
//		(func1)(*mPhysics);
	#else
//		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, scale, false, mProfileZoneManager);
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, scale, false, gVisualDebugger);
	#endif
		ASSERT(mPhysics);
//		mPhysics->registerDeletionListener(*this, PxDeletionEventFlag::eUSER_RELEASE);
	}

	bool status = PxInitExtensions(*mPhysics, gVisualDebugger);
	ASSERT(status);
//	gDefaultCPUDispatcher = PxDefaultCpuDispatcherCreate(mParams.ThreadIndexToNbThreads(mParams.mNbThreadsIndex), null);
	gDefaultCPUDispatcher = PxDefaultCpuDispatcherCreate(mParams.ThreadIndexToNbThreads(mParams.mNbThreadsIndex), null, PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_THREAD);
//	gDefaultCPUDispatcher = PxDefaultCpuDispatcherCreate(16, null);

//	gDispatcher = PxDefaultCpuDispatcherCreate(inNumThreads, NULL, PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_THREAD);
//	gDispatcher = PxDefaultCpuDispatcherCreate(inNumThreads, NULL, PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_PROCESSOR, 8);


	CreateCooking(scale, PxMeshPreprocessingFlag::eWELD_VERTICES);

	{
		ASSERT(!mScene);
		PxSceneDesc sceneDesc(scale);
		SetupSceneDesc(sceneDesc, desc, mParams, gDefaultCPUDispatcher, &gSimulationEventCallback, &gNewContactModifyCallback);

#ifdef MODIFY_CONTACTS
		sceneDesc.contactModifyCallback		= &gContactModifyCallback;
		sceneDesc.filterShader				= ContactModifySimulationFilterShader;
#endif

#ifdef PHYSX_SUPPORT_GPU
		if(mParams.mUseGPU)
		{
			printf("Using GPU\n");
			sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
			sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
			// TODO: expose this. More partitions = slower.
			sceneDesc.gpuMaxNumPartitions = 8;
//			sceneDesc.gpuMaxNumPartitions = 32;
/*			sceneDesc.gpuDynamicsConfig.patchStreamCapacity *= 2; //KS - must increase because we can exceed the default 4MB buffer with the arena demo!
			sceneDesc.gpuDynamicsConfig.contactStreamCapacity *= 2; //KS - must increase because we can exceed the default 4MB buffer with the arena demo!
			sceneDesc.gpuDynamicsConfig.contactStreamCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.forceStreamCapacity *= 2;
//			sceneDesc.gpuDynamicsConfig.frictionBufferCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.patchStreamCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.tempBufferCapacity *= 2;*/

	#ifdef PHYSX_SUPPORT_GPU_NEW_MEMORY_CONFIG
			sceneDesc.gpuDynamicsConfig.maxRigidContactCount *= 2;
			sceneDesc.gpuDynamicsConfig.maxRigidPatchCount *= 2;
	#else
			sceneDesc.gpuDynamicsConfig.constraintBufferCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.contactBufferCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.contactStreamSize *= 2;
			sceneDesc.gpuDynamicsConfig.patchStreamSize *= 2;
			sceneDesc.gpuDynamicsConfig.forceStreamCapacity *= 2;
	#endif
			sceneDesc.gpuDynamicsConfig.tempBufferCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.heapCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.foundLostPairsCapacity *= 2;

#ifdef TEST_FLUIDS
			//sceneDesc.gpuDynamicsConfig.maxParticleContacts *= 2;
#endif

			PxCudaContextManagerDesc cudaContextManagerDesc;
			cudaContextManagerDesc.interopMode = PxCudaInteropMode::OGL_INTEROP;
//printf("Checkpoint 00\n");
			gCudaContextManager = PxCreateCudaContextManager(*mFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
//printf("Checkpoint 01\n");
			if(gCudaContextManager && !gCudaContextManager->contextIsValid())
			{
				gCudaContextManager->release();
				gCudaContextManager = NULL;
			}	
//			if(gCudaContextManager)
//				sceneDesc.gpuDispatcher = gCudaContextManager->getGpuDispatcher();	//Set the GPU dispatcher, used by GRB to dispatch CUDA kernels.
				sceneDesc.cudaContextManager = gCudaContextManager;
		}
#endif

		if(gUseCustomSQ)
		{
			sceneDesc.sceneQuerySystem = gSceneQuerySystem = CreatePEELCustomSceneQuerySystem();
		}

		mScene = mPhysics->createScene(sceneDesc);
		ASSERT(mScene);

#if USE_PVD
		if(gVisualDebugger && mScene)
		{
			PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
//			if(gEnablePVDProfile)
				gVisualDebugger->connect(*transport, PxPvdInstrumentationFlag::ePROFILE);
//			else
//				gVisualDebugger->connect(*transport, PxPvdInstrumentationFlag::eALL);
/*			PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
			if (pvdClient)
			{
				pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
				pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
				pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
			}*/
		}
#endif
#ifdef USE_NEW_PVD
		PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
		if(pvdClient)
		{
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
#endif
	}

	SetupBroadphase(desc, mParams, mScene);

#ifdef TEST_FLUIDS
	if(gCudaContextManager)
	{
//printf("Checkpoint 02\n");
		const bool useLargeFluid = true;
		//const PxU32 maxDiffuseParticles = useLargeFluid ? 2000000 : 100000;
		const PxU32 maxDiffuseParticles = useLargeFluid ? 1000000 : 100000;
		//const PxU32 maxDiffuseParticles = 10000000;
		const PxVec3 FluidPos(0.0f, 20.0f, 0.0f);
		PxPBDParticleSystem* particleSystem = initParticles(*mPhysics, *mScene, mParams, 50, 120 * (useLargeFluid ? 5 : 1), 30, FluidPos, 0.1f, maxDiffuseParticles);
		allocParticleBuffers(*mPhysics, *mScene, particleSystem);
		gParticleSystem = particleSystem;
//printf("Checkpoint 03\n");
	}
#endif

	InitCommon();
	UpdateFromUI();

//	return mScene;
}

#ifdef REMOVED
static void setupJoint(PxArticulationJoint* j)
{
	j->setExternalCompliance(1.0f);
	j->setDamping(0.0f);

/*	j->setSwingLimitEnabled(true);
	j->setSwingLimitEnabled(false);
//	j->setSwingLimit(PxPi/6, PxPi/6);
	j->setSwingLimit(0.00001f, 0.00001f);
//	j->setTwistLimitEnabled(true);
	j->setTwistLimitEnabled(false);
//	j->setTwistLimit(-PxPi/12, PxPi/12);
	j->setTwistLimit(-0.00001f, 0.00001f);

	if(0)
	{
//		const float Limit = 0.00001f;
		const float Limit = 0.01f;
		j->setSwingLimitEnabled(true);
		j->setSwingLimit(Limit, Limit);
		j->setTwistLimitEnabled(true);
		j->setTwistLimit(-Limit, Limit);
	}*/
}

static void setupJoint(PxArticulationJoint* j, const PINT_ARTICULATED_BODY_CREATE& bc)
{
//	setupJoint(j);
	j->setSwingLimitEnabled(bc.mEnableSwingLimit);
	j->setSwingLimit(bc.mSwingYLimit, bc.mSwingZLimit);

	j->setTwistLimitEnabled(bc.mEnableTwistLimit);
	j->setTwistLimit(bc.mTwistLowerLimit, bc.mTwistUpperLimit);

	if(bc.mUseMotor)
	{
		if(bc.mMotor.mExternalCompliance!=0.0f)
			j->setExternalCompliance(bc.mMotor.mExternalCompliance);
		if(bc.mMotor.mInternalCompliance!=0.0f)
			j->setInternalCompliance(bc.mMotor.mInternalCompliance);
		j->setDamping(bc.mMotor.mDamping);
		j->setStiffness(bc.mMotor.mStiffness);
		if(!bc.mMotor.mTargetVelocity.IsNotUsed())
			j->setTargetVelocity(ToPxVec3(bc.mMotor.mTargetVelocity));
		if(!bc.mMotor.mTargetOrientation.IsNotUsed())
			j->setTargetOrientation(ToPxQuat(bc.mMotor.mTargetOrientation));
	}
}
#endif

void PhysX::TestNewFeature()
{
	return;
#ifdef REMOVED
	{
		PxHeightFieldDesc heightFieldDesc;
		heightFieldDesc.nbColumns			= 33;
		heightFieldDesc.nbRows				= 33;
//		heightFieldDesc.thickness			= 0.0f;
//		heightFieldDesc.convexEdgeThreshold	= 0.0f;

		heightFieldDesc.samples.data		= new PxU32[heightFieldDesc.nbColumns*heightFieldDesc.nbRows];
		heightFieldDesc.samples.stride		= sizeof(PxU32);

		char* currentByte = (char*)heightFieldDesc.samples.data;
		for (PxU32 row = 0; row < heightFieldDesc.nbRows; row++)
		{
			for (PxU32 column = 0; column < heightFieldDesc.nbColumns; column++)
			{
				PxI16 height = row % 2;
				PxHeightFieldSample* currentSample = (PxHeightFieldSample*)currentByte;
				currentSample->height = height;
				currentSample->height = 0;
/*				if (column == 2)
				{
					currentSample->materialIndex0 = PxHeightFieldMaterial::eHOLE;
					currentSample->materialIndex1 = PxHeightFieldMaterial::eHOLE;
				}
				else*/
				{
					currentSample->materialIndex0 = 0;
					currentSample->materialIndex1 = 0;
				}
				currentSample->clearTessFlag();
				currentByte += heightFieldDesc.samples.stride;
			}
		}

		PxHeightField* pHeightField = mCooking->createHeightField(heightFieldDesc, mPhysics->getPhysicsInsertionCallback());
		delete[] static_cast<const PxU32*>(heightFieldDesc.samples.data);


		PxHeightFieldGeometry hg;
		hg.heightField = pHeightField;

		hg.heightScale	= 1.0f;
		hg.rowScale		= 1000.0f;
		hg.columnScale	= 1000.0f;


		const PxTransform pose(PxVec3(40000.0f, 0.0f, 0.0f));
		PxRigidActor* actor = static_cast<PxRigidActor *>(mPhysics->createRigidStatic(pose));

		PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, hg, *mDefaultMaterial);


//		shape.setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
//		shape->setFlag(PxShapeFlag::eVISUALIZATION, debug_viz_flag);
//		shape.setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, gUseCCD);
		shape->setContactOffset(mParams.mContactOffset);
		shape->setRestOffset(mParams.mRestOffset);
		shape->setQueryFilterData(PxFilterData(1, 0, 0, 0));

		mScene->addActor(*actor);
	}
	return;
#endif

#ifdef REMOVED
	PxArticulation* articulation = (PxArticulation*)CreateArticulation();

//	PxArticulationLink* link = articulation->createLink(parent, linkPose);
//	PxRigidActorExt::createExclusiveShape(*link, linkGeometry, material);
//	PxRigidBodyExt::updateMassAndInertia(*link, 1.0f);

	const bool UseFiltering = true;
	if(UseFiltering)
	{
		const PintDisabledGroups DG(1, 2);
		SetDisabledGroups(1, &DG);
	}

	const float Radius = 1.0f;
	const udword NbSpheres = 20;
	const udword NbRows = 1;
	const Point Dir(1.0f, 0.0f, 0.0f);
//	const Point Extents = Dir * Radius;
	const Point Extents = Dir * (Radius + 1.0f);
	const Point PosOffset = Dir * 0.0f;

	Matrix3x3 m;
	m.RotZ(degToRad(90.0f));

//	PINT_SPHERE_CREATE SphereDesc;
	PINT_CAPSULE_CREATE SphereDesc;
//	SphereDesc.mRenderer	= CreateSphereRenderer(Radius);		#####
	SphereDesc.mRadius		= Radius;
	SphereDesc.mHalfHeight	= 1.0f;
	SphereDesc.mLocalRot	= m;

	for(udword i=0;i<NbRows;i++)
	{
		PintObjectHandle Handles[NbSpheres];
		Point Positions[NbSpheres];

		Point Pos(0.0f, 40.0f, float(i)*Radius*4.0f);

		Positions[0] = Pos;
		udword GroupBit = 0;
//		Handles[0] = CreateStaticObject(pint, &SphereDesc, Pos);
		{
			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes			= &SphereDesc;
//			ObjectDesc.mMass			= 0.0f;
			ObjectDesc.mMass			= 1.0f;
			ObjectDesc.mPosition		= Pos;
			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
			Handles[0] = CreateArticulationLink(articulation, null, *this, ObjectDesc);
//			Handles[0] = CreatePintObject(*this, ObjectDesc);

				// Make it "static"...
//				PINT_SPHERICAL_JOINT_CREATE Desc;
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mLocalAxis0 = Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1 = Point(0.0f, 0.0f, 1.0f);
				Desc.mObject0		= null;
				Desc.mObject1		= Handles[0];
				// #### WTF !!!!!
				Desc.mLocalPivot0	= Pos + Point(0.0f, 0.0f, 0.0f);
//				Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Pos + Point(0.0f, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
				PintJointHandle JointHandle = CreateJoint(Desc);
				ASSERT(JointHandle);

		}
		Pos += (PosOffset + Extents)*2.0f;

		for(udword i=1;i<NbSpheres-1;i++)
		{
			Positions[i] = Pos;
//			Handles[i] = CreateDynamicObject(pint, &SphereDesc, Pos);
			{
				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes			= &SphereDesc;
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
				// Note that this already creates the joint between the objects!
				Handles[i] = CreateArticulationLink(articulation, (PxArticulationLink*)Handles[i-1], *this, ObjectDesc);

				// ...so we setup the joint data immediately
				PxArticulationJoint* joint = ((PxArticulationLink*)Handles[i])->getInboundJoint();
				if(joint)
				{
					joint->setParentPose(PxTransform(ToPxVec3(Extents + PosOffset)));
					joint->setChildPose(PxTransform(ToPxVec3(-Extents - PosOffset)));
					setupJoint(joint);
				}
			}
			Pos += (PosOffset + Extents)*2.0f;
		}

		const Point BoxExtents(10.0f, 10.0f, 10.0f);
		{
			const udword i=NbSpheres-1;
			PINT_BOX_CREATE BoxDesc;
//			BoxDesc.mRenderer	= CreateBoxRenderer(BoxExtents);		#####
			BoxDesc.mExtents	= BoxExtents;

			//###
			Pos.x += BoxExtents.x - Radius;

			Positions[i] = Pos;
//			Handles[i] = CreateDynamicObject(pint, &SphereDesc, Pos);
			{
				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes			= &BoxDesc;
				ObjectDesc.mMass			= 100.0f;
				ObjectDesc.mPosition		= Pos;
				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
				Handles[i] = CreateArticulationLink(articulation, (PxArticulationLink*)Handles[i-1], *this, ObjectDesc);

				printf("Big Mass: %f\n", ((PxArticulationLink*)Handles[i])->getMass());

				PxArticulationJoint* joint = ((PxArticulationLink*)Handles[i])->getInboundJoint();
				if(joint)
				{
					joint->setParentPose(PxTransform(ToPxVec3(Extents + PosOffset)));
					joint->setChildPose(PxTransform(PxVec3(-BoxExtents.x, 0.0f, 0.0f)));
					setupJoint(joint);
				}
			}
			Pos += (PosOffset + Extents)*2.0f;
		}
	}
	AddArticulationToScene(articulation);

	if(1)
	{
		PINT_BOX_CREATE BoxDesc;
		BoxDesc.mExtents	= Point(2.0f, 4.0f, 20.0f);

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes			= &BoxDesc;
		ObjectDesc.mMass			= 1.0f;
		ObjectDesc.mPosition		= Point(0.0f, -50.0f, -15.0f);
		ObjectDesc.mCollisionGroup	= 2;
		PintObjectHandle h = CreateObject(ObjectDesc);

		if(1)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mLocalAxis0	= Point(0.0f, 1.0f, 0.0f);
			Desc.mLocalAxis1	= Point(0.0f, 1.0f, 0.0f);
			Desc.mObject0		= null;
			Desc.mObject1		= h;
			// #### WTF !!!!!
			Desc.mLocalPivot0	= ObjectDesc.mPosition;
			Desc.mLocalPivot1	= Point(0.0f, 0.0f, 0.0f);
			PintJointHandle JointHandle = CreateJoint(Desc);
			ASSERT(JointHandle);
		}
	}
#endif
}

void PhysX::Close()
{
#ifdef TEST_FLUIDS
	if(gParticleSystem)
	{
		clearupParticleBuffers();
		SAFE_RELEASE(gParticleSystem);
	}
#endif

	gSimulationEventCallback.Release();

	CloseCommon();
	CloseVehicles();
#ifdef PHYSX_SUPPORT_CHARACTERS
	ReleaseControllerManager();
#endif
#ifdef PHYSX_SUPPORT_CHARACTERS2
	mCCT.ReleaseControllerManager();
#endif

	SAFE_RELEASE(mDefaultMaterial)

	SAFE_RELEASE(mScene)
#if !PHYSX_SUPPORT_IMMEDIATE_COOKING
	SAFE_RELEASE(mCooking)
#endif
	DELETESINGLE(gDefaultCPUDispatcher);

	PxCloseExtensions();

	SAFE_RELEASE(gSceneQuerySystem)

	SAFE_RELEASE(mPhysics)
#ifdef PHYSX_SUPPORT_GPU
	SAFE_RELEASE(gCudaContextManager);
#endif
//	SAFE_RELEASE(mProfileZoneManager)

	if(gVisualDebugger)
	{
		PxPvdTransport* transport = gVisualDebugger->getTransport();
		gVisualDebugger->disconnect();
		gVisualDebugger->release();
		gVisualDebugger = null;
		SAFE_RELEASE(transport);
	}

	SAFE_RELEASE(mFoundation)
	DELETESINGLE(gDefaultErrorCallback);
	DELETESINGLE(gDefaultAllocator);
}

void PhysX::UpdateFromUI()
{
	SharedUpdateFromUI();
}

udword PhysX::Update(float dt)
{
#ifdef DEBUG_SPY_PROFILER
	printf("\n\nPINT UPDATE\n");
#endif
//	mScene->shiftOrigin(PxVec3(0.0f));

	if(0)
	{
		const udword NbPts = 1000000;
		static Point* Src = null;
		static Point* Dst = null;
		if(!Src)
		{
			Src = ICE_NEW(Point)[NbPts];
			ZeroMemory(Src, sizeof(Point)*NbPts);
			Dst = ICE_NEW(Point)[NbPts];
		}
		Matrix4x4 M;
		M.Zero();

		udword Time;
		StartProfile(Time);
		{
			const Point* S = Src;
			Point* D = Dst;
			udword N = NbPts;
			while(N--)
			{
				TransformPoint4x3(*D++, *S++, M);
			}
		}
		EndProfile(Time);
		printf("Time: %d\n", Time);
	}

	UpdateVehiclesAndPhysX(dt);

	gSimulationEventCallback.sendContacts();

	return gDefaultAllocator->mCurrentMemory;
}

Point PhysX::GetMainColor()
{
//	return Point(238.0f/255.0f, 223.0f/255.0f, 204.0f/255.0f);	// AntiqueWhite2
//	return Point(255.0f/255.0f, 218.0f/255.0f, 185.0f/255.0f);	// PeachPuff
//	return Point(230.0f/255.0f, 230.0f/255.0f, 250.0f/255.0f);	// Lavender
	return Point(255.0f/255.0f, 211.0f/255.0f, 155.0f/255.0f);	// Burlywood1
}

///////////////////////////////////////////////////////////////////////////////


#include "..\PINT_Common\PINT_CommonPhysX3_Queries.h"

void PhysX_CQS::Close()
{
//	mOverlaps.reset();
	PhysX::Close();
}

udword PhysX_CQS::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	if(gSceneQuerySystem)
	{
		ASSERT(mScene);

		const PxQueryFilterData PF = GetSQFilterData();
		const PxSceneQueryFlags sqFlags = GetRaycastQueryFlags(mParams);
		PxQueryFilterCallback* FCB = GetSQFilterCallback();

		PxRaycastHit hit;
		PxQueryFilterData fd1 = PF;

		udword NbHits = 0;
		while(nb--)
		{
			if(PEEL_RaycastClosest((const PxVec3&)(raycasts->mOrigin), (const PxVec3&)(raycasts->mDir), raycasts->mMaxDist, hit, sqFlags, fd1, FCB))
			{
				NbHits++;
				FillResultStruct(*dest, hit);
			}
			else
			{
				dest->SetNoHit();
			}

			raycasts++;
			dest++;
		}
		return NbHits;
	}

	return PhysX::BatchRaycasts(context, nb, dest, raycasts);
}

udword PhysX_CQS::BatchRaycastAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)
{
	if(gSceneQuerySystem)
	{
		ASSERT(mScene);

		const PxQueryFilterData PF = GetSQFilterData();
		PxQueryFilterCallback* FCB = GetSQFilterCallback();

		udword NbHits = 0;
		while(nb--)
		{
			const bool b = PEEL_RaycastAny(ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, PF, FCB);

			NbHits += b;
			dest->mHit = b;
			raycasts++;
			dest++;
		}
		return NbHits;
	}

	return PhysX::BatchRaycastAny(context, nb, dest, raycasts);
}

template<class PintOverlapDataT, class PxGeometryT, class AdapterT>
static udword _BatchShapeOverlapAny(udword nb, PintBooleanHit* restrict_ dest, const PintOverlapDataT* restrict_ overlaps, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	udword NbHits = 0;
	while(nb--)
	{
		PxTransform Pose;
		const PxGeometryT Geom = AdapterT::ToPxGeometry(Pose, overlaps);

		const bool b = PEEL_OverlapAny(Geom, Pose, filterData, filterCB);
		NbHits += b;
		dest->mHit = b;
		overlaps++;
		dest++;
	}
	return NbHits;
}

udword PhysX_CQS::BatchSphereOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxSphereGeometry ToPxGeometry(PxTransform& pose, const PintSphereOverlapData* overlaps)
		{
			return ToPxSphereGeometry(pose, overlaps->mSphere);
		}};
		return _BatchShapeOverlapAny<PintSphereOverlapData, PxSphereGeometry, IceToPx>(nb, dest, overlaps, GetSQFilterData(), GetSQFilterCallback());
	}

	return PhysX::BatchSphereOverlapAny(context, nb, dest, overlaps);
}

udword PhysX_CQS::BatchBoxOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxBoxGeometry ToPxGeometry(PxTransform& pose, const PintBoxOverlapData* overlaps)
		{
			return ToPxBoxGeometry(pose, overlaps->mBox);
		}};
		return _BatchShapeOverlapAny<PintBoxOverlapData, PxBoxGeometry, IceToPx>(nb, dest, overlaps, GetSQFilterData(), GetSQFilterCallback());
	}

	return PhysX::BatchBoxOverlapAny(context, nb, dest, overlaps);
}

udword PhysX_CQS::BatchCapsuleOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxCapsuleGeometry ToPxGeometry(PxTransform& pose, const PintCapsuleOverlapData* overlaps)
		{
			return ToPxCapsuleGeometry(pose, overlaps->mCapsule);
		}};
		return _BatchShapeOverlapAny<PintCapsuleOverlapData, PxCapsuleGeometry, IceToPx>(nb, dest, overlaps, GetSQFilterData(), GetSQFilterCallback());
	}

	return PhysX::BatchCapsuleOverlapAny(context, nb, dest, overlaps);
}

template<class PintOverlapDataT, class PxGeometryT, class AdapterT>
static udword _BatchShapeOverlapObjects(udword nb, PintMultipleHits* restrict_ dest, Container& stream, const PintOverlapDataT* restrict_ overlaps, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	udword Offset = 0;
	udword NbHits = 0;
	PxOverlapHit Hits[4096];
	while(nb--)
	{
		PxTransform Pose;
		const PxGeometryT Geom = AdapterT::ToPxGeometry(Pose, overlaps);

		const PxU32 Nb = PEEL_OverlapAll(Geom, Pose, Hits, 4096, filterData, filterCB);
		NbHits += Nb;
		dest->mNbHits = Nb;
		dest->mOffset = Offset;
		Offset += Nb;

		if(Nb)
		{
			PintOverlapHit* buffer = reinterpret_cast<PintOverlapHit*>(stream.Reserve(Nb*(sizeof(PintOverlapHit)/sizeof(udword))));
			for(PxU32 i=0;i<Nb;i++)
			{
				buffer[i].mTouchedActor	= PintActorHandle(Hits[i].actor);
				buffer[i].mTouchedShape	= PintShapeHandle(Hits[i].shape);
			}
		}

		overlaps++;
		dest++;
	}
	return NbHits;
}

udword PhysX_CQS::BatchSphereOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxSphereGeometry ToPxGeometry(PxTransform& pose, const PintSphereOverlapData* overlaps)
		{
			return ToPxSphereGeometry(pose, overlaps->mSphere);
		}};
		return _BatchShapeOverlapObjects<PintSphereOverlapData, PxSphereGeometry, IceToPx>(nb, dest, stream, overlaps, GetSQFilterData(), GetSQFilterCallback());
	}

	return PhysX::BatchSphereOverlapObjects(context, nb, dest, stream, overlaps);
}

udword PhysX_CQS::BatchBoxOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxBoxGeometry ToPxGeometry(PxTransform& pose, const PintBoxOverlapData* overlaps)
		{
			return ToPxBoxGeometry(pose, overlaps->mBox);
		}};
		return _BatchShapeOverlapObjects<PintBoxOverlapData, PxBoxGeometry, IceToPx>(nb, dest, stream, overlaps, GetSQFilterData(), GetSQFilterCallback());
	}
	return PhysX::BatchBoxOverlapObjects(context, nb, dest, stream, overlaps);
}

udword PhysX_CQS::BatchCapsuleOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxCapsuleGeometry ToPxGeometry(PxTransform& pose, const PintCapsuleOverlapData* overlaps)
		{
			return ToPxCapsuleGeometry(pose, overlaps->mCapsule);
		}};
		return _BatchShapeOverlapObjects<PintCapsuleOverlapData, PxCapsuleGeometry, IceToPx>(nb, dest, stream, overlaps, GetSQFilterData(), GetSQFilterCallback());
	}
	return PhysX::BatchCapsuleOverlapObjects(context, nb, dest, stream, overlaps);
}

template<class PintSweepDataT, class PxGeometryT, class AdapterT>
static udword _BatchSweeps(	udword nb, PintRaycastHit* restrict_ dest, const PintSweepDataT* restrict_ sweeps,
							PxSceneQueryFlags sweepQueryFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	udword NbHits = 0;
	while(nb--)
	{
		PxTransform Pose;
		const PxGeometryT Geom = AdapterT::ToPxGeometry(Pose, sweeps);

		PxSweepHit Hit;
		if(PEEL_SweepClosest(Geom, Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, Hit, sweepQueryFlags, filterData, filterCB))
		{
			NbHits++;
			FillResultStruct(*dest, Hit);
		}
		else
		{
			dest->SetNoHit();
		}

		sweeps++;
		dest++;
	}
	return NbHits;
}

udword PhysX_CQS::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxBoxGeometry ToPxGeometry(PxTransform& pose, const PintBoxSweepData* sweeps)
		{
			return ToPxBoxGeometry(pose, sweeps->mBox);
		}};
		return _BatchSweeps<PintBoxSweepData, PxBoxGeometry, IceToPx>(nb, dest, sweeps, GetSweepQueryFlags(mParams), GetSQFilterData(), GetSQFilterCallback());
	}

	return PhysX::BatchBoxSweeps(context, nb, dest, sweeps);
}

udword PhysX_CQS::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxSphereGeometry ToPxGeometry(PxTransform& pose, const PintSphereSweepData* sweeps)
		{
			return ToPxSphereGeometry(pose, sweeps->mSphere);
		}};
		return _BatchSweeps<PintSphereSweepData, PxSphereGeometry, IceToPx>(nb, dest, sweeps, GetSweepQueryFlags(mParams), GetSQFilterData(), GetSQFilterCallback());
	}

	return PhysX::BatchSphereSweeps(context, nb, dest, sweeps);
}

udword PhysX_CQS::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	if(gSceneQuerySystem)
	{
		struct IceToPx{static inline_ PxCapsuleGeometry ToPxGeometry(PxTransform& pose, const PintCapsuleSweepData* sweeps)
		{
			return ToPxCapsuleGeometry(pose, sweeps->mCapsule);
		}};
		return _BatchSweeps<PintCapsuleSweepData, PxCapsuleGeometry, IceToPx>(nb, dest, sweeps, GetSweepQueryFlags(mParams), GetSQFilterData(), GetSQFilterCallback());
	}

	return PhysX::BatchCapsuleSweeps(context, nb, dest, sweeps);
}

udword PhysX_CQS::BatchConvexSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)
{
	if(gSceneQuerySystem)
	{
		ASSERT(mScene);

		const PxQueryFilterData PF = GetSQFilterData();
		const PxSceneQueryFlags sweepQueryFlags = GetSweepQueryFlags(mParams);
		PxQueryFilterCallback* FCB = GetSQFilterCallback();

		PxConvexMeshGeometry Geom;

		udword NbHits = 0;
		while(nb--)
		{
			const PxTransform Pose(ToPxVec3(sweeps->mTransform.mPos), ToPxQuat(sweeps->mTransform.mRot));

			Geom.convexMesh = reinterpret_cast<PxConvexMesh*>(mConvexObjects.mObjects[sweeps->mConvexObjectIndex]);

			PxSweepHit Hit;
			if(PEEL_SweepClosest(Geom, Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, Hit, sweepQueryFlags, PF, FCB))
			{
				NbHits++;
				FillResultStruct(*dest, Hit);
			}
			else
			{
				dest->SetNoHit();
			}

			sweeps++;
			dest++;
		}
		return NbHits;
	}

	return PhysX::BatchConvexSweeps(context, nb, dest, sweeps);
}

ICE_COMPILE_TIME_ASSERT(sizeof(PxGeomIndexPair)==sizeof(udword)*2);

extern bool gTest;

udword PhysX_CQS::FindTriangles_MeshMeshOverlap(PintSQThreadContext context, PintActorHandle handle0, PintActorHandle handle1, Container& results)
{
	const PxRigidActor* Actor0 = reinterpret_cast<const PxRigidActor*>(handle0);
	const PxRigidActor* Actor1 = reinterpret_cast<const PxRigidActor*>(handle1);

	PX_ASSERT(Actor0 && Actor0->getNbShapes()==1);
	PX_ASSERT(Actor1 && Actor1->getNbShapes()==1);

	PxShape* Shape0 = null;
	Actor0->getShapes(&Shape0, 1, 0);
	PX_ASSERT(Shape0);

	PxShape* Shape1 = null;
	Actor1->getShapes(&Shape1, 1, 0);
	PX_ASSERT(Shape1);

	PxTriangleMeshGeometry meshGeom0;
	Shape0->getTriangleMeshGeometry(meshGeom0);

	PxTriangleMeshGeometry meshGeom1;
	Shape1->getTriangleMeshGeometry(meshGeom1);

	if(gTest)
	{
		meshGeom0.scale.scale = PxVec3(1.00001f);
		meshGeom1.scale.scale = PxVec3(1.00001f);
	}

	const PxTransform meshPose0 = PxShapeExt::getGlobalPose(*Shape0, *Actor0);
	const PxTransform meshPose1 = PxShapeExt::getGlobalPose(*Shape1, *Actor1);

	results.Reset();

	class MyReportCallback : public PxReportCallback<PxGeomIndexPair>
	{
		Container& mResults;

		public:
		MyReportCallback(Container& results) : mResults(results)
		{
			mCapacity = 256;
			//mCapacity = 1;
		}

		virtual	bool	flushResults(PxU32 nbItems, const PxGeomIndexPair* items)
		{
			udword* dst = mResults.Reserve(nbItems*2);
			CopyMemory(dst, items, nbItems*2*sizeof(udword));
			return true;
			//return false;
		}
	};

	MyReportCallback cb(results);
	if(!PxMeshQuery::findOverlapTriangleMesh(cb, meshGeom0, meshPose0, meshGeom1, meshPose1, PxGeometryQueryFlags(0)))
		return 0;
	return results.GetNbEntries()/2;
}


class MyUICallback : public UICallback
{
	public:
	virtual	void			UIModificationCallback()
	{
		if(gPhysX)
			gPhysX->UpdateFromUI();
	}

}gUICallback;

void PhysX_Init(const PINT_WORLD_CREATE& desc)
{
	PhysX3::GetOptionsFromGUI(desc.GetTestName());

	PhysX3_InitFilterShader();

	ASSERT(!gPhysX);
	gPhysX = ICE_NEW(PhysX_CQS)(PhysX3::GetEditableParams());
	gPhysX->Init(desc);
}

void PhysX_Close()
{
	if(gPhysX)
	{
		gPhysX->Close();
		delete gPhysX;
		gPhysX = null;
	}
}

PhysX* GetPhysX()
{
	return gPhysX;
}

///////////////////////////////////////////////////////////////////////////////

IceWindow* PhysX_InitGUI(IceWidget* parent, PintGUIHelper& helper)
{
	return PhysX3::InitSharedGUI(parent, helper, gUICallback);
}

void PhysX_CloseGUI()
{
	PhysX3::CloseSharedGUI();
}

///////////////////////////////////////////////////////////////////////////////

#include "..\PINT_Common\PINT_CommonPhysX3_PerTestUI.h"

IceWindow* PhysXPlugIn::InitGUI(IceWidget* parent, PintGUIHelper& helper)	{ return PhysX_InitGUI(parent, helper);	}
void PhysXPlugIn::CloseGUI()												{ PhysX_CloseGUI();						}
void PhysXPlugIn::Init(const PINT_WORLD_CREATE& desc)						{ PhysX_Init(desc);						}
void PhysXPlugIn::Close()													{ PhysX_Close();						}
Pint* PhysXPlugIn::GetPint()												{ return GetPhysX();					}
const char*	PhysXPlugIn::GetTestGUIName()									{ return "PhysX 5.1i";					}
static PhysXPlugIn gPlugIn;
PintPlugin*	GetPintPlugin()													{ return &gPlugIn;						}

