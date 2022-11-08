///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "..\Pint.h"
#include "..\PintShapeRenderer.h"
#include "PINT_Common.h"
#include "PINT_CommonPhysX3.h"
#include "PINT_CommonPhysX5_Fluid.h"

#if PHYSX_SUPPORT_CUSTOM_GEOMETRY
//	#include "PINT_CommonPhysX5_CustomGeom.h"
	#include "extensions/PxCustomGeometryExt.h"
#endif

#pragma warning(disable:4355)	// 'this' : used in base member initializer list

#ifdef PHYSX_SUPPORT_CHARACTERS
	#error obsolete codepath
#endif

#if PHYSX_SUPPORT_RAYCAST_CCD
	#include "PxRaycastCCD.h"
#endif

#if PHYSX_SUPPORT_GEAR_JOINT
	#include "PxGearJoint.h"
#endif

#if PHYSX_SUPPORT_RACK_JOINT
	#include "PxRackAndPinionJoint.h"
#endif

#if PHYSX_SUPPORT_CHAIN_JOINT
	#include "PxChainJoint.h"
#endif

#if PHYSX_SUPPORT_PORTAL_JOINT
	#include "PxPortalJoint.h"
#endif

bool gTest = false;

static	const	bool	gEnableCollisionBetweenJointed	= false;
//extern PEEL_PhysX3_AllocatorCallback* gDefaultAllocator;

using namespace PhysX3;

extern udword gNbPhysXErrors;

#ifdef TEST_FLUIDS
extern PxPBDParticleSystem* gParticleSystem;
#endif

///////////////////////////////////////////////////////////////////////////////

static const char* gDebugVizNames[] =
{
	"Enable debug visualization",
	"Visualize body axes",
	"Visualize body mass axes",
	"Visualize linear velocity",
	"Visualize angular velocity",
	"Visualize contact points",
	"Visualize contact normals",
	"Visualize actor axes",
	"Visualize collision AABBs",
	"Visualize collision shapes",
	"Visualize collision axes",
	"Visualize collision compounds",
	"Visualize collision face normals",
	"Visualize collision edges",
	"Visualize collision statics",
	"Visualize collision dynamics",
	"Visualize joint local frames",
	"Visualize joint limits",
#ifndef IS_PHYSX_3_2
	"Visualize MBP regions",
#endif
};

static PxVisualizationParameter::Enum gDebugVizIndex[] =
{
	PxVisualizationParameter::eSCALE,
	PxVisualizationParameter::eBODY_AXES,
	PxVisualizationParameter::eBODY_MASS_AXES,
	PxVisualizationParameter::eBODY_LIN_VELOCITY,
	PxVisualizationParameter::eBODY_ANG_VELOCITY,
	PxVisualizationParameter::eCONTACT_POINT,
	PxVisualizationParameter::eCONTACT_NORMAL,
	PxVisualizationParameter::eACTOR_AXES,
	PxVisualizationParameter::eCOLLISION_AABBS,
	PxVisualizationParameter::eCOLLISION_SHAPES,
	PxVisualizationParameter::eCOLLISION_AXES,
	PxVisualizationParameter::eCOLLISION_COMPOUNDS,
	PxVisualizationParameter::eCOLLISION_FNORMALS,
	PxVisualizationParameter::eCOLLISION_EDGES,
	PxVisualizationParameter::eCOLLISION_STATIC,
	PxVisualizationParameter::eCOLLISION_DYNAMIC,
	PxVisualizationParameter::eJOINT_LOCAL_FRAMES,
	PxVisualizationParameter::eJOINT_LIMITS,
#ifndef IS_PHYSX_3_2
	PxVisualizationParameter::eMBP_REGIONS,
#endif
};

static const udword gNbDebugVizParams = ICE_ARRAYSIZE(gDebugVizIndex);

static bool gDebugVizParams[gNbDebugVizParams] = {0};

bool PhysX3::IsDebugVizEnabled(){ return gDebugVizParams[0];}

///////////////////////////////////////////////////////////////////////////////

MemoryOutputStream::MemoryOutputStream(PEEL_PhysX3_AllocatorCallback* allocator) :
	mCallback	(allocator),
	mData		(NULL),
	mSize		(0),
	mCapacity	(0)
{
}

MemoryOutputStream::~MemoryOutputStream()
{
	if(mData)
	{
		if(mCallback)	mCallback->deallocate(mData);
		else			delete[] mData;
	}
}

PxU32 MemoryOutputStream::write(const void* src, PxU32 size)
{
	const PxU32 expectedSize = mSize + size;
	if(expectedSize > mCapacity)
	{
//		mCapacity = expectedSize + 4096;
		mCapacity = NextPowerOfTwo(expectedSize);
		PxU8* newData = mCallback ? (PxU8*)mCallback->allocate(mCapacity, null, null, 0) : new PxU8[mCapacity];
		PX_ASSERT(newData!=NULL);

		if(newData)
		{
			memcpy(newData, mData, mSize);
			if(mCallback)	mCallback->deallocate(mData);
			else			delete[] mData;
		}
		mData = newData;
	}
	memcpy(mData+mSize, src, size);
	mSize += size;
	return size;
}

///////////////////////////////////////////////////////////////////////////////

MemoryInputData::MemoryInputData(PxU8* data, PxU32 length) :
	mSize	(length),
	mData	(data),
	mPos	(0)
{
}

PxU32 MemoryInputData::read(void* dest, PxU32 count)
{
	PxU32 length = PxMin<PxU32>(count, mSize-mPos);
	memcpy(dest, mData+mPos, length);
	mPos += length;
	return length;
}

PxU32 MemoryInputData::getLength() const
{
	return mSize;
}

void MemoryInputData::seek(PxU32 offset)
{
	mPos = PxMin<PxU32>(mSize, offset);
}

PxU32 MemoryInputData::tell() const
{
	return mPos;
}

///////////////////////////////////////////////////////////////////////////////

MotorData::MotorData()
{
}

MotorData::~MotorData()
{
}

///////////////////////////////////////////////////////////////////////////////

static const PxQuat q = PxShortestRotation(PxVec3(0.0f, 1.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.0f));

void PhysX3::ComputeCapsuleTransform(PR& dst, const PR& src)
{
	// ### PhysX is weird with capsules
/*	Matrix3x3 Rot;
	Rot.RotZ(HALFPI);
	Quat QQ = src.mRot * Quat(Rot);*/

	Quat QQ = src.mRot * ToQuat(q);

	dst.mPos = src.mPos;
	dst.mRot = QQ;
}

///////////////////////////////////////////////////////////////////////////////

// PxSetGroup does not work with shared shapes, since they are already attached to the
// actor. This version working on shapes directly should be called before attaching
// the shape to actors.
/*void PhysX3::SetGroup(PxShape& shape, PxU16 collision_group)
{
	// retrieve current group mask
	PxFilterData fd = shape.getSimulationFilterData();
	fd.word0 = collision_group;
	// set new filter data
	shape.setSimulationFilterData(fd);
}*/

///////////////////////////////////////////////////////////////////////////////

PxRigidBody* PhysX3::GetRigidBody(PintActorHandle handle)
{
	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(!RigidActor)
	{
#ifdef DEPRECATED
		PxShape* Shape = GetShapeFromHandle(handle);
		ASSERT(Shape);
	#if PHYSX_SUPPORT_SHARED_SHAPES
		RigidActor = Shape->getActor();
	#else
		RigidActor = &Shape->getActor();
	#endif
#else
		ASSERT(0);
#endif
	}

	PxRigidBody* RigidBody = null;
#ifdef IS_PHYSX_3_2
	if(RigidActor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
	{
		PxRigidDynamic* RigidDynamic = static_cast<PxRigidDynamic*>(RigidActor);
		if(RigidDynamic->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC)
			return null;

		RigidBody = static_cast<PxRigidBody*>(RigidActor);
	}
	else if(RigidActor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		RigidBody = static_cast<PxRigidBody*>(RigidActor);

	return RigidBody;
#else
	if(RigidActor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		RigidBody = static_cast<PxRigidBody*>(RigidActor);
	else if(RigidActor->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		RigidBody = static_cast<PxRigidBody*>(RigidActor);

	if(RigidBody && !(RigidBody->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC))
		return RigidBody;
#endif
	return null;
}

///////////////////////////////////////////////////////////////////////////////

static void SetMassProperties(const PINT_OBJECT_CREATE& desc, PxRigidBody& rigid_body)
{
//	const PxVec3& LocalCOMOffset = ToPxVec3(desc.mCOMLocalOffset);

	const PxVec3 ZeroCOM(0.0f);

	if(desc.mMassForInertia<0.0f)
	{
		//bool status = PxRigidBodyExt::setMassAndUpdateInertia(rigid_body, desc.mMass, &ZeroCOM);
		bool status = PxRigidBodyExt::setMassAndUpdateInertia(rigid_body, desc.mMass/*, desc.mCOMLocalOffset.IsNonZero() ? &LocalCOMOffset : NULL*/);
//		bool status = PxRigidBodyExt::updateMassAndInertia(rigid_body, desc.mMass/*, desc.mCOMLocalOffset.IsNonZero() ? &LocalCOMOffset : NULL*/);
		ASSERT(status);
	}
	else
	{
		//bool status = PxRigidBodyExt::setMassAndUpdateInertia(rigid_body, desc.mMassForInertia, &ZeroCOM);
		bool status = PxRigidBodyExt::setMassAndUpdateInertia(rigid_body, desc.mMassForInertia/*, desc.mCOMLocalOffset.IsNonZero() ? &LocalCOMOffset : NULL*/);
		ASSERT(status);
		rigid_body.setMass(desc.mMass);
	}

	if(desc.mCOMLocalOffset.IsNonZero())
	{
		PxTransform Pose = rigid_body.getCMassLocalPose();
		Pose.p += ToPxVec3(desc.mCOMLocalOffset);
		rigid_body.setCMassLocalPose(Pose);
	}

	if(0)
	{
		PxTransform GlobalPose = rigid_body.getGlobalPose();

		PxTransform Pose = rigid_body.getCMassLocalPose();
		//printf("%f %f %f\n", Pose.p.x, Pose.p.y, Pose.p.z);
		Pose.p = PxVec3(0.0f);
		//Pose.q = GlobalPose.q;
		Pose.q = PxQuat(PxIdentity);
		rigid_body.setCMassLocalPose(Pose);
	}

//	rigid_body.setMassSpaceInertiaTensor(PxVec3(1.0f));
}

///////////////////////////////////////////////////////////////////////////////

//#define CAPTURE_VECTORS
#ifdef CAPTURE_VECTORS
PxU32 gNbLocalVectors = 0;
PxVec3 gLocalVectors[256];

static void CaptureLocalVectors(const PxJoint& joint, const PxVec3* wsAnchor, const PxVec3* axisIn)
{
	PxRigidActor* actors[2];
	joint.getActors(actors[0], actors[1]);

	PxTransform localPose[2];
	for(PxU32 i=0; i<2; i++)
		localPose[i] = PxTransform(PxIdentity);

	// 1) global anchor
	if(wsAnchor)
	{
		//transform anchorPoint to local space
		for(PxU32 i=0; i<2; i++)
			localPose[i].p = actors[i] ? actors[i]->getGlobalPose().transformInv(*wsAnchor) : *wsAnchor;
	}

gLocalVectors[gNbLocalVectors++] = localPose[0].p;
gLocalVectors[gNbLocalVectors++] = localPose[1].p;

	// 2) global axis
	if(axisIn)
	{
		PxVec3 localAxis[2], localNormal[2];

		//find 2 orthogonal vectors.
		//gotta do this in world space, if we choose them
		//separately in local space they won't match up in worldspace.
		PxVec3 axisw = *axisIn;
		axisw.normalize();

		PxVec3 normalw, binormalw;
		::normalToTangents(axisw, binormalw, normalw);
		//because axis is supposed to be the Z axis of a frame with the other two being X and Y, we need to negate
		//Y to make the frame right handed. Note that the above call makes a right handed frame if we pass X --> Y,Z, so 
		//it need not be changed.

		for(PxU32 i=0; i<2; i++)
		{
			if(actors[i])
			{
				const PxTransform& m = actors[i]->getGlobalPose();
				PxMat33 mM(m.q);
				localAxis[i]   = mM.transformTranspose(axisw);
				localNormal[i] = mM.transformTranspose(normalw);
			}
			else
			{
				localAxis[i] = axisw;
				localNormal[i] = normalw;
			}

//			PxVec3 tmp = localAxis[i].cross(localNormal[i]);
//gLocalVectors[gNbLocalVectors++] = localAxis[i];
gLocalVectors[gNbLocalVectors++] = localNormal[i];
//gLocalVectors[gNbLocalVectors++] = tmp;

			PxMat33 rot(localAxis[i], localNormal[i], localAxis[i].cross(localNormal[i]));
			
			localPose[i].q = PxQuat(rot);
			localPose[i].q.normalize();

PxMat33 rot2;
//rot2.column0 = localAxis[i];
//::normalToTangents(localAxis[i], rot2.column1, rot2.column2);
rot2.column1 = localNormal[i];
::normalToTangents(localNormal[i], rot2.column2, rot2.column0);
const PxQuat q0(rot2);

//			const PxVec3 axisCandidate0 = localPose[i].q.rotate(PxVec3(1.0f, 0.0f, 0.0f));
//			const PxVec3 axisCandidate1 = localPose[i].q.rotateInv(PxVec3(1.0f, 0.0f, 0.0f));
//const PxQuat q0 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), axisCandidate0);
//const PxQuat q1 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), axisCandidate1);

//const PxQuat q0 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), rot.column0);
//const PxQuat q1 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), rot.column1);
//const PxQuat q2 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), rot.column2);
int stop=1;

		}
	}
}
#endif

/*static void PxSetJointGlobalFrame(PxJoint& joint, PxRigidActor* actor, const PxVec3& localAnchor, const PxVec3& localAxis, PxU32 i)
{
	PxQuat localRot;
	{
		//find 2 orthogonal vectors.
		//gotta do this in world space, if we choose them
		//separately in local space they won't match up in worldspace.
		PxVec3 axisw = actor ? actor->getGlobalPose().rotate(localAxis) : localAxis;
		axisw.normalize();

		PxVec3 normalw, binormalw;
		::normalToTangents(axisw, binormalw, normalw);

		const PxVec3 localNormal = actor ? actor->getGlobalPose().rotateInv(normalw) : normalw;

		const PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
		localRot = PxQuat(rot);
		localRot.normalize();
	}

	joint.setLocalPose(PxJointActorIndex::Enum(i), PxTransform(localAnchor, localRot));
}*/

static bool HingeNeedsLimits(float minLimitAngle, float maxLimitAngle)
{
	if(minLimitAngle>maxLimitAngle)
		return false;

	// Legacy condition kept for already serialized files
	return (minLimitAngle!=MIN_FLOAT || maxLimitAngle!=MAX_FLOAT);
}

// For hinge joints emulated with D6, motor params are implicit with PxRevoluteJoint
static const float gHingeDriveStiffness = 0.0f;
//static const float gHingeDriveDamping = PX_MAX_F32;
static const float gHingeDriveDamping = 1000.0f;
static const float gHingeDriveForceLimit = PX_MAX_F32;
static const bool gHingeDriveIsAcceleration = false;

PintJointHandle SharedPhysX::CreateJoint(PxPhysics& physics, const PINT_JOINT_CREATE& desc)
{
	const bool use_d6_joint = mParams.mUseD6Joint;
	const bool enable_projection = mParams.mEnableJointProjection;
	const float projection_linear_tolerance = mParams.mProjectionLinearTolerance;
	const float projection_angular_tolerance = mParams.mProjectionAngularTolerance * DEGTORAD;
	const float ContactDistance = GetJointContactDistance(mParams);

	PxRigidActor* actor0 = (PxRigidActor*)desc.mObject0;
	PxRigidActor* actor1 = (PxRigidActor*)desc.mObject1;

//	projection_angular_tolerance *= DEGTORAD;

	PxJoint* CreatedJoint = null;
	bool NeedsExtendedLimits = false;

	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			const PINT_SPHERICAL_JOINT_CREATE& jc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);

			if(0)
			{
				printf("mObject0: %p\n", jc.mObject0);
				printf("mObject1: %p\n", jc.mObject1);
				printf("mLocalPivot0: %f %f %f\n", jc.mLocalPivot0.mPos.x, jc.mLocalPivot0.mPos.y, jc.mLocalPivot0.mPos.z);
				printf("mLocalPivot1: %f %f %f\n", jc.mLocalPivot1.mPos.x, jc.mLocalPivot1.mPos.y, jc.mLocalPivot1.mPos.z);		
			}

			const PxTransform LocalFrame0(ToPxTransform(jc.mLocalPivot0));
			const PxTransform LocalFrame1(ToPxTransform(jc.mLocalPivot1));

			if(use_d6_joint)
			{
				PxD6Joint* j = PxD6JointCreate(physics, actor0, LocalFrame0, actor1, LocalFrame1);
				ASSERT(j);
				if(j)
				{
					CreatedJoint = j;
					j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
					j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
					j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);

					SetupD6Projection(j, enable_projection, projection_linear_tolerance, projection_angular_tolerance);
				}
			}
			else
			{
				CreatedJoint = CreateSphericalJoint(mParams, physics, jc, actor0, actor1, LocalFrame0, LocalFrame1);
			}
		}
		break;

		case PINT_JOINT_HINGE:
		{
			const PINT_HINGE_JOINT_CREATE& jc = static_cast<const PINT_HINGE_JOINT_CREATE&>(desc);

			if(0)
			{
				const PxQuat aq0 = actor0->getGlobalPose().q;
				const PxQuat aq1 = actor1->getGlobalPose().q;
				printf("%f\n", aq0.dot(aq1));
			}
			const PxQuat q0 = ComputeJointQuat(actor0, ToPxVec3(jc.mLocalAxis0));
			const PxQuat q1 = ComputeJointQuat(actor1, ToPxVec3(jc.mLocalAxis1));

//			const float limitContactDistance = 10.0f;
//			const float limitContactDistance = 0.05f;
//			const float limitContactDistance = 270.0f*4.0f*DEGTORAD;
//			const float limitContactDistance = 0.0f;
//			const float limitContactDistance = FLT_MAX;

			const float minLimitAngle = jc.mLimits.mMinValue;
			const float maxLimitAngle = jc.mLimits.mMaxValue;

			const PxJointAngularLimitPair limit(minLimitAngle, maxLimitAngle, ContactDistance);
			const bool IsLimitedHinge = HingeNeedsLimits(minLimitAngle, maxLimitAngle);
			NeedsExtendedLimits = IsLimitedHinge && (minLimitAngle<=-PI || maxLimitAngle>=PI);
//			limit.restitution	= 0.0f;

			if(use_d6_joint)
			{
//				const PxQuat q0 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), ToPxVec3(jc.mLocalAxis0));
//				const PxQuat q1 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), ToPxVec3(jc.mLocalAxis1));

				PxD6Joint* j = PxD6JointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0), q0), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1), q1));
				ASSERT(j);

/*			if(1)
			{
				PxSetJointGlobalFrame(*j, actor0, ToPxVec3(jc.mLocalPivot0), ToPxVec3(jc.mLocalAxis0), 0);
				PxSetJointGlobalFrame(*j, actor1, ToPxVec3(jc.mLocalPivot1), ToPxVec3(jc.mLocalAxis1), 1);
			}*/

				if(j)
				{
					CreatedJoint = j;
					j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);

					if(IsLimitedHinge)
					{
	//					j->setTwistLimitEnabled(true);
						//j->setTwistLimit(bc.mTwistLowerLimit, bc.mTwistUpperLimit);
						j->setTwistLimit(limit);
					j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
					}

					SetupD6Projection(j, enable_projection, projection_linear_tolerance, projection_angular_tolerance);


					if(jc.mUseMotor)
					{
						const PxD6JointDrive drive(gHingeDriveStiffness, gHingeDriveDamping, gHingeDriveForceLimit, gHingeDriveIsAcceleration);
						j->setDrive(PxD6Drive::eTWIST, drive);
	//	virtual void				setDrivePosition(const PxTransform& pose, bool autowake = true)	= 0;
	//					j->setDriveVelocity(PxVec3(0.0f), PxVec3(0.0f, 0.0f, jc.mDriveVelocity));
						j->setDriveVelocity(PxVec3(0.0f), PxVec3(-jc.mDriveVelocity, 0.0f, 0.0f));

//							PxVec3 tmp, tmp2;
//							j->getDriveVelocity(tmp, tmp2);
						//printf("CHECKPOINT %f\n", jc.mDriveVelocity);
//						printf("CHECKPOINT %f\n", tmp2.x);

	/*					j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
						j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, false);

						PxReal v = j->getDriveVelocity();
						PxReal l = j->getDriveForceLimit();
						PxReal r = j->getDriveGearRatio();

						j->setDriveVelocity(jc.mDriveVelocity);
						j->setDriveForceLimit(l);
						j->setDriveGearRatio(r);*/
					}
				}
			}
			else
			{

				// "The axis of the hinge is specified as the direction of the x-axis in the body's joint frame."
				// ### which one??
/*				const PxQuat q0 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), ToPxVec3(jc.mLocalAxis0));
				const PxQuat q1 = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), ToPxVec3(jc.mLocalAxis1));*/

				PxRevoluteJoint* j = PxRevoluteJointCreate(physics,	actor0, PxTransform(ToPxVec3(jc.mLocalPivot0), q0),
																	actor1, PxTransform(ToPxVec3(jc.mLocalPivot1), q1));
				ASSERT(j);

				if(j)
				{
					CreatedJoint = j;
	/*				if(!jc.mGlobalAnchor.IsNotUsed() && !jc.mGlobalAxis.IsNotUsed())
					{
						const PxVec3 GlobalAnchor = ToPxVec3(jc.mGlobalAnchor);
						const PxVec3 GlobalAxis = ToPxVec3(jc.mGlobalAxis);
	#ifdef CAPTURE_VECTORS
						CaptureLocalVectors(*j, &GlobalAnchor, &GlobalAxis);
	#endif
						PxSetJointGlobalFrame(*j, &GlobalAnchor, &GlobalAxis);
					}*/

		/*			if(0)
					{
						PxSetJointGlobalFrame(*j, actor0, ToPxVec3(jc.mLocalPivot0), ToPxVec3(jc.mLocalAxis0), 0);
						PxSetJointGlobalFrame(*j, actor1, ToPxVec3(jc.mLocalPivot1), ToPxVec3(jc.mLocalAxis1), 1);
					}*/


					// ### what about axes?
	//	const PxQuat q = Ps::computeQuatFromNormal(up);
	//	const PxQuat q = Ps::rotationArc(PxVec3(1.0f, 0.0f, 0.0f), up);

					if(0)
					{
						// ### really tedious to setup!
						const PxTransform m0 = actor0->getGlobalPose();
						const PxTransform m1 = actor1->getGlobalPose();
						PxVec3 wsAnchor;
						{
							PxVec3 wp0 = m0.transform(ToPxVec3(jc.mLocalPivot0));
							PxVec3 wp1 = m1.transform(ToPxVec3(jc.mLocalPivot1));
							wsAnchor = (wp0+wp1)*0.5f;
						}
						PxVec3 wsAxis;
						{
							PxVec3 wp0 = m0.rotate(ToPxVec3(jc.mLocalAxis0));
							PxVec3 wp1 = m1.rotate(ToPxVec3(jc.mLocalAxis1));
							wsAxis = (wp0+wp1)*0.5f; 
							wsAxis.normalize();
						}
						PxSetJointGlobalFrame(*j, &wsAnchor, &wsAxis);
					}

					if(IsLimitedHinge)
					{
						j->setLimit(limit);
		//				j->setLimit(PxJointLimitPair(minLimitAngle, maxLimitAngle, TWOPI));
						j->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
					}

					if(enable_projection)
					{
						j->setProjectionLinearTolerance(projection_linear_tolerance);
						j->setProjectionAngularTolerance(projection_angular_tolerance);
					}
	/*
			PxRevoluteJoint* rv = PxRevoluteJointCreate(physics, b0->mBody, PxTransform::createIdentity(), b1->mBody, PxTransform::createIdentity());
			mJoints[i] = rv;
			rv->setConstraintFlags(PxConstraintFlag::ePROJECTION);
			rv->setProjectionLinearTolerance(0.1f);
			if(1)
			{
				PxJointLimitPair limit(-PxPi/2, PxPi/2, 0.05f);
				limit.restitution	= 0.0f;
				limit.lower			= -0.2f;
				limit.upper			= 0.2f;
				rv->setLimit(limit);

				rv->setRevoluteJointFlags(PxRevoluteJointFlag::eLIMIT_ENABLED);
			}

			PxSetJointGlobalFrame(*rv, &globalAnchor, &globalAxis);
	*/

					if(jc.mUseMotor)
					{
						j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
						j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, false);

						PxReal v = j->getDriveVelocity();
						PxReal l = j->getDriveForceLimit();
						PxReal r = j->getDriveGearRatio();

						j->setDriveVelocity(jc.mDriveVelocity);
						j->setDriveForceLimit(l);
						j->setDriveGearRatio(r);

	/*					ASSERT(!j->userData);
						MotorData* MD = ICE_NEW(MotorData);
						j->userData = MD;
						// We store the ptr in a redundant array just to be able to conveniently release the objects when closing.
						mMotorData.push_back(MD);*/
					}

	//				j->getConstraint()->setFlag(PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, true);
				}
			}
		}
		break;

		case PINT_JOINT_HINGE2:
		{
			const PINT_HINGE2_JOINT_CREATE& jc = static_cast<const PINT_HINGE2_JOINT_CREATE&>(desc);

			const PxQuat q0 = ToPxQuat(jc.mLocalPivot0.mRot);
			const PxQuat q1 = ToPxQuat(jc.mLocalPivot1.mRot);

//			const float limitContactDistance = 10.0f;
//			const float limitContactDistance = 0.05f;
//			const float limitContactDistance = 270.0f*4.0f*DEGTORAD;
//			const float limitContactDistance = 0.0f;
//			const float limitContactDistance = FLT_MAX;

			const float minLimitAngle = jc.mLimits.mMinValue;
			const float maxLimitAngle = jc.mLimits.mMaxValue;

			const PxJointAngularLimitPair limit(minLimitAngle, maxLimitAngle, ContactDistance);
			const bool IsLimitedHinge = HingeNeedsLimits(minLimitAngle, maxLimitAngle);
			NeedsExtendedLimits = IsLimitedHinge && (minLimitAngle<=-PI || maxLimitAngle>=PI);
//			limit.restitution	= 0.0f;

			if(use_d6_joint)
			{
				PxD6Joint* j = PxD6JointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0.mPos), q0), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1.mPos), q1));
				ASSERT(j);

				if(j)
				{
					CreatedJoint = j;
					j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);

					if(IsLimitedHinge)
					{
	//					j->setTwistLimitEnabled(true);
						//j->setTwistLimit(bc.mTwistLowerLimit, bc.mTwistUpperLimit);
						j->setTwistLimit(limit);
					j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
					}

					SetupD6Projection(j, enable_projection, projection_linear_tolerance, projection_angular_tolerance);

					if(jc.mUseMotor)
					{
						const PxD6JointDrive drive(gHingeDriveStiffness, gHingeDriveDamping, gHingeDriveForceLimit, gHingeDriveIsAcceleration);
						j->setDrive(PxD6Drive::eTWIST, drive);
						j->setDriveVelocity(PxVec3(0.0f), PxVec3(-jc.mDriveVelocity, 0.0f, 0.0f));
						//printf("CHECKPOINT\n");
					}
				}
			}
			else
			{
				// "The axis of the hinge is specified as the direction of the x-axis in the body's joint frame."
				PxRevoluteJoint* j = PxRevoluteJointCreate(physics,	actor0, PxTransform(ToPxVec3(jc.mLocalPivot0.mPos), q0),
																	actor1, PxTransform(ToPxVec3(jc.mLocalPivot1.mPos), q1));
				ASSERT(j);

				if(j)
				{
					CreatedJoint = j;
					if(IsLimitedHinge)
					{
						j->setLimit(limit);
		//				j->setLimit(PxJointLimitPair(minLimitAngle, maxLimitAngle, TWOPI));
						j->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
					}

					if(enable_projection)
					{
						j->setProjectionLinearTolerance(projection_linear_tolerance);
						j->setProjectionAngularTolerance(projection_angular_tolerance);
					}

					if(jc.mUseMotor)
					{
						j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
						j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, false);

						PxReal v = j->getDriveVelocity();
						PxReal l = j->getDriveForceLimit();
						PxReal r = j->getDriveGearRatio();

						j->setDriveVelocity(jc.mDriveVelocity);
						j->setDriveForceLimit(l);
						j->setDriveGearRatio(r);

		/*					ASSERT(!j->userData);
						MotorData* MD = ICE_NEW(MotorData);
						j->userData = MD;
						// We store the ptr in a redundant array just to be able to conveniently release the objects when closing.
						mMotorData.push_back(MD);*/
					}

		//			j->getConstraint()->setFlag(PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, true);
				}
			}
		}
		break;

		case PINT_JOINT_FIXED:
		{
			const PINT_FIXED_JOINT_CREATE& jc = static_cast<const PINT_FIXED_JOINT_CREATE&>(desc);

			PxTransform pose0(PxIdentity);
			PxTransform pose1(PxIdentity);
			if(actor0)
				pose0 = actor0->getGlobalPose();
			if(actor1)
				pose1 = actor1->getGlobalPose();
//			const PxQuat q = pose0.q * pose1.q.getConjugate();
			const PxQuat q = pose1.q.getConjugate() * pose0.q;

			const PxTransform LocalFrame0(ToPxVec3(jc.mLocalPivot0));
			const PxTransform LocalFrame1(ToPxVec3(jc.mLocalPivot1), q);

			if(use_d6_joint)
			{
//				PxD6Joint* j = PxD6JointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0)), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1)));
				PxD6Joint* j = PxD6JointCreate(physics, actor0, LocalFrame0, actor1, LocalFrame1);
				ASSERT(j);

				if(j)
				{
					CreatedJoint = j;
					SetupD6Projection(j, enable_projection, projection_linear_tolerance, projection_angular_tolerance);
				}
			}
			else
			{
//				PxQuat q(1.1f, -2.2f, 3.3f, 4.4f);
//				q.normalize();
//				PxFixedJoint* j = PxFixedJointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0), q), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1)));
				CreatedJoint = CreateFixedJoint(mParams, physics, jc, actor0, actor1, LocalFrame0, LocalFrame1);
			}
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			const PINT_PRISMATIC_JOINT_CREATE& jc = static_cast<const PINT_PRISMATIC_JOINT_CREATE&>(desc);

			const PxQuat q0 = jc.mLocalAxis0.IsNonZero() ? ComputeJointQuat(actor0, ToPxVec3(jc.mLocalAxis0)) : ToPxQuat(jc.mLocalPivot0.mRot);
			const PxQuat q1 = jc.mLocalAxis1.IsNonZero() ? ComputeJointQuat(actor1, ToPxVec3(jc.mLocalAxis1)) : ToPxQuat(jc.mLocalPivot1.mRot);

			if(1 && use_d6_joint)
			{
				Point LocalPivot0 = jc.mLocalPivot0.mPos;
				Point LocalPivot1 = jc.mLocalPivot1.mPos;

/*if(jc.mMinLimit<=jc.mMaxLimit)
{
	const float Range = jc.mMaxLimit - jc.mMinLimit;
	LocalPivot0.x += Range*0.5f;
//	LocalPivot1.x += Range*0.5f;
}*/

				PxD6Joint* j = PxD6JointCreate(physics,	actor0, PxTransform(ToPxVec3(LocalPivot0), q0),
														actor1, PxTransform(ToPxVec3(LocalPivot1), q1));
				ASSERT(j);

				if(j)
				{
					CreatedJoint = j;
					// We free the X axis because that's the one used/hardcoded in dedicated prismatic joints.
					j->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);

					if(IsPrismaticLimitEnabled(jc.mLimits))
					{
						const float MinLimit = jc.mLimits.mMinValue;
						const float MaxLimit = jc.mLimits.mMaxValue;
	#ifdef NEW_D6_API
						if(1)	// New API
						{
							j->setMotion(PxD6Axis::eX, PxD6Motion::eLIMITED);

	//						const PxJointLinearLimitPair Limits(MinLimit, MaxLimit, PxSpring(jc.mSpringStiffness, jc.mSpringDamping));
							//### these hard joints are better looking against limits but don't support "springs", at least in this form
	//						const PxJointLinearLimitPair Limits(PxTolerancesScale(), MinLimit, MaxLimit, ContactDistance);


							PxJointLinearLimitPair Limits(MinLimit, MaxLimit, PxSpring(jc.mSpring.mStiffness, jc.mSpring.mDamping));
							if(jc.mSpring.mStiffness==0.0f && jc.mSpring.mDamping==0.0f)
							{
								//### these hard joints are better looking against limits but don't support "springs", at least in this form
								Limits = PxJointLinearLimitPair(PxTolerancesScale(), MinLimit, MaxLimit, ContactDistance);
							}


							j->setLinearLimit(PxD6Axis::eX, Limits);
						}
	#endif

	/*					j->setMotion(PxD6Axis::eX, PxD6Motion::eLIMITED);

	PxJointLinearLimit Limit(jc.mMaxLimit*0.5f, PxSpring(jc.mSpringStiffness, jc.mSpringDamping));
	// This is going to be for the only free DOF, i.e. X, i.e. what we want
	j->setLinearLimit(Limit);
	j->setMotion(PxD6Axis::eY, PxD6Motion::eLIMITED);
	*/

	/*
							const PxJointLinearLimitPair Limits(jc.mMinLimit, jc.mMaxLimit, PxSpring(jc.mSpringStiffness, jc.mSpringDamping));
							//### these hard joints are better looking against limits but don't support "springs", at least in this form
		//					const PxJointLinearLimitPair Limits(PxTolerancesScale(), jc.mMinLimit, jc.mMaxLimit);
		////				Limits.contactDistance	= 10.0f;
							j->setLinearLimitX(Limits);

	//							j->setMotion(PxD6Axis::eX, PxD6Motion::eLOCKED);
								j->setMotion(PxD6Axis::eY, PxD6Motion::eLIMITED);
								j->setLinearLimitY(Limits);
	//							j->setMotion(PxD6Axis::eZ, PxD6Motion::eLIMITED);
	//							j->setLinearLimitZ(Limits);
	*/

					}

					SetupD6Projection(j, enable_projection, projection_linear_tolerance, projection_angular_tolerance);
				}
			}
			else
			{
				CreatedJoint = CreatePrismaticJoint(mParams, physics, jc, actor0, actor1, PxTransform(ToPxVec3(jc.mLocalPivot0.mPos), q0), PxTransform(ToPxVec3(jc.mLocalPivot1.mPos), q1));
			}
		}
		break;

		case PINT_JOINT_DISTANCE:
		{
			const PINT_DISTANCE_JOINT_CREATE& jc = static_cast<const PINT_DISTANCE_JOINT_CREATE&>(desc);

//#define TEMP_TEST
#ifdef TEMP_TEST
			const PxVec3 LocalAxis(0.0f, 1.0f, 0.0f);
			const PxQuat q0 = ComputeJointQuat(actor0, LocalAxis);
			const PxQuat q1 = ComputeJointQuat(actor1, LocalAxis);
			CreatedJoint = CreateDistanceJoint(mParams, physics, jc, actor0, actor1, PxTransform(ToPxVec3(jc.mLocalPivot0), q0), PxTransform(ToPxVec3(jc.mLocalPivot1), q1));
#else
			CreatedJoint = CreateDistanceJoint(mParams, physics, jc, actor0, actor1, PxTransform(ToPxVec3(jc.mLocalPivot0)), PxTransform(ToPxVec3(jc.mLocalPivot1)));
#endif
		}
		break;

		case PINT_JOINT_D6:
		{
			const PINT_D6_JOINT_CREATE& jc = static_cast<const PINT_D6_JOINT_CREATE&>(desc);

			const PxQuat q0 = ToPxQuat(jc.mLocalPivot0.mRot);
			const PxQuat q1 = ToPxQuat(jc.mLocalPivot1.mRot);

			PxD6Joint* j = PxD6JointCreate(physics,	actor0, PxTransform(ToPxVec3(jc.mLocalPivot0.mPos), q0),
													actor1, PxTransform(ToPxVec3(jc.mLocalPivot1.mPos), q1));
			ASSERT(j);

			if(j)
			{
				CreatedJoint = j;
				// Angular limits
				if(1)
				{
	/*				const Point& AngularLimitsMin = jc.mAngularLimitsMin;
					const Point& AngularLimitsMax = jc.mAngularLimitsMax;

					if(AngularLimitsMin.x>AngularLimitsMax.x)
						j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
					if(AngularLimitsMin.y>AngularLimitsMax.y)
						j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
					if(AngularLimitsMin.z>AngularLimitsMax.z)
						j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);

					if(AngularLimitsMin.x==AngularLimitsMax.x)
						j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLOCKED);
					if(AngularLimitsMin.y==AngularLimitsMax.y)
						j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLOCKED);
					if(AngularLimitsMin.z==AngularLimitsMax.z)
						j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);

					if(AngularLimitsMin.x<AngularLimitsMax.x)
					{
						j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
						const PxJointAngularLimitPair Limits(AngularLimitsMin.x, AngularLimitsMax.x, FLT_MAX);
	//					const PxJointAngularLimitPair Limits(AngularLimitsMin.x, AngularLimitsMax.x);
						j->setTwistLimit(Limits);
					}
					if(AngularLimitsMin.y<AngularLimitsMax.y)
						j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
					if(AngularLimitsMin.z<AngularLimitsMax.z)
						j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);*/

					// Twist
					if(jc.mMinTwist>jc.mMaxTwist)
						j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
					if(jc.mMinTwist==jc.mMaxTwist)
						j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLOCKED);
					if(jc.mMinTwist<jc.mMaxTwist)
					{
						j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLIMITED);
						const PxJointAngularLimitPair Limits(jc.mMinTwist, jc.mMaxTwist, ContactDistance);
	//					const PxJointAngularLimitPair Limits(jc.mMinTwist, jc.mMaxTwist);
						j->setTwistLimit(Limits);
					}

					// Swing
					float MaxSwingY = jc.mMaxSwingY;
					if(MaxSwingY==0.0f)
						j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLOCKED);
					else if(MaxSwingY>0.0f)
						j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLIMITED);
					else if(MaxSwingY<0.0f)
						j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);

					float MaxSwingZ = jc.mMaxSwingZ;
					if(MaxSwingZ==0.0f)
						j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);
					else if(MaxSwingZ>0.0f)
						j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLIMITED);
					else if(MaxSwingZ<0.0f)
						j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);

					if(MaxSwingY>0.0f || MaxSwingZ>0.0f)
					{
						if(MaxSwingY<0.0f)
							MaxSwingY = 0.0f;
						if(MaxSwingZ<0.0f)
							MaxSwingZ = 0.0f;

	//					j->setSwingLimit(PxJointLimitCone(MaxSwingY, MaxSwingZ));
						j->setSwingLimit(PxJointLimitCone(MaxSwingY, MaxSwingZ, ContactDistance));
	//					j->setSwingLimit(PxJointLimitCone(MaxSwingY, MaxSwingZ, 0.0f));
	//					j->setSwingLimit(PxJointLimitCone(MaxSwingY, MaxSwingZ, PxSpring(100.0f, 10.0f)));
					}
				}

				// Linear limits
				if(1)
				{
					const AABB& LinearLimits = jc.mLinearLimits;

					for(udword Index=0;Index<3;Index++)
					{
						const PxD6Axis::Enum Axis = PxD6Axis::Enum(Index);
						const float MinLimit = LinearLimits.mMin[Index];
						const float MaxLimit = LinearLimits.mMax[Index];

						if(MinLimit>MaxLimit)
							j->setMotion(Axis, PxD6Motion::eFREE);
						if(MinLimit==MaxLimit)
							j->setMotion(Axis, PxD6Motion::eLOCKED);
						if(MinLimit<MaxLimit)
						{
							j->setMotion(Axis, PxD6Motion::eLIMITED);
			//				const PxJointLinearLimitPair Limits(jc.mMinLimit, jc.mMaxLimit, PxSpring(jc.mSpringStiffness, jc.mSpringDamping));
							//### these hard joints are better looking against limits but don't support "springs", at least in this form

	#ifdef NEW_D6_API
							//const PxJointLinearLimitPair Limits(MinLimit, MaxLimit, PxSpring(500.0f, 30.0f));
							const PxJointLinearLimitPair Limits(PxTolerancesScale(), MinLimit, MaxLimit, ContactDistance);
							j->setLinearLimit(Axis, Limits);
	#endif
						}
					}
				}


				if(jc.mMotorFlags)
				{
	//					const PxD6JointDrive drive(0.0f, PX_MAX_F32, PX_MAX_F32, false);
	//					const PxD6JointDrive drive(10000.0f, 100.0f, PX_MAX_F32, false);
	//					const PxD6JointDrive drive(10000.0f, 100.0f, PX_MAX_F32, true);
	//				const PxD6JointDrive drive(10000000.0f, 0.0f, PX_MAX_F32, true);
					const PxD6JointDrive drive(jc.mMotorStiffness, jc.mMotorDamping, PX_MAX_F32, true);
	//					const PxD6JointDrive drive(100.0f, 10.0f, PX_MAX_F32, true);
					if(jc.mMotorFlags & PINT_D6_MOTOR_DRIVE_X)
						j->setDrive(PxD6Drive::eX, drive);
					if(jc.mMotorFlags & PINT_D6_MOTOR_DRIVE_Y)
						j->setDrive(PxD6Drive::eY, drive);
					if(jc.mMotorFlags & PINT_D6_MOTOR_DRIVE_Z)
						j->setDrive(PxD6Drive::eZ, drive);
	/*
					j->setDrivePosition(PxTransform(PxVec3(2.0f, 0.0f, 0.0f)));
	//					j->setDrivePosition(PxTransform(PxVec3(0.0f, 0.0f, 0.0f)));
					j->setDriveVelocity(PxVec3(0.0f), PxVec3(0.0f));*/
				}
			}
		}
		break;

#if PHYSX_SUPPORT_GEAR_JOINT
		case PINT_JOINT_GEAR:
		{
			const PINT_GEAR_JOINT_CREATE& jc = static_cast<const PINT_GEAR_JOINT_CREATE&>(desc);

			const PxQuat q0 = ToPxQuat(jc.mLocalPivot0.mRot);
			const PxQuat q1 = ToPxQuat(jc.mLocalPivot1.mRot);

			PxGearJoint* j = PxGearJointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0.mPos), q0), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1.mPos), q1));
			ASSERT(j);

			if(j)
			{
				j->setGearRatio(jc.mGearRatio);
//				j->setRadius0(jc.mRadius0);
//				j->setRadius1(jc.mRadius1);
				//j->setErrorSign(jc.mErrorSign);

				PxJoint* j0 = reinterpret_cast<PxJoint*>(jc.mHinge0);
				PxJoint* j1 = reinterpret_cast<PxJoint*>(jc.mHinge1);
				//TODO: check types here
				PxRevoluteJoint* rev0 = static_cast<PxRevoluteJoint*>(j0);
				PxRevoluteJoint* rev1 = static_cast<PxRevoluteJoint*>(j1);
				j->setHinges(rev0, rev1);

				CreatedJoint = j;
				mGearJoints.push_back(j);
			}
		}
		break;
#endif

#if PHYSX_SUPPORT_RACK_JOINT
		case PINT_JOINT_RACK_AND_PINION:
		{
			const PINT_RACK_AND_PINION_JOINT_CREATE& jc = static_cast<const PINT_RACK_AND_PINION_JOINT_CREATE&>(desc);

			const PxQuat q0 = ToPxQuat(jc.mLocalPivot0.mRot);
			const PxQuat q1 = ToPxQuat(jc.mLocalPivot1.mRot);

			PxRackAndPinionJoint* j = PxRackAndPinionJointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0.mPos), q0), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1.mPos), q1));
			ASSERT(j);

			if(j)
			{
				j->setData(jc.mNbRackTeeth, jc.mNbPinionTeeth, jc.mRackLength);
				//j->setErrorSign(jc.mErrorSign);

				PxJoint* j0 = reinterpret_cast<PxJoint*>(jc.mHinge);
				PxJoint* j1 = reinterpret_cast<PxJoint*>(jc.mPrismatic);
				//TODO: check types here
				PxRevoluteJoint* rev = static_cast<PxRevoluteJoint*>(j0);
				PxPrismaticJoint* prism = static_cast<PxPrismaticJoint*>(j1);
				j->setJoints(rev, prism );


	/*			j->setGearRatio(jc.mGearRatio);
				j->setRadius0(jc.mRadius0);
				j->setRadius1(jc.mRadius1);

				PxJoint* j0 = reinterpret_cast<PxJoint*>(jc.mHinge0);
				PxJoint* j1 = reinterpret_cast<PxJoint*>(jc.mHinge1);
				//TODO: check types here
				PxRevoluteJoint* rev0 = static_cast<PxRevoluteJoint*>(j0);
				PxRevoluteJoint* rev1 = static_cast<PxRevoluteJoint*>(j1);
				j->setHinges(rev0, rev1);*/

				CreatedJoint = j;
				mRackJoints.push_back(j);
			}
		}
		break;
#endif

#if PHYSX_SUPPORT_CHAIN_JOINT
		case PINT_JOINT_CHAIN:
		{
			const PINT_CHAIN_JOINT_CREATE& jc = static_cast<const PINT_CHAIN_JOINT_CREATE&>(desc);

			const PxQuat q0 = ToPxQuat(jc.mLocalPivot0.mRot);
			const PxQuat q1 = ToPxQuat(jc.mLocalPivot1.mRot);

			PxChainJoint* j = PxChainJointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0.mPos), q0), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1.mPos), q1));
			ASSERT(j);

			if(j)
			{
				j->setData(jc.mNbRackTeeth, jc.mNbPinionTeeth, jc.mRackLength);

				PxJoint* j0 = reinterpret_cast<PxJoint*>(jc.mHinge);
				PxJoint* j1 = reinterpret_cast<PxJoint*>(jc.mPrismatic);
				//TODO: check types here
				PxRevoluteJoint* rev = static_cast<PxRevoluteJoint*>(j0);
				PxPrismaticJoint* prism = static_cast<PxPrismaticJoint*>(j1);
				j->setJoints(rev, prism );

				CreatedJoint = j;
				//mRackJoints.push_back(j);
			}
		}
		break;
#endif

#if PHYSX_SUPPORT_PORTAL_JOINT
		case PINT_JOINT_PORTAL:
		{
			const PINT_PORTAL_JOINT_CREATE& jc = static_cast<const PINT_PORTAL_JOINT_CREATE&>(desc);

//			const PxQuat q0 = ToPxQuat(jc.mLocalPivot0.mRot);
//			const PxQuat q1 = ToPxQuat(jc.mLocalPivot1.mRot);

//			PxPortalJoint* j = PxPortalJointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0)), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1)));
			PxPortalJoint* j = PxPortalJointCreate(physics, actor0, PxTransform(ToPxVec3(jc.mLocalPivot0)), actor1, PxTransform(ToPxVec3(jc.mLocalPivot1)));
			ASSERT(j);

			if(j)
			{
				CreatedJoint = j;
				j->setRelativePose(PxTransform(ToPxVec3(jc.mRelPose.mPos), ToPxQuat(jc.mRelPose.mRot)));
			}
		}
		break;
#endif
	}

	if(CreatedJoint)
	{
#if PHYSX_SUPPORT_EXTENDED_LIMITS
		CreatedJoint->setConstraintFlag(PxConstraintFlag::eENABLE_EXTENDED_LIMITS, NeedsExtendedLimits);
#endif
#ifdef PHYSX_SUPPORT_LINEAR_COEFF
		CreatedJoint->getConstraint()->setLinearCoefficient(params.mLinearCoeff);
#endif
		CreatedJoint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, gEnableCollisionBetweenJointed);
		CreatedJoint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
		CreatedJoint->setConstraintFlag(PxConstraintFlag::ePROJECTION, enable_projection);
#if PHYSX_SUPPORT_DISABLE_PREPROCESSING
		const bool disable_preprocessing = mParams.mDisablePreprocessing;
		CreatedJoint->setConstraintFlag(PxConstraintFlag::eDISABLE_PREPROCESSING, disable_preprocessing);
#endif
#ifndef IS_PHYSX_3_2
		const float inverse_inertia_scale = mParams.mInverseInertiaScale;
		const float inverse_mass_scale = mParams.mInverseMassScale;

	#if PHYSX_REMOVE_JOINT_32_COMPATIBILITY
	#else
		const bool enable_32_compatibility = mParams.mEnableJoint32Compatibility;
		CreatedJoint->setConstraintFlag(PxConstraintFlag::eDEPRECATED_32_COMPATIBILITY, enable_32_compatibility);
	#endif
		if(inverse_inertia_scale>=0.0f)
		{
			CreatedJoint->setInvInertiaScale0(inverse_inertia_scale);
			CreatedJoint->setInvInertiaScale1(inverse_inertia_scale);
		}

		if(inverse_mass_scale>=0.0f)
		{
			CreatedJoint->setInvMassScale0(inverse_mass_scale);
			CreatedJoint->setInvMassScale1(inverse_mass_scale);
		}
#endif
		// - Stabilization can create artefacts on jointed objects so we just disable it
		// - Ropes and thin cables can go through each other easily if we limit their depenetration velocity. So we just disable
		// this feature for jointed & articulated objects.
		struct Local
		{
			static void Disable(PxRigidActor* actor)
			{
				if(actor && actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
				{
					PxRigidDynamic* RD = static_cast<PxRigidDynamic*>(actor);
#if PHYSX_SUPPORT_STABILIZATION_FLAG
					RD->setStabilizationThreshold(0.0f);
#endif
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
					RD->setMaxDepenetrationVelocity(MAX_FLOAT);
#endif
				}
			}
		};
		Local::Disable(actor0);
		Local::Disable(actor1);

		SetJointName(CreatedJoint, desc.mName);
	}
	else
	{
		printf("Joint creation failed!\n");
	}

	return PintJointHandle(CreatedJoint);
}

///////////////////////////////////////////////////////////////////////////////

static SharedPhysX* gPhysX = null;

SharedPhysX::SharedPhysX(const EditableParams& params) :
	mFoundation				(null),
	mPhysics				(null),
	mScene					(null),
#if PHYSX_SUPPORT_IMMEDIATE_COOKING
	mCookingParams			(PxTolerancesScale()),
#else
	mCooking				(null),
#endif
	mDefaultMaterial		(null),
#ifdef PHYSX_SUPPORT_CHARACTERS
	mControllerManager		(null),
#endif
#if PHYSX_SUPPORT_HEIGHTFIELDS
	mHFData					(null),
	mHFDataSize				(0),
#endif
	mSceneAPI				(*this),
	mActorAPI				(*this),
	mShapeAPI				(*this),
	mJointAPI				(*this),
#ifdef PHYSX_SUPPORT_CHARACTERS2
	mCCT					(*this),
#endif
#if PHYSX_SUPPORT_SCRATCH_BUFFER
	mScratchPad				(null),
	mScratchPadSize			(0),
#endif
#ifndef IS_PHYSX_3_2
	mInvisibles				(null),
#endif
	mParams					(params),
	mContactNotifThreshold	(FLT_MAX),
	mEnableContactNotif		(false)
{
	gPhysX = this;
#ifdef SHARED_SHAPES_USE_HASH
	mSphereShapes = null;
#endif
}

SharedPhysX::~SharedPhysX()
{
#ifdef SHARED_SHAPES_USE_HASH
	ASSERT(!mSphereShapes);
#endif

#if PHYSX_SUPPORT_HEIGHTFIELDS
	DELETEARRAY(mHFData);
#endif

#ifndef IS_PHYSX_3_2
	ASSERT(!mInvisibles);
#endif
	ASSERT(!mFoundation);
#if !PHYSX_SUPPORT_IMMEDIATE_COOKING
	ASSERT(!mCooking);
#endif
	ASSERT(!mDefaultMaterial);
#ifdef PHYSX_SUPPORT_CHARACTERS
	ASSERT(!mControllerManager);
#endif
	ASSERT(!mPhysics);
	ASSERT(!mScene);
#if PHYSX_SUPPORT_SCRATCH_BUFFER
	ASSERT(!mScratchPad);
#endif
}

/*void SharedPhysX::Close()
{
}*/

static void GetDebugVizOptionsFromGUI();

void SharedPhysX::SharedUpdateFromUI()
{
	if(!mScene)
		return;

//	printf("SharedUpdateFromUI\n");

/*
	Common_GetFromEditBox(gParams.mDebugVizScale, gPhysXUI->mEditBox_DebugVizScale, 0.0f, FLT_MAX);
	for(udword i=0;i<gNbDebugVizParams;i++)
		gDebugVizParams[i] = gPhysXUI->mCheckBox_DebugVis[i]->IsChecked();
*/
// Commented out because it kills the hardcoded per-test debug viz settings in the demo. Not entirely sure it this breaks something.
//	GetDebugVizOptionsFromGUI();

	mScene->setVisualizationParameter(gDebugVizIndex[0], gDebugVizParams[0] ? mParams.mDebugVizScale : 0.0f);

	for(udword i=1;i<gNbDebugVizParams;i++)
		mScene->setVisualizationParameter(gDebugVizIndex[i], float(gDebugVizParams[i]));
}

void SharedPhysX::SetGravity(const Point& gravity)
{
	ASSERT(mScene);
	mScene->setGravity(ToPxVec3(gravity));
}

#if PHYSX_SUPPORT_SCRATCH_BUFFER
static const udword gIndexToScratchSize[] = {
	0,				// Disabled
	16*2*1024,		// 32 Kb
	16*8*1024,		// 128 Kb
	16*16*1024,		// 256 Kb
	16*32*1024,		// 512 Kb
	16*64*1024,		// 1024 Kb
	16*128*1024,	// 2048 Kb
};
#endif

#if PHYSX_SUPPORT_RAYCAST_CCD
static RaycastCCDManager* gRaycastCCDManager = null;
#endif

void SharedPhysX::InitCommon()
{
/*	for(PxU32 i=0;i<64000;i++)
	{
		const float staticFriction = UnitRandomFloat();
		const float dynamicFriction = UnitRandomFloat();
		const float restitution = UnitRandomFloat();
		const PINT_MATERIAL_CREATE Desc(staticFriction, dynamicFriction, restitution);
		CreateMaterial(Desc);
	}*/

	// Create default material
	{
		const PINT_MATERIAL_CREATE Desc(mParams.mDefaultStaticFriction, mParams.mDefaultDynamicFriction, 0.0f);
		mDefaultMaterial = CreateMaterial(Desc);
		ASSERT(mDefaultMaterial);
	}

#if PHYSX_SUPPORT_RAYCAST_CCD
	if(mParams.mEnableRaycastCCDStatic || mParams.mEnableRaycastCCDDynamic)
		gRaycastCCDManager = new RaycastCCDManager(mScene);
#endif

#if PHYSX_SUPPORT_SCRATCH_BUFFER
	udword Size = mParams.mScratchSize;
	if(Size)
	{
		Size = gIndexToScratchSize[Size];
		mScratchPadSize = Size;
		mScratchPad = _aligned_malloc(Size, 16);
	}
#endif
}

void SharedPhysX::CloseCommon()
{
	mDebugVizHelper.Empty();

#ifdef SHARED_SHAPES_USE_HASH
	DELETESINGLE(mSphereShapes);
#endif

#ifndef IS_PHYSX_3_2
	DELETESINGLE(mInvisibles);
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
	DELETESINGLE(gRaycastCCDManager);
#endif

#if PHYSX_SUPPORT_SCRATCH_BUFFER
	if(mScratchPad)
	{
		_aligned_free(mScratchPad);
		mScratchPad = null;
		mScratchPadSize = 0;
	}
#endif

	mActorManager.Reset();

	const udword Size = udword(mMotorData.size());
	for(udword i=0;i<Size;i++)
	{
		MotorData* MD = mMotorData[i];
		DELETESINGLE(MD);
	}
}

#ifndef IS_PHYSX_3_2
void SharedPhysX::CreateCooking(const PxTolerancesScale& scale, PxMeshPreprocessingFlags mesh_preprocess_params)
{
#if !PHYSX_SUPPORT_IMMEDIATE_COOKING
	ASSERT(!mCooking);
#endif
	PxCookingParams Params(scale);
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE
	#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
		Params.midphaseDesc.setToDefault(mParams.mMidPhaseType);
	#else
		Params.midphaseStructure = mParams.mMidPhaseType;
	#endif
#endif
#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
	#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
		if(Params.midphaseDesc.getType() == PxMeshMidPhase::eBVH33)
		{		
			Params.midphaseDesc.mBVH33Desc.meshCookingHint = mParams.mMeshCookingHint;
		}
		else if(Params.midphaseDesc.getType() == PxMeshMidPhase::eBVH34)
		{			
			if(mParams.mMeshCookingHint==PxMeshCookingHint::eCOOKING_PERFORMANCE)
				Params.midphaseDesc.mBVH34Desc.PHYSX_NUM_PRIMS_PER_LEAF = 15;
			else
				Params.midphaseDesc.mBVH34Desc.PHYSX_NUM_PRIMS_PER_LEAF = 4;
//Params.midphaseDesc.mBVH34Desc.PHYSX_NUM_PRIMS_PER_LEAF = 2;
		}
	#else
		Params.meshCookingHint = mParams.mMeshCookingHint;
	#endif
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
	if(Params.midphaseDesc.getType() == PxMeshMidPhase::eBVH34)
		Params.midphaseDesc.mBVH34Desc.PHYSX_NUM_PRIMS_PER_LEAF = mParams.mNbTrisPerLeaf;
#else
	#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
	Params.meshCookingHint = mParams.mMeshCookingHint;
	#endif
#endif
#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
	// Starting with PhysX 5 the BVH34 midphase can be either quantized or not.
	#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
		// This only makes sense for the latest BVH34 API.
		if(Params.midphaseDesc.getType() == PxMeshMidPhase::eBVH34)
			Params.midphaseDesc.mBVH34Desc.quantized = mParams.mQuantizedTrees;
	#endif
#endif
#if PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
	if(Params.midphaseDesc.getType() == PxMeshMidPhase::eBVH34)
		Params.midphaseDesc.mBVH34Desc.buildStrategy = mParams.mMeshBuildStrategy;
#endif

//	Params.buildTriangleAdjacencies = false;

#if PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE
	Params.meshPreprocessParams = mParams.mPrecomputeActiveEdges ? PxMeshPreprocessingFlags(0) : PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
#else
	Params.meshPreprocessParams = PxMeshPreprocessingFlags(0);
#endif

	if(mParams.mPCM)
	{
		Params.meshWeldTolerance = 0.001f;
//		Params.meshPreprocessParams |= PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES | PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES | PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES);
//		Params.meshPreprocessParams |= PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
		Params.meshPreprocessParams |= mesh_preprocess_params;
	}

#if PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT
	Params.gaussMapLimit = mParams.mGaussMapLimit;
#endif

#ifdef PHYSX_SUPPORT_GPU
	if(mParams.mUseGPU)
		Params.BUILD_GPU_DATA = true;
#endif

	//printf("Params.meshPreprocessParams: %d\n", Params.meshPreprocessParams);

#if PHYSX_SUPPORT_IMMEDIATE_COOKING
	mCookingParams = Params;
#else
	#ifdef USE_LOAD_LIBRARY
		mCooking = (func2)(PX_PHYSICS_VERSION, *mFoundation, Params);
	#else
		mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, Params);
	#endif
		ASSERT(mCooking);
#endif
}
#endif

void SharedPhysX::SetupArticulationLink(PxArticulationLink& link, const PINT_OBJECT_CREATE& desc)
{
	// - solver iteration counts are set on the articulation itself
	// - sleep threshold is set on the articulation itself
	// - articulations don't support linear/angular damping
	// - articulations don't support velocity limits

#if PHYSX_SUPPORT_ARTICULATIONS
	link.setLinearVelocity(ToPxVec3(desc.mLinearVelocity));
	link.setAngularVelocity(ToPxVec3(desc.mAngularVelocity));
#endif
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
	// Ropes and thin cables can go through each other easily if we limit their depenetration velocity. So we just disable
	// this feature for jointed & articulated objects.
//	link.setMaxDepenetrationVelocity(mParams.mMaxDepenVelocity);
	link.setMaxDepenetrationVelocity(MAX_FLOAT);
//	PxReal mdp = link.getMaxDepenetrationVelocity();
//	printf("MDP: %f\n", mdp);
#endif

	::SetMassProperties(desc, link);

//rigidDynamic.setMass(desc.mMass);
//rigidDynamic.setMassSpaceInertiaTensor(PxVec3(1.0f, 1.0f, 1.0f));
//	printf("Mass: %f\n", rigidDynamic.getMass());
//	const PxVec3 msit = rigidDynamic.getMassSpaceInertiaTensor();
//	printf("MSIT: %f %f %f\n", msit.x, msit.y, msit.z);

#ifdef IS_PHYSX_3_2
//	if(!mParams.mEnableSleeping)
//		link.wakeUp(9999999999.0f);
//	link.setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, desc.mKinematic);
#else
	link.setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, mParams.mEnableCCD);
	link.setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, desc.mKinematic);
#endif
#if PHYSX_SUPPORT_ANGULAR_CCD
	link.setRigidBodyFlag(PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, mParams.mEnableAngularCCD);
#endif
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
	link.setRigidBodyFlag(PxRigidBodyFlag::eENABLE_GYROSCOPIC_FORCES, mParams.mGyro);
#endif

#if PHYSX_SUPPORT_RCA
	// Thse functions didn't exist for older articulations
	link.setLinearDamping(mParams.mLinearDamping);
	link.setAngularDamping(mParams.mAngularDamping);
	link.setMaxAngularVelocity(mParams.mMaxAngularVelocity);
#endif
#if PHYSX_SUPPORT_RCA_CFM_SCALE
	link.setCfmScale(mParams.mRCACfmScale);
#endif
}

void SharedPhysX::SetupDynamic(PxRigidDynamic& rigidDynamic, const PINT_OBJECT_CREATE& desc)
{
	rigidDynamic.setLinearDamping(mParams.mLinearDamping);
	rigidDynamic.setAngularDamping(mParams.mAngularDamping);
	if(!desc.mKinematic)
	{
		rigidDynamic.setLinearVelocity(ToPxVec3(desc.mLinearVelocity));
		rigidDynamic.setAngularVelocity(ToPxVec3(desc.mAngularVelocity));
	}
	rigidDynamic.setMaxAngularVelocity(mParams.mMaxAngularVelocity);
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
	rigidDynamic.setMaxDepenetrationVelocity(mParams.mMaxDepenVelocity);
#endif
//	printf("%f\n", rigidDynamic.getSleepThreshold());
	rigidDynamic.setSleepThreshold(mParams.mSleepThreshold);
#if PHYSX_SUPPORT_STABILIZATION_FLAG
//	float st = rigidDynamic.getStabilizationThreshold();	// 0.0025
	rigidDynamic.setStabilizationThreshold(mParams.mStabilizationThreshold);
#endif
	if(!desc.mKinematic)
		::SetMassProperties(desc, rigidDynamic);

//rigidDynamic.setMass(desc.mMass);
//rigidDynamic.setMassSpaceInertiaTensor(PxVec3(1.0f, 1.0f, 1.0f));
//	printf("Mass: %f\n", rigidDynamic.getMass());
//	const PxVec3 msit = rigidDynamic.getMassSpaceInertiaTensor();
//	printf("MSIT: %f %f %f\n", msit.x, msit.y, msit.z);

	rigidDynamic.setSolverIterationCounts(mParams.mSolverIterationCountPos, mParams.mSolverIterationCountVel);

#ifdef IS_PHYSX_3_2
	if(!mParams.mEnableSleeping)
		rigidDynamic.wakeUp(9999999999.0f);
	rigidDynamic.setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, desc.mKinematic);
#else
	if(!desc.mKinematic)	// CCD is not supported on kinematics!
		rigidDynamic.setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, mParams.mEnableCCD);
	rigidDynamic.setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, desc.mKinematic);
#endif

#if PHYSX_SUPPORT_ANGULAR_CCD
	rigidDynamic.setRigidBodyFlag(PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, mParams.mEnableAngularCCD);
#endif

#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
	rigidDynamic.setRigidBodyFlag(PxRigidBodyFlag::eENABLE_GYROSCOPIC_FORCES, mParams.mGyro);
#endif

#if PHYSX_SUPPORT_RAYCAST_CCD
	if(gRaycastCCDManager)
	{
		PxShape* S;
		rigidDynamic.getShapes(&S, 1);
		gRaycastCCDManager->registerRaycastCCDObject(&rigidDynamic, S);
	}
#endif

//	rigidDynamic.setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);

#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
	if(mEnableContactNotif)
		rigidDynamic.setContactReportThreshold(mContactNotifThreshold);
	if(mParams.mEnableContactNotif)
		rigidDynamic.setContactReportThreshold(mParams.mContactNotifThreshold);
#endif

/*	if(0 && desc.mName && strstr(desc.mName, "wheel")!=null)
	{
		printf("Found wheel: %s\n", desc.mName);
		rigidDynamic.setContactSlopCoefficient(0.05f);
		//rigidDynamic.setContactSlopCoefficient(0.5f);
		//rigidDynamic.setContactSlopCoefficient(5.0f);
	}*/
}

void SharedPhysX::ApplyLocalTorques()
{
	const udword Size = udword(mLocalTorques.size());
	for(udword j=0;j<Size;j++)
	{
		const LocalTorque& Current = mLocalTorques[j];

		PxRigidBody* RigidBody = GetRigidBody(Current.mHandle);
		if(RigidBody)
		{
			const PxVec3 GlobalTorque = RigidBody->getGlobalPose().rotate(ToPxVec3(Current.mLocalTorque));
//			RigidBody->addTorque(GlobalTorque, PxForceMode::eFORCE, true);
			RigidBody->addTorque(GlobalTorque, PxForceMode::eACCELERATION, true);
//			RigidBody->addTorque(GlobalTorque, PxForceMode::eVELOCITY_CHANGE, true);
//			RigidBody->addTorque(GlobalTorque, PxForceMode::eIMPULSE, true);
		}
	}
}

void SharedPhysX::UpdateJointErrors()
{
#if PHYSX_SUPPORT_GEAR_JOINT_OLD
	if(1)
	{
		const udword NbGearJoints = udword(mGearJoints.size());
		for(udword k=0;k<NbGearJoints;k++)
		{
			PxGearJoint* j = mGearJoints[k];
			j->updateError();
		}
	}
#endif
#if PHYSX_SUPPORT_RACK_JOINT_OLD
	if(1)
	{
		const udword NbRackJoints = udword(mRackJoints.size());
		for(udword k=0;k<NbRackJoints;k++)
		{
			PxRackAndPinionJoint* j = mRackJoints[k];
			j->updateError();
		}
	}
#endif
}

//#define CAPTURE_VECTORS
#ifdef CAPTURE_VECTORS
extern PxU32 gNbLocalVectors;
extern PxVec3 gLocalVectors[256];
#endif

void SharedPhysX::UpdateCommon(float dt)
{
	mActorManager.UpdateTimestamp();

#ifdef CAPTURE_VECTORS
	SaveAsSource("D://tmp//LocalVectors.cpp", "LocalVectors", gLocalVectors, gNbLocalVectors*sizeof(PxVec3), gNbLocalVectors*sizeof(PxVec3), PACK_NONE);
#endif

	if(mScene)
	{
		if(1)
		{
#ifdef TEST_FLUIDS
			if(gParticleSystem)
				onBeforeRenderParticles(gParticleSystem);
#endif

			const udword NbSubsteps = mParams.mNbSubsteps;
			const float sdt = dt/float(NbSubsteps);
			for(udword i=0;i<NbSubsteps;i++)
			{
				if(NbSubsteps>1)
					ApplyLocalTorques();

#if PHYSX_SUPPORT_SQ_UPDATE_MODE
				const bool IsLastSubstep = i==NbSubsteps-1;
				mScene->setSceneQueryUpdateMode(IsLastSubstep ? PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_ENABLED : PxSceneQueryUpdateMode::eBUILD_DISABLED_COMMIT_DISABLED);
#endif
				mScene->simulate(sdt, null, GetScratchPad(), GetScratchPadSize());
				mScene->fetchResults(true);
//				mScene->fetchResults(false);
#if TEST_FLUIDS
				mScene->fetchResultsParticleSystem();
#endif
				UpdateJointErrors();
			}
		}

/*		mScene->setFlag(PxSceneFlag::eENABLE_MANUAL_QUERY_UPDATE, gSQManualFlushUpdates);
		if(gSQManualFlushUpdates)
			mScene->flushQueryUpdates();*/

/*		if(0)
		{
			PxActor* Actors[2048];
			udword Nb = mScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, Actors, 2048);
			for(udword i=0;i<Nb;i++)
			{
				PxVec3 LinVel = ((PxRigidDynamic*)Actors[i])->getLinearVelocity();
				//LinVel.x = 0.0f;
				LinVel.y = 0.0f;
				//LinVel.z = 0.0f;
				((PxRigidDynamic*)Actors[i])->setLinearVelocity(LinVel);
				//printf("%f\n", LinVel.y);
			}
		}*/

/*		if(0)
		{
			PxActor* Actors[2048];
			udword Nb = mScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, Actors, 2048);
			for(udword i=0;i<Nb;i++)
			{
				PxVec3 AngVel = ((PxRigidDynamic*)Actors[i])->getAngularVelocity();
				AngVel.x = 0.0f;
				AngVel.y = 0.0f;
				AngVel.z = 0.0f;
				((PxRigidDynamic*)Actors[i])->setAngularVelocity(AngVel);
				//printf("%f\n", LinVel.y);
			}
		}*/
	}
	EndCommonUpdate();
}

void SharedPhysX::EndCommonUpdate()
{
	mLocalTorques.clear();

	if(mScene && mParams.mFlushSimulation)
#ifdef IS_PHYSX_3_2
		mScene->flush();
#else
		mScene->flushSimulation();
#endif

#if PHYSX_SUPPORT_RAYCAST_CCD
	if(gRaycastCCDManager)
		gRaycastCCDManager->doRaycastCCD(mParams.mEnableRaycastCCDDynamic);
#endif
	//UpdateJointErrors();
}

void SharedPhysX::SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)
{
	for(udword i=0;i<nb_groups;i++)
		//PxSetGroupCollisionFlag(groups[i].mGroup0, groups[i].mGroup1, false);
		PhysX3_SetGroupCollisionFlag(groups[i].mGroup0, groups[i].mGroup1, false);
}

PintActorHandle SharedPhysX::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	const udword NbShapes = desc.GetNbShapes();
	if(!NbShapes)
		return null;

	ASSERT(mPhysics);
	ASSERT(mScene);

	const PxTransform pose(ToPxVec3(desc.mPosition), ToPxQuat(desc.mRotation));
	ASSERT(pose.isValid());

	PxRigidActor* actor;
	PxRigidDynamic* rigidDynamic = null;
	if(desc.mMass!=0.0f)
	{
		rigidDynamic = mPhysics->createRigidDynamic(pose);
		ASSERT(rigidDynamic);
		actor = rigidDynamic;
//		mDynamicActors.push_back(rigidDynamic);

		// Need to setup kine flag before adding shapes, to enable kinematic meshes
#ifdef IS_PHYSX_3_2
		rigidDynamic->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, desc.mKinematic);
#else
		rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, desc.mKinematic);
#endif
	}
	else
	{
		PxRigidStatic* rigidStatic = mPhysics->createRigidStatic(pose);
		ASSERT(rigidStatic);
		actor = rigidStatic;
//		mStaticActors.push_back(rigidStatic);
	}

	SetActorName(actor, desc.mName);

	CreateShapes(desc, actor, desc.mCollisionGroup, null);

	if(rigidDynamic)
		SetupDynamic(*rigidDynamic, desc);

	// Removed since doesn't work with shared shapes
//	PxSetGroup(*actor, desc.mCollisionGroup);

	if(desc.mAddToWorld)
	{
#if PHYSX_SUPPORT_BVH_STRUCTURE
		if(0)	// Compound pruner test
		{
			PxU32 nbBounds = 0;
			PxBounds3* bounds = PxRigidActorExt::getRigidActorShapeLocalBoundsList(*actor, nbBounds);

			PxBVHDesc bvhDesc;
			bvhDesc.bounds.count	= nbBounds;
			bvhDesc.bounds.data		= bounds;
			bvhDesc.bounds.stride	= sizeof(PxBounds3);

	#if PHYSX_SUPPORT_IMMEDIATE_COOKING
			PxBVH* bvh = PxCreateBVH(bvhDesc, mPhysics->getPhysicsInsertionCallback());
	#else
			PxBVH* bvh = mCooking->createBVH(bvhDesc, mPhysics->getPhysicsInsertionCallback());
	#endif

			PX_FREE(bounds);

			AddActorToScene(actor, bvh);

			bvh->release();
		}
		else
#endif
			AddActorToScene(actor);

		if(rigidDynamic && !desc.mKinematic)
			SetupSleeping(rigidDynamic, mParams.mEnableSleeping);
	}
	else
	{
		if(rigidDynamic && !desc.mKinematic)
		{
			//SetupSleeping(rigidDynamic, mParams.mEnableSleeping);
			if(!mParams.mEnableSleeping)
				rigidDynamic->setWakeCounter(9999999999.0f);
		}
	}

	return CreateHandle(actor);
}

bool SharedPhysX::ReleaseObject(PintActorHandle handle)
{
	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(RigidActor)
	{
#if PHYSX_SUPPORT_RAYCAST_CCD
	#if PHYSX_SUPPORT_RAYCAST_CCD_UNREGISTER
		if(gRaycastCCDManager)
		{
			// ### not great, to revisit
			if(RigidActor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
			{
				PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(RigidActor);

				PxShape* S;
				Dyna->getShapes(&S, 1);

				gRaycastCCDManager->unregisterRaycastCCDObject(Dyna, S);
			}
		}
	#endif
#endif

		RemoveActor(RigidActor);

		// ### what about ConvexRender/etc?
		RigidActor->release();
		return true;
	}

#ifdef DEPRECATED
	PxShape* Shape = GetShapeFromHandle(handle);
	if(Shape)
	{
	#if PHYSX_SUPPORT_SHARED_SHAPES
		// ### this is potentially null
		RigidActor = Shape->getActor();
	#else
		RigidActor = &Shape->getActor();
	#endif
		RemoveActor(RigidActor);
		RigidActor->release();
		return true;
	}
#endif
	ASSERT(0);
	return false;
}

PintJointHandle SharedPhysX::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	ASSERT(mPhysics);
	return CreateJoint(*mPhysics, desc);
}

bool SharedPhysX::ReleaseJoint(PintJointHandle handle)
{
	if(!handle)
		return false;
	PxJoint* j = reinterpret_cast<PxJoint*>(handle);
	j->release();
	return true;
}

bool SharedPhysX::SetDriveEnabled(PintJointHandle handle, bool flag)
{
#ifdef IS_PHYSX_3_2
	return false;
#else
	PxJoint* J = reinterpret_cast<PxJoint*>(handle);
	if(J->getConcreteType()==PxJointConcreteType::eREVOLUTE)
	{
		PxRevoluteJoint* RevoluteJoint = static_cast<PxRevoluteJoint*>(J);
		RevoluteJoint->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, flag);
		return true;
	}
	return false;
#endif
}

bool SharedPhysX::SetDrivePosition(PintJointHandle handle, const PR& pose)
{
#ifdef IS_PHYSX_3_2
	return false;
#else
	PxJoint* J = reinterpret_cast<PxJoint*>(handle);
	if(J->getConcreteType()!=PxJointConcreteType::eD6)
		return false;
	PxD6Joint* D6 = static_cast<PxD6Joint*>(J);

	const PxTransform Pose(ToPxVec3(pose.mPos), ToPxQuat(pose.mRot));
	D6->setDrivePosition(Pose);
	return true;
#endif
}

bool SharedPhysX::SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)
{
#ifdef IS_PHYSX_3_2
	return false;
#else
	PxJoint* J = reinterpret_cast<PxJoint*>(handle);
	if(J->getConcreteType()==PxJointConcreteType::eREVOLUTE)
	{
		PxRevoluteJoint* RevoluteJoint = static_cast<PxRevoluteJoint*>(J);
		// The revolute joint only supports one float so this is going to be clumsy. We use angular.x by convention for now.
		RevoluteJoint->setDriveVelocity(angular.x);
		return true;
	}
	if(J->getConcreteType()==PxJointConcreteType::eD6)
	{
		PxD6Joint* D6 = static_cast<PxD6Joint*>(J);

		// To be consistent with the (necessary) sign change done when we setup the motor but yeah this is bad
		const Point NegAngular = -angular;
		D6->setDriveVelocity(ToPxVec3(linear), ToPxVec3(NegAngular));
		//D6->setDriveVelocity(ToPxVec3(linear), ToPxVec3(angular));
		return true;
	}
	return false;
#endif
}

PR SharedPhysX::GetDrivePosition(PintJointHandle handle)
{
#ifdef IS_PHYSX_3_2
	PR Idt;
	Idt.Identity();
	return Idt;
#else
	const PxJoint* J = reinterpret_cast<const PxJoint*>(handle);
	if(J->getConcreteType()!=PxJointConcreteType::eD6)
		return PR(Idt);

	const PxD6Joint* D6 = static_cast<const PxD6Joint*>(J);
	const PxTransform Pose = D6->getDrivePosition();
	return PR(ToPoint(Pose.p), ToQuat(Pose.q));
#endif
}

/*void SharedPhysX::SetGearJointError(PintJointHandle handle, float error)
{
#if PHYSX_SUPPORT_GEAR_JOINT
	PxGearJoint* J = reinterpret_cast<PxGearJoint*>(handle);
	J->setError(error);
#else
	printf("ERROR: SetGearJointError called but not supported!\n");
#endif
}*/

void SharedPhysX::SetPortalJointRelativePose(PintJointHandle handle, const PR& pose)
{
#if PHYSX_SUPPORT_PORTAL_JOINT
	PxPortalJoint* J = reinterpret_cast<PxPortalJoint*>(handle);
	J->setRelativePose(ToPxTransform(pose));
#else
	printf("ERROR: SetPortalJointRelativePose called but not supported!\n");
#endif
}


PR SharedPhysX::GetWorldTransform(PintActorHandle handle)
{
	PxTransform Pose;

	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(RigidActor)
	{
		Pose = RigidActor->getGlobalPose();
	}
	else
	{
#ifdef DEPRECATED
		PxShape* Shape = GetShapeFromHandle(handle);
		ASSERT(Shape);
	#if PHYSX_SUPPORT_SHARED_SHAPES
		ASSERT(Shape->getActor());
		Pose = PxShapeExt::getGlobalPose(*Shape, *Shape->getActor());
	#else
		Pose = PxShapeExt::getGlobalPose(*Shape);
	#endif
#else
		ASSERT(0);
#endif
	}

	return ToPR(Pose);
}

void SharedPhysX::SetWorldTransform(PintActorHandle handle, const PR& pose)
{
	const PxTransform Pose(ToPxVec3(pose.mPos), ToPxQuat(pose.mRot));

	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(RigidActor)
	{
		RigidActor->setGlobalPose(Pose);
//		mScene->resetFiltering(*RigidActor);
		mActorManager.UpdateTimestamp();
#if PHYSX_SUPPORT_RAYCAST_CCD
		// Need to reset the witness point in raycast CCD manager
	#if PHYSX_SUPPORT_RAYCAST_CCD_UNREGISTER
		if(gRaycastCCDManager)
		{
			// ### not great, to revisit
			if(RigidActor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
			{
				PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(RigidActor);

				PxShape* S;
				Dyna->getShapes(&S, 1);

				gRaycastCCDManager->unregisterRaycastCCDObject(Dyna, S);
				gRaycastCCDManager->registerRaycastCCDObject(Dyna, S);
			}
		}
	#endif
#endif
	}
	else
	{
/*		PxShape* Shape = GetShapeFromHandle(handle);
		ASSERT(Shape);
#if PHYSX_SUPPORT_SHARED_SHAPES
		ASSERT(Shape->getActor());
		Pose = PxShapeExt::getGlobalPose(*Shape, *Shape->getActor());
#else
		Pose = PxShapeExt::getGlobalPose(*Shape);
#endif*/
		ASSERT(0);
	}
}

#ifdef DEPRECATED
bool SharedPhysX::GetBounds(PintActorHandle handle, AABB& bounds)
{
	const PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(RigidActor)
	{
		const PxBounds3 pxBounds = RigidActor->getWorldBounds();
		bounds.mMin = ToPoint(pxBounds.minimum);
		bounds.mMax = ToPoint(pxBounds.maximum);
		return true;
	}
	else
	{
#ifdef DEPRECATED
		PxShape* Shape = GetShapeFromHandle(handle);
		ASSERT(Shape);
		//?????????
//		PxBounds3 pxBounds = Shape->getWorldBounds();
#else
		ASSERT(0);
#endif
	}

//	return PR(ToPoint(Pose.p), ToQuat(Pose.q));
	return false;
}

const char* SharedPhysX::GetName(PintActorHandle handle)
{
	const PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(RigidActor)
	{
		return RigidActor->getName();
	}
	else
	{
#ifdef DEPRECATED
		PxShape* Shape = GetShapeFromHandle(handle);
		ASSERT(Shape);
		return Shape->getName();
#else
		ASSERT(0);
		return null;
#endif
	}
}

bool SharedPhysX::SetName(PintActorHandle handle, const char* name)
{
	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(RigidActor)
	{
		SetActorName(RigidActor, name);
	}
	else
	{
#ifdef DEPRECATED
		PxShape* Shape = GetShapeFromHandle(handle);
		ASSERT(Shape);
		Shape->setName(name);
#else
		ASSERT(0);
#endif
	}
	return true;
}
#endif

/*void SharedPhysX::ApplyActionAtPoint(PintObjectHandle handle, PintActionType action_type, const Point& action, const Point& pos)
{
#ifndef PHYSX3_DISABLE_SHARED_APPLY_ACTION
	PxRigidBody* RigidBody = GetRigidBody(handle);
	if(RigidBody)
	{
		//### local space/world space confusion in this
		PxForceMode::Enum mode;
		if(action_type==PINT_ACTION_FORCE)
			mode = PxForceMode::eFORCE;
		else if(action_type==PINT_ACTION_IMPULSE)
			mode = PxForceMode::eIMPULSE;
		else ASSERT(0);

//		printf("Picking force magnitude: %f\n", action.Magnitude());
		PxRigidBodyExt::addForceAtPos(*RigidBody, ToPxVec3(action), ToPxVec3(pos), mode);
	}
#endif
}*/

void SharedPhysX::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
#ifndef PHYSX3_DISABLE_SHARED_APPLY_ACTION
	PxRigidBody* RigidBody = GetRigidBody(handle);
	if(RigidBody)
	{
		if(!(RigidBody->getActorFlags() & PxActorFlag::eDISABLE_SIMULATION))
			PxRigidBodyExt::addForceAtPos(*RigidBody, ToPxVec3(world_impulse), ToPxVec3(world_pos), PxForceMode::eIMPULSE);
	}
#endif
}

void SharedPhysX::AddLocalTorque(PintActorHandle handle, const Point& local_torque)
{
#ifndef PHYSX3_DISABLE_SHARED_APPLY_ACTION
	if(mParams.mNbSubsteps>1)
	{
		mLocalTorques.push_back(LocalTorque(handle, local_torque));
	}
	else
	{
		PxRigidBody* RigidBody = GetRigidBody(handle);
		if(RigidBody)
		{
			const PxVec3 GlobalTorque = RigidBody->getGlobalPose().rotate(ToPxVec3(local_torque));
	//		RigidBody->addTorque(GlobalTorque, PxForceMode::eFORCE, true);
			RigidBody->addTorque(GlobalTorque, PxForceMode::eACCELERATION, true);
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////

#ifdef DEPRECATED
static Point GetWorldVelocity(PintActorHandle handle, bool angular)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(RigidBody)
	{
		const PxVec3 GlobalVelocity = angular ? RigidBody->getAngularVelocity() : RigidBody->getLinearVelocity();
		return ToPoint(GlobalVelocity);
	}
	return Point(0.0f, 0.0f, 0.0f);
}

Point SharedPhysX::GetWorldLinearVelocity(PintActorHandle handle)
{
	return GetWorldVelocity(handle, false);
}

Point SharedPhysX::GetWorldAngularVelocity(PintActorHandle handle)
{
	return GetWorldVelocity(handle, true);
}

static void SetWorldVelocity(PintActorHandle handle, const Point& velocity, bool angular)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(RigidBody)
	{
		const PxVec3 GlobalVelocity = ToPxVec3(velocity);

		if(angular)
			RigidBody->setAngularVelocity(GlobalVelocity);
		else
			RigidBody->setLinearVelocity(GlobalVelocity);
	}
}

void SharedPhysX::SetWorldLinearVelocity(PintActorHandle handle, const Point& linear_velocity)
{
	SetWorldVelocity(handle, linear_velocity, false);
}

void SharedPhysX::SetWorldAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
	SetWorldVelocity(handle, angular_velocity, true);
}
#endif

///////////////////////////////////////////////////////////////////////////////

static Point GetLocalVelocity(PintActorHandle handle, bool angular)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(RigidBody)
	{
		const PxTransform Pose = RigidBody->getGlobalPose();

		const PxVec3 GlobalVelocity = angular ? RigidBody->getAngularVelocity() : RigidBody->getLinearVelocity();

		const PxVec3 LocalVelocity = Pose.rotateInv(GlobalVelocity);

		return ToPoint(LocalVelocity);
	}
	return Point(0.0f, 0.0f, 0.0f);
}

Point SharedPhysX::GetAngularVelocity(PintActorHandle handle)
{
	return GetLocalVelocity(handle, true);
}

Point SharedPhysX::GetLinearVelocity(PintActorHandle handle)
{
	return GetLocalVelocity(handle, false);
}

static void SetLocalVelocity(PintActorHandle handle, const Point& local_velocity, bool angular)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(RigidBody)
	{
		const PxTransform Pose = RigidBody->getGlobalPose();

		const PxVec3 GlobalVelocity = Pose.rotate(ToPxVec3(local_velocity));

		// ### this simple version doesn't work anymore in PhysX 5.1
/*		if(angular)
			RigidBody->setAngularVelocity(GlobalVelocity);
		else
			RigidBody->setLinearVelocity(GlobalVelocity);*/

		if(RigidBody->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		{
			PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(RigidBody);
			if(angular)
				Dyna->setAngularVelocity(GlobalVelocity);
			else
				Dyna->setLinearVelocity(GlobalVelocity);
		}
#if PHYSX_SUPPORT_ARTICULATIONS
		else if(RigidBody->getConcreteType()==PxConcreteType::eARTICULATION_LINK)
		{
			PxArticulationLink* Link = static_cast<PxArticulationLink*>(RigidBody);
			if(angular)
				Link->setAngularVelocity(GlobalVelocity);
			else
				Link->setLinearVelocity(GlobalVelocity);
		}
#endif
	}
}

void SharedPhysX::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
	SetLocalVelocity(handle, angular_velocity, true);
}

void SharedPhysX::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)
{
	SetLocalVelocity(handle, linear_velocity, false);
}

#ifdef DEPRECATED
float SharedPhysX::GetMass(PintActorHandle handle)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
		return 0.0f;
	return RigidBody->getMass();
}

void SharedPhysX::SetMass(PintActorHandle handle, float mass)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
		return;
	RigidBody->setMass(mass);
}

Point SharedPhysX::GetLocalInertia(PintActorHandle handle)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
		return Point(0.0f, 0.0f, 0.0f);
	return ToPoint(RigidBody->getMassSpaceInertiaTensor());
}

void SharedPhysX::SetLocalInertia(PintActorHandle handle, const Point& inertia)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(!RigidBody)
		return;
	RigidBody->setMassSpaceInertiaTensor(ToPxVec3(inertia));
}

udword SharedPhysX::GetShapes(PintObjectHandle* shapes, PintObjectHandle handle)
{
//	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	PxRigidActor* RigidActor = (PxRigidActor*)handle;
	return RigidActor->getShapes((PxShape**)shapes, 3);
}

void SharedPhysX::SetLocalRot(PintObjectHandle handle, const Quat& q)
{
//	PxShape* Shape = GetShapeFromHandle(handle);
	PxShape* Shape = (PxShape*)handle;
	PxTransform lp = Shape->getLocalPose();
	lp.q = ToPxQuat(q);
	Shape->setLocalPose(lp);
}

///////////////////////////////////////////////////////////////////////////////

bool SharedPhysX::EnableGravity(PintActorHandle handle, bool flag)
{
	PxRigidActor* Actor = GetActorFromHandle(handle);
	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
		return false;

	PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(Actor);
	Dyna->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !flag);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

float SharedPhysX::GetLinearDamping(PintActorHandle handle)
{
	PxRigidActor* Actor = GetActorFromHandle(handle);
	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
		return 0.0f;

	PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(Actor);
	return Dyna->getLinearDamping();
}

float SharedPhysX::GetAngularDamping(PintActorHandle handle)
{
	PxRigidActor* Actor = GetActorFromHandle(handle);
	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
		return 0.0f;

	PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(Actor);
	return Dyna->getAngularDamping();
}

void SharedPhysX::SetLinearDamping(PintActorHandle handle, float damping)
{
	PxRigidActor* Actor = GetActorFromHandle(handle);
	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
		return;

	PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(Actor);
	Dyna->setLinearDamping(damping);
}

void SharedPhysX::SetAngularDamping(PintActorHandle handle, float damping)
{
	PxRigidActor* Actor = GetActorFromHandle(handle);
	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
		return;

	PxRigidDynamic* Dyna = static_cast<PxRigidDynamic*>(Actor);
	Dyna->setAngularDamping(damping);
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool SharedPhysX::SetKinematicPose(PintActorHandle handle, const Point& pos)
{
	if(!IsKinematic(handle))
		return false;
	PxRigidActor* Actor = GetActorFromHandle(handle);
//	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
//		return false;

	PxRigidDynamic* Kine = static_cast<PxRigidDynamic*>(Actor);
	Kine->setKinematicTarget(PxTransform(ToPxVec3(pos)));
	mActorManager.UpdateTimestamp();
	return true;
}

bool SharedPhysX::SetKinematicPose(PintActorHandle handle, const PR& pr)
{
	if(!IsKinematic(handle))
		return false;
	PxRigidActor* Actor = GetActorFromHandle(handle);
//	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
//		return false;

	PxRigidDynamic* Kine = static_cast<PxRigidDynamic*>(Actor);
	Kine->setKinematicTarget(PxTransform(ToPxVec3(pr.mPos), ToPxQuat(pr.mRot)));
	mActorManager.UpdateTimestamp();
	return true;
}

bool SharedPhysX::IsKinematic(PintActorHandle handle)
{
	PxRigidActor* Actor = GetActorFromHandle(handle);
	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
		return false;

	PxRigidDynamic* Kine = static_cast<PxRigidDynamic*>(Actor);
#ifdef IS_PHYSX_3_2
	return Kine->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC;
#else
	return Kine->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC;
#endif
}

bool SharedPhysX::EnableKinematic(PintActorHandle handle, bool flag)
{
	PxRigidActor* Actor = GetActorFromHandle(handle);
	if(!Actor || Actor->getConcreteType()!=PxConcreteType::eRIGID_DYNAMIC)
		return false;

	PxRigidDynamic* Kine = static_cast<PxRigidDynamic*>(Actor);
#ifdef IS_PHYSX_3_2
	Kine->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, flag);
#else
	Kine->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, flag);
#endif
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static PxConvexMesh* CreatePhysXConvex(PxPhysics* physics,
#if PHYSX_SUPPORT_IMMEDIATE_COOKING
	const PxCookingParams& cookingParams,
#else
	PxCooking& cooking,
#endif
	udword nb_verts, const Point* verts, PxConvexFlags flags)
{
	ASSERT(physics);

	PxConvexMeshDesc ConvexDesc;
	ConvexDesc.points.count		= nb_verts;
	ConvexDesc.points.stride	= sizeof(PxVec3);
	ConvexDesc.points.data		= verts;
	ConvexDesc.flags			= flags;

	PxConvexMesh* NewConvex;
#if PHYSX_SUPPORT_INSERTION_CALLBACK
	#if PHYSX_SUPPORT_IMMEDIATE_COOKING
		NewConvex = PxCreateConvexMesh(cookingParams, ConvexDesc, physics->getPhysicsInsertionCallback());
	#else
		NewConvex = cooking.createConvexMesh(ConvexDesc, physics->getPhysicsInsertionCallback());
	#endif
#else
	{
		MemoryOutputStream buf;
		if(!cooking->cookConvexMesh(ConvexDesc, buf))
			return null;

		MemoryInputData input(buf.getData(), buf.getSize());
		NewConvex = physics->createConvexMesh(input);
	//	printf("3.4 convex: %d vertices\n", NewConvex->getNbVertices());
	}
#endif
	return NewConvex;
}

PxConvexMesh* SharedPhysX::CreatePhysXConvex(udword nb_verts, const Point* verts, PxConvexFlags flags)
{
	return ::CreatePhysXConvex(mPhysics, GetCooking(), nb_verts, verts, flags);
}

PintConvexHandle SharedPhysX::CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index)
{
	PxConvexMesh* ConvexMesh = ::CreatePhysXConvex(mPhysics, GetCooking(), desc.mNbVerts, desc.mVerts, mParams.GetConvexFlags());

	const udword Index = mConvexObjects.AddObject(ConvexMesh);
	if(index)
		*index = Index;
	return PintConvexHandle(ConvexMesh);
}

bool SharedPhysX::DeleteConvexObject(PintConvexHandle handle, const PintConvexIndex* index)
{
	if(!mConvexObjects.DeleteObject(handle, index))
		return false;

	PxConvexMesh* ConvexMesh = reinterpret_cast<PxConvexMesh*>(handle);
	ConvexMesh->release();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static PxTriangleMesh* CreatePhysXMesh(PxPhysics* physics,
#if PHYSX_SUPPORT_IMMEDIATE_COOKING
	const PxCookingParams& cookingParams,
#else
	PxCooking& cooking,
#endif
	const PintSurfaceInterface& surface, bool deformable, bool dynamic)
{
	ASSERT(physics);

//		PxCookingParams Params = cooking->getParams();
//		Params.midphaseDesc.mBVH34Desc.quantized = false;
//		cooking->setParams(Params);

#if PHYSX_SUPPORT_DEFORMABLE_MESHES
	#if PHYSX_SUPPORT_IMMEDIATE_COOKING
		PxCookingParams Params = cookingParams;
		if(deformable)
		{
			Params.midphaseDesc.mBVH34Desc.quantized = false;
			PxU32 newValue = PxU32(Params.meshPreprocessParams);
			newValue &= ~PxMeshPreprocessingFlag::eWELD_VERTICES;
			newValue |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
			newValue |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
			Params.meshPreprocessParams = PxMeshPreprocessingFlag::Enum(newValue);
	//		printf("Params.meshPreprocessParams: %d\n", Params.meshPreprocessParams);
		}
	#else
		PxMeshPreprocessingFlags SavedFlags;
		#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
		bool SavedQ;
		#endif
		if(deformable)
		{
			PxCookingParams Params = cooking.getParams();
			SavedFlags = Params.meshPreprocessParams;
		#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
			SavedQ = Params.midphaseDesc.mBVH34Desc.quantized;
			Params.midphaseDesc.mBVH34Desc.quantized = false;
		#endif
			PxU32 newValue = PxU32(Params.meshPreprocessParams);
			newValue &= ~PxMeshPreprocessingFlag::eWELD_VERTICES;
			newValue |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
			newValue |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
			Params.meshPreprocessParams = PxMeshPreprocessingFlag::Enum(newValue);
			cooking.setParams(Params);
	//		printf("Params.meshPreprocessParams: %d\n", Params.meshPreprocessParams);
		}
	#endif
#endif

	PxTriangleMeshDesc MeshDesc;
	MeshDesc.points.count		= surface.mNbVerts;
	MeshDesc.points.stride		= sizeof(PxVec3);
	MeshDesc.points.data		= surface.mVerts;
	MeshDesc.triangles.count	= surface.mNbFaces;
	MeshDesc.triangles.stride	= sizeof(udword)*3;
	MeshDesc.triangles.data		= surface.mDFaces;
//	MeshDesc.flags				= PxMeshFlag::eFLIPNORMALS;
//	MeshDesc.flags				= 0;

#if PHYSX_SUPPORT_DYNAMIC_MESHES
	PxSDFDesc sdfDesc;
	if(dynamic)
	{
		const_cast<PxCookingParams&>(cookingParams).meshPreprocessParams |= PxMeshPreprocessingFlag::eENABLE_INERTIA;

		//sdfDesc.spacing = 0.05f;
		sdfDesc.spacing = 0.005f;
		//sdfDesc.spacing = 0.0025f;
		//sdfDesc.spacing = 0.001f;
		//sdfDesc.spacing = 0.0005f;
		sdfDesc.subgridSize = 6;
		sdfDesc.bitsPerSubgridPixel = PxSdfBitsPerSubgridPixel::e16_BIT_PER_PIXEL;
		sdfDesc.numThreadsForSdfConstruction = 16;

		MeshDesc.sdfDesc = &sdfDesc;
	}
	else
	{
		const_cast<PxCookingParams&>(cookingParams).meshPreprocessParams.clear(PxMeshPreprocessingFlag::eENABLE_INERTIA);
	}
#endif

	if(0)
	{
		FILE* fp = fopen("D:/tmp/bounds.data", "wb");
		if(fp)
		{
			fwrite(&surface.mNbFaces, 1, sizeof(udword), fp);
			for(udword i=0;i<surface.mNbFaces;i++)
			{
				udword vref0 = surface.mDFaces[i*3+0];
				udword vref1 = surface.mDFaces[i*3+1];
				udword vref2 = surface.mDFaces[i*3+2];
				const Point& p0 = surface.mVerts[vref0];
				const Point& p1 = surface.mVerts[vref1];
				const Point& p2 = surface.mVerts[vref2];
				AABB Box;
				Box.mMin = Box.mMax = p0;
				Box.Extend(p1);
				Box.Extend(p2);
				fwrite(&Box, 1, sizeof(AABB), fp);
			}
			fclose(fp);
		}
	}

//	gDefaultAllocator->mLog = true;
//	printf("gDefaultAllocator->mCurrentMemory: %d\n", gDefaultAllocator->mCurrentMemory);
//	printf("gDefaultAllocator->mNbAllocs: %d\n", gDefaultAllocator->mNbAllocs);

	PxTriangleMesh* NewMesh = null;
#if PHYSX_SUPPORT_INSERTION_CALLBACK
	{
//		udword NbAllocs = gDefaultAllocator->mTotalNbAllocs;
	#if PHYSX_SUPPORT_IMMEDIATE_COOKING
		NewMesh = PxCreateTriangleMesh(Params, MeshDesc, physics->getPhysicsInsertionCallback());
	#else
		NewMesh = cooking.createTriangleMesh(MeshDesc, physics->getPhysicsInsertionCallback());
	#endif
//		NbAllocs = gDefaultAllocator->mTotalNbAllocs - NbAllocs;
//		printf("NbAllocs: %d\n", NbAllocs);
	}
#else
	{
//		printf("PhysX 3.3: cooking mesh: %d verts, %d faces\n", surface.mNbVerts, surface.mNbFaces);

//		MemoryOutputStream buf(gDefaultAllocator);
		MemoryOutputStream buf;
		if(cooking->cookTriangleMesh(MeshDesc, buf))
		{
			//	printf("gDefaultAllocator->mCurrentMemory: %d\n", gDefaultAllocator->mCurrentMemory);
			//	printf("gDefaultAllocator->mNbAllocs: %d\n", gDefaultAllocator->mNbAllocs);
			//	gDefaultAllocator->mLog = false;

			//		printf("PhysX 3.3: creating mesh... ");

			MemoryInputData input(buf.getData(), buf.getSize());
			//udword MemBefore = gDefaultAllocator->mCurrentMemory;
			NewMesh = physics->createTriangleMesh(input);
			//udword MemAfter = gDefaultAllocator->mCurrentMemory;
			//printf("PhysX 3.3 mesh memory: %d Kb\n", (MemAfter - MemBefore)/1024);
			//	printf("Done.\n");
		}
	}
#endif

//	printf("gDefaultAllocator->mCurrentMemory: %d\n", gDefaultAllocator->mCurrentMemory);
//	printf("gDefaultAllocator->mNbAllocs: %d\n", gDefaultAllocator->mNbAllocs);
//	gDefaultAllocator->mLog = false;

#if PHYSX_SUPPORT_DEFORMABLE_MESHES
	#if !PHYSX_SUPPORT_IMMEDIATE_COOKING
	if(deformable)
	{
		PxCookingParams Params = cooking.getParams();
	#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
		Params.midphaseDesc.mBVH34Desc.quantized = SavedQ;
	#endif
		Params.meshPreprocessParams = SavedFlags;
		cooking.setParams(Params);
//		printf("Params.meshPreprocessParams: %d\n", Params.meshPreprocessParams);
	}
	#endif
#endif

	return NewMesh;
}

PintMeshHandle SharedPhysX::CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)
{
	PxTriangleMesh* TriangleMesh = CreatePhysXMesh(desc.GetSurface(), desc.mDeformable, desc.mDynamic);

	const udword Index = mMeshObjects.AddObject(TriangleMesh);
	if(index)
		*index = Index;
	return PintMeshHandle(TriangleMesh);
}

bool SharedPhysX::DeleteMeshObject(PintMeshHandle handle, const PintMeshIndex* index)
{
	if(!mMeshObjects.DeleteObject(handle, index))
		return false;

	PxTriangleMesh* TriangleMesh = reinterpret_cast<PxTriangleMesh*>(handle);
	TriangleMesh->release();
	return true;
}

PxTriangleMesh* SharedPhysX::CreatePhysXMesh(const PintSurfaceInterface& surface, bool deformable, bool dynamic)
{
	return ::CreatePhysXMesh(mPhysics, GetCooking(), surface, deformable, dynamic);
}

/*PxTriangleMesh* SharedPhysX::CreateTriangleMesh(const SurfaceInterface& surface, PintShapeRenderer* renderer, bool deformable)
{
	ASSERT(mCooking);
	ASSERT(mPhysics);

	if(mParams.mShareMeshData && renderer)
	{
		const udword Size = udword(mMeshes.size());
		for(udword i=0;i<Size;i++)
		{
			const MeshRender& CurrentMesh = mMeshes[i];
			if(CurrentMesh.mRenderer==renderer)
			{
				return CurrentMesh.mTriangleMesh;
			}
		}
	}

	PxTriangleMesh* NewMesh = CreatePhysXMesh(mPhysics, mCooking, surface, deformable);

	if(renderer)
		mMeshes.push_back(MeshRender(NewMesh, renderer));

	return NewMesh;
}*/

///////////////////////////////////////////////////////////////////////////////

#if PHYSX_SUPPORT_HEIGHTFIELDS
PintHeightfieldHandle SharedPhysX::CreateHeightfieldObject(const PINT_HEIGHTFIELD_DATA_CREATE& desc, PintHeightfieldData& data, PintHeightfieldIndex* index)
{
	ASSERT(mPhysics);
#if !PHYSX_SUPPORT_IMMEDIATE_COOKING
	ASSERT(mCooking);
#endif

	// Questionable PhysX design here: "row" is along X !

	const PxU32 NbRows = desc.mNbV;
	const PxU32 NbCols = desc.mNbU;
	const PxU32 NbSamples = NbCols * NbRows;

	float maxHeight = MIN_FLOAT;
	float minHeight = MAX_FLOAT;
	if(desc.mHeights)
	{
		for(udword i=0;i<NbSamples;i++)
		{
			const float v = desc.mHeights[i];
			if(v>maxHeight)
				maxHeight = v;
			if(v<minHeight)
				minHeight = v;
		}
	}
	else
		maxHeight = minHeight = desc.mUniqueValue;

	const float deltaHeight = maxHeight - minHeight;
	const float quantization = (PxReal)0x7fff;
	float heightScale = 1.0f;
	if(deltaHeight==0.0f)
	{
		heightScale = minHeight != 0.0f ? minHeight : 1.0f;
	}
	else
	{
		heightScale = PxMax(deltaHeight / quantization, PX_MIN_HEIGHTFIELD_Y_SCALE);
	}

	PxHeightFieldSample* samples = new PxHeightFieldSample[NbSamples];
	for(PxU32 i=0;i<NbSamples;i++)
	{
		const float v = desc.mHeights ? desc.mHeights[i] : desc.mUniqueValue;

		PxHeightFieldSample& smp = samples[i];
		//smp.height = 0;

		PxI16 height;
		if(deltaHeight!=0.0f)
			height = PxI16(quantization * ((v - minHeight) / deltaHeight));
		else
			//height = minHeight < 0.f ? PxI16(-1) : PxI16(1);
			height = minHeight != 0.0f ? 1 : 0;
		smp.height = height;

		smp.materialIndex0 = 0;
		smp.materialIndex1 = 0;
		//smp.clearTessFlag();
		smp.setTessFlag();
		//smp.materialIndex0 = PxHeightFieldMaterial::eHOLE;
		//smp.materialIndex1 = PxHeightFieldMaterial::eHOLE;
	}

	PxHeightFieldDesc hfDesc;
	hfDesc.nbColumns		= NbCols;
	hfDesc.nbRows			= NbRows;
	hfDesc.format			= PxHeightFieldFormat::eS16_TM;
	hfDesc.samples.data		= samples;
	hfDesc.samples.stride	= sizeof(PxHeightFieldSample);
	//hfDesc.flags			= PxHeightFieldFlag::eNO_BOUNDARY_EDGES;
	#if PHYSX_SUPPORT_HEIGHTFIELD_THICKNESS
	hfDesc.thickness		= 0.0f;
	#endif

	#if PHYSX_SUPPORT_IMMEDIATE_COOKING
	PxHeightField* HF = PxCreateHeightField(hfDesc, mPhysics->getPhysicsInsertionCallback());
	#else
	PxHeightField* HF = mCooking->createHeightField(hfDesc, mPhysics->getPhysicsInsertionCallback());
	#endif

	DELETEARRAY(samples);

	const udword Index = mHeightfieldObjects.AddObject(HF);
	if(index)
		*index = Index;

	if(Index>=mHFDataSize)
	{
		const udword Capacity = NextPowerOfTwo(Index+1);
		HFCompanionData* NewData = ICE_NEW(HFCompanionData)[Capacity];
		if(mHFData)
			CopyMemory(NewData, mHFData, mHFDataSize*sizeof(HFCompanionData));
		DELETEARRAY(mHFData);
		mHFData = NewData;
	}
	mHFData[Index].mHeightScale = heightScale;
	mHFData[Index].mMinHeight = minHeight;

	data.mHeightScale = heightScale;
	data.mMinHeight = minHeight;
	data.mMaxHeight = maxHeight;

	return PintHeightfieldHandle(HF);
}

bool SharedPhysX::DeleteHeightfieldObject(PintHeightfieldHandle handle, const PintHeightfieldIndex* index)
{
	if(!mHeightfieldObjects.DeleteObject(handle, index))
		return false;

	PxHeightField* HF = reinterpret_cast<PxHeightField*>(handle);
	HF->release();
	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////

PxMaterial* SharedPhysX::CreateMaterial(const PINT_MATERIAL_CREATE& desc)
{
	const PxU32 NbMaterials = udword(mMaterials.size());
	for(PxU32 i=0;i<NbMaterials;i++)
	{
		PxMaterial* M = mMaterials[i];
		if(		M->getRestitution()==desc.mRestitution
			&&	M->getStaticFriction()==desc.mStaticFriction
			&&	M->getDynamicFriction()==desc.mDynamicFriction)
		{
			return M;
		}
	}

	ASSERT(mPhysics);
	PxMaterial* M = mPhysics->createMaterial(desc.mStaticFriction, desc.mDynamicFriction, desc.mRestitution);
	ASSERT(M);
	M->setFlag(PxMaterialFlag::eDISABLE_STRONG_FRICTION, mParams.mDisableStrongFriction);
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
	M->setFlag(PxMaterialFlag::eIMPROVED_PATCH_FRICTION, mParams.mImprovedPatchFriction);
#endif

	if(desc.mStaticFriction==0.0f && desc.mDynamicFriction==0.0f)		
		M->setFrictionCombineMode(PxCombineMode::eMULTIPLY);

//	M->setFrictionCombineMode(PxCombineMode::eMIN);
//	M->setRestitutionCombineMode(PxCombineMode::eMIN);
	PxCombineMode::Enum defMode = M->getFrictionCombineMode();
	mMaterials.push_back(M);
	return M;
}

///////////////////////////////////////////////////////////////////////////////

#ifndef IS_PHYSX_3_2
PxShape* SharedPhysX::CreateSphereShape(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxSphereGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group)
{
	if(!create->CanShare(mParams.mShareShapes))
		return CreateNonSharedShape(*this, create, actor, geometry, material, local_pose, collision_group, mParams);

	const float Radius = geometry.radius;
#ifdef SHARED_SHAPES_USE_HASH

	if(!mSphereShapes)
		mSphereShapes = new(SharedSpheres);

	const InternalSphereShape iss(Radius, &material, create->mRenderer, local_pose, collision_group);

	// sigh
	ShapePtr e = (*mSphereShapes)[iss];
	if(e.mShape)
	{
		actor->attachShape(*e.mShape);
		return e.mShape;
	}
	else
	{
		PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);
		e.mShape = NewShape;
		return NewShape;
	}

/*	const InternalSphereShape iss(Radius, &material, create->mRenderer, local_pose, collision_group);

	if(!mSphereShapes)
		mSphereShapes = new(SharedSpheres);

	const SharedSpheres::Entry* e = mSphereShapes->find(iss);
	if(e)
	{
		actor->attachShape(*e->second);
		return e->second;
	}

	PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);

	mSphereShapes->insert(iss, NewShape);

	return NewShape;*/
#else
	PxShape* S = reinterpret_cast<PxShape*>(mSphereShapes.FindShape(Radius, &material, create->mRenderer, ToPR(local_pose), collision_group));
	if(S)
	{
		//printf("Sharing shape\n");
		actor->attachShape(*S);
		return S;
	}

	PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);

	mSphereShapes.RegisterShape(Radius, NewShape, &material, create->mRenderer, ToPR(local_pose), collision_group);
	return NewShape;
#endif
}

PxShape* SharedPhysX::CreateBoxShape(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxBoxGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group)
{
	if(!create->CanShare(mParams.mShareShapes))
		return CreateNonSharedShape(*this, create, actor, geometry, material, local_pose, collision_group, mParams);

	PxShape* S = reinterpret_cast<PxShape*>(mBoxShapes.FindShape(ToPoint(geometry.halfExtents), &material, create->mRenderer, ToPR(local_pose), collision_group));
	if(S)
	{
		//printf("Sharing shape\n");
		actor->attachShape(*S);
		return S;
	}

	PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);

	mBoxShapes.RegisterShape(ToPoint(geometry.halfExtents), NewShape, &material, create->mRenderer, ToPR(local_pose), collision_group);

	return NewShape;
}

PxShape* SharedPhysX::CreateCapsuleShape(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxCapsuleGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group)
{
	if(!create->CanShare(mParams.mShareShapes))
		return CreateNonSharedShape(*this, create, actor, geometry, material, local_pose, collision_group, mParams);

	PxShape* S = reinterpret_cast<PxShape*>(mCapsuleShapes.FindShape(geometry.radius, geometry.halfHeight, &material, create->mRenderer, ToPR(local_pose), collision_group));
	if(S)
	{
		//printf("Sharing shape\n");
		actor->attachShape(*S);
		return S;
	}

	PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);

	mCapsuleShapes.RegisterShape(geometry.radius, geometry.halfHeight, NewShape, &material, create->mRenderer, ToPR(local_pose), collision_group);

	return NewShape;
}

#if PHYSX_SUPPORT_CUSTOM_GEOMETRY
PxShape* SharedPhysX::CreateCylinderShape(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxCustomGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group)
{
	if(!create->CanShare(mParams.mShareShapes))
		return CreateNonSharedShape(*this, create, actor, geometry, material, local_pose, collision_group, mParams);

//	ASSERT(geometry.getCustomType()==1);
//####
//	const InternalCustomConvex* custom1 = (const InternalCustomConvex*)geometry.callbacks;
//PxCustomGeometryExt::Callbacks* cb = (PxCustomGeometryExt::Callbacks*)geometry.callbacks;

PxCustomGeometryExt::CylinderCallbacks* CustomCylinder = (PxCustomGeometryExt::CylinderCallbacks*)geometry.callbacks;


//	ASSERT(custom1->type==InternalCustomConvex::eCylinder);
//	const InternalCustomCylinder* CustomCylinder = static_cast<const InternalCustomCylinder*>(custom1);

//ASSERT(cb->getCustomType() == PxCustomGeometryExt::CylinderData::CUSTOM_TYPE);
//const PxCustomGeometryExt::CylinderData* CustomCylinder = cb->get<PxCustomGeometryExt::CylinderData>();

	PxShape* S = reinterpret_cast<PxShape*>(mCylinderShapes.FindShape(CustomCylinder->radius, CustomCylinder->height, &material, create->mRenderer, ToPR(local_pose), collision_group));
	if(S)
	{
		//printf("Sharing shape\n");
		actor->attachShape(*S);
		return S;
	}

	PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);

	mCylinderShapes.RegisterShape(CustomCylinder->radius, CustomCylinder->height, NewShape, &material, create->mRenderer, ToPR(local_pose), collision_group);
	return NewShape;
}
#endif

PxShape* SharedPhysX::CreateConvexShape(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxConvexMeshGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group)
{
	if(!create->CanShare(mParams.mShareShapes))
		return CreateNonSharedShape(*this, create, actor, geometry, material, local_pose, collision_group, mParams);

	PxShape* S = reinterpret_cast<PxShape*>(mConvexShapes.FindShape(geometry.convexMesh, &material, create->mRenderer, ToPR(local_pose), collision_group));
	if(S)
	{
		//printf("Sharing shape\n");
		actor->attachShape(*S);
		return S;
	}

	PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);

	mConvexShapes.RegisterShape(geometry.convexMesh, NewShape, &material, create->mRenderer, ToPR(local_pose), collision_group);
	return NewShape;
}

PxShape* SharedPhysX::CreateMeshShape(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxTriangleMeshGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group)
{
	if(!create->CanShare(mParams.mShareShapes))
		return CreateNonSharedShape(*this, create, actor, geometry, material, local_pose, collision_group, mParams);

	PxShape* S = reinterpret_cast<PxShape*>(mMeshShapes.FindShape(geometry.triangleMesh, &material, create->mRenderer, ToPR(local_pose), collision_group));
	if(S)
	{
		//printf("Sharing shape\n");
		actor->attachShape(*S);
		return S;
	}

	PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);

	mMeshShapes.RegisterShape(geometry.triangleMesh, NewShape, &material, create->mRenderer, ToPR(local_pose), collision_group);
	return NewShape;
}

#if PHYSX_SUPPORT_HEIGHTFIELDS
PxShape* SharedPhysX::CreateHeightfieldShape(const PINT_SHAPE_CREATE* create, PxRigidActor* actor, const PxHeightFieldGeometry& geometry, const PxMaterial& material, const PxTransform& local_pose, PxU16 collision_group)
{
	if(!create->CanShare(mParams.mShareShapes))
		return CreateNonSharedShape(*this, create, actor, geometry, material, local_pose, collision_group, mParams);

	// #### maybe missing scales here
	PxShape* S = reinterpret_cast<PxShape*>(mHeightfieldShapes.FindShape(geometry.heightField, &material, create->mRenderer, ToPR(local_pose), collision_group));
	if(S)
	{
		//printf("Sharing shape\n");
		actor->attachShape(*S);
		return S;
	}

	PxShape* NewShape = CreateSharedShape(*this, mPhysics, create, actor, geometry, material, local_pose, collision_group, mParams);

	mHeightfieldShapes.RegisterShape(geometry.heightField, NewShape, &material, create->mRenderer, ToPR(local_pose), collision_group);
	return NewShape;
}
#endif

/*void SharedPhysX::onRelease(const PxBase* observed, void* userData, PxDeletionEventFlag::Enum deletionEvent)
{
	//printf("onRelease: %x\n", observed);

	ASSERT(deletionEvent == PxDeletionEventFlag::eUSER_RELEASE);  // the only type we registered for

//	if(!(observed->getConcreteType()==PxConcreteType:: eRIGID_DYNAMIC || observed->getConcreteType()==PxConcreteType:: eRIGID_STATIC ||
//		observed->getConcreteType()==PxConcreteType::eSHAPE))
//		return;
	if(observed->getConcreteType()==PxConcreteType::eTRIANGLE_MESH_BVH34)
	{
		int stop=1;
	}
}*/

void SharedPhysX::ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)
{
	ASSERT(user_data);
	const LocalShapeCreationParams* Params = reinterpret_cast<const LocalShapeCreationParams*>(user_data);
	PxRigidActor* actor				= Params->mActor;
	const PintCollisionGroup group	= Params->mGroup;

	PxTransform LocalPose(ToPxVec3(create.mLocalPos), ToPxQuat(create.mLocalRot));

	PxMaterial* ShapeMaterial = mDefaultMaterial;
	if(create.mMaterial)
	{
		ShapeMaterial = CreateMaterial(*create.mMaterial);
		ASSERT(ShapeMaterial);
	}

	PxShape* shape = null;
	if(create.mType==PINT_SHAPE_SPHERE)
	{
		const PINT_SPHERE_CREATE& SphereCreate = static_cast<const PINT_SPHERE_CREATE&>(create);
		shape = CreateSphereShape(&create, actor, PxSphereGeometry(SphereCreate.mRadius), *ShapeMaterial, LocalPose, group);
		//shape = CreateCapsuleShape(&create, actor, PxCapsuleGeometry(SphereCreate.mRadius, 0.0f), *ShapeMaterial, LocalPose, group);
	}
	else if(create.mType==PINT_SHAPE_BOX)
	{
		const PINT_BOX_CREATE& BoxCreate = static_cast<const PINT_BOX_CREATE&>(create);
		shape = CreateBoxShape(&create, actor, PxBoxGeometry(ToPxVec3(BoxCreate.mExtents)), *ShapeMaterial, LocalPose, group);
	}
	else if(create.mType==PINT_SHAPE_CAPSULE)
	{
		const PINT_CAPSULE_CREATE& CapsuleCreate = static_cast<const PINT_CAPSULE_CREATE&>(create);

/*			// ### PhysX is weird with capsules
		Matrix3x3 Rot;
		Rot.RotY(HALFPI);
		LocalPose.q *= ToPxQuat(Quat(Rot));
*/
		const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.0f));
		LocalPose.q *= q;
//			LocalPose.q = q * LocalPose.q;

		shape = CreateCapsuleShape(&create, actor, PxCapsuleGeometry(CapsuleCreate.mRadius, CapsuleCreate.mHalfHeight), *ShapeMaterial, LocalPose, group);
	}
#if PHYSX_SUPPORT_CUSTOM_GEOMETRY
	else if(create.mType==PINT_SHAPE_CYLINDER)
	{
		const PINT_CYLINDER_CREATE& CylinderCreate = static_cast<const PINT_CYLINDER_CREATE&>(create);

//			const PxCustomGeometry CustomGeom = InternalCustomCylinder::getGeometry(CylinderCreate.mHalfHeight * 2.0f, CylinderCreate.mRadius, 1, 0.0f);
// ####
// where is this released?
//PxCustomGeometryExt::Callbacks* cb = PxCustomGeometryExt::createCylinderCallbacks(CylinderCreate.mHalfHeight * 2.0f, CylinderCreate.mRadius, 1, 0.0f);
//const PxCustomGeometry CustomGeom(*cb);

//PxCustomGeometryExt::CylinderCallbacks* cb = new PxCustomGeometryExt::CylinderCallbacks(CylinderCreate.mHalfHeight * 2.0f, CylinderCreate.mRadius, 1, 0.0f);
PxCustomGeometryExt::CylinderCallbacks* cb = new PxCustomGeometryExt::CylinderCallbacks(CylinderCreate.mHalfHeight * 2.0f, CylinderCreate.mRadius, 1, 0.0f);
const PxCustomGeometry CustomGeom(*cb);

		shape = CreateCylinderShape(&create, actor, CustomGeom, *ShapeMaterial, LocalPose, group);
	}
#endif
	else if(create.mType==PINT_SHAPE_CONVEX)
	{
		const PINT_CONVEX_CREATE& ConvexCreate = static_cast<const PINT_CONVEX_CREATE&>(create);

//			PxConvexMesh* ConvexMesh = CreateConvexMesh(ConvexCreate.mVerts, ConvexCreate.mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX|PxConvexFlag::eINFLATE_CONVEX, create.mRenderer);
//			PxConvexMesh* ConvexMesh = CreateConvexMesh(ConvexCreate.mVerts, ConvexCreate.mNbVerts, PxConvexFlag::eCOMPUTE_CONVEX, create.mRenderer);
		PxConvexMesh* ConvexMesh = CreateConvexMesh(ConvexCreate.mVerts, ConvexCreate.mNbVerts, mParams.GetConvexFlags(), create.mRenderer, mParams.mShareMeshData);
		ASSERT(ConvexMesh);

		PxConvexMeshGeometry ConvexGeom(ConvexMesh);
#if PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS
		if(mParams.mUseTightConvexBounds)
			ConvexGeom.meshFlags = PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;
		else
			ConvexGeom.meshFlags = PxConvexMeshGeometryFlag::Enum(0);
#endif
		shape = CreateConvexShape(&create, actor, ConvexGeom, *ShapeMaterial, LocalPose, group);
	}
	else if(create.mType==PINT_SHAPE_MESH)
	{
		const PINT_MESH_CREATE& MeshCreate = static_cast<const PINT_MESH_CREATE&>(create);

		PxTriangleMesh* TriangleMesh = CreateTriangleMesh(MeshCreate.GetSurface(), create.mRenderer, MeshCreate.mDeformable, mParams.mShareMeshData, Params->mIsDynamic);
		ASSERT(TriangleMesh);

		if(1)
			shape = CreateMeshShape(&create, actor, PxTriangleMeshGeometry(TriangleMesh), *ShapeMaterial, LocalPose, group);
		else
		{
			// Fake scale to test other codepaths within PhysX
			PxTriangleMeshGeometry meshGeom(TriangleMesh);
			//meshGeom.scale.scale = PxVec3(-1.0f);
			meshGeom.scale.scale = PxVec3(1.0001f);
			shape = CreateMeshShape(&create, actor, meshGeom, *ShapeMaterial, LocalPose, group);
		}
	}
	else if(create.mType==PINT_SHAPE_MESH2)
	{
		const PINT_MESH_CREATE2& MeshCreate = static_cast<const PINT_MESH_CREATE2&>(create);

		PxTriangleMesh* TriangleMesh = reinterpret_cast<PxTriangleMesh*>(MeshCreate.mTriangleMesh);
		ASSERT(TriangleMesh);

		shape = CreateMeshShape(&create, actor, PxTriangleMeshGeometry(TriangleMesh), *ShapeMaterial, LocalPose, group);
	}
#if PHYSX_SUPPORT_HEIGHTFIELDS
	else if(create.mType==PINT_SHAPE_HEIGHTFIELD)
	{
		const PINT_HEIGHTFIELD_CREATE& HeightfieldCreate = static_cast<const PINT_HEIGHTFIELD_CREATE&>(create);

		PxHeightField* HF = reinterpret_cast<PxHeightField*>(HeightfieldCreate.mHeightfield);
		ASSERT(HF);

		PxReal heightScale = 1.0f;
		const PxReal rowScale = HeightfieldCreate.mScaleV;
		const PxReal columnScale = HeightfieldCreate.mScaleU;

		udword Index;
		if(mHeightfieldObjects.mObjects.Contains(HF, &Index))
		{
			const HFCompanionData& HFData = mHFData[Index];
			heightScale = HFData.mHeightScale;

			//LocalPose.p.y += HFData.mMinHeight;
		}
		else
			ASSERT(0);

		const PxHeightFieldGeometry hfGeom(HF, PxMeshGeometryFlag::Enum(0), heightScale, rowScale, columnScale);

		shape = CreateHeightfieldShape(&create, actor, hfGeom, *ShapeMaterial, LocalPose, group);
	}
#endif
	else ASSERT(0);

	const char* forced_name = Params->mForcedName;
	if(forced_name && shape)
		shape->setName(forced_name);
}

void SharedPhysX::CreateShapes(const PINT_OBJECT_CREATE& desc, PxRigidActor* actor, PintCollisionGroup group, const char* forced_name)
{
	ASSERT(actor);
	ASSERT(mDefaultMaterial);

	LocalShapeCreationParams Params;
	Params.mForcedName	= forced_name;
	Params.mActor		= actor;
	Params.mGroup		= group;
	Params.mIsDynamic	= actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC;

	desc.GetNbShapes(this, &Params);
}
#endif

///////////////////////////////////////////////////////////////////////////////

PintAggregateHandle SharedPhysX::CreateAggregate(udword max_size, bool enable_self_collision)
{
	ASSERT(mPhysics);
	// TODO: where are these released?
/*#if PHYSX_SUPPORT_GPU_AGGREGATES
	// Assumes single-shape actors to avoid updating the PEEL API for now
	PxAggregate* Aggregate = mPhysics->createAggregate(max_size, max_size, enable_self_collision);
#else*/
	PxAggregate* Aggregate = mPhysics->createAggregate(max_size, enable_self_collision);
//#endif
	return PintAggregateHandle(Aggregate);
}

bool SharedPhysX::AddToAggregate(PintActorHandle object, PintAggregateHandle aggregate)
{
	PxRigidActor* Actor = GetActorFromHandle(object);
	if(!Actor)
		return false;

	PxAggregate* Aggregate = reinterpret_cast<PxAggregate*>(aggregate);
	return Aggregate->addActor(*Actor);
}

bool SharedPhysX::AddAggregateToScene(PintAggregateHandle aggregate)
{
	PxAggregate* Aggregate = reinterpret_cast<PxAggregate*>(aggregate);
	mScene->addAggregate(*Aggregate);

	const udword NbActors = Aggregate->getNbActors();
	for(udword i=0;i<NbActors;i++)
	{
		PxActor* Actor;
		udword N = Aggregate->getActors(&Actor, 1, i);
		ASSERT(N==1);

		AddActorToManager(static_cast<PxRigidActor*>(Actor));

		if(Actor->getConcreteType()==PxConcreteType::eRIGID_DYNAMIC)
		{
			PxRigidDynamic* RigidDynamic = static_cast<PxRigidDynamic*>(Actor);
#ifdef IS_PHYSX_3_2
			if(!(RigidDynamic->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC))
#else
			if(!(RigidDynamic->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC))
#endif
				SetupSleeping(RigidDynamic, mParams.mEnableSleeping);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#if PHYSX_SUPPORT_ARTICULATIONS
PintArticHandle SharedPhysX::CreateArticulation(const PINT_ARTICULATION_CREATE&)
{
	if(mParams.mDisableArticulations)
		return null;

	PxArticulation* Articulation = mPhysics->createArticulation();
	Articulation->setSleepThreshold(mParams.mSleepThreshold);

#if PHYSX_SUPPORT_STABILIZATION_FLAG
	// Stabilization can create artefacts on jointed objects so we just disable it
	Articulation->setStabilizationThreshold(0.0f);
#endif

	if(0)
	{
		PxU32 minPositionIters, minVelocityIters;
		Articulation->getSolverIterationCounts(minPositionIters, minVelocityIters);
		printf("minPositionIters: %d\n", minPositionIters);
		printf("minVelocityIters: %d\n", minVelocityIters);
	}
	Articulation->setSolverIterationCounts(mParams.mSolverIterationCountPos, mParams.mSolverIterationCountVel);
	
	// Projection
	Articulation->setMaxProjectionIterations(mParams.mMaxProjectionIterations);
	Articulation->setSeparationTolerance(mParams.mSeparationTolerance);

	//
	Articulation->setExternalDriveIterations(mParams.mExternalDriveIterations);
	Articulation->setInternalDriveIterations(mParams.mInternalDriveIterations);

	return PintArticHandle(Articulation);
}

bool SharedPhysX::AddArticulationToScene(PintArticHandle articulation)
{
	PxArticulation* Articulation = (PxArticulation*)articulation;

	// TODO: hack to detect that adding the articulation failed on the GPU. We should revisit this and return bool or something.
	const udword NbErrors = gNbPhysXErrors;
	mScene->addArticulation(*Articulation);
	if(gNbPhysXErrors!=NbErrors)
		return false;

	SetupSleeping(Articulation, mParams.mEnableSleeping);

	const udword NbLinks = Articulation->getNbLinks();
/*	// doesn't compile with 3.3.4
	for(udword i=0;i<NbLinks;i++)
	{
		PxArticulationLink* Link;
		Articulation->getLinks(&Link, 1, i);
		AddActorToManager(Link);
	}*/
	PxArticulationLink** Links = (PxArticulationLink**)ICE_ALLOC(sizeof(PxArticulationLink*)*NbLinks);
	Articulation->getLinks(Links, NbLinks);
	for(udword i=0;i<NbLinks;i++)
		AddActorToManager(Links[i]);
	ICE_FREE(Links);
	return true;
}

static void setupJoint(PxArticulationJoint* j, const PINT_ARTICULATED_BODY_CREATE& bc)
{
//	setupJoint(j);

	j->setSwingLimitEnabled(bc.mEnableSwingLimit);
	if(bc.mEnableSwingLimit)
		j->setSwingLimit(bc.mSwingYLimit, bc.mSwingZLimit);

	j->setTwistLimitEnabled(bc.mEnableTwistLimit);
	if(bc.mEnableTwistLimit)
		j->setTwistLimit(bc.mTwistLowerLimit, bc.mTwistUpperLimit);

	if(bc.mUseMotor)
	{
		if(bc.mMotor.mExternalCompliance!=0.0f)
			j->setExternalCompliance(bc.mMotor.mExternalCompliance);
		if(bc.mMotor.mInternalCompliance!=0.0f)
			j->setInternalCompliance(bc.mMotor.mInternalCompliance);
		j->setDamping(bc.mMotor.mDamping);
#ifdef IS_PHYSX_3_2
		j->setSpring(bc.mMotor.mStiffness);
#else
		j->setStiffness(bc.mMotor.mStiffness);
#endif
		if(!bc.mMotor.mTargetVelocity.IsNotUsed())
			j->setTargetVelocity(ToPxVec3(bc.mMotor.mTargetVelocity));
		if(!bc.mMotor.mTargetOrientation.IsNotUsed())
			j->setTargetOrientation(ToPxQuat(bc.mMotor.mTargetOrientation));
	}
}

PintActorHandle SharedPhysX::CreateArticulatedObject(const PINT_OBJECT_CREATE& oc, const PINT_ARTICULATED_BODY_CREATE& bc, PintArticHandle articulation)
{
	PxArticulation* Articulation = reinterpret_cast<PxArticulation*>(articulation);

	// Note that this already creates the joint between the objects!
	PintActorHandle h = CreateArticulationLink(Articulation, (PxArticulationLink*)bc.mParent, *this, oc);

	// ...so we setup the joint data immediately
	PxArticulationJoint* joint = static_cast<PxArticulationJoint*>(((PxArticulationLink*)h)->getInboundJoint());
	if(joint)	// Will be null for root link
	{
//		PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 1.0f));
		const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), ToPxVec3(bc.mX));

		joint->setParentPose(PxTransform(ToPxVec3(bc.mLocalPivot0), q));
		joint->setChildPose(PxTransform(ToPxVec3(bc.mLocalPivot1), q));
		setupJoint(joint, bc);
	}
	return h;
}

void SharedPhysX::SetArticulatedMotor(PintActorHandle handle, const PINT_ARTICULATED_MOTOR_CREATE& motor)
{
	PxRigidBody* RigidBody = PhysX3::GetRigidBody(handle);
	if(RigidBody)
	{
		PxArticulationLink* actor = static_cast<PxArticulationLink*>(RigidBody);
		PxArticulationJoint* j = static_cast<PxArticulationJoint*>(actor->getInboundJoint());
		if(j)
		{
			if(motor.mExternalCompliance!=0.0f)
				j->setExternalCompliance(motor.mExternalCompliance);
			if(motor.mInternalCompliance!=0.0f)
				j->setInternalCompliance(motor.mInternalCompliance);
			j->setDamping(motor.mDamping);
#ifdef IS_PHYSX_3_2
			j->setSpring(motor.mStiffness);
#else
			j->setStiffness(motor.mStiffness);
#endif
			j->setTargetVelocity(ToPxVec3(motor.mTargetVelocity));
		}
	}
}

// TODO: refactor with CreateJoint
PintActorHandle SharedPhysX::CreateArticulationLink(PxArticulation* articulation, PxArticulationLink* parent, Pint& pint, const PINT_OBJECT_CREATE& desc)
{
	udword NbShapes = desc.GetNbShapes();
	if(!NbShapes)
		return null;

//	ASSERT(mPhysics);
//	ASSERT(mScene);

	const PxTransform pose(ToPxVec3(desc.mPosition), ToPxQuat(desc.mRotation));

	PxArticulationLink* actor = articulation->createLink(parent, pose);

/*	PxRigidActor* actor;
	PxRigidDynamic* rigidDynamic = null;
	if(desc.mMass!=0.0f)
	{
		rigidDynamic = mPhysics->createRigidDynamic(pose);
		ASSERT(rigidDynamic);
		actor = rigidDynamic;
	}
	else
	{
		PxRigidStatic* rigidStatic = mPhysics->createRigidStatic(pose);
		ASSERT(rigidStatic);
		actor = rigidStatic;
	}*/

	CreateShapes(desc, actor, desc.mCollisionGroup, null);

	if(actor)
		SetupArticulationLink(*actor, desc);

	// Removed since doesn't work with shared shapes
//	PxSetGroup(*actor, desc.mCollisionGroup);

/*	if(desc.mAddToWorld)
	{
		mScene->addActor(*actor);

		if(rigidDynamic && !desc.mKinematic)
			SetupSleeping(rigidDynamic, mParams.mEnableSleeping);
	}*/
	return CreateHandle(actor);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef IS_PHYSX_3_2
PxQueryHitType::Enum SharedPhysX::preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
{
	if(mInvisibles && mInvisibles->contains(PintActorHandle(actor)))
		return PxQueryHitType::eNONE;

	return PxQueryHitType::eBLOCK;
}

PxQueryHitType::Enum SharedPhysX::postFilter(const PxFilterData& filterData, const PxQueryHit& hit)
{
	return PxQueryHitType::eBLOCK;
}

bool SharedPhysX::SetSQFlag(PintActorHandle actor, bool flag)
{
	if(!mInvisibles)
		mInvisibles = new _hashset<PintActorHandle>;

	if(flag)
	{
		mInvisibles->erase(actor);

		if(!mInvisibles->size())
		{
			DELETESINGLE(mInvisibles);
		}
	}
	else
	{
		mInvisibles->insert(actor);
	}
	return true;
}

bool SharedPhysX::SetSQFlag(PintActorHandle actor, PintShapeHandle shape, bool flag)
{
	ASSERT(!"not implemented");
	return false;
}

bool SharedPhysX::ResetSQFilters()
{
	DELETESINGLE(mInvisibles);
	return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////

static	const	bool	gDumpSceneBoundsEachFrame	= false;
static			bool	gVisualizeMBPRegions		= false;

static void RenderShapeGeneric(PintRender& renderer, PxShape* shape, const PR& IcePose)
{
	const PxGeometryType::Enum geomType = shape->getGeometryType();
	if(geomType==PxGeometryType::eSPHERE)
	{
		PxSphereGeometry geometry;
		bool status = shape->getSphereGeometry(geometry);
		ASSERT(status);

		renderer.DrawSphere(geometry.radius, IcePose);
	}
	else if(geomType==PxGeometryType::eBOX)
	{
		PxBoxGeometry geometry;
		bool status = shape->getBoxGeometry(geometry);
		ASSERT(status);

		renderer.DrawBox(ToPoint(geometry.halfExtents), IcePose);
	}
	else if(geomType==PxGeometryType::eCAPSULE)
	{
		ASSERT(0);
	}
	else if(geomType==PxGeometryType::eCONVEXMESH)
	{
		ASSERT(0);
	}
	else ASSERT(0);
}

static inline_ void RenderShape(PintRender& renderer, PxShape* shape, const PxTransform& shapePose)
{
	if(!renderer.SetCurrentShape(PintShapeHandle(shape)))
		return;

	// TODO: don't create the ICE pose, convert to matrix directly here with SIMD, pass OpenGL data to renderers
	const PR IcePose = ToPR(shapePose);

	// TODO: SIMD convert or even in the shader itself
//	QuatGetMat33V(const QuatVArg q, Vec3V& column0, Vec3V& column1, Vec3V& column2)

	ASSERT(shape->userData);
	if(shape->userData)
	{
		PintShapeRenderer* shapeRenderer = reinterpret_cast<PintShapeRenderer*>(shape->userData);
		if(shape->getGeometryType()!=PxGeometryType::eCAPSULE)	// ### VCALL
		{
//			shapeRenderer->Render(IcePose);
			renderer.DrawShape(shapeRenderer, IcePose);
		}
		else
		{
			PR CapsuleTransform;
			PhysX3::ComputeCapsuleTransform(CapsuleTransform, IcePose);
//			shapeRenderer->Render(CapsuleTransform);
			renderer.DrawShape(shapeRenderer, CapsuleTransform);
		}
	}
	// Reactivated for internal XPs
	else RenderShapeGeneric(renderer, shape, IcePose);
}

void SharedPhysX::Render(PintRender& renderer, PintRenderPass render_pass)
{
	if(!mScene)
		return;

#ifdef TEST_FLUIDS0
	if(gParticleSystem)
		renderParticles(renderer, gParticleSystem);
#endif

//	mScene->checkResults(true);

//	const PxU32 TS = mScene->getTimestamp();
//	printf("Render TS: %d\n", TS);

	AABB GlobalBounds;
	GlobalBounds.SetEmpty();

	PxU32 nbVisibleActors = 0;
	const Plane* FrustumPlanes = renderer.GetFrustumPlanes();

	if(1)
	{
#define USE_ACTOR_MANAGER_CACHE

		// New version bypassing the slow getActors call and merging regular actors & articulation links
		for(udword pass=0;pass<2;pass++)
		{
			udword NbActors = pass ? mActorManager.GetNbStaticActors() : mActorManager.GetNbDynamicActors();
			const ActorData* Actors = pass ? mActorManager.GetStaticActors() : mActorManager.GetDynamicActors();

#ifdef USE_ACTOR_MANAGER_CACHE
			const ActorCachedData* CachedData = pass ? mActorManager.GetStaticActorsCachedData() : mActorManager.GetDynamicActorsCachedData();
#endif
			if(!FrustumPlanes)
				nbVisibleActors += NbActors;

			while(NbActors--)
			{
				const ActorData& CurrentActor = *Actors++;
#ifdef USE_ACTOR_MANAGER_CACHE
				const ActorCachedData& CurrentCachedData = *CachedData++;
#endif
				PxRigidActor* rigidActor = CurrentActor.mActor;

				if(renderer.SetCurrentActor(PintActorHandle(rigidActor)))
				{
					if(gDumpSceneBoundsEachFrame)
					{
						const PxBounds3 ActorBounds = rigidActor->getWorldBounds();
						AABB tmp;
						tmp.mMin = ToPoint(ActorBounds.minimum);
						tmp.mMax = ToPoint(ActorBounds.maximum);
						GlobalBounds.Add(tmp);
					}

					if(FrustumPlanes)
					{
						// TODO: use fixed bounding sphere, don't recompute bounds
#ifdef USE_ACTOR_MANAGER_CACHE
						const PxBounds3& ActorBounds = CurrentCachedData.mGlobalBounds;
#else
						const PxBounds3 ActorBounds = rigidActor->getWorldBounds();
#endif
						udword OutClipMask;
						if(!PlanesAABBOverlap((const AABB&)ActorBounds, FrustumPlanes, OutClipMask, (1<<6)-1))
							continue;
						nbVisibleActors++;
					}

					if(CurrentActor.mSingleShape)
					{
						// TODO: keep current poses in actor manager (for sleeping objects)
#ifdef USE_ACTOR_MANAGER_CACHE
						const PxTransform& Pose = CurrentCachedData.mGlobalPose;
#else
						const PxTransform Pose = rigidActor->getGlobalPose();
#endif
						RenderShape(renderer, CurrentActor.mSingleShape, Pose);
					}
					else
					{
#ifdef USE_ACTOR_MANAGER_CACHE
						const PxTransform& ActorPose = CurrentCachedData.mGlobalPose;
#else
						const PxTransform ActorPose = rigidActor->getGlobalPose();
#endif
						PxU32 nbShapes = rigidActor->getNbShapes();
						for(PxU32 j=0;j<nbShapes;j++)
						{
							PxShape* shape = null;
							PxU32 nb = rigidActor->getShapes(&shape, 1, j);
							ASSERT(nb==1);
							ASSERT(shape);

							// Going through PxShapeExt::getGlobalPose recomputes the actor pose for each shape
/*#if PHYSX_SUPPORT_SHARED_SHAPES
							// TODO: keep current poses in actor manager (for sleeping objects)
							const PxTransform Pose = PxShapeExt::getGlobalPose(*shape, *rigidActor);
#else
							const PxTransform Pose = PxShapeExt::getGlobalPose(*shape);
#endif*/
							const PxTransform Pose = ActorPose * shape->getLocalPose();

							RenderShape(renderer, shape, Pose);
						}
					}
				}
			}
		}
	}
#ifdef RETIRED
	else
	{
	#ifdef IS_PHYSX_3_2
		const PxActorTypeSelectionFlags selectionFlags = PxActorTypeSelectionFlag::eRIGID_STATIC | PxActorTypeSelectionFlag::eRIGID_DYNAMIC;
	#else
		const PxActorTypeFlags selectionFlags = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
	#endif
		const PxU32 nbActors = mScene->getNbActors(selectionFlags);

	//	udword TotalTime=0;
	#define BUFFER_SIZE		2048
		PxActor* buffer[BUFFER_SIZE];
	//	PxActor** buffer = (PxActor**)ICE_ALLOC(sizeof(PxActor*)*nbActors);

		PxU32 nbProcessed = 0;
		while(nbProcessed!=nbActors)
		{
	//		udword Time;
	//		StartProfile(Time);
			const PxU32 nb = mScene->getActors(selectionFlags, buffer, BUFFER_SIZE, nbProcessed);
	//		const PxU32 nb = mScene->getActors(selectionFlags, buffer, nbActors, nbProcessed);
	//		EndProfile(Time);
	//		TotalTime += Time;
			nbProcessed += nb;

			for(PxU32 i=0;i<nb;i++)
			{
				PxActor* actor = buffer[i];
				const PxType type = actor->getConcreteType();
				if(type==PxConcreteType::eRIGID_STATIC || type==PxConcreteType::eRIGID_DYNAMIC)
				{
					PxRigidActor* rigidActor = static_cast<PxRigidActor*>(actor);

					if(gDumpSceneBoundsEachFrame)
					{
						const PxBounds3 ActorBounds = rigidActor->getWorldBounds();
						AABB tmp;
						tmp.mMin = ToPoint(ActorBounds.minimum);
						tmp.mMax = ToPoint(ActorBounds.maximum);
						GlobalBounds.Add(tmp);
					}

					if(FrustumPlanes)
					{
						const PxBounds3 ActorBounds = rigidActor->getWorldBounds();
						udword OutClipMask;
						if(!PlanesAABBOverlap((const AABB&)ActorBounds, FrustumPlanes, OutClipMask, (1<<6)-1))
							continue;
					}

					nbVisibleActors++;

	//				const PxTransform pose = rigidActor->getGlobalPose();

					PxU32 nbShapes = rigidActor->getNbShapes();
					for(PxU32 j=0;j<nbShapes;j++)
					{
						PxShape* shape = null;
						PxU32 nb = rigidActor->getShapes(&shape, 1, j);
						ASSERT(nb==1);
						ASSERT(shape);

	#if PHYSX_SUPPORT_SHARED_SHAPES
						// TODO: skip work when local pose is identity
						const PxTransform Pose = PxShapeExt::getGlobalPose(*shape, *rigidActor);
	//					const PxTransform Pose = rigidActor->getGlobalPose();
	#else
						const PxTransform Pose = PxShapeExt::getGlobalPose(*shape);
	#endif
						// TODO: don't create the ICE pose, convert to matrix directly here with SIMD, pass OpenGL data to renderers
						const PR IcePose(ToPoint(Pose.p), ToQuat(Pose.q));

	//					QuatGetMat33V(const QuatVArg q, Vec3V& column0, Vec3V& column1, Vec3V& column2)

						ASSERT(shape->userData);
						if(shape->userData)
						{
							PintShapeRenderer* shapeRenderer = (PintShapeRenderer*)shape->userData;

							const PxGeometryType::Enum geomType = shape->getGeometryType();
							if(geomType==PxGeometryType::eCAPSULE)
							{
								// ### PhysX is weird with capsules
	/*							Matrix3x3 Rot;
								Rot.RotZ(HALFPI);
								Quat QQ = IcePose.mRot * Quat(Rot);*/

								// ### precompute
	/*							const PxQuat q = PxShortestRotation(PxVec3(0.0f, 1.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.0f));
								Quat QQ = IcePose.mRot * ToQuat(q);

								shapeRenderer->Render(PR(IcePose.mPos, QQ));*/

								PR CapsuleTransform;
								PhysX3::ComputeCapsuleTransform(CapsuleTransform, IcePose);
								shapeRenderer->Render(CapsuleTransform);
							}
							else
							{
								shapeRenderer->Render(IcePose);
							}
						}
						// Reactivated for internal XPs
						else
						{
							const PxGeometryType::Enum geomType = shape->getGeometryType();
							if(geomType==PxGeometryType::eSPHERE)
							{
								PxSphereGeometry geometry;
								bool status = shape->getSphereGeometry(geometry);
								ASSERT(status);

								renderer.DrawSphere(geometry.radius, IcePose);
							}
							else if(geomType==PxGeometryType::eBOX)
							{
								PxBoxGeometry geometry;
								bool status = shape->getBoxGeometry(geometry);
								ASSERT(status);

								renderer.DrawBox(ToPoint(geometry.halfExtents), IcePose);
							}
							else if(geomType==PxGeometryType::eCAPSULE)
							{
								ASSERT(0);
							}
							else if(geomType==PxGeometryType::eCONVEXMESH)
							{
								ASSERT(0);
							}
							else ASSERT(0);
						}
					}
				}
			}
		}

	//	ICE_FREE(buffer);
	//	printf("Time: %d\n", TotalTime/1024);

		if(0)
			printf("%d visible actors\n", nbVisibleActors);

		const PxU32 NbArticulations = mScene->getNbArticulations();
		for(PxU32 i=0;i<NbArticulations;i++)
		{
			PxArticulation* Articulation;
			mScene->getArticulations(&Articulation, 1, i);
			const PxU32 NbLinks = Articulation->getNbLinks();
			PxArticulationLink* Links[256];
			PxU32 Nb = Articulation->getLinks(Links, 256);
			for(PxU32 jj=0;jj<NbLinks;jj++)
			{
				PxRigidActor* rigidActor = Links[jj];

				PxU32 nbShapes = rigidActor->getNbShapes();
				for(PxU32 j=0;j<nbShapes;j++)
				{
					PxShape* shape = null;
					PxU32 nb = rigidActor->getShapes(&shape, 1, j);
					ASSERT(nb==1);
					ASSERT(shape);

#if PHYSX_SUPPORT_SHARED_SHAPES
					const PxTransform Pose = PxShapeExt::getGlobalPose(*shape, *rigidActor);
#else
					const PxTransform Pose = PxShapeExt::getGlobalPose(*shape);
#endif
					const PR IcePose(ToPoint(Pose.p), ToQuat(Pose.q));

					ASSERT(shape->userData);
					if(shape->userData)
					{
						PintShapeRenderer* shapeRenderer = (PintShapeRenderer*)shape->userData;

						const PxGeometryType::Enum geomType = shape->getGeometryType();
						if(geomType==PxGeometryType::eCAPSULE)
						{
							// ### PhysX is weird with capsules
/*							Matrix3x3 Rot;
							Rot.RotZ(HALFPI);
							Quat QQ = IcePose.mRot * Quat(Rot);*/

							// ### precompute
/*							const PxQuat q = PxShortestRotation(PxVec3(0.0f, 1.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.0f));
							Quat QQ = IcePose.mRot * ToQuat(q);

							shapeRenderer->Render(PR(IcePose.mPos, QQ));*/
							PR CapsuleTransform;
							PhysX3::ComputeCapsuleTransform(CapsuleTransform, IcePose);
							shapeRenderer->Render(CapsuleTransform);
						}
						else
						{
							shapeRenderer->Render(IcePose);
						}
					}
					// Reactivated for internal XPs
					else
					{
						const PxGeometryType::Enum geomType = shape->getGeometryType();
						if(geomType==PxGeometryType::eSPHERE)
						{
							PxSphereGeometry geometry;
							bool status = shape->getSphereGeometry(geometry);
							ASSERT(status);

							renderer.DrawSphere(geometry.radius, IcePose);
						}
						else if(geomType==PxGeometryType::eBOX)
						{
							PxBoxGeometry geometry;
							bool status = shape->getBoxGeometry(geometry);
							ASSERT(status);

							renderer.DrawBox(ToPoint(geometry.halfExtents), IcePose);
						}
						else if(geomType==PxGeometryType::eCAPSULE)
						{
							PxCapsuleGeometry geometry;
							bool status = shape->getCapsuleGeometry(geometry);
							ASSERT(status);

							// ### precompute
/*							const PxQuat q = PxShortestRotation(PxVec3(0.0f, 1.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.0f));
							Quat QQ = IcePose.mRot * ToQuat(q);

							renderer.DrawCapsule(geometry.radius, geometry.halfHeight*2.0f, PR(IcePose.mPos, QQ));*/
							PR CapsuleTransform;
							PhysX3::ComputeCapsuleTransform(CapsuleTransform, IcePose);
							renderer.DrawCapsule(geometry.radius, geometry.halfHeight*2.0f, CapsuleTransform);
						}
						else if(geomType==PxGeometryType::eCONVEXMESH)
						{
							ASSERT(0);
						}
						else ASSERT(0);
					}
				}
			}
		}
	}
#endif

	if(gDumpSceneBoundsEachFrame)
	{
		printf("Min.x = %f\n", GlobalBounds.mMin.x);
		printf("Min.y = %f\n", GlobalBounds.mMin.y);
		printf("Min.z = %f\n", GlobalBounds.mMin.z);
		printf("Max.x = %f\n", GlobalBounds.mMax.x);
		printf("Max.y = %f\n", GlobalBounds.mMax.y);
		printf("Max.z = %f\n", GlobalBounds.mMax.z);
		printf("\n");
	}
}

void SharedPhysX::RenderDebugData(PintRender& renderer)
{
	if(!mScene)
		return;

#ifdef TEST_FLUIDS
	if(gParticleSystem)
		renderParticles(renderer, gParticleSystem);
#endif

	const PxRenderBuffer& RenderBuffer = mScene->getRenderBuffer();
//	const PxRenderBuffer& RenderBuffer = mScene->fillRenderBuffer();
	udword NbLines = RenderBuffer.getNbLines();
	const PxDebugLine* Lines = RenderBuffer.getLines();

	if(0)
	{
		for(udword i=0;i<NbLines;i++)
		{
			Point LineColor;
			LineColor.z = float(Lines[i].color0&0xff)/255.0f;
			LineColor.y = float((Lines[i].color0>>8)&0xff)/255.0f;
			LineColor.x = float((Lines[i].color0>>16)&0xff)/255.0f;
			renderer.DrawLine(ToPoint(Lines[i].pos0), ToPoint(Lines[i].pos1), LineColor);
		}
	}
	else
	{
		mDebugVizHelper.Reset();

		Point CurrentColor(0.0f, 0.0f, 0.0f);
		udword PrevColor = 0;

		//printf("NbLines: %d\n", NbLines);

		for(udword i=0;i<NbLines;i++)
		{
			const udword NextColor = Lines[i].color0;
			if(NextColor!=PrevColor)
			{
				renderer.DrawLines(mDebugVizHelper.GetNbVertices()/2, mDebugVizHelper.GetVertices(), CurrentColor);
				mDebugVizHelper.Reset();

				PrevColor = NextColor;
				CurrentColor.z = float(NextColor&0xff)/255.0f;
				CurrentColor.y = float((NextColor>>8)&0xff)/255.0f;
				CurrentColor.x = float((NextColor>>16)&0xff)/255.0f;
			}

			mDebugVizHelper.AddVertex(ToPoint(Lines[i].pos0));
			mDebugVizHelper.AddVertex(ToPoint(Lines[i].pos1));
		}
		renderer.DrawLines(mDebugVizHelper.GetNbVertices()/2, mDebugVizHelper.GetVertices(), CurrentColor);
		mDebugVizHelper.Reset();
	}

	udword NbTris = RenderBuffer.getNbTriangles();
	const PxDebugTriangle* Triangles = RenderBuffer.getTriangles();
	const Point TrisColor(1.0f, 1.0f, 1.0f);
	for(udword i=0;i<NbTris;i++)
	{
		renderer.DrawTriangle(ToPoint(Triangles[i].pos0), ToPoint(Triangles[i].pos1), ToPoint(Triangles[i].pos2), TrisColor);
	}

#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
	if(gVisualizeMBPRegions && mParams.mBroadPhaseType == PxBroadPhaseType::eMBP)
	{
		PxU32 NbRegions = mScene->getNbBroadPhaseRegions();
		for(PxU32 i=0;i<NbRegions;i++)
		{
			PxBroadPhaseRegionInfo Region;
			mScene->getBroadPhaseRegions(&Region, 1, i);
	#if PHYSX_SUPPORT_PX_BROADPHASE_PABP
			if(Region.mActive)
	#else
			if(Region.active)
	#endif
			{
	#if PHYSX_SUPPORT_PX_BROADPHASE_PABP
				const Point m = ToPoint(Region.mRegion.mBounds.minimum);
				const Point M = ToPoint(Region.mRegion.mBounds.maximum);
	#else
				const Point m = ToPoint(Region.region.bounds.minimum);
				const Point M = ToPoint(Region.region.bounds.maximum);
	#endif
				AABB Bounds;
				Bounds.SetMinMax(m, M);

				renderer.DrawWireframeAABB(1, &Bounds, Point(1.0f, 0.0f, 0.0f));
			}
		}
	}
#endif
}

#ifdef PHYSX_SUPPORT_CHARACTERS
PxControllerManager* SharedPhysX::GetControllerManager()
{
	ASSERT(mScene);
	PxControllerManager* CM = PxCreateControllerManager(*mScene, false);
	mControllerManager = CM;
	return CM;
}

void SharedPhysX::ReleaseControllerManager()
{
	SAFE_RELEASE(mControllerManager);
}

PintCharacterHandle	SharedPhysX::CreateCharacter(const PINT_CHARACTER_CREATE& create)
{
	PxCapsuleControllerDesc capsuleCCTDesc;
	capsuleCCTDesc.height		= create.mCapsule.mHalfHeight*2.0f;
	capsuleCCTDesc.radius		= create.mCapsule.mRadius;
	capsuleCCTDesc.position		= PxExtendedVec3(create.mPosition.x, create.mPosition.y, create.mPosition.z);
	capsuleCCTDesc.scaleCoeff	= 0.8f;
	capsuleCCTDesc.scaleCoeff	= 1.0f;
	capsuleCCTDesc.stepOffset	= 0.1f;
	capsuleCCTDesc.slopeLimit	= 0.0f;
	capsuleCCTDesc.material		= mDefaultMaterial;

	PxControllerManager* CM = GetControllerManager();
	PxController* CCT = CM->createController(capsuleCCTDesc);
	if(CCT)
	{
		PxRigidDynamic* dyna = CCT->getActor();
		if(dyna)
		{
			PxShape* shape = null;
			dyna->getShapes(&shape, 1);
			if(shape)
				shape->userData = create.mCapsule.mRenderer;
			AddActorToManager(dyna);
		}
	}
	return CCT;
}

PintActorHandle SharedPhysX::GetCharacterActor(PintCharacterHandle h)
{
	PxController* CCT = (PxController*)h;
	if(!CCT)
		return null;
	return CCT->getActor();
}

udword SharedPhysX::MoveCharacter(PintCharacterHandle h, const Point& disp)
{
	PxController* CCT = (PxController*)h;
	if(!CCT)
		return 0;

	const PxF32 minDist = 0.0f;
	const PxF32 elapsedTime = 0.0f;

	const PxControllerFilters filters;

	return CCT->move(ToPxVec3(disp), minDist, elapsedTime, filters, null);
}
#endif

///////////////////////////////////////////////////////////////////////////////

enum PhysXGUIElement
{
	PHYSX_GUI_MAIN,
	//
	PHYSX_GUI_TEST,
	//
	PHYSX_GUI_ENABLE_SLEEPING,
	PHYSX_GUI_ENABLE_SQ,
	PHYSX_GUI_ENABLE_CCD,
#if PHYSX_SUPPORT_ANGULAR_CCD
	PHYSX_GUI_ENABLE_ANGULAR_CCD,
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
	PHYSX_GUI_ENABLE_RAYCAST_CCD,
	PHYSX_GUI_ENABLE_RAYCAST_CCD_DYNA_DYNA,
#endif
	PHYSX_GUI_SQ_FILTER_OUT,
	PHYSX_GUI_SQ_INITIAL_OVERLAP,
//	PHYSX_GUI_SQ_MANUAL_FLUSH_UPDATES,
	PHYSX_GUI_SQ_PRECISE_SWEEPS,
	PHYSX_GUI_SQ_BOTH_SIDES,
	PHYSX_GUI_SHARE_MESH_DATA,
	PHYSX_GUI_SHARE_SHAPES,
#if PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS
	PHYSX_GUI_TIGHT_CONVEX_BOUNDS,
#endif
	PHYSX_GUI_PCM,
#if PHYSX_SUPPORT_ADAPTIVE_FORCE
	PHYSX_GUI_ADAPTIVE_FORCE,
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
	PHYSX_GUI_STABILIZATION,
#endif
#if PHYSX_SUPPORT_TGS
	PHYSX_GUI_TGS,
#endif
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
	PHYSX_GUI_GYRO,
#endif
#ifdef PHYSX_SUPPORT_SSE_FLAG
	PHYSX_GUI_ENABLE_SSE,
#endif
	PHYSX_GUI_ENABLE_ACTIVE_TRANSFORMS,
	PHYSX_GUI_ENABLE_CONTACT_CACHE,
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
	PHYSX_GUI_ENABLE_CONTACT_NOTIF,
#endif
	PHYSX_GUI_FLUSH_SIMULATION,
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
	PHYSX_GUI_IMPROVED_PATCH_FRICTION,
#endif
#if PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
	PHYSX_GUI_FRICTION_EVERY_ITERATION,
#endif
	PHYSX_GUI_DISABLE_STRONG_FRICTION,
	PHYSX_GUI_ENABLE_ONE_DIR_FRICTION,
	PHYSX_GUI_ENABLE_TWO_DIR_FRICTION,
	PHYSX_GUI_USE_PVD,
	PHYSX_GUI_USE_FULL_PVD_CONNECTION,
//	PHYSX_GUI_DRAW_MBP_REGIONS,
#ifdef PHYSX_SUPPORT_GPU
	PHYSX_GUI_USE_GPU,
#endif
	//
	PHYSX_GUI_NB_THREADS,
	PHYSX_GUI_STATIC_PRUNER,
	PHYSX_GUI_DYNAMIC_PRUNER,
#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
	PHYSX_GUI_BROAD_PHASE,
#endif
	PHYSX_GUI_SCRATCH_BUFFER,
	//
//	PHYSX_GUI_NUM_16K_CONTACT_DATA_BLOCKS,
	PHYSX_GUI_GLOBAL_BOX_SIZE,
#if PHYSX_SUPPORT_SUBSTEPS
	PHYSX_GUI_CONFIG_DESC,
	PHYSX_GUI_CONFIG_COMBO_BOX,
#endif
	//
	PHYSX_GUI_USE_D6_JOINT,
#if PHYSX_SUPPORT_DISABLE_PREPROCESSING
	PHYSX_GUI_DISABLE_PREPROCESSING,
#endif
#ifndef IS_PHYSX_3_2
	#if PHYSX_REMOVE_JOINT_32_COMPATIBILITY
	#else
	PHYSX_GUI_ENABLE_JOINT_32_COMPATIBILITY,
	#endif
#endif
	PHYSX_GUI_ENABLE_JOINT_PROJECTION,
//	PHYSX_GUI_ENABLE_JOINT_CONTACT_DISTANCE,
	PHYSX_GUI_LIMITS_CONTACT_DISTANCE,
#if PHYSX_SUPPORT_ARTICULATIONS || PHYSX_SUPPORT_RCA
	PHYSX_GUI_DISABLE_ARTICULATIONS,
#endif
	// Cooking
#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
	PHYSX_GUI_MESH_COOKING_HINT,
#endif
#if PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
	PHYSX_GUI_MESH_BUILD_STRATEGY,
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE
	PHYSX_GUI_MID_PHASE,
#endif
#if PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE
	PHYSX_GUI_PRECOMPUTE_ACTIVE_EDGES,
#endif
#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
	PHYSX_GUI_QUANTIZED_TREES,
#endif
	// Vehicles
#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
	PHYSX_GUI_USE_SUSPENSION_SWEEPS,
	PHYSX_GUI_FORCE_SPHERE_WHEELS,
	PHYSX_GUI_USE_BLOCKING_SWEEPS,
	PHYSX_GUI_DRAW_SWEPT_WHEELS,
#endif
	PHYSX_GUI_DRAW_SUSPENSION_CASTS,
	PHYSX_GUI_DRAW_PHYSX_TELEMETRY,
#ifdef PHYSX_SUPPORT_VEHICLE_FIX
	PHYSX_GUI_USE_FIX,
#endif
#if PHYSX_SUPPORT_FLUIDS
	PHYSX_GUI_FLUID_PARTICLE_CCD,
#endif

	//
	PHYSX_GUI_DEBUG_VIZ_SCALE,
	PHYSX_GUI_ENABLE_DEBUG_VIZ,	// MUST BE LAST
};

EditableParams::EditableParams() :
	// Main
	mNbThreadsIndex				(0),
#if PHYSX_SUPPORT_SCRATCH_BUFFER
	mScratchSize				(0),
#endif
#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
	#if PHYSX_SUPPORT_PX_BROADPHASE_PABP
		mBroadPhaseType			(PxBroadPhaseType::ePABP),
	#else
		#if PHYSX_SUPPORT_PX_BROADPHASE_ABP
		mBroadPhaseType			(PxBroadPhaseType::eABP),
		#else
		mBroadPhaseType			(PxBroadPhaseType::eSAP),
		#endif
	#endif
	mMBPSubdivLevel				(4),
	mMBPRange					(1000.0f),
#endif
	mEnableCCD					(false),
#if PHYSX_SUPPORT_ANGULAR_CCD
	mEnableAngularCCD			(false),
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
	mEnableRaycastCCDStatic		(false),
	mEnableRaycastCCDDynamic	(false),
#endif
	mShareMeshData				(true),
	mShareShapes				(true),
#if PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS
	mUseTightConvexBounds		(true),
#endif
	mPCM						(true),
#ifdef PHYSX_SUPPORT_SSE_FLAG
	mEnableSSE					(true),
#endif
	mEnableActiveTransforms		(false),
	mEnableContactCache			(true),
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
	mEnableContactNotif			(false),
#endif
	mFlushSimulation			(false),
#ifdef PINT_SUPPORT_PVD	// Defined in project's properties
	mUsePVD						(true),
#else
	mUsePVD						(false),
#endif
	mUseFullPvdConnection		(true),
#ifdef PHYSX_SUPPORT_GPU
	mUseGPU						(false),
#endif
	//mGlobalBoxSize			(10000.0f),
	//mContactOffset			(0.002f),
	mContactOffset				(0.02f),
	mRestOffset					(0.0f),
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
	mContactNotifThreshold		(0.0f),
#endif
#if PHYSX_SUPPORT_SUBSTEPS
	mNbSubsteps					(1),
#endif
	// Dynamics
	mMaxBiasCoeff				(-1.0f),
	mDefaultStaticFriction		(0.5f),
	mDefaultDynamicFriction		(0.5f),
	mFrictionOffsetThreshold	(0.04f),
#if PHYSX_SUPPORT_TORSION_FRICTION
	mTorsionalPatchRadius		(0.0f),
	mMinTorsionalPatchRadius	(0.0f),
#endif
	mEnableSleeping				(false),
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
	mImprovedPatchFriction		(false),
#endif
#if PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
	mFrictionEveryIteration		(false),
#endif
	mDisableStrongFriction		(false),
	mEnableOneDirFriction		(false),
	mEnableTwoDirFriction		(false),
#if PHYSX_SUPPORT_ADAPTIVE_FORCE
	mAdaptiveForce				(false),
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
	mStabilization				(true),
#endif
#if PHYSX_SUPPORT_TGS
	mTGS						(false),
#endif
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
	mGyro						(false),
#endif
#ifndef IS_PHYSX_3_2
	mMaxNbCCDPasses				(1),
#endif
	mSolverIterationCountPos	(4),
	mSolverIterationCountVel	(1),
	mLinearDamping				(0.1f),
	mAngularDamping				(0.05f),
	mMaxAngularVelocity			(100.0f),
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
	mMaxDepenVelocity			(3.0f),
#endif
	mSleepThreshold				(0.05f),	// 0.05		0.01 / 0.02
#if PHYSX_SUPPORT_STABILIZATION_FLAG
	mStabilizationThreshold		(0.0025f),	// 0.0025	0.001
#endif
	// Scene queries
	mStaticPruner				(PxPruningStructureType::eDYNAMIC_AABB_TREE),
	mDynamicPruner				(PxPruningStructureType::eDYNAMIC_AABB_TREE),
	mSQDynamicRebuildRateHint	(100),
	mSQFlag						(true),
	mSQFilterOutAllShapes		(false),
	mSQInitialOverlap			(false),
	//mSQManualFlushUpdates		(true),
	mSQPreciseSweeps			(false),
	mSQBothSides				(false),
	// Joints
	mEnableJointProjection		(false),
//	mEnableJointContactDistance	(false),
	mUseD6Joint					(false),
#if PHYSX_SUPPORT_DISABLE_PREPROCESSING
	mDisablePreprocessing		(false),
#endif
#ifndef IS_PHYSX_3_2
	#if PHYSX_REMOVE_JOINT_32_COMPATIBILITY
	#else
	mEnableJoint32Compatibility	(false),
	#endif
#endif
	mProjectionLinearTolerance	(0.1f),
	mProjectionAngularTolerance	(180.0f),
#ifndef IS_PHYSX_3_2
	mInverseInertiaScale		(1.0f),
	mInverseMassScale			(1.0f),
#endif
#ifdef PHYSX_SUPPORT_LINEAR_COEFF
	mLinearCoeff				(0.0f),
#endif
	mLimitsContactDistance		(0),
	// Articulations
#if PHYSX_SUPPORT_ARTICULATIONS || PHYSX_SUPPORT_RCA
	mDisableArticulations		(false),
#endif
#if PHYSX_SUPPORT_ARTICULATIONS
	mMaxProjectionIterations	(16),
	mSeparationTolerance		(0.001f),
	mExternalDriveIterations	(4),
	mInternalDriveIterations	(4),
#endif
#if PHYSX_SUPPORT_RCA_CFM_SCALE
	mRCACfmScale				(0.025f),
#endif
#if PHYSX_SUPPORT_RCA_DOF_SCALE
	mRCADofScale				(1.0f),
#endif
#if PHYSX_SUPPORT_RCA_ARMATURE
	mRCAArmature				(0.0f),
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE
	mMidPhaseType				(PxMeshMidPhase::eBVH34),
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
	mNbTrisPerLeaf				(4),
#endif
#if PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
	mMeshBuildStrategy			(PxBVH34BuildStrategy::eDEFAULT),
#endif
#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
	mMeshCookingHint			(PxMeshCookingHint::eSIM_PERFORMANCE),
#endif
#if PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT
	mGaussMapLimit				(32),
#endif
#if PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE
	mPrecomputeActiveEdges		(true),
#endif
#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
	mQuantizedTrees				(true),
#endif
#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
	mUseSuspensionSweeps		(true),
	mForceSphereWheels			(false),
	mUseBlockingSweeps			(true),
	mDrawSweptWheels			(false),
#endif
	mDrawSuspensionCasts		(false),
	mDrawPhysXTelemetry			(false),
#ifdef PHYSX_SUPPORT_VEHICLE_FIX
	mUseFix						(false),
#endif
#if PHYSX_SUPPORT_STEER_FILTER
	mSteerFilter				(2.5f),
#endif
#if PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
	mSweepInflation				(0.01f),
#endif
#if PHYSX_SUPPORT_FLUIDS
	// Fluids
	mFluidFriction				(0.05f),
	mFluidDamping				(0.05f),
	mFluidAdhesion				(0.0f),
	mFluidViscosity				(0.001f),
//	mFluidVorticityConfinement	(0.5f),
	mFluidVorticityConfinement	(10.0f),
//	mFluidSurfaceTension		(0.005f),
	mFluidSurfaceTension		(0.00704f),
//	mFluidCohesion				(0.01f),
	mFluidCohesion				(0.00704f),
	mFluidLift					(0.0f),
	mFluidDrag					(0.0f),
	mFluidCflCoefficient		(1.0f),
	mFluidGravityScale			(1.0f),
	mFluidParticleMass			(4.0f),
	mFluidParticleCCD			(false),
#endif
	mDebugVizScale				(1.0f),
	mForceJointLimitDebugViz	(false),
	mLast						(true)
{
}

static EditableParams gParams;

const EditableParams& PhysX3::GetEditableParams()
{
	return gParams;
}

#define MAX_NB_DEBUG_VIZ_PARAMS	32

	struct PhysXUI : public Allocateable
	{
						PhysXUI(UICallback& callback, udword nb_debug_viz_params, bool* debug_viz_params, const char** debug_viz_names);
						~PhysXUI();

		UICallback&		mCallback;

		udword			mNbDebugVizParams;
		bool*			mDebugVizParams;
		const char**	mDebugVizNames;

		Widgets*		mPhysXGUI;
		ComboBoxPtr		mComboBox_NbThreads;
		ComboBoxPtr		mComboBox_StaticPruner;
		ComboBoxPtr		mComboBox_DynamicPruner;
		ComboBoxPtr		mComboBox_LimitsContactDistance;
#if PHYSX_SUPPORT_SCRATCH_BUFFER
		ComboBoxPtr		mComboBox_ScratchSize;
#endif
#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
		ComboBoxPtr		mComboBox_BroadPhase;
		EditBoxPtr		mEditBox_MBPSubdivLevel;
		EditBoxPtr		mEditBox_MBPRange;
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE
		ComboBoxPtr		mComboBox_MidPhase;
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
		EditBoxPtr		mEditBox_NbTrisPerLeaf;
#endif
#if PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
		ComboBoxPtr		mComboBox_MeshBuildStrategy;
#endif
#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
		ComboBoxPtr		mComboBox_MeshCookingHint;
#endif
		EditBoxPtr		mEditBox_ProjectionLinearTolerance;
		EditBoxPtr		mEditBox_ProjectionAngularTolerance;
		EditBoxPtr		mEditBox_InverseInertiaScale;
		EditBoxPtr		mEditBox_InverseMassScale;
#ifdef PHYSX_SUPPORT_LINEAR_COEFF
		EditBoxPtr		mEditBox_LinearCoeff;
#endif
#if PHYSX_SUPPORT_ARTICULATIONS
		EditBoxPtr		mEditBox_MaxProjectionIterations;
		EditBoxPtr		mEditBox_SeparationTolerance;
		EditBoxPtr		mEditBox_ExternalDriveIterations;
		EditBoxPtr		mEditBox_InternalDriveIterations;
#endif
#if PHYSX_SUPPORT_RCA_CFM_SCALE
		EditBoxPtr		mEditBox_RCACfmScale;
#endif
#if PHYSX_SUPPORT_RCA_DOF_SCALE
		EditBoxPtr		mEditBox_RCADofScale;
#endif
#if PHYSX_SUPPORT_RCA_ARMATURE
		EditBoxPtr		mEditBox_RCAArmature;
#endif
		EditBoxPtr		mEditBox_MaxNbCCDPasses;
		EditBoxPtr		mEditBox_SolverIterPos;
		EditBoxPtr		mEditBox_SolverIterVel;
		EditBoxPtr		mEditBox_LinearDamping;
		EditBoxPtr		mEditBox_AngularDamping;
		EditBoxPtr		mEditBox_MaxAngularVelocity;
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
		EditBoxPtr		mEditBox_MaxDepenVelocity;
#endif
		//EditBoxPtr	mEditBox_GlobalBoxSize;
		EditBoxPtr		mEditBox_DefaultStaticFriction;
		EditBoxPtr		mEditBox_DefaultDynamicFriction;
		EditBoxPtr		mEditBox_FrictionOffsetThreshold;
		EditBoxPtr		mEditBox_MaxBiasCoeff;
#if PHYSX_SUPPORT_TORSION_FRICTION
		EditBoxPtr		mEditBox_TorsionalPatchRadius;
		EditBoxPtr		mEditBox_MinTorsionalPatchRadius;
#endif
		EditBoxPtr		mEditBox_ContactOffset;
		EditBoxPtr		mEditBox_RestOffset;
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
		EditBoxPtr		mEditBox_ContactNotifThreshold;
#endif
#if PHYSX_SUPPORT_SUBSTEPS
		EditBoxPtr		mEditBox_NbSubsteps;
		EditBoxPtr		mEditBox_ConfigDesc;
		ComboBoxPtr		mComboBox_Config;
#endif
		EditBoxPtr		mEditBox_SleepThreshold;
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		EditBoxPtr		mEditBox_StabilizationThreshold;
#endif
#if PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT
		EditBoxPtr		mEditBox_GaussMapLimit;
#endif
#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
		CheckBoxPtr		mCheckBox_QuantizedTrees;
#endif
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
		CheckBoxPtr		mCheckBox_ImprovedPatchFriction;
#endif
#if PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
		CheckBoxPtr		mCheckBox_FrictionEveryIteration;
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		CheckBoxPtr		mCheckBox_Stabilization;
#endif
#if PHYSX_SUPPORT_TGS
		CheckBoxPtr		mCheckBox_TGS;
#endif
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
		CheckBoxPtr		mCheckBox_Gyro;
#endif
		CheckBoxPtr		mCheckBox_CCD;
#if PHYSX_SUPPORT_ANGULAR_CCD
		CheckBoxPtr		mCheckBox_AngularCCD;
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
		CheckBoxPtr		mCheckBox_RaycastCCDStatic;
		CheckBoxPtr		mCheckBox_RaycastCCDDynamic;
#endif
		CheckBoxPtr		mCheckBox_Sleeping;
		CheckBoxPtr		mCheckBox_PVD;
		CheckBoxPtr		mCheckBox_FullPVD;
		CheckBoxPtr		mCheckBox_SQ_FilterOutAllShapes;
		CheckBoxPtr		mCheckBox_SQ_InitialOverlap;
		//CheckBoxPtr	mCheckBox_SQ_ManualFlushUpdates;
		CheckBoxPtr		mCheckBox_SQ_PreciseSweeps;
		CheckBoxPtr		mCheckBox_SQ_BothSides;
		EditBoxPtr		mEditBox_SQ_RebuildRateHint;
		//CheckBoxPtr	mCheckBox_DrawMBPRegions;
#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
		CheckBoxPtr		mCheckBox_UseSuspensionSweeps;
		CheckBoxPtr		mCheckBox_ForceSphereWheels;
		CheckBoxPtr		mCheckBox_UseBlockingSweeps;
		CheckBoxPtr		mCheckBox_DrawSweptWheels;
#endif
		CheckBoxPtr		mCheckBox_DrawSuspensionCasts;
		CheckBoxPtr		mCheckBox_DrawPhysXTelemetry;
		CheckBoxPtr		mCheckBox_UseFix;
#if PHYSX_SUPPORT_STEER_FILTER
		EditBoxPtr		mEditBox_SteerFilter;
#endif
#if PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
		EditBoxPtr		mEditBox_SweepInflation;
#endif
#if PHYSX_SUPPORT_FLUIDS
		EditBoxPtr		mEditBox_FluidFriction;
		EditBoxPtr		mEditBox_FluidDamping;
		EditBoxPtr		mEditBox_FluidAdhesion;
		EditBoxPtr		mEditBox_FluidViscosity;
		EditBoxPtr		mEditBox_FluidVorticityConfinement;
		EditBoxPtr		mEditBox_FluidSurfaceTension;
		EditBoxPtr		mEditBox_FluidCohesion;
		EditBoxPtr		mEditBox_FluidLift;
		EditBoxPtr		mEditBox_FluidDrag;
		EditBoxPtr		mEditBox_FluidCflCoefficient;
		EditBoxPtr		mEditBox_FluidGravityScale;
		EditBoxPtr		mEditBox_FluidParticleMass;
		CheckBoxPtr		mCheckBox_FluidParticleCCD;
#endif
		EditBoxPtr		mEditBox_DebugVizScale;
		IceCheckBox*	mCheckBox_DebugVis[MAX_NB_DEBUG_VIZ_PARAMS];
	};

PhysXUI::PhysXUI(UICallback& callback, udword nb_debug_viz_params, bool* debug_viz_params, const char** debug_viz_names) :
	mCallback			(callback),
	mNbDebugVizParams	(nb_debug_viz_params),
	mDebugVizParams		(debug_viz_params),
	mDebugVizNames		(debug_viz_names),
	mPhysXGUI			(null)
{
	ASSERT(nb_debug_viz_params<MAX_NB_DEBUG_VIZ_PARAMS);
	for(udword i=0;i<MAX_NB_DEBUG_VIZ_PARAMS;i++)
		mCheckBox_DebugVis[i] = null;
}

PhysXUI::~PhysXUI()
{
	Common_CloseGUI(mPhysXGUI);
}

static PhysXUI* gPhysXUI = null;
extern "C"	__declspec(dllexport)	PintPlugin*	GetPintPlugin();

static void GetDebugVizOptionsFromGUI()
{
	if(!gPhysXUI)
		return;

	Common_GetFromEditBox(gParams.mDebugVizScale, gPhysXUI->mEditBox_DebugVizScale, 0.0f, FLT_MAX);
	for(udword i=0;i<gNbDebugVizParams;i++)
		gDebugVizParams[i] = gPhysXUI->mCheckBox_DebugVis[i]->IsChecked();
}

//	TODO: revisit gPEEL_GetOptionsFromGUI: some edit boxes are missing here so is it really useful? => the ones here are changed when test is reset, not at runtime
//	===> it's mandatory to add params there that can be changed by the per-test UI, otherwise they're not reset!
void PhysX3::GetOptionsFromGUI(const char* test_name)
{
	if(!gPhysXUI)
		return;

	// TODO: add missing UI elements here

	if(gPhysXUI->mComboBox_NbThreads)
	{
		const udword Index = gPhysXUI->mComboBox_NbThreads->GetSelectedIndex();
		gParams.mNbThreadsIndex = Index;
	}

	if(gPhysXUI->mComboBox_StaticPruner)
	{
		const udword Index = gPhysXUI->mComboBox_StaticPruner->GetSelectedIndex();
		gParams.mStaticPruner = PxPruningStructureType::Enum(Index);
	}

	if(gPhysXUI->mComboBox_DynamicPruner)
	{
		const udword Index = gPhysXUI->mComboBox_DynamicPruner->GetSelectedIndex();
		gParams.mDynamicPruner = PxPruningStructureType::Enum(Index);
	}

	if(gPhysXUI->mComboBox_LimitsContactDistance)
	{
		const udword Index = gPhysXUI->mComboBox_LimitsContactDistance->GetSelectedIndex();
		gParams.mLimitsContactDistance = Index;
	}

#if PHYSX_SUPPORT_SCRATCH_BUFFER
	if(gPhysXUI->mComboBox_ScratchSize)
	{
		const udword Index = gPhysXUI->mComboBox_ScratchSize->GetSelectedIndex();
		gParams.mScratchSize = Index;
	}
#endif

#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
	if(gPhysXUI->mComboBox_BroadPhase)
	{
		const udword Index = gPhysXUI->mComboBox_BroadPhase->GetSelectedIndex();
		gParams.mBroadPhaseType = PxBroadPhaseType::Enum(Index);
	}
#endif

#if PHYSX_SUPPORT_PX_MESH_MIDPHASE
	if(gPhysXUI->mComboBox_MidPhase)
	{
		const udword Index = gPhysXUI->mComboBox_MidPhase->GetSelectedIndex();
		gParams.mMidPhaseType = PxMeshMidPhase::Enum(Index);
	}
#endif

#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
	Common_GetFromEditBox(gParams.mNbTrisPerLeaf, gPhysXUI->mEditBox_NbTrisPerLeaf);
#endif

#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
	if(gPhysXUI->mComboBox_MeshCookingHint)
	{
		const udword Index = gPhysXUI->mComboBox_MeshCookingHint->GetSelectedIndex();
		gParams.mMeshCookingHint = PxMeshCookingHint::Enum(Index);
	}
#endif
#if PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
	if(gPhysXUI->mComboBox_MeshBuildStrategy)
	{
		const udword Index = gPhysXUI->mComboBox_MeshBuildStrategy->GetSelectedIndex();
		gParams.mMeshBuildStrategy = PxBVH34BuildStrategy::Enum(Index);
	}
#endif

#ifndef IS_PHYSX_3_2
	Common_GetFromEditBox(gParams.mInverseInertiaScale, gPhysXUI->mEditBox_InverseInertiaScale, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mInverseMassScale, gPhysXUI->mEditBox_InverseMassScale, 0.0f, FLT_MAX);
#endif
#ifdef PHYSX_SUPPORT_LINEAR_COEFF
	Common_GetFromEditBox(gParams.mLinearCoeff, gPhysXUI->mEditBox_LinearCoeff, 0.0f, FLT_MAX);
#endif
#if PHYSX_SUPPORT_ARTICULATIONS
	Common_GetFromEditBox(gParams.mMaxProjectionIterations, gPhysXUI->mEditBox_MaxProjectionIterations);
	Common_GetFromEditBox(gParams.mSeparationTolerance, gPhysXUI->mEditBox_SeparationTolerance, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mInternalDriveIterations, gPhysXUI->mEditBox_InternalDriveIterations);
	Common_GetFromEditBox(gParams.mExternalDriveIterations, gPhysXUI->mEditBox_ExternalDriveIterations);
#endif
#if PHYSX_SUPPORT_RCA_CFM_SCALE
	Common_GetFromEditBox(gParams.mRCACfmScale, gPhysXUI->mEditBox_RCACfmScale, 0.0f, FLT_MAX);
#endif
#if PHYSX_SUPPORT_RCA_DOF_SCALE
	Common_GetFromEditBox(gParams.mRCADofScale, gPhysXUI->mEditBox_RCADofScale, 0.0f, FLT_MAX);
#endif
#if PHYSX_SUPPORT_RCA_ARMATURE
	Common_GetFromEditBox(gParams.mRCAArmature, gPhysXUI->mEditBox_RCAArmature, 0.0f, FLT_MAX);
#endif
	Common_GetFromEditBox(gParams.mProjectionLinearTolerance, gPhysXUI->mEditBox_ProjectionLinearTolerance, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mProjectionAngularTolerance, gPhysXUI->mEditBox_ProjectionAngularTolerance, 0.0f, FLT_MAX);
#ifndef IS_PHYSX_3_2
	Common_GetFromEditBox(gParams.mMaxNbCCDPasses, gPhysXUI->mEditBox_MaxNbCCDPasses);
#endif
	Common_GetFromEditBox(gParams.mSolverIterationCountPos, gPhysXUI->mEditBox_SolverIterPos);
	Common_GetFromEditBox(gParams.mSolverIterationCountVel, gPhysXUI->mEditBox_SolverIterVel);
	Common_GetFromEditBox(gParams.mLinearDamping, gPhysXUI->mEditBox_LinearDamping, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mAngularDamping, gPhysXUI->mEditBox_AngularDamping, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mMaxAngularVelocity, gPhysXUI->mEditBox_MaxAngularVelocity, 0.0f, FLT_MAX);
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
	Common_GetFromEditBox(gParams.mMaxDepenVelocity, gPhysXUI->mEditBox_MaxDepenVelocity, 0.0f, FLT_MAX);
#endif
	Common_GetFromEditBox(gParams.mMaxBiasCoeff, gPhysXUI->mEditBox_MaxBiasCoeff, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mDefaultStaticFriction, gPhysXUI->mEditBox_DefaultStaticFriction, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mDefaultDynamicFriction, gPhysXUI->mEditBox_DefaultDynamicFriction, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mFrictionOffsetThreshold, gPhysXUI->mEditBox_FrictionOffsetThreshold, 0.0f, FLT_MAX);
#if PHYSX_SUPPORT_TORSION_FRICTION
	Common_GetFromEditBox(gParams.mTorsionalPatchRadius, gPhysXUI->mEditBox_TorsionalPatchRadius, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mMinTorsionalPatchRadius, gPhysXUI->mEditBox_MinTorsionalPatchRadius, 0.0f, FLT_MAX);
#endif
	Common_GetFromEditBox(gParams.mContactOffset, gPhysXUI->mEditBox_ContactOffset, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mRestOffset, gPhysXUI->mEditBox_RestOffset, -FLT_MAX, FLT_MAX);
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
	Common_GetFromEditBox(gParams.mContactNotifThreshold, gPhysXUI->mEditBox_ContactNotifThreshold, 0.0f, FLT_MAX);
#endif
#if PHYSX_SUPPORT_SUBSTEPS
	Common_GetFromEditBox(gParams.mNbSubsteps, gPhysXUI->mEditBox_NbSubsteps);
#endif
	Common_GetFromEditBox(gParams.mSleepThreshold, gPhysXUI->mEditBox_SleepThreshold, 0.0f, FLT_MAX);
#if PHYSX_SUPPORT_STABILIZATION_FLAG
	Common_GetFromEditBox(gParams.mStabilizationThreshold, gPhysXUI->mEditBox_StabilizationThreshold, 0.0f, FLT_MAX);
#endif
#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
	Common_GetFromEditBox(gParams.mMBPSubdivLevel, gPhysXUI->mEditBox_MBPSubdivLevel);
	Common_GetFromEditBox(gParams.mMBPRange, gPhysXUI->mEditBox_MBPRange, 0.0f, FLT_MAX);
#endif
	Common_GetFromEditBox(gParams.mSQDynamicRebuildRateHint, gPhysXUI->mEditBox_SQ_RebuildRateHint);
#if PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT
	Common_GetFromEditBox(gParams.mGaussMapLimit, gPhysXUI->mEditBox_GaussMapLimit);
#endif

#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
	if(gPhysXUI->mCheckBox_ImprovedPatchFriction)
		gParams.mImprovedPatchFriction = gPhysXUI->mCheckBox_ImprovedPatchFriction->IsChecked();
#endif
#if PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
	if(gPhysXUI->mCheckBox_FrictionEveryIteration)
		gParams.mFrictionEveryIteration = gPhysXUI->mCheckBox_FrictionEveryIteration->IsChecked();
#endif
#if PHYSX_SUPPORT_TGS
	if(gPhysXUI->mCheckBox_TGS)
		gParams.mTGS = gPhysXUI->mCheckBox_TGS->IsChecked();
#endif
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
	if(gPhysXUI->mCheckBox_Gyro)
		gParams.mGyro = gPhysXUI->mCheckBox_Gyro->IsChecked();
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
	if(gPhysXUI->mCheckBox_Stabilization)
		gParams.mStabilization = gPhysXUI->mCheckBox_Stabilization->IsChecked();
#endif
	if(gPhysXUI->mCheckBox_Sleeping)
		gParams.mEnableSleeping = gPhysXUI->mCheckBox_Sleeping->IsChecked();
	if(gPhysXUI->mCheckBox_CCD)
		gParams.mEnableCCD = gPhysXUI->mCheckBox_CCD->IsChecked();
#if PHYSX_SUPPORT_ANGULAR_CCD
	if(gPhysXUI->mCheckBox_AngularCCD)
		gParams.mEnableAngularCCD = gPhysXUI->mCheckBox_AngularCCD->IsChecked();
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
	if(gPhysXUI->mCheckBox_RaycastCCDStatic)
		gParams.mEnableRaycastCCDStatic = gPhysXUI->mCheckBox_RaycastCCDStatic->IsChecked();
	if(gPhysXUI->mCheckBox_RaycastCCDDynamic)
		gParams.mEnableRaycastCCDDynamic = gPhysXUI->mCheckBox_RaycastCCDDynamic->IsChecked();
#endif

#if PHYSX_SUPPORT_STEER_FILTER
	Common_GetFromEditBox(gParams.mSteerFilter, gPhysXUI->mEditBox_SteerFilter, 0.0f, FLT_MAX);
#endif
#if PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
	Common_GetFromEditBox(gParams.mSweepInflation, gPhysXUI->mEditBox_SweepInflation, 0.0f, FLT_MAX);
#endif

#if PHYSX_SUPPORT_FLUIDS
	Common_GetFromEditBox(gParams.mFluidFriction, gPhysXUI->mEditBox_FluidFriction, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidDamping, gPhysXUI->mEditBox_FluidDamping, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidAdhesion, gPhysXUI->mEditBox_FluidAdhesion, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidViscosity, gPhysXUI->mEditBox_FluidViscosity, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidVorticityConfinement, gPhysXUI->mEditBox_FluidVorticityConfinement, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidSurfaceTension, gPhysXUI->mEditBox_FluidSurfaceTension, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidCohesion, gPhysXUI->mEditBox_FluidCohesion, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidLift, gPhysXUI->mEditBox_FluidLift, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidDrag, gPhysXUI->mEditBox_FluidDrag, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidCflCoefficient, gPhysXUI->mEditBox_FluidCflCoefficient, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidGravityScale, gPhysXUI->mEditBox_FluidGravityScale, -FLT_MAX, FLT_MAX);
	Common_GetFromEditBox(gParams.mFluidParticleMass, gPhysXUI->mEditBox_FluidParticleMass, -FLT_MAX, FLT_MAX);

	if(gPhysXUI->mCheckBox_FluidParticleCCD)
		gParams.mFluidParticleCCD = gPhysXUI->mCheckBox_FluidParticleCCD->IsChecked();
#endif

/*	if(gPhysXUI->mEditBox_GlobalBoxSize)
	{
		float tmp;
		bool status = gPhysXUI->mEditBox_GlobalBoxSize->GetTextAsFloat(tmp);
		ASSERT(status);
		ASSERT(tmp>=0.0f);
		gGlobalBoxSize = tmp;
	}*/

//	Common_GetFromEditBox(gParams.mDebugVizScale, gPhysXUI->mEditBox_DebugVizScale, 0.0f, FLT_MAX);
//	for(udword i=0;i<gNbDebugVizParams;i++)
//		gDebugVizParams[i] = gPhysXUI->mCheckBox_DebugVis[i]->IsChecked();
	GetDebugVizOptionsFromGUI();

	gParams.mForceJointLimitDebugViz = false;
	if(test_name)
		GetPintPlugin()->ApplyTestUIParams(test_name);
	if(gParams.mForceJointLimitDebugViz)
	{
		gParams.mDebugVizScale = 0.5f;
		gDebugVizParams[0] = true;
		for(udword i=0;i<gNbDebugVizParams;i++)
		{
			if(	gDebugVizIndex[i]==PxVisualizationParameter::eJOINT_LIMITS
			||	gDebugVizIndex[i]==PxVisualizationParameter::eJOINT_LOCAL_FRAMES
				)
			{
				gDebugVizParams[i] = true;
			}
		}
	}
	else
	{
		if(0)//###DEMO	// Hardcoded for demo script
		{
			if(strcmp(test_name, "ConvexStack")==0)
			{
				gParams.mDebugVizScale = 1.0f;
				gDebugVizParams[0] = true;
				for(udword i=0;i<gNbDebugVizParams;i++)
				{
					if(gDebugVizIndex[i]==PxVisualizationParameter::eBODY_MASS_AXES)
						gDebugVizParams[i] = true;
				}
			}
			else if(strcmp(test_name, "StableSphericalChain")==0
				||	strcmp(test_name, "CylinderStack")==0
				||	strcmp(test_name, "CatenaryBridge")==0)
			{
				gParams.mDebugVizScale = 1.0f;
				gDebugVizParams[0] = true;
				for(udword i=0;i<gNbDebugVizParams;i++)
				{
					if(gDebugVizIndex[i]==PxVisualizationParameter::eCOLLISION_DYNAMIC)
						gDebugVizParams[i] = true;
				}
			}
			else if(strcmp(test_name, "CCDTest_DynamicBoxesVsStaticContainer2")==0)
			{
				gParams.mDebugVizScale = 1.0f;
				gDebugVizParams[0] = true;
				for(udword i=0;i<gNbDebugVizParams;i++)
				{
					if(gDebugVizIndex[i]==PxVisualizationParameter::eBODY_LIN_VELOCITY)
						gDebugVizParams[i] = true;
				}
			}
			else if(strcmp(test_name, "ArticulatedVehicle")==0)
			{
				gParams.mDebugVizScale = 0.75f;
				gDebugVizParams[0] = true;
				for(udword i=0;i<gNbDebugVizParams;i++)
				{
					if(gDebugVizIndex[i]==PxVisualizationParameter::eJOINT_LOCAL_FRAMES || gDebugVizIndex[i]==PxVisualizationParameter::eJOINT_LIMITS)
						gDebugVizParams[i] = true;
				}
			}
			else if(strcmp(test_name, "ArticulatedChain_MCArticulation")==0)
			{
				gParams.mDebugVizScale = 0.5f;
				gDebugVizParams[0] = true;
				for(udword i=0;i<gNbDebugVizParams;i++)
				{
					if(gDebugVizIndex[i]==PxVisualizationParameter::eJOINT_LOCAL_FRAMES)
						gDebugVizParams[i] = true;
				}
			}
			else if(strcmp(test_name, "KinematicCharacter")==0)
			{
				gParams.mDebugVizScale = 1.0f;
				gDebugVizParams[0] = true;
				for(udword i=0;i<gNbDebugVizParams;i++)
				{
					if(gDebugVizIndex[i]==PxVisualizationParameter::eCOLLISION_COMPOUNDS)
						gDebugVizParams[i] = true;
				}
			}
			else if(strcmp(test_name, "ContactAndRestOffsets")==0)
			{
				gParams.mDebugVizScale = 1.0f;
				gDebugVizParams[0] = true;
				for(udword i=0;i<gNbDebugVizParams;i++)
				{
					if(gDebugVizIndex[i]==PxVisualizationParameter::eCONTACT_NORMAL || gDebugVizIndex[i]==PxVisualizationParameter::eCONTACT_POINT)
						gDebugVizParams[i] = true;
				}
			}
		}
	}
}

#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
	// ### would be easier to use a callback here
	class BPComboBox : public IceComboBox
	{
		public:
								BPComboBox(const ComboBoxDesc& desc) : IceComboBox(desc)	{}
		virtual	void			OnComboBoxEvent(ComboBoxEvent event)
		{
			if(event==CBE_SELECTION_CHANGED)
			{
				const udword Index = GetSelectedIndex();
				const bool UsesMBP = PxBroadPhaseType::Enum(Index)==PxBroadPhaseType::eMBP;

				if(gPhysXUI->mEditBox_MBPSubdivLevel)
					gPhysXUI->mEditBox_MBPSubdivLevel->SetEnabled(UsesMBP);

				if(gPhysXUI->mEditBox_MBPRange)
					gPhysXUI->mEditBox_MBPRange->SetEnabled(UsesMBP);
			}
		}
	};
#endif

static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
{
	const udword NB_DEBUG_VIZ_PARAMS = gPhysXUI->mNbDebugVizParams;
	bool* gDebugVizParams = gPhysXUI->mDebugVizParams;
	const char** gDebugVizNames = gPhysXUI->mDebugVizNames;

	const udword id = check_box.GetID();
	switch(id)
	{
#if PHYSX_SUPPORT_ARTICULATIONS || PHYSX_SUPPORT_RCA
		case PHYSX_GUI_DISABLE_ARTICULATIONS:
			gParams.mDisableArticulations = checked;
			break;
#endif
		case PHYSX_GUI_USE_D6_JOINT:
			gParams.mUseD6Joint = checked;
			break;
#if PHYSX_SUPPORT_DISABLE_PREPROCESSING
		case PHYSX_GUI_DISABLE_PREPROCESSING:
			gParams.mDisablePreprocessing = checked;
			break;
#endif
#ifndef IS_PHYSX_3_2
	#if PHYSX_REMOVE_JOINT_32_COMPATIBILITY
	#else
		case PHYSX_GUI_ENABLE_JOINT_32_COMPATIBILITY:
			gParams.mEnableJoint32Compatibility = checked;
			break;
	#endif
#endif
		case PHYSX_GUI_ENABLE_JOINT_PROJECTION:
			gParams.mEnableJointProjection = checked;
			if(gPhysXUI->mEditBox_ProjectionLinearTolerance)
				gPhysXUI->mEditBox_ProjectionLinearTolerance->SetEnabled(checked);
			if(gPhysXUI->mEditBox_ProjectionAngularTolerance)
				gPhysXUI->mEditBox_ProjectionAngularTolerance->SetEnabled(checked);
			break;

/*		case PHYSX_GUI_ENABLE_JOINT_CONTACT_DISTANCE:
			gParams.mEnableJointContactDistance = checked;
			break;*/

		case PHYSX_GUI_TEST:
			gTest = checked;
			break;

		case PHYSX_GUI_ENABLE_SLEEPING:
			gParams.mEnableSleeping = checked;
			break;
		case PHYSX_GUI_ENABLE_SQ:
			gParams.mSQFlag = checked;
			if(gPhysXUI->mCheckBox_SQ_FilterOutAllShapes)
				gPhysXUI->mCheckBox_SQ_FilterOutAllShapes->SetEnabled(checked);
			if(gPhysXUI->mCheckBox_SQ_InitialOverlap)
				gPhysXUI->mCheckBox_SQ_InitialOverlap->SetEnabled(checked);
//			if(gPhysXUI->mCheckBox_SQ_ManualFlushUpdates)
//				gPhysXUI->mCheckBox_SQ_ManualFlushUpdates->SetEnabled(checked);
			if(gPhysXUI->mCheckBox_SQ_PreciseSweeps)
				gPhysXUI->mCheckBox_SQ_PreciseSweeps->SetEnabled(checked);
			if(gPhysXUI->mCheckBox_SQ_BothSides)
				gPhysXUI->mCheckBox_SQ_BothSides->SetEnabled(checked);
			if(gPhysXUI->mEditBox_SQ_RebuildRateHint)
				gPhysXUI->mEditBox_SQ_RebuildRateHint->SetEnabled(checked);
			if(gPhysXUI->mComboBox_StaticPruner)
				gPhysXUI->mComboBox_StaticPruner->SetEnabled(checked);
			if(gPhysXUI->mComboBox_DynamicPruner)
				gPhysXUI->mComboBox_DynamicPruner->SetEnabled(checked);
			break;
		case PHYSX_GUI_ENABLE_CCD:
			gParams.mEnableCCD = checked;
			break;
#if PHYSX_SUPPORT_ANGULAR_CCD
		case PHYSX_GUI_ENABLE_ANGULAR_CCD:
			gParams.mEnableAngularCCD = checked;
			break;
#endif
#if PHYSX_SUPPORT_RAYCAST_CCD
		case PHYSX_GUI_ENABLE_RAYCAST_CCD:
			gParams.mEnableRaycastCCDStatic = checked;
			break;
		case PHYSX_GUI_ENABLE_RAYCAST_CCD_DYNA_DYNA:
			gParams.mEnableRaycastCCDDynamic = checked;
			break;
#endif
		case PHYSX_GUI_SQ_FILTER_OUT:
			gParams.mSQFilterOutAllShapes = checked;
			break;
		case PHYSX_GUI_SQ_INITIAL_OVERLAP:
			gParams.mSQInitialOverlap = checked;
			break;
/*		case PHYSX_GUI_SQ_MANUAL_FLUSH_UPDATES:
			gParams.mSQManualFlushUpdates = checked;
			break;*/
		case PHYSX_GUI_SQ_PRECISE_SWEEPS:
			gParams.mSQPreciseSweeps = checked;
			break;
		case PHYSX_GUI_SQ_BOTH_SIDES:
			gParams.mSQBothSides = checked;
			break;
		case PHYSX_GUI_SHARE_MESH_DATA:
			gParams.mShareMeshData = checked;
			break;
		case PHYSX_GUI_SHARE_SHAPES:
			gParams.mShareShapes = checked;
			break;
#if PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS
		case PHYSX_GUI_TIGHT_CONVEX_BOUNDS:
			gParams.mUseTightConvexBounds = checked;
			break;
#endif
		case PHYSX_GUI_PCM:
			gParams.mPCM = checked;
			break;
#if PHYSX_SUPPORT_ADAPTIVE_FORCE
		case PHYSX_GUI_ADAPTIVE_FORCE:
			gParams.mAdaptiveForce = checked;
			break;
#endif
#if PHYSX_SUPPORT_STABILIZATION_FLAG
		case PHYSX_GUI_STABILIZATION:
			gParams.mStabilization = checked;
			break;
#endif
#if PHYSX_SUPPORT_TGS
		case PHYSX_GUI_TGS:
			gParams.mTGS = checked;
			break;
#endif
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
		case PHYSX_GUI_GYRO:
			gParams.mGyro = checked;
			break;
#endif
#ifdef PHYSX_SUPPORT_SSE_FLAG
		case PHYSX_GUI_ENABLE_SSE:
			gParams.mEnableSSE = checked;
			break;
#endif
		case PHYSX_GUI_ENABLE_ACTIVE_TRANSFORMS:
			gParams.mEnableActiveTransforms = checked;
			break;
		case PHYSX_GUI_ENABLE_CONTACT_CACHE:
			gParams.mEnableContactCache = checked;
			break;
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
		case PHYSX_GUI_ENABLE_CONTACT_NOTIF:
			gParams.mEnableContactNotif = checked;
			break;
#endif
		case PHYSX_GUI_FLUSH_SIMULATION:
			gParams.mFlushSimulation = checked;
			break;
#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
		case PHYSX_GUI_IMPROVED_PATCH_FRICTION:
			gParams.mImprovedPatchFriction = checked;
			break;
#endif
#if PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
		case PHYSX_GUI_FRICTION_EVERY_ITERATION:
			gParams.mFrictionEveryIteration = checked;
			break;
#endif
		case PHYSX_GUI_DISABLE_STRONG_FRICTION:
			gParams.mDisableStrongFriction = checked;
			break;
		case PHYSX_GUI_ENABLE_ONE_DIR_FRICTION:
			gParams.mEnableOneDirFriction = checked;
			break;
		case PHYSX_GUI_ENABLE_TWO_DIR_FRICTION:
			gParams.mEnableTwoDirFriction = checked;
			break;
		case PHYSX_GUI_USE_PVD:
			gParams.mUsePVD = checked;
			if(gPhysXUI->mCheckBox_FullPVD)
				gPhysXUI->mCheckBox_FullPVD->SetEnabled(checked);
			break;
		case PHYSX_GUI_USE_FULL_PVD_CONNECTION:
			gParams.mUseFullPvdConnection = checked;
			break;
#ifdef PHYSX_SUPPORT_GPU
		case PHYSX_GUI_USE_GPU:
			gParams.mUseGPU = checked;
			break;
#endif
//		case PHYSX_GUI_DRAW_MBP_REGIONS:
//			gVisualizeMBPRegions = checked;
//			break;
#if PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE
		case PHYSX_GUI_PRECOMPUTE_ACTIVE_EDGES:
			gParams.mPrecomputeActiveEdges = checked;
			break;
#endif
#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
		case PHYSX_GUI_QUANTIZED_TREES:
			gParams.mQuantizedTrees = checked;
			break;
#endif

#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
		case PHYSX_GUI_USE_SUSPENSION_SWEEPS:
			gParams.mUseSuspensionSweeps = checked;
			if(gPhysXUI->mCheckBox_ForceSphereWheels)
				gPhysXUI->mCheckBox_ForceSphereWheels->SetEnabled(checked);		
			if(gPhysXUI->mCheckBox_UseBlockingSweeps)
				gPhysXUI->mCheckBox_UseBlockingSweeps->SetEnabled(checked);		
			if(gPhysXUI->mCheckBox_DrawSweptWheels)
				gPhysXUI->mCheckBox_DrawSweptWheels->SetEnabled(checked);		
			break;

		case PHYSX_GUI_FORCE_SPHERE_WHEELS:
			gParams.mForceSphereWheels = checked;
			break;

		case PHYSX_GUI_USE_BLOCKING_SWEEPS:
			gParams.mUseBlockingSweeps = checked;
			break;

		case PHYSX_GUI_DRAW_SWEPT_WHEELS:
			gParams.mDrawSweptWheels = checked;
			break;
#endif
		case PHYSX_GUI_DRAW_SUSPENSION_CASTS:
			gParams.mDrawSuspensionCasts = checked;
			break;

		case PHYSX_GUI_DRAW_PHYSX_TELEMETRY:
			gParams.mDrawPhysXTelemetry = checked;
			break;
			
#ifdef PHYSX_SUPPORT_VEHICLE_FIX
		case PHYSX_GUI_USE_FIX:
			gParams.mUseFix = checked;
			break;
#endif

#if PHYSX_SUPPORT_FLUIDS
		case PHYSX_GUI_FLUID_PARTICLE_CCD:
			gParams.mFluidParticleCCD = checked;
			break;
#endif

		case PHYSX_GUI_ENABLE_DEBUG_VIZ:
			{
				gDebugVizParams[0] = checked;
				for(udword i=1;i<NB_DEBUG_VIZ_PARAMS;i++)
				{
					gPhysXUI->mCheckBox_DebugVis[i]->SetEnabled(checked);
				}
			}
			break;
	}

	if(id>PHYSX_GUI_ENABLE_DEBUG_VIZ && id<PHYSX_GUI_ENABLE_DEBUG_VIZ+NB_DEBUG_VIZ_PARAMS)
	{
		gDebugVizParams[id-PHYSX_GUI_ENABLE_DEBUG_VIZ] = checked;
	}

	gPhysXUI->mCallback.UIModificationCallback();
}

static udword GetNumberOfLogicalThreads()
{
#if (_WIN32_WINNT >= 0x0601)
	udword groups = GetActiveProcessorGroupCount();
	udword totalProcessors = 0;
	for(uword i=0; i<groups; i++) 
		totalProcessors += udword(GetActiveProcessorCount(i));
	return totalProcessors;
#else
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#endif
}

static IceEditBox* CreateEditBox(PintGUIHelper& helper, IceWindow* parent, sdword x, sdword& y, const char* label, const char* value, EditBoxFilter filter, sdword label_width, sdword edit_box_x)
{
	const sdword YStep = 20;
	const sdword LabelOffsetY = 2;
	const sdword EditBoxWidth = 60;

	helper.CreateLabel(parent, x, y+LabelOffsetY, label_width, 20, label, gPhysXUI->mPhysXGUI);
	IceEditBox* EB = helper.CreateEditBox(parent, INVALID_ID, x+edit_box_x, y, EditBoxWidth, 20, value, gPhysXUI->mPhysXGUI, filter, null);
	y += YStep;

	return EB;
}

IceWindow* PhysX3::InitSharedGUI(IceWidget* parent, PintGUIHelper& helper, UICallback& callback)
{
	// UICallback should be called when a UI modification appears, to take the change into account immediately (without restarting the scene).
	// This is currently used e.g. for checkboxes controlling the debug viz params. This can also be used for edit boxes with this class:
	struct Callbacks
	{
		static void EditBoxChange(const IceEditBox& edit_box, udword param, void* user_data)
		{
			UICallback* cb = reinterpret_cast<UICallback*>(user_data);
			cb->UIModificationCallback();
		}
	};

	const udword nb_debug_viz_params = gNbDebugVizParams;
	bool* debug_viz_params = gDebugVizParams;
	const char** debug_viz_names = gDebugVizNames;

	ASSERT(!gPhysXUI);
	gPhysXUI = ICE_NEW(PhysXUI)(callback, nb_debug_viz_params, debug_viz_params, debug_viz_names);

	IceWindow* Main_ = helper.CreateMainWindow(gPhysXUI->mPhysXGUI, parent, PHYSX_GUI_MAIN, "PhysX3 options");

	const sdword YStep = 20;
	const sdword YStepCB = 18;
	const sdword YStart = 4;
	sdword y = YStart;

	const sdword OffsetX = 90;
	const sdword LabelOffsetY = 2;

	const sdword EditBoxWidth = 60;
	const udword CheckBoxWidth = 190;

	// Tab control
	enum TabIndex
	{
		TAB_MAIN,
		TAB_DYNAMICS,
		TAB_JOINTS,
		TAB_SCENE_QUERIES,
		TAB_COOKING,
		TAB_VEHICLES,
#if PHYSX_SUPPORT_FLUIDS
		TAB_FLUIDS,
#endif
		TAB_DEBUG_VIZ,
		TAB_COUNT,
	};
	IceWindow* Tabs[TAB_COUNT];
	{
		TabControlDesc TCD;
		TCD.mParent	= Main_;
		TCD.mX		= 0;
		TCD.mY		= 0;
//		TCD.mWidth	= MainWidth - WD.mX - BorderSize;
//		TCD.mHeight	= MainHeight - BorderSize*2;
		TCD.mWidth	= 500;
		TCD.mHeight	= 410;
		IceTabControl* TabControl = ICE_NEW(IceTabControl)(TCD);
		gPhysXUI->mPhysXGUI->Register(TabControl);

		for(udword i=0;i<TAB_COUNT;i++)
		{
			WindowDesc WD;
			WD.mParent	= Main_;
			WD.mX		= 0;
			WD.mY		= 0;
			WD.mWidth	= 500;
			WD.mHeight	= 410;
			WD.mLabel	= "Tab";
			WD.mType	= WINDOW_DIALOG;
			IceWindow* Tab = ICE_NEW(IceWindow)(WD);
			gPhysXUI->mPhysXGUI->Register(Tab);
			Tab->SetVisible(true);
			Tabs[i] = Tab;
		}
		TabControl->Add(Tabs[TAB_MAIN], "Main");
		TabControl->Add(Tabs[TAB_DYNAMICS], "Dynamics");
		TabControl->Add(Tabs[TAB_JOINTS], "Joints");
		TabControl->Add(Tabs[TAB_SCENE_QUERIES], "Scene queries");
		TabControl->Add(Tabs[TAB_COOKING], "Cooking");
		TabControl->Add(Tabs[TAB_VEHICLES], "Vehicles");
#if PHYSX_SUPPORT_FLUIDS
		TabControl->Add(Tabs[TAB_FLUIDS], "Fluids");
#endif
		TabControl->Add(Tabs[TAB_DEBUG_VIZ], "Debug vis.");

		// TAB_MAIN
		{
			IceWindow* TabWindow = Tabs[TAB_MAIN];
			sdword y = YStart;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 90, 20, "Num threads:", gPhysXUI->mPhysXGUI);

			gPhysXUI->mComboBox_NbThreads = CreateComboBox<IceComboBox>(TabWindow, PHYSX_GUI_NB_THREADS, 4+OffsetX, y, 150, 20, "Num threads", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_NbThreads->Add("Single threaded");
			gPhysXUI->mComboBox_NbThreads->Add("1 main + 1 worker thread");
			gPhysXUI->mComboBox_NbThreads->Add("1 main + 2 worker threads");
			gPhysXUI->mComboBox_NbThreads->Add("1 main + 3 worker threads");
			gPhysXUI->mComboBox_NbThreads->Add("1 main + 4 worker threads");
			gPhysXUI->mComboBox_NbThreads->Add("1 main + 8 worker threads");
			gPhysXUI->mComboBox_NbThreads->Add("1 main + 12 worker threads");
			gPhysXUI->mComboBox_NbThreads->Add("1 main + 15 worker threads");
			gPhysXUI->mComboBox_NbThreads->Select(0);
			y += YStep;

#if PHYSX_SUPPORT_SCRATCH_BUFFER
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 90, 20, "Scratch buffer:", gPhysXUI->mPhysXGUI);
			gPhysXUI->mComboBox_ScratchSize = CreateComboBox<IceComboBox>(TabWindow, PHYSX_GUI_SCRATCH_BUFFER, 4+OffsetX, y, 150, 20, "Scratch buffer", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_ScratchSize->Add("Disabled");
			gPhysXUI->mComboBox_ScratchSize->Add("32 Kb");
			gPhysXUI->mComboBox_ScratchSize->Add("128 Kb");
			gPhysXUI->mComboBox_ScratchSize->Add("256 Kb");
			gPhysXUI->mComboBox_ScratchSize->Add("512 Kb");
			gPhysXUI->mComboBox_ScratchSize->Add("1024 Kb");
			gPhysXUI->mComboBox_ScratchSize->Add("2048 Kb");
			gPhysXUI->mComboBox_ScratchSize->Select(gParams.mScratchSize);
			y += YStep;
#endif

#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 90, 20, "Broad phase:", gPhysXUI->mPhysXGUI);
			gPhysXUI->mComboBox_BroadPhase = CreateComboBox<BPComboBox>(TabWindow, PHYSX_GUI_BROAD_PHASE, 4+OffsetX, y, 150, 20, "Broad phase", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_BroadPhase->Add("eSAP");
			gPhysXUI->mComboBox_BroadPhase->Add("eMBP");
	#if PHYSX_SUPPORT_PX_BROADPHASE_ABP
			gPhysXUI->mComboBox_BroadPhase->Add("eABP");
	#endif
	#if PHYSX_SUPPORT_PX_BROADPHASE_PABP
			gPhysXUI->mComboBox_BroadPhase->Add("ePABP");
	#endif
			gPhysXUI->mComboBox_BroadPhase->Select(gParams.mBroadPhaseType);
			y += YStep;
#endif
			y += YStep;

			{
				gPhysXUI->mCheckBox_CCD = helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_CCD, 4, y, CheckBoxWidth, 20, "Enable CCD", gPhysXUI->mPhysXGUI, gParams.mEnableCCD, gCheckBoxCallback);
				y += YStepCB;

#if PHYSX_SUPPORT_ANGULAR_CCD
				gPhysXUI->mCheckBox_AngularCCD = helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_ANGULAR_CCD, 4, y, CheckBoxWidth, 20, "Enable angular CCD", gPhysXUI->mPhysXGUI, gParams.mEnableAngularCCD, gCheckBoxCallback);
				y += YStepCB;
#endif

#if PHYSX_SUPPORT_RAYCAST_CCD
				gPhysXUI->mCheckBox_RaycastCCDStatic = helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_RAYCAST_CCD, 4, y, CheckBoxWidth, 20, "Enable raycast CCD", gPhysXUI->mPhysXGUI, gParams.mEnableRaycastCCDStatic, gCheckBoxCallback);
				y += YStepCB;

				gPhysXUI->mCheckBox_RaycastCCDDynamic = helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_RAYCAST_CCD_DYNA_DYNA, 4, y, CheckBoxWidth, 20, "Enable raycast CCD for dynamics", gPhysXUI->mPhysXGUI, gParams.mEnableRaycastCCDDynamic, gCheckBoxCallback);
				y += YStepCB;
#endif
			}
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_SHARE_MESH_DATA, 4, y, CheckBoxWidth, 20, "Share mesh data", gPhysXUI->mPhysXGUI, gParams.mShareMeshData, gCheckBoxCallback);
			y += YStepCB;

			helper.CreateCheckBox(TabWindow, PHYSX_GUI_SHARE_SHAPES, 4, y, CheckBoxWidth, 20, "Share shapes", gPhysXUI->mPhysXGUI, gParams.mShareShapes, gCheckBoxCallback);
			y += YStepCB;

#if PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_TIGHT_CONVEX_BOUNDS, 4, y, CheckBoxWidth, 20, "Tight convex bounds", gPhysXUI->mPhysXGUI, gParams.mUseTightConvexBounds, gCheckBoxCallback);
			y += YStepCB;
#endif
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_PCM, 4, y, CheckBoxWidth, 20, "Enable PCM", gPhysXUI->mPhysXGUI, gParams.mPCM, gCheckBoxCallback);
			y += YStepCB;

#ifdef PHYSX_SUPPORT_SSE_FLAG
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_SSE, 4, y, CheckBoxWidth, 20, "Enable SSE", gPhysXUI->mPhysXGUI, gParams.mEnableSSE, gCheckBoxCallback);
			y += YStepCB;
#endif
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_ACTIVE_TRANSFORMS, 4, y, CheckBoxWidth, 20, "Enable active transforms", gPhysXUI->mPhysXGUI, gParams.mEnableActiveTransforms, gCheckBoxCallback);
			y += YStepCB;

			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_CONTACT_CACHE, 4, y, CheckBoxWidth, 20, "Enable contact cache", gPhysXUI->mPhysXGUI, gParams.mEnableContactCache, gCheckBoxCallback);
			y += YStepCB;

#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_CONTACT_NOTIF, 4, y, CheckBoxWidth, 20, "Enable contact notifications", gPhysXUI->mPhysXGUI, gParams.mEnableContactNotif, gCheckBoxCallback);
			y += YStepCB;
#endif

			helper.CreateCheckBox(TabWindow, PHYSX_GUI_FLUSH_SIMULATION, 4, y, CheckBoxWidth, 20, "Flush simulation buffers", gPhysXUI->mPhysXGUI, gParams.mFlushSimulation, gCheckBoxCallback);
			y += YStepCB;

			gPhysXUI->mCheckBox_PVD = helper.CreateCheckBox(TabWindow, PHYSX_GUI_USE_PVD, 4, y, CheckBoxWidth, 20, "Use PVD", gPhysXUI->mPhysXGUI, gParams.mUsePVD, gCheckBoxCallback);
			y += YStepCB;

			gPhysXUI->mCheckBox_FullPVD = helper.CreateCheckBox(TabWindow, PHYSX_GUI_USE_FULL_PVD_CONNECTION, 4, y, CheckBoxWidth, 20, "Full connection", gPhysXUI->mPhysXGUI, gParams.mUseFullPvdConnection, gCheckBoxCallback);
#ifdef PINT_SUPPORT_PVD	// Defined in project's properties
//			gPhysXUI->mCheckBox_FullPVD->SetEnabled(gUsePVD);
#else
			gPhysXUI->mCheckBox_PVD->SetEnabled(false);
			gPhysXUI->mCheckBox_FullPVD->SetEnabled(false);
#endif
			y += YStepCB;

#ifdef PHYSX_SUPPORT_GPU
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_USE_GPU, 4, y, CheckBoxWidth, 20, "Use GPU", gPhysXUI->mPhysXGUI, gParams.mUseGPU, gCheckBoxCallback);
			y += YStepCB;
#endif
			y += YStepCB;

			y = 100;
			const udword x2 = 200;
//			const udword x2 = 4;


			const sdword EditBoxX = 130;
			const sdword LabelWidth = 130;

//		helper.CreateLabel(TabWindow, x2, y+LabelOffsetY, LabelWidth, 20, "World bounds size:", gPhysXUI->mPhysXGUI);
//		gPhysXUI->mEditBox_GlobalBoxSize = helper.CreateEditBox(TabWindow, PHYSX_GUI_GLOBAL_BOX_SIZE, 4+EditBoxX, y, EditBoxWidth, 20, helper.Convert(gGlobalBoxSize), gPhysXUI->mPhysXGUI, EDITBOX_FLOAT_POSITIVE, null);
//		y += YStep;

			gPhysXUI->mEditBox_ContactOffset = CreateEditBox(helper, TabWindow, x2, y, "Contact offset:", helper.Convert(gParams.mContactOffset), EDITBOX_FLOAT, LabelWidth, EditBoxX);
			gPhysXUI->mEditBox_RestOffset = CreateEditBox(helper, TabWindow, x2, y, "Rest offset:", helper.Convert(gParams.mRestOffset), EDITBOX_FLOAT, LabelWidth, EditBoxX);
#if PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
			gPhysXUI->mEditBox_ContactNotifThreshold = CreateEditBox(helper, TabWindow, x2, y, "Contact notif threshold:", helper.Convert(gParams.mContactNotifThreshold), EDITBOX_FLOAT, LabelWidth, EditBoxX);
#endif
#if PHYSX_SUPPORT_SUBSTEPS
			gPhysXUI->mEditBox_NbSubsteps = CreateEditBox(helper, TabWindow, x2, y, "Nb substeps:", _F("%d", gParams.mNbSubsteps), EDITBOX_INTEGER_POSITIVE, LabelWidth, EditBoxX);
#endif
#if PHYSX_SUPPORT_PX_BROADPHASE_TYPE
			gPhysXUI->mEditBox_MBPSubdivLevel = CreateEditBox(helper, TabWindow, x2, y, "MBP subdiv level:", _F("%d", gParams.mMBPSubdivLevel), EDITBOX_INTEGER_POSITIVE, LabelWidth, EditBoxX);
			gPhysXUI->mEditBox_MBPRange = CreateEditBox(helper, TabWindow, x2, y, "MBP range:", _F("%f", gParams.mMBPRange), EDITBOX_FLOAT_POSITIVE, LabelWidth, EditBoxX);

			const bool UsesMBP = gParams.mBroadPhaseType==PxBroadPhaseType::eMBP;
			gPhysXUI->mEditBox_MBPSubdivLevel->SetEnabled(UsesMBP);
			gPhysXUI->mEditBox_MBPRange->SetEnabled(UsesMBP);
#endif

#if PHYSX_SUPPORT_SUBSTEPS
			const udword x3 = 200;
			y += YStep;

			static const char* ConfigDesc0 = "4 solver iterations, single-threaded,\nno substeps, sleeping disabled.";
			static const char* ConfigDesc1 = "4 solver iterations, multi-threaded,\nno substeps, sleeping enabled.";
			static const char* ConfigDesc2 = "32 solver iterations, multi-threaded,\n2 substeps, sleeping enabled.";

			class ConfigComboBox : public IceComboBox
			{
				public:
								ConfigComboBox(const ComboBoxDesc& desc) : IceComboBox(desc)	{}

						void	SetupMultithreaded()
								{
									const udword NbThreads = GetNumberOfLogicalThreads();
									const udword Index = gParams.NbThreadsToThreadIndex(NbThreads);
									gPhysXUI->mComboBox_NbThreads->Select(Index);
								}

				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
								{
									if(event==CBE_SELECTION_CHANGED)
									{
										const udword SelectedIndex = GetSelectedIndex();
										if(SelectedIndex==0)
										{
											gPhysXUI->mEditBox_NbSubsteps->SetText("1");
											gPhysXUI->mEditBox_SolverIterPos->SetText("4");
											gPhysXUI->mComboBox_NbThreads->Select(0);
											gParams.mEnableSleeping = false;
											gPhysXUI->mCheckBox_Sleeping->SetChecked(gParams.mEnableSleeping);
#if PHYSX_SUPPORT_TGS
//											gParams.mTGS = false;
//											gPhysXUI->mCheckBox_TGS->SetChecked(gParams.mTGS);
#endif
											gPhysXUI->mEditBox_ConfigDesc->SetMultilineText(ConfigDesc0);
										}
										else if(SelectedIndex==1)
										{
											gPhysXUI->mEditBox_NbSubsteps->SetText("1");
											gPhysXUI->mEditBox_SolverIterPos->SetText("4");
											//gPhysXUI->mComboBox_NbThreads->Select(4);
											SetupMultithreaded();
											gParams.mEnableSleeping = true;
											gPhysXUI->mCheckBox_Sleeping->SetChecked(gParams.mEnableSleeping);
#if PHYSX_SUPPORT_TGS
//											gParams.mTGS = false;
//											gPhysXUI->mCheckBox_TGS->SetChecked(gParams.mTGS);
#endif
											gPhysXUI->mEditBox_ConfigDesc->SetMultilineText(ConfigDesc1);
										}
										else if(SelectedIndex==2)
										{
											gPhysXUI->mEditBox_NbSubsteps->SetText("2");
											gPhysXUI->mEditBox_SolverIterPos->SetText("32");
											//gPhysXUI->mComboBox_NbThreads->Select(4);
											SetupMultithreaded();
											gParams.mEnableSleeping = true;
											gPhysXUI->mCheckBox_Sleeping->SetChecked(gParams.mEnableSleeping);
#if PHYSX_SUPPORT_TGS
//											gParams.mTGS = true;
//											gPhysXUI->mCheckBox_TGS->SetChecked(gParams.mTGS);
#endif
											gPhysXUI->mEditBox_ConfigDesc->SetMultilineText(ConfigDesc2);
										}
									}
								}
			};

			{
			gPhysXUI->mComboBox_Config = CreateComboBox<ConfigComboBox>(TabWindow, PHYSX_GUI_CONFIG_COMBO_BOX, x3, y, 250, 20, "Configuration", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_Config->Add("Configure for comparison tests (default)");
			gPhysXUI->mComboBox_Config->Add("Configure for better performance");
			gPhysXUI->mComboBox_Config->Add("Configure for better sim quality");
			gPhysXUI->mComboBox_Config->Select(0);
			y += YStep;
			}

			gPhysXUI->mEditBox_ConfigDesc = helper.CreateEditBox(TabWindow, PHYSX_GUI_CONFIG_DESC, x3, y, 250, 60, "", gPhysXUI->mPhysXGUI, EDITBOX_TEXT, null);
			gPhysXUI->mEditBox_ConfigDesc->SetMultilineText(ConfigDesc0);
			gPhysXUI->mEditBox_ConfigDesc->SetReadOnly(true);
#endif
		}

		// TAB_DYNAMICS
		{
			const sdword LabelWidth = 100+20;
			const sdword EditBoxX = LabelWidth + 10;

			IceWindow* TabWindow = Tabs[TAB_DYNAMICS];
			sdword y = YStart;

			{
				const sdword LabelWidth2 = 130;
				const sdword EditBoxX2 = LabelWidth2 + 10;
				sdword YSaved = y;
				const sdword xf = 200;
				const sdword xf2 = xf+8;

				y += YStepCB*2;

				helper.CreateCheckBox(TabWindow, PHYSX_GUI_DISABLE_STRONG_FRICTION, xf2, y, CheckBoxWidth, 20, "Disable strong friction", gPhysXUI->mPhysXGUI, gParams.mDisableStrongFriction, gCheckBoxCallback);
				y += YStepCB;

				helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_ONE_DIR_FRICTION, xf2, y, CheckBoxWidth, 20, "Enable one dir. friction", gPhysXUI->mPhysXGUI, gParams.mEnableOneDirFriction, gCheckBoxCallback);
				y += YStepCB;

				helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_TWO_DIR_FRICTION, xf2, y, CheckBoxWidth, 20, "Enable two dir. friction", gPhysXUI->mPhysXGUI, gParams.mEnableTwoDirFriction, gCheckBoxCallback);
				y += YStepCB;

#if PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
				gPhysXUI->mCheckBox_ImprovedPatchFriction = helper.CreateCheckBox(TabWindow, PHYSX_GUI_IMPROVED_PATCH_FRICTION, xf2, y, CheckBoxWidth, 20, "Improved patch friction", gPhysXUI->mPhysXGUI, gParams.mImprovedPatchFriction, gCheckBoxCallback);
				y += YStepCB;
#endif
#if PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
				gPhysXUI->mCheckBox_FrictionEveryIteration = helper.CreateCheckBox(TabWindow, PHYSX_GUI_FRICTION_EVERY_ITERATION, xf2, y, CheckBoxWidth, 20, "Friction every iteration", gPhysXUI->mPhysXGUI, gParams.mFrictionEveryIteration, gCheckBoxCallback);
				y += YStepCB;
#endif
				y += YStepCB;

				gPhysXUI->mEditBox_DefaultStaticFriction = CreateEditBox(helper, TabWindow, xf2, y, "Default static friction:", helper.Convert(gParams.mDefaultStaticFriction), EDITBOX_FLOAT_POSITIVE, LabelWidth2, EditBoxX2);
				gPhysXUI->mEditBox_DefaultDynamicFriction = CreateEditBox(helper, TabWindow, xf2, y, "Default dynamic friction:", helper.Convert(gParams.mDefaultDynamicFriction), EDITBOX_FLOAT_POSITIVE, LabelWidth2, EditBoxX2);
				gPhysXUI->mEditBox_FrictionOffsetThreshold = CreateEditBox(helper, TabWindow, xf2, y, "Friction offset threshold:", helper.Convert(gParams.mFrictionOffsetThreshold), EDITBOX_FLOAT_POSITIVE, LabelWidth2, EditBoxX2);

#if PHYSX_SUPPORT_TORSION_FRICTION
				gPhysXUI->mEditBox_TorsionalPatchRadius = CreateEditBox(helper, TabWindow, xf2, y, "Torsional patch radius:", helper.Convert(gParams.mTorsionalPatchRadius), EDITBOX_FLOAT_POSITIVE, LabelWidth2, EditBoxX2);
				gPhysXUI->mEditBox_MinTorsionalPatchRadius = CreateEditBox(helper, TabWindow, xf2, y, "Min torsional patch radius:", helper.Convert(gParams.mMinTorsionalPatchRadius), EDITBOX_FLOAT_POSITIVE, LabelWidth2, EditBoxX2);
#endif
				{
					// Cannot use helper function here because "mType" is missing from wrapper.
					EditBoxDesc EBD;
					EBD.mParent		= TabWindow;
					EBD.mX			= xf;
					EBD.mY			= YSaved;
					EBD.mWidth		= 250;
					EBD.mHeight		= 300;
					EBD.mLabel		= "============ Friction settings ============";
					EBD.mFilter		= EDITBOX_TEXT;
					EBD.mType		= EDITBOX_READ_ONLY;
					IceEditBox* EB = ICE_NEW(IceEditBox)(EBD);
					EB->SetVisible(true);
					gPhysXUI->mPhysXGUI->Register(EB);
				}
				y = YSaved;
			}

			gPhysXUI->mCheckBox_Sleeping = helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_SLEEPING, 4, y, CheckBoxWidth, 20, "Enable sleeping", gPhysXUI->mPhysXGUI, gParams.mEnableSleeping, gCheckBoxCallback);
			y += YStepCB;

#if PHYSX_SUPPORT_ADAPTIVE_FORCE
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ADAPTIVE_FORCE, 4, y, CheckBoxWidth, 20, "Adaptive force", gPhysXUI->mPhysXGUI, gParams.mAdaptiveForce, gCheckBoxCallback);
			y += YStepCB;
#endif

#if PHYSX_SUPPORT_STABILIZATION_FLAG
			gPhysXUI->mCheckBox_Stabilization = helper.CreateCheckBox(TabWindow, PHYSX_GUI_STABILIZATION, 4, y, CheckBoxWidth, 20, "Stabilization", gPhysXUI->mPhysXGUI, gParams.mStabilization, gCheckBoxCallback);
			y += YStepCB;
#endif
#if PHYSX_SUPPORT_TGS
			gPhysXUI->mCheckBox_TGS = helper.CreateCheckBox(TabWindow, PHYSX_GUI_TGS, 4, y, CheckBoxWidth, 20, "Enable TGS", gPhysXUI->mPhysXGUI, gParams.mTGS, gCheckBoxCallback);
			y += YStepCB;
#endif
#if PHYSX_SUPPORT_GYROSCOPIC_FORCES
			gPhysXUI->mCheckBox_Gyro = helper.CreateCheckBox(TabWindow, PHYSX_GUI_GYRO, 4, y, CheckBoxWidth, 20, "Enable gyroscopic forces", gPhysXUI->mPhysXGUI, gParams.mGyro, gCheckBoxCallback);
			y += YStepCB;
#endif
			y += YStep;

#ifndef IS_PHYSX_3_2
			gPhysXUI->mEditBox_MaxNbCCDPasses = CreateEditBox(helper, TabWindow, 4, y, "Max nb CCD passes:", _F("%d", gParams.mMaxNbCCDPasses), EDITBOX_INTEGER_POSITIVE, LabelWidth, EditBoxX);
#endif
			gPhysXUI->mEditBox_SolverIterPos = CreateEditBox(helper, TabWindow, 4, y, "Solver iter pos:", _F("%d", gParams.mSolverIterationCountPos), EDITBOX_INTEGER_POSITIVE, LabelWidth, EditBoxX);
			gPhysXUI->mEditBox_SolverIterVel = CreateEditBox(helper, TabWindow, 4, y, "Solver iter vel:", _F("%d", gParams.mSolverIterationCountVel), EDITBOX_INTEGER_POSITIVE, LabelWidth, EditBoxX);
			gPhysXUI->mEditBox_LinearDamping = CreateEditBox(helper, TabWindow, 4, y, "Linear damping:", helper.Convert(gParams.mLinearDamping), EDITBOX_FLOAT_POSITIVE, LabelWidth, EditBoxX);
			gPhysXUI->mEditBox_AngularDamping = CreateEditBox(helper, TabWindow, 4, y, "Angular damping:", helper.Convert(gParams.mAngularDamping), EDITBOX_FLOAT_POSITIVE, LabelWidth, EditBoxX);
			gPhysXUI->mEditBox_MaxAngularVelocity = CreateEditBox(helper, TabWindow, 4, y, "Max angular velocity:", helper.Convert(gParams.mMaxAngularVelocity), EDITBOX_FLOAT_POSITIVE, LabelWidth, EditBoxX);
#if PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
			gPhysXUI->mEditBox_MaxDepenVelocity = CreateEditBox(helper, TabWindow, 4, y, "Max depen. velocity:", helper.Convert(gParams.mMaxDepenVelocity), EDITBOX_FLOAT_POSITIVE, LabelWidth, EditBoxX);
#endif
			gPhysXUI->mEditBox_SleepThreshold = CreateEditBox(helper, TabWindow, 4, y, "Sleep threshold:", helper.Convert(gParams.mSleepThreshold), EDITBOX_FLOAT_POSITIVE, LabelWidth, EditBoxX);
#if PHYSX_SUPPORT_STABILIZATION_FLAG
			gPhysXUI->mEditBox_StabilizationThreshold = CreateEditBox(helper, TabWindow, 4, y, "Stabilization threshold:", helper.Convert(gParams.mStabilizationThreshold), EDITBOX_FLOAT_POSITIVE, LabelWidth, EditBoxX);
#endif
			gPhysXUI->mEditBox_MaxBiasCoeff = CreateEditBox(helper, TabWindow, 4, y, "Max bias coeff:", helper.Convert(gParams.mMaxBiasCoeff), EDITBOX_FLOAT, LabelWidth, EditBoxX);
		}

		// TAB_JOINTS
		{
			IceWindow* TabWindow = Tabs[TAB_JOINTS];
			sdword y = YStart;

			const sdword xj = 4;

			helper.CreateCheckBox(TabWindow, PHYSX_GUI_USE_D6_JOINT, xj, y, CheckBoxWidth, 20, "Use D6 joint if possible", gPhysXUI->mPhysXGUI, gParams.mUseD6Joint, gCheckBoxCallback);
			y += YStepCB;

#if PHYSX_SUPPORT_DISABLE_PREPROCESSING
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_DISABLE_PREPROCESSING, xj, y, CheckBoxWidth, 20, "Disable preprocessing", gPhysXUI->mPhysXGUI, gParams.mDisablePreprocessing, gCheckBoxCallback);
			y += YStepCB;
#endif
#ifndef IS_PHYSX_3_2
	#if PHYSX_REMOVE_JOINT_32_COMPATIBILITY
	#else
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_JOINT_32_COMPATIBILITY, xj, y, CheckBoxWidth, 20, "Enable 3.2 compatibility", gPhysXUI->mPhysXGUI, gParams.mEnableJoint32Compatibility, gCheckBoxCallback);
			y += YStepCB;
	#endif
#endif
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_JOINT_PROJECTION, xj, y, CheckBoxWidth, 20, "Enable joint projection", gPhysXUI->mPhysXGUI, gParams.mEnableJointProjection, gCheckBoxCallback);
//			y += YStepCB;

//			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_JOINT_CONTACT_DISTANCE, xj, y, CheckBoxWidth, 20, "Enable contact distance (for limits)", gPhysXUI->mPhysXGUI, gParams.mEnableJointContactDistance, gCheckBoxCallback);
//			y += YStepCB;

			y += YStep;

			const sdword LabelWidthJ = 150;
			gPhysXUI->mEditBox_ProjectionLinearTolerance = CreateEditBox(helper, TabWindow, xj, y, "Projection linear tolerance:", helper.Convert(gParams.mProjectionLinearTolerance), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
			gPhysXUI->mEditBox_ProjectionLinearTolerance->SetEnabled(gParams.mEnableJointProjection);

			gPhysXUI->mEditBox_ProjectionAngularTolerance = CreateEditBox(helper, TabWindow, xj, y, "Projection angular tolerance:", helper.Convert(gParams.mProjectionAngularTolerance), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
			gPhysXUI->mEditBox_ProjectionAngularTolerance->SetEnabled(gParams.mEnableJointProjection);

#ifndef IS_PHYSX_3_2
			gPhysXUI->mEditBox_InverseInertiaScale = CreateEditBox(helper, TabWindow, xj, y, "Inverse inertia scale:", helper.Convert(gParams.mInverseInertiaScale), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
			gPhysXUI->mEditBox_InverseMassScale = CreateEditBox(helper, TabWindow, xj, y, "Inverse mass scale:", helper.Convert(gParams.mInverseMassScale), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
#endif

#ifdef PHYSX_SUPPORT_LINEAR_COEFF
			gPhysXUI->mEditBox_LinearCoeff = CreateEditBox(helper, TabWindow, xj, y, "Linear coefficient:", helper.Convert(gParams.mLinearCoeff), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
#endif

			{
				helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 90, 20, "Contact distance:", gPhysXUI->mPhysXGUI);
				gPhysXUI->mComboBox_LimitsContactDistance = CreateComboBox<IceComboBox>(TabWindow, PHYSX_GUI_LIMITS_CONTACT_DISTANCE, 4+OffsetX, y, 150, 20, "Limits contact distance", gPhysXUI->mPhysXGUI, null);
				gPhysXUI->mComboBox_LimitsContactDistance->Add("Infinite");
				gPhysXUI->mComboBox_LimitsContactDistance->Add("Partial (PhysX default)");
				gPhysXUI->mComboBox_LimitsContactDistance->Add("Zero");
				gPhysXUI->mComboBox_LimitsContactDistance->Select(gParams.mLimitsContactDistance);
				y += YStep;
			}

			y += YStep;

			{
				sdword YSaved = y;

				y += 20;
				const sdword xa = xj + 10;

#if PHYSX_SUPPORT_ARTICULATIONS || PHYSX_SUPPORT_RCA
				helper.CreateCheckBox(TabWindow, PHYSX_GUI_DISABLE_ARTICULATIONS, xa, y, CheckBoxWidth, 20, "Disable articulations", gPhysXUI->mPhysXGUI, gParams.mDisableArticulations, gCheckBoxCallback);
				y += YStepCB;
				y += YStepCB;
#endif
#if PHYSX_SUPPORT_ARTICULATIONS
				gPhysXUI->mEditBox_MaxProjectionIterations = CreateEditBox(helper, TabWindow, xa, y, "Max Projection Iterations:", _F("%d", gParams.mMaxProjectionIterations), EDITBOX_INTEGER_POSITIVE, LabelWidthJ, LabelWidthJ);
				gPhysXUI->mEditBox_SeparationTolerance = CreateEditBox(helper, TabWindow, xa, y, "Separation tolerance:", helper.Convert(gParams.mSeparationTolerance), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
				gPhysXUI->mEditBox_InternalDriveIterations = CreateEditBox(helper, TabWindow, xa, y, "Internal drive iterations:", _F("%d", gParams.mInternalDriveIterations), EDITBOX_INTEGER_POSITIVE, LabelWidthJ, LabelWidthJ);
				gPhysXUI->mEditBox_ExternalDriveIterations = CreateEditBox(helper, TabWindow, xa, y, "External drive iterations:", _F("%d", gParams.mExternalDriveIterations), EDITBOX_INTEGER_POSITIVE, LabelWidthJ, LabelWidthJ);
#endif

#if PHYSX_SUPPORT_RCA_CFM_SCALE
				gPhysXUI->mEditBox_RCACfmScale = CreateEditBox(helper, TabWindow, xa, y, "RCA CFM scale:", helper.Convert(gParams.mRCACfmScale), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
#endif
#if PHYSX_SUPPORT_RCA_DOF_SCALE
				gPhysXUI->mEditBox_RCADofScale = CreateEditBox(helper, TabWindow, xa, y, "RCA DOF scale:", helper.Convert(gParams.mRCADofScale), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
#endif
#if PHYSX_SUPPORT_RCA_ARMATURE
				gPhysXUI->mEditBox_RCAArmature = CreateEditBox(helper, TabWindow, xa, y, "RCA armature:", helper.Convert(gParams.mRCAArmature), EDITBOX_FLOAT_POSITIVE, LabelWidthJ, LabelWidthJ);
#endif
				{
					// Cannot use helper function here because "mType" is missing from wrapper.
					EditBoxDesc EBD;
					EBD.mParent		= TabWindow;
					EBD.mX			= xj;
					EBD.mY			= YSaved;
					EBD.mWidth		= 250;
					EBD.mHeight		= 150;
					EBD.mLabel		= "============ Articulations ============";
					EBD.mFilter		= EDITBOX_TEXT;
					EBD.mType		= EDITBOX_READ_ONLY;
					IceEditBox* EB = ICE_NEW(IceEditBox)(EBD);
					EB->SetVisible(true);
					gPhysXUI->mPhysXGUI->Register(EB);
				}
			}
		}

		// TAB_SCENE_QUERIES
		{
			IceWindow* TabWindow = Tabs[TAB_SCENE_QUERIES];
			sdword y = YStart;

			helper.CreateCheckBox(TabWindow, PHYSX_GUI_TEST, 4, y, CheckBoxWidth, 20, "Test", gPhysXUI->mPhysXGUI, gTest, gCheckBoxCallback);
			y += YStepCB;

			helper.CreateCheckBox(TabWindow, PHYSX_GUI_ENABLE_SQ, 4, y, CheckBoxWidth, 20, "Enable scene queries", gPhysXUI->mPhysXGUI, gParams.mSQFlag, gCheckBoxCallback);
			y += YStepCB;

			gPhysXUI->mCheckBox_SQ_FilterOutAllShapes = helper.CreateCheckBox(TabWindow, PHYSX_GUI_SQ_FILTER_OUT, 4, y, CheckBoxWidth, 20, "Filter out all shapes (DEBUG)", gPhysXUI->mPhysXGUI, gParams.mSQFilterOutAllShapes, gCheckBoxCallback);
			gPhysXUI->mCheckBox_SQ_FilterOutAllShapes->SetEnabled(gParams.mSQFlag);
			y += YStepCB;

			gPhysXUI->mCheckBox_SQ_InitialOverlap = helper.CreateCheckBox(TabWindow, PHYSX_GUI_SQ_INITIAL_OVERLAP, 4, y, CheckBoxWidth, 20, "eINITIAL_OVERLAP flag (sweeps)", gPhysXUI->mPhysXGUI, gParams.mSQInitialOverlap, gCheckBoxCallback);
			gPhysXUI->mCheckBox_SQ_InitialOverlap->SetEnabled(gParams.mSQFlag);
			y += YStepCB;

	/*		gPhysXUI->mCheckBox_SQ_ManualFlushUpdates = helper.CreateCheckBox(TabWindow, PHYSX_GUI_SQ_MANUAL_FLUSH_UPDATES, 4, y, CheckBoxWidth, 20, "Manual flush updates", gPhysXUI->mPhysXGUI, gParams.mSQManualFlushUpdates, gCheckBoxCallback);
			gPhysXUI->mCheckBox_SQ_ManualFlushUpdates->SetEnabled(gParams.mSQFlag);
			y += YStepCB;*/

			gPhysXUI->mCheckBox_SQ_PreciseSweeps = helper.CreateCheckBox(TabWindow, PHYSX_GUI_SQ_PRECISE_SWEEPS, 4, y, CheckBoxWidth, 20, "Precise sweeps", gPhysXUI->mPhysXGUI, gParams.mSQPreciseSweeps, gCheckBoxCallback);
			gPhysXUI->mCheckBox_SQ_PreciseSweeps->SetEnabled(gParams.mSQFlag);
			y += YStepCB;

			gPhysXUI->mCheckBox_SQ_BothSides = helper.CreateCheckBox(TabWindow, PHYSX_GUI_SQ_BOTH_SIDES, 4, y, CheckBoxWidth, 20, "Both sides", gPhysXUI->mPhysXGUI, gParams.mSQBothSides, gCheckBoxCallback);
			gPhysXUI->mCheckBox_SQ_BothSides->SetEnabled(gParams.mSQFlag);
			y += YStepCB;

			y += YStepCB;
			const sdword xj = 4;
			const sdword LabelWidthJ = 120;
			gPhysXUI->mEditBox_SQ_RebuildRateHint = CreateEditBox(helper, TabWindow, xj, y, "Tree rebuild rate hint:", _F("%d", gParams.mSQDynamicRebuildRateHint), EDITBOX_INTEGER_POSITIVE, LabelWidthJ, LabelWidthJ);
			gPhysXUI->mEditBox_SQ_RebuildRateHint->SetEnabled(gParams.mSQFlag);

			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 90, 20, "Static pruner:", gPhysXUI->mPhysXGUI);
			gPhysXUI->mComboBox_StaticPruner = CreateComboBox<IceComboBox>(TabWindow, PHYSX_GUI_STATIC_PRUNER, 4+OffsetX, y, 150, 20, "Static pruner", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_StaticPruner->Add("eNONE");
			gPhysXUI->mComboBox_StaticPruner->Add("eDYNAMIC_AABB_TREE");
			gPhysXUI->mComboBox_StaticPruner->Add("eSTATIC_AABB_TREE");
			//gPhysXUI->mComboBox_StaticPruner->Add("eLAST");
			gPhysXUI->mComboBox_StaticPruner->Select(gParams.mStaticPruner);
			gPhysXUI->mComboBox_StaticPruner->SetEnabled(gParams.mSQFlag);
			y += YStep;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 90, 20, "Dynamic pruner:", gPhysXUI->mPhysXGUI);
			gPhysXUI->mComboBox_DynamicPruner = CreateComboBox<IceComboBox>(TabWindow, PHYSX_GUI_DYNAMIC_PRUNER, 4+OffsetX, y, 150, 20, "Dynamic pruner", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_DynamicPruner->Add("eNONE");
			gPhysXUI->mComboBox_DynamicPruner->Add("eDYNAMIC_AABB_TREE");
			gPhysXUI->mComboBox_DynamicPruner->Select(gParams.mDynamicPruner);
			gPhysXUI->mComboBox_DynamicPruner->SetEnabled(gParams.mSQFlag);
			y += YStep;
		}

		// TAB_COOKING
		{
			const sdword EditBoxX = 100;
			const sdword LocalLabelWidth = 160;
			const sdword LocalOffsetX = 160;

			IceWindow* TabWindow = Tabs[TAB_COOKING];
			sdword y = YStart;

#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LocalLabelWidth, 20, "Mesh cooking hint:", gPhysXUI->mPhysXGUI);
			gPhysXUI->mComboBox_MeshCookingHint = CreateComboBox<IceComboBox>(TabWindow, PHYSX_GUI_MESH_COOKING_HINT, 4+LocalOffsetX, y, 150, 20, "Mesh cooking hint", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_MeshCookingHint->Add("eSIM_PERFORMANCE");
			gPhysXUI->mComboBox_MeshCookingHint->Add("eCOOKING_PERFORMANCE");
			gPhysXUI->mComboBox_MeshCookingHint->Select(gParams.mMeshCookingHint);
			y += YStep;
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE
			// ### would be easier to use a callback here
			class MPComboBox : public IceComboBox
			{
				public:
										MPComboBox(const ComboBoxDesc& desc) : IceComboBox(desc)	{}
				virtual	void			OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
						const udword Index = GetSelectedIndex();
						const bool UsesBVH34 = Index==1;
#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
						if(gPhysXUI->mCheckBox_QuantizedTrees)
							gPhysXUI->mCheckBox_QuantizedTrees->SetEnabled(UsesBVH34);
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
						if(gPhysXUI->mEditBox_NbTrisPerLeaf)
							gPhysXUI->mEditBox_NbTrisPerLeaf->SetEnabled(UsesBVH34);
	#if PHYSX_SUPPORT_PX_MESH_COOKING_HINT
						if(gPhysXUI->mComboBox_MeshCookingHint)
							gPhysXUI->mComboBox_MeshCookingHint->SetEnabled(!UsesBVH34);
	#endif
	#if PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
						if(gPhysXUI->mComboBox_MeshBuildStrategy)
							gPhysXUI->mComboBox_MeshBuildStrategy->SetEnabled(UsesBVH34);
	#endif
#endif
					}
				}
			};

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LocalLabelWidth, 20, "Midphase:", gPhysXUI->mPhysXGUI);
			gPhysXUI->mComboBox_MidPhase = CreateComboBox<MPComboBox>(TabWindow, PHYSX_GUI_MID_PHASE, 4+LocalOffsetX, y, 150, 20, "Midphase", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_MidPhase->Add("eBVH33");
			gPhysXUI->mComboBox_MidPhase->Add("eBVH34");
			gPhysXUI->mComboBox_MidPhase->Select(gParams.mMidPhaseType);
			y += YStep;
#endif
#if PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, LocalLabelWidth, 20, "Mesh build strategy:", gPhysXUI->mPhysXGUI);
			gPhysXUI->mComboBox_MeshBuildStrategy = CreateComboBox<IceComboBox>(TabWindow, PHYSX_GUI_MESH_BUILD_STRATEGY, 4+LocalOffsetX, y, 150, 20, "Mesh build strategy", gPhysXUI->mPhysXGUI, null);
			gPhysXUI->mComboBox_MeshBuildStrategy->Add("eFAST");
			gPhysXUI->mComboBox_MeshBuildStrategy->Add("eDEFAULT");
			gPhysXUI->mComboBox_MeshBuildStrategy->Add("eSAH");
			gPhysXUI->mComboBox_MeshBuildStrategy->Select(gParams.mMeshBuildStrategy);
			y += YStep;
#endif
#if PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
			gPhysXUI->mCheckBox_QuantizedTrees = helper.CreateCheckBox(TabWindow, PHYSX_GUI_QUANTIZED_TREES, 4, y, CheckBoxWidth, 20, "Quantized trees", gPhysXUI->mPhysXGUI, gParams.mQuantizedTrees, gCheckBoxCallback);
			y += YStepCB;
#endif
#if PHYSX_SUPPORT_PX_MESH_MIDPHASE2
			gPhysXUI->mEditBox_NbTrisPerLeaf = CreateEditBox(helper, TabWindow, 4, y, "Nb tris per leaf:", _F("%d", gParams.mNbTrisPerLeaf), EDITBOX_INTEGER_POSITIVE, 90, EditBoxX);
#endif
			y += YStepCB;
#if PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT
			gPhysXUI->mEditBox_GaussMapLimit = CreateEditBox(helper, TabWindow, 4, y, "GaussMap limit:", _F("%d", gParams.mGaussMapLimit), EDITBOX_INTEGER_POSITIVE, 90, EditBoxX);
#endif
#if PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE
			helper.CreateCheckBox(TabWindow, PHYSX_GUI_PRECOMPUTE_ACTIVE_EDGES, 4, y, CheckBoxWidth, 20, "Precompute active edges", gPhysXUI->mPhysXGUI, gParams.mPrecomputeActiveEdges, gCheckBoxCallback);
			y += YStepCB;
#endif

#if PHYSX_SUPPORT_PX_MESH_MIDPHASE
			if(gPhysXUI->mComboBox_MidPhase)
				gPhysXUI->mComboBox_MidPhase->OnComboBoxEvent(CBE_SELECTION_CHANGED);
#endif
		}

		// TAB_VEHICLES
		{
			const sdword EditBoxX = 100;

			IceWindow* TabWindow = Tabs[TAB_VEHICLES];
			sdword y = YStart;

#if PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
			gPhysXUI->mCheckBox_UseSuspensionSweeps = helper.CreateCheckBox(TabWindow, PHYSX_GUI_USE_SUSPENSION_SWEEPS, 4, y, CheckBoxWidth, 20, "Use sweeps (instead of raycasts)", gPhysXUI->mPhysXGUI, gParams.mUseSuspensionSweeps, gCheckBoxCallback);
			y += YStepCB;

			gPhysXUI->mCheckBox_ForceSphereWheels = helper.CreateCheckBox(TabWindow, PHYSX_GUI_FORCE_SPHERE_WHEELS, 4, y, CheckBoxWidth, 20, "Force sphere wheels", gPhysXUI->mPhysXGUI, gParams.mForceSphereWheels, gCheckBoxCallback);
			y += YStepCB;

			gPhysXUI->mCheckBox_UseBlockingSweeps = helper.CreateCheckBox(TabWindow, PHYSX_GUI_USE_BLOCKING_SWEEPS, 4, y, CheckBoxWidth, 20, "Use blocking sweeps", gPhysXUI->mPhysXGUI, gParams.mUseBlockingSweeps, gCheckBoxCallback);
			y += YStepCB;

			gPhysXUI->mCheckBox_DrawSweptWheels = helper.CreateCheckBox(TabWindow, PHYSX_GUI_DRAW_SWEPT_WHEELS, 4, y, CheckBoxWidth, 20, "Debug draw swept wheels", gPhysXUI->mPhysXGUI, gParams.mDrawSweptWheels, gCheckBoxCallback);
			y += YStepCB;
#endif
			gPhysXUI->mCheckBox_DrawSuspensionCasts = helper.CreateCheckBox(TabWindow, PHYSX_GUI_DRAW_SUSPENSION_CASTS, 4, y, CheckBoxWidth, 20, "Debug draw hit data", gPhysXUI->mPhysXGUI, gParams.mDrawSuspensionCasts, gCheckBoxCallback);
			y += YStepCB;

			gPhysXUI->mCheckBox_DrawPhysXTelemetry = helper.CreateCheckBox(TabWindow, PHYSX_GUI_DRAW_PHYSX_TELEMETRY, 4, y, CheckBoxWidth, 20, "Debug PhysX telemetry", gPhysXUI->mPhysXGUI, gParams.mDrawPhysXTelemetry, gCheckBoxCallback);
			y += YStepCB;

#ifdef PHYSX_SUPPORT_VEHICLE_FIX
			gPhysXUI->mCheckBox_UseFix = helper.CreateCheckBox(TabWindow, PHYSX_GUI_USE_FIX, 4, y, CheckBoxWidth, 20, "Use experimental fix", gPhysXUI->mPhysXGUI, gParams.mUseFix, gCheckBoxCallback);
			y += YStepCB;
#endif
#if PHYSX_SUPPORT_STEER_FILTER
			y += YStep;
			gPhysXUI->mEditBox_SteerFilter = CreateEditBox(helper, TabWindow, 4, y, "Steer filter:", helper.Convert(gParams.mSteerFilter), EDITBOX_FLOAT_POSITIVE, 90, EditBoxX);
#endif
#if PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
			y += YStep;
			gPhysXUI->mEditBox_SweepInflation = CreateEditBox(helper, TabWindow, 4, y, "Sweep inflation:", helper.Convert(gParams.mSweepInflation), EDITBOX_FLOAT_POSITIVE, 90, EditBoxX);
#endif

			// TODO: add vehicle params here
		}

#if PHYSX_SUPPORT_FLUIDS
		// TAB_FLUIDS
		{
			const sdword EditBoxX = 100;

			IceWindow* TabWindow = Tabs[TAB_FLUIDS];
			sdword y = YStart;

			gPhysXUI->mEditBox_FluidFriction = CreateEditBox(helper, TabWindow, 4, y, "Friction:", helper.Convert(gParams.mFluidFriction), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidDamping = CreateEditBox(helper, TabWindow, 4, y, "Damping:", helper.Convert(gParams.mFluidDamping), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidAdhesion = CreateEditBox(helper, TabWindow, 4, y, "Adhesion:", helper.Convert(gParams.mFluidAdhesion), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidViscosity = CreateEditBox(helper, TabWindow, 4, y, "Viscosity:", helper.Convert(gParams.mFluidViscosity), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidVorticityConfinement = CreateEditBox(helper, TabWindow, 4, y, "Vorticity:", helper.Convert(gParams.mFluidVorticityConfinement), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidSurfaceTension = CreateEditBox(helper, TabWindow, 4, y, "Surface tension:", helper.Convert(gParams.mFluidSurfaceTension), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidCohesion = CreateEditBox(helper, TabWindow, 4, y, "Cohesion:", helper.Convert(gParams.mFluidCohesion), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidLift = CreateEditBox(helper, TabWindow, 4, y, "Lift:", helper.Convert(gParams.mFluidLift), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidDrag = CreateEditBox(helper, TabWindow, 4, y, "Drag:", helper.Convert(gParams.mFluidDrag), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidCflCoefficient = CreateEditBox(helper, TabWindow, 4, y, "Cfl coeff:", helper.Convert(gParams.mFluidCflCoefficient), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidGravityScale = CreateEditBox(helper, TabWindow, 4, y, "Gravity scale:", helper.Convert(gParams.mFluidGravityScale), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mEditBox_FluidParticleMass = CreateEditBox(helper, TabWindow, 4, y, "Particle mass:", helper.Convert(gParams.mFluidParticleMass), EDITBOX_FLOAT, 90, EditBoxX);
			gPhysXUI->mCheckBox_FluidParticleCCD = helper.CreateCheckBox(TabWindow, PHYSX_GUI_FLUID_PARTICLE_CCD, 4, y, CheckBoxWidth, 20, "Particle CCD", gPhysXUI->mPhysXGUI, gParams.mFluidParticleCCD, gCheckBoxCallback);
			y += YStepCB;
		}
#endif

		// TAB_DEBUG_VIZ
		{
			const sdword EditBoxX = 100;

			const udword NB_DEBUG_VIZ_PARAMS = gPhysXUI->mNbDebugVizParams;
			bool* gDebugVizParams = gPhysXUI->mDebugVizParams;
			const char** gDebugVizNames = gPhysXUI->mDebugVizNames;

			IceWindow* TabWindow = Tabs[TAB_DEBUG_VIZ];
			sdword y = YStart;

			helper.CreateLabel(TabWindow, 4, y+LabelOffsetY, 90, 20, "Debug viz scale:", gPhysXUI->mPhysXGUI);
			gPhysXUI->mEditBox_DebugVizScale = helper.CreateEditBox(TabWindow, PHYSX_GUI_DEBUG_VIZ_SCALE, 4+EditBoxX, y, EditBoxWidth, 20, _F("%f", gParams.mDebugVizScale), gPhysXUI->mPhysXGUI, EDITBOX_FLOAT_POSITIVE, Callbacks::EditBoxChange);
			gPhysXUI->mEditBox_DebugVizScale->SetUserData(&callback);

			y += YStep;

			sdword LastY = Common_CreateDebugVizUI(TabWindow, 10, y, gCheckBoxCallback, PHYSX_GUI_ENABLE_DEBUG_VIZ, NB_DEBUG_VIZ_PARAMS, gDebugVizParams, gDebugVizNames, gPhysXUI->mCheckBox_DebugVis, gPhysXUI->mPhysXGUI);
		}
	}
//	gPhysXUI->mCheckBox_DrawMBPRegions = helper.CreateCheckBox(Main, PHYSX_GUI_DRAW_MBP_REGIONS, 290+10, LastY, CheckBoxWidth, 20, "Draw MBP regions", gPhysXUI->mPhysXGUI, gVisualizeMBPRegions, gCheckBoxCallback);

	if(gPhysXUI && gPhysXUI->mComboBox_Config)
	{
		gPhysXUI->mComboBox_Config->Select(1);
		gPhysXUI->mComboBox_Config->OnComboBoxEvent(CBE_SELECTION_CHANGED);
	}

	return Main_;
}

void PhysX3::CloseSharedGUI()
{
	DELETESINGLE(gPhysXUI);
}

