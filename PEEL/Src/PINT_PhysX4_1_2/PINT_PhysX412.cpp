///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_PhysX412.h"
#include "..\PINT_Common\PINT_Common.h"
#include "..\PINT_Common\PINT_CommonPhysX3_Setup.h"

#include "extensions\PxExtensionsAPI.h"
//#include "common/PxIO.h"
#include "common/PxRenderBuffer.h"
//#include "physxprofilesdk/PxProfileZoneManager.h"
//#include "physxprofilesdk/PxProfileSDK.h"
//#include "NvProfileZoneManager.h"

#include "PhysX5\PhysX5_ExtDefaultCpuDispatcher.h"


//#define MODIFY_CONTACTS

//..\..\..\PEEL_Externals\PhysX-3.4_Trunk\Lib\vc11win32\PhysX3Extensions.lib

//#define USE_LOAD_LIBRARY

//#define USE_FORCE_THRESHOLD

static PhysX* gPhysX = null;

///////////////////////////////////////////////////////////////////////////////

// TODO: the API has completely changed :(   ...make this work again
#define	PVD_HOST	"127.0.0.1"
//#define SUPPORT_PVD
//#define SUPPORT_PVD2

/*
PHYSX_PROFILE_SDK
PX_DEBUG=1
PX_CHECKED=1
PX_SUPPORT_PVD=1
*/

#ifdef SUPPORT_PVD

#include "pvd\PxPvd.h"

using namespace physx::debugger;
using namespace physx::debugger::comm;

//#include "PvdConnection.h"
//#include "PvdConnectionManager.h"

class PVDHelper : public PvdConnectionHandler //receive notifications when pvd is connected and disconnected.
{
public:
						PVDHelper(PxPhysics* physics) : mPhysics(physics)	{}

	// PvdConnectionHandler
	virtual	void		onPvdSendClassDescriptions(PvdConnection&) {}
	virtual	void		onPvdConnected(PvdConnection& inFactory);
	virtual	void		onPvdDisconnected(PvdConnection& inFactory);
	//~PvdConnectionHandler

			void		togglePvdConnection();
			void		createPvdConnection();

			PxPhysics*	mPhysics;
};

void PVDHelper::togglePvdConnection()
{
	PvdConnectionManager* pvd = mPhysics->getPvdConnectionManager();
	if(!pvd)
		return;

	if(pvd->isConnected())
		pvd->disconnect();
	else
		createPvdConnection();
}

void PVDHelper::createPvdConnection()
{
	PvdConnectionManager* pvd = mPhysics->getPvdConnectionManager();
	if(!pvd)
		return;

/*	if(0)
	{
		PxDebuggerConnectionFlags PDebuggerFlags;
		PDebuggerFlags |= PxExtensionConnectionType::Debug;
		PDebuggerFlags |= PxExtensionConnectionType::Profile;
		PDebuggerFlags |= PxExtensionConnectionType::Memory;

		PxExtensionVisualDebugger::connect( 
						pvd,
						PVD_HOST,
						5425,
						3000,
						PDebuggerFlags
						);
		return;
	}*/

	//The connection flags state overall what data is to be sent to PVD.  Currently
	//the Debug connection flag requires support from the implementation (don't send
	//the data when debug isn't set) but the other two flags, profile and memory
	//are taken care of by the PVD SDK.

	//Use these flags for a clean profile trace with minimal overhead
	//TConnectionFlagsType theConnectionFlags( PvdConnectionType::Profile )
//	PxVisualDebuggerConnectionFlags theConnectionFlags( PxVisualDebuggerExt::getAllConnectionFlags() );
//	PxVisualDebuggerConnectionFlags theConnectionFlags( PxVisualDebuggerConnectionFlags( PxVisualDebuggerConnectionFlag::Profile|PxVisualDebuggerConnectionFlag::Memory ) );
//	if(!gUseFullPvdConnection)
//		theConnectionFlags = PxVisualDebuggerConnectionFlag::Profile;

	PxVisualDebuggerConnectionFlags theConnectionFlags;
	if(gUseFullPvdConnection)
		theConnectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();
	else
		theConnectionFlags = PxVisualDebuggerConnectionFlag::ePROFILE;

	//Create a pvd connection that writes data straight to the filesystem.  This is
	//the fastest connection on windows for various reasons.  First, the transport is quite fast as
	//pvd writes data in blocks and filesystems work well with that abstraction.
	//Second, you don't have the PVD application parsing data and using CPU and memory bandwidth
	//while your application is running.
	//PxExtensionVisualDebugger::connect(pvd,"c:\\temp.pxd2", PxDebuggerConnectionFlags( (PxU32)theConnectionFlags));
	
	//The normal way to connect to pvd.  PVD needs to be running at the time this function is called.
	//We don't worry about the return value because we are already registered as a listener for connections
	//and thus our onPvdConnected call will take care of setting up our basic connection state.
/*	PVD::PvdConnection* theConnection = PxVisualDebuggerExt::createConnection(mPhysics->getPvdConnectionManager(), PVD_HOST, 5425, 10, theConnectionFlags );
	if(theConnection)
		theConnection->release();*/
	PxVisualDebuggerExt::createConnection(pvd, PVD_HOST, 5425, 10, theConnectionFlags);
}

void PVDHelper::onPvdConnected(PvdConnection& )
{
	//setup joint visualization.  This gets piped to pvd.
//	mPhysics->getVisualDebugger()->setVisualizeConstraints(true);
	mPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONTACTS, true);
	mPhysics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS, true);
}

void PVDHelper::onPvdDisconnected(PvdConnection& )
{
}
#endif

///////////////////////////////////////////////////////////////////////////////

//static PxDefaultAllocator* gDefaultAllocator = null;
//static PxDefaultErrorCallback* gDefaultErrorCallback = null;

/*static*/ PEEL_PhysX3_AllocatorCallback* gDefaultAllocator = null;
//static PxAllocatorCallback* gDefaultAllocator = null;
/*static*/ PxErrorCallback* gDefaultErrorCallback = null;

static PxCpuDispatcher* gDefaultCPUDispatcher = null;
#ifdef SUPPORT_PVD
static PVDHelper* gPVDHelper = null;
#endif

#ifdef PHYSX_SUPPORT_GPU
static PxCudaContextManager* gCudaContextManager = NULL;
#endif

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
	return _F("PhysX 4.1.2 (%dT)", Suffix);
}

const char* PhysX::GetUIName() const
{
	return "PhysX 4.1.2";
}

void PhysX::GetCaps(PintCaps& caps) const
{
	caps.mSupportContactNotifications	= true;
	caps.mSupportContactModifications	= true;
	caps.mSupportRigidBodySimulation	= true;
	caps.mSupportMassForInertia			= true;
	caps.mSupportKinematics				= true;
	caps.mSupportCollisionGroups		= true;
	caps.mSupportCompounds				= true;
	caps.mSupportConvexes				= true;
	caps.mSupportMeshes					= true;
	caps.mSupportDeformableMeshes		= true;
	caps.mSupportHeightfields			= true;
	caps.mSupportAggregates				= true;
	caps.mSupportMCArticulations		= true;
	caps.mSupportRCArticulations		= true;
	caps.mSupportSphericalJoints		= true;
	caps.mSupportHingeJoints			= true;
	caps.mSupportFixedJoints			= true;
	caps.mSupportPrismaticJoints		= true;
	caps.mSupportDistanceJoints			= true;
	caps.mSupportD6Joints				= true;
	caps.mSupportGearJoints				= true;
	caps.mSupportRackJoints				= true;
	caps.mSupportPortalJoints			= true;
	caps.mSupportRaycasts				= true;
	caps.mSupportBoxSweeps				= true;
	caps.mSupportSphereSweeps			= true;
	caps.mSupportCapsuleSweeps			= true;
	caps.mSupportConvexSweeps			= true;
	caps.mSupportSphereOverlaps			= true;
	caps.mSupportBoxOverlaps			= true;
	caps.mSupportCapsuleOverlaps		= true;
	caps.mSupportConvexOverlaps			= true;
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
typedef physx::PxPhysics*	(*PxFunction_CreateBasePhysics)	(physx::PxU32 version, physx::PxFoundation& foundation, const physx::PxTolerancesScale& scale, bool trackOutstandingAllocations, physx::PxProfileZoneManager* profileZoneManager);
typedef void				(*PxFunction_RegisterPCM)		(physx::PxPhysics& physics);
typedef physx::PxCooking*	(*PxFunction_CreateCooking)		(physx::PxU32 version, physx::PxFoundation& foundation, const physx::PxCookingParams& params);
typedef physx::PxFoundation*(*PxFunction_CreateFoundation)	(physx::PxU32 version, physx::PxAllocatorCallback& allocator, physx::PxErrorCallback& errorCallback);
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
{
#ifdef USE_LOAD_LIBRARY
	udword FPUEnv[256];
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
	}
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
	mFoundation = (func3)(PX_PHYSICS_VERSION, *gDefaultAllocator, *gDefaultErrorCallback);
#else
//	mFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, *gDefaultAllocator, *gDefaultErrorCallback);
	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *gDefaultAllocator, *gDefaultErrorCallback);
#endif
	ASSERT(mFoundation);

//	nvidia::pvdsdk::NvPvd* PvdSDK = null;
#ifdef SUPPORT_PVD
	if(gUsePVD)
	{
		ASSERT(!mProfileZoneManager);
		mProfileZoneManager = &PxProfileZoneManager::createProfileZoneManager(mFoundation);
		ASSERT(mProfileZoneManager);
	}
#endif

	PxPvd* pvd = null;
#ifdef SUPPORT_PVD2
	pvd = PxCreatePvd(*mFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	pvd->connect(*transport,PxPvdInstrumentationFlag::eALL);
	mPVD = pvd;
	mTransport = transport;
#endif

	PxTolerancesScale scale;
	{
		ASSERT(!mPhysics);
	#ifdef USE_LOAD_LIBRARY
		mPhysics = (func0)(PX_PHYSICS_VERSION, *mFoundation, scale, false, mProfileZoneManager);
	//	PxRegisterArticulations(*mPhysics);
	//	PxRegisterHeightFields(*mPhysics);
		(func1)(*mPhysics);
	#else
//		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, scale, false, mProfileZoneManager);
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, scale, false, pvd);
	#endif
		ASSERT(mPhysics);
	}

//	bool status = PxInitExtensions(*mPhysics, PvdSDK);
	bool status = PxInitExtensions(*mPhysics, null);
	ASSERT(status);
	//gDefaultCPUDispatcher = PxDefaultCpuDispatcherCreate(mParams.ThreadIndexToNbThreads(mParams.mNbThreadsIndex), null);
	gDefaultCPUDispatcher = physx5::PxDefaultCpuDispatcherCreate(mParams.ThreadIndexToNbThreads(mParams.mNbThreadsIndex), null, physx5::PxDefaultCpuDispatcherWaitForWorkMode::eYIELD_THREAD);

	CreateCooking(scale, PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES));

#ifdef SUPPORT_PVD
	if(gUsePVD)
	{
		gPVDHelper = new PVDHelper(mPhysics);

		gPVDHelper->togglePvdConnection();

		if(mPhysics->getPvdConnectionManager())
			mPhysics->getPvdConnectionManager()->addHandler(*gPVDHelper);
	}
#endif

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
			sceneDesc.gpuDynamicsConfig.constraintBufferCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.contactBufferCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.tempBufferCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.contactStreamSize *= 2;
			sceneDesc.gpuDynamicsConfig.patchStreamSize *= 2;
			sceneDesc.gpuDynamicsConfig.forceStreamCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.heapCapacity *= 2;
			sceneDesc.gpuDynamicsConfig.foundLostPairsCapacity *= 2;

			PxCudaContextManagerDesc cudaContextManagerDesc;
			cudaContextManagerDesc.interopMode = PxCudaInteropMode::OGL_INTEROP;
			gCudaContextManager = PxCreateCudaContextManager(*mFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
			if(gCudaContextManager)
			{
				if(!gCudaContextManager->contextIsValid())
				{
					gCudaContextManager->release();
					gCudaContextManager = NULL;
				}
			}	
//			if(gCudaContextManager)
//				sceneDesc.gpuDispatcher = gCudaContextManager->getGpuDispatcher();	//Set the GPU dispatcher, used by GRB to dispatch CUDA kernels.
				sceneDesc.cudaContextManager = gCudaContextManager;
		}
#endif

		mScene = mPhysics->createScene(sceneDesc);
		ASSERT(mScene);
	}

	SetupBroadphase(desc, mParams, mScene);

	InitCommon();
	UpdateFromUI();
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
	gSimulationEventCallback.Release();

	CloseCommon();
	CloseVehicles();
#ifdef PHYSX_SUPPORT_CHARACTERS
	ReleaseControllerManager();
#endif
#ifdef PHYSX_SUPPORT_CHARACTERS2
	mCCT.ReleaseControllerManager();
#endif

#ifdef SUPPORT_PVD
	if(gPVDHelper)
		gPVDHelper->togglePvdConnection();
	DELETESINGLE(gPVDHelper);
#endif

	SAFE_RELEASE(mDefaultMaterial)
	SAFE_RELEASE(mScene)
	SAFE_RELEASE(mCooking)
	DELETESINGLE(gDefaultCPUDispatcher);

	PxCloseExtensions();

	SAFE_RELEASE(mPhysics)
#ifdef PHYSX_SUPPORT_GPU
	SAFE_RELEASE(gCudaContextManager);
#endif
//	SAFE_RELEASE(mProfileZoneManager)
#ifdef SUPPORT_PVD2
	SAFE_RELEASE(mPVD);
	SAFE_RELEASE(mTransport);
#endif

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
	return Point(238.0f/255.0f, 223.0f/255.0f, 204.0f/255.0f);
}

///////////////////////////////////////////////////////////////////////////////

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
	gPhysX = ICE_NEW(PhysX)(PhysX3::GetEditableParams());
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
const char*	PhysXPlugIn::GetTestGUIName()									{ return "PhysX 4.1.2";					}
static PhysXPlugIn gPlugIn;
PintPlugin*	GetPintPlugin()													{ return &gPlugIn;						}

