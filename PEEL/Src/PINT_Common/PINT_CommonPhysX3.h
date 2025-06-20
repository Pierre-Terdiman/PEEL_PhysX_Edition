///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_H
#define PINT_COMMON_PHYSX3_H

#include "PINT_CommonShapes.h"
#include "PINT_CommonPhysX3_Defines.h"

#include "..\Pint.h"
#include "PINT_CommonPhysX3_Base.h"
#include "PINT_CommonPhysX3_Error.h"
#include "PINT_CommonPhysX3_Allocator.h"
#include "PINT_CommonPhysX3_Scene.h"
#include "PINT_CommonPhysX3_Actor.h"
#include "PINT_CommonPhysX3_ActorManager.h"
#include "PINT_CommonPhysX3_Shape.h"
#include "PINT_CommonPhysX3_Joint.h"
#include "PINT_CommonPhysX3_Mesh.h"
#include "PINT_CommonPhysX3_FilterShader.h"
#include "PINT_CommonPhysX3_DebugViz.h"
#include "PINT_CommonPhysX_FoundationAPI.h"
#include "PINT_CommonPhysX_Names.h"

#include <Core/RockHashMap.h>

	inline_ u32 ComputeHash(const PintActorHandle p)
	{
		return Rock::ComputeHash64(uint64_t(p));
	}

#ifdef PHYSX_SUPPORT_CHARACTERS2
	#include "PINT_CommonPhysX3_CCT.h"
#endif

#if PHYSX_SUPPORT_GEAR_JOINT
	namespace physx
	{
		class PxGearJoint;
	}
#endif
#if PHYSX_SUPPORT_RACK_JOINT
	namespace physx
	{
		class PxRackAndPinionJoint;
	}
#endif
#if PHYSX_SUPPORT_PORTAL_JOINT
	namespace physx
	{
		class PxPortalJoint;
	}
#endif

	inline_	PintActorHandle CreateHandle(PxRigidActor* actor)
	{
#ifdef DEPRECATED
		const size_t binary = size_t(actor);
		ASSERT(!(binary&1));
		return PintObjectHandle(binary);
#else
		return PintActorHandle(actor);
#endif
	}

/*	inline_	PintObjectHandle CreateHandle(PxShape* shape)
	{
		const size_t binary = size_t(shape);
		ASSERT(!(binary&1));
		return PintObjectHandle(binary|1);
	}*/

	inline_	PxRigidActor* GetActorFromHandle(PintActorHandle handle)
	{
#ifdef DEPRECATED
		const size_t binary = size_t(handle);
		return (binary & 1) ? null : (PxRigidActor*)binary;
#else
		return reinterpret_cast<PxRigidActor*>(handle);
#endif
	}

#ifdef DEPRECATED
	inline_	PxShape* GetShapeFromHandle(PintObjectHandle handle)
	{
		const size_t binary = size_t(handle);
		return (binary & 1) ? (PxShape*)(binary&~1) : null;
	}
#endif

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

	namespace PhysX3
	{
		void			ComputeCapsuleTransform(PR& dst, const PR& src);
//		void			SetGroup(PxShape& shape, PxU16 collision_group);
		PxRigidBody*	GetRigidBody(PintActorHandle handle);
	}

	struct EditableParams
	{
										EditableParams();
		// Main
		udword							mNbThreadsIndex;

		inline_	const char*	ThreadIndexToName(udword index)	const
		{
			if(index==0)
				return "Single threaded";
			if(index==1)
				return "1 main + 1 worker thread";
			if(index==2)
				return "1 main + 2 worker threads";
			if(index==3)
				return "1 main + 3 worker threads";
			if(index==4)
				return "1 main + 4 worker threads";
			if(index==5)
				return "1 main + 8 worker threads";
			if(index==6)
				return "1 main + 12 worker threads";
			if(index==7)
				return "1 main + 15 worker threads";
			return null;
		}

		inline_	udword	ThreadIndexToNbThreads(udword index)	const
		{
			if(index==0)
				return 0;	// "Single threaded"
			if(index==1)
				return 1;	// "1 main + 1 worker thread"
			if(index==2)
				return 2;	// "1 main + 2 worker threads"
			if(index==3)
				return 3;	// "1 main + 3 worker threads"
			if(index==4)
				return 4;	// "1 main + 4 worker threads"
			if(index==5)
				return 8;	// "1 main + 8 worker threads"
			if(index==6)
				return 12;	// "1 main + 12 worker threads"
			if(index==7)
				return 15;	// "1 main + 15 worker threads"
			return null;
		}

		inline_	udword	NbThreadsToThreadIndex(udword nb_threads)	const
		{
			if(nb_threads==0 || nb_threads==1)
				return 0;	// "Single threaded"
			/*if(nb_threads==0)
				return 0;
			if(nb_threads==1)
				return 1;*/
			if(nb_threads==2)
				return 2;		// "1 main + 2 worker threads"
			if(nb_threads<=4)
				return 4;		// "1 main + 4 worker threads"
			if(nb_threads<=8)
				return 5;		// "1 main + 8 worker threads"
			if(nb_threads<=12)
				return 6;		// "1 main + 12 worker threads"
			return 7;			// "1 main + 15 worker threads"
		}

#if PHYSX_SUPPORT_SCRATCH_BUFFER
		udword							mScratchSize;
#endif
#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
		PxBroadPhaseType::Enum			mBroadPhaseType;
		udword							mMBPSubdivLevel;
		float							mMBPRange;
#endif
#if PHYSX_SUPPORT_CPU_DISPATCHER_MODE
		PxDefaultCpuDispatcherWaitForWorkMode::Enum		mCPUDispatcherMode;
#endif
		bool							mGroundPlane;
		bool							mEnableCCD;
#if PHYSX_SUPPORT_ANGULAR_CCD
		bool							mEnableAngularCCD;
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
		bool							mEnableRaycastCCDStatic;
		bool							mEnableRaycastCCDDynamic;
#endif
		bool							mShareMeshData;
		bool							mShareShapes;
#if PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS
		bool							mUseTightConvexBounds;
#endif
#if PHYSX_SUPPORT_CONVEX_CORE_GEOMETRY
		bool							mUseConvexCoreCylinders;
#endif
		bool							mPCM;
#ifdef PHYSX_SUPPORT_SSE_FLAG
		bool							mEnableSSE;
#endif
		bool							mEnableActiveTransforms;
		bool							mEnableContactCache;
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
		bool							mEnableContactNotif;
#endif
		bool							mFlushSimulation;
		bool							mEnhancedDeterminism;
		bool							mUsePVD;
		bool							mUseFullPvdConnection;
#if PHYSX_SUPPORT_GPU
		bool							mUseGPU;
		inline_	PxConvexFlags			GetConvexFlags()	const
		{
			return mUseGPU ? (PxConvexFlag::eCOMPUTE_CONVEX|PxConvexFlag_eGPU_COMPATIBLE) : PxConvexFlag::eCOMPUTE_CONVEX;
		}
	#if PHYSX_SUPPORT_DIRECT_GPU
		bool							mUseDirectGPU;
	#endif
#else
		inline_	PxConvexFlags			GetConvexFlags()	const
		{
			return PxConvexFlag::eCOMPUTE_CONVEX;
		}
#endif
		//float							mGlobalBoxSize;
		float							mContactOffset;
		float							mRestOffset;
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
		float							mContactNotifThreshold;
#endif
#if PHYSX_SUPPORT_DYNAMIC_MESHES
		float							mSDFSpacing;
#endif
#if PHYSX_SUPPORT_SUBSTEPS
		udword							mNbSubsteps;
#endif

		// Dynamics
		udword							mSolverBatchSize;
		float							mMaxBiasCoeff;
		float							mDefaultStaticFriction;
		float							mDefaultDynamicFriction;
		float							mDefaultRestitution;
#if PHYSX_SUPPORT_COMPLIANT_CONTACTS
		float							mDefaultContactStiffness;
		float							mDefaultContactDamping;
#endif
		float							mFrictionOffsetThreshold;
#if PHYSX_SUPPORT_TORSION_FRICTION
		float							mTorsionalPatchRadius;
		float							mMinTorsionalPatchRadius;
#endif
		bool							mEnableSleeping;
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
		bool							mImprovedPatchFriction;
#endif
#if PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
		bool							mFrictionEveryIteration;
#endif
#if PHYSX_SUPPORT_EXTERNAL_FORCES_EVERY_ITERATION
		bool							mExternalForcesEveryIteration;
#endif
		bool							mDisableStrongFriction;
#if PHYSX_SUPPORT_POINT_FRICTION
		bool							mEnableOneDirFriction;
		bool							mEnableTwoDirFriction;
#endif
#if PHYSX_SUPPORT_ADAPTIVE_FORCE
		bool							mAdaptiveForce;
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		bool							mStabilization;
#endif
#if PHYSX_SUPPORT_TGS
		bool							mTGS;
#endif
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
		bool							mGyro;
#endif
#ifndef IS_PHYSX_3_2
		udword							mMaxNbCCDPasses;
#endif
		udword							mSolverIterationCountPos;
		udword							mSolverIterationCountVel;
		float							mLinearDamping;
		float							mAngularDamping;
		float							mMaxAngularVelocity;
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
		float							mMaxDepenVelocity;
#endif
		float							mSleepThreshold;
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		float							mStabilizationThreshold;
#endif
		// Scene queries
		PxPruningStructureType::Enum	mStaticPruner;
		PxPruningStructureType::Enum	mDynamicPruner;
		udword							mSQDynamicRebuildRateHint;
		bool							mSQFlag;
		bool							mSQFilterOutAllShapes;
		bool							mSQInitialOverlap;
		//bool							mSQManualFlushUpdates;
		bool							mSQPreciseSweeps;
		bool							mSQBothSides;

		// Joints
#if PHYSX_SUPPORT_JOINT_PROJECTION
		bool							mEnableJointProjection;
#endif
#if PHYSX_SUPPORT_JOINT_CONTACT_DISTANCE
//		bool							mEnableJointContactDistance;
#endif
		bool							mUseD6Joints;
#if PHYSX_SUPPORT_DISABLE_PREPROCESSING
		bool							mDisablePreprocessing;
#endif
#ifndef IS_PHYSX_3_2
	#if PHYSX_REMOVE_JOINT_32_COMPATIBILITY
	#else
		bool							mEnableJoint32Compatibility;
	#endif
#endif
		float							mProjectionLinearTolerance;
		float							mProjectionAngularTolerance;
#ifndef IS_PHYSX_3_2
		float							mInverseInertiaScale;
		float							mInverseMassScale;
#endif
#ifdef PHYSX_SUPPORT_LINEAR_COEFF
		float							mLinearCoeff;
#endif
#if PHYSX_SUPPORT_JOINT_CONTACT_DISTANCE
		udword							mLimitsContactDistance;
#endif
		// Articulations
#if PHYSX_SUPPORT_ARTICULATIONS || PHYSX_SUPPORT_RCA
		bool							mDisableArticulations;
#endif
#if PHYSX_SUPPORT_ARTICULATIONS
		udword							mMaxProjectionIterations;
		float							mSeparationTolerance;
		udword							mExternalDriveIterations;
		udword							mInternalDriveIterations;
#endif
#if PHYSX_SUPPORT_RCA_CFM_SCALE
		float							mRCACfmScale;
#endif
#if PHYSX_SUPPORT_RCA_DOF_SCALE
		float							mRCADofScale;
#endif
#if PHYSX_SUPPORT_RCA_ARMATURE
		float							mRCAArmature;
#endif
		// Cooking
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE
		PxMeshMidPhase::Enum			mMidPhaseType;
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
		udword							mNbTrisPerLeaf;
#endif
#if PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
		PxBVH34BuildStrategy::Enum		mMeshBuildStrategy;
#endif
#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
		PxMeshCookingHint::Enum			mMeshCookingHint;
#endif
#if PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT
		udword							mGaussMapLimit;
#endif
#if PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE
		bool							mPrecomputeActiveEdges;
#endif
#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
		bool							mQuantizedTrees;
#endif

		// Vehicles
#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
		bool							mUseSuspensionSweeps;
		bool							mForceSphereWheels;
		bool							mUseBlockingSweeps;
		bool							mDrawSweptWheels;
#endif
		bool							mDrawSuspensionCasts;
		bool							mDrawPhysXTelemetry;
#ifdef PHYSX_SUPPORT_VEHICLE_FIX
		bool							mUseFix;
#endif
#if PHYSX_SUPPORT_STEER_FILTER
		float							mSteerFilter;
#endif
#if PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
		float							mSweepInflation;
#endif
#if PHYSX_SUPPORT_GPU
		udword							mNbGpuPartitions;
		udword							mGpuBuffersSizeMultiplier;
#endif
#if PHYSX_SUPPORT_FLUIDS
		// Fluids
		float							mFluidFriction;
		float							mFluidDamping;
		float							mFluidAdhesion;
		float							mFluidViscosity;
		float							mFluidVorticityConfinement;
		float							mFluidSurfaceTension;
		float							mFluidCohesion;
		float							mFluidLift;
		float							mFluidDrag;
		float							mFluidCflCoefficient;
		float							mFluidGravityScale;
		float							mFluidParticleMass;
		bool							mFluidParticleCCD;
#endif

		// Debug viz
		float							mDebugVizScale;
		bool							mForceJointLimitDebugViz;

		bool							mLast;
	};

	class MotorData : public Allocateable
	{
		public:
			MotorData() :
				mAxis				(PxArticulationAxis::eCOUNT),
				mStiffness			(0.0f),
				mDamping			(0.0f),
				mMaxForce			(0.0f),
				mAccelerationDrive	(false)
			{
			}

			~MotorData()
			{
			}

		PxArticulationAxis::Enum	mAxis;
		float						mStiffness;
		float						mDamping;
		float						mMaxForce;
		bool						mAccelerationDrive;
	};

	struct LocalShapeCreationParams
	{
		const char*			mForcedName;
		PxRigidActor*		mActor;
		PintCollisionGroup	mGroup;
		bool				mIsDynamic;
	};

	class SharedPhysX : public Pint, public MeshManager, public PintShapeEnumerateCallback//, public PxDeletionListener
#ifndef IS_PHYSX_3_2
		, public PxQueryFilterCallback
#endif
	{
		public:
											SharedPhysX(const EditableParams& params, const char* test_name);
		virtual								~SharedPhysX();

		// PxDeletionListener
//		virtual	void						onRelease(const PxBase* observed, void* userData, PxDeletionEventFlag::Enum deletionEvent);
		//~PxDeletionListener

		// PintShapeEnumerateCallback
		virtual	void						ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)	override;
		//~PintShapeEnumerateCallback

#ifndef IS_PHYSX_3_2
		// PxQueryFilterCallback
		virtual PxQueryHitType::Enum		preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)	override;
		virtual PxQueryHitType::Enum		postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor)	/*override*/;
		virtual PxQueryHitType::Enum		postFilter(const PxFilterData& filterData, const PxQueryHit& hit)													/*override*/;
		//~PxQueryFilterCallback
#endif
		virtual	void						SetGravity(const Point& gravity)	override;
//		virtual	void						Close();

		virtual	void						SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)	override;
		virtual	PintActorHandle				CreateObject(const PINT_OBJECT_CREATE& desc)							override;
		virtual	bool						ReleaseObject(PintActorHandle handle)									override;
		virtual	PintJointHandle				CreateJoint(const PINT_JOINT_CREATE& desc)								override;
		virtual	bool						ReleaseJoint(PintJointHandle handle)									override;

#ifndef IS_PHYSX_3_2
		virtual	bool						SetSQFlag(PintActorHandle actor, bool flag)							override;
		virtual	bool						SetSQFlag(PintActorHandle actor, PintShapeHandle shape, bool flag)	override;
		virtual	bool						ResetSQFilters()													override;
#endif

		virtual	PR							GetWorldTransform(PintActorHandle handle)					override;
		virtual	void						SetWorldTransform(PintActorHandle handle, const PR& pose)	override;

#ifdef DEPRECATED
		virtual	bool						GetBounds(PintActorHandle handle, AABB& bounds);

		virtual	const char*					GetName(PintActorHandle handle);
		virtual	bool						SetName(PintActorHandle handle, const char* name);
#endif

//		virtual	void						ApplyActionAtPoint(PintActorHandle handle, PintActionType action_type, const Point& action, const Point& pos);
		virtual	void						AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)	override;
		virtual	void						AddLocalTorque(PintActorHandle handle, const Point& local_torque)										override;

		virtual	Point						GetLinearVelocity(PintActorHandle handle)								override;
		virtual	void						SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)	override;

		virtual	Point						GetAngularVelocity(PintActorHandle handle)									override;
		virtual	void						SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)	override;

#ifdef DEPRECATED
			virtual	Point					GetWorldLinearVelocity(PintActorHandle handle);
			virtual	void					SetWorldLinearVelocity(PintActorHandle handle, const Point& linear_velocity);

			virtual	Point					GetWorldAngularVelocity(PintActorHandle handle);
			virtual	void					SetWorldAngularVelocity(PintActorHandle handle, const Point& angular_velocity);

		virtual	float						GetMass(PintActorHandle handle);
		virtual	void						SetMass(PintActorHandle handle, float mass);

		virtual	Point						GetLocalInertia(PintActorHandle handle);
		virtual	void						SetLocalInertia(PintActorHandle handle, const Point& inertia);

		virtual	udword						GetShapes(PintObjectHandle* shapes, PintObjectHandle handle);
		virtual	void						SetLocalRot(PintObjectHandle handle, const Quat& q);

		virtual	bool						EnableGravity(PintActorHandle handle, bool flag);

		virtual	float						GetLinearDamping(PintActorHandle handle);
		virtual	float						GetAngularDamping(PintActorHandle handle);
		virtual	void						SetLinearDamping(PintActorHandle handle, float damping);
		virtual	void						SetAngularDamping(PintActorHandle handle, float damping);
#endif

		virtual	bool						SetKinematicPose(PintActorHandle handle, const Point& pos)	override;
		virtual	bool						SetKinematicPose(PintActorHandle handle, const PR& pr)		override;
		virtual	bool						IsKinematic(PintActorHandle handle)							override;
		virtual	bool						EnableKinematic(PintActorHandle handle, bool flag)			override;

		virtual	PintConvexHandle			CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index)	override;
		virtual	bool						DeleteConvexObject(PintConvexHandle handle, const PintConvexIndex* index)		override;
		virtual	PintMeshHandle				CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)		override;
		virtual	bool						DeleteMeshObject(PintMeshHandle handle, const PintMeshIndex* index)				override;
#if PHYSX_SUPPORT_HEIGHTFIELDS
		virtual	PintHeightfieldHandle		CreateHeightfieldObject(const PINT_HEIGHTFIELD_DATA_CREATE& desc, PintHeightfieldData& data, PintHeightfieldIndex* index)	override;
		virtual	bool						DeleteHeightfieldObject(PintHeightfieldHandle handle, const PintHeightfieldIndex* index)									override;
#endif
		virtual	PintAggregateHandle			CreateAggregate(udword max_size, bool enable_self_collision)			override;
		virtual	bool						AddToAggregate(PintActorHandle object, PintAggregateHandle aggregate)	override;
		virtual	bool						AddAggregateToScene(PintAggregateHandle aggregate)						override;

#if PHYSX_SUPPORT_ARTICULATIONS
		virtual	PintArticHandle				CreateArticulation(const PINT_ARTICULATION_CREATE&)																		override;
		virtual	PintActorHandle				CreateArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_ARTICULATED_BODY_CREATE&, PintArticHandle articulation)	override;
		virtual	bool						AddArticulationToScene(PintArticHandle articulation)																	override;
		virtual	void						SetArticulatedMotor(PintActorHandle object, const PINT_ARTICULATED_MOTOR_CREATE& motor)									override;
#endif
#if PHYSX_SUPPORT_RCA
		virtual	PintArticHandle				CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE&)																	override;
		virtual	PintActorHandle				CreateRCArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_RC_ARTICULATED_BODY_CREATE&, PintArticHandle articulation)	override;
		virtual	bool						AddRCArticulationToScene(PintArticHandle articulation)																		override;
		virtual	bool						AddRCArticulationToAggregate(PintArticHandle articulation, PintAggregateHandle aggregate)									override;
		virtual	bool						SetRCADriveEnabled(PintActorHandle handle, bool flag)																		override;
		virtual	bool						SetRCADriveVelocity(PintActorHandle handle, float velocity)																	override;
		virtual	bool						SetRCADrivePosition(PintActorHandle handle, float position)																	override;
#endif

		virtual	udword						BatchRaycasts				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)							override;
		virtual	udword						BatchRaycastAny				(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)							override;
		virtual	udword						BatchRaycastAll				(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintRaycastData* raycasts)	override;
//		virtual	bool						Picking						(PintSQThreadContext context, PintRaycastHit* dest, const PintRaycastData& raycast, PintQueryFilterCallback* cb);

		virtual	udword						BatchBoxSweeps				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)		override;
		virtual	udword						BatchSphereSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)	override;
		virtual	udword						BatchCapsuleSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)	override;
		virtual	udword						BatchConvexSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)	override;

		virtual	udword						BatchSphereOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)	override;
		virtual	udword						BatchBoxOverlapAny			(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)		override;
		virtual	udword						BatchCapsuleOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)	override;
		virtual	udword						BatchConvexOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintConvexOverlapData* overlaps)	override;

		virtual	udword						BatchSphereOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)	override;
		virtual	udword						BatchBoxOverlapObjects		(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)		override;
		virtual	udword						BatchCapsuleOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)	override;

		virtual	udword						FindTriangles_MeshSphereOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)	override;
		virtual	udword						FindTriangles_MeshBoxOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)		override;
		virtual	udword						FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)	override;

		virtual	bool						SetDriveEnabled(PintJointHandle handle, bool flag)									override;
		virtual	bool						SetDrivePosition(PintJointHandle handle, const PR& pose)							override;
		virtual	bool						SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)	override;
		virtual	PR							GetDrivePosition(PintJointHandle handle)											override;
//		virtual	void						SetGearJointError(PintJointHandle handle, float error)								override;
		virtual	void						SetPortalJointRelativePose(PintJointHandle handle, const PR& pose)					override;

				PintJointHandle				CreateJoint(PxPhysics& physics, const PINT_JOINT_CREATE& desc);
#if PHYSX_SUPPORT_ARTICULATIONS
				PintActorHandle				CreateArticulationLink(PxArticulation* articulation, PxArticulationLink* parent, Pint& pint, const PINT_OBJECT_CREATE& desc);
#endif
#if PHYSX_SUPPORT_RCA
				PintActorHandle				CreateRCArticulationLink(PxArticulationReducedCoordinate* articulation, PxArticulationLink* parent, Pint& pint, const PINT_OBJECT_CREATE& desc);
#endif

		virtual	void						Render(PintRender& renderer, PintRenderPass render_pass)	override;
		virtual	void						RenderDebugData(PintRender& renderer)						override;

		virtual	Pint_Scene*					GetSceneAPI()	override	{ return &mSceneAPI;	}
		virtual	Pint_Actor*					GetActorAPI()	override	{ return &mActorAPI;	}
		virtual	Pint_Shape*					GetShapeAPI()	override	{ return &mShapeAPI;	}
		virtual	Pint_Joint*					GetJointAPI()	override	{ return &mJointAPI;	}

#ifdef PHYSX_SUPPORT_CHARACTERS
		virtual	PintCharacterHandle			CreateCharacter(const PINT_CHARACTER_CREATE& create)	override;
		virtual	PintActorHandle				GetCharacterActor(PintCharacterHandle h)				override;
		virtual	udword						MoveCharacter(PintCharacterHandle h, const Point& disp)	override;
#endif
#ifdef PHYSX_SUPPORT_CHARACTERS2
		virtual	Pint_Character*				GetCharacterAPI()	override	{ return &mCCT;	}
#endif
		inline_	void						AddActorToScene(PxRigidActor* actor)
											{
												mScene->addActor(*actor);
												mActorManager.Add(actor);
											}

#if PHYSX_SUPPORT_BVH_STRUCTURE
		inline_	void						AddActorToScene(PxRigidActor* actor, PxBVH* bvh)
											{
												mScene->addActor(*actor, bvh);
												mActorManager.Add(actor);
											}
#endif

		inline_	void						AddActorToManager(PxRigidActor* actor)
											{
												mActorManager.Add(actor);
											}

		inline_	void						RemoveActor(PxRigidActor* actor)
											{
												bool status = mActorManager.Remove(actor);
												ASSERT(status);
											}

		inline_	PxPhysics*					GetPhysics()				{ return mPhysics;			}
#if PHYSX_SUPPORT_IMMEDIATE_COOKING
		inline_	const PxCookingParams&		GetCooking()		const	{ return mCookingParams;				}
#else
		inline_	PxCooking&					GetCooking()				{ ASSERT(mCooking)	return *mCooking;	}
#endif
		inline_	PxScene*					GetScene()					{ return mScene;			}
		inline_	PxMaterial*					GetDefaultMaterial()		{ return mDefaultMaterial;	}
		inline_	ActorManager&				GetActorManager()			{ return mActorManager;		}
		inline_	const EditableParams&		GetParams()	const			{ return mParams;			}

				void						SetActorName(PxRigidActor* actor, const char* name)	{ mNames.SetNameT<PxRigidActor>(actor, name);	}
				void						SetShapeName(PxShape* shape, const char* name)		{ mNames.SetNameT<PxShape>(shape, name);		}
				void						SetJointName(PxJoint* joint, const char* name)		{ mNames.SetNameT<PxJoint>(joint, name);		}

		protected:
				void						SharedUpdateFromUI();

				PxFoundation*				mFoundation;
				PxPhysics*					mPhysics;
				PxScene*					mScene;
#if PHYSX_SUPPORT_IMMEDIATE_COOKING
				PxCookingParams				mCookingParams;
#else
				PxCooking*					mCooking;
#endif
				PxMaterial*					mDefaultMaterial;
#ifdef PHYSX_SUPPORT_CHARACTERS
				PxControllerManager*		mControllerManager;
#endif
				MeshObjectManager			mConvexObjects;
				MeshObjectManager			mMeshObjects;
#if PHYSX_SUPPORT_HEIGHTFIELDS
				MeshObjectManager			mHeightfieldObjects;
				struct HFCompanionData : public Allocateable
				{
					HFCompanionData() : mHeightScale(0.0f), mMinHeight(0.0f)	{}

					float	mHeightScale;
					float	mMinHeight;
				};
				HFCompanionData*			mHFData;
				udword						mHFDataSize;
#endif
				ActorManager				mActorManager;

				PhysX_SceneAPI				mSceneAPI;
				PhysX_ActorAPI				mActorAPI;
				PhysX_ShapeAPI				mShapeAPI;
				PhysX_JointAPI				mJointAPI;

#ifdef PHYSX_SUPPORT_CHARACTERS2
				PhysX_CCT_API				mCCT;
#endif
				const EditableParams&		mParams;
				float						mContactNotifThreshold;
				bool						mEnableContactNotif;

#ifndef IS_PHYSX_3_2
				void						CreateCooking(const PxTolerancesScale& scale, PxMeshPreprocessingFlags mesh_preprocess_params);
#endif
				PxMaterial*					CreateMaterial(const PINT_MATERIAL_CREATE& desc);
//				PxConvexMesh*				CreateConvexMesh(const Point* verts, udword vertCount, PxConvexFlags flags, PintShapeRenderer* renderer);
		// MeshManager
		virtual	PxConvexMesh*				CreatePhysXConvex(udword nb_verts, const Point* verts, PxConvexFlags flags)			override;
		virtual	PxTriangleMesh*				CreatePhysXMesh(const PintSurfaceInterface& surface, bool deformable, bool dynamic)	override;
		//~MeshManager
//				PxTriangleMesh*				CreateTriangleMesh(const SurfaceInterface& surface, PintShapeRenderer* renderer, bool deformable);

#if PHYSX_SUPPORT_SCRATCH_BUFFER
		inline_	void*						GetScratchPad()				{ return mScratchPad;		}
		inline_	udword						GetScratchPadSize()	const	{ return mScratchPadSize;	}
#else
		inline_	void*						GetScratchPad()				{ return null;				}
		inline_	udword						GetScratchPadSize()	const	{ return 0;					}
#endif
				void						SetupDynamic(PxRigidDynamic& rigidDynamic, const PINT_OBJECT_CREATE& desc);
				void						SetupArticulationLink(PxArticulationLink& link, const PINT_OBJECT_CREATE& desc);
				void						InitCommon();
				void						CloseCommon();
				void						UpdateCommon(float dt);
				void						EndCommonUpdate();
				void						ApplyLocalTorques();
				void						UpdateJointErrors();

#ifdef IS_PHYSX_3_2
		inline_	PxSceneQueryFilterCallback*	GetSQFilterCallback()
											{
												return null;
											}

		inline_	PxSceneQueryFilterData		GetSQFilterData()
											{
												const PxQueryFlags qf = PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC;
												return PxSceneQueryFilterData(PxFilterData(!mParams.mSQFilterOutAllShapes, mParams.mSQFilterOutAllShapes, 0, 0), qf);
											}
#else
		inline_	PxQueryFilterCallback*		GetSQFilterCallback()
											{
												return mInvisibles ? this : null;
											}

		inline_	PxQueryFilterData			GetSQFilterData()
											{
												PxQueryFlags qf = PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC;
												if(mInvisibles)
													qf |= PxQueryFlag::ePREFILTER;
												return PxQueryFilterData(PxFilterData(!mParams.mSQFilterOutAllShapes, mParams.mSQFilterOutAllShapes, 0, 0), qf);
											}
#endif

#ifdef PHYSX_SUPPORT_CHARACTERS
				PxControllerManager*		GetControllerManager();
				void						ReleaseControllerManager();
#endif
		protected:
#ifdef SHARED_SHAPES_USE_HASH
				typedef PxHashMap<InternalSphereShape, ShapePtr>	SharedSpheres;
				SharedSpheres*						mSphereShapes;
#else
				SharedSphereShapes					mSphereShapes;
#endif
				SharedBoxShapes						mBoxShapes;
				SharedCapsuleShapes					mCapsuleShapes;
#if PHYSX_SUPPORT_CUSTOM_GEOMETRY
				SharedCapsuleShapes					mCylinderShapes;
				PtrContainer						mCylinderCBs;
#endif
				SharedMeshShapes					mConvexShapes;
				SharedMeshShapes					mMeshShapes;
#if PHYSX_SUPPORT_HEIGHTFIELDS
				SharedMeshShapes					mHeightfieldShapes;
#endif

#ifdef IS_PHYSX_3_2
		// Virtual on purpose, overriden in PhysX3.2 Pint plugin
		virtual	void								CreateShapes			(const PINT_OBJECT_CREATE& desc, PxRigidActor* actor, PintCollisionGroup group, const char* forced_name){}
#else
		private:
				PintActorHandle						CreateGroundPlane(const PINT_OBJECT_CREATE& desc);
#if PHYSX_SUPPORT_RCA
				void								SetupRCAJoint(const EditableParams& params, PxArticulationJointReducedCoordinate* j, const PINT_RC_ARTICULATED_BODY_CREATE& bc);
				PxArticulationJointReducedCoordinate*	RetrieveJointData(PintActorHandle handle, MotorData& data)	const;
#endif
				PxShape*							CreateSphereShape		(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxSphereGeometry& geometry,		const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group);
				PxShape*							CreateBoxShape			(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxBoxGeometry& geometry,			const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group);
				PxShape*							CreateCapsuleShape		(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxCapsuleGeometry& geometry,		const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group);
#if PHYSX_SUPPORT_CUSTOM_GEOMETRY
				PxShape*							CreateCylinderShape		(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxCustomGeometry& geometry,		const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group);
#endif
#if PHYSX_SUPPORT_CONVEX_CORE_GEOMETRY
				PxShape*							CreateConvexCoreShape	(const PINT_CYLINDER_CREATE* create, PxRigidActor* actor, const PxConvexCoreGeometry& geometry,	const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group);
#endif
				PxShape*							CreateConvexShape		(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxConvexMeshGeometry& geometry,	const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group);
				PxShape*							CreateMeshShape			(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxTriangleMeshGeometry& geometry,	const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group);
#if PHYSX_SUPPORT_HEIGHTFIELDS
				PxShape*							CreateHeightfieldShape	(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxHeightFieldGeometry& geometry,	const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group);
#endif
		protected:
				void								CreateShapes			(const PINT_OBJECT_CREATE& desc, PxRigidActor* actor, PintCollisionGroup group, const char* forced_name);
#endif

#if PHYSX_SUPPORT_SCRATCH_BUFFER
				void*						mScratchPad;
				udword						mScratchPadSize;
#endif
				std::vector<PxMaterial*>	mMaterials;

				struct LocalTorque
				{
					LocalTorque(PintActorHandle handle, const Point& local_torque) :
						mHandle		(handle),
						mLocalTorque(local_torque)
					{
					}
					PintActorHandle			mHandle;
					Point					mLocalTorque;
				};
				std::vector<LocalTorque>	mLocalTorques;
				std::vector<MotorData*>		mMotorData;
#ifdef PHYSX_NO_USERDATA_RCA_JOINT
				// Unfortunately some PhysX versions didn't have userData in PxArticulationJointReducedCoordinate
				PxHashMap<PxArticulationJointReducedCoordinate*, const MotorData*>*	mJointRCA_UserData;
#endif
#if PHYSX_SUPPORT_GEAR_JOINT
				std::vector<PxGearJoint*>	mGearJoints;
#endif
#if PHYSX_SUPPORT_RACK_JOINT
				std::vector<PxRackAndPinionJoint*>	mRackJoints;
#endif
#ifndef IS_PHYSX_3_2
				PxHashSet<PintActorHandle>*	mInvisibles;
				//Rock::CoalescedHashSet<PintActorHandle>*	mInvisibles;
#endif
				DebugVizHelper				mDebugVizHelper;

				Names						mNames;
	};

	template<class T>
	inline_ void SetupSleeping(T* dynamic, bool enable_sleeping)
	{
#ifdef IS_PHYSX_3_2
		if(!enable_sleeping)
			dynamic->wakeUp(9999999999.0f);
#else
		dynamic->wakeUp();
		if(!enable_sleeping)
			dynamic->setWakeCounter(9999999999.0f);
#endif
	}

	inline_ void SetSceneFlag(PxSceneDesc& desc, PxSceneFlag::Enum flag, bool b)
	{
		if(b)
			desc.flags	|= flag;
		else
			desc.flags	&= ~flag;
	}

	class UICallback
	{
		public:
		virtual	void			UIModificationCallback()	= 0;
	};

	namespace PhysX3
	{
		IceWindow*				InitSharedGUI(IceWidget* parent, PintGUIHelper& helper, UICallback& callback);
		const EditableParams&	GetEditableParams();
		void					GetOptionsFromGUI(const char* test_name);
		void					GetOptionsFromOverride(PintOverride* overrideParams);
		void					CloseSharedGUI();
		bool					IsDebugVizEnabled();
	}

	inline_	void	SetupShape(SharedPhysX& physx, const EditableParams& params, const PINT_SHAPE_CREATE* create, PxShape& shape, PxU16 collision_group, bool debug_viz_flag)
	{
		if(create->mFlags & SHAPE_FLAGS_INACTIVE)
		{
			shape.setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape.setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
			//shape.setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, params.mSQFlag);
		}
		else
		{
			//shape.setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape.setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, params.mSQFlag);
		}

		shape.setFlag(PxShapeFlag::eVISUALIZATION, debug_viz_flag);
//		shape.setFlag(PxShapeFlag::eUSE_SWEPT_BOUNDS, gUseCCD);
		shape.setRestOffset(params.mRestOffset);
		shape.setContactOffset(params.mContactOffset);
//		const float contactOffset = shape.getContactOffset();	// 0.02
//		const float restOffset = shape.getRestOffset();		// 0.0
//		printf("contactOffset: %f\n", contactOffset);
//		printf("restOffset: %f\n", restOffset);

#if PHYSX_SUPPORT_TORSION_FRICTION
		shape.setTorsionalPatchRadius(params.mTorsionalPatchRadius);
		shape.setMinTorsionalPatchRadius(params.mMinTorsionalPatchRadius);
#endif
		// Setup query filter data so that we can filter out all shapes - debug purpose
		if(params.mSQFlag)
			shape.setQueryFilterData(PxFilterData(1, 0, 0, 0));

		if(create->mRenderer)
			shape.userData = create->mRenderer;

		physx.SetShapeName(&shape, create->mName);

//		PhysX3::SetGroup(shape, collision_group);
		PhysX3_SetGroup(shape, collision_group);
	}

#ifndef IS_PHYSX_3_2
	inline_ void SetupShape_(SharedPhysX& physx, const PINT_SHAPE_CREATE* create, PxShape* shape, const PxTransform& local_pose, PxU16 collision_group, const EditableParams& params)
	{
		if(shape)
		{
			shape->setLocalPose(local_pose);

			SetupShape(physx, params, create, *shape, collision_group, PhysX3::IsDebugVizEnabled());
		}
	}

	inline_ PxShape* CreateNonSharedShape(SharedPhysX& physx, const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group, const EditableParams& params)
	{
		ASSERT(actor);

#if PHYSX_SUPPORT_RIGIDACTOREX_CREATE_EXCLUSIVE_SHAPE
		PxShape* NewShape = PxRigidActorExt::createExclusiveShape(*actor, geometry, material);
#else
		PxShape* NewShape = actor->createShape(geometry, material);
#endif
		ASSERT(NewShape);

		SetupShape_(physx, create, NewShape, local_pose, collision_group, params);

		return NewShape;
	}

	inline_ PxShape* CreateSharedShape(SharedPhysX& physx, PxPhysics* physics, const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group, const EditableParams& params)
	{
		ASSERT(physics);
		ASSERT(actor);

		PxShape* NewShape = physics->createShape(geometry, material, false);
		ASSERT(NewShape);

		SetupShape_(physx, create, NewShape, local_pose, collision_group, params);

		actor->attachShape(*NewShape);

		//NewShape->release();

		return NewShape;
	}
#endif

#endif