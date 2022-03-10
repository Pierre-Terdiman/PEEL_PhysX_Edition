///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Camera.h"
#include "Cylinder.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "Loader_Bin.h"
#include "ProceduralTrack.h"
#include "MyConvex.h"
#include "GUI_Helpers.h"
#include "GLFontRenderer.h"

///////////////////////////////////////////////////////////////////////////////

/*static Point Transform(Pint& pint, PintActorHandle actor, const Point& p)
{
	//actor->getGlobalPose().transform(p)
	// TODO: optimize / simplify this
	const PR pr = pint.GetWorldTransform(actor);
	Matrix4x4 M = pr;
	return p * M;
}*/

static Point TransformInv(Pint& pint, PintActorHandle actor, const Point& p)
{
	//actor->getGlobalPose().transformInv(p)
	// TODO: optimize / simplify this
	const PR pr = pint.GetWorldTransform(actor);
	Matrix4x4 M = pr;
	Matrix4x4 InvM;
	InvertPRMatrix(InvM, M);
	return p * InvM;
}

static Quat GetConjugateQ(Pint& pint, PintActorHandle actor)
{
	// TODO: optimize / simplify this
	const PR pr = pint.GetWorldTransform(actor);
	return pr.mRot.GetConjugate();
}

static const char* gDesc_ScissorLift = "Scissor lift. Adapted from the PhysX reduced coordinates articulation snippet.";

START_TEST(ScissorLift, CATEGORY_RCARTICULATIONS, gDesc_ScissorLift)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(3.96f, 2.64f, 3.98f), Point(-0.69f, -0.32f, -0.65f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportCollisionGroups || !caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		// Group 0 is already used by the default static environment so we start with group 1
		const PintCollisionGroup Layer0_Group = 1;
		const PintDisabledGroups DG(Layer0_Group, Layer0_Group);
		pint.SetDisabledGroups(1, &DG);

		PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE());
//		gArticulation->setSolverIterationCounts(32);

		const bool SetupLimits = true;

		const float runnerLength = 2.0f;
		const float placementDistance = 1.8f;

		const float cosAng = (placementDistance) / (runnerLength);

		const float angle = SafeACos(cosAng);

		const float sinAng = sinf(angle);

		const Quat leftRot(AngleAxis(-angle, Point(1.0f, 0.0f, 0.0f)));
		const Quat rightRot(AngleAxis(angle, Point(1.0f, 0.0f, 0.0f)));

		const Point BasePos(0.0f, 0.25f, 0.0f);

		//(1) Create base...
//		PxArticulationLink* base = gArticulation->createLink(NULL, PxTransform(PxVec3(0.f, 0.25f, 0.f)));
//		PxRigidActorExt::createExclusiveShape(*base, PxBoxGeometry(0.5f, 0.25f, 1.5f), *gMaterial);
//		PxRigidBodyExt::updateMassAndInertia(*base, 3.f);
		PintActorHandle base;
		{
			const Point Extents(0.5f, 0.25f, 1.5f);
//			const Point Extents(0.25f, 0.25f, 1.5f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 3.0f;
			ObjectDesc.mPosition	= BasePos;

			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			base = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
		}

		if(0)
		{
			const float WheelRadius = 0.2f;
			PintShapeRenderer* Renderer = CreateSphereRenderer(WheelRadius);

			for(udword i=0;i<4;i++)
			{
				Point Delta;
				Delta.x = i&1 ? -0.5f : 0.5f;
				Delta.y = -0.25f;
				Delta.z = i&2 ? -1.5f : 1.5f;

				PINT_SPHERE_CREATE SphereDesc(WheelRadius);
				SphereDesc.mRenderer	= Renderer;

				PINT_OBJECT_CREATE ObjectDesc(&SphereDesc);
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mPosition	= BasePos + Delta;

				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType			= PINT_JOINT_HINGE;
				ArticulatedDesc.mJointType			= PINT_JOINT_SPHERICAL;
				ArticulatedDesc.mParent				= base;
				ArticulatedDesc.mLocalPivot0.mPos	= Delta;
				ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, 0.0f, 0.f);
//				ArticulatedDesc.mAxisIndex			= X_;
				ArticulatedDesc.mFrictionCoeff		= 0.0f;
/*
				ArticulatedDesc.mMinTwistLimit		(1.0f),
				ArticulatedDesc.mMaxTwistLimit		(-1.0f),
				ArticulatedDesc.mMinSwing1Limit		(1.0f),
				ArticulatedDesc.mMaxSwing1Limit		(-1.0f),
				ArticulatedDesc.mMinSwing2Limit		(1.0f),
				ArticulatedDesc.mMaxSwing2Limit		(-1.0f),
				ArticulatedDesc.mFrictionCoeff		(0.5f),
*/
				PintActorHandle Wheel = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				(void)Wheel;
			}

		}



		//Now create the slider and fixed joints...

//		PxArticulationLink* leftRoot = gArticulation->createLink(base, PxTransform(PxVec3(0.f, 0.55f, -0.9f)));
//		PxRigidActorExt::createExclusiveShape(*leftRoot, PxBoxGeometry(0.5f, 0.05f, 0.05f), *gMaterial);
//		PxRigidBodyExt::updateMassAndInertia(*leftRoot, 1.f);
//
//		PxArticulationJointReducedCoordinate* joint = static_cast<PxArticulationJointReducedCoordinate*>(leftRoot->getInboundJoint());
//		joint->setJointType(PxArticulationJointType::eFIX);
//		joint->setParentPose(PxTransform(PxVec3(0.f, 0.25f, -0.9f)));
//		joint->setChildPose(PxTransform(PxVec3(0.f, -0.05f, 0.f)));

		PintActorHandle leftRoot;
		{
			const Point Extents(0.5f, 0.05f, 0.05f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.f, 0.55f, -0.9f);

			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mJointType			= PINT_JOINT_FIXED;
			ArticulatedDesc.mParent				= base;
			ArticulatedDesc.mLocalPivot0.mPos	= Point(0.f, 0.25f, -0.9f);
			ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, -0.05f, 0.f);
			leftRoot = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
		}

//		PxArticulationLink* rightRoot = gArticulation->createLink(base, PxTransform(PxVec3(0.f, 0.55f, 0.9f)));
//		PxRigidActorExt::createExclusiveShape(*rightRoot, PxBoxGeometry(0.5f, 0.05f, 0.05f), *gMaterial);
//		PxRigidBodyExt::updateMassAndInertia(*rightRoot, 1.f);
		//Set up the drive joint...	
//		gDriveJoint = static_cast<PxArticulationJointReducedCoordinate*>(rightRoot->getInboundJoint());
//		gDriveJoint->setJointType(PxArticulationJointType::ePRISMATIC);
//		gDriveJoint->setMotion(PxArticulationAxis::eZ, PxArticulationMotion::eLIMITED);
//		gDriveJoint->setLimit(PxArticulationAxis::eZ, -1.4f, 0.2f);
//		gDriveJoint->setDrive(PxArticulationAxis::eZ, 100000.f, 0.f, PX_MAX_F32);

//		gDriveJoint->setParentPose(PxTransform(PxVec3(0.f, 0.25f, 0.9f)));
//		gDriveJoint->setChildPose(PxTransform(PxVec3(0.f, -0.05f, 0.f)));

		PintActorHandle rightRoot;
		{
			const Point Extents(0.5f, 0.05f, 0.05f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.f, 0.55f, 0.9f);

			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mJointType			= PINT_JOINT_PRISMATIC;
			ArticulatedDesc.mAxisIndex			= Z_;
			ArticulatedDesc.mMinLimit			= -1.4f;
			ArticulatedDesc.mMaxLimit			= 0.2f;
			ArticulatedDesc.mParent				= base;
			ArticulatedDesc.mLocalPivot0.mPos	= Point(0.f, 0.25f, 0.9f);
			ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, -0.05f, 0.f);
			rightRoot = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
		}

		//

//		const udword linkHeight = 1;
		const udword linkHeight = 3;
//		const udword linkHeight = 7;
//		PxArticulationLink* currLeft = leftRoot, *currRight = rightRoot;
		PintActorHandle currLeft = leftRoot;
		PintActorHandle currRight = rightRoot;
		Quat rightParentRot;	rightParentRot.Identity();
		Quat leftParentRot;		leftParentRot.Identity();

		for(udword i=0; i<linkHeight; ++i)
		{
			const Point pos(0.5f, 0.55f + 0.1f*(1 + i), 0.f);

//			PxArticulationLink* leftLink = gArticulation->createLink(currLeft, PxTransform(pos + PxVec3(0.f, sinAng*(2 * i + 1), 0.f), leftRot));
//			PxRigidActorExt::createExclusiveShape(*leftLink, PxBoxGeometry(0.05f, 0.05f, 1.f), *gMaterial);
//			PxRigidBodyExt::updateMassAndInertia(*leftLink, 1.f);

			const Point leftAnchorLocation = pos + Point(0.f, sinAng*(2 * i), -0.9f);

//			joint = static_cast<PxArticulationJointReducedCoordinate*>(leftLink->getInboundJoint());
//			joint->setParentPose(PxTransform(currLeft->getGlobalPose().transformInv(leftAnchorLocation), leftParentRot));
//			joint->setChildPose(PxTransform(PxVec3(0.f, 0.f, -1.f), rightRot));
//			joint->setJointType(PxArticulationJointType::eREVOLUTE);
//			joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eLIMITED);
//			joint->setLimit(PxArticulationAxis::eTWIST, -PxPi, angle);

			PintActorHandle leftLink;
			{
				const Point Extents(0.05f, 0.05f, 1.f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= pos + Point(0.f, sinAng*(2 * i + 1), 0.f);
				ObjectDesc.mRotation		= leftRot;
				ObjectDesc.mCollisionGroup	= Layer0_Group;

				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType	= PINT_JOINT_HINGE;
				ArticulatedDesc.mAxisIndex	= X_;
				if(SetupLimits)
				{
					ArticulatedDesc.mMinLimit	= -PI;
					ArticulatedDesc.mMaxLimit	= angle;
				}
				else
				{
					ArticulatedDesc.mMinLimit	= -PI;
					ArticulatedDesc.mMaxLimit	= PI;
				}
				ArticulatedDesc.mParent		= currLeft;

				ArticulatedDesc.mLocalPivot0.mPos	= TransformInv(pint, currLeft, leftAnchorLocation);
				ArticulatedDesc.mLocalPivot0.mRot	= leftParentRot;
				ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, 0.f, -1.f);
				ArticulatedDesc.mLocalPivot1.mRot	= rightRot;
				leftLink = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}

			leftParentRot = leftRot;

//			PxArticulationLink* rightLink = gArticulation->createLink(currRight, PxTransform(pos + PxVec3(0.f, sinAng*(2 * i + 1), 0.f), rightRot));
//			PxRigidActorExt::createExclusiveShape(*rightLink, PxBoxGeometry(0.05f, 0.05f, 1.f), *gMaterial);
//			PxRigidBodyExt::updateMassAndInertia(*rightLink, 1.f);

			const Point rightAnchorLocation = pos + Point(0.f, sinAng*(2 * i), 0.9f);

//			joint = static_cast<PxArticulationJointReducedCoordinate*>(rightLink->getInboundJoint());
//			joint->setJointType(PxArticulationJointType::eREVOLUTE);
//			joint->setParentPose(PxTransform(currRight->getGlobalPose().transformInv(rightAnchorLocation), rightParentRot));
//			joint->setChildPose(PxTransform(PxVec3(0.f, 0.f, 1.f), leftRot));
//			joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eLIMITED);
//			joint->setLimit(PxArticulationAxis::eTWIST, -angle, PxPi);

			PintActorHandle rightLink;
			{
				const Point Extents(0.05f, 0.05f, 1.f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= pos + Point(0.f, sinAng*(2 * i + 1), 0.f);
				ObjectDesc.mRotation		= rightRot;
				ObjectDesc.mCollisionGroup	= Layer0_Group;

				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType	= PINT_JOINT_HINGE;
				ArticulatedDesc.mAxisIndex	= X_;
				if(SetupLimits)
				{
					ArticulatedDesc.mMinLimit	= -angle;
					ArticulatedDesc.mMaxLimit	= PI;
				}
				else
				{
					ArticulatedDesc.mMinLimit	= -PI;
					ArticulatedDesc.mMaxLimit	= PI;
				}
				ArticulatedDesc.mParent		= currRight;

				ArticulatedDesc.mLocalPivot0.mPos	= TransformInv(pint, currRight, rightAnchorLocation);
				ArticulatedDesc.mLocalPivot0.mRot	= rightParentRot;
				ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, 0.f, 1.f);
				ArticulatedDesc.mLocalPivot1.mRot	= leftRot;
				rightLink = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}

			rightParentRot = rightRot;

			{
//			PxD6Joint* d6joint = PxD6JointCreate(*gPhysics, leftLink, PxTransform(PxIdentity), rightLink, PxTransform(PxIdentity));
//			d6joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
//			d6joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
//			d6joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
				PINT_SPHERICAL_JOINT_CREATE Desc;
				Desc.mObject0	= leftLink;
				Desc.mObject1	= rightLink;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}

			currLeft = rightLink;
			currRight = leftLink;
		}
	
#ifdef TOSEE
		PxArticulationLink* leftTop = gArticulation->createLink(currLeft, currLeft->getGlobalPose().transform(PxTransform(PxVec3(-0.5f, 0.f, -1.0f), leftParentRot)));
		PxRigidActorExt::createExclusiveShape(*leftTop, PxBoxGeometry(0.5f, 0.05f, 0.05f), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*leftTop, 1.f);

		joint = static_cast<PxArticulationJointReducedCoordinate*>(leftTop->getInboundJoint());
		joint->setParentPose(PxTransform(PxVec3(0.f, 0.f, -1.f), currLeft->getGlobalPose().q.getConjugate()));
		joint->setChildPose(PxTransform(PxVec3(0.5f, 0.f, 0.f), leftTop->getGlobalPose().q.getConjugate()));
		joint->setJointType(PxArticulationJointType::eREVOLUTE);
		joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eFREE);
		//joint->setDrive(PxArticulationAxis::eTWIST, 0.f, 10.f, PX_MAX_F32);
#endif

/*
-		combo	{q={x=0.000000000 y=0.000000000 z=0.000000000 ...} p={x=-1.78813934e-007 y=3.46533942 z=-0.900000334 } }	physx::PxTransform
-		q	{x=0.000000000 y=0.000000000 z=0.000000000 ...}	physx::PxQuat
		x	0.000000000	float
		y	0.000000000	float
		z	0.000000000	float
		w	1.00000012	float
-		p	{x=-1.78813934e-007 y=3.46533942 z=-0.900000334 }	physx::PxVec3
		x	-1.78813934e-007	float
		y	3.46533942	float
		z	-0.900000334	float

-		qq	{x=-0.000000000 y=-0.000000000 z=-0.000000000 ...}	physx::PxQuat
		x	-0.000000000	float
		y	-0.000000000	float
		z	-0.000000000	float
		w	1.00000000	float
*/

		PintActorHandle leftTop;
		{
			const Point Extents(0.5f, 0.05f, 0.05f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

			PR tmp0 = pint.GetWorldTransform(currLeft);
			PR tmp1(Point(-0.5f, 0.f, -1.0f), leftParentRot);
			Matrix4x4 M0 = tmp0;
			Matrix4x4 M1 = tmp1;
			Matrix4x4 M = M1 * M0;
			PR combo = M;

			PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
			ObjectDesc.mMass			= 1.0f;
//			ObjectDesc.mPosition		= Transform(pint, currLeft, Point(-0.5f, 0.f, -1.0f));
//			ObjectDesc.mRotation		= leftParentRot;
			ObjectDesc.mPosition		= combo.mPos;
			ObjectDesc.mRotation		= combo.mRot;
			ObjectDesc.mCollisionGroup	= Layer0_Group;

			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mJointType	= PINT_JOINT_HINGE;
			ArticulatedDesc.mAxisIndex	= X_;
			ArticulatedDesc.mMinLimit	= -PI;
			ArticulatedDesc.mMaxLimit	= PI;
			ArticulatedDesc.mParent		= currLeft;

			ArticulatedDesc.mLocalPivot0.mPos	= Point(0.f, 0.f, -1.f);
			ArticulatedDesc.mLocalPivot0.mRot	= GetConjugateQ(pint, currLeft);
			ArticulatedDesc.mLocalPivot1.mPos	= Point(0.5f, 0.f, 0.f);
			ArticulatedDesc.mLocalPivot1.mRot	= combo.mRot.GetConjugate();//GetConjugateQ(pint, leftTop);
			leftTop = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
		}

#ifdef TOSEE
		PxArticulationLink* rightTop = gArticulation->createLink(currRight, currRight->getGlobalPose().transform(PxTransform(PxVec3(-0.5f, 0.f, 1.0f), rightParentRot)));
		PxRigidActorExt::createExclusiveShape(*rightTop, PxCapsuleGeometry(0.05f, 0.8f), *gMaterial);
		//PxRigidActorExt::createExclusiveShape(*rightTop, PxBoxGeometry(0.5f, 0.05f, 0.05f), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*rightTop, 1.f);

		joint = static_cast<PxArticulationJointReducedCoordinate*>(rightTop->getInboundJoint());
		joint->setParentPose(PxTransform(PxVec3(0.f, 0.f, 1.f), currRight->getGlobalPose().q.getConjugate()));
		joint->setChildPose(PxTransform(PxVec3(0.5f, 0.f, 0.f), rightTop->getGlobalPose().q.getConjugate()));
		joint->setJointType(PxArticulationJointType::eREVOLUTE);
		joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eFREE);
		//joint->setDrive(PxArticulationAxis::eTWIST, 0.f, 10.f, PX_MAX_F32);
#endif

//#define FOR_IMM_MODE
		PintActorHandle rightTop;
		{
#ifdef FOR_IMM_MODE
			const Point Extents(0.5f, 0.05f, 0.05f);
			PINT_BOX_CREATE BoxDesc(Extents);
			BoxDesc.mRenderer	= CreateBoxRenderer(Extents);
#else
//			PINT_CAPSULE_CREATE CapsuleDesc(0.05f, 0.8f);
			PINT_CAPSULE_CREATE CapsuleDesc(0.05f, 0.5f);
			CapsuleDesc.mRenderer	= CreateCapsuleRenderer(CapsuleDesc.mRadius, CapsuleDesc.mHalfHeight*2.0f);
			Matrix3x3 CapsuleRot;
//			CapsuleRot.RotX(90.0f * DEGTORAD);
//			CapsuleRot.RotY(90.0f * DEGTORAD);
			CapsuleRot.RotZ(90.0f * DEGTORAD);
			CapsuleDesc.mLocalRot = CapsuleRot;
#endif
			PR tmp0 = pint.GetWorldTransform(currRight);
			PR tmp1(Point(-0.5f, 0.f, 1.0f), rightParentRot);
			Matrix4x4 M0 = tmp0;
			Matrix4x4 M1 = tmp1;
			Matrix4x4 M = M1 * M0;
			PR combo = M;

			PINT_OBJECT_CREATE ObjectDesc;
#ifdef FOR_IMM_MODE
			ObjectDesc.SetShape(&BoxDesc);
#else
			ObjectDesc.SetShape(&CapsuleDesc);
#endif
			ObjectDesc.mMass			= 1.0f;
//			ObjectDesc.mPosition		= Transform(pint, currLeft, Point(-0.5f, 0.f, -1.0f));
//			ObjectDesc.mRotation		= leftParentRot;
			ObjectDesc.mPosition		= combo.mPos;
			ObjectDesc.mRotation		= combo.mRot;
			ObjectDesc.mCollisionGroup	= Layer0_Group;

			PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mJointType	= PINT_JOINT_HINGE;
			ArticulatedDesc.mAxisIndex	= X_;
			ArticulatedDesc.mMinLimit	= -PI;
			ArticulatedDesc.mMaxLimit	= PI;
			ArticulatedDesc.mParent		= currRight;

//		joint->setParentPose(PxTransform(PxVec3(0.f, 0.f, 1.f), currRight->getGlobalPose().q.getConjugate()));
//		joint->setChildPose(PxTransform(PxVec3(0.5f, 0.f, 0.f), rightTop->getGlobalPose().q.getConjugate()));

			ArticulatedDesc.mLocalPivot0.mPos	= Point(0.f, 0.f, 1.f);
			ArticulatedDesc.mLocalPivot0.mRot	= GetConjugateQ(pint, currRight);
			ArticulatedDesc.mLocalPivot1.mPos	= Point(0.5f, 0.f, 0.f);
			ArticulatedDesc.mLocalPivot1.mRot	= combo.mRot.GetConjugate();//GetConjugateQ(pint, leftTop);
			rightTop = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
		}



		currLeft = leftRoot;
		currRight = rightRoot;

		rightParentRot.Identity();
		leftParentRot.Identity();

		for(udword i=0; i<linkHeight; ++i)
		{
			const Point pos(-0.5f, 0.55f + 0.1f*(1 + i), 0.f);

	//		PxArticulationLink* leftLink = gArticulation->createLink(currLeft, PxTransform(pos + PxVec3(0.f, sinAng*(2 * i + 1), 0.f), leftRot));
	//		PxRigidActorExt::createExclusiveShape(*leftLink, PxBoxGeometry(0.05f, 0.05f, 1.f), *gMaterial);
	//		PxRigidBodyExt::updateMassAndInertia(*leftLink, 1.f);

			const Point leftAnchorLocation = pos + Point(0.f, sinAng*(2 * i), -0.9f);

	//		joint = static_cast<PxArticulationJointReducedCoordinate*>(leftLink->getInboundJoint());
	//		joint->setJointType(PxArticulationJointType::eREVOLUTE);
	//		joint->setParentPose(PxTransform(currLeft->getGlobalPose().transformInv(leftAnchorLocation), leftParentRot));
	//		joint->setChildPose(PxTransform(PxVec3(0.f, 0.f, -1.f), rightRot));
	//			joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eLIMITED);
	//			joint->setLimit(PxArticulationAxis::eTWIST, -PxPi, angle);

			PintActorHandle leftLink;
			{
				const Point Extents(0.05f, 0.05f, 1.f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= pos + Point(0.f, sinAng*(2 * i + 1), 0.f);
				ObjectDesc.mRotation		= leftRot;
				ObjectDesc.mCollisionGroup	= Layer0_Group;

				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType	= PINT_JOINT_HINGE;
				ArticulatedDesc.mAxisIndex	= X_;
				if(SetupLimits)
				{
					ArticulatedDesc.mMinLimit	= -PI;
					ArticulatedDesc.mMaxLimit	= angle;
				}
				else
				{
					ArticulatedDesc.mMinLimit	= -PI;
					ArticulatedDesc.mMaxLimit	= PI;
				}
				ArticulatedDesc.mParent		= currLeft;

				ArticulatedDesc.mLocalPivot0.mPos	= TransformInv(pint, currLeft, leftAnchorLocation);
				ArticulatedDesc.mLocalPivot0.mRot	= leftParentRot;
				ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, 0.f, -1.f);
				ArticulatedDesc.mLocalPivot1.mRot	= rightRot;
				leftLink = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}

			leftParentRot = leftRot;

	//		PxArticulationLink* rightLink = gArticulation->createLink(currRight, PxTransform(pos + PxVec3(0.f, sinAng*(2 * i + 1), 0.f), rightRot));
	//		PxRigidActorExt::createExclusiveShape(*rightLink, PxBoxGeometry(0.05f, 0.05f, 1.f), *gMaterial);
	//		PxRigidBodyExt::updateMassAndInertia(*rightLink, 1.f);

			const Point rightAnchorLocation = pos + Point(0.f, sinAng*(2 * i), 0.9f);

	//		joint = static_cast<PxArticulationJointReducedCoordinate*>(rightLink->getInboundJoint());
	//		joint->setParentPose(PxTransform(currRight->getGlobalPose().transformInv(rightAnchorLocation), rightParentRot));
	//		joint->setJointType(PxArticulationJointType::eREVOLUTE);
	//		joint->setChildPose(PxTransform(PxVec3(0.f, 0.f, 1.f), leftRot));
	//		joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eLIMITED);
	//		joint->setLimit(PxArticulationAxis::eTWIST, -angle, PxPi);

			PintActorHandle rightLink;
			{
				const Point Extents(0.05f, 0.05f, 1.f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= pos + Point(0.f, sinAng*(2 * i + 1), 0.f);
				ObjectDesc.mRotation		= rightRot;
				ObjectDesc.mCollisionGroup	= Layer0_Group;

				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType	= PINT_JOINT_HINGE;
				ArticulatedDesc.mAxisIndex	= X_;
				if(SetupLimits)
				{
					ArticulatedDesc.mMinLimit	= -angle;
					ArticulatedDesc.mMaxLimit	= PI;
				}
				else
				{
					ArticulatedDesc.mMinLimit	= -PI;
					ArticulatedDesc.mMaxLimit	= PI;
				}
				ArticulatedDesc.mParent		= currRight;

				ArticulatedDesc.mLocalPivot0.mPos	= TransformInv(pint, currRight, rightAnchorLocation);
				ArticulatedDesc.mLocalPivot0.mRot	= rightParentRot;
				ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, 0.f, 1.f);
				ArticulatedDesc.mLocalPivot1.mRot	= leftRot;
				rightLink = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}

			rightParentRot = rightRot;
			{
//			PxD6Joint* d6joint = PxD6JointCreate(*gPhysics, leftLink, PxTransform(PxIdentity), rightLink, PxTransform(PxIdentity));
//			d6joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
//			d6joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
//			d6joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
				PINT_SPHERICAL_JOINT_CREATE Desc;
				Desc.mObject0	= leftLink;
				Desc.mObject1	= rightLink;
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}

			currLeft = rightLink;
			currRight = leftLink;
		}

		{
/*		PxD6Joint* d6joint = PxD6JointCreate(*gPhysics, currLeft, PxTransform(PxVec3(0.f, 0.f, -1.f)), leftTop, PxTransform(PxVec3(-0.5f, 0.f, 0.f)));
		d6joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
		d6joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
		d6joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
	*/
				PINT_SPHERICAL_JOINT_CREATE Desc;
				Desc.mObject0			= currLeft;
				Desc.mLocalPivot0.mPos	= Point(0.f, 0.f, -1.f);
				Desc.mObject1			= leftTop;
				Desc.mLocalPivot1.mPos	= Point(-0.5f, 0.f, 0.f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
		}

		// TODO: double-check pivots here
		{
				PINT_SPHERICAL_JOINT_CREATE Desc;
				Desc.mObject0			= currRight;
				Desc.mLocalPivot0.mPos	= Point(0.f, 0.f, 1.f);
				Desc.mObject1			= rightTop;
				Desc.mLocalPivot1.mPos	= Point(-0.5f, 0.f, 0.f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
		}

#ifdef TOSEE
		const PxTransform topPose(PxVec3(0.f, leftTop->getGlobalPose().p.y + 0.15f, 0.f));

		PxArticulationLink* top = gArticulation->createLink(leftTop, topPose);
		PxRigidActorExt::createExclusiveShape(*top, PxBoxGeometry(0.5f, 0.1f, 1.5f), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*top, 1.f);

		joint = static_cast<PxArticulationJointReducedCoordinate*>(top->getInboundJoint());
		joint->setJointType(PxArticulationJointType::eFIX);
		joint->setParentPose(PxTransform(PxVec3(0.f, 0.0f, 0.f)));
		joint->setChildPose(PxTransform(PxVec3(0.f, -0.15f, -0.9f)));

		gScene->addArticulation(*gArticulation);
#endif

			PintActorHandle top;
			{
				const PR topTransform = pint.GetWorldTransform(leftTop);

				const Point topPose(0.f, topTransform.mPos.y + 0.15f, 0.f);

				const Point Extents(0.5f, 0.1f, 1.5f);
				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= 1.0f;
				ObjectDesc.mPosition		= topPose;
//				ObjectDesc.mCollisionGroup	= Layer0_Group;

				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mJointType	= PINT_JOINT_FIXED;
				ArticulatedDesc.mParent		= leftTop;

				ArticulatedDesc.mLocalPivot0.mPos	= Point(0.0f, 0.0f, 0.0f);
				ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, -0.15f, -0.9f);
				top = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}

		pint.AddRCArticulationToScene(RCA);

#ifdef TOSEE
		for (PxU32 i = 0; i < gArticulation->getNbLinks(); ++i)
		{
			PxArticulationLink* link;
			gArticulation->getLinks(&link, 1, i);

			link->setLinearDamping(0.2f);
			link->setAngularDamping(0.2f);

			link->setMaxAngularVelocity(20.f);
			link->setMaxLinearVelocity(100.f);

			if (link != top)
			{
				for (PxU32 b = 0; b < link->getNbShapes(); ++b)
				{
					PxShape* shape;
					link->getShapes(&shape, 1, b);

					shape->setSimulationFilterData(PxFilterData(0, 0, 1, 0));
				}
			}
		}

		const PxVec3 halfExt(0.25f);
		const PxReal density(0.5f);

		PxRigidDynamic* box0 = gPhysics->createRigidDynamic(PxTransform(PxVec3(-0.25f, 5.f, 0.5f)));
		PxRigidActorExt::createExclusiveShape(*box0, PxBoxGeometry(halfExt), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*box0, density);

		gScene->addActor(*box0);

		PxRigidDynamic* box1 = gPhysics->createRigidDynamic(PxTransform(PxVec3(0.25f, 5.f, 0.5f)));
		PxRigidActorExt::createExclusiveShape(*box1, PxBoxGeometry(halfExt), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*box1, density);

		gScene->addActor(*box1);

		PxRigidDynamic* box2 = gPhysics->createRigidDynamic(PxTransform(PxVec3(-0.25f, 4.5f, 0.5f)));
		PxRigidActorExt::createExclusiveShape(*box2, PxBoxGeometry(halfExt), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*box2, density);

		gScene->addActor(*box2);

		PxRigidDynamic* box3 = gPhysics->createRigidDynamic(PxTransform(PxVec3(0.25f, 4.5f, 0.5f)));
		PxRigidActorExt::createExclusiveShape(*box3, PxBoxGeometry(halfExt), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*box3, density);

		gScene->addActor(*box3);

		PxRigidDynamic* box4 = gPhysics->createRigidDynamic(PxTransform(PxVec3(-0.25f, 5.f, 0.f)));
		PxRigidActorExt::createExclusiveShape(*box4, PxBoxGeometry(halfExt), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*box4, density);

		gScene->addActor(*box4);

		PxRigidDynamic* box5 = gPhysics->createRigidDynamic(PxTransform(PxVec3(0.25f, 5.f, 0.f)));
		PxRigidActorExt::createExclusiveShape(*box5, PxBoxGeometry(halfExt), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*box5, density);

		gScene->addActor(*box5);

		PxRigidDynamic* box6 = gPhysics->createRigidDynamic(PxTransform(PxVec3(-0.25f, 4.5f, 0.f)));
		PxRigidActorExt::createExclusiveShape(*box6, PxBoxGeometry(halfExt), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*box6, density);

		gScene->addActor(*box6);

		PxRigidDynamic* box7 = gPhysics->createRigidDynamic(PxTransform(PxVec3(0.25f, 4.5f, 0.f)));
		PxRigidActorExt::createExclusiveShape(*box7, PxBoxGeometry(halfExt), *gMaterial);
		PxRigidBodyExt::updateMassAndInertia(*box7, density);

		gScene->addActor(*box7);
#endif
		return true;
	}

END_TEST(ScissorLift)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_MotorizedHinges = "Motorized hinges test scene from APE.";

START_TEST(MotorizedHinges, CATEGORY_RCARTICULATIONS, gDesc_MotorizedHinges)

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(4.25f, 12.43f, 41.98f), Point(0.01f, -0.37f, -0.93f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual void	Close(Pint& pint)
	{
		PintActorHandle* UserData = (PintActorHandle*)pint.mUserData;
		ICE_FREE(UserData);
		pint.mUserData = null;

		TestBase::Close(pint);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

//const PintDisabledGroups DG(1, 1);
//pint.SetDisabledGroups(1, &DG);
//		const float Size = 50.0f;

//		for(sdword yyy=0;yyy<50;yyy++)
		{
//			for(sdword xxx=0;xxx<50;xxx++)
			{
				const PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(true));
//				const PintArticHandle RCA = 0;

				const float ex = 1.0f;
				const float ey = 1.0f;
				float ez = 1.0f;

				float x = 0.0f;
				float y = 1.0f;
				float z = 0.0f;

				// First we create a static anchor object

				PintActorHandle box0;
				{
					const Point Extents(ex, ey, ez);
//					const Point StaticPos(x+float(xxx-15)*Size, y, z+float(yyy-15)*Size);
					const Point StaticPos(x, y, z);

					PINT_BOX_CREATE BoxDesc(Extents);
					BoxDesc.mRenderer = CreateBoxRenderer(Extents);

					PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
					ObjectDesc.mPosition	= StaticPos;
//		ObjectDesc.mCollisionGroup = 1;
					if(RCA)
					{
						ObjectDesc.mMass	= 1.0f;
						PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
						box0 = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
					}
					else
					{
						ObjectDesc.mMass	= 0.0f;
						box0 = CreatePintObject(pint, ObjectDesc);
					}
				}

				// Then we create a number of rotating links

				const float growth = 1.5f;
			//	float velocity = 0.25f;
				float velocity = 1.0f;

				const udword Nb = 5;
				PintActorHandle* Handles = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*Nb);
				pint.mUserData = Handles;

				for(udword i=0;i<Nb;i++)
				{
					x += ex*2.0f;
					z += ez;

					const Point localAnchor0(ex, 0.0f, ez);

					ez *= growth;
					z += ez;
					PintActorHandle box1;
					{
						const Point Extents(ex, ey, ez);
						const Point DynamicPos(x, y, z);
//						const Point DynamicPos(x+float(xxx-15)*Size, y, z+float(yyy-15)*Size);

						PINT_BOX_CREATE BoxDesc(Extents);
						BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		//				box1 = CreateSimpleObject(pint, &BoxDesc, float(i)+1.0f, DynamicPos);
						PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
						ObjectDesc.mPosition	= DynamicPos;
						ObjectDesc.mMass		= float(i)+1.0f;
//		ObjectDesc.mCollisionGroup = 1;

						if(RCA)
						{
		//					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
		//					box0 = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);

							PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
							ArticulatedDesc.mJointType	= PINT_JOINT_HINGE;
							ArticulatedDesc.mAxisIndex	= X_;
		/*					if(SetupLimits)
							{
								ArticulatedDesc.mMinLimit	= -PI;
								ArticulatedDesc.mMaxLimit	= angle;
							}
							else*/
							{
								ArticulatedDesc.mMinLimit	= PI;
								ArticulatedDesc.mMaxLimit	= -PI;
							}
							ArticulatedDesc.mParent		= box0;

		/*					ArticulatedDesc.mLocalPivot0.mPos	= TransformInv(pint, currLeft, leftAnchorLocation);
							ArticulatedDesc.mLocalPivot0.mRot	= leftParentRot;
							ArticulatedDesc.mLocalPivot1.mPos	= Point(0.f, 0.f, -1.f);
							ArticulatedDesc.mLocalPivot1.mRot	= rightRot;*/

							ArticulatedDesc.mLocalPivot0.mPos	= localAnchor0;
							ArticulatedDesc.mLocalPivot1.mPos	= Point(-ex, 0.0f, -ez);
		//					ArticulatedDesc.mLocalAxis0		= Point(1.0f, 0.0f, 0.0f);
		//					ArticulatedDesc.mLocalAxis1		= Point(1.0f, 0.0f, 0.0f);

							ArticulatedDesc.mUseMotor			= true;
		//					ArticulatedDesc.mMotor.mStiffness	= 1000.0f;
		//					ArticulatedDesc.mMotor.mDamping		= 100.0f;
							ArticulatedDesc.mTargetVel			= velocity;
							//TODO: revisit these
							ArticulatedDesc.mMotor.mStiffness	= 0.0f;
							ArticulatedDesc.mMotor.mDamping		= 10000.0f;
							ArticulatedDesc.mFrictionCoeff		= 0.0f;


							box1 = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
						}
						else
						{
							box1 = CreatePintObject(pint, ObjectDesc);
						}

						Handles[i] = box1;
					}

					if(!RCA)
					{
						PINT_HINGE_JOINT_CREATE Desc;
						Desc.mObject0		= box0;
						Desc.mObject1		= box1;
						Desc.mLocalPivot0	= localAnchor0;
						Desc.mLocalPivot1	= Point(-ex, 0.0f, -ez);
						Desc.mLocalAxis0	= Point(1.0f, 0.0f, 0.0f);
						Desc.mLocalAxis1	= Point(1.0f, 0.0f, 0.0f);

						if(1)
						{
							Desc.mUseMotor		= true;
							Desc.mDriveVelocity	= velocity;
						}

						PintJointHandle JointHandle = pint.CreateJoint(Desc);
						ASSERT(JointHandle);
					}

					box0 = box1;

		//			velocity *= 2.0f;
				}

				if(RCA)
					pint.AddRCArticulationToScene(RCA);
			}
		}
		return true;
	}

/*	virtual	float		DrawDebugText(Pint& pint, GLFontRenderer& renderer, float y, float text_scale)
	{
		const PintActorHandle* Handles = (const PintActorHandle*)pint.mUserData;
		for(udword i=0;i<5;i++)
			y = PrintAngularVelocity(pint, renderer, Handles[i], y, text_scale);
		return y;
	}*/

END_TEST(MotorizedHinges)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DoubleHinge = "Double hinge.";

START_TEST(DoubleHinge, CATEGORY_RCARTICULATIONS, gDesc_DoubleHinge)

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.11f, 7.16f, 5.43f), Point(-0.65f, -0.26f, -0.71f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		const PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(false));

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mPosition	= Point(0.0f, 3.0f, 0.0f);
		ObjectDesc.mMass		= 1.0f;

		// works
//		ObjectDesc.mLinearVelocity = Point(10000.0f, 10000.0f, 10000.0f);
//		ObjectDesc.mAngularVelocity = Point(1.0f, 0.0f, 0.0f);

		PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
		ArticulatedDesc.mFrictionCoeff = 0.0f;
		ArticulatedDesc.mParent = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);

		ObjectDesc.mPosition	= Point(0.0f, 5.0f, 0.0f);

		// doesn't work
//		ObjectDesc.mLinearVelocity = Point(10000.0f, 10000.0f, 10000.0f);

		ArticulatedDesc.mLocalPivot0.mPos = Point(0.0f, 1.0f, 0.0f);
		ArticulatedDesc.mLocalPivot1.mPos = Point(0.0f, -1.0f, 0.0f);
		ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

		ArticulatedDesc.mMinTwistLimit	= -45.0f * DEGTORAD;
		ArticulatedDesc.mMaxTwistLimit	= 45.0f * DEGTORAD;
		ArticulatedDesc.mMinSwing1Limit	= -75.0f * DEGTORAD;
		ArticulatedDesc.mMaxSwing1Limit	= 30.0f * DEGTORAD;
		ArticulatedDesc.mMinSwing2Limit	= 0.0f;
		ArticulatedDesc.mMaxSwing2Limit	= 0.0f;

//		ObjectDesc.mAngularVelocity = Point(100.0f, 0.0f, 0.0f);
//		ObjectDesc.mAngularVelocity = Point(0.0f, 100.0f, 0.0f);
//		ObjectDesc.mLinearVelocity = Point(0.0f, 0.0f, 10000.0f);
//		ObjectDesc.mAngularVelocity = Point(0.0f, 0.0f, 10000.0f);

		PintActorHandle h = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
		(void)h;

		pint.AddRCArticulationToScene(RCA);

//		pint.SetLinearVelocity(h, Point(10000.0f, 0.0f, 0.0f));
//		pint.SetAngularVelocity(h, Point(10000.0f, 0.0f, 0.0f));

		return true;
	}

END_TEST(DoubleHinge)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DoubleHinge2 = "Double hinge 2.";

START_TEST(DoubleHinge2, CATEGORY_RCARTICULATIONS, gDesc_DoubleHinge2)

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(5.11f, 7.16f, 5.43f), Point(-0.65f, -0.26f, -0.71f));
		desc.mGravity.Zero();
		SetDefEnv(desc, false);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation)
			return false;

		const PintDisabledGroups CollisionGroups(1, 1);
		pint.SetDisabledGroups(1, &CollisionGroups);

		const PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(true));

		const Point Extents(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc(Extents);
		BoxDesc.mRenderer = CreateBoxRenderer(Extents);

		const Point Extents2(1.0f, 1.0f, 1.0f);
		PINT_BOX_CREATE BoxDesc2(Extents2);
		BoxDesc2.mRenderer = CreateNullRenderer();//CreateBoxRenderer(Extents2);

		const Point StartPos(0.0f, 3.0f, 0.0f);

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mPosition	= StartPos;
		ObjectDesc.mMass		= 1.0f;
		ObjectDesc.mCollisionGroup	= 1;
		PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
		ArticulatedDesc.mFrictionCoeff = 0.02f;
		ObjectDesc.SetShape(&BoxDesc);
		ArticulatedDesc.mParent = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);

		ObjectDesc.mPosition	= StartPos + Point(0.0f, Extents.y/*+Extents2.y*/, 0.0f);

		ArticulatedDesc.mLocalPivot0.mPos = Point(0.0f, Extents.y, 0.0f);
		ArticulatedDesc.mLocalPivot1.mPos = Point(0.0f, 0.0f/*-Extents2.y*/, 0.0f);
		ArticulatedDesc.mJointType = PINT_JOINT_HINGE;

		ArticulatedDesc.mAxisIndex	= X_;
		ArticulatedDesc.mMinLimit	= -45.0f * DEGTORAD;
		ArticulatedDesc.mMaxLimit	= 45.0f * DEGTORAD;

/*		ArticulatedDesc.mMinTwistLimit	= -45.0f * DEGTORAD;
		ArticulatedDesc.mMaxTwistLimit	= 45.0f * DEGTORAD;
		ArticulatedDesc.mMinSwing1Limit	= -75.0f * DEGTORAD;
		ArticulatedDesc.mMaxSwing1Limit	= 30.0f * DEGTORAD;
		ArticulatedDesc.mMinSwing2Limit	= 0.0f;
		ArticulatedDesc.mMaxSwing2Limit	= 0.0f;*/

		ObjectDesc.SetShape(&BoxDesc2);
		ArticulatedDesc.mParent = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);


		ObjectDesc.mPosition	= StartPos + Point(0.0f, (Extents.y/*+Extents2.y*/)*2.0f, 0.0f);

		ArticulatedDesc.mLocalPivot0.mPos = Point(0.0f, 0.0f/*Extents2.y*/, 0.0f);
		ArticulatedDesc.mLocalPivot1.mPos = Point(0.0f, -Extents.y, 0.0f);
		ArticulatedDesc.mJointType = PINT_JOINT_HINGE;

		ArticulatedDesc.mAxisIndex	= Y_;
		ArticulatedDesc.mMinLimit	= -75.0f * DEGTORAD;
		ArticulatedDesc.mMaxLimit	= 30.0f * DEGTORAD;

		ObjectDesc.SetShape(&BoxDesc);
		pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);



		pint.AddRCArticulationToScene(RCA);

		return true;
	}

END_TEST(DoubleHinge2)

///////////////////////////////////////////////////////////////////////////////

// TODO:
// - check collision filters
// - timestep & #iters
// - switch to densities instead of masses
// - try to remap right_hip_xyz to avoid instabilities
// - fix feet axes
// - fix shoulder axes
// - try multiples hinges & invisible links
// - add pos actuators (also sliders in the UI) => are they actually pos actuators?

static const bool gUseFixedJoints = false;
static const bool gAddArms = true;

static float gTimestep = 0.005f;
static float gJointFriction = 0.02f;
static float gMaxJointVelocity = 20.0f;
static bool gUseFakeLimits = false;

static Point MujocoPosition(const Point& p)
{
	return Point(p.x, p.z, p.y);
}

static void SetupMujocoSphere(PINT_SPHERE_CREATE& desc, const Point& pos, float radius)
{
	desc.mRadius	= radius;
	desc.mLocalPos	= MujocoPosition(pos);
	desc.mRenderer	= CreateSphereRenderer(desc.mRadius);
}

static void SetupMujocoCapsule(PINT_CAPSULE_CREATE& desc, const Point& from, const Point& to, float radius)
{
	desc.mRadius = radius;
	desc.mHalfHeight = from.Distance(to)*0.5f;

	Point dir = MujocoPosition(to) - MujocoPosition(from);
	dir.Normalize();

	Matrix3x3 rot;
	rot.FromTo(Point(0.0f, 1.0f, 0.0f), dir);
	desc.mLocalRot = rot;

	const Point center = (from + to)*0.5f;
	desc.mLocalPos = MujocoPosition(center);

	desc.mRenderer	= CreateCapsuleRenderer(desc.mRadius, desc.mHalfHeight*2.0f);
}

static void SetupTwistLimits(PINT_RC_ARTICULATED_BODY_CREATE& desc, float min_limit, float max_limit)
{
	desc.mMinTwistLimit		= min_limit * DEGTORAD;
	desc.mMaxTwistLimit		= max_limit * DEGTORAD;
}

static void SetupSwing1Limits(PINT_RC_ARTICULATED_BODY_CREATE& desc, float min_limit, float max_limit)
{
	desc.mMinSwing1Limit	= min_limit * DEGTORAD;
	desc.mMaxSwing1Limit	= max_limit * DEGTORAD;
}

static void SetupSwing2Limits(PINT_RC_ARTICULATED_BODY_CREATE& desc, float min_limit, float max_limit)
{
	desc.mMinSwing2Limit	= min_limit * DEGTORAD;
	desc.mMaxSwing2Limit	= max_limit * DEGTORAD;
}

static void SetupArticBodyDesc(PINT_RC_ARTICULATED_BODY_CREATE& desc)
{
	desc.mFrictionCoeff = gJointFriction;
	desc.mMaxJointVelocity = gMaxJointVelocity;

	if(0)
	{
		desc.mMotor.mStiffness = 0.0f;
		desc.mMotor.mDamping = 10000.0f;
		desc.mMotor.mMaxForce = 10000.0f;
		desc.mMotor.mAccelerationDrive = false;

		desc.mUseMotor = true;
		desc.mTargetVel = 0.0f;
	}

	if(gUseFixedJoints)
	{
		if(desc.mParent)
			desc.mJointType = PINT_JOINT_FIXED;
	}

	if(gUseFakeLimits)
	{
		const float epsilon = 5.0f * DEGTORAD;
		desc.mMinLimit = -epsilon;
		desc.mMaxLimit = epsilon;
		SetupTwistLimits(desc, -epsilon, epsilon);
//		desc.mMinSwing1Limit = -epsilon;
//		desc.mMaxSwing1Limit = epsilon;
//		desc.mMinSwing2Limit = -epsilon;
//		desc.mMaxSwing2Limit = epsilon;
		SetupSwing1Limits(desc, -epsilon, epsilon);
		SetupSwing2Limits(desc, -epsilon, epsilon);
	}
}

static void SetupLink(PINT_OBJECT_CREATE& desc, const char* name, const Point& pos, const PINT_SHAPE_CREATE* shapes, float mass)
{
	desc.mName				= name;
	desc.mMass				= mass;
//	desc.mMassForInertia	= mass*10.0f;
	desc.mPosition			= MujocoPosition(pos);
	desc.SetShape(shapes);
}

class MatrixStack
{
	public:
			MatrixStack();

	void	Transform(Point& pos, Quat& rot);
	void	Push(const Point& pos, const Quat& rot);
	void	Pop();

	private:
	udword		mNbMatrices;
	Matrix4x4	mMat[64];
};

MatrixStack::MatrixStack() : mNbMatrices(0)
{
}

void MatrixStack::Transform(Point& pos, Quat& rot)
{
	if(!mNbMatrices)
		return;

	Matrix4x4 Combo;
	Combo.Identity();
/*	for(udword i=0;i<mNbMatrices;i++)
	{
		Combo *= mMat[i];
	}*/
	Combo = mMat[mNbMatrices-1];

	Matrix4x4 M = rot;
	M.SetTrans(pos);

	Matrix4x4 tmp0 = Combo * M;
	Matrix4x4 tmp1 = M * Combo;
	PR FinalPose = tmp0;
	pos = FinalPose.mPos;
	rot = FinalPose.mRot;
}

void MatrixStack::Push(const Point& pos, const Quat& rot)
{
	Matrix4x4 M = rot;
	M.SetTrans(pos);
	mMat[mNbMatrices++] = M;
}

void MatrixStack::Pop()
{
	mNbMatrices--;
}


static const char* gDesc_MujocoHumanoid = "Humanoid model from Mujoco.";

START_TEST(MujocoHumanoid, CATEGORY_RCARTICULATIONS, gDesc_MujocoHumanoid)

	CheckBoxPtr		mCheckBox_FixedBase;
	CheckBoxPtr		mCheckBox_Debug_UseFakeLimits;
	EditBoxPtr		mEditBox_JointFriction;
	EditBoxPtr		mEditBox_MaxJointVelocity;
	EditBoxPtr		mEditBox_Timestep;

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 200;
		WD.mLabel	= "MujocoHumanoid";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword OffsetX = 120;
		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 120;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;

		{
//			y += YStep;

			mCheckBox_FixedBase = helper.CreateCheckBox(UI, 0, 4, y, 200, 20, "Fixed base", &UIElems, false, null, null);
			y += YStep;

			mCheckBox_Debug_UseFakeLimits = helper.CreateCheckBox(UI, 0, 4, y, 200, 20, "DEBUG: use small limits", &UIElems, false, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Joint friction:", &UIElems);
//			mEditBox_JointFriction = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.02", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			mEditBox_JointFriction = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.5", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Max joint velocity:", &UIElems);
			mEditBox_MaxJointVelocity = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "20.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;

			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Timestep:", &UIElems);
			mEditBox_Timestep = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.005", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
	
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}

	virtual	float	GetRenderData(Point& center)	const	{ return 10000.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(4.25f, 12.43f, 41.98f), Point(0.01f, -0.37f, -0.93f));
		//desc.mCamera[0] = PintCameraPose(Point(-1.20f, 1.54f, 0.65f), Point(0.82f, -0.13f, -0.56f));
		//desc.mCamera[0] = PintCameraPose(Point(1.36f, 1.80f, -0.79f), Point(-0.75f, -0.49f, 0.45f));
		desc.mCamera[0] = PintCameraPose(Point(1.20f, 1.18f, -1.77f), Point(-0.50f, -0.20f, 0.84f));
		//desc.mGravity.Zero();
		SetDefEnv(desc, true);

		gUseFakeLimits = mCheckBox_Debug_UseFakeLimits ? mCheckBox_Debug_UseFakeLimits->IsChecked() : false;
		gJointFriction = GetFloat(0.02f, mEditBox_JointFriction);
		gMaxJointVelocity = GetFloat(20.0f, mEditBox_MaxJointVelocity);
		gTimestep = GetFloat(0.005f, mEditBox_Timestep);

		//<option timestep="0.005" iterations="50" tolerance="1e-10" solver="Newton" jacobian="dense" cone="pyramidal"/>
		desc.mTimestep = gTimestep;
		desc.mNbSimulateCallsPerFrame = udword((1.0f/60.0f)/desc.mTimestep);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRCArticulations || !caps.mSupportRigidBodySimulation || !caps.mSupportCompounds)
			return false;

		const bool FixedBase = mCheckBox_FixedBase ? mCheckBox_FixedBase->IsChecked() : false;
		const PintArticHandle RCA = pint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(FixedBase));

		MatrixStack MS;

		PintActorHandle torso;
Point torsoPos;
		{
			//<geom name="torso1" type="capsule" fromto="0 -.07 0 0 .07 0"  size="0.07"/>
			PINT_CAPSULE_CREATE torso1Desc;
			torso1Desc.mName = "torso1";
			SetupMujocoCapsule(torso1Desc, Point(0.0f, -0.07f, 0.0f), Point(0.0f, 0.07f, 0.0f), 0.07f);

			//<geom name="head" type="sphere" pos="0 0 .19" size=".09"/>
			PINT_SPHERE_CREATE headDesc;
			headDesc.mName = "head";
			SetupMujocoSphere(headDesc, Point(0.0f, 0.0f, 0.19f), 0.09f);

			//<geom name="uwaist" type="capsule" fromto="-.01 -.06 -.12 -.01 .06 -.12" size="0.06"/>
			PINT_CAPSULE_CREATE uwaistDesc;
			uwaistDesc.mName = "uwaist";
			SetupMujocoCapsule(uwaistDesc, Point(-0.01f, -0.06f, -0.12f), Point(-0.01f, 0.06f, -0.12f), 0.06f);

			//<body name="torso" pos="0 0 1.4">
			torso1Desc.SetNext(&headDesc);
			headDesc.SetNext(&uwaistDesc);

			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "torso", Point(0.0f, 0.0f, 1.4f), &torso1Desc, 8.3f);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				SetupArticBodyDesc(ArticulatedDesc);
				torso = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				torso = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
torsoPos = ObjectDesc.mPosition;
		}

		PintActorHandle lwaist;
Point lwaistPos;
		{
			//<geom name="lwaist" type="capsule" fromto="0 -.06 0 0 .06 0"  size="0.06" />
			PINT_CAPSULE_CREATE lwaistDesc;
			SetupMujocoCapsule(lwaistDesc, Point(0.0f, -0.06f, 0.0f), Point(0.0f, 0.06f, 0.0f), 0.06f);

			//<body name="lwaist" pos="-.01 0 -0.260" quat="1.000 0 -0.002 0" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "lwaist", Point(-0.01f, 0.0f, -0.260f), &lwaistDesc, 2.0f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = torso;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.065f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - torsoPos;

/*					// Rel joint pose
					Point JointRelPos = MujocoPosition(Point(0.0f, 0.0f, 0.065f));
					Quat JointRelRot(1.0f, 0.0f, 0.0f, 0.0f);
					// World joint pose
					MS.Transform(JointRelPos, JointRelRot);*/

					// Parent
//					ArticulatedDesc.mLocalPivot0.mPos = TorsoPos - JointRelPos;
					// Child
//					ArticulatedDesc.mLocalPivot1.mPos = ObjectDesc.mPosition - JointRelPos;


//					ArticulatedDesc.mLocalPivot0.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.065f));
					//MS.Transform(ArticulatedDesc.mLocalPivot0.mPos, ArticulatedDesc.mLocalPivot0.mRot);
//					ArticulatedDesc.mLocalPivot0.mPos = -ArticulatedDesc.mLocalPivot1.mPos;

//					ArticulatedDesc.mMinTwistLimit		= 0.0f;
//					ArticulatedDesc.mMaxTwistLimit		= 0.0f;
					SetupTwistLimits(ArticulatedDesc, 0.0f, 0.0f);

					//<joint name="abdomen_z" type="hinge" pos="0 0 0.065" axis="0 0 1" range="-45 45" damping="5" stiffness="20" armature="0.02" />
//					ArticulatedDesc.mMinSwing1Limit		= -45.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 45.0f * DEGTORAD;
					SetupSwing1Limits(ArticulatedDesc, -45.0f, 45.0f);

					//<joint name="abdomen_y" type="hinge" pos="0 0 0.065" axis="0 1 0" range="-75 30" damping="5" stiffness="10" armature="0.02" />
//					ArticulatedDesc.mMinSwing2Limit		= -30.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 75.0f * DEGTORAD;
					SetupSwing2Limits(ArticulatedDesc, -30.0f, 75.0f);

					//LockJoint(ArticulatedDesc);
//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				lwaist = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				lwaist = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
lwaistPos = ObjectDesc.mPosition;
		}

		PintActorHandle pelvis;
Point pelvisPos;
		{
			//<geom name="butt" type="capsule" fromto="-.02 -.07 0 -.02 .07 0"  size="0.09" />
			PINT_CAPSULE_CREATE buttDesc;
			SetupMujocoCapsule(buttDesc, Point(-0.02f, -0.07f, 0.0f), Point(-0.02f, 0.07f, 0.0f), 0.09f);

			//<body name="pelvis" pos="0 0 -0.165" quat="1.000 0 -0.002 0" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "pelvis", Point(0.0f, 0.0f, -0.165f), &buttDesc, 5.9f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = lwaist;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.1f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - lwaistPos;

					//<joint name="abdomen_x" type="hinge" pos="0 0 0.1" axis="1 0 0" range="-35 35" damping="5" stiffness="10" armature="0.02" />
					ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
					ArticulatedDesc.mAxisIndex = X_;
					ArticulatedDesc.mMinLimit = -35.0f * DEGTORAD;
					ArticulatedDesc.mMaxLimit = 35.0f * DEGTORAD;

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				pelvis = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				pelvis = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
pelvisPos = ObjectDesc.mPosition;
		}

		PintActorHandle right_thigh;
Point right_thighPos;
		{
			//<geom name="right_thigh1" type="capsule" fromto="0 0 0 0 0.01 -.34"  size="0.06" />
			PINT_CAPSULE_CREATE right_thigh1Desc;
			SetupMujocoCapsule(right_thigh1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.01f, -0.34f), 0.06f);

			//<body name="right_thigh" pos="0 -0.1 -0.04" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "right_thigh", Point(0.0f, -0.1f, -0.04f), &right_thigh1Desc, 4.5f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = pelvis;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - pelvisPos;

					//<joint name="right_hip_x" type="hinge" pos="0 0 0" axis="1 0 0" range="-25 5"   damping="5" stiffness="10" armature="0.01" />
//					ArticulatedDesc.mMinTwistLimit		= -5.0f * DEGTORAD;
//					ArticulatedDesc.mMaxTwistLimit		= 25.0f * DEGTORAD;
					SetupTwistLimits(ArticulatedDesc, -5.0f, 25.0f);

					//<joint name="right_hip_z" type="hinge" pos="0 0 0" axis="0 0 1" range="-60 35"  damping="5" stiffness="10" armature="0.01" />
//					ArticulatedDesc.mMinSwing1Limit		= -35.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 60.0f * DEGTORAD;
					SetupSwing1Limits(ArticulatedDesc, -35.0f, 60.0f);

					//<joint name="right_hip_y" type="hinge" pos="0 0 0" axis="0 1 0" range="-120 20" damping="5" stiffness="20" armature="0.01" />
//					ArticulatedDesc.mMinSwing2Limit		= -20.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 120.0f * DEGTORAD;
					SetupSwing2Limits(ArticulatedDesc, -20.0f, 120.0f);

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				right_thigh = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				right_thigh = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
right_thighPos = ObjectDesc.mPosition;
		}

		PintActorHandle right_shin;
Point right_shinPos;
		{
			//<geom name="right_shin1" type="capsule" fromto="0 0 0 0 0 -.3"   size="0.049" />
			PINT_CAPSULE_CREATE right_shin1Desc;
			SetupMujocoCapsule(right_shin1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, -0.3f), 0.049f);

			//<body name="right_shin" pos="0 0.01 -0.403" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "right_shin", Point(0.0f, 0.01f, -0.403f), &right_shin1Desc, 2.6f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = right_thigh;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.02f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - right_thighPos;

					//<joint name="right_knee" type="hinge" pos="0 0 .02" axis="0 -1 0" range="-160 -2" stiffness="1" armature="0.0060" />
					ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
					ArticulatedDesc.mAxisIndex = Z_;
					ArticulatedDesc.mMinLimit = -160.0f * DEGTORAD;
					ArticulatedDesc.mMaxLimit = -2.0f * DEGTORAD;

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				right_shin = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				right_shin = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
right_shinPos = ObjectDesc.mPosition;
		}

		PintActorHandle right_foot;
Point right_footPos;
		{
			//<geom name="right_foot_cap1" type="capsule" fromto="-.07 -0.02 0 0.14 -0.04 0"  size="0.027" />
			PINT_CAPSULE_CREATE right_foot_cap1Desc;
			SetupMujocoCapsule(right_foot_cap1Desc, Point(-0.07f, -0.02f, 0.0f), Point(0.14f, -0.04f, 0.0f), 0.027f);

			//<geom name="right_foot_cap2" type="capsule" fromto="-.07 0 0 0.14  0.02 0"  size="0.027" />
			PINT_CAPSULE_CREATE right_foot_cap2Desc;
			SetupMujocoCapsule(right_foot_cap2Desc, Point(-0.07f, 0.0f, 0.0f), Point(0.14f, 0.02f, 0.0f), 0.027f);

			//<body name="right_foot" pos="0 0 -.39" >
			right_foot_cap1Desc.SetNext(&right_foot_cap2Desc);
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "right_foot", Point(0.0f, 0.0f, -0.39f), &right_foot_cap1Desc, 1.1f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = right_shin;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.06f));	//####

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - right_shinPos;

					//<joint name="right_ankle_y" type="hinge" pos="0 0 0.08" axis="0 1 0"   range="-50 50" stiffness="4" armature="0.0008" />
					//<joint name="right_ankle_x" type="hinge" pos="0 0 0.04" axis="1 0 0.5" range="-50 50" stiffness="1"  armature="0.0006" />

//					ArticulatedDesc.mMinTwistLimit		= -50.0f * DEGTORAD;
//					ArticulatedDesc.mMaxTwistLimit		= 50.0f * DEGTORAD;
					SetupTwistLimits(ArticulatedDesc, -50.0f, 50.0f);

//					ArticulatedDesc.mMinSwing1Limit		= 0.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 0.0f * DEGTORAD;
					SetupSwing1Limits(ArticulatedDesc, 0.0f, 0.0f);

//					ArticulatedDesc.mMinSwing2Limit		= -50.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 50.0f * DEGTORAD;
					SetupSwing2Limits(ArticulatedDesc, -50.0f, 50.0f);

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				right_foot = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				right_foot = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
right_footPos = ObjectDesc.mPosition;
		}

		MS.Pop();
		MS.Pop();
		MS.Pop();

		PintActorHandle left_thigh;
Point left_thighPos;
		{
			//<geom name="left_thigh1" type="capsule" fromto="0 0 0 0 -0.01 -.34"  size="0.06" />
			PINT_CAPSULE_CREATE left_thigh1Desc;
			SetupMujocoCapsule(left_thigh1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.0f, -0.01f, -0.34f), 0.06f);

			//<body name="left_thigh" pos="0 0.1 -0.04" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "left_thigh", Point(0.0f, 0.1f, -0.04f), &left_thigh1Desc, 4.5f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = pelvis;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - pelvisPos;

					//<joint name="left_hip_x" type="hinge" pos="0 0 0" axis="-1 0 0" range="-25 5"  damping="5" stiffness="10" armature="0.01" />
//					ArticulatedDesc.mMinTwistLimit		= -25.0f * DEGTORAD;
//					ArticulatedDesc.mMaxTwistLimit		= 5.0f * DEGTORAD;
//						ArticulatedDesc.mMinTwistLimit		= 0.0f;
//						ArticulatedDesc.mMaxTwistLimit		= 0.0f;
					SetupTwistLimits(ArticulatedDesc, -25.0f, 5.0f);

					//<joint name="left_hip_z" type="hinge" pos="0 0 0" axis="0 0 -1" range="-60 35" damping="5" stiffness="10" armature="0.01" />
//					ArticulatedDesc.mMinSwing1Limit		= -60.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 35.0f * DEGTORAD;
//						ArticulatedDesc.mMinSwing1Limit		= 0.0f;
//						ArticulatedDesc.mMaxSwing1Limit		= 0.0f;
					SetupSwing1Limits(ArticulatedDesc, -60.0f, 35.0f);

					//<joint name="left_hip_y" type="hinge" pos="0 0 0" axis="0 1 0" range="-120 20" damping="5" stiffness="20" armature="0.01" />
//					ArticulatedDesc.mMinSwing2Limit		= -20.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 120.0f * DEGTORAD;
//						ArticulatedDesc.mMinSwing2Limit		= 0.0f;
//						ArticulatedDesc.mMaxSwing2Limit		= 0.0f;
					SetupSwing2Limits(ArticulatedDesc, -20.0f, 120.0f);

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				left_thigh = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				left_thigh = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
left_thighPos = ObjectDesc.mPosition;
		}

		PintActorHandle left_shin;
Point left_shinPos;
		{
			//<geom name="left_shin1" type="capsule" fromto="0 0 0 0 0 -.3"   size="0.049" />
			PINT_CAPSULE_CREATE left_shin1Desc;
			SetupMujocoCapsule(left_shin1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, -0.3f), 0.049f);

			//<body name="left_shin" pos="0 -0.01 -0.403" >
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "left_shin", Point(0.0f, -0.01f, -0.403f), &left_shin1Desc, 2.6f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = left_thigh;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.02f));

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - left_thighPos;

					//<joint name="left_knee" type="hinge" pos="0 0 .02" axis="0 -1 0" range="-160 -2" stiffness="1" armature="0.0060" />
					ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
					ArticulatedDesc.mAxisIndex = Z_;
					ArticulatedDesc.mMinLimit = -160.0f * DEGTORAD;
					ArticulatedDesc.mMaxLimit = -2.0f * DEGTORAD;

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				left_shin = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				left_shin = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
left_shinPos = ObjectDesc.mPosition;
		}

		PintActorHandle left_foot;
Point left_footPos;
		{
			//<geom name="left_foot_cap1" type="capsule" fromto="-.07 0.02 0 0.14 0.04 0"  size="0.027" />
			PINT_CAPSULE_CREATE left_foot_cap1Desc;
			SetupMujocoCapsule(left_foot_cap1Desc, Point(-0.07f, 0.02f, 0.0f), Point(0.14f, 0.04f, 0.0f), 0.027f);

			//<geom name="left_foot_cap2" type="capsule" fromto="-.07 0 0 0.14  -0.02 0"  size="0.027" />
			PINT_CAPSULE_CREATE left_foot_cap2Desc;
			SetupMujocoCapsule(left_foot_cap2Desc, Point(-0.07f, 0.0f, 0.0f), Point(0.14f, -0.02f, 0.0f), 0.027f);

			//<body name="left_foot" pos="0 0 -.39" >
			left_foot_cap1Desc.SetNext(&left_foot_cap2Desc);
			PINT_OBJECT_CREATE ObjectDesc;
			SetupLink(ObjectDesc, "left_foot", Point(0.0f, 0.0f, -0.39f), &left_foot_cap1Desc, 1.1f);
			MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

			if(RCA)
			{
				PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = left_shin;
				if(1)
				{
					ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

					ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.06f));	//####

					Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
					ArticulatedDesc.mLocalPivot0.mPos = wp - left_shinPos;

					//	<joint name="left_ankle_y" type="hinge" pos="0 0 0.08" axis="0 1 0"   range="-50 50"  stiffness="4" armature="0.0008" />
					//	<joint name="left_ankle_x" type="hinge" pos="0 0 0.04" axis="1 0 0.5" range="-50 50"  stiffness="1"  armature="0.0006" />

//					ArticulatedDesc.mMinTwistLimit		= -50.0f * DEGTORAD;
//					ArticulatedDesc.mMaxTwistLimit		= 50.0f * DEGTORAD;
					SetupTwistLimits(ArticulatedDesc, -50.0f, 50.0f);

//					ArticulatedDesc.mMinSwing1Limit		= 0.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing1Limit		= 0.0f * DEGTORAD;
					SetupSwing1Limits(ArticulatedDesc, 0.0f, 0.0f);

//					ArticulatedDesc.mMinSwing2Limit		= -50.0f * DEGTORAD;
//					ArticulatedDesc.mMaxSwing2Limit		= 50.0f * DEGTORAD;
					SetupSwing2Limits(ArticulatedDesc, -50.0f, 50.0f);

//					ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
				}
				SetupArticBodyDesc(ArticulatedDesc);
				left_foot = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
			}
			else
				left_foot = CreatePintObject(pint, ObjectDesc);

			MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
left_footPos = ObjectDesc.mPosition;
		}                        
                        
		MS.Pop();
		MS.Pop();
		MS.Pop();

		MS.Pop();
		MS.Pop();

		if(gAddArms)
		{
			PintActorHandle right_upper_arm;
	Point right_upper_armPos;
			{
				//<geom name="right_uarm1" type="capsule" fromto="0 0 0 .16 -.16 -.16"  size="0.04 0.16" />
				PINT_CAPSULE_CREATE right_uarm1Desc;
				SetupMujocoCapsule(right_uarm1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.16f, -0.16f, -0.16f), 0.04f);

				//<body name="right_upper_arm" pos="0 -0.17 0.06" >
				PINT_OBJECT_CREATE ObjectDesc;
				SetupLink(ObjectDesc, "right_upper_arm", Point(0.0f, -0.17f, 0.06f), &right_uarm1Desc, 1.6f);
				MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = torso;
					if(1)
					{
						ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

						ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

						Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
						ArticulatedDesc.mLocalPivot0.mPos = wp - torsoPos;

						Point axis0(2.0f, 1.0f, 1.0f);	axis0.Normalize();
						Point axis1(0.0f, -1.0f, 1.0f);	axis1.Normalize();
						Point cr = axis0^axis1;

						Matrix3x3 fromTo;
						fromTo.FromTo(Point(1.0f, 0.0f, 0.0f), axis0);
						ArticulatedDesc.mLocalPivot0.mRot = fromTo;
						ArticulatedDesc.mLocalPivot1.mRot = fromTo;

	//                <joint name="right_shoulder1" type="hinge" pos="0 0 0" axis="2 1 1"  range="-85 60" stiffness="1" armature="0.0068" />
	//                <joint name="right_shoulder2" type="hinge" pos="0 0 0" axis="0 -1 1" range="-85 60" stiffness="1"  armature="0.0051" />

//						ArticulatedDesc.mMinTwistLimit		= -85.0f * DEGTORAD;
//						ArticulatedDesc.mMaxTwistLimit		= 60.0f * DEGTORAD;
						SetupTwistLimits(ArticulatedDesc, -85.0f, 60.0f);

//						ArticulatedDesc.mMinSwing1Limit		= 0.0f * DEGTORAD;
//						ArticulatedDesc.mMaxSwing1Limit		= 0.0f * DEGTORAD;
						SetupSwing1Limits(ArticulatedDesc, 0.0f, 0.0f);

//						ArticulatedDesc.mMinSwing2Limit		= -85.0f * DEGTORAD;
//						ArticulatedDesc.mMaxSwing2Limit		= 60.0f * DEGTORAD;
						SetupSwing2Limits(ArticulatedDesc, -85.0f, 60.0f);
					}
					SetupArticBodyDesc(ArticulatedDesc);
					right_upper_arm = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
					right_upper_arm = CreatePintObject(pint, ObjectDesc);

				MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
	right_upper_armPos = ObjectDesc.mPosition;
			}

			PintActorHandle right_lower_arm;
	Point right_lower_armPos;
			{
				//<geom name="right_larm" type="capsule" fromto="0.01 0.01 0.01 .17 .17 .17"  size="0.031" />
				PINT_CAPSULE_CREATE right_larmDesc;
				SetupMujocoCapsule(right_larmDesc, Point(0.01f, 0.01f, 0.01f), Point(0.17f, 0.17f, 0.17f), 0.031f);

				//<geom name="right_hand" type="sphere" pos=".18 .18 .18"  size="0.04"/>
				PINT_SPHERE_CREATE right_handDesc;
				SetupMujocoSphere(right_handDesc, Point(0.18f, 0.18f, 0.18f), 0.04f);

				//<body name="right_lower_arm" pos=".18 -.18 -.18" >
				right_larmDesc.SetNext(&right_handDesc);
				PINT_OBJECT_CREATE ObjectDesc;
				SetupLink(ObjectDesc, "right_lower_arm", Point(0.18f, -0.18f, -0.18f), &right_larmDesc, 1.2f);
				MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = right_upper_arm;
					if(1)
					{
						ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

						ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

						Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
						ArticulatedDesc.mLocalPivot0.mPos = wp - right_upper_armPos;

	//          <joint name="right_elbow" type="hinge" pos="0 0 0" axis="0 -1 1" range="-90 50"  stiffness="0" armature="0.0028" />

						ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
						ArticulatedDesc.mAxisIndex = Z_;
						ArticulatedDesc.mMinLimit = -90.0f * DEGTORAD;
						ArticulatedDesc.mMaxLimit = 50.0f * DEGTORAD;
					}
					SetupArticBodyDesc(ArticulatedDesc);
					right_lower_arm = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
					right_lower_arm = CreatePintObject(pint, ObjectDesc);

				MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
	right_lower_armPos = ObjectDesc.mPosition;
			}        

			MS.Pop();
			MS.Pop();

			PintActorHandle left_upper_arm;
	Point left_upper_armPos;
			{
				//<geom name="left_uarm1" type="capsule" fromto="0 0 0 .16 .16 -.16"  size="0.04 0.16" />
				PINT_CAPSULE_CREATE left_uarm1Desc;
				SetupMujocoCapsule(left_uarm1Desc, Point(0.0f, 0.0f, 0.0f), Point(0.16f, 0.16f, -0.16f), 0.04f);

				//<body name="left_upper_arm" pos="0 0.17 0.06" >
				PINT_OBJECT_CREATE ObjectDesc;
				SetupLink(ObjectDesc, "left_upper_arm", Point(0.0f, 0.17f, 0.06f), &left_uarm1Desc, 1.6f);
				MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = torso;
					if(1)
					{
						ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

						ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

						Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
						ArticulatedDesc.mLocalPivot0.mPos = wp - torsoPos;

						Point axis0(2.0f, -1.0f, 1.0f);	axis0.Normalize();
						Point axis1(0.0f, 1.0f, 1.0f);	axis1.Normalize();
						Point cr = axis0^axis1;

						Matrix3x3 fromTo;
						fromTo.FromTo(Point(1.0f, 0.0f, 0.0f), axis0);
						ArticulatedDesc.mLocalPivot0.mRot = fromTo;
						ArticulatedDesc.mLocalPivot1.mRot = fromTo;

	//                <joint name="left_shoulder1" type="hinge" pos="0 0 0" axis="2 -1 1" range="-60 85" stiffness="1" armature="0.0068" />
	//                <joint name="left_shoulder2" type="hinge" pos="0 0 0" axis="0 1 1" range="-60 85"  stiffness="1" armature="0.0051" />

//						ArticulatedDesc.mMinTwistLimit		= -60.0f * DEGTORAD;
//						ArticulatedDesc.mMaxTwistLimit		= 85.0f * DEGTORAD;
						SetupTwistLimits(ArticulatedDesc, -60.0f, 85.0f);

//						ArticulatedDesc.mMinSwing1Limit		= 0.0f * DEGTORAD;
//						ArticulatedDesc.mMaxSwing1Limit		= 0.0f * DEGTORAD;
						SetupSwing1Limits(ArticulatedDesc, 0.0f, 0.0f);

//						ArticulatedDesc.mMinSwing2Limit		= -60.0f * DEGTORAD;
//						ArticulatedDesc.mMaxSwing2Limit		= 85.0f * DEGTORAD;
						SetupSwing2Limits(ArticulatedDesc, -60.0f, 85.0f);
					}
					SetupArticBodyDesc(ArticulatedDesc);
					left_upper_arm = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
					left_upper_arm = CreatePintObject(pint, ObjectDesc);

				MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
	left_upper_armPos = ObjectDesc.mPosition;


			}

			PintActorHandle left_lower_arm;
	Point left_lower_armPos;
			{
				//<geom name="left_larm" type="capsule" fromto="0.01 -0.01 0.01 .17 -.17 .17"  size="0.031" />
				PINT_CAPSULE_CREATE left_larmDesc;
				SetupMujocoCapsule(left_larmDesc, Point(0.01f, -0.01f, 0.01f), Point(0.17f, -0.17f, 0.17f), 0.031f);

				//<geom name="left_hand" type="sphere" pos=".18 -.18 .18"  size="0.04"/>
				PINT_SPHERE_CREATE left_handDesc;
				SetupMujocoSphere(left_handDesc, Point(0.18f, -0.18f, 0.18f), 0.04f);

				//<body name="left_lower_arm" pos=".18 .18 -.18" >
				left_larmDesc.SetNext(&left_handDesc);
				PINT_OBJECT_CREATE ObjectDesc;
				SetupLink(ObjectDesc, "left_lower_arm", Point(0.18f, 0.18f, -0.18f), &left_larmDesc, 1.2f);
				MS.Transform(ObjectDesc.mPosition, ObjectDesc.mRotation);

				if(RCA)
				{
					PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
					ArticulatedDesc.mParent = left_upper_arm;
					if(1)
					{
						ArticulatedDesc.mJointType = PINT_JOINT_SPHERICAL;

						ArticulatedDesc.mLocalPivot1.mPos = MujocoPosition(Point(0.0f, 0.0f, 0.0f));

						Point wp = ArticulatedDesc.mLocalPivot1.mPos + ObjectDesc.mPosition;
						ArticulatedDesc.mLocalPivot0.mPos = wp - left_upper_armPos;

	//                    <joint name="left_elbow" type="hinge" pos="0 0 0" axis="0 -1 -1" range="-90 50" stiffness="0" armature="0.0028" />
						ArticulatedDesc.mJointType = PINT_JOINT_HINGE;
						ArticulatedDesc.mAxisIndex = Z_;
						ArticulatedDesc.mMinLimit = -90.0f * DEGTORAD;
						ArticulatedDesc.mMaxLimit = 50.0f * DEGTORAD;
					}
					SetupArticBodyDesc(ArticulatedDesc);
					left_lower_arm = pint.CreateRCArticulatedObject(ObjectDesc, ArticulatedDesc, RCA);
				}
				else
					left_lower_arm = CreatePintObject(pint, ObjectDesc);

				MS.Push(ObjectDesc.mPosition, ObjectDesc.mRotation);
	left_lower_armPos = ObjectDesc.mPosition;
			}
		}

		if(RCA)
			pint.AddRCArticulationToScene(RCA);

		return true;
	}

END_TEST(MujocoHumanoid)

///////////////////////////////////////////////////////////////////////////////

