///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PHYSICS_DATA_H
#define PHYSICS_DATA_H

#include "..\Pint.h"

	// These classes are basically "editor classes" and only used in editor mode.
	//
	// These are kind of redundant/duplicates with the ZCB2 chunk classes. Differences are:
	// - chunks work with IDs, which make things more complicated when editing
	// - the ZCB2Exporter takes ownership of the chunks after RegisterUserDefinedChunk(), which one needs
	//   to call for exporting. This means that if we'd store the data in ZCB2 chunk classes directly,
	//   e.g. in the editor, then that data would vanish/become corrupted after export. We'd need to
	//   change ICE to fix this one. (*)
	//
	// (*) We actually did that now. Oh well.
	//
	// We could eventually merge these two sets of classes but maybe the decoupling will be handy, we'll see.
	// It could be cleaner to reuse these classes (not tied to an export framework) in other contexts.
	//
	// In fact I think we already see a reason to decouple things: sometimes we want to export to
	// another format (USD, etc) and we don't need the ZCB2 chunks at all in this case. We could still
	// choose to make the editor-classes based on ZCB2 chunks nonetheless, but the first issue with IDs
	// would remain.

	class CollisionGroupsData : public Allocateable
	{
		public:
									CollisionGroupsData(udword nb_groups, const PintDisabledGroups* groups);
		virtual						~CollisionGroupsData();

		udword						mNbGroups;
		PintDisabledGroups*			mGroups;
	};

	class BaseData : public Allocateable
	{
		public:
									BaseData()	: mID(INVALID_ID), mUserData(null)	{}
		virtual						~BaseData()										{}

		String						mName;
		mutable udword				mID;	// Set during export
		void*						mUserData;
	};

	class SceneData : public BaseData
	{
		public:
									SceneData(const PINT_WORLD_CREATE&, const char* scene_desc);
		virtual						~SceneData()	{}

		String						mDesc;
		AABB						mGlobalBounds;
		PintCameraPose				mCamera[PINT_MAX_CAMERA_POSES];
		Point						mGravity;
		udword						mNbSimulateCallsPerFrame;
		float						mTimestep;
		bool						mCreateDefaultEnvironment;
	};

	class ActorData : public BaseData
	{
		public:
									ActorData(const PINT_OBJECT_CREATE&);
		virtual						~ActorData()	{}

		PtrContainer				mShapes;			// ShapeData*
		Point						mPosition;
		Quat						mRotation;
		Point						mCOMLocalOffset;
		Point						mLinearVelocity;
		Point						mAngularVelocity;
		float						mMass;
		float						mMassForInertia;	// If negative, use the same as mMass.
		PintCollisionGroup			mCollisionGroup;	// 0-31
		bool						mKinematic;
		bool						mAddToWorld;

		// Not exported
		PtrContainer				mJoints;			// JointData*
	};

	class MaterialData : public BaseData
	{
		public:
									MaterialData(const PINT_MATERIAL_CREATE&);
		virtual						~MaterialData()	{}

		float						mStaticFriction;
		float						mDynamicFriction;
		float						mRestitution;
	};

	class ConvexMeshData : public BaseData
	{
		public:
									ConvexMeshData(const PINT_CONVEX_DATA_CREATE&);
		virtual						~ConvexMeshData();

		udword						mNbVerts;
		Point*						mVerts;
	};

	class MeshData : public BaseData
	{
		public:
									MeshData(const PINT_MESH_DATA_CREATE&);
		virtual						~MeshData();

		udword						mNbVerts;
		Point*						mVerts;
		udword						mNbTris;
		IndexedTriangle*			mTris;
	};

	class ShapeData : public BaseData
	{
		public:
									ShapeData(const PINT_SHAPE_CREATE&);
		virtual						~ShapeData()	{}

		PintShape					mType;
		Point						mLocalPos;
		Quat						mLocalRot;
		const MaterialData*			mMaterial;
		/*const*/ PintShapeRenderer*	mRenderer;
	};

	class SphereShapeData : public ShapeData
	{
		public:
									SphereShapeData(const PINT_SPHERE_CREATE&);
		virtual						~SphereShapeData()	{}

		float						mRadius;
	};

	class CapsuleShapeData : public ShapeData
	{
		public:
									CapsuleShapeData(const PINT_CAPSULE_CREATE&);
		virtual						~CapsuleShapeData()	{}

		float						mRadius;
		float						mHalfHeight;
	};

	class CylinderShapeData : public ShapeData
	{
		public:
									CylinderShapeData(const PINT_CYLINDER_CREATE&);
		virtual						~CylinderShapeData()	{}

		float						mRadius;
		float						mHalfHeight;
	};

	class BoxShapeData : public ShapeData
	{
		public:
									BoxShapeData(const PINT_BOX_CREATE&);
		virtual						~BoxShapeData()	{}

		Point						mExtents;
	};

	class ConvexShapeData : public ShapeData
	{
		public:
									ConvexShapeData(const PINT_CONVEX_CREATE&);
		virtual						~ConvexShapeData();

		udword						mNbVerts;
		Point*						mVerts;
	};

	class MeshShapeData : public ShapeData
	{
		public:
									MeshShapeData(const PINT_MESH_CREATE&);
		virtual						~MeshShapeData();

		udword						mNbVerts;
		Point*						mVerts;
		udword						mNbTris;
		IndexedTriangle*			mTris;
	};

	class MeshShapeData2 : public ShapeData
	{
		public:
									MeshShapeData2(const PINT_MESH_CREATE2&);
		virtual						~MeshShapeData2();

		MeshData*					mMeshData;
	};

	class JointData : public BaseData
	{
		public:
									JointData(const PINT_JOINT_CREATE&);
		virtual						~JointData()	{}

		PintJoint					mType;
		ActorData*					mObject0;
		ActorData*					mObject1;
	};

	class SphericalJointData : public JointData
	{
		public:
									SphericalJointData(const PINT_SPHERICAL_JOINT_CREATE&);
		virtual						~SphericalJointData()	{}

		Point						mLocalPivot0;
		Point						mLocalPivot1;
	};

	class FixedJointData : public JointData
	{
		public:
									FixedJointData(const PINT_FIXED_JOINT_CREATE&);
		virtual						~FixedJointData()	{}

		Point						mLocalPivot0;
		Point						mLocalPivot1;
	};

	class DistanceJointData : public JointData
	{
		public:
									DistanceJointData(const PINT_DISTANCE_JOINT_CREATE&);
		virtual						~DistanceJointData()	{}

		Point						mLocalPivot0;
		Point						mLocalPivot1;
		float						mMinDistance;
		float						mMaxDistance;
	};

	class PrismaticJointData : public JointData
	{
		public:
									PrismaticJointData(const PINT_PRISMATIC_JOINT_CREATE&);
		virtual						~PrismaticJointData()	{}

		PR							mLocalPivot0;
		PR							mLocalPivot1;
		Point						mLocalAxis0;
		Point						mLocalAxis1;
		float						mMinLimit;
		float						mMaxLimit;
		float						mSpringStiffness;
		float						mSpringDamping;
	};

	class HingeJointData : public JointData
	{
		public:
									HingeJointData(const PINT_HINGE_JOINT_CREATE&);
		virtual						~HingeJointData()	{}

		Point						mLocalPivot0;
		Point						mLocalPivot1;
		Point						mLocalAxis0;
		Point						mLocalAxis1;
		float						mMinLimitAngle;
		float						mMaxLimitAngle;
		Point						mGlobalAnchor;
		Point						mGlobalAxis;
		bool						mUseMotor;
		float						mDriveVelocity;
	};

	class Hinge2JointData : public JointData
	{
		public:
									Hinge2JointData(const PINT_HINGE2_JOINT_CREATE&);
		virtual						~Hinge2JointData()	{}

		PR							mLocalPivot0;
		PR							mLocalPivot1;
		float						mMinLimitAngle;
		float						mMaxLimitAngle;
		bool						mUseMotor;
		float						mDriveVelocity;
	};

	class D6JointData : public JointData
	{
		public:
									D6JointData(const PINT_D6_JOINT_CREATE&);
		virtual						~D6JointData()	{}

		PR							mLocalPivot0;
		PR							mLocalPivot1;
		AABB						mLinearLimits;
		float						mMinTwist;
		float						mMaxTwist;
		float						mMaxSwingY;
		float						mMaxSwingZ;
		udword						mMotorFlags;
		float						mMotorStiffness;
		float						mMotorDamping;
	};

	class GearJointData : public JointData
	{
		public:
									GearJointData(const PINT_GEAR_JOINT_CREATE&);
		virtual						~GearJointData()	{}

		JointData*					mHinge0;
		JointData*					mHinge1;
		PR							mLocalPivot0;
		PR							mLocalPivot1;
		float						mGearRatio;
//		float						mRadius0;
//		float						mRadius1;
//		float						mErrorSign;
	};

	class RackJointData : public JointData
	{
		public:
									RackJointData(const PINT_RACK_AND_PINION_JOINT_CREATE&);
		virtual						~RackJointData()	{}

		JointData*					mHinge;
		JointData*					mPrismatic;
		PR							mLocalPivot0;
		PR							mLocalPivot1;
		udword						mNbRackTeeth;
		udword						mNbPinionTeeth;
		float						mRackLength;
//		float						mErrorSign;
	};

	class AggregateData : public BaseData
	{
		public:
									AggregateData(udword max_size, bool enable_self_collision);
		virtual						~AggregateData()	{}

		PtrContainer				mActors;			// ActorData*

		udword						mMaxSize;
		bool						mSelfCollision;
		bool						mAddedToScene;
	};

	class CharacterData : public BaseData
	{
		public:
									CharacterData();
		virtual						~CharacterData()	{}
	};

	class VehicleData : public BaseData
	{
		public:
									VehicleData();
		virtual						~VehicleData()	{}
	};

#endif
