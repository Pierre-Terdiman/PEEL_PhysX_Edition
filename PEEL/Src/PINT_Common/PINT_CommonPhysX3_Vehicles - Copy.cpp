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

#define DISABLE_VEHICLE_SQ_VS_DYNAMICS

#ifdef PHYSX_DEPRECATED_CREATE_SHAPE
	#include "PxRigidActorExt.h"
#endif

///////////////////////////////////

using namespace PhysX3;
using namespace physx;

static const char* gChassisDebugName = "Chassis";
static const char* gWheelDebugName = "Wheel";

extern PEEL_PhysX3_AllocatorCallback* gDefaultAllocator;
extern PxErrorCallback* gDefaultErrorCallback;

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

//#define CHECK_MSG(exp, msg) (!!(exp) || (physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, msg), 0) )
#define CHECK_MSG(exp, msg) (!!(exp) || (gDefaultErrorCallback->reportError(physx::PxErrorCode::eINVALID_PARAMETER, msg, __FILE__, __LINE__), 0) )
#define SIZEALIGN16(size) (((unsigned)(size)+15)&((unsigned)(~15)));

SampleVehicleSceneQueryData* SampleVehicleSceneQueryData::allocate(const PxU32 maxNumWheels)
{
	const PxU32 size0 = SIZEALIGN16(sizeof(SampleVehicleSceneQueryData));
#ifdef USE_SWEEPS
	//#### isn't this wrong? NB_SWEEP_HITS_PER_WHEEL should be on size2?
	const PxU32 size1 = SIZEALIGN16(sizeof(PxSweepQueryResult)*maxNumWheels*NB_SWEEP_HITS_PER_WHEEL);
	const PxU32 size2 = SIZEALIGN16(sizeof(PxSweepHit)*maxNumWheels);
#else
	const PxU32 size1 = SIZEALIGN16(sizeof(PxRaycastQueryResult)*maxNumWheels);
	const PxU32 size2 = SIZEALIGN16(sizeof(PxRaycastHit)*maxNumWheels);
#endif
	const PxU32 size = size0 + size1 + size2;
//	SampleVehicleSceneQueryData* sqData = (SampleVehicleSceneQueryData*)PX_ALLOC(size, "PxVehicleNWSceneQueryData");
	SampleVehicleSceneQueryData* sqData = (SampleVehicleSceneQueryData*)gDefaultAllocator->allocate(size, "PxVehicleNWSceneQueryData", __FILE__, __LINE__);
	sqData->init();
	PxU8* ptr = (PxU8*) sqData;
	ptr += size0;
#ifdef USE_SWEEPS
	sqData->mSqResults = (PxSweepQueryResult*)ptr;
	for(PxU32 i=0;i<maxNumWheels;i++)
	{
		sqData->mSqResults[i].nbTouches = 0;
		sqData->mSqResults[i].touches = null;
	}
#else
	sqData->mSqResults = (PxRaycastQueryResult*)ptr;
	for(PxU32 i=0;i<maxNumWheels;i++)
	{
		sqData->mSqResults[i].nbTouches = 0;
		sqData->mSqResults[i].touches = null;
	}
#endif
	sqData->mNbSqResults = maxNumWheels*NB_SWEEP_HITS_PER_WHEEL;
	ptr += size1;
#ifdef USE_SWEEPS
	sqData->mSqHitBuffer = (PxSweepHit*)ptr;
#else
	sqData->mSqHitBuffer = (PxRaycastHit*)ptr;
#endif
	ptr += size2;
	sqData->mNumQueries = maxNumWheels;
	return sqData;
}

void SampleVehicleSceneQueryData::free()
{
//	PX_FREE(this);
	gDefaultAllocator->deallocate(this);
}

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

class MyBatchQuery : public PxBatchQuery, public PxQueryFilterCallback
{
	public:

	virtual void	setRunOnSpu(bool runOnSpu)	{}
	virtual bool	getRunOnSpu()				{ return false;}

	virtual PxQueryHitType::Enum preFilter(
		const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
	{
		if(shape->getName()==gChassisDebugName || shape->getName()==gWheelDebugName)
			return PxQueryHitType::eNONE;

		return PxQueryHitType::eBLOCK;
	}

	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit)
	{
		return PxQueryHitType::eBLOCK;
	}

	MyBatchQuery() : mNb(0), mScene(null), mDesc(0, 0, 0)
	{
	}

	void init(PxScene* scene, const PxBatchQueryDesc& desc)
	{
		mScene = scene;
		mDesc = desc;
	}

	virtual	void							execute()
	{
		mNb = 0;
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

	virtual	const void*						getFilterShaderData() const
	{
		ASSERT(0);
		return null;
	}

	virtual	PxU32							getFilterShaderDataSize() const
	{
		ASSERT(0);
		return 0;
	}

	virtual PxClientID						getOwnerClient() const
	{
		ASSERT(0);
		return 0;
	}

 	virtual	void							setUserMemory(const PxBatchQueryMemory&)
	{
		ASSERT(0);
	}

 	virtual	const PxBatchQueryMemory&		getUserMemory()
	{
		return mDesc.queryMemory;
	}

	virtual	void							release()
	{
		ASSERT(0);
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

/*		PX_INLINE bool raycastSingle(PxScene* scene,
		const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
		PxSceneQueryFlags outputFlags, PxRaycastHit& hit,
		const PxSceneQueryFilterData& filterData = PxSceneQueryFilterData(),
		PxSceneQueryFilterCallback* filterCall = NULL, const PxSceneQueryCache* cache = NULL,
		PxClientID queryClient = PX_DEFAULT_CLIENT)*/

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

		mNb++;
	}

	// For 3.3.0
	virtual void raycast(
		const PxVec3& origin, const PxVec3& unitDir, PxReal distance = PX_MAX_F32, PxU16 maxTouchHits = 0,
#ifdef PHYSX_DEPRECATED_DISTANCE
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
		PxQueryFilterData filterDataCopy = filterData;
#ifdef DISABLE_VEHICLE_SQ_VS_DYNAMICS
		filterDataCopy.flags &= ~PxQueryFlag::eDYNAMIC;
#endif
		// "sweep single"
		PxSweepBuffer buf;
//		PxQueryFilterData fd1 = filterData;
//		fd1.clientId = queryClient;
		mScene->sweep(geometry, pose, unitDir, distance, buf, hitFlags, filterDataCopy, this, cache, inflation);
//		hit = buf.block;
//		return buf.hasBlock;

		if(buf.hasBlock)
		{
			mDesc.queryMemory.userSweepResultBuffer[mNb].block = buf.block;
			mDesc.queryMemory.userSweepResultBuffer[mNb].hasBlock = true;
		}
		else
			mDesc.queryMemory.userSweepResultBuffer[mNb].hasBlock = false;

		mNb++;
	}

	// For 3.3.0
	virtual void sweep(
		const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
		PxU16 maxTouchHits = 0,
#ifdef PHYSX_DEPRECATED_DISTANCE
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
	PxScene*			mScene;
	PxBatchQueryDesc	mDesc;

}gMyBatchQuery;



PxBatchQuery* SampleVehicleSceneQueryData::setUpBatchedSceneQuery(PxScene* scene)
{
#ifdef USE_SWEEPS
	PxBatchQueryDesc sqDesc(0, mNbSqResults, 0);
	sqDesc.queryMemory.userSweepResultBuffer = mSqResults;
	sqDesc.queryMemory.userSweepTouchBuffer = mSqHitBuffer;
	sqDesc.queryMemory.sweepTouchBufferSize = mNumQueries*NB_SWEEP_HITS_PER_WHEEL;
#else
	PxBatchQueryDesc sqDesc(mNbSqResults, 0, 0);
	sqDesc.queryMemory.userRaycastResultBuffer = mSqResults;
	sqDesc.queryMemory.userRaycastTouchBuffer = mSqHitBuffer;
	sqDesc.queryMemory.raycastTouchBufferSize = mNumQueries;
#endif
	sqDesc.preFilterShader = MySampleVehicleWheelRaycastPreFilter;

	gMyBatchQuery.init(scene, sqDesc);
	return &gMyBatchQuery;
//	return scene->createBatchQuery(sqDesc);
}

///////////////////////////////////////////////////////////////////////////////

static const PxVehicleKeySmoothingData gKeySmoothingData=
{
	{
		3.0f,	//rise rate eANALOG_INPUT_ACCEL
		3.0f,	//rise rate eANALOG_INPUT_BRAKE
		10.0f,	//rise rate eANALOG_INPUT_HANDBRAKE
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
	},
	{
		5.0f,	//fall rate eANALOG_INPUT__ACCEL
		5.0f,	//fall rate eANALOG_INPUT__BRAKE
		10.0f,	//fall rate eANALOG_INPUT__HANDBRAKE
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
	}
};

/*static const PxVehicleKeySmoothingData gKeySmoothingData=
{
	{
		3.0f,	//rise rate eANALOG_INPUT_ACCEL
		3.0f,	//rise rate eANALOG_INPUT_BRAKE
		10.0f,	//rise rate eANALOG_INPUT_HANDBRAKE
//		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
//		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
		0.05f,	//rise rate eANALOG_INPUT_STEER_LEFT
		0.05f,	//rise rate eANALOG_INPUT_STEER_RIGHT
	},
	{
		5.0f,	//fall rate eANALOG_INPUT__ACCEL
		5.0f,	//fall rate eANALOG_INPUT__BRAKE
		10.0f,	//fall rate eANALOG_INPUT__HANDBRAKE
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
	}
};*/

static const PxVehiclePadSmoothingData gCarPadSmoothingData=
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

static PxF32 gSteerVsForwardSpeedData[2*8]=
{
/*	0.0f,		0.5f,
	5.0f,		0.4f,
	30.0f,		0.05f,
	120.0f,		0.02f,
*/
/*
	0.0f,		1.0f,
	5.0f,		1.0f,
	30.0f,		1.0f,
	120.0f,		1.0f,
*/

	// Good one
/*	0.0f,		0.5f,
	5.0f,		0.2f,
	30.0f,		0.08f,
	120.0f,		0.075f,*/

	// Default


	0.0f,		1.0f,
//	0.0f,		0.75f,
	5.0f,		0.75f,
	30.0f,		0.125f,
	120.0f,		0.1f,
/*
	// Not bad
	0.0f,		1.0f,
	5.0f,		0.75f,
	30.0f,		0.02f,
	120.0f,		0.01f,
*/

/*
	0.0f,		1.0f,
	5.0f,		0.75f,
	30.0f,		0.35f,
	120.0f,		0.2f,
*/


	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32
};
//static PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData,4);

///////////////////////////////////////////////////////////////////////////////

SampleVehicle_VehicleController::SampleVehicle_VehicleController()
{
	clear();
}

SampleVehicle_VehicleController::~SampleVehicle_VehicleController()
{
}

void SampleVehicle_VehicleController::clear()
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

	mUseKeyInputs			= true;
	mToggleAutoGears		= false;
	mIsMovingForwardSlowly	= true;
	mInReverseMode			= false;
}

void SampleVehicle_VehicleController::processRawInputs(const PxF32 dtime, const bool useAutoGears, PxVehicleDrive4WRawInputData& rawInputData)
{
	// Keyboard
	{
		rawInputData.setDigitalAccel(mKeyPressedAccel);
		rawInputData.setDigitalBrake(mKeyPressedBrake);
		rawInputData.setDigitalHandbrake(mKeyPressedHandbrake);
		rawInputData.setDigitalSteerLeft(mKeyPressedSteerLeft);
		rawInputData.setDigitalSteerRight(mKeyPressedSteerRight);
		rawInputData.setGearUp(mKeyPressedGearUp);
		rawInputData.setGearDown(mKeyPressedGearDown);

/*		mUseKeyInputs=
			(mKeyPressedAccel || mKeyPressedBrake  || mKeyPressedHandbrake || 
			 mKeyPressedSteerLeft || mKeyPressedSteerRight || 
			 mKeyPressedGearUp || mKeyPressedGearDown);*/
	}

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

/*		if(mUseKeyInputs && ((mGamepadAccel+mGamepadCarBrake+mGamepadCarSteer)!=0.0f ||  mGamepadGearup || mGamepadGeardown || mGamepadCarHandbrake))
		{
			mUseKeyInputs=false;
		}*/

		if(!mUseKeyInputs)
		{
			rawInputData.setAnalogAccel(mGamepadAccel);
			rawInputData.setAnalogBrake(mGamepadCarBrake);
			rawInputData.setAnalogHandbrake(mGamepadCarHandbrake ? 1.0f : 0.0f);
			rawInputData.setAnalogSteer(mGamepadCarSteer);
			rawInputData.setGearUp(mGamepadGearup);
			rawInputData.setGearDown(mGamepadGeardown);
		}
	}

	if(useAutoGears && (rawInputData.getGearDown() || rawInputData.getGearUp()))
	{
		rawInputData.setGearDown(false);
		rawInputData.setGearUp(false);
	}

//	mNumSamples++;
}

#define THRESHOLD_FORWARD_SPEED (0.1f) 
#define THRESHOLD_SIDEWAYS_SPEED (0.2f)
#define THRESHOLD_ROLLING_BACKWARDS_SPEED (0.1f)

void SampleVehicle_VehicleController::processAutoReverse(const PxVehicleWheels& focusVehicle, const PxVehicleDriveDynData& driveDynData, const PxVehicleWheelQueryResult& vehicleWheelQueryResults,
 const PxVehicleDrive4WRawInputData& carRawInputs, bool& toggleAutoReverse, bool& newIsMovingForwardSlowly) const
{
	newIsMovingForwardSlowly = false;
	toggleAutoReverse = false;

	if(driveDynData.getUseAutoGears())
	{
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
			if(mUseKeyInputs)
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
			if(PxVehicleGearsData::eFIRST == currentGear  && forwardSpeed < -THRESHOLD_ROLLING_BACKWARDS_SPEED)
			{
				isMovingBackwards = true;
			}
			else if(PxVehicleGearsData::eREVERSE == currentGear && forwardSpeed > THRESHOLD_ROLLING_BACKWARDS_SPEED)
			{
				isMovingBackwards = true;
			}

			//Check if the car is moving slowly.
			if(forwardSpeedAbs < THRESHOLD_FORWARD_SPEED && sidewaysSpeedAbs < THRESHOLD_SIDEWAYS_SPEED)
			{
				isMovingForwardSlowly=true;
			}

			//Now work if we need to toggle from forwards gear to reverse gear or vice versa.
			if(isMovingBackwards)
			{
				if(!accelRaw && !brakeRaw && !handbrakeRaw && (currentGear == targetGear))			
				{
					//The car is rolling against the gear and the player is doing nothing to stop this.
					toggleAutoReverse = true;
				}
			}
			else if(prevIsMovingForwardSlowly && isMovingForwardSlowly)
			{
				if((currentGear > PxVehicleGearsData::eNEUTRAL) && brakeRaw && !accelRaw && (currentGear == targetGear))
				{
					//The car was moving slowly in forward gear without player input and is now moving slowly with player input that indicates the 
					//player wants to switch to reverse gear.
					toggleAutoReverse = true;
				}
				else if(currentGear == PxVehicleGearsData::eREVERSE && accelRaw && !brakeRaw && (currentGear == targetGear))
				{
					//The car was moving slowly in reverse gear without player input and is now moving slowly with player input that indicates the 
					//player wants to switch to forward gear.
					toggleAutoReverse = true;
				}
			}

			//If the car was brought to rest through braking then the player needs to release the brake then reapply
			//to indicate that the gears should toggle between reverse and forward.
//			if(isMovingForwardSlowly && !brakeRaw && !accelRaw && !handbrakeRaw)
			if(isMovingForwardSlowly)
			{
				newIsMovingForwardSlowly = true;
			}
		}
	}
}

void SampleVehicle_VehicleController::update(const PxF32 timestep, const PxVehicleWheelQueryResult& vehicleWheelQueryResults, PxVehicleWheels& focusVehicle)
{
	PxVehicleDriveDynData* driveDynData=NULL;
	switch(focusVehicle.getVehicleType())
	{
	case PxVehicleTypes::eDRIVE4W:
		{
			PxVehicleDrive4W& vehDrive4W=(PxVehicleDrive4W&)focusVehicle;
			driveDynData=&vehDrive4W.mDriveDynData;
		}
		break;
/*	case PxVehicleTypes::eDRIVENW:
		{
			PxVehicleDriveNW& vehDriveNW=(PxVehicleDriveNW&)focusVehicle;
			driveDynData=&vehDriveNW.mDriveDynData;
		}
		break;*/
	default:
		PX_ASSERT(false);
		break;
	}

	//Toggle autogear flag
	if(mToggleAutoGears)
	{
		driveDynData->toggleAutoGears();
		mToggleAutoGears = false;
	}

	//Store raw inputs in replay stream if in recording mode.
	//Set raw inputs from replay stream if in replay mode.
	//Store raw inputs from active stream in handy arrays so we don't need to worry
	//about which stream (live input or replay) is active.
	//Work out if we are using keys or gamepad controls depending on which is being used
	//(gamepad selected if both are being used).
	PxVehicleDrive4WRawInputData carRawInputs;
	{
		processRawInputs(timestep,driveDynData->getUseAutoGears(),carRawInputs);
	}

	//Work out if the car is to flip from reverse to forward gear or from forward gear to reverse.
	bool toggleAutoReverse = false;
	bool newIsMovingForwardSlowly = false;
	{
		processAutoReverse(focusVehicle, *driveDynData, vehicleWheelQueryResults, carRawInputs, toggleAutoReverse, newIsMovingForwardSlowly);
	}
	mIsMovingForwardSlowly = newIsMovingForwardSlowly;


	//If the car is to flip gear direction then switch gear as appropriate.
	if(toggleAutoReverse)
	{
		mInReverseMode = !mInReverseMode;
		
		if(mInReverseMode)
		{
			driveDynData->forceGearChange(PxVehicleGearsData::eREVERSE);
		}
		else
		{
			driveDynData->forceGearChange(PxVehicleGearsData::eFIRST);
		}
	}

//	printf("mUseKeyInputs: %d\n", mUseKeyInputs);

	//If in reverse mode then swap the accel and brake.
	if(mInReverseMode)
	{
		if(mUseKeyInputs)
		{
			{
				const bool accel=carRawInputs.getDigitalAccel();
				const bool brake=carRawInputs.getDigitalBrake();
				carRawInputs.setDigitalAccel(brake);
				carRawInputs.setDigitalBrake(accel);
			}
		}
		else
		{
			{
				const PxF32 accel=carRawInputs.getAnalogAccel();
				const PxF32 brake=carRawInputs.getAnalogBrake();
				carRawInputs.setAnalogAccel(brake);
				carRawInputs.setAnalogBrake(accel);
			}
		}
	}

	{
		const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);

		PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData, 4);

		// Now filter the raw input values and apply them to focus vehicle
		// as floats for brake,accel,handbrake,steer and bools for gearup,geardown.
		if(mUseKeyInputs)
		{
			PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs
				(gKeySmoothingData,gSteerVsForwardSpeedTable,carRawInputs,timestep,isInAir,(PxVehicleDrive4W&)focusVehicle);
		}
		else
		{
			PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs
				(gCarPadSmoothingData,gSteerVsForwardSpeedTable,carRawInputs,timestep,isInAir,(PxVehicleDrive4W&)focusVehicle);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

SampleVehicle_VehicleManager::SampleVehicle_VehicleManager() 
:	mNumVehicles(0),
	mSqWheelRaycastBatchQuery(NULL)
{
}

SampleVehicle_VehicleManager::~SampleVehicle_VehicleManager()
{
}

void SampleVehicle_VehicleManager::init(PxPhysics& physics, const PxMaterial** drivableSurfaceMaterials, const PxVehicleDrivableSurfaceType* drivableSurfaceTypes)
{
	//Initialise the sdk.
	PxInitVehicleSDK(physics);

	//Set the basis vectors.
	const PxVec3 up(0.0f, 1.0f, 0.0f);
	const PxVec3 forward(0.0f, 0.0f, 1.0f);
	PxVehicleSetBasisVectors(up, forward);

	//Set the vehicle update mode to be immediate velocity changes.
	PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);
	
	//Initialise all vehicle ptrs to null.
	for(PxU32 i=0;i<MAX_NUM_4W_VEHICLES;i++)
	{
		mVehicles[i]=NULL;
	}

	//Scene query data for to allow raycasts for all suspensions of all vehicles.
	mSqData = SampleVehicleSceneQueryData::allocate(MAX_NUM_4W_VEHICLES*4);

	//Data to store reports for each wheel.
	mWheelQueryResults = SampleVehicleWheelQueryResults::allocate(MAX_NUM_4W_VEHICLES*4);

	//Set up the friction values arising from combinations of tire type and surface type.
//	mSurfaceTirePairs = PxVehicleDrivableSurfaceToTireFrictionPairs::allocate(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES);
//	mSurfaceTirePairs->setup(MAX_NUM_TIRE_TYPES, MAX_NUM_SURFACE_TYPES, drivableSurfaceMaterials, drivableSurfaceTypes);
	// One tire type & one surface type for now
	const PxU32 nbTireTypes = 2;	// Front & rear
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
}

void SampleVehicle_VehicleManager::setTireFrictionMultiplier(float frontTireFrictionMultiplier, float rearTireFrictionMultiplier)
{
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
	mSurfaceTirePairs->setTypePairFriction(0, 0, frontTireFrictionMultiplier);
	mSurfaceTirePairs->setTypePairFriction(0, 1, rearTireFrictionMultiplier);
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
/*		case PxVehicleTypes::eDRIVENW:
			{
				PxVehicleDriveNW* veh=(PxVehicleDriveNW*)mVehicles[i];
				veh->free();
			}
			break;*/
		default:
			PX_ASSERT(false);
			break;
		}
	}

	//Deallocate scene query data that was used for suspension raycasts.
	mSqData->free();

	//Deallocate buffers that store wheel reports.
	mWheelQueryResults->free();

	//Release the  friction values used for combinations of tire type and surface type.
	mSurfaceTirePairs->release();

	//Scene query.
	if(mSqWheelRaycastBatchQuery)
	{
		mSqWheelRaycastBatchQuery=NULL;
	}

	PxCloseVehicleSDK();
}

/*void SampleVehicle_VehicleManager::addVehicle(const PxU32 i, PxVehicleWheels* vehicle)		
{ 
	mVehicles[i] = vehicle; 

	const PxU32 numWheels = vehicle->mWheelsSimData.getNbWheels();
	mVehicleWheelQueryResults[i].nbWheelQueryResults = numWheels;
	mVehicleWheelQueryResults[i].wheelQueryResults = mWheelQueryResults->addVehicle(numWheels);

	mNumVehicles++;
}*/


/*void SampleVehicle_VehicleManager::resetNWCar(const PxTransform& transform, const PxU32 vehicleId)
{
	PX_ASSERT(vehicleId<mNumVehicles);
	resetNWCar(transform,mVehicles[vehicleId]);
}*/

void SampleVehicle_VehicleManager::suspensionRaycasts(PxScene* scene)
{
	//Create a scene query if we haven't already done so.
	if(NULL==mSqWheelRaycastBatchQuery)
	{
		mSqWheelRaycastBatchQuery=mSqData->setUpBatchedSceneQuery(scene);
	}

#ifdef USE_SWEEPS
	{
		PxSceneReadLock scopedLock(*scene);

		PxVehicleSuspensionSweeps(mSqWheelRaycastBatchQuery,
			mNumVehicles, mVehicles,
			mSqData->getSweepQueryResultBufferSize(), mSqData->getSweepQueryResultBuffer(), NB_SWEEP_HITS_PER_WHEEL
//			,null, 0.5f, 0.5f
			);
	}
#else
	{
		//Raycasts.
		PxSceneReadLock scopedLock(*scene);
		PxVehicleSuspensionRaycasts(mSqWheelRaycastBatchQuery,
			mNumVehicles, mVehicles,
			mSqData->getRaycastQueryResultBufferSize(), mSqData->getRaycastQueryResultBuffer());
	}
#endif
/*
	void PxVehicleSuspensionRaycasts
		(PxBatchQuery* batchQuery, 
		 const PxU32 nbVehicles, PxVehicleWheels** vehicles,
		 const PxU32 nbSceneQueryResults, PxRaycastQueryResult* sceneQueryResults, 
		 const bool* vehiclesToRaycast = NULL);

	void PxVehicleSuspensionSweeps
		(PxBatchQuery* batchQuery, 
		 const PxU32 nbVehicles, PxVehicleWheels** vehicles, 
		 const PxU32 nbSceneQueryResults, PxSweepQueryResult* sceneQueryResults,  const PxU16 nbHitsPerQuery, 
		 const bool* vehiclesToSweep = NULL,
		 const PxF32 sweepWidthScale = 1.0f, const PxF32 sweepRadiusScale = 1.0f);
*/


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
}


#if PX_DEBUG_VEHICLE_ON2

void SampleVehicle_VehicleManager::updateAndRecordTelemetryData
(const PxF32 timestep, const PxVec3& gravity, PxVehicleWheels* focusVehicle, PxVehicleTelemetryData* telemetryData)
{
	PX_ASSERT(focusVehicle && telemetryData);

	//Update all vehicles except for focusVehicle.
	PxVehicleWheels* vehicles[MAX_NUM_4W_VEHICLES];
	PxVehicleWheelQueryResult vehicleWheelQueryResults[MAX_NUM_4W_VEHICLES];
	PxVehicleWheelQueryResult focusVehicleWheelQueryResults[1];
	PxU32 numVehicles=0;
	for(PxU32 i=0;i<mNumVehicles;i++)
	{
		if(focusVehicle!=mVehicles[i])
		{
			vehicles[numVehicles]=mVehicles[i];
			vehicleWheelQueryResults[numVehicles]=mVehicleWheelQueryResults[i];
			numVehicles++;
		}
		else
		{
			focusVehicleWheelQueryResults[0]=mVehicleWheelQueryResults[i];
		}
	}
	PxVehicleUpdates(timestep,gravity,*mSurfaceTirePairs,numVehicles,vehicles,vehicleWheelQueryResults);


	//Update the vehicle for which we want to record debug data.
	PxVehicleUpdateSingleVehicleAndStoreTelemetryData(timestep,gravity,*mSurfaceTirePairs,focusVehicle,focusVehicleWheelQueryResults,*telemetryData);
}

#else

void SampleVehicle_VehicleManager::update(const PxF32 timestep, const PxVec3& gravity)
{
	PxVehicleUpdates(timestep, gravity, *mSurfaceTirePairs, mNumVehicles, mVehicles, mVehicleWheelQueryResults);
}

#endif //PX_DEBUG_VEHICLE_ON

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
/*	case PxVehicleTypes::eDRIVENW:
		{
			PxVehicleDriveNW* vehDriveNW=(PxVehicleDriveNW*)vehWheels;
			//Set the car back to its rest state.
			vehDriveNW->setToRestState();
			//Set the car to first gear.
			vehDriveNW->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		}
		break;*/
	default:
		PX_ASSERT(false);
		break;
	}

	//Set the car's transform to be the start transform.
	PxRigidDynamic* actor=vehWheels->getRigidDynamicActor();
	PxSceneWriteLock scopedLock(*actor->getScene());
	actor->setGlobalPose(startTransform);
}

static void SetupVehicleChassisShape(PxShape* shape)
{
	shape->setName(gChassisDebugName);

/*
//	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, gSQFlag);
	shape->setFlag(PxShapeFlag::eVISUALIZATION, gDebugVizParams[0]);
//	shape->setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, gUseCCD);
	shape->setContactOffset(gContactOffset);
	shape->setRestOffset(gRestOffset);

	// Setup query filter data so that we can filter out all shapes - debug purpose
	if(gSQFlag)
		shape->setQueryFilterData(PxFilterData(1, 0, 0, 0));

//	if(create->mRenderer)
//		shape->userData = create->mRenderer;
*/
	shape->setQueryFilterData(PxFilterData(1, 0, 0, 0));
	PhysX3::SetGroup(*shape, 0);
//	shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
}

static void SetupVehicleWheelShape(PxShape* shape)
{
	shape->setName(gWheelDebugName);
/*
//	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, gSQFlag);
	shape->setFlag(PxShapeFlag::eVISUALIZATION, gDebugVizParams[0]);
//	shape->setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, gUseCCD);
	shape->setContactOffset(gContactOffset);
	shape->setRestOffset(gRestOffset);

	// Setup query filter data so that we can filter out all shapes - debug purpose
	if(gSQFlag)
		shape->setQueryFilterData(PxFilterData(1, 0, 0, 0));

//	if(create->mRenderer)
//		shape->userData = create->mRenderer;
*/
//	PhysX3::SetGroup(*shape, 0);
	PxSetGroupCollisionFlag(0, 31, false);
	PhysX3::SetGroup(*shape, 31);
//	shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
	shape->setQueryFilterData(PxFilterData(1, 0, 0, 0));
}

static void setupActor
(PxRigidDynamic* vehActor, 
 const PxGeometry** wheelGeometries, const PxTransform* wheelLocalPoses, const PxU32 numWheelGeometries, const PxMaterial* wheelMaterial,
 const PxGeometry** chassisGeometries, const PxTransform* chassisLocalPoses, const PxU32 numChassisGeometries, const PxMaterial* chassisMaterial,
 const PxVehicleChassisData& chassisData,
 PxPhysics* physics)
{
	//Add all the wheel shapes to the actor.
	for(PxU32 i=0;i<numWheelGeometries;i++)
	{
#ifdef PHYSX_DEPRECATED_CREATE_SHAPE
		PxShape* wheelShape = PxRigidActorExt::createExclusiveShape(*vehActor, *wheelGeometries[i], *wheelMaterial);
#else
		PxShape* wheelShape = vehActor->createShape(*wheelGeometries[i], *wheelMaterial);
#endif
		SetupVehicleWheelShape(wheelShape);
		wheelShape->setLocalPose(wheelLocalPoses[i]);
//		printf("wheelLocalPoses[i]: %f %f %f\n", wheelLocalPoses[i].p.x, wheelLocalPoses[i].p.y, wheelLocalPoses[i].p.z);
	}

	//Add the chassis shapes to the actor.
	for(PxU32 i=0;i<numChassisGeometries;i++)
	{
#ifdef PHYSX_DEPRECATED_CREATE_SHAPE
		PxShape* chassisShape = PxRigidActorExt::createExclusiveShape(*vehActor, *chassisGeometries[i], *chassisMaterial);
#else
		PxShape* chassisShape = vehActor->createShape(*chassisGeometries[i], *chassisMaterial);
#endif
		SetupVehicleChassisShape(chassisShape);
		chassisShape->setLocalPose(chassisLocalPoses[i]);
	}

	vehActor->setMass(chassisData.mMass);
	vehActor->setMassSpaceInertiaTensor(chassisData.mMOI);
	vehActor->setCMassLocalPose(PxTransform(chassisData.mCMOffset,PxQuat(PxIdentity)));
//	vehActor->setCMassLocalPose(PxTransform(-chassisData.mCMOffset,PxQuat(PxIdentity)));

	// TODO: setup dynamic like in SharedPhysX::SetupDynamic
#ifdef PHYSX_SUPPORT_STABILIZATION_FLAG
	vehActor->setStabilizationThreshold(0.0f);
#endif
}

static PxRigidDynamic* createVehicleActor4W(const PxVehicleChassisData& chassisData, PxConvexMesh** wheelConvexMeshes, PxConvexMesh* chassisConvexMesh,
											PxScene& scene, PxPhysics& physics, const PxMaterial& material, const PxTransform chassisLocalPose)
{
	//We need a rigid body actor for the vehicle.
	//Don't forget to add the actor the scene after setting up the associated vehicle.
	PxRigidDynamic* vehActor = physics.createRigidDynamic(PxTransform(PxIdentity));

	//We need to add wheel collision shapes, their local poses, a material for the wheels, and a simulation filter for the wheels.
	PxConvexMeshGeometry frontLeftWheelGeom(wheelConvexMeshes[0]);
	PxConvexMeshGeometry frontRightWheelGeom(wheelConvexMeshes[1]);
	PxConvexMeshGeometry rearLeftWheelGeom(wheelConvexMeshes[2]);
	PxConvexMeshGeometry rearRightWheelGeom(wheelConvexMeshes[3]);
	const PxGeometry* wheelGeometries[4]={&frontLeftWheelGeom,&frontRightWheelGeom,&rearLeftWheelGeom,&rearRightWheelGeom};
	const PxTransform wheelLocalPoses[4]={PxTransform(PxIdentity),PxTransform(PxIdentity),PxTransform(PxIdentity),PxTransform(PxIdentity)};
	const PxMaterial& wheelMaterial=material;

	//We need to add chassis collision shapes, their local poses, a material for the chassis, and a simulation filter for the chassis.
	PxConvexMeshGeometry chassisConvexGeom(chassisConvexMesh);
	const PxGeometry* chassisGeoms[1]={&chassisConvexGeom};
//	const PxTransform chassisLocalPoses[1]={PxTransform(PxIdentity)};
	const PxTransform chassisLocalPoses[1]={chassisLocalPose};
	const PxMaterial& chassisMaterial=material;

	//Set up the physx rigid body actor with shapes, local poses, and filters.
	setupActor(
		vehActor,
		wheelGeometries, wheelLocalPoses, 4, &wheelMaterial,
		chassisGeoms, chassisLocalPoses, 1, &chassisMaterial,
		chassisData,
		&physics);

	return vehActor;
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

static void createVehicle4WSimulationData(PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes,
										  PxVehicleWheelsSimData& wheelsData, PxVehicleDriveSimData4W& driveData, PxVehicleChassisData& chassisData,
										  const PINT_VEHICLE_CREATE& desc)
{
	//Extract the chassis AABB dimensions from the chassis convex mesh.
	const PxVec3 chassisDims = computeChassisAABBDimensions(chassisConvexMesh);
//	printf("chassisDims: %f %f %f\n", chassisDims.x, chassisDims.y, chassisDims.z);

	//The origin is at the center of the chassis mesh.
	//Set the center of mass to be below this point and a little towards the front.
	const PxVec3 chassisCMOffset(0.0f, -chassisDims.y*0.5f + desc.mChassisCMOffsetY, desc.mChassisCMOffsetZ);

	//Now compute the chassis mass and moment of inertia.
	//Use the moment of inertia of a cuboid as an approximate value for the chassis moi.
	PxVec3 chassisMOI
		((chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z)*desc.mChassisMass/12.0f,
		 (chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z)*desc.mChassisMass/12.0f,
		 (chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y)*desc.mChassisMass/12.0f);
	//A bit of tweaking here.  The car will have more responsive turning if we reduce the 	
	//y-component of the chassis moment of inertia.
	chassisMOI.y *= desc.mChassisMOICoeffY;

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
	PxVehicleWheelData wheels[4];
	for(PxU32 i=0;i<4;i++)
	{
		wheels[i].mRadius	= wheelRadii[i];
		wheels[i].mMass		= desc.mWheelMass;
		wheels[i].mMOI		= wheelMOIs[i];
		wheels[i].mWidth	= wheelWidths[i];
	}
	//Disable the handbrake from the front wheels and enable for the rear wheels
	wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxHandBrakeTorque=0.0f;
	wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxHandBrakeTorque=0.0f;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque=4000.0f;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque=4000.0f;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque=0.0f;
	wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque=0.0f;
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
		tires[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mType	= 0;
		tires[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mType	= 0;
		tires[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mType		= 1;
		tires[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mType	= 1;

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
		wheelCentreCMOffsets[i]=wheelCentreOffsets[i]-chassisCMOffset;
		suspForceAppCMOffsets[i]=PxVec3(wheelCentreCMOffsets[i].x, CMOffset, wheelCentreCMOffsets[i].z);
		tireForceAppCMOffsets[i]=PxVec3(wheelCentreCMOffsets[i].x, CMOffset, wheelCentreCMOffsets[i].z);
	}

	//Now add the wheel, tire and suspension data.
	for(PxU32 i=0;i<4;i++)
	{
		wheelsData.setWheelData(i,wheels[i]);
		wheelsData.setTireData(i,tires[i]);
		wheelsData.setSuspensionData(i,susps[i]);
		wheelsData.setSuspTravelDirection(i,suspTravelDirections[i]);
		wheelsData.setWheelCentreOffset(i,wheelCentreCMOffsets[i]);
		wheelsData.setSuspForceAppPointOffset(i,suspForceAppCMOffsets[i]);
		wheelsData.setTireForceAppPointOffset(i,tireForceAppCMOffsets[i]);
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

PxVehicleDrive4W* SampleVehicle_VehicleManager::create4WVehicle(SharedPhysX_Vehicles* sharedPhysX, PxPhysics& physics, PxCooking& cooking, const PxMaterial& material,
																PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
																bool useAutoGearFlag, const PINT_VEHICLE_CREATE& desc)
{
	PX_ASSERT(mNumVehicles<MAX_NUM_4W_VEHICLES);

	PxScene* scene = sharedPhysX->GetScene();
	ASSERT(scene);

	PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(4);
	PxVehicleDriveSimData4W driveSimData;
	PxVehicleChassisData chassisData;
	createVehicle4WSimulationData(chassisConvexMesh, wheelConvexMeshes4, *wheelsSimData, driveSimData, chassisData, desc);

	//Instantiate and finalize the vehicle using physx.
	PxRigidDynamic* vehActor = createVehicleActor4W(chassisData, wheelConvexMeshes4, chassisConvexMesh, *scene, physics, material, ToPxTransform(desc.mChassisLocalPose));

#ifdef DISABLE_WHEELS
	wheelsSimData->disableWheel(2);
	wheelsSimData->disableWheel(3);
#endif

#ifdef PHYSX_SUPPORT_LIMIT_SUSPENSION_EXPANSION_VELOCITY
	wheelsSimData->setFlags(PxVehicleWheelsSimFlag::eLIMIT_SUSPENSION_EXPANSION_VELOCITY);
#endif

#ifdef PHYSX_SUPPORT_ANTIROLLBAR
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
		PxSceneWriteLock scopedLock(*scene);
//		scene.addActor(*vehActor);
		sharedPhysX->AddActor(vehActor);
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
	const PxTransform startTransform(ToPxVec3(desc.mStartPose.mPos), ToPxQuat(desc.mStartPose.mRot));
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

#define MAX_NUM_INDEX_BUFFERS	16

namespace
{
/*	class VehicleData : public Allocateable
	{
		public:
											VehicleData();
											~VehicleData();
	};*/

	class VehicleTest : public Allocateable
	{
		public:
											VehicleTest(SharedPhysX_Vehicles* physx);
											~VehicleTest();

			PxVehicleDrive4W*				Init(	SharedPhysX_Vehicles* physx,
													PxConvexMesh* chassis_mesh,
													PxConvexMesh* wheel_mesh0, PintShapeRenderer* renderer0,
													PxConvexMesh* wheel_mesh1, PintShapeRenderer* renderer1,
													PxConvexMesh* wheel_mesh2, PintShapeRenderer* renderer2,
													PxConvexMesh* wheel_mesh3, PintShapeRenderer* renderer3,
													const PINT_VEHICLE_CREATE& desc);

			void							Update(float dt);

			PxScene*						mScene;
			SampleVehicle_VehicleController	mVehicleController;
			SampleVehicle_VehicleManager	mVehicleManager;
			PxVehicleDrivableSurfaceType	mVehicleDrivableSurfaceTypes[MAX_NUM_INDEX_BUFFERS];
			PxMaterial*						mStandardMaterials[MAX_NUM_INDEX_BUFFERS];
			PxMaterial*						mChassisMaterial;

			PINT_VEHICLE_INPUT				mInput;
	};
}

//static VehicleTest* gVehicleTest = null;

void SharedPhysX_Vehicles::CloseVehicles()
{
	const PxU32 nbVehicles = mVehicles.size();
	for(PxU32 i=0;i<nbVehicles;i++)
	{
		Vehicle* v = mVehicles[i];
		VehicleTest* vt = reinterpret_cast<VehicleTest*>(v->mImpl);
		DELETESINGLE(vt);
		DELETESINGLE(v);
	}
	mVehicles.reset();

//	DELETESINGLE(gVehicleTest);
}

void SharedPhysX_Vehicles::UpdateVehicles(float dt)
{
	const udword NbSubsteps = mParams.mNbSubsteps;
	const float sdt = dt/float(NbSubsteps);

//	if(gVehicleTest)
//	{
//		gVehicleTest->Update(dt);

	const PxU32 nbVehicles = mVehicles.size();
	for(PxU32 j=0;j<nbVehicles;j++)
	{
		Vehicle* v = mVehicles[j];
		VehicleTest* vt = reinterpret_cast<VehicleTest*>(v->mImpl);

		for(udword i=0;i<NbSubsteps;i++)
		{
//			gVehicleTest->Update(sdt);
			vt->Update(sdt);
		}
	}
}

#ifdef PHYSX_SUPPORT_GEAR_JOINT_
	#include "PxGearJoint.h"
#endif
void SharedPhysX_Vehicles::UpdateVehiclesAndPhysX(float dt)
{
#ifdef PHYSX_SUPPORT_GEAR_JOINT_
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
		const udword NbSubsteps = mParams.mNbSubsteps;
		const float sdt = dt/float(NbSubsteps);
		for(udword i=0;i<NbSubsteps;i++)
		{
//			if(gVehicleTest)
//				gVehicleTest->Update(sdt);
			const PxU32 nbVehicles = mVehicles.size();
			for(PxU32 j=0;j<nbVehicles;j++)
			{
				Vehicle* v = mVehicles[j];
				VehicleTest* vt = reinterpret_cast<VehicleTest*>(v->mImpl);
				vt->Update(sdt);
			}

			if(mScene)
			{
				if(NbSubsteps>1)
					ApplyLocalTorques();

#ifdef PHYSX_SUPPORT_SQ_UPDATE_MODE
				const bool IsLastSubstep = i==NbSubsteps-1;
				mScene->setSceneQueryUpdateMode(IsLastSubstep ? PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_ENABLED : PxSceneQueryUpdateMode::eBUILD_DISABLED_COMMIT_DISABLED);
#endif
				mScene->simulate(sdt, null, GetScratchPad(), GetScratchPadSize());
				mScene->fetchResults(true);
//				mScene->fetchResults(false);
			}
		}

		EndCommonUpdate();
	}
}

VehicleTest::VehicleTest(SharedPhysX_Vehicles* physx) : mScene(null)
{
	mScene = physx->GetScene();
	PxCooking& cooking = *physx->GetCooking();
	PxPhysics& physics = *physx->GetPhysics();

	mChassisMaterial = physics.createMaterial(0.0f, 0.0f, 0.0f);
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

	mVehicleManager.init(physics, (const PxMaterial**)mStandardMaterials, mVehicleDrivableSurfaceTypes);
}

VehicleTest::~VehicleTest()
{
	mVehicleManager.shutdown();
}

void VehicleTest::Update(float dt)
{
		mVehicleController.setCarKeyboardInputs(
/*			mControlInputs.getAccelKeyPressed(),
			mControlInputs.getBrakeKeyPressed(),
			mControlInputs.getHandbrakeKeyPressed(),
			mControlInputs.getSteerLeftKeyPressed(),
			mControlInputs.getSteerRightKeyPressed(),
			mControlInputs.getGearUpKeyPressed(),
			mControlInputs.getGearDownKeyPressed()*/
			mInput.mAccelerate,
			mInput.mBrake,	false,
//			false,	mInput.mBrake,
			mInput.mRight,
			mInput.mLeft,
			false,
			false
			);
		mVehicleController.setCarGamepadInputs(
/*			mControlInputs.getAccel(),
			mControlInputs.getBrake(),
			mControlInputs.getSteer(),
			mControlInputs.getGearUp(),
			mControlInputs.getGearDown(),
			mControlInputs.getHandbrake()*/
			false,
			false,
			false,
			false,
			false,
			false
			);

//	updateVehicleController(dtime);
	const PxU32 mPlayerVehicle=0;
	mVehicleController.update(dt, mVehicleManager.getVehicleWheelQueryResults(mPlayerVehicle), *mVehicleManager.getVehicle(mPlayerVehicle));
//	mVehicleController.update(dt, mVehicleManager.getVehicleWheelQueryResults(1), *mVehicleManager.getVehicle(1));

	mVehicleManager.suspensionRaycasts(mScene);
//return;

	if(1 && dt>0.0f)
	{
//#if PX_DEBUG_VEHICLE_ON
//		mVehicleManager.updateAndRecordTelemetryData(dt, mScene->getGravity(), mVehicleManager.getVehicle(mPlayerVehicle), mTelemetryData4W);
//#else
		mVehicleManager.update(dt, mScene->getGravity());
//#endif
	}
}




////////////////////////////////////////////////////////////////
//VEHICLE SETUP DATA 
////////////////////////////////////////////////////////////////

PxVehicleDrive4W* VehicleTest::Init(SharedPhysX_Vehicles* physx, PxConvexMesh* chassis_mesh,
									PxConvexMesh* wheel_mesh0, PintShapeRenderer* renderer0,
									PxConvexMesh* wheel_mesh1, PintShapeRenderer* renderer1,
									PxConvexMesh* wheel_mesh2, PintShapeRenderer* renderer2,
									PxConvexMesh* wheel_mesh3, PintShapeRenderer* renderer3,
									const PINT_VEHICLE_CREATE& desc)
{
	PxCooking& cooking = *physx->GetCooking();
	PxPhysics& physics = *physx->GetPhysics();

//	mVehicleManager.init(physics, (const PxMaterial**)mStandardMaterials, mVehicleDrivableSurfaceTypes);
	mVehicleManager.setTireFrictionMultiplier(desc.mFrontTireFrictionMultiplier, desc.mRearTireFrictionMultiplier);

	PxConvexMesh* wm[4] = { wheel_mesh0, wheel_mesh1, wheel_mesh2, wheel_mesh3 };

	PxVehicleDrive4W* Vehicle4W = mVehicleManager.create4WVehicle(physx, physics, cooking, *mChassisMaterial, chassis_mesh, wm, true, desc);
	ASSERT(Vehicle4W);

	// ### Wheels & chassis all added to the same compound????
	PxRigidDynamic* RD = Vehicle4W->getRigidDynamicActor();
	ASSERT(RD->getNbShapes()==5);
	PxShape* Shapes[16];
	RD->getShapes(Shapes, 16);
	Shapes[0]->userData = renderer0;//desc.mWheels.mRenderer;
	Shapes[1]->userData = renderer1;//desc.mWheels.mRenderer;
	Shapes[2]->userData = renderer2;//desc.mWheels.mRenderer;
	Shapes[3]->userData = renderer3;//desc.mWheels.mRenderer;
	Shapes[4]->userData = desc.mChassis->mRenderer;

//	PxTransform localPose0 = Shapes[0]->getLocalPose();
//	printf("LocalPose0: %f %f %f\n", localPose0.p.x, localPose0.p.y, localPose0.p.z);

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

	// TODO: make this per-vehicle
	CopyMemory(gSteerVsForwardSpeedData, vehicle.mSteerVsForwardSpeedData, sizeof(float)*8);

//	if(!gVehicleTest)
//		gVehicleTest = ICE_NEW(VehicleTest)(this);

	VehicleTest* vt = ICE_NEW(VehicleTest)(this);

	const PINT_CONVEX_CREATE* ChassisDesc = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mChassis);
	PxConvexMesh* ChassisMesh = CreateConvexMesh(ChassisDesc->mVerts, ChassisDesc->mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX, vehicle.mChassis->mRenderer);
	ASSERT(ChassisMesh);

	const PINT_CONVEX_CREATE* WheelDesc0 = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mWheels[0]);
	PxConvexMesh* WheelMesh0 = CreateConvexMesh(WheelDesc0->mVerts, WheelDesc0->mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX, WheelDesc0->mRenderer);
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
		WheelMesh1 = CreateConvexMesh(wheel1->mVerts, wheel1->mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX, wheel1->mRenderer);
		ASSERT(WheelMesh1);
		Renderer1 = wheel1->mRenderer;

		ASSERT(vehicle.mWheels[2]->mType==PINT_SHAPE_CONVEX);
		const PINT_CONVEX_CREATE* wheel2 = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mWheels[2]);
		WheelMesh2 = CreateConvexMesh(wheel2->mVerts, wheel2->mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX, wheel2->mRenderer);
		ASSERT(WheelMesh2);
		Renderer2 = wheel2->mRenderer;
		
		ASSERT(vehicle.mWheels[3]->mType==PINT_SHAPE_CONVEX);
		const PINT_CONVEX_CREATE* wheel3 = static_cast<const PINT_CONVEX_CREATE*>(vehicle.mWheels[3]);
		WheelMesh3 = CreateConvexMesh(wheel3->mVerts, wheel3->mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX, wheel3->mRenderer);
		ASSERT(WheelMesh3);
		Renderer3 = wheel3->mRenderer;
	}

	PxVehicleDrive4W* Vehicle4W = vt->Init(	this, ChassisMesh,
											WheelMesh0, WheelDesc0->mRenderer,
											WheelMesh1, Renderer1,
											WheelMesh2, Renderer2,
											WheelMesh3, Renderer3,
											vehicle);
	ASSERT(Vehicle4W);

	data.mChassis = Vehicle4W->getRigidDynamicActor();

	Vehicle* v = ICE_NEW(Vehicle);
	v->mVehicle = Vehicle4W;
	v->mImpl	= vt;
	mVehicles.pushBack(v);
	return v;
//	return Vehicle4W;
}

bool SharedPhysX_Vehicles::SetVehicleInput(PintVehicleHandle vehicle, const PINT_VEHICLE_INPUT& input)
{
	Vehicle* v = reinterpret_cast<Vehicle*>(vehicle);
	if(!v)
		return false;
//	PxVehicleDrive4W* Vehicle4W = v->mVehicle;
	VehicleTest* vt = reinterpret_cast<VehicleTest*>(v->mImpl);

//	PxVehicleDrive4W* Vehicle4W = reinterpret_cast<PxVehicleDrive4W*>(vehicle);
//	if(!Vehicle4W)
//		return false;

//	if(gVehicleTest)
//		gVehicleTest->mInput = input;
	vt->mInput = input;

	return true;
}

bool SharedPhysX_Vehicles::GetVehicleInfo(PintVehicleHandle vehicle, PintVehicleInfo& info)
{
//	PxVehicleDrive4W* Vehicle4W = reinterpret_cast<PxVehicleDrive4W*>(vehicle);
//	if(!Vehicle4W)
//		return false;
	Vehicle* v = reinterpret_cast<Vehicle*>(vehicle);
	if(!v)
		return false;
	PxVehicleDrive4W* Vehicle4W = v->mVehicle;

	const PxVehicleDriveDynData& driveDynData = Vehicle4W->mDriveDynData;

	info.mForwardSpeed	= Vehicle4W->computeForwardSpeed();
	info.mSidewaysSpeed	= Vehicle4W->computeSidewaysSpeed();
	info.mCurrentGear	= driveDynData.getCurrentGear();
	info.mRevs			= driveDynData.getEngineRotationSpeed();
	return true;
}

bool SharedPhysX_Vehicles::ResetVehicleData(PintVehicleHandle vehicle)
{
//	PxVehicleDrive4W* Vehicle4W = reinterpret_cast<PxVehicleDrive4W*>(vehicle);
//	if(!Vehicle4W)
//		return false;
	Vehicle* v = reinterpret_cast<Vehicle*>(vehicle);
	if(!v)
		return false;
	PxVehicleDrive4W* Vehicle4W = v->mVehicle;

	// Set the car back to its rest state.
	Vehicle4W->setToRestState();

	// Set the car to first gear.
	Vehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);

	return true;
}

bool SharedPhysX_Vehicles::	AddShape(PintVehicleHandle vehicle, const PINT_SHAPE_CREATE& create)
{
//	PxVehicleDrive4W* Vehicle4W = reinterpret_cast<PxVehicleDrive4W*>(vehicle);
//	if(!Vehicle4W)
//		return false;
	Vehicle* v = reinterpret_cast<Vehicle*>(vehicle);
	if(!v)
		return false;
	PxVehicleDrive4W* Vehicle4W = v->mVehicle;

	CreateShapes(&create, Vehicle4W->getRigidDynamicActor(), 0);

	return true;
}

Pint_Vehicle* SharedPhysX_Vehicles::GetVehicleAPI()
{
	return this;
}

