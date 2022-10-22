///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// PT: updated with Jolt code from 22/10/2022

/*
Differences with PhysX:
- no support for articulations
- no support for aggregates AFAIK
- no support for gyroscopic forces AFAIK
- only one friction parameter (no support for static/dynamic friction)
- no way to automatically disable collisions between jointed objects
- speculative contact distance is a global value, not automatically adjusted at runtime
- linear CCD & speculative contacts can both be enabled at the same time
- Jolt does support per-joint friction value on regular joints (PhysX only supports it on RC articulation links)
- shapes with non-idt local poses or with a non-zero COM are a special case (same design as Bullet/Havok), while it's the default in PhysX.
- number of iterations are per-scene settings in Jolt, per-actor in PhysX
- supports per-body gravity factor, PhysX doesn't
- collision-groups / sim filtering data is per-shape in PhysX, while it seems per-actor in Jolt
- hinge limits range is only 2*PI in Jolt, twice that in PhysX
- there's no separation between convex/mesh objects and shapes in Jolt
- I think Jolt has zero runtime allocations (JR: actually there is at least 1, but it is very minimal)
- 4 iterations in Jolt is not enough, some stacks collapse, joints feel too soft (low Baumgarte by default)
- Jolt uses AVX2/AVX/SSE4, PhysX sticks to SSE2 (JR: Jolt supports SSE2 now too)

TODO:
- more per-test UI
- vehicles, CCTs
- prismatic springs
- finish joints
- overlaps/sweeps
- try path constraint
- make rectangle selection work with Jolt

"The higher the frequency, the quicker the body will move to its target. If you set a frequency of 1 then it will oscillate 1 time per second around the target position,
if you set it to 20 it will oscillate 20 times per second around the target position. You should never go higher than half of your simulation frequency (otherwise: instability).

Damping prevents overshoot. A damping of 0 means that the body will oscillate forever around its target position (in theory that is, with the integrator that Jolt uses that won't happen).
A damping of 1 will barely overshoot the target and have almost no oscillation, but it will also take slightly longer to reach the target. I would say sensible ranges for frequency are 0.1-20 and for damping 0-1."
*/

#include "PINT_Jolt.h"

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/PointConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Constraints/SliderConstraint.h>
#include <Jolt/Physics/Constraints/SixDOFConstraint.h>
#include <Jolt/Physics/Constraints/RackAndPinionConstraint.h>
#include <Jolt/Physics/Constraints/GearConstraint.h>

#include "..\PINT_Common\PINT_Ice.h"
//#include "..\PINT_Common\PINT_Common.cpp"

#include "..\PINT_Common\PINT_Common.h"
//#include "..\PINT_Common\PINT_IceAllocatorSwitch.h"
#include "..\PintShapeRenderer.h"

#include <iostream>
#include <cstdarg>
#include <thread>
#include <fstream>

inline_ Point	ToPoint(const Vec3& p)	{ return Point(p.GetX(), p.GetY(), p.GetZ());	}
inline_ Vec3	ToVec3(const Point& p)	{ return Vec3(p.x, p.y, p.z);					}

typedef JPH::Quat		JQuat;
typedef IceMaths::Quat	IQuat;
inline_ IQuat	ToIQuat(const JQuat& q)	{ return IQuat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());	}
inline_ JQuat	ToJQuat(const IQuat& q)	{ return JQuat(q.p.x, q.p.y, q.p.z, q.w);				}

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char *inFMT, ...)
{ 
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);

	// Print to the TTY
	cout << buffer << endl;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN64
	#pragma comment(lib, "../../Ice/Lib64/IceCore64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceMaths64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Contact64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Meshmerizer64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceImageWork64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceGUI64.lib")
//	#pragma comment(lib, "../../Ice/Lib64/IML64.lib")

	#ifdef _DEBUG
		#pragma comment(lib, "../../../../PEEL_Externals/Jolt/Lib/x64/Debug/Jolt.lib")
	#else
		#pragma comment(lib, "../../../../PEEL_Externals/Jolt/Lib/x64/Release/Jolt.lib")
	#endif
#else
	#pragma comment(lib, "../../Ice/Lib/IceCore.lib")
	#pragma comment(lib, "../../Ice/Lib/IceMaths.lib")
	#pragma comment(lib, "../../Ice/Lib/Contact.lib")
	#pragma comment(lib, "../../Ice/Lib/Meshmerizer.lib")
	#pragma comment(lib, "../../Ice/Lib/IceImageWork.lib")
	#pragma comment(lib, "../../Ice/Lib/IceGUI.lib")
	#pragma comment(lib, "../../Ice/Lib/IML.lib")
#endif

///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include <limits>

///////////////////////////////////////////////////////////////////////////////

static bool		gEnableCCD					= false;
static bool		gAllowSleeping				= true;
static bool		gAllowShapeSharing			= true;
static bool		gBackfaceCulling			= true;
static udword	gNbThreads					= 0;
static udword	gNbSubsteps					= 1;
static udword	gTempAllocSize				= 100;		// Mb
static udword	gMaxBodies					= 65536;	// "This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error. For a real project use something in the order of 65536."
static udword	gMaxBodyPairs				= 65536;	// "This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping body pairs based on their bounding boxes and will insert them into a queue for the narrowphase).
														// If you make this buffer too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient. For a real project use something in the order of 65536."
static udword	gMaxContactConstraints		= 65536;	// "This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this number then these contacts will be ignored and bodies will start
														// interpenetrating / fall through the world. For a real project use something in the order of 65536."
static udword	gNbBodyMutexes				= 0;		// "This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings."
static udword	gNbPosIter					= 2;		// Default value in Jolt = 2
static udword	gNbVelIter					= 10;		// Default value in Jolt = 10. "Note that this needs to be >= 2 in order for friction to work (friction is applied using the non-penetration impulse from the previous iteration)"
static float	gLinearDamping				= 0.1f;		// Same default value as in PEEL. Jolt default is 0.05.
static float	gAngularDamping				= 0.05f;	// Same default value as in PEEL.
static float	gSpeculativeContactDistance	= 0.02f;	// Default value in Jolt
static float	gPenetrationSlop			= 0.02f;	// Default value in Jolt
static float	gBaumgarte					= 0.2f;		// Default value in Jolt
static float	gDefaultFriction			= 0.5f;		// Same default value as in PEEL. Default value in Jolt = 0.2
static float	gDefaultRestitution			= 0.0f;		// Default value in Jolt

///////////////////////////////////////////////////////////////////////////////

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{ 
	// Print to the TTY
	cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << endl;

	// Breakpoint
	return true;
};

#endif // JPH_ENABLE_ASSERTS

////

/// Layer that objects can be in, determines which other objects it can collide with
namespace Layers
{
	static constexpr uint8 NON_MOVING = 0;
	static constexpr uint8 MOVING = 1;
	static constexpr uint8 NUM_LAYERS = 2;
};

/// Function that determines if two object layers can collide
inline bool MyObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2)
{
	switch (inObject1)
	{
	case Layers::NON_MOVING:
		return inObject2 == Layers::MOVING;
	case Layers::MOVING:
		return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING;
	default:
		JPH_ASSERT(false);
		return false;
	}
};

/// Broadphase layers
namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr uint NUM_LAYERS(2);
};

/// BroadPhaseLayerInterface implementation
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
									BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual uint					GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char *			GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		switch ((BroadPhaseLayer::Type)inLayer)
		{
		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
}gBroadPhaseLayerInterface;

/// Function that determines if two broadphase layers can collide
inline bool MyBroadPhaseCanCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2)
{
	switch (inLayer1)
	{
	case Layers::NON_MOVING:
		return inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::MOVING:
		return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING;
	default:
		JPH_ASSERT(false);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////

static JobSystemThreadPool* gJobSystem = null;
static PhysicsSystem* gPhysicsSystem = null;

// Custom filtering, based on Jolt's GroupFilterTable code + new support for filtering of jointed bodies
class MyGroupFilterTable final : public GroupFilter
{
private:
	using GroupID = CollisionGroup::GroupID;
	using SubGroupID = CollisionGroup::SubGroupID;

public:
	MyGroupFilterTable()
	{
		for(int i=0;i<32;i++)
			for(int j=0;j<32;j++)
				mMyTable[j][i]=true;
	}

	~MyGroupFilterTable()
	{
		DELETESINGLE(mPM);
	}

	void	DisableCollision(SubGroupID inSubGroup1, SubGroupID inSubGroup2)
	{
		mMyTable[inSubGroup1][inSubGroup2] = mMyTable[inSubGroup2][inSubGroup1] = false;
	}

	void	EnableCollision(SubGroupID inSubGroup1, SubGroupID inSubGroup2)
	{
		mMyTable[inSubGroup1][inSubGroup2] = mMyTable[inSubGroup2][inSubGroup1] = true;
	}

	void	DisableJointedBodies(udword id0, udword id1)
	{
		if(!mPM)
			mPM = new PairManager;
		mPM->AddPair(id0, id1);
	}

	virtual bool	CanCollide(const CollisionGroup &inGroup1, const CollisionGroup &inGroup2) const override
	{	
		// Filter bodies connected by a joint
		if(mPM && mPM->FindPair(inGroup1.GetGroupID(), inGroup2.GetGroupID()))
			return false;

		return mMyTable[inGroup1.GetSubGroupID()][inGroup2.GetSubGroupID()];
	}

private:
	bool			mMyTable[32][32];
	PairManager*	mPM = nullptr;
};

static Ref<MyGroupFilterTable> gGroupFilter;

class MyTempAllocatorImpl final : public TempAllocator
{
public:
	/// Constructs the allocator with a maximum allocatable size of inSize
	explicit						MyTempAllocatorImpl(uint inSize) :
		mBase(static_cast<uint8 *>(malloc(inSize))),
		mSize(inSize)
	{
	}

	/// Destructor, frees the block
	virtual							~MyTempAllocatorImpl() override
	{
		JPH_ASSERT(mTop == 0);
		free(mBase);
	}

	// See: TempAllocator
	virtual void *					Allocate(uint inSize) override
	{
		if (inSize == 0)
		{
			return nullptr;
		}
		else
		{
			uint new_top = mTop + AlignUp(inSize, 16);
			if (new_top > mSize)
				JPH_CRASH; // Out of memory
			void *address = mBase + mTop;
			mTop = new_top;
			mHighWaterMark = TMax(mHighWaterMark, mTop);
			return address;
		}
	}

	// See: TempAllocator
	virtual void					Free(void *inAddress, uint inSize) override
	{
		if (inAddress == nullptr)
		{
			JPH_ASSERT(inSize == 0);
		}
		else
		{
			mTop -= AlignUp(inSize, 16);
			if (mBase + mTop != inAddress)
				JPH_CRASH; // Freeing in the wrong order
		}
	}

	uint8*	mBase;				///< Base address of the memory block
	uint	mSize;				///< Size of the memory block
	uint	mTop = 0;			///< Current top of the stack
	uint	mHighWaterMark = 0;
};

static MyTempAllocatorImpl* gTempAllocator = null;

///////////////////////////////////////////////////////////////////////////////

atomic<size_t> gCurrentMemory = 0;
atomic<size_t> gMaxMemory = 0;

// Add a tag to an allocation to track its size
static void *TagAllocation(void *inPointer, size_t inAlignment, size_t inSize)
{
	// Update current memory consumption
	size_t CurrentMemory = gCurrentMemory.fetch_add(inSize) + inSize;

	// Update max memory
	size_t MaxMemory = gMaxMemory;
	while (MaxMemory < CurrentMemory && !gMaxMemory.compare_exchange_weak(MaxMemory, CurrentMemory)) { };

	// Store size
	*reinterpret_cast<size_t *>(inPointer) = inSize;

	// Return actual block
	return reinterpret_cast<uint8*>(inPointer) + inAlignment;
}

// Remove tag from allocation
static void *UntagAllocation(void *inPointer, size_t inAlignment)
{
	uint8* p = reinterpret_cast<uint8*>(inPointer) - inAlignment;

	// Update current memory
	gCurrentMemory -= *reinterpret_cast<size_t*>(p);

	return p;
}

static void *AllocateHook(size_t inSize)
{
	return TagAllocation(malloc(inSize + 16), 16, inSize);
}

static void FreeHook(void *inBlock)
{
	free(UntagAllocation(inBlock, 16));
}

static void *AlignedAllocateHook(size_t inSize, size_t inAlignment)
{
	ASSERT(inAlignment <= 64);
	return TagAllocation(_aligned_malloc(inSize + 64, inAlignment), 64, inSize);
}

static void AlignedFreeHook(void *inBlock)
{
	_aligned_free(UntagAllocation(inBlock, 64));
}

///////////////////////////////////////////////////////////////////////////////

Jolt_SceneAPI::Jolt_SceneAPI(Pint& pint) : Pint_Scene(pint)
{
}

Jolt_SceneAPI::~Jolt_SceneAPI()
{
}

bool Jolt_SceneAPI::AddActors(udword nb_actors, const PintActorHandle* actors)
{
	BodyInterface& body_interface = gPhysicsSystem->GetBodyInterface();

	while(nb_actors--)
	{
		Body* CurrentActor = reinterpret_cast<Body*>(*actors++);
		body_interface.AddBody(CurrentActor->GetID(), CurrentActor->IsStatic() ? EActivation::DontActivate : EActivation::Activate);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

Jolt_ActorAPI::Jolt_ActorAPI(Pint& pint) : Pint_Actor(pint)
{
}

Jolt_ActorAPI::~Jolt_ActorAPI()
{
}

const char* Jolt_ActorAPI::GetName(PintActorHandle handle) const
{
	const Body* Actor = reinterpret_cast<const Body*>(handle);
	ASSERT(Actor);
	return null;
}

bool Jolt_ActorAPI::SetName(PintActorHandle handle, const char* name)
{
	Body* Actor = reinterpret_cast<Body*>(handle);
	ASSERT(Actor);
	return false;
}

bool Jolt_ActorAPI::GetWorldBounds(PintActorHandle handle, AABB& bounds) const
{
	const Body* Actor = reinterpret_cast<const Body*>(handle);
	ASSERT(Actor);

	const AABox& Bounds = Actor->GetWorldSpaceBounds();
	bounds.mMin = ToPoint(Bounds.mMin);
	bounds.mMax = ToPoint(Bounds.mMax);
	return true;
}

static MotionProperties* GetDynamicData(PintActorHandle handle)
{
	Body* Actor = reinterpret_cast<Body*>(handle);
	ASSERT(Actor);

	if(Actor->IsStatic())
		return null;

	MotionProperties* MP = Actor->GetMotionProperties();
	if(!MP)
	{
		printf("Motion properties not available!\n");
		return null;
	}
	return MP;
}

float Jolt_ActorAPI::GetLinearDamping(PintActorHandle handle) const
{
	const MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return 0.0f;

	return MP->GetLinearDamping();
}

bool Jolt_ActorAPI::SetLinearDamping(PintActorHandle handle, float damping)
{
	MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return false;

	MP->SetLinearDamping(damping);
	return true;
}

float Jolt_ActorAPI::GetAngularDamping(PintActorHandle handle) const
{
	const MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return 0.0f;

	return MP->GetAngularDamping();
}

bool Jolt_ActorAPI::SetAngularDamping(PintActorHandle handle, float damping)
{
	MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return false;

	MP->SetAngularDamping(damping);
	return true;
}

bool Jolt_ActorAPI::GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space) const
{
	ASSERT(world_space);

	const MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
	{
		linear_velocity.Zero();
		return false;
	}

	linear_velocity = ToPoint(MP->GetLinearVelocity());
	return true;
}

bool Jolt_ActorAPI::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)
{
	ASSERT(world_space);

	MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return false;

	MP->SetLinearVelocityClamped(ToVec3(linear_velocity));
	return true;
}

bool Jolt_ActorAPI::GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space) const
{
	ASSERT(world_space);

	const MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
	{
		angular_velocity.Zero();
		return false;
	}

	angular_velocity = ToPoint(MP->GetAngularVelocity());
	return true;
}

bool Jolt_ActorAPI::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)
{
	ASSERT(world_space);

	MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return false;

	MP->SetAngularVelocityClamped(ToVec3(angular_velocity));
	return true;
}

float Jolt_ActorAPI::GetMass(PintActorHandle handle) const
{
	const MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return 0.0f;

	return 1.0f / MP->GetInverseMass();
}

bool Jolt_ActorAPI::GetLocalInertia(PintActorHandle handle, Point& inertia) const
{
	const MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return false;

	const Vec3 inv = MP->GetInverseInertiaDiagonal();
	inertia = Point(1.0f/inv.GetX(), 1.0f/inv.GetY(), 1.0f/inv.GetZ());
	return true;
}

///////////////////////////////////////////////////////////////////////////////

JoltPint::JoltPint() : mSceneAPI(*this), mActorAPI(*this)
{
}

JoltPint::~JoltPint()
{
}

const char* JoltPint::GetName() const
{
	const int inNumThreads = gNbThreads ? gNbThreads : thread::hardware_concurrency() - 1;

	const char* Name = "Jolt";

	return _F("%s (%dT)", Name, inNumThreads);
}

const char* JoltPint::GetUIName() const
{
	return "Jolt";
}

void JoltPint::GetCaps(PintCaps& caps) const
{
	caps.mSupportRigidBodySimulation	= true;
	caps.mSupportCylinders				= true;
	caps.mSupportConvexes				= true;
	caps.mSupportMeshes					= true;
/*	caps.mSupportDeformableMeshes		= true;*/
	caps.mSupportHeightfields			= true;
/*	caps.mSupportContactNotifications	= true;
	caps.mSupportContactModifications	= true;
	caps.mSupportMassForInertia			= true;*/
	caps.mSupportKinematics				= true;
	caps.mSupportCollisionGroups		= true;
	caps.mSupportCompounds				= true;
	caps.mSupportAggregates				= true;	// Pretend we do

	caps.mSupportSphericalJoints		= true;
	caps.mSupportHingeJoints			= true;
	caps.mSupportFixedJoints			= true;
	caps.mSupportPrismaticJoints		= true;
	caps.mSupportDistanceJoints			= true;
	caps.mSupportD6Joints				= true;
	caps.mSupportGearJoints				= true;
	caps.mSupportRackJoints				= true;
/*	caps.mSupportPortalJoints			= true;
	caps.mSupportMCArticulations		= false;
	caps.mSupportRCArticulations		= true;*/

	caps.mSupportRaycasts				= true;

	caps.mSupportBoxSweeps				= true;
	caps.mSupportSphereSweeps			= true;
	caps.mSupportCapsuleSweeps			= true;
/*	caps.mSupportConvexSweeps			= true;*/

	caps.mSupportSphereOverlaps			= true;
/*	caps.mSupportBoxOverlaps			= true;
	caps.mSupportCapsuleOverlaps		= true;
	caps.mSupportConvexOverlaps			= true;
	caps.mSupportMeshMeshOverlaps		= true;

	caps.mSupportVehicles				= true;
	caps.mSupportCharacters				= true;*/
}

void JoltPint::Init(const PINT_WORLD_CREATE& desc)
{
	// Override the default allocators to measure memory consumption
	Allocate = AllocateHook;
	Free = FreeHook;
	AlignedAllocate = AlignedAllocateHook;
	AlignedFree = AlignedFreeHook;

	// Install callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

	// Create a new factory
	Factory::sInstance = new Factory;

	// Register all Jolt physics types
	RegisterTypes();

	// We need a temp allocator for temporary allocations during the physics update. We're
	// pre-allocating 10 MB to avoid having to do allocations during the physics update. 
	// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
	// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
	// malloc / free.
	gTempAllocator = new MyTempAllocatorImpl(gTempAllocSize * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	const int inNumThreads = gNbThreads ? gNbThreads : thread::hardware_concurrency() - 1;
	gJobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, inNumThreads);

	gGroupFilter = new MyGroupFilterTable();

	// Now we can create the actual physics system.
	gPhysicsSystem = new PhysicsSystem;
	gPhysicsSystem->Init(gMaxBodies, gNbBodyMutexes, gMaxBodyPairs, gMaxContactConstraints, gBroadPhaseLayerInterface, MyBroadPhaseCanCollide, MyObjectCanCollide);

	gPhysicsSystem->SetGravity(ToVec3(desc.mGravity));

	PhysicsSettings Settings;
	Settings.mNumVelocitySteps				= gNbVelIter;
	Settings.mNumPositionSteps				= gNbPosIter;
	Settings.mAllowSleeping					= gAllowSleeping;
	Settings.mSpeculativeContactDistance	= gSpeculativeContactDistance;
	Settings.mPenetrationSlop				= gPenetrationSlop;
	Settings.mBaumgarte						= gBaumgarte;

	gPhysicsSystem->SetPhysicsSettings(Settings);
}

void JoltPint::Close()
{
	DELETESINGLE(gPhysicsSystem);
	gGroupFilter = nullptr;
	DELETESINGLE(gJobSystem);
	DELETESINGLE(gTempAllocator);
	DELETESINGLE(Factory::sInstance);
}

void JoltPint::SetGravity(const Point& gravity)
{
	if(gPhysicsSystem)
		gPhysicsSystem->SetGravity(ToVec3(gravity));
}

udword JoltPint::Update(float dt)
{
	// Reset high watermark for this frame
	gTempAllocator->mHighWaterMark = gTempAllocator->mTop;
	gMaxMemory = gCurrentMemory.load();

	if(gPhysicsSystem)
	{
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int cCollisionSteps = 1;

		// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
		const int cIntegrationSubSteps = gNbSubsteps;

		// Step the world
		gPhysicsSystem->Update(dt, cCollisionSteps, cIntegrationSubSteps, gTempAllocator, gJobSystem);
	}

	// Return high watermark for memory consumption
	return udword(gTempAllocator->mHighWaterMark + gMaxMemory);
}

Point JoltPint::GetMainColor()
{
	return Point(0.5f, 0.9f, 0.8f);
}

static inline_ void BindRenderer(JPH::Shape* shape, PintShapeRenderer* renderer)
{
	shape->SetUserData(uint64(renderer));
}

static inline_ PintShapeRenderer* RetrieveRenderer(const JoltPint&, const JPH::Shape*, uint64 user_data)
{
	return reinterpret_cast<PintShapeRenderer*>(user_data);
}

void JoltPint::Render(PintRender& renderer, PintRenderPass render_pass)
{
	if(!gPhysicsSystem)
		return;

	const BodyLockInterface &BLI = gPhysicsSystem->GetBodyLockInterfaceNoLock();

	AllHitCollisionCollector<TransformedShapeCollector> Collector;
	
	BodyIDVector BodyIDs;
	gPhysicsSystem->GetBodies(BodyIDs);
	for (BodyID ID : BodyIDs)
	{
		BodyLockRead Lock(BLI, ID);
		if (Lock.SucceededAndIsInBroadPhase())
		{
			const Body* Current = &Lock.GetBody();
			if(!renderer.SetCurrentActor(PintActorHandle(Current)))
				continue;

			// Collect all leaf shapes
			Collector.Reset();
			Current->GetShape()->CollectTransformedShapes(AABox(Vec3::sReplicate(-1.0e6f), Vec3::sReplicate(1.0e6f)), Current->GetCenterOfMassPosition(), Current->GetRotation(), JPH::Vec3::sReplicate(1.0f), JPH::SubShapeIDCreator(), Collector, {});

			// Render them
			for (const TransformedShape& TS : Collector.mHits)
			{
				PintShapeRenderer* Renderer = RetrieveRenderer(*this, TS.mShape, TS.mShape->GetUserData());
				if(Renderer)
				{
					const PR Pose(ToPoint(TS.mShapePositionCOM - TS.mShapeRotation * TS.mShape->GetCenterOfMass()), ToIQuat(TS.mShapeRotation));
					renderer.DrawShape(Renderer, Pose);
				}
			}
		}
	}
}

void JoltPint::RenderDebugData(PintRender& renderer)
{
}

static void SetupMaterial(BodyCreationSettings& settings, const PINT_SHAPE_CREATE* create)
{
	if(create->mMaterial)
	{
		settings.mRestitution = create->mMaterial->mRestitution;
		settings.mFriction = create->mMaterial->mDynamicFriction;
	}
	else
	{
		settings.mRestitution = gDefaultRestitution;
		settings.mFriction = gDefaultFriction;
	}
}

static void SetupStaticActorSettings(BodyCreationSettings& settings, const PINT_SHAPE_CREATE* create)
{
	SetupMaterial(settings, create);
}

static void SetupDynamicActorSettings(BodyCreationSettings& settings, const PINT_SHAPE_CREATE* create, const PINT_OBJECT_CREATE& desc)
{
	SetupMaterial(settings, create);

	settings.mMotionQuality = gEnableCCD ? EMotionQuality::LinearCast : EMotionQuality::Discrete;

	settings.mAllowSleeping = gAllowSleeping;
	settings.mLinearDamping = gLinearDamping;
	settings.mAngularDamping = gAngularDamping;

	settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	settings.mMassPropertiesOverride.mMass = desc.mMass;

	settings.mMaxLinearVelocity = MAX_FLOAT;	// Some PEEL tests require more than Jolt's refault
	settings.mMaxAngularVelocity = 100.0f;
	settings.mLinearVelocity = ToVec3(desc.mLinearVelocity);
	settings.mAngularVelocity = ToVec3(desc.mAngularVelocity);
}

static void SetupOffsetShape(Ref<JPH::Shape>& shape, const PR& local_pose)
{
	if(!local_pose.IsIdentity())
	{
		const RotatedTranslatedShapeSettings OffsetShapeSettings(ToVec3(local_pose.mPos), ToJQuat(local_pose.mRot), shape);

		const RotatedTranslatedShapeSettings::ShapeResult r = OffsetShapeSettings.Create();
		if(r.IsValid())
			shape = r.Get();
		else
			ASSERT(0);
	}
}

static Ref<JPH::Shape> CreateMeshShape(const SurfaceInterface& surface)
{
	VertexList pts;
	for(udword i=0;i<surface.mNbVerts;i++)
		pts.push_back(Float3(surface.mVerts[i].x, surface.mVerts[i].y, surface.mVerts[i].z));

	IndexedTriangleList tris;
	for(udword i=0;i<surface.mNbFaces;i++)
	{
		udword VRef0, VRef1, VRef2;
		if(surface.mDFaces)
		{
			VRef0 = surface.mDFaces[i*3+0];
			VRef1 = surface.mDFaces[i*3+1];
			VRef2 = surface.mDFaces[i*3+2];
		}
		else
		{
			VRef0 = surface.mWFaces[i*3+0];
			VRef1 = surface.mWFaces[i*3+1];
			VRef2 = surface.mWFaces[i*3+2];
		}
		tris.push_back(JPH::IndexedTriangle(VRef0, VRef1, VRef2));
	}

	MeshShapeSettings settings(pts, tris);
	settings.mMaxTrianglesPerLeaf = 4;

	return settings.Create().Get();
}

Ref<JPH::Shape> JoltPint::CreateShape(const PINT_SHAPE_CREATE* shape_create)
{
	Ref<JPH::Shape> shape;

	PintShapeRenderer* Renderer = shape_create->mRenderer;
	const PR LocalPose(shape_create->mLocalPos, shape_create->mLocalRot);

	// Check if we can share the shape
	bool AllowSharing = shape_create->CanShare(gAllowShapeSharing);
	if (AllowSharing)
		for (CachedShape &S : mCachedShapes)
			if (S.mRenderer == Renderer && S.mLocalPose.mPos == LocalPose.mPos && S.mLocalPose.mRot == LocalPose.mRot)
				return S.mShape;

	switch(shape_create->mType)
	{
		case PINT_SHAPE_SPHERE:
		{
			const PINT_SPHERE_CREATE* Create = static_cast<const PINT_SPHERE_CREATE*>(shape_create);

			shape = new SphereShape(Create->mRadius);
		}
		break;

		case PINT_SHAPE_CAPSULE:
		{
			const PINT_CAPSULE_CREATE* Create = static_cast<const PINT_CAPSULE_CREATE*>(shape_create);

			shape = new CapsuleShape(Create->mHalfHeight, Create->mRadius);
		}
		break;

		case PINT_SHAPE_CYLINDER:
		{
			const PINT_CYLINDER_CREATE* Create = static_cast<const PINT_CYLINDER_CREATE*>(shape_create);

			shape = new CylinderShape(Create->mHalfHeight, Create->mRadius, TMin(cDefaultConvexRadius, Create->mHalfHeight));
		}
		break;

		case PINT_SHAPE_BOX:
		{
			const PINT_BOX_CREATE* Create = static_cast<const PINT_BOX_CREATE*>(shape_create);

			const Vec3 Extents = ToVec3(Create->mExtents);
			shape = new BoxShape(Extents, TMin(cDefaultConvexRadius, Extents.ReduceMin()));
		}
		break;

		case PINT_SHAPE_CONVEX:
		{
			const PINT_CONVEX_CREATE* Create = static_cast<const PINT_CONVEX_CREATE*>(shape_create);

			Array<Vec3> pts;
			pts.reserve(Create->mNbVerts);
			for(udword i=0;i<Create->mNbVerts;i++)
				pts.push_back(ToVec3(Create->mVerts[i]));

			const ConvexHullShapeSettings settings(pts, cDefaultConvexRadius);
			ConvexHullShapeSettings::ShapeResult r = settings.Create();
			if(r.IsValid())
				shape = r.Get();
			else
			{
				// Determine average position
				Vec3 Avg = Vec3::sZero();
				for (Vec3 P : pts)
					Avg += P;
				Avg = Avg / float(pts.size());

				// Determine radius
				float RadiusSq = 0.0f;
				for (Vec3 P : pts)
					RadiusSq = max(RadiusSq, (P - Avg).LengthSq());

				// We should only take this path for very small hulls that are below the hull tolerance
				ASSERT(RadiusSq <= Square(settings.mHullTolerance))

				// Hull could not be built, create a sphere instead
				shape = new SphereShape(sqrt(RadiusSq));
				if (!Avg.IsNearZero())
				{
					RotatedTranslatedShapeSettings sphere_settings(Avg, JQuat::sIdentity(), shape);
					shape = sphere_settings.Create().Get();
				}
			}
		}
		break;

		case PINT_SHAPE_MESH:
		{
			const PINT_MESH_CREATE* Create = static_cast<const PINT_MESH_CREATE*>(shape_create);

			shape = CreateMeshShape(Create->GetSurface());
		}
		break;

		case PINT_SHAPE_MESH2:
		{
			const PINT_MESH_CREATE2* Create = static_cast<const PINT_MESH_CREATE2*>(shape_create);

			const IndexedSurface* IS = reinterpret_cast<const IndexedSurface*>(Create->mTriangleMesh);
			shape = CreateMeshShape(IS->GetSurfaceInterface());
		}
		break;

		case PINT_SHAPE_HEIGHTFIELD:
		{
			const PINT_HEIGHTFIELD_CREATE* Create = static_cast<const PINT_HEIGHTFIELD_CREATE *>(shape_create);

			const HeightFieldShapeSettings* HF = reinterpret_cast<HeightFieldShapeSettings *>(Create->mHeightfield);
			ASSERT(HF);

			HeightFieldShapeSettings Copy = *HF;
			Copy.mScale *= Vec3(Create->mScaleV, 1.0f, Create->mScaleU);
			shape = Copy.Create().Get();
		}
		break;

		default:
			ASSERT(0);
		break;
	};

	BindRenderer(shape, Renderer);

	SetupOffsetShape(shape, LocalPose);

	// Remember the shape for sharing
	if (AllowSharing)
		mCachedShapes.push_back({ Renderer, LocalPose, shape });

	return shape;
}

PintActorHandle JoltPint::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	ASSERT(gPhysicsSystem);
	if(!gPhysicsSystem)
		return null;

	const udword NbShapes = desc.GetNbShapes();
	if(!NbShapes)
		return null;

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	BodyInterface& body_interface = gPhysicsSystem->GetBodyInterface();

	const PINT_SHAPE_CREATE* ShapeCreate = desc.GetFirstShape();

	Ref<JPH::Shape> NewShape;
	if(NbShapes==1)
	{
		NewShape = CreateShape(ShapeCreate);
	}
	else
	{
		class MyPintShapeEnumerateCallback : public PintShapeEnumerateCallback
		{
			public:
					MyPintShapeEnumerateCallback(JoltPint& pint) : mPint(pint) {}
			virtual	~MyPintShapeEnumerateCallback() = default;

			virtual	void	ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)
			{
				Ref<JPH::Shape> NewShape = mPint.CreateShape(&create);

				if (NewShape->GetSubType() == EShapeSubType::RotatedTranslated)
				{
					RotatedTranslatedShape* RTShape = static_cast<RotatedTranslatedShape*>(NewShape.GetPtr());
					mCompoundShape.AddShape(RTShape->GetPosition(), RTShape->GetRotation(), RTShape->GetInnerShape());
				}
				else
				{
					mCompoundShape.AddShape(Vec3::sZero(), JQuat::sIdentity(), NewShape);
				}
			}

			JoltPint&					mPint;
			StaticCompoundShapeSettings	mCompoundShape;
		};

		MyPintShapeEnumerateCallback CB(*this);
		desc.GetNbShapes(&CB);

		StaticCompoundShapeSettings::ShapeResult r = CB.mCompoundShape.Create();
		NewShape = r.Get();
	}

	// Offset COM if requested
	if (desc.mCOMLocalOffset.IsNonZero())
	{
		OffsetCenterOfMassShapeSettings settings(ToVec3(desc.mCOMLocalOffset), NewShape);
		NewShape = settings.Create().Get();
	}

	const Vec3 Pos(ToVec3(desc.mPosition));
	const JQuat Rot(ToJQuat(desc.mRotation));

	Body* NewBody = null;
	const bool IsDynamic = desc.mMass!=0.0f;

	if(NewShape)
	{
		if(!IsDynamic)
		{
			BodyCreationSettings BodyCreate(NewShape, Pos, Rot, EMotionType::Static, Layers::NON_MOVING);
			SetupStaticActorSettings(BodyCreate, ShapeCreate);
			NewBody = body_interface.CreateBody(BodyCreate);
		}
		else
		{
			const EMotionType MT = desc.mKinematic ? EMotionType::Kinematic : EMotionType::Dynamic;
			BodyCreationSettings BodyCreate(NewShape, Pos, Rot, MT, Layers::MOVING);
			SetupDynamicActorSettings(BodyCreate, ShapeCreate, desc);
			NewBody = body_interface.CreateBody(BodyCreate);
		}
	}

	if(NewBody)
	{
		if(desc.mAddToWorld)
			body_interface.AddBody(NewBody->GetID(), IsDynamic ? EActivation::Activate : EActivation::DontActivate);

		// TODO: keeping the sequence number for now because I didn't bother flushing the map when objects are deleted. A proper implementation would revisit this.
		NewBody->SetCollisionGroup(CollisionGroup(gGroupFilter, NewBody->GetID().GetIndexAndSequenceNumber(), CollisionGroup::SubGroupID(desc.mCollisionGroup)));
	}

	return PintActorHandle(NewBody);
}

bool JoltPint::ReleaseObject(PintActorHandle handle)
{
	Body* Actor = reinterpret_cast<Body*>(handle);
	ASSERT(Actor);

	BodyInterface& body_interface = gPhysicsSystem->GetBodyInterface();

	BodyID ID = Actor->GetID();
	// Remove the body from the physics system. Note that the body itself keeps all of its state and can be re-added at any time.
	body_interface.RemoveBody(ID);
	// Destroy the body. After this the body ID is no longer valid.
	body_interface.DestroyBody(ID);

	return true;
}

PintJointHandle JoltPint::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	Body* Actor0 = reinterpret_cast<Body*>(desc.mObject0);
	Body* Actor1 = reinterpret_cast<Body*>(desc.mObject1);

	if(!Actor0)
		Actor0 = &Body::sFixedToWorld;
	if(!Actor1)
		Actor1 = &Body::sFixedToWorld;

	Constraint* J = null;

	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			const PINT_SPHERICAL_JOINT_CREATE& jc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);

			PointConstraintSettings settings;
			settings.mSpace = EConstraintSpace::LocalToBodyCOM;
			settings.mPoint1 = ToVec3(jc.mLocalPivot0.mPos) - Actor0->GetShape()->GetCenterOfMass();
			settings.mPoint2 = ToVec3(jc.mLocalPivot1.mPos) - Actor1->GetShape()->GetCenterOfMass();

			J = settings.Create(*Actor0, *Actor1);
		}
		break;

		case PINT_JOINT_HINGE:
		{
			const PINT_HINGE_JOINT_CREATE& jc = static_cast<const PINT_HINGE_JOINT_CREATE&>(desc);

			HingeConstraintSettings settings;
			settings.mSpace = EConstraintSpace::LocalToBodyCOM;
			settings.mPoint1 = ToVec3(jc.mLocalPivot0) - Actor0->GetShape()->GetCenterOfMass();
			settings.mPoint2 = ToVec3(jc.mLocalPivot1) - Actor1->GetShape()->GetCenterOfMass();

			settings.mHingeAxis1 = ToVec3(jc.mLocalAxis0);
			settings.mHingeAxis2 = ToVec3(jc.mLocalAxis1);

			settings.mNormalAxis1 = settings.mHingeAxis1.GetNormalizedPerpendicular();
			settings.mNormalAxis2 = (Actor1->GetInverseCenterOfMassTransform() * Actor0->GetCenterOfMassTransform()).Multiply3x3(settings.mNormalAxis1).Normalized();

			if(IsHingeLimitEnabled(jc.mLimits))
			{
				if(jc.mLimits.mMinValue<-MAX_FLOAT*0.5f && jc.mLimits.mMaxValue>MAX_FLOAT*0.5f)
				{
				}
				else
				{
					settings.mLimitsMin = jc.mLimits.mMinValue;
					settings.mLimitsMax = jc.mLimits.mMaxValue;
				}
			}

			HingeConstraint* NewJoint = static_cast<HingeConstraint*>(settings.Create(*Actor0, *Actor1));
			J = NewJoint;

			if(jc.mUseMotor)
			{
				NewJoint->SetMotorState(EMotorState::Velocity);
				NewJoint->SetTargetAngularVelocity(jc.mDriveVelocity);
			}
		}
		break;

		case PINT_JOINT_HINGE2:
		{
			const PINT_HINGE2_JOINT_CREATE& jc = static_cast<const PINT_HINGE2_JOINT_CREATE&>(desc);

			HingeConstraintSettings settings;
			settings.mSpace = EConstraintSpace::LocalToBodyCOM;
			settings.mPoint1 = ToVec3(jc.mLocalPivot0.mPos) - Actor0->GetShape()->GetCenterOfMass();
			settings.mPoint2 = ToVec3(jc.mLocalPivot1.mPos) - Actor1->GetShape()->GetCenterOfMass();

			const Matrix3x3 LocalFrame0 = jc.mLocalPivot0.mRot;
			const Matrix3x3 LocalFrame1 = jc.mLocalPivot1.mRot;
			settings.mHingeAxis1 = ToVec3(LocalFrame0[0]);
			settings.mHingeAxis2 = ToVec3(LocalFrame1[0]);
			settings.mNormalAxis1 = ToVec3(LocalFrame0[1]);
			settings.mNormalAxis2 = ToVec3(LocalFrame1[1]);

			if(IsHingeLimitEnabled(jc.mLimits))
			{
				if(jc.mLimits.mMinValue<-MAX_FLOAT*0.5f && jc.mLimits.mMaxValue>MAX_FLOAT*0.5f)
				{
				}
				else
				{
					settings.mLimitsMin = jc.mLimits.mMinValue;
					settings.mLimitsMax = jc.mLimits.mMaxValue;
				}
			}

			HingeConstraint* NewJoint = static_cast<HingeConstraint*>(settings.Create(*Actor0, *Actor1));
			J = NewJoint;

			if(jc.mUseMotor)
			{
				NewJoint->SetMotorState(EMotorState::Velocity);
				NewJoint->SetTargetAngularVelocity(jc.mDriveVelocity);
			}
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			const PINT_PRISMATIC_JOINT_CREATE& jc = static_cast<const PINT_PRISMATIC_JOINT_CREATE&>(desc);

			SliderConstraintSettings settings;
			settings.mSpace = EConstraintSpace::LocalToBodyCOM;
			settings.mPoint1 = ToVec3(jc.mLocalPivot0.mPos) - Actor0->GetShape()->GetCenterOfMass();
			settings.mPoint2 = ToVec3(jc.mLocalPivot1.mPos) - Actor1->GetShape()->GetCenterOfMass();

			if(jc.mLocalAxis0.IsNonZero())
			{
				settings.mSliderAxis1	= ToVec3(jc.mLocalAxis0);
				settings.mSliderAxis2	= ToVec3(jc.mLocalAxis1);
				settings.mNormalAxis1	= settings.mSliderAxis1.GetNormalizedPerpendicular();
				settings.mNormalAxis2	= (Actor1->GetInverseCenterOfMassTransform() * Actor0->GetCenterOfMassTransform()).Multiply3x3(settings.mNormalAxis1).Normalized();
			}
			else
			{
				const Matrix3x3 LocalFrame0 = jc.mLocalPivot0.mRot;
				const Matrix3x3 LocalFrame1 = jc.mLocalPivot1.mRot;
				settings.mSliderAxis1 = ToVec3(LocalFrame0[0]);
				settings.mSliderAxis2 = ToVec3(LocalFrame1[0]);
				settings.mNormalAxis1 = ToVec3(LocalFrame0[1]);
				settings.mNormalAxis2 = ToVec3(LocalFrame1[1]);
			}
			
			if(IsPrismaticLimitEnabled(jc.mLimits))
			{
				settings.mLimitsMin = jc.mLimits.mMinValue;
				settings.mLimitsMax = jc.mLimits.mMaxValue;
			}

			if (jc.mSpring.mStiffness > 0.0f)
			{
				// TODO: Convert properties
				settings.mFrequency = 2.0f;
				settings.mDamping = 1.0f;
			}

			J = settings.Create(*Actor0, *Actor1);
		}
		break;

		case PINT_JOINT_FIXED:
		{
			const PINT_FIXED_JOINT_CREATE& jc = static_cast<const PINT_FIXED_JOINT_CREATE&>(desc);

			FixedConstraintSettings settings;
			settings.mSpace = EConstraintSpace::LocalToBodyCOM;
			settings.mPoint1 = ToVec3(jc.mLocalPivot0) - Actor0->GetShape()->GetCenterOfMass();
			settings.mPoint2 = ToVec3(jc.mLocalPivot1) - Actor1->GetShape()->GetCenterOfMass();

			J = settings.Create(*Actor0, *Actor1);
		}
		break;

		case PINT_JOINT_DISTANCE:
		{
			const PINT_DISTANCE_JOINT_CREATE& jc = static_cast<const PINT_DISTANCE_JOINT_CREATE&>(desc);

			DistanceConstraintSettings settings;
			settings.mSpace			= EConstraintSpace::LocalToBodyCOM;
			settings.mPoint1		= ToVec3(jc.mLocalPivot0) - Actor0->GetShape()->GetCenterOfMass();
			settings.mPoint2		= ToVec3(jc.mLocalPivot1) - Actor1->GetShape()->GetCenterOfMass();
			settings.mMinDistance	= jc.mLimits.mMinValue <0.0f ? 0.0f : jc.mLimits.mMinValue;
			settings.mMaxDistance	= jc.mLimits.mMaxValue <0.0f ? MAX_FLOAT : jc.mLimits.mMaxValue;
			settings.mFrequency		= 0.0f;
			settings.mDamping		= 0.0f;

			J = settings.Create(*Actor0, *Actor1);
		}
		break;

		case PINT_JOINT_D6:
		{
			const PINT_D6_JOINT_CREATE& jc = static_cast<const PINT_D6_JOINT_CREATE&>(desc);

			SixDOFConstraintSettings settings;
			settings.mSpace = EConstraintSpace::LocalToBodyCOM;
			settings.mPosition1 = ToVec3(jc.mLocalPivot0.mPos) - Actor0->GetShape()->GetCenterOfMass();
			settings.mPosition2 = ToVec3(jc.mLocalPivot1.mPos) - Actor1->GetShape()->GetCenterOfMass();

			const Matrix3x3 LocalFrame0 = jc.mLocalPivot0.mRot;
			const Matrix3x3 LocalFrame1 = jc.mLocalPivot1.mRot;
			settings.mAxisX1 = ToVec3(LocalFrame0[0]);
			settings.mAxisX2 = ToVec3(LocalFrame1[0]);
			settings.mAxisY1 = ToVec3(LocalFrame0[1]);
			settings.mAxisY2 = ToVec3(LocalFrame1[1]);

			using EAxis = SixDOFConstraintSettings::EAxis;
			settings.mLimitMin[EAxis::TranslationX] = jc.mLinearLimits.mMin.x;
			settings.mLimitMin[EAxis::TranslationY] = jc.mLinearLimits.mMin.y;
			settings.mLimitMin[EAxis::TranslationZ] = jc.mLinearLimits.mMin.z;
			settings.mLimitMax[EAxis::TranslationX] = jc.mLinearLimits.mMax.x;
			settings.mLimitMax[EAxis::TranslationY] = jc.mLinearLimits.mMax.y;
			settings.mLimitMax[EAxis::TranslationZ] = jc.mLinearLimits.mMax.z;

			if (jc.mMinTwist <= jc.mMaxTwist)
			{
				settings.mLimitMin[EAxis::RotationX] = jc.mMinTwist;
				settings.mLimitMax[EAxis::RotationX] = jc.mMaxTwist;
			}
			if (jc.mMaxSwingY >= 0.0f)
			{
				settings.mLimitMin[EAxis::RotationY] = -jc.mMaxSwingY;
				settings.mLimitMax[EAxis::RotationY] = jc.mMaxSwingY;
			}
			if (jc.mMaxSwingZ >= 0.0f)
			{
				settings.mLimitMin[EAxis::RotationZ] = -jc.mMaxSwingZ;
				settings.mLimitMax[EAxis::RotationZ] = jc.mMaxSwingZ;
			}

			SixDOFConstraint* NewJoint = static_cast<SixDOFConstraint*>(settings.Create(*Actor0, *Actor1));
			J = NewJoint;

			// We don't know how this joint is going to be driven yet, tentatively set it to velocity until the call to SetDriveVelocity/SetDrivePosition
			if(jc.mMotorFlags & PINT_D6_MOTOR_DRIVE_X)
				NewJoint->SetMotorState(EAxis::TranslationX, EMotorState::Velocity);
			if(jc.mMotorFlags & PINT_D6_MOTOR_DRIVE_Y)
				NewJoint->SetMotorState(EAxis::TranslationY, EMotorState::Velocity);
			if(jc.mMotorFlags & PINT_D6_MOTOR_DRIVE_Z)
				NewJoint->SetMotorState(EAxis::TranslationZ, EMotorState::Velocity);
		}
		break;

		case PINT_JOINT_RACK_AND_PINION:
		{
			const PINT_RACK_AND_PINION_JOINT_CREATE& jc = static_cast<const PINT_RACK_AND_PINION_JOINT_CREATE&>(desc);

			TwoBodyConstraint* Hinge = ((TwoBodyConstraint *)jc.mHinge);
			TwoBodyConstraint* Prismatic = ((TwoBodyConstraint *)jc.mPrismatic);

			RackAndPinionConstraintSettings settings;
			settings.mSpace			= EConstraintSpace::LocalToBodyCOM;
			// TODO: This is very fragile but I have no idea how I'm supposed to get the constraint axis from the creation settings as mLocalPivot0/1 is usually not filled in
			settings.mHingeAxis		= Hinge->GetConstraintToBody2Matrix().GetAxisX();
			settings.mSliderAxis	= Prismatic->GetConstraintToBody2Matrix().GetAxisX();
			settings.SetRatio(jc.mNbRackTeeth, jc.mRackLength, jc.mNbPinionTeeth);

			RackAndPinionConstraint* NewJoint = static_cast<RackAndPinionConstraint*>(settings.Create(*Actor0, *Actor1));
			NewJoint->SetConstraints(Hinge, Prismatic);
			J = NewJoint;
		}
		break;

		case PINT_JOINT_GEAR:
		{
			const PINT_GEAR_JOINT_CREATE& jc = static_cast<const PINT_GEAR_JOINT_CREATE&>(desc);

			TwoBodyConstraint* Hinge0 = ((TwoBodyConstraint *)jc.mHinge0);
			TwoBodyConstraint* Hinge1 = ((TwoBodyConstraint *)jc.mHinge1);

			GearConstraintSettings settings;
			settings.mSpace			= EConstraintSpace::LocalToBodyCOM;
			// TODO: This is very fragile but I have no idea how I'm supposed to get the constraint axis from the creation settings as mLocalPivot0/1 is usually not filled in
			settings.mHingeAxis1	= Hinge0->GetConstraintToBody2Matrix().GetAxisX();
			settings.mHingeAxis2	= Hinge1->GetConstraintToBody2Matrix().GetAxisX();
			settings.mRatio			= 1.0f / jc.mGearRatio;

			GearConstraint* NewJoint = static_cast<GearConstraint*>(settings.Create(*Actor0, *Actor1));
			NewJoint->SetConstraints(Hinge0, Hinge1);
			J = NewJoint;
		}
		break;

		default:
			ASSERT(0);
		break;
	}
	
	if (J)
	{
		gPhysicsSystem->AddConstraint(J);

		if(gGroupFilter)
			gGroupFilter->DisableJointedBodies(Actor0->GetID().GetIndexAndSequenceNumber(), Actor1->GetID().GetIndexAndSequenceNumber());
	}

	return PintJointHandle(J);
}

bool JoltPint::ReleaseJoint(PintJointHandle handle)
{
	Constraint* Joint = reinterpret_cast<Constraint*>(handle);
	ASSERT(Joint);
	gPhysicsSystem->RemoveConstraint(Joint);
	return true;
}

void JoltPint::SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)
{
	if(!gGroupFilter)
		return;
	for(udword i=0;i<nb_groups;i++)
		gGroupFilter->DisableCollision(groups[i].mGroup0, groups[i].mGroup1);
}

PintMeshHandle JoltPint::CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)
{
	const PintSurfaceInterface&	PSI = desc.GetSurface();

	IndexedSurface* IS = new IndexedSurface;
	IS->Init(PSI.mNbFaces, PSI.mNbVerts, PSI.mVerts, (IceMaths::IndexedTriangle*)PSI.mDFaces);

	if(index)
		*index = INVALID_ID;

	return PintMeshHandle(IS);
}

bool JoltPint::DeleteMeshObject(PintMeshHandle handle, const PintMeshIndex* index)
{ 
	delete reinterpret_cast<IndexedSurface*>(handle);
	return true;
}

PintHeightfieldHandle JoltPint::CreateHeightfieldObject(const PINT_HEIGHTFIELD_DATA_CREATE& desc, PintHeightfieldData& data, PintHeightfieldIndex* index)
{
	data.mHeightScale = 1.0f;
	data.mMinHeight = FLT_MAX;
	data.mMaxHeight = -FLT_MAX;

	HeightFieldShapeSettings* Settings = new HeightFieldShapeSettings;
	Settings->mSampleCount = max(desc.mNbU, desc.mNbV);
	Settings->mHeightSamples.resize(Settings->mSampleCount * Settings->mSampleCount);
	Settings->mScale = Vec3(float(desc.mNbV) / Settings->mSampleCount, 1.0f, float(desc.mNbU) / Settings->mSampleCount); // U and V seem to be Z and X so we have to flip everything

	for (uint Y = 0; Y < Settings->mSampleCount; Y++)
	{
		for (uint X = 0; X < Settings->mSampleCount; X++)
		{
			// Jolt heightfields need to be square, resample the heightfield if this is not the case
			float XSF = float(X * desc.mNbU) / Settings->mSampleCount;
			float YSF = float(Y * desc.mNbV) / Settings->mSampleCount;
			uint XS = uint(XSF);
			uint YS = uint(YSF);
			XSF -= float(XS);
			YSF -= float(YS);
			uint XSPlus1 = min(XS + 1, desc.mNbU - 1);
			uint YSPlus1 = min(YS + 1, desc.mNbV - 1);

			// Multiply by the stride
			YS *= desc.mNbU;
			YSPlus1 *= desc.mNbU;

			float *out_sample = Settings->mHeightSamples.data() + X * Settings->mSampleCount + Y;
			if (desc.mHeights != nullptr)
			{
				float v1 = (1.0f - XSF) * desc.mHeights[YS + XS] + XSF * desc.mHeights[YS + XSPlus1];
				float v2 = (1.0f - XSF) * desc.mHeights[YSPlus1 + XS] + XSF * desc.mHeights[YSPlus1 + XSPlus1];
				*out_sample = (1.0f - YSF) * v1 + YSF * v2;
			}
			else
				*out_sample = desc.mUniqueValue;

			data.mMinHeight = min(data.mMinHeight, *out_sample);
			data.mMaxHeight = max(data.mMaxHeight, *out_sample);

			out_sample++;
		}
	}

	// The PEEL API expects the shape to start vertically at 0, so we need to offset the shape
	Settings->mOffset = Vec3(0, -data.mMinHeight, 0);

	Settings->AddRef();
	return PintHeightfieldHandle(Settings);
}

bool JoltPint::DeleteHeightfieldObject(PintHeightfieldHandle handle, const PintHeightfieldIndex* index)
{
	reinterpret_cast<HeightFieldShapeSettings*>(handle)->Release();
	return true;
}

static inline_ void FillResultStruct(const RayCast& raycast, PintRaycastHit& hit, const RayCastResult& result)
{
	const Vec3 outPosition = raycast.GetPointOnRay(result.mFraction);

	hit.mImpact		= ToPoint(outPosition);
	hit.mDistance	= (outPosition - raycast.mOrigin).Length();

	BodyLockRead lock(gPhysicsSystem->GetBodyLockInterfaceNoLock(), result.mBodyID);
	if (lock.Succeeded())
	{
		const Body& hit_body = lock.GetBody();

		const Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, outPosition);
		hit.mNormal			= ToPoint(normal);

		hit.mTouchedActor	= PintActorHandle(&hit_body);
		hit.mTouchedShape	= null;
	}
	else
	{
		hit.mNormal.Zero();
		hit.mTouchedActor	= null;
		hit.mTouchedShape	= null;
	}

	hit.mTriangleIndex	= INVALID_ID;
}

udword JoltPint::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	if(!gPhysicsSystem)
		return 0;

	const bool CullBackFaces = gBackfaceCulling;

	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();

	RayCastSettings inRayCastSettings;

	udword NbHits = 0;
	while(nb--)
	{
		RayCast R;
		R.mOrigin = ToVec3(raycasts->mOrigin);
		R.mDirection = ToVec3(raycasts->mDir) * raycasts->mMaxDist;

		if(CullBackFaces)
		{
			ClosestHitCollisionCollector<CastRayCollector> collector;
			NPQ.CastRay(R, inRayCastSettings, collector);

			if(collector.HadHit())
			{
				NbHits++;
				FillResultStruct(R, *dest, collector.mHit);
			}
			else
				dest->SetNoHit();
		}
		else
		{
			RayCastResult ioHit;
			if(NPQ.CastRay(R, ioHit))
			{
				NbHits++;
				FillResultStruct(R, *dest, ioHit);
			}
			else
				dest->SetNoHit();
		}

		raycasts++;
		dest++;
	}
	return NbHits;
}

udword JoltPint::BatchRaycastAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)
{
	if(!gPhysicsSystem)
		return 0;

	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();

	RayCastSettings inRayCastSettings;
	inRayCastSettings.mBackFaceMode = gBackfaceCulling ? EBackFaceMode::IgnoreBackFaces : EBackFaceMode::CollideWithBackFaces;

	udword NbHits = 0;
	while(nb--)
	{
		RayCast R;
		R.mOrigin = ToVec3(raycasts->mOrigin);
		R.mDirection = ToVec3(raycasts->mDir) * raycasts->mMaxDist;

		AnyHitCollisionCollector<CastRayCollector> collector;
		NPQ.CastRay(R, inRayCastSettings, collector);

		if(collector.HadHit())
		{
			NbHits++;
			dest->mHit = true;
		}
		else
			dest->SetNoHit();

		raycasts++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();

	ShapeCastSettings settings;
	udword NbHits = 0;
	while(nb--)
	{
		const float MaxDist = sweeps->mMaxDist < 5000.0f ? sweeps->mMaxDist : 5000.0f;

		const BoxShape QueryShape(ToVec3(sweeps->mBox.mExtents));

		const Point& row0 = sweeps->mBox.mRot[0];
		const Point& row1 = sweeps->mBox.mRot[1];
		const Point& row2 = sweeps->mBox.mRot[2];

		const Vec4 v0(row0.x, row0.y, row0.z, 0.0f);
		const Vec4 v1(row1.x, row1.y, row1.z, 0.0f);
		const Vec4 v2(row2.x, row2.y, row2.z, 0.0f);
		const Vec4 v3(sweeps->mBox.mCenter.x, sweeps->mBox.mCenter.y, sweeps->mBox.mCenter.z, 1.0f);

		const ShapeCast shape_cast { &QueryShape, Vec3::sReplicate(1.0f), Mat44(v0,v1,v2,v3), ToVec3(sweeps->mDir * MaxDist) };

		ClosestHitCollisionCollector<CastShapeCollector> collector;
		NPQ.CastShape(shape_cast, settings, collector);

		if(collector.HadHit())
		{
			NbHits++;

			const Vec3 outPosition = collector.mHit.mContactPointOn2;

			dest->mImpact	= ToPoint(outPosition);
			dest->mDistance = collector.mHit.mFraction * MaxDist;

			BodyLockRead lock(gPhysicsSystem->GetBodyLockInterfaceNoLock(), collector.mHit.mBodyID2);
			if (lock.Succeeded())
			{
				const Body& hit_body = lock.GetBody();

				const Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(collector.mHit.mSubShapeID2, outPosition);
				dest->mNormal			= ToPoint(normal);

				dest->mTouchedActor	= PintActorHandle(&hit_body);
				dest->mTouchedShape	= null;
			}
			else
			{
				dest->mNormal.Zero();
				dest->mTouchedActor	= null;
				dest->mTouchedShape	= null;
			}

			dest->mTriangleIndex	= INVALID_ID;
		}
		else
			dest->SetNoHit();

		sweeps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();

	ShapeCastSettings settings;

	udword NbHits = 0;
	while(nb--)
	{
		const float MaxDist = sweeps->mMaxDist < 5000.0f ? sweeps->mMaxDist : 5000.0f;

		const SphereShape QueryShape(sweeps->mSphere.mRadius);
		const ShapeCast shape_cast { &QueryShape, Vec3::sReplicate(1.0f), Mat44::sTranslation(ToVec3(sweeps->mSphere.mCenter)), ToVec3(sweeps->mDir * MaxDist) };

		ClosestHitCollisionCollector<CastShapeCollector> collector;
		NPQ.CastShape(shape_cast, settings, collector);

		if(collector.HadHit())
		{
			NbHits++;
			const Vec3 outPosition = collector.mHit.mContactPointOn2;

			dest->mImpact	= ToPoint(outPosition);
			dest->mDistance = collector.mHit.mFraction * MaxDist;

			BodyLockRead lock(gPhysicsSystem->GetBodyLockInterfaceNoLock(), collector.mHit.mBodyID2);
			if (lock.Succeeded())
			{
				const Body& hit_body = lock.GetBody();

				const Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(collector.mHit.mSubShapeID2, outPosition);
				dest->mNormal			= ToPoint(normal);

				dest->mTouchedActor	= PintActorHandle(&hit_body);
				dest->mTouchedShape	= null;
			}
			else
			{
				dest->mNormal.Zero();
				dest->mTouchedActor	= null;
				dest->mTouchedShape	= null;
			}

			dest->mTriangleIndex	= INVALID_ID;
		}
		else
			dest->SetNoHit();

		sweeps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

static IQuat _ShortestRotation(const Point& v0, const Point& v1)
{
	const float d = v0|v1;
	const Point cross = v0^v1;

	IQuat q = d>-1.0f ? IQuat(1.0f + d, cross.x, cross.y, cross.z)
					: fabsf(v0.x)<0.1f ? IQuat(0.0f, 0.0f, v0.z, -v0.y) : IQuat(0.0f, v0.y, -v0.x, 0.0f);

	q.Normalize();

	return q;
}

udword JoltPint::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();

	ShapeCastSettings settings;
	udword NbHits = 0;
	while(nb--)
	{
		const float MaxDist = sweeps->mMaxDist < 5000.0f ? sweeps->mMaxDist : 5000.0f;

		// ### optimize this
		const Point Center = (sweeps->mCapsule.mP0 + sweeps->mCapsule.mP1)*0.5f;
		Point CapsuleAxis = sweeps->mCapsule.mP1 - sweeps->mCapsule.mP0;
		const float M = CapsuleAxis.Magnitude();
		CapsuleAxis /= M;
		const IQuat q = _ShortestRotation(Point(0.0f, 1.0f, 0.0f), CapsuleAxis);

		const CapsuleShape QueryShape(M*0.5f, sweeps->mCapsule.mRadius);

		const ShapeCast shape_cast { &QueryShape, Vec3::sReplicate(1.0f), Mat44::sRotationTranslation(ToJQuat(q), ToVec3(Center)), ToVec3(sweeps->mDir * MaxDist) };

		ClosestHitCollisionCollector<CastShapeCollector> collector;
		NPQ.CastShape(shape_cast, settings, collector);

		if(collector.HadHit())
		{
			NbHits++;
			const Vec3 outPosition = collector.mHit.mContactPointOn2;

			dest->mImpact	= ToPoint(outPosition);
			dest->mDistance = collector.mHit.mFraction * MaxDist;

			BodyLockRead lock(gPhysicsSystem->GetBodyLockInterfaceNoLock(), collector.mHit.mBodyID2);
			if (lock.Succeeded())
			{
				const Body& hit_body = lock.GetBody();

				const Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(collector.mHit.mSubShapeID2, outPosition);
				dest->mNormal			= ToPoint(normal);

				dest->mTouchedActor	= PintActorHandle(&hit_body);
				dest->mTouchedShape	= null;
			}
			else
			{
				dest->mNormal.Zero();
				dest->mTouchedActor	= null;
				dest->mTouchedShape	= null;
			}

			dest->mTriangleIndex	= INVALID_ID;
		}
		else
			dest->SetNoHit();

		sweeps++;
		dest++;
	}
	return NbHits;
}

/*udword JoltPint::BatchConvexSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)
{
	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();

	ShapeCastSettings settings;
	udword NbHits = 0;
	while(nb--)
	{
		const float MaxDist = sweeps->mMaxDist < 5000.0f ? sweeps->mMaxDist : 5000.0f;

		const ConvexShape QueryShape(M*0.5f, sweeps->mCapsule.mRadius);

		const ShapeCast shape_cast { &QueryShape, Vec3::sReplicate(1.0f), Mat44::sRotationTranslation(ToJQuat(q), ToVec3(Center)), ToVec3(sweeps->mDir * MaxDist) };

		ClosestHitCollisionCollector<CastShapeCollector> collector;
		NPQ.CastShape(shape_cast, settings, collector);

		if(collector.HadHit())
		{
			NbHits++;
//			dest->mDistance = collector.mHit.mFraction * MaxDist;

				//const Vec3 outPosition = ToVec3(sweeps->mBox.mCenter) + collector.mHit.mFraction * shape_cast.mDirection;
				const Vec3 outPosition = collector.mHit.mContactPointOn2;

				dest->mImpact	= ToPoint(outPosition);
				dest->mDistance = collector.mHit.mFraction * MaxDist;

				//BodyLockRead lock(gPhysicsSystem->GetBodyLockInterface(), collector.mHit.mBodyID2);
				BodyLockRead lock(gPhysicsSystem->GetBodyLockInterfaceNoLock(), collector.mHit.mBodyID2);
				if (lock.Succeeded())
				{
					const Body& hit_body = lock.GetBody();

					const Vec3 normal = hit_body.GetWorldSpaceSurfaceNormal(collector.mHit.mSubShapeID2, outPosition);
					dest->mNormal			= ToPoint(normal);

					dest->mTouchedActor	= PintActorHandle(&hit_body);
					dest->mTouchedShape	= null;
				}
				else
				{
					dest->mNormal.Zero();
					dest->mTouchedActor	= null;
					dest->mTouchedShape	= null;
				}

				dest->mTriangleIndex	= INVALID_ID;
		}
		else
			dest->SetNoHit();

		sweeps++;
		dest++;
	}
	return NbHits;
}*/

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::BatchSphereOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)
{
	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();

	CollideShapeSettings settings;
	settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;

	udword NbHits = 0;
	while(nb--)
	{
		const SphereShape QueryShape(overlaps->mSphere.mRadius);

		AnyHitCollisionCollector<CollideShapeCollector> collector;
		NPQ.CollideShape(&QueryShape, Vec3::sReplicate(1.0f), Mat44::sTranslation(ToVec3(overlaps->mSphere.mCenter)), settings, collector);

		if(collector.HadHit())
		{
			NbHits++;
			dest->mHit = true;
		}
		else
			dest->SetNoHit();

		overlaps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::BatchSphereOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)
{
	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();

	CollideShapeSettings settings;
	settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;

	udword Offset = 0;
	udword NbHits = 0;
	while(nb--)
	{
		const SphereShape QueryShape(overlaps->mSphere.mRadius);

		AllHitCollisionCollector<CollideShapeCollector> collector;
		NPQ.CollideShape(&QueryShape, Vec3::sReplicate(1.0f), Mat44::sTranslation(ToVec3(overlaps->mSphere.mCenter)), settings, collector);

		const udword Nb = udword(collector.mHits.size());
		NbHits += Nb;
		dest->mNbHits = Nb;
		dest->mOffset = Offset;
		Offset += Nb;

		if(Nb)
		{
			PintOverlapHit* buffer = reinterpret_cast<PintOverlapHit*>(stream.Reserve(Nb*(sizeof(PintOverlapHit)/sizeof(udword))));
			for(udword i=0;i<Nb;i++)
			{
				BodyLockRead lock(gPhysicsSystem->GetBodyLockInterfaceNoLock(), collector.mHits[i].mBodyID2);
				if(lock.Succeeded())
				{
					const Body& hit_body = lock.GetBody();
					buffer[i].mTouchedActor	= PintActorHandle(&hit_body);
					buffer[i].mTouchedShape	= null;
				}
				else
				{
					buffer[i].mTouchedActor	= null;
					buffer[i].mTouchedShape	= null;
				}
			}
		}

		overlaps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

/*udword JoltPint::BatchBoxOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::BatchBoxOverlapObjects(PintSQThreadContext context, udword nb, PintOverlapObjectHit* dest, const PintBoxOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::BatchCapsuleOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)
{
	return 0;
}

udword JoltPint::BatchCapsuleOverlapObjects(PintSQThreadContext context, udword nb, PintOverlapObjectHit* dest, const PintCapsuleOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::FindTriangles_MeshSphereOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintSphereOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::FindTriangles_MeshBoxOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintBoxOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintCapsuleOverlapData* overlaps)
{
	return 0;
}*/

///////////////////////////////////////////////////////////////////////////////

PR JoltPint::GetWorldTransform(PintActorHandle handle)
{
	const Body* Actor = reinterpret_cast<const Body*>(handle);
	ASSERT(Actor);

	const Vec3 p = Actor->GetPosition();
	const JQuat r = Actor->GetRotation();

	return PR(ToPoint(p), ToIQuat(r));
}

void JoltPint::SetWorldTransform(PintActorHandle handle, const PR& pose)
{
	Body* Actor = reinterpret_cast<Body*>(handle);
	ASSERT(Actor);

	BodyInterface& BI = gPhysicsSystem->GetBodyInterface();
	BI.SetPositionAndRotation(Actor->GetID(), ToVec3(pose.mPos), ToJQuat(pose.mRot), EActivation::DontActivate);
}

void JoltPint::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
	Body* Actor = reinterpret_cast<Body*>(handle);
	ASSERT(Actor);

	if(Actor->IsStatic())
		return;

	Actor->AddImpulse(ToVec3(world_impulse), ToVec3(world_pos));

	BodyInterface& BI = gPhysicsSystem->GetBodyInterface();
	BI.ActivateBody(Actor->GetID());
}

Point JoltPint::GetLinearVelocity(PintActorHandle handle)
{
	const MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return Point(0.0f, 0.0f, 0.0f);

	return ToPoint(MP->GetLinearVelocity());
}

void JoltPint::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)
{
	MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return;

	MP->SetLinearVelocityClamped(ToVec3(linear_velocity));
}

Point JoltPint::GetAngularVelocity(PintActorHandle handle)
{
	const MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return Point(0.0f, 0.0f, 0.0f);

	return ToPoint(MP->GetAngularVelocity());
}

void JoltPint::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
	MotionProperties* MP = GetDynamicData(handle);
	if(!MP)
		return;

	MP->SetAngularVelocityClamped(ToVec3(angular_velocity));
}

bool JoltPint::SetKinematicPose(PintActorHandle handle, const Point& pos)
{
	Body* Actor = reinterpret_cast<Body*>(handle);
	ASSERT(Actor);

	const JQuat q = Actor->GetRotation();

	Actor->MoveKinematic(ToVec3(pos), q, 1.0f/60.0f);
	return true;
}

bool JoltPint::SetKinematicPose(PintActorHandle handle, const PR& pr)
{
	Body* Actor = reinterpret_cast<Body*>(handle);
	ASSERT(Actor);

	Actor->MoveKinematic(ToVec3(pr.mPos), ToJQuat(pr.mRot), 1.0f/60.0f);
	return true;
}

bool JoltPint::IsKinematic(PintActorHandle handle)
{
	const Body* Actor = reinterpret_cast<const Body*>(handle);
	ASSERT(Actor);
	return Actor->IsKinematic();
}

bool JoltPint::EnableKinematic(PintActorHandle handle, bool flag)
{
	const Body* Actor = reinterpret_cast<const Body*>(handle);
	ASSERT(Actor);

	BodyInterface& BI = gPhysicsSystem->GetBodyInterface();
	BI.SetMotionType(Actor->GetID(), flag ? EMotionType::Kinematic : EMotionType::Dynamic, EActivation::Activate);
	return true;
}

bool JoltPint::SetDriveEnabled(PintJointHandle handle, bool flag)
{
	Constraint* Joint = reinterpret_cast<Constraint*>(handle);
	ASSERT(Joint);
	const EConstraintSubType JT = Joint->GetSubType();
	if(JT==EConstraintSubType::Hinge)
	{
		HingeConstraint* Hinge = static_cast<HingeConstraint*>(Joint);
		Hinge->SetMotorState(flag ? EMotorState::Velocity : EMotorState::Off);	// ### to refine for position drives
	}
	else
		ASSERT(0);
	return true;
}

bool JoltPint::SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)
{
	TwoBodyConstraint* Joint = reinterpret_cast<TwoBodyConstraint*>(handle);
	ASSERT(Joint);
	const EConstraintSubType JT = Joint->GetSubType();
	if(JT==EConstraintSubType::Hinge)
	{
		HingeConstraint* Hinge = static_cast<HingeConstraint*>(Joint);
		// See notes in SharedPhysX::SetDriveVelocity
		Hinge->SetTargetAngularVelocity(angular.x);
	}
	else if (JT==EConstraintSubType::SixDOF)
	{
		SixDOFConstraint* D6 = static_cast<SixDOFConstraint*>(Joint);
		D6->SetTargetVelocityCS(ToVec3(linear));
		D6->SetTargetAngularVelocityCS(ToVec3(angular));

		// Set the motors that were activated to velocity now
		using EAxis = SixDOFConstraintSettings::EAxis;
		for (int Axis = 0; Axis < 6; Axis++)
			if (D6->GetMotorState((EAxis)Axis) != EMotorState::Off)
				D6->SetMotorState((EAxis)Axis, EMotorState::Velocity);
	}
	else
		ASSERT(0);

	// Prevent the bodies from going to sleep
	gPhysicsSystem->GetBodyInterface().ActivateConstraint(Joint);
	return true;
}

bool JoltPint::SetDrivePosition(PintJointHandle handle, const PR& pose)
{ 
	TwoBodyConstraint* Joint = reinterpret_cast<TwoBodyConstraint*>(handle);
	ASSERT(Joint);
	const EConstraintSubType JT = Joint->GetSubType();
	if (JT==EConstraintSubType::SixDOF)
	{
		SixDOFConstraint* D6 = static_cast<SixDOFConstraint*>(Joint);
		D6->SetTargetPositionCS(ToVec3(pose.mPos));
		D6->SetTargetOrientationCS(ToJQuat(pose.mRot));

		// Set the motors that were activated to position now
		using EAxis = SixDOFConstraintSettings::EAxis;
		for (int Axis = 0; Axis < 6; Axis++)
			if (D6->GetMotorState((EAxis)Axis) != EMotorState::Off)
				D6->SetMotorState((EAxis)Axis, EMotorState::Position);
	}
	else
		ASSERT(0);
	
	// Prevent the bodies from going to sleep
	gPhysicsSystem->GetBodyInterface().ActivateConstraint(Joint);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static JoltPint* gJolt = null;
static void gJolt_GetOptionsFromGUI(const char*);

void Jolt_Init(const PINT_WORLD_CREATE& desc)
{
	gJolt_GetOptionsFromGUI(desc.GetTestName());

	ASSERT(!gJolt);
	gJolt = ICE_NEW(JoltPint);
	gJolt->Init(desc);
}

void Jolt_Close()
{
	if(gJolt)
	{
		gJolt->Close();
		delete gJolt;
		gJolt = null;
	}
}

JoltPint* GetJolt()
{
	return gJolt;
}

///////////////////////////////////////////////////////////////////////////////

static Widgets*	gJoltGUI = null;

static IceCheckBox*	gCheckBox_AllowSleeping = null;
static IceCheckBox*	gCheckBox_AllowShapeSharing = null;
static IceCheckBox*	gCheckBox_CCD = null;
static IceCheckBox*	gCheckBox_BackfaceCulling  = null;

static IceEditBox* gEditBox_NbThreads = null;
static IceEditBox* gEditBox_NbSubsteps = null;
static IceEditBox* gEditBox_TempAllocSize = null;
static IceEditBox* gEditBox_MaxBodies = null;
static IceEditBox* gEditBox_MaxBodyPairs = null;
static IceEditBox* gEditBox_MaxContactConstraints = null;
static IceEditBox* gEditBox_NbBodyMutexes = null;
static IceEditBox* gEditBox_NbPosIter = null;
static IceEditBox* gEditBox_NbVelIter = null;
static IceEditBox* gEditBox_LinearDamping = null;
static IceEditBox* gEditBox_AngularDamping = null;
static IceEditBox* gEditBox_SpeculativeContactDistance = null;
static IceEditBox* gEditBox_PenetrationSlop = null;
static IceEditBox* gEditBox_Baumgarte = null;
static IceEditBox* gEditBox_Friction = null;
static IceEditBox* gEditBox_Restitution = null;

enum JoltGUIElement
{
	JOLT_GUI_MAIN,
	//
	JOLT_GUI_ALLOW_SLEEPING,
	JOLT_GUI_ALLOW_SHAPE_SHARING,
	JOLT_GUI_ENABLE_CCD,
	JOLT_GUI_BACKFACE_CULLING,
	//
};

static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
{
	const udword id = check_box.GetID();
	switch(id)
	{
		case JOLT_GUI_ALLOW_SLEEPING:
			gAllowSleeping = checked;
			break;
		case JOLT_GUI_ALLOW_SHAPE_SHARING:
			gAllowShapeSharing = checked;
			break;
		case JOLT_GUI_ENABLE_CCD:
			gEnableCCD = checked;
			break;
		case JOLT_GUI_BACKFACE_CULLING:
			gBackfaceCulling = checked;
			break;
	}
}

static void gJolt_GetOptionsFromGUI(const char* test_name)
{
	if(gCheckBox_AllowSleeping)
		gAllowSleeping = gCheckBox_AllowSleeping->IsChecked();
	if(gCheckBox_AllowShapeSharing)
		gAllowShapeSharing = gCheckBox_AllowShapeSharing->IsChecked();
	if(gCheckBox_CCD)
		gEnableCCD = gCheckBox_CCD->IsChecked();
	if(gCheckBox_BackfaceCulling)
		gBackfaceCulling = gCheckBox_BackfaceCulling->IsChecked();

	Common_GetFromEditBox(gNbThreads, gEditBox_NbThreads);
	Common_GetFromEditBox(gNbSubsteps, gEditBox_NbSubsteps);
	Common_GetFromEditBox(gTempAllocSize, gEditBox_TempAllocSize);
	Common_GetFromEditBox(gMaxBodies, gEditBox_MaxBodies);
	Common_GetFromEditBox(gMaxBodyPairs, gEditBox_MaxBodyPairs);
	Common_GetFromEditBox(gMaxContactConstraints, gEditBox_MaxContactConstraints);
	Common_GetFromEditBox(gNbBodyMutexes, gEditBox_NbBodyMutexes);
	Common_GetFromEditBox(gNbPosIter, gEditBox_NbPosIter);
	Common_GetFromEditBox(gNbVelIter, gEditBox_NbVelIter);
	Common_GetFromEditBox(gLinearDamping, gEditBox_LinearDamping, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gAngularDamping, gEditBox_AngularDamping, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gSpeculativeContactDistance, gEditBox_SpeculativeContactDistance, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gPenetrationSlop, gEditBox_PenetrationSlop, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gBaumgarte, gEditBox_Baumgarte, 0.0f, 1.0f);
	Common_GetFromEditBox(gDefaultFriction, gEditBox_Friction, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gDefaultRestitution, gEditBox_Restitution, 0.0f, MAX_FLOAT);

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

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, label, gJoltGUI);
	IceEditBox* EB = helper.CreateEditBox(parent, INVALID_ID, 4+EditBoxX2, y, EditBoxWidth, 20, value, gJoltGUI, filter, null);
	y += YStep;
	return EB;
}

IceWindow* Jolt_InitGUI(IceWidget* parent, PintGUIHelper& helper)
{
	IceWindow* Main = helper.CreateMainWindow(gJoltGUI, parent, JOLT_GUI_MAIN, "Jolt options");

	const sdword YStep = 20;
	const sdword YStepCB = 16;
	sdword y = 4;

	const sdword OffsetX = 90;
	const sdword LabelOffsetY = 2;
	const sdword EditBoxWidth = 60;
	const udword CheckBoxWidth = 190;
	const sdword LabelWidth = 160;
	const sdword EditBoxX2 = LabelWidth + 10;

	gCheckBox_AllowSleeping = helper.CreateCheckBox(Main, JOLT_GUI_ALLOW_SLEEPING, 4, y, CheckBoxWidth, 20, "Allow sleeping", gJoltGUI, gAllowSleeping, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_AllowShapeSharing = helper.CreateCheckBox(Main, JOLT_GUI_ALLOW_SHAPE_SHARING, 4, y, CheckBoxWidth, 20, "Allow shape sharing", gJoltGUI, gAllowShapeSharing, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_CCD = helper.CreateCheckBox(Main, JOLT_GUI_ENABLE_CCD, 4, y, CheckBoxWidth, 20, "Enable CCD", gJoltGUI, gEnableCCD, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_BackfaceCulling = helper.CreateCheckBox(Main, JOLT_GUI_BACKFACE_CULLING, 4, y, CheckBoxWidth, 20, "Backface culling (scene queries)", gJoltGUI, gBackfaceCulling, gCheckBoxCallback);
	y += YStep;

	gEditBox_NbThreads					= CreateEditBox(helper, Main, y, "Nb threads (0==automatic):", _F("%d", gNbThreads), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbSubsteps					= CreateEditBox(helper, Main, y, "Nb substeps:", _F("%d", gNbSubsteps), EDITBOX_INTEGER_POSITIVE);
	gEditBox_TempAllocSize				= CreateEditBox(helper, Main, y, "Tmp alloc size (Mb):", _F("%d", gTempAllocSize), EDITBOX_INTEGER_POSITIVE);
	gEditBox_MaxBodies					= CreateEditBox(helper, Main, y, "Max bodies:", _F("%d", gMaxBodies), EDITBOX_INTEGER_POSITIVE);
	gEditBox_MaxBodyPairs				= CreateEditBox(helper, Main, y, "Max body pairs:", _F("%d", gMaxBodyPairs), EDITBOX_INTEGER_POSITIVE);
	gEditBox_MaxContactConstraints		= CreateEditBox(helper, Main, y, "Max contact constraints:", _F("%d", gMaxContactConstraints), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbBodyMutexes				= CreateEditBox(helper, Main, y, "Nb body mutexes:", _F("%d", gNbBodyMutexes), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbPosIter					= CreateEditBox(helper, Main, y, "Nb pos iter:", _F("%d", gNbPosIter), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbVelIter					= CreateEditBox(helper, Main, y, "Nb vel iter:", _F("%d", gNbVelIter), EDITBOX_INTEGER_POSITIVE);
	gEditBox_LinearDamping				= CreateEditBox(helper, Main, y, "Linear damping:", helper.Convert(gLinearDamping), EDITBOX_FLOAT_POSITIVE);
	gEditBox_AngularDamping				= CreateEditBox(helper, Main, y, "Angular damping:", helper.Convert(gAngularDamping), EDITBOX_FLOAT_POSITIVE);
	gEditBox_SpeculativeContactDistance	= CreateEditBox(helper, Main, y, "Speculative contact distance:", helper.Convert(gSpeculativeContactDistance), EDITBOX_FLOAT_POSITIVE);
	gEditBox_PenetrationSlop			= CreateEditBox(helper, Main, y, "Max penetration slop:", helper.Convert(gPenetrationSlop), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Baumgarte					= CreateEditBox(helper, Main, y, "Baumgarte:", helper.Convert(gBaumgarte), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Friction					= CreateEditBox(helper, Main, y, "Default friction:", helper.Convert(gDefaultFriction), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Restitution				= CreateEditBox(helper, Main, y, "Default restitution:", helper.Convert(gDefaultRestitution), EDITBOX_FLOAT_POSITIVE);

	auto make_snapshot = [](IceButton& button, void* user_data) { 
		PhysicsScene Scene;
		Scene.FromPhysicsSystem(gPhysicsSystem);
		ofstream Stream;
		Stream.open("snapshot.bin", ofstream::out | ofstream::binary | ofstream::trunc);
		if (Stream.is_open()) 
		{
			StreamOutWrapper Wrapper(Stream);
			Scene.SaveBinaryState(Wrapper, true, false);
		}
	};
	helper.CreateButton(Main, 0, 4, y, 130, 20, "Save snapshot.bin", gJoltGUI, make_snapshot, nullptr);
	y += YStep;

	return Main;
}

void Jolt_CloseGUI()
{
	Common_CloseGUI(gJoltGUI);
	gCheckBox_AllowSleeping = null;
	gCheckBox_AllowShapeSharing = null;
	gCheckBox_CCD = null;
	gCheckBox_BackfaceCulling = null;
	gEditBox_NbThreads = null;
	gEditBox_NbSubsteps = null;
	gEditBox_TempAllocSize = null;
	gEditBox_MaxBodies = null;
	gEditBox_MaxBodyPairs = null;
	gEditBox_MaxContactConstraints = null;
	gEditBox_NbBodyMutexes = null;
	gEditBox_NbPosIter = null;
	gEditBox_NbVelIter = null;
	gEditBox_LinearDamping = null;
	gEditBox_AngularDamping = null;
	gEditBox_SpeculativeContactDistance = null;
	gEditBox_PenetrationSlop = null;
	gEditBox_Baumgarte = null;
	gEditBox_Friction = null;
	gEditBox_Restitution = null;
}

///////////////////////////////////////////////////////////////////////////////

class JoltPlugIn : public PintPlugin
{
	public:
	virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper)	override	{ return Jolt_InitGUI(parent, helper);	}
	virtual	void		CloseGUI()											override	{ Jolt_CloseGUI();						}

	virtual	void		Init(const PINT_WORLD_CREATE& desc)					override	{ Jolt_Init(desc);						}
	virtual	void		Close()												override	{ Jolt_Close();							}

	virtual	Pint*		GetPint()											override	{ return GetJolt();						}

	virtual	IceWindow*	InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)	override;
	virtual	void		CloseTestGUI()																					override;
	virtual	const char*	GetTestGUIName()																				override	{ return "Jolt";	}
	virtual	void		ApplyTestUIParams(const char* test_name)														override;
};
static JoltPlugIn gPlugIn;

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

static const sdword YStep = 20;

static IceCheckBox*	gCheckBox_Override = null;
static IceCheckBox*	gCheckBox_Generic0 = null;

static const char* gDesc_CCD =
"By design the CCD tests fail without CCD enabled.\n\
Check this box to make them pass using the\n\
default Jolt CCD algorithm.";

static IceWindow* CreateUI_CCD(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, const char* test_name)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void CCD(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gCheckBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::CCD, null);
	y += YStep;

	bool EnableCCD = true;

	const sdword x2 = x + 10;
	const sdword CheckBoxWidth2 = 220;
	{
		sdword YSaved = y;
		y += 50;

		ASSERT(!gCheckBox_Generic0);
		gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, x2, y, CheckBoxWidth2, 20, "Enable Jolt CCD", &owner, EnableCCD, null, null);
		y += YStep;

		IceEditBox* EB = helper.CreateEditBox(TabWindow, 0, x, YSaved, 260, 80, "", &owner, EDITBOX_TEXT, null);
		EB->SetReadOnly(true);
		EB->SetMultilineText(gDesc_CCD);
		y += 20;
	}

	return TabWindow;
}

static bool IsCCDTest(const char* test_name)
{
	return strncmp(test_name, "CCDTest_", 8)==0;
}

IceWindow* JoltPlugIn::InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	if(IsCCDTest(test_name))
		return CreateUI_CCD(parent, helper, owner, test_name);

	return null;
}

void JoltPlugIn::CloseTestGUI()
{
	gCheckBox_Override = null;
	gCheckBox_Generic0 = null;
}

void JoltPlugIn::ApplyTestUIParams(const char* test_name)
{
	const bool ApplySettings = gCheckBox_Override ? gCheckBox_Override->IsChecked() : false;
	if(!ApplySettings)
		return;

	if(IsCCDTest(test_name))
	{
		gEnableCCD = gCheckBox_Generic0->IsChecked();
		return;
	}
}
