///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX4 plug-ins, so put only the code here that is "the same" for all versions.

// This file contains code related to Reduced Coordinates Articulations.
// It is controlled by PHYSX_SUPPORT_RCA at the header level. At the cpp level, just add or don't add this file to the project.

#include "stdafx.h"
#include "..\Pint.h"
#include "PINT_Common.h"
#include "PINT_CommonPhysX3.h"

extern udword gNbPhysXErrors;

PintArticHandle SharedPhysX::CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE& create)
{
	if(mParams.mDisableArticulations)
		return null;

	PxArticulationReducedCoordinate* Articulation = mPhysics->createArticulationReducedCoordinate();
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

	if(create.mFixBase)
		Articulation->setArticulationFlags(PxArticulationFlag::eFIX_BASE);

	return PintArticHandle(Articulation);
}

bool SharedPhysX::AddRCArticulationToScene(PintArticHandle articulation)
{
	PxArticulationReducedCoordinate* Articulation = reinterpret_cast<PxArticulationReducedCoordinate*>(articulation);

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

bool SharedPhysX::AddRCArticulationToAggregate(PintArticHandle articulation, PintAggregateHandle aggregate)
{
	PxArticulationReducedCoordinate* Articulation = reinterpret_cast<PxArticulationReducedCoordinate*>(articulation);
	PxAggregate* Aggregate = (PxAggregate*)aggregate;
	return Aggregate->addArticulation(*Articulation);
}

static void SetModeAndLimits(const EditableParams& params, PxArticulationJointReducedCoordinate* j, PxArticulationAxis::Enum axis, float min_limit, float max_limit)
{
	if(min_limit>max_limit)
		j->setMotion(axis, PxArticulationMotion::eFREE);
	else if(min_limit==max_limit)
		j->setMotion(axis, PxArticulationMotion::eLOCKED);
	else if(min_limit<max_limit)
	{
		j->setMotion(axis, PxArticulationMotion::eLIMITED);
		j->setLimit(axis, min_limit, max_limit);
	}

#if PHYSX_SUPPORT_RCA_DOF_SCALE
	j->setDofScale(axis, params.mRCADofScale);
#endif
#if PHYSX_SUPPORT_RCA_ARMATURE
	j->setArmature(axis, params.mRCAArmature);
#endif
}

static void setupJoint(const EditableParams& params, PxArticulationJointReducedCoordinate* j, const PINT_RC_ARTICULATED_BODY_CREATE& bc)
{
//	setupJoint(j);

//PxArticulationJointReducedCoordinate
/*
		virtual	void								setMotion(PxArticulationAxis::Enum axis, PxArticulationMotion::Enum motion) = 0;
		virtual	PxArticulationMotion::Enum			getMotion(PxArticulationAxis::Enum axis) const = 0;

		virtual void setLimit(PxArticulationAxis::Enum axis, const PxReal lowLimit, const PxReal highLimit) = 0;
		virtual void getLimit(PxArticulationAxis::Enum axis, PxReal& lowLimit, PxReal& highLimit) = 0;
		virtual void setDrive(PxArticulationAxis::Enum axis, const PxReal stiffness, const PxReal damping, const PxReal maxForce, bool isAccelerationDrive = false) = 0;
		virtual void getDrive(PxArticulationAxis::Enum axis, PxReal& stiffness, PxReal& damping, PxReal& maxForce, bool& isAcceleration) = 0;
		virtual void setDriveTarget(PxArticulationAxis::Enum axis, const PxReal target) = 0;
		virtual void setDriveVelocity(PxArticulationAxis::Enum axis, const PxReal targetVel) = 0;
		virtual PxReal getDriveTarget(PxArticulationAxis::Enum axis) = 0;
		virtual PxReal getDriveVelocity(PxArticulationAxis::Enum axis) = 0;

		virtual	void			setFrictionCoefficient(const PxReal coefficient) = 0;
		virtual	PxReal			getFrictionCoefficient() const = 0;
		virtual	const char*		getConcreteTypeName() const { return "PxArticulationJointReducedCoordinate"; }

		virtual void	setMaxJointVelocity(const PxReal maxJointV) = 0;
		virtual PxReal	getMaxJointVelocity() const = 0;
*/

	if(bc.mJointType==PINT_JOINT_SPHERICAL)
	{
		j->setJointType(PxArticulationJointType::eSPHERICAL);
//		j->setMotion(PxArticulationAxis::eSWING2, PxArticulationMotion::eFREE);
//		j->setMotion(PxArticulationAxis::eSWING1, PxArticulationMotion::eFREE);
//		j->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eFREE);
		SetModeAndLimits(params, j, PxArticulationAxis::eTWIST, bc.mMinTwistLimit, bc.mMaxTwistLimit);
		SetModeAndLimits(params, j, PxArticulationAxis::eSWING1, bc.mMinSwing1Limit, bc.mMaxSwing1Limit);
		SetModeAndLimits(params, j, PxArticulationAxis::eSWING2, bc.mMinSwing2Limit, bc.mMaxSwing2Limit);
	}
	else if(bc.mJointType==PINT_JOINT_FIXED)
		j->setJointType(PxArticulationJointType::eFIX);
	else if(bc.mJointType==PINT_JOINT_PRISMATIC)
	{
		j->setJointType(PxArticulationJointType::ePRISMATIC);

		PxArticulationAxis::Enum Axis = PxArticulationAxis::eCOUNT;
		if(bc.mAxisIndex==X_)
			Axis = PxArticulationAxis::eX;
		else if(bc.mAxisIndex==Y_)
			Axis = PxArticulationAxis::eY;
		else if(bc.mAxisIndex==Z_)
			Axis = PxArticulationAxis::eZ;

/*		const float MinLimit = bc.mMinLimit;
		const float MaxLimit = bc.mMaxLimit;

		if(MinLimit>MaxLimit)
			j->setMotion(Axis, PxArticulationMotion::eFREE);
		if(MinLimit==MaxLimit)
			j->setMotion(Axis, PxArticulationMotion::eLOCKED);
		if(MinLimit<MaxLimit)
		{
			j->setMotion(Axis, PxArticulationMotion::eLIMITED);
			j->setLimit(Axis, MinLimit, MaxLimit);
		}*/
		SetModeAndLimits(params, j, Axis, bc.mMinLimit, bc.mMaxLimit);

//		j->setMotion(PxArticulationAxis::eZ, PxArticulationMotion::eLIMITED);
//		j->setLimit(PxArticulationAxis::eZ, -1.4f, 0.2f);
	}
	else if(bc.mJointType==PINT_JOINT_HINGE)
	{
		j->setJointType(PxArticulationJointType::eREVOLUTE);

		PxArticulationAxis::Enum Axis = PxArticulationAxis::eCOUNT;
		if(bc.mAxisIndex==X_)
			Axis = PxArticulationAxis::eTWIST;
		else if(bc.mAxisIndex==Y_)
			Axis = PxArticulationAxis::eSWING1;
		else if(bc.mAxisIndex==Z_)
			Axis = PxArticulationAxis::eSWING2;

/*		const float MinLimit = bc.mMinLimit;
		const float MaxLimit = bc.mMaxLimit;

		if(MinLimit>MaxLimit)
			j->setMotion(Axis, PxArticulationMotion::eFREE);
		if(MinLimit==MaxLimit)
			j->setMotion(Axis, PxArticulationMotion::eLOCKED);
		if(MinLimit<MaxLimit)
		{
			j->setMotion(Axis, PxArticulationMotion::eLIMITED);
			j->setLimit(Axis, MinLimit, MaxLimit);
		}*/
		SetModeAndLimits(params, j, Axis, bc.mMinLimit, bc.mMaxLimit);

		if(bc.mUseMotor)
		{
#ifdef PHYSX_SUPPORT_OLD_4_0_API
			j->setDrive(Axis, bc.mMotor.mStiffness, bc.mMotor.mDamping, bc.mMotor.mMaxForce, bc.mMotor.mAccelerationDrive);
#else
			j->setDrive(Axis, bc.mMotor.mStiffness, bc.mMotor.mDamping, bc.mMotor.mMaxForce, bc.mMotor.mAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE);
#endif
			j->setDriveVelocity(Axis, bc.mTargetVel);
//			j->userData = 0;

/*					ASSERT(!j->userData);
					MotorData* MD = ICE_NEW(MotorData);
					j->userData = MD;
					// We store the ptr in a redundant array just to be able to conveniently release the objects when closing.
					mMotorData.push_back(MD);*/

		}


/*		virtual void setDrive(PxArticulationAxis::Enum axis, const PxReal stiffness, const PxReal damping, const PxReal maxForce, bool isAccelerationDrive = false) = 0;
		virtual void getDrive(PxArticulationAxis::Enum axis, PxReal& stiffness, PxReal& damping, PxReal& maxForce, bool& isAcceleration) = 0;
		virtual void setDriveTarget(PxArticulationAxis::Enum axis, const PxReal target) = 0;
		virtual void setDriveVelocity(PxArticulationAxis::Enum axis, const PxReal targetVel) = 0;*/

//	PxD6JointDrive(PxReal driveStiffness, PxReal driveDamping, PxReal driveForceLimit, bool isAcceleration = false)


//		j->setDrive(PxArticulationAxis::eTWIST, 0.0f, PX_MAX_F32, PX_MAX_F32, false);
//		j->setDriveVelocity(PxArticulationAxis::eTWIST, 1.0f);

	}
	else
		ASSERT(0);

	j->setFrictionCoefficient(bc.mFrictionCoeff);
	j->setMaxJointVelocity(bc.mMaxJointVelocity);

/*	j->setSwingLimitEnabled(bc.mEnableSwingLimit);
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
	}*/
}

PintActorHandle SharedPhysX::CreateRCArticulatedObject(const PINT_OBJECT_CREATE& oc, const PINT_RC_ARTICULATED_BODY_CREATE& bc, PintArticHandle articulation)
{
	PxArticulationReducedCoordinate* Articulation = reinterpret_cast<PxArticulationReducedCoordinate*>(articulation);

	// Note that this already creates the joint between the objects!
	const PintActorHandle h = CreateRCArticulationLink(Articulation, (PxArticulationLink*)bc.mParent, *this, oc);

	// ...so we setup the joint data immediately
	PxArticulationJointReducedCoordinate* joint = static_cast<PxArticulationJointReducedCoordinate*>(((PxArticulationLink*)h)->getInboundJoint());
	if(joint)	// Will be null for root link
	{
		joint->setParentPose(ToPxTransform(bc.mLocalPivot0));
		joint->setChildPose(ToPxTransform(bc.mLocalPivot1));
		::setupJoint(mParams, joint, bc);
	}
	return h;
}

bool SharedPhysX::SetRCADriveEnabled(PintActorHandle handle, bool flag)
{
	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(!RigidActor || RigidActor->getConcreteType()!=PxConcreteType::eARTICULATION_LINK)
		return false;

	PxArticulationLink* Link = static_cast<PxArticulationLink*>(RigidActor);

	PxArticulationJointReducedCoordinate* j = static_cast<PxArticulationJointReducedCoordinate*>(Link->getInboundJoint());

	// TODO: articulation joints now have user-data (in 5.0), so use them
//	j->userData = null;

	// Ok this is clumsy: articulation joints don't have userData! So we could have stored the active axis there but instead
	// we need to figure it out at runtime here (or we could have used a hashmap but that's kind of heavy for nothing)
	PxArticulationAxis::Enum Axis = PxArticulationAxis::eCOUNT;
	{
		if(j->getJointType()!=PxArticulationJointType::eREVOLUTE)
		{
			ASSERT(0);	// Not implemented yet
			return false;
		}

		if(j->getMotion(PxArticulationAxis::eTWIST)!=PxArticulationMotion::eLOCKED)
			Axis = PxArticulationAxis::eTWIST;
		else if(j->getMotion(PxArticulationAxis::eSWING1)!=PxArticulationMotion::eLOCKED)
			Axis = PxArticulationAxis::eSWING1;
		else if(j->getMotion(PxArticulationAxis::eSWING2)!=PxArticulationMotion::eLOCKED)
			Axis = PxArticulationAxis::eSWING2;
		else
			return false;	// All 3 axes locked!
	}

	// TODO: this is so clumsy, can we have a flag please?
	PxReal stiffness;
	PxReal damping;
	PxReal maxForce;
#ifdef PHYSX_SUPPORT_OLD_4_0_API
	bool isAcceleration;
	j->getDrive(Axis, stiffness, damping, maxForce, isAcceleration);
#else
	PxArticulationDriveType::Enum driveType;
	j->getDrive(Axis, stiffness, damping, maxForce, driveType);
#endif
	// TODO: hardcoded because gaaaah no userData
	if(flag)
#ifdef PHYSX_SUPPORT_OLD_4_0_API
		j->setDrive(Axis, 0.0f, 1000.0f, FLT_MAX, false);
#else
		j->setDrive(Axis, 0.0f, 1000.0f, FLT_MAX, PxArticulationDriveType::eFORCE);
#endif
	else
#ifdef PHYSX_SUPPORT_OLD_4_0_API
		j->setDrive(Axis, 0.0f, 0.0f, maxForce, isAcceleration);
#else
		j->setDrive(Axis, 0.0f, 0.0f, maxForce, driveType);
#endif

//	j->setDriveVelocity(Axis, velocity);
	return true;
}

bool SharedPhysX::SetRCADriveVelocity(PintActorHandle handle, float velocity)
{
	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(!RigidActor || RigidActor->getConcreteType()!=PxConcreteType::eARTICULATION_LINK)
		return false;

	PxArticulationLink* Link = static_cast<PxArticulationLink*>(RigidActor);

	PxArticulationJointReducedCoordinate* j = static_cast<PxArticulationJointReducedCoordinate*>(Link->getInboundJoint());
//	PxArticulationJoint* j = static_cast<PxArticulationJoint*>(Link->getInboundJoint());

//	Joint->userData = 0;
//	Link->userData = 0;
//	joint->

	// Ok this is clumsy: articulation joints don't have userData! So we could have stored the active axis there but instead
	// we need to figure it out at runtime here (or we could have used a hashmap but that's kind of heavy for nothing)
	PxArticulationAxis::Enum Axis = PxArticulationAxis::eCOUNT;
	{
		if(j->getJointType()!=PxArticulationJointType::eREVOLUTE)
		{
			ASSERT(0);	// Not implemented yet
			return false;
		}

		if(j->getMotion(PxArticulationAxis::eTWIST)!=PxArticulationMotion::eLOCKED)
			Axis = PxArticulationAxis::eTWIST;
		else if(j->getMotion(PxArticulationAxis::eSWING1)!=PxArticulationMotion::eLOCKED)
			Axis = PxArticulationAxis::eSWING1;
		else if(j->getMotion(PxArticulationAxis::eSWING2)!=PxArticulationMotion::eLOCKED)
			Axis = PxArticulationAxis::eSWING2;
		else
			return false;	// All 3 axes locked!
	}

//	j->setDrive(Axis, bc.mMotor.mStiffness, bc.mMotor.mDamping, bc.mMotor.mMaxForce, bc.mMotor.mAccelerationDrive);
	j->setDriveVelocity(Axis, velocity);
	return true;
}

// TODO: refactor with CreateJoint
PintActorHandle SharedPhysX::CreateRCArticulationLink(PxArticulationReducedCoordinate* articulation, PxArticulationLink* parent, Pint& pint, const PINT_OBJECT_CREATE& desc)
{
	// It's actually legal to create actors without shapes
//	udword NbShapes = desc.GetNbShapes();
//	if(!NbShapes)
//		return null;

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

	SetActorName(actor, desc.mName);

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
