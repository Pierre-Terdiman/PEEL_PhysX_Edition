///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_HAVOK_H
#define PINT_COMMON_HAVOK_H

	#define	HAVOK_MAIN_COLOR	Point(0.0f, 0.75f, 1.0f)

	inline_ Point			ToPoint(const hkVector4& p)		{ return Point(p(0), p(1), p(2));											}
	inline_ Quat			ToQuat(const hkQuaternion& q)	{ return Quat(q(3), q(0), q(1), q(2));										}
	inline_ Quat			ToQuat(const hkRotation& q)		{ return ToQuat(hkQuaternion(q));											}
	inline_ hkVector4		ToHkVector4(const Point& p)		{ return hkVector4(p.x, p.y, p.z);											}
	inline_ hkQuaternion	ToHkQuaternion(const Quat& q)	{ return hkQuaternion(q.p.x, q.p.y, q.p.z, q.w);							}
	inline_ PR				ToPR(const hkTransform& pose)	{ return PR(ToPoint(pose.getTranslation()), ToQuat(pose.getRotation()));	}

// ### this is a bit bonkers!
/*#define HAVOK_RIGIDBODY_FROM_BOX(obb)							\
	const hkpBoxShape BoxShape(ToHkVector4(obb.mExtents), 0.0f);\
																\
	hkpRigidBodyCinfo ci;										\
	ci.m_shape		= &BoxShape;								\
	ci.m_motionType	= hkpMotion::MOTION_FIXED;					\
	ci.m_rotation	= ToHkQuaternion(Quat(obb.mRot));			\
	ci.m_position	= ToHkVector4(obb.mCenter);					\
	hkpRigidBody rigidBody(ci);*/

#define HAVOK_COLLIDABLE_FROM_BOX(obb)													\
	const hkpBoxShape BoxShape(ToHkVector4(obb.mExtents), 0.0f);						\
	const hkTransform Pose(ToHkQuaternion(Quat(obb.mRot)), ToHkVector4(obb.mCenter));	\
	const hkpCollidable Collidable(&BoxShape, &Pose);

// ### this is a bit bonkers!
/*#define HAVOK_RIGIDBODY_FROM_SPHERE(sphere)						\
	const hkpSphereShape SphereShape(sphere.mRadius);			\
																\
	hkpRigidBodyCinfo ci;										\
	ci.m_shape		= &SphereShape;								\
	ci.m_motionType	= hkpMotion::MOTION_FIXED;					\
	ci.m_rotation	= hkQuaternion(0.0f, 0.0f, 0.0f, 1.0f);		\
	ci.m_position	= ToHkVector4(sphere.mCenter);				\
	hkpRigidBody rigidBody(ci);*/

#define HAVOK_COLLIDABLE_FROM_SPHERE(sphere)													\
	const hkpSphereShape SphereShape(sphere.mRadius);											\
	const hkTransform Pose(hkQuaternion(0.0f, 0.0f, 0.0f, 1.0f), ToHkVector4(sphere.mCenter));	\
	const hkpCollidable Collidable(&SphereShape, &Pose);

// ### this is a bit bonkers!
/*#define HAVOK_RIGIDBODY_FROM_CAPSULE(capsule, center)																			\
	const hkpCapsuleShape CapsuleShape(ToHkVector4(capsule.mP0 - center), ToHkVector4(capsule.mP1 - center), capsule.mRadius);	\
																																\
	hkpRigidBodyCinfo ci;																										\
	ci.m_shape		= &CapsuleShape;																							\
	ci.m_motionType	= hkpMotion::MOTION_FIXED;																					\
	ci.m_rotation	= hkQuaternion(0.0f, 0.0f, 0.0f, 1.0f);																		\
	ci.m_position	= ToHkVector4(center);																						\
	hkpRigidBody rigidBody(ci);*/

#define HAVOK_COLLIDABLE_FROM_CAPSULE(capsule, center)																			\
	const hkpCapsuleShape CapsuleShape(ToHkVector4(capsule.mP0 - center), ToHkVector4(capsule.mP1 - center), capsule.mRadius);	\
	const hkTransform Pose(hkQuaternion(0.0f, 0.0f, 0.0f, 1.0f), ToHkVector4(center));											\
	const hkpCollidable Collidable(&CapsuleShape, &Pose);

	// We store the initial radius/half-height in our own internal capsule shape, to make sure that the conversion between
	// Havok's format and ours doesn't slightly change the values. We need exact values for shape sharing.
	struct InternalCapsuleShape
	{
		hkpCapsuleShape*	mShape;
		float				mRadius;
		float				mHalfHeight;
	};

	hkpSphereShape*			FindSphereShape	(bool share_shapes, std::vector<hkpSphereShape*>& shapes,			const PINT_SPHERE_CREATE& sphere_create);
	hkpCapsuleShape*		FindCapsuleShape(bool share_shapes, std::vector<InternalCapsuleShape>& shapes,		const PINT_CAPSULE_CREATE& capsule_create);
	hkpBoxShape*			FindBoxShape	(bool share_shapes, std::vector<hkpBoxShape*>& shapes,				const PINT_BOX_CREATE& box_create);
	hkpConvexVerticesShape*	FindConvexShape	(bool share_shapes, std::vector<hkpConvexVerticesShape*>& shapes,	const PINT_CONVEX_CREATE& convex_create, const hkStridedVertices& strided_verts);

	struct HavokMeshRender
	{
		HavokMeshRender(hkpShape* shape, PintShapeRenderer* renderer) :
			mTriangleMesh	(shape),
			mRenderer		(renderer)
		{
		}
		hkpShape*			mTriangleMesh;
		PintShapeRenderer*	mRenderer;
	};

#ifndef HAVOK660
	// We store the initial radius/half-height in our own internal cylinder shape, to make sure that the conversion between
	// Havok's format and ours doesn't slightly change the values. We need exact values for shape sharing.
	struct InternalCylinderShape
	{
		hkpCylinderShape*	mShape;
		float				mRadius;
		float				mHalfHeight;
	};
	hkpCylinderShape*		FindCylinderShape(bool share_shapes, std::vector<InternalCylinderShape>& shapes,	const PINT_CYLINDER_CREATE& cylinder_create);

	enum HavokMeshFormat
	{
		HAVOK_BV_COMPRESSED_MESH_SHAPE,
		HAVOK_EXTENDED_MESH_SHAPE,
	};

//	hkpShape* CreateMeshShape(const PINT_MESH_CREATE& create, HavokMeshFormat format);

	hkpShape* CreateMeshShape(const PINT_MESH_CREATE& create, HavokMeshFormat format, std::vector<HavokMeshRender>& meshes, PintShapeRenderer* renderer);

	udword Havok_GetAllocatedMemory();

	struct EditableParams
	{
												EditableParams();

		udword									mNbThreads;
		hkpRigidBodyCinfo::SolverDeactivation	mSolverDeactivation;
		hkpWorldCinfo::BroadPhaseType			mBroadPhaseType;
		hkpWorldCinfo::ContactPointGeneration	mContactPointGeneration;
		HavokMeshFormat							mMeshFormat;
		udword									mCCDQuality;
		float									mLinearDamping;
		float									mAngularDamping;
		float									mCollisionTolerance;
		float									mGlobalBoxSize;
		float									mMaxLinearVelocity;
		udword									mSolverIterationCount;
		udword									mSolverBufferSize;
		udword									mSizeOfToiEventQueue;
		bool									mEnableSleeping;
		bool									mShareShapes;
		bool									mUseVDB;
		bool									mUseCCD;
	};

	class UICallback
	{
		public:
		virtual	void			UIModificationCallback()	= 0;
	};

	namespace Havok_
	{
		IceWindow*				InitSharedGUI(IceWidget* parent, PintGUIHelper& helper, UICallback& callback);
		const EditableParams&	GetEditableParams();
		void					GetOptionsFromGUI();
		void					CloseSharedGUI();
	}

#define SUPPORT_HAVOK_ARTICULATION

	class SharedHavok : public Pint
	{
		public:
														SharedHavok(const EditableParams& params);
		virtual											~SharedHavok();

		virtual	void									Render(PintRender& renderer, PintRenderPass render_pass);
		virtual	void									RenderDebugData(PintRender& renderer);

		virtual	PintActorHandle							CreateObject(const PINT_OBJECT_CREATE& desc);
		virtual	bool									ReleaseObject(PintActorHandle handle);
		virtual	PintJointHandle							CreateJoint(const PINT_JOINT_CREATE& desc);

#ifdef SUPPORT_HAVOK_ARTICULATION
		virtual	PintArticHandle							CreateArticulation(const PINT_ARTICULATION_CREATE&);
		virtual	PintActorHandle							CreateArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_ARTICULATED_BODY_CREATE&, PintArticHandle articulation);
		virtual	bool									AddArticulationToScene(PintArticHandle articulation);
#endif
				const EditableParams&					mParams;

				hkpWorld*								mPhysicsWorld;
				hkMemoryRouter*							mMemoryRouter;
				char*									mStackBuffer;
//				hkJobThreadPool*						mThreadPool;
				hkJobQueue*								mJobQueue;
				hkpPhysicsContext*						mContext;
				hkVisualDebugger*						mVdb;

//				std::vector<hkpRigidBody*>				mRigidBodies;
				Container								mRigidBodies;
				Container								mPhantoms;
				std::vector<hkpSphereShape*>			mSphereShapes;
				std::vector<hkpBoxShape*>				mBoxShapes;
				std::vector<InternalCapsuleShape>		mCapsuleShapes;
				std::vector<InternalCylinderShape>		mCylinderShapes;
				std::vector<hkpConvexVerticesShape*>	mConvexShapes;
				std::vector<HavokMeshRender>			mMeshes;

				std::vector<hkpConvexVerticesShape*>	mConvexObjects;

		protected:
				hkpWorld*								SetupWorld(const PINT_WORLD_CREATE& desc, hkJobQueue* job_queue);
				void									FillRigidBodyInfo(hkpRigidBodyCinfo& info, const PINT_OBJECT_CREATE& desc, hkpShape* shape);
				hkpRigidBody*							CreateObject(const hkpRigidBodyCinfo& info, const PINT_OBJECT_CREATE& create, hkpShape* shape);
				PintActorHandle							CreateCompoundObject(const PINT_OBJECT_CREATE& desc);
	};

#endif
#endif
