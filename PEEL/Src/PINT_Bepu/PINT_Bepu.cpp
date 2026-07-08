///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PINT_Bepu.h"

#include "..\PINT_Common\PINT_Ice.h"
#include "..\PINT_Common\PINT_Common.h"
#include "..\PintShapeRenderer.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN64
	#pragma comment(lib, "../../Ice/Lib64/IceCore64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceMaths64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Contact64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Meshmerizer64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceImageWork64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceGUI64.lib")

	// The Bepu import lib is the same for Debug & Release: it is just a NativeAOT import library
	// referencing AbominationInterop.dll (which must live next to PEEL.exe in Build64).
	#ifdef _DEBUG
		#pragma comment(lib, "../../../../PEEL_Externals/Bepu/Lib/x64/Debug/Bepu.lib")
	#else
		#pragma comment(lib, "../../../../PEEL_Externals/Bepu/Lib/x64/Release/Bepu.lib")
	#endif
#else
	#error Bepu plugin is 64-bit only
#endif

///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include <limits>
#include <vector>
#include <unordered_map>
#include <mutex>

#include "BepuPhysics.h"

///////////////////////////////////////////////////////////////////////////////

// PEEL <-> Bepu math conversions. Bepu is right-handed, Y-up, like PEEL. Capsules & cylinders
// are Y-aligned in both, so no local rotation fixup is needed (unlike PhysX).
static inline_ Bepu::Vector3	ToBepu(const Point& p)				{ return Bepu::Vector3(p.x, p.y, p.z);					}
static inline_ Point			ToPoint(const Bepu::Vector3& v)		{ return Point(v.X, v.Y, v.Z);							}
static inline_ Bepu::Quaternion	ToBepu(const Quat& q)				{ return Bepu::Quaternion(q.p.x, q.p.y, q.p.z, q.w);	}
static inline_ Quat				ToQuat(const Bepu::Quaternion& q)	{ Quat r; r.p.x = q.X; r.p.y = q.Y; r.p.z = q.Z; r.w = q.W; return r;	}
static inline_ Bepu::RigidPose	ToBepu(const PR& pr)				{ return Bepu::RigidPose(ToBepu(pr.mPos), ToBepu(pr.mRot));				}
static inline_ PR				ToPR(const Bepu::RigidPose& pose)	{ return PR(ToPoint(pose.Position), ToQuat(pose.Orientation));			}

static inline_ PR Combine(const PR& a, const PR& b)
{
	return PR(a.mPos + a.mRot.Rotate(b.mPos), a.mRot * b.mRot);
}

static inline_ PR InversePR(const PR& p)
{
	const Quat InvRot(p.mRot.w, -p.mRot.p);
	return PR(InvRot.Rotate(-p.mPos), InvRot);
}

static inline_ bool IsIdentity(const PR& p)
{
	return p.mPos.IsZero() && p.mRot.p.IsZero();
}

// Quat to rotation matrix, avoiding dependencies on Ice conversion conventions.
static void QuatToMatrix(const Quat& q, float m[3][3])
{
	const float x = q.p.x, y = q.p.y, z = q.p.z, w = q.w;
	const float x2 = x + x, y2 = y + y, z2 = z + z;
	const float xx = x * x2, xy = x * y2, xz = x * z2;
	const float yy = y * y2, yz = y * z2, zz = z * z2;
	const float wx = w * x2, wy = w * y2, wz = w * z2;
	m[0][0] = 1.0f - (yy + zz);	m[0][1] = xy - wz;			m[0][2] = xz + wy;
	m[1][0] = xy + wz;			m[1][1] = 1.0f - (xx + zz);	m[1][2] = yz - wx;
	m[2][0] = xz - wy;			m[2][1] = yz + wx;			m[2][2] = 1.0f - (xx + yy);
}

// Builds a quaternion from an orthonormal basis given as the world images of the local X/Y/Z axes.
static Quat BasisToQuat(const Point& x, const Point& y, const Point& z)
{
	const float m00 = x.x, m01 = y.x, m02 = z.x;
	const float m10 = x.y, m11 = y.y, m12 = z.y;
	const float m20 = x.z, m21 = y.z, m22 = z.z;
	const float Trace = m00 + m11 + m22;
	Quat q;
	if(Trace>0.0f)
	{
		const float S = sqrtf(Trace + 1.0f) * 2.0f;
		q.w = 0.25f * S;
		q.p.x = (m21 - m12) / S;
		q.p.y = (m02 - m20) / S;
		q.p.z = (m10 - m01) / S;
	}
	else if(m00>m11 && m00>m22)
	{
		const float S = sqrtf(1.0f + m00 - m11 - m22) * 2.0f;
		q.w = (m21 - m12) / S;
		q.p.x = 0.25f * S;
		q.p.y = (m01 + m10) / S;
		q.p.z = (m02 + m20) / S;
	}
	else if(m11>m22)
	{
		const float S = sqrtf(1.0f + m11 - m00 - m22) * 2.0f;
		q.w = (m02 - m20) / S;
		q.p.x = (m01 + m10) / S;
		q.p.y = 0.25f * S;
		q.p.z = (m12 + m21) / S;
	}
	else
	{
		const float S = sqrtf(1.0f + m22 - m00 - m11) * 2.0f;
		q.w = (m10 - m01) / S;
		q.p.x = (m02 + m20) / S;
		q.p.y = (m12 + m21) / S;
		q.p.z = 0.25f * S;
	}
	return q;
}

// Builds a twist basis for Bepu's Twist* constraints: Z = twist axis, X = zero-angle reference.
// Mirrors the CreateBasis helper from the Bepu demos; x does not need to be perpendicular to z.
static Quat CreateTwistBasis(const Point& z, const Point& x)
{
	Point Z = z;
	Z.Normalize();
	Point Y = Z ^ x;
	Y.Normalize();
	const Point X = Y ^ Z;
	return BasisToQuat(X, Y, Z);
}

// Computes S' = R * S * R^T for a symmetric 3x3 matrix S (used for inertia tensors).
static void RotateSymmetric(const Bepu::Symmetric3x3& s, const float r[3][3], Bepu::Symmetric3x3& out)
{
	const float S[3][3] = {	{ s.XX, s.YX, s.ZX },
							{ s.YX, s.YY, s.ZY },
							{ s.ZX, s.ZY, s.ZZ } };
	float RS[3][3];
	for(udword i=0;i<3;i++)
		for(udword j=0;j<3;j++)
			RS[i][j] = r[i][0]*S[0][j] + r[i][1]*S[1][j] + r[i][2]*S[2][j];

	out.XX = RS[0][0]*r[0][0] + RS[0][1]*r[0][1] + RS[0][2]*r[0][2];
	out.YX = RS[1][0]*r[0][0] + RS[1][1]*r[0][1] + RS[1][2]*r[0][2];
	out.YY = RS[1][0]*r[1][0] + RS[1][1]*r[1][1] + RS[1][2]*r[1][2];
	out.ZX = RS[2][0]*r[0][0] + RS[2][1]*r[0][1] + RS[2][2]*r[0][2];
	out.ZY = RS[2][0]*r[1][0] + RS[2][1]*r[1][1] + RS[2][2]*r[1][2];
	out.ZZ = RS[2][0]*r[2][0] + RS[2][1]*r[2][1] + RS[2][2]*r[2][2];
}

///////////////////////////////////////////////////////////////////////////////

// UI-controlled settings
static udword	gNbThreads					= 0;		// 0 = automatic
static udword	gNbSubsteps					= 1;
static udword	gNbVelIter					= 8;
static float	gLinearDamping				= 0.03f;
static float	gAngularDamping				= 0.03f;
static float	gDefaultFriction			= 0.5f;
static float	gDefaultRestitution			= 0.0f;
static float	gContactHertz				= 30.0f;
static float	gContactDampingRatio		= 1.0f;
static float	gMaxRecoveryVelocity		= 2.0f;
static float	gJointHertz					= 30.0f;
static float	gJointDampingRatio			= 1.0f;
static float	gSpeculativeContactDistance	= 0.0f;		// 0 = unbounded
static bool		gAllowSleeping				= true;
static bool		gEnableCCD					= false;
static bool		gFlipMeshWinding			= true;		// Bepu mesh triangles are one-sided, PEEL winding is the other way around
static Bepu::AngularIntegrationMode gAngularMode = Bepu::AngularIntegrationMode::Nonconserving;
// When true, gravity/damping run through the native (C++) scalar callback: one interop transition
// per body per integration pass. When false (default), they run through the builtin managed-side
// implementation, matching what a regular C# user of the engine gets. Kept as an option to make
// the interop overhead directly measurable in PEEL.
static bool		gUseNativeIntegrationCallback = false;
// Bepu budgets a convex manifold's friction as (mu/nbContacts) * sum(normal impulses), i.e. mu
// times the AVERAGE contact impulse. A box resting on a face (4 contacts) thus gets 1/4 of the
// Coulomb friction that PhysX/Jolt would apply for the same mu. Scaling mu by the contact count
// in the manifold callback restores standard Coulomb semantics (mu * total normal force),
// exactly, regardless of how the load distributes over the contacts. Nonconvex manifolds use
// per-contact friction in Bepu and need no compensation.
static bool		gScaleFrictionByContactCount = true;
// Whether contacts are generated between bodies connected by a joint. Engines differ here:
// PhysX disables them by default, Bepu has no automatic mechanism (filtering is up to the
// narrow phase callbacks, i.e. up to us). Default matches the PhysX convention.
static bool		gCollideJointed				= false;
// Integrate unconstrained bodies with the solver's substep count instead of a single full-dt
// step. Off by default (engine demo default); turning it on with several substeps greatly
// reduces the dissipation of the implicit gyroscopic integrator (e.g. Dzhanibekov tumbling).
static bool		gSubstepUnconstrained		= false;
static bool		gDrawContacts				= false;
// Maximum torque joint motors can apply (0 = unlimited). Bepu contacts are soft spring
// constraints, so a motor with a large force budget wins against an obstacle and grinds
// through it; a finite budget lets contacts stall the motor, like PhysX's drive force limit.
// The default suits PEEL-scale scenes (~1kg bodies, ~2m arms, gravity torques ~20 N.m).
static float	gMotorMaxForce				= 100.0f;
// Motor constraint damping. Matches the Bepu demos' envelope (softness ~1e-4); infinite damping
// makes interacting velocity constraints (e.g. motor + gear) pump energy.
static const float gMotorDamping			= 10000.0f;
// Sweep accuracy (distance units). Bepu's default sweep tuning converges to ~1e-5 x shape size
// with up to 25 iterations per candidate pair ("lean towards precision" per the engine comment);
// millimeter accuracy is plenty for PEEL and much cheaper.
static float	gSweepConvergence			= 0.001f;
static const float gSweepMinProgression		= 0.05f;
static const sdword gSweepMaxIterations		= 16;
static bool		gHasGUI						= false;

static inline_ float GetMotorMaxForce()
{
	return gMotorMaxForce>0.0f ? gMotorMaxForce : MAX_FLOAT;
}

///////////////////////////////////////////////////////////////////////////////

class BepuActor;

// Per-collidable data needed by the narrow-phase callbacks (materials & collision groups) and
// by scene queries (owner actor). Indexed by raw Bepu handle value; bodies & statics live in
// separate handle spaces.
struct CollidableData
{
	BepuActor*	mActor;
	float		mFriction;
	float		mRestitution;
	uword		mGroup;

	CollidableData() : mActor(null), mFriction(0.5f), mRestitution(0.0f), mGroup(0)	{}
};

static bool						gInteropInitialized	= false;
static Bepu::SimulationHandle	gSimulation			= { 0 };
static Bepu::BufferPoolHandle	gBufferPool			= { 0 };
static Bepu::ThreadDispatcherHandle	gDispatcher		= { 0 };
static Point					gGravity(0.0f, 0.0f, 0.0f);
static udword					gDisabledGroupMask[32];
static std::vector<CollidableData>	gBodyData;
static std::vector<CollidableData>	gStaticData;

// Cached by PrepareForIntegration, consumed by the velocity integration callbacks.
static Point					gGravityDt(0.0f, 0.0f, 0.0f);
static float					gLinearDampingDt	= 1.0f;
static float					gAngularDampingDt	= 1.0f;

static void PushBuiltinIntegratorState()
{
	if(gSimulation.RawValue && !gUseNativeIntegrationCallback)
	{
		Bepu::Vector3 Gravity = ToBepu(gGravity);
		Bepu::SetBuiltinPoseIntegratorState(&Gravity, gLinearDamping, gAngularDamping);
	}
}

class BepuActor : public Allocateable
{
	public:
		struct ShapeRecord
		{
			PintShapeRenderer*	mRenderer;
			PR					mLocalPose;		// Relative to the Bepu body frame
			bool				mIdentity;
		};

		std::vector<ShapeRecord>	mShapes;
		PR							mBodyToActor;		// ActorPose = BepuPose * mBodyToActor
		Bepu::TypedIndex			mShape;
		sdword						mHandle;			// BodyHandle or StaticHandle value
		float						mMass;
		Bepu::BodyInertia			mLocalInertia;		// Inverse form, dynamics only
		// Per-actor damping overrides (negative = use the global integrator damping). Bepu has no
		// per-body damping, so these are applied as an explicit velocity decay before each step.
		// PEEL's picking tool relies on this (it sets damping 5.0 on the picked object).
		float						mLinearDampingOverride;
		float						mAngularDampingOverride;
		PR							mKinematicTarget;	// Body-space target pose for velocity-driven kinematics
		udword						mArrayIndex;
		bool						mIsStatic;
		bool						mIsKinematic;
		bool						mOwnsShape;
		bool						mIdentityBodyToActor;
		bool						mHasDampingOverride;
		bool						mKinematicTargetPending;	// A new target was set this frame
		bool						mKinematicMoving;			// Velocities were applied last step

	BepuActor() : mHandle(-1), mMass(0.0f), mLinearDampingOverride(-1.0f), mAngularDampingOverride(-1.0f),
		mArrayIndex(INVALID_ID), mIsStatic(false), mIsKinematic(false), mOwnsShape(true), mIdentityBodyToActor(true), mHasDampingOverride(false),
		mKinematicTargetPending(false), mKinematicMoving(false)
	{
		mKinematicTarget.Identity();
		mBodyToActor.Identity();
		mShape.Packed = 0;
		ZeroMemory(&mLocalInertia, sizeof(mLocalInertia));
	}
};

static std::vector<BepuActor*>	gActors;
static std::vector<BepuActor*>	gDampingOverrides;
// Kinematics driven by SetKinematicPose. Targets are converted to velocities each Update (like
// PhysX's setKinematicTarget) so that contacts against them are solved at the velocity level:
// a teleported kinematic looks stationary to the solver, and objects resting on it can only be
// pushed out by the (soft, capped) penetration recovery - they sink through moving platforms.
static std::vector<BepuActor*>	gKinematicTargets;

static void QueueKinematicTarget(BepuActor* actor, const PR& body_target)
{
	actor->mKinematicTarget = body_target;
	if(!actor->mKinematicTargetPending && !actor->mKinematicMoving)
		gKinematicTargets.push_back(actor);
	actor->mKinematicTargetPending = true;
}

static void RegisterDampingOverride(BepuActor* actor)
{
	if(!actor->mHasDampingOverride)
	{
		actor->mHasDampingOverride = true;
		gDampingOverrides.push_back(actor);
	}
}

// Shared triangle mesh wrapper for the PINT_SHAPE_MESH2 / CreateMeshObject path.
class BepuMeshObject : public Allocateable
{
	public:
		Bepu::Mesh			mMesh;
		Bepu::TypedIndex	mShapeIndex;
};

// Convex objects used by convex sweeps/overlaps (CreateConvexObject / PintConvexIndex).
// Bepu recenters hull points on their center of mass, so queries offset the pose by mCenterOffset
// to keep PEEL's original vertex-cloud frame.
class BepuConvexObject : public Allocateable
{
	public:
		Bepu::ConvexHull	mHull;
		Point				mCenterOffset;
};

static std::vector<BepuConvexObject*>	gConvexObjects;	// Indexed by PintConvexIndex, slots can be null after deletion

static inline_ BepuActor* GetActor(PintActorHandle handle)	{ return reinterpret_cast<BepuActor*>(handle);	}

static const CollidableData& GetCollidableData(Bepu::CollidableReference ref)
{
	static const CollidableData DefaultData;
	const udword Index = udword(ref.GetRawHandleValue());
	if(ref.GetMobility()==Bepu::CollidableMobility::Static)
		return Index < gStaticData.size() ? gStaticData[Index] : DefaultData;
	return Index < gBodyData.size() ? gBodyData[Index] : DefaultData;
}

static void RegisterCollidableData(bool is_static, sdword handle, const CollidableData& data)
{
	std::vector<CollidableData>& Array = is_static ? gStaticData : gBodyData;
	if(udword(handle) >= Array.size())
		Array.resize(handle + 256);
	Array[handle] = data;
}

static BepuActor* GetActorFromCollidable(Bepu::CollidableReference ref)
{
	const udword Index = udword(ref.GetRawHandleValue());
	if(ref.GetMobility()==Bepu::CollidableMobility::Static)
		return Index < gStaticData.size() ? gStaticData[Index].mActor : null;
	return Index < gBodyData.size() ? gBodyData[Index].mActor : null;
}

///////////////////////////////////////////////////////////////////////////////
// Joints
///////////////////////////////////////////////////////////////////////////////

class BepuJoint : public Allocateable
{
	public:
		Bepu::ConstraintHandle	mHandle;
		Bepu::ConstraintHandle	mLimitHandle;	// Optional companion constraint (hinge TwistLimit / prismatic LinearAxisLimit)
		Bepu::ConstraintHandle	mMotorHandle;	// Optional motor constraint (e.g. hinge AngularAxisMotor)
		Bepu::ConstraintHandle	mExtraHandle;	// Optional companion constraint (prismatic AngularServo orientation lock)
		Bepu::BodyHandle		mBody0;
		Bepu::BodyHandle		mBody1;
		Bepu::Vector3			mMotorAxisA;	// Hinge axis in body A's frame, for motor updates
		float					mDriveVelocity;
		PintJoint				mType;
		udword					mArrayIndex;
		uint64_t				mPairKey;		// Collidable pair key for contact filtering, 0 if a side is the world
		bool					mMotorEnabled;

	BepuJoint() : mDriveVelocity(0.0f), mArrayIndex(INVALID_ID), mPairKey(0), mMotorEnabled(false)
	{
		mHandle.Value = -1;
		mLimitHandle.Value = -1;
		mMotorHandle.Value = -1;
		mExtraHandle.Value = -1;
		mBody0.Value = -1;
		mBody1.Value = -1;
		mMotorAxisA = Bepu::Vector3(0.0f, 0.0f, 1.0f);
	}
};

static std::vector<BepuJoint*>	gJoints;

// Refcounted set of directly-jointed collidable pairs, used to disable contact generation
// between them. Only mutated outside of Timestep, so the callbacks can read it lock-free.
static std::unordered_map<uint64_t, udword>	gJointedPairs;

static inline_ uint64_t EncodeCollidableKey(bool is_static, sdword handle)
{
	return (is_static ? 0x80000000ull : 0ull) | uint64_t(udword(handle));
}

static inline_ uint64_t EncodePairKey(uint64_t a, uint64_t b)
{
	return a<b ? (a<<32)|b : (b<<32)|a;
}

static uint64_t MakeJointPairKey(const BepuActor* actor0, const BepuActor* actor1)
{
	if(!actor0 || !actor1)
		return 0;	// World-connected: the anchor body has no collidable, nothing to filter
	return EncodePairKey(EncodeCollidableKey(actor0->mIsStatic, actor0->mHandle), EncodeCollidableKey(actor1->mIsStatic, actor1->mHandle));
}

// Contact visualization. Records are written by the narrow phase callbacks (multithreaded)
// and drawn by RenderDebugData.
struct ContactRecord
{
	Point	mPos;
	Point	mNormal;
	float	mDepth;
};
static std::vector<ContactRecord>	gContactRecords;
static std::mutex					gContactMutex;

static Point GetCollidablePosition(Bepu::CollidableReference ref)
{
	if(ref.GetMobility()==Bepu::CollidableMobility::Static)
		return ToPoint(Bepu::GetStatic(gSimulation, ref.GetStaticHandle())->Pose.Position);
	return ToPoint(Bepu::GetBodyDynamics(gSimulation, ref.GetBodyHandle())->Motion.Pose.Position);
}

// Bepu constraints connect bodies only. Joints to the world (null actor) or to static actors are
// routed to a shapeless kinematic body sitting at the origin: since it never moves, attaching
// with world-space offsets is exact.
static Bepu::BodyHandle	gWorldAnchor = { -1 };

static Bepu::BodyHandle GetWorldAnchor()
{
	if(gWorldAnchor.Value<0)
	{
		Bepu::TypedIndex NoShape;
		NoShape.Packed = 0;
		const Bepu::CollidableDescription Collidable(NoShape, 0.0f, MAX_FLOAT, Bepu::ContinuousDetection::Discrete());
		const Bepu::BodyActivityDescription Activity(-1.0f, 32);	// Never sleeps
		gWorldAnchor = Bepu::AddBody(gSimulation, Bepu::BodyDescription::CreateKinematic(Bepu::RigidPose(), Bepu::BodyVelocity(), Collidable, Activity));
	}
	return gWorldAnchor;
}

static PR GetActorWorldPose(const BepuActor* actor)
{
	PR BodyPose;
	if(actor->mIsStatic)
		BodyPose = ToPR(Bepu::GetStatic(gSimulation, Bepu::StaticHandle{ actor->mHandle })->Pose);
	else
		BodyPose = ToPR(Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ actor->mHandle })->Motion.Pose);
	return actor->mIdentityBodyToActor ? BodyPose : Combine(BodyPose, actor->mBodyToActor);
}

// Converts a PEEL actor-local joint frame to the frame of the Bepu body the constraint will be
// attached to, resolving null (world) and static actors to the world anchor body.
static Bepu::BodyHandle ResolveJointFrame(const BepuActor* actor, const PR& pivot_actor, PR& pivot_body)
{
	if(!actor)
	{
		// World joint: PEEL provides the pivot in world space, which is the anchor's local space.
		pivot_body = pivot_actor;
		return GetWorldAnchor();
	}
	if(actor->mIsStatic)
	{
		// Statics never move: fold the static's pose into the anchor-local frame.
		pivot_body = Combine(GetActorWorldPose(actor), pivot_actor);
		return GetWorldAnchor();
	}
	pivot_body = actor->mIdentityBodyToActor ? pivot_actor : Combine(actor->mBodyToActor, pivot_actor);
	return Bepu::BodyHandle{ actor->mHandle };
}

// Same as above for axis directions.
static Point ResolveJointAxis(const BepuActor* actor, const Point& axis_actor)
{
	if(!actor)
		return axis_actor;
	if(actor->mIsStatic)
		return GetActorWorldPose(actor).mRot.Rotate(axis_actor);
	return actor->mIdentityBodyToActor ? axis_actor : actor->mBodyToActor.mRot.Rotate(axis_actor);
}

static PR GetJointBodyPose(Bepu::BodyHandle handle)
{
	return ToPR(Bepu::GetBodyDynamics(gSimulation, handle)->Motion.Pose);
}

///////////////////////////////////////////////////////////////////////////////
// Narrow phase callbacks
///////////////////////////////////////////////////////////////////////////////

static bool NP_AllowContactGeneration(Bepu::SimulationHandle, int32_t, Bepu::CollidableReference a, Bepu::CollidableReference b, float*)
{
	// Bepu can ask about kinematic-kinematic pairs; they cannot generate constraints.
	if(a.GetMobility()!=Bepu::CollidableMobility::Dynamic && b.GetMobility()!=Bepu::CollidableMobility::Dynamic)
		return false;

	const CollidableData& DataA = GetCollidableData(a);
	const CollidableData& DataB = GetCollidableData(b);
	if(gDisabledGroupMask[DataA.mGroup] & (1u<<DataB.mGroup))
		return false;

	if(!gCollideJointed && !gJointedPairs.empty())
	{
		const uint64_t KeyA = EncodeCollidableKey(a.GetMobility()==Bepu::CollidableMobility::Static, sdword(a.GetRawHandleValue()));
		const uint64_t KeyB = EncodeCollidableKey(b.GetMobility()==Bepu::CollidableMobility::Static, sdword(b.GetRawHandleValue()));
		if(gJointedPairs.find(EncodePairKey(KeyA, KeyB))!=gJointedPairs.end())
			return false;
	}
	return true;
}

static bool NP_AllowContactGenerationBetweenChildren(Bepu::SimulationHandle, int32_t, Bepu::CollidablePair, int32_t, int32_t)
{
	return true;
}

static void FillMaterial(Bepu::CollidablePair pair, Bepu::PairMaterialProperties* material)
{
	const CollidableData& DataA = GetCollidableData(pair.A);
	const CollidableData& DataB = GetCollidableData(pair.B);

	material->FrictionCoefficient = (DataA.mFriction + DataB.mFriction) * 0.5f;

	// Bepu has no classical restitution: bounce comes from an undamped contact spring with an
	// unbounded recovery velocity. We approximate restitution r by reducing the spring damping.
	const float Restitution = TMax(DataA.mRestitution, DataB.mRestitution);
	if(Restitution>0.0f)
	{
		material->ContactSpringSettings = Bepu::SpringSettings(gContactHertz, (1.0f - Restitution) * gContactDampingRatio);
		material->MaximumRecoveryVelocity = MAX_FLOAT;
	}
	else
	{
		material->ContactSpringSettings = Bepu::SpringSettings(gContactHertz, gContactDampingRatio);
		material->MaximumRecoveryVelocity = gMaxRecoveryVelocity;
	}
}

static bool NP_ConfigureConvexContactManifold(Bepu::SimulationHandle, int32_t, Bepu::CollidablePair pair, Bepu::ConvexContactManifold* manifold, Bepu::PairMaterialProperties* material)
{
	FillMaterial(pair, material);
	if(gScaleFrictionByContactCount && manifold->Count>1)
		material->FrictionCoefficient *= float(manifold->Count);

	if(gDrawContacts && manifold->Count)
	{
		// Manifold offsets are world-space offsets from collidable A's position.
		const Point PosA = GetCollidablePosition(pair.A);
		std::lock_guard<std::mutex> Lock(gContactMutex);
		for(int32_t i=0;i<manifold->Count;i++)
		{
			ContactRecord Record;
			Record.mPos = PosA + ToPoint(manifold->Contacts[i].Offset);
			Record.mNormal = ToPoint(manifold->Normal);
			Record.mDepth = manifold->Contacts[i].Depth;
			gContactRecords.push_back(Record);
		}
	}
	return true;
}

static bool NP_ConfigureNonconvexContactManifold(Bepu::SimulationHandle, int32_t, Bepu::CollidablePair pair, Bepu::NonconvexContactManifold* manifold, Bepu::PairMaterialProperties* material)
{
	FillMaterial(pair, material);

	if(gDrawContacts && manifold->Count)
	{
		const Point PosA = GetCollidablePosition(pair.A);
		std::lock_guard<std::mutex> Lock(gContactMutex);
		for(int32_t i=0;i<manifold->Count;i++)
		{
			ContactRecord Record;
			Record.mPos = PosA + ToPoint(manifold->Contacts[i].Offset);
			Record.mNormal = ToPoint(manifold->Contacts[i].Normal);
			Record.mDepth = manifold->Contacts[i].Depth;
			gContactRecords.push_back(Record);
		}
	}
	return true;
}

static bool NP_ConfigureChildContactManifold(Bepu::SimulationHandle, int32_t, Bepu::CollidablePair, int32_t, int32_t, Bepu::ConvexContactManifold*)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Pose integration callbacks (gravity + damping)
///////////////////////////////////////////////////////////////////////////////

static void PI_PrepareForIntegration(Bepu::SimulationHandle, float dt)
{
	gLinearDampingDt = powf(TClamp(1.0f - gLinearDamping, 0.0f, 1.0f), dt);
	gAngularDampingDt = powf(TClamp(1.0f - gAngularDamping, 0.0f, 1.0f), dt);
	gGravityDt = gGravity * dt;
}

static void PI_IntegrateVelocityScalar(Bepu::SimulationHandle, int32_t, Bepu::Vector3, Bepu::Quaternion, Bepu::BodyInertia, int32_t, float, Bepu::BodyVelocity* velocity)
{
	velocity->Linear.X = (velocity->Linear.X + gGravityDt.x) * gLinearDampingDt;
	velocity->Linear.Y = (velocity->Linear.Y + gGravityDt.y) * gLinearDampingDt;
	velocity->Linear.Z = (velocity->Linear.Z + gGravityDt.z) * gLinearDampingDt;
	velocity->Angular.X *= gAngularDampingDt;
	velocity->Angular.Y *= gAngularDampingDt;
	velocity->Angular.Z *= gAngularDampingDt;
}

template<class VelocityT, class MaskT, udword LaneCount>
static inline_ void IntegrateVelocityBundle(const MaskT& mask, VelocityT* velocities)
{
	const int32_t* Mask = &mask.V0;
	float* LX = (float*)&velocities->Linear.X;
	float* LY = (float*)&velocities->Linear.Y;
	float* LZ = (float*)&velocities->Linear.Z;
	float* AX = (float*)&velocities->Angular.X;
	float* AY = (float*)&velocities->Angular.Y;
	float* AZ = (float*)&velocities->Angular.Z;
	for(udword i=0;i<LaneCount;i++)
	{
		if(!Mask[i])
			continue;
		LX[i] = (LX[i] + gGravityDt.x) * gLinearDampingDt;
		LY[i] = (LY[i] + gGravityDt.y) * gLinearDampingDt;
		LZ[i] = (LZ[i] + gGravityDt.z) * gLinearDampingDt;
		AX[i] *= gAngularDampingDt;
		AY[i] *= gAngularDampingDt;
		AZ[i] *= gAngularDampingDt;
	}
}

static void PI_IntegrateVelocitySIMD128(Bepu::SimulationHandle, Bepu::Vector128I, Bepu::Vector3SIMD128*, Bepu::QuaternionSIMD128*, Bepu::BodyInertiaSIMD128*, Bepu::Vector128I mask, int32_t, Bepu::Vector128F, Bepu::BodyVelocitySIMD128* velocities)
{
	IntegrateVelocityBundle<Bepu::BodyVelocitySIMD128, Bepu::Vector128I, 4>(mask, velocities);
}

static void PI_IntegrateVelocitySIMD256(Bepu::SimulationHandle, Bepu::Vector256I, Bepu::Vector3SIMD256*, Bepu::QuaternionSIMD256*, Bepu::BodyInertiaSIMD256*, Bepu::Vector256I mask, int32_t, Bepu::Vector256F, Bepu::BodyVelocitySIMD256* velocities)
{
	IntegrateVelocityBundle<Bepu::BodyVelocitySIMD256, Bepu::Vector256I, 8>(mask, velocities);
}

///////////////////////////////////////////////////////////////////////////////

Bepu_SceneAPI::Bepu_SceneAPI(Pint& pint) : Pint_Scene(pint)
{
}

Bepu_SceneAPI::~Bepu_SceneAPI()
{
}

bool Bepu_SceneAPI::AddActors(udword nb_actors, const PintActorHandle* actors)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////

Bepu_ActorAPI::Bepu_ActorAPI(Pint& pint) : Pint_Actor(pint)
{
}

Bepu_ActorAPI::~Bepu_ActorAPI()
{
}

const char* Bepu_ActorAPI::GetName(PintActorHandle handle) const
{
	return null;
}

bool Bepu_ActorAPI::SetName(PintActorHandle handle, const char* name)
{
	return false;
}

bool Bepu_ActorAPI::GetWorldBounds(PintActorHandle handle, AABB& bounds) const
{
	const BepuActor* Actor = GetActor(handle);
	Bepu::Vector3 Min, Max;
	if(Actor->mIsStatic)
		Bepu::GetStaticBoundingBoxInBroadPhase(gSimulation, Bepu::StaticHandle{ Actor->mHandle }, &Min, &Max);
	else
		Bepu::GetBodyBoundingBoxInBroadPhase(gSimulation, Bepu::BodyHandle{ Actor->mHandle }, &Min, &Max);
	bounds.SetMinMax(ToPoint(Min), ToPoint(Max));
	return true;
}

void Bepu_ActorAPI::WakeUp(PintActorHandle handle)
{
	// Sleeping is disabled by default. Waking a slept body isn't exposed by the interop API yet.
}

float Bepu_ActorAPI::GetLinearDamping(PintActorHandle handle) const
{
	const BepuActor* Actor = GetActor(handle);
	return Actor->mLinearDampingOverride>=0.0f ? Actor->mLinearDampingOverride : gLinearDamping;
}

bool Bepu_ActorAPI::SetLinearDamping(PintActorHandle handle, float damping)
{
	BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return false;
	Actor->mLinearDampingOverride = damping;
	RegisterDampingOverride(Actor);
	return true;
}

float Bepu_ActorAPI::GetAngularDamping(PintActorHandle handle) const
{
	const BepuActor* Actor = GetActor(handle);
	return Actor->mAngularDampingOverride>=0.0f ? Actor->mAngularDampingOverride : gAngularDamping;
}

bool Bepu_ActorAPI::SetAngularDamping(PintActorHandle handle, float damping)
{
	BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return false;
	Actor->mAngularDampingOverride = damping;
	RegisterDampingOverride(Actor);
	return true;
}

bool Bepu_ActorAPI::GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space) const
{
	const BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return false;
	const Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
	linear_velocity = ToPoint(Dynamics->Motion.Velocity.Linear);
	return true;
}

bool Bepu_ActorAPI::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)
{
	BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return false;
	Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
	Dynamics->Motion.Velocity.Linear = ToBepu(linear_velocity);
	return true;
}

bool Bepu_ActorAPI::GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space) const
{
	const BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return false;
	const Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
	angular_velocity = ToPoint(Dynamics->Motion.Velocity.Angular);
	return true;
}

bool Bepu_ActorAPI::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)
{
	BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return false;
	Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
	Dynamics->Motion.Velocity.Angular = ToBepu(angular_velocity);
	return true;
}

float Bepu_ActorAPI::GetMass(PintActorHandle handle) const
{
	return GetActor(handle)->mMass;
}

bool Bepu_ActorAPI::GetLocalInertia(PintActorHandle handle, Point& inertia) const
{
	const BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic || Actor->mIsKinematic)
		return false;
	const Bepu::Symmetric3x3& InvI = Actor->mLocalInertia.InverseInertiaTensor;
	inertia.x = InvI.XX!=0.0f ? 1.0f/InvI.XX : 0.0f;
	inertia.y = InvI.YY!=0.0f ? 1.0f/InvI.YY : 0.0f;
	inertia.z = InvI.ZZ!=0.0f ? 1.0f/InvI.ZZ : 0.0f;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

BepuPint::BepuPint() : mSceneAPI(*this), mActorAPI(*this)
{
}

BepuPint::~BepuPint()
{
}

static udword GetNbThreads()
{
	udword NbThreads = gNbThreads;
	if(!NbThreads)
	{
		const udword NbPlatformThreads = udword(Bepu::GetPlatformThreadCount());
		NbThreads = NbPlatformThreads > 4 ? NbPlatformThreads - 1 : NbPlatformThreads;
	}
	return NbThreads;
}

const char* BepuPint::GetName() const
{
	return _F("Bepu2 (%uT)", GetNbThreads());
}

const char* BepuPint::GetUIName() const
{
	return _F("Bepu2 (%uT)", GetNbThreads());
}

void BepuPint::GetCaps(PintCaps& caps) const
{
	caps.mSupportRigidBodySimulation	= true;
	caps.mSupportCylinders				= true;
	caps.mSupportConvexes				= true;
	caps.mSupportMeshes					= true;
	caps.mSupportKinematics				= true;
	caps.mSupportCollisionGroups		= true;
	caps.mSupportCompounds				= true;
	caps.mSupportMassForInertia			= true;
	caps.mSupportRaycasts				= true;
	caps.mSupportBoxSweeps				= true;
	caps.mSupportSphereSweeps			= true;
	caps.mSupportCapsuleSweeps			= true;
	caps.mSupportSphereOverlaps			= true;
	caps.mSupportBoxOverlaps			= true;
	caps.mSupportCapsuleOverlaps		= true;
	caps.mSupportConvexSweeps			= true;
	caps.mSupportConvexOverlaps			= true;
	caps.mSupportSphericalJoints		= true;
	caps.mSupportHingeJoints			= true;
	caps.mSupportFixedJoints			= true;
	caps.mSupportDistanceJoints			= true;
	caps.mSupportPrismaticJoints		= true;
	caps.mSupportGearJoints				= true;
	// No sweeps/overlaps, D6/rack joints, spherical cone limits, or prismatic springs yet.
	// (Bepu has no angular-to-linear coupling constraint for rack-and-pinion.)
}

void BepuPint::Init(const PINT_WORLD_CREATE& desc)
{
	if(!gInteropInitialized)
	{
		Bepu::Initialize();
		gInteropInitialized = true;
	}

	ZeroMemory(gDisabledGroupMask, sizeof(gDisabledGroupMask));
	gGravity = desc.mGravity;
	gWorldAnchor.Value = -1;

	gBufferPool = Bepu::CreateBufferPool(131072, 16);

	udword NbThreads = gNbThreads;
	if(!gHasGUI)
	{
		// Headless usage (no GUI, e.g. the HostTest benchmark): allow overriding settings
		// through the environment.
		char EnvValue[16];
		if(GetEnvironmentVariableA("PEEL_BEPU_THREADS", EnvValue, sizeof(EnvValue)))
			NbThreads = udword(atoi(EnvValue));
		if(GetEnvironmentVariableA("PEEL_BEPU_NATIVE_INTEGRATION", EnvValue, sizeof(EnvValue)))
			gUseNativeIntegrationCallback = EnvValue[0]=='1';
		if(GetEnvironmentVariableA("PEEL_BEPU_SWEEP_CONVERGENCE", EnvValue, sizeof(EnvValue)))
			gSweepConvergence = float(atof(EnvValue));
		if(GetEnvironmentVariableA("PEEL_BEPU_SUBSTEPS", EnvValue, sizeof(EnvValue)))
			gNbSubsteps = udword(atoi(EnvValue));
		if(GetEnvironmentVariableA("PEEL_BEPU_SUBSTEP_UNCONSTRAINED", EnvValue, sizeof(EnvValue)))
			gSubstepUnconstrained = EnvValue[0]=='1';
		if(GetEnvironmentVariableA("PEEL_BEPU_GYRO", EnvValue, sizeof(EnvValue)) && EnvValue[0]=='1')
		{
			gAngularMode = Bepu::AngularIntegrationMode::ConserveMomentumWithGyroscopicTorque;
			gAngularDamping = 0.0f;
			gLinearDamping = 0.0f;
		}
	}
	if(!NbThreads)
	{
		const udword NbPlatformThreads = udword(Bepu::GetPlatformThreadCount());
		NbThreads = NbPlatformThreads > 4 ? NbPlatformThreads - 1 : NbPlatformThreads;
	}
	if(NbThreads>1)
		gDispatcher = Bepu::CreateThreadDispatcher(int32_t(NbThreads), 16384);
	else
		gDispatcher.RawValue = 0;

	Bepu::NarrowPhaseCallbacks NPCallbacks = {};
	NPCallbacks.AllowContactGenerationFunction				= &NP_AllowContactGeneration;
	NPCallbacks.AllowContactGenerationBetweenChildrenFunction = &NP_AllowContactGenerationBetweenChildren;
	NPCallbacks.ConfigureConvexContactManifoldFunction		= &NP_ConfigureConvexContactManifold;
	NPCallbacks.ConfigureNonconvexContactManifoldFunction	= &NP_ConfigureNonconvexContactManifold;
	NPCallbacks.ConfigureChildContactManifoldFunction		= &NP_ConfigureChildContactManifold;

	Bepu::PoseIntegratorCallbacks PICallbacks = {};
	PICallbacks.AngularIntegrationMode				= gAngularMode;
	PICallbacks.AllowSubstepsForUnconstrainedBodies	= gSubstepUnconstrained;
	PICallbacks.IntegrateVelocityForKinematics		= false;
	PICallbacks.PrepareForIntegration				= &PI_PrepareForIntegration;

	// Default: builtin managed-side gravity/damping integration (zero interop transitions in the
	// integration kernel, equivalent to what a regular C# user gets). The native scalar callback
	// (one transition + AoSoA transpose per body per pass) is kept as a measurable option.
	// Note the vectorized native callbacks cannot be used at all: NativeAOT cannot marshal
	// by-value Vector256<T> parameters to unmanaged function pointers.
	if(gUseNativeIntegrationCallback)
	{
		PICallbacks.UseScalarCallback = true;
		PICallbacks.IntegrateVelocityScalar = &PI_IntegrateVelocityScalar;
	}
	else
	{
		PICallbacks.UseBuiltinGravityDampingCallback = true;
	}

	const Bepu::SolveDescription SolveDesc = Bepu::SolveDescription(int32_t(gNbVelIter), int32_t(gNbSubsteps));

	gSimulation = Bepu::CreateSimulation(gBufferPool, NPCallbacks, PICallbacks, SolveDesc, Bepu::SimulationAllocationSizes());

	PushBuiltinIntegratorState();

	// The SIMD width was decided when the NativeAOT DLL was compiled (IlcInstructionSet in
	// AbominationInterop.csproj), not at runtime. This reports what got baked in.
	const Bepu::SIMDWidth Width = Bepu::GetSIMDWidth();
	const udword Bits = Width==Bepu::SIMD128 ? 128 : Width==Bepu::SIMD256 ? 256 : 512;
	printf("Bepu: %d-bit SIMD (%d-wide float bundles), %d threads, %d substep(s) x %d velocity iterations, %s integration\n",
		Bits, Bits/32, NbThreads, gNbSubsteps, gNbVelIter, gUseNativeIntegrationCallback ? "native-callback" : "builtin");
}

void BepuPint::Close()
{
	const udword NbActors = udword(gActors.size());
	for(udword i=0;i<NbActors;i++)
		DELETESINGLE(gActors[i]);
	gActors.clear();
	gDampingOverrides.clear();
	gKinematicTargets.clear();
	gBodyData.clear();
	gStaticData.clear();

	const udword NbConvexes = udword(gConvexObjects.size());
	for(udword i=0;i<NbConvexes;i++)
		DELETESINGLE(gConvexObjects[i]);
	gConvexObjects.clear();

	const udword NbJoints = udword(gJoints.size());
	for(udword i=0;i<NbJoints;i++)
		DELETESINGLE(gJoints[i]);
	gJoints.clear();
	gJointedPairs.clear();
	gContactRecords.clear();
	gWorldAnchor.Value = -1;

	if(gSimulation.RawValue)
	{
		Bepu::DestroySimulation(gSimulation);
		gSimulation.RawValue = 0;
	}
	if(gDispatcher.RawValue)
	{
		Bepu::DestroyThreadDispatcher(gDispatcher);
		gDispatcher.RawValue = 0;
	}
	if(gBufferPool.RawValue)
	{
		// Releases everything still allocated from the pool (meshes, hulls, compound children...).
		Bepu::DestroyBufferPool(gBufferPool);
		gBufferPool.RawValue = 0;
	}
}

void BepuPint::SetGravity(const Point& gravity)
{
	gGravity = gravity;
	PushBuiltinIntegratorState();
}

udword BepuPint::Update(float dt)
{
	// Per-actor damping overrides: explicit velocity decay, applied only to the (few) actors that
	// have one - typically just PEEL's picked object. exp(-damping*dt) behaves sensibly for large
	// damping values like the picking tool's 5.0, unlike the clamped (1-damping)^dt formula used
	// for the global damping.
	const udword NbOverrides = gSimulation.RawValue ? udword(gDampingOverrides.size()) : 0;
	for(udword i=0;i<NbOverrides;i++)
	{
		const BepuActor* Actor = gDampingOverrides[i];
		Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
		if(Actor->mLinearDampingOverride>=0.0f)
		{
			const float Factor = expf(-Actor->mLinearDampingOverride * dt);
			Dynamics->Motion.Velocity.Linear.X *= Factor;
			Dynamics->Motion.Velocity.Linear.Y *= Factor;
			Dynamics->Motion.Velocity.Linear.Z *= Factor;
		}
		if(Actor->mAngularDampingOverride>=0.0f)
		{
			const float Factor = expf(-Actor->mAngularDampingOverride * dt);
			Dynamics->Motion.Velocity.Angular.X *= Factor;
			Dynamics->Motion.Velocity.Angular.Y *= Factor;
			Dynamics->Motion.Velocity.Angular.Z *= Factor;
		}
	}

	if(gDrawContacts)
	{
		std::lock_guard<std::mutex> Lock(gContactMutex);
		gContactRecords.clear();
	}

	// Kinematic targets: convert this frame's target poses into velocities so the solver sees
	// the kinematics moving. Kinematics without a fresh target are stopped.
	if(gSimulation.RawValue && dt>0.0f)
	{
		udword NbKinematics = udword(gKinematicTargets.size());
		for(udword i=0;i<NbKinematics;)
		{
			BepuActor* Actor = gKinematicTargets[i];
			Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
			if(Actor->mKinematicTargetPending)
			{
				const PR Current = ToPR(Dynamics->Motion.Pose);
				const Point LinVel = (Actor->mKinematicTarget.mPos - Current.mPos) / dt;

				Quat Delta = Actor->mKinematicTarget.mRot * Quat(Current.mRot.w, -Current.mRot.p);
				if(Delta.w<0.0f)
				{
					Delta.w = -Delta.w;
					Delta.p = -Delta.p;
				}
				Point AngVel(0.0f, 0.0f, 0.0f);
				const float SinHalf = Delta.p.Magnitude();
				if(SinHalf>1e-6f)
				{
					const float Angle = 2.0f * atan2f(SinHalf, Delta.w);
					AngVel = Delta.p * (Angle / (SinHalf * dt));
				}

				Dynamics->Motion.Velocity.Linear = ToBepu(LinVel);
				Dynamics->Motion.Velocity.Angular = ToBepu(AngVel);
				Actor->mKinematicTargetPending = false;
				Actor->mKinematicMoving = true;
				i++;
			}
			else
			{
				// No new target: stop the kinematic and drop it from the list.
				Dynamics->Motion.Velocity.Linear = Bepu::Vector3(0.0f, 0.0f, 0.0f);
				Dynamics->Motion.Velocity.Angular = Bepu::Vector3(0.0f, 0.0f, 0.0f);
				Actor->mKinematicMoving = false;
				NbKinematics--;
				gKinematicTargets[i] = gKinematicTargets[NbKinematics];
				gKinematicTargets.pop_back();
			}
		}
	}

	if(gSimulation.RawValue)
		Bepu::Timestep(gSimulation, dt, gDispatcher);

	// Snap the driven kinematics onto their exact targets to avoid integration drift.
	{
		const udword NbKinematics = udword(gKinematicTargets.size());
		for(udword i=0;i<NbKinematics;i++)
		{
			const BepuActor* Actor = gKinematicTargets[i];
			if(Actor->mKinematicMoving)
				Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle })->Motion.Pose = ToBepu(Actor->mKinematicTarget);
		}
	}

	// PEEL's timers wrap this whole function, so the memory stat query (which walks the buffer
	// pool's allocations under a lock, plus one pool per worker thread) must not run every frame
	// or it pollutes the timings. Refresh it periodically instead.
	static udword Counter = 0;
	static udword CachedMemory = 0;
	if((Counter++ & 31)==0)
	{
		udword Memory = udword(Bepu::GetAllocatedMemorySizeInPool(gBufferPool));
		if(gDispatcher.RawValue)
			Memory += udword(Bepu::GetAllocatedMemorySizeInThreadDispatcher(gDispatcher));
		CachedMemory = Memory;
	}
	return CachedMemory;
}

Point BepuPint::GetMainColor()
{
	//return Point(254.0f/255.0f, 240.0f/255.0f, 128.0f/255.0f);
	return Point(254.0f/255.0f, 140.0f/255.0f, 128.0f/255.0f);
}

void BepuPint::Render(PintRender& renderer, PintRenderPass render_pass)
{
	if(!gSimulation.RawValue)
		return;

	const udword NbActors = udword(gActors.size());
	for(udword i=0;i<NbActors;i++)
	{
		BepuActor* Actor = gActors[i];
		if(!renderer.SetCurrentActor(PintActorHandle(Actor)))
			continue;

		PR BodyPose;
		if(Actor->mIsStatic)
			BodyPose = ToPR(Bepu::GetStatic(gSimulation, Bepu::StaticHandle{ Actor->mHandle })->Pose);
		else
			BodyPose = ToPR(Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle })->Motion.Pose);

		const udword NbShapes = udword(Actor->mShapes.size());
		for(udword j=0;j<NbShapes;j++)
		{
			const BepuActor::ShapeRecord& Record = Actor->mShapes[j];
			if(!Record.mRenderer)
				continue;
			renderer.DrawShape(Record.mRenderer, Record.mIdentity ? BodyPose : Combine(BodyPose, Record.mLocalPose));
		}
	}
}

void BepuPint::RenderDebugData(PintRender& renderer)
{
	if(!gDrawContacts)
		return;

	// Contacts recorded by the narrow phase callbacks during the last Update. Red = penetrating,
	// green = speculative (negative depth).
	std::lock_guard<std::mutex> Lock(gContactMutex);
	const udword NbContacts = udword(gContactRecords.size());
	for(udword i=0;i<NbContacts;i++)
	{
		const ContactRecord& Record = gContactRecords[i];
		const Point Color = Record.mDepth>=0.0f ? Point(1.0f, 0.2f, 0.2f) : Point(0.2f, 1.0f, 0.2f);
		renderer.DrawLine(Record.mPos, Record.mPos + Record.mNormal*0.5f, Color);
		const float S = 0.1f;
		renderer.DrawLine(Record.mPos - Point(S, 0.0f, 0.0f), Record.mPos + Point(S, 0.0f, 0.0f), Color);
		renderer.DrawLine(Record.mPos - Point(0.0f, S, 0.0f), Record.mPos + Point(0.0f, S, 0.0f), Color);
		renderer.DrawLine(Record.mPos - Point(0.0f, 0.0f, S), Record.mPos + Point(0.0f, 0.0f, S), Color);
	}
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	// Collects shapes from both the legacy linked list and the newer "extra shapes" codepath.
	class ShapeCollector : public PintShapeEnumerateCallback
	{
		public:
		std::vector<const PINT_SHAPE_CREATE*>	mShapes;

		virtual void ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data) override
		{
			mShapes.push_back(&create);
		}
	};
}

// Creates a Bepu shape for one PINT shape. For convex hulls Bepu recenters the points on their
// center of mass: the returned 'recenter_offset' must be added to the shape's local position.
static Bepu::TypedIndex CreateBepuShape(const PINT_SHAPE_CREATE* create, Point& recenter_offset)
{
	recenter_offset.Zero();

	Bepu::TypedIndex Index;
	Index.Packed = 0;

	switch(create->mType)
	{
		case PINT_SHAPE_SPHERE:
		{
			const PINT_SPHERE_CREATE* Create = static_cast<const PINT_SPHERE_CREATE*>(create);
			Bepu::Sphere Shape;
			Shape.Radius = Create->mRadius;
			Index = Bepu::AddSphere(gSimulation, Shape);
		}
		break;

		case PINT_SHAPE_CAPSULE:
		{
			const PINT_CAPSULE_CREATE* Create = static_cast<const PINT_CAPSULE_CREATE*>(create);
			Bepu::Capsule Shape;
			Shape.Radius = Create->mRadius;
			Shape.HalfLength = Create->mHalfHeight;
			Index = Bepu::AddCapsule(gSimulation, Shape);
		}
		break;

		case PINT_SHAPE_CYLINDER:
		{
			const PINT_CYLINDER_CREATE* Create = static_cast<const PINT_CYLINDER_CREATE*>(create);
			Bepu::Cylinder Shape;
			Shape.Radius = Create->mRadius;
			Shape.HalfLength = Create->mHalfHeight;
			Index = Bepu::AddCylinder(gSimulation, Shape);
		}
		break;

		case PINT_SHAPE_BOX:
		{
			const PINT_BOX_CREATE* Create = static_cast<const PINT_BOX_CREATE*>(create);
			// PEEL boxes use half extents, the Bepu::Box constructor takes full sizes.
			Index = Bepu::AddBox(gSimulation, Bepu::Box(Create->mExtents.x*2.0f, Create->mExtents.y*2.0f, Create->mExtents.z*2.0f));
		}
		break;

		case PINT_SHAPE_CONVEX:
		{
			const PINT_CONVEX_CREATE* Create = static_cast<const PINT_CONVEX_CREATE*>(create);
			std::vector<Bepu::Vector3> Pts(Create->mNbVerts);
			for(udword i=0;i<Create->mNbVerts;i++)
				Pts[i] = ToBepu(Create->mVerts[i]);
			Bepu::Vector3 CenterOfMass;
			Bepu::ConvexHull Hull = Bepu::CreateConvexHull(gBufferPool, Bepu::Buffer<Bepu::Vector3>(Pts.data(), int32_t(Create->mNbVerts)), &CenterOfMass);
			recenter_offset = ToPoint(CenterOfMass);
			Index = Bepu::AddConvexHull(gSimulation, Hull);
		}
		break;

		default:
		break;
	}
	return Index;
}

// Builds a Bepu mesh shape from a PEEL surface. The triangle buffer is allocated from the Bepu
// buffer pool because the Mesh keeps referencing it.
static Bepu::Mesh CreateBepuMesh(const PintSurfaceInterface& surface)
{
	const udword NbTris = surface.mNbFaces;
	Bepu::ByteBuffer TriBuffer = Bepu::Allocate(gBufferPool, int32_t(NbTris * sizeof(Bepu::Triangle)));
	Bepu::Buffer<Bepu::Triangle> Triangles(TriBuffer);

	for(udword i=0;i<NbTris;i++)
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

		if(gFlipMeshWinding)
			TSwap(VRef1, VRef2);

		Bepu::Triangle& T = Triangles[int32_t(i)];
		T.A = ToBepu(surface.mVerts[VRef0]);
		T.B = ToBepu(surface.mVerts[VRef1]);
		T.C = ToBepu(surface.mVerts[VRef2]);
	}

	return Bepu::CreateMesh(gBufferPool, Triangles, Bepu::Vector3(1.0f, 1.0f, 1.0f));
}

// Shape volume, used to distribute a compound's total mass over its children like PhysX's
// uniform-density convention. Convex volumes are estimated from the vertex cloud's AABB.
static float GetShapeVolume(const PINT_SHAPE_CREATE* create)
{
	switch(create->mType)
	{
		case PINT_SHAPE_BOX:
		{
			const Point& E = static_cast<const PINT_BOX_CREATE*>(create)->mExtents;
			return 8.0f * E.x * E.y * E.z;
		}
		case PINT_SHAPE_SPHERE:
		{
			const float R = static_cast<const PINT_SPHERE_CREATE*>(create)->mRadius;
			return (4.0f/3.0f) * PI * R * R * R;
		}
		case PINT_SHAPE_CAPSULE:
		{
			const PINT_CAPSULE_CREATE* CC = static_cast<const PINT_CAPSULE_CREATE*>(create);
			return PI * CC->mRadius * CC->mRadius * (2.0f * CC->mHalfHeight + (4.0f/3.0f) * CC->mRadius);
		}
		case PINT_SHAPE_CYLINDER:
		{
			const PINT_CYLINDER_CREATE* CC = static_cast<const PINT_CYLINDER_CREATE*>(create);
			return PI * CC->mRadius * CC->mRadius * 2.0f * CC->mHalfHeight;
		}
		case PINT_SHAPE_CONVEX:
		{
			const PINT_CONVEX_CREATE* CC = static_cast<const PINT_CONVEX_CREATE*>(create);
			if(!CC->mNbVerts)
				return 1.0f;
			Point Min = CC->mVerts[0], Max = CC->mVerts[0];
			for(udword i=1;i<CC->mNbVerts;i++)
			{
				Min = Min.Min(CC->mVerts[i]);
				Max = Max.Max(CC->mVerts[i]);
			}
			const Point D = Max - Min;
			return D.x * D.y * D.z * 0.5f;	// Rough hull-in-box factor
		}
		default:
			return 1.0f;
	}
}

PintActorHandle BepuPint::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	if(!gSimulation.RawValue)
		return null;

	ShapeCollector Collector;
	const udword NbShapes = desc.GetNbShapes(&Collector);
	if(!NbShapes)
		return null;

	const bool IsKinematic = desc.mKinematic;
	const bool IsDynamic = desc.mMass!=0.0f && !IsKinematic;
	const bool IsStatic = !IsDynamic && !IsKinematic;

	BepuActor* Actor = ICE_NEW(BepuActor);
	Actor->mIsStatic = IsStatic;
	Actor->mIsKinematic = IsKinematic;
	Actor->mMass = desc.mMass;

	const PR ActorPose(desc.mPosition, desc.mRotation);

	// Explicit COM offset (dynamics only). A non-zero offset forces the compound path so that the
	// Bepu body origin can be placed on the center of mass, as Bepu requires.
	const Point COMOffset = IsDynamic ? desc.mCOMLocalOffset : Point(0.0f, 0.0f, 0.0f);

	Bepu::TypedIndex ShapeIndex;
	ShapeIndex.Packed = 0;
	PR BodyOffset;	// BepuBodyPose = ActorPose * BodyOffset
	BodyOffset.Identity();
	Bepu::BodyInertia Inertia = {};
	const float MassForInertia = desc.mMassForInertia<0.0f ? desc.mMass : desc.mMassForInertia;

	const PINT_SHAPE_CREATE* FirstShape = Collector.mShapes[0];
	const bool SingleShape = NbShapes==1;
	const bool SingleMesh = SingleShape && (FirstShape->mType==PINT_SHAPE_MESH || FirstShape->mType==PINT_SHAPE_MESH2);

	if(SingleMesh)
	{
		// Meshes cannot be compound children in Bepu, and dynamic meshes aren't supported.
		if(IsDynamic)
		{
			printf("Bepu: dynamic mesh objects are not supported.\n");
			DELETESINGLE(Actor);
			return null;
		}

		if(FirstShape->mType==PINT_SHAPE_MESH)
		{
			const PINT_MESH_CREATE* MeshCreate = static_cast<const PINT_MESH_CREATE*>(FirstShape);
			const Bepu::Mesh MeshData = CreateBepuMesh(MeshCreate->GetSurface());
			ShapeIndex = Bepu::AddMesh(gSimulation, MeshData);
			Actor->mOwnsShape = true;
		}
		else
		{
			const PINT_MESH_CREATE2* MeshCreate = static_cast<const PINT_MESH_CREATE2*>(FirstShape);
			const BepuMeshObject* MeshObject = reinterpret_cast<const BepuMeshObject*>(MeshCreate->mTriangleMesh);
			if(!MeshObject)
			{
				DELETESINGLE(Actor);
				return null;
			}
			ShapeIndex = MeshObject->mShapeIndex;
			Actor->mOwnsShape = false;	// Shared shape, owned by the mesh object
		}

		const PR LocalPose(FirstShape->mLocalPos, FirstShape->mLocalRot);
		BodyOffset = LocalPose;

		BepuActor::ShapeRecord Record;
		Record.mRenderer = FirstShape->mRenderer;
		Record.mLocalPose.Identity();
		Record.mIdentity = true;
		Actor->mShapes.push_back(Record);
	}
	else if(SingleShape && COMOffset.IsZero() && desc.mExplicitInertiaTensor.IsNotUsed())
	{
		// Common fast path: one shape, no explicit mass properties. The shape's local pose (if any)
		// is folded into the body pose. For convex hulls the Bepu recentering offset is folded too,
		// which keeps the body origin on the center of mass.
		Point RecenterOffset;
		ShapeIndex = CreateBepuShape(FirstShape, RecenterOffset);
		if(!ShapeIndex.Exists())
		{
			printf("Bepu: unsupported shape type in CreateObject.\n");
			DELETESINGLE(Actor);
			return null;
		}

		const PR LocalPose(FirstShape->mLocalPos, FirstShape->mLocalRot);
		const PR RecenterPose(RecenterOffset, Quat(Idt));
		BodyOffset = Combine(LocalPose, RecenterPose);

		if(IsDynamic)
			Inertia = Bepu::ComputeConvexInertia(gSimulation, ShapeIndex, MassForInertia);

		BepuActor::ShapeRecord Record;
		Record.mRenderer = FirstShape->mRenderer;
		Record.mLocalPose = PR(Point(-RecenterOffset.x, -RecenterOffset.y, -RecenterOffset.z), Quat(Idt));
		Record.mIdentity = RecenterOffset.IsZero();
		Actor->mShapes.push_back(Record);
	}
	else
	{
		// Compound path: multiple shapes, explicit COM offset, or explicit inertia with a mass local
		// pose. The Bepu body origin is placed on the (given or computed) center of mass.
		Bepu::ByteBuffer ChildBuffer = Bepu::Allocate(gBufferPool, int32_t(NbShapes * sizeof(Bepu::CompoundChild)));
		Bepu::Buffer<Bepu::CompoundChild> Children(ChildBuffer);

		std::vector<PR> ChildPoses(NbShapes);

		for(udword i=0;i<NbShapes;i++)
		{
			const PINT_SHAPE_CREATE* Create = Collector.mShapes[i];
			if(Create->mType==PINT_SHAPE_MESH || Create->mType==PINT_SHAPE_MESH2)
			{
				printf("Bepu: meshes cannot be part of compound objects.\n");
				Bepu::DeallocateById(gBufferPool, ChildBuffer.Id);
				DELETESINGLE(Actor);
				return null;
			}

			Point RecenterOffset;
			Bepu::TypedIndex ChildIndex = CreateBepuShape(Create, RecenterOffset);
			if(!ChildIndex.Exists())
			{
				printf("Bepu: unsupported shape type in compound.\n");
				Bepu::DeallocateById(gBufferPool, ChildBuffer.Id);
				DELETESINGLE(Actor);
				return null;
			}

			const PR LocalPose(Create->mLocalPos, Create->mLocalRot);
			const PR ChildPose = Combine(LocalPose, PR(RecenterOffset, Quat(Idt)));
			ChildPoses[i] = ChildPose;

			Bepu::CompoundChild& Child = Children[int32_t(i)];
			Child.LocalOrientation = ToBepu(ChildPose.mRot);
			Child.LocalPosition = ToBepu(ChildPose.mPos);
			Child.ShapeIndex = ChildIndex;
		}

		Point COM(0.0f, 0.0f, 0.0f);
		if(IsDynamic)
		{
			// Distribute the total mass over the children proportionally to their volumes
			// (uniform density, matching the PhysX convention PEEL tests assume). Equal splitting
			// distorts compound inertia tensors - e.g. the Dzhanibekov T-handle's flip dynamics.
			std::vector<float> ChildMasses(NbShapes);
			float TotalVolume = 0.0f;
			for(udword i=0;i<NbShapes;i++)
			{
				ChildMasses[i] = GetShapeVolume(Collector.mShapes[i]);
				TotalVolume += ChildMasses[i];
			}
			for(udword i=0;i<NbShapes;i++)
				ChildMasses[i] = TotalVolume>0.0f ? MassForInertia * ChildMasses[i] / TotalVolume : MassForInertia / float(NbShapes);
			const Bepu::Buffer<float> MassBuffer(ChildMasses.data(), int32_t(NbShapes));

			// The Bepu body origin must sit on the center of mass. Three cases:
			bool ShiftChildren = false;
			bool ComputeInertiaAfterShift = false;
			if(!desc.mExplicitInertiaTensor.IsNotUsed())
			{
				// 1) Explicit inertia: diagonal tensor expressed in the "mass local pose" frame,
				// whose position is the center of mass.
				const Point& I = desc.mExplicitInertiaTensor;
				Bepu::Symmetric3x3 InvDiag = {};
				InvDiag.XX = I.x!=0.0f ? 1.0f/I.x : 0.0f;
				InvDiag.YY = I.y!=0.0f ? 1.0f/I.y : 0.0f;
				InvDiag.ZZ = I.z!=0.0f ? 1.0f/I.z : 0.0f;

				float R[3][3];
				QuatToMatrix(desc.mExplicitMassLocalPose.mRot, R);
				RotateSymmetric(InvDiag, R, Inertia.InverseInertiaTensor);

				COM = desc.mExplicitMassLocalPose.mPos;
				ShiftChildren = !COM.IsZero();
			}
			else if(!COMOffset.IsZero())
			{
				// 2) Explicit COM offset: shift the children, then compute the inertia about it.
				COM = COMOffset;
				ShiftChildren = true;
				ComputeInertiaAfterShift = true;
			}
			else
			{
				// 3) Nothing explicit: let Bepu compute the COM and recenter the children on it.
				Bepu::Vector3 ComputedCOM;
				Inertia = Bepu::ComputeCompoundInertiaWithRecentering(gSimulation, Children, MassBuffer, &ComputedCOM);
				COM = ToPoint(ComputedCOM);
			}

			if(ShiftChildren)
			{
				for(udword i=0;i<NbShapes;i++)
				{
					Bepu::CompoundChild& Child = Children[int32_t(i)];
					Child.LocalPosition.X -= COM.x;
					Child.LocalPosition.Y -= COM.y;
					Child.LocalPosition.Z -= COM.z;
				}
				if(ComputeInertiaAfterShift)
					Inertia = Bepu::ComputeCompoundInertia(gSimulation, Children, MassBuffer);
			}
		}

		// Small compounds use the simple list-based type, large ones get the tree-accelerated one.
		if(NbShapes<=8)
		{
			Bepu::Compound CompoundShape;
			CompoundShape.Children = Children;
			ShapeIndex = Bepu::AddCompound(gSimulation, CompoundShape);
		}
		else
		{
			Bepu::BigCompound CompoundShape = Bepu::CreateBigCompound(gSimulation, gBufferPool, Children);
			ShapeIndex = Bepu::AddBigCompound(gSimulation, CompoundShape);
		}

		BodyOffset = PR(COM, Quat(Idt));

		for(udword i=0;i<NbShapes;i++)
		{
			const PINT_SHAPE_CREATE* Create = Collector.mShapes[i];
			BepuActor::ShapeRecord Record;
			Record.mRenderer = Create->mRenderer;
			// Renderers use the original (non-recentered) shape geometry.
			Record.mLocalPose = PR(Create->mLocalPos - COM, Create->mLocalRot);
			Record.mIdentity = Record.mLocalPose.mPos.IsZero() && Record.mLocalPose.mRot.p.IsZero();
			Actor->mShapes.push_back(Record);
		}
	}

	// Inverse mass: PEEL's mMassForInertia only affects the inertia tensor, not the mass itself.
	if(IsDynamic)
		Inertia.InverseMass = 1.0f / desc.mMass;

	const PR BodyPose = IsIdentity(BodyOffset) ? ActorPose : Combine(ActorPose, BodyOffset);
	Actor->mBodyToActor = InversePR(BodyOffset);
	Actor->mIdentityBodyToActor = IsIdentity(BodyOffset);
	Actor->mShape = ShapeIndex;

	// Material & collision group, per collidable (from the first shape - Bepu materials are
	// per-pair, not per-shape).
	CollidableData Data;
	Data.mActor = Actor;
	Data.mGroup = desc.mCollisionGroup;
	if(FirstShape->mMaterial)
	{
		Data.mFriction = FirstShape->mMaterial->mDynamicFriction;
		Data.mRestitution = FirstShape->mMaterial->mRestitution;
	}
	else
	{
		Data.mFriction = gDefaultFriction;
		Data.mRestitution = gDefaultRestitution;
	}

	if(IsStatic)
	{
		const Bepu::StaticHandle Handle = Bepu::AddStatic(gSimulation, Bepu::StaticDescription::Create(ToBepu(BodyPose), ShapeIndex));
		Actor->mHandle = Handle.Value;
		RegisterCollidableData(true, Handle.Value, Data);
	}
	else
	{
		const float MaxSpeculativeMargin = gSpeculativeContactDistance>0.0f ? gSpeculativeContactDistance : MAX_FLOAT;
		const Bepu::ContinuousDetection Continuity = gEnableCCD ? Bepu::ContinuousDetection::Continuous(1e-3f, 1e-3f) : Bepu::ContinuousDetection::Passive();
		const Bepu::CollidableDescription Collidable(ShapeIndex, 0.0f, MaxSpeculativeMargin, Continuity);
		const Bepu::BodyActivityDescription Activity(gAllowSleeping ? 0.01f : -1.0f, 32);
		const Bepu::BodyVelocity Velocity(ToBepu(desc.mLinearVelocity), ToBepu(desc.mAngularVelocity));

		const Bepu::BodyDescription BodyDesc = IsKinematic ?
			Bepu::BodyDescription::CreateKinematic(ToBepu(BodyPose), Velocity, Collidable, Activity) :
			Bepu::BodyDescription::CreateDynamic(ToBepu(BodyPose), Velocity, Inertia, Collidable, Activity);

		const Bepu::BodyHandle Handle = Bepu::AddBody(gSimulation, BodyDesc);
		Actor->mHandle = Handle.Value;
		Actor->mLocalInertia = Inertia;
		RegisterCollidableData(false, Handle.Value, Data);
	}

	Actor->mArrayIndex = udword(gActors.size());
	gActors.push_back(Actor);

	return PintActorHandle(Actor);
}

bool BepuPint::ReleaseObject(PintActorHandle handle)
{
	BepuActor* Actor = GetActor(handle);
	if(!Actor)
		return false;

	if(Actor->mIsStatic)
		Bepu::RemoveStatic(gSimulation, Bepu::StaticHandle{ Actor->mHandle });
	else
		Bepu::RemoveBody(gSimulation, Bepu::BodyHandle{ Actor->mHandle });

	if(Actor->mOwnsShape && Actor->mShape.Exists())
		Bepu::RemoveAndDestroyShapeRecursively(gSimulation, gBufferPool, Actor->mShape);
	else if(!Actor->mOwnsShape && Actor->mShape.Exists())
	{
		// Shared shape (mesh object): just remove the body, keep the shape alive.
	}

	// Swap-remove from the render list
	const udword Index = Actor->mArrayIndex;
	const udword LastIndex = udword(gActors.size()) - 1;
	if(Index!=LastIndex)
	{
		gActors[Index] = gActors[LastIndex];
		gActors[Index]->mArrayIndex = Index;
	}
	gActors.pop_back();

	if(Actor->mKinematicTargetPending || Actor->mKinematicMoving)
	{
		const udword NbKinematics = udword(gKinematicTargets.size());
		for(udword i=0;i<NbKinematics;i++)
		{
			if(gKinematicTargets[i]==Actor)
			{
				gKinematicTargets[i] = gKinematicTargets[NbKinematics-1];
				gKinematicTargets.pop_back();
				break;
			}
		}
	}

	if(Actor->mHasDampingOverride)
	{
		const udword NbOverrides = udword(gDampingOverrides.size());
		for(udword i=0;i<NbOverrides;i++)
		{
			if(gDampingOverrides[i]==Actor)
			{
				gDampingOverrides[i] = gDampingOverrides[NbOverrides-1];
				gDampingOverrides.pop_back();
				break;
			}
		}
	}

	DELETESINGLE(Actor);
	return true;
}

PintJointHandle BepuPint::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	if(!gSimulation.RawValue)
		return null;

	BepuActor* Actor0 = GetActor(desc.mObject0);
	BepuActor* Actor1 = GetActor(desc.mObject1);

	// Joints where neither side is a dynamic body are meaningless (both would resolve to the
	// world anchor, i.e. a self-constraint).
	const bool Dynamic0 = Actor0 && !Actor0->mIsStatic;
	const bool Dynamic1 = Actor1 && !Actor1->mIsStatic;
	if(!Dynamic0 && !Dynamic1)
		return null;

	// Joint spring settings. 30Hz/1.0 matches the Bepu demos; like contacts, joint springs are
	// only stable up to about half the integration (substep) rate - raise substeps along with
	// the hertz. Long jointed chains under load need both raised to avoid visible stretching.
	const Bepu::SpringSettings JointSpring = Bepu::SpringSettings(gJointHertz, gJointDampingRatio);

	Bepu::ConstraintHandle Handle;
	Handle.Value = -1;
	Bepu::ConstraintHandle LimitHandle;
	LimitHandle.Value = -1;
	Bepu::ConstraintHandle MotorHandle;
	MotorHandle.Value = -1;
	Bepu::ConstraintHandle ExtraHandle;
	ExtraHandle.Value = -1;
	Bepu::BodyHandle JointBody0;
	JointBody0.Value = -1;
	Bepu::BodyHandle JointBody1;
	JointBody1.Value = -1;
	Bepu::Vector3 MotorAxisA(0.0f, 0.0f, 1.0f);
	float DriveVelocity = 0.0f;
	bool MotorEnabled = false;

	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			const PINT_SPHERICAL_JOINT_CREATE& JC = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);
			// Cone limits are not supported yet.
			PR Pivot0, Pivot1;
			const Bepu::BodyHandle H0 = ResolveJointFrame(Actor0, JC.mLocalPivot0, Pivot0);
			const Bepu::BodyHandle H1 = ResolveJointFrame(Actor1, JC.mLocalPivot1, Pivot1);

			Bepu::BallSocket Desc;
			Desc.LocalOffsetA = ToBepu(Pivot0.mPos);
			Desc.LocalOffsetB = ToBepu(Pivot1.mPos);
			Desc.SpringSettings = JointSpring;
			Handle = Bepu::AddBallSocket(gSimulation, H0, H1, &Desc);
		}
		break;

		case PINT_JOINT_HINGE:
		case PINT_JOINT_HINGE2:
		{
			Point PivotPos0, PivotPos1, Axis0, Axis1;
			bool UseMotor, UseLimits;
			float LimitMin = 0.0f, LimitMax = 0.0f;
			if(desc.mType==PINT_JOINT_HINGE)
			{
				const PINT_HINGE_JOINT_CREATE& JC = static_cast<const PINT_HINGE_JOINT_CREATE&>(desc);
				PivotPos0 = JC.mLocalPivot0;
				PivotPos1 = JC.mLocalPivot1;
				Axis0 = JC.mLocalAxis0;
				Axis1 = JC.mLocalAxis1;

				if(!JC.mGlobalAnchor.IsNotUsed() && !JC.mGlobalAxis.IsNotUsed())
				{
					// World-space anchor/axis variant: convert to actor-local frames.
					const PR ActorPose0 = Actor0 ? GetActorWorldPose(Actor0) : PR(Idt);
					const PR ActorPose1 = Actor1 ? GetActorWorldPose(Actor1) : PR(Idt);
					const PR Inv0 = InversePR(ActorPose0);
					const PR Inv1 = InversePR(ActorPose1);
					PivotPos0 = Inv0.mPos + Inv0.mRot.Rotate(JC.mGlobalAnchor);
					PivotPos1 = Inv1.mPos + Inv1.mRot.Rotate(JC.mGlobalAnchor);
					Axis0 = Inv0.mRot.Rotate(JC.mGlobalAxis);
					Axis1 = Inv1.mRot.Rotate(JC.mGlobalAxis);
				}

				UseMotor = JC.mUseMotor;
				DriveVelocity = JC.mDriveVelocity;
				UseLimits = IsHingeLimitEnabled(JC.mLimits);
				LimitMin = JC.mLimits.mMinValue;
				LimitMax = JC.mLimits.mMaxValue;
			}
			else
			{
				const PINT_HINGE2_JOINT_CREATE& JC = static_cast<const PINT_HINGE2_JOINT_CREATE&>(desc);
				// Hinge2 uses full joint frames; the hinge axis is the frame's X axis.
				PivotPos0 = JC.mLocalPivot0.mPos;
				PivotPos1 = JC.mLocalPivot1.mPos;
				Axis0 = JC.mLocalPivot0.mRot.Rotate(Point(1.0f, 0.0f, 0.0f));
				Axis1 = JC.mLocalPivot1.mRot.Rotate(Point(1.0f, 0.0f, 0.0f));

				UseMotor = JC.mUseMotor;
				DriveVelocity = JC.mDriveVelocity;
				UseLimits = IsHingeLimitEnabled(JC.mLimits);
				LimitMin = JC.mLimits.mMinValue;
				LimitMax = JC.mLimits.mMaxValue;
			}

			PR Pivot0, Pivot1;
			const Bepu::BodyHandle H0 = ResolveJointFrame(Actor0, PR(PivotPos0, Quat(Idt)), Pivot0);
			const Bepu::BodyHandle H1 = ResolveJointFrame(Actor1, PR(PivotPos1, Quat(Idt)), Pivot1);
			Point AxisBody0 = ResolveJointAxis(Actor0, Axis0);
			Point AxisBody1 = ResolveJointAxis(Actor1, Axis1);
			AxisBody0.Normalize();
			AxisBody1.Normalize();

			Bepu::Hinge Desc;
			Desc.LocalOffsetA = ToBepu(Pivot0.mPos);
			Desc.LocalHingeAxisA = ToBepu(AxisBody0);
			Desc.LocalOffsetB = ToBepu(Pivot1.mPos);
			Desc.LocalHingeAxisB = ToBepu(AxisBody1);
			Desc.SpringSettings = JointSpring;
			Handle = Bepu::AddHinge(gSimulation, H0, H1, &Desc);

			if(Handle.Value>=0)
			{
				JointBody0 = H0;
				JointBody1 = H1;
				MotorAxisA = ToBepu(AxisBody0);

				if(UseLimits)
				{
					// Angle limits, relative to the creation pose: both bases share the same
					// world-space zero-angle reference, so the current twist measures zero.
					const PR PoseA = GetJointBodyPose(H0);
					const PR PoseB = GetJointBodyPose(H1);
					const Point WorldAxis = PoseA.mRot.Rotate(AxisBody0);
					const Point E = fabsf(WorldAxis.x)<0.9f ? Point(1.0f, 0.0f, 0.0f) : Point(0.0f, 1.0f, 0.0f);
					Point WorldRef = E ^ WorldAxis;
					WorldRef.Normalize();
					const Quat InvA(PoseA.mRot.w, -PoseA.mRot.p);
					const Quat InvB(PoseB.mRot.w, -PoseB.mRot.p);

					Bepu::TwistLimit LimitDesc;
					LimitDesc.LocalBasisA = ToBepu(CreateTwistBasis(AxisBody0, InvA.Rotate(WorldRef)));
					LimitDesc.LocalBasisB = ToBepu(CreateTwistBasis(AxisBody1, InvB.Rotate(WorldRef)));
					LimitDesc.MinimumAngle = LimitMin;
					LimitDesc.MaximumAngle = LimitMax;
					LimitDesc.SpringSettings = JointSpring;
					LimitHandle = Bepu::AddTwistLimit(gSimulation, H0, H1, &LimitDesc);
				}

				if(UseMotor)
				{
					Bepu::AngularAxisMotor MotorDesc;
					MotorDesc.LocalAxisA = MotorAxisA;
					// Bepu's motor targets the velocity of A relative to B; PEEL/PhysX drive
					// object1 relative to object0. Hence the sign flip.
					MotorDesc.TargetVelocity = -DriveVelocity;
					MotorDesc.Settings = Bepu::MotorSettings(GetMotorMaxForce(), gMotorDamping);
					MotorHandle = Bepu::AddAngularAxisMotor(gSimulation, H0, H1, &MotorDesc);
					MotorEnabled = true;
				}
			}
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			const PINT_PRISMATIC_JOINT_CREATE& JC = static_cast<const PINT_PRISMATIC_JOINT_CREATE&>(desc);

			// Old API provides explicit axes; the newer one encodes the slide axis as the X axis
			// of the pivot frames.
			Point Axis0 = JC.mLocalAxis0;
			Point Axis1 = JC.mLocalAxis1;
			if(Axis0.IsZero())
			{
				Axis0 = JC.mLocalPivot0.mRot.Rotate(Point(1.0f, 0.0f, 0.0f));
				Axis1 = JC.mLocalPivot1.mRot.Rotate(Point(1.0f, 0.0f, 0.0f));
			}

			PR Pivot0, Pivot1;
			const Bepu::BodyHandle H0 = ResolveJointFrame(Actor0, PR(JC.mLocalPivot0.mPos, Quat(Idt)), Pivot0);
			const Bepu::BodyHandle H1 = ResolveJointFrame(Actor1, PR(JC.mLocalPivot1.mPos, Quat(Idt)), Pivot1);
			Point AxisBody0 = ResolveJointAxis(Actor0, Axis0);
			AxisBody0.Normalize();

			// Prismatic = point-on-line (2 linear DOF removed) + angular lock (3 angular DOF removed).
			Bepu::PointOnLineServo LineDesc;
			LineDesc.LocalOffsetA = ToBepu(Pivot0.mPos);
			LineDesc.LocalOffsetB = ToBepu(Pivot1.mPos);
			LineDesc.LocalDirection = ToBepu(AxisBody0);
			LineDesc.ServoSettings = Bepu::ServoSettings();
			LineDesc.SpringSettings = JointSpring;
			Handle = Bepu::AddPointOnLineServo(gSimulation, H0, H1, &LineDesc);

			if(Handle.Value>=0)
			{
				const PR PoseA = GetJointBodyPose(H0);
				const PR PoseB = GetJointBodyPose(H1);
				const Quat InvA(PoseA.mRot.w, -PoseA.mRot.p);

				Bepu::AngularServo AngularDesc;
				AngularDesc.TargetRelativeRotationLocalA = ToBepu(InvA * PoseB.mRot);
				AngularDesc.SpringSettings = JointSpring;
				AngularDesc.ServoSettings = Bepu::ServoSettings();
				ExtraHandle = Bepu::AddAngularServo(gSimulation, H0, H1, &AngularDesc);

				if(IsPrismaticLimitEnabled(JC.mLimits))
				{
					Bepu::LinearAxisLimit LimitDesc;
					LimitDesc.LocalOffsetA = ToBepu(Pivot0.mPos);
					LimitDesc.LocalOffsetB = ToBepu(Pivot1.mPos);
					LimitDesc.LocalAxis = ToBepu(AxisBody0);
					LimitDesc.MinimumOffset = JC.mLimits.mMinValue;
					LimitDesc.MaximumOffset = JC.mLimits.mMaxValue;
					LimitDesc.SpringSettings = JointSpring;
					LimitHandle = Bepu::AddLinearAxisLimit(gSimulation, H0, H1, &LimitDesc);
				}

				if(JC.mSpring.mStiffness>0.0f)
					printf("Bepu: prismatic joint springs are not supported yet.\n");
			}
		}
		break;

		case PINT_JOINT_FIXED:
		{
			// Weld: lock the current relative pose of the two bodies. PEEL fixed joints are
			// created with the bodies already in their intended relative pose, so the explicit
			// pivots are redundant here.
			PR Dummy;
			const Bepu::BodyHandle H0 = ResolveJointFrame(Actor0, PR(Idt), Dummy);
			const Bepu::BodyHandle H1 = ResolveJointFrame(Actor1, PR(Idt), Dummy);

			const PR Pose0 = GetJointBodyPose(H0);
			const PR Pose1 = GetJointBodyPose(H1);
			const Quat Inv0(Pose0.mRot.w, -Pose0.mRot.p);

			Bepu::Weld Desc;
			Desc.LocalOffset = ToBepu(Inv0.Rotate(Pose1.mPos - Pose0.mPos));
			Desc.LocalOrientation = ToBepu(Inv0 * Pose1.mRot);
			Desc.SpringSettings = JointSpring;
			Handle = Bepu::AddWeld(gSimulation, H0, H1, &Desc);
		}
		break;

		case PINT_JOINT_DISTANCE:
		{
			const PINT_DISTANCE_JOINT_CREATE& JC = static_cast<const PINT_DISTANCE_JOINT_CREATE&>(desc);
			PR Pivot0, Pivot1;
			const Bepu::BodyHandle H0 = ResolveJointFrame(Actor0, PR(JC.mLocalPivot0, Quat(Idt)), Pivot0);
			const Bepu::BodyHandle H1 = ResolveJointFrame(Actor1, PR(JC.mLocalPivot1, Quat(Idt)), Pivot1);

			const bool MinEnabled = IsMinDistanceLimitEnabled(JC.mLimits);
			const bool MaxEnabled = IsMaxDistanceLimitEnabled(JC.mLimits);
			if(MinEnabled || MaxEnabled)
			{
				Bepu::DistanceLimit Desc;
				Desc.LocalOffsetA = ToBepu(Pivot0.mPos);
				Desc.LocalOffsetB = ToBepu(Pivot1.mPos);
				Desc.MinimumDistance = MinEnabled ? JC.mLimits.mMinValue : 0.0f;
				Desc.MaximumDistance = MaxEnabled ? JC.mLimits.mMaxValue : 1e10f;
				Desc.SpringSettings = JointSpring;
				Handle = Bepu::AddDistanceLimit(gSimulation, H0, H1, &Desc);
			}
			else
			{
				// No limits: rigid rod at the current distance between the attachment points.
				const PR Pose0 = GetJointBodyPose(H0);
				const PR Pose1 = GetJointBodyPose(H1);
				const Point World0 = Pose0.mPos + Pose0.mRot.Rotate(Pivot0.mPos);
				const Point World1 = Pose1.mPos + Pose1.mRot.Rotate(Pivot1.mPos);

				Bepu::DistanceServo Desc;
				Desc.LocalOffsetA = ToBepu(Pivot0.mPos);
				Desc.LocalOffsetB = ToBepu(Pivot1.mPos);
				Desc.TargetDistance = World0.Distance(World1);
				Desc.ServoSettings = Bepu::ServoSettings();
				Desc.SpringSettings = JointSpring;
				Handle = Bepu::AddDistanceServo(gSimulation, H0, H1, &Desc);
			}
		}
		break;

		case PINT_JOINT_GEAR:
		{
			const PINT_GEAR_JOINT_CREATE& JC = static_cast<const PINT_GEAR_JOINT_CREATE&>(desc);
			// Both gear wheels must be bodies (dynamic or kinematic).
			if(!Dynamic0 || !Dynamic1)
				break;
			const Bepu::BodyHandle H0 = Bepu::BodyHandle{ Actor0->mHandle };
			const Bepu::BodyHandle H1 = Bepu::BodyHandle{ Actor1->mHandle };

			// Gear axis: recover it in world space from the first referenced hinge joint, then
			// express it in gear body 0's local frame. NOTE: Bepu's gear constraint measures BOTH
			// bodies around this single axis, so only parallel gear configurations work; bevel
			// setups (perpendicular axes, e.g. ThreeLegoGears) are an engine limitation.
			Point WorldAxis0(0.0f, 0.0f, 1.0f);
			const BepuJoint* Hinge0 = reinterpret_cast<const BepuJoint*>(JC.mHinge0);
			if(Hinge0 && Hinge0->mBody0.Value>=0)
			{
				const PR HingeBodyPose = GetJointBodyPose(Hinge0->mBody0);
				WorldAxis0 = HingeBodyPose.mRot.Rotate(ToPoint(Hinge0->mMotorAxisA));
			}
			Point WorldAxis1 = WorldAxis0;
			const BepuJoint* Hinge1 = reinterpret_cast<const BepuJoint*>(JC.mHinge1);
			if(Hinge1 && Hinge1->mBody0.Value>=0)
			{
				const PR HingeBodyPose = GetJointBodyPose(Hinge1->mBody0);
				WorldAxis1 = HingeBodyPose.mRot.Rotate(ToPoint(Hinge1->mMotorAxisA));
			}
			if(fabsf(WorldAxis0 | WorldAxis1) < 0.99f)
				printf("Bepu: gear joint with non-parallel axes (bevel) is not supported by the engine's gear constraint.\n");

			const PR Pose0 = GetJointBodyPose(H0);
			const Quat Inv0(Pose0.mRot.w, -Pose0.mRot.p);
			Point AxisLocal0 = Inv0.Rotate(WorldAxis0);
			AxisLocal0.Normalize();

			Bepu::AngularAxisGearMotor GearDesc;
			GearDesc.LocalAxisA = ToBepu(AxisLocal0);
			// Meshing external gears counter-rotate: positive PEEL/PhysX ratios map to a negative
			// velocity scale (ratio = r0/r1, so w1 = -ratio * w0).
			GearDesc.VelocityScale = -JC.mGearRatio;
			GearDesc.Settings = Bepu::MotorSettings(GetMotorMaxForce(), gMotorDamping);
			Handle = Bepu::AddAngularAxisGearMotor(gSimulation, H0, H1, &GearDesc);
		}
		break;

		default:
			printf("Bepu: joint type %d is not supported yet.\n", desc.mType);
		break;
	}

	if(Handle.Value<0)
		return null;

	BepuJoint* Joint = ICE_NEW(BepuJoint);
	Joint->mHandle = Handle;
	Joint->mLimitHandle = LimitHandle;
	Joint->mMotorHandle = MotorHandle;
	Joint->mExtraHandle = ExtraHandle;
	Joint->mBody0 = JointBody0;
	Joint->mBody1 = JointBody1;
	Joint->mMotorAxisA = MotorAxisA;
	Joint->mDriveVelocity = DriveVelocity;
	Joint->mMotorEnabled = MotorEnabled;
	Joint->mType = desc.mType;
	Joint->mArrayIndex = udword(gJoints.size());
	Joint->mPairKey = MakeJointPairKey(Actor0, Actor1);
	if(Joint->mPairKey)
		gJointedPairs[Joint->mPairKey]++;
	gJoints.push_back(Joint);
	return PintJointHandle(Joint);
}

bool BepuPint::ReleaseJoint(PintJointHandle handle)
{
	BepuJoint* Joint = reinterpret_cast<BepuJoint*>(handle);
	if(!Joint)
		return false;

	Bepu::RemoveConstraint(gSimulation, Joint->mHandle);
	if(Joint->mLimitHandle.Value>=0)
		Bepu::RemoveConstraint(gSimulation, Joint->mLimitHandle);
	if(Joint->mMotorHandle.Value>=0)
		Bepu::RemoveConstraint(gSimulation, Joint->mMotorHandle);
	if(Joint->mExtraHandle.Value>=0)
		Bepu::RemoveConstraint(gSimulation, Joint->mExtraHandle);

	if(Joint->mPairKey)
	{
		std::unordered_map<uint64_t, udword>::iterator It = gJointedPairs.find(Joint->mPairKey);
		if(It!=gJointedPairs.end() && --It->second==0)
			gJointedPairs.erase(It);
	}

	const udword Index = Joint->mArrayIndex;
	const udword LastIndex = udword(gJoints.size()) - 1;
	if(Index!=LastIndex)
	{
		gJoints[Index] = gJoints[LastIndex];
		gJoints[Index]->mArrayIndex = Index;
	}
	gJoints.pop_back();

	DELETESINGLE(Joint);
	return true;
}

void BepuPint::SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)
{
	for(udword i=0;i<nb_groups;i++)
	{
		const PintCollisionGroup Group0 = groups[i].mGroup0;
		const PintCollisionGroup Group1 = groups[i].mGroup1;
		ASSERT(Group0<32 && Group1<32);
		gDisabledGroupMask[Group0] |= 1u<<Group1;
		gDisabledGroupMask[Group1] |= 1u<<Group0;
	}
}

PintConvexHandle BepuPint::CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index)
{
	if(!gSimulation.RawValue || !desc.mNbVerts)
		return null;

	std::vector<Bepu::Vector3> Pts(desc.mNbVerts);
	for(udword i=0;i<desc.mNbVerts;i++)
		Pts[i] = ToBepu(desc.mVerts[i]);

	Bepu::Vector3 CenterOfMass;
	BepuConvexObject* Convex = ICE_NEW(BepuConvexObject);
	Convex->mHull = Bepu::CreateConvexHull(gBufferPool, Bepu::Buffer<Bepu::Vector3>(Pts.data(), int32_t(desc.mNbVerts)), &CenterOfMass);
	Convex->mCenterOffset = ToPoint(CenterOfMass);

	if(index)
		*index = udword(gConvexObjects.size());
	gConvexObjects.push_back(Convex);
	return PintConvexHandle(Convex);
}

bool BepuPint::DeleteConvexObject(PintConvexHandle handle, const PintConvexIndex* index)
{
	BepuConvexObject* Convex = reinterpret_cast<BepuConvexObject*>(handle);
	if(!Convex)
		return false;

	// Keep index stability: null the slot instead of compacting.
	const udword NbConvexes = udword(gConvexObjects.size());
	for(udword i=0;i<NbConvexes;i++)
	{
		if(gConvexObjects[i]==Convex)
		{
			gConvexObjects[i] = null;
			break;
		}
	}
	Bepu::DestroyConvexHull(gBufferPool, &Convex->mHull);
	DELETESINGLE(Convex);
	return true;
}

PintMeshHandle BepuPint::CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)
{
	if(!gSimulation.RawValue)
		return null;

	BepuMeshObject* MeshObject = ICE_NEW(BepuMeshObject);
	MeshObject->mMesh = CreateBepuMesh(desc.GetSurface());
	MeshObject->mShapeIndex = Bepu::AddMesh(gSimulation, MeshObject->mMesh);
	if(index)
		*index = 0;
	return PintMeshHandle(MeshObject);
}

bool BepuPint::DeleteMeshObject(PintMeshHandle handle, const PintMeshIndex* index)
{
	BepuMeshObject* MeshObject = reinterpret_cast<BepuMeshObject*>(handle);
	if(!MeshObject)
		return false;
	Bepu::RemoveAndDestroyShapeRecursively(gSimulation, gBufferPool, MeshObject->mShapeIndex);
	DELETESINGLE(MeshObject);
	return true;
}

PintHeightfieldHandle BepuPint::CreateHeightfieldObject(const PINT_HEIGHTFIELD_DATA_CREATE& desc, PintHeightfieldData& data, PintHeightfieldIndex* index)
{
	return null;
}

bool BepuPint::DeleteHeightfieldObject(PintHeightfieldHandle handle, const PintHeightfieldIndex* index)
{
	return false;
}

// Rays are handed to the interop layer in fixed-size chunks: keeps everything on the stack and
// amortizes the native<->managed transition without unbounded temporary storage.
static const udword gRaycastChunkSize = 256;

udword BepuPint::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	if(!gSimulation.RawValue)
		return 0;

	Bepu::RayQueryData Queries[gRaycastChunkSize];
	Bepu::RayHitData Hits[gRaycastChunkSize];

	udword NbHits = 0;
	while(nb)
	{
		const udword Count = TMin(nb, gRaycastChunkSize);

		for(udword i=0;i<Count;i++)
		{
			const PintRaycastData& Ray = raycasts[i];
			Queries[i].Origin = ToBepu(Ray.mOrigin);
			Queries[i].MaximumT = Ray.mMaxDist;
			Queries[i].Direction = ToBepu(Ray.mDir);
		}

		Bepu::RayCastClosestBatch(gSimulation, Queries, Hits, int32_t(Count));

		for(udword i=0;i<Count;i++)
		{
			const Bepu::RayHitData& Hit = Hits[i];
			if(Hit.Hit)
			{
				NbHits++;
				dest[i].mTouchedActor	= PintActorHandle(GetActorFromCollidable(Hit.Collidable));
				dest[i].mTouchedShape	= null;	// Per-shape handles aren't tracked by this plugin yet
				dest[i].mImpact			= raycasts[i].mOrigin + raycasts[i].mDir * Hit.T;
				dest[i].mNormal			= ToPoint(Hit.Normal);
				dest[i].mDistance		= Hit.T;
				dest[i].mTriangleIndex	= udword(Hit.ChildIndex);
			}
			else
			{
				dest[i].SetNoHit();
			}
		}

		raycasts += Count;
		dest += Count;
		nb -= Count;
	}
	return NbHits;
}

udword BepuPint::BatchRaycastAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)
{
	if(!gSimulation.RawValue)
		return 0;

	Bepu::RayQueryData Queries[gRaycastChunkSize];
	int32_t Results[gRaycastChunkSize];

	udword NbHits = 0;
	while(nb)
	{
		const udword Count = TMin(nb, gRaycastChunkSize);

		for(udword i=0;i<Count;i++)
		{
			const PintRaycastData& Ray = raycasts[i];
			Queries[i].Origin = ToBepu(Ray.mOrigin);
			Queries[i].MaximumT = Ray.mMaxDist;
			Queries[i].Direction = ToBepu(Ray.mDir);
		}

		NbHits += udword(Bepu::RayCastAnyBatch(gSimulation, Queries, Results, int32_t(Count)));

		for(udword i=0;i<Count;i++)
			dest[i].mHit = Results[i]!=0;

		raycasts += Count;
		dest += Count;
		nb -= Count;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

// Rotation taking the local Y axis (Bepu's capsule axis) onto the given unit direction.
static Quat ShortestArcFromY(const Point& dir)
{
	const float D = dir.y;	// dot(Y, dir)
	Quat Q;
	if(D > 0.9999f)
		return Quat(Idt);
	if(D < -0.9999f)
	{
		// 180 degrees about any perpendicular axis
		Q.w = 0.0f;
		Q.p = Point(0.0f, 0.0f, 1.0f);
		return Q;
	}
	Q.p = Point(0.0f, 1.0f, 0.0f) ^ dir;
	Q.w = 1.0f + D;
	const float Mag = sqrtf(Q.w*Q.w + (Q.p|Q.p));
	Q.w /= Mag;
	Q.p *= 1.0f/Mag;
	return Q;
}

// Converts one sweep result to PEEL's format. Returns 1 if the sweep hit anything.
static udword ConvertSweepHit(const Bepu::SweepHitData& hit, const Point& start_pos, const Point& dir, PintRaycastHit& dest)
{
	if(hit.Hit==1)
	{
		dest.mTouchedActor	= PintActorHandle(GetActorFromCollidable(hit.Collidable));
		dest.mTouchedShape	= null;
		dest.mImpact		= ToPoint(hit.Location);
		dest.mNormal		= ToPoint(hit.Normal);
		dest.mDistance		= hit.T;
		dest.mTriangleIndex	= INVALID_ID;
		return 1;
	}
	if(hit.Hit==2)
	{
		// Swept shape started in overlap: PhysX-style initial-overlap report.
		dest.mTouchedActor	= PintActorHandle(GetActorFromCollidable(hit.Collidable));
		dest.mTouchedShape	= null;
		dest.mImpact		= start_pos;
		dest.mNormal		= -dir;
		dest.mDistance		= 0.0f;
		dest.mTriangleIndex	= INVALID_ID;
		return 1;
	}
	dest.SetNoHit();
	return 0;
}

static const udword gSweepChunkSize = 256;

udword BepuPint::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	if(!gSimulation.RawValue)
		return 0;

	Bepu::BoxSweepQuery Queries[gSweepChunkSize];
	Bepu::SweepHitData Hits[gSweepChunkSize];

	udword NbHits = 0;
	while(nb)
	{
		const udword Count = TMin(nb, gSweepChunkSize);
		for(udword i=0;i<Count;i++)
		{
			const PintBoxSweepData& Sweep = sweeps[i];
			const Quat BoxRot = Sweep.mBox.mRot;	// Ice matrix-to-quat conversion
			Queries[i].Orientation = ToBepu(BoxRot);
			Queries[i].Position = ToBepu(Sweep.mBox.mCenter);
			Queries[i].HalfExtents = ToBepu(Sweep.mBox.mExtents);
			Queries[i].Direction = ToBepu(Sweep.mDir);
			Queries[i].MaximumT = Sweep.mMaxDist;
		}

		Bepu::SweepBoxClosestBatch(gSimulation, gBufferPool, Queries, Hits, int32_t(Count), gSweepMinProgression, gSweepConvergence, gSweepMaxIterations);

		for(udword i=0;i<Count;i++)
			NbHits += ConvertSweepHit(Hits[i], sweeps[i].mBox.mCenter, sweeps[i].mDir, dest[i]);

		sweeps += Count;
		dest += Count;
		nb -= Count;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword BepuPint::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	if(!gSimulation.RawValue)
		return 0;

	Bepu::SphereSweepQuery Queries[gSweepChunkSize];
	Bepu::SweepHitData Hits[gSweepChunkSize];

	udword NbHits = 0;
	while(nb)
	{
		const udword Count = TMin(nb, gSweepChunkSize);
		for(udword i=0;i<Count;i++)
		{
			const PintSphereSweepData& Sweep = sweeps[i];
			Queries[i].Position = ToBepu(Sweep.mSphere.mCenter);
			Queries[i].Radius = Sweep.mSphere.mRadius;
			Queries[i].Direction = ToBepu(Sweep.mDir);
			Queries[i].MaximumT = Sweep.mMaxDist;
		}

		Bepu::SweepSphereClosestBatch(gSimulation, gBufferPool, Queries, Hits, int32_t(Count), gSweepMinProgression, gSweepConvergence, gSweepMaxIterations);

		for(udword i=0;i<Count;i++)
			NbHits += ConvertSweepHit(Hits[i], sweeps[i].mSphere.mCenter, sweeps[i].mDir, dest[i]);

		sweeps += Count;
		dest += Count;
		nb -= Count;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword BepuPint::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	if(!gSimulation.RawValue)
		return 0;

	Bepu::CapsuleSweepQuery Queries[gSweepChunkSize];
	Bepu::SweepHitData Hits[gSweepChunkSize];

	udword NbHits = 0;
	while(nb)
	{
		const udword Count = TMin(nb, gSweepChunkSize);
		for(udword i=0;i<Count;i++)
		{
			const PintCapsuleSweepData& Sweep = sweeps[i];
			// PEEL capsules are segments + radius; Bepu capsules are Y-aligned in local space.
			const Point Center = (Sweep.mCapsule.mP0 + Sweep.mCapsule.mP1) * 0.5f;
			Point Delta = Sweep.mCapsule.mP1 - Sweep.mCapsule.mP0;
			const float Length = Delta.Magnitude();
			Queries[i].Orientation = ToBepu(Length>1e-6f ? ShortestArcFromY(Delta / Length) : Quat(Idt));
			Queries[i].Position = ToBepu(Center);
			Queries[i].Radius = Sweep.mCapsule.mRadius;
			Queries[i].HalfLength = Length * 0.5f;
			Queries[i].Direction = ToBepu(Sweep.mDir);
			Queries[i].MaximumT = Sweep.mMaxDist;
		}

		Bepu::SweepCapsuleClosestBatch(gSimulation, gBufferPool, Queries, Hits, int32_t(Count), gSweepMinProgression, gSweepConvergence, gSweepMaxIterations);

		for(udword i=0;i<Count;i++)
		{
			const Point Center = (sweeps[i].mCapsule.mP0 + sweeps[i].mCapsule.mP1) * 0.5f;
			NbHits += ConvertSweepHit(Hits[i], Center, sweeps[i].mDir, dest[i]);
		}

		sweeps += Count;
		dest += Count;
		nb -= Count;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

static const udword gOverlapCapacity = 512;

// Shared tail for the overlap-objects queries: writes one PintMultipleHits entry and appends the
// touched actors to the stream, Jolt-plugin style.
static udword ReportOverlaps(const Bepu::CollidableReference* buffer, int32_t nb_raw, PintMultipleHits* dest, Container& stream, udword& offset)
{
	const udword Nb = TMin(udword(nb_raw), gOverlapCapacity);
	dest->mNbHits = Nb;
	dest->mOffset = offset;
	offset += Nb;

	if(Nb)
	{
		PintOverlapHit* Hits = reinterpret_cast<PintOverlapHit*>(stream.Reserve(Nb*(sizeof(PintOverlapHit)/sizeof(udword))));
		for(udword i=0;i<Nb;i++)
		{
			Hits[i].mTouchedActor = PintActorHandle(GetActorFromCollidable(buffer[i]));
			Hits[i].mTouchedShape = null;
		}
	}
	return Nb;
}

udword BepuPint::BatchSphereOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)
{
	if(!gSimulation.RawValue)
		return 0;

	udword NbHits = 0;
	Bepu::CollidableReference Buffer[1];
	for(udword i=0;i<nb;i++)
	{
		Bepu::SphereSweepQuery Query;
		Query.Position = ToBepu(overlaps[i].mSphere.mCenter);
		Query.Radius = overlaps[i].mSphere.mRadius;
		Query.Direction = Bepu::Vector3(0.0f, 0.0f, 0.0f);
		Query.MaximumT = 0.0f;
		const bool Hit = Bepu::OverlapSphere(gSimulation, gBufferPool, &Query, Buffer, 1)>0;
		dest[i].mHit = Hit;
		NbHits += Hit;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword BepuPint::BatchSphereOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)
{
	if(!gSimulation.RawValue)
		return 0;

	udword NbHits = 0;
	udword Offset = 0;
	Bepu::CollidableReference Buffer[gOverlapCapacity];
	for(udword i=0;i<nb;i++)
	{
		Bepu::SphereSweepQuery Query;
		Query.Position = ToBepu(overlaps[i].mSphere.mCenter);
		Query.Radius = overlaps[i].mSphere.mRadius;
		Query.Direction = Bepu::Vector3(0.0f, 0.0f, 0.0f);
		Query.MaximumT = 0.0f;
		const int32_t Nb = Bepu::OverlapSphere(gSimulation, gBufferPool, &Query, Buffer, int32_t(gOverlapCapacity));
		NbHits += ReportOverlaps(Buffer, Nb, dest + i, stream, Offset);
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword BepuPint::BatchBoxOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)
{
	if(!gSimulation.RawValue)
		return 0;

	udword NbHits = 0;
	Bepu::CollidableReference Buffer[1];
	for(udword i=0;i<nb;i++)
	{
		const Quat BoxRot = overlaps[i].mBox.mRot;
		Bepu::BoxSweepQuery Query;
		Query.Orientation = ToBepu(BoxRot);
		Query.Position = ToBepu(overlaps[i].mBox.mCenter);
		Query.HalfExtents = ToBepu(overlaps[i].mBox.mExtents);
		Query.Direction = Bepu::Vector3(0.0f, 0.0f, 0.0f);
		Query.MaximumT = 0.0f;
		const bool Hit = Bepu::OverlapBox(gSimulation, gBufferPool, &Query, Buffer, 1)>0;
		dest[i].mHit = Hit;
		NbHits += Hit;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword BepuPint::BatchBoxOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)
{
	if(!gSimulation.RawValue)
		return 0;

	udword NbHits = 0;
	udword Offset = 0;
	Bepu::CollidableReference Buffer[gOverlapCapacity];
	for(udword i=0;i<nb;i++)
	{
		const Quat BoxRot = overlaps[i].mBox.mRot;
		Bepu::BoxSweepQuery Query;
		Query.Orientation = ToBepu(BoxRot);
		Query.Position = ToBepu(overlaps[i].mBox.mCenter);
		Query.HalfExtents = ToBepu(overlaps[i].mBox.mExtents);
		Query.Direction = Bepu::Vector3(0.0f, 0.0f, 0.0f);
		Query.MaximumT = 0.0f;
		const int32_t Nb = Bepu::OverlapBox(gSimulation, gBufferPool, &Query, Buffer, int32_t(gOverlapCapacity));
		NbHits += ReportOverlaps(Buffer, Nb, dest + i, stream, Offset);
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

static void SetupCapsuleOverlapQuery(Bepu::CapsuleSweepQuery& query, const PintCapsuleOverlapData& overlap)
{
	const Point Center = (overlap.mCapsule.mP0 + overlap.mCapsule.mP1) * 0.5f;
	Point Delta = overlap.mCapsule.mP1 - overlap.mCapsule.mP0;
	const float Length = Delta.Magnitude();
	query.Orientation = ToBepu(Length>1e-6f ? ShortestArcFromY(Delta / Length) : Quat(Idt));
	query.Position = ToBepu(Center);
	query.Radius = overlap.mCapsule.mRadius;
	query.HalfLength = Length * 0.5f;
	query.Direction = Bepu::Vector3(0.0f, 0.0f, 0.0f);
	query.MaximumT = 0.0f;
}

udword BepuPint::BatchCapsuleOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)
{
	if(!gSimulation.RawValue)
		return 0;

	udword NbHits = 0;
	Bepu::CollidableReference Buffer[1];
	for(udword i=0;i<nb;i++)
	{
		Bepu::CapsuleSweepQuery Query;
		SetupCapsuleOverlapQuery(Query, overlaps[i]);
		const bool Hit = Bepu::OverlapCapsule(gSimulation, gBufferPool, &Query, Buffer, 1)>0;
		dest[i].mHit = Hit;
		NbHits += Hit;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

// Builds a convex query from PEEL data, compensating for Bepu's hull recentering. Returns null
// if the referenced convex object doesn't exist.
static const BepuConvexObject* SetupConvexQuery(Bepu::ConvexSweepQuery& query, const PintConvexOverlapData& overlap)
{
	const BepuConvexObject* Convex = overlap.mConvexObjectIndex < gConvexObjects.size() ? gConvexObjects[overlap.mConvexObjectIndex] : null;
	if(!Convex)
		return null;
	query.Hull = const_cast<Bepu::ConvexHull*>(&Convex->mHull);
	query.Orientation = ToBepu(overlap.mTransform.mRot);
	query.Position = ToBepu(overlap.mTransform.mPos + overlap.mTransform.mRot.Rotate(Convex->mCenterOffset));
	query.Direction = Bepu::Vector3(0.0f, 0.0f, 0.0f);
	query.MaximumT = 0.0f;
	return Convex;
}

udword BepuPint::BatchConvexSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)
{
	if(!gSimulation.RawValue)
		return 0;

	Bepu::ConvexSweepQuery Queries[gSweepChunkSize];
	Bepu::SweepHitData Hits[gSweepChunkSize];

	udword NbHits = 0;
	while(nb)
	{
		const udword Count = TMin(nb, gSweepChunkSize);
		for(udword i=0;i<Count;i++)
		{
			if(SetupConvexQuery(Queries[i], sweeps[i]))
			{
				Queries[i].Direction = ToBepu(sweeps[i].mDir);
				Queries[i].MaximumT = sweeps[i].mMaxDist;
			}
			else
			{
				// Unknown convex: neutralize the query (zero-length sweep of nothing).
				static Bepu::ConvexHull DummyHull;
				Queries[i].Hull = &DummyHull;
				Queries[i].MaximumT = 0.0f;
			}
		}

		Bepu::SweepConvexHullClosestBatch(gSimulation, gBufferPool, Queries, Hits, int32_t(Count), gSweepMinProgression, gSweepConvergence, gSweepMaxIterations);

		for(udword i=0;i<Count;i++)
			NbHits += ConvertSweepHit(Hits[i], sweeps[i].mTransform.mPos, sweeps[i].mDir, dest[i]);

		sweeps += Count;
		dest += Count;
		nb -= Count;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword BepuPint::BatchConvexOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintConvexOverlapData* overlaps)
{
	if(!gSimulation.RawValue)
		return 0;

	udword NbHits = 0;
	Bepu::CollidableReference Buffer[1];
	for(udword i=0;i<nb;i++)
	{
		Bepu::ConvexSweepQuery Query;
		bool Hit = false;
		if(SetupConvexQuery(Query, overlaps[i]))
			Hit = Bepu::OverlapConvexHull(gSimulation, gBufferPool, &Query, Buffer, 1)>0;
		dest[i].mHit = Hit;
		NbHits += Hit;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword BepuPint::BatchConvexOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintConvexOverlapData* overlaps)
{
	if(!gSimulation.RawValue)
		return 0;

	udword NbHits = 0;
	udword Offset = 0;
	Bepu::CollidableReference Buffer[gOverlapCapacity];
	for(udword i=0;i<nb;i++)
	{
		Bepu::ConvexSweepQuery Query;
		int32_t Nb = 0;
		if(SetupConvexQuery(Query, overlaps[i]))
			Nb = Bepu::OverlapConvexHull(gSimulation, gBufferPool, &Query, Buffer, int32_t(gOverlapCapacity));
		NbHits += ReportOverlaps(Buffer, Nb, dest + i, stream, Offset);
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword BepuPint::BatchCapsuleOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)
{
	if(!gSimulation.RawValue)
		return 0;

	udword NbHits = 0;
	udword Offset = 0;
	Bepu::CollidableReference Buffer[gOverlapCapacity];
	for(udword i=0;i<nb;i++)
	{
		Bepu::CapsuleSweepQuery Query;
		SetupCapsuleOverlapQuery(Query, overlaps[i]);
		const int32_t Nb = Bepu::OverlapCapsule(gSimulation, gBufferPool, &Query, Buffer, int32_t(gOverlapCapacity));
		NbHits += ReportOverlaps(Buffer, Nb, dest + i, stream, Offset);
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

static inline_ PR GetBodyPR(const BepuActor* actor)
{
	if(actor->mIsStatic)
		return ToPR(Bepu::GetStatic(gSimulation, Bepu::StaticHandle{ actor->mHandle })->Pose);
	return ToPR(Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ actor->mHandle })->Motion.Pose);
}

PR BepuPint::GetWorldTransform(PintActorHandle handle)
{
	const BepuActor* Actor = GetActor(handle);
	const PR BodyPose = GetBodyPR(Actor);
	return Actor->mIdentityBodyToActor ? BodyPose : Combine(BodyPose, Actor->mBodyToActor);
}

void BepuPint::SetWorldTransform(PintActorHandle handle, const PR& pose)
{
	BepuActor* Actor = GetActor(handle);
	const PR BodyPose = Actor->mIdentityBodyToActor ? pose : Combine(pose, InversePR(Actor->mBodyToActor));

	if(Actor->mIsStatic)
	{
		// Goes through the description so that the broadphase bounds get updated.
		Bepu::StaticDescription Desc = Bepu::GetStaticDescription(gSimulation, Bepu::StaticHandle{ Actor->mHandle });
		Desc.Pose = ToBepu(BodyPose);
		Bepu::ApplyStaticDescription(gSimulation, Bepu::StaticHandle{ Actor->mHandle }, Desc);
	}
	else
	{
		Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
		Dynamics->Motion.Pose = ToBepu(BodyPose);
	}
}

void BepuPint::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
	const BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic || Actor->mIsKinematic)
		return;

	Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });

	const Bepu::BodyInertia& LocalInertia = Dynamics->Inertia.Local;

	// Linear response
	Dynamics->Motion.Velocity.Linear.X += world_impulse.x * LocalInertia.InverseMass;
	Dynamics->Motion.Velocity.Linear.Y += world_impulse.y * LocalInertia.InverseMass;
	Dynamics->Motion.Velocity.Linear.Z += world_impulse.z * LocalInertia.InverseMass;

	// Angular response: w += (R * I^-1_local * R^T) * ((p - com) x J)
	const Point COM = ToPoint(Dynamics->Motion.Pose.Position);
	const Point R = world_pos - COM;
	const Point AngularImpulse = R ^ world_impulse;

	float Rot[3][3];
	QuatToMatrix(ToQuat(Dynamics->Motion.Pose.Orientation), Rot);
	Bepu::Symmetric3x3 InvIWorld;
	RotateSymmetric(LocalInertia.InverseInertiaTensor, Rot, InvIWorld);

	Dynamics->Motion.Velocity.Angular.X += InvIWorld.XX*AngularImpulse.x + InvIWorld.YX*AngularImpulse.y + InvIWorld.ZX*AngularImpulse.z;
	Dynamics->Motion.Velocity.Angular.Y += InvIWorld.YX*AngularImpulse.x + InvIWorld.YY*AngularImpulse.y + InvIWorld.ZY*AngularImpulse.z;
	Dynamics->Motion.Velocity.Angular.Z += InvIWorld.ZX*AngularImpulse.x + InvIWorld.ZY*AngularImpulse.y + InvIWorld.ZZ*AngularImpulse.z;
}

Point BepuPint::GetLinearVelocity(PintActorHandle handle)
{
	const BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return Point(0.0f, 0.0f, 0.0f);
	return ToPoint(Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle })->Motion.Velocity.Linear);
}

void BepuPint::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)
{
	const BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return;
	Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle })->Motion.Velocity.Linear = ToBepu(linear_velocity);
}

Point BepuPint::GetAngularVelocity(PintActorHandle handle)
{
	const BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return Point(0.0f, 0.0f, 0.0f);
	return ToPoint(Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle })->Motion.Velocity.Angular);
}

void BepuPint::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
	const BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return;
	Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle })->Motion.Velocity.Angular = ToBepu(angular_velocity);
}

bool BepuPint::SetKinematicPose(PintActorHandle handle, const Point& pos)
{
	BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return false;
	Bepu::BodyDynamics* Dynamics = Bepu::GetBodyDynamics(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
	// The body frame can be offset from the actor frame; only the translation part changes here.
	const PR ActorPose(pos, ToQuat(Dynamics->Motion.Pose.Orientation) * Actor->mBodyToActor.mRot);
	const PR BodyPose = Actor->mIdentityBodyToActor ? ActorPose : Combine(ActorPose, InversePR(Actor->mBodyToActor));
	QueueKinematicTarget(Actor, BodyPose);
	return true;
}

bool BepuPint::SetKinematicPose(PintActorHandle handle, const PR& pr)
{
	BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic)
		return false;
	const PR BodyPose = Actor->mIdentityBodyToActor ? pr : Combine(pr, InversePR(Actor->mBodyToActor));
	QueueKinematicTarget(Actor, BodyPose);
	return true;
}

bool BepuPint::IsKinematic(PintActorHandle handle)
{
	return GetActor(handle)->mIsKinematic;
}

bool BepuPint::EnableKinematic(PintActorHandle handle, bool flag)
{
	BepuActor* Actor = GetActor(handle);
	if(Actor->mIsStatic || Actor->mIsKinematic==flag)
		return false;

	// Kinematic bodies in Bepu are just bodies with zero inverse mass & inertia.
	Bepu::BodyDescription Desc = Bepu::GetBodyDescription(gSimulation, Bepu::BodyHandle{ Actor->mHandle });
	if(flag)
	{
		Bepu::BodyInertia Zero = {};
		Desc.LocalInertia = Zero;
	}
	else
	{
		Desc.LocalInertia = Actor->mLocalInertia;
	}
	Bepu::ApplyBodyDescription(gSimulation, Bepu::BodyHandle{ Actor->mHandle }, Desc);
	Actor->mIsKinematic = flag;
	return true;
}

static void PushMotorState(BepuJoint* joint)
{
	if(joint->mMotorHandle.Value<0)
		return;
	Bepu::AngularAxisMotor Desc;
	Desc.LocalAxisA = joint->mMotorAxisA;
	// Bepu's motor targets the velocity of A relative to B; PEEL/PhysX drive object1 relative
	// to object0. Hence the sign flip.
	Desc.TargetVelocity = -joint->mDriveVelocity;
	Desc.Settings = Bepu::MotorSettings(joint->mMotorEnabled ? GetMotorMaxForce() : 0.0f, gMotorDamping);
	Bepu::UpdateAngularAxisMotor(gSimulation, joint->mMotorHandle, &Desc);
}

static bool EnsureHingeMotor(BepuJoint* joint)
{
	if(joint->mMotorHandle.Value>=0)
		return true;
	if(joint->mBody0.Value<0 || joint->mBody1.Value<0)
		return false;
	Bepu::AngularAxisMotor Desc;
	Desc.LocalAxisA = joint->mMotorAxisA;
	// Sign flip: see PushMotorState.
	Desc.TargetVelocity = -joint->mDriveVelocity;
	Desc.Settings = Bepu::MotorSettings(GetMotorMaxForce(), gMotorDamping);
	joint->mMotorHandle = Bepu::AddAngularAxisMotor(gSimulation, joint->mBody0, joint->mBody1, &Desc);
	return joint->mMotorHandle.Value>=0;
}

bool BepuPint::SetDriveEnabled(PintJointHandle handle, bool flag)
{
	BepuJoint* Joint = reinterpret_cast<BepuJoint*>(handle);
	if(!Joint || (Joint->mType!=PINT_JOINT_HINGE && Joint->mType!=PINT_JOINT_HINGE2))
		return false;
	if(flag && !EnsureHingeMotor(Joint))
		return false;
	Joint->mMotorEnabled = flag;
	PushMotorState(Joint);
	return true;
}

bool BepuPint::SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)
{
	BepuJoint* Joint = reinterpret_cast<BepuJoint*>(handle);
	if(!Joint || (Joint->mType!=PINT_JOINT_HINGE && Joint->mType!=PINT_JOINT_HINGE2))
		return false;
	// PEEL convention for hinges: the scalar target velocity around the hinge axis is in angular.x.
	Joint->mDriveVelocity = angular.x;
	if(!EnsureHingeMotor(Joint))
		return false;
	Joint->mMotorEnabled = true;
	PushMotorState(Joint);
	return true;
}

bool BepuPint::SetDrivePosition(PintJointHandle handle, const PR& pose)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////

static BepuPint* gBepu = null;
static void gBepu_GetOptionsFromGUI(const char*);

void Bepu_Init(const PINT_WORLD_CREATE& desc)
{
	gBepu_GetOptionsFromGUI(desc.GetTestName());

	ASSERT(!gBepu);
	gBepu = ICE_NEW(BepuPint);
	gBepu->Init(desc);
}

void Bepu_Close()
{
	if(gBepu)
	{
		gBepu->Close();
		delete gBepu;
		gBepu = null;
	}
}

BepuPint* GetBepu()
{
	return gBepu;
}

///////////////////////////////////////////////////////////////////////////////

static Widgets*	gBepuGUI = null;

static IceCheckBox*	gCheckBox_AllowSleeping = null;
static IceCheckBox*	gCheckBox_CCD = null;
static IceCheckBox*	gCheckBox_FlipMeshWinding = null;
static IceCheckBox*	gCheckBox_Gyroscopic = null;
static IceCheckBox*	gCheckBox_NativeIntegration = null;
static IceCheckBox*	gCheckBox_ScaleFriction = null;
static IceCheckBox*	gCheckBox_CollideJointed = null;
static IceCheckBox*	gCheckBox_DrawContacts = null;
static IceCheckBox*	gCheckBox_SubstepUnconstrained = null;

static IceEditBox* gEditBox_NbThreads = null;
static IceEditBox* gEditBox_NbSubsteps = null;
static IceEditBox* gEditBox_NbVelIter = null;
static IceEditBox* gEditBox_LinearDamping = null;
static IceEditBox* gEditBox_AngularDamping = null;
static IceEditBox* gEditBox_SpeculativeContactDistance = null;
static IceEditBox* gEditBox_Friction = null;
static IceEditBox* gEditBox_Restitution = null;
static IceEditBox* gEditBox_ContactHertz = null;
static IceEditBox* gEditBox_ContactDampingRatio = null;
static IceEditBox* gEditBox_MaxRecoveryVelocity = null;
static IceEditBox* gEditBox_JointHertz = null;
static IceEditBox* gEditBox_JointDampingRatio = null;
static IceEditBox* gEditBox_MotorMaxForce = null;
static IceEditBox* gEditBox_SweepConvergence = null;

enum BepuGUIElement
{
	Bepu_GUI_MAIN,
	//
	Bepu_GUI_ALLOW_SLEEPING,
	Bepu_GUI_ENABLE_CCD,
	Bepu_GUI_FLIP_MESH_WINDING,
	Bepu_GUI_GYROSCOPIC,
	Bepu_GUI_NATIVE_INTEGRATION,
	Bepu_GUI_SCALE_FRICTION,
	Bepu_GUI_COLLIDE_JOINTED,
	Bepu_GUI_DRAW_CONTACTS,
	Bepu_GUI_SUBSTEP_UNCONSTRAINED,
	//
};

static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
{
	const udword id = check_box.GetID();
	switch(id)
	{
		case Bepu_GUI_ALLOW_SLEEPING:
			gAllowSleeping = checked;
			break;
		case Bepu_GUI_ENABLE_CCD:
			gEnableCCD = checked;
			break;
		case Bepu_GUI_FLIP_MESH_WINDING:
			gFlipMeshWinding = checked;
			break;
		case Bepu_GUI_GYROSCOPIC:
			gAngularMode = checked ? Bepu::AngularIntegrationMode::ConserveMomentumWithGyroscopicTorque : Bepu::AngularIntegrationMode::Nonconserving;
			break;
		case Bepu_GUI_NATIVE_INTEGRATION:
			gUseNativeIntegrationCallback = checked;
			break;
		case Bepu_GUI_SCALE_FRICTION:
			gScaleFrictionByContactCount = checked;
			break;
		case Bepu_GUI_COLLIDE_JOINTED:
			gCollideJointed = checked;
			break;
		case Bepu_GUI_DRAW_CONTACTS:
			gDrawContacts = checked;
			break;
		case Bepu_GUI_SUBSTEP_UNCONSTRAINED:
			gSubstepUnconstrained = checked;
			break;
	}
}

static void gBepu_GetOptionsFromGUI(const char* test_name)
{
	if(gCheckBox_AllowSleeping)
		gAllowSleeping = gCheckBox_AllowSleeping->IsChecked();
	if(gCheckBox_CCD)
		gEnableCCD = gCheckBox_CCD->IsChecked();
	if(gCheckBox_FlipMeshWinding)
		gFlipMeshWinding = gCheckBox_FlipMeshWinding->IsChecked();
	if(gCheckBox_Gyroscopic)
		gAngularMode = gCheckBox_Gyroscopic->IsChecked() ? Bepu::AngularIntegrationMode::ConserveMomentumWithGyroscopicTorque : Bepu::AngularIntegrationMode::Nonconserving;
	if(gCheckBox_NativeIntegration)
		gUseNativeIntegrationCallback = gCheckBox_NativeIntegration->IsChecked();
	if(gCheckBox_ScaleFriction)
		gScaleFrictionByContactCount = gCheckBox_ScaleFriction->IsChecked();
	if(gCheckBox_CollideJointed)
		gCollideJointed = gCheckBox_CollideJointed->IsChecked();
	if(gCheckBox_DrawContacts)
		gDrawContacts = gCheckBox_DrawContacts->IsChecked();
	if(gCheckBox_SubstepUnconstrained)
		gSubstepUnconstrained = gCheckBox_SubstepUnconstrained->IsChecked();

	Common_GetFromEditBox(gNbThreads, gEditBox_NbThreads);
	Common_GetFromEditBox(gNbSubsteps, gEditBox_NbSubsteps);
	Common_GetFromEditBox(gNbVelIter, gEditBox_NbVelIter);
	Common_GetFromEditBox(gLinearDamping, gEditBox_LinearDamping, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gAngularDamping, gEditBox_AngularDamping, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gSpeculativeContactDistance, gEditBox_SpeculativeContactDistance, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gDefaultFriction, gEditBox_Friction, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gDefaultRestitution, gEditBox_Restitution, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gContactHertz, gEditBox_ContactHertz, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gContactDampingRatio, gEditBox_ContactDampingRatio, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gMaxRecoveryVelocity, gEditBox_MaxRecoveryVelocity, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gJointHertz, gEditBox_JointHertz, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gJointDampingRatio, gEditBox_JointDampingRatio, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gMotorMaxForce, gEditBox_MotorMaxForce, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gSweepConvergence, gEditBox_SweepConvergence, 0.0f, MAX_FLOAT);

	if(!gNbSubsteps)
		gNbSubsteps = 1;
	if(!gNbVelIter)
		gNbVelIter = 1;

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

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, label, gBepuGUI);
	IceEditBox* EB = helper.CreateEditBox(parent, INVALID_ID, 4+EditBoxX2, y, EditBoxWidth, 20, value, gBepuGUI, filter, null);
	y += YStep;
	return EB;
}

IceWindow* Bepu_InitGUI(IceWidget* parent, PintGUIHelper& helper)
{
	gHasGUI = true;
	IceWindow* Main = helper.CreateMainWindow(gBepuGUI, parent, Bepu_GUI_MAIN, "Bepu options");

	const sdword YStep = 20;
	const sdword YStepCB = 16;
	sdword y = 4;

	const udword CheckBoxWidth = 190;

	gCheckBox_AllowSleeping = helper.CreateCheckBox(Main, Bepu_GUI_ALLOW_SLEEPING, 4, y, CheckBoxWidth, 20, "Allow sleeping", gBepuGUI, gAllowSleeping, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_CCD = helper.CreateCheckBox(Main, Bepu_GUI_ENABLE_CCD, 4, y, CheckBoxWidth, 20, "Enable CCD", gBepuGUI, gEnableCCD, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_FlipMeshWinding = helper.CreateCheckBox(Main, Bepu_GUI_FLIP_MESH_WINDING, 4, y, CheckBoxWidth, 20, "Flip mesh winding", gBepuGUI, gFlipMeshWinding, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_Gyroscopic = helper.CreateCheckBox(Main, Bepu_GUI_GYROSCOPIC, 4, y, CheckBoxWidth, 20, "Gyroscopic forces", gBepuGUI, gAngularMode!=Bepu::AngularIntegrationMode::Nonconserving, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_NativeIntegration = helper.CreateCheckBox(Main, Bepu_GUI_NATIVE_INTEGRATION, 4, y, CheckBoxWidth, 20, "Native (C++) integration callback", gBepuGUI, gUseNativeIntegrationCallback, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_ScaleFriction = helper.CreateCheckBox(Main, Bepu_GUI_SCALE_FRICTION, 4, y, CheckBoxWidth, 20, "Coulomb friction (scale by nb contacts)", gBepuGUI, gScaleFrictionByContactCount, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_CollideJointed = helper.CreateCheckBox(Main, Bepu_GUI_COLLIDE_JOINTED, 4, y, CheckBoxWidth, 20, "Collision between jointed objects", gBepuGUI, gCollideJointed, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_DrawContacts = helper.CreateCheckBox(Main, Bepu_GUI_DRAW_CONTACTS, 4, y, CheckBoxWidth, 20, "Draw contacts", gBepuGUI, gDrawContacts, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_SubstepUnconstrained = helper.CreateCheckBox(Main, Bepu_GUI_SUBSTEP_UNCONSTRAINED, 4, y, CheckBoxWidth, 20, "Substeps for unconstrained bodies", gBepuGUI, gSubstepUnconstrained, gCheckBoxCallback);
	y += YStep;

	gEditBox_NbThreads					= CreateEditBox(helper, Main, y, "Nb threads (0==automatic):", _F("%d", gNbThreads), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbSubsteps					= CreateEditBox(helper, Main, y, "Nb substeps:", _F("%d", gNbSubsteps), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbVelIter					= CreateEditBox(helper, Main, y, "Nb vel iter per substep:", _F("%d", gNbVelIter), EDITBOX_INTEGER_POSITIVE);
	gEditBox_LinearDamping				= CreateEditBox(helper, Main, y, "Linear damping:", helper.Convert(gLinearDamping), EDITBOX_FLOAT_POSITIVE);
	gEditBox_AngularDamping				= CreateEditBox(helper, Main, y, "Angular damping:", helper.Convert(gAngularDamping), EDITBOX_FLOAT_POSITIVE);
	gEditBox_SpeculativeContactDistance	= CreateEditBox(helper, Main, y, "Max speculative dist (0==inf):", helper.Convert(gSpeculativeContactDistance), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Friction					= CreateEditBox(helper, Main, y, "Default friction:", helper.Convert(gDefaultFriction), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Restitution				= CreateEditBox(helper, Main, y, "Default restitution:", helper.Convert(gDefaultRestitution), EDITBOX_FLOAT_POSITIVE);
	gEditBox_ContactHertz				= CreateEditBox(helper, Main, y, "Contact hertz:", helper.Convert(gContactHertz), EDITBOX_FLOAT_POSITIVE);
	gEditBox_ContactDampingRatio		= CreateEditBox(helper, Main, y, "Contact damping ratio:", helper.Convert(gContactDampingRatio), EDITBOX_FLOAT_POSITIVE);
	gEditBox_MaxRecoveryVelocity		= CreateEditBox(helper, Main, y, "Max recovery velocity:", helper.Convert(gMaxRecoveryVelocity), EDITBOX_FLOAT_POSITIVE);
	gEditBox_JointHertz					= CreateEditBox(helper, Main, y, "Joint hertz:", helper.Convert(gJointHertz), EDITBOX_FLOAT_POSITIVE);
	gEditBox_JointDampingRatio			= CreateEditBox(helper, Main, y, "Joint damping ratio:", helper.Convert(gJointDampingRatio), EDITBOX_FLOAT_POSITIVE);
	gEditBox_MotorMaxForce				= CreateEditBox(helper, Main, y, "Motor max force (0==unlimited):", helper.Convert(gMotorMaxForce), EDITBOX_FLOAT_POSITIVE);
	gEditBox_SweepConvergence			= CreateEditBox(helper, Main, y, "Sweep accuracy (dist):", helper.Convert(gSweepConvergence), EDITBOX_FLOAT_POSITIVE);

	return Main;
}

void Bepu_CloseGUI()
{
	Common_CloseGUI(gBepuGUI);
	gCheckBox_AllowSleeping = null;
	gCheckBox_CCD = null;
	gCheckBox_FlipMeshWinding = null;
	gCheckBox_Gyroscopic = null;
	gCheckBox_NativeIntegration = null;
	gCheckBox_ScaleFriction = null;
	gCheckBox_CollideJointed = null;
	gCheckBox_DrawContacts = null;
	gCheckBox_SubstepUnconstrained = null;
	gEditBox_NbThreads = null;
	gEditBox_NbSubsteps = null;
	gEditBox_NbVelIter = null;
	gEditBox_LinearDamping = null;
	gEditBox_AngularDamping = null;
	gEditBox_SpeculativeContactDistance = null;
	gEditBox_Friction = null;
	gEditBox_Restitution = null;
	gEditBox_ContactHertz = null;
	gEditBox_ContactDampingRatio = null;
	gEditBox_MaxRecoveryVelocity = null;
	gEditBox_MotorMaxForce = null;
	gEditBox_SweepConvergence = null;
	gEditBox_JointHertz = null;
	gEditBox_JointDampingRatio = null;
}

///////////////////////////////////////////////////////////////////////////////

class BepuPlugIn : public PintPlugin
{
	public:
	virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper)	override	{ return Bepu_InitGUI(parent, helper);	}
	virtual	void		CloseGUI()											override	{ Bepu_CloseGUI();						}

	virtual	void		Init(const PINT_WORLD_CREATE& desc)					override	{ Bepu_Init(desc);						}
	virtual	void		Close()												override	{ Bepu_Close();						}

	virtual	Pint*		GetPint()											override	{ return GetBepu();					}

	virtual	IceWindow*	InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)	override;
	virtual	void		CloseTestGUI()																					override;
	virtual	const char*	GetTestGUIName()																				override	{ return "Bepu";	}
	virtual	void		ApplyTestUIParams(const char* test_name)														override;
};
static BepuPlugIn gPlugIn;

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
static const udword CheckBoxWidth = 200;

static IceCheckBox*	gCheckBox_Override = null;
static IceCheckBox*	gCheckBox_Generic0 = null;

static const char* gDesc_CCD =
"By design the CCD tests fail without CCD enabled.\n\
Check this box to make them pass using the\n\
Bepu 'continuous' collision detection mode.";

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
		gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, x2, y, CheckBoxWidth2, 20, "Enable Bepu CCD", &owner, EnableCCD, null, null);
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

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_Dzhanibekov(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, bool enabled)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void Gyro(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gCheckBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::Gyro, null);
	y += YStep;

	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable gyroscopic forces", &owner, enabled, null, null);
	y += YStep;

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

IceWindow* BepuPlugIn::InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	if(IsCCDTest(test_name))
		return CreateUI_CCD(parent, helper, owner, test_name);

	if(	strcmp(test_name, "Dzhanibekov")==0)
		return CreateUI_Dzhanibekov(parent, helper, owner, true);

	return null;
}

void BepuPlugIn::CloseTestGUI()
{
	gCheckBox_Override = null;
	gCheckBox_Generic0 = null;
}

void BepuPlugIn::ApplyTestUIParams(const char* test_name)
{
	const bool ApplySettings = gCheckBox_Override ? gCheckBox_Override->IsChecked() : false;
	if(!ApplySettings)
		return;

	if(IsCCDTest(test_name))
	{
		gEnableCCD = gCheckBox_Generic0->IsChecked();
		return;
	}

	if(strcmp(test_name, "Dzhanibekov")==0)
	{
		if(gCheckBox_Generic0->IsChecked())
		{
			gAngularMode = Bepu::AngularIntegrationMode::ConserveMomentumWithGyroscopicTorque;
			gAngularDamping = 0.0f;
			// The implicit gyroscopic integrator dissipates the intermediate-axis mode heavily at
			// full-dt steps (tumbling dies within seconds). Integrating the free body with the
			// solver's substeps preserves it.
			gSubstepUnconstrained = true;
			if(gNbSubsteps<8)
				gNbSubsteps = 8;
		}
		return;
	}
}
