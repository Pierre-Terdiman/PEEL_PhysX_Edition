///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX4_IMM_PINT_H
#define PINT_COMMON_PHYSX4_IMM_PINT_H

#include "..\Pint.h"
#include "PINT_CommonPhysX3_Mesh.h"

	class ImmediateScene;

	class PhysXImmActor : public Pint_Actor
	{
		public:
								PhysXImmActor(Pint& pint) : Pint_Actor(pint)	{}
		virtual					~PhysXImmActor()								{}

		virtual	bool			GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space)	const;
		virtual	bool			SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space);

		virtual	bool			GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space)	const;
		virtual	bool			SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space);

		virtual	float			GetMass(PintActorHandle handle)	const;
	};

	class PhysXImm : public Pint, public MeshManager, public PintShapeEnumerateCallback
	{
		public:
								PhysXImm();
		virtual					~PhysXImm();

		// Pint
		virtual	const char*		GetName()				const;
		virtual	void			GetCaps(PintCaps& caps)	const;
//		virtual	udword			GetFlags()				const	{ return PINT_IS_ACTIVE;	}
		virtual	void			Init(const PINT_WORLD_CREATE& desc);
		virtual	void			SetGravity(const Point& gravity);
		virtual	void			Close();
		virtual	udword			Update(float dt);
		virtual	Point			GetMainColor();
		virtual	void			Render(PintRender& renderer, PintRenderPass render_pass);
		virtual	void			RenderDebugData(PintRender& renderer);
		virtual	void			SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups);
		virtual	PintActorHandle	CreateObject(const PINT_OBJECT_CREATE& desc);
		virtual	bool			ReleaseObject(PintActorHandle handle);
		virtual	PintJointHandle	CreateJoint(const PINT_JOINT_CREATE& desc);

		virtual	PR				GetWorldTransform(PintActorHandle handle);
		virtual	void			SetWorldTransform(PintActorHandle handle, const PR& pose);

		virtual	void			AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos);

		virtual	udword			BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts);

		virtual	PintArticHandle	CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE&);
		virtual	PintActorHandle	CreateRCArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_RC_ARTICULATED_BODY_CREATE&, PintArticHandle articulation);
		virtual	bool			AddRCArticulationToScene(PintArticHandle articulation);
//		virtual	bool			AddRCArticulationToAggregate(PintArticHandle articulation, PintObjectHandle aggregate)									{ NotImplemented("AddRCArticulationToAggregate");	return false;	}
//		virtual	bool			SetRCADriveEnabled(PintObjectHandle handle, bool flag)																	{ NotImplemented("SetRCADriveEnabled");				return false;	}
//		virtual	bool			SetRCADriveVelocity(PintObjectHandle handle, float velocity)															{ NotImplemented("SetRCADriveVelocity");			return false;	}
		//~Pint

		virtual	Pint_Actor*		GetActorAPI()	{ return &mActorAPI;	}

		// MeshManager
		virtual	PxConvexMesh*	CreatePhysXConvex(udword nb_verts, const Point* verts, PxConvexFlags flags);
		virtual	PxTriangleMesh*	CreatePhysXMesh(const PintSurfaceInterface& surface, bool deformable);
		//~MeshManager

		// PintShapeEnumerateCallback
		virtual	void			ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)	override;
		//~PintShapeEnumerateCallback

				PhysXImmActor	mActorAPI;
				ImmediateScene*	mScene;
				PxFoundation*	mFoundation;
				PxCooking*		mCooking;
#ifdef IMM_NEEDS_PX_PHYSICS
				PxPhysics*		mPhysics;
#endif

		private:
	};

#endif
