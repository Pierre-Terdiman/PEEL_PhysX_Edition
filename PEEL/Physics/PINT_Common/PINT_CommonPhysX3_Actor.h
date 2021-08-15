///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_ACTOR_H
#define PINT_COMMON_PHYSX3_ACTOR_H

#include "..\Pint.h"

	class PhysX_ActorAPI : public Pint_Actor
	{
		public:
								PhysX_ActorAPI(Pint& pint);
		virtual					~PhysX_ActorAPI();

		virtual	const char*		GetName(PintActorHandle handle)					const;
		virtual	bool			SetName(PintActorHandle handle, const char* name);

		virtual	udword			GetNbShapes(PintActorHandle handle)				const;
		virtual	PintShapeHandle	GetShape(PintActorHandle handle, udword index)	const;

		virtual	udword			GetNbJoints(PintActorHandle handle)				const;
		virtual	PintJointHandle	GetJoint(PintActorHandle handle, udword index)	const;

		virtual	bool			GetWorldBounds(PintActorHandle handle, AABB& bounds)	const;

		virtual	void			WakeUp(PintActorHandle handle);

		virtual	bool			SetGravityFlag(PintActorHandle handle, bool flag);
		virtual	bool			SetDebugVizFlag(PintActorHandle handle, bool flag);
		virtual	bool			SetSimulationFlag(PintActorHandle handle, bool flag);

		virtual	float			GetLinearDamping(PintActorHandle handle)		const;
		virtual	bool			SetLinearDamping(PintActorHandle handle, float damping);

		virtual	float			GetAngularDamping(PintActorHandle handle)		const;
		virtual	bool			SetAngularDamping(PintActorHandle handle, float damping);

		virtual	bool			GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space)	const;
		virtual	bool			SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space);

		virtual	bool			GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space)	const;
		virtual	bool			SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space);

		virtual	float			GetMass(PintActorHandle handle)	const;
		virtual	bool			SetMass(PintActorHandle handle, float mass);

		virtual	bool			GetLocalInertia(PintActorHandle handle, Point& inertia)	const;
		virtual	bool			SetLocalInertia(PintActorHandle handle, const Point& inertia);
	};

#endif