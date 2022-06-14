///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_JOLT_H
#define PINT_JOLT_H

	#include "..\Pint.h"
	#include "..\PINT_Common\PINT_CommonShapes.h"
	#include <map>

	// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
	// You can use Jolt.h in your precompiled header to speed up compilation.
	#include <Jolt/Jolt.h>

	#include <Jolt/Physics/Collision/Shape/Shape.h>

	using namespace JPH;

	class Jolt_SceneAPI : public Pint_Scene
	{
		public:
								Jolt_SceneAPI(Pint& pint);
		virtual					~Jolt_SceneAPI();

		virtual	bool			AddActors(udword nb_actors, const PintActorHandle* actors)	override;
	};

	class Jolt_ActorAPI : public Pint_Actor
	{
		public:
								Jolt_ActorAPI(Pint& pint);
		virtual					~Jolt_ActorAPI();

		virtual	const char*		GetName(PintActorHandle handle)					const	override;
		virtual	bool			SetName(PintActorHandle handle, const char* name)		override;

/*		virtual	udword			GetNbShapes(PintActorHandle handle)				const	override;
		virtual	PintShapeHandle	GetShape(PintActorHandle handle, udword index)	const	override;

		virtual	udword			GetNbJoints(PintActorHandle handle)				const	override;
		virtual	PintJointHandle	GetJoint(PintActorHandle handle, udword index)	const	override;*/

		virtual	bool			GetWorldBounds(PintActorHandle handle, AABB& bounds)	const	override;

/*		virtual	void			WakeUp(PintActorHandle handle)	override;

		virtual	bool			SetGravityFlag(PintActorHandle handle, bool flag)		override;
		virtual	bool			SetDebugVizFlag(PintActorHandle handle, bool flag)		override;
		virtual	bool			SetSimulationFlag(PintActorHandle handle, bool flag)	override;*/

		virtual	float			GetLinearDamping(PintActorHandle handle)		const	override;
		virtual	bool			SetLinearDamping(PintActorHandle handle, float damping)	override;

		virtual	float			GetAngularDamping(PintActorHandle handle)			const	override;
		virtual	bool			SetAngularDamping(PintActorHandle handle, float damping)	override;

		virtual	bool			GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space)	const	override;
		virtual	bool			SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)	override;

		virtual	bool			GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space)	const	override;
		virtual	bool			SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)		override;

		virtual	float			GetMass(PintActorHandle handle)	const			override;
/*		virtual	bool			SetMass(PintActorHandle handle, float mass)		override;*/

		virtual	bool			GetLocalInertia(PintActorHandle handle, Point& inertia)	const	override;
/*		virtual	bool			SetLocalInertia(PintActorHandle handle, const Point& inertia)	override;

		virtual	bool			GetCMassLocalPose(PintActorHandle handle, PR& pose)		const	override;
		virtual	bool			SetCMassLocalPose(PintActorHandle handle, const PR& pose)		override;*/
	};

	class JoltPint : public Pint
	{
		public:
										JoltPint();
		virtual							~JoltPint();

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
		virtual	PintMeshHandle			CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)	override;
		virtual bool					DeleteMeshObject(PintMeshHandle handle, const PintMeshIndex* index)		override;
		virtual	PintHeightfieldHandle	CreateHeightfieldObject(const PINT_HEIGHTFIELD_DATA_CREATE& desc, PintHeightfieldData& data, PintHeightfieldIndex* index=null);
		virtual	bool					DeleteHeightfieldObject(PintHeightfieldHandle handle=null, const PintHeightfieldIndex* index=null);
		Ref<JPH::Shape>					CreateShape(const PINT_SHAPE_CREATE* shape_create);
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
//		virtual	udword					BatchConvexSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)	override;
		//
		virtual	udword					BatchSphereOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)		override;
		virtual	udword					BatchSphereOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)	override;
/*		virtual	udword					BatchBoxOverlapAny			(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps);
		virtual	udword					BatchBoxOverlapObjects		(PintSQThreadContext context, udword nb, PintOverlapObjectHit* dest, const PintBoxOverlapData* overlaps);
		virtual	udword					BatchCapsuleOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps);
		virtual	udword					BatchCapsuleOverlapObjects	(PintSQThreadContext context, udword nb, PintOverlapObjectHit* dest, const PintCapsuleOverlapData* overlaps);
		//
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

		Jolt_SceneAPI					mSceneAPI;
		Jolt_ActorAPI					mActorAPI;
//		Jolt_ShapeAPI					mShapeAPI;
//		Jolt_JointAPI					mJointAPI;

		struct CachedShape
		{
			PintShapeRenderer*			mRenderer;
			PR							mLocalPose;
			Ref<JPH::Shape>				mShape;
		};

		vector<CachedShape>				mCachedShapes;
	};

	IceWindow*	Jolt_InitGUI(IceWidget* parent, PintGUIHelper& helper);
	void		Jolt_CloseGUI();
	void		Jolt_Init(const PINT_WORLD_CREATE& desc);
	void		Jolt_Close();
	JoltPint*	GetJolt();

	extern "C"	__declspec(dllexport)	PintPlugin*	GetPintPlugin();

#endif