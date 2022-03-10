///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_JOINT_H
#define PINT_COMMON_PHYSX3_JOINT_H

#include "..\Pint.h"

	class PhysX_JointAPI : public Pint_Joint
	{
		public:
							PhysX_JointAPI(Pint& pint);
		virtual				~PhysX_JointAPI();

		virtual	const char*	GetName(PintJointHandle handle)														const	override;
		virtual	bool		SetName(PintJointHandle handle, const char* name)											override;

		virtual	PintJoint	GetType(PintJointHandle handle)														const	override;

		virtual	bool		GetActors(PintJointHandle handle, PintActorHandle& actor0, PintActorHandle& actor1)	const	override;
		virtual	bool		SetActors(PintJointHandle handle, PintActorHandle actor0, PintActorHandle actor1)			override;

		virtual	bool		GetFrames(PintJointHandle handle, PR* frame0, PR* frame1)							const	override;
		virtual	bool		SetFrames(PintJointHandle handle, const PR* frame0, const PR* frame1)						override;

		virtual	bool		GetLimits(PintJointHandle handle, PintLimits& limits, udword index)					const	override;
		virtual	bool		SetLimits(PintJointHandle handle, const PintLimits& limits, udword index)					override;

		virtual	bool		GetSpring(PintJointHandle handle, PintSpring& spring)								const	override;
		virtual	bool		SetSpring(PintJointHandle handle, const PintSpring& spring)									override;

#if PHYSX_SUPPORT_GEAR_JOINT
		virtual	bool		GetGearRatio(PintJointHandle handle, float& ratio)									const	override;
		virtual	bool		SetGearRatio(PintJointHandle handle, float ratio)											override;
#endif

		virtual	bool		GetHingeDynamicData(PintJointHandle handle, PintHingeDynamicData& data)				const	override;
		virtual	bool		GetD6DynamicData(PintJointHandle handle, PintD6DynamicData& data)					const	override;
	};

	void normalToTangents(const PxVec3& n, PxVec3& t1, PxVec3& t2);
	PxQuat ComputeJointQuat(PxRigidActor* actor, const PxVec3& localAxis);
	void SetupD6Projection(PxD6Joint* j, bool enable_projection, float projection_linear_tolerance, float projection_angular_tolerance);
	struct EditableParams;
	float GetJointContactDistance(const EditableParams& params);

	PxSphericalJoint*	CreateSphericalJoint(const EditableParams& params, PxPhysics& physics, const PINT_SPHERICAL_JOINT_CREATE& jc,	PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1);
	PxRevoluteJoint*	CreateRevoluteJoint2(const EditableParams& params, PxPhysics& physics, const PINT_HINGE2_JOINT_CREATE& jc,		PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1);
	PxFixedJoint*		CreateFixedJoint	(const EditableParams& params, PxPhysics& physics, const PINT_FIXED_JOINT_CREATE& jc,		PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1);
	PxPrismaticJoint*	CreatePrismaticJoint(const EditableParams& params, PxPhysics& physics, const PINT_PRISMATIC_JOINT_CREATE& jc,	PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1);
	PxDistanceJoint*	CreateDistanceJoint	(const EditableParams& params, PxPhysics& physics, const PINT_DISTANCE_JOINT_CREATE& jc,	PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1);

#endif