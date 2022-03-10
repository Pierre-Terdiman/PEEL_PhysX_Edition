///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_EDITOR_H
#define PINT_EDITOR_H

#include ".\Pint.h"
#include ".\ZCB\PINT_ZCB2.h"
#include ".\PhysicsData.h"

	// A built-in "plugin" to provide basic editing features

	class Stats
	{
		public:
										Stats();

				struct Shared
				{
					inline_	Shared() : mNbUnique(0), mNbRef(0)	{}
					udword	mNbUnique;
					udword	mNbRef;
				};

				udword					mNbStatics;
				udword					mNbDynamics;
				udword					mNbStaticCompounds;
				udword					mNbDynamicCompounds;

				udword					mNbJoints;
				udword					mNbSphericalJoints;
				udword					mNbHingeJoints;
				udword					mNbPrismaticJoints;
				udword					mNbFixedJoints;
				udword					mNbDistanceJoints;
				udword					mNbD6Joints;
				udword					mNbGearJoints;
				udword					mNbRackJoints;
				udword					mNbPortalJoints;

				Shared					mSphereShapes;
				Shared					mCapsuleShapes;
				Shared					mCylinderShapes;
				Shared					mBoxShapes;
				Shared					mConvexShapes;
				Shared					mMeshShapes;

				udword					mTotalNbVerts;
				udword					mTotalNbTris;

				Shared					mMaterials;
				udword					mNbAggregates;
				udword					mNbArticulations;
				udword					mNbRCArticulations;
				udword					mNbVehicles;
				udword					mNbCharacters;
	};

	// We capture the stats used when creating objects here, and we'll use them to
	// update the stats on releasing objects. We could just read the data from the
	// native handles instead, but if that data has changed and the stats haven't been
	// properly updated there might be a mismatch between the two. These captured stats
	// here help identify & debug these issues.
	class EditorObject : public Allocateable
	{
		public:

		enum Type
		{
			UNDEFINED,
			ACTOR,
			SHAPE,
			JOINT,
			MESH,
			MATERIAL,
			AGGREGATE,
			CHARACTER,
			VEHICLE,
		};

						EditorObject(Type type);
		virtual			~EditorObject()	{}

		inline_	Type	GetType()	const	{ return mEditorType;	}

		private:
				Type	mEditorType;
		public:
				void*	mNativeHandle;
				udword	mRefCount;
				bool	mExportable;
	};

	class EditorActor : public EditorObject
	{
		public:
					EditorActor();
		virtual		~EditorActor();

		Point		mInitPos;
		Quat		mInitRot;
		bool		mIsStatic;
		bool		mIsCompound;
	};

	class EditorShape : public EditorObject
	{
		public:
					EditorShape();
		virtual		~EditorShape();

		PintShape	mShapeType;
		udword		mNbTris;	// For meshes
		udword		mNbVerts;	// For meshes
	};

	class EditorJoint : public EditorObject
	{
		public:
					EditorJoint();
		virtual		~EditorJoint();
	};

	class EditorMesh : public EditorObject
	{
		public:
					EditorMesh();
		virtual		~EditorMesh();
	};

	class EditorMaterial : public EditorObject
	{
		public:
					EditorMaterial();
		virtual		~EditorMaterial();
	};

	class EditorAggregate : public EditorObject
	{
		public:
					EditorAggregate();
		virtual		~EditorAggregate();
	};

	class EditorCharacter : public EditorObject
	{
		public:
					EditorCharacter();
		virtual		~EditorCharacter();

		PintActorHandle	dummy;
	};

	class EditorVehicle : public EditorObject
	{
		public:
					EditorVehicle();
		virtual		~EditorVehicle();

		PintActorHandle	dummy;
	};

	inline_ void* GetNativeHandle(const BaseData* data)
	{
		if(!data)
			return null;
		return data->mUserData ? reinterpret_cast<EditorObject*>(data->mUserData)->mNativeHandle : null;
	}

	inline_ PintActorHandle		GetNativeActorHandle(const BaseData* data)		{ return PintActorHandle(GetNativeHandle(data));		}
	inline_ PintShapeHandle		GetNativeShapeHandle(const BaseData* data)		{ return PintShapeHandle(GetNativeHandle(data));		}
	inline_ PintJointHandle		GetNativeJointHandle(const BaseData* data)		{ return PintJointHandle(GetNativeHandle(data));		}
	inline_ PintVehicleHandle	GetNativeVehicleHandle(const BaseData* data)	{ return PintVehicleHandle(GetNativeHandle(data));		}
	inline_ PintCharacterHandle	GetNativeCharacterHandle(const BaseData* data)	{ return PintCharacterHandle(GetNativeHandle(data));	}

	inline_ ActorData*			HandleToActorData(void* handle)			{ return reinterpret_cast<ActorData*>(handle);			}
	inline_ const ActorData*	HandleToActorData(const void* handle)	{ return reinterpret_cast<const ActorData*>(handle);	}
	inline_ ShapeData*			HandleToShapeData(void* handle)			{ return reinterpret_cast<ShapeData*>(handle);			}
	inline_ const ShapeData*	HandleToShapeData(const void* handle)	{ return reinterpret_cast<const ShapeData*>(handle);	}
	inline_ JointData*			HandleToJointData(void* handle)			{ return reinterpret_cast<JointData*>(handle);			}
	inline_ const JointData*	HandleToJointData(const void* handle)	{ return reinterpret_cast<const JointData*>(handle);	}
	inline_ VehicleData*		HandleToVehicleData(void* handle)		{ return reinterpret_cast<VehicleData*>(handle);		}
	inline_ CharacterData*		HandleToCharacterData(void* handle)		{ return reinterpret_cast<CharacterData*>(handle);		}

	struct MergeParams
	{
		MergeParams(const char* name=null, float mass=-1.0f, const PR* pose=null/*, bool kinematic=false*/) : mName(name), mMass(mass), mPose(pose)//, mKinematic(kinematic)
		{
		}

		const char*		mName;
		float			mMass;
		const PR*		mPose;
//		bool			mKinematic;
	};

	class MergeInterface
	{
		public:
		virtual	PINT_SHAPE_CREATE*	CreateShape(const ShapeData* shape_data)	= 0;
	};

	class EditorSceneAPI : public Pint_Scene
	{
		public:
						EditorSceneAPI(Pint& pint)	: Pint_Scene(pint)	{}

//		virtual	bool	SetGravity(const Point& gravity);
//		virtual	Point	GetGravity()						const;

		virtual	bool	AddActors(udword nb_actors, const PintActorHandle* actors)				override;

		virtual	void	GetActors(Reporter& reporter)									const	override;
		virtual	void	Cull(udword nb_planes, const Plane* planes, Reporter& reporter)	const	override;
	};

	class EditorActorAPI : public Pint_Actor
	{
		public:
								EditorActorAPI(Pint& pint)	: Pint_Actor(pint)	{}

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

		virtual	float			GetAngularDamping(PintActorHandle handle)		const		override;
		virtual	bool			SetAngularDamping(PintActorHandle handle, float damping)	override;

		virtual	bool			GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space)	const;
		virtual	bool			SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space);

		virtual	bool			GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space)	const;
		virtual	bool			SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space);

		virtual	float			GetMass(PintActorHandle handle)		const	override;
		virtual	bool			SetMass(PintActorHandle handle, float mass)	override;

		virtual	bool			GetLocalInertia(PintActorHandle handle, Point& inertia)	const	override;
		virtual	bool			SetLocalInertia(PintActorHandle handle, const Point& inertia)	override;

		virtual	bool			GetCMassLocalPose(PintActorHandle handle, PR& pose)		const	override;
		virtual	bool			SetCMassLocalPose(PintActorHandle handle, const PR& pose)		override;
	};

	class EditorShapeAPI : public Pint_Shape
	{
		public:
									EditorShapeAPI(Pint& pint)	: Pint_Shape(pint)	{}

		virtual	const char*			GetName(PintShapeHandle handle)				const	override;
		virtual	bool				SetName(PintShapeHandle handle, const char* name)	override;

		virtual	PintShape			GetType(PintShapeHandle handle)				const	override;

		virtual	PR					GetWorldTransform(PintActorHandle actor, PintShapeHandle shape)				const	override;

		virtual	bool				GetWorldBounds(PintActorHandle actor, PintShapeHandle shape, AABB& bounds)	const	override;

		// Mesh-related, temp design/location
//		virtual	bool				GetTriangleMeshData(SurfaceInterface& surface, PintShapeHandle handle, bool)	const	{ return mPint.NotImplemented("Pint_Shape::GetTriangleMeshData");	}
		virtual	bool				GetTriangle(Triangle& tri, PintShapeHandle handle, udword index)				const	override;
		virtual	bool				GetIndexedTriangle(IndexedTriangle& tri, PintShapeHandle handle, udword index)	const	override;
//		virtual	bool				FindTouchedTriangles(Container& indices, PintSQThreadContext context, PintShapeHandle handle, const PR& pose, const PintSphereOverlapData& sphere)	const	{ return mPint.NotImplemented("Pint_Shape::FindTouchedTriangles");	}
//		virtual	bool				Refit(PintShapeHandle shape, PintActorHandle actor)	{ return mPint.NotImplemented("Pint_Shape::Refit");	}

		// Experimental / questionable
		virtual	PintShapeRenderer*	GetShapeRenderer(PintShapeHandle handle)	const	override;
	};

	class EditorJointAPI : public Pint_Joint
	{
		public:
							EditorJointAPI(Pint& pint)	: Pint_Joint(pint)	{}

		virtual	const char*	GetName(PintJointHandle handle)				const	override;
		virtual	bool		SetName(PintJointHandle handle, const char* name)	override;

		virtual	PintJoint	GetType(PintJointHandle handle)				const	override;

		virtual	bool		GetActors(PintJointHandle handle, PintActorHandle& actor0, PintActorHandle& actor1)	const	override;
		virtual	bool		SetActors(PintJointHandle handle, PintActorHandle actor0, PintActorHandle actor1)			override;

		virtual	bool		GetFrames(PintJointHandle handle, PR* frame0, PR* frame1)		const	override;
		virtual	bool		SetFrames(PintJointHandle handle, const PR* frame0, const PR* frame1)	override;

		// ### purely experimental design
		virtual	bool		GetLimits(PintJointHandle handle, PintLimits& limits, udword index)	const	override;
		virtual	bool		SetLimits(PintJointHandle handle, const PintLimits& limits, udword index)	override;

		virtual	bool		GetSpring(PintJointHandle handle, PintSpring& spring)	const	override;
		virtual	bool		SetSpring(PintJointHandle handle, const PintSpring& spring)		override;

		virtual	bool		GetGearRatio(PintJointHandle handle, float& ratio)		const	override;
		virtual	bool		SetGearRatio(PintJointHandle handle, float ratio)				override;

		virtual	bool		GetHingeDynamicData(PintJointHandle handle, PintHingeDynamicData& data)	const	override;
		virtual	bool		GetD6DynamicData(PintJointHandle handle, PintD6DynamicData& data)		const	override;
	};

	class EditorVehicleAPI : public Pint_Vehicle
	{
		public:
									EditorVehicleAPI(Pint& pint) : Pint_Vehicle(pint)	{}

		virtual	PintVehicleHandle	CreateVehicle(PintVehicleData& data, const PINT_VEHICLE_CREATE& vehicle)	override;
		virtual	bool				SetVehicleInput(PintVehicleHandle vehicle, const PINT_VEHICLE_INPUT& input)	override;
		virtual	PintActorHandle		GetVehicleActor(PintVehicleHandle vehicle)							const	override;
		virtual	bool				GetVehicleInfo(PintVehicleHandle vehicle, PintVehicleInfo& info)	const	override;
/*		virtual	bool				ResetVehicleData(PintVehicleHandle vehicle);
		virtual	bool				AddActor(PintVehicleHandle vehicle, PintActorHandle actor);
		virtual	bool				AddShape(PintVehicleHandle vehicle, const PINT_SHAPE_CREATE& create);*/
	};

	class EditorCharacterAPI : public Pint_Character
	{
		public:
									EditorCharacterAPI(Pint& pint)	: Pint_Character(pint)	{}

		virtual	PintCharacterHandle	CreateCharacter(const PINT_CHARACTER_CREATE& create)	override;
		virtual	PintActorHandle		GetCharacterActor(PintCharacterHandle h)				override;
		virtual	udword				MoveCharacter(PintCharacterHandle h, const Point& disp)	override;
	};

	class EditorPlugin : public Pint, public PintShapeEnumerateCallback
	{
		friend class EditorCharacterAPI;
		friend class EditorVehicleAPI;
		public:
											EditorPlugin();
		virtual								~EditorPlugin();

		// PintShapeEnumerateCallback
		virtual	void						ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)	override;
		//~PintShapeEnumerateCallback

		// Pint
		virtual	const char*					GetName()				const		override;
		virtual	const char*					GetUIName()				const		override;
		virtual	void						GetCaps(PintCaps& caps)	const		override;
		// Return 0 to disable the raytracing window, etc
		virtual	udword						GetFlags()				const		override;
		virtual	void						Init(const PINT_WORLD_CREATE& desc)	override;
//		virtual	PintSceneHandle				Init(const PINT_WORLD_CREATE& desc);
		virtual	void						Close()								override;
		virtual	udword						Update(float dt)					override;
		virtual	Point						GetMainColor()						override;
		virtual	void						Render(PintRender& renderer, PintRenderPass render_pass)	override;
		virtual	void						RenderDebugData(PintRender& renderer)						override;

		// Scene-related functions
		virtual	void						SetGravity(const Point& gravity)	override;
		virtual	void						SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)	override;

		virtual	PintConvexHandle			CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index)	override;
//		virtual	bool						DeleteConvexObject(PintConvexHandle handle=null, const PintConvexIndex* index=null)		{ NotImplemented("DeleteConvexObject");	return false;	}
		virtual	PintMeshHandle				CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)	override;
//		virtual	bool						DeleteMeshObject(PintMeshHandle handle=null, const PintMeshIndex* index=null)			{ NotImplemented("DeleteMeshObject");	return false;	}

		virtual	PintActorHandle				CreateObject(const PINT_OBJECT_CREATE& desc)	override;
		virtual	bool						ReleaseObject(PintActorHandle handle)			override;

		virtual	PintJointHandle				CreateJoint(const PINT_JOINT_CREATE& desc)	override;
		virtual	bool						ReleaseJoint(PintJointHandle handle)		override;

		virtual	bool						SetSQFlag(PintActorHandle actor, bool flag)	override;
//		virtual	bool						SetSQFlag(PintActorHandle actor, PintShapeHandle shape, bool flag);
		virtual	bool						ResetSQFilters()	override;

		virtual	udword						BatchRaycasts				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)	override;
#ifdef TOSEE
		virtual	udword						BatchRaycastAny				(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts);
		virtual	udword						BatchRaycastAll				(PintSQThreadContext context, udword nb, PintMultipleHits* dest, const PintRaycastData* raycasts);
		virtual	udword						BatchBoxSweeps				(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps);
		virtual	udword						BatchSphereSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps);
#endif
		virtual	udword						BatchCapsuleSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)	override;
#ifdef TOSEE
		virtual	udword						BatchConvexSweeps			(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps);
		virtual	udword						BatchSphereOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps);
		virtual	udword						BatchSphereOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, const PintSphereOverlapData* overlaps);
		virtual	udword						BatchBoxOverlapAny			(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps);
		virtual	udword						BatchBoxOverlapObjects		(PintSQThreadContext context, udword nb, PintMultipleHits* dest, const PintBoxOverlapData* overlaps);
		virtual	udword						BatchCapsuleOverlapAny		(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps);
		virtual	udword						BatchCapsuleOverlapObjects	(PintSQThreadContext context, udword nb, PintMultipleHits* dest, const PintCapsuleOverlapData* overlaps);
		virtual	udword						FindTriangles_MeshSphereOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintSphereOverlapData* overlaps);
		virtual	udword						FindTriangles_MeshBoxOverlap	(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintBoxOverlapData* overlaps);
		virtual	udword						FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintCapsuleOverlapData* overlaps);
#endif
		virtual	udword						FindTriangles_MeshMeshOverlap	(PintSQThreadContext context, PintActorHandle handle0, PintActorHandle handle1, Container& results)	override;

		virtual	PR							GetWorldTransform(PintActorHandle handle)					override;
		virtual	void						SetWorldTransform(PintActorHandle handle, const PR& pose)	override;

#ifdef DEPRECATED
		virtual	const char*					GetName(PintActorHandle handle);
		virtual	bool						SetName(PintActorHandle handle, const char* name);

		//
		virtual	bool						GetBounds(PintActorHandle handle, AABB& bounds);
#endif

#ifdef TOSEE
		virtual	void						UpdateNonProfiled(float dt);
#endif

		virtual	void						AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)	override;
		virtual	void						AddLocalTorque(PintActorHandle handle, const Point& local_torque)										override;

		virtual	Point						GetLinearVelocity(PintActorHandle handle)								override;
		virtual	void						SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)	override;

		virtual	Point						GetAngularVelocity(PintActorHandle handle)									override;
		virtual	void						SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)	override;

#ifdef TOSEE
		virtual	udword						GetShapes(PintActorHandle* shapes, PintActorHandle handle);
		virtual	void						SetLocalRot(PintActorHandle handle, const Quat& q);
		virtual	bool						EnableGravity(PintActorHandle handle, bool flag)	{ NotImplemented("EnableGravity");	return false;	}
#endif

#ifdef DEPRECATED
		virtual	float						GetMass(PintActorHandle handle);
		virtual	void						SetMass(PintActorHandle handle, float mass);
		virtual	Point						GetLocalInertia(PintActorHandle handle);
		virtual	void						SetLocalInertia(PintActorHandle handle, const Point& inertia);
		virtual	float						GetLinearDamping(PintActorHandle handle);
		virtual	float						GetAngularDamping(PintActorHandle handle);
		virtual	void						SetLinearDamping(PintActorHandle handle, float damping);
		virtual	void						SetAngularDamping(PintActorHandle handle, float damping);
#endif

		virtual	bool						SetKinematicPose(PintActorHandle handle, const Point& pos)	override;
		virtual	bool						SetKinematicPose(PintActorHandle handle, const PR& pr)		override;
		virtual	bool						IsKinematic(PintActorHandle handle)							override;
		virtual	bool						EnableKinematic(PintActorHandle handle, bool flag)			override;

		virtual	PintSQThreadContext			CreateSQThreadContext()						override;
		virtual	void						ReleaseSQThreadContext(PintSQThreadContext)	override;

		virtual	PintAggregateHandle			CreateAggregate(udword max_size, bool enable_self_collision)			override;
		virtual	bool						AddToAggregate(PintActorHandle object, PintAggregateHandle aggregate)	override;
		virtual	bool						AddAggregateToScene(PintAggregateHandle aggregate)						override;
#ifdef TOSEE
		virtual	PintArticHandle				CreateArticulation(const PINT_ARTICULATION_CREATE&);
		virtual	PintActorHandle				CreateArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_ARTICULATED_BODY_CREATE&, PintArticHandle articulation);
		virtual	bool						AddArticulationToScene(PintArticHandle articulation);
		virtual	void						SetArticulatedMotor(PintActorHandle object, const PINT_ARTICULATED_MOTOR_CREATE& motor);
		virtual	PintArticHandle				CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE&);
		virtual	PintActorHandle				CreateRCArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_RC_ARTICULATED_BODY_CREATE&, PintArticHandle articulation);
		virtual	bool						AddRCArticulationToScene(PintArticHandle articulation);
//		virtual	void						SetRCArticulatedMotor(PintActorHandle object, const PINT_ARTICULATED_MOTOR_CREATE& motor);
		virtual	bool						AddRCArticulationToAggregate(PintArticHandle articulation, PintActorHandle aggregate);
		virtual	bool						SetRCADriveEnabled(PintActorHandle handle, bool flag);
		virtual	bool						SetRCADriveVelocity(PintActorHandle handle, float velocity);
#endif
#ifdef NOT_WORKING
		virtual	PintVehicleHandle			CreateVehicle(PintVehicleData& data, const PINT_VEHICLE_CREATE& vehicle);
		virtual	void						SetVehicleInput(PintVehicleHandle vehicle, const PINT_VEHICLE_INPUT& input);
		virtual	bool						GetVehicleInfo(PintVehicleHandle vehicle, PintVehicleInfo& info);
#endif
		virtual	bool						SetDriveEnabled(PintJointHandle handle, bool flag)									override;
		virtual	bool						SetDrivePosition(PintJointHandle handle, const PR& pose)							override;
		virtual	bool						SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)	override;
		virtual	PR							GetDrivePosition(PintJointHandle handle)											override;
#ifdef TOSEE
		virtual	void						SetGearJointError(PintJointHandle handle, float error);
#endif

		virtual	Pint_Scene*					GetSceneAPI()		override;
		virtual	Pint_Actor*					GetActorAPI()		override;
		virtual	Pint_Shape*					GetShapeAPI()		override;
		virtual	Pint_Joint*					GetJointAPI()		override;
		virtual	Pint_Vehicle*				GetVehicleAPI()		override;
		virtual	Pint_Character*				GetCharacterAPI()	override;

#ifdef TOSEE
		virtual	void						TestNewFeature();
#endif
		//~Pint

				void						HideActors(udword nb, PintActorHandle* handles);
				void						ResetPoses(udword nb, PintActorHandle* handles);
				PintActorHandle				MergeActors(udword nb, const PintActorHandle* handles, MergeInterface& merge_interface, const MergeParams& params);
				PintActorHandle				CreateSingleConvex(udword nb, const PintActorHandle* handles);
				PintActorHandle				CreateSingleMesh(udword nb, const PintActorHandle* handles);
				PintActorHandle				ConvexDecomp(PintActorHandle);
				void						GetAllActors(PtrContainer& actors);

		inline_	SceneData*					GetEditorSceneData()					{ return mEditorScene;											}
		inline_	const SceneData*			GetEditorSceneData()			const	{ return mEditorScene;											}
		inline_	const CollisionGroupsData*	GetEditorCollisionGroupsData()	const	{ return mCollisionGroups;										}
		inline_	udword						GetNbEditorActors()				const	{ return mEditorActors.GetNbEntries();							}
		inline_	const ActorData*			GetEditorActor(udword i)		const	{ return HandleToActorData(mEditorActors.GetEntry(i));			}
		inline_	udword						GetNbEditorMaterials()			const	{ return mEditorMaterials.GetNbEntries();						}
		inline_	const MaterialData*			GetEditorMaterial(udword i)		const	{ return (const MaterialData*)mEditorMaterials.GetEntry(i);		}
		inline_	udword						GetNbEditorJoints()				const	{ return mEditorJoints.GetNbEntries();							}
		inline_	const JointData*			GetEditorJoint(udword i)		const	{ return HandleToJointData(mEditorJoints.GetEntry(i));			}
		inline_	udword						GetNbEditorMeshes()				const	{ return mEditorMeshes.GetNbEntries();							}
		inline_	const MeshData*				GetEditorMesh(udword i)			const	{ return (const MeshData*)mEditorMeshes.GetEntry(i);			}
		inline_	udword						GetNbEditorAggregates()			const	{ return mEditorAggregates.GetNbEntries();						}
		inline_	const AggregateData*		GetEditorAggregate(udword i)	const	{ return (const AggregateData*)mEditorAggregates.GetEntry(i);	}

		inline_	const PtrContainer&			GetEditorSphereShapes()			const	{ return mEditorSphereShapes;	}
		inline_	PtrContainer&				GetEditorSphereShapes()					{ return mEditorSphereShapes;	}
		inline_	const PtrContainer&			GetEditorCapsuleShapes()		const	{ return mEditorCapsuleShapes;	}
		inline_	PtrContainer&				GetEditorCapsuleShapes()				{ return mEditorCapsuleShapes;	}
		inline_	const PtrContainer&			GetEditorCylinderShapes()		const	{ return mEditorCylinderShapes;	}
		inline_	PtrContainer&				GetEditorCylinderShapes()				{ return mEditorCylinderShapes;	}
		inline_	const PtrContainer&			GetEditorBoxShapes()			const	{ return mEditorBoxShapes;		}
		inline_	PtrContainer&				GetEditorBoxShapes()					{ return mEditorBoxShapes;		}
		inline_	const PtrContainer&			GetEditorConvexShapes()			const	{ return mEditorConvexShapes;	}
		inline_	PtrContainer&				GetEditorConvexShapes()					{ return mEditorConvexShapes;	}
		inline_	const PtrContainer&			GetEditorMeshShapes()			const	{ return mEditorMeshShapes;		}
		inline_	PtrContainer&				GetEditorMeshShapes()					{ return mEditorMeshShapes;		}
		inline_	const PtrContainer&			GetEditorMeshShapes2()			const	{ return mEditorMeshShapes2;	}
		inline_	PtrContainer&				GetEditorMeshShapes2()					{ return mEditorMeshShapes2;	}

		private:
				EditorSceneAPI				mSceneAPI;
				EditorActorAPI				mActorAPI;
				EditorShapeAPI				mShapeAPI;
				EditorJointAPI				mJointAPI;
				EditorVehicleAPI			mVehicleAPI;
				EditorCharacterAPI			mCharacterAPI;
				SceneData*					mEditorScene;
				CollisionGroupsData*		mCollisionGroups;
				PtrContainer				mEditorActors;			// EditorActorData*
				PtrContainer				mEditorMaterials;		// MaterialData*
				PtrContainer				mEditorSphereShapes;	// SphereShapeData*
				PtrContainer				mEditorCapsuleShapes;	// CapsuleShapeData*
				PtrContainer				mEditorCylinderShapes;	// CylinderShapeData*
				PtrContainer				mEditorBoxShapes;		// BoxShapeData*
				PtrContainer				mEditorConvexShapes;	// ConvexShapeData*
				PtrContainer				mEditorMeshShapes;		// MeshShapeData*
				PtrContainer				mEditorMeshShapes2;		// MeshShapeData2*
				PtrContainer				mEditorJoints;			// JointData*
				PtrContainer				mEditorMeshes;			// MeshData*
				PtrContainer				mEditorAggregates;		// AggregateData*
				PtrContainer				mEditorCharacters;		// CharacterData*
				PtrContainer				mEditorVehicles;		// VehicleData*

				Stats						mStats;
		public:
				PintActorHandle				CreateDummyEditorActor(PintActorHandle native_handle);
				PintActorHandle				RemapActorHandle(PintActorHandle native_handle) const;
				PintActorHandle				RemapActorAndShapeHandles(PintActorHandle native_handle, PintShapeHandle& shape_handle) const;
		private:
				MaterialData*				FindMaterial(const PINT_MATERIAL_CREATE& create) const;
		const	MaterialData*				RegisterMaterial(const PINT_MATERIAL_CREATE& create, bool add_to_database);

				SphereShapeData*			FindSphereShape(const PINT_SPHERE_CREATE& create, const MaterialData* material) const;
		const	SphereShapeData*			RegisterSphereShape(const PINT_SPHERE_CREATE& create, const MaterialData* material, bool add_to_database);

				CapsuleShapeData*			FindCapsuleShape(const PINT_CAPSULE_CREATE& create, const MaterialData* material) const;
		const	CapsuleShapeData*			RegisterCapsuleShape(const PINT_CAPSULE_CREATE& create, const MaterialData* material, bool add_to_database);

				CylinderShapeData*			FindCylinderShape(const PINT_CYLINDER_CREATE& create, const MaterialData* material) const;
		const	CylinderShapeData*			RegisterCylinderShape(const PINT_CYLINDER_CREATE& create, const MaterialData* material, bool add_to_database);

				BoxShapeData*				FindBoxShape(const PINT_BOX_CREATE& create, const MaterialData* material) const;
		const	BoxShapeData*				RegisterBoxShape(const PINT_BOX_CREATE& create, const MaterialData* material, bool add_to_database);

				ConvexShapeData*			FindConvexShape(const PINT_CONVEX_CREATE& create, const MaterialData* material) const;
		const	ConvexShapeData*			RegisterConvexShape(const PINT_CONVEX_CREATE& create, const MaterialData* material, bool add_to_database);

				MeshShapeData*				FindMeshShape(const PINT_MESH_CREATE& create, const MaterialData* material) const;
		const	MeshShapeData*				RegisterMeshShape(const PINT_MESH_CREATE& create, const MaterialData* material, bool add_to_database);

				MeshShapeData2*				FindMeshShape2(const PINT_MESH_CREATE2& create, const MaterialData* material) const;
		const	MeshShapeData2*				RegisterMeshShape2(const PINT_MESH_CREATE2& create, const MaterialData* material, bool add_to_database);

				void						UpdateUI();
	};

	inline_ bool CanExport(const BaseData* current)
	{
		return current->mUserData && ((const EditorObject*)current->mUserData)->mExportable;
	}

	IceWindow*			Editor_InitGUI(IceWidget* parent, PintGUIHelper& helper);
	void				Editor_CloseGUI();
	void				Editor_Init(const PINT_WORLD_CREATE& desc);
	void				Editor_Close();
	EditorPlugin*		GetEditor();

	// The following functions use the fact that the editor is built as part of PEEL directly,
	// so we don't have to expose all of them to a plugin interface.
	PintPlugin*			GetEditorPlugin();
	void				SetEditedPlugin(PintPlugin* edited);
	bool				IsEditor(Pint* pint);
	void				Editor_HideActors(udword nb, PintActorHandle* handles);
	void				Editor_ResetPoses(udword nb, PintActorHandle* handles);
	PintActorHandle		Editor_CreateCompound(udword nb, PintActorHandle* handles, const MergeParams& params);
	PintActorHandle		Editor_CreateSingleConvex(udword nb, PintActorHandle* handles);
	PintActorHandle		Editor_CreateSingleMesh(udword nb, PintActorHandle* handles);
	void				Editor_GetAllActors(PtrContainer& actors);
	PintActorHandle		Editor_MakeStatic(PintActorHandle h);
	PintActorHandle		Editor_MakeDynamic(PintActorHandle h);
//	PintActorHandle		Editor_MakeKinematic(PintActorHandle h);
	PintActorHandle		Editor_MakeSphere(PintActorHandle h);
	PintActorHandle		Editor_MakeBox(PintActorHandle h);
	PintActorHandle		Editor_MakeConvex(PintActorHandle h);
	PintActorHandle		Editor_MakeMesh(PintActorHandle h);
	PintActorHandle		Editor_MakeConvexDecomp(PintActorHandle h);
	PintActorHandle		Editor_GetNativeHandle(PintActorHandle h);
	PintShapeHandle		Editor_GetNativeHandle(PintShapeHandle h);

#endif
