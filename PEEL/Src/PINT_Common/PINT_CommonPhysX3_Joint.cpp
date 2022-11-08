///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_Joint.h"
#include "PINT_CommonPhysX3.h"

#if PHYSX_SUPPORT_GEAR_JOINT
	#include "PxGearJoint.h"
#endif

#if PHYSX_SUPPORT_RACK_JOINT
	#include "PxRackAndPinionJoint.h"
#endif

PhysX_JointAPI::PhysX_JointAPI(Pint& pint) : Pint_Joint(pint)
{
}

PhysX_JointAPI::~PhysX_JointAPI()
{
}

///////////////////////////////////////////////////////////////////////////////

const char* PhysX_JointAPI::GetName(PintJointHandle handle) const
{
	const PxJoint* Joint = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(Joint);
	return Joint->getName();
}

bool PhysX_JointAPI::SetName(PintJointHandle handle, const char* name)
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	physx.SetJointName(Joint, name);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

PintJoint PhysX_JointAPI::GetType(PintJointHandle handle) const
{
	const PxJoint* Joint = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(Joint);

	switch(Joint->getConcreteType())
	{
#ifdef IS_PHYSX_3_2
		case PxConcreteType::eUSER_SPHERICAL_JOINT:		return PINT_JOINT_SPHERICAL;
		case PxConcreteType::eUSER_REVOLUTE_JOINT:		return PINT_JOINT_HINGE;
		case PxConcreteType::eUSER_PRISMATIC_JOINT:		return PINT_JOINT_PRISMATIC;
		case PxConcreteType::eUSER_FIXED_JOINT:			return PINT_JOINT_FIXED;
		case PxConcreteType::eUSER_DISTANCE_JOINT:		return PINT_JOINT_DISTANCE;
		case PxConcreteType::eUSER_D6_JOINT:			return PINT_JOINT_D6;
//		case PxConcreteType::eCONTACT:					return PINT_JOINT_SPHERICAL;
#else
		case PxJointConcreteType::eSPHERICAL:		return PINT_JOINT_SPHERICAL;
		case PxJointConcreteType::eREVOLUTE:		return PINT_JOINT_HINGE;
		case PxJointConcreteType::ePRISMATIC:		return PINT_JOINT_PRISMATIC;
		case PxJointConcreteType::eFIXED:			return PINT_JOINT_FIXED;
		case PxJointConcreteType::eDISTANCE:		return PINT_JOINT_DISTANCE;
		case PxJointConcreteType::eD6:				return PINT_JOINT_D6;
//		case PxJointConcreteType::eCONTACT:			return PINT_JOINT_SPHERICAL;
	#if PHYSX_SUPPORT_PORTAL_JOINT
		// ###
	#endif
	#if PHYSX_SUPPORT_NEW_JOINT_TYPES
		case PxJointConcreteType::eGEAR:			return PINT_JOINT_GEAR;
	#endif
	#if PHYSX_SUPPORT_NEW_JOINT_TYPES
		case PxJointConcreteType::eRACK_AND_PINION:	return PINT_JOINT_RACK_AND_PINION;
	#endif
#endif
	}
	return PINT_JOINT_UNDEFINED;
}

///////////////////////////////////////////////////////////////////////////////

bool PhysX_JointAPI::GetActors(PintJointHandle handle, PintActorHandle& actor0, PintActorHandle& actor1) const
{
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	ASSERT(Joint);

	PxRigidActor* pxActor0;
	PxRigidActor* pxActor1;
	Joint->getActors(pxActor0, pxActor1);

	actor0 = PintActorHandle(pxActor0);
	actor1 = PintActorHandle(pxActor1);
	return true;
}

bool PhysX_JointAPI::SetActors(PintJointHandle handle, PintActorHandle actor0, PintActorHandle actor1)
{
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	ASSERT(Joint);

	PxRigidActor* RigidActor0 = reinterpret_cast<PxRigidActor*>(actor0);
	PxRigidActor* RigidActor1 = reinterpret_cast<PxRigidActor*>(actor1);

	Joint->setActors(RigidActor0, RigidActor1);

	/*PxScene* scene = Joint->getScene();
	if(scene)
	{
		if(RigidActor0)
			scene->resetFiltering(*RigidActor0);
		if(RigidActor1)
			scene->resetFiltering(*RigidActor1);
	}*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool PhysX_JointAPI::GetFrames(PintJointHandle handle, PR* frame0, PR* frame1) const
{
	const PxJoint* Joint = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(Joint);

	if(frame0)
	{
		const PxTransform Pose0 = Joint->getLocalPose(PxJointActorIndex::eACTOR0);
		*frame0 = ToPR(Pose0);
	}

	if(frame1)
	{
		const PxTransform Pose1 = Joint->getLocalPose(PxJointActorIndex::eACTOR1);
		*frame1 = ToPR(Pose1);
	}
	return true;
}

bool PhysX_JointAPI::SetFrames(PintJointHandle handle, const PR* frame0, const PR* frame1)
{
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	ASSERT(Joint);

	if(frame0)
		Joint->setLocalPose(PxJointActorIndex::eACTOR0, ToPxTransform(*frame0));

	if(frame1)
		Joint->setLocalPose(PxJointActorIndex::eACTOR1, ToPxTransform(*frame1));

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool PhysX_JointAPI::GetLimits(PintJointHandle handle, PintLimits& limits, udword index) const
{
	const PxJoint* Joint = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(Joint);

	switch(GetType(handle))
	{
		case PINT_JOINT_SPHERICAL:
		{
			ASSERT(index==0);
			const PxSphericalJoint* SJ = static_cast<const PxSphericalJoint*>(Joint);

			if(SJ->getSphericalJointFlags() & PxSphericalJointFlag::eLIMIT_ENABLED)
			{
				const PxJointLimitCone coneLimits = SJ->getLimitCone();
				// ### names
				limits.mMinValue = coneLimits.yAngle;
				limits.mMaxValue = coneLimits.zAngle;
			}
			else
			{
				SetSphericalLimitDisabled(limits);
			}
			return true;

/*
		const bool ValidLimits = IsSphericalLimitEnabled(jc.mLimits);
		if(ValidLimits)
		{
			// ### names
			const PxJointLimitCone limits(jc.mLimits.mMinValue, jc.mLimits.mMaxValue);
			j->setLimitCone(limits);
		}
		j->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, ValidLimits);
*/

		}
		break;

		case PINT_JOINT_HINGE:
		case PINT_JOINT_HINGE2:
		{
			ASSERT(index==0);
			const PxRevoluteJoint* RJ = static_cast<const PxRevoluteJoint*>(Joint);

			if(RJ->getRevoluteJointFlags() & PxRevoluteJointFlag::eLIMIT_ENABLED)
			{
				const PxJointAngularLimitPair limit = RJ->getLimit();
				limits.mMinValue = limit.lower;
				limits.mMaxValue = limit.upper;
			}
			else
			{
				SetHingeLimitDisabled(limits);
			}
			return true;
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			ASSERT(index==0);
			const PxPrismaticJoint* PJ = static_cast<const PxPrismaticJoint*>(Joint);

			if(PJ->getPrismaticJointFlags() & PxPrismaticJointFlag::eLIMIT_ENABLED)
			{
				const PxJointLinearLimitPair limit = PJ->getLimit();
				limits.mMinValue = limit.lower;
				limits.mMaxValue = limit.upper;
			}
			else
			{
				SetPrismaticLimitDisabled(limits);
			}
			return true;
		}
		break;

/*		case PINT_JOINT_FIXED:
		{
		}
		break;*/

		case PINT_JOINT_DISTANCE:
		{
			ASSERT(index==0);
			const PxDistanceJoint* DJ = static_cast<const PxDistanceJoint*>(Joint);

			const PxDistanceJointFlags flags = DJ->getDistanceJointFlags();
			if(flags & PxDistanceJointFlag::eMIN_DISTANCE_ENABLED)
				limits.mMinValue = DJ->getMinDistance();
			else
				SetMinDistanceLimitDisabled(limits);

			if(flags & PxDistanceJointFlag::eMAX_DISTANCE_ENABLED)
				limits.mMaxValue = DJ->getMaxDistance();
			else
				SetMaxDistanceLimitDisabled(limits);

			return true;
		}
		break;

#ifdef NEW_D6_API
		case PINT_JOINT_D6:
		{
			ASSERT(index<3);
			const PxD6Joint* D6 = static_cast<const PxD6Joint*>(Joint);

			const PxD6Axis::Enum Axis = PxD6Axis::Enum(index);

			const PxD6Motion::Enum M = D6->getMotion(Axis);
			if(M==PxD6Motion::eLIMITED)
			{
				const PxJointLinearLimitPair limit = D6->getLinearLimit(Axis);
				limits.mMinValue = limit.lower;
				limits.mMaxValue = limit.upper;
			}
			else if(M==PxD6Motion::eLOCKED)
			{
				limits.mMinValue = limits.mMaxValue = 0.0f;
			}
			else // PxD6Motion::eFREE
			{
				SetD6LinearLimitDisabled(limits);
			}
			return true;
		}
		break;
#endif

/*		case PINT_JOINT_GEAR:
		{
		}
		break;

		case PINT_JOINT_RACK_AND_PINION:
		{
		}
		break;

		case PINT_JOINT_CHAIN:
		{
		}
		break;

		case PINT_JOINT_PORTAL:
		{
		}
		break;*/
	};
	return false;
}

bool PhysX_JointAPI::SetLimits(PintJointHandle handle, const PintLimits& limits, udword index)
{
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	ASSERT(Joint);

	// Contact distance is a nasty parameter that creates annoying issues for no reason so I'll just overwrite it all the time
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);
	const float ContactDistance = GetJointContactDistance(physx.GetParams());

	switch(GetType(handle))
	{
		case PINT_JOINT_SPHERICAL:
		{
			ASSERT(index==0);
			PxSphericalJoint* SJ = static_cast<PxSphericalJoint*>(Joint);
			PxJointLimitCone coneLimits = SJ->getLimitCone();		// Fetch current limits to preserve other parameters
			coneLimits.PHYSX_CONTACT_DISTANCE = ContactDistance;	// ...except that one

			const bool ValidLimits = IsSphericalLimitEnabled(limits);
			if(ValidLimits)
			{
				// ### names
				coneLimits.yAngle = limits.mMinValue;
				coneLimits.zAngle = limits.mMaxValue;
			}
			else
			{
				coneLimits.yAngle = 0.0f;
				coneLimits.zAngle = 0.0f;
			}
			SJ->setLimitCone(coneLimits);
			SJ->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, ValidLimits);
		}
		break;

		case PINT_JOINT_HINGE:
		case PINT_JOINT_HINGE2:
		{
			ASSERT(index==0);
			PxRevoluteJoint* RJ = static_cast<PxRevoluteJoint*>(Joint);
			PxJointAngularLimitPair limit = RJ->getLimit();	// Fetch current limits to preserve other parameters
			limit.PHYSX_CONTACT_DISTANCE = ContactDistance;		// ...except that one

			const bool ValidLimits = IsHingeLimitEnabled(limits);
			if(ValidLimits)
			{
				limit.lower = limits.mMinValue;
				limit.upper = limits.mMaxValue;
			}
			else
			{
				// min > max is invalid in PhysX so we have to override the values and lose the actual settings
				limit.lower = 0.0f;
				limit.upper = 0.0f;
			}
			RJ->setLimit(limit);
			RJ->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, ValidLimits);

			return true;
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			ASSERT(index==0);
			PxPrismaticJoint* PJ = static_cast<PxPrismaticJoint*>(Joint);
			PxJointLinearLimitPair limit = PJ->getLimit();		// Fetch current limits to preserve other parameters
			limit.PHYSX_CONTACT_DISTANCE = ContactDistance;		// ...except that one

			const bool ValidLimits = IsPrismaticLimitEnabled(limits);
			if(ValidLimits)
			{
				limit.lower = limits.mMinValue;
				limit.upper = limits.mMaxValue;
			}
			else
			{
				// min > max is invalid in PhysX so we have to override the values and lose the actual settings
				limit.lower = 0.0f;
				limit.upper = 0.0f;
			}
			PJ->setLimit(limit);
			PJ->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, ValidLimits);

			return true;
		}
		break;

		case PINT_JOINT_DISTANCE:
		{
			ASSERT(index==0);
			PxDistanceJoint* DJ = static_cast<PxDistanceJoint*>(Joint);

			// ### tolerance?

			const bool ValidMinLimit = IsMinDistanceLimitEnabled(limits);
			DJ->setMinDistance(ValidMinLimit ? limits.mMinValue : 0.0f);
			DJ->setDistanceJointFlag(PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, ValidMinLimit);

			const bool ValidMaxLimit = IsMaxDistanceLimitEnabled(limits);
			DJ->setMaxDistance(ValidMaxLimit ? limits.mMaxValue : 0.0f);
			DJ->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, ValidMaxLimit);
			return true;
		}
		break;

		case PINT_JOINT_D6:
		{
			ASSERT(index<3);
			PxD6Joint* D6 = static_cast<PxD6Joint*>(Joint);

			const PxD6Axis::Enum Axis = PxD6Axis::Enum(index);

			float MinLimit = limits.mMinValue;
			float MaxLimit = limits.mMaxValue;

			const bool ValidLimits = IsD6LinearLimitEnabled(limits);
			if(ValidLimits)
			{
				if(MinLimit==MaxLimit)
					D6->setMotion(Axis, PxD6Motion::eLOCKED);
				else
					D6->setMotion(Axis, PxD6Motion::eLIMITED);
			}
			else
			{
				// min > max is invalid in PhysX so we have to override the values and lose the actual settings
				MinLimit = MaxLimit = 0.0f;
				D6->setMotion(Axis, PxD6Motion::eFREE);
			}

#ifdef NEW_D6_API
			PxJointLinearLimitPair limit = D6->getLinearLimit(Axis);	// Fetch current limits to preserve other parameters
			limit.PHYSX_CONTACT_DISTANCE = ContactDistance;				// ...except that one
			limit.lower = MinLimit;
			limit.upper = MaxLimit;
			D6->setLinearLimit(Axis, limit);
#endif
			return true;
		}
		break;
	};
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool PhysX_JointAPI::GetSpring(PintJointHandle handle, PintSpring& spring) const
{
	const PxJoint* Joint = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(Joint);

	switch(GetType(handle))
	{
		case PINT_JOINT_PRISMATIC:
		{
			const PxPrismaticJoint* PJ = static_cast<const PxPrismaticJoint*>(Joint);
			const PxJointLinearLimitPair limit = PJ->getLimit();
			spring.mStiffness = limit.stiffness;
			spring.mDamping = limit.damping;
			return true;
		}
		break;
	};
	return false;
}

bool PhysX_JointAPI::SetSpring(PintJointHandle handle, const PintSpring& spring)
{
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	ASSERT(Joint);

	switch(GetType(handle))
	{
		case PINT_JOINT_PRISMATIC:
		{
			PxPrismaticJoint* PJ = static_cast<PxPrismaticJoint*>(Joint);
			PxJointLinearLimitPair limit = PJ->getLimit();
			limit.stiffness = spring.mStiffness;
			limit.damping = spring.mDamping;
			PJ->setLimit(limit);
			return true;
		}
		break;
	};
	return false;
}

///////////////////////////////////////////////////////////////////////////////

#if PHYSX_SUPPORT_GEAR_JOINT
bool PhysX_JointAPI::GetGearRatio(PintJointHandle handle, float& ratio) const
{
	const PxJoint* Joint = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(Joint);

	switch(GetType(handle))
	{
		case PINT_JOINT_GEAR:
		{
			const PxGearJoint* GJ = static_cast<const PxGearJoint*>(Joint);
			ratio = GJ->getGearRatio();
			return true;
		}
		break;
	};
	return false;
}

bool PhysX_JointAPI::SetGearRatio(PintJointHandle handle, float ratio)
{
	PxJoint* Joint = reinterpret_cast<PxJoint*>(handle);
	ASSERT(Joint);

	switch(GetType(handle))
	{
		case PINT_JOINT_GEAR:
		{
			PxGearJoint* GJ = static_cast<PxGearJoint*>(Joint);
			GJ->setGearRatio(ratio);
			return true;
		}
		break;
	};
	return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////

bool PhysX_JointAPI::GetHingeDynamicData(PintJointHandle handle, PintHingeDynamicData& data) const
{
	const PxJoint* J = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(J);

	switch(GetType(handle))
	{
		case PINT_JOINT_HINGE:
		{
			const PxRevoluteJoint* RJ = static_cast<const PxRevoluteJoint*>(J);
			data.mTwistAngle = RJ->getAngle();
			return true;
		}
		break;

		case PINT_JOINT_D6:
		{
			const PxD6Joint* D6 = static_cast<const PxD6Joint*>(J);
			data.mTwistAngle = D6->getTwist();
			return true;
		}
		break;
	};
	return false;
}

bool PhysX_JointAPI::GetD6DynamicData(PintJointHandle handle, PintD6DynamicData& data) const
{
	const PxJoint* J = reinterpret_cast<const PxJoint*>(handle);
	ASSERT(J);

	switch(GetType(handle))
	{
		case PINT_JOINT_D6:
		{
			const PxD6Joint* D6 = static_cast<const PxD6Joint*>(J);
			data.mTwistAngle = D6->getTwist();
			data.mSwingYAngle = D6->getSwingYAngle();
			data.mSwingZAngle = D6->getSwingZAngle();
			return true;
		}
		break;
	};
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void normalToTangents(const PxVec3& n, PxVec3& t1, PxVec3& t2)
{
	const PxReal m_sqrt1_2 = PxReal(0.7071067811865475244008443621048490);
	if(fabsf(n.z) > m_sqrt1_2)
	{
		const PxReal a = n.y*n.y + n.z*n.z;
		const PxReal k = PxReal(1.0)/PxSqrt(a);
		t1 = PxVec3(0,-n.z*k,n.y*k);
		t2 = PxVec3(a*k,-n.x*t1.z,n.x*t1.y);
	}
	else 
	{
		const PxReal a = n.x*n.x + n.y*n.y;
		const PxReal k = PxReal(1.0)/PxSqrt(a);
		t1 = PxVec3(-n.y*k,n.x*k,0);
		t2 = PxVec3(-n.z*t1.y,n.z*t1.x,a*k);
	}
	t1.normalize();
	t2.normalize();
}

PxQuat ComputeJointQuat(PxRigidActor* actor, const PxVec3& localAxis)
{
//	return PxQuat(PxIdentity);

	//find 2 orthogonal vectors.
	//gotta do this in world space, if we choose them
	//separately in local space they won't match up in worldspace.
	PxVec3 axisw = actor ? actor->getGlobalPose().rotate(localAxis) : localAxis;
	axisw.normalize();

	PxVec3 normalw, binormalw;
	::normalToTangents(axisw, binormalw, normalw);

	const PxVec3 localNormal = actor ? actor->getGlobalPose().rotateInv(normalw) : normalw;

	const PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
	PxQuat q(rot);
	q.normalize();

/*		if(q.w<0.0f)
		{
			printf("Negating quat...\n");
			q = -q;
		}*/

	return q;
}

void SetupD6Projection(PxD6Joint* j, bool enable_projection, float projection_linear_tolerance, float projection_angular_tolerance)
{
	if(enable_projection)
	{
		j->setProjectionLinearTolerance(projection_linear_tolerance);
		j->setProjectionAngularTolerance(projection_angular_tolerance);
	}
}

float GetJointContactDistance(const EditableParams& params)
{
	if(params.mLimitsContactDistance==0)
		return FLT_MAX;
	else if(params.mLimitsContactDistance==1)
		return -1.0f;
	else
		return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////

// Note about limits: in PEEL we use invalid limits like min>max to indicate "no limits" (free joints).
// In PhysX this is invalid and it produces an error in debug builds. A side effect of that error is that
// the other limit-related parameters (spring params, contact distance, etc) are not set when PhysX early
// exits and returns the warning. So we lose these parameters and they're incorrect if we enable proper
// limits later. As a result we have to modify the given values sent by PEEL when creating the joints.

///////////////////////////////////////////////////////////////////////////////

PxSphericalJoint* CreateSphericalJoint(const EditableParams& params, PxPhysics& physics, const PINT_SPHERICAL_JOINT_CREATE& jc, PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1)
{
	PxSphericalJoint* j = PxSphericalJointCreate(physics, actor0, localFrame0, actor1, localFrame1);
	ASSERT(j);
	if(j)
	{
		const bool ValidLimits = IsSphericalLimitEnabled(jc.mLimits);
		if(ValidLimits)
		{
			// ### names
			const PxJointLimitCone limits(jc.mLimits.mMinValue, jc.mLimits.mMaxValue);
			j->setLimitCone(limits);
		}
		j->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, ValidLimits);

/*		if(0 && jc.mName && strcmp(jc.mName, "HardcodedTest")==0)
		{
			PxJointLimitCone limitCone = j->getLimitCone();
			PxReal swingY = j->getSwingYAngle();
			PxReal swingZ = j->getSwingZAngle();

			//j->setLimitCone(PxJointLimitCone(PI/4, PI/4));
			j->setLimitCone(PxJointLimitCone(PI/8, PI/8));

			PxTransform tmp = localFrame0;
			tmp.q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 1.0f));
			//tmp.q = PxShortestRotation(PxVec3(0.0f, 0.0f, 1.0f), PxVec3(1.0f, 0.0f, 0.0f));
			//tmp.q = PxQuat(PxIdentity);
			j->setLocalPose(PxJointActorIndex::eACTOR0, tmp);

			tmp.p = localFrame1.p;
			tmp.q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, -1.0f));
			j->setLocalPose(PxJointActorIndex::eACTOR1, tmp);

			j->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, true);
		}*/

		if(params.mEnableJointProjection)
		{
			// Angular tolerance not used for spherical joints
			j->setProjectionLinearTolerance(params.mProjectionLinearTolerance);
		}
	}
	return j;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

PxFixedJoint* CreateFixedJoint(const EditableParams& params, PxPhysics& physics, const PINT_FIXED_JOINT_CREATE& jc, PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1)
{
	PxFixedJoint* j = PxFixedJointCreate(physics, actor0, localFrame0, actor1, localFrame1);
	ASSERT(j);
	if(j)
	{
		if(params.mEnableJointProjection)
		{
			j->setProjectionLinearTolerance(params.mProjectionLinearTolerance);
			j->setProjectionAngularTolerance(params.mProjectionAngularTolerance * DEGTORAD);
		}
	}
	return j;
}

///////////////////////////////////////////////////////////////////////////////

PxPrismaticJoint* CreatePrismaticJoint(const EditableParams& params, PxPhysics& physics, const PINT_PRISMATIC_JOINT_CREATE& jc, PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1)
{
	PxPrismaticJoint* j = PxPrismaticJointCreate(physics, actor0, localFrame0, actor1, localFrame1);
	ASSERT(j);
	if(j)
	{
		float MinLimit = 0.0f;	// See above note about limits
		float MaxLimit = 0.0f;
		const bool ValidLimits = IsHingeLimitEnabled(jc.mLimits);
		if(ValidLimits)
		{
			MinLimit = jc.mLimits.mMinValue;
			MaxLimit = jc.mLimits.mMaxValue;
		}

		// We setup the limits even when invalid, to preserve other params (spring, etc)
#ifdef IS_PHYSX_3_2
		PxJointLimitPair Limits(MinLimit, MaxLimit, 100.0f);
		Limits.spring = jc.mSpringStiffness;
		Limits.damping = jc.mSpringDamping;
#else
		PxJointLinearLimitPair Limits(MinLimit, MaxLimit, PxSpring(jc.mSpring.mStiffness, jc.mSpring.mDamping));
		if(jc.mSpring.mStiffness==0.0f && jc.mSpring.mDamping==0.0f)
		{
			const float ContactDistance = GetJointContactDistance(params);

			//### these hard joints are better looking against limits but don't support "springs", at least in this form
			Limits = PxJointLinearLimitPair(PxTolerancesScale(), MinLimit, MaxLimit, ContactDistance);
		}
#endif
		j->setLimit(Limits);
		j->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, ValidLimits);

		if(params.mEnableJointProjection)
		{
			j->setProjectionLinearTolerance(params.mProjectionLinearTolerance);
			j->setProjectionAngularTolerance(params.mProjectionAngularTolerance * DEGTORAD);
		}

		//j->setBreakForce(1000.0f, 1000.0f);
		//j->setConstraintFlags(PxConstraintFlags flags)	= 0;
	}
	return j;
}

///////////////////////////////////////////////////////////////////////////////

PxDistanceJoint* CreateDistanceJoint(const EditableParams& params, PxPhysics& physics, const PINT_DISTANCE_JOINT_CREATE& jc, PxRigidActor* actor0, PxRigidActor* actor1, const PxTransform& localFrame0, const PxTransform& localFrame1)
{
	PxDistanceJoint* j = PxDistanceJointCreate(physics, actor0, localFrame0, actor1, localFrame1);
	ASSERT(j);
	if(j)
	{
		if(IsMaxDistanceLimitEnabled(jc.mLimits))
		{
			j->setMaxDistance(jc.mLimits.mMaxValue);
			j->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
		}
		else j->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, false);

		if(IsMinDistanceLimitEnabled(jc.mLimits))
		{
			j->setMinDistance(jc.mLimits.mMinValue);
			j->setDistanceJointFlag(PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, true);
		}
		else j->setDistanceJointFlag(PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, false);

//		j->setTolerance(0.0f);

/*		if(params.mEnableJointProjection)
		{
			j->setProjectionLinearTolerance(params.mProjectionLinearTolerance);
			j->setProjectionAngularTolerance(params.mProjectionAngularTolerance * DEGTORAD);
		}*/
//		j->setTolerance(params.mProjectionLinearTolerance);
	}
	return j;
}

///////////////////////////////////////////////////////////////////////////////
