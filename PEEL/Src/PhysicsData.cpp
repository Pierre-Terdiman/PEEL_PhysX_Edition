///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhysicsData.h"

CollisionGroupsData::CollisionGroupsData(udword nb_groups, const PintDisabledGroups* groups)
{
	mNbGroups	= nb_groups;
	mGroups		= ICE_NEW(PintDisabledGroups)[nb_groups];
	CopyMemory(mGroups, groups, sizeof(PintDisabledGroups)*nb_groups);
}

CollisionGroupsData::~CollisionGroupsData()
{
	DELETEARRAY(mGroups);
}

SceneData::SceneData(const PINT_WORLD_CREATE& desc, const char* scene_desc) :
	mGlobalBounds				(desc.mGlobalBounds),
	mGravity					(desc.mGravity),
	mNbSimulateCallsPerFrame	(desc.mNbSimulateCallsPerFrame),
	mTimestep					(desc.mTimestep),
	mCreateDefaultEnvironment	(desc.mCreateDefaultEnvironment)
{
	CopyMemory(mCamera, desc.mCamera, PINT_MAX_CAMERA_POSES*sizeof(PintCameraPose));
	if(desc.GetTestName())
		mName.Set(desc.GetTestName());
//	if(desc.GetTestDesc())
//		mDesc.Set(desc.GetTestDesc());
	if(scene_desc)
		mDesc.Set(scene_desc);
}

ActorData::ActorData(const PINT_OBJECT_CREATE& desc) :
	mPosition		(desc.mPosition),
	mRotation		(desc.mRotation),
	mCOMLocalOffset	(desc.mCOMLocalOffset),
	mLinearVelocity	(desc.mLinearVelocity),
	mAngularVelocity(desc.mAngularVelocity),
	mMass			(desc.mMass),
	mMassForInertia	(desc.mMassForInertia),
	mCollisionGroup	(desc.mCollisionGroup),
	mKinematic		(desc.mKinematic),
	mAddToWorld		(desc.mAddToWorld)
{
	if(desc.mName)
		mName.Set(desc.mName);
}

MaterialData::MaterialData(const PINT_MATERIAL_CREATE& desc) :
	mStaticFriction		(desc.mStaticFriction),
	mDynamicFriction	(desc.mDynamicFriction),
	mRestitution		(desc.mRestitution)
{
	if(desc.mName)
		mName.Set(desc.mName);
}

ShapeData::ShapeData(const PINT_SHAPE_CREATE& desc) :
	mType		(desc.mType),
	mLocalPos	(desc.mLocalPos),
	mLocalRot	(desc.mLocalRot),
	mMaterial	(null),
	mRenderer	(desc.mRenderer)
{
	if(desc.mName)
		mName.Set(desc.mName);
}

SphereShapeData::SphereShapeData(const PINT_SPHERE_CREATE& desc) :
	ShapeData	(desc),
	mRadius		(desc.mRadius)
{
}

CapsuleShapeData::CapsuleShapeData(const PINT_CAPSULE_CREATE& desc) :
	ShapeData	(desc),
	mRadius		(desc.mRadius),
	mHalfHeight	(desc.mHalfHeight)
{
}

CylinderShapeData::CylinderShapeData(const PINT_CYLINDER_CREATE& desc) :
	ShapeData	(desc),
	mRadius		(desc.mRadius),
	mHalfHeight	(desc.mHalfHeight)
{
}

BoxShapeData::BoxShapeData(const PINT_BOX_CREATE& desc) :
	ShapeData	(desc),
	mExtents	(desc.mExtents)
{
}

ConvexShapeData::ConvexShapeData(const PINT_CONVEX_CREATE& desc) :
	ShapeData	(desc)
{
	mNbVerts	= desc.mNbVerts;
	mVerts		= ICE_NEW(Point)[desc.mNbVerts];
	CopyMemory(mVerts, desc.mVerts, desc.mNbVerts*sizeof(Point));
}

ConvexShapeData::~ConvexShapeData()
{
	DELETEARRAY(mVerts);
}

MeshShapeData::MeshShapeData(const PINT_MESH_CREATE& desc) :
	ShapeData	(desc)
{
	const SurfaceInterface& SI = desc.GetSurface();

	mNbVerts	= SI.mNbVerts;
	mVerts		= ICE_NEW(Point)[SI.mNbVerts];
	CopyMemory(mVerts, SI.mVerts, SI.mNbVerts*sizeof(Point));

	mNbTris		= SI.mNbFaces;
	mTris		= ICE_NEW(IndexedTriangle)[SI.mNbFaces];
	ASSERT(SI.mDFaces);
	ASSERT(!SI.mWFaces);
	CopyMemory(mTris, SI.mDFaces, SI.mNbFaces*sizeof(IndexedTriangle));
}

MeshShapeData::~MeshShapeData()
{
	DELETEARRAY(mTris);
	DELETEARRAY(mVerts);
}

MeshShapeData2::MeshShapeData2(const PINT_MESH_CREATE2& desc) :
	ShapeData	(desc),
	mMeshData	(null)
{
	mMeshData = reinterpret_cast<MeshData*>(desc.mTriangleMesh);
}

MeshShapeData2::~MeshShapeData2()
{
}

//

ConvexMeshData::ConvexMeshData(const PINT_CONVEX_DATA_CREATE& desc)
{
	mNbVerts	= desc.mNbVerts;
	mVerts		= ICE_NEW(Point)[desc.mNbVerts];
	CopyMemory(mVerts, desc.mVerts, desc.mNbVerts*sizeof(Point));
}

ConvexMeshData::~ConvexMeshData()
{
	DELETEARRAY(mVerts);
}

MeshData::MeshData(const PINT_MESH_DATA_CREATE& desc)
{
	const SurfaceInterface& SI = desc.GetSurface();

	mNbVerts	= SI.mNbVerts;
	mVerts		= ICE_NEW(Point)[SI.mNbVerts];
	CopyMemory(mVerts, SI.mVerts, SI.mNbVerts*sizeof(Point));

	mNbTris		= SI.mNbFaces;
	mTris		= ICE_NEW(IndexedTriangle)[SI.mNbFaces];
	ASSERT(SI.mDFaces);
	ASSERT(!SI.mWFaces);
	CopyMemory(mTris, SI.mDFaces, SI.mNbFaces*sizeof(IndexedTriangle));
}

MeshData::~MeshData()
{
	DELETEARRAY(mTris);
	DELETEARRAY(mVerts);
}

//

JointData::JointData(const PINT_JOINT_CREATE& desc) :
	mType		(desc.mType),
	mObject0	((ActorData*)desc.mObject0),
	mObject1	((ActorData*)desc.mObject1)
{
	if(desc.mName)
		mName.Set(desc.mName);
}

SphericalJointData::SphericalJointData(const PINT_SPHERICAL_JOINT_CREATE& desc) :
	JointData		(desc),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1),
	mLimits			(desc.mLimits)
{
}

FixedJointData::FixedJointData(const PINT_FIXED_JOINT_CREATE& desc) :
	JointData		(desc),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1)
{
}

DistanceJointData::DistanceJointData(const PINT_DISTANCE_JOINT_CREATE& desc) :
	JointData		(desc),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1),
	mLimits			(desc.mLimits)
{
}

PrismaticJointData::PrismaticJointData(const PINT_PRISMATIC_JOINT_CREATE& desc) :
	JointData		(desc),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1),
	mLocalAxis0		(desc.mLocalAxis0),
	mLocalAxis1		(desc.mLocalAxis1),
	mLimits			(desc.mLimits),
	mSpring			(desc.mSpring)
{
}

HingeJointData::HingeJointData(const PINT_HINGE_JOINT_CREATE& desc) :
	JointData		(desc),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1),
	mLocalAxis0		(desc.mLocalAxis0),
	mLocalAxis1		(desc.mLocalAxis1),
	mLimits			(desc.mLimits),
	mGlobalAnchor	(desc.mGlobalAnchor),
	mGlobalAxis		(desc.mGlobalAxis),
	mUseMotor		(desc.mUseMotor),
	mDriveVelocity	(desc.mDriveVelocity)
{
}

Hinge2JointData::Hinge2JointData(const PINT_HINGE2_JOINT_CREATE& desc) :
	JointData		(desc),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1),
	mLimits			(desc.mLimits),
	mUseMotor		(desc.mUseMotor),
	mDriveVelocity	(desc.mDriveVelocity)
{
}

D6JointData::D6JointData(const PINT_D6_JOINT_CREATE& desc) :
	JointData		(desc),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1),
	mLinearLimits	(desc.mLinearLimits),
	mMinTwist		(desc.mMinTwist),
	mMaxTwist		(desc.mMaxTwist),
	mMaxSwingY		(desc.mMaxSwingY),
	mMaxSwingZ		(desc.mMaxSwingZ),
	mMotorFlags		(desc.mMotorFlags),
	mMotorStiffness	(desc.mMotorStiffness),
	mMotorDamping	(desc.mMotorDamping)
{
}

GearJointData::GearJointData(const PINT_GEAR_JOINT_CREATE& desc) :
	JointData		(desc),
	mHinge0			((JointData*)desc.mHinge0),
	mHinge1			((JointData*)desc.mHinge1),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1),
	mGearRatio		(desc.mGearRatio)
//	mRadius0		(desc.mRadius0),
//	mRadius1		(desc.mRadius1)
//	mErrorSign		(desc.mErrorSign)
{
}

RackJointData::RackJointData(const PINT_RACK_AND_PINION_JOINT_CREATE& desc) :
	JointData		(desc),
	mHinge			((JointData*)desc.mHinge),
	mPrismatic		((JointData*)desc.mPrismatic),
	mLocalPivot0	(desc.mLocalPivot0),
	mLocalPivot1	(desc.mLocalPivot1),
	mNbRackTeeth	(desc.mNbRackTeeth),
	mNbPinionTeeth	(desc.mNbPinionTeeth),
	mRackLength		(desc.mRackLength)
//	mErrorSign		(desc.mErrorSign)
{
}

AggregateData::AggregateData(udword max_size, bool enable_self_collision) :
	mMaxSize		(max_size),
	mSelfCollision	(enable_self_collision),
	mAddedToScene	(false)
{
}

CharacterData::CharacterData()
{
}

VehicleData::VehicleData()
{
}

