///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_Actor.h"
#include "PINT_CommonPhysX3.h"

PhysX_ActorAPI::PhysX_ActorAPI(Pint& pint) : Pint_Actor(pint)
{
}

PhysX_ActorAPI::~PhysX_ActorAPI()
{
}

const char* PhysX_ActorAPI::GetName(PintActorHandle handle) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	ASSERT(Actor);
	return Actor->getName();
}

bool PhysX_ActorAPI::SetName(PintActorHandle handle, const char* name)
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);
	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(handle);
	physx.SetActorName(Actor, name);
	return true;
}

udword PhysX_ActorAPI::GetNbShapes(PintActorHandle handle) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	ASSERT(Actor);
	return Actor->getNbShapes();
}

PintShapeHandle PhysX_ActorAPI::GetShape(PintActorHandle handle, udword index) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	ASSERT(Actor);
	PxShape* shape = null;
	PxU32 nb = Actor->getShapes(&shape, 1, index);
	ASSERT(nb==1);
	return PintShapeHandle(shape);
}

udword PhysX_ActorAPI::GetNbJoints(PintActorHandle handle) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	ASSERT(Actor);
	return Actor->getNbConstraints();
}

PintJointHandle PhysX_ActorAPI::GetJoint(PintActorHandle handle, udword index) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	ASSERT(Actor);
	PxConstraint* constraint = null;
	PxU32 nb = Actor->getConstraints(&constraint, 1, index);
	ASSERT(nb==1);

	PxU32 typeID;
	PxJoint* j = reinterpret_cast<PxJoint*>(constraint->getExternalReference(typeID));
	ASSERT(typeID==PxConstraintExtIDs::eJOINT);
	return PintJointHandle(j);
}

bool PhysX_ActorAPI::GetWorldBounds(PintActorHandle handle, AABB& bounds) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	ASSERT(Actor);
	const PxBounds3 pxBounds = Actor->getWorldBounds();
	bounds.mMin = ToPoint(pxBounds.minimum);
	bounds.mMax = ToPoint(pxBounds.maximum);
	return true;
}

void PhysX_ActorAPI::WakeUp(PintActorHandle handle)
{
	PxRigidActor* RigidActor = reinterpret_cast<PxRigidActor*>(handle);

	if(RigidActor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
	{
		PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(RigidActor);
		Dyna->wakeUp();
	}
	else if(RigidActor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
	{
		PxArticulationLink* Link = static_cast<PxArticulationLink*>(RigidActor);
		Link->getArticulation().wakeUp();
	}
}

static bool SetFlag(PintActorHandle handle, PxActorFlag::Enum actorFlag, bool flag)
{
	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(handle);
	if(!Actor)
		return false;

	if(Actor->getConcreteType()==PxConcreteType::eRIGID_STATIC)
		static_cast<PxRigidStatic*>(Actor)->setActorFlag(actorFlag, flag);
	else if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		static_cast<PxRigidDynamic*>(Actor)->setActorFlag(actorFlag, flag);
	else if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		static_cast<PxArticulationLink*>(Actor)->setActorFlag(actorFlag, flag);
	else
		PX_ASSERT(0);
	return true;
}

bool PhysX_ActorAPI::SetGravityFlag(PintActorHandle handle, bool flag)
{
	return SetFlag(handle, PxActorFlag::eDISABLE_GRAVITY, !flag);
}

bool PhysX_ActorAPI::SetDebugVizFlag(PintActorHandle handle, bool flag)
{
	return SetFlag(handle, PxActorFlag::eVISUALIZATION, flag);
}

bool PhysX_ActorAPI::SetSimulationFlag(PintActorHandle handle, bool flag)
{
#ifdef IS_PHYSX_3_2
	return false;
#else
	return SetFlag(handle, PxActorFlag::eDISABLE_SIMULATION, !flag);
#endif
}

float PhysX_ActorAPI::GetLinearDamping(PintActorHandle handle) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	if(Actor)
	{
		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
			return static_cast<const PxRigidDynamic*>(Actor)->getLinearDamping();

#if PHYSX_SUPPORT_DAMPING_ON_ARTICULATION_LINKS
		if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
			return static_cast<const PxArticulationLink*>(Actor)->getLinearDamping();
#endif
	}
	return 0.0f;
}

bool PhysX_ActorAPI::SetLinearDamping(PintActorHandle handle, float damping)
{
	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(handle);
	if(Actor)
	{
		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		{
			static_cast<PxRigidDynamic*>(Actor)->setLinearDamping(damping);
			return true;
		}

#if PHYSX_SUPPORT_DAMPING_ON_ARTICULATION_LINKS
		if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		{
			static_cast<PxArticulationLink*>(Actor)->setLinearDamping(damping);
			return true;
		}
#endif
	}
	return false;
}

float PhysX_ActorAPI::GetAngularDamping(PintActorHandle handle) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	if(Actor)
	{
		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
			return static_cast<const PxRigidDynamic*>(Actor)->getAngularDamping();

#if PHYSX_SUPPORT_DAMPING_ON_ARTICULATION_LINKS
		if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
			return static_cast<const PxArticulationLink*>(Actor)->getAngularDamping();
#endif
	}
	return 0.0f;
}

bool PhysX_ActorAPI::SetAngularDamping(PintActorHandle handle, float damping)
{
	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(handle);
	if(Actor)
	{
		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		{
			static_cast<PxRigidDynamic*>(Actor)->setAngularDamping(damping);
			return true;
		}

#if PHYSX_SUPPORT_DAMPING_ON_ARTICULATION_LINKS
		if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		{
			static_cast<PxArticulationLink*>(Actor)->setAngularDamping(damping);
			return true;
		}
#endif
	}
	return false;
}

bool PhysX_ActorAPI::GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space) const
{
	ASSERT(world_space);
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	if(Actor)
	{
		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		{
			linear_velocity = ToPoint(static_cast<const PxRigidDynamic*>(Actor)->getLinearVelocity());
			return true;
		}

		if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		{
			linear_velocity = ToPoint(static_cast<const PxArticulationLink*>(Actor)->getLinearVelocity());
			return true;
		}
	}
	return false;
}

bool PhysX_ActorAPI::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)
{
	ASSERT(world_space);
	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(handle);
	if(Actor)
	{
		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		{
			static_cast<PxRigidDynamic*>(Actor)->setLinearVelocity(ToPxVec3(linear_velocity));
			return true;
		}

#if PHYSX_SUPPORT_ARTICULATIONS
		if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		{
			static_cast<PxArticulationLink*>(Actor)->setLinearVelocity(ToPxVec3(linear_velocity));
			return true;
		}
#endif
	}
	return false;
}

bool PhysX_ActorAPI::GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space) const
{
	ASSERT(world_space);
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(handle);
	if(Actor)
	{
		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		{
			angular_velocity = ToPoint(static_cast<const PxRigidDynamic*>(Actor)->getAngularVelocity());
			return true;
		}

		if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		{
			angular_velocity = ToPoint(static_cast<const PxArticulationLink*>(Actor)->getAngularVelocity());
			return true;
		}
	}
	return false;
}

bool PhysX_ActorAPI::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)
{
	ASSERT(world_space);
	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(handle);
	if(Actor)
	{
		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		{
			static_cast<PxRigidDynamic*>(Actor)->setAngularVelocity(ToPxVec3(angular_velocity));
			return true;
		}

#if PHYSX_SUPPORT_ARTICULATIONS
		if(Actor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		{
			static_cast<PxArticulationLink*>(Actor)->setAngularVelocity(ToPxVec3(angular_velocity));
			return true;
		}
#endif
	}
	return false;
}

float PhysX_ActorAPI::GetMass(PintActorHandle handle) const
{
	const PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
		return 0.0f;
	return RigidBody->getMass();
}

bool PhysX_ActorAPI::SetMass(PintActorHandle handle, float mass)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
		return false;
	RigidBody->setMass(mass);
	return true;
}

bool PhysX_ActorAPI::GetLocalInertia(PintActorHandle handle, Point& inertia) const
{
	const PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
	{
		inertia.Zero();
		return false;
	}
	inertia = ToPoint(RigidBody->getMassSpaceInertiaTensor());
	return true;
}

bool PhysX_ActorAPI::SetLocalInertia(PintActorHandle handle, const Point& inertia)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
		return false;
	RigidBody->setMassSpaceInertiaTensor(ToPxVec3(inertia));
	return true;
}

bool PhysX_ActorAPI::GetCMassLocalPose(PintActorHandle handle, PR& pose) const
{
	const PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
	{
		pose.Identity();
		return false;
	}
	pose = ToPR(RigidBody->getCMassLocalPose());
	return true;
}

bool PhysX_ActorAPI::SetCMassLocalPose(PintActorHandle handle, const PR& pose)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
		return false;
	RigidBody->setCMassLocalPose(ToPxTransform(pose));
	return true;
}

