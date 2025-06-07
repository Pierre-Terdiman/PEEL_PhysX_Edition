///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_MUJOCO_3_3_2_H
#define PINT_MUJOCO_3_3_2_H

	#include "..\Pint.h"
	#include "..\PINT_Common\PINT_CommonShapes.h"
	#include <map>

	#include <mujoco/mujoco.h>
	#include <mujoco/mjspec.h>
	#include <Core/RockArray.h>

	class MuJoCo_ActorAPI : public Pint_Actor
	{
		public:
								MuJoCo_ActorAPI(Pint& pint);
		virtual					~MuJoCo_ActorAPI();

		virtual	const char*		GetName(PintActorHandle handle)					const	override;
/*		virtual	bool			SetName(PintActorHandle handle, const char* name)		override;

		virtual	udword			GetNbShapes(PintActorHandle handle)				const	override;
		virtual	PintShapeHandle	GetShape(PintActorHandle handle, udword index)	const	override;

		virtual	udword			GetNbJoints(PintActorHandle handle)				const	override;
		virtual	PintJointHandle	GetJoint(PintActorHandle handle, udword index)	const	override;*/

		virtual	bool			GetWorldBounds(PintActorHandle handle, AABB& bounds)	const	override;

		/*virtual	void			WakeUp(PintActorHandle handle)	override;

		virtual	bool			SetGravityFlag(PintActorHandle handle, bool flag)		override;
		virtual	bool			SetDebugVizFlag(PintActorHandle handle, bool flag)		override;
		virtual	bool			SetSimulationFlag(PintActorHandle handle, bool flag)	override;*/

		virtual	float			GetLinearDamping(PintActorHandle handle)		const	override;
		virtual	bool			SetLinearDamping(PintActorHandle handle, float damping)	override;

		virtual	float			GetAngularDamping(PintActorHandle handle)			const	override;
		virtual	bool			SetAngularDamping(PintActorHandle handle, float damping)	override;

/*		virtual	bool			GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space)	const	override;
		virtual	bool			SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)	override;*/

		virtual	bool			GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space)	const	override;
		virtual	bool			SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)		override;

		virtual	float			GetMass(PintActorHandle handle)	const			override;
		/*virtual	bool			SetMass(PintActorHandle handle, float mass)		override;*/

		virtual	bool			GetLocalInertia(PintActorHandle handle, Point& inertia)	const	override;
/*		virtual	bool			SetLocalInertia(PintActorHandle handle, const Point& inertia)	override;

		virtual	bool			GetCMassLocalPose(PintActorHandle handle, PR& pose)		const	override;
		virtual	bool			SetCMassLocalPose(PintActorHandle handle, const PR& pose)		override;*/
	};


	class MuJoCoPint : public Pint, public PintShapeEnumerateCallback
	{
		public:
										MuJoCoPint();
		virtual							~MuJoCoPint();

		// Pint
		virtual	const char*				GetName()				const	override;
		virtual	const char*				GetUIName()				const	override;
		virtual	void					GetCaps(PintCaps& caps)	const	override;
		virtual	void					Init(const PINT_WORLD_CREATE& desc)	override;
		virtual	void					SetGravity(const Point& gravity)	override;
		virtual	void					SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)	override;
		virtual	void					Close()	override;
		virtual	udword					Update(float dt)	override;
		virtual	Point					GetMainColor()	override;
		virtual	void					Render(PintRender& renderer, PintRenderPass render_pass)	override;
		virtual	void					RenderDebugData(PintRender& renderer)	override;

		virtual	PintActorHandle			CreateObject(const PINT_OBJECT_CREATE& desc)	override;
		virtual	bool					ReleaseObject(PintActorHandle handle)	override;
		virtual	PintJointHandle			CreateJoint(const PINT_JOINT_CREATE& desc)	override;
		virtual	bool					ReleaseJoint(PintJointHandle handle) override;

		virtual	bool					ResetSQFilters()	override	{ return false;	}

		virtual	udword					BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)	override;

		virtual	PR						GetWorldTransform(PintActorHandle handle)	override;

		virtual	void					AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)	override;

		virtual	Point					GetAngularVelocity(PintActorHandle handle)	override;
		virtual	void					SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)	override;

		virtual	PintArticHandle			CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE&)	override;
		virtual	PintActorHandle			CreateRCArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_RC_ARTICULATED_BODY_CREATE&, PintArticHandle articulation)	override;
		virtual	bool					AddRCArticulationToScene(PintArticHandle articulation)	override;
		//virtual	bool					AddRCArticulationToAggregate(PintArticHandle articulation, PintAggregateHandle aggregate);
		virtual	bool					SetRCADriveEnabled(PintActorHandle handle, bool flag)	override;
		virtual	bool					SetRCADriveVelocity(PintActorHandle handle, float velocity)	override;
		virtual	bool					SetRCADrivePosition(PintActorHandle handle, float position)	override;

		virtual	Pint_Actor*				GetActorAPI()	override	{ return &mActorAPI;	}

		//~Pint

		// PintShapeEnumerateCallback
		virtual	void					ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)	override;
		//~PintShapeEnumerateCallback

		MuJoCo_ActorAPI	mActorAPI;

		mjSpec*			mSpec;
		mjModel*		mModel;
		mjData*			mData;
		mjThreadPool*	mThreadPool;
		mjsBody*		mWorld;

		struct BodyData
		{
			mjsBody*	mBody;
			//udword		mBodyID;
			udword		mActuatorID;
			PR			mInitialPoseWorldSpace;
			udword		mNbVisuals;
			udword		mVisualIndex;

			void		Init()
			{
				mBody			= null;
				mActuatorID		= INVALID_ID;
				mNbVisuals		= 0;
				mVisualIndex	= INVALID_ID;
				mInitialPoseWorldSpace.Identity();
			}
		};
		Rock::Array<BodyData>	mBodyData;

		struct VisualData
		{
			PintShapeRenderer*	mRenderer;
			PR					mLocalPose;
		};
		Rock::Array<VisualData>	mVisualData;

		struct ShapeData
		{
			PintShapeRenderer*	mRenderer;
			udword				mBodyID;
			udword				mGroupID;
			udword				mGeomID;	// ### probably not needed
		};
		Rock::Array<ShapeData>	mShapeData;

		struct MeshData
		{
			PintShapeRenderer*	mRenderer;
			udword				mMeshID;
		};
		Rock::Array<MeshData>	mMeshData;

		struct ArticulationData
		{
			udword	mFixedBase;
		};
		Rock::Array<ArticulationData>	mArticulationData;

		struct ActuatorData
		{
			//udword	mJointID;
			udword	mActuatorID;
			udword	mActuatorID2;
			float	mTargetVelocity;
			float	mTargetPos;
			BOOL	mEnabled;
		};
		Rock::Array<ActuatorData>	mActuatorData;

		struct InitialVels
		{
			udword	mBodyID;
			Point	mLinear;
			Point	mAngular;
		};
		Rock::Array<InitialVels>	mInitialVelocities;

		Rock::Array<udword>	mBodyIDsToClearOut;

		udword	mBodyID;	// ### might be redundant with mBodyData size now
		udword	mGeomID;
		udword	mJointID;
		udword	mConvexMeshID;

		private:
		PintActorHandle	CreateGroundPlane();
		BodyData&		AddBody(mjsBody* parent, udword* body_id = null, const PR* world_pose = null);
		BodyData&		RegisterBody(mjsBody* body, udword bodyID, const PR* pose = null);
		mjsJoint*		AddJoint(mjsBody* body, mjtJoint type, udword* joint_id = null);
		void			RenderAABBs(PintRender& renderer) const;
		void			RenderGeoms(PintRender& renderer)	const;
		void			CompileModel();
	};

	IceWindow*	MuJoCo_InitGUI(IceWidget* parent, PintGUIHelper& helper);
	void		MuJoCo_CloseGUI();
	void		MuJoCo_Init(const PINT_WORLD_CREATE& desc);
	void		MuJoCo_Close();
	MuJoCoPint*	GetMuJoCo();

	DECLARE_PINT_EXPORTS

#endif