///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_BEPU_H
#define PINT_BEPU_H

	#include "..\Pint.h"
	#include "..\PINT_Common\PINT_CommonShapes.h"

	class Bepu_SceneAPI : public Pint_Scene
	{
		public:
								Bepu_SceneAPI(Pint& pint);
		virtual					~Bepu_SceneAPI();

		virtual	bool			AddActors(udword nb_actors, const PintActorHandle* actors)	override;
	};

	class Bepu_ActorAPI : public Pint_Actor
	{
		public:
								Bepu_ActorAPI(Pint& pint);
		virtual					~Bepu_ActorAPI();

		virtual	const char*		GetName(PintActorHandle handle)							const	override;
		virtual	bool			SetName(PintActorHandle handle, const char* name)				override;
		virtual	bool			GetWorldBounds(PintActorHandle handle, AABB& bounds)	const	override;
		virtual	void			WakeUp(PintActorHandle handle)									override;
		virtual	float			GetLinearDamping(PintActorHandle handle)				const	override;
		virtual	bool			SetLinearDamping(PintActorHandle handle, float damping)			override;
		virtual	float			GetAngularDamping(PintActorHandle handle)				const	override;
		virtual	bool			SetAngularDamping(PintActorHandle handle, float damping)		override;
		virtual	bool			GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space)	const	override;
		virtual	bool			SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)	override;
		virtual	bool			GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space)const	override;
		virtual	bool			SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)	override;
		virtual	float			GetMass(PintActorHandle handle)							const	override;
		virtual	bool			GetLocalInertia(PintActorHandle handle, Point& inertia)	const	override;
	};

	class BepuPint : public Pint
	{
		public:
										BepuPint();
		virtual							~BepuPint();

		// Pint
		virtual	const char*				GetName()				const	override;
		virtual	const char*				GetUIName()				const	override;
		virtual	void					GetCaps(PintCaps& caps)	const	override;
		virtual	void					Init(const PINT_WORLD_CREATE& desc)	override;
		virtual	void					SetGravity(const Point& gravity)	override;
		virtual	void					Close()	override;
		virtual	udword					Update(float dt)	override;
		virtual	Point					GetMainColor()	override;
		virtual	void					Render(PintRender& renderer, PintRenderPass render_pass)	override;
		virtual	void					RenderDebugData(PintRender& renderer)	override;

		virtual	void					SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)		override;
		virtual	PintConvexHandle		CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index)	override;
		virtual	bool					DeleteConvexObject(PintConvexHandle handle, const PintConvexIndex* index)		override;
		virtual	PintMeshHandle			CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)	override;
		virtual bool					DeleteMeshObject(PintMeshHandle handle, const PintMeshIndex* index)		override;
		virtual	PintHeightfieldHandle	CreateHeightfieldObject(const PINT_HEIGHTFIELD_DATA_CREATE& desc, PintHeightfieldData& data, PintHeightfieldIndex* index=null);
		virtual	bool					DeleteHeightfieldObject(PintHeightfieldHandle handle=null, const PintHeightfieldIndex* index=null);
		virtual	PintActorHandle			CreateObject(const PINT_OBJECT_CREATE& desc)	override;
		virtual	bool					ReleaseObject(PintActorHandle handle)	override;
		virtual	PintJointHandle			CreateJoint(const PINT_JOINT_CREATE& desc)	override;
		virtual	bool					ReleaseJoint(PintJointHandle handle) override;

		virtual	bool					ResetSQFilters				()	{ return false;}

		//
		virtual	udword					BatchRaycasts				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)	override;
		virtual	udword					BatchRaycastAny				(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)	override;
		//
		virtual	udword					BatchBoxSweeps				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)		override;
		virtual	udword					BatchSphereSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)	override;
		virtual	udword					BatchCapsuleSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)	override;
		virtual	udword					BatchConvexSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)	override;
		//
		virtual	udword					BatchSphereOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)		override;
		virtual	udword					BatchSphereOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)	override;
		virtual	udword					BatchBoxOverlapAny			(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)			override;
		virtual	udword					BatchBoxOverlapObjects		(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)		override;
		virtual	udword					BatchCapsuleOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)		override;
		virtual	udword					BatchCapsuleOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)	override;
		virtual	udword					BatchConvexOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintConvexOverlapData* overlaps)		override;
		virtual	udword					BatchConvexOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintConvexOverlapData* overlaps)	override;
/*		//
		virtual	udword					FindTriangles_MeshSphereOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintSphereOverlapData* overlaps);
		virtual	udword					FindTriangles_MeshBoxOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintBoxOverlapData* overlaps);
		virtual	udword					FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintCapsuleOverlapData* overlaps);*/

		virtual	PR						GetWorldTransform(PintActorHandle handle)					override;
		virtual	void					SetWorldTransform(PintActorHandle handle, const PR& pose)	override;

		virtual	void					AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)	override;

		virtual	Point					GetLinearVelocity(PintActorHandle handle)	override;
		virtual	void					SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)	override;

		virtual	Point					GetAngularVelocity(PintActorHandle handle)	override;
		virtual	void					SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)	override;

		virtual	bool					SetKinematicPose(PintActorHandle handle, const Point& pos)	override;
		virtual	bool					SetKinematicPose(PintActorHandle handle, const PR& pr)		override;
		virtual	bool					IsKinematic(PintActorHandle handle)							override;
		virtual	bool					EnableKinematic(PintActorHandle handle, bool flag)			override;

		virtual	PintAggregateHandle		CreateAggregate(udword max_size, bool enable_self_collision)	override			{ return null;		}

		virtual	bool					SetDriveEnabled(PintJointHandle handle, bool flag)									override;
		virtual	bool					SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)	override;
		virtual	bool					SetDrivePosition(PintJointHandle handle, const PR& pose)							override;

		virtual	Pint_Scene*				GetSceneAPI()	override	{ return &mSceneAPI;	}
		virtual	Pint_Actor*				GetActorAPI()	override	{ return &mActorAPI;	}
//		virtual	Pint_Shape*				GetShapeAPI()	override	{ return &mShapeAPI;	}
//		virtual	Pint_Joint*				GetJointAPI()	override	{ return &mJointAPI;	}

		Bepu_SceneAPI					mSceneAPI;
		Bepu_ActorAPI					mActorAPI;
//		Bepu_ShapeAPI					mShapeAPI;
//		Bepu_JointAPI					mJointAPI;
	};

	IceWindow*	Bepu_InitGUI(IceWidget* parent, PintGUIHelper& helper);
	void		Bepu_CloseGUI();
	void		Bepu_Init(const PINT_WORLD_CREATE& desc);
	void		Bepu_Close();
	BepuPint*	GetBepu();

	DECLARE_PINT_EXPORTS

#endif