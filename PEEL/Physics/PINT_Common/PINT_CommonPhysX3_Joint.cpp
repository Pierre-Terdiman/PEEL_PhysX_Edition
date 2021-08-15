///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_Joint.h"
#include "PINT_CommonPhysX3.h"

PhysX_JointAPI::PhysX_JointAPI(Pint& pint) : Pint_Joint(pint)
{
}

PhysX_JointAPI::~PhysX_JointAPI()
{
}

const char* PhysX_JointAPI::GetName(PintJointHandle handle) const
{
	const PxJoint* Joint = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(Joint);
	return Joint->getName();
}

bool PhysX_JointAPI::SetName(PintJointHandle handle, const char* name)
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	physx.SetJointName(Joint, name);
	return true;
}

PintJoint PhysX_JointAPI::GetType(PintJointHandle handle) const
{
	const PxJoint* Joint = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(Joint);

	switch(Joint->getConcreteType())
	{
#ifdef IS_PHYSX_3_2
		case PxConcreteType::eUSER_SPHERICAL_JOINT:		return PINT_JOINT_SPHERICAL;
		case PxConcreteType::eUSER_REVOLUTE_JOINT:		return PINT_JOINT_HINGE;
		case PxConcreteType::eUSER_PRISMATIC_JOINT:		return PINT_JOINT_PRISMATIC;
		case PxConcreteType::eUSER_FIXED_JOINT:			return PINT_JOINT_FIXED;
		case PxConcreteType::eUSER_DISTANCE_JOINT:		return PINT_JOINT_DISTANCE;
		case PxConcreteType::eUSER_D6_JOINT:			return PINT_JOINT_D6;
//		case PxConcreteType::eCONTACT:					return PINT_JOINT_SPHERICAL;
#else
		case PxJointConcreteType::eSPHERICAL:	return PINT_JOINT_SPHERICAL;
		case PxJointConcreteType::eREVOLUTE:	return PINT_JOINT_HINGE;
		case PxJointConcreteType::ePRISMATIC:	return PINT_JOINT_PRISMATIC;
		case PxJointConcreteType::eFIXED:		return PINT_JOINT_FIXED;
		case PxJointConcreteType::eDISTANCE:	return PINT_JOINT_DISTANCE;
		case PxJointConcreteType::eD6:			return PINT_JOINT_D6;
//		case PxJointConcreteType::eCONTACT:		return PINT_JOINT_SPHERICAL;
#endif
	}
	return PINT_JOINT_UNDEFINED;
}

bool PhysX_JointAPI::GetActors(PintJointHandle handle, PintActorHandle& actor0, PintActorHandle& actor1) const
{
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	ASSERT(Joint);

	PxRigidActor* pxActor0;
	PxRigidActor* pxActor1;
	Joint->getActors(pxActor0, pxActor1);

	actor0 = PintActorHandle(pxActor0);
	actor1 = PintActorHandle(pxActor1);
	return true;
}

bool PhysX_JointAPI::GetFrames(PintJointHandle handle, PR& frame0, PR& frame1) const
{
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	ASSERT(Joint);

	const PxTransform Pose0 = Joint->getLocalPose(PxJointActorIndex::eACTOR0);
	const PxTransform Pose1 = Joint->getLocalPose(PxJointActorIndex::eACTOR1);

	frame0 = ToPR(Pose0);
	frame1 = ToPR(Pose1);
	return true;
}
