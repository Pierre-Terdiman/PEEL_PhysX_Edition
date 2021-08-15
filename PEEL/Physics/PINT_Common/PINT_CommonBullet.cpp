///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all Bullet plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonBullet.h"

Point Bullet::GetMainColor()
{
	return BULLET_MAIN_COLOR;
}

	// A special version to keep track of the triangle index & enable backface culling
	struct MyClosestRayResultCallback : public btCollisionWorld::RayResultCallback
	{
		MyClosestRayResultCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld) : m_rayFromWorld(rayFromWorld), m_rayToWorld(rayToWorld)
		{
#ifdef BULLET_BACKFACE_CULLING
			m_flags = btTriangleRaycastCallback::kF_FilterBackfaces;
#endif
		}

		btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
		btVector3	m_rayToWorld;

		btVector3	m_hitNormalWorld;
		btVector3	m_hitPointWorld;

		udword		m_TriangleIndex;

		virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
		{
			//caller already does the filter on the m_closestHitFraction
			ASSERT(rayResult.m_hitFraction <= m_closestHitFraction);

			if(rayResult.m_localShapeInfo)
				m_TriangleIndex	= rayResult.m_localShapeInfo->m_triangleIndex;
			else
				m_TriangleIndex	= INVALID_ID;

			m_closestHitFraction = rayResult.m_hitFraction;
			m_collisionObject = rayResult.m_collisionObject;
			if(normalInWorldSpace)
			{
				m_hitNormalWorld = rayResult.m_hitNormalLocal;
			}
			else
			{
				///need to transform normal into worldspace
				m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
			}
			m_hitPointWorld.setInterpolate3(m_rayFromWorld,m_rayToWorld,rayResult.m_hitFraction);
			return rayResult.m_hitFraction;
		}
	};

static inline_ void FillResultStruct(PintRaycastHit& hit, const MyClosestRayResultCallback& result, float max_dist)
{
//	hit.mObject			= result.m_collisionObject;
	hit.mTouchedActor	= (PintActorHandle)btRigidBody::upcast(result.m_collisionObject);
	hit.mTouchedShape	= null;
	hit.mImpact			= ToPoint(result.m_hitPointWorld);
	hit.mNormal			= ToPoint(result.m_hitNormalWorld);
//	hit.mDistance		= origin.Distance(hit.mImpact);
	hit.mDistance		= result.m_closestHitFraction * max_dist;
	hit.mTriangleIndex	= result.m_TriangleIndex;
}

udword Bullet::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	ASSERT(mDynamicsWorld);

	udword NbHits = 0;
	while(nb--)
	{
		const btVector3 from = ToBtVector3(raycasts->mOrigin);
		const btVector3 target = ToBtVector3(raycasts->mOrigin + raycasts->mDir * raycasts->mMaxDist);

//		btCollisionWorld::ClosestRayResultCallback resultCallback(from, target);
		MyClosestRayResultCallback resultCallback(from, target);

		mDynamicsWorld->rayTest(from, target, resultCallback);

		if(resultCallback.m_collisionObject)
		{
			NbHits++;
			FillResultStruct(*dest, resultCallback, raycasts->mMaxDist);
		}
		else
		{
			dest->SetNoHit();
		}
		raycasts++;
		dest++;
	}
	return NbHits;
}

udword Bullet::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	ASSERT(mDynamicsWorld);

	udword NbHits = 0;
	while(nb--)
	{
		const btBoxShape BoxShape(ToBtVector3(sweeps->mBox.mExtents));

		const btQuaternion Rot = ToBtQuaternion(Quat(sweeps->mBox.mRot));

		const btTransform from(Rot, ToBtVector3(sweeps->mBox.mCenter));
		const btTransform to(Rot, ToBtVector3(sweeps->mBox.mCenter + sweeps->mDir * sweeps->mMaxDist));

//		btCollisionWorld::ClosestConvexResultCallback resultCallback(from.getOrigin(), to.getOrigin());
		MyClosestConvexResultCallback resultCallback(from.getOrigin(), to.getOrigin());

		mDynamicsWorld->convexSweepTest(&BoxShape, from, to, resultCallback);

		if(resultCallback.m_hitCollisionObject)
		{
			NbHits++;
			FillResultStruct(*dest, resultCallback, sweeps->mMaxDist);
		}
		else
		{
			dest->SetNoHit();
		}

		sweeps++;
		dest++;
	}
	return NbHits;
}

udword Bullet::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	ASSERT(mDynamicsWorld);

	udword NbHits = 0;
	const btQuaternion Rot = btQuaternion::getIdentity();
	while(nb--)
	{
		const btSphereShape SphereShape(sweeps->mSphere.mRadius);

		const btTransform from(Rot, ToBtVector3(sweeps->mSphere.mCenter));
		const btTransform to(Rot, ToBtVector3(sweeps->mSphere.mCenter + sweeps->mDir * sweeps->mMaxDist));

//		btCollisionWorld::ClosestConvexResultCallback resultCallback(from.getOrigin(), to.getOrigin());
		MyClosestConvexResultCallback resultCallback(from.getOrigin(), to.getOrigin());

		mDynamicsWorld->convexSweepTest(&SphereShape, from, to, resultCallback);

		if(resultCallback.m_hitCollisionObject)
		{
			NbHits++;
			FillResultStruct(*dest, resultCallback, sweeps->mMaxDist);
		}
		else
		{
			dest->SetNoHit();
		}

		sweeps++;
		dest++;
	}
	return NbHits;
}

static Quat ShortestRotation(const Point& v0, const Point& v1)
{
	const float d = v0|v1;
	const Point cross = v0^v1;

	Quat q = d>-1.0f ? Quat(1.0f + d, cross.x, cross.y, cross.z)
					: fabsf(v0.x)<0.1f ? Quat(0.0f, 0.0f, v0.z, -v0.y) : Quat(0.0f, v0.y, -v0.x, 0.0f);

	q.Normalize();

	return q;
}

udword Bullet::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	ASSERT(mDynamicsWorld);

	udword NbHits = 0;
	while(nb--)
	{
		const Point Center = (sweeps->mCapsule.mP0 + sweeps->mCapsule.mP1)*0.5f;
		Point CapsuleAxis = sweeps->mCapsule.mP1 - sweeps->mCapsule.mP0;
		const float Height = CapsuleAxis.Magnitude();
		CapsuleAxis /= Height;
//		const Quat q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), CapsuleAxis);
		const Quat q = ShortestRotation(Point(0.0f, 1.0f, 0.0f), CapsuleAxis);

		const btCapsuleShape CapsuleShape(sweeps->mCapsule.mRadius, Height);

		const btQuaternion Rot = ToBtQuaternion(q);

		const btTransform from(Rot, ToBtVector3(Center));
		const btTransform to(Rot, ToBtVector3(Center + sweeps->mDir * sweeps->mMaxDist));

//		btCollisionWorld::ClosestConvexResultCallback resultCallback(from.getOrigin(), to.getOrigin());
		MyClosestConvexResultCallback resultCallback(from.getOrigin(), to.getOrigin());

		mDynamicsWorld->convexSweepTest(&CapsuleShape, from, to, resultCallback);

		if(resultCallback.m_hitCollisionObject)
		{
			NbHits++;
			FillResultStruct(*dest, resultCallback, sweeps->mMaxDist);
		}
		else
		{
			dest->SetNoHit();
		}

		sweeps++;
		dest++;
	}
	return NbHits;
}

PintConvexHandle Bullet::CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index)
{
	btConvexHullShape* shape = new btConvexHullShape(&desc.mVerts->x, desc.mNbVerts, sizeof(Point));
	ASSERT(shape);

//	if(desc.mRenderer)
//		shape->setUserPointer(desc.mRenderer);

	const udword CurrentSize = mConvexObjects.size();
	mConvexObjects.push_back(shape);
	if(index)
		*index = CurrentSize;
	return PintConvexHandle(shape);
}

udword Bullet::BatchConvexSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)
{
	ASSERT(mDynamicsWorld);

	udword NbHits = 0;
	while(nb--)
	{
		btConvexHullShape* CapsuleShape = mConvexObjects[sweeps->mConvexObjectIndex];

		const Point& Center = sweeps->mTransform.mPos;
		const Quat& q = sweeps->mTransform.mRot;

/*		const Point Center = (sweeps->mCapsule.mP0 + sweeps->mCapsule.mP1)*0.5f;
		Point CapsuleAxis = sweeps->mCapsule.mP1 - sweeps->mCapsule.mP0;
		const float Height = CapsuleAxis.Magnitude();
		CapsuleAxis /= Height;
//		const Quat q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), CapsuleAxis);
		const Quat q = ShortestRotation(Point(0.0f, 1.0f, 0.0f), CapsuleAxis);

		const btCapsuleShape CapsuleShape(sweeps->mCapsule.mRadius, Height);*/

		const btQuaternion Rot = ToBtQuaternion(q);

		const btTransform from(Rot, ToBtVector3(Center));
		const btTransform to(Rot, ToBtVector3(Center + sweeps->mDir * sweeps->mMaxDist));

//		btCollisionWorld::ClosestConvexResultCallback resultCallback(from.getOrigin(), to.getOrigin());
		MyClosestConvexResultCallback resultCallback(from.getOrigin(), to.getOrigin());

		mDynamicsWorld->convexSweepTest(CapsuleShape, from, to, resultCallback);

		if(resultCallback.m_hitCollisionObject)
		{
			NbHits++;
			FillResultStruct(*dest, resultCallback, sweeps->mMaxDist);
		}
		else
		{
			dest->SetNoHit();
		}

		sweeps++;
		dest++;
	}
	return NbHits;
}

void Bullet::SetGravity(const Point& gravity)
{
	ASSERT(mDynamicsWorld);
	mDynamicsWorld->setGravity(ToBtVector3(gravity));
}

PR Bullet::GetWorldTransform(PintActorHandle handle)
{
	const btRigidBody* body = (const btRigidBody*)handle;

//	body->getCenterOfMassTransform()

	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	return ToPR(trans);
}

void Bullet::SetWorldTransform(PintActorHandle handle, const PR& pose)
{
	btRigidBody* body = (btRigidBody*)handle;

	btTransform trans;
	trans.setOrigin(ToBtVector3(pose.mPos));
	trans.setRotation(ToBtQuaternion(pose.mRot));

	body->getMotionState()->setWorldTransform(trans);
}

#ifdef REMOVED
void Bullet::ApplyActionAtPoint(PintObjectHandle handle, PintActionType action_type, const Point& action, const Point& pos)
{
	btRigidBody* body = (btRigidBody*)handle;

	if(body->isStaticObject() || body->isKinematicObject())
		return;

	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
//	trans = body->getCenterOfMassTransform();

	const btVector3 rel_pos = ToBtVector3(pos) - trans.getOrigin();

	if(action_type==PINT_ACTION_FORCE)
	{
		body->applyForce(ToBtVector3(action), rel_pos);
	}
	else if(action_type==PINT_ACTION_IMPULSE)
	{
		body->applyImpulse(ToBtVector3(action), rel_pos);
	}
/*	else if(action_type==PINT_ACTION_TORQUE)
	{
		const btVector3 worldTorque = trans * ToBtVector3(action);
//		body->applyTorqueImpulse(worldTorque);	//###this makes the tank explode
//		body->applyTorque(worldTorque);			//###this almost doesn't move the tank
	}*/
	else ASSERT(0);
}
#endif

void Bullet::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
	btRigidBody* body = (btRigidBody*)handle;

	if(body->isStaticObject() || body->isKinematicObject())
		return;

	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
//	trans = body->getCenterOfMassTransform();

	const btVector3 rel_pos = ToBtVector3(world_pos) - trans.getOrigin();

	body->applyImpulse(ToBtVector3(world_impulse), rel_pos);
}

void Bullet::AddLocalTorque(PintActorHandle handle, const Point& local_torque)
{
	btRigidBody* body = (btRigidBody*)handle;

	if(body->isStaticObject() || body->isKinematicObject())
		return;

//	btTransform trans;
//	body->getMotionState()->getWorldTransform(trans);

//	const btVector3& AngularFactor = body->getAngularFactor();
//	printf("%f  | %f  | %f\n", AngularFactor.x(), AngularFactor.y(), AngularFactor.z());

//	const btVector3 WorldTorque = trans * ToBtVector3(local_torque);
//	body->applyTorqueImpulse(WorldTorque);	//###this makes the tank explode
//	body->applyTorque(WorldTorque);				//###this almost doesn't move the tank
	body->applyTorque(ToBtVector3(local_torque));	//###this almost doesn't move the tank
}

Point Bullet::GetAngularVelocity(PintActorHandle handle)
{
	const btRigidBody* body = (const btRigidBody*)handle;
	return ToPoint(body->getAngularVelocity());
}

void Bullet::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
	btRigidBody* body = (btRigidBody*)handle;
	body->setAngularVelocity(ToBtVector3(angular_velocity));
}

float Bullet::GetMass(PintActorHandle handle)
{
	const btRigidBody* body = (const btRigidBody*)handle;
	const float InvMass = body->getInvMass();
	if(InvMass==0.0f)
		return 0.0f;
	return 1.0f/InvMass;
}

Point Bullet::GetLocalInertia(PintActorHandle handle)
{
	const btRigidBody* body = (const btRigidBody*)handle;
	const btVector3& InvInertia = body->getInvInertiaDiagLocal();
	return Point(1.0f/InvInertia.x(), 1.0f/InvInertia.y(), 1.0f/InvInertia.z());
}

void Bullet::SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)
{
	for(udword i=0;i<nb_groups;i++)
	{
		const udword btGroup0 = RemapCollisionGroup(groups[i].mGroup0);
		const udword btGroup1 = RemapCollisionGroup(groups[i].mGroup1);
		ASSERT(btGroup0<32);
		ASSERT(btGroup1<32);

		udword Mask0 = mGroupMasks[btGroup0];
		Mask0 ^= 1<<btGroup1;
		mGroupMasks[btGroup0] = Mask0;

		udword Mask1 = mGroupMasks[btGroup1];
		Mask1 ^= 1<<btGroup0;
		mGroupMasks[btGroup1] = Mask0;
	}
}

bool Bullet::ReleaseObject(PintActorHandle handle)
{
	//### shapes and stuff?
	btRigidBody* body = (btRigidBody*)handle;
	mDynamicsWorld->removeRigidBody(body);
	DELETESINGLE(body);
	return true;
}

static btTransform CreateFrame(const Point& local_pivot, const Point& local_axis)
{
	btTransform frame;

	Point Right, Up;
	ComputeBasis(local_axis, Right, Up);

	frame.setOrigin(ToBtVector3(local_pivot));

	btMatrix3x3 basis;
	basis[0] = ToBtVector3(local_axis);
	basis[1] = ToBtVector3(Right);
	basis[2] = ToBtVector3(Up);
	frame.setBasis(basis);

	return frame;
}

//static const float maxMotorImpulse = FLT_MAX;
static const float maxMotorImpulse = 100.0f;

PintJointHandle Bullet::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	ASSERT(mDynamicsWorld);

	btRigidBody* body0 = (btRigidBody*)desc.mObject0;
	btRigidBody* body1 = (btRigidBody*)desc.mObject1;

	btTypedConstraint* constraint = null;

	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			const PINT_SPHERICAL_JOINT_CREATE& jc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);

			constraint = new btPoint2PointConstraint(*body0, *body1, ToBtVector3(jc.mLocalPivot0), ToBtVector3(jc.mLocalPivot1));
			ASSERT(constraint);
		}
		break;

		case PINT_JOINT_HINGE:
		{
			const PINT_HINGE_JOINT_CREATE& jc = static_cast<const PINT_HINGE_JOINT_CREATE&>(desc);

			if(1)
			{
				ASSERT(jc.mGlobalAnchor.IsNotUsed());
				ASSERT(jc.mGlobalAxis.IsNotUsed());

//				const btTransform frameInA = CreateFrame(jc.mLocalPivot0, jc.mLocalAxis0);
//				const btTransform frameInB = CreateFrame(jc.mLocalPivot1, jc.mLocalAxis1);
//				btHingeConstraint* hc = new btHingeConstraint(*body0, *body1, frameInA, frameInB, false);

				btHingeConstraint* hc = new btHingeConstraint(	*body0, *body1,
																ToBtVector3(jc.mLocalPivot0), ToBtVector3(jc.mLocalPivot1),
																ToBtVector3(jc.mLocalAxis0), ToBtVector3(jc.mLocalAxis1));
				ASSERT(hc);
				constraint = hc;

				if(jc.mMinLimitAngle!=MIN_FLOAT || jc.mMaxLimitAngle!=MAX_FLOAT)
					hc->setLimit(jc.mMinLimitAngle, jc.mMaxLimitAngle);
//				hc->setLimit(0.0f, 0.0f, 1.0f);

				if(jc.mUseMotor)
					hc->enableAngularMotor(true, -jc.mDriveVelocity, maxMotorImpulse);
			}
			else
			{
				const btTransform frameInA = CreateFrame(jc.mLocalPivot0, jc.mLocalAxis0);
				const btTransform frameInB = CreateFrame(jc.mLocalPivot1, jc.mLocalAxis1);

				btGeneric6DofConstraint* hc = new btGeneric6DofConstraint(*body0, *body1, frameInA, frameInB, false);
				ASSERT(hc);
				constraint = hc;

//				hc->setAngularLowerLimit(btVector3(jc.mMinLimitAngle,0,0));
//				hc->setAngularUpperLimit(btVector3(jc.mMaxLimitAngle,0,0));

/*				btVector3 lowerSliderLimit = btVector3(-20,0,0);
				btVector3 hiSliderLimit = btVector3(-10,0,0);
			//	btVector3 lowerSliderLimit = btVector3(-20,-5,-5);
			//	btVector3 hiSliderLimit = btVector3(-10,5,5);
				spSlider1->setLinearLowerLimit(lowerSliderLimit);
				spSlider1->setLinearUpperLimit(hiSliderLimit);
				spSlider1->setAngularLowerLimit(btVector3(0,0,0));
				spSlider1->setAngularUpperLimit(btVector3(0,0,0));*/
			}
		}
		break;

		case PINT_JOINT_FIXED:
		{
			const PINT_FIXED_JOINT_CREATE& jc = static_cast<const PINT_FIXED_JOINT_CREATE&>(desc);

			if(1)	// Emulating fixed joint with limited hinge
			{
				const Point LocalAxis(1,0,0);
				btHingeConstraint* hc = new btHingeConstraint(	*body0, *body1,
																ToBtVector3(jc.mLocalPivot0), ToBtVector3(jc.mLocalPivot1),
																ToBtVector3(LocalAxis), ToBtVector3(LocalAxis));
				ASSERT(hc);

				hc->setLimit(0.0f, 0.0f, 1.0f);

				constraint = hc;
			}
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			const PINT_PRISMATIC_JOINT_CREATE& jc = static_cast<const PINT_PRISMATIC_JOINT_CREATE&>(desc);

			const btTransform frameInA = CreateFrame(jc.mLocalPivot0.mPos, jc.mLocalAxis0);
			const btTransform frameInB = CreateFrame(jc.mLocalPivot1.mPos, jc.mLocalAxis1);

			btSliderConstraint* sc = new btSliderConstraint(	*body0, *body1,
																frameInA, frameInB,
																false);

/*			if(jc.mMinLimit<=jc.mMaxLimit)
			{
				sc->setUpperLinLimit(jc.mMaxLimit);
				sc->setLowerLinLimit(jc.mMinLimit);
			}*/

			constraint = sc;
		}
		break;
	}

	if(constraint)
	{
		mDynamicsWorld->addConstraint(constraint, true);
		mConstraints.push_back(constraint);
	}
	return PintJointHandle(constraint);
}

bool Bullet::SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)
{
	btTypedConstraint* constraint = reinterpret_cast<btTypedConstraint*>(handle);
	if(constraint->getUserConstraintType()==HINGE_CONSTRAINT_TYPE)
	{
		btHingeConstraint* hc = static_cast<btHingeConstraint*>(constraint);
		// The revolute joint only supports one float so this is going to be clumsy. We use angular.x by convention for now.
		hc->enableAngularMotor(true, -angular.x, maxMotorImpulse);
	}
	return false;
}
