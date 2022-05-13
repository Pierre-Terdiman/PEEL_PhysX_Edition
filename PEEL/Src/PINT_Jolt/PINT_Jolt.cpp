///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// Uses Jolt libs from 12/03/2022

/*
Differences with PhysX:
- no support for articulations
- no support for aggregates AFAIK
- no support for gyroscopic forces AFAIK
- only one friction parameter (no support for static/dynamic friction)
- no way to use a custom allocator for now (maybe because it uses std vectors etc)
- no way to automatically disable collisions between jointed objects
- speculative contact distance is a global value, not automatically adjusted at runtime
- linear CCD & speculative contacts can both be enabled at the same time
- Jolt does support per-joint friction value on regular joints (PhysX only supports it on RC articulation links)
- no support for springs on (prismatic) limits
- PhysX's joints use local frames, Jolt does not (pros & cons here) -> JR: This is supported now.
- shapes with non-idt local poses or with a non-zero COM are a special case (same design as Bullet/Havok), while it's the default in PhysX.
- number of iterations are per-scene settings in Jolt, per-actor in PhysX
- supports per-body gravity factor, PhysX doesn't
- collision-groups / sim filtering data is per-shape in PhysX, while it seems per-actor in Jolt
- hinge limits range is only 2*PI in Jolt, twice that in PhysX
- there's no separation between convex/mesh objects and shapes in Jolt
- I think Jolt has zero runtime allocations (JR: actually there is at least 1, but it is very minimal)
- 4 iterations in Jolt is not enough, some stacks collapse, joints feel too soft (low Baumgarte by default)
- Jolt uses AVX2/AVX/SSE4, PhysX sticks to SSE2

TODO:
- COM shapes
- more per-test UI
- vehicles, CCTs
- cleanup render code
- prismatic springs
- finish joints
- overlaps/sweeps
- try path constraint
- heightfields
- make rectangle selection work with Jolt

"The higher the frequency, the quicker the body will move to its target. If you set a frequency of 1 then it will oscillate 1 time per second around the target position,
if you set it to 20 it will oscillate 20 times per second around the target position. You should never go higher than half of your simulation frequency (otherwise: instability).

Damping prevents overshoot. A damping of 0 means that the body will oscillate forever around its target position (in theory that is, with the integrator that Jolt uses that won't happen).
A damping of 1 will barely overshoot the target and have almost no oscillation, but it will also take slightly longer to reach the target. I would say sensible ranges for frequency are 0.1-20 and for damping 0-1."
*/

#include "PINT_Jolt.h"

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation.
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
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

#include "..\PINT_Common\PINT_Ice.h"
//#include "..\PINT_Common\PINT_Common.cpp"

#include "..\PINT_Common\PINT_Common.h"
//#include "..\PINT_Common\PINT_IceAllocatorSwitch.h"
#include "..\PintShapeRenderer.h"

#include <iostream>
#include <cstdarg>
#include <thread>

using namespace JPH;

static const udword gNbActorData = sizeof(JoltPint::ActorData)/sizeof(udword);

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
		#pragma comment(lib, "../../../../PEEL_Externals/JoltPhysics/Build/VS2022_CL/Debug/Jolt.lib")
	#else
		#pragma comment(lib, "../../../../PEEL_Externals/JoltPhysics/Build/VS2022_CL/Distribution/Jolt.lib")
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
// Using 1/4 iterations here following this document: https://jrouwe.nl/jolt/JoltPhysicsMulticoreScaling.pdf
static udword	gNbPosIter					= 1;		// Default value in Jolt = 2
static udword	gNbVelIter					= 4;		// Default value in Jolt = 10. "Note that this needs to be >= 2 in order for friction to work (friction is applied using the non-penetration impulse from the previous iteration)"
static float	gLinearDamping				= 0.1f;		// Same default value as in PEEL. Jolt default is 0.05.
static float	gAngularDamping				= 0.05f;	// Same default value as in PEEL.
static float	gSpeculativeContactDistance	= 0.02f;	// Default value in Jolt
static float	gMaxPenetrationDistance		= 0.2f;		// Default value in Jolt
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
	static constexpr uint8 UNUSED1 = 0; // 4 unused values so that broadphase layers values don't match with object layer values (for testing purposes)
	static constexpr uint8 UNUSED2 = 1;
	static constexpr uint8 UNUSED3 = 2;
	static constexpr uint8 UNUSED4 = 3;
	static constexpr uint8 NON_MOVING = 4;
	static constexpr uint8 MOVING = 5;
	static constexpr uint8 DEBRIS = 6; // Example: Debris collides only with NON_MOVING
	static constexpr uint8 NUM_LAYERS = 7;
};

/// Function that determines if two object layers can collide
inline bool MyObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2)
{
	switch (inObject1)
	{
	case Layers::UNUSED1:
	case Layers::UNUSED2:
	case Layers::UNUSED3:
	case Layers::UNUSED4:
		return false;
	case Layers::NON_MOVING:
		return inObject2 == Layers::MOVING || inObject2 == Layers::DEBRIS;
	case Layers::MOVING:
		return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING;
	case Layers::DEBRIS:
		return inObject2 == Layers::NON_MOVING;
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
	static constexpr BroadPhaseLayer DEBRIS(2);
	static constexpr BroadPhaseLayer UNUSED(3);
	static constexpr uint NUM_LAYERS(4);
};

/// BroadPhaseLayerInterface implementation
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
									BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::UNUSED1] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::UNUSED2] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::UNUSED3] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::UNUSED4] = BroadPhaseLayers::UNUSED;
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		mObjectToBroadPhase[Layers::DEBRIS] = BroadPhaseLayers::DEBRIS;
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
		case (BroadPhaseLayer::Type)BroadPhaseLayers::DEBRIS:		return "DEBRIS";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::UNUSED:		return "UNUSED";
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
	case Layers::DEBRIS:
		return inLayer2 == BroadPhaseLayers::NON_MOVING;
	case Layers::UNUSED1:
	case Layers::UNUSED2:
	case Layers::UNUSED3:
		return false;			
	default:
		JPH_ASSERT(false);
		return false;
	}
}

////

// An example contact listener
class MyContactListener : public ContactListener
{
public:
	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, const CollideShapeResult &inCollisionResult) override
	{
		cout << "Contact validate callback" << endl;

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
		cout << "A contact was added" << endl;
	}

	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
	{
		cout << "A contact was persisted" << endl;
	}

	virtual void			OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
	{ 
		cout << "A contact was removed" << endl;
	}
};

// An example activation listener
class MyBodyActivationListener : public BodyActivationListener
{
public:
	virtual void		OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
		cout << "A body got activated" << endl;
	}

	virtual void		OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
	{
		cout << "A body went to sleep" << endl;
	}
};

///////////////////////////////////////////////////////////////////////////////

static JobSystemThreadPool* gJobSystem = null;
static PhysicsSystem* gPhysicsSystem = null;

// Custom filtering, based on Jolt's GroupFilterTable code + new support for filtering of jointed bodies
class MyGroupFilterTable final : public GroupFilter
{
private:
	using GroupID = CollisionGroup::GroupID;
	using SubGroupID = CollisionGroup::SubGroupID;

/*	int		GetBit(SubGroupID inSubGroup1, SubGroupID inSubGroup2) const
	{
		if (inSubGroup1 > inSubGroup2)
			swap(inSubGroup1, inSubGroup2);
		JPH_ASSERT(inSubGroup2 < mNumSubGroups);
		return (inSubGroup2 * (inSubGroup2 - 1)) / 2 + inSubGroup1;
	}*/

public:
	explicit	MyGroupFilterTable(uint inNumSubGroups = 0) : mNumSubGroups(inNumSubGroups), mPM(null)
	{
		// TODO: for some insane reason mTable doesn't work in the pile-of-ragdolls scene. Looks like "group 0" is not handled properly. No time to waste on this.
//		int table_size = ((inNumSubGroups * (inNumSubGroups - 1)) / 2 + 7) / 8;
//		mTable.resize(table_size, 0xff);
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
		//int bit = GetBit(inSubGroup1, inSubGroup2);
		//mTable[bit >> 3] &= (0xff ^ (1 << (bit & 0b111)));
		mMyTable[inSubGroup1][inSubGroup2] = mMyTable[inSubGroup2][inSubGroup1] = false;
	}

	void	EnableCollision(SubGroupID inSubGroup1, SubGroupID inSubGroup2)
	{
		//int bit = GetBit(inSubGroup1, inSubGroup2);
		//mTable[bit >> 3] |= 1 << (bit & 0b111);
		mMyTable[inSubGroup1][inSubGroup2] = mMyTable[inSubGroup2][inSubGroup1] = true;
	}

	void	DisableJointedBodies(udword id0, udword id1)
	{
		if(!mPM)
			mPM = new PairManager;
		mPM->AddPair(id0, id1);
	}

	virtual bool			CanCollide(const CollisionGroup &inGroup1, const CollisionGroup &inGroup2) const override
	{	
		// Filter bodies connected by a joint
		if(mPM && mPM->FindPair(inGroup1.GetGroupID(), inGroup2.GetGroupID()))
			return false;

		return mMyTable[inGroup1.GetSubGroupID()][inGroup2.GetSubGroupID()];

//		int bit = GetBit(inGroup1.GetSubGroupID(), inGroup2.GetSubGroupID());
//		return (mTable[bit >> 3] & (1 << (bit & 0b111))) != 0;

/*		int bit = GetBit(inGroup1.GetSubGroupID(), inGroup2.GetSubGroupID());
		bool b = (mTable[bit >> 3] & (1 << (bit & 0b111))) != 0;
		if(!b)
		{
			if(!inGroup1.GetSubGroupID() || !inGroup2.GetSubGroupID())
			{
				int stop=1;
				(void)stop;
				return true;
			}
		}

		return true;*/
	}

private:
	uint			mNumSubGroups;
	//vector<uint8>	mTable;	// wtf doesn't work for group=0 ?
	bool			mMyTable[32][32];
	PairManager*	mPM;
};

static MyGroupFilterTable* gGroupFilter = null;
//static Ref<GroupFilterTable> gGroupFilter;

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
/*	caps.mSupportDeformableMeshes		= true;
	caps.mSupportHeightfields			= true;
	caps.mSupportContactNotifications	= true;
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
/*	caps.mSupportD6Joints				= true;
	caps.mSupportGearJoints				= true;
	caps.mSupportRackJoints				= true;
	caps.mSupportPortalJoints			= true;
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

	gGroupFilter = new MyGroupFilterTable(32);
	//printf("GroupFilter: %d\n", gGroupFilter->GetRefCount());

	// Now we can create the actual physics system.
	gPhysicsSystem = new PhysicsSystem;
	gPhysicsSystem->Init(gMaxBodies, gNbBodyMutexes, gMaxBodyPairs, gMaxContactConstraints, gBroadPhaseLayerInterface, MyBroadPhaseCanCollide, MyObjectCanCollide);

	gPhysicsSystem->SetGravity(ToVec3(desc.mGravity));

	PhysicsSettings Settings;
	Settings.mNumVelocitySteps				= gNbVelIter;
	Settings.mNumPositionSteps				= gNbPosIter;
	Settings.mAllowSleeping					= gAllowSleeping;
	Settings.mSpeculativeContactDistance	= gSpeculativeContactDistance;
	Settings.mMaxPenetrationDistance		= gMaxPenetrationDistance;
	Settings.mBaumgarte						= gBaumgarte;

/*
struct PhysicsSettings
{
	/// Size of body pairs array, corresponds to the maximum amount of potential body pairs that can be in flight at any time.
	/// Setting this to a low value will use less memory but slow down simulation as threads may run out of narrow phase work.
	int			mMaxInFlightBodyPairs = 16384;

	/// How many PhysicsStepListeners to notify in 1 batch
	int			mStepListenersBatchSize = 8;

	/// How many step listener batches are needed before spawning another job (set to INT_MAX if no parallelism is desired)
	int			mStepListenerBatchesPerJob = 1;

	/// How much bodies are allowed to sink into eachother (unit: meters)
	float		mPenetrationSlop = 0.02f;

	/// Fraction of its inner radius a body must move per step to enable casting for the LinearCast motion quality
	float		mLinearCastThreshold = 0.75f;

	/// Fraction of its inner radius a body may penetrate another body for the LinearCast motion quality
	float		mLinearCastMaxPenetration = 0.25f;

	/// Max squared distance to use to determine if two points are on the same plane for determining the contact manifold between two shape faces (unit: meter^2)
	float		mManifoldToleranceSq = 1.0e-6f;

	/// Maximum relative delta position for body pairs to be able to reuse collision results from last frame (units: meter^2)
	float		mBodyPairCacheMaxDeltaPositionSq = Square(0.001f); ///< 1 mm

	/// Maximum relative delta orientation for body pairs to be able to reuse collision results from last frame, stored as cos(max angle / 2)
	float		mBodyPairCacheCosMaxDeltaRotationDiv2 = 0.99984769515639123915701155881391f; ///< cos(2 degrees / 2)

	/// Maximum angle between normals that allows manifolds between different sub shapes of the same body pair to be combined
	float		mContactNormalCosMaxDeltaRotation = 0.99619469809174553229501040247389f; ///< cos(5 degree)

	/// Maximum allowed distance between old and new contact point to preserve contact forces for warm start (units: meter^2)
	float		mContactPointPreserveLambdaMaxDistSq = Square(0.01f); ///< 1 cm

	/// Minimal velocity needed before a collision can be elastic (unit: m)
	float		mMinVelocityForRestitution = 1.0f;

	/// Time before object is allowed to go to sleep (unit: seconds)
	float		mTimeBeforeSleep = 0.5f;

	/// Velocity of points on bounding box of object below which an object can be considered sleeping (unit: m/s)
	float		mPointVelocitySleepThreshold = 0.03f;

	///@name These variables are mainly for debugging purposes, they allow turning on/off certain subsystems. You probably want to leave them alone.
	///@{

	/// Whether or not to use warm starting for constraints (initially applying previous frames impulses)
	bool		mConstraintWarmStart = true;

	/// Whether or not to use the body pair cache, which removes the need for narrow phase collision detection when orientation between two bodies didn't change
	bool		mUseBodyPairContactCache = true;

	/// Whether or not to reduce manifolds with similar contact normals into one contact manifold
	bool		mUseManifoldReduction = true;

	/// When false, we prevent collision against non-active (shared) edges. Mainly for debugging the algorithm.
	bool		mCheckActiveEdges = true;

	///@}
};
*/
	gPhysicsSystem->SetPhysicsSettings(Settings);


/*	// A body activation listener gets notified when bodies activate and go to sleep
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	MyBodyActivationListener body_activation_listener;
	physics_system.SetBodyActivationListener(&body_activation_listener);

	// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	MyContactListener contact_listener;
	physics_system.SetContactListener(&contact_listener);

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	BodyInterface& body_interface = gPhysicsSystem->GetBodyInterface();

	// Optional step: Before starting the physics simulation you can optimize the broad phase. This improves collision detection performance (it's pointless here because we only have 2 bodies).
	// You should definitely not call this every frame or when e.g. streaming in a new level section as it is an expensive operation.
	// Instead insert all new objects in batches instead of 1 at a time to keep the broad phase efficient.
	physics_system.OptimizeBroadPhase();
*/

	if(0)
	{
		struct Friction
		{
			static float CombineFunction(const Body &inBody1, const SubShapeID &inSubShapeID1, const Body &inBody2, const SubShapeID &inSubShapeID2)
			{
				float f0 = inBody1.GetFriction();
				float f1 = inBody2.GetFriction();
				return 1.0f;
			}
		};
		gPhysicsSystem->SetCombineFriction(Friction::CombineFunction);
	}
}

void JoltPint::Close()
{
	mActors.Empty();

	DELETESINGLE(gPhysicsSystem);
	//printf("GroupFilter: %d\n", gGroupFilter->GetRefCount());
	//DELETESINGLE(gGroupFilter);
	gGroupFilter = null;
	DELETESINGLE(gJobSystem);
	DELETESINGLE(gTempAllocator);
	DELETESINGLE(Factory::sInstance);

	{
//		AllocSwitch _;

//		DeleteOwnedObjects<EmbreeMesh>(mMeshes);
//		DeleteOwnedObjects<EmbreeActor>(mActors);
	}
}

void JoltPint::SetGravity(const Point& gravity)
{
	if(gPhysicsSystem)
		gPhysicsSystem->SetGravity(ToVec3(gravity));
}

udword JoltPint::Update(float dt)
{
	if(gPhysicsSystem)
	{
		// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
		const int cCollisionSteps = 1;

		// If you want more accurate step results you can do multiple sub steps within a collision step. Usually you would set this to 1.
		const int cIntegrationSubSteps = gNbSubsteps;

		// Step the world
		gPhysicsSystem->Update(dt, cCollisionSteps, cIntegrationSubSteps, gTempAllocator, gJobSystem);
	}

	// TODO: this is only the per-frame temporary memory, it doesn't take into account the persistent data (bodies, triangle meshes, etc)
	return gTempAllocator->mHighWaterMark;
}

Point JoltPint::GetMainColor()
{
	return Point(0.5f, 0.9f, 0.8f);
}

static inline_ void BindRenderer(JPH::Shape* shape, udword, PintShapeRenderer* renderer)
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

	BodyInterface& body_interface = gPhysicsSystem->GetBodyInterface();

	const udword NbActors = mActors.GetNbEntries()/gNbActorData;
	const JoltPint::ActorData* AD = (const JoltPint::ActorData*)mActors.GetEntries();
	for(udword i=0;i<NbActors;i++)
	{
		const Body* Current = reinterpret_cast<const Body*>(AD[i].mBody);
		if(!renderer.SetCurrentActor(PintActorHandle(Current)))
			continue;

		const JPH::Shape* S = Current->GetShape();
		// TODO: revisit this one
//		if(!renderer.SetCurrentShape(PintShapeHandle(S)))
//			continue;

		const Vec3 outPosition = Current->GetPosition();
		const JQuat outRotation = Current->GetRotation();

		const EShapeType Type = S->GetType();
		const EShapeSubType SubType = S->GetSubType();

		if(Type==EShapeType::Compound && SubType==EShapeSubType::StaticCompound)
		{
			const StaticCompoundShape* SCS = static_cast<const StaticCompoundShape*>(S);

			const udword NbSubShapes = SCS->GetNumSubShapes();
			for(udword j=0;j<NbSubShapes;j++)
			{
				const StaticCompoundShape::SubShape& SS = SCS->GetSubShape(j);

				const EShapeType Type2 = SS.mShape->GetType();
				const EShapeSubType SubType2 = SS.mShape->GetSubType();
				if(Type2==EShapeType::Decorated && SubType2==EShapeSubType::RotatedTranslated)
				{
					const JPH::Shape* SSS = SS.mShape;

					const RotatedTranslatedShape* RTS2 = static_cast<const RotatedTranslatedShape*>(SSS);

					const JPH::Shape* InnerShape = RTS2->GetInnerShape();
					PintShapeRenderer* Renderer = RetrieveRenderer(*this, InnerShape, SSS->GetUserData());
					if(Renderer)
					{
						const PR Pose(ToPoint(outPosition), ToIQuat(outRotation));
						const PR Pose2(ToPoint(RTS2->GetPosition()), ToIQuat(RTS2->GetRotation()));

						// TODO: isn't there an official way to compute this in Jolt? Looks clumsy.
						PR Pose3 = Pose2;
						Pose3 *= Pose;

						renderer.DrawShape(Renderer, Pose3);
					}
				}
				else
				{
					//PintShapeRenderer* Renderer = RetrieveRenderer(*this, SS.mShape, SCS->GetSubShapeUserData(j));
					PintShapeRenderer* Renderer = RetrieveRenderer(*this, SS.mShape, SS.mShape->GetUserData());
					if(Renderer)
					{
						const PR Pose(ToPoint(outPosition), ToIQuat(outRotation));
						const PR Pose2(ToPoint(SS.GetPositionCOM()), ToIQuat(SS.GetRotation()));

						// TODO: isn't there an official way to compute this in Jolt? Looks clumsy.
						PR Pose3 = Pose2;
						Pose3 *= Pose;

						renderer.DrawShape(Renderer, Pose3);
					}
				}
			}


		}
		else
		{
			if(Type==EShapeType::Decorated && SubType==EShapeSubType::RotatedTranslated)
			{
				const RotatedTranslatedShape* RTS = static_cast<const RotatedTranslatedShape*>(S);

				const JPH::Shape* InnerShape = RTS->GetInnerShape();
				PintShapeRenderer* Renderer = RetrieveRenderer(*this, InnerShape, S->GetUserData());
				if(Renderer)
				{
					const PR Pose(ToPoint(outPosition), ToIQuat(outRotation));
					const PR Pose2(ToPoint(RTS->GetPosition()), ToIQuat(RTS->GetRotation()));

					// TODO: isn't there an official way to compute this in Jolt? Looks clumsy.
					PR Pose3 = Pose2;
					Pose3 *= Pose;

					renderer.DrawShape(Renderer, Pose3);
				}
			}
/*			else if(Type==EShapeType::Decorated && SubType==EShapeSubType::OffsetCenterOfMass)
			{
				const OffsetCenterOfMassShape* RTS = static_cast<const OffsetCenterOfMassShape*>(S);

				const JPH::Shape* InnerShape = RTS->GetInnerShape();
				PintShapeRenderer* Renderer = RetrieveRenderer(*this, InnerShape, S->GetUserData());
				if(Renderer)
				{
					const PR Pose(ToPoint(outPosition), ToIQuat(outRotation));
					const PR Pose2(ToPoint(RTS->GetPosition()), ToIQuat(RTS->GetRotation()));

					// TODO: isn't there an official way to compute this in Jolt? Looks clumsy.
					PR Pose3 = Pose2;
					Pose3 *= Pose;

					renderer.DrawShape(Renderer, Pose3);
				}
			}*/
			else
			{
				PintShapeRenderer* Renderer = RetrieveRenderer(*this, S, S->GetUserData());
				if(Renderer)
				{
					const PR Pose(ToPoint(outPosition), ToIQuat(outRotation));
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

//	ObjectLayer				mObjectLayer = 0;												///< The collision layer this body belongs to (determines if two objects can collide)
//	CollisionGroup			mCollisionGroup;												///< The collision group this body belongs to (determines if two objects can collide)

	settings.mMaxLinearVelocity = MAX_FLOAT;	// Some PEEL tests require more than Jolt's refault
	settings.mMaxAngularVelocity = 100.0f;
	//BodyCreate.mGravityFactor = 

/*
///@name Mass properties of the body (by default calculated by the shape)
EOverrideMassProperties	mOverrideMassProperties = EOverrideMassProperties::CalculateMassAndInertia; ///< Determines how mMassPropertiesOverride will be used
float					mInertiaMultiplier = 1.0f;										///< When calculating the inertia (not when it is provided) the calculated inertia will be multiplied by this value
MassProperties			mMassPropertiesOverride;										///< Contains replacement mass settings which override the automatically calculated values
*/
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

/*	if(0)
	{
		const OffsetCenterOfMassShapeSettings OffCOMSettings(Vec3(-1, 0, 0), shape);
		const OffsetCenterOfMassShapeSettings::ShapeResult r = OffCOMSettings.Create();
		if(r.IsValid())
			shape = r.Get();
		else
			ASSERT(0);
	}*/

}

/*
This crashes (deletes the shape on return):

	static JPH::Shape* CreateMeshShape(const PintSurfaceInterface& surface)

	Ref<JPH::Shape>& shape;
	shape = CreateMeshShape(surface);

But this works:

	static void CreateMeshShape(Ref<JPH::Shape>& shape, const PintSurfaceInterface& surface)

	Ref<JPH::Shape>& shape;
	CreateMeshShape(shape, surface);

This is not cool.
*/

static void CreateMeshShape(Ref<JPH::Shape>& shape, const PintSurfaceInterface& surface)
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

	//NewSettings = new MeshShapeSettings(pts, tris);

	MeshShapeSettings settings(pts, tris);
	settings.mMaxTrianglesPerLeaf = 4;

	shape = settings.Create().Get();
}

static void CreateShape(Ref<JPH::Shape>& shape, JoltPint& pint, const PINT_SHAPE_CREATE* shape_create)
{
	const float inConvexRadius = cDefaultConvexRadius;
	const PhysicsMaterial* inMaterial = nullptr;

	//JPH::Shape* NewShape = null;
//	ShapeSettings* NewSettings = null;
	PintShapeRenderer* Renderer = shape_create->mRenderer;

	bool AllowSharing = gAllowShapeSharing;
	if(shape_create->mSharing==SHAPE_SHARING_YES)
		AllowSharing = true;
	else if(shape_create->mSharing==SHAPE_SHARING_NO)
		AllowSharing = false;

	const PR LocalPose(shape_create->mLocalPos, shape_create->mLocalRot);

	// In PhysX the collision group is stored in PxFilterData, and thus it's used as a part of the shape identifier (when sharing shapes).
	// In Jolt it's only used per-body (not per-shape) so we don't actually need it here.
	const PintCollisionGroup CollisionGroup = 0;

	switch(shape_create->mType)
	{
		case PINT_SHAPE_SPHERE:
		{
			const PINT_SPHERE_CREATE* Create = static_cast<const PINT_SPHERE_CREATE*>(shape_create);

			// TODO: why did we use the collision group here already?
			shape = AllowSharing ? reinterpret_cast<JPH::Shape*>(pint.mSphereShapes.FindShape(Create->mRadius, inMaterial, Renderer, LocalPose, CollisionGroup)) : null;
			if(!shape)
			{
				shape = new SphereShape(Create->mRadius, inMaterial);
				SetupOffsetShape(shape, LocalPose);
				const udword Index = pint.mSphereShapes.RegisterShape(Create->mRadius, shape, inMaterial, Renderer, LocalPose, CollisionGroup);
				BindRenderer(shape, Index, Renderer);
			}
		}
		break;

		case PINT_SHAPE_CAPSULE:
		{
			const PINT_CAPSULE_CREATE* Create = static_cast<const PINT_CAPSULE_CREATE*>(shape_create);

			shape = AllowSharing ? reinterpret_cast<JPH::Shape*>(pint.mCapsuleShapes.FindShape(Create->mRadius, Create->mHalfHeight, inMaterial, Renderer, LocalPose, CollisionGroup)) : null;
			if(!shape)
			{
				shape = new CapsuleShape(Create->mHalfHeight, Create->mRadius, inMaterial);
				SetupOffsetShape(shape, LocalPose);
				const udword Index = pint.mCapsuleShapes.RegisterShape(Create->mRadius, Create->mHalfHeight, shape, inMaterial, Renderer, LocalPose, CollisionGroup);
				BindRenderer(shape, Index, Renderer);
			}
		}
		break;

		case PINT_SHAPE_CYLINDER:
		{
			const PINT_CYLINDER_CREATE* Create = static_cast<const PINT_CYLINDER_CREATE*>(shape_create);

			shape = AllowSharing ? reinterpret_cast<JPH::Shape*>(pint.mCylinderShapes.FindShape(Create->mRadius, Create->mHalfHeight, inMaterial, Renderer, LocalPose, CollisionGroup)) : null;
			if(!shape)
			{
				shape = new CylinderShape(Create->mHalfHeight, Create->mRadius, TMin(inConvexRadius, Create->mHalfHeight), inMaterial);
				SetupOffsetShape(shape, LocalPose);
				const udword Index = pint.mCylinderShapes.RegisterShape(Create->mRadius, Create->mHalfHeight, shape, inMaterial, Renderer, LocalPose, CollisionGroup);
				BindRenderer(shape, Index, Renderer);
			}
		}
		break;

		case PINT_SHAPE_BOX:
		{
			const PINT_BOX_CREATE* Create = static_cast<const PINT_BOX_CREATE*>(shape_create);

			shape = AllowSharing ? reinterpret_cast<JPH::Shape*>(pint.mBoxShapes.FindShape(Create->mExtents, inMaterial, Renderer, LocalPose, CollisionGroup)) : null;
			if(!shape)
			{
				const Vec3 Extents = ToVec3(Create->mExtents);
				shape = new BoxShape(Extents, TMin(inConvexRadius, Extents.ReduceMin()), inMaterial);
				SetupOffsetShape(shape, LocalPose);
				const udword Index = pint.mBoxShapes.RegisterShape(Create->mExtents, shape, inMaterial, Renderer, LocalPose, CollisionGroup);
				BindRenderer(shape, Index, Renderer);
			}
		}
		break;

		case PINT_SHAPE_CONVEX:
		{
			const PINT_CONVEX_CREATE* Create = static_cast<const PINT_CONVEX_CREATE*>(shape_create);

			// TODO: we don't have a mesh ptr here so we rely on the renderer ptr instead. Probably not very reliable.
			shape = AllowSharing ?  reinterpret_cast<JPH::Shape*>(pint.mConvexShapes.FindShape(null, inMaterial, Renderer, LocalPose, CollisionGroup)) : null;
			if(!shape)
			{
				vector<Vec3> pts;
				for(udword i=0;i<Create->mNbVerts;i++)
					pts.push_back(ToVec3(Create->mVerts[i]));

				//NewSettings = new ConvexHullShapeSettings(pts, inConvexRadius, inMaterial);

				const ConvexHullShapeSettings settings(pts, inConvexRadius, inMaterial);
				//const ConvexHullShapeSettings settings(pts, 0.0f, inMaterial);
				//NewShape = settings.Create().Get();
				ConvexHullShapeSettings::ShapeResult r = settings.Create();
				if(r.IsValid())
					shape = r.Get();
				else
				{
					ASSERT(0);
				}

				SetupOffsetShape(shape, LocalPose);

				const udword Index = pint.mConvexShapes.RegisterShape(null, shape, inMaterial, Renderer, LocalPose, CollisionGroup);
				BindRenderer(shape, Index, Renderer);
			}
		}
		break;

		case PINT_SHAPE_MESH:
		{
			const PINT_MESH_CREATE* Create = static_cast<const PINT_MESH_CREATE*>(shape_create);

			// TODO: we don't have a mesh ptr here so we rely on the renderer ptr instead. Probably not very reliable.
			shape = AllowSharing ? reinterpret_cast<JPH::Shape*>(pint.mMeshShapes.FindShape(null, inMaterial, Renderer, LocalPose, CollisionGroup)) : null;
			if(!shape)
			{
				const PintSurfaceInterface&	PSI = Create->GetSurface();

				CreateMeshShape(shape, PSI);

				SetupOffsetShape(shape, LocalPose);

				const udword Index = pint.mMeshShapes.RegisterShape(null, shape, inMaterial, Renderer, LocalPose, CollisionGroup);
				BindRenderer(shape, Index, Renderer);
			}
		}
		break;

		case PINT_SHAPE_MESH2:
		{
			const PINT_MESH_CREATE2* Create = static_cast<const PINT_MESH_CREATE2*>(shape_create);

//			JPH::Shape* S = reinterpret_cast<JPH::Shape*>(Create->mTriangleMesh);
//			shape = S;

			// TODO: we don't have a mesh ptr here so we rely on the renderer ptr instead. Probably not very reliable.
			shape = AllowSharing ? reinterpret_cast<JPH::Shape*>(pint.mMeshShapes.FindShape(null, inMaterial, Renderer, LocalPose, CollisionGroup)) : null;
			if(!shape)
			{
				// TODO: revisit this, it's clumsy.

				const IndexedSurface* IS = reinterpret_cast<const IndexedSurface*>(Create->mTriangleMesh);

				PintSurfaceInterface PSI;
				static_cast<SurfaceInterface&>(PSI) = IS->GetSurfaceInterface();
				//PSI.Init(IS->GetSurfaceInterface());

				CreateMeshShape(shape, PSI);

				SetupOffsetShape(shape, LocalPose);

				const udword Index = pint.mMeshShapes.RegisterShape(null, shape, inMaterial, Renderer, LocalPose, CollisionGroup);
				BindRenderer(shape, Index, Renderer);
			}

		}
		break;

//		PINT_SHAPE_HEIGHTFIELD,

		default:
			ASSERT(0);
		break;
	};
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
		CreateShape(NewShape, *this, ShapeCreate);
	}
	else
	{
		class MyPintShapeEnumerateCallback : public PintShapeEnumerateCallback
		{
			public:
					MyPintShapeEnumerateCallback(JoltPint& pint, PintCollisionGroup collision_group) : mPint(pint), mCollisionGroup(collision_group)	{}
			virtual	~MyPintShapeEnumerateCallback(){}

			virtual	void	ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)
			{
				if(0)
				{
					PINT_SHAPE_CREATE& Create = const_cast<PINT_SHAPE_CREATE&>(create);
					const Point LocalPos = Create.mLocalPos;
					const IQuat LocalRot = Create.mLocalRot;
					Create.mLocalPos.Zero();
					Create.mLocalRot.Identity();
				}

				Ref<JPH::Shape> NewShape;
				CreateShape(NewShape, mPint, &create);

				mCompoundShape.AddShape(Vec3(0.0f, 0.0f, 0.0f), JQuat(0.0f, 0.0f, 0.0f, 1.0f), NewShape);
				//mCompoundShape.AddShape(ToVec3(LocalPos), ToJQuat(LocalRot), NewShape);
			}

			JoltPint&					mPint;
			StaticCompoundShapeSettings	mCompoundShape;
			const PintCollisionGroup	mCollisionGroup;
		};

		MyPintShapeEnumerateCallback CB(*this, desc.mCollisionGroup);
		desc.GetNbShapes(&CB);

		StaticCompoundShapeSettings::ShapeResult r = CB.mCompoundShape.Create();
		NewShape = r.Get();
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

		if(IsDynamic)
		{
			NewBody->SetLinearVelocityClamped(ToVec3(desc.mLinearVelocity));
			NewBody->SetAngularVelocityClamped(ToVec3(desc.mAngularVelocity));

			//const MotionProperties* MP = NewBody->GetMotionProperties();
			//printf("Mass: %f\n", 1.0f/MP->GetInverseMass());
		}

		// TODO: keeping the sequence number for now because I didn't bother flushing the map when objects are deleted. A proper implementation would revisit this.
		NewBody->SetCollisionGroup(CollisionGroup(gGroupFilter, NewBody->GetID().GetIndexAndSequenceNumber(), CollisionGroup::SubGroupID(desc.mCollisionGroup)));
		//printf("GroupFilter: %d\n", gGroupFilter->GetRefCount());

		ActorData* AD = ICE_RESERVE(ActorData, mActors);
		AD->mBody = NewBody;
		//AD->mRenderer = Renderer;
	}

	return PintActorHandle(NewBody);
}

bool JoltPint::ReleaseObject(PintActorHandle handle)
{
	Body* Actor = reinterpret_cast<Body*>(handle);
	ASSERT(Actor);

	// TODO: optimize this
	udword NbActors = mActors.GetNbEntries()/gNbActorData;
	JoltPint::ActorData* AD = (JoltPint::ActorData*)mActors.GetEntries();
	for(udword i=0;i<NbActors;i++)
	{
		if(AD[i].mBody==Actor)
		{
			AD[i] = AD[--NbActors];
			mActors.ForceSize(NbActors*gNbActorData);

			// TODO: release shared shapes here

			BodyInterface& body_interface = gPhysicsSystem->GetBodyInterface();

			const BodyID& ID = Actor->GetID();
			// Remove the body from the physics system. Note that the body itself keeps all of its state and can be re-added at any time.
			body_interface.RemoveBody(ID);
			// Destroy the body. After this the body ID is no longer valid.
			body_interface.DestroyBody(ID);

			return true;
		}
	}
	return false;
}

// TODO: refactor
static void normalToTangents(const Point& n, Point& t1, Point& t2)
{
	const float m_sqrt1_2 = float(0.7071067811865475244008443621048490);
	if(fabsf(n.z) > m_sqrt1_2)
	{
		const float a = n.y*n.y + n.z*n.z;
		const float k = 1.0f/sqrtf(a);
		t1 = Point(0,-n.z*k,n.y*k);
		t2 = Point(a*k,-n.x*t1.z,n.x*t1.y);
	}
	else
	{
		const float a = n.x*n.x + n.y*n.y;
		const float k = 1.0f/sqrtf(a);
		t1 = Point(-n.y*k,n.x*k,0);
		t2 = Point(-n.z*t1.y,n.z*t1.x,a*k);
	}
	t1.Normalize();
	t2.Normalize();
}

// TODO: refactor
/*static JQuat ComputeJointQuat(Body* actor, const Vec3& localAxis)
{
	//find 2 orthogonal vectors.
	//gotta do this in world space, if we choose them
	//separately in local space they won't match up in worldspace.

	const Vec3 axisw = actor ? actor->GetWorldTransform().Multiply3x3(localAxis).Normalized() : localAxis;

	Point normalw, binormalw;
	::normalToTangents(ToPoint(axisw), binormalw, normalw);

	const Vec3 localNormal = actor ? actor->GetWorldTransform().Multiply3x3Transposed(ToVec3(normalw)) : ToVec3(normalw);

	const Mat44 rot(Vec4(localAxis, 0.0f), Vec4(localNormal, 0.0f), Vec4(localAxis.Cross(localNormal), 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
	return rot.GetQuaternion().Normalized();
}*/

static Vec3 ComputeJointWorldSpaceNormal(Body* actor, const Mat44& m, const Vec3& localAxis)
{
	const Vec3 axisw = actor ? m.Multiply3x3(localAxis).Normalized() : localAxis;

	Point normalw, binormalw;
	::normalToTangents(ToPoint(axisw), binormalw, normalw);

	return ToVec3(normalw);
}

PintJointHandle JoltPint::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	Body* Actor0 = reinterpret_cast<Body*>(desc.mObject0);
	Body* Actor1 = reinterpret_cast<Body*>(desc.mObject1);

	if(!Actor0)
		Actor0 = &Body::sFixedToWorld;
	if(!Actor1)
		Actor1 = &Body::sFixedToWorld;

	const Mat44 M0 = Actor0->GetWorldTransform();
	const Mat44 M1 = Actor1->GetWorldTransform();

	Constraint* J = null;

	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			const PINT_SPHERICAL_JOINT_CREATE& jc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);

			PointConstraintSettings settings;
			settings.mPoint1 = Actor0->GetPosition() + M0.Multiply3x3(ToVec3(jc.mLocalPivot0.mPos));
			settings.mPoint2 = Actor1->GetPosition() + M1.Multiply3x3(ToVec3(jc.mLocalPivot1.mPos));

			J = settings.Create(*Actor0, *Actor1);

			gPhysicsSystem->AddConstraint(J);
		}
		break;

		case PINT_JOINT_HINGE:
		{
			const PINT_HINGE_JOINT_CREATE& jc = static_cast<const PINT_HINGE_JOINT_CREATE&>(desc);

			HingeConstraintSettings settings;
			settings.mPoint1 = Actor0->GetPosition() + M0.Multiply3x3(ToVec3(jc.mLocalPivot0));
			settings.mPoint2 = Actor1->GetPosition() + M1.Multiply3x3(ToVec3(jc.mLocalPivot1));

			settings.mHingeAxis1 = M0.Multiply3x3(ToVec3(jc.mLocalAxis0));
			settings.mHingeAxis2 = M1.Multiply3x3(ToVec3(jc.mLocalAxis1));

			settings.mNormalAxis1 = ComputeJointWorldSpaceNormal(Actor0, M0, ToVec3(jc.mLocalAxis0));
			settings.mNormalAxis2 = ComputeJointWorldSpaceNormal(Actor1, M1, ToVec3(jc.mLocalAxis1));

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

			gPhysicsSystem->AddConstraint(NewJoint);
		}
		break;

		case PINT_JOINT_HINGE2:
		{
			const PINT_HINGE2_JOINT_CREATE& jc = static_cast<const PINT_HINGE2_JOINT_CREATE&>(desc);

			HingeConstraintSettings settings;
			settings.mPoint1 = Actor0->GetPosition() + M0.Multiply3x3(ToVec3(jc.mLocalPivot0.mPos));
			settings.mPoint2 = Actor1->GetPosition() + M1.Multiply3x3(ToVec3(jc.mLocalPivot1.mPos));

			const Matrix3x3 LocalFrame0 = jc.mLocalPivot0.mRot;
			const Matrix3x3 LocalFrame1 = jc.mLocalPivot1.mRot;

			//settings.mHingeAxis1 = M0.Multiply3x3(ToVec3(jc.mLocalAxis0));
			//settings.mHingeAxis2 = M1.Multiply3x3(ToVec3(jc.mLocalAxis1));
			settings.mHingeAxis1 = M0.Multiply3x3(ToVec3(LocalFrame0[0]));
			settings.mHingeAxis2 = M1.Multiply3x3(ToVec3(LocalFrame1[0]));

			//settings.mNormalAxis1 = ComputeJointWorldSpaceNormal(Actor0, M0, ToVec3(jc.mLocalAxis0));
			//settings.mNormalAxis2 = ComputeJointWorldSpaceNormal(Actor1, M1, ToVec3(jc.mLocalAxis1));
			settings.mNormalAxis1 = M0.Multiply3x3(ToVec3(LocalFrame0[1]));
			settings.mNormalAxis2 = M1.Multiply3x3(ToVec3(LocalFrame1[1]));

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

			gPhysicsSystem->AddConstraint(NewJoint);
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			const PINT_PRISMATIC_JOINT_CREATE& jc = static_cast<const PINT_PRISMATIC_JOINT_CREATE&>(desc);

			SliderConstraintSettings settings;
			settings.SetPoint(*Actor0, *Actor1);

			if(jc.mLocalAxis0.IsNonZero())
			{
				settings.mSliderAxis1	= M0.Multiply3x3(ToVec3(jc.mLocalAxis0));
				settings.mSliderAxis2	= M0.Multiply3x3(ToVec3(jc.mLocalAxis1));
				settings.mNormalAxis1	= settings.mSliderAxis1.GetNormalizedPerpendicular();
				settings.mNormalAxis2	= settings.mSliderAxis2.GetNormalizedPerpendicular();
			}
			else
				ASSERT(0);

			if(IsPrismaticLimitEnabled(jc.mLimits))
			{
				//if(jc.mSpring.mDamping!=0.0f && jc.mSpring.mStiffness!=0.0f)
				if(1)
				{
					settings.mLimitsMin = jc.mLimits.mMinValue;
					settings.mLimitsMax = jc.mLimits.mMaxValue;
				}
				else
				{
					// ### Trying to emulate PhysX's soft limits with additional spring constraints but mapping of spring params is unclear
					//settings.mSliderAxis
					//	Point::Project
					DistanceConstraintSettings settings;
					settings.mPoint1		= Actor0->GetPosition();
					settings.mPoint2		= Actor1->GetPosition();
					settings.mFrequency		= jc.mSpring.mStiffness;
					settings.mDamping		= jc.mSpring.mDamping;
					const float Mass = 2.5f;
					settings.mFrequency		= sqrtf(jc.mSpring.mStiffness/Mass);
					//settings.mFrequency		= 2.5f;
					//settings.mDamping		= jc.mSpring.mDamping/(2.0f*sqrtf(jc.mSpring.mStiffness*Mass));
					settings.mDamping		= jc.mSpring.mDamping/(2.0f*Mass*settings.mFrequency);
					//settings.mDamping		= 0.5f;
					gPhysicsSystem->AddConstraint(settings.Create(*Actor0, *Actor1));
				}
			}

			J = settings.Create(*Actor0, *Actor1);

			gPhysicsSystem->AddConstraint(J);
		}
		break;

		case PINT_JOINT_FIXED:
		{
			const PINT_FIXED_JOINT_CREATE& jc = static_cast<const PINT_FIXED_JOINT_CREATE&>(desc);

			FixedConstraintSettings settings;

			J = settings.Create(*Actor0, *Actor1);

			gPhysicsSystem->AddConstraint(J);
		}
		break;

		case PINT_JOINT_DISTANCE:
		{
			const PINT_DISTANCE_JOINT_CREATE& jc = static_cast<const PINT_DISTANCE_JOINT_CREATE&>(desc);

			DistanceConstraintSettings settings;
			settings.mPoint1		= Actor0->GetPosition() + M0.Multiply3x3(ToVec3(jc.mLocalPivot0));
			settings.mPoint2		= Actor1->GetPosition() + M1.Multiply3x3(ToVec3(jc.mLocalPivot1));
			settings.mMinDistance	= jc.mLimits.mMinValue <0.0f ? 0.0f : jc.mLimits.mMinValue;
			settings.mMaxDistance	= jc.mLimits.mMaxValue <0.0f ? MAX_FLOAT : jc.mLimits.mMaxValue;
			settings.mFrequency		= 0.0f;
			settings.mDamping		= 0.0f;

			J = settings.Create(*Actor0, *Actor1);

			gPhysicsSystem->AddConstraint(J);
		}
		break;

		default:
			ASSERT(0);
		break;
	}

	if(J && gGroupFilter)
	{
		gGroupFilter->DisableJointedBodies(Actor0->GetID().GetIndexAndSequenceNumber(), Actor1->GetID().GetIndexAndSequenceNumber());
	}

	return PintJointHandle(J);
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

/*	// AFAIK there's no "mesh object" in Jolt so I'll create a shape instead.

	Ref<JPH::Shape> shape;
	CreateMeshShape(shape, desc.GetSurface());

	shape->AddRef();

	if(index)
		*index = INVALID_ID;

	return PintMeshHandle(shape.GetPtr());*/
}

///////////////////////////////////////////////////////////////////////////////

static inline_ void FillResultStruct(const RayCast& raycast, PintRaycastHit& hit, const RayCastResult& result)
{
	const Vec3 outPosition = raycast.mOrigin + result.mFraction * raycast.mDirection;

	hit.mImpact		= ToPoint(outPosition);
	//hit.mDistance	= result.mFraction;
	hit.mDistance	= (outPosition - raycast.mOrigin).Length();

	//BodyLockRead lock(gPhysicsSystem->GetBodyLockInterface(), result.mBodyID);
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
	//const BodyInterface& BI = gPhysicsSystem->GetBodyInterface();

	RayCastSettings inRayCastSettings;
	//inRayCastSettings.mBackFaceMode = EBackFaceMode::IgnoreBackFaces;

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
	//const BodyInterface& BI = gPhysicsSystem->GetBodyInterface();

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

		//const ShapeCast shape_cast { normal_sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(ToVec3(sweeps->mBox.mCenter)), ToVec3(sweeps->mDir * MaxDist) };
		const ShapeCast shape_cast { &QueryShape, Vec3::sReplicate(1.0f), Mat44(v0,v1,v2,v3), ToVec3(sweeps->mDir * MaxDist) };

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
}

///////////////////////////////////////////////////////////////////////////////

udword JoltPint::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	const NarrowPhaseQuery& NPQ = gPhysicsSystem->GetNarrowPhaseQuery();


/*
		{
			// Create shape cast
			Ref<Shape> normal_sphere = new SphereShape(1.0f);
			ShapeCast shape_cast { normal_sphere, Vec3::sReplicate(1.0f), Mat44::sTranslation(Vec3(0, 11, 0)), Vec3(0, 1, 0) };

			// Shape is intersecting at the start
			AllHitCollisionCollector<CastShapeCollector> collector;
			c.GetSystem()->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector);
			CHECK(collector.mHits.size() == 1);
			const ShapeCastResult &result = collector.mHits.front();
			CHECK(result.mBodyID2 == body2.GetID());
			CHECK_APPROX_EQUAL(result.mFraction, 0.0f);
			CHECK_APPROX_EQUAL(result.mPenetrationAxis.Normalized(), Vec3(0, -1, 0), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mPenetrationDepth, 1.0f, 1.0e-5f);
			CHECK_APPROX_EQUAL(result.mContactPointOn1, Vec3(0, 10, 0), 1.0e-3f);
			CHECK_APPROX_EQUAL(result.mContactPointOn2, Vec3(0, 11, 0), 1.0e-3f);
			CHECK(!result.mIsBackFaceHit);
		}

*/





	ShapeCastSettings settings;
	//settings.mReturnDeepestPoint = true;
	//settings.mBackFaceModeTriangles = EBackFaceMode::CollideWithBackFaces;
	//settings.mBackFaceModeConvex = EBackFaceMode::CollideWithBackFaces;
	//settings.mCollisionTolerance = 1.0e-5f; // Increased precision
	//settings.mPenetrationTolerance = 1.0e-5f;

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
			//FillResultStruct(R, *dest, ioHit);

/*			CHECK(collector.mHit.mBodyID2 == bodies.front()->GetID());
			CHECK_APPROX_EQUAL(collector.mHit.mFraction, 4.0f / 10.0f);
			CHECK_APPROX_EQUAL(collector.mHit.mPenetrationAxis.Normalized(), Vec3(1, 0, 0), 2.0e-2f);
			CHECK_APPROX_EQUAL(collector.mHit.mPenetrationDepth, 0.0f);
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn1, Vec3(0, 0, 0));
			CHECK_APPROX_EQUAL(collector.mHit.mContactPointOn2, Vec3(0, 0, 0));
			CHECK(!collector.mHit.mIsBackFaceHit);*/

//				const Vec3 outPosition = ToVec3(sweeps->mSphere.mCenter) + collector.mHit.mFraction * shape_cast.mDirection;
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
}

///////////////////////////////////////////////////////////////////////////////

static IQuat _ShortestRotation(const Point& v0, const Point& v1)
{
	const float d = v0|v1;
	const Point cross = v0^v1;

	IQuat q = d>-1.0f ? IQuat(1.0f + d, cross.x, cross.y, cross.z)
//					: fabsf(v0.x)<0.1f ? Quat(0.0f, 0.0f, v0.z, -v0.y) : Quat(0.0f, v0.y, -v0.x, 0.0f);
					: fabsf(v0.x)<0.1f ? IQuat(0.0f, 0.0f, v0.z, -v0.y) : IQuat(0.0f, v0.y, -v0.x, 0.0f);
//	PxQuat q = d > -1 ? PxQuat(cross.x, cross.y, cross.z, 1 + d) : PxAbs(v0.x) < 0.1f ? PxQuat(0.0f, v0.z, -v0.y, 0.0f)
//	                                                                                  : PxQuat(v0.y, -v0.x, 0.0f, 0.0f);

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
	const EConstraintType JT = Joint->GetType();
	if(JT==EConstraintType::Hinge)
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
	Constraint* Joint = reinterpret_cast<Constraint*>(handle);
	ASSERT(Joint);
	const EConstraintType JT = Joint->GetType();
	if(JT==EConstraintType::Hinge)
	{
		HingeConstraint* Hinge = static_cast<HingeConstraint*>(Joint);
		// See notes in SharedPhysX::SetDriveVelocity
		Hinge->SetTargetAngularVelocity(angular.x);
	}
	else
		ASSERT(0);
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
static IceEditBox* gEditBox_MaxPenetrationDistance = null;
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
	Common_GetFromEditBox(gMaxPenetrationDistance, gEditBox_MaxPenetrationDistance, 0.0f, MAX_FLOAT);
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
//	y += YStepCB;
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
	gEditBox_MaxPenetrationDistance		= CreateEditBox(helper, Main, y, "Max penetration distance:", helper.Convert(gMaxPenetrationDistance), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Baumgarte					= CreateEditBox(helper, Main, y, "Baumgarte:", helper.Convert(gBaumgarte), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Friction					= CreateEditBox(helper, Main, y, "Default friction:", helper.Convert(gDefaultFriction), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Restitution				= CreateEditBox(helper, Main, y, "Default restitution:", helper.Convert(gDefaultRestitution), EDITBOX_FLOAT_POSITIVE);

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
	gEditBox_MaxPenetrationDistance = null;
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
//	WD.mWidth	= WD.mWidth;
//	WD.mHeight	= TCD.mHeight;
//	WD.mLabel	= "Tab";
	WD.mType	= WINDOW_DIALOG;
	IceWindow* TabWindow = ICE_NEW(IceWindow)(WD);
	owner.Register(TabWindow);
//	TabWindow->SetVisible(true);
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
