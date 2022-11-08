///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#include "PINT_PhysX50_Imm.h"
#include "..\PINT_Common\PINT_CommonPhysX4_ImmPint.h"

#include "..\PINT_Common\PINT_Common.h"

#include "PxPhysicsAPI.h"
#include "PxImmediateMode.h"

#include "..\PINT_Common\PINT_CommonPhysX_FoundationAPI.h"

//#include "Extensions\ExtConstraintHelper.h"

#include "..\PintShapeRenderer.h"

#include "..\PINT_Common\PINT_CommonPhysX3_Error.h"
#include "..\PINT_Common\PINT_CommonPhysX3_Allocator.h"
#include "..\PINT_Common\PINT_CommonPhysX3_Base.h"

using namespace physx;
using namespace immediate;

/*
PEEL Plugin TODO:
- allow statics to be created at any time
- must return index+1 because PEEL calling code sometimes asserts that returned handle isn't a null ptr

- support for collision groups
- support for local poses
- support for compounds

- share all shapes

*/

/*
updateArticulations: 0
updateBounds: 95
broadPhase: 207686
narrowPhase: 12419
buildSolverBodyData: 332
buildSolverConstraintDesc: 139
createContactConstraints: 11306
solveAndIntegrate: 4952

updateArticulations: 0
updateBounds: 96
broadPhase: 39092
narrowPhase: 11419
buildSolverBodyData: 330
buildSolverConstraintDesc: 114
createContactConstraints: 6723
solveAndIntegrate: 4042
*/

// TODO: move to separate file & share
#ifdef IMM_NEEDS_PX_PHYSICS
	class MemoryOutputStream : public PxOutputStream
	{
	public:
						MemoryOutputStream(PEEL_PhysX3_AllocatorCallback* allocator=null);
	virtual				~MemoryOutputStream();

			PxU32		write(const void* src, PxU32 count);

			PxU32		getSize()	const	{	return mSize; }
			PxU8*		getData()	const	{	return mData; }
	private:
			PEEL_PhysX3_AllocatorCallback*	mCallback;
			PxU8*		mData;
			PxU32		mSize;
			PxU32		mCapacity;
	};

	class MemoryInputData : public PxInputData
	{
	public:
						MemoryInputData(PxU8* data, PxU32 length);

			PxU32		read(void* dest, PxU32 count);
			PxU32		getLength() const;
			void		seek(PxU32 pos);
			PxU32		tell() const;

	private:
			PxU32		mSize;
			const PxU8*	mData;
			PxU32		mPos;
	};

MemoryOutputStream::MemoryOutputStream(PEEL_PhysX3_AllocatorCallback* allocator) :
	mCallback	(allocator),
	mData		(NULL),
	mSize		(0),
	mCapacity	(0)
{
}

MemoryOutputStream::~MemoryOutputStream()
{
	if(mData)
	{
		if(mCallback)	mCallback->deallocate(mData);
		else			delete[] mData;
	}
}

PxU32 MemoryOutputStream::write(const void* src, PxU32 size)
{
	const PxU32 expectedSize = mSize + size;
	if(expectedSize > mCapacity)
	{
//		mCapacity = expectedSize + 4096;
		mCapacity = NextPowerOfTwo(expectedSize);
		PxU8* newData = mCallback ? (PxU8*)mCallback->allocate(mCapacity, null, null, 0) : new PxU8[mCapacity];
		PX_ASSERT(newData!=NULL);

		if(newData)
		{
			memcpy(newData, mData, mSize);
			if(mCallback)	mCallback->deallocate(mData);
			else			delete[] mData;
		}
		mData = newData;
	}
	memcpy(mData+mSize, src, size);
	mSize += size;
	return size;
}

///////////////////////////////////////////////////////////////////////////////

MemoryInputData::MemoryInputData(PxU8* data, PxU32 length) :
	mSize	(length),
	mData	(data),
	mPos	(0)
{
}

PxU32 MemoryInputData::read(void* dest, PxU32 count)
{
	PxU32 length = PxMin<PxU32>(count, mSize-mPos);
	memcpy(dest, mData+mPos, length);
	mPos += length;
	return length;
}

PxU32 MemoryInputData::getLength() const
{
	return mSize;
}

void MemoryInputData::seek(PxU32 offset)
{
	mPos = PxMin<PxU32>(mSize, offset);
}

PxU32 MemoryInputData::tell() const
{
	return mPos;
}

#endif

#include "..\PINT_Common\PINT_CommonPhysX4_Imm.h"

static PxVec3		gGravity(0.0f, -9.81f, 0.0f);
static const float	gContactDistance			= 0.1f;
static const float	gMeshContactMargin			= 0.01f;
static const float	gToleranceLength			= 1.0f;
static const float	gBounceThreshold			= -2.0f;
static const float	gFrictionOffsetThreshold	= 0.04f;
static const float	gCorrelationDistance		= 0.025f;
static const float	gBoundsInflation			= 0.02f;
static const float	gStaticFriction				= 0.5f;
static const float	gDynamicFriction			= 0.5f;
static const float	gRestitution				= 0.0f;
static const float	gMaxDepenetrationVelocity	= 3.0f;
static const float	gMaxContactImpulse			= FLT_MAX;
static const float	gLinearDamping				= 0.1f;
static const float	gAngularDamping				= 0.05f;
static const float	gMaxLinearVelocity			= 100.0f;
static const float	gMaxAngularVelocity			= 100.0f;
static const PxU32	gNbIterPos					= 4;
static const PxU32	gNbIterVel					= 1;

static bool gUseTGS = false;
static bool gUsePersistentContacts = true;
static bool gBatchContacts = true;

#define	NB_DEBUG_VIZ_PARAMS	2
static			bool	gDebugVizParams[NB_DEBUG_VIZ_PARAMS] = {0};
static	const	char*	gDebugVizNames[NB_DEBUG_VIZ_PARAMS] =
{
	"Enable debug visualization",
//	"Draw wireframe",
	"Draw AABBs",
//	"Draw contact points",
//	"Draw constraints",
//	"Draw constraints limits",
};
enum DebugViz
{
	DEBUG_VIZ_ENABLE,
	DEBUG_VIZ_BOUNDS,
};

static PX_FORCE_INLINE ImmActorHandle PintHandleToImmHandle(PintActorHandle handle)
{
	return ImmActorHandle(size_t(handle)-1);
}

static PX_FORCE_INLINE PintActorHandle ImmHandleToPintHandle(ImmActorHandle handle)
{
	return PintActorHandle(size_t(handle)+1);	//###TODO: revisit this
}

/*static Dy::ArticulationV* createImmediateArticulation(bool fixBase, Array<Dy::ArticulationV*>& articulations)
{
	PxFeatherstoneArticulationData data;
	data.flags = fixBase ? PxArticulationFlag::eFIX_BASE : PxArticulationFlag::Enum(0);
	Dy::ArticulationV* immArt = PxCreateFeatherstoneArticulation(data);
	articulations.pushBack(immArt);
	return immArt;
}*/

static void setupCommonLinkData(PxFeatherstoneArticulationLinkData& data, Dy::ArticulationLinkHandle parent, const PxTransform& pose, const MassProps& massProps)
{
	data.parent					= parent;
	data.pose					= pose;
	data.inverseMass			= massProps.mInvMass;
	data.inverseInertia			= massProps.mInvInertia;
	data.linearDamping			= gLinearDamping;
	data.angularDamping			= gAngularDamping;
	data.maxLinearVelocitySq	= gMaxLinearVelocity * gMaxLinearVelocity;
	data.maxAngularVelocitySq	= gMaxAngularVelocity * gMaxAngularVelocity;
}

static void SetModeAndLimits(PxFeatherstoneArticulationLinkData& data, PxArticulationAxis::Enum axis, float min_limit, float max_limit)
{
	if(min_limit>max_limit)
		data.inboundJoint.motion[axis] = PxArticulationMotion::eFREE;
	if(min_limit==max_limit)
		data.inboundJoint.motion[axis] = PxArticulationMotion::eLOCKED;
	if(min_limit<max_limit)
	{
		data.inboundJoint.motion[axis] = PxArticulationMotion::eLIMITED;
		data.inboundJoint.limits[axis].low = min_limit;
		data.inboundJoint.limits[axis].high = max_limit;
	}
}

static void setupJointLinkData(PxFeatherstoneArticulationLinkData& data, const PINT_RC_ARTICULATED_BODY_CREATE& bc)
{
	if(bc.mJointType==PINT_JOINT_SPHERICAL)
	{
		data.inboundJoint.type	= PxArticulationJointType::eSPHERICAL;
//		data.inboundJoint.motion[PxArticulationAxis::eSWING2] = PxArticulationMotion::eFREE;
//		data.inboundJoint.motion[PxArticulationAxis::eSWING1] = PxArticulationMotion::eFREE;
//		data.inboundJoint.motion[PxArticulationAxis::eTWIST] = PxArticulationMotion::eFREE;
		SetModeAndLimits(data, PxArticulationAxis::eTWIST, bc.mMinTwistLimit, bc.mMaxTwistLimit);
		SetModeAndLimits(data, PxArticulationAxis::eSWING1, bc.mMinSwing1Limit, bc.mMaxSwing1Limit);
		SetModeAndLimits(data, PxArticulationAxis::eSWING2, bc.mMinSwing2Limit, bc.mMaxSwing2Limit);
	}
	else if(bc.mJointType==PINT_JOINT_HINGE)
	{
		data.inboundJoint.type	= PxArticulationJointType::eREVOLUTE;

		PxArticulationAxis::Enum Axis = PxArticulationAxis::eCOUNT;
		if(bc.mAxisIndex==X_)
			Axis = PxArticulationAxis::eTWIST;
		else if(bc.mAxisIndex==Y_)
			Axis = PxArticulationAxis::eSWING1;
		else if(bc.mAxisIndex==Z_)
			Axis = PxArticulationAxis::eSWING2;

/*		const float MinLimit = bc.mMinLimit;
		const float MaxLimit = bc.mMaxLimit;

		if(MinLimit>MaxLimit)
			data.inboundJoint.motion[Axis] = PxArticulationMotion::eFREE;
		if(MinLimit==MaxLimit)
			data.inboundJoint.motion[Axis] = PxArticulationMotion::eLOCKED;
		if(MinLimit<MaxLimit)
		{
			data.inboundJoint.motion[Axis] = PxArticulationMotion::eLIMITED;
			data.inboundJoint.limits[Axis].low = MinLimit;
			data.inboundJoint.limits[Axis].high = MaxLimit;
		}*/
		SetModeAndLimits(data, Axis, bc.mMinLimit, bc.mMaxLimit);

		if(bc.mUseMotor)
		{
			data.inboundJoint.drives[Axis].stiffness = bc.mMotor.mStiffness;
			data.inboundJoint.drives[Axis].damping = bc.mMotor.mDamping;
			data.inboundJoint.drives[Axis].maxForce = bc.mMotor.mMaxForce;
//			data.inboundJoint.drives[Axis].isAcceleration = bc.mMotor.mAccelerationDrive;
			data.inboundJoint.drives[Axis].driveType = bc.mMotor.mAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE;
			data.inboundJoint.targetVel[Axis] = bc.mTargetVel;
		}
	}
	else if(bc.mJointType==PINT_JOINT_PRISMATIC)
	{
		data.inboundJoint.type	= PxArticulationJointType::ePRISMATIC;

		PxArticulationAxis::Enum Axis = PxArticulationAxis::eCOUNT;
		if(bc.mAxisIndex==X_)
			Axis = PxArticulationAxis::eX;
		else if(bc.mAxisIndex==Y_)
			Axis = PxArticulationAxis::eY;
		else if(bc.mAxisIndex==Z_)
			Axis = PxArticulationAxis::eZ;

/*		const float MinLimit = bc.mMinLimit;
		const float MaxLimit = bc.mMaxLimit;

		if(MinLimit>MaxLimit)
			data.inboundJoint.motion[Axis] = PxArticulationMotion::eFREE;
		if(MinLimit==MaxLimit)
			data.inboundJoint.motion[Axis] = PxArticulationMotion::eLOCKED;
		if(MinLimit<MaxLimit)
		{
			data.inboundJoint.motion[Axis] = PxArticulationMotion::eLIMITED;
			data.inboundJoint.limits[Axis].low = MinLimit;
			data.inboundJoint.limits[Axis].high = MaxLimit;
		}*/
		SetModeAndLimits(data, Axis, bc.mMinLimit, bc.mMaxLimit);

//		j->setMotion(PxArticulationAxis::eZ, PxArticulationMotion::eLIMITED);
//		j->setLimit(PxArticulationAxis::eZ, -1.4f, 0.2f);
	}
	else if(bc.mJointType==PINT_JOINT_FIXED)
	{
		data.inboundJoint.type	= PxArticulationJointType::eFIX;
	}
	else ASSERT(0);

	data.inboundJoint.parentPose			= ToPxTransform(bc.mLocalPivot0);
	data.inboundJoint.childPose				= ToPxTransform(bc.mLocalPivot1);
	data.inboundJoint.frictionCoefficient	= bc.mFrictionCoeff;
	data.inboundJoint.maxJointVelocity		= bc.mMaxJointVelocity;
}

///////////////////////////////////////////////////////////////////////////////

static PEEL_PhysX3_AllocatorCallback* gDefaultAllocator = null;
static PxErrorCallback* gDefaultErrorCallback = null;
static ImmediateScene* gScene = null;	//#####

///////////////////////////////////////////////////////////////////////////////

PhysXImm::PhysXImm() :
	mActorAPI	(*this),
	mScene		(null),
	mFoundation	(null),
	mCooking	(null)
#ifdef IMM_NEEDS_PX_PHYSICS
	,mPhysics	(null)
#endif
{
}

PhysXImm::~PhysXImm()
{
}

void PhysXImm::GetCaps(PintCaps& caps) const
{
	caps.mSupportRigidBodySimulation = true;
	caps.mSupportConvexes = true;
	caps.mSupportMeshes = true;
	caps.mSupportSphericalJoints = true;
	caps.mSupportRCArticulations = true;
	caps.mSupportCollisionGroups = true;
	caps.mSupportCompounds = true;
	caps.mSupportRaycasts = true;
}

void PhysXImm::Init(const PINT_WORLD_CREATE& desc)
{
	gDefaultAllocator = new PEEL_PhysX3_AllocatorCallback;
	gDefaultErrorCallback = new PEEL_PhysX3_ErrorCallback;

	mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *gDefaultAllocator, *gDefaultErrorCallback);

	PxTolerancesScale scale;
#ifdef IMM_NEEDS_PX_PHYSICS
	{
		ASSERT(!mPhysics);
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, scale);
		ASSERT(mPhysics);
	}
#endif

	PxRegisterImmediateArticulations();

	{
		ASSERT(!mCooking);
		PxCookingParams Params(scale);
		Params.midphaseDesc.setToDefault(PxMeshMidPhase::eBVH34);
		Params.midphaseDesc.mBVH34Desc.numPrimsPerLeaf = 4;
// TODO: expose this to UI
//Params.midphaseDesc.mBVH34Desc.numPrimsPerLeaf = 2;
//Params.midphaseDesc.mBVH34Desc.quantized = false;
		mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, Params);
		ASSERT(mCooking);
	}

	gGravity = ToPxVec3(desc.mGravity);

	mScene = new ImmediateScene(gUseTGS, gUsePersistentContacts, gBatchContacts);
	gScene = mScene;
}

void PhysXImm::SetGravity(const Point& gravity)
{
	gGravity = ToPxVec3(gravity);
}

void PhysXImm::Close()
{
	ReleaseMeshes();
#ifdef PHYSX_NEW_PUBLIC_API
	PX_DELETE(mScene);
#else
	PX_DELETE_AND_RESET(mScene);
#endif
	gScene = null;
	SAFE_RELEASE(mCooking);
#ifdef IMM_NEEDS_PX_PHYSICS
	SAFE_RELEASE(mPhysics);
#endif
	SAFE_RELEASE(mFoundation);
	DELETESINGLE(gDefaultErrorCallback);
	DELETESINGLE(gDefaultAllocator);
}

udword PhysXImm::Update(float dt)
{
	if(!mScene)
		return gDefaultAllocator->mCurrentMemory;

//	if(gPause && !gOneFrame)
//		return;
//	gOneFrame = false;

	const float invDt = 1.0f/dt;
	const bool printTimings = false;

	{
		unsigned long long time = __rdtsc();
		mScene->updateArticulations(dt, gGravity, gNbIterPos);
		time = __rdtsc() - time;
		if(printTimings)
			printf("updateArticulations: %d           \n", PxU32(time/1024));
	}
	{
		unsigned long long time = __rdtsc();
		mScene->updateBounds(gBoundsInflation);
		time = __rdtsc() - time;
		if(printTimings)
			printf("updateBounds: %d           \n", PxU32(time/1024));
	}
	{
		unsigned long long time = __rdtsc();
		mScene->broadPhase();
		time = __rdtsc() - time;
		if(printTimings)
			printf("broadPhase: %d           \n", PxU32(time/1024));
	}
	{
		unsigned long long time = __rdtsc();
		mScene->narrowPhase(gContactDistance, gMeshContactMargin, gToleranceLength, gStaticFriction, gDynamicFriction, gRestitution);
		time = __rdtsc() - time;
		if(printTimings)
			printf("narrowPhase: %d           \n", PxU32(time/1024));
	}
	{
		unsigned long long time = __rdtsc();
		mScene->buildSolverBodyData(dt, gGravity, gMaxDepenetrationVelocity, gMaxContactImpulse, gLinearDamping, gAngularDamping, gMaxLinearVelocity, gMaxAngularVelocity);
		time = __rdtsc() - time;
		if(printTimings)
			printf("buildSolverBodyData: %d           \n", PxU32(time/1024));
	}
	{
		unsigned long long time = __rdtsc();
		mScene->buildSolverConstraintDesc();
		time = __rdtsc() - time;
		if(printTimings)
			printf("buildSolverConstraintDesc: %d           \n", PxU32(time/1024));
	}
	{
		unsigned long long time = __rdtsc();
		mScene->createContactConstraints(dt, invDt, gBounceThreshold, gFrictionOffsetThreshold, gCorrelationDistance, 1.0f, gNbIterPos);
		time = __rdtsc() - time;
		if(printTimings)
			printf("createContactConstraints: %d           \n", PxU32(time/1024));
	}
	{
		unsigned long long time = __rdtsc();
		mScene->solveAndIntegrate(dt, gNbIterPos, gNbIterVel);
		time = __rdtsc() - time;
		if(printTimings)
			printf("solveAndIntegrate: %d           \n", PxU32(time/1024));
	}

	return gDefaultAllocator->mCurrentMemory;
}

Point PhysXImm::GetMainColor()
{
	return Point(238.0f/255.0f, 223.0f/255.0f, 204.0f/255.0f);
}

static const PxQuat q = PxShortestRotation(PxVec3(0.0f, 1.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.0f));

static void /*PhysX3::*/ComputeCapsuleTransform(PR& dst, const PR& src)
{
	// ### PhysX is weird with capsules
/*	Matrix3x3 Rot;
	Rot.RotZ(HALFPI);
	Quat QQ = src.mRot * Quat(Rot);*/

	Quat QQ = src.mRot * ToQuat(q);

	dst.mPos = src.mPos;
	dst.mRot = QQ;
}

void PhysXImm::Render(PintRender& renderer, PintRenderPass render_pass)
{
	if(!mScene)
		return;

	PxU32 NbActors;
	const ImmediateActor* Actors = mScene->getActors(NbActors);

	PxU32 NbPoses;
	const PxTransform* GlobalPoses = mScene->getActorGlobalPoses(NbPoses);
	PX_ASSERT(NbPoses==NbActors);

	while(NbActors--)
	{
		const ImmediateActor& CurrentActor = *Actors++;
		const PxTransform& CurrentPose = *GlobalPoses++;

		const PxU32 nbShapes = CurrentActor.mNbShapes;
		for(PxU32 i=0;i<nbShapes;i++)
		{
			const ImmediateShape& CurrentShape = mScene->getShape(CurrentActor.mShapeHandle[i]);
			const PxGeometry CurrentGeom = CurrentShape.mGeom.any();
			const PxTransform& LocalPose = CurrentShape.mLocalPose;

	//		const PR IcePose(ToPR(CurrentPose));
			const PxTransform WorldPose = CurrentPose * LocalPose;
			/*const*/ PR IcePose(ToPR(WorldPose));


	const PxGeometryType::Enum geomType = CurrentGeom.getType();
	if(geomType==PxGeometryType::eCAPSULE)
	{
	//	PR CapsuleTransform;
	//	PhysX3::ComputeCapsuleTransform(CapsuleTransform, IcePose);
		/*PhysX3::*/ComputeCapsuleTransform(IcePose, IcePose);
	}

			if(CurrentShape.mUserData)
			{
				PintShapeRenderer* renderer = reinterpret_cast<PintShapeRenderer*>(CurrentShape.mUserData);
				renderer->_Render(IcePose);
			}
			else
				ASSERT(0);
		}
	}
}

void PhysXImm::RenderDebugData(PintRender& renderer)
{
	if(!mScene)
		return;

	if(gDebugVizParams[DEBUG_VIZ_BOUNDS])
	{
		const Point color(1.0f, 1.0f, 0.0f);
		//const PxU32 size = mScene->mBounds.size();
		//const PxBounds3* bounds = mScene->mBounds.begin();
		PxU32 size;
		const PxBounds3* bounds = mScene->getBounds(size);
		for(PxU32 i=0;i<size;i++)
		{
			AABB box;
			box.mMin = ToPoint(bounds[i].minimum);
			box.mMax = ToPoint(bounds[i].maximum);
			renderer.DrawWireframeAABB(1, &box, color);
		}
	}
}

void PhysXImm::SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)
{
}

static void SetupActor(const PINT_OBJECT_CREATE& desc, ImmediateScene* scene, ImmActorHandle handle)
{
	ImmediateActor& actor = scene->getActor(handle);
	actor.mLinearVelocity = ToPxVec3(desc.mLinearVelocity);
	actor.mAngularVelocity = ToPxVec3(desc.mAngularVelocity);
}

#ifndef SHAPE_CENTRIC
static ImmActorHandle CreateActor(const PINT_OBJECT_CREATE& desc, const PxGeometry& geom, const PxTransform& pose, const PINT_SHAPE_CREATE* shape)
{
	PxTransform localPose(PxIdentity);
	if(geom.getType()==PxGeometryType::eCAPSULE)
	{
		const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.0f));
		localPose = PxTransform(q);
	}
	const ImmShapeHandle shapeHandle = mScene->createShape(geom, localPose, shape->mRenderer);

	ImmActorHandle id;
	if(desc.mMass!=0.0f)
	{
		MassProps massProps;
		imm_computeMassProps(massProps, geom, desc.mMass);

		id = mScene->createActor(shapeHandle, pose, desc.mCollisionGroup, &massProps, NULL);
	}
	else
	{
		id = mScene->createActor(shapeHandle, pose, desc.mCollisionGroup, NULL, NULL);
	}

/*	if(geom.getType()==PxGeometryType::eCAPSULE)
	{
		const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.0f));
		mScene->mShapeLocalPoses[id].q *= q;
	}*/
	return id;
}
#endif

PxConvexMesh* PhysXImm::CreatePhysXConvex(udword nb_verts, const Point* verts, PxConvexFlags flags)
{
	ASSERT(mCooking);

	PxConvexMeshDesc ConvexDesc;
	ConvexDesc.points.count		= nb_verts;
	ConvexDesc.points.stride	= sizeof(PxVec3);
	ConvexDesc.points.data		= verts;
	ConvexDesc.flags			= flags;

#ifdef IMM_NEEDS_PX_PHYSICS
	MemoryOutputStream buf;
	if(!mCooking->cookConvexMesh(ConvexDesc, buf))
		return null;

	MemoryInputData input(buf.getData(), buf.getSize());
	return mPhysics->createConvexMesh(input);
#else
	return mCooking->createConvexMesh(ConvexDesc, mCooking->getStandaloneInsertionCallback());
#endif
}

PxTriangleMesh* PhysXImm::CreatePhysXMesh(const PintSurfaceInterface& surface, bool deformable)
{
	ASSERT(mCooking);
	ASSERT(!deformable);

	PxTriangleMeshDesc MeshDesc;
	MeshDesc.points.count		= surface.mNbVerts;
	MeshDesc.points.stride		= sizeof(PxVec3);
	MeshDesc.points.data		= surface.mVerts;
	MeshDesc.triangles.count	= surface.mNbFaces;
	MeshDesc.triangles.stride	= sizeof(udword)*3;
	MeshDesc.triangles.data		= surface.mDFaces;
#ifdef IMM_NEEDS_PX_PHYSICS
	MemoryOutputStream buf;
	if(!mCooking->cookTriangleMesh(MeshDesc, buf))
		return null;

	MemoryInputData input(buf.getData(), buf.getSize());
	return mPhysics->createTriangleMesh(input);
#else
	return mCooking->createTriangleMesh(MeshDesc, mCooking->getStandaloneInsertionCallback());
#endif
}

	struct LocalShapeCreationParams
	{
//		const char*			mForcedName;
//		PxRigidActor*		mActor;
//		PintCollisionGroup	mGroup;
//		bool				mIsDynamic;
		udword				mShapeIndex;
		ImmShapeHandle		mShapes[16];
		PxMassProperties	mMassProperties[16];
		PxTransform			mLocalPoses[16];
	};

/*
	LocalShapeCreationParams Params;
	Params.mForcedName	= forced_name;
	Params.mActor		= actor;
	Params.mGroup		= group;
	Params.mIsDynamic	= actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC;

	desc.GetNbShapes(this, &Params);
*/

void PhysXImm::ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)
{
	ASSERT(user_data);
	LocalShapeCreationParams* Params = reinterpret_cast<LocalShapeCreationParams*>(user_data);

	udword ShapeIndex = Params->mShapeIndex;

	PxTransform LocalPose(ToPxVec3(create.mLocalPos), ToPxQuat(create.mLocalRot));

/*	PxMaterial* ShapeMaterial = mDefaultMaterial;
	if(create.mMaterial)
	{
		ShapeMaterial = CreateMaterial(*create.mMaterial);
		ASSERT(ShapeMaterial);
	}

	PxShape* shape = null;*/
	if(create.mType==PINT_SHAPE_SPHERE)
	{
		const PINT_SPHERE_CREATE* SphereCreate = static_cast<const PINT_SPHERE_CREATE*>(&create);

		const PxSphereGeometry sphereGeom(SphereCreate->mRadius);
		//h = CreateActor(desc, sphereGeom, pose, CurrentShape);

		Params->mLocalPoses[ShapeIndex] = LocalPose;
		Params->mMassProperties[ShapeIndex] = PxMassProperties(sphereGeom);
		Params->mShapes[ShapeIndex++] = mScene->createShape(sphereGeom, LocalPose, create.mRenderer);
	}
	else if(create.mType==PINT_SHAPE_BOX)
	{
		const PINT_BOX_CREATE* BoxCreate = static_cast<const PINT_BOX_CREATE*>(&create);
//			shape = CreateBoxShape(CurrentShape, actor, PxBoxGeometry(ToPxVec3(BoxCreate->mExtents)), *ShapeMaterial, LocalPose, desc.mCollisionGroup);
//			ASSERT(0);

		const PxBoxGeometry BoxGeom(ToPxVec3(BoxCreate->mExtents));
//			h = CreateActor(desc, BoxGeom, pose, CurrentShape);
		Params->mLocalPoses[ShapeIndex] = LocalPose;
		Params->mMassProperties[ShapeIndex] = PxMassProperties(BoxGeom);
		Params->mShapes[ShapeIndex++] = mScene->createShape(BoxGeom, LocalPose, create.mRenderer);
	}
	else if(create.mType==PINT_SHAPE_CAPSULE)
	{
		const PINT_CAPSULE_CREATE* CapsuleCreate = static_cast<const PINT_CAPSULE_CREATE*>(&create);

		const PxCapsuleGeometry CapsuleGeom(CapsuleCreate->mRadius, CapsuleCreate->mHalfHeight);
		//h = CreateActor(desc, CapsuleGeom, pose, CurrentShape);

		const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.0f));
		LocalPose.q *= q;

		Params->mLocalPoses[ShapeIndex] = LocalPose;
		Params->mMassProperties[ShapeIndex] = PxMassProperties(CapsuleGeom);
		Params->mShapes[ShapeIndex++] = mScene->createShape(CapsuleGeom, LocalPose, create.mRenderer);
	}
	else if(create.mType==PINT_SHAPE_CONVEX)
	{
		const PINT_CONVEX_CREATE* ConvexCreate = static_cast<const PINT_CONVEX_CREATE*>(&create);

		const bool share_meshes = true;
		PxConvexMesh* ConvexMesh = CreateConvexMesh(ConvexCreate->mVerts, ConvexCreate->mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX, create.mRenderer, share_meshes);

		const PxConvexMeshGeometry ConvexGeom(ConvexMesh);

		Params->mLocalPoses[ShapeIndex] = LocalPose;
		Params->mMassProperties[ShapeIndex] = PxMassProperties(ConvexGeom);
		Params->mShapes[ShapeIndex++] = mScene->createShape(ConvexGeom, LocalPose, create.mRenderer);
	}
	else if(create.mType==PINT_SHAPE_MESH)
	{
		const PINT_MESH_CREATE* MeshCreate = static_cast<const PINT_MESH_CREATE*>(&create);

		const bool share_meshes = true;
		const bool dynamic = false;
		PxTriangleMesh* TriangleMesh = CreateTriangleMesh(MeshCreate->GetSurface(), create.mRenderer, MeshCreate->mDeformable, share_meshes, dynamic);

		const PxTriangleMeshGeometry MeshGeom(TriangleMesh);

		Params->mLocalPoses[ShapeIndex] = LocalPose;
		Params->mMassProperties[ShapeIndex] = PxMassProperties(MeshGeom);
		Params->mShapes[ShapeIndex++] = mScene->createShape(MeshGeom, LocalPose, create.mRenderer);
	}
	else ASSERT(0);

	Params->mShapeIndex = ShapeIndex;
}

PintActorHandle PhysXImm::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	const udword NbShapes = desc.GetNbShapes();
	if(!NbShapes)
		return null;

	ASSERT(mScene);

	const PxTransform pose(ToPxVec3(desc.mPosition), ToPxQuat(desc.mRotation));
	ASSERT(pose.isValid());


		LocalShapeCreationParams Params;
		Params.mShapeIndex = 0;

		desc.GetNbShapes(this, &Params);

#ifdef REMOVED

	udword ShapeIndex = 0;
	ImmShapeHandle Shapes[16];
	PxMassProperties MassProperties[16];
	PxTransform LocalPoses[16];

	const PINT_SHAPE_CREATE* CurrentShape = desc.mShapes;
	ASSERT(CurrentShape);
	while(CurrentShape)
	{
		PX_ASSERT(ShapeIndex<16);

		PxTransform LocalPose(ToPxVec3(CurrentShape->mLocalPos), ToPxQuat(CurrentShape->mLocalRot));

/*		PxMaterial* ShapeMaterial = mDefaultMaterial;
		if(CurrentShape->mMaterial)
		{
			ShapeMaterial = CreateMaterial(*CurrentShape->mMaterial);
			ASSERT(ShapeMaterial);
		}

		PxShape* shape = null;*/
		if(CurrentShape->mType==PINT_SHAPE_SPHERE)
		{
			const PINT_SPHERE_CREATE* SphereCreate = static_cast<const PINT_SPHERE_CREATE*>(CurrentShape);

			const PxSphereGeometry sphereGeom(SphereCreate->mRadius);
			//h = CreateActor(desc, sphereGeom, pose, CurrentShape);

			LocalPoses[ShapeIndex] = LocalPose;
			MassProperties[ShapeIndex] = PxMassProperties(sphereGeom);
			Shapes[ShapeIndex++] = mScene->createShape(sphereGeom, LocalPose, CurrentShape->mRenderer);
		}
		else if(CurrentShape->mType==PINT_SHAPE_BOX)
		{
			const PINT_BOX_CREATE* BoxCreate = static_cast<const PINT_BOX_CREATE*>(CurrentShape);
//			shape = CreateBoxShape(CurrentShape, actor, PxBoxGeometry(ToPxVec3(BoxCreate->mExtents)), *ShapeMaterial, LocalPose, desc.mCollisionGroup);
//			ASSERT(0);

			const PxBoxGeometry BoxGeom(ToPxVec3(BoxCreate->mExtents));
//			h = CreateActor(desc, BoxGeom, pose, CurrentShape);
			LocalPoses[ShapeIndex] = LocalPose;
			MassProperties[ShapeIndex] = PxMassProperties(BoxGeom);
			Shapes[ShapeIndex++] = mScene->createShape(BoxGeom, LocalPose, CurrentShape->mRenderer);
		}
		else if(CurrentShape->mType==PINT_SHAPE_CAPSULE)
		{
			const PINT_CAPSULE_CREATE* CapsuleCreate = static_cast<const PINT_CAPSULE_CREATE*>(CurrentShape);

			const PxCapsuleGeometry CapsuleGeom(CapsuleCreate->mRadius, CapsuleCreate->mHalfHeight);
			//h = CreateActor(desc, CapsuleGeom, pose, CurrentShape);

			const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.0f));
			LocalPose.q *= q;

			LocalPoses[ShapeIndex] = LocalPose;
			MassProperties[ShapeIndex] = PxMassProperties(CapsuleGeom);
			Shapes[ShapeIndex++] = mScene->createShape(CapsuleGeom, LocalPose, CurrentShape->mRenderer);
		}
		else if(CurrentShape->mType==PINT_SHAPE_CONVEX)
		{
			const PINT_CONVEX_CREATE* ConvexCreate = static_cast<const PINT_CONVEX_CREATE*>(CurrentShape);

			const bool share_meshes = true;
			PxConvexMesh* ConvexMesh = CreateConvexMesh(ConvexCreate->mVerts, ConvexCreate->mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX, CurrentShape->mRenderer, share_meshes);

			const PxConvexMeshGeometry ConvexGeom(ConvexMesh);

			LocalPoses[ShapeIndex] = LocalPose;
			MassProperties[ShapeIndex] = PxMassProperties(ConvexGeom);
			Shapes[ShapeIndex++] = mScene->createShape(ConvexGeom, LocalPose, CurrentShape->mRenderer);
		}
		else if(CurrentShape->mType==PINT_SHAPE_MESH)
		{
			const PINT_MESH_CREATE* MeshCreate = static_cast<const PINT_MESH_CREATE*>(CurrentShape);

			const bool share_meshes = true;
			PxTriangleMesh* TriangleMesh = CreateTriangleMesh(MeshCreate->mSurface, CurrentShape->mRenderer, MeshCreate->mDeformable, share_meshes);

			const PxTriangleMeshGeometry MeshGeom(TriangleMesh);

			LocalPoses[ShapeIndex] = LocalPose;
			MassProperties[ShapeIndex] = PxMassProperties(MeshGeom);
			Shapes[ShapeIndex++] = mScene->createShape(MeshGeom, LocalPose, CurrentShape->mRenderer);
		}
		else ASSERT(0);

		CurrentShape = CurrentShape->mNext;
		//ASSERT(!CurrentShape);
	}
#endif
	PX_ASSERT(Params.mShapeIndex==NbShapes);

	ImmActorHandle h;
	if(desc.mMass!=0.0f)
	{
//		MassProps massProps;
//		imm_computeMassProps(massProps, geom, desc.mMass);

		PxMassProperties inertia = PxMassProperties::sum(Params.mMassProperties, Params.mLocalPoses, NbShapes);

		inertia = inertia * (desc.mMass/inertia.mass);

		PxQuat orient;
		const PxVec3 diagInertia = PxMassProperties::getMassSpaceInertia(inertia.inertiaTensor, orient);
//		body->setMass(inertia.mass);
//		body->setCMassLocalPose(PxTransform(inertia.centerOfMass, orient));
//		body->setMassSpaceInertiaTensor(diagInertia);

		MassProps props;
		props.mInvMass = 1.0f/inertia.mass;
		props.mInvInertia.x = diagInertia.x == 0.0f ? 0.0f : 1.0f/diagInertia.x;
		props.mInvInertia.y = diagInertia.y == 0.0f ? 0.0f : 1.0f/diagInertia.y;
		props.mInvInertia.z = diagInertia.z == 0.0f ? 0.0f : 1.0f/diagInertia.z;

//props.mInvMass = 1.0f;
//props.mInvInertia = PxVec3(1.0f);


		h = mScene->createActor(NbShapes, Params.mShapes, pose, desc.mCollisionGroup, &props, NULL);
	}
	else
	{
		h = mScene->createActor(NbShapes, Params.mShapes, pose, desc.mCollisionGroup, NULL, NULL);
	}

	SetupActor(desc, mScene, h);

	return ImmHandleToPintHandle(h);
}

bool PhysXImm::ReleaseObject(PintActorHandle handle)
{
	return false;
}

PintJointHandle PhysXImm::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	const ImmActorHandle id0 = PintHandleToImmHandle(desc.mObject0);
	const ImmActorHandle id1 = PintHandleToImmHandle(desc.mObject1);

	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			const PINT_SPHERICAL_JOINT_CREATE& jc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);
			mScene->createSphericalJoint(	id0, id1,
											PxTransform(ToPxVec3(jc.mLocalPivot0.mPos)), PxTransform(ToPxVec3(jc.mLocalPivot1.mPos)),
											&mScene->getActorGlobalPose(id0), &mScene->getActorGlobalPose(id1));
		}
		break;
	}
	return PintJointHandle(42);
}

PintArticHandle PhysXImm::CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE& create)
{
	Dy::ArticulationV* immArt = mScene->createArticulation(create.mFixBase);
	//Dy::ArticulationV* immArt = createImmediateArticulation(create.mFixBase, mScene->mArticulations);
	mScene->addArticulationToScene(immArt);
	return PintArticHandle(immArt);
}

bool PhysXImm::AddRCArticulationToScene(PintArticHandle articulation)
{
	Dy::ArticulationV* immArt = reinterpret_cast<Dy::ArticulationV*>(articulation);
//	mScene->addArticulationToScene(immArt);
	return true;
}

PintActorHandle PhysXImm::CreateRCArticulatedObject(const PINT_OBJECT_CREATE& oc, const PINT_RC_ARTICULATED_BODY_CREATE& bc, PintArticHandle articulation)
{
	Dy::ArticulationV* immArt = reinterpret_cast<Dy::ArticulationV*>(articulation);

	const udword NbShapes = oc.GetNbShapes();
	if(!NbShapes)
		return null;

	ASSERT(0);

	//
#ifdef REMOVED
	udword ShapeIndex = 0;
	ImmShapeHandle Shapes[16];
	PxMassProperties MassProperties[16];
	PxTransform LocalPoses[16];

	//

	const PINT_SHAPE_CREATE* CurrentShape = oc.mShapes;
	ASSERT(CurrentShape);
	while(CurrentShape)
	{
		PX_ASSERT(ShapeIndex<16);

		PxTransform LocalPose(ToPxVec3(CurrentShape->mLocalPos), ToPxQuat(CurrentShape->mLocalRot));

/*
		PxMaterial* ShapeMaterial = mDefaultMaterial;
		if(CurrentShape->mMaterial)
		{
			ShapeMaterial = CreateMaterial(*CurrentShape->mMaterial);
			ASSERT(ShapeMaterial);
		}

		PxShape* shape = null;*/
		if(CurrentShape->mType==PINT_SHAPE_SPHERE)
		{
			const PINT_SPHERE_CREATE* SphereCreate = static_cast<const PINT_SPHERE_CREATE*>(CurrentShape);

			const PxSphereGeometry sphereGeom(SphereCreate->mRadius);
			//h = CreateActor(desc, sphereGeom, pose, CurrentShape);

			LocalPoses[ShapeIndex] = LocalPose;
			MassProperties[ShapeIndex] = PxMassProperties(sphereGeom);
			Shapes[ShapeIndex++] = mScene->createShape(sphereGeom, LocalPose, CurrentShape->mRenderer);
		}
		else if(CurrentShape->mType==PINT_SHAPE_BOX)
		{
			const PINT_BOX_CREATE* BoxCreate = static_cast<const PINT_BOX_CREATE*>(CurrentShape);
//			shape = CreateBoxShape(CurrentShape, actor, PxBoxGeometry(ToPxVec3(BoxCreate->mExtents)), *ShapeMaterial, LocalPose, desc.mCollisionGroup);
//			ASSERT(0);

			const PxBoxGeometry boxGeom(ToPxVec3(BoxCreate->mExtents));
//			h = CreateActor(desc, boxGeom, pose, CurrentShape);
			LocalPoses[ShapeIndex] = LocalPose;
			MassProperties[ShapeIndex] = PxMassProperties(boxGeom);
			Shapes[ShapeIndex++] = mScene->createShape(boxGeom, LocalPose, CurrentShape->mRenderer);
		}
		else if(CurrentShape->mType==PINT_SHAPE_CAPSULE)
		{
			const PINT_CAPSULE_CREATE* CapsuleCreate = static_cast<const PINT_CAPSULE_CREATE*>(CurrentShape);

			const PxCapsuleGeometry capsuleGeom(CapsuleCreate->mRadius, CapsuleCreate->mHalfHeight);
			//h = CreateActor(desc, capsuleGeom, pose, CurrentShape);

			const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.0f));
			LocalPose.q *= q;

			LocalPoses[ShapeIndex] = LocalPose;
			MassProperties[ShapeIndex] = PxMassProperties(capsuleGeom);
			Shapes[ShapeIndex++] = mScene->createShape(capsuleGeom, LocalPose, CurrentShape->mRenderer);
		}
		else if(CurrentShape->mType==PINT_SHAPE_CONVEX)
		{
			const PINT_CONVEX_CREATE* ConvexCreate = static_cast<const PINT_CONVEX_CREATE*>(CurrentShape);
			ASSERT(0);
		}
		else if(CurrentShape->mType==PINT_SHAPE_MESH)
		{
			const PINT_MESH_CREATE* MeshCreate = static_cast<const PINT_MESH_CREATE*>(CurrentShape);
			ASSERT(0);
		}
		else ASSERT(0);

		CurrentShape = CurrentShape->mNext;
//		ASSERT(!CurrentShape);
	}

	PX_ASSERT(ShapeIndex==NbShapes);

			ASSERT(oc.mMass!=0.0f);

//			MassProps massProps;
//			imm_computeMassProps(massProps, boxGeom, oc.mMass);

		MassProps props;
		{
		PxMassProperties inertia = PxMassProperties::sum(MassProperties, LocalPoses, NbShapes);

		inertia = inertia * (oc.mMass/inertia.mass);

		PxQuat orient;
		const PxVec3 diagInertia = PxMassProperties::getMassSpaceInertia(inertia.inertiaTensor, orient);
//		body->setMass(inertia.mass);
//		body->setCMassLocalPose(PxTransform(inertia.centerOfMass, orient));
//		body->setMassSpaceInertiaTensor(diagInertia);

		props.mInvMass = 1.0f/inertia.mass;
		props.mInvInertia.x = diagInertia.x == 0.0f ? 0.0f : 1.0f/diagInertia.x;
		props.mInvInertia.y = diagInertia.y == 0.0f ? 0.0f : 1.0f/diagInertia.y;
		props.mInvInertia.z = diagInertia.z == 0.0f ? 0.0f : 1.0f/diagInertia.z;
		}
//props.mInvMass = 1.0f;
//props.mInvInertia = PxVec3(1.0f);


//		h = mScene->createActor(NbShapes, Shapes, pose, desc.mCollisionGroup, &props, NULL);


			Dy::ArticulationLinkHandle parent;
			ImmActorHandle parentID;
			if(bc.mParent)
			{
				parentID = PintHandleToImmHandle(bc.mParent);
				parent = mScene->getActor(parentID).mLink;
			}
			else
			{
				parentID = ImmActorHandle(0xffffffff);
				parent = null;
			}

	const PxTransform pose(ToPxVec3(oc.mPosition), ToPxQuat(oc.mRotation));
	ASSERT(pose.isValid());

	PxFeatherstoneArticulationLinkData linkData;
//			setupCommonLinkData(linkData, parent, pose, massProps);
			setupCommonLinkData(linkData, parent, pose, props);

			setupJointLinkData(linkData, bc);

			Dy::ArticulationLinkHandle link = PxAddArticulationLink(immArt, linkData);
//			ImmActorHandle linkID = mScene->createActor(shapeHandle, pose, oc.mCollisionGroup, &massProps, link);
			ImmActorHandle linkID = mScene->createActor(NbShapes, Shapes, pose, oc.mCollisionGroup, &props, link);

			if(parentID!=ImmActorHandle(0xffffffff))
				mScene->disableCollision(parentID, linkID);	//###TODO: revisit this

	SetupActor(oc, mScene, linkID);

	return ImmHandleToPintHandle(linkID);
#endif
	return null;
}

static PX_FORCE_INLINE ImmediateActor& getActor(PintActorHandle handle)
{
	return gScene->getActor(PintHandleToImmHandle(handle));
}

PR PhysXImm::GetWorldTransform(PintActorHandle handle)
{
	ASSERT(handle);
	const ImmActorHandle id = PintHandleToImmHandle(handle);
	return ToPR(mScene->getActorGlobalPose(id));
}

void PhysXImm::SetWorldTransform(PintActorHandle handle, const PR& pose)
{
	ASSERT(handle);
	const ImmActorHandle id = PintHandleToImmHandle(handle);
	mScene->setActorGlobalPose(id, ToPxTransform(pose));
}

void PhysXImm::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
	ASSERT(handle);
	const ImmActorHandle id = PintHandleToImmHandle(handle);
	ImmediateActor& actor = getActor(handle);
	//PX_ASSERT(0);
	if(actor.mLink)
	{
		Dy::ArticulationV* articulation = PxGetLinkArticulation(actor.mLink);
		PxU32 linkIndex = PxGetLinkIndex(actor.mLink);

		PxArticulationCache* cache = PxCreateArticulationCache(articulation);

#ifdef PHYSX_NEW_PUBLIC_API
		PxCopyInternalStateToArticulationCache(articulation, *cache, PxArticulationCacheFlag::eALL);
		PxApplyArticulationCache(articulation, *cache, PxArticulationCacheFlag::eALL);
#else
		PxCopyInternalStateToArticulationCache(articulation, *cache, PxArticulationCache::eALL);
		PxApplyArticulationCache(articulation, *cache, PxArticulationCache::eALL);
#endif

		PxReleaseArticulationCache(*cache);
	}
	else
		actor.mExternalForce = ToPxVec3(world_impulse*20.0f);
//	printf("%f %f %f\n", world_impulse.x, world_impulse.y, world_impulse.z);
}

bool PhysXImmActor::GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space) const
{
	ASSERT(handle);
	linear_velocity = ToPoint(getActor(handle).mLinearVelocity);
	return true;
}

bool PhysXImmActor::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)
{
	ASSERT(handle);
	getActor(handle).mLinearVelocity = ToPxVec3(linear_velocity);
	return true;
}

bool PhysXImmActor::GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space) const
{
	ASSERT(handle);
	angular_velocity = ToPoint(getActor(handle).mAngularVelocity);
	return true;
}

bool PhysXImmActor::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)
{
	ASSERT(handle);
	getActor(handle).mAngularVelocity = ToPxVec3(angular_velocity);
	return true;
}

float PhysXImmActor::GetMass(PintActorHandle handle) const
{
	ASSERT(handle);
	return 1.0f/getActor(handle).mMassProps.mInvMass;
}

udword PhysXImm::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	udword NbHits = 0;
	while(nb--)
	{
		ImmRaycastHit hit;
		if(mScene->raycastClosest(ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, hit))
		{
			dest->mTouchedActor = ImmHandleToPintHandle(hit.mTouchedActor);
			dest->mTouchedShape = null;//ImmHandleToPintHandle(hit.mTouchedShape);
			dest->mDistance = hit.mDistance;
			dest->mImpact = ToPoint(hit.mPos);
			dest->mNormal = ToPoint(hit.mNormal);
			dest->mTriangleIndex = INVALID_ID;
			NbHits++;
		}
		else
		{
			dest->SetNoHit();
		}

		dest++;
		raycasts++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

static Widgets*		gImmGUI = null;
/*static IceEditBox*	gEditBox_SolverIter = null;
static IceEditBox*	gEditBox_DefaultFriction = null;
static IceEditBox*	gEditBox_LinearDamping = null;
static IceEditBox*	gEditBox_AngularDamping = null;
static IceEditBox*	gEditBox_Erp = null;
static IceEditBox*	gEditBox_Erp2 = null;
static IceEditBox*	gEditBox_Tau = null;
static IceEditBox*	gEditBox_CollisionMargin = null;
static IceComboBox*	gComboBox_Gyro = null;*/
static IceCheckBox*	gCheckBox_DebugVis[NB_DEBUG_VIZ_PARAMS] = {0};

enum ImmGUIElement
{
	IMM_GUI_MAIN,
	//
	IMM_GUI_ENABLE_PERSISTENT_CONTACTS,
	IMM_GUI_ENABLE_BATCHED_CONTACTS,
	IMM_GUI_ENABLE_TGS,
	//
	IMM_GUI_ENABLE_DEBUG_VIZ,	// MUST BE LAST
};

static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
{
	const udword id = check_box.GetID();
	switch(id)
	{
		case IMM_GUI_ENABLE_PERSISTENT_CONTACTS:
			gUsePersistentContacts = checked;
			break;

		case IMM_GUI_ENABLE_BATCHED_CONTACTS:
			gBatchContacts = checked;
			break;		

		case IMM_GUI_ENABLE_TGS:
			gUseTGS = checked;
			break;		

		case IMM_GUI_ENABLE_DEBUG_VIZ:
		{
			gDebugVizParams[0] = checked;
			for(udword i=1;i<NB_DEBUG_VIZ_PARAMS;i++)
			{
				gCheckBox_DebugVis[i]->SetEnabled(checked);
			}
		}
		break;
	}

	if(id>IMM_GUI_ENABLE_DEBUG_VIZ && id<IMM_GUI_ENABLE_DEBUG_VIZ+NB_DEBUG_VIZ_PARAMS)
	{
		gDebugVizParams[id-IMM_GUI_ENABLE_DEBUG_VIZ] = checked;
	}
}

/*static void gBullet_GetOptionsFromGUI()
{
	if(gEditBox_SolverIter)
	{
		sdword tmp;
		bool status = gEditBox_SolverIter->GetTextAsInt(tmp);
		ASSERT(status);
		ASSERT(tmp>=0);
		gSolverIterationCount = udword(tmp);
	}

	if(gEditBox_DefaultFriction)
	{
		float tmp;
		bool status = gEditBox_DefaultFriction->GetTextAsFloat(tmp);
		ASSERT(status);
		ASSERT(tmp>=0.0f);
		gDefaultFriction = tmp;
	}

	if(gEditBox_LinearDamping)
	{
		float tmp;
		bool status = gEditBox_LinearDamping->GetTextAsFloat(tmp);
		ASSERT(status);
		ASSERT(tmp>=0.0f);
		gLinearDamping = tmp;
	}

	if(gEditBox_AngularDamping)
	{
		float tmp;
		bool status = gEditBox_AngularDamping->GetTextAsFloat(tmp);
		ASSERT(status);
		ASSERT(tmp>=0.0f);
		gAngularDamping = tmp;
	}

	if(gEditBox_Erp)
	{
		float tmp;
		bool status = gEditBox_Erp->GetTextAsFloat(tmp);
		ASSERT(status);
		gErp = tmp;
	}

	if(gEditBox_Erp2)
	{
		float tmp;
		bool status = gEditBox_Erp2->GetTextAsFloat(tmp);
		ASSERT(status);
		gErp2 = tmp;
	}

	if(gEditBox_Tau)
	{
		float tmp;
		bool status = gEditBox_Tau->GetTextAsFloat(tmp);
		ASSERT(status);
		gTau = tmp;
	}

	if(gEditBox_CollisionMargin)
	{
		float tmp;
		bool status = gEditBox_CollisionMargin->GetTextAsFloat(tmp);
		ASSERT(status);
		ASSERT(tmp>=0.0f);
		gCollisionMargin = tmp;
	}

	if(gComboBox_Gyro)
	{
		gGyroscopicIndex = gComboBox_Gyro->GetSelectedIndex();
	}

}*/

IceWindow* PhysXImm_InitGUI(IceWidget* parent, PintGUIHelper& helper)
{
	IceWindow* Main = helper.CreateMainWindow(gImmGUI, parent, IMM_GUI_MAIN, "PhysX ImmMode options");

	const sdword YStep = 20;
	const sdword YStepCB = 16;
	sdword y = 4;

	{
		const udword CheckBoxWidth = 200;

		helper.CreateCheckBox(Main, IMM_GUI_ENABLE_PERSISTENT_CONTACTS, 4, y, CheckBoxWidth, 20, "Persistent contacts", gImmGUI, gUsePersistentContacts, gCheckBoxCallback);
		y += YStepCB;

		helper.CreateCheckBox(Main, IMM_GUI_ENABLE_BATCHED_CONTACTS, 4, y, CheckBoxWidth, 20, "Batch contacts", gImmGUI, gBatchContacts, gCheckBoxCallback);
		y += YStepCB;

		helper.CreateCheckBox(Main, IMM_GUI_ENABLE_TGS, 4, y, CheckBoxWidth, 20, "TGS", gImmGUI, gUseTGS, gCheckBoxCallback);
		y += YStepCB;
	}
	Common_CreateDebugVizUI(Main, 290, 20, gCheckBoxCallback, IMM_GUI_ENABLE_DEBUG_VIZ, NB_DEBUG_VIZ_PARAMS, gDebugVizParams, gDebugVizNames, gCheckBox_DebugVis, gImmGUI);

	y += YStep;

/*	const sdword OffsetX = 90;
	const sdword LabelWidth = 90;
	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword ColumnOffsetX = 200;
	{
		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelWidth, 20, "Num solver iter:", gBulletGUI);
		gEditBox_SolverIter = helper.CreateEditBox(Main, BULLET_GUI_SOLVER_ITER, 4+OffsetX, y, EditBoxWidth, 20, _F("%d", gSolverIterationCount), gBulletGUI, EDITBOX_INTEGER_POSITIVE, null);

			helper.CreateLabel(Main, 4+ColumnOffsetX, y+LabelOffsetY, LabelWidth, 20, "Default friction:", gBulletGUI);
			gEditBox_DefaultFriction = helper.CreateEditBox(Main, BULLET_GUI_DEFAULT_FRICTION, 4+OffsetX+ColumnOffsetX, y, EditBoxWidth, 20, helper.Convert(gDefaultFriction), gBulletGUI, EDITBOX_FLOAT_POSITIVE, null);

		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelWidth, 20, "Linear damping:", gBulletGUI);
		gEditBox_LinearDamping = helper.CreateEditBox(Main, BULLET_GUI_LINEAR_DAMPING, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gLinearDamping), gBulletGUI, EDITBOX_FLOAT_POSITIVE, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelWidth, 20, "Angular damping:", gBulletGUI);
		gEditBox_AngularDamping = helper.CreateEditBox(Main, BULLET_GUI_ANGULAR_DAMPING, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gAngularDamping), gBulletGUI, EDITBOX_FLOAT_POSITIVE, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelWidth, 20, "ERP:", gBulletGUI);
		gEditBox_Erp = helper.CreateEditBox(Main, BULLET_GUI_ERP, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gErp), gBulletGUI, EDITBOX_FLOAT, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelWidth, 20, "ERP2:", gBulletGUI);
		gEditBox_Erp2 = helper.CreateEditBox(Main, BULLET_GUI_ERP2, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gErp2), gBulletGUI, EDITBOX_FLOAT, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelWidth, 20, "Tau:", gBulletGUI);
		gEditBox_Tau = helper.CreateEditBox(Main, BULLET_GUI_TAU, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gTau), gBulletGUI, EDITBOX_FLOAT, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelWidth, 20, "Collision margin:", gBulletGUI);
		gEditBox_CollisionMargin = helper.CreateEditBox(Main, BULLET_GUI_COLLISION_MARGIN, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gCollisionMargin), gBulletGUI, EDITBOX_FLOAT_POSITIVE, null);
		y += YStep;

		{
			ComboBoxDesc CBBD;
			CBBD.mParent	= Main;
			CBBD.mX			= 4+OffsetX;
			CBBD.mWidth		= 300;
			CBBD.mHeight	= 20;

			helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelWidth, 20, "Gyroscopic force:", gBulletGUI);
			CBBD.mID		= BULLET_GUI_GYRO;
			CBBD.mY			= y;
			CBBD.mLabel		= "Gyroscopic";
			gComboBox_Gyro = ICE_NEW(IceComboBox)(CBBD);
			gBulletGUI->Register(gComboBox_Gyro);
			gComboBox_Gyro->Add("Disabled");
			gComboBox_Gyro->Add("BT_ENABLE_GYROSCOPIC_FORCE_EXPLICIT");
			gComboBox_Gyro->Add("BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_WORLD");
			gComboBox_Gyro->Add("BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY");
			gComboBox_Gyro->Select(0);
			gComboBox_Gyro->SetVisible(true);
			y += YStep;
		}
	}

	y += YStep;*/

	return Main;
}

void PhysXImm_CloseGUI()
{
	Common_CloseGUI(gImmGUI);

/*	gEditBox_SolverIter = null;
	gEditBox_DefaultFriction = null;
	gEditBox_LinearDamping = null;
	gEditBox_AngularDamping = null;
	gEditBox_Erp = null;
	gEditBox_Erp2 = null;
	gEditBox_Tau = null;
	gEditBox_CollisionMargin = null;
	gComboBox_Gyro = null;*/
	for(udword i=0;i<NB_DEBUG_VIZ_PARAMS;i++)
		gCheckBox_DebugVis[i] = null;
}

///////////////////////////////////////////////////////////////////////////////

