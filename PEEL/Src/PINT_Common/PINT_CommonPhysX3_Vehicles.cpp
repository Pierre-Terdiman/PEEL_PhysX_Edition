///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_Vehicles.h"
#include "PINT_CommonPhysX3_DebugViz.h"
#if PHYSX_SUPPORT_VEHICLE5
	#include "PINT_CommonPhysX5_Vehicles.h"
#endif
#include "PINT_CommonPhysX5_Fluid.h"

#include "vehicle/PxVehicleSDK.h"
#include "PxFiltering.h"

#include "vehicle/PxVehicleDrive4W.h"
#include "vehicle/PxVehicleUtilControl.h"
#include "vehicle/PxVehicleUtil.h"

#include "vehicle/PxVehicleUpdate.h"
#include "vehicle/PxVehicleTireFriction.h"
#include "vehicle/PxVehicleUtilSetup.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"
#include "geometry/PxConvexMesh.h"
#include "geometry/PxConvexMeshGeometry.h"

//#define DISABLE_VEHICLE_SQ_VS_DYNAMICS

#if PHYSX_DEPRECATED_CREATE_SHAPE
	#include "PxRigidActorExt.h"
#endif

#ifdef TEST_FLUIDS
extern PxPBDParticleSystem* gParticleSystem;
#endif

///////////////////////////////////

using namespace PhysX3;
using namespace physx;

static const char* gChassisDebugName = "Chassis";
static const char* gWheelDebugName = "Wheel";

static const bool gUpdateVehicles = true;
//static const bool gSetupVehicleContactModif = false;
#ifdef PHYSX_SUPPORT_VEHICLE_XP
static const bool gSetupCollisionGroups = false;
extern bool gVehicleXP;
#else
static const bool gSetupCollisionGroups = true;
#endif

#ifdef PHYSX_SUPPORT_VEHICLE_XP
	static const bool gLogXP = false;
#else
	static const bool gLogXP = false;
#endif

static const PintCollisionGroup gChassisCollisionGroup = 0;
static const PintCollisionGroup gWheelCollisionGroup = 31;

extern PEEL_PhysX3_AllocatorCallback* gDefaultAllocator;
extern PxErrorCallback* gDefaultErrorCallback;

static Vertices* gDebugPts = null;
static Vertices* gDebugSegments = null;
static Vertices* gDebugSegments2 = null;

static const PxU32 mPlayerVehicle=0;

#ifdef PHYSX_SUPPORT_VEHICLE_FIX
extern bool gUseFix;
#endif

///////////////////////////////////////////////////////////////////////////////

SampleVehicleWheelQueryResults* SampleVehicleWheelQueryResults::allocate(const PxU32 maxNumWheels)
{
	const PxU32 size = sizeof(SampleVehicleWheelQueryResults) + sizeof(PxWheelQueryResult)*maxNumWheels;
//	SampleVehicleWheelQueryResults* resData = (SampleVehicleWheelQueryResults*)PX_ALLOC(size, PX_DEBUG_EXP("SampleVehicleWheelQueryResults"));
	SampleVehicleWheelQueryResults* resData = (SampleVehicleWheelQueryResults*)gDefaultAllocator->allocate(size, "SampleVehicleWheelQueryResults", __FILE__, __LINE__);
	resData->init();
	PxU8* ptr = (PxU8*) resData;
	ptr += sizeof(SampleVehicleWheelQueryResults);
	resData->mWheelQueryResults = (PxWheelQueryResult*)ptr;
	ptr +=  sizeof(PxWheelQueryResult)*maxNumWheels;
	resData->mMaxNumWheels=maxNumWheels;
	for(PxU32 i=0;i<maxNumWheels;i++)
	{
		new(&resData->mWheelQueryResults[i]) PxWheelQueryResult();
	}
	return resData;
}

void SampleVehicleWheelQueryResults::free()
{
//	PX_FREE(this);
	gDefaultAllocator->deallocate(this);
}

PxWheelQueryResult* SampleVehicleWheelQueryResults::addVehicle(const PxU32 numWheels)
{
	PX_ASSERT((mNumWheels + numWheels) <= mMaxNumWheels);
	PxWheelQueryResult* r = &mWheelQueryResults[mNumWheels];
	mNumWheels += numWheels;
	return r;
}

///////////////////////////////////////////////////////////////////////////////

#define SIZEALIGN16(size) (((unsigned)(size)+15)&((unsigned)(~15)));

#if PHYSX_SUPPORT_VEHICLE5
#else
SampleVehicleSceneQueryData* SampleVehicleSceneQueryData::allocate(const PxU32 maxNumWheels, const bool use_sweeps)
{
	const PxU32 size0 = SIZEALIGN16(sizeof(SampleVehicleSceneQueryData));
	PxU32 size1, size2;
	if(use_sweeps)
	{
		//#### isn't this wrong? NB_SWEEP_HITS_PER_WHEEL should be on size2?
		size1 = SIZEALIGN16(sizeof(PxSweepQueryResult)*maxNumWheels*NB_SWEEP_HITS_PER_WHEEL);
		size2 = SIZEALIGN16(sizeof(PxSweepHit)*maxNumWheels);
//		size1 = SIZEALIGN16(sizeof(PxSweepQueryResult)*maxNumWheels);
//		size2 = SIZEALIGN16(sizeof(PxSweepHit)*maxNumWheels*NB_SWEEP_HITS_PER_WHEEL);
	}
	else
	{
		size1 = SIZEALIGN16(sizeof(PxRaycastQueryResult)*maxNumWheels);
		size2 = SIZEALIGN16(sizeof(PxRaycastHit)*maxNumWheels);
	}
	const PxU32 size = size0 + size1 + size2;
//	SampleVehicleSceneQueryData* sqData = (SampleVehicleSceneQueryData*)PX_ALLOC(size, "PxVehicleNWSceneQueryData");
	SampleVehicleSceneQueryData* sqData = (SampleVehicleSceneQueryData*)gDefaultAllocator->allocate(size, "PxVehicleNWSceneQueryData", __FILE__, __LINE__);
	PxU8* ptr = (PxU8*) sqData;
	ptr += size0;
	if(use_sweeps)
	{
		sqData->mSweepSqResults = (PxSweepQueryResult*)ptr;
		for(PxU32 i=0;i<maxNumWheels;i++)
		{
			sqData->mSweepSqResults[i].nbTouches = 0;
			sqData->mSweepSqResults[i].touches = null;
		}
	}
	else
	{
		sqData->mRaycastSqResults = (PxRaycastQueryResult*)ptr;
		for(PxU32 i=0;i<maxNumWheels;i++)
		{
			sqData->mRaycastSqResults[i].nbTouches = 0;
			sqData->mRaycastSqResults[i].touches = null;
		}
	}
	sqData->mNbSqResults = maxNumWheels*NB_SWEEP_HITS_PER_WHEEL;
	ptr += size1;

	if(use_sweeps)
		sqData->mSweepSqHitBuffer = (PxSweepHit*)ptr;
	else
		sqData->mRaycastSqHitBuffer = (PxRaycastHit*)ptr;

	ptr += size2;
	sqData->mNumQueries = maxNumWheels;
	return sqData;
}

void SampleVehicleSceneQueryData::free()
{
//	PX_FREE(this);
	gDefaultAllocator->deallocate(this);
}
#endif

static PxQueryHitType::Enum MySampleVehicleWheelRaycastPreFilter(	
	PxFilterData filterData0, 
	PxFilterData filterData1,
	const void* constantBlock, PxU32 constantBlockSize,
	PxHitFlags& queryFlags)
{
	//filterData0 is the vehicle suspension raycast.
	//filterData1 is the shape potentially hit by the raycast.
	PX_UNUSED(queryFlags);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);
	PX_UNUSED(filterData0);
//	return ((0 == (filterData1.word3 & SAMPLEVEHICLE_DRIVABLE_SURFACE)) ? PxQueryHitType::eNONE : PxQueryHitType::eBLOCK);
	return PxQueryHitType::eBLOCK;
}

static inline_ PxQueryHitType::Enum _preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags, bool useBlockingSweeps)
{
#ifdef DISABLE_VEHICLE_SQ_VS_DYNAMICS
	if(actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		return PxQueryHitType::eNONE;
#endif
	// ### TODO: consider abusing the name pointer as a second user-data pointer
	const char* shapeName = shape->getName();	// ### VCALL
	if(shapeName)
	{
		//if(gSetupCollisionGroups)
		{
			if(shapeName==gChassisDebugName || shapeName==gWheelDebugName)
				return PxQueryHitType::eNONE;
		}
		/*else
		{
			// ### not great to call strncmp here
			if(shapeName==gChassisDebugName || strncmp(shapeName, gWheelDebugName, 5)==0)
				return PxQueryHitType::eNONE;
		}*/
	}

	return useBlockingSweeps ? PxQueryHitType::eBLOCK : PxQueryHitType::eTOUCH;
}

#if PHYSX_SUPPORT_VEHICLE5
class MyFilter : public PxQueryFilterCallback
{
public:

	MyFilter() : mUseBlockingSweeps(true)	{}

	bool	mUseBlockingSweeps;

	virtual PxQueryHitType::Enum preFilter(
		const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
	{
		return _preFilter(filterData, shape, actor, queryFlags, mUseBlockingSweeps);
	}

	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit)
	{
		// ### for some reason the post filter isn't called anymore in PhysX 5
		ASSERT(0);
		return PxQueryHitType::eNONE;
	}
}gMyFilter;
#else
class MyBatchQuery : public PxBatchQuery, public PxQueryFilterCallback
{
	public:

	virtual void	setRunOnSpu(bool runOnSpu)	{}
	virtual bool	getRunOnSpu()				{ return false;}

	virtual PxQueryHitType::Enum preFilter(
		const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
	{
		return _preFilter(filterData, shape, actor, queryFlags, mUseBlockingSweeps);
	}

	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit)
	{
		// In PhysX 4 and before the post filter is called for sweeps, but not for raycasts
		return mUseBlockingSweeps ? PxQueryHitType::eBLOCK : PxQueryHitType::eTOUCH;
	}

	MyBatchQuery() : mNb(0), mScene(null), mDesc(0, 0, 0), mUseBlockingSweeps(true)
	{
	}

	void init(PxScene* scene, const PxBatchQueryDesc& desc)
	{
		mScene = scene;
		mDesc = desc;
	}

	virtual	void	execute()
	{
		mNb = 0;
		mNbTouchHits = 0;
	}

	virtual	PxBatchQueryPreFilterShader getPreFilterShader() const
	{
		ASSERT(0);
		return null;
	}

	virtual	PxBatchQueryPostFilterShader getPostFilterShader() const
	{
		return null;
	}

	virtual	const void*	getFilterShaderData() const
	{
		ASSERT(0);
		return null;
	}

	virtual	PxU32	getFilterShaderDataSize() const
	{
		ASSERT(0);
		return 0;
	}

	virtual PxClientID	getOwnerClient() const
	{
		ASSERT(0);
		return 0;
	}

 	virtual	void	setUserMemory(const PxBatchQueryMemory&)
	{
		ASSERT(0);
	}

 	virtual	const PxBatchQueryMemory&	getUserMemory()
	{
		//static PxBatchQueryMemory unused(0,0,0);
		//return unused;
		return mDesc.queryMemory;
	}

	virtual	void	release()
	{
		mRecorder.Release();
	}

	virtual void raycast(
		const PxVec3& origin, const PxVec3& unitDir, PxReal distance = PX_MAX_F32, PxU16 maxTouchHits = 0,
		PxHitFlags hitFlags = PxHitFlag::eDEFAULT,
		const PxQueryFilterData& filterData = PxQueryFilterData(),
		void* userData = NULL, const PxQueryCache* cache = NULL)
	{
		PxQueryFilterData filterDataCopy = filterData;
#ifdef DISABLE_VEHICLE_SQ_VS_DYNAMICS
		filterDataCopy.flags &= ~PxQueryFlag::eDYNAMIC;
#endif

		PxRaycastBuffer buf;
//		PxQueryFilterData fd1 = filterData; fd1.clientId = queryClient;
		mScene->raycast(origin, unitDir, distance, buf, hitFlags, filterDataCopy, this, cache);
//		hit = buf.block;
//		return buf.hasBlock;

		if(buf.hasBlock)
		{
			mDesc.queryMemory.userRaycastResultBuffer[mNb].block = buf.block;
			mDesc.queryMemory.userRaycastResultBuffer[mNb].hasBlock = true;
		}
		else
			mDesc.queryMemory.userRaycastResultBuffer[mNb].hasBlock = false;

		mRecorder.RecordRaycast(origin, unitDir, distance, mNb++);
	}

	// For 3.3.0
	virtual void raycast(
		const PxVec3& origin, const PxVec3& unitDir, PxReal distance = PX_MAX_F32, PxU16 maxTouchHits = 0,
#if PHYSX_DEPRECATED_DISTANCE
		PxHitFlags hitFlags = PxHitFlag::ePOSITION|PxHitFlag::eNORMAL,
#else
		PxHitFlags hitFlags = PxHitFlag::ePOSITION|PxHitFlag::eNORMAL|PxHitFlag::eDISTANCE,
#endif
		const PxQueryFilterData& filterData = PxQueryFilterData(),
		void* userData = NULL, const PxQueryCache* cache = NULL) const
	{
		const_cast<MyBatchQuery*>(this)->raycast(origin, unitDir, distance, maxTouchHits, hitFlags, filterData, userData, cache);
	}

	virtual void overlap(
		const PxGeometry& geometry, const PxTransform& pose, PxU16 maxTouchHits = 0,
		const PxQueryFilterData& filterData = PxQueryFilterData(), void* userData=NULL, const PxQueryCache* cache = NULL) const
	{
		ASSERT(0);
	}

	virtual void overlap(
		const PxGeometry& geometry, const PxTransform& pose, PxU16 maxTouchHits = 0,
		const PxQueryFilterData& filterData = PxQueryFilterData(), void* userData=NULL, const PxQueryCache* cache = NULL)
	{
		ASSERT(0);
	}

	virtual void sweep(
		const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
		PxU16 maxTouchHits, PxHitFlags hitFlags,
		const PxQueryFilterData& filterData, void* userData, const PxQueryCache* cache,
		const PxReal inflation)
	{
#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
		PxQueryFilterData filterDataCopy = filterData;
#ifdef DISABLE_VEHICLE_SQ_VS_DYNAMICS
		filterDataCopy.flags &= ~PxQueryFlag::eDYNAMIC;
#endif

		if(mUseBlockingSweeps)
		{
			PxSweepHit hit;
			if(sweepSingle(mScene, geometry, pose, unitDir, distance, hitFlags, hit, filterDataCopy, this, cache, PX_DEFAULT_CLIENT, inflation))
			{
//				if(hit.distance==0.0f)
//					hit.distance = FLT_EPSILON;

				mDesc.queryMemory.userSweepResultBuffer[mNb].block = hit;
				mDesc.queryMemory.userSweepResultBuffer[mNb].hasBlock = true;
				//printf("%f %f %f\n", hit.normal.x, hit.normal.y, hit.normal.z);
				//hit.normal = PxVec3(0.0f, 1.0f, 0.0f);
			}
			else
				mDesc.queryMemory.userSweepResultBuffer[mNb].hasBlock = false;
		}
		else		
		{
			bool blockingHit;
			PxSweepHit hits[NB_SWEEP_HITS_PER_WHEEL];

			PxI32 nbHits = sweepMultiple(mScene, geometry, pose, unitDir, distance, hitFlags, hits, NB_SWEEP_HITS_PER_WHEEL, blockingHit, filterDataCopy, this, cache, PX_DEFAULT_CLIENT, inflation);
			ASSERT(!blockingHit);
			mDesc.queryMemory.userSweepResultBuffer[mNb].hasBlock = false;
			mDesc.queryMemory.userSweepResultBuffer[mNb].nbTouches = nbHits;
			//mDesc.queryMemory.userSweepResultBuffer[mNb].touches = gSqData->mSweepSqHitBuffer[mNb*NB_SWEEP_HITS_PER_WHEEL];

			mDesc.queryMemory.userSweepResultBuffer[mNb].touches = &mTouchHits[mNbTouchHits];
			mNbTouchHits += nbHits;
			ASSERT(mNbTouchHits<100);

			for(PxI32 i=0;i<nbHits;i++)
			{
				mDesc.queryMemory.userSweepResultBuffer[mNb].touches[i] = hits[i];
			}
		}

		mRecorder.RecordSweep(geometry, pose, unitDir, distance, mNb++);
#endif
	}

	// For 3.3.0
	virtual void sweep(
		const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
		PxU16 maxTouchHits = 0,
#if PHYSX_DEPRECATED_DISTANCE
		PxHitFlags hitFlags = PxHitFlag::ePOSITION|PxHitFlag::eNORMAL,
#else
		PxHitFlags hitFlags = PxHitFlag::ePOSITION|PxHitFlag::eNORMAL|PxHitFlag::eDISTANCE,
#endif
		const PxQueryFilterData& filterData = PxQueryFilterData(), void* userData=NULL, const PxQueryCache* cache = NULL,
		const PxReal inflation = 0.f) const
	{
		const_cast<MyBatchQuery*>(this)->sweep(geometry, pose, unitDir, distance, maxTouchHits, hitFlags, filterData, userData, cache, inflation);
	}

	udword				mNb;
	udword				mNbTouchHits;
	PxScene*			mScene;
	PxBatchQueryDesc	mDesc;

	// Keep track of casts for debug viz
	SQRecorder			mRecorder;

	PxSweepHit			mTouchHits[100];

	bool				mUseBlockingSweeps;

}gMyBatchQuery;

PxBatchQuery* SampleVehicleSceneQueryData::setUpBatchedSceneQuery(PxScene* scene, const bool use_sweeps)
{
	if(use_sweeps)
	{
		PxBatchQueryDesc sqDesc(0, mNbSqResults, 0);
		sqDesc.queryMemory.userSweepResultBuffer = mSweepSqResults;
		sqDesc.queryMemory.userSweepTouchBuffer = mSweepSqHitBuffer;
		sqDesc.queryMemory.sweepTouchBufferSize = mNumQueries*NB_SWEEP_HITS_PER_WHEEL;
		sqDesc.preFilterShader = MySampleVehicleWheelRaycastPreFilter;
		gMyBatchQuery.init(scene, sqDesc);
	}
	else
	{
		PxBatchQueryDesc sqDesc(mNbSqResults, 0, 0);
		sqDesc.queryMemory.userRaycastResultBuffer = mRaycastSqResults;
		sqDesc.queryMemory.userRaycastTouchBuffer = mRaycastSqHitBuffer;
		sqDesc.queryMemory.raycastTouchBufferSize = mNumQueries;
		sqDesc.preFilterShader = MySampleVehicleWheelRaycastPreFilter;
		gMyBatchQuery.init(scene, sqDesc);
	}

	return &gMyBatchQuery;
//	return scene->createBatchQuery(sqDesc);
}
#endif

///////////////////////////////////////////////////////////////////////////////

static bool gUseKeyboard = true;

static /*const*/ PxVehicleKeySmoothingData gKeySmoothingData=
{
	{
		3.0f,	//rise rate eANALOG_INPUT_ACCEL
		3.0f,	//rise rate eANALOG_INPUT_BRAKE
		10.0f,	//rise rate eANALOG_INPUT_HANDBRAKE
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
//		0.25f,	//rise rate eANALOG_INPUT_STEER_LEFT
//		0.25f,	//rise rate eANALOG_INPUT_STEER_RIGHT
	},
	{
		5.0f,	//fall rate eANALOG_INPUT__ACCEL
		5.0f,	//fall rate eANALOG_INPUT__BRAKE
		10.0f,	//fall rate eANALOG_INPUT__HANDBRAKE
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
//		0.5f,	//fall rate eANALOG_INPUT_STEER_LEFT
//		0.5f	//fall rate eANALOG_INPUT_STEER_RIGHT
	}
};

static /*const*/ PxVehiclePadSmoothingData gCarPadSmoothingData=
{
	{
		6.0f,	//rise rate eANALOG_INPUT_ACCEL		
		6.0f,	//rise rate eANALOG_INPUT_BRAKE		
		12.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT	
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT	
	},
	{
		10.0f,	//fall rate eANALOG_INPUT_ACCEL		
		10.0f,	//fall rate eANALOG_INPUT_BRAKE		
		12.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT	
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT	
	}
};

// Warning: the table entries are in m/s (and internally compared to "computeForwardSpeed()")
static const PxF32 gDefaultSteerVsForwardSpeedData[2*8]=
{
	0.0f,		1.0f,
	5.0f,		0.75f,
	30.0f,		0.125f,
	120.0f,		0.1f,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32
};

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class VehicleData;

	class VehicleController
	{
	public:

		VehicleController();
		~VehicleController();

		void setCarKeyboardInputs(bool accel, bool brake, bool handbrake, bool steerleft, bool steerright, bool gearup, bool geardown)
		{
			mKeyPressedAccel		= accel;
			mKeyPressedBrake		= brake;
			mKeyPressedHandbrake	= handbrake;
			mKeyPressedSteerLeft	= steerleft;
			mKeyPressedSteerRight	= steerright;
			mKeyPressedGearUp		= gearup;
			mKeyPressedGearDown		= geardown;
		}

		void setCarGamepadInputs(PxF32 accel, PxF32 brake, PxF32 steer, bool gearup, bool geardown, bool handbrake)
		{
			mGamepadAccel			= accel;
			mGamepadCarBrake		= brake;
			mGamepadCarSteer		= steer;
			mGamepadGearup			= gearup;
			mGamepadGeardown		= geardown;
			mGamepadCarHandbrake	= handbrake;
		}

		void update(const PxF32 dtime, const PxVehicleWheelQueryResult& vehicleWheelQueryResults, PxVehicleWheels& focusVehicle, const VehicleData& data);

		void clear();

	private:

		bool			mKeyPressedAccel;
		bool			mKeyPressedGearUp;
		bool			mKeyPressedGearDown;
		bool			mKeyPressedBrake;
		bool			mKeyPressedHandbrake;
		bool			mKeyPressedSteerLeft;
		bool			mKeyPressedSteerRight;

		PxF32			mGamepadAccel;
		bool			mGamepadGearup;
		bool			mGamepadGeardown;
		PxF32			mGamepadCarBrake;
		PxF32			mGamepadCarSteer;
		bool			mGamepadCarHandbrake;

		//Auto-reverse mode.
		bool			mIsMovingForwardSlowly;
		bool			mInReverseMode;

		//Update 
		void processRawInputs(const PxF32 timestep, const bool useAutoGears, PxVehicleDrive4WRawInputData& rawInputData);
		void processAutoReverse(const VehicleData& data,
								const PxVehicleWheels& focusVehicle, const PxVehicleDriveDynData& driveDynData, const PxVehicleWheelQueryResult& vehicleWheelQueryResults,
								const PxVehicleDrive4WRawInputData& rawInputData, bool& toggleAutoReverse, bool& newIsMovingForwardSlowly) const;
	};
}

namespace
{
	class VehicleData : public Allocateable
	{
		public:
			VehicleData() :
				mVehicle				(null),
				mThresholdForwardSpeed	(0.1f),
				mImmediateAutoReverse	(false)
			{}
			~VehicleData()
			{}
		PxVehicleDrive4W*		mVehicle;
		PINT_VEHICLE_INPUT		mInput;
#if PHYSX_SUPPORT_STEER_FILTER
		PxVehicleSteerFilter	mSteerFilter;
#endif
		VehicleController		mVehicleController;
		float					mSteerVsForwardSpeedData[2*8];
		float					mThresholdForwardSpeed;
		bool					mImmediateAutoReverse;
	};
}

VehicleController::VehicleController()
{
	clear();
}

VehicleController::~VehicleController()
{
}

void VehicleController::clear()
{
	mKeyPressedAccel		= false;
	mKeyPressedGearUp		= false;
	mKeyPressedGearDown		= false;

	mKeyPressedBrake		= false;
	mKeyPressedHandbrake	= false;
	mKeyPressedSteerLeft	= false;
	mKeyPressedSteerRight	= false;

	mGamepadAccel			= 0.0f;
	mGamepadGearup			= false;			
	mGamepadGeardown		= false;

	mGamepadCarBrake		= 0.0f;
	mGamepadCarSteer		= 0.0f;
	mGamepadCarHandbrake	= false;

	mIsMovingForwardSlowly	= true;
	mInReverseMode			= false;
}

void VehicleController::processRawInputs(const PxF32 dtime, const bool useAutoGears, PxVehicleDrive4WRawInputData& rawInputData)
{
	// Switch to keyboard use if a key is pressed, switch to gamepad use if the gamepad is used
	{
		const bool useKeyboard = (mKeyPressedAccel || mKeyPressedBrake || mKeyPressedHandbrake || mKeyPressedSteerLeft || mKeyPressedSteerRight || mKeyPressedGearUp || mKeyPressedGearDown);
		if(useKeyboard)
			gUseKeyboard = true;

		const bool useGamepad = ((mGamepadAccel+mGamepadCarBrake+mGamepadCarSteer)!=0.0f || mGamepadGearup || mGamepadGeardown || mGamepadCarHandbrake);
		if(useGamepad)
			gUseKeyboard = false;

		//printf("gUseKeyboard: %d\n", gUseKeyboard);
	}

	// Keyboard
	if(gUseKeyboard)
	{
		rawInputData.setDigitalAccel(mKeyPressedAccel);
		rawInputData.setDigitalBrake(mKeyPressedBrake);
		rawInputData.setDigitalHandbrake(mKeyPressedHandbrake);
		rawInputData.setDigitalSteerLeft(mKeyPressedSteerLeft);
		rawInputData.setDigitalSteerRight(mKeyPressedSteerRight);
		rawInputData.setGearUp(mKeyPressedGearUp);
		rawInputData.setGearDown(mKeyPressedGearDown);

		if(0)
		{
			printf("mKeyPressedAccel: %d\n", mKeyPressedAccel);
			printf("mKeyPressedBrake: %d\n", mKeyPressedBrake);
			printf("mKeyPressedHandbrake: %d\n", mKeyPressedHandbrake);
			printf("mKeyPressedSteerLeft: %d\n", mKeyPressedSteerLeft);
			printf("mKeyPressedSteerRight: %d\n", mKeyPressedSteerRight);
			printf("mKeyPressedGearUp: %d\n", mKeyPressedGearUp);
			printf("mKeyPressedGearDown: %d\n", mKeyPressedGearDown);
			printf("\n");
		}
	}
	else
	// Gamepad
	{
		if(mGamepadAccel<0.0f || mGamepadAccel>1.01f)
			//getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal accel value from gamepad", __FILE__, __LINE__);
			ASSERT(0);

		if(mGamepadCarBrake<0.0f || mGamepadCarBrake>1.01f)
//			getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal brake value from gamepad", __FILE__, __LINE__);
			ASSERT(0);

		if(PxAbs(mGamepadCarSteer)>1.01f)
//			getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal steer value from gamepad", __FILE__, __LINE__);
			ASSERT(0);

		rawInputData.setAnalogAccel(mGamepadAccel);
		rawInputData.setAnalogBrake(mGamepadCarBrake);
		rawInputData.setAnalogHandbrake(mGamepadCarHandbrake ? 1.0f : 0.0f);
		rawInputData.setAnalogSteer(mGamepadCarSteer);
		rawInputData.setGearUp(mGamepadGearup);
		rawInputData.setGearDown(mGamepadGeardown);

		if(0)
		{
			printf("mGamepadAccel: %f\n", mGamepadAccel);
			printf("mGamepadCarBrake: %f\n", mGamepadCarBrake);
			printf("mGamepadCarHandbrake: %d\n", mGamepadCarHandbrake);
			printf("mGamepadCarSteer: %f\n", mGamepadCarSteer);
			printf("\n");
		}
	}

	if(useAutoGears && (rawInputData.getGearDown() || rawInputData.getGearUp()))
	{
		rawInputData.setGearDown(false);
		rawInputData.setGearUp(false);
	}
}

//#define THRESHOLD_FORWARD_SPEED (0.1f)	// ###
//#define THRESHOLD_FORWARD_SPEED (5.0f)
#define THRESHOLD_SIDEWAYS_SPEED (0.2f)
#define THRESHOLD_ROLLING_BACKWARDS_SPEED (0.1f)

void VehicleController::processAutoReverse(	const VehicleData& data, const PxVehicleWheels& focusVehicle, const PxVehicleDriveDynData& driveDynData, const PxVehicleWheelQueryResult& vehicleWheelQueryResults,
											const PxVehicleDrive4WRawInputData& carRawInputs, bool& toggleAutoReverse, bool& newIsMovingForwardSlowly) const
{
	newIsMovingForwardSlowly = false;
	toggleAutoReverse = false;

	if(!driveDynData.getUseAutoGears())
		return;

//	printf("ThresholdForwardSpeed: %f\n", data.mThresholdForwardSpeed);
//	printf("mImmediateAutoReverse: %d\n", data.mImmediateAutoReverse);

	//If the car is travelling very slowly in forward gear without player input and the player subsequently presses the brake then we want the car to go into reverse gear
	//If the car is travelling very slowly in reverse gear without player input and the player subsequently presses the accel then we want the car to go into forward gear
	//If the car is in forward gear and is travelling backwards then we want to automatically put the car into reverse gear.
	//If the car is in reverse gear and is travelling forwards then we want to automatically put the car into forward gear.
	//(If the player brings the car to rest with the brake the player needs to release the brake then reapply it 
	//to indicate they want to toggle between forward and reverse.)

	const bool prevIsMovingForwardSlowly=mIsMovingForwardSlowly;
	bool isMovingForwardSlowly=false;
	bool isMovingBackwards=false;
	const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);
	if(!isInAir)
	{
		bool accelRaw,brakeRaw,handbrakeRaw;
		if(gUseKeyboard)
		{
			accelRaw=carRawInputs.getDigitalAccel();
			brakeRaw=carRawInputs.getDigitalBrake();
			handbrakeRaw=carRawInputs.getDigitalHandbrake();
		}
		else
		{
			accelRaw=carRawInputs.getAnalogAccel() > 0 ? true : false;
			brakeRaw=carRawInputs.getAnalogBrake() > 0 ? true : false;
			handbrakeRaw=carRawInputs.getAnalogHandbrake() > 0 ? true : false;
		}

		const PxF32 forwardSpeed = focusVehicle.computeForwardSpeed();
		const PxF32 forwardSpeedAbs = PxAbs(forwardSpeed);
		const PxF32 sidewaysSpeedAbs = PxAbs(focusVehicle.computeSidewaysSpeed());
		const PxU32 currentGear = driveDynData.getCurrentGear();
		const PxU32 targetGear = driveDynData.getTargetGear();

		//Check if the car is rolling against the gear (backwards in forward gear or forwards in reverse gear).
		if(PxVehicleGearsData::eFIRST == currentGear && forwardSpeed < -THRESHOLD_ROLLING_BACKWARDS_SPEED)
			isMovingBackwards = true;
		else if(PxVehicleGearsData::eREVERSE == currentGear && forwardSpeed > THRESHOLD_ROLLING_BACKWARDS_SPEED)
			isMovingBackwards = true;

		//Check if the car is moving slowly.
//		if(forwardSpeedAbs < THRESHOLD_FORWARD_SPEED && sidewaysSpeedAbs < THRESHOLD_SIDEWAYS_SPEED)
		if(forwardSpeedAbs < data.mThresholdForwardSpeed && sidewaysSpeedAbs < THRESHOLD_SIDEWAYS_SPEED)
			isMovingForwardSlowly=true;
		//printf("forwardSpeedAbs: %f\n", forwardSpeedAbs);

		//Now work if we need to toggle from forwards gear to reverse gear or vice versa.
		if(isMovingBackwards)
		{
			if(!accelRaw && !brakeRaw && !handbrakeRaw && (currentGear == targetGear))			
			{
				//The car is rolling against the gear and the player is doing nothing to stop this.
				toggleAutoReverse = true;
				//printf("CHECKPOINT0\n");
			}
		}
		else if(data.mImmediateAutoReverse || (prevIsMovingForwardSlowly && isMovingForwardSlowly))
		{
			if((currentGear > PxVehicleGearsData::eNEUTRAL) && brakeRaw && !accelRaw && (currentGear == targetGear))
			{
				//The car was moving slowly in forward gear without player input and is now moving slowly with player input that indicates the 
				//player wants to switch to reverse gear.
				toggleAutoReverse = true;
				//printf("CHECKPOINT1\n");
			}
			else if(currentGear == PxVehicleGearsData::eREVERSE && accelRaw && !brakeRaw && (currentGear == targetGear))
			{
				//The car was moving slowly in reverse gear without player input and is now moving slowly with player input that indicates the 
				//player wants to switch to forward gear.
				toggleAutoReverse = true;
				//printf("CHECKPOINT2\n");
			}
		}

		//If the car was brought to rest through braking then the player needs to release the brake then reapply
		//to indicate that the gears should toggle between reverse and forward.
//		if(isMovingForwardSlowly && !brakeRaw && !accelRaw && !handbrakeRaw)
		if(isMovingForwardSlowly)
			newIsMovingForwardSlowly = true;
	}

	//printf("toggleAutoReverse: %d\n", toggleAutoReverse);
	//printf("isMovingBackwards: %d\n", isMovingBackwards);
}

void VehicleController::update(const PxF32 timestep, const PxVehicleWheelQueryResult& vehicleWheelQueryResults, PxVehicleWheels& focusVehicle, const VehicleData& data)
{
	PX_ASSERT(focusVehicle.getVehicleType()==PxVehicleTypes::eDRIVE4W);

	PxVehicleDrive4W& vehDrive4W = static_cast<PxVehicleDrive4W&>(focusVehicle);
	PxVehicleDriveDynData* driveDynData = &vehDrive4W.mDriveDynData;

	//Store raw inputs in replay stream if in recording mode.
	//Set raw inputs from replay stream if in replay mode.
	//Store raw inputs from active stream in handy arrays so we don't need to worry
	//about which stream (live input or replay) is active.
	//Work out if we are using keys or gamepad controls depending on which is being used
	//(gamepad selected if both are being used).
	PxVehicleDrive4WRawInputData carRawInputs;
	processRawInputs(timestep,driveDynData->getUseAutoGears(),carRawInputs);

	//Work out if the car is to flip from reverse to forward gear or from forward gear to reverse.
	bool toggleAutoReverse = false;
	bool newIsMovingForwardSlowly = false;
	processAutoReverse(data, focusVehicle, *driveDynData, vehicleWheelQueryResults, carRawInputs, toggleAutoReverse, newIsMovingForwardSlowly);
	mIsMovingForwardSlowly = newIsMovingForwardSlowly;

	//printf("mIsMovingForwardSlowly: %d\n", mIsMovingForwardSlowly);

	//If the car is to flip gear direction then switch gear as appropriate.
	if(toggleAutoReverse)
	{
		mInReverseMode = !mInReverseMode;
		
		if(mInReverseMode)
			driveDynData->forceGearChange(PxVehicleGearsData::eREVERSE);
		else
			driveDynData->forceGearChange(PxVehicleGearsData::eFIRST);
	}

	//If in reverse mode then swap the accel and brake.
	if(mInReverseMode)
	{
		if(gUseKeyboard)
		{
			const bool accel=carRawInputs.getDigitalAccel();
			const bool brake=carRawInputs.getDigitalBrake();
			carRawInputs.setDigitalAccel(brake);
			carRawInputs.setDigitalBrake(accel);
		}
		else
		{
			const PxF32 accel=carRawInputs.getAnalogAccel();
			const PxF32 brake=carRawInputs.getAnalogBrake();
			carRawInputs.setAnalogAccel(brake);
			carRawInputs.setAnalogBrake(accel);
		}
	}

	{
		const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);

		// We create it each time because the gSteerVsForwardSpeedData can change
		const PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(data.mSteerVsForwardSpeedData, 4);

		// Now filter the raw input values and apply them to focus vehicle
		// as floats for brake,accel,handbrake,steer and bools for gearup,geardown.
		if(gUseKeyboard)
		{
			PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs
				(gKeySmoothingData, gSteerVsForwardSpeedTable, carRawInputs, timestep, isInAir, vehDrive4W
#if PHYSX_SUPPORT_STEER_FILTER
				, data.mSteerFilter
#endif
				);
		}
		else
		{
			PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs
				(gCarPadSmoothingData, gSteerVsForwardSpeedTable, carRawInputs, timestep, isInAir, vehDrive4W
#if PHYSX_SUPPORT_STEER_FILTER
				, data.mSteerFilter
#endif
				);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

SampleVehicle_VehicleManager::SampleVehicle_VehicleManager() :
	mNumVehicles				(0),
#if PHYSX_SUPPORT_VEHICLE5
	mBatchQueryExt				(NULL),
#else
	mSqData						(NULL),
	mSqWheelRaycastBatchQuery	(NULL),
#endif
	mWheelQueryResults			(NULL),
	mSurfaceTirePairs			(NULL)
{
	for(PxU32 i=0;i<MAX_NUM_4W_VEHICLES;i++)
		mVehicles[i]=NULL;
}

SampleVehicle_VehicleManager::~SampleVehicle_VehicleManager()
{
}

void SampleVehicle_VehicleManager::init(PxPhysics& physics, PxScene& scene, const PxMaterial** drivableSurfaceMaterials, const PxVehicleDrivableSurfaceType* drivableSurfaceTypes, const bool use_sweeps)
{
	//Initialise the sdk.
	PxInitVehicleSDK(physics);

	//Set the basis vectors.
	const PxVec3 up(0.0f, 1.0f, 0.0f);
	const PxVec3 forward(0.0f, 0.0f, 1.0f);
	PxVehicleSetBasisVectors(up, forward);

	//Set the vehicle update mode to be immediate velocity changes.
	PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);
	
	//Scene query data for to allow raycasts for all suspensions of all vehicles.
#if PHYSX_SUPPORT_VEHICLE5
//	mBatchQueryExt = createMyBatchQueryExt(scene, &gMyFilter, MAX_NUM_4W_VEHICLES*4, MAX_NUM_4W_VEHICLES*4, MAX_NUM_4W_VEHICLES*4, MAX_NUM_4W_VEHICLES*4, 0, 0);
	mBatchQueryExt = createMyBatchQueryExt(scene, &gMyFilter, MAX_NUM_4W_VEHICLES*4, 0,
															MAX_NUM_4W_VEHICLES*4, MAX_NUM_4W_VEHICLES*4*NB_SWEEP_HITS_PER_WHEEL,
															0, 0);
#else
	mSqData = SampleVehicleSceneQueryData::allocate(MAX_NUM_4W_VEHICLES*4, use_sweeps);
#endif
	//Data to store reports for each wheel.
	mWheelQueryResults = SampleVehicleWheelQueryResults::allocate(MAX_NUM_4W_VEHICLES*4);

	//Set up the friction values arising from combinations of tire type and surface type.
//	mSurfaceTirePairs = PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES);
//	mSurfaceTirePairs->setup(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES, drivableSurfaceMaterials, drivableSurfaceTypes);
	// One tire type & one surface type for now
	const PxU32 nbTireTypes = 2*MAX_NUM_4W_VEHICLES;	// Front & rear
	const PxU32 nbSurfaceTypes = 1;
	mSurfaceTirePairs = PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(nbTireTypes, nbSurfaceTypes);
	mSurfaceTirePairs->setup(nbTireTypes, nbSurfaceTypes, drivableSurfaceMaterials, drivableSurfaceTypes);
/*	for(PxU32 i=0;i<MAX_NUM_SURFACE_TYPES;i++)
	{
		for(PxU32 j=0;j<MAX_NUM_TIRE_TYPES;j++)
		{
//			mSurfaceTirePairs->setTypePairFriction(i,j,gTireFrictionMultipliers[i][j]);
			//### 
			mSurfaceTirePairs->setTypePairFriction(i, j, tireFrictionMultiplier);
		}
	}*/

//	PxVehicleSetSweepHitRejectionAngles(PI, PI);
	//PxVehicleSetSweepHitRejectionAngles(PI/6.0f, PI/6.0f);
	//PxVehicleSetSweepHitRejectionAngles(PI/3.0f, PI/3.0f);
}

void SampleVehicle_VehicleManager::shutdown()
{
	//Remove the N-wheeled vehicles.
	for(PxU32 i=0;i<mNumVehicles;i++)
	{
		switch(mVehicles[i]->getVehicleType())
		{
		case PxVehicleTypes::eDRIVE4W:
			{
				PxVehicleDrive4W* veh=(PxVehicleDrive4W*)mVehicles[i];
				veh->free();
			}
			break;
		default:
			PX_ASSERT(false);
			break;
		}
	}

	//Deallocate scene query data that was used for suspension raycasts.
#if PHYSX_SUPPORT_VEHICLE5
	if(mBatchQueryExt)
	{
		mBatchQueryExt->release();
		mBatchQueryExt = NULL;
	}
#else
	if(mSqData)
	{
		mSqData->free();
		mSqData = NULL;
	}
#endif
	//Deallocate buffers that store wheel reports.
	if(mWheelQueryResults)
	{
		mWheelQueryResults->free();
		mWheelQueryResults = NULL;
	}

	//Release the  friction values used for combinations of tire type and surface type.
	if(mSurfaceTirePairs)
	{
		mSurfaceTirePairs->release();
		mSurfaceTirePairs = NULL;
	}

#if PHYSX_SUPPORT_VEHICLE5
#else
	//Scene query.
	mSqWheelRaycastBatchQuery=NULL;
#endif

	DELETESINGLE(gDebugSegments2);
	DELETESINGLE(gDebugSegments);
	DELETESINGLE(gDebugPts);

	PxCloseVehicleSDK();
}

void SampleVehicle_VehicleManager::suspensionRaycasts(PxScene* scene, const bool use_sweeps, float sweep_inflation)
{
	const PxF32 sweepWidthScale = 1.0f;
	const PxF32 sweepRadiusScale = 1.0f;

#if PHYSX_SUPPORT_VEHICLE5
//	PxSceneReadLock scopedLock(*scene);
	PX_ASSERT(mBatchQueryExt);
	if(use_sweeps)
	{
	#if PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
		PxVehicleSuspensionSweeps(mBatchQueryExt, mNumVehicles, mVehicles, NB_SWEEP_HITS_PER_WHEEL, null, sweepWidthScale, sweepRadiusScale, sweep_inflation);
	#else
		PxVehicleSuspensionSweeps(mBatchQueryExt, mNumVehicles, mVehicles, NB_SWEEP_HITS_PER_WHEEL);
	#endif
	}
	else
		PxVehicleSuspensionRaycasts(mBatchQueryExt, mNumVehicles, mVehicles);
#else
	//Create a scene query if we haven't already done so.
	if(!mSqWheelRaycastBatchQuery)
		mSqWheelRaycastBatchQuery = mSqData->setUpBatchedSceneQuery(scene, use_sweeps);

	{
//		PxSceneReadLock scopedLock(*scene);
		if(use_sweeps)
		{
	#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
		#if PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
			PxVehicleSuspensionSweeps(mSqWheelRaycastBatchQuery,
				mNumVehicles, mVehicles,
				mSqData->getSweepQueryResultBufferSize(), mSqData->getSweepQueryResultBuffer(), NB_SWEEP_HITS_PER_WHEEL
				,null, sweepWidthScale, sweepRadiusScale, sweep_inflation
				);
		#else
			PxVehicleSuspensionSweeps(mSqWheelRaycastBatchQuery,
				mNumVehicles, mVehicles,
				mSqData->getSweepQueryResultBufferSize(), mSqData->getSweepQueryResultBuffer(), NB_SWEEP_HITS_PER_WHEEL
	//			,null, 0.5f, 0.5f
				);
		#endif
	#endif
		}
		else
		{
			PxVehicleSuspensionRaycasts(mSqWheelRaycastBatchQuery,
				mNumVehicles, mVehicles,
				mSqData->getRaycastQueryResultBufferSize(), mSqData->getRaycastQueryResultBuffer());
		}
	}

#ifdef REMOVED
#ifdef USE_SWEEPS
	if(0)
	{
		printf("\n");
		PxU32 nb = mSqData->getSweepQueryResultBufferSize();
		PxSweepQueryResult* buffer = mSqData->getSweepQueryResultBuffer();
		for(PxU32 i=0;i<4;i++)
		{
			if(buffer[i].getNbAnyHits())
			{
				PxSweepHit& h = (PxSweepHit&)buffer[i].getAnyHit(0);
				printf("%d: Hit distance: %f (%s)\n", i, h.distance, h.shape->getName());
//				h.distance = 0.0f;
//				h.distance = 0.8f;
			}
		}
	}

#else
	if(0)
	{
		static float Memories[4] = {0,0,0,0};
		PxRaycastQueryResult* buffer = mSqData->getRaycastQueryResultBuffer();
		for(PxU32 i=0;i<4;i++)
		{
			if(buffer[i].getNbAnyHits())
			{
				PxRaycastHit& h = (PxRaycastHit&)buffer[i].getAnyHit(0);
				//printf("%d: Hit distance: %f\n", i, h.distance);
				FeedbackFilter(h.distance, Memories[i], 0.0f);
				h.distance = Memories[i];
			}
		}
	}

	if(0)
	{
		PxU32 nb = mSqData->getRaycastQueryResultBufferSize();
		PxRaycastQueryResult* buffer = mSqData->getRaycastQueryResultBuffer();
		for(PxU32 i=0;i<4;i++)
		{
			if(buffer[i].getNbAnyHits())
			{
				PxRaycastHit& h = (PxRaycastHit&)buffer[i].getAnyHit(0);
				printf("%d: Hit distance: %f\n", i, h.distance);
	//			h.distance = 0.0f;
			}
		}
	}
#endif
#endif
#endif
}

#if PINT_DEBUG_VEHICLE_ON

void SampleVehicle_VehicleManager::updateAndRecordTelemetryData(const PxF32 timestep, const PxVec3& gravity, PxVehicleWheels* focusVehicle, PxVehicleTelemetryData* telemetryData)
{
	PX_ASSERT(focusVehicle && telemetryData);

	//Update all vehicles except for focusVehicle.
	PxVehicleWheels* vehicles[MAX_NUM_4W_VEHICLES];
	PxVehicleWheelQueryResult vehicleWheelQueryResults[MAX_NUM_4W_VEHICLES];
	PxVehicleWheelQueryResult focusVehicleWheelQueryResults[1];
	PxU32 numExtraVehicles=0;
	for(PxU32 i=0;i<mNumVehicles;i++)
	{
		if(focusVehicle!=mVehicles[i])
		{
			vehicles[numExtraVehicles]=mVehicles[i];
			vehicleWheelQueryResults[numExtraVehicles]=mVehicleWheelQueryResults[i];
			numExtraVehicles++;
		}
		else
		{
			focusVehicleWheelQueryResults[0]=mVehicleWheelQueryResults[i];
		}
	}
	if(numExtraVehicles)
		PxVehicleUpdates(timestep, gravity, *mSurfaceTirePairs, numExtraVehicles, vehicles, vehicleWheelQueryResults);

	//Update the vehicle for which we want to record debug data.
	PxVehicleUpdateSingleVehicleAndStoreTelemetryData(timestep, gravity, *mSurfaceTirePairs, focusVehicle, focusVehicleWheelQueryResults, *telemetryData);
//	PxVehicleUpdates(timestep, gravity, *mSurfaceTirePairs, 1, &focusVehicle, focusVehicleWheelQueryResults);
}

#else

void SampleVehicle_VehicleManager::update(const PxF32 timestep, const PxVec3& gravity)
{
	PxVehicleUpdates(timestep, gravity, *mSurfaceTirePairs, mNumVehicles, mVehicles, mVehicleWheelQueryResults);
}

#endif

void SampleVehicle_VehicleManager::resetNWCar(const PxTransform& startTransform, PxVehicleWheels* vehWheels)
{
	switch(vehWheels->getVehicleType())
	{
	case PxVehicleTypes::eDRIVE4W:
		{
			PxVehicleDrive4W* vehDrive4W=(PxVehicleDrive4W*)vehWheels;
			//Set the car back to its rest state.
			vehDrive4W->setToRestState();
			//Set the car to first gear.
			vehDrive4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		}
		break;
	default:
		PX_ASSERT(false);
		break;
	}

	//Set the car's transform to be the start transform.
	PxRigidDynamic* actor=vehWheels->getRigidDynamicActor();
	//PxSceneWriteLock scopedLock(*actor->getScene());
	actor->setGlobalPose(startTransform);
}

static void SetupVehicleShape(PxShape* shape, const char* name, const PxTransform& local_pose, PintCollisionGroup group)
{
	const EditableParams& Params = PhysX3::GetEditableParams();

	// We don't disable eSCENE_QUERY_SHAPE entirely, otherwise we couldn't pick the vehicle with the mouse or raytrace it.
	// Instead, the filtering (to make sure that the wheel's raycasts/sweeps ignore the wheels themselves and the chassis)
	// is done in the pre-filter during hardcoded chassis & wheel names.
	//
	// Renderer will be stored in userData later in the setup code.

	shape->setName(name);
//	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	shape->setFlag(PxShapeFlag::eVISUALIZATION, PhysX3::IsDebugVizEnabled());
	shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, Params.mSQFlag);
	shape->setLocalPose(local_pose);

	shape->setContactOffset(Params.mContactOffset);
#ifdef PHYSX_SUPPORT_VEHICLE_XP
	// ###
	shape->setContactOffset(0.2f);
#endif
	shape->setRestOffset(Params.mRestOffset);

	// Setup query filter data so that we can filter out all shapes - debug purpose
	if(Params.mSQFlag)
		shape->setQueryFilterData(PxFilterData(1, 0, 0, 0));

	if(gSetupCollisionGroups)
		PhysX3_SetGroup(*shape, group);
}

static void SetupVehicleChassisShape(PxShape* shape, const PxTransform& local_pose)
{
	SetupVehicleShape(shape, gChassisDebugName, local_pose, gChassisCollisionGroup);
}

static void SetupVehicleWheelShape(SharedPhysX& physx, PxShape* shape, const PxTransform& local_pose, PxU32 wheel_id)
{
	SetupVehicleShape(shape, gWheelDebugName, local_pose, gWheelCollisionGroup);
	if(gSetupCollisionGroups)
		PhysX3_SetGroupCollisionFlag(gChassisCollisionGroup, gWheelCollisionGroup, false);

/*	if(!gSetupCollisionGroups)
	{
		// Override wheel name with wheel ID to pass it to contact modify callback
		const char* WheelName = _F("%s%d", gWheelDebugName, wheel_id);
		// Make that string persistent
		physx.SetShapeName(shape, WheelName);
	}*/
}

static void setupActor(	SharedPhysX& physx, PxRigidDynamic* vehActor, 
						const PxGeometry** wheelGeometries, const PxTransform* wheelLocalPoses, const PxU32 numWheelGeometries, const PxMaterial* wheelMaterial,
						const PxGeometry** chassisGeometries, const PxTransform* chassisLocalPoses, const PxU32 numChassisGeometries, const PxMaterial* chassisMaterial,
						const PxVehicleChassisData& chassisData,
						PxPhysics* physics)
{
	//Add all the wheel shapes to the actor.
	for(PxU32 i=0;i<numWheelGeometries;i++)
	{
#if PHYSX_DEPRECATED_CREATE_SHAPE
		PxShape* wheelShape = PxRigidActorExt::createExclusiveShape(*vehActor, *wheelGeometries[i], *wheelMaterial);
#else
		PxShape* wheelShape = vehActor->createShape(*wheelGeometries[i], *wheelMaterial);
#endif
		SetupVehicleWheelShape(physx, wheelShape, wheelLocalPoses[i], i);
//		printf("wheelLocalPoses[i]: %f %f %f\n", wheelLocalPoses[i].p.x, wheelLocalPoses[i].p.y, wheelLocalPoses[i].p.z);
	}

	//Add the chassis shapes to the actor.
	for(PxU32 i=0;i<numChassisGeometries;i++)
	{
#if PHYSX_DEPRECATED_CREATE_SHAPE
		PxShape* chassisShape = PxRigidActorExt::createExclusiveShape(*vehActor, *chassisGeometries[i], *chassisMaterial);
#else
		PxShape* chassisShape = vehActor->createShape(*chassisGeometries[i], *chassisMaterial);
#endif
		SetupVehicleChassisShape(chassisShape, chassisLocalPoses[i]);
	}

	vehActor->setMass(chassisData.mMass);
	vehActor->setMassSpaceInertiaTensor(chassisData.mMOI);
	vehActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset));
//	vehActor->setCMassLocalPose(PxTransform(-chassisData.mCMOffset,PxQuat(PxIdentity)));

	//bool status = PxRigidBodyExt::setMassAndUpdateInertia(*vehActor, chassisData.mMass);

	// TODO: setup dynamic like in SharedPhysX::SetupDynamic
#if PHYSX_SUPPORT_STABILIZATION_FLAG
	vehActor->setStabilizationThreshold(0.0f);
#endif

#ifdef PHYSX_SUPPORT_VEHICLE_XP
	// ###
	vehActor->setWakeCounter(9999999999.0f);
#endif
}

static PxRigidDynamic* createVehicleActor4W(SharedPhysX& physx, const PxVehicleChassisData& chassisData, PxConvexMesh** wheelConvexMeshes, PxConvexMesh* chassisConvexMesh,
											PxScene& scene, PxPhysics& physics, const PxMaterial& material, const PxTransform chassisLocalPose, bool useSphereWheels)
{
	//We need a rigid body actor for the vehicle.
	//Don't forget to add the actor the scene after setting up the associated vehicle.
	PxRigidDynamic* vehActor = physics.createRigidDynamic(PxTransform(PxIdentity));

	const PxMaterial& wheelMaterial = material;
	const PxMaterial& chassisMaterial = material;

	//We need to add chassis collision shapes, their local poses, a material for the chassis, and a simulation filter for the chassis.
	PxConvexMeshGeometry chassisConvexGeom(chassisConvexMesh);
	const PxGeometry* chassisGeoms[1] = {&chassisConvexGeom};

//	const PxTransform chassisLocalPoses[1] = {PxTransform(PxIdentity)};
	const PxTransform chassisLocalPoses[1] = {chassisLocalPose};

	if(useSphereWheels)
	{
		const Sphere S0(wheelConvexMeshes[0]->getNbVertices(), reinterpret_cast<const Point*>(wheelConvexMeshes[0]->getVertices()));
		const Sphere S1(wheelConvexMeshes[1]->getNbVertices(), reinterpret_cast<const Point*>(wheelConvexMeshes[1]->getVertices()));
		const Sphere S2(wheelConvexMeshes[2]->getNbVertices(), reinterpret_cast<const Point*>(wheelConvexMeshes[2]->getVertices()));
		const Sphere S3(wheelConvexMeshes[3]->getNbVertices(), reinterpret_cast<const Point*>(wheelConvexMeshes[3]->getVertices()));

		const PxSphereGeometry SG0(S0.mRadius);
		const PxSphereGeometry SG1(S1.mRadius);
		const PxSphereGeometry SG2(S2.mRadius);
		const PxSphereGeometry SG3(S3.mRadius);

		const PxGeometry* wheelGeometries[4] = {&SG0, &SG1, &SG2, &SG3};
		const PxTransform wheelLocalPoses[4] = {PxTransform(ToPxVec3(S0.mCenter)), PxTransform(ToPxVec3(S1.mCenter)), PxTransform(ToPxVec3(S2.mCenter)), PxTransform(ToPxVec3(S3.mCenter))};

		//Set up the physx rigid body actor with shapes, local poses, and filters.
		setupActor(physx, vehActor, wheelGeometries, wheelLocalPoses, 4, &wheelMaterial, chassisGeoms, chassisLocalPoses, 1, &chassisMaterial, chassisData, &physics);

		return vehActor;
	}
	else
	{
		//We need to add wheel collision shapes, their local poses, a material for the wheels, and a simulation filter for the wheels.
		const PxConvexMeshGeometry frontLeftWheelGeom(wheelConvexMeshes[0]);
		const PxConvexMeshGeometry frontRightWheelGeom(wheelConvexMeshes[1]);
		const PxConvexMeshGeometry rearLeftWheelGeom(wheelConvexMeshes[2]);
		const PxConvexMeshGeometry rearRightWheelGeom(wheelConvexMeshes[3]);
		const PxGeometry* wheelGeometries[4] = {&frontLeftWheelGeom, &frontRightWheelGeom, &rearLeftWheelGeom, &rearRightWheelGeom};
		const PxTransform wheelLocalPoses[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};

		//Set up the physx rigid body actor with shapes, local poses, and filters.
		setupActor(physx, vehActor, wheelGeometries, wheelLocalPoses, 4, &wheelMaterial, chassisGeoms, chassisLocalPoses, 1, &chassisMaterial, chassisData, &physics);

		return vehActor;
	}
}

static void computeWheelWidthsAndRadii(PxU32 nbWheels, PxConvexMesh** wheelConvexMeshes, PxF32* wheelWidths, PxF32* wheelRadii)
{
	for(PxU32 i=0;i<nbWheels;i++)
	{
		if(1)
		{
			AABB Box;
			ComputeAABB(Box, reinterpret_cast<const Point*>(wheelConvexMeshes[i]->getVertices()), wheelConvexMeshes[i]->getNbVertices());
//			return ToPxVec3(Box.mMax - Box.mMin);
			wheelWidths[i] = Box.mMax .x - Box.mMin.x;
			wheelRadii[i] = PxMax(Box.mMax .y, Box.mMax .z)*0.975f;
	//		wheelRadii[i]*=1.5f;
		}
		else
		{
			const PxU32 numWheelVerts = wheelConvexMeshes[i]->getNbVertices();
			const PxVec3* wheelVerts = wheelConvexMeshes[i]->getVertices();
			PxVec3 wheelMin(PX_MAX_F32);
			PxVec3 wheelMax(-PX_MAX_F32);
			for(PxU32 j=0;j<numWheelVerts;j++)
			{
				wheelMin.x = PxMin(wheelMin.x, wheelVerts[j].x);
				wheelMin.y = PxMin(wheelMin.y, wheelVerts[j].y);
				wheelMin.z = PxMin(wheelMin.z, wheelVerts[j].z);
				wheelMax.x = PxMax(wheelMax.x, wheelVerts[j].x);
				wheelMax.y = PxMax(wheelMax.y, wheelVerts[j].y);
				wheelMax.z = PxMax(wheelMax.z, wheelVerts[j].z);
			}
			wheelWidths[i] = wheelMax.x - wheelMin.x;
			wheelRadii[i] = PxMax(wheelMax.y, wheelMax.z)*0.975f;
	//		wheelRadii[i]*=1.5f;
		}
	}
}

static PxVec3 computeChassisAABBDimensions(const PxConvexMesh* chassis)
{
	if(1)
	{
		AABB Box;
		ComputeAABB(Box, reinterpret_cast<const Point*>(chassis->getVertices()), chassis->getNbVertices());
		return ToPxVec3(Box.mMax - Box.mMin);
	}
	else
	{
		const PxU32 numChassisVerts = chassis->getNbVertices();
		const PxVec3* chassisVerts = chassis->getVertices();
		PxVec3 chassisMin(PX_MAX_F32);
		PxVec3 chassisMax(-PX_MAX_F32);
		for(PxU32 i=0;i<numChassisVerts;i++)
		{
			chassisMin.x = PxMin(chassisMin.x, chassisVerts[i].x);
			chassisMin.y = PxMin(chassisMin.y, chassisVerts[i].y);
			chassisMin.z = PxMin(chassisMin.z, chassisVerts[i].z);
			chassisMax.x = PxMax(chassisMax.x, chassisVerts[i].x);
			chassisMax.y = PxMax(chassisMax.y, chassisVerts[i].y);
			chassisMax.z = PxMax(chassisMax.z, chassisVerts[i].z);
		}
		const PxVec3 chassisDims = chassisMax - chassisMin;
		return chassisDims;
	}
}

static void createVehicle4WSimulationData(udword vehicle_id, PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes,
										  PxVehicleWheelsSimData& wheelsData, PxVehicleDriveSimData4W& driveData, PxVehicleChassisData& chassisData,
										  const PINT_VEHICLE_CREATE& desc)
{
	//Extract the chassis AABB dimensions from the chassis convex mesh.
	const PxVec3 chassisDims = computeChassisAABBDimensions(chassisConvexMesh);
	//printf("chassisDims: %f %f %f\n", chassisDims.x, chassisDims.y, chassisDims.z);

//#define DEFAULT_CODE
#ifdef DEFAULT_CODE
	//The origin is at the center of the chassis mesh.
	//Set the center of mass to be below this point and a little towards the front.
	const PxVec3 chassisCMOffset(0.0f, -chassisDims.y*0.5f + desc.mChassisCMOffsetY, desc.mChassisCMOffsetZ);
//	const PxVec3 chassisCMOffset(0.0f);

	//Now compute the chassis mass and moment of inertia.
	//Use the moment of inertia of a cuboid as an approximate value for the chassis moi.
	PxVec3 chassisMOI
		((chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z)*desc.mChassisMass/12.0f,
		 (chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z)*desc.mChassisMass/12.0f,
		 (chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y)*desc.mChassisMass/12.0f);
	//A bit of tweaking here.  The car will have more responsive turning if we reduce the 	
	//y-component of the chassis moment of inertia.
	chassisMOI.y *= desc.mChassisMOICoeffY;
#else
	PxVec3 chassisCMOffset;
	PxVec3 chassisMOI;
	{
		const PxVec3 extents = chassisDims * 0.5f;

		//const PxReal mass = 8.0f * extents.x * extents.y * extents.z;

		const PxReal s = (1.0f/3.0f);// * mass;

		const PxReal x = extents.x*extents.x;
		const PxReal y = extents.y*extents.y;
		const PxReal z = extents.z*extents.z;

		chassisMOI = PxVec3(y+z, z+x, x+y) * s * desc.mChassisMass;///mass;
		chassisMOI.y *= desc.mChassisMOICoeffY;
		chassisCMOffset = PxVec3(0.0f, desc.mChassisCMOffsetY, desc.mChassisCMOffsetZ);
	}
#endif
/*chassisMOI.x = 1354.0f;
chassisMOI.y = 2083.0f;
chassisMOI.z = 770.0f;*/

	//Let's set up the chassis data structure now.
	chassisData.mMass = desc.mChassisMass;
	chassisData.mMOI = chassisMOI;
	chassisData.mCMOffset = chassisCMOffset;
//		chassisData.mMOI = PxVec3(2000.0f, 2000.0f, 1000.0f);
//		chassisData.mCMOffset = PxVec3(0.0f);
//		chassisData.mCMOffset = PxVec3(0.0f, 0.55f, 0.0f);
//		printf("chassisData.mCMOffset: %f %f %f\n", chassisData.mCMOffset.x, chassisData.mCMOffset.y, chassisData.mCMOffset.z);

	const PxVec3* wheelCentreOffsets = reinterpret_cast<const PxVec3*>(desc.mWheelOffset);
	//Compute the sprung masses of each suspension spring using a helper function.
	PxF32 suspSprungMasses[4];
	PxVehicleComputeSprungMasses(4, wheelCentreOffsets, chassisCMOffset, desc.mChassisMass, 1, suspSprungMasses);
//	suspSprungMasses[0] = suspSprungMasses[1] = suspSprungMasses[2] = suspSprungMasses[3] = desc.mChassisMass/4.0f;

	if(0)
	{
		//mSpringStrength*mMaxDroop = mSprungMass*gravitationalAcceleration
		float SprungMass = (desc.mSuspSpringStrength * desc.mSuspMaxDroop)/9.81f;
		suspSprungMasses[0] = suspSprungMasses[1] = suspSprungMasses[2] = suspSprungMasses[3] = SprungMass;
	}
//	suspSprungMasses[0] = suspSprungMasses[1] = suspSprungMasses[2] = suspSprungMasses[3] = 0.0f;
//	suspSprungMasses[0] = suspSprungMasses[1] = suspSprungMasses[2] = suspSprungMasses[3] = (3000.0f+1500.0f)/4.0f;

	PxVehicleWheelData wheels[4];
	{
		//Extract the wheel radius and width from the wheel convex meshes.
		PxF32 wheelWidths[4];
		PxF32 wheelRadii[4];
		computeWheelWidthsAndRadii(4, wheelConvexMeshes, wheelWidths, wheelRadii);

		//Now compute the wheel masses and inertias components around the axle's axis.
		//http://en.wikipedia.org/wiki/List_of_moments_of_inertia
		PxF32 wheelMOIs[4];
		for(PxU32 i=0;i<4;i++)
		{
			wheelMOIs[i] = 0.5f*desc.mWheelMass*wheelRadii[i]*wheelRadii[i];
		}

		//Let's set up the wheel data structures now with radius, mass, and moi.
		for(PxU32 i=0;i<4;i++)
		{
			wheels[i].mRadius	= wheelRadii[i];
			wheels[i].mMass		= desc.mWheelMass;
			wheels[i].mMOI		= wheelMOIs[i];
			wheels[i].mWidth	= wheelWidths[i];
		}
	}

	//Disable the handbrake from the front wheels and enable for the rear wheels
	wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxHandBrakeTorque		= desc.mWheelMaxHandBrakeTorqueFront;
	wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxHandBrakeTorque	= desc.mWheelMaxHandBrakeTorqueFront;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque		= desc.mWheelMaxHandBrakeTorqueRear;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque		= desc.mWheelMaxHandBrakeTorqueRear;
	//Enable steering for the front wheels and disable for the front wheels.
	const float MaxSteerFront = desc.mWheelMaxSteerFront;
	wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = MaxSteerFront;
	wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = MaxSteerFront;
	const float MaxSteerRear = desc.mWheelMaxSteerRear;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxSteer = MaxSteerRear;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxSteer = MaxSteerRear;

	const float MaxBrakeTorqueFront = desc.mWheelMaxBrakeTorqueFront;
	const float MaxBrakeTorqueRear = desc.mWheelMaxBrakeTorqueRear;
	wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxBrakeTorque = MaxBrakeTorqueFront;
	wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxBrakeTorque = MaxBrakeTorqueFront;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxBrakeTorque = MaxBrakeTorqueRear;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxBrakeTorque = MaxBrakeTorqueRear;

	//Let's set up the tire data structures now.
	//Put slicks on the front tires and wets on the rear tires.
	PxVehicleTireData tires[4];
//	tires[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mType=TIRE_TYPE_SLICKS;
//	tires[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mType=TIRE_TYPE_SLICKS;
//	tires[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mType=TIRE_TYPE_WETS;
//	tires[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mType=TIRE_TYPE_WETS;
		tires[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mType	= vehicle_id*2+0;
		tires[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mType	= vehicle_id*2+0;
		tires[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mType		= vehicle_id*2+1;
		tires[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mType	= vehicle_id*2+1;

	//Let's set up the suspension data structures now.
	PxVehicleSuspensionData susps[4];
	for(PxU32 i=0;i<4;i++)
	{
		susps[i].mMaxCompression	= desc.mSuspMaxCompression;
		susps[i].mMaxDroop			= desc.mSuspMaxDroop;
		susps[i].mSpringStrength	= desc.mSuspSpringStrength;
		susps[i].mSpringDamperRate	= desc.mSuspSpringDamperRate;
	}
	susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mSprungMass	= suspSprungMasses[PxVehicleDrive4WWheelOrder::eFRONT_LEFT];
	susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mSprungMass	= suspSprungMasses[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT];
	susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mSprungMass	= suspSprungMasses[PxVehicleDrive4WWheelOrder::eREAR_LEFT];
	susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mSprungMass	= suspSprungMasses[PxVehicleDrive4WWheelOrder::eREAR_RIGHT];

	//Set up the camber.
	//Remember that the left and right wheels need opposite camber so that the car preserves symmetry about the forward direction.
	//Set the camber to 0.0f when the spring is neither compressed or elongated.
	const PxF32 camberAngleAtRest = desc.mSuspCamberAngleAtRest;
	susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtRest	= camberAngleAtRest;
	susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtRest	= -camberAngleAtRest;
	susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtRest		= camberAngleAtRest;
	susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtRest	= -camberAngleAtRest;
	//Set the wheels to camber inwards at maximum droop (the left and right wheels almost form a V shape)
	const PxF32 camberAngleAtMaxDroop = desc.mSuspCamberAngleAtMaxDroop;
	susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxDroop	= camberAngleAtMaxDroop;
	susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxDroop	= -camberAngleAtMaxDroop;
	susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxDroop		= camberAngleAtMaxDroop;
	susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxDroop	= -camberAngleAtMaxDroop;
	//Set the wheels to camber outwards at maximum compression (the left and right wheels almost form a A shape).
	const PxF32 camberAngleAtMaxCompression = -desc.mSuspCamberAngleAtMaxCompr;
	susps[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mCamberAtMaxCompression	= camberAngleAtMaxCompression;
	susps[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mCamberAtMaxCompression	= -camberAngleAtMaxCompression;
	susps[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mCamberAtMaxCompression	= camberAngleAtMaxCompression;
	susps[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mCamberAtMaxCompression	= -camberAngleAtMaxCompression;

	//We need to set up geometry data for the suspension, wheels, and tires.
	//We already know the wheel centers described as offsets from the actor center and the center of mass offset from actor center.
	//From here we can approximate application points for the tire and suspension forces.
	//Lets assume that the suspension travel directions are absolutely vertical.
	//Also assume that we apply the tire and suspension forces 30cm below the center of mass.
	PxVec3 suspTravelDirections[4]={PxVec3(0,-1,0),PxVec3(0,-1,0),PxVec3(0,-1,0),PxVec3(0,-1,0)};
	PxVec3 wheelCentreCMOffsets[4];
	PxVec3 suspForceAppCMOffsets[4];
	PxVec3 tireForceAppCMOffsets[4];
	const float CMOffset = -desc.mForceApplicationCMOffsetY;
	for(PxU32 i=0;i<4;i++)
	{
		wheelCentreCMOffsets[i] = wheelCentreOffsets[i] - chassisCMOffset;
		suspForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, CMOffset, wheelCentreCMOffsets[i].z);
		tireForceAppCMOffsets[i] = PxVec3(wheelCentreCMOffsets[i].x, CMOffset, wheelCentreCMOffsets[i].z);

		//suspForceAppCMOffsets[i] = tireForceAppCMOffsets[i] = wheelCentreCMOffsets[i];
	}

	//Now add the wheel, tire and suspension data.
	for(PxU32 i=0;i<4;i++)
	{
		wheelsData.setWheelData(i, wheels[i]);
		wheelsData.setTireData(i, tires[i]);
		wheelsData.setSuspensionData(i, susps[i]);
		wheelsData.setSuspTravelDirection(i, suspTravelDirections[i]);
		wheelsData.setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
		wheelsData.setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
		wheelsData.setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
	}

	//Set the car to perform 3 sub-steps when it moves with a forwards speed of less than 5.0 
	//and with a single step when it moves at speed greater than or equal to 5.0.
//	wheelsData.setSubStepCount(5.0f, 3, 1);
//	wheelsData.setSubStepCount(5.0f, 1, 1);
//	wheelsData.setSubStepCount(5.0f, 32, 32);
//	wheelsData.setSubStepCount(5.0f, 100, 100);
	wheelsData.setSubStepCount(5.0f, desc.mWheelSubsteps, desc.mWheelSubsteps);

	//Now set up the differential, engine, gears, clutch, and ackermann steering.

	//Diff
	PxVehicleDifferential4WData diff;
	diff.mType					= PxVehicleDifferential4WData::Enum(desc.mDifferential);
	diff.mFrontRearSplit		= desc.mFrontRearSplit;
	diff.mFrontLeftRightSplit	= desc.mFrontLeftRightSplit;
	diff.mRearLeftRightSplit	= desc.mRearLeftRightSplit;
	diff.mCentreBias			= desc.mCentreBias;
	diff.mFrontBias				= desc.mFrontBias;
	diff.mRearBias				= desc.mRearBias;
	driveData.setDiffData(diff);
	
	//Engine
	PxVehicleEngineData engine;
	engine.mMOI			= desc.mEngineMOI;
	engine.mPeakTorque	= desc.mEnginePeakTorque;
	engine.mMaxOmega	= desc.mEngineMaxOmega;

//			engine.mDampingRateFullThrottle = 1.0f;
//			engine.mDampingRateZeroThrottleClutchEngaged = 1.0f;
//			engine.mDampingRateZeroThrottleClutchDisengaged = 1.0f;

	driveData.setEngineData(engine);

	//Gears
	PxVehicleGearsData gears;
	gears.mSwitchTime = desc.mGearsSwitchTime;
	driveData.setGearsData(gears);

	//Clutch
	PxVehicleClutchData clutch;
	clutch.mStrength = desc.mClutchStrength;
	driveData.setClutchData(clutch);

	//Ackermann steer accuracy
	PxVehicleAckermannGeometryData ackermann;
	ackermann.mAccuracy			= desc.mAckermannAccuracy;
	ackermann.mAxleSeparation	= fabsf(wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].z - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].z);
	ackermann.mFrontWidth		= fabsf(wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].x - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].x);
	ackermann.mRearWidth		= fabsf(wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].x - wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT].x);
	driveData.setAckermannGeometryData(ackermann);
}

//#define DISABLE_WHEELS

PxVehicleDrive4W* SampleVehicle_VehicleManager::create4WVehicle(SharedPhysX_Vehicles& sharedPhysX,
																const PxMaterial& material,
																PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
																bool useAutoGearFlag, const PINT_VEHICLE_CREATE& desc, bool useSphereWheels)
{
	PX_ASSERT(mNumVehicles<MAX_NUM_4W_VEHICLES);

	PxPhysics& physics = *sharedPhysX.GetPhysics();

	PxScene* scene = sharedPhysX.GetScene();
	ASSERT(scene);

//	mVehicleManager.init(physics, (const PxMaterial**)mStandardMaterials, mVehicleDrivableSurfaceTypes);
//	setTireFrictionMultiplier(mNumVehicles, desc.mFrontTireFrictionMultiplier, desc.mRearTireFrictionMultiplier);
	//void SampleVehicle_VehicleManager::setTireFrictionMultiplier(udword vehicle_id, float frontTireFrictionMultiplier, float rearTireFrictionMultiplier)
	{
		const udword vehicle_id = mNumVehicles;
		float frontTireFrictionMultiplier = desc.mFrontTireFrictionMultiplier;
		float rearTireFrictionMultiplier = desc.mRearTireFrictionMultiplier;

		ASSERT(vehicle_id<MAX_NUM_4W_VEHICLES);

		//###############
	/*	for(PxU32 i=0;i<MAX_NUM_SURFACE_TYPES;i++)
		{
			for(PxU32 j=0;j<MAX_NUM_TIRE_TYPES;j++)
			{
	//			mSurfaceTirePairs->setTypePairFriction(i,j,gTireFrictionMultipliers[i][j]);
				//### 
				mSurfaceTirePairs->setTypePairFriction(i, j, tireFrictionMultiplier);
			}
		}*/
		// One tire type & one surface type for now
		mSurfaceTirePairs->setTypePairFriction(0, vehicle_id*2+0, frontTireFrictionMultiplier);
		mSurfaceTirePairs->setTypePairFriction(0, vehicle_id*2+1, rearTireFrictionMultiplier);
	}

	PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(4);
	PxVehicleDriveSimData4W driveSimData;
	PxVehicleChassisData chassisData;
	createVehicle4WSimulationData(mNumVehicles, chassisConvexMesh, wheelConvexMeshes4, *wheelsSimData, driveSimData, chassisData, desc);

	//Instantiate and finalize the vehicle using physx.
	PxRigidDynamic* vehActor = createVehicleActor4W(sharedPhysX, chassisData, wheelConvexMeshes4, chassisConvexMesh, *scene, physics, material, ToPxTransform(desc.mChassisLocalPose), useSphereWheels);

#ifdef DISABLE_WHEELS
	wheelsSimData->disableWheel(2);
	wheelsSimData->disableWheel(3);
#endif

#if PHYSX_SUPPORT_LIMIT_SUSPENSION_EXPANSION_VELOCITY
	wheelsSimData->setFlags(PxVehicleWheelsSimFlag::eLIMIT_SUSPENSION_EXPANSION_VELOCITY);
#endif
#if PHYSX_SUPPORT_NEW_VEHICLE_SUSPENSION_FLAGS
	wheelsSimData->setFlags(PxVehicleWheelsSimFlag::eLIMIT_SUSPENSION_EXPANSION_VELOCITY|PxVehicleWheelsSimFlag::eDISABLE_INTERNAL_CYLINDER_PLANE_INTERSECTION_TEST|PxVehicleWheelsSimFlag::eDISABLE_SUSPENSION_FORCE_PROJECTION);
#endif

#if PHYSX_SUPPORT_ANTIROLLBAR
	if(desc.mAntirollBarStiffness!=0.0f)
	{
		PxVehicleAntiRollBarData arb;
		arb.mWheel0 = 0;
		arb.mWheel1 = 1;
//		arb.mStiffness	= 50000.0f;
		arb.mStiffness	= desc.mAntirollBarStiffness;
		wheelsSimData->addAntiRollBarData(arb);
		arb.mWheel0 = 2;
		arb.mWheel1 = 3;
		wheelsSimData->addAntiRollBarData(arb);
	}
#endif

	//Create a car.
	PxVehicleDrive4W* car = PxVehicleDrive4W::allocate(4);
	car->setup(&physics,vehActor,*wheelsSimData,driveSimData,0);

	//Free the sim data because we don't need that any more.
	wheelsSimData->free();

	//Don't forget to add the actor to the scene.
	{
		//PxSceneWriteLock scopedLock(*scene);
		sharedPhysX.AddActorToScene(vehActor);
	}

	//Set up the mapping between wheel and actor shape.
	car->mWheelsSimData.setWheelShapeMapping(0,0);
	car->mWheelsSimData.setWheelShapeMapping(1,1);
	car->mWheelsSimData.setWheelShapeMapping(2,2);
	car->mWheelsSimData.setWheelShapeMapping(3,3);
#ifdef DISABLE_WHEELS
	car->mWheelsSimData.setWheelShapeMapping(2, -1);
	car->mWheelsSimData.setWheelShapeMapping(3, -1);
#endif

	//Set up the scene query filter data for each suspension line.
	bool gSQFilterOutAllShapes = false;
	PxFilterData vehQryFilterData(!gSQFilterOutAllShapes, gSQFilterOutAllShapes, 0, 0);
	car->mWheelsSimData.setSceneQueryFilterData(0, vehQryFilterData);
	car->mWheelsSimData.setSceneQueryFilterData(1, vehQryFilterData);
	car->mWheelsSimData.setSceneQueryFilterData(2, vehQryFilterData);
	car->mWheelsSimData.setSceneQueryFilterData(3, vehQryFilterData);

	//Set the transform and the instantiated car and set it be to be at rest.
	const PxTransform startTransform(ToPxTransform(desc.mStartPose));
	resetNWCar(startTransform, car);
	//Set the autogear mode of the instantiate car.
	car->mDriveDynData.setUseAutoGears(useAutoGearFlag);

	//Increment the number of vehicles
	mVehicles[mNumVehicles]=car;
	mVehicleWheelQueryResults[mNumVehicles].nbWheelQueryResults=4;
	mVehicleWheelQueryResults[mNumVehicles].wheelQueryResults=mWheelQueryResults->addVehicle(4);
	mNumVehicles++;
	return car;
}

///////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4355)	// 'this' : used in base member initializer list

SharedPhysX_Vehicles::SharedPhysX_Vehicles(const EditableParams& params) :
	SharedPhysX		(params),
	Pint_Vehicle	(*this)
{
}

SharedPhysX_Vehicles::~SharedPhysX_Vehicles()
{
}

///////////////////////////////////////////////////////////////////////////////

#define WHEEL_TANGENT_VELOCITY_MULTIPLIER 0.1f
#define MAX_IMPULSE MAX_FLOAT

static udword gVehicleTimestamp = 0;

//#define MAX_NUM_INDEX_BUFFERS	16
#define MAX_NUM_INDEX_BUFFERS	1

typedef PxHashMap<const PxRigidActor*, PxVehicleDrive4W*>	ActorToVehicle;
typedef PxHashMap<const PxShape*, PxU32>					ShapeToWheelData;

namespace
{
	class VehicleTest : public Allocateable
	{
		public:
											VehicleTest(SharedPhysX_Vehicles& physx, bool use_sweeps, float sweep_inflation);
											~VehicleTest();

			struct VehiclePtrs
			{
				PxRigidDynamic*	mActor;
				PxShape*		mChassisShape;
				PxShape*		mWheelhapes[4];
			};

			PxVehicleDrive4W*				CreateVehicle(	VehiclePtrs& vehicle_ptrs,
															PxConvexMesh* chassis_mesh,
															PxConvexMesh* wheel_mesh0, PintShapeRenderer* renderer0,
															PxConvexMesh* wheel_mesh1, PintShapeRenderer* renderer1,
															PxConvexMesh* wheel_mesh2, PintShapeRenderer* renderer2,
															PxConvexMesh* wheel_mesh3, PintShapeRenderer* renderer3,
															const PINT_VEHICLE_CREATE& desc, bool useSphereWheels);

			void							Update(float dt, const PtrContainer& vehicles);

			SharedPhysX_Vehicles&			mOwner;
			ActorToVehicle					mVehicleMap;
			ShapeToWheelData				mWheelMap;
			PxScene*						mScene;
			SampleVehicle_VehicleManager	mVehicleManager;
			PxVehicleDrivableSurfaceType	mVehicleDrivableSurfaceTypes[MAX_NUM_INDEX_BUFFERS];
			PxMaterial*						mStandardMaterials[MAX_NUM_INDEX_BUFFERS];
			PxMaterial*						mChassisMaterial;
			const bool						mUseSweeps;
			const float						mSweepInflation;

#if PINT_DEBUG_VEHICLE_ON
			PxU32							mDebugRenderActiveGraphChannelWheel;
			PxU32							mDebugRenderActiveGraphChannelEngine;
			PxVehicleTelemetryData*			mTelemetryData4W;
			PxF32							mForwardSpeedHud;

			void							drawWheels();
			void							drawVehicleDebug(PintRender& renderer);
			void							drawHud(PintRender& renderer);
			void							drawGraphsAndPrintTireSurfaceTypes(PintRender& renderer, const PxVehicleWheels& focusVehicle, const PxVehicleWheelQueryResult& focusVehicleWheelQueryResults);
#endif
			void							setupTelemetryData();
			void							clearTelemetryData();
	};
}

static VehicleTest* gVehicleTest = null;

#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
namespace
{
class VehicleContactModifyCallback : public PxContactModifyCallback
{
	public:
	virtual void onContactModify(PxContactModifyPair* const pairs, PxU32 count)
	{
		if(gLogXP)
			printf("VehicleContactModifyCallback %d\n", gVehicleTimestamp);
		for(PxU32 i=0; i<count; i++)
		{
			const PxRigidActor** actors = pairs[i].actor;
			const PxShape** shapes = pairs[i].shape;

			//Search for actors that represent vehicles and shapes that represent wheels.
			for(PxU32 j=0; j<2; j++)
			{
				const PxActor* actor = actors[j];

				if(actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
				{
					const PxRigidDynamic* dyna = static_cast<const PxRigidDynamic*>(actor);

					const ActorToVehicle::Entry* isVehicle = gVehicleTest->mVehicleMap.find(dyna);
					if(isVehicle)
					{
						const PxVehicleWheels* vehicle = isVehicle->second;
						PX_ASSERT(vehicle->getRigidDynamicActor() == actor);

						const PxShape* shape = shapes[j];
						const ShapeToWheelData::Entry* isWheel = gVehicleTest->mWheelMap.find(shape);
						if(isWheel)
						//const char* ShapeName = shape->getName();	// ### VCALL
						//if(ShapeName && strncmp(ShapeName, gWheelDebugName, 5)==0)
						{
							//const PxU32 wheelId = ::atoi(ShapeName+5);	// ### I know.... we need to revisit this at some point
							const PxU32 wheelId = isWheel->second;
							PX_ASSERT(wheelId < vehicle->mWheelsSimData.getNbWheels());

							//Modify wheel contacts.
							PxVehicleModifyWheelContacts(*vehicle, wheelId, WHEEL_TANGENT_VELOCITY_MULTIPLIER, MAX_IMPULSE, pairs[i]);
						}
					}
				}
			}
		}
	}
}gVehicleContactModifyCallback;
}
#endif

VehicleTest::VehicleTest(SharedPhysX_Vehicles& owner, bool use_sweeps, float sweep_inflation) : mOwner(owner), mScene(null), mUseSweeps(use_sweeps), mSweepInflation(sweep_inflation)
{
#if PINT_DEBUG_VEHICLE_ON
	mDebugRenderActiveGraphChannelWheel = 0;
	mDebugRenderActiveGraphChannelEngine = 0;
	mTelemetryData4W = null;
	mForwardSpeedHud = 0.0f;
#endif

	mScene = owner.GetScene();
	PxPhysics& physics = *owner.GetPhysics();

	//mChassisMaterial = physics.createMaterial(0.0f, 0.0f, 0.0f);
	mChassisMaterial = physics.createMaterial(0.5f, 0.5f, 0.0f);
	ASSERT(mChassisMaterial);

	for(PxU32 i=0;i<MAX_NUM_INDEX_BUFFERS;i++)
	{
		mStandardMaterials[i] = NULL;
		mVehicleDrivableSurfaceTypes[i].mType = PxVehicleDrivableSurfaceType::eSURFACE_TYPE_UNKNOWN;
	}

/*	const PxF32 restitutions[MAX_NUM_SURFACE_TYPES] = {0.2f, 0.2f, 0.2f, 0.2f};
	const PxF32 staticFrictions[MAX_NUM_SURFACE_TYPES] = {0.5f, 0.5f, 0.5f, 0.5f};
	const PxF32 dynamicFrictions[MAX_NUM_SURFACE_TYPES] = {0.5f, 0.5f, 0.5f, 0.5f};

	for(PxU32 i=0;i<MAX_NUM_SURFACE_TYPES;i++) 
	{
		//Create a new material.
		mStandardMaterials[i] = physics.createMaterial(staticFrictions[i], dynamicFrictions[i], restitutions[i]);
		ASSERT(mStandardMaterials[i]);

		//Set up the drivable surface type that will be used for the new material.
		mVehicleDrivableSurfaceTypes[i].mType = i;
	}*/

	const float restitution = 0.2f;
	const float staticFriction = 0.5f;
	const float dynamicFriction = 0.5f;
	mStandardMaterials[0] = physics.createMaterial(staticFriction, dynamicFriction, restitution);
	ASSERT(mStandardMaterials[0]);

	//Set up the drivable surface type that will be used for the new material.
	mVehicleDrivableSurfaceTypes[0].mType = 0;

	mVehicleManager.init(physics, *mScene, (const PxMaterial**)mStandardMaterials, mVehicleDrivableSurfaceTypes, use_sweeps);

#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
	if(!gSetupCollisionGroups)
	//if(gSetupVehicleContactModif)
	{
		if(mScene->getContactModifyCallback())
		{
			printf("ERROR: test already has a contact modify callback!\n");
		}
		else
		{
			mScene->setContactModifyCallback(&gVehicleContactModifyCallback);
			gFilterShaderExtraPairFlags |= PxPairFlag::eMODIFY_CONTACTS;
		}
	}
#endif

	setupTelemetryData();
}

VehicleTest::~VehicleTest()
{
#if PINT_DEBUG_VEHICLE_ON
	if(mTelemetryData4W)
	{
		mTelemetryData4W->free();
		mTelemetryData4W = null;
	}
#endif
	mVehicleManager.shutdown();
#if PHYSX_SUPPORT_VEHICLE5
#else
	gMyBatchQuery.release();
#endif
}

void VehicleTest::setupTelemetryData()
{
#if PINT_DEBUG_VEHICLE_ON
	if(!mTelemetryData4W)
	{
		mDebugRenderActiveGraphChannelWheel=PxVehicleWheelGraphChannel::eWHEEL_OMEGA;
		mDebugRenderActiveGraphChannelEngine=PxVehicleDriveGraphChannel::eENGINE_REVS;

		const PxF32 graphSizeX=0.25f;
		const PxF32 graphSizeY=0.25f;
		const PxF32 engineGraphPosX=0.5f;
		const PxF32 engineGraphPosY=0.5f;
		const PxF32 wheelGraphPosX[4]={0.75f,0.25f,0.75f,0.25f};
		const PxF32 wheelGraphPosY[4]={0.75f,0.75f,0.25f,0.25f};
		const PxVec3 backgroundColor(255,255,255);
		const PxVec3 lineColorHigh(255,0,0);
		const PxVec3 lineColorLow(0,0,0);

		mTelemetryData4W = PxVehicleTelemetryData::allocate(4);

		mTelemetryData4W->setup
			 (graphSizeX,graphSizeY,
			  engineGraphPosX,engineGraphPosY,
			  wheelGraphPosX,wheelGraphPosY,
			  backgroundColor,lineColorHigh,lineColorLow);
	}
#endif
}

void VehicleTest::clearTelemetryData()
{
#if PINT_DEBUG_VEHICLE_ON
	if(mTelemetryData4W)
		mTelemetryData4W->clear();
#endif
}

void VehicleTest::Update(float dt, const PtrContainer& vehicles)
{
	if(!gUpdateVehicles)
		return;
	gVehicleTimestamp++;

	{
		const udword NbVehicles = vehicles.GetNbEntries();
		for(udword i=0;i<NbVehicles;i++)
		{
			VehicleData* VD = reinterpret_cast<VehicleData*>(vehicles.GetEntry(i));

			const PINT_VEHICLE_INPUT& Input = VD->mInput;

			VD->mVehicleController.setCarKeyboardInputs(
				Input.mKeyboard_Accelerate,
				Input.mKeyboard_Brake,
				Input.mKeyboard_HandBrake,
				// ### TODO: investigate why left/right is flipped here
				Input.mKeyboard_Right,
				Input.mKeyboard_Left,
				false,	// Gear up key (manual)
				false	// Gear down key (manual)
				);

			VD->mVehicleController.setCarGamepadInputs(
				Input.mGamepad_Accel,
				Input.mGamepad_Brake,
				Input.mGamepad_Steer,
				false,	// Gear up key (manual)
				false,	// Gear down key (manual)
				Input.mGamepad_Handbrake
				);

			ASSERT(VD->mVehicle==mVehicleManager.getVehicle(i));

			VD->mVehicleController.update(dt, mVehicleManager.getVehicleWheelQueryResults(i/*mPlayerVehicle*/), *mVehicleManager.getVehicle(i/*mPlayerVehicle*/), *VD);
		}
	}

	if(0)
	{
		const udword NbInternalVehicleSubsteps = 8;
		const float vdt = dt/float(NbInternalVehicleSubsteps);
		if(vdt>0.0f)
		{
			for(udword i=0;i<NbInternalVehicleSubsteps;i++)
			{
				mVehicleManager.suspensionRaycasts(mScene, mUseSweeps, mSweepInflation);

#if PINT_DEBUG_VEHICLE_ON
				if(mTelemetryData4W)
					mVehicleManager.updateAndRecordTelemetryData(vdt, mScene->getGravity(), mVehicleManager.getVehicle(mPlayerVehicle), mTelemetryData4W);
#else
				mVehicleManager.update(vdt, mScene->getGravity());
#endif
			}
		}
	}
	else
	{
		mVehicleManager.suspensionRaycasts(mScene, mUseSweeps, mSweepInflation);

		if(1 && dt>0.0f)
		{
#if PINT_DEBUG_VEHICLE_ON
			if(mTelemetryData4W)
				mVehicleManager.updateAndRecordTelemetryData(dt, mScene->getGravity(), mVehicleManager.getVehicle(mPlayerVehicle), mTelemetryData4W);
#else
			mVehicleManager.update(dt, mScene->getGravity());
#endif
		}
		//mVehicleManager.suspensionRaycasts(mScene);
	}


#if PINT_DEBUG_VEHICLE_ON
	{
		//PxSceneReadLock scopedLock(*mScene);

		//Cache forward speed for the HUD to avoid making API calls while vehicle update is running
		const PxVehicleWheels& focusVehicle = *mVehicleManager.getVehicle(mPlayerVehicle);
		mForwardSpeedHud = focusVehicle.computeForwardSpeed();
	}
#endif
}

PxVehicleDrive4W* VehicleTest::CreateVehicle(	VehiclePtrs& vehicle_ptrs,
												PxConvexMesh* chassis_mesh,
												PxConvexMesh* wheel_mesh0, PintShapeRenderer* renderer0,
												PxConvexMesh* wheel_mesh1, PintShapeRenderer* renderer1,
												PxConvexMesh* wheel_mesh2, PintShapeRenderer* renderer2,
												PxConvexMesh* wheel_mesh3, PintShapeRenderer* renderer3,
												const PINT_VEHICLE_CREATE& desc, bool useSphereWheels)
{
	PxConvexMesh* wm[4] = { wheel_mesh0, wheel_mesh1, wheel_mesh2, wheel_mesh3 };

	PxVehicleDrive4W* Vehicle4W = mVehicleManager.create4WVehicle(mOwner, *mChassisMaterial, chassis_mesh, wm, true, desc, useSphereWheels);
	ASSERT(Vehicle4W);

	PxRigidDynamic* RD = Vehicle4W->getRigidDynamicActor();
	ASSERT(RD->getNbShapes()==5);
	PxShape* Shapes[16];
	RD->getShapes(Shapes, 16);

	vehicle_ptrs.mActor = RD;
	for(udword i=0;i<4;i++)
		vehicle_ptrs.mWheelhapes[i] = Shapes[i];
	vehicle_ptrs.mChassisShape	= Shapes[4];

	Shapes[0]->userData = renderer0;//desc.mWheels.mRenderer;
	Shapes[1]->userData = renderer1;//desc.mWheels.mRenderer;
	Shapes[2]->userData = renderer2;//desc.mWheels.mRenderer;
	Shapes[3]->userData = renderer3;//desc.mWheels.mRenderer;
	Shapes[4]->userData = desc.mChassis->mRenderer;

	for(udword i=0;i<4;i++)
		mWheelMap.insert(Shapes[i], i);

	// The userData pointer is already used by the generic actor manager.
	mVehicleMap.insert(RD, Vehicle4W);

	return Vehicle4W;
}

PintVehicleHandle SharedPhysX_Vehicles::CreateVehicle(PintVehicleData& data, const PINT_VEHICLE_CREATE& vehicle)
{
	if(!vehicle.mChassis || vehicle.mChassis->mType!=PINT_SHAPE_CONVEX)
		return null;

	if(!vehicle.mWheels[0] || vehicle.mWheels[0]->mType!=PINT_SHAPE_CONVEX)
		return null;

//	if(vehicle.mWheels[1] && (!vehicle.mWheels[2] || !vehicle.mWheels[3]))
//		return null;

	const float* src = vehicle.mSmoothingData;
	for(udword i=0;i<2;i++)
	{
		const float AccelRise = *src++;
		const float AccelFall = *src++;
		const float BrakeRise = *src++;
		const float BrakeFall = *src++;
		const float HandBrakeRise = *src++;
		const float HandBrakeFall = *src++;
		const float SteeringRise = *src++;
		const float SteeringFall = *src++;

		float* dst = i ? gCarPadSmoothingData.mRiseRates : gKeySmoothingData.mRiseRates;
		*dst++ = AccelRise;
		*dst++ = BrakeRise;
		*dst++ = HandBrakeRise;
		*dst++ = SteeringRise;
		*dst++ = SteeringRise;
		*dst++ = AccelFall;
		*dst++ = BrakeFall;
		*dst++ = HandBrakeFall;
		*dst++ = SteeringFall;
		*dst++ = SteeringFall;	
	}

#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
	const bool UseSweeps = mParams.mUseSuspensionSweeps;
#else
	const bool UseSweeps = false;
#endif

#if PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
	const float SweepInflation = mParams.mSweepInflation;
#else
	const float SweepInflation = 0.0f;
#endif

	if(!gVehicleTest)
		gVehicleTest = ICE_NEW(VehicleTest)(*this, UseSweeps, SweepInflation);

	const PxConvexFlags convexFlags = mParams.GetConvexFlags();

	const PINT_CONVEX_CREATE* ChassisDesc = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mChassis);
	PxConvexMesh* ChassisMesh = CreateConvexMesh(ChassisDesc->mVerts, ChassisDesc->mNbVerts, convexFlags, vehicle.mChassis->mRenderer, mParams.mShareMeshData);
	ASSERT(ChassisMesh);

	const PINT_CONVEX_CREATE* WheelDesc0 = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mWheels[0]);
	PxConvexMesh* WheelMesh0 = CreateConvexMesh(WheelDesc0->mVerts, WheelDesc0->mNbVerts, convexFlags, WheelDesc0->mRenderer, mParams.mShareMeshData);
	ASSERT(WheelMesh0);

	PintShapeRenderer* Renderer1 = WheelDesc0->mRenderer;
	PintShapeRenderer* Renderer2 = WheelDesc0->mRenderer;
	PintShapeRenderer* Renderer3 = WheelDesc0->mRenderer;

	PxConvexMesh* WheelMesh1 = WheelMesh0;
	PxConvexMesh* WheelMesh2 = WheelMesh0;
	PxConvexMesh* WheelMesh3 = WheelMesh0;

	if(vehicle.mWheels[1])
	{
		ASSERT(vehicle.mWheels[1]->mType==PINT_SHAPE_CONVEX);
		const PINT_CONVEX_CREATE* wheel1 = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mWheels[1]);
		WheelMesh1 = CreateConvexMesh(wheel1->mVerts, wheel1->mNbVerts, convexFlags, wheel1->mRenderer, mParams.mShareMeshData);
		ASSERT(WheelMesh1);
		Renderer1 = wheel1->mRenderer;

		ASSERT(vehicle.mWheels[2]->mType==PINT_SHAPE_CONVEX);
		const PINT_CONVEX_CREATE* wheel2 = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mWheels[2]);
		WheelMesh2 = CreateConvexMesh(wheel2->mVerts, wheel2->mNbVerts, convexFlags, wheel2->mRenderer, mParams.mShareMeshData);
		ASSERT(WheelMesh2);
		Renderer2 = wheel2->mRenderer;
		
		ASSERT(vehicle.mWheels[3]->mType==PINT_SHAPE_CONVEX);
		const PINT_CONVEX_CREATE* wheel3 = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mWheels[3]);
		WheelMesh3 = CreateConvexMesh(wheel3->mVerts, wheel3->mNbVerts, convexFlags, wheel3->mRenderer, mParams.mShareMeshData);
		ASSERT(WheelMesh3);
		Renderer3 = wheel3->mRenderer;
	}

#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
	const bool forceSphereWheels = mParams.mForceSphereWheels;
#else
	const bool forceSphereWheels = false;
#endif

	VehicleTest::VehiclePtrs VP;
	PxVehicleDrive4W* Vehicle4W = gVehicleTest->CreateVehicle(	VP, ChassisMesh,
																WheelMesh0, WheelDesc0->mRenderer,
																WheelMesh1, Renderer1,
																WheelMesh2, Renderer2,
																WheelMesh3, Renderer3,
																vehicle, forceSphereWheels);
	ASSERT(Vehicle4W);

	//data.mChassis = PintActorHandle(Vehicle4W->getRigidDynamicActor());
	data.mChassisActor		= PintActorHandle(VP.mActor);
	data.mChassisShape		= PintShapeHandle(VP.mChassisShape);
	for(udword i=0;i<4;i++)
		data.mWheelShapes[i] = PintShapeHandle(VP.mWheelhapes[i]);

	VehicleData* VD = ICE_NEW(VehicleData);
#if PHYSX_SUPPORT_STEER_FILTER
	VD->mSteerFilter.mSharpness = mParams.mSteerFilter;
#endif
	VD->mThresholdForwardSpeed	= vehicle.mThresholdForwardSpeed;
	VD->mImmediateAutoReverse	= vehicle.mImmediateAutoReverse;
	VD->mVehicle = Vehicle4W;
	CopyMemory(VD->mSteerVsForwardSpeedData, gDefaultSteerVsForwardSpeedData, sizeof(float)*16);
	CopyMemory(VD->mSteerVsForwardSpeedData, vehicle.mSteerVsForwardSpeedData, sizeof(float)*8);

	mVehicles.AddPtr(VD);

	return PintVehicleHandle(VD);
}

bool SharedPhysX_Vehicles::SetVehicleInput(PintVehicleHandle vehicle, const PINT_VEHICLE_INPUT& input)
{
	VehicleData* VD = reinterpret_cast<VehicleData*>(vehicle);
	PxVehicleDrive4W* Vehicle4W = VD->mVehicle;
	if(!Vehicle4W)
		return false;

	VD->mInput = input;
//	if(gVehicleTest)
//		gVehicleTest->mInput = input;

	return true;
}

PintActorHandle SharedPhysX_Vehicles::GetVehicleActor(PintVehicleHandle vehicle) const
{
	VehicleData* VD = reinterpret_cast<VehicleData*>(vehicle);
	PxVehicleDrive4W* Vehicle4W = VD->mVehicle;
	if(!Vehicle4W)
		return null;
	return PintActorHandle(Vehicle4W->getRigidDynamicActor());
}

bool SharedPhysX_Vehicles::GetVehicleInfo(PintVehicleHandle vehicle, PintVehicleInfo& info) const
{
	VehicleData* VD = reinterpret_cast<VehicleData*>(vehicle);
	PxVehicleDrive4W* Vehicle4W = VD->mVehicle;
	if(!Vehicle4W)
		return false;

	const PxVehicleDriveDynData& driveDynData = Vehicle4W->mDriveDynData;

	info.mForwardSpeed	= Vehicle4W->computeForwardSpeed();
	info.mSidewaysSpeed	= Vehicle4W->computeSidewaysSpeed();
	info.mCurrentGear	= driveDynData.getCurrentGear();
	info.mRevs			= driveDynData.getEngineRotationSpeed();
	return true;
}

bool SharedPhysX_Vehicles::ResetVehicleData(PintVehicleHandle vehicle)
{
	VehicleData* VD = reinterpret_cast<VehicleData*>(vehicle);
	PxVehicleDrive4W* Vehicle4W = VD->mVehicle;
	if(!Vehicle4W)
		return false;

	// Set the car back to its rest state.
	Vehicle4W->setToRestState();

	// Set the car to first gear.
	Vehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);

	if(gVehicleTest)
		gVehicleTest->clearTelemetryData();

	return true;
}

bool SharedPhysX_Vehicles::AddActor(PintVehicleHandle vehicle, PintActorHandle actor)
{
	VehicleData* VD = reinterpret_cast<VehicleData*>(vehicle);
	PxVehicleDrive4W* Vehicle4W = VD->mVehicle;
	if(!Vehicle4W)
		return false;

	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(actor);
	ASSERT(Actor);

	//ASSERT(!Actor->getName());
	//Actor->setName(gChassisDebugName);

	const PxU32 nbShapes = Actor->getNbShapes();
	for(PxU32 i=0;i<nbShapes;i++)
	{
		PxShape* shape = null;
		Actor->getShapes(&shape, 1, i);
		ASSERT(shape);
		shape->setName(gChassisDebugName);
	}
	return true;
}

bool SharedPhysX_Vehicles::AddShape(PintVehicleHandle vehicle, const PINT_SHAPE_CREATE& create)
{
	VehicleData* VD = reinterpret_cast<VehicleData*>(vehicle);
	PxVehicleDrive4W* Vehicle4W = VD->mVehicle;
	if(!Vehicle4W)
		return false;

	//CreateShapes(&create, Vehicle4W->getRigidDynamicActor(), gChassisCollisionGroup, gChassisDebugName);

	LocalShapeCreationParams Params;
	Params.mForcedName	= gChassisDebugName;
	Params.mActor		= Vehicle4W->getRigidDynamicActor();
	Params.mGroup		= gChassisCollisionGroup;
	Params.mIsDynamic	= Params.mActor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC;
	ReportShape(create, 0, &Params);
	return true;
}

void SharedPhysX_Vehicles::CloseVehicles()
{
	DELETESINGLE(gVehicleTest);
	const udword NbPtrs = mVehicles.GetNbEntries();
	for(udword i=0;i<NbPtrs;i++)
	{
		VehicleData* VD = reinterpret_cast<VehicleData*>(mVehicles.GetEntry(i));
		DELETESINGLE(VD);
	}
	mVehicles.Empty();
}

void SharedPhysX_Vehicles::UpdateVehicles(float dt)
{
	if(gVehicleTest)
	{
//		gVehicleTest->Update(dt);
		const udword NbSubsteps = mParams.mNbSubsteps;
		const float sdt = dt/float(NbSubsteps);
		for(udword i=0;i<NbSubsteps;i++)
		{
			gVehicleTest->Update(sdt, mVehicles);
		}
	}
}

#if PHYSX_SUPPORT_GEAR_JOINT_
	#include "PxGearJoint.h"
#endif
void SharedPhysX_Vehicles::UpdateVehiclesAndPhysX(float dt)
{
	mActorManager.UpdateTimestamp();

	if(gDebugPts)
		gDebugPts->Reset();
	if(gDebugSegments)
		gDebugSegments->Reset();
	if(gDebugSegments2)
		gDebugSegments2->Reset();

#ifdef PHYSX_SUPPORT_VEHICLE_FIX
	gUseFix = mParams.mUseFix;
#endif

#ifdef PHYSX_SUPPORT_VEHICLE_XP
	gVehicleXP = true;
//#else
//	gVehicleXP = false;
#endif

#if PHYSX_SUPPORT_VEHICLE5
	if(gVehicleTest)
		gVehicleTest->mVehicleManager.mBatchQueryExt->resetSQRecorder();
#else
//	gMyBatchQuery.mSweeps.Reset();
//	gMyBatchQuery.mRaycasts.Reset();
	gMyBatchQuery.mRecorder.Reset();
#endif

#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
	bool useBlockingSweeps = mParams.mUseBlockingSweeps;
	if(gVehicleTest && !gVehicleTest->mUseSweeps)
		useBlockingSweeps = true;	// Raycasts don't support touch hits
	
	#if PHYSX_SUPPORT_VEHICLE5
	gMyFilter.mUseBlockingSweeps = useBlockingSweeps;
	#else
	gMyBatchQuery.mUseBlockingSweeps = useBlockingSweeps;
	#endif
#endif

#if PHYSX_SUPPORT_GEAR_JOINT_
	if(0)
	{
		const udword NbGearJoints = mGearJoints.size();
		for(udword i=0;i<NbGearJoints;i++)
		{
			PxGearJoint* j = mGearJoints[i];
			j->updateError();
		}
	}
#endif

	if(0)
	{
		UpdateVehicles(dt);
		UpdateCommon(dt);
	}
	else
	{
#ifdef TEST_FLUIDS
		if(gParticleSystem)
			onBeforeRenderParticles(gParticleSystem);
#endif

		const udword NbSubsteps = mParams.mNbSubsteps;
		const float sdt = dt/float(NbSubsteps);
		for(udword i=0;i<NbSubsteps;i++)
		{
			if(0 && gVehicleTest)
				gVehicleTest->Update(sdt, mVehicles);

			if(mScene)
			{
				if(NbSubsteps>1)
					ApplyLocalTorques();

#if PHYSX_SUPPORT_SQ_UPDATE_MODE
				const bool IsLastSubstep = i==NbSubsteps-1;
				mScene->setSceneQueryUpdateMode(IsLastSubstep ? PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_ENABLED : PxSceneQueryUpdateMode::eBUILD_DISABLED_COMMIT_DISABLED);
#endif
				mScene->simulate(sdt, null, GetScratchPad(), GetScratchPadSize());
				mScene->fetchResults(true);
//				mScene->fetchResults(false);
#if TEST_FLUIDS
				mScene->fetchResultsParticleSystem();
#endif

				UpdateJointErrors();
			}

			// Works better here in case we move the car kinematically, or when it falls from a distance to the ground
			if(1 && gVehicleTest)
				gVehicleTest->Update(sdt, mVehicles);

#ifdef PHYSX_SUPPORT_VEHICLE_XP
void resetIgnoredPoints();
resetIgnoredPoints();
#endif
		}

		EndCommonUpdate();
	}
}


void drawPt(const PxVec3& pos)
{
	if(!gDebugPts)
		gDebugPts = ICE_NEW(Vertices);
	gDebugPts->AddVertex(ToPoint(pos));
}

void drawSeg(const PxVec3& p0, const PxVec3& p1)
{
	if(!gDebugSegments)
		gDebugSegments = ICE_NEW(Vertices);
	gDebugSegments->AddVertex(ToPoint(p0));
	gDebugSegments->AddVertex(ToPoint(p1));
}

void drawSeg(const PxVec3& p0, const PxVec3& p1, const PxVec3& color)
{
	if(!gDebugSegments2)
		gDebugSegments2 = ICE_NEW(Vertices);
	gDebugSegments2->AddVertex(ToPoint(p0));
	gDebugSegments2->AddVertex(ToPoint(p1));
	gDebugSegments2->AddVertex(ToPoint(color));
}

void SharedPhysX_Vehicles::RenderDebugData(PintRender& renderer)
{
	SharedPhysX::RenderDebugData(renderer);

#if PINT_DEBUG_VEHICLE_ON
	if(gVehicleTest && mParams.mDrawPhysXTelemetry)
	{
		gVehicleTest->drawHud(renderer);
		gVehicleTest->drawVehicleDebug(renderer);
		gVehicleTest->drawGraphsAndPrintTireSurfaceTypes(renderer, *gVehicleTest->mVehicleManager.getVehicle(mPlayerVehicle), gVehicleTest->mVehicleManager.getVehicleWheelQueryResults(mPlayerVehicle));
	}
#endif

	if(gDebugPts)
	{
		const udword Nb = gDebugPts->GetNbVertices();
		const Point* V = gDebugPts->GetVertices();
		for(udword i=0;i<Nb;i++)
		{
			const Point& pt = V[i];
			const float Scale = 0.6f;
			renderer.DrawLine(pt, pt + Point(Scale, 0.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
			renderer.DrawLine(pt, pt + Point(0.0f, Scale, 0.0f), Point(0.0f, 1.0f, 0.0f));
			renderer.DrawLine(pt, pt + Point(0.0f, 0.0f, Scale), Point(0.0f, 0.0f, 1.0f));
		}

		//gDebugPts->Reset();
	}

	if(gDebugSegments)
	{
		const udword Nb = gDebugSegments->GetNbVertices();
		const Point* V = gDebugSegments->GetVertices();
		for(udword i=0;i<Nb/2;i++)
		{
			const Point& p0 = V[i*2+0];
			const Point& p1 = V[i*2+1];
			renderer.DrawLine(p0, p1, Point(1.0f, 1.0f, 0.0f));
		}

		//gDebugSegments->Reset();
	}

	if(gDebugSegments2)
	{
		const udword Nb = gDebugSegments2->GetNbVertices();
		const Point* V = gDebugSegments2->GetVertices();
		for(udword i=0;i<Nb/3;i++)
		{
			const Point& p0 = V[i*3+0];
			const Point& p1 = V[i*3+1];
			renderer.DrawLine(p0, p1, V[i*3+2]);
		}

		//gDebugSegments2->Reset();
	}

	const bool RecordSQ = mParams.mDrawSuspensionCasts
#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
		|| mParams.mDrawSweptWheels
#endif
		;

#if PHYSX_SUPPORT_VEHICLE5
	if(gVehicleTest)
	{
		gVehicleTest->mVehicleManager.mBatchQueryExt->drawSQSweeps(renderer, mParams.mDrawSweptWheels && mParams.mUseBlockingSweeps);
		gVehicleTest->mVehicleManager.mBatchQueryExt->drawSQRaycasts(renderer);
		gVehicleTest->mVehicleManager.mBatchQueryExt->enableSQRecorder(RecordSQ);
	}
#else
	#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
	gMyBatchQuery.mRecorder.DrawSweeps(renderer, gMyBatchQuery.mDesc.queryMemory.userSweepResultBuffer, mParams.mDrawSweptWheels && mParams.mUseBlockingSweeps);
	#endif
	gMyBatchQuery.mRecorder.DrawRaycasts(renderer, gMyBatchQuery.mDesc.queryMemory.userRaycastResultBuffer);
	gMyBatchQuery.mRecorder.SetEnabled(RecordSQ);
#endif
}

Pint_Vehicle* SharedPhysX_Vehicles::GetVehicleAPI()
{
	return this;
}



#if PINT_DEBUG_VEHICLE_ON
void VehicleTest::drawWheels()
{
/*	PxSceneReadLock scopedLock(*mScene);
	const RendererColor colorPurple(255, 0, 255);

	for(PxU32 i=0;i<mVehicleManager.getNbVehicles();i++)
	{
		//Draw a rotating arrow to get an idea of the wheel rotation speed.
		PxVehicleWheels* veh=mVehicleManager.getVehicle(i);
		const PxRigidDynamic* actor=veh->getRigidDynamicActor();
		PxShape* shapeBuffer[PX_MAX_NB_WHEELS];
		actor->getShapes(shapeBuffer,veh->mWheelsSimData.getNbWheels());
		const PxTransform vehGlobalPose=actor->getGlobalPose();
		const PxU32 numWheels=veh->mWheelsSimData.getNbWheels();
		for(PxU32 j=0;j<numWheels;j++)
		{
			const PxTransform wheelTransform=vehGlobalPose.transform(shapeBuffer[j]->getLocalPose());
			const PxF32 wheelRadius=veh->mWheelsSimData.getWheelData(j).mRadius;
			const PxF32 wheelHalfWidth=veh->mWheelsSimData.getWheelData(j).mWidth*0.5f;
			PxVec3 offset=wheelTransform.q.getBasisVector0()*wheelHalfWidth;
			offset*= (veh->mWheelsSimData.getWheelCentreOffset(j).x > 0) ? 1.0f : -1.0f;
			const PxVec3 arrow=wheelTransform.rotate(PxVec3(0,0,1));
			getDebugRenderer()->addLine(wheelTransform.p+offset, wheelTransform.p+offset+arrow*wheelRadius, colorPurple);
		}
	}*/
}

void VehicleTest::drawVehicleDebug(PintRender& renderer)
{
//	PxSceneReadLock scopedLock(*mScene);
//	const RendererColor colorColl(255, 0, 0);
//	const RendererColor colorCol2(0, 255, 0);
//	const RendererColor colorCol3(0, 0, 255);
	const Point colorColl(1.0f, 0, 0);
	const Point colorCol2(0, 1.0f, 0);
	const Point colorCol3(0, 0, 1.0f);
	
	const PxVec3* tireForceAppPoints=mTelemetryData4W->getTireforceAppPoints();
	const PxVec3* suspForceAppPoints=mTelemetryData4W->getSuspforceAppPoints();

	const PxVehicleWheels& vehicle4W=*mVehicleManager.getVehicle(mPlayerVehicle);
	const PxVehicleWheelQueryResult& vehicleWheelQueryResults=mVehicleManager.getVehicleWheelQueryResults(mPlayerVehicle);
	const PxRigidDynamic* actor=vehicle4W.getRigidDynamicActor();
	const PxU32 numWheels=vehicle4W.mWheelsSimData.getNbWheels();
	PxVec3 v[8];
	PxVec3 w[8];
	PxF32 l[8];
	for(PxU32 i=0;i<numWheels;i++)
	{
		v[i] = vehicleWheelQueryResults.wheelQueryResults[i].suspLineStart;
		w[i] = vehicleWheelQueryResults.wheelQueryResults[i].suspLineDir;
		l[i] = vehicleWheelQueryResults.wheelQueryResults[i].suspLineLength;
	}

	const PxTransform t=actor->getGlobalPose().transform(actor->getCMassLocalPose());
	const PxVec3 dirs[3]={t.rotate(PxVec3(1,0,0)),t.rotate(PxVec3(0,1,0)),t.rotate(PxVec3(0,0,1))};
//	getDebugRenderer()->addLine(t.p, t.p + dirs[0]*4.0f, colorColl);
//	getDebugRenderer()->addLine(t.p, t.p + dirs[1]*4.0f, colorColl);
//	getDebugRenderer()->addLine(t.p, t.p + dirs[2]*4.0f, colorColl);
	renderer.DrawLine(ToPoint(t.p), ToPoint(t.p + dirs[0]*4.0f), colorColl);
	renderer.DrawLine(ToPoint(t.p), ToPoint(t.p + dirs[1]*4.0f), colorColl);
	renderer.DrawLine(ToPoint(t.p), ToPoint(t.p + dirs[2]*4.0f), colorColl);

	for(PxU32 j=0;j<numWheels;j++)
	{
//		getDebugRenderer()->addLine(v[j], v[j]+w[j]*l[j], colorColl);
		renderer.DrawLine(ToPoint(v[j]), ToPoint(v[j]+w[j]*l[j]), colorColl);

		//Draw all tire force app points.
		const PxVec3& appPoint = tireForceAppPoints[j];
//		getDebugRenderer()->addLine(appPoint - dirs[0], appPoint + dirs[0], colorCol2);
//		getDebugRenderer()->addLine(appPoint - dirs[2], appPoint + dirs[2], colorCol2);
		renderer.DrawLine(ToPoint(appPoint - dirs[0]), ToPoint(appPoint + dirs[0]), colorCol2);
		renderer.DrawLine(ToPoint(appPoint - dirs[2]), ToPoint(appPoint + dirs[2]), colorCol2);

		//Draw all susp force app points.
		const PxVec3& appPoint2 = suspForceAppPoints[j];
//		getDebugRenderer()->addLine(appPoint2 - dirs[0], appPoint2 + dirs[0], colorCol3);
//		getDebugRenderer()->addLine(appPoint2 - dirs[2], appPoint2 + dirs[2], colorCol3);
		renderer.DrawLine(ToPoint(appPoint2 - dirs[0]), ToPoint(appPoint2 + dirs[0]), colorCol3);
		renderer.DrawLine(ToPoint(appPoint2 - dirs[2]), ToPoint(appPoint2 + dirs[2]), colorCol3);
	}
}

/*static void drawBox2D(Renderer* renderer, PxF32 minX, PxF32 maxX, PxF32 minY, PxF32 maxY, const RendererColor& color, PxF32 alpha)
{
	ScreenQuad sq;
	sq.mX0				= minX;
	sq.mY0				= 1.0f - minY;
	sq.mX1				= maxX;
	sq.mY1				= 1.0f - maxY;
	sq.mLeftUpColor		= color;
	sq.mRightUpColor	= color;
	sq.mLeftDownColor	= color;
	sq.mRightDownColor	= color;
	sq.mAlpha			= alpha;

	renderer->drawScreenQuad(sq);
}

static void print(Renderer* renderer, PxF32 x, PxF32 y, PxF32 scale_, const char* text)
{
	PxU32 width, height;
	renderer->getWindowSize(width, height);

	y = 1.0f - y;

	const PxReal scale = scale_*20.0f;
	const PxReal shadowOffset = 6.0f;
	const RendererColor textColor(255, 255, 255, 255);

	renderer->print(PxU32(x*PxF32(width)), PxU32(y*PxF32(height)), text, scale, shadowOffset, textColor);
}*/

static void drawLine2D(PintRender& renderer, const PxReal* verts, const Point& color)
{
	renderer.DrawLine2D(verts[0], verts[2], 1.0f-verts[1], 1.0f-verts[3], color);

/*	Point* tmp = (Point*)StackAlloc(sizeof(Point)*nbVerts);
	for(udword i=0;i<nbVerts;i++)
	{
		tmp[i].x = *vertices++;
		tmp[i].y = *vertices++;
		tmp[i].z = 0.0f;
	}
	renderer.DrawLines2D(tmp, nbVerts, color);*/
}

void VehicleTest::drawHud(PintRender& renderer)
{
	const PxVehicleWheels& focusVehicle = *mVehicleManager.getVehicle(mPlayerVehicle);
	PxVehicleDriveDynData* driveDynData=NULL;
	PxVehicleDriveSimData* driveSimData=NULL;
	switch(focusVehicle.getVehicleType())
	{
	case PxVehicleTypes::eDRIVE4W:
		{
			PxVehicleDrive4W& vehDrive4W=(PxVehicleDrive4W&)focusVehicle;
			driveDynData=&vehDrive4W.mDriveDynData;
			driveSimData=&vehDrive4W.mDriveSimData;
		}
		break;
	default:
		PX_ASSERT(false);
		break;
	}

	const PxU32 currentGear=driveDynData->getCurrentGear();
	const PxF32 vz=mForwardSpeedHud*3.6f;
	const PxF32 revs=driveDynData->getEngineRotationSpeed();
	const PxF32 maxRevs=driveSimData->getEngineData().mMaxOmega*60*0.5f/PxPi;//Convert from radians per second to rpm
	const PxF32 invMaxRevs=driveSimData->getEngineData().getRecipMaxOmega();

	//Draw gears and speed.

	const PxF32 x=0.5f;
//	const PxF32 y=0.018f;
	const PxF32 y=0.8f;
	const PxF32 length=0.1f;
	const PxF32 textheight=0.02f;

	const Point TextColor(1.0f, 1.0f, 1.0f);

	//Renderer* renderer = getRenderer();
//	drawBox2D(renderer, x-length-textheight, x+length+textheight, y, y+length+textheight, RendererColor(255, 255, 255), 0.5f);
	renderer.DrawRectangle2D(x-length-textheight, x+length+textheight, y, y+length+textheight, Point(1.0f, 1.0f, 1.0f), 0.5f);

	//Gear
	char gear[PxVehicleGearsData::eGEARSRATIO_COUNT][64]=
	{
		"R","N","1","2","3","4","5"
	};
//	print(renderer, x-0.25f*textheight, y+0.02f, 0.02f, gear[currentGear]);
	renderer.Print(x-0.25f*textheight, y+0.02f, 0.02f, gear[currentGear], TextColor);

	//Speed
	char speed[64];
	sprintf(speed, "%1.0f %s",  PxAbs(vz), "kmph");
//	print(renderer, x-textheight, y+length-textheight, textheight, speed);
	renderer.Print(x-textheight, y+length-textheight, textheight, speed, TextColor);

	//Revs
	{
		const PxF32 xy[4]={x, 1.0f-y, x-length, 1.0f-(y+length)};
		drawLine2D(renderer, xy, Point(0, 0, 1));

		char buffer[64];
		sprintf(buffer, "%d \n", 0);
//		print(renderer, x-length, y+length, textheight, buffer);
		renderer.Print(x-length, y+length, textheight, buffer, TextColor);
	}
	{
		const PxF32 xy[4]={x, 1.0f-y, x+length, 1.0f-(y+length)};
		drawLine2D(renderer, xy, Point(0, 0, 1));

		char buffer[64];
		sprintf(buffer, "%1.0f \n", maxRevs);
//		print(renderer, x+length-2*textheight, y+length, textheight, buffer);
		renderer.Print(x+length-2*textheight, y+length, textheight, buffer, TextColor);
	}
	{
		const PxF32 alpha=revs*invMaxRevs;
		const PxF32 dx=-(1.0f-alpha)*length + alpha*length;
		const PxF32 xy[4]={x, 1.0f-y, x+dx, 1.0f-(y+length)};
		drawLine2D(renderer, xy, Point(1, 0, 0));
	}
}

/*static PX_FORCE_INLINE RendererColor getColor(const PxVec3& c)
{
	return RendererColor(PxU8(c.x), PxU8(c.y), PxU8(c.z));
}*/

//static PX_FORCE_INLINE void convertColors(const PxVec3* src, RendererColor* dst)
static PX_FORCE_INLINE void convertColors(const PxVec3* src, Point* dst)
{
	for(PxU32 i=0;i<PxVehicleGraph::eMAX_NB_SAMPLES;i++)
//		*dst++ = getColor(src[i]);
		*dst++ = ToPoint(src[i]);
}

static void convertY(PxF32* xy)
{
	for(PxU32 i=0;i<PxVehicleGraph::eMAX_NB_SAMPLES;i++)
		xy[2*i+1]=1.0f-xy[2*i+1];
}

void drawGraphsAndPrintTireSurfaceTypesN
(const PxVehicleTelemetryData& telemetryData, const PxU32* tireTypes, const PxU32* surfaceTypes,
 const PxU32 activeEngineGraphChannel, const PxU32 activeWheelGraphChannel, PintRender& renderer)
{

	PxF32 xy[2*PxVehicleGraph::eMAX_NB_SAMPLES];
	PxVec3 color[PxVehicleGraph::eMAX_NB_SAMPLES];
	//RendererColor rendererColor[PxVehicleGraph::eMAX_NB_SAMPLES];
	Point rendererColor[PxVehicleGraph::eMAX_NB_SAMPLES];
	char title[PxVehicleGraph::eMAX_NB_TITLE_CHARS];

	const PxU32 numWheelGraphs=telemetryData.getNbWheelGraphs();

	for(PxU32 i=0;i<numWheelGraphs;i++)
	{
		PxF32 xMin,xMax,yMin,yMax;
		telemetryData.getWheelGraph(i).getBackgroundCoords(xMin,yMin,xMax,yMax);
		const PxVec3& backgroundColor=telemetryData.getWheelGraph(i).getBackgroundColor();
		const PxF32 alpha=telemetryData.getWheelGraph(i).getBackgroundAlpha();
		//drawBox2D(renderer, xMin,xMax,yMin,yMax, getColor(backgroundColor),alpha);
		renderer.DrawRectangle2D(xMin,xMax,yMin,yMax, ToPoint(backgroundColor),alpha);

		telemetryData.getWheelGraph(i).computeGraphChannel(activeWheelGraphChannel,xy,color,title);
		convertY(xy);
		convertColors(color, rendererColor);
		//renderer->drawLines2D(PxVehicleGraph::eMAX_NB_SAMPLES, xy, rendererColor);
		//renderer.DrawLines2D(PxVehicleGraph::eMAX_NB_SAMPLES, xy, ToPoint(rendererColor));
		{
			const float* vertices = xy;
			const Point* tmp = rendererColor;
			udword NbToGo = PxVehicleGraph::eMAX_NB_SAMPLES-1;
			while(NbToGo--)
			{
				float x0 = vertices[0];
				float y0 = 1.0f-vertices[1];
				float x1 = vertices[2];
				float y1 = 1.0f-vertices[3];
				renderer.DrawLine2D(x0, x1, y0, y1, *tmp++);
				vertices += 2;
			}
		}

		//print(renderer, xMin,yMax-0.02f, 0.02f, title);
		renderer.Print(xMin,yMax-0.02f, 0.02f, title, Point(1.0f, 1.0f, 1.0f));

		const PxU32 tireType=tireTypes[i];
		const PxU32 tireSurfaceType=surfaceTypes[i];

/*		if (PxVehicleDrivableSurfaceType::eSURFACE_TYPE_UNKNOWN!=tireSurfaceType)
		{
			const char* surfaceType = SurfaceTypeNames::getName(tireSurfaceType);
			const PxF32 friction=TireFrictionMultipliers::getValue(tireSurfaceType, tireType);
			char surfaceDetails[64];
			sprintf(surfaceDetails, "%s %1.2f \n", surfaceType, friction);
			//print(renderer, xMin+0.1f, yMax-0.12f, 0.02f, surfaceDetails);
			renderer.Print(xMin+0.1f, yMax-0.12f, 0.02f, surfaceDetails, Point(1.0f, 1.0f, 1.0f));
		}*/
	}

	PxF32 xMin,xMax,yMin,yMax;
	telemetryData.getEngineGraph().getBackgroundCoords(xMin,yMin,xMax,yMax);
	const PxVec3& backgroundColor=telemetryData.getEngineGraph().getBackgroundColor();
	const PxF32 alpha=telemetryData.getEngineGraph().getBackgroundAlpha();
//	drawBox2D(renderer, xMin,xMax,yMin,yMax, getColor(backgroundColor),alpha);
	renderer.DrawRectangle2D(xMin,xMax,yMin,yMax, ToPoint(backgroundColor),alpha);

	telemetryData.getEngineGraph().computeGraphChannel(activeEngineGraphChannel,xy,color,title);
	convertY(xy);
	convertColors(color, rendererColor);
	//renderer->drawLines2D(PxVehicleGraph::eMAX_NB_SAMPLES, xy, rendererColor);
	{
		const float* vertices = xy;
		const Point* tmp = rendererColor;
		udword NbToGo = PxVehicleGraph::eMAX_NB_SAMPLES-1;
		while(NbToGo--)
		{
			float x0 = vertices[0];
			float y0 = 1.0f-vertices[1];
			float x1 = vertices[2];
			float y1 = 1.0f-vertices[3];
			renderer.DrawLine2D(x0, x1, y0, y1, *tmp++);
			vertices += 2;
		}
	}

//	print(renderer, xMin,yMax-0.02f,0.02f,title);
	renderer.Print(xMin,yMax-0.02f,0.02f,title, Point(1.0f, 1.0f, 1.0f));
}

void VehicleTest::drawGraphsAndPrintTireSurfaceTypes(PintRender& renderer, const PxVehicleWheels& focusVehicle, const PxVehicleWheelQueryResult& focusVehicleWheelQueryResults)
{
	PxU32 tireTypes[8];
	PxU32 surfaceTypes[8];
	const PxU32 numWheels=focusVehicle.mWheelsSimData.getNbWheels();
	PX_ASSERT(numWheels<=8);
	for(PxU32 i=0;i<numWheels;i++)
	{
		tireTypes[i]=focusVehicle.mWheelsSimData.getTireData(i).mType;
		surfaceTypes[i]=focusVehicleWheelQueryResults.wheelQueryResults[i].tireSurfaceType;
	}

	PxVehicleTelemetryData* vehTelData = mTelemetryData4W;

	drawGraphsAndPrintTireSurfaceTypesN(*vehTelData, tireTypes, surfaceTypes, mDebugRenderActiveGraphChannelEngine, mDebugRenderActiveGraphChannelWheel, renderer);
}

#endif
