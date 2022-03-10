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

		virtual	const char*		GetName(PintActorHandle handle)					const	override;
		virtual	bool			SetName(PintActorHandle handle, const char* name)		override;

		virtual	udword			GetNbShapes(PintActorHandle handle)				const	override;
		virtual	PintShapeHandle	GetShape(PintActorHandle handle, udword index)	const	override;

		virtual	udword			GetNbJoints(PintActorHandle handle)				const	override;
		virtual	PintJointHandle	GetJoint(PintActorHandle handle, udword index)	const	override;

		virtual	bool			GetWorldBounds(PintActorHandle handle, AABB& bounds)	const	override;

		virtual	void			WakeUp(PintActorHandle handle)	override;

		virtual	bool			SetGravityFlag(PintActorHandle handle, bool flag)		override;
		virtual	bool			SetDebugVizFlag(PintActorHandle handle, bool flag)		override;
		virtual	bool			SetSimulationFlag(PintActorHandle handle, bool flag)	override;

		virtual	float			GetLinearDamping(PintActorHandle handle)		const	override;
		virtual	bool			SetLinearDamping(PintActorHandle handle, float damping)	override;

		virtual	float			GetAngularDamping(PintActorHandle handle)			const	override;
		virtual	bool			SetAngularDamping(PintActorHandle handle, float damping)	override;

		virtual	bool			GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space)	const	override;
		virtual	bool			SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)	override;

		virtual	bool			GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space)	const	override;
		virtual	bool			SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)		override;

		virtual	float			GetMass(PintActorHandle handle)	const			override;
		virtual	bool			SetMass(PintActorHandle handle, float mass)		override;

		virtual	bool			GetLocalInertia(PintActorHandle handle, Point& inertia)	const	override;
		virtual	bool			SetLocalInertia(PintActorHandle handle, const Point& inertia)	override;

		virtual	bool			GetCMassLocalPose(PintActorHandle handle, PR& pose)		const	override;
		virtual	bool			SetCMassLocalPose(PintActorHandle handle, const PR& pose)		override;
	};

#endif