///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// A work-in-progress PEEL plugin for MuJoCo

#include "PINT_MuJoCo332.h"

#include "..\PINT_Common\PINT_Ice.h"
#include "..\PINT_Common\PINT_Common.h"
#include "..\PintShapeRenderer.h"
#include "foundation\PxTransform.h"
#include "foundation\PxMat33.h"
#include <Core/RockAllocator.h>

#ifdef _WIN64
	#pragma comment(lib, "../../Ice/Lib64/IceCore64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceMaths64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Contact64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Meshmerizer64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceImageWork64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceGUI64.lib")
//	#pragma comment(lib, "../../Ice/Lib64/IML64.lib")

	//#pragma comment(lib, "../../../../PEEL_Externals/MuJoCo_3_3_2/lib/Debug/mujoco.lib")
	#pragma comment(lib, "../../../../PEEL_Externals/MuJoCo_3_3_2/lib/Release/mujoco.lib")
#else
	#pragma comment(lib, "../../Ice/Lib/IceCore.lib")
	#pragma comment(lib, "../../Ice/Lib/IceMaths.lib")
	#pragma comment(lib, "../../Ice/Lib/Contact.lib")
	#pragma comment(lib, "../../Ice/Lib/Meshmerizer.lib")
	#pragma comment(lib, "../../Ice/Lib/IceImageWork.lib")
	#pragma comment(lib, "../../Ice/Lib/IceGUI.lib")
	//#pragma comment(lib, "../../Ice/Lib/IML.lib")
#endif

using namespace physx;
using namespace Rock;

///////////////////////////////////////////////////////////////////////////////

static udword	gSolver				= mjSOL_NEWTON;
static udword	gCone				= mjCONE_PYRAMIDAL;
static udword	gJacobian			= mjJAC_AUTO;
static udword	gIntegrator			= mjINT_IMPLICIT;//mjINT_EULER;

static udword	gIterations			= 100;
static udword	gLSIterations		= 50;
static udword	gNoSlipIterations	= 0;
static udword	gCCDIterations		= 50;
static udword	gSubsteps			= 1;
static udword	gNbThreads			= 0;

static float	gFriction			= 0.5f;
static float	gFrictionLoss		= 0.0f;
static float	gDamping			= 0.0f;
static float	gSolRef0			= 0.02f;
static float	gSolRef1			= 1.0f;
static float	gImpRatio			= 1.0f;
static float	gMargin				= 0.0f;
static float	gGap				= 0.0f;
static float	gKv					= 1.0f;
static float	gKp					= 1.0f;

static bool		gPlaneGeomAsGround	= true;
static bool		gMultiCCD			= true;
static bool		gDebugDraw_Contacts	= false;
static bool		gDebugDraw_AABBs	= false;
static bool		gDebugDraw_BodyAxes	= false;
static bool		gDebugDraw_Inertia	= false;

///////////////////////////////////////////////////////////////////////////////

// ICE <=> MuJoCo conversions

static inline_ Point	ToPoint(const mjtNum* p)	{ return Point(float(p[0]), float(p[1]), float(p[2]));	}
static inline_ Quat		ToQuat(const mjtNum* q)		{ return Quat(float(q[0]), float(q[1]), float(q[2]), float(q[3]));	}

static inline_ void		ToMjtNumVec(mjtNum* dst, const Point& v)
{
	dst[0] = mjtNum(v.x);
	dst[1] = mjtNum(v.y);
	dst[2] = mjtNum(v.z);
}

static inline_ void		ToMjtNumQuat(mjtNum* dst, const Quat& q)
{
	dst[0] = mjtNum(q.w);
	dst[1] = mjtNum(q.p.x);
	dst[2] = mjtNum(q.p.y);
	dst[3] = mjtNum(q.p.z);
}

///////////////////////////////////////////////////////////////////////////////

// PhysX <=> MuJoCo conversions

static inline_ PxVec3	ToPxVec3(const mjtNum* p)	{ return PxVec3(float(p[0]), float(p[1]), float(p[2]));	}
static inline_ PxQuat	ToPxQuat(const mjtNum* q)	{ return PxQuat(float(q[1]), float(q[2]), float(q[3]), float(q[0]));	}

static inline_ void		ToMjtNumVec(mjtNum* dst, const PxVec3& v)
{
	dst[0] = mjtNum(v.x);
	dst[1] = mjtNum(v.y);
	dst[2] = mjtNum(v.z);
}

static inline_ void		ToMjtNumQuat(mjtNum* dst, const PxQuat& q)
{
	dst[0] = mjtNum(q.w);
	dst[1] = mjtNum(q.x);
	dst[2] = mjtNum(q.y);
	dst[3] = mjtNum(q.z);
}

///////////////////////////////////////////////////////////////////////////////

// ICE <=> PhysX conversions

static inline_ const Point&		ToPoint(const PxVec3& p)	{ return (const Point&)p;	}
static inline_ const PxVec3&	ToPxVec3(const Point& p)	{ return (const PxVec3&)p;	}

ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.x)==OFFSET_OF(PxQuat, x));
ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.y)==OFFSET_OF(PxQuat, y));
ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.z)==OFFSET_OF(PxQuat, z));
ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, w)==OFFSET_OF(PxQuat, w));
static inline_ const Quat&		ToQuat(const PxQuat& q)		{ return (const Quat&)q;	}
static inline_ const PxQuat&	ToPxQuat(const Quat& q)		{ return (const PxQuat&)q;	}

static inline_	const PxTransform	ToPxTransform(const PR& pose)	{ return PxTransform(ToPxVec3(pose.mPos), ToPxQuat(pose.mRot));	}
static inline_	const PR			ToPR(const PxTransform& pose)	{ return PR(ToPoint(pose.p), ToQuat(pose.q));	}

///////////////////////////////////////////////////////////////////////////////

// 0 reserved to mark null pointers / failures
static inline_ PintActorHandle bodyIDToPintActor(udword body_id)	{ return PintActorHandle(size_t(body_id + 1));	}
static inline_ udword pintActorToBodyID(PintActorHandle handle)		{ return udword(size_t(handle)) - 1;			}
static inline_ PintShapeHandle geomIDToPintShape(udword geom_id)	{ return PintShapeHandle(size_t(geom_id + 1));	}
static inline_ udword pintShapeToGeomID(PintShapeHandle handle)		{ return udword(size_t(handle)) - 1;			}
static inline_ PintArticHandle bodyIDToPintArtic(udword body_id)	{ return PintArticHandle(size_t(body_id + 1));	}
static inline_ udword pintArticToBodyID(PintArticHandle handle)		{ return udword(size_t(handle)) - 1;			}

///////////////////////////////////////////////////////////////////////////////

// Actor/body world pose:
static inline_ PR getBodyWorldPose(const mjData* data, udword body_id)
{
	const mjtNum* pos = data->xpos + body_id * 3;
	const mjtNum* quat = data->xquat + body_id * 4;

	return PR(ToPoint(pos), ToQuat(quat));
}

// Shape local pose:
static inline_ PR getShapeLocalPose(const mjModel* model, udword geom_id)
{
	const mjtNum* localPos = model->geom_pos + geom_id * 3;
	const mjtNum* localRot = model->geom_quat + geom_id * 4;

	return PR(ToPoint(localPos), ToQuat(localRot));
}

///////////////////////////////////////////////////////////////////////////////

MuJoCo_ActorAPI::MuJoCo_ActorAPI(Pint& pint) : Pint_Actor(pint)
{
}

MuJoCo_ActorAPI::~MuJoCo_ActorAPI()
{
}

const char* MuJoCo_ActorAPI::GetName(PintActorHandle handle) const
{
	MuJoCoPint& pint = static_cast<MuJoCoPint&>(mPint);
	const udword bodyID = pintActorToBodyID(handle);
	return pint.mModel->names + pint.mModel->name_bodyadr[bodyID];
}

bool MuJoCo_ActorAPI::GetWorldBounds(PintActorHandle handle, AABB& bounds) const
{
	// This is only used to draw the selected box corners - return false to disable this

	MuJoCoPint& pint = static_cast<MuJoCoPint&>(mPint);
	const udword bodyID = pintActorToBodyID(handle);

	const udword nbGeoms = pint.mModel->body_geomnum[bodyID];
	const udword geomStart = pint.mModel->body_geomadr[bodyID];

	bounds.SetEmpty();

	const PR BodyWorldPose = getBodyWorldPose(pint.mData, bodyID);

	for(udword i=0; i<nbGeoms; i++)
	{
		const udword geomID = geomStart + i;

		PR ShapeWorldPose = getShapeLocalPose(pint.mModel, geomID);
		ShapeWorldPose *= BodyWorldPose;

		const mjtNum* LocalAABB = pint.mModel->geom_aabb + geomID * 6;
		const Point c = ToPoint(LocalAABB);
		const Point e = ToPoint(LocalAABB + 3);

		AABB LocalBounds;
		LocalBounds.SetCenterExtents(c, e);

		AABB WorldBounds;
		LocalBounds.Rotate(ShapeWorldPose, WorldBounds);

		bounds.Add(WorldBounds);
	}
	return true;
}

float MuJoCo_ActorAPI::GetLinearDamping(PintActorHandle handle) const
{
	return 0.0f;
}

bool MuJoCo_ActorAPI::SetLinearDamping(PintActorHandle handle, float damping)
{
	return false;
}

float MuJoCo_ActorAPI::GetAngularDamping(PintActorHandle handle) const
{
	return 0.0f;
}

bool MuJoCo_ActorAPI::SetAngularDamping(PintActorHandle handle, float damping)
{
	return false;
}

bool MuJoCo_ActorAPI::GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space) const
{
	return false;
}

bool MuJoCo_ActorAPI::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)
{
	return false;
}

float MuJoCo_ActorAPI::GetMass(PintActorHandle handle) const
{
	MuJoCoPint& pint = static_cast<MuJoCoPint&>(mPint);
	const udword bodyID = pintActorToBodyID(handle);
	return float(pint.mModel->body_mass[bodyID]);
}

bool MuJoCo_ActorAPI::GetLocalInertia(PintActorHandle handle, Point& inertia) const
{
	MuJoCoPint& pint = static_cast<MuJoCoPint&>(mPint);
	const udword bodyID = pintActorToBodyID(handle);
	inertia = ToPoint(&pint.mModel->body_inertia[bodyID * 3]);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

MuJoCoPint::MuJoCoPint() :
	mActorAPI		(*this),
	mSpec			(null),
	mModel			(null),
	mData			(null),
	mThreadPool		(null),
	mWorld			(null),
	mBodyID			(INVALID_ID),
	mGeomID			(INVALID_ID),
	mJointID		(INVALID_ID),
	mConvexMeshID	(INVALID_ID)
{
}

MuJoCoPint::~MuJoCoPint()
{
}

const char* MuJoCoPint::GetName() const
{
	return "MuJoCo 3.3.2";
}

const char* MuJoCoPint::GetUIName() const
{
	return "MuJoCo 3.3.2";
}

Point MuJoCoPint::GetMainColor()
{
	return Point(105.0f/255.0f, 140.0f/255.0f, 255.0f/255.0f);
}

void MuJoCoPint::GetCaps(PintCaps& caps) const
{
	caps.mSupportRigidBodySimulation = true;
	caps.mSupportCollisionGroups = true;
	caps.mSupportCompounds = true;
	caps.mSupportCylinders = true;
	caps.mSupportConvexes = true;
	//caps.mSupportMeshes = true;
	caps.mSupportRaycasts = true;
	caps.mSupportRCArticulations = true;
	caps.mSupportSphericalJoints = true;
	caps.mSupportFixedJoints = true;
}

///////////////////////////////////////////////////////////////////////////////

static void MyErrorCallback(const char* msg)
{
	printf("MuJoCo error: %s", msg);
}

///////////////////////////////////////////////////////////////////////////////

static udword	gTotalNbAllocs	= 0;
static udword	gNbAllocs		= 0;
static udword	gCurrentMemory	= 0;

static inline_ int atomicIncrement(volatile int* val)
{
	return (int)InterlockedIncrement((volatile LONG*)val);
}

static inline_ int atomicDecrement(volatile int* val)
{
	return (int)InterlockedDecrement((volatile LONG*)val);
}

static inline_ int atomicAdd(volatile int* val, int delta)
{
	return (int)InterlockedAdd((volatile LONG*)val, delta);
}

namespace
{
	struct Header
	{
		udword	mMagic;
		udword	mSize;
	};
}

ICE_COMPILE_TIME_ASSERT(sizeof(Header)<=64);

static void* MyAlloc(size_t size)
{
	// Doc says "the user allocator must allocate memory aligned on 8-byte boundaries" but this is wrong, that crashes. You need 64-byte boundaries.

	// pad size to multiple of 64
	if((size%64))
		size += 64 - (size%64);

//	return _aligned_malloc(size, 64);

	char* memory = (char*)_aligned_malloc(size + 64, 64);

	Header* H = (Header*)memory;
	H->mMagic	= 0x12345678;
	H->mSize	= udword(size);

	atomicIncrement((int*)&gTotalNbAllocs);
	atomicIncrement((int*)&gNbAllocs);
	atomicAdd((int*)&gCurrentMemory, int(size));

	return memory + 64;
}

static void MyFree(void* ptr)
{
//	if(ptr)
//		_aligned_free(ptr);

	if(!ptr)
		return;

	char* bptr = (char*)ptr;
	Header* H = (Header*)(bptr - 64);
	ASSERT(H->mMagic==0x12345678);
	const udword Size = H->mSize;
	_aligned_free(H);

	atomicDecrement((int*)&gNbAllocs);
	atomicAdd((int*)&gCurrentMemory, -(int)Size);
}

///////////////////////////////////////////////////////////////////////////////

#define PEEL_GROUP_SIZE	32
static bool gFilterCallbackEnabled = false;
static bool gCollisionTable[PEEL_GROUP_SIZE][PEEL_GROUP_SIZE];

static void setGroupCollisionFlag(PxU16 group1, PxU16 group2, bool enable)
{
	ASSERT(group1 < PEEL_GROUP_SIZE);
	ASSERT(group2 < PEEL_GROUP_SIZE);
	gCollisionTable[group1][group2] = enable;
	gCollisionTable[group2][group1] = enable;
}

static MuJoCoPint*	gMuJoCoPint = null;
static int MyContactFilter(const mjModel* m, mjData* d, int geom1, int geom2)
{
	const MuJoCoPint::ShapeData* shapeData = gMuJoCoPint->mShapeData.Begin();

	const udword nbGeoms = gMuJoCoPint->mShapeData.Size();
	ASSERT(udword(geom1)<nbGeoms);
	ASSERT(udword(geom2)<nbGeoms);

	// ### potentially wrong mapping here

	// #### this is ridiculous
	const u32 RenderID1 = geom1 ? IR(m->geom_rgba[geom1*4]) : 0;
	const u32 RenderID2 = geom2 ? IR(m->geom_rgba[geom2*4]) : 0;

	if(!gCollisionTable[shapeData[RenderID1].mGroupID][shapeData[RenderID2].mGroupID])
	//if(!gCollisionTable[shapeData[geom1].mGroupID][shapeData[geom2].mGroupID])
		return 1;

	return 0;
	//return 1;
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class RockAllocatorWrapper : public Rock::Allocator
	{
		public:
						RockAllocatorWrapper()	{}
		virtual			~RockAllocatorWrapper()	{}

		virtual void*	malloc(size_t size, Rock::MemoryType /*type*/)	override
		{
			return _aligned_malloc(size, 16);
		}

		virtual void*	mallocDebug(size_t size, const char* filename, u32 line, const char* class_name, Rock::MemoryType /*type*/, bool /*from_new*/)	override
		{
			return _aligned_malloc(size, 16);
		}

		virtual void	free(void* memory, bool /*from_new*/)	override
		{
			_aligned_free(memory);
		}
	};
	static RockAllocatorWrapper gWrapper;
}

///////////////////////////////////////////////////////////////////////////////

MuJoCoPint::BodyData& MuJoCoPint::RegisterBody(mjsBody* body, udword bodyID, const PR* pose)
{
	// MuJoCo doesn't expose a clear mjsBody <-> body ID mapping so I guess we need to do that ourselves
	BodyData& bodyData = mBodyData.Insert();
	bodyData.Init();

	bodyData.mBody		= body;
	//bodyData.mBodyID	= bodyID;
	if(pose)
		bodyData.mInitialPoseWorldSpace = *pose;

	return bodyData;
}

void MuJoCoPint::Init(const PINT_WORLD_CREATE& desc)
{
	void SetupAllocator(Rock::Allocator* allocator);
	SetupAllocator(&gWrapper);

	gTotalNbAllocs	= 0;
	gNbAllocs		= 0;
	gCurrentMemory	= 0;
	gMuJoCoPint		= this;

	for(PxU16 j=0;j<PEEL_GROUP_SIZE;j++)
		for(PxU16 i=0;i<PEEL_GROUP_SIZE;i++)
			setGroupCollisionFlag(i, j, true);

	mju_user_error = MyErrorCallback;
	mju_user_warning = MyErrorCallback;
	mju_user_malloc = MyAlloc;
	mju_user_free = MyFree;
	mjcb_contactfilter = MyContactFilter;

	mSpec = mj_makeSpec();
	//mjs_defaultSpec(mSpec);

	mj_defaultOption(&mSpec->option);

	mSpec->option.solver = gSolver;
	mSpec->option.cone = gCone;
	mSpec->option.jacobian = gJacobian;
	mSpec->option.integrator = gIntegrator;

	mSpec->option.iterations = gIterations;					// maximum number of main solver iterations
	mSpec->option.ls_iterations = gLSIterations;			// maximum number of CG/Newton linesearch iterations
	mSpec->option.noslip_iterations = gNoSlipIterations;	// maximum number of noslip solver iterations
	mSpec->option.ccd_iterations = gCCDIterations;			// maximum number of convex collision solver iterations

	mSpec->option.impratio	= gImpRatio;

/*
  int disableflags;               // bit flags for disabling standard features
  int enableflags;                // bit flags for enabling optional features
  int disableactuator;            // bit flags for disabling actuators by group id*/

	//mSpec->option.disableflags |= mjDSBL_CONTACT;
	//mSpec->option.disableflags |= mjDSBL_GRAVITY;
	//mSpec->option.disableflags |= mjDSBL_WARMSTART;
	//mSpec->option.disableflags |= mjDSBL_CONSTRAINT;
	
	if(gMultiCCD)
		mSpec->option.enableflags |= mjENBL_MULTICCD;	// Otherwise cylinder stacks are really unstable (e.g. "CylinderStack" test)

	mSpec->option.timestep = mjtNum(desc.mTimestep) / mjtNum(gSubsteps);
	//printf("timestep: %f\n", mSpec->option.timestep);

	ToMjtNumVec(mSpec->option.gravity, desc.mGravity);

	// 0.9 0.95 0.001 0.5 2.0
	/*mSpec->option.o_solimp[0] = 0.0f;
	mSpec->option.o_solimp[1] = 0.0f;
	mSpec->option.o_solimp[2] = 0.0f;
	mSpec->option.o_solimp[3] = 0.0f;
	mSpec->option.o_solimp[4] = 0.0f;*/

	mWorld = mjs_findBody(mSpec, "world");
	mBodyID = 1;	// 0 reserved for world
	RegisterBody(mWorld, 0);

	mGeomID = 0;
	mJointID = 0;
	mConvexMeshID = 0;

	if(desc.mCreateDefaultEnvironment && gPlaneGeomAsGround)
		CreateGroundPlane();
}

void MuJoCoPint::Close()
{
	mBodyData.Reset();
	mVisualData.Reset();
	mShapeData.Reset();
	mMeshData.Reset();
	mArticulationData.Reset();
	mActuatorData.Reset();
	mInitialVelocities.Reset();
	mBodyIDsToClearOut.Reset();

	if(mThreadPool)
	{
		mju_threadPoolDestroy(mThreadPool);
		mThreadPool = null;
	}
	if(mSpec)
	{
		mj_deleteSpec(mSpec);
		mSpec = null;
	}
	if(mData)
	{
		mj_deleteData(mData);
		mData = null;
	}
	if(mModel)
	{
		mj_deleteModel(mModel);
		mModel = null;
	}

	mWorld = null;

	if(gNbAllocs)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED|FOREGROUND_INTENSITY);
		printf("MuJoCo: %d leaks found (%d bytes)\n", gNbAllocs, gCurrentMemory);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
	}
}

void MuJoCoPint::SetGravity(const Point& gravity)
{
}

void MuJoCoPint::SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)
{
	for(udword i=0;i<nb_groups;i++)
		setGroupCollisionFlag(groups[i].mGroup0, groups[i].mGroup1, false);
}

void MuJoCoPint::CompileModel()
{
	if(mModel && mData)
		return;

	if(mSpec)
	{
		mModel = mj_compile(mSpec, null);
		if(mModel)
		{
			if(mjs_isWarning(mSpec))
			{
				const char* err = mjs_getError(mSpec);
				if(err)
					printf("%s\n", err);
			}

			mData = mj_makeData(mModel);

			if(gNbThreads)
			{
				mThreadPool = mju_threadPoolCreate(gNbThreads);
				mju_bindThreadPool(mData, mThreadPool);
			}

			// Apply initial velocities
			udword Nb = mInitialVelocities.Size();
			if(Nb)
			{
				const InitialVels* vels = mInitialVelocities.Begin();

				for(udword i=0;i<Nb;i++)
				{
					const udword bodyID = vels->mBodyID;
					const udword nbJoints = mModel->body_jntnum[bodyID];
					ASSERT(nbJoints == 1);
					const udword jointStart = mModel->body_jntadr[bodyID];
					const udword jointID = jointStart;

					const int dof_adr = mModel->jnt_dofadr[jointID]; // DOF index for the free joint
					ToMjtNumVec(&mData->qvel[dof_adr + 0], vels->mLinear);
					ToMjtNumVec(&mData->qvel[dof_adr + 3], vels->mAngular);
					vels++;
				}

				mInitialVelocities.Clear();
			}

			/*struct Sort
			{
				static int func(void const* ptr0, void const* ptr1)
				{
					const ShapeData* data0 = reinterpret_cast<const ShapeData*>(ptr0);
					const ShapeData* data1 = reinterpret_cast<const ShapeData*>(ptr1);
					if(data0->mBodyID > data1->mBodyID)
						return 1;
					if(data0->mBodyID < data1->mBodyID)
						return -1;
					if(data0->mGeomID > data1->mGeomID)
						return 1;
					if(data0->mGeomID < data1->mGeomID)
						return -1;
					return 0;
				}
			};

			qsort(mShapeData.Begin(), mShapeData.Size(), sizeof(ShapeData), Sort::func);*/

			if(0)
			{
				int res = mj_saveXML(mSpec, "f:/tmp/mujoco_from_peel.xml", "error", 0);
				printf("Saved: %d\n", res);
			}

			printf("MuJoCo model successfully compiled:\n");
			printf("   - %d bodies\n", mModel->nbody);
			printf("   - %d generalized coordinates\n", mModel->nq);
			printf("   - %d DOFs\n", mModel->nv);
			printf("   - %d actuators\n", mModel->nu);
			printf("   - %d BVH\n", mModel->nbvh);
			printf("   - %d joints\n", mModel->njnt);
			printf("   - %d geoms\n", mModel->ngeom);
			printf("   - %d meshes\n", mModel->nmesh);
			printf("   - %d equality constraints\n", mModel->neq);
			printf("   - %d tendons\n", mModel->ntendon);
		}
		else
		{
			//printf("Model failed to compile!\n");
			const char* err = mjs_getError(mSpec);
			if(err)
				printf("%s\n", err);
		}

		//mj_forward(mModel, mData);
	}
}

udword MuJoCoPint::Update(float dt)
{
	// Lazy model compilation the first time we're called. Necessary as the PINT interface doesn't have the equivalent of an "EndModel" call.
	CompileModel();

	if(mModel && mData)
	{
		udword Nb = mActuatorData.Size();
		if(Nb)
		{
			const ActuatorData* actuators = mActuatorData.Begin();
			while(Nb--)
			{
				if(actuators->mEnabled)	// ### doesn't really make a difference
				{
					//mData->ctrl[actuators->mJointID - 1] = actuators->mTargetVelocity;
					if(actuators->mActuatorID != INVALID_ID)
						mData->ctrl[actuators->mActuatorID] = actuators->mTargetVelocity;
					if(actuators->mActuatorID2 != INVALID_ID)
						mData->ctrl[actuators->mActuatorID2] = actuators->mTargetPos;
				}
				actuators++;
			}
		}


		const mjtNum simstart = mData->time;
		//while(mData->time - simstart < dt*0.1f)	// this line to expose basic ccd collision failures
		while(mData->time - simstart < dt)
			mj_step(mModel, mData);

		// Zero external forces (from mouse manip)
		const udword NbToClear = mBodyIDsToClearOut.Size();
		for(udword i=0;i<NbToClear;i++)
		{
			const udword bodyID = mBodyIDsToClearOut[i];
			mjtNum* force = mData->xfrc_applied + 6*bodyID;
			ZeroMemory(force, sizeof(mjtNum) * 6);
		}
		mBodyIDsToClearOut.Clear();
	}

	return gCurrentMemory;
}

void MuJoCoPint::RenderAABBs(PintRender& renderer) const
{
	if(!mData || !mModel)
		return;

	const Point BoundsColor(1.0f, 1.0f, 0.0f);
	const udword nbBodies = mModel->nbody;
	for(udword i=0; i<nbBodies; i++)
	{
		const udword bodyID = i;
			
		const udword nbGeoms = mModel->body_geomnum[bodyID];
		const udword geomStart = mModel->body_geomadr[bodyID];

		const PR BodyWorldPose = getBodyWorldPose(mData, bodyID);

		for(udword i=0; i<nbGeoms; i++)
		{
			const udword geomID = geomStart + i;

			PR ShapeLocalPose = getShapeLocalPose(mModel, geomID);

			PR ShapeWorldPose = ShapeLocalPose;
			ShapeWorldPose *= BodyWorldPose;

			const mjtNum* LocalAABB = mModel->geom_aabb + geomID * 6;
			const Point c = ToPoint(LocalAABB);
			const Point e = ToPoint(LocalAABB + 3);

			AABB LocalBounds;
			LocalBounds.SetCenterExtents(c, e);

			AABB WorldBounds;
			LocalBounds.Rotate(ShapeWorldPose, WorldBounds);

			renderer.DrawWireframeAABB(1, &WorldBounds, BoundsColor);
		}
	}
}

void MuJoCoPint::RenderGeoms(PintRender& renderer) const
{
	if(!mData || !mModel)
		return;

	const ShapeData* shapeData = mShapeData.Begin();
	const udword nbBodies = mModel->nbody;
	for(udword j=0; j<nbBodies; j++)
	{
		const udword bodyID = j;
		if(renderer.SetCurrentActor(bodyIDToPintActor(bodyID)))
		{
			const udword nbGeoms = mModel->body_geomnum[bodyID];
			const udword geomStart = mModel->body_geomadr[bodyID];

			const PR BodyWorldPose = getBodyWorldPose(mData, bodyID);

			{
				const BodyData& bodyData = mBodyData[bodyID];
				for(udword v=0; v<bodyData.mNbVisuals; v++)
				{
					const VisualData& visualData = mVisualData[bodyData.mVisualIndex + v];
					if(visualData.mRenderer)
					//if(renderer.SetCurrentShape())
					{
						PR ShapeWorldPose = visualData.mLocalPose;
						ShapeWorldPose *= BodyWorldPose;

						renderer.DrawShape(visualData.mRenderer, ShapeWorldPose);
					}
				}
			}

			for(udword i=0; i<nbGeoms; i++)
			{
				const udword geomID = geomStart + i;
				if(shapeData[geomID].mRenderer)
				//if(renderer.SetCurrentShape())
				{
					PR ShapeLocalPose = getShapeLocalPose(mModel, geomID);

#ifdef REMOVED
					if(mModel->geom_type[geomID] == mjGEOM_CAPSULE/* || mModel->geom_type[geomID] == mjGEOM_CYLINDER*/)
					{
						Matrix3x3 m;
						m.FromTo(Point(0,1,0), Point(0,0,1));

						Matrix3x3 m2 = ShapeLocalPose.mRot;
						//Matrix3x3 m0 = m * m2;
						Matrix3x3 m0 = m2 * m;
						ShapeLocalPose.mRot = m0;
					}
					else
#endif
					if(mModel->geom_type[geomID] == mjGEOM_CYLINDER || mModel->geom_type[geomID] == mjGEOM_CAPSULE)
					{
						Matrix3x3 m;
						m.FromTo(Point(0,1,0), Point(0,0,1));

						Matrix3x3 m2 = ShapeLocalPose.mRot;
						Matrix3x3 m0 = m * m2;
						//Matrix3x3 m0 = m2 * m;
						ShapeLocalPose.mRot = m0;
					}
					else if(mModel->geom_type[geomID] == mjGEOM_MESH)
					{
						const udword meshID = mModel->geom_dataid[geomID];

						const mjtNum* meshPos = mModel->mesh_pos + meshID * 3;
						const mjtNum* meshRot = mModel->mesh_quat + meshID * 4;

						mjtNum inverse_mesh_pos[3];
						mjtNum inverse_mesh_quat[4];
						mju_negPose(inverse_mesh_pos, inverse_mesh_quat, meshPos, meshRot);

						const mjtNum* localPos = mModel->geom_pos + geomID * 3;
						const mjtNum* localRot = mModel->geom_quat + geomID * 4;

						mjtNum recovered_pos[3];
						mjtNum recovered_quat[4];
						mju_mulPose(recovered_pos, recovered_quat, localPos, localRot, inverse_mesh_pos, inverse_mesh_quat);

						ShapeLocalPose.mPos = ToPoint(recovered_pos);
						ShapeLocalPose.mRot = ToQuat(recovered_quat);
					}
					else if(mModel->geom_type[geomID] == mjGEOM_PLANE)
					{
						int stop = 1;
					}

					PR ShapeWorldPose = ShapeLocalPose;
					ShapeWorldPose *= BodyWorldPose;

					//renderer.DrawShape(shapeData[geomID].mRenderer, ShapeWorldPose);

					// #### this is ridiculous
					const u32 RenderID = IR(mModel->geom_rgba[geomID*4]);
					//const u32 RenderID = mModel->geom_conaffinity[geomID];
					renderer.DrawShape(shapeData[RenderID].mRenderer, ShapeWorldPose);
				}
			}
		}
	}
}

void MuJoCoPint::Render(PintRender& renderer, PintRenderPass render_pass)
{
	RenderGeoms(renderer);
}

void MuJoCoPint::RenderDebugData(PintRender& renderer)
{
	if(!mModel || !mData)
		return;

	if(gDebugDraw_BodyAxes)
	{
		const Point red(1.0f, 0.0f, 0.0f);
		const Point green(0.0f, 1.0f, 0.0f);
		const Point blue(0.0f, 0.0f, 1.0f);

		const udword NbBodies = mModel->nbody;
		for(udword i=0;i<NbBodies;i++)
		{
			const Point p = ToPoint(mData->xpos + i * 3);
			const PxQuat q = ToPxQuat(mData->xquat + i * 4);

			const PxMat33 rot(q);

			const Point axis0 = ToPoint(rot.column0);
			const Point axis1 = ToPoint(rot.column1);
			const Point axis2 = ToPoint(rot.column2);

			renderer.DrawLine(p, p + axis0, red);
			renderer.DrawLine(p, p + axis1, green);
			renderer.DrawLine(p, p + axis2, blue);
		}
	}

	if(gDebugDraw_Contacts)
	{
		const Point ContactColor(0.0f, 1.0f, 1.0f);

		udword NbContacts = mData->ncon;
		//printf("%d\n", NbContacts);
		const mjContact* Contacts = mData->contact;
		for(udword i=0;i<NbContacts;i++)
		{
			const Point pt = ToPoint(Contacts->pos);
			const Point nrm = ToPoint(Contacts->frame);
			renderer.DrawLine(pt, pt - nrm, ContactColor);
			Contacts++;
		}
	}

	if(gDebugDraw_Inertia)
	{
		const udword NbBodies = mModel->nbody;
		for(udword i=0;i<NbBodies;i++)
		{
			mjtNum Ixx = mModel->body_inertia[3*i+0];
			mjtNum Iyy = mModel->body_inertia[3*i+1];
			mjtNum Izz = mModel->body_inertia[3*i+2];
			mjtNum mass = mModel->body_mass[i];
			mjtNum scale_inertia = mju_sqrt(3);

			mjtNum sz[3];
			sz[0] = mju_sqrt((Iyy + Izz - Ixx) / (2 * mass)) * scale_inertia;
			sz[1] = mju_sqrt((Ixx + Izz - Iyy) / (2 * mass)) * scale_inertia;
			sz[2] = mju_sqrt((Ixx + Iyy - Izz) / (2 * mass)) * scale_inertia;

			// scale with mass if enabled
			//if (vopt->flags[mjVIS_SCLINERTIA])
			if(0)
			{
				// density = mass / volume
				mjtNum scale_volume = 8.0;
				mjtNum volume = scale_volume * sz[0]*sz[1]*sz[2];
				mjtNum density = mass / mju_max(mjMINVAL, volume);

				// scale = root3(density)
				mjtNum scale = mju_pow(density*0.001, 1.0/3.0);

				// scale sizes, so that box/ellipsoid with density of 1000 has same mass
				sz[0] *= scale;
				sz[1] *= scale;
				sz[2] *= scale;
			}

			mjtNum* xipos = mData->xipos+3*i;	// Cartesian position of body com                   (nbody x 3)
			mjtNum* ximat = mData->ximat+9*i;	// Cartesian orientation of body inertia            (nbody x 9)

			OBB box;
			box.mCenter = Point(float(xipos[0]), float(xipos[1]), float(xipos[2]));
			box.mExtents = Point(float(sz[0]), float(sz[1]), float(sz[2]));
			box.mRot.m[0][0] = float(ximat[0]);
			box.mRot.m[1][0] = float(ximat[1]);
			box.mRot.m[2][0] = float(ximat[2]);
			box.mRot.m[0][1] = float(ximat[3]);
			box.mRot.m[1][1] = float(ximat[4]);
			box.mRot.m[2][1] = float(ximat[5]);
			box.mRot.m[0][2] = float(ximat[6]);
			box.mRot.m[1][2] = float(ximat[7]);
			box.mRot.m[2][2] = float(ximat[8]);
			renderer.DrawWireframeOBB(box, Point(1.0f, 1.0f, 1.0f));
		}
	}


	//RenderGeoms(renderer, false, gDebugDraw_AABBs);
	if(gDebugDraw_AABBs)
		RenderAABBs(renderer);
}

namespace
{
	struct LocalShapeCreationParams
	{
		LocalShapeCreationParams(mjsBody* body, udword body_id, float mass, PintCollisionGroup group) :
			mBody(body), mBodyID(body_id), mMass(mass), mCollisionGroup(group)	{}

		mjsBody*			mBody;
		udword				mBodyID;
		float				mMass;
		PintCollisionGroup	mCollisionGroup;
	};
}

// TODO: expose this to plugins
static Quat ShortestRotation(const Point& v0, const Point& v1)
{
	const float d = v0|v1;
	const Point cross = v0^v1;

	Quat q = d>-1.0f ? Quat(1.0f + d, cross.x, cross.y, cross.z)
					: fabsf(v0.x)<0.1f ? Quat(0.0f, 0.0f, v0.z, -v0.y) : Quat(0.0f, v0.y, -v0.x, 0.0f);

	q.Normalize();

	return q;
}

void MuJoCoPint::ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)
{
	const LocalShapeCreationParams* params = reinterpret_cast<const LocalShapeCreationParams*>(user_data);

	if(create.mFlags & SHAPE_FLAGS_INACTIVE)
	{
		// Store visual / inactive shapes outside of MuJoCo. Maybe we could use "sites" for this.
		BodyData& bodyData = mBodyData[params->mBodyID];
		if(!bodyData.mNbVisuals)
			bodyData.mVisualIndex = mVisualData.Size();
		bodyData.mNbVisuals++;

		VisualData& visualData = mVisualData.Insert();
		visualData.mRenderer		= create.mRenderer;
		visualData.mLocalPose.mPos	= create.mLocalPos;
		visualData.mLocalPose.mRot	= create.mLocalRot;
		return;
	}

	udword geomID = INVALID_ID;

	mjsGeom* geom = mjs_addGeom(params->mBody, null);
	if(geom)
	{
		struct Local
		{
			static void fixRotation(mjsGeom* geom, const Quat& local_rot)
			{
				if(0)
				{
					Matrix3x3 m;
					m.FromTo(Point(0,0,1), Point(0,1,0));
					//m.Identity();
					//Quat q = m * local_rot;
					//Quat q = local_rot * m;

					Matrix3x3 m2 = local_rot;
					Matrix3x3 m0 = m * m2;
					//Matrix3x3 m0 = m2 * m;
					Quat q = m0;

					geom->quat[0] = q.w;
					geom->quat[1] = q.p.x;
					geom->quat[2] = q.p.y;
					geom->quat[3] = q.p.z;
				}
				else
				{
					const Quat m = ShortestRotation(Point(0.0f, 0.0f, 1.0f), Point(0.0f, 1.0f, 0.0f));
					//PxQuat qqq = PxShortestRotation(PxVec3(0.0f, 0.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.0f));

					const Quat q = local_rot * m;
					geom->quat[0] = q.w;
					geom->quat[1] = q.p.x;
					geom->quat[2] = q.p.y;
					geom->quat[3] = q.p.z;
				}
			}
		};

		//mjs_defaultGeom(geom);	### bad idea, it nullifies meshname

		geomID = mGeomID++;

		ToMjtNumVec(geom->pos, create.mLocalPos);
		ToMjtNumQuat(geom->quat, create.mLocalRot);

		if(create.mType == PINT_SHAPE_SPHERE)
		{
			const PINT_SPHERE_CREATE& sphere = static_cast<const PINT_SPHERE_CREATE&>(create);

			geom->type = mjGEOM_SPHERE;
			geom->size[0] = double(sphere.mRadius);
			geom->size[1] = 0.0;
			geom->size[2] = 0.0;
		}
		else if(create.mType == PINT_SHAPE_CAPSULE)
		{
			const PINT_CAPSULE_CREATE& capsule = static_cast<const PINT_CAPSULE_CREATE&>(create);

			geom->type = mjGEOM_CAPSULE;
			geom->size[0] = double(capsule.mRadius);
			geom->size[1] = double(capsule.mHalfHeight);
			geom->size[2] = 0.0;

			Local::fixRotation(geom, create.mLocalRot);
		}
		else if(create.mType == PINT_SHAPE_BOX)
		{
			const PINT_BOX_CREATE& box = static_cast<const PINT_BOX_CREATE&>(create);

			geom->type = mjGEOM_BOX;
			ToMjtNumVec(geom->size, box.mExtents);
		}
		else if(create.mType == PINT_SHAPE_CYLINDER)
		{
			const PINT_CYLINDER_CREATE& cylinder = static_cast<const PINT_CYLINDER_CREATE&>(create);

			geom->type = mjGEOM_CYLINDER;
			geom->size[0] = double(cylinder.mRadius);
			geom->size[1] = double(cylinder.mHalfHeight);
			geom->size[2] = 0.0;

			Local::fixRotation(geom, create.mLocalRot);
		}
		else if(create.mType == PINT_SHAPE_CONVEX)
		{
			const PINT_CONVEX_CREATE& convex = static_cast<const PINT_CONVEX_CREATE&>(create);

			geom->type = mjGEOM_MESH;
			
			udword convexMeshID = INVALID_ID;
			{
				// Look for same mesh, avoid creating the same mesh multiple times
				udword Nb = mMeshData.Size();
				if(Nb)
				{
					const MeshData* data = mMeshData.Begin();
					while(Nb--)
					{
						if(data->mRenderer == convex.mRenderer)
						{
							convexMeshID = data->mMeshID;
							break;
						}
						data++;
					}
				}
			}

			if(convexMeshID != INVALID_ID)
			{
				const char* convexName = _F("convex%d", convexMeshID);
				mjs_setString(geom->meshname, convexName);
			}
			else
			{
				mjsMesh* mesh = mjs_addMesh(mSpec, null);
				//mjs_defaultMesh(mesh);

				convexMeshID = mConvexMeshID++;

				const char* convexName = _F("convex%d", convexMeshID);
				mjs_setString(mesh->name, convexName);
				mjs_setString(geom->meshname, convexName);

				MeshData& meshData = mMeshData.Insert();
				meshData.mRenderer = convex.mRenderer;
				meshData.mMeshID = convexMeshID;

				//mesh->maxhullvert = maxhullvert;
				const Point* src = convex.mVerts;
				mjFloatVec* dst = mesh->uservert;
					std::vector<float> vertices;
					dst = &vertices;
				dst->reserve(convex.mNbVerts * 3);
				for(udword v=0; v<convex.mNbVerts; v++)
				{
					dst->push_back(src->x);
					dst->push_back(src->y);
					dst->push_back(src->z);
					src++;
				}

				//*mesh->uservert = vertices;
				//mesh->uservert = dst;
				//mesh->uservert = std::move(*dst);
				//mesh->uservert->assign(dst->begin(), dst->end());
				mjs_setFloat(mesh->uservert, dst->data(), (int)dst->size());	//### well it wasn't obvious that we had to use this, everything else crashed
			}
		}
		else if(create.mType == PINT_SHAPE_MESH)
		{
			const PINT_MESH_CREATE& mesh = static_cast<const PINT_MESH_CREATE&>(create);

			geom->type = mjGEOM_FLEX;
			
			/*udword convexMeshID = INVALID_ID;
			{
				// Look for same mesh, avoid creating the same mesh multiple times
				udword Nb = mMeshData.Size();
				if(Nb)
				{
					const MeshData* data = mMeshData.Begin();
					while(Nb--)
					{
						if(data->mRenderer == convex.mRenderer)
						{
							convexMeshID = data->mMeshID;
							break;
						}
						data++;
					}
				}
			}

			if(convexMeshID != INVALID_ID)
			{
				const char* convexName = _F("convex%d", convexMeshID);
				mjs_setString(geom->meshname, convexName);
			}
			else*/
			{
				mjsFlex* flex = mjs_addFlex(mSpec);
				//mjs_defaultMesh(mesh);

u32				convexMeshID = mConvexMeshID++;

				const char* convexName = _F("convex%d", convexMeshID);
				mjs_setString(flex->name, convexName);
				//mjs_setString(geom->meshname, convexName);

/*				MeshData& meshData = mMeshData.Insert();
				meshData.mRenderer = convex.mRenderer;
				meshData.mMeshID = convexMeshID;*/

				{
					const Point* src = mesh.GetSurface().mVerts;
					std::vector<double> vertices;
					mjDoubleVec* dst = &vertices;
					const udword NbVerts = mesh.GetSurface().mNbVerts;
					dst->reserve(NbVerts * 3);
					for(udword v=0; v<NbVerts; v++)
					{
						dst->push_back(src->x);
						dst->push_back(src->y);
						dst->push_back(src->z);
						src++;
					}
					mjs_setDouble(flex->vert, dst->data(), (int)dst->size());
				}
				{
					const udword* dfaces = mesh.GetSurface().mDFaces;
					const uword* wfaces = mesh.GetSurface().mWFaces;
					std::vector<int> triangles;
					mjIntVec* dst = &triangles;
					const udword NbFaces = mesh.GetSurface().mNbFaces;
					dst->reserve(NbFaces * 3);
					for(udword v=0; v<NbFaces; v++)
					{
						int vref0, vref1, vref2;
						if(dfaces)
						{
							vref0 = dfaces[v*3 + 0];
							vref1 = dfaces[v*3 + 1];
							vref2 = dfaces[v*3 + 2];
						}
						else
						{
							vref0 = wfaces[v*3 + 0];
							vref1 = wfaces[v*3 + 1];
							vref2 = wfaces[v*3 + 2];
						}
						dst->push_back(vref0);
						dst->push_back(vref1);
						dst->push_back(vref2);
					}
					mjs_setInt(flex->elem, dst->data(), (int)dst->size());
				}
			}
		}
		else
		{
			ASSERT(0);
		}

		geom->margin = gMargin;
		geom->gap = gGap;
		//1
		//0.005
		//0.0001
		if(1)
		{
			// "one-sided friction coefficients: slide, roll, spin"
			geom->friction[0] = gFriction;
			geom->friction[1] = 0.0;
			geom->friction[2] = 0.0;
		}
		if(create.mMaterial)
		{
			geom->friction[0] = create.mMaterial->mDynamicFriction;
		}

//		if(params->mMass != 0.0f)
//			geom->priority = 1;

		//geom->condim = 1;
		geom->mass = params->mMass;
//  double mass;                     // used to compute density
//  double density;                  // used to compute mass and inertia from volume or surface
//  mjtGeomInertia typeinertia;      // selects between surface and volume inertia

		geom->solref[0] = gSolRef0;
		geom->solref[1] = gSolRef1;
		//geom->solmix = 0.0f;
		//geom->solimp[0] = 100.0;

		mjs_setString(geom->name, _F("geom%d", geomID));

		// ### this is quite tedious
		//geom->group = geomID;
		//geom->conaffinity = geomID;
		geom->rgba[0] = FR(geomID);
	}

	ShapeData& shapeData = mShapeData.Insert();
	shapeData.mRenderer		= create.mRenderer;
	shapeData.mBodyID		= params->mBodyID;
	shapeData.mGroupID		= params->mCollisionGroup;
	shapeData.mGeomID		= geomID;
}

MuJoCoPint::BodyData& MuJoCoPint::AddBody(mjsBody* parent, udword* body_id, const PR* world_pose)
{
	mjsBody* body = mjs_addBody(parent, null);

	const udword bodyID = mBodyID++;
	if(body_id)
		*body_id = bodyID;

	BodyData& bodyData = RegisterBody(body, bodyID, world_pose);

	mjs_setString(body->name, _F("body%d", bodyID));	// ### test, we could use the PINT_OBJECT_CREATE name instead
	return bodyData;
}

PintActorHandle MuJoCoPint::CreateGroundPlane()
{
	mjsGeom* geom = mjs_addGeom(mWorld, null);
	if(geom)
	{
		udword geomID = mGeomID++;

		geom->type = mjGEOM_PLANE;

		Matrix3x3 m;
		m.FromTo(Point(0,0,1), Point(0,1,0));

		Quat q = m;

		geom->quat[0] = q.w;
		geom->quat[1] = q.p.x;
		geom->quat[2] = q.p.y;
		geom->quat[3] = q.p.z;


		geom->size[0] = 1.0;
		geom->size[1] = 1.0;
		geom->size[2] = 0.01;
		geom->margin = gMargin;
		geom->gap = gGap;
		if(1)
		{
			// "one-sided friction coefficients: slide, roll, spin"
			geom->friction[0] = gFriction;
			geom->friction[1] = 0.0;
			geom->friction[2] = 0.0;
		}
		//geom->friction[0] = create.mMaterial->mDynamicFriction;
		geom->solref[0] = gSolRef0;
		geom->solref[1] = gSolRef1;
		//geom->solmix = 0.0f;
		//geom->solimp[0] = 100.0;

		ShapeData& shapeData = mShapeData.Insert();
		shapeData.mRenderer		= null;
		shapeData.mBodyID		= 0;
		shapeData.mGroupID		= 0;//params->mCollisionGroup;
		shapeData.mGeomID		= geomID;

		return bodyIDToPintActor(0);
	}
	return null;
}

PintActorHandle MuJoCoPint::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	if(!desc.mAddToDatabase && gPlaneGeomAsGround)
	{
		// ### well this doesn't work here for some reason - the geom ID to mShapeData mapping cannot be implicit actually,
		// seems this breaks it. If we create the plane last in e.g. box tower scene, it's the last entry in mShapeData (index 32)
		// but it's the first one listed internally in MuJoCo (index 0), since it's the first geom of the world body. So we create
		// the ground plane first in Init(), which fixes it.
		//return CreateGroundPlane();
		return null;
	}

	udword bodyID;
	BodyData& bodyData = AddBody(mWorld, &bodyID);
	mjsBody* body = bodyData.mBody;

	//mjs_defaultBody(body);

	if(!desc.mExplicitInertiaTensor.IsNotUsed())
	{
		body->mass = desc.mMass;
		ToMjtNumVec(body->ipos, desc.mExplicitMassLocalPose.mPos);
		ToMjtNumQuat(body->iquat, desc.mExplicitMassLocalPose.mRot);
		ToMjtNumVec(body->inertia, desc.mExplicitInertiaTensor);
	}

	ToMjtNumVec(body->pos, desc.mPosition);
	ToMjtNumQuat(body->quat, desc.mRotation);

	if(desc.mMass != 0.0f)
	{
		udword jointID;
		AddJoint(body, mjJNT_FREE, &jointID);

		if(!desc.mLinearVelocity.IsZero() || !desc.mAngularVelocity.IsZero())
		{
			InitialVels& vel = mInitialVelocities.Insert();
			vel.mBodyID = bodyID;
			vel.mLinear = desc.mLinearVelocity;
			vel.mAngular = desc.mAngularVelocity;
		}
	}

	const udword nbShapes = desc.GetNbShapes();
	//printf("%d shapes\n", nbShapes);

	LocalShapeCreationParams Params(body, bodyID, desc.mMass / float(nbShapes), desc.mCollisionGroup);
	desc.GetNbShapes(this, &Params);

	return bodyIDToPintActor(bodyID);
}

bool MuJoCoPint::ReleaseObject(PintActorHandle handle)
{
	return false;
}

PintJointHandle MuJoCoPint::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	mjsEquality* equality = null;
	if(desc.mType == PINT_JOINT_SPHERICAL)
	{
		// We need this for loop joints
		const PINT_SPHERICAL_JOINT_CREATE& sjc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);

		equality = mjs_addEquality(mSpec, null);
		equality->type = mjEQ_CONNECT;

		// ### determined using BasicJointAPI PEEL test, for some reason not the same as for fixed
		ToMjtNumVec(equality->data, sjc.mLocalPivot0.mPos);
		ToMjtNumVec(equality->data + 3, sjc.mLocalPivot1.mPos);
	}
	else if(desc.mType == PINT_JOINT_FIXED)
	{
		const PINT_FIXED_JOINT_CREATE& fjc = static_cast<const PINT_FIXED_JOINT_CREATE&>(desc);

		equality = mjs_addEquality(mSpec, null);
		equality->type = mjEQ_WELD;

		// ### determined using BasicJointAPI PEEL test, for some reason not the same as for spherical
		ToMjtNumVec(equality->data, fjc.mLocalPivot1);
		ToMjtNumVec(equality->data + 3, fjc.mLocalPivot0);
	}

	if(equality)
	{
		const udword bodyID0 = pintActorToBodyID(desc.mObject0);
		const udword bodyID1 = pintActorToBodyID(desc.mObject1);

		mjs_setString(equality->name1, _F("body%d", bodyID0));
		mjs_setString(equality->name2, _F("body%d", bodyID1));

		equality->objtype = mjOBJ_BODY;
	}

	return PintJointHandle(equality);
}

bool MuJoCoPint::ReleaseJoint(PintJointHandle handle)
{
	return false;
}

static void inline_ SetupLimits(mjsJoint* j, double min_limit, double max_limit)
{
	j->limited = true;
	j->range[0] = min_limit;
	j->range[1] = max_limit;
}

mjsJoint* MuJoCoPint::AddJoint(mjsBody* body, mjtJoint type, udword* joint_id)
{
	mjsJoint* j = mjs_addJoint(body, null);
	j->type = type;

	// It seems there is no notion of "linear damping" and "angular damping" on rigid bodies.
	// We can use this single damping coeff on joints instead, to at least prevent explosions
	// of quickly spinning bodies. ### actually it still explodes even with damping
	j->damping = gDamping;

	j->frictionloss = gFrictionLoss;

	j->limited = false;

	const udword jointID = mJointID++;
	if(joint_id)
		*joint_id = jointID;

	mjs_setString(j->name, _F("joint%d", jointID));

	return j;
}

PintArticHandle MuJoCoPint::CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE& desc)
{
	const udword articID = mArticulationData.Size();

	ArticulationData& articulationData = mArticulationData.Insert();
	articulationData.mFixedBase = desc.mFixBase;

	return bodyIDToPintArtic(articID);
}

PintActorHandle MuJoCoPint::CreateRCArticulatedObject(const PINT_OBJECT_CREATE& desc, const PINT_RC_ARTICULATED_BODY_CREATE& jc, PintArticHandle articHandle)
{
	const udword articID = pintArticToBodyID(articHandle);
	const ArticulationData* articulationData = mArticulationData.Begin();
	const ArticulationData& articulation = articulationData[articID];

	//

	mjsBody* parent = mWorld;
	PR parentPoseWorldSpace;
	if(jc.mParent)
	{
		const udword parentBodyID = pintActorToBodyID(jc.mParent);

		const BodyData* bodyData = mBodyData.Begin();
		udword NbBodies = mBodyData.Size();
		ASSERT(parentBodyID < NbBodies);

		parent = bodyData[parentBodyID].mBody;
		parentPoseWorldSpace = bodyData[parentBodyID].mInitialPoseWorldSpace;
	}

	// We need to keep track of initial world pose for articulation links.
	// We cannot retrieve them from MuJoCo, as it only stores the relative poses in mjsBody.
	const PR initialWorldPose(desc.mPosition, desc.mRotation);

	udword bodyID;
	BodyData& bodyData = AddBody(parent, &bodyID, &initialWorldPose);
	mjsBody* body = bodyData.mBody;

	// ###refactor
	if(!desc.mExplicitInertiaTensor.IsNotUsed())
	{
		body->mass = desc.mMass;
		ToMjtNumVec(body->ipos, desc.mExplicitMassLocalPose.mPos);
		ToMjtNumQuat(body->iquat, desc.mExplicitMassLocalPose.mRot);
		ToMjtNumVec(body->inertia, desc.mExplicitInertiaTensor);
	}

	if(jc.mParent)
	{
		const PxTransform worldPose = ToPxTransform(initialWorldPose);
		//const PxTransform parentWorldPose = ToPxTransform(PR(ToPoint(parent->pos), ToQuat(parent->quat)));
		//const PxTransform parentWorldPose = ToPxTransform(parentBodyData->mInitialPoseWorldSpace);
		const PxTransform parentWorldPose = ToPxTransform(parentPoseWorldSpace);
		
		const PxTransform relPose = parentWorldPose.transformInv(worldPose);
		//const PxTransform relPose2 = worldPose.transformInv(parentWorldPose);
		ToMjtNumVec(body->pos, ToPoint(relPose.p));
			//ToMjtNumVec(body->pos, ToPoint(worldPose.p - parentWorldPose.p));	// #### ffs
		//ToMjtNumQuat(body->quat, ToQuat(relPose.q.getConjugate()));
		ToMjtNumQuat(body->quat, ToQuat(relPose.q));

		//printf("%f %f %f\n", relPose.p.x, relPose.p.y, relPose.p.z);
			//ToMjtNumVec(body->pos, Point(desc.mPosition.x, 0.0f, desc.mPosition.z));

		//ToMjtNumVec(body->pos, desc.mPosition);
		//ToMjtNumQuat(body->quat, desc.mRotation);
	}
	else
	{
		ToMjtNumVec(body->pos, desc.mPosition);
		ToMjtNumQuat(body->quat, desc.mRotation);

		if(!articulation.mFixedBase)
		{
			udword jointID;
			AddJoint(body, mjJNT_FREE, &jointID);

			/*if(!desc.mLinearVelocity.IsZero() || !desc.mAngularVelocity.IsZero())
			{
				InitialVels* vel = ICE_RESERVE(InitialVels, mInitialVelocities);
				vel->mID = jointID;
				vel->mLinear = desc.mLinearVelocity;
				vel->mAngular = desc.mAngularVelocity;
			}*/
		}
		else
		{
			// trick MuJoCo into thinking the root is dynamic, to preserve automatic collision filtering between parent & child bodies (sigh)
			if(1)
			{
				udword jointID;
				mjsJoint* j = AddJoint(body, mjJNT_HINGE, &jointID);
				ToMjtNumVec(j->pos, Point(0.0f, 0.0f, 0.0f));
				ToMjtNumVec(j->axis, Point(0.0f, 1.0f, 0.0f));
				//j->damping = 1e32f;
				SetupLimits(j, -0.0001, 0.0001);
			}
		}
	}

	if(jc.mParent)
	{
		if(jc.mJointType == PINT_JOINT_SPHERICAL)
		{
			mjsJoint* j = AddJoint(body, mjJNT_BALL);

			ToMjtNumVec(j->pos, jc.mLocalPivot1.mPos);
			//ToMjtNumVec(j->axis, ToPoint(ToPxQuat(jc.mLocalPivot1.mRot).getBasisVector0()));

const PxQuat qq = ToPxQuat(jc.mLocalPivot1.mRot);
			if(jc.mAxisIndex == X_)
			{
				ToMjtNumVec(j->axis, Point(1.0f, 0.0f, 0.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector0()));
			}
			else if(jc.mAxisIndex == Y_)
			{
				ToMjtNumVec(j->axis, Point(0.0f, 1.0f, 0.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector1()));
			}
			else if(jc.mAxisIndex == Z_)
			{
				ToMjtNumVec(j->axis, Point(0.0f, 0.0f, 1.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector2()));
			}

		}
		else if(jc.mJointType == PINT_JOINT_PRISMATIC)
		{
			mjsJoint* j = AddJoint(body, mjJNT_SLIDE);

			ToMjtNumVec(j->pos, jc.mLocalPivot1.mPos);

const PxQuat qq = ToPxQuat(jc.mLocalPivot1.mRot);
			if(jc.mAxisIndex == X_)
			{
				ToMjtNumVec(j->axis, Point(1.0f, 0.0f, 0.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector0()));
			}
			else if(jc.mAxisIndex == Y_)
			{
				ToMjtNumVec(j->axis, Point(0.0f, 1.0f, 0.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector1()));
			}
			else if(jc.mAxisIndex == Z_)
			{
				ToMjtNumVec(j->axis, Point(0.0f, 0.0f, 1.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector2()));
			}

			if(jc.mMinLimit < jc.mMaxLimit)
				SetupLimits(j, jc.mMinLimit, jc.mMaxLimit);
		}
		else if(jc.mJointType == PINT_JOINT_FIXED)
		{
			// Trying to emulate a fixed joint with a limited hinge
			udword jointID;
			mjsJoint* j = AddJoint(body, mjJNT_HINGE, &jointID);
			ToMjtNumVec(j->pos, Point(0.0f, 0.0f, 0.0f));
			ToMjtNumVec(j->axis, Point(0.0f, 1.0f, 0.0f));
			SetupLimits(j, -0.0001, 0.0001);
		}
		else if(jc.mJointType == PINT_JOINT_HINGE || jc.mJointType == PINT_JOINT_HINGE2)
		{
			udword jointID;
			mjsJoint* j = AddJoint(body, mjJNT_HINGE, &jointID);

			ToMjtNumVec(j->pos, jc.mLocalPivot1.mPos);

/*				//Matrix3x3 r = initialWorldPose.mRot;
				Matrix3x3 r = parentPoseWorldSpace.mRot;
				ToMjtNumVec(j->pos, jc.mLocalPivot1.mPos * r);
*/
/*				j->pos[0] = -1.414;
				j->pos[1] = 0.0;
				j->pos[2] = 1.414;*/


/*
const PxTransform worldPose = ToPxTransform(initialWorldPose);
//const PxTransform parentWorldPose = ToPxTransform(PR(ToPoint(parent->pos), ToQuat(parent->quat)));
//const PxTransform parentWorldPose = ToPxTransform(parentBodyData->mInitialPoseWorldSpace);
const PxTransform parentWorldPose = ToPxTransform(parentPoseWorldSpace);
		
const PxTransform relPose = parentWorldPose.transformInv(worldPose);

PxVec3 worldSpaceAnchor = worldPose.transform(ToPxVec3(jc.mLocalPivot1.mPos));
//ToMjtNumVec(j->pos, relPose.rotate(ToPxVec3(jc.mLocalPivot1.mPos)));
//ToMjtNumVec(j->pos, relPose.rotate(worldSpaceAnchor));
ToMjtNumVec(j->pos, parentWorldPose.transformInv(worldSpaceAnchor));
*/


			//ToMjtNumVec(j->axis, ToPoint(ToPxQuat(jc.mLocalPivot1.mRot).getBasisVector0()));
			//ASSERT(ToPxQuat(jc.mLocalPivot1.mRot).isIdentity());
const PxQuat qq = ToPxQuat(jc.mLocalPivot1.mRot);
			if(jc.mAxisIndex == X_)
			{
				ToMjtNumVec(j->axis, Point(1.0f, 0.0f, 0.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector0()));
			}
			else if(jc.mAxisIndex == Y_)
			{
				ToMjtNumVec(j->axis, Point(0.0f, 1.0f, 0.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector1()));
			}
			else if(jc.mAxisIndex == Z_)
			{
				ToMjtNumVec(j->axis, Point(0.0f, 0.0f, 1.0f));
ToMjtNumVec(j->axis, ToPoint(qq.getBasisVector2()));
			}
	//			j->ref	= 0.0f;

			if(jc.mMinLimit < jc.mMaxLimit)
				SetupLimits(j, jc.mMinLimit * RADTODEG, jc.mMaxLimit * RADTODEG);

			if(jc.mMotorFlags != PINT_MOTOR_NONE)
			{
				// We can use the drive params to determine if we need a velocity or position actuator, but this only works
				// if the drive params are available right from the start - unless we want to always create two of them?

				const udword actuatorID = mActuatorData.Size();
				bodyData.mActuatorID = actuatorID;
				ActuatorData& actuatorData = mActuatorData.Insert();

				actuatorData.mActuatorID = INVALID_ID;
				actuatorData.mActuatorID2 = INVALID_ID;

				if(jc.mMotorFlags & PINT_MOTOR_VELOCITY)
				{
					actuatorData.mActuatorID = actuatorID;
					actuatorData.mTargetVelocity = jc.mTargetVel;
					actuatorData.mEnabled = TRUE;

					mjsActuator* actuator = mjs_addActuator(mSpec, null);
					mjs_setString(actuator->target, _F("joint%d", jointID));
					actuator->trntype = mjTRN_JOINT;

					if(1)	// Actuator/velocity shortcut
					{
						//const double kv = 10000.0;
						const double kv = jc.mMotor.mDamping;
						actuator->dyntype = mjDYN_NONE;
						actuator->dynprm[0] = 1.0;
						actuator->dynprm[1] = 0.0;
						actuator->dynprm[2] = 0.0;

						actuator->gaintype = mjGAIN_FIXED;
						actuator->gainprm[0] = kv;
						actuator->gainprm[1] = 0.0;
						actuator->gainprm[2] = 0.0;

						actuator->biastype = mjBIAS_AFFINE;
						actuator->biasprm[0] = 0.0;
						actuator->biasprm[1] = 0.0;
						actuator->biasprm[2] = -kv;
					}
				}

				if(0)
				{
					//todo: map these params to MuJoCo's
					//PINT_RC_ARTICULATED_MOTOR_CREATE	mMotor;
					//actuatorData->mJointID = jointID;
					actuatorData.mActuatorID = actuatorID;
					actuatorData.mTargetVelocity = jc.mTargetVel;
	#ifdef ANYMALC
					actuatorData.mTargetVelocity = 0.0f;
	#endif
					actuatorData.mEnabled = TRUE;

					mjsActuator* actuator = mjs_addActuator(mSpec, null);
					mjs_setString(actuator->target, _F("joint%d", jointID));
					actuator->trntype = mjTRN_JOINT;

					if(1)	// Actuator/velocity shortcut
					{
						//const double kv = 10000.0;
						const double kv = jc.mMotor.mDamping;
						actuator->dyntype = mjDYN_NONE;
						actuator->dynprm[0] = 1.0;
						actuator->dynprm[1] = 0.0;
						actuator->dynprm[2] = 0.0;

						actuator->gaintype = mjGAIN_FIXED;
						actuator->gainprm[0] = kv;
						actuator->gainprm[1] = 0.0;
						actuator->gainprm[2] = 0.0;

						actuator->biastype = mjBIAS_AFFINE;
						actuator->biasprm[0] = 0.0;
						actuator->biasprm[1] = 0.0;
						actuator->biasprm[2] = -kv;
					}
					if(0)
					{
						const double kv = 10000.0;
						actuator->dyntype = mjDYN_NONE;
						actuator->dynprm[0] = 1.0;
						actuator->dynprm[1] = 0.0;
						actuator->dynprm[2] = 0.0;

						actuator->gaintype = mjGAIN_FIXED;
						actuator->gainprm[0] = kv;
						actuator->gainprm[1] = 0.0;
						actuator->gainprm[2] = 0.0;

						actuator->biastype = mjBIAS_NONE;
						actuator->biasprm[0] = 0.0;
						actuator->biasprm[1] = 0.0;
						actuator->biasprm[2] = 0.0;
					}
#ifdef ANYMALC
					if(1)	// Actuator/position shortcut
					{
						const double kv = 10.0;
						const double kp = 200.0;
						//const double kv = 100.0;
						//const double kp = 1000.0;

						actuator->dyntype = mjDYN_NONE;
						actuator->dynprm[0] = 0.0;
						actuator->dynprm[1] = 0.0;
						actuator->dynprm[2] = 0.0;

						actuator->gaintype = mjGAIN_FIXED;
						actuator->gainprm[0] = kp;
						actuator->gainprm[1] = 0.0;
						actuator->gainprm[2] = 0.0;

						actuator->biastype = mjBIAS_AFFINE;
						actuator->biasprm[0] = 0.0;
						actuator->biasprm[1] = -kp;
						actuator->biasprm[2] = -kv;
					}
#endif
				}


				if(jc.mMotorFlags & PINT_MOTOR_POSITION)
				{
					//actuatorData->mJointID = jointID;
					actuatorData.mActuatorID2 = actuatorID;
					actuatorData.mTargetPos = jc.mTargetPos;
					actuatorData.mEnabled = TRUE;

					mjsActuator* actuator = mjs_addActuator(mSpec, null);
					mjs_setString(actuator->target, _F("joint%d", jointID));
					actuator->trntype = mjTRN_JOINT;

					if(1)	// Actuator/position shortcut
					{
						//#### hmmm
						const double kv = gKv * jc.mMotor.mDamping;
						const double kp = gKp * jc.mMotor.mStiffness;

						actuator->dyntype = mjDYN_NONE;
						actuator->dynprm[0] = 0.0;
						actuator->dynprm[1] = 0.0;
						actuator->dynprm[2] = 0.0;

						actuator->gaintype = mjGAIN_FIXED;
						actuator->gainprm[0] = kp;
						actuator->gainprm[1] = 0.0;
						actuator->gainprm[2] = 0.0;

						actuator->biastype = mjBIAS_AFFINE;
						actuator->biasprm[0] = 0.0;
						actuator->biasprm[1] = -kp;
						actuator->biasprm[2] = -kv;
					}
				}
			}
		}
		else
		{
			ASSERT(0);
		}
	}

	const udword nbShapes = desc.GetNbShapes();

	LocalShapeCreationParams Params(body, bodyID, desc.mMass / float(nbShapes), desc.mCollisionGroup);
	desc.GetNbShapes(this, &Params);

	PintActorHandle h = bodyIDToPintActor(bodyID);
	return h;
}

bool MuJoCoPint::AddRCArticulationToScene(PintArticHandle articulation)
{
	return true;
}

static MuJoCoPint::ActuatorData* GetActuatorData(PintActorHandle handle, Rock::Array<MuJoCoPint::BodyData>& body_data, Rock::Array<MuJoCoPint::ActuatorData>& actuator_data)
{
	const udword bodyID = pintActorToBodyID(handle);

	const MuJoCoPint::BodyData* bodyData = body_data.Begin();
	ASSERT(bodyID < body_data.Size());

	const udword actuatorID = bodyData[bodyID].mActuatorID;
	if(actuatorID == INVALID_ID)
		return false;

	MuJoCoPint::ActuatorData* actuatorData = actuator_data.Begin();
	ASSERT(actuatorID < actuator_data.Size());
	return &actuatorData[actuatorID];
}

bool MuJoCoPint::SetRCADriveEnabled(PintActorHandle handle, bool flag)
{
	ActuatorData* actuatorData = GetActuatorData(handle, mBodyData, mActuatorData);
	if(!actuatorData)
		return false;

	actuatorData->mEnabled = flag;

	return true;
}

bool MuJoCoPint::SetRCADriveVelocity(PintActorHandle handle, float velocity)
{
	ActuatorData* actuatorData = GetActuatorData(handle, mBodyData, mActuatorData);
	if(!actuatorData)
		return false;

	actuatorData->mTargetVelocity = velocity;

	return true;
}

bool MuJoCoPint::SetRCADrivePosition(PintActorHandle handle, float position)
{
	ActuatorData* actuatorData = GetActuatorData(handle, mBodyData, mActuatorData);
	if(!actuatorData)
		return false;

	actuatorData->mTargetPos = position;

	return true;
}

PR MuJoCoPint::GetWorldTransform(PintActorHandle handle)
{
	if(!handle)
	{
		PR Idt;
		Idt.Identity();
		return Idt;
	}

	const udword bodyID = pintActorToBodyID(handle);

	if(mData)
	{
		const mjtNum* pos = mData->xpos + bodyID * 3;
		const mjtNum* quat = mData->xquat + bodyID * 4;
		return PR(ToPoint(pos), ToQuat(quat));
	}
	else
	{
		// Else this has been called before the first update call, and mData isn't available!
		return mBodyData[bodyID].mInitialPoseWorldSpace;
	}
}

Point MuJoCoPint::GetAngularVelocity(PintActorHandle handle)
{
	const udword bodyID = pintActorToBodyID(handle);

	if(0)
	{
		const udword nbJoints = mModel->body_jntnum[bodyID];
		ASSERT(nbJoints == 1);
		const udword jointStart = mModel->body_jntadr[bodyID];
		const udword jointID = jointStart;

		const int dof_adr = mModel->jnt_dofadr[jointID]; // DOF index for the free joint
		//ToMjtNumVec(&mData->qvel[dof_adr + 0], vels->mLinear);
		//ToMjtNumVec(&mData->qvel[dof_adr + 3], vels->mAngular);
		return ToPoint(&mData->qvel[dof_adr + 3]);
	}

	return ToPoint(&mData->cvel[bodyID * 6]);		// ##### ??? angular vel is first?
	//return ToPoint(&mData->cvel[bodyID * 6 + 3]);

	/*
	const mjtNum* pos = mData->xpos + bodyID * 3;
	const mjtNum* quat = mData->xquat + bodyID * 4;

	//const PR WorldPose(ToPoint(pos), ToQuat(quat));
	//const Matrix4x4 m44(WorldPose);
	const Matrix3x3 m33(ToQuat(quat));
	return ToPoint(&mData->cvel[bodyID * 6 + 3]) * m33;
	*/
}

void MuJoCoPint::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
}

void MuJoCoPint::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
	ASSERT(mData);	// Else this has been called before the first update call, and mData isn't available!
	if(mData)
	{
		const udword bodyID = pintActorToBodyID(handle);
		mjtNum* force = mData->xfrc_applied + 6*bodyID;

		//### todo: do that better

		//Point action = world_impulse * float(mModel->body_mass[bodyID]);
		Point action = world_impulse * 20.0f;

		ToMjtNumVec(force, action);

		const mjtNum* pos = mData->xpos + bodyID * 3;
		Point com = ToPoint(pos);	// ###

		const Point torque = (world_pos - com)^action;

		ToMjtNumVec(force + 3, torque);

		mBodyIDsToClearOut.PushBack(bodyID);
	}
	else
		printf("MuJoCoPint::AddWorldImpulseAtWorldPos: mData not available\n");
}

udword MuJoCoPint::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	if(!mModel || !mData)
		return 0;

	const udword nbColors = GetNbColors();

	udword NbHits = 0;
	while(nb--)
	{
		const mjtNum pnt[3] = { raycasts->mOrigin.x, raycasts->mOrigin.y, raycasts->mOrigin.z};
		const mjtNum vec[3] = { raycasts->mDir.x, raycasts->mDir.y, raycasts->mDir.z};

		int geomID;

		const mjtNum d = mj_ray(mModel, mData, pnt, vec, null, 1, -1, &geomID);
		if(d>=0.0f && d<=raycasts->mMaxDist)
		{
			const ShapeData* shapeData = mShapeData.Begin();
			const udword bodyID = shapeData[geomID].mBodyID;
			dest->mTouchedActor = bodyIDToPintActor(bodyID);
			dest->mTouchedShape = geomIDToPintShape(geomID);	// ### not sure about that one
			dest->mDistance = float(d);
			dest->mImpact = raycasts->mOrigin + float(d) * raycasts->mDir;
			dest->mNormal = Point(0.0f, 0.0f, 0.0f);
				// ### MuJoCo doesn't return an impact normal so we return geom-indexed colors just to see something in raytraced view
				const udword color = GetColor(geomID % nbColors);
				dest->mNormal.x = -((color & 0xff)/128.0f);
				dest->mNormal.y = -(((color>>8) & 0xff)/128.0f);
				dest->mNormal.z = -(((color>>16) & 0xff)/128.0f);
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

static MuJoCoPint* gMuJoCo = null;
static void gMuJoCo_GetOptionsFromGUI(const char*);

void MuJoCo_Init(const PINT_WORLD_CREATE& desc)
{
	gMuJoCo_GetOptionsFromGUI(desc.GetTestName());

	ASSERT(!gMuJoCo);
	gMuJoCo = ICE_NEW(MuJoCoPint);
	gMuJoCo->Init(desc);
}

void MuJoCo_Close()
{
	if(gMuJoCo)
	{
		gMuJoCo->Close();
		delete gMuJoCo;
		gMuJoCo = null;
	}
}

MuJoCoPint* GetMuJoCo()
{
	return gMuJoCo;
}

///////////////////////////////////////////////////////////////////////////////

static Widgets*	gMuJoCoGUI = null;

static IceEditBox* gEditBox_NbThreads = null;
static IceEditBox* gEditBox_Iterations = null;
static IceEditBox* gEditBox_LSIterations = null;
static IceEditBox* gEditBox_NoSlipIterations = null;
static IceEditBox* gEditBox_CCDIterations = null;
static IceEditBox* gEditBox_Substeps = null;
static IceEditBox* gEditBox_Friction = null;
static IceEditBox* gEditBox_FrictionLoss = null;
static IceEditBox* gEditBox_Damping = null;
static IceEditBox* gEditBox_SolRef0 = null;
static IceEditBox* gEditBox_SolRef1 = null;
static IceEditBox* gEditBox_ImpRatio = null;
static IceEditBox* gEditBox_Margin = null;
static IceEditBox* gEditBox_Gap = null;
static IceEditBox* gEditBox_Kv = null;
static IceEditBox* gEditBox_Kp = null;
static IceComboBox* gComboBox_Solver = null;
static IceComboBox* gComboBox_Cone = null;
static IceComboBox* gComboBox_Jacobian = null;
static IceComboBox* gComboBox_Integrator = null;
static IceCheckBox*	gCheckBox_Plane = null;
static IceCheckBox*	gCheckBox_MultiCCD = null;
static IceCheckBox*	gCheckBox_DebugDrawContacts = null;
static IceCheckBox*	gCheckBox_DebugDrawAABBs = null;
static IceCheckBox*	gCheckBox_DebugDrawBodyAxes = null;
static IceCheckBox*	gCheckBox_DebugDrawInertia = null;

enum MuJoCoGUIElement
{
	MUJOCO_GUI_MAIN,
	//
	MUJOCO_GUI_PLANE,
	MUJOCO_GUI_MULTI_CCD,
	MUJOCO_GUI_DEBUG_DRAW_CONTACTS,
	MUJOCO_GUI_DEBUG_DRAW_AABBS,
	MUJOCO_GUI_DEBUG_DRAW_BODY_AXES,
	MUJOCO_GUI_DEBUG_DRAW_INERTIA,
	//
};

static void gMuJoCo_GetOptionsFromGUI(const char* test_name)
{
	Common_GetFromEditBox(gNbThreads, gEditBox_NbThreads);
	Common_GetFromEditBox(gIterations, gEditBox_Iterations);
	Common_GetFromEditBox(gLSIterations, gEditBox_LSIterations);
	Common_GetFromEditBox(gNoSlipIterations, gEditBox_NoSlipIterations);
	Common_GetFromEditBox(gCCDIterations, gEditBox_CCDIterations);
	Common_GetFromEditBox(gSubsteps, gEditBox_Substeps);
	Common_GetFromEditBox(gFriction, gEditBox_Friction, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gFrictionLoss, gEditBox_FrictionLoss, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gDamping, gEditBox_Damping, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gSolRef0, gEditBox_SolRef0, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gSolRef1, gEditBox_SolRef1, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gImpRatio, gEditBox_ImpRatio, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gMargin, gEditBox_Margin, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gGap, gEditBox_Gap, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gKv, gEditBox_Kv, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gKp, gEditBox_Kp, -FLT_MAX, FLT_MAX);

	if(gComboBox_Solver)
		gSolver = gComboBox_Solver->GetSelectedIndex();

	if(gComboBox_Cone)
		gCone = gComboBox_Cone->GetSelectedIndex();

	if(gComboBox_Jacobian)
		gJacobian = gComboBox_Jacobian->GetSelectedIndex();

	if(gComboBox_Integrator)
		gIntegrator = gComboBox_Integrator->GetSelectedIndex();

	if(test_name)
		GetPintPlugin()->ApplyTestUIParams(test_name);
}

static IceEditBox* CreateEditBox(PintGUIHelper& helper, IceWindow* parent, sdword& y, const char* label, const char* value, EditBoxFilter filter)
{
	const sdword YStep = 20;
	const sdword LabelOffsetY = 2;
	const sdword EditBoxWidth = 60;
	const sdword LabelWidth = 160;
	const sdword EditBoxX2 = LabelWidth + 10;

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, label, gMuJoCoGUI);
	IceEditBox* EB = helper.CreateEditBox(parent, INVALID_ID, 4+EditBoxX2, y, EditBoxWidth, 20, value, gMuJoCoGUI, filter, null);
	y += YStep;
	return EB;
}

static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
{
	const udword id = check_box.GetID();
	switch(id)
	{
		case MUJOCO_GUI_PLANE:
			gPlaneGeomAsGround = checked;
			break;
		case MUJOCO_GUI_MULTI_CCD:
			gMultiCCD = checked;
			break;
		case MUJOCO_GUI_DEBUG_DRAW_CONTACTS:
			gDebugDraw_Contacts = checked;
			break;
		case MUJOCO_GUI_DEBUG_DRAW_AABBS:
			gDebugDraw_AABBs = checked;
			break;
		case MUJOCO_GUI_DEBUG_DRAW_BODY_AXES:
			gDebugDraw_BodyAxes = checked;
			break;
		case MUJOCO_GUI_DEBUG_DRAW_INERTIA:
			gDebugDraw_Inertia = checked;
			break;
	}
}

static const char* gTooltip_Substeps = "Timestep will be (1/60) divided by the number os substeps (e.g. 2 substeps = 120 Hz sim frequency)";

IceWindow* MuJoCo_InitGUI(IceWidget* parent, PintGUIHelper& helper)
{
	IceWindow* Main = helper.CreateMainWindow(gMuJoCoGUI, parent, MUJOCO_GUI_MAIN, "MuJoCo options");

	const sdword YStep = 20;
	const sdword YStepCB = 16;
	sdword y = 4;

	const sdword OffsetX = 90;
	const sdword LabelOffsetY = 2;
	const sdword EditBoxWidth = 60;
	const udword CheckBoxWidth = 190;
	const sdword LabelWidth = 160;
	const sdword EditBoxX2 = LabelWidth + 10;

	{
		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Solver:", gMuJoCoGUI);

		gComboBox_Solver = CreateComboBox<IceComboBox>(Main, 0, 4+OffsetX, y, 150, 20, "Solver", gMuJoCoGUI, null);
		gComboBox_Solver->Add("PGS");
		gComboBox_Solver->Add("CG");
		gComboBox_Solver->Add("NEWTON");
		gComboBox_Solver->Select(gSolver);
		y += YStep;
	}

	{
		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Cone:", gMuJoCoGUI);

		gComboBox_Cone = CreateComboBox<IceComboBox>(Main, 0, 4+OffsetX, y, 150, 20, "Cone", gMuJoCoGUI, null);
		gComboBox_Cone ->Add("mjCONE_PYRAMIDAL");
		gComboBox_Cone ->Add("mjCONE_ELLIPTIC");
		gComboBox_Cone ->Select(gCone);
		y += YStep;
	}

	{
		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Jacobian:", gMuJoCoGUI);

		gComboBox_Jacobian = CreateComboBox<IceComboBox>(Main, 0, 4+OffsetX, y, 150, 20, "Jacobian", gMuJoCoGUI, null);
		gComboBox_Jacobian->Add("DENSE");
		gComboBox_Jacobian->Add("SPARSE");
		gComboBox_Jacobian->Add("AUTO");
		gComboBox_Jacobian->Select(gJacobian);
		y += YStep;
	}

	{
		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Integrator:", gMuJoCoGUI);

		gComboBox_Integrator = CreateComboBox<IceComboBox>(Main, 0, 4+OffsetX, y, 150, 20, "Integrator", gMuJoCoGUI, null);
		gComboBox_Integrator->Add("mjINT_EULER");
		gComboBox_Integrator->Add("mjINT_RK4");
		gComboBox_Integrator->Add("mjINT_IMPLICIT");
		gComboBox_Integrator->Add("mjINT_IMPLICITFAST");
		gComboBox_Integrator->Select(gIntegrator);
		y += YStep;
	}

	y += YStep;

	gEditBox_NbThreads			= CreateEditBox(helper, Main, y, "Nb threads:", _F("%d", gNbThreads), EDITBOX_INTEGER_POSITIVE);
	gEditBox_Iterations			= CreateEditBox(helper, Main, y, "Iterations:", _F("%d", gIterations), EDITBOX_INTEGER_POSITIVE);
	gEditBox_LSIterations		= CreateEditBox(helper, Main, y, "LS iterations:", _F("%d", gLSIterations), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NoSlipIterations	= CreateEditBox(helper, Main, y, "NoSlip iterations:", _F("%d", gNoSlipIterations), EDITBOX_INTEGER_POSITIVE);
	gEditBox_CCDIterations		= CreateEditBox(helper, Main, y, "CCD iterations:", _F("%d", gCCDIterations), EDITBOX_INTEGER_POSITIVE);
	gEditBox_Substeps			= CreateEditBox(helper, Main, y, "Substeps:", _F("%d", gSubsteps), EDITBOX_INTEGER_POSITIVE);
	gEditBox_Friction			= CreateEditBox(helper, Main, y, "Default friction:", _F("%f", gFriction), EDITBOX_FLOAT);
	gEditBox_FrictionLoss		= CreateEditBox(helper, Main, y, "Friction loss:", _F("%f", gFrictionLoss), EDITBOX_FLOAT);
	gEditBox_Damping			= CreateEditBox(helper, Main, y, "Damping:", _F("%f", gDamping), EDITBOX_FLOAT);
	gEditBox_SolRef0			= CreateEditBox(helper, Main, y, "SolRef0:", _F("%f", gSolRef0), EDITBOX_FLOAT);
	gEditBox_SolRef1			= CreateEditBox(helper, Main, y, "SolRef1:", _F("%f", gSolRef1), EDITBOX_FLOAT);
	gEditBox_ImpRatio			= CreateEditBox(helper, Main, y, "ImpRatio:", _F("%f", gImpRatio), EDITBOX_FLOAT);
	gEditBox_Margin				= CreateEditBox(helper, Main, y, "Margin:", _F("%f", gMargin), EDITBOX_FLOAT);
	gEditBox_Gap				= CreateEditBox(helper, Main, y, "Gap:", _F("%f", gGap), EDITBOX_FLOAT);
	gEditBox_Kv					= CreateEditBox(helper, Main, y, "Kv multiplier:", _F("%f", gKv), EDITBOX_FLOAT);
	gEditBox_Kp					= CreateEditBox(helper, Main, y, "Kp multiplier:", _F("%f", gKp), EDITBOX_FLOAT);

	gEditBox_Substeps->AddToolTip(gTooltip_Substeps);

	y += YStepCB;

	gCheckBox_Plane				= helper.CreateCheckBox(Main, MUJOCO_GUI_PLANE, 4, y, CheckBoxWidth, 20, "PlaneGeom as ground", gMuJoCoGUI, gPlaneGeomAsGround, gCheckBoxCallback);
	y += YStepCB;
	gCheckBox_MultiCCD			= helper.CreateCheckBox(Main, MUJOCO_GUI_MULTI_CCD, 4, y, CheckBoxWidth, 20, "Multi CCD", gMuJoCoGUI, gMultiCCD, gCheckBoxCallback);
	y += YStepCB;
	gCheckBox_DebugDrawContacts	= helper.CreateCheckBox(Main, MUJOCO_GUI_DEBUG_DRAW_CONTACTS, 4, y, CheckBoxWidth, 20, "Debug draw contacts", gMuJoCoGUI, gDebugDraw_Contacts, gCheckBoxCallback);
	y += YStepCB;
	gCheckBox_DebugDrawAABBs	= helper.CreateCheckBox(Main, MUJOCO_GUI_DEBUG_DRAW_AABBS, 4, y, CheckBoxWidth, 20, "Debug draw AABBs", gMuJoCoGUI, gDebugDraw_AABBs, gCheckBoxCallback);
	y += YStepCB;
	gCheckBox_DebugDrawBodyAxes	= helper.CreateCheckBox(Main, MUJOCO_GUI_DEBUG_DRAW_BODY_AXES, 4, y, CheckBoxWidth, 20, "Debug draw body axes", gMuJoCoGUI, gDebugDraw_BodyAxes, gCheckBoxCallback);
	y += YStepCB;
	gCheckBox_DebugDrawInertia	= helper.CreateCheckBox(Main, MUJOCO_GUI_DEBUG_DRAW_INERTIA, 4, y, CheckBoxWidth, 20, "Debug draw inertia", gMuJoCoGUI, gDebugDraw_Inertia, gCheckBoxCallback);
	y += YStepCB;

	return Main;
}

void MuJoCo_CloseGUI()
{
	Common_CloseGUI(gMuJoCoGUI);
	gEditBox_NbThreads = null;
	gEditBox_Iterations = null;
	gEditBox_LSIterations = null;
	gEditBox_NoSlipIterations = null;
	gEditBox_CCDIterations = null;
	gEditBox_Substeps = null;
	gEditBox_Friction = null;
	gEditBox_FrictionLoss = null;
	gEditBox_Damping = null;
	gEditBox_SolRef0 = null;
	gEditBox_SolRef1 = null;
	gEditBox_ImpRatio = null;
	gEditBox_Margin = null;
	gEditBox_Gap = null;
	gEditBox_Kv = null;
	gEditBox_Kp = null;
	gComboBox_Solver = null;
	gComboBox_Cone = null;
	gComboBox_Jacobian = null;
	gComboBox_Integrator = null;
	gCheckBox_Plane = null;
	gCheckBox_MultiCCD = null;
	gCheckBox_DebugDrawContacts = null;
	gCheckBox_DebugDrawAABBs = null;
	gCheckBox_DebugDrawBodyAxes = null;
	gCheckBox_DebugDrawInertia = null;
}

///////////////////////////////////////////////////////////////////////////////

class MuJoCoPlugIn : public PintPlugin
{
	public:
	virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper)	override	{ return MuJoCo_InitGUI(parent, helper);	}
	virtual	void		CloseGUI()											override	{ MuJoCo_CloseGUI();						}

	virtual	void		Init(const PINT_WORLD_CREATE& desc)					override	{ MuJoCo_Init(desc);						}
	virtual	void		Close()												override	{ MuJoCo_Close();							}

	virtual	Pint*		GetPint()											override	{ return GetMuJoCo();						}

	virtual	IceWindow*	InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)	override;
	virtual	void		CloseTestGUI()																					override;
	virtual	const char*	GetTestGUIName()																				override	{ return "MuJoCo";	}
	virtual	void		ApplyTestUIParams(const char* test_name)														override;
};
static MuJoCoPlugIn gPlugIn;

PintPlugin*	GetPintPlugin()
{
	return &gPlugIn;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateTabWindow(IceWidget* parent, Widgets& owner)
{
	WindowDesc WD;
	WD.mParent	= parent;
	WD.mX		= 0;
	WD.mY		= 0;
	WD.mType	= WINDOW_DIALOG;
	IceWindow* TabWindow = ICE_NEW(IceWindow)(WD);
	owner.Register(TabWindow);
	return TabWindow;
}

static const sdword OffsetX = 90;
static const sdword EditBoxWidth = 60;
static const sdword LabelOffsetY = 2;
static const sdword YStep = 20;
static const sdword LabelWidth = 90;
static const sdword EditBoxX = 100;
static const udword CheckBoxWidth = 200;

static IceCheckBox*	gCheckBox_Override = null;
static IceCheckBox*	gCheckBox_Generic0 = null;
static IceEditBox* gEditBox_Generic0 = null;
static IceEditBox* gEditBox_Generic1 = null;
static IceEditBox* gEditBox_Generic2 = null;
static IceEditBox* gEditBox_Generic3 = null;

void MuJoCoPlugIn::CloseTestGUI()
{
	gCheckBox_Override = null;
	gCheckBox_Generic0 = null;
	gEditBox_Generic0 = null;
	gEditBox_Generic1 = null;
	gEditBox_Generic2 = null;
	gEditBox_Generic3 = null;
}

static IceWindow* CreateUI_AnymalC(IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void AnymalC(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gEditBox_Generic0->SetEnabled(checked);
		gEditBox_Generic1->SetEnabled(checked);
		gEditBox_Generic2->SetEnabled(checked);
		gEditBox_Generic3->SetEnabled(checked);
		gCheckBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::AnymalC, null);
	y += YStep;

	// impratio="100"
	helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 100, 20, "ImpRatio:", &owner);
	ASSERT(!gEditBox_Generic0);
	gEditBox_Generic0 = helper.CreateEditBox(TabWindow, 0, 4+20+EditBoxX, y, EditBoxWidth, 20, "100.0", &owner, EDITBOX_FLOAT, null, null);
	y += YStep;

	// damping="1"
	helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 100, 20, "Damping:", &owner);
	ASSERT(!gEditBox_Generic1);
	gEditBox_Generic1 = helper.CreateEditBox(TabWindow, 0, 4+20+EditBoxX, y, EditBoxWidth, 20, "1.0", &owner, EDITBOX_FLOAT, null, null);
	y += YStep;

	// frictionloss="0.1"
	helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 100, 20, "Friction loss:", &owner);
	ASSERT(!gEditBox_Generic2);
	gEditBox_Generic2 = helper.CreateEditBox(TabWindow, 0, 4+20+EditBoxX, y, EditBoxWidth, 20, "0.1", &owner, EDITBOX_FLOAT, null, null);
	y += YStep;

	// Substeps - I didn't find a way to make it look good enough at 60 Hz so far
	helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 100, 20, "Nb substeps:", &owner);
	ASSERT(!gEditBox_Generic3);
	gEditBox_Generic3 = helper.CreateEditBox(TabWindow, 0, 4+20+EditBoxX, y, EditBoxWidth, 20, "2", &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	y += YStep;

	// cone="elliptic"
	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Force elliptic cone", &owner, true, null, null);
	y += YStep;

	return TabWindow;
}

IceWindow* MuJoCoPlugIn::InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	if(strcmp(test_name, "AnymalC")==0)
		return CreateUI_AnymalC(parent, helper, owner);

	return null;
}

void MuJoCoPlugIn::ApplyTestUIParams(const char* test_name)
{
	const bool ApplySettings = gCheckBox_Override ? gCheckBox_Override->IsChecked() : false;
	if(!ApplySettings)
		return;

	if(strcmp(test_name, "AnymalC")==0)
	{
		Common_GetFromEditBox(gImpRatio, gEditBox_Generic0, -MAX_FLOAT, MAX_FLOAT);
		Common_GetFromEditBox(gDamping, gEditBox_Generic1, -MAX_FLOAT, MAX_FLOAT);
		Common_GetFromEditBox(gFrictionLoss, gEditBox_Generic2, -MAX_FLOAT, MAX_FLOAT);
		if(gCheckBox_Generic0->IsChecked())
			gCone = mjCONE_ELLIPTIC;
		Common_GetFromEditBox(gSubsteps, gEditBox_Generic3);
		return;
	}
}

