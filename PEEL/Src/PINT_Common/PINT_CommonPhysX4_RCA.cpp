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
	PxAggregate* Aggregate = reinterpret_cast<PxAggregate*>(aggregate);
	bool status = Aggregate->addArticulation(*Articulation);
	if(status)
	{
		// AddAggregateToScene already adds links to the actor manager and setup sleeping for rigid bodies.
		// We only have to setup sleeping for the articulation itself here.
		//SetupSleeping(Articulation, mParams.mEnableSleeping);
		//Articulation->wakeUp();	// ### crash
		if(!mParams.mEnableSleeping)
			Articulation->setWakeCounter(9999999999.0f);
	}
	return status;
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
#if PHYSX_SUPPORT_RCA_NEW_LIMIT_API
		j->setLimitParams(axis, PxArticulationLimit(min_limit, max_limit));
#else
		j->setLimit(axis, min_limit, max_limit);
#endif
	}

#if PHYSX_SUPPORT_RCA_DOF_SCALE
	j->setDofScale(axis, params.mRCADofScale);
#endif
#if PHYSX_SUPPORT_RCA_ARMATURE
	j->setArmature(axis, params.mRCAArmature);
#endif
}

/*
PxArticulationJointReducedCoordinate API:
		virtual		PxArticulationLink&	getParentArticulationLink() const = 0;
		virtual		void				setParentPose(const PxTransform& pose) = 0;
		virtual		PxTransform			getParentPose() const = 0;
		virtual		PxArticulationLink&	getChildArticulationLink() const = 0;
		virtual		void				setChildPose(const PxTransform& pose) = 0;
		virtual		PxTransform			getChildPose() const = 0;
		virtual		void				setJointType(PxArticulationJointType::Enum jointType) = 0;
		virtual		PxArticulationJointType::Enum	getJointType() const = 0;
		virtual		void				setMotion(PxArticulationAxis::Enum axis, PxArticulationMotion::Enum motion) = 0;
		virtual		PxArticulationMotion::Enum	getMotion(PxArticulationAxis::Enum axis) const = 0;
		virtual		void				setLimitParams(PxArticulationAxis::Enum axis, const PxArticulationLimit& limit) = 0;
		virtual		PxArticulationLimit	getLimitParams(PxArticulationAxis::Enum axis) const = 0;
		virtual		void				setDriveParams(PxArticulationAxis::Enum axis, const PxArticulationDrive& drive) = 0;
		virtual		PxArticulationDrive	getDriveParams(PxArticulationAxis::Enum axis) const = 0;	
		virtual		void				setDriveTarget(PxArticulationAxis::Enum axis, const PxReal target, bool autowake = true) = 0;
		virtual		PxReal				getDriveTarget(PxArticulationAxis::Enum axis) const = 0;	
		virtual		void				setDriveVelocity(PxArticulationAxis::Enum axis, const PxReal targetVel, bool autowake = true) = 0;
		virtual		PxReal				getDriveVelocity(PxArticulationAxis::Enum axis) const = 0;	
		virtual		void				setArmature(PxArticulationAxis::Enum axis, const PxReal armature) = 0;
		virtual		PxReal				getArmature(PxArticulationAxis::Enum axis) const = 0;
		virtual	PX_DEPRECATED	void	setFrictionCoefficient(const PxReal coefficient) = 0;
		virtual	PX_DEPRECATED	PxReal	getFrictionCoefficient() const = 0;
		virtual void setFrictionParams(PxArticulationAxis::Enum axis, const PxJointFrictionParams& jointFrictionParams) = 0;
		virtual PxJointFrictionParams getFrictionParams(PxArticulationAxis::Enum axis) const = 0;
		virtual	 PX_DEPRECATED	void				setMaxJointVelocity(const PxReal maxJointV) = 0;
		virtual	 PX_DEPRECATED	PxReal				getMaxJointVelocity() const = 0;
		virtual		void				setMaxJointVelocity(PxArticulationAxis::Enum axis, const PxReal maxJointV) = 0;
		virtual		PxReal				getMaxJointVelocity(PxArticulationAxis::Enum axis) const = 0;
		virtual		void				setJointPosition(PxArticulationAxis::Enum axis, const PxReal jointPos) = 0;
		virtual		PxReal				getJointPosition(PxArticulationAxis::Enum axis) const = 0;
		virtual		void				setJointVelocity(PxArticulationAxis::Enum axis, const PxReal jointVel) = 0;
		virtual		PxReal				getJointVelocity(PxArticulationAxis::Enum axis) const = 0;
*/

static void SetupDrive(PxArticulationJointReducedCoordinate* j, const MotorData& data)
{
	// Unfortunately this part of the API keeps changing :(
#ifdef PHYSX_SUPPORT_OLD_4_0_API
	j->setDrive(data.mAxis, data.mStiffness, data.mDamping, data.mMaxForce, data.mAccelerationDrive);
#else
	#if PHYSX_SUPPORT_RCA_NEW_LIMIT_API
	j->setDriveParams(data.mAxis, PxArticulationDrive(data.mStiffness, data.mDamping, data.mMaxForce, data.mAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE));
	#else
	j->setDrive(data.mAxis, data.mStiffness, data.mDamping, data.mMaxForce, data.mAccelerationDrive ? PxArticulationDriveType::eACCELERATION : PxArticulationDriveType::eFORCE);
	#endif
#endif
}

void SharedPhysX::SetupRCAJoint(const EditableParams& params, PxArticulationJointReducedCoordinate* j, const PINT_RC_ARTICULATED_BODY_CREATE& bc)
{
	if(bc.mJointType==PINT_JOINT_SPHERICAL)
	{
		j->setJointType(PxArticulationJointType::eSPHERICAL);
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

		SetModeAndLimits(params, j, Axis, bc.mMinLimit, bc.mMaxLimit);
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

		SetModeAndLimits(params, j, Axis, bc.mMinLimit, bc.mMaxLimit);

		if(bc.mMotorFlags != PINT_MOTOR_NONE)
		{
			// We need to save the motor parameters just to be able to implement SetRCADriveEnabled, as there is bo enable/disable flag to simply disable the drive.
			MotorData* MD = ICE_NEW(MotorData);
			MD->mAxis				= Axis;
			MD->mStiffness			= bc.mMotor.mStiffness;
			MD->mDamping			= bc.mMotor.mDamping;
			MD->mMaxForce			= bc.mMotor.mMaxForce;
			MD->mAccelerationDrive	= bc.mMotor.mAccelerationDrive;
			mMotorData.push_back(MD);
#ifdef PHYSX_NO_USERDATA_RCA_JOINT
			if(!mJointRCA_UserData)
				mJointRCA_UserData = new PxHashMap<PxArticulationJointReducedCoordinate*, const MotorData*>;
			mJointRCA_UserData->insert(j, MD);
#else
			ASSERT(!j->userData);
			j->userData = MD;
#endif
			SetupDrive(j, *MD);

			j->setDriveVelocity(Axis, bc.mTargetVel);
			j->setDriveTarget(Axis, bc.mTargetPos);
		}
	}
	else
		ASSERT(0);

	j->setFrictionCoefficient(bc.mFrictionCoeff);
	j->setMaxJointVelocity(bc.mMaxJointVelocity);
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
#ifdef PHYSX_NO_USERDATA_RCA_JOINT
#else
		joint->userData = null;
#endif
		joint->setParentPose(ToPxTransform(bc.mLocalPivot0));
		joint->setChildPose(ToPxTransform(bc.mLocalPivot1));
		SetupRCAJoint(mParams, joint, bc);
	}
	return h;
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

PxArticulationJointReducedCoordinate* SharedPhysX::RetrieveJointData(PintActorHandle handle, MotorData& data)	const
{
	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(!RigidActor || RigidActor->getConcreteType()!=PxConcreteType::eARTICULATION_LINK)
		return null;

	PxArticulationLink* Link = static_cast<PxArticulationLink*>(RigidActor);

	PxArticulationJointReducedCoordinate* j = static_cast<PxArticulationJointReducedCoordinate*>(Link->getInboundJoint());
	if(!j)
		return null;

#ifdef PHYSX_NO_USERDATA_RCA_JOINT
	const MotorData* MD = mJointRCA_UserData ? mJointRCA_UserData->find(j)->second : null;
#else
	const MotorData* MD = reinterpret_cast<const MotorData*>(j->userData);
#endif
	if(!MD)
		return null;

	data = *MD;

	return j;
}

bool SharedPhysX::SetRCADriveEnabled(PintActorHandle handle, bool flag)
{
	MotorData MD;
	PxArticulationJointReducedCoordinate* j = RetrieveJointData(handle, MD);
	if(!j)
		return false;

	if(!flag)
		MD.mStiffness = MD.mDamping = 0.0f;

	SetupDrive(j, MD);

//	j->setDriveVelocity(Axis, velocity);
	return true;
}

bool SharedPhysX::SetRCADriveVelocity(PintActorHandle handle, float velocity)
{
	MotorData MD;
	PxArticulationJointReducedCoordinate* j = RetrieveJointData(handle, MD);
	if(!j)
		return false;

	j->setDriveVelocity(MD.mAxis, velocity);
	return true;
}

bool SharedPhysX::SetRCADrivePosition(PintActorHandle handle, float position)
{
	MotorData MD;
	PxArticulationJointReducedCoordinate* j = RetrieveJointData(handle, MD);
	if(!j)
		return false;

	j->setDriveTarget(MD.mAxis, position);
	return true;
}
