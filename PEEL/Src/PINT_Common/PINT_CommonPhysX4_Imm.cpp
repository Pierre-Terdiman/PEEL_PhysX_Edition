///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "..\PINT_Common\PINT_Common.h"
#include "..\PINT_Common\PINT_CommonPhysX_FoundationAPI.h"
#include "..\PINT_Common\PINT_CommonPhysX4_Imm.h"

#include "solver/PxSolverDefs.h"

using namespace physx;
using namespace immediate;

#define SHAPE_CENTRIC

void imm_computeMassProps(MassProps& props, const PxGeometry& geometry, float mass)
{
	if(mass!=0.0f)
	{
		PxMassProperties inertia(geometry);
		inertia = inertia * (mass/inertia.mass);

if(geometry.getType()==PxGeometryType::eCAPSULE)
{
	const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.0f));
	const PxTransform localPose(q);

	inertia = PxMassProperties::sum(&inertia, &localPose, 1);

//	rotate(PxMat33(transform.q));
//	translate(transform.p);
}


		PxQuat orient;
		const PxVec3 diagInertia = PxMassProperties::getMassSpaceInertia(inertia.inertiaTensor, orient);
		//TSwap(diagInertia.x, diagInertia.y);
//		body->setMass(inertia.mass);
//		body->setCMassLocalPose(PxTransform(inertia.centerOfMass, orient));
//		body->setMassSpaceInertiaTensor(diagInertia);
		props.mInvMass = 1.0f/inertia.mass;
		props.mInvInertia.x = diagInertia.x == 0.0f ? 0.0f : 1.0f/diagInertia.x;
		props.mInvInertia.y = diagInertia.y == 0.0f ? 0.0f : 1.0f/diagInertia.y;
		props.mInvInertia.z = diagInertia.z == 0.0f ? 0.0f : 1.0f/diagInertia.z;

//		props.mInvMass = 1.0f;
//		props.mInvInertia = PxVec3(1.0f);
	}
	else
	{
		props.mInvMass = 0.0f;
		props.mInvInertia = PxVec3(0.0f);
	}
}

ImmediateScene::ImmediateScene(bool useTGS, bool usePersistency, bool batchContacts) :
	mNbStaticActors		(0),
	mNbArticulationLinks(0),
	mUseTGS				(useTGS),
	mUsePersistency		(usePersistency),
	mBatchContacts		(batchContacts)
{
	mCacheAllocator = new TestCacheAllocator;
	mConstraintAllocator = new TestConstraintAllocator;
}

ImmediateScene::~ImmediateScene()
{
	reset();

	DELETESINGLE(mConstraintAllocator);
	DELETESINGLE(mCacheAllocator);
}

void ImmediateScene::reset()
{
	mShapes.reset();
	mActorGlobalPoses.reset();
#ifdef SHAPE_CENTRIC
	mShapeBounds.reset();
	mBPEntries.reset();
#else
	mBounds.reset();
#endif
	mActors.reset();
	mTGSSolverBodyData.reset();
	mTGSSolverBodies.reset();
	mTGSSolverBodyTxInertias.reset();
	mSolverBodyData.reset();
	mSolverBodies.reset();
	mBroadphasePairs.reset();
	mContactPairs.reset();
	mContactPoints.reset();
	mSolverConstraintDesc.reset();
	mOrderedSolverConstraintDesc.reset();	// BATCH_CONTACTS
	mHeaders.reset();
	mContactForces.reset();
	mMotionLinearVelocity.reset();
	mMotionAngularVelocity.reset();

	mFilteredPairs.clear();

	const PxU32 size = mArticulations.size();
	for(PxU32 i=0;i<size;i++)	
		PxReleaseArticulation(mArticulations[i]);

	mArticulations.reset();
	mJointData.reset();			// TEST_IMMEDIATE_JOINTS
	mPersistentPairs.clear();	// WITH_PERSISTENCY
	mNbStaticActors = mNbArticulationLinks = 0;
}

ImmShapeHandle ImmediateScene::createShape(const PxGeometry& geometry, const PxTransform& localPose, void* userData)
{
	const ImmShapeHandle shapeHandle = ImmShapeHandle(size_t(mShapes.size()));
	mShapes.pushBack(ImmediateShape(geometry, localPose, userData));
	return shapeHandle;
}

#ifndef SHAPE_CENTRIC
ImmActorHandle ImmediateScene::createActor(ImmShapeHandle shape, const PxTransform& pose, PxU32 group, const MassProps* massProps, Dy::ArticulationLinkHandle link)
{
	const PxU32 id = mActors.size();
	// PT: we don't support compounds in this simple snippet. 1 actor = 1 shape/geom. 
	PX_ASSERT(mShapes.size()==id+1);
	PX_ASSERT(mActorGlobalPoses.size()==id);
#ifdef SHAPE_CENTRIC
//	PX_ASSERT(mShapeBounds.size()==id);
#else
	PX_ASSERT(mBounds.size()==id);
#endif
	const bool isStaticActor = !massProps;
	if(isStaticActor)
	{
		PX_ASSERT(!link);
		mNbStaticActors++;
	}
	else
	{
		// PT: make sure we don't create dynamic actors after static ones. We could reorganize the array but
		// in this simple snippet we just enforce the order in which actors are created.
		PX_ASSERT(!mNbStaticActors);
		if(link)
			mNbArticulationLinks++;
	}

	ImmediateActor::Type type;
	if(isStaticActor)
		type	= ImmediateActor::eSTATIC;
	else if(link)
		type	= ImmediateActor::eLINK;
	else
		type	= ImmediateActor::eDYNAMIC;

	ImmediateActor actor(type, group);
	actor.mLinearVelocity	= PxVec3(0.0f);
	actor.mAngularVelocity	= PxVec3(0.0f);
	actor.mLink				= link;
//	actor.mShapeIndex		= shape;
	actor.mNbShapes			= 1;
	actor.mShapeHandle[0]	= shape;


	if(massProps)
		actor.mMassProps	= *massProps;
	else	
	{
		actor.mMassProps.mInvMass = 0.0f;
		actor.mMassProps.mInvInertia = PxVec3(0.0f);
	}

	mActors.pushBack(actor);

	if(mUseTGS)
	{
		mTGSSolverBodyData.pushBack(PxTGSSolverBodyData());
		mTGSSolverBodies.pushBack(PxTGSSolverBodyVel());
		mTGSSolverBodyTxInertias.pushBack(PxTGSSolverBodyTxInertia());
	}
	else
	{
		mSolverBodyData.pushBack(PxSolverBodyData());
		mSolverBodies.pushBack(PxSolverBody());
	}

	mActorGlobalPoses.pushBack(pose);
#ifdef SHAPE_CENTRIC
//	mShapeBounds.pushBack(PxBounds3());
#else
	mBounds.pushBack(PxBounds3());
#endif
	return ImmActorHandle(id);
}
#endif

/*bool ImmediateScene::addShape(ImmActorHandle actorHandle, ImmShapeHandle shapeHandle)
{
	ImmediateActor& actor = getActor(actorHandle);
	PX_ASSERT(actor.mShapeIndex==ImmShapeHandle(0xffffffff));
	actor.mShapeIndex = shapeHandle;
	return true;
}*/

ImmActorHandle ImmediateScene::createActor(PxU32 nbShapes, const ImmShapeHandle* shapes, const PxTransform& pose, PxU32 group, const MassProps* massProps, Dy::ArticulationLinkHandle link)
{
	const PxU32 id = mActors.size();
	// PT: we don't support compounds in this simple snippet. 1 actor = 1 shape/geom. 
//	PX_ASSERT(mShapes.size()==id+1);
	PX_ASSERT(nbShapes<16);
	PX_ASSERT(mActorGlobalPoses.size()==id);
#ifdef SHAPE_CENTRIC
#else
	PX_ASSERT(mBounds.size()==id);
#endif

	const bool isStaticActor = !massProps;
	if(isStaticActor)
	{
		PX_ASSERT(!link);
		mNbStaticActors++;
	}
	else
	{
		// PT: make sure we don't create dynamic actors after static ones. We could reorganize the array but
		// in this simple snippet we just enforce the order in which actors are created.
		PX_ASSERT(!mNbStaticActors);
		if(link)
			mNbArticulationLinks++;
	}

	ImmediateActor::Type type;
	if(isStaticActor)
		type	= ImmediateActor::eSTATIC;
	else if(link)
		type	= ImmediateActor::eLINK;
	else
		type	= ImmediateActor::eDYNAMIC;

	ImmediateActor actor(type, group);
	actor.mLinearVelocity	= PxVec3(0.0f);
	actor.mAngularVelocity	= PxVec3(0.0f);
	actor.mExternalForce	= PxVec3(0.0f);
	actor.mLink				= link;
//	actor.mShapeIndex		= shape;
	actor.mNbShapes			= nbShapes;
	for(PxU32 i=0;i<nbShapes;i++)
	{
		actor.mShapeHandle[i] = shapes[i];
#ifdef SHAPE_CENTRIC
//		mShapeBounds.pushBack(PxBounds3());
#endif
	}

	if(massProps)
		actor.mMassProps	= *massProps;
	else	
	{
		actor.mMassProps.mInvMass = 0.0f;
		actor.mMassProps.mInvInertia = PxVec3(0.0f);
	}

	mActors.pushBack(actor);

	if(mUseTGS)
	{
		mTGSSolverBodyData.pushBack(PxTGSSolverBodyData());
		mTGSSolverBodies.pushBack(PxTGSSolverBodyVel());
		mTGSSolverBodyTxInertias.pushBack(PxTGSSolverBodyTxInertia());
	}
	else
	{
		mSolverBodyData.pushBack(PxSolverBodyData());
		mSolverBodies.pushBack(PxSolverBody());
	}

	mActorGlobalPoses.pushBack(pose);
#ifndef SHAPE_CENTRIC
	mBounds.pushBack(PxBounds3());
#endif
	return ImmActorHandle(size_t(id));
}

Dy::ArticulationV* ImmediateScene::createArticulation(bool fixBase)
{
	PxFeatherstoneArticulationData data;
	data.flags = fixBase ? PxArticulationFlag::eFIX_BASE : PxArticulationFlag::Enum(0);
	return PxCreateFeatherstoneArticulation(data);
}

void ImmediateScene::addArticulationToScene(Dy::ArticulationV* art)
{
	mArticulations.pushBack(art);
}

// TEST_IMMEDIATE_JOINTS
void ImmediateScene::createSphericalJoint(ImmActorHandle id0, ImmActorHandle id1, const PxTransform& localFrame0, const PxTransform& localFrame1, const PxTransform* pose0, const PxTransform* pose1)
{
	const bool isStatic0 = getActor(id0).mType == ImmediateActor::eSTATIC;
	const bool isStatic1 = getActor(id1).mType == ImmediateActor::eSTATIC;

	MyJointData jointData;
	jointData.mActors[0]		= id0;
	jointData.mActors[1]		= id1;
	jointData.mLocalFrames[0]	= localFrame0;
	jointData.mLocalFrames[1]	= localFrame1;
	if(isStatic0)
		jointData.c2b[0]		= pose0->getInverse().transformInv(localFrame0);
	else
		jointData.c2b[0]		= localFrame0;
	if(isStatic1)
		jointData.c2b[1]		= pose1->getInverse().transformInv(localFrame1);
	else
		jointData.c2b[1]		= localFrame1;

	jointData.initInvMassScale();

	mJointData.pushBack(jointData);
	disableCollision(id0, id1);
}

void ImmediateScene::updateArticulations(float dt, const PxVec3& gravity, PxU32 nbIterPos)
{
	const PxU32 nbArticulations = mArticulations.size();
	if(mUseTGS)
	{
		const float stepDt = dt/nbIterPos;
		const float invTotalDt = 1.0f/dt;
		const float stepInvDt = 1.0f/stepDt;
		for(PxU32 i=0;i<nbArticulations;i++)
			PxComputeUnconstrainedVelocitiesTGS(mArticulations[i], gravity, stepDt, dt, stepInvDt, invTotalDt);
	}
	else
	{
		for(PxU32 i=0;i<nbArticulations;i++)
			PxComputeUnconstrainedVelocities(mArticulations[i], gravity, dt);
	}
}

void ImmediateScene::updateBounds(float boundsInflation)
{
	struct Local
	{
		static void computeShapeBounds(PxBounds3& bounds, const PxGeometry& currentGeom, const PxTransform& currentPose, float boundsInflation)
		{
			switch(currentGeom.getType())
			{
				case PxGeometryType::ePLANE:
				{
					bounds = PxBounds3::centerExtents(PxVec3(0.0f), PxVec3(10000.0f));
				}
				break;
				case PxGeometryType::eBOX:
				{
					const PxBoxGeometry& shape = static_cast<const PxBoxGeometry&>(currentGeom);
					const PxVec3 extents = shape.halfExtents + PxVec3(boundsInflation);
					bounds = PxBounds3::basisExtent(currentPose.p, PxMat33(currentPose.q), extents);
				}
				break;

				case PxGeometryType::eSPHERE:
				{
					const PxSphereGeometry& shape = static_cast<const PxSphereGeometry&>(currentGeom);
					const PxVec3 extents(shape.radius + boundsInflation);
					bounds.minimum = currentPose.p - extents;
					bounds.maximum = currentPose.p + extents;
				}
				break;

				case PxGeometryType::eCAPSULE:
				{
					const PxCapsuleGeometry& shape = static_cast<const PxCapsuleGeometry&>(currentGeom);
					const PxVec3 d = currentPose.q.getBasisVector0();
					PxVec3 extents;
					for(PxU32 ax = 0; ax<3; ax++)
						extents[ax] = PxAbs(d[ax]) * shape.halfHeight + shape.radius + boundsInflation;
					bounds.minimum = currentPose.p - extents;
					bounds.maximum = currentPose.p + extents;
				}
				break;

				case PxGeometryType::eCONVEXMESH:
				case PxGeometryType::eTRIANGLEMESH:
				case PxGeometryType::eHEIGHTFIELD:
				case PxGeometryType::eGEOMETRY_COUNT:
				case PxGeometryType::eINVALID:
					//PX_ASSERT(0);
					bounds = PxBounds3::centerExtents(PxVec3(0.0f), PxVec3(10000.0f));
				break;
			}
		}
	};

#ifdef SHAPE_CENTRIC
	mShapeBounds.reset();
	mBPEntries.reset();
#endif

	// PT: in this snippet we simply recompute all bounds each frame (i.e. even static ones)
	const PxU32 nbActors = mActors.size();
//	PxU32 k=0;
	for(PxU32 i=0;i<nbActors;i++)
	{
		const PxTransform& globalPose = mActorGlobalPoses[i];

		const PxU32 nbShapes = mActors[i].mNbShapes;
		for(PxU32 j=0;j<nbShapes;j++)
		{
			const ImmShapeHandle shapeHandle = mActors[i].mShapeHandle[j];
			const PxU32 shapeIndex = PxU32(size_t(shapeHandle));
			const ImmediateShape& currentShape = mShapes[shapeIndex];
			const PxGeometry& currentGeom = currentShape.mGeom.any();
			const PxTransform& shapeLocalPose = currentShape.mLocalPose;
			const PxTransform currentPose = globalPose * shapeLocalPose;

#ifdef SHAPE_CENTRIC
			PxBounds3 bounds;
	#ifdef PHYSX_SUPPORT_PX_GEOMETRY_QUERY_FLAGS
//			Local::computeShapeBounds(bounds, currentGeom, currentPose, boundsInflation);
			PxGeometryQuery::computeGeomBounds(bounds, currentGeom, currentPose, boundsInflation, 1.0f, PxGeometryQueryFlag::Enum(0));
//			Local::computeShapeBounds(mShapeBounds[k++], currentGeom, currentPose, boundsInflation);
	#else
			bounds = PxGeometryQuery::getWorldBounds(currentGeom, currentPose, 1.0f + boundsInflation);
	#endif
			mShapeBounds.pushBack(bounds);

			mBPEntries.pushBack(BPEntry(ImmActorHandle(size_t(i)), shapeHandle));
#else
			Local::computeShapeBounds(mBounds[i], currentGeom, currentPose, boundsInflation);
#endif
		}
	}
//	PX_ASSERT(k==mShapeBounds.size());
}

//bool CompleteBoxPruning(udword nb, const AABB* list, Container& pairs);

void ImmediateScene::broadPhase()
{
	if(0)
		return;
	// PT: in this snippet we simply do a brute-force O(n^2) broadphase between all actors

#ifdef SHAPE_CENTRIC
	mBroadphasePairs.clear();
	const PxU32 nbEntries = mBPEntries.size();
	PX_ASSERT(nbEntries==mShapeBounds.size());
	const PxBounds3* shapeBounds = mShapeBounds.begin();

	for(PxU32 i=0; i<nbEntries; i++)
	{
		const BPEntry& entry0 = mBPEntries[i];
		const ImmediateActor& actor0 = getActor(entry0.mActor);
		const ImmediateActor::Type type0 = actor0.mType;

		for(PxU32 j=i+1; j<nbEntries; j++)
		{
			const BPEntry& entry1 = mBPEntries[j];
			if(entry0.mActor==entry1.mActor)
				continue;
			const ImmediateActor& actor1 = getActor(entry1.mActor);
			const ImmediateActor::Type type1 = actor1.mType;

			// Filtering
			{
				if(type0==ImmediateActor::eSTATIC && type1==ImmediateActor::eSTATIC)
					continue;

				if(isCollisionDisabled(entry0.mActor, entry1.mActor))
					continue;
			}

			if(shapeBounds[i].intersects(shapeBounds[j]))
			{
				BPEntryPair bpp;
				bpp.mBPEntry0 = i;
				bpp.mBPEntry1 = j;
				mBroadphasePairs.pushBack(bpp);
			}
			else if(mUsePersistency)
			{
				//#### TODO: fix this!!!!!!!!!
				// ### doesn't work with shared shapes, and also shape ptrs should be sorted
				const _hashmap<ShapePair, PersistentContactPair>::Entry* e = mPersistentPairs.find(ShapePair(entry0.mShape, entry1.mShape));
				if(e)
				{
					PersistentContactPair& persistentData = const_cast<PersistentContactPair&>(e->second);
					//No collision detection performed at all so clear contact cache and friction data
					persistentData.reset();
				}
			}
		}
	}
#else
	mBroadphasePairs.clear();
	const PxU32 nbActors = mActors.size();

//	Container Candidates;
//	CompleteBoxPruning(nbActors, (const AABB*)mBounds.begin(), Candidates);

//	PxU32 NbCandidates = Candidates.GetNbEntries()/2;
//	const udword* Cnds = Candidates.GetEntries();
	for(PxU32 i=0; i<nbActors; i++)
//	for(PxU32 i=0; i<NbCandidates; i++)
//	while(NbCandidates--)
	{
//		udword i = *Cnds++;
//		udword j = *Cnds++;
//		if(i>j)
//			TSwap(i,j);

		const ImmediateActor::Type type0 = mActors[i].mType;

		for(PxU32 j=i+1; j<nbActors; j++)
		{
			const ImmediateActor::Type type1 = mActors[j].mType;

			// Filtering
			{
				if(type0==ImmediateActor::eSTATIC && type1==ImmediateActor::eSTATIC)
					continue;

#ifdef REMOVED	// hardcoded in snippet
				if(mActors[i].mCollisionGroup==1 && mActors[j].mCollisionGroup==1)
					continue;
#endif

				if(isCollisionDisabled(ImmActorHandle(i), ImmActorHandle(j)))	//###TODO: revisit this
//				if(isCollisionDisabled(i, j))
					continue;

//if(type0==ImmediateActor::eLINK && type1==ImmediateActor::eLINK)
//	continue;
			}

//			ASSERT(mBounds[i].intersects(mBounds[j]));

			//###TODO: revisit this
			const ImmActorHandle h0 = ImmActorHandle(i);
			const ImmActorHandle h1 = ImmActorHandle(j);
			const ActorPair ids(h0, h1);
//			const ActorPair ids(ImmActorHandle(i), ImmActorHandle(j));	//### WTF... this one doesn't compile

			if(mBounds[i].intersects(mBounds[j]))
			{
				mBroadphasePairs.pushBack(ids);
			}
			else if(mUsePersistency)
			{
				const HashMap<ActorPair, PersistentContactPair>::Entry* e = mPersistentPairs.find(ids);
				if(e)
				{
					PersistentContactPair& persistentData = const_cast<PersistentContactPair&>(e->second);
					//No collision detection performed at all so clear contact cache and friction data
					persistentData.reset();
				}
			}
		}
	}
#endif
}

void ImmediateScene::narrowPhase(float contactDistance, float meshContactMargin, float toleranceLength, float staticFriction, float dynamicFriction, float restitution)
{
	class ContactRecorder : public PxContactRecorder
	{
		public:
						ContactRecorder(ImmediateScene* scene,
#ifdef SHAPE_CENTRIC
							const ActorPair& actorPair,
							const ShapePair& shapePair,
#else
							const ActorPair& pair,
#endif
							float staticFriction, float dynamicFriction, float restitution) :
							mScene			(scene),
#ifdef SHAPE_CENTRIC
							mActorPair		(actorPair),
							mShapePair		(shapePair),
#else
							mPair			(pair),
#endif
							mStaticFriction	(staticFriction),
							mDynamicFriction(dynamicFriction),
							mRestitution	(restitution),
							mHasContacts	(false)
							{
							}

		virtual	bool	recordContacts(const Gu::ContactPoint* contactPoints, const PxU32 nbContacts, const PxU32 /*index*/)
		{
#ifdef SHAPE_CENTRIC
			mScene->mContactPairs.pushBack(ImmediateScene::ContactPair(mActorPair, mShapePair, nbContacts, mScene->mContactPoints.size()));
#else
			mScene->mContactPairs.pushBack(ImmediateScene::ContactPair(mPair, nbContacts, mScene->mContactPoints.size()));
#endif
			mHasContacts = true;

			for(PxU32 i=0; i<nbContacts; i++)
			{
				// Fill in solver-specific data that our contact gen does not produce...
				Gu::ContactPoint point = contactPoints[i];
				point.maxImpulse		= PX_MAX_F32;
				point.targetVel			= PxVec3(0.0f);
				point.staticFriction	= mStaticFriction;
				point.dynamicFriction	= mDynamicFriction;
				point.restitution		= mRestitution;
				point.materialFlags		= 0;
				mScene->mContactPoints.pushBack(point);
			}
			return true;
		}

		ImmediateScene*	mScene;
#ifdef SHAPE_CENTRIC
		ActorPair		mActorPair;
		ShapePair		mShapePair;
#else
		ActorPair		mPair;
#endif
		float			mStaticFriction;
		float			mDynamicFriction;
		float			mRestitution;
		bool			mHasContacts;
	};

	mCacheAllocator->reset();
	mConstraintAllocator->release();
	mContactPairs.reset();
	mContactPoints.reset();

	const PxU32 nbPairs = mBroadphasePairs.size();
	for(PxU32 i=0;i<nbPairs;i++)
	{
#ifdef SHAPE_CENTRIC
		const BPEntryPair& bpPair = mBroadphasePairs[i];
		const BPEntry& bpEntry0 = mBPEntries[bpPair.mBPEntry0];
		const BPEntry& bpEntry1 = mBPEntries[bpPair.mBPEntry1];

		const ImmActorHandle actor0 = bpEntry0.mActor;
		const ImmActorHandle actor1 = bpEntry1.mActor;

		const ImmShapeHandle shape0 = bpEntry0.mShape;
		const ImmShapeHandle shape1 = bpEntry1.mShape;

		const ImmediateShape& currentShape0 = mShapes[PxU32(size_t(shape0))];
		const ImmediateShape& currentShape1 = mShapes[PxU32(size_t(shape1))];

		const PxGeometry* pxGeom0 = &currentShape0.mGeom.any();
		const PxGeometry* pxGeom1 = &currentShape1.mGeom.any();

		const PxU32 id0 = PxU32(size_t(actor0));
		const PxU32 id1 = PxU32(size_t(actor1));

		const PxTransform tr0 = mActorGlobalPoses[id0] * currentShape0.mLocalPose;
		const PxTransform tr1 = mActorGlobalPoses[id1] * currentShape1.mLocalPose;

		const ActorPair actorPair(actor0, actor1);
		const ShapePair shapePair(shape0, shape1);

		ContactRecorder contactRecorder(this, actorPair, shapePair, staticFriction, dynamicFriction, restitution);

		if(mUsePersistency)
		{
			//###TODO: fix this
			PersistentContactPair& persistentData = mPersistentPairs[shapePair];

			PxGenerateContacts(&pxGeom0, &pxGeom1, &tr0, &tr1, &persistentData.cache, 1, contactRecorder, contactDistance, meshContactMargin, toleranceLength, *mCacheAllocator);
			if(!contactRecorder.mHasContacts)
			{
				//Contact generation run but no touches found so clear cached friction data
				persistentData.frictions = NULL;
				persistentData.nbFrictions = 0;
			}
		}
		else
		{
			PxCache cache;
			PxGenerateContacts(&pxGeom0, &pxGeom1, &tr0, &tr1, &cache, 1, contactRecorder, contactDistance, meshContactMargin, toleranceLength, *mCacheAllocator);
		}
	}
#else
		const ActorPair& pair = mBroadphasePairs[i];

		const PxU32 id0 = PxU32(pair.mID0);
		const PxU32 id1 = PxU32(pair.mID1);

		const PxU32 shapeIndex0 = PxU32(mActors[id0].mShapeIndex);
		const PxU32 shapeIndex1 = PxU32(mActors[id1].mShapeIndex);

		const ImmediateShape& currentShape0 = mShapes[shapeIndex0];
		const ImmediateShape& currentShape1 = mShapes[shapeIndex1];

		const PxGeometry* pxGeom0 = &currentShape0.mGeom.any();
		const PxGeometry* pxGeom1 = &currentShape1.mGeom.any();

		const PxTransform tr0 = mActorGlobalPoses[id0] * currentShape0.mLocalPose;
		const PxTransform tr1 = mActorGlobalPoses[id1] * currentShape1.mLocalPose;

		ContactRecorder contactRecorder(this, pair, staticFriction, dynamicFriction, restitution);

		if(mUsePersistency)
		{
			PersistentContactPair& persistentData = mPersistentPairs[pair];

			PxGenerateContacts(&pxGeom0, &pxGeom1, &tr0, &tr1, &persistentData.cache, 1, contactRecorder, contactDistance, meshContactMargin, toleranceLength, *mCacheAllocator);
			if(!contactRecorder.mHasContacts)
			{
				//Contact generation run but no touches found so clear cached friction data
				persistentData.frictions = NULL;
				persistentData.nbFrictions = 0;
			}
		}
		else
		{
			PxCache cache;
			PxGenerateContacts(&pxGeom0, &pxGeom1, &tr0, &tr1, &cache, 1, contactRecorder, contactDistance, meshContactMargin, toleranceLength, *mCacheAllocator);
		}
	}
#endif

	if(0)
		printf("Narrow-phase: %d contacts    \r", mContactPoints.size());
}

void ImmediateScene::buildSolverBodyData(float dt, const PxVec3& gravity, float maxDepenetrationVelocity, float maxContactImpulse, float linearDamping, float angularDamping, float maxLinearVelocity, float maxAngularVelocity)
{
	const PxU32 nbActors = mActors.size();
	for(PxU32 i=0;i<nbActors;i++)
	{
		if(mActors[i].mType==ImmediateActor::eSTATIC)
		{
			if(mUseTGS)
				PxConstructStaticSolverBodyTGS(mActorGlobalPoses[i], mTGSSolverBodies[i], mTGSSolverBodyTxInertias[i], mTGSSolverBodyData[i]);
			else
				PxConstructStaticSolverBody(mActorGlobalPoses[i], mSolverBodyData[i]);
		}
		else if(!mActors[i].mLink)
		{
			PxRigidBodyData data;
			data.linearVelocity				= mActors[i].mLinearVelocity;
			data.angularVelocity			= mActors[i].mAngularVelocity;
			data.invMass					= mActors[i].mMassProps.mInvMass;
			data.invInertia					= mActors[i].mMassProps.mInvInertia;
			data.body2World					= mActorGlobalPoses[i];
			data.maxDepenetrationVelocity	= maxDepenetrationVelocity;
			data.maxContactImpulse			= maxContactImpulse;
			data.linearDamping				= linearDamping;
			data.angularDamping				= angularDamping;
			data.maxLinearVelocitySq		= maxLinearVelocity*maxLinearVelocity;
			data.maxAngularVelocitySq		= maxAngularVelocity*maxAngularVelocity;

			const PxVec3 totalForce = mActors[i].mExternalForce + gravity;
			mActors[i].mExternalForce = PxVec3(0.0f);

//			printf("%f %f %f\n", totalForce.x, totalForce.y, totalForce.z);

			if(mUseTGS)
				PxConstructSolverBodiesTGS(&data, &mTGSSolverBodies[i], &mTGSSolverBodyTxInertias[i], &mTGSSolverBodyData[i], 1, totalForce, dt);
			else
				PxConstructSolverBodies(&data, &mSolverBodyData[i], 1, totalForce, dt);
		}
	}
}

template<const bool aorb>
static PX_FORCE_INLINE void setupBodyPtr(PxSolverConstraintDesc& desc, PxTGSSolverBodyVel* solverBody)
{
	if(!aorb)
	{
		desc.tgsBodyA	= solverBody;
		desc.linkIndexA	= IMM_LINK_INDEX;
	}
	else
	{
		desc.tgsBodyB	= solverBody;
		desc.linkIndexB	= IMM_LINK_INDEX;
	}
}

template<const bool aorb>
static PX_FORCE_INLINE void setupBodyPtr(PxSolverConstraintDesc& desc, PxSolverBody* solverBody)
{
	if(!aorb)
	{
		desc.bodyA		= solverBody;
		desc.linkIndexA	= IMM_LINK_INDEX;
	}
	else
	{
		desc.bodyB		= solverBody;
		desc.linkIndexB	= IMM_LINK_INDEX;
	}
}

template<class T, const bool aorb>
static void setupConstraintDesc(PxSolverConstraintDesc& desc, const ImmediateActor* actors, T* solverBodies, ImmActorHandle handle)
{
	const PxU32 id = PxU32(size_t(handle));	//###TODO: revisit this

	if(!aorb)
		desc.bodyADataIndex	= id;
	else
		desc.bodyBDataIndex	= id;

	Dy::ArticulationLinkHandle link = actors[id].mLink;
	if(link)
	{
		if(!aorb)
		{
			desc.articulationA	= PxGetLinkArticulation(link);
			desc.linkIndexA		= PxU16(PxGetLinkIndex(link));	// ### PxTo8
		}
		else
		{
			desc.articulationB	= PxGetLinkArticulation(link);
			desc.linkIndexB		= PxU16(PxGetLinkIndex(link));	// ### PxTo8
		}
	}
	else
	{
		setupBodyPtr<aorb>(desc, &solverBodies[id]);
	}
}

void ImmediateScene::buildSolverConstraintDesc()
{
	const PxU32 nbContactPairs = mContactPairs.size();
	const PxU32 nbJoints = mJointData.size();	// TEST_IMMEDIATE_JOINTS
	mSolverConstraintDesc.resize(nbContactPairs+nbJoints);

	for(PxU32 i=0; i<nbContactPairs; i++)
	{
		const ContactPair& pair = mContactPairs[i];
		PxSolverConstraintDesc& desc = mSolverConstraintDesc[i];

		if(mUseTGS)
		{
#ifdef SHAPE_CENTRIC
			setupConstraintDesc<PxTGSSolverBodyVel, false>(desc, mActors.begin(), mTGSSolverBodies.begin(), pair.mActorPair.mID0);
			setupConstraintDesc<PxTGSSolverBodyVel, true>(desc, mActors.begin(), mTGSSolverBodies.begin(), pair.mActorPair.mID1);
#else
			setupConstraintDesc<PxTGSSolverBodyVel, false>(desc, mActors.begin(), mTGSSolverBodies.begin(), pair.mID0);
			setupConstraintDesc<PxTGSSolverBodyVel, true>(desc, mActors.begin(), mTGSSolverBodies.begin(), pair.mID1);
#endif
		}
		else
		{
#ifdef SHAPE_CENTRIC
			setupConstraintDesc<PxSolverBody, false>(desc, mActors.begin(), mSolverBodies.begin(), pair.mActorPair.mID0);
			setupConstraintDesc<PxSolverBody, true>(desc, mActors.begin(), mSolverBodies.begin(), pair.mActorPair.mID1);
#else
			setupConstraintDesc<PxSolverBody, false>(desc, mActors.begin(), mSolverBodies.begin(), pair.mID0);
			setupConstraintDesc<PxSolverBody, true>(desc, mActors.begin(), mSolverBodies.begin(), pair.mID1);
#endif
		}

		//Cache pointer to our contact data structure and identify which type of constraint this is. We'll need this later after batching.
		//If we choose not to perform batching and instead just create a single header per-pair, then this would not be necessary because
		//the constraintDescs would not have been reordered
		desc.constraint				= reinterpret_cast<PxU8*>(const_cast<ContactPair*>(&pair));
		desc.constraintLengthOver16	= PxSolverConstraintDesc::eCONTACT_CONSTRAINT;
	}

	// TEST_IMMEDIATE_JOINTS
	for(PxU32 i=0; i<nbJoints; i++)
	{
		const MyJointData& jointData = mJointData[i];
		PxSolverConstraintDesc& desc = mSolverConstraintDesc[nbContactPairs+i];

		const ImmActorHandle id0 = jointData.mActors[0];
		const ImmActorHandle id1 = jointData.mActors[1];

		if(mUseTGS)
		{
			setupConstraintDesc<PxTGSSolverBodyVel, false>(desc, mActors.begin(), mTGSSolverBodies.begin(), id0);
			setupConstraintDesc<PxTGSSolverBodyVel, true>(desc, mActors.begin(), mTGSSolverBodies.begin(), id1);
		}
		else
		{
			setupConstraintDesc<PxSolverBody, false>(desc, mActors.begin(), mSolverBodies.begin(), id0);
			setupConstraintDesc<PxSolverBody, true>(desc, mActors.begin(), mSolverBodies.begin(), id1);
		}
		desc.constraint	= reinterpret_cast<PxU8*>(const_cast<MyJointData*>(&jointData));
		desc.constraintLengthOver16 = PxSolverConstraintDesc::eJOINT_CONSTRAINT;
	}
}


// LIMIT

/*static Dy::ArticulationV* createImmediateArticulation(bool fixBase, Array<Dy::ArticulationV*>& articulations)
{
	PxFeatherstoneArticulationData data;
	data.flags = fixBase ? PxArticulationFlag::eFIX_BASE : PxArticulationFlag::Enum(0);
	Dy::ArticulationV* immArt = PxCreateFeatherstoneArticulation(data);
	articulations.pushBack(immArt);
	return immArt;
}

static void setupCommonLinkData(PxFeatherstoneArticulationLinkData& data, Dy::ArticulationLinkHandle parent, const PxTransform& pose, const MassProps& massProps,
								float linearDamping, float angularDamping, float maxLinearVelocity, float maxAngularVelocity, float jointFrictionCoefficient)
{
	data.parent								= parent;
	data.pose								= pose;
	data.inverseMass						= massProps.mInvMass;
	data.inverseInertia						= massProps.mInvInertia;
	data.linearDamping						= linearDamping;
	data.angularDamping						= angularDamping;
	data.maxLinearVelocitySq				= maxLinearVelocity * maxLinearVelocity;
	data.maxAngularVelocitySq				= maxAngularVelocity * maxAngularVelocity;
	data.inboundJoint.frictionCoefficient	= jointFrictionCoefficient;
}*/

// TEST_IMMEDIATE_JOINTS
// PT: this is copied from PxExtensions, it's the solver prep function for spherical joints
static PxU32 SphericalJointSolverPrep(Px1DConstraint* constraints,
	PxVec3& body0WorldOffset,
	PxU32 /*maxConstraints*/,
	PxConstraintInvMassScale& invMassScale,
	const void* constantBlock,							  
	const PxTransform& bA2w,
	const PxTransform& bB2w,
	bool /*useExtendedLimits*/,
	PxVec3& cA2wOut, PxVec3& cB2wOut)
{
	const MyJointData& data = *reinterpret_cast<const MyJointData*>(constantBlock);

	PxTransform cA2w, cB2w;
	Ext::joint::ConstraintHelper ch(constraints, invMassScale, cA2w, cB2w, body0WorldOffset, data, bA2w, bB2w);

	if(cB2w.q.dot(cA2w.q)<0.0f)
		cB2w.q = -cB2w.q;

/*	if(data.jointFlags & PxSphericalJointFlag::eLIMIT_ENABLED)
	{
		PxQuat swing, twist;
		Ps::separateSwingTwist(cA2w.q.getConjugate() * cB2w.q, swing, twist);
		PX_ASSERT(PxAbs(swing.x)<1e-6f);

		// PT: TODO: refactor with D6 joint code
		PxVec3 axis;
		PxReal error;
		const PxReal pad = data.limit.isSoft() ? 0.0f : data.limit.contactDistance;
		const Cm::ConeLimitHelperTanLess coneHelper(data.limit.yAngle, data.limit.zAngle, pad);
		const bool active = coneHelper.getLimit(swing, axis, error);				
		if(active)
			ch.angularLimit(cA2w.rotate(axis), error, data.limit);
	}*/

	PxVec3 ra, rb;
	ch.prepareLockedAxes(cA2w.q, cB2w.q, cA2w.transformInv(cB2w.p), 7, 0, ra, rb);
	cA2wOut = ra + bA2w.p;
	cB2wOut = rb + bB2w.p;

	return ch.getCount();
}

template<const bool aorb>
static void setupDescTGS(PxTGSSolverContactDesc& contactDesc, const ImmediateActor* actors, const PxTGSSolverBodyTxInertia* txInertias, const PxTGSSolverBodyData* solverBodyData, 
	const PxTransform* poses, const ImmActorHandle handle)
{
	PxTransform& bodyFrame = aorb ? contactDesc.bodyFrame1 : contactDesc.bodyFrame0;
	PxSolverConstraintPrepDescBase::BodyState& bodyState = aorb ? contactDesc.bodyState1 : contactDesc.bodyState0;
	const PxTGSSolverBodyData*& data = aorb ? contactDesc.bodyData1 : contactDesc.bodyData0;
	const PxTGSSolverBodyTxInertia*& txI = aorb ? contactDesc.body1TxI : contactDesc.body0TxI;

	const PxU32 id = PxU32(size_t(handle));	//###TODO: revisit this

	Dy::ArticulationLinkHandle link = actors[id].mLink;
	if(link)
	{
		PxLinkData linkData;
		bool status = PxGetLinkData(link, linkData);
		PX_ASSERT(status);
		PX_UNUSED(status);

		data = NULL;
		txI = NULL;
		bodyFrame = linkData.pose;
		bodyState = PxSolverConstraintPrepDescBase::eARTICULATION;
	}
	else
	{
		data = &solverBodyData[id];
		txI = &txInertias[id];
		bodyFrame = poses[id];
		bodyState = actors[id].mType == ImmediateActor::eDYNAMIC ? PxSolverConstraintPrepDescBase::eDYNAMIC_BODY : PxSolverConstraintPrepDescBase::eSTATIC_BODY;
	}
}

template<const bool aorb>
static void setupDescPGS(PxSolverContactDesc& contactDesc, const ImmediateActor* actors, const PxSolverBodyData* solverBodyData, const ImmActorHandle handle)
{
	PxTransform& bodyFrame = aorb ? contactDesc.bodyFrame1 : contactDesc.bodyFrame0;
	PxSolverConstraintPrepDescBase::BodyState& bodyState = aorb ? contactDesc.bodyState1 : contactDesc.bodyState0;
	const PxSolverBodyData*& data = aorb ? contactDesc.data1 : contactDesc.data0;

	const PxU32 id = PxU32(size_t(handle));	//###TODO: revisit this

	Dy::ArticulationLinkHandle link = actors[id].mLink;
	if(link)
	{
		PxLinkData linkData;
		bool status = PxGetLinkData(link, linkData);
		PX_ASSERT(status);
		PX_UNUSED(status);

		data		= NULL;
		bodyFrame	= linkData.pose;
		bodyState	= PxSolverConstraintPrepDescBase::eARTICULATION;
	}
	else
	{
		data		= &solverBodyData[id];
		bodyFrame	= solverBodyData[id].body2World;
		bodyState	= actors[id].mType == ImmediateActor::eDYNAMIC ? PxSolverConstraintPrepDescBase::eDYNAMIC_BODY : PxSolverConstraintPrepDescBase::eSTATIC_BODY;
	}
}

template<const bool aorb>
static void setupJointDescTGS(PxTGSSolverConstraintPrepDesc& jointDesc, const ImmediateActor* actors, PxTGSSolverBodyTxInertia* txInertias, PxTGSSolverBodyData* solverBodyData, PxTransform* poses, const PxU32 bodyDataIndex)
{
	if(!aorb)
	{
		jointDesc.bodyData0 = &solverBodyData[bodyDataIndex];
		jointDesc.body0TxI = &txInertias[bodyDataIndex];
	}
	else
	{
		jointDesc.bodyData1 = &solverBodyData[bodyDataIndex];
		jointDesc.body1TxI = &txInertias[bodyDataIndex];
	}

	PxTransform& bodyFrame = aorb ? jointDesc.bodyFrame1 : jointDesc.bodyFrame0;
	PxSolverConstraintPrepDescBase::BodyState& bodyState = aorb ? jointDesc.bodyState1 : jointDesc.bodyState0;

	if(actors[bodyDataIndex].mLink)
	{
		PxLinkData linkData;
		bool status = PxGetLinkData(actors[bodyDataIndex].mLink, linkData);
		PX_ASSERT(status);
		PX_UNUSED(status);

		bodyFrame = linkData.pose;
		bodyState = PxSolverConstraintPrepDescBase::eARTICULATION;
	}
	else
	{
		//This may seem redundant but the bodyFrame is not defined by the bodyData object when using articulations.
		// PT: TODO: this is a bug in the immediate mode snippet
		if(actors[bodyDataIndex].mType == ImmediateActor::eSTATIC)
		{
			bodyFrame = PxTransform(PxIdentity);
			bodyState = PxSolverConstraintPrepDescBase::eSTATIC_BODY;
		}
		else
		{
			bodyFrame = poses[bodyDataIndex];
			bodyState = PxSolverConstraintPrepDescBase::eDYNAMIC_BODY;
		}
	}
}

template<const bool aorb>
static void setupJointDescPGS(PxSolverConstraintPrepDesc& jointDesc, const ImmediateActor* actors, PxSolverBodyData* solverBodyData, const PxU32 bodyDataIndex)
{
	if(!aorb)
		jointDesc.data0	= &solverBodyData[bodyDataIndex];
	else
		jointDesc.data1	= &solverBodyData[bodyDataIndex];

	PxTransform& bodyFrame = aorb ? jointDesc.bodyFrame1 : jointDesc.bodyFrame0;
	PxSolverConstraintPrepDescBase::BodyState& bodyState = aorb ? jointDesc.bodyState1 : jointDesc.bodyState0;

	if(actors[bodyDataIndex].mLink)
	{
		PxLinkData linkData;
		bool status = PxGetLinkData(actors[bodyDataIndex].mLink, linkData);
		PX_ASSERT(status);
		PX_UNUSED(status);

		bodyFrame	= linkData.pose;
		bodyState	= PxSolverConstraintPrepDescBase::eARTICULATION;
	}
	else
	{
		//This may seem redundant but the bodyFrame is not defined by the bodyData object when using articulations.
		// PT: TODO: this is a bug in the immediate mode snippet
		if(actors[bodyDataIndex].mType == ImmediateActor::eSTATIC)
		{
			bodyFrame	= PxTransform(PxIdentity);
			bodyState	= PxSolverConstraintPrepDescBase::eSTATIC_BODY;
		}
		else
		{
			bodyFrame	= solverBodyData[bodyDataIndex].body2World;
			bodyState	= PxSolverConstraintPrepDescBase::eDYNAMIC_BODY;
		}
	}
}

template<class T>
void ImmediateScene::processContactHeader(PxConstraintBatchHeader& header, PxSolverConstraintDesc* orderedDescs, float invDt, float stepInvDt, float bounceThreshold, float frictionOffsetThreshold, float correlationDistance)
{
	struct Local
	{
		static void setupContactDesc(PxTGSSolverContactDesc& contactDesc, const PxSolverConstraintDesc& constraintDesc, const ContactPair& pair, const ImmediateActor* actors,
			const PxSolverBodyData* solverBodyData,
			const PxTGSSolverBodyData* tgsSolverBodyData, const PxTGSSolverBodyTxInertia* inertias, const PxTransform* actorGlobalPoses
			)
		{
#ifdef SHAPE_CENTRIC
			setupDescTGS<false>(contactDesc, actors, inertias, tgsSolverBodyData, actorGlobalPoses, pair.mActorPair.mID0);
			setupDescTGS<true>(contactDesc, actors, inertias, tgsSolverBodyData, actorGlobalPoses, pair.mActorPair.mID1);
#else
			setupDescTGS<false>(contactDesc, actors, inertias, tgsSolverBodyData, actorGlobalPoses, pair.mID0);
			setupDescTGS<true>(contactDesc, actors, inertias, tgsSolverBodyData, actorGlobalPoses, pair.mID1);
#endif
			contactDesc.body0 = constraintDesc.tgsBodyA;
			contactDesc.body1 = constraintDesc.tgsBodyB;

			contactDesc.torsionalPatchRadius	= 0.0f;
			contactDesc.minTorsionalPatchRadius	= 0.0f;
		}

		static void setupContactDesc(PxSolverContactDesc& contactDesc, const PxSolverConstraintDesc& constraintDesc, const ContactPair& pair, const ImmediateActor* actors,
			const PxSolverBodyData* solverBodyData,
			const PxTGSSolverBodyData* tgsSolverBodyData, const PxTGSSolverBodyTxInertia* inertias, const PxTransform* actorGlobalPoses
			)
		{
#ifdef SHAPE_CENTRIC
			setupDescPGS<false>(contactDesc, actors, solverBodyData, pair.mActorPair.mID0);
			setupDescPGS<true>(contactDesc, actors, solverBodyData, pair.mActorPair.mID1);
#else
			setupDescPGS<false>(contactDesc, actors, solverBodyData, pair.mID0);
			setupDescPGS<true>(contactDesc, actors, solverBodyData, pair.mID1);
#endif
			contactDesc.body0 = constraintDesc.bodyA;
			contactDesc.body1 = constraintDesc.bodyB;
		}

		static PX_FORCE_INLINE void createContactConstraints(TestConstraintAllocator* allocator, PxTGSSolverContactDesc* contactDescs, PxConstraintBatchHeader& header, float invDt, float stepInvDt, float bounceThreshold, float frictionOffsetThreshold, float correlationDistance)
		{
			PxCreateContactConstraintsTGS(&header, 1, contactDescs, *allocator, stepInvDt, invDt, bounceThreshold, frictionOffsetThreshold, correlationDistance);
		}

		static PX_FORCE_INLINE void createContactConstraints(TestConstraintAllocator* allocator, PxSolverContactDesc* contactDescs, PxConstraintBatchHeader& header, float invDt, float stepInvDt, float bounceThreshold, float frictionOffsetThreshold, float correlationDistance)
		{
			PxCreateContactConstraints(&header, 1, contactDescs, *allocator, invDt, bounceThreshold, frictionOffsetThreshold, correlationDistance);
		}
	};

	T contactDescs[4];

	PersistentContactPair* persistentPairs[4];	// WITH_PERSISTENCY

	for(PxU32 a=0; a<header.stride; a++)
	{
		PxSolverConstraintDesc& constraintDesc = orderedDescs[header.startIndex + a];
		if(!constraintDesc.constraint)
			continue;

		//Extract the contact pair that we saved in this structure earlier.
		const ContactPair& pair = *reinterpret_cast<const ContactPair*>(constraintDesc.constraint);

		T& contactDesc = contactDescs[a];

		Local::setupContactDesc(contactDesc, constraintDesc, pair, mActors.begin(),
			mSolverBodyData.begin(),
			mTGSSolverBodyData.begin(), mTGSSolverBodyTxInertias.begin(), mActorGlobalPoses.begin());

		contactDesc.contactForces	= &mContactForces[pair.mStartContactIndex];
		contactDesc.contacts		= &mContactPoints[pair.mStartContactIndex];
		contactDesc.numContacts		= pair.mNbContacts;

		if(mUsePersistency)
		{
#ifdef SHAPE_CENTRIC
			const _hashmap<ShapePair, PersistentContactPair>::Entry* e = mPersistentPairs.find(pair.mShapePair);
#else
			const _hashmap<ActorPair, PersistentContactPair>::Entry* e = mPersistentPairs.find(pair);
#endif
			PX_ASSERT(e);
			{
				PersistentContactPair& pcp = const_cast<PersistentContactPair&>(e->second);
				contactDesc.frictionPtr		= pcp.frictions;
				contactDesc.frictionCount	= PxU8(pcp.nbFrictions);
				persistentPairs[a]			= &pcp;
			}
		}
		else
		{
			contactDesc.frictionPtr			= NULL;
			contactDesc.frictionCount		= 0;
		}
		contactDesc.disableStrongFriction	= false;
		contactDesc.hasMaxImpulse			= false;
		contactDesc.hasForceThresholds		= false;
		contactDesc.shapeInteraction		= NULL;
		contactDesc.restDistance			= 0.0f;
		contactDesc.maxCCDSeparation		= PX_MAX_F32;

		contactDesc.desc					= &constraintDesc;
		contactDesc.invMassScales.angular0 = contactDesc.invMassScales.angular1 = contactDesc.invMassScales.linear0 = contactDesc.invMassScales.linear1 = 1.0f;
	}

	Local::createContactConstraints(mConstraintAllocator, contactDescs, header, invDt, stepInvDt, bounceThreshold, frictionOffsetThreshold, correlationDistance);

	if(mUsePersistency)
	{
		//Cache friction information...
		for (PxU32 a = 0; a < header.stride; ++a)
		{
			const T& contactDesc = contactDescs[a];
			PersistentContactPair& pcp = *persistentPairs[a];
			pcp.frictions = contactDesc.frictionPtr;
			pcp.nbFrictions = contactDesc.frictionCount;
		}
	}
}

template<class T>
void ImmediateScene::processJointHeader(PxConstraintBatchHeader& header, PxSolverConstraintDesc* orderedDescs, float dt, float invDt, float stepDt, float stepInvDt, float lengthScale)
{
	struct Local
	{
		static void setupJointDesc(PxTGSSolverConstraintPrepDesc& jointDesc, PxSolverConstraintDesc& constraintDesc, const ImmediateActor* actors,
			PxSolverBodyData* solverBodyData,
			PxTGSSolverBodyData* tgsSolverBodyData, PxTGSSolverBodyTxInertia* inertias, PxTransform* actorGlobalPoses
			)
		{
			jointDesc.body0 = constraintDesc.tgsBodyA;
			jointDesc.body1 = constraintDesc.tgsBodyB;
			setupJointDescTGS<false>(jointDesc, actors, inertias, tgsSolverBodyData, actorGlobalPoses, constraintDesc.bodyADataIndex);
			setupJointDescTGS<true>(jointDesc, actors, inertias, tgsSolverBodyData, actorGlobalPoses, constraintDesc.bodyBDataIndex);
		}

		static void setupJointDesc(PxSolverConstraintPrepDesc& jointDesc, PxSolverConstraintDesc& constraintDesc, const ImmediateActor* actors,
			PxSolverBodyData* solverBodyData,
			PxTGSSolverBodyData* tgsSolverBodyData, PxTGSSolverBodyTxInertia* inertias, PxTransform* actorGlobalPoses
			)
		{
			jointDesc.body0 = constraintDesc.bodyA;
			jointDesc.body1 = constraintDesc.bodyB;
			setupJointDescPGS<false>(jointDesc, actors, solverBodyData, constraintDesc.bodyADataIndex);
			setupJointDescPGS<true>(jointDesc, actors, solverBodyData, constraintDesc.bodyBDataIndex);
		}

		static PX_FORCE_INLINE void createJointConstraints(TestConstraintAllocator* allocator, PxConstraintBatchHeader& header, PxImmediateConstraint* constraints, PxTGSSolverConstraintPrepDesc* jointDescs, float dt, float invDt, float stepDt, float stepInvDt, float lengthScale)
		{
			PxCreateJointConstraintsWithImmediateShadersTGS(&header, 1, constraints, jointDescs, *allocator, stepDt, dt, stepInvDt, invDt, lengthScale);
		}

		static PX_FORCE_INLINE void createJointConstraints(TestConstraintAllocator* allocator, PxConstraintBatchHeader& header, PxImmediateConstraint* constraints, PxSolverConstraintPrepDesc* jointDescs, float dt, float invDt, float stepDt, float stepInvDt, float lengthScale)
		{
			PxCreateJointConstraintsWithImmediateShaders(&header, 1, constraints, jointDescs, *allocator, dt, invDt);
		}
	};

	T jointDescs[4];
	PxImmediateConstraint constraints[4];

	header.startIndex += mContactPairs.size();

	for(PxU32 a=0; a<header.stride; a++)
	{
		PxSolverConstraintDesc& constraintDesc = orderedDescs[header.startIndex + a];
		//Extract the contact pair that we saved in this structure earlier.
		const MyJointData& jd = *reinterpret_cast<const MyJointData*>(constraintDesc.constraint);

		constraints[a].prep = SphericalJointSolverPrep;
		constraints[a].constantBlock = &jd;

		T& jointDesc = jointDescs[a];

		Local::setupJointDesc(jointDesc, constraintDesc, mActors.begin(),
			mSolverBodyData.begin(),
			mTGSSolverBodyData.begin(), mTGSSolverBodyTxInertias.begin(), mActorGlobalPoses.begin());

		jointDesc.desc					= &constraintDesc;
		jointDesc.writeback				= NULL;
		jointDesc.linBreakForce			= PX_MAX_F32;
		jointDesc.angBreakForce			= PX_MAX_F32;
		jointDesc.minResponseThreshold	= 0;
		jointDesc.disablePreprocessing	= false;
		jointDesc.improvedSlerp			= false;
		jointDesc.driveLimitsAreForces	= false;
		jointDesc.invMassScales.angular0 = jointDesc.invMassScales.angular1 = jointDesc.invMassScales.linear0 = jointDesc.invMassScales.linear1 = 1.0f;
	}

	Local::createJointConstraints(mConstraintAllocator, header, constraints, jointDescs, dt, invDt, stepDt, stepInvDt, lengthScale);
}

void ImmediateScene::createContactConstraints(float dt, float invDt, float bounceThreshold, float frictionOffsetThreshold, float correlationDistance,
float lengthScale, const PxU32 nbPosIterations)
{
	const float stepDt = dt/float(nbPosIterations);
	const float stepInvDt = invDt*float(nbPosIterations);

	/*const*/ PxU32 nbContactHeaders;
	/*const*/ PxU32 nbJointHeaders;
	PxSolverConstraintDesc* orderedDescs;
	if(mBatchContacts)
	{
		mHeaders.resize(mSolverConstraintDesc.size());

		const PxU32 nbBodies = mActors.size() - mNbStaticActors;

		mOrderedSolverConstraintDesc.resize(mSolverConstraintDesc.size());
		orderedDescs = mOrderedSolverConstraintDesc.begin();

		if(mUseTGS)
		{
			nbContactHeaders = PxBatchConstraintsTGS(	mSolverConstraintDesc.begin(), mContactPairs.size(), mTGSSolverBodies.begin(), nbBodies,
														mHeaders.begin(), orderedDescs,
														mArticulations.begin(), mArticulations.size());

			//2 batch the joints...
			nbJointHeaders = PxBatchConstraintsTGS(	mSolverConstraintDesc.begin() + mContactPairs.size(), mJointData.size(), mTGSSolverBodies.begin(), nbBodies,
													mHeaders.begin() + nbContactHeaders, orderedDescs + mContactPairs.size(),
													mArticulations.begin(), mArticulations.size());
		}
		else
		{
			//1 batch the contacts
			nbContactHeaders = PxBatchConstraints(	mSolverConstraintDesc.begin(), mContactPairs.size(), mSolverBodies.begin(), nbBodies,
													mHeaders.begin(), orderedDescs,
													mArticulations.begin(), mArticulations.size());

			//2 batch the joints...
			nbJointHeaders = PxBatchConstraints(	mSolverConstraintDesc.begin() + mContactPairs.size(), mJointData.size(), mSolverBodies.begin(), nbBodies,
													mHeaders.begin() + nbContactHeaders, orderedDescs + mContactPairs.size(),
													mArticulations.begin(), mArticulations.size());
		}

		const PxU32 totalHeaders = nbContactHeaders + nbJointHeaders;

		mHeaders.forceSize_Unsafe(totalHeaders);
	}
	else
	{
		orderedDescs = mSolverConstraintDesc.begin();

		nbContactHeaders = mContactPairs.size();

		nbJointHeaders = mJointData.size();	// TEST_IMMEDIATE_JOINTS
		PX_ASSERT(nbContactHeaders+nbJointHeaders==mSolverConstraintDesc.size());
		mHeaders.resize(nbContactHeaders+nbJointHeaders);
		
		// We are bypassing the constraint batching so we create dummy PxConstraintBatchHeaders
		for(PxU32 i=0; i<nbContactHeaders; i++)
		{
			PxConstraintBatchHeader& hdr = mHeaders[i];
			hdr.startIndex = i;
			hdr.stride = 1;
			hdr.constraintType = PxSolverConstraintDesc::eCONTACT_CONSTRAINT;
		}

		// TEST_IMMEDIATE_JOINTS
		for(PxU32 i=0; i<nbJointHeaders; i++)
		{
			PxConstraintBatchHeader& hdr = mHeaders[nbContactHeaders+i];
			hdr.startIndex = i;
			hdr.stride = 1;
			hdr.constraintType = PxSolverConstraintDesc::eJOINT_CONSTRAINT;
		}
	}

	mContactForces.resize(mContactPoints.size());

	for(PxU32 i=0; i<nbContactHeaders; i++)
	{
		PxConstraintBatchHeader& header = mHeaders[i];
		PX_ASSERT(header.constraintType == PxSolverConstraintDesc::eCONTACT_CONSTRAINT);

		if(mUseTGS)
			processContactHeader<PxTGSSolverContactDesc>(header, orderedDescs, invDt, stepInvDt, bounceThreshold, frictionOffsetThreshold, correlationDistance);
		else
			processContactHeader<PxSolverContactDesc>(header, orderedDescs, invDt, stepInvDt, bounceThreshold, frictionOffsetThreshold, correlationDistance);
	}

	// TEST_IMMEDIATE_JOINTS
	for(PxU32 i=0; i<nbJointHeaders; i++)
	{
		PxConstraintBatchHeader& header = mHeaders[nbContactHeaders+i];
		PX_ASSERT(header.constraintType == PxSolverConstraintDesc::eJOINT_CONSTRAINT);

		if(mUseTGS)
			processJointHeader<PxTGSSolverConstraintPrepDesc>(header, orderedDescs, dt, invDt, stepDt, stepInvDt, lengthScale);
		else
			processJointHeader<PxSolverConstraintPrepDesc>(header, orderedDescs, dt, invDt, stepDt, stepInvDt, lengthScale);
	}
}

void ImmediateScene::solveAndIntegrate(float dt, PxU32 nbIterPos, PxU32 nbIterVel)
{
#ifdef PRINT_TIMINGS
	unsigned long long time0 = __rdtsc();
#endif
	const PxU32 totalNbActors = mActors.size();
	const PxU32 nbDynamicActors = totalNbActors - mNbStaticActors - mNbArticulationLinks;
	const PxU32 nbDynamic = nbDynamicActors + mNbArticulationLinks;

	mMotionLinearVelocity.resize(nbDynamic);
	mMotionAngularVelocity.resize(nbDynamic);

//	PxMemZero(mSolverBodies.begin(), mSolverBodies.size() * sizeof(PxSolverBody));

	const PxU32 nbArticulations = mArticulations.size();
	Dy::ArticulationV** articulations = mArticulations.begin();

	if(mUseTGS)
	{
		const float stepDt = dt/float(nbIterPos);

		PxSolveConstraintsTGS(mHeaders.begin(), mHeaders.size(),
			mBatchContacts ? mOrderedSolverConstraintDesc.begin() : mSolverConstraintDesc.begin(),
			mTGSSolverBodies.begin(), mTGSSolverBodyTxInertias.begin(),
			nbDynamic, nbIterPos, nbIterVel, stepDt, 1.0f / stepDt, nbArticulations, articulations);
	}
	else
	{
		PxMemZero(mSolverBodies.begin(), mSolverBodies.size() * sizeof(PxSolverBody));

		PxSolveConstraints(	mHeaders.begin(), mHeaders.size(),
							mBatchContacts ? mOrderedSolverConstraintDesc.begin() : mSolverConstraintDesc.begin(),
							mSolverBodies.begin(),
							mMotionLinearVelocity.begin(), mMotionAngularVelocity.begin(), nbDynamic, nbIterPos, nbIterVel,
							dt, 1.0f/dt, nbArticulations, articulations);
	}

#ifdef PRINT_TIMINGS
	unsigned long long time1 = __rdtsc();
#endif

	if(mUseTGS)
	{
		PxIntegrateSolverBodiesTGS(mTGSSolverBodies.begin(), mTGSSolverBodyTxInertias.begin(), mActorGlobalPoses.begin(), nbDynamicActors, dt);
		for (PxU32 i = 0; i<nbArticulations; i++)
			PxUpdateArticulationBodiesTGS(articulations[i], dt);

		for (PxU32 i = 0; i<nbDynamicActors; i++)
		{
			PX_ASSERT(mActors[i].mType == ImmediateActor::eDYNAMIC);
			const PxTGSSolverBodyVel& data = mTGSSolverBodies[i];
			mActors[i].mLinearVelocity = data.linearVelocity;
			mActors[i].mAngularVelocity = data.angularVelocity;
		}
	}
	else
	{
		PxIntegrateSolverBodies(mSolverBodyData.begin(), mSolverBodies.begin(), mMotionLinearVelocity.begin(), mMotionAngularVelocity.begin(), nbDynamicActors, dt);
		for(PxU32 i=0;i<nbArticulations;i++)
			PxUpdateArticulationBodies(articulations[i], dt);

		for(PxU32 i=0;i<nbDynamicActors;i++)
		{
			PX_ASSERT(mActors[i].mType==ImmediateActor::eDYNAMIC);
			const PxSolverBodyData& data = mSolverBodyData[i];
			mActors[i].mLinearVelocity = data.linearVelocity;
			mActors[i].mAngularVelocity = data.angularVelocity;
			mActorGlobalPoses[i] = data.body2World;
		}
	}

	for(PxU32 i=0;i<mNbArticulationLinks;i++)
	{
		const PxU32 j = nbDynamicActors + i;
		PX_ASSERT(mActors[j].mType==ImmediateActor::eLINK);

		PxLinkData data;
		bool status = PxGetLinkData(mActors[j].mLink, data);
		PX_ASSERT(status);
		PX_UNUSED(status);

		mActors[j].mLinearVelocity = data.linearVelocity;
		mActors[j].mAngularVelocity = data.angularVelocity;
		mActorGlobalPoses[j] = data.pose;
	}

#ifdef PRINT_TIMINGS
	unsigned long long time2 = __rdtsc();
	printf("solve: %d           \n", (time1-time0)/1024);
	printf("integrate: %d           \n", (time2-time1)/1024);
#endif
}

bool ImmediateScene::raycastClosest(PxGeomRaycastHit& pxHit, float& minDist, PxU32 i, const PxVec3& origin, const PxVec3& dir, float dist, ImmRaycastHit& hit)
{
	const PxBounds3& bounds = mShapeBounds[i];
	const BPEntry& entry = mBPEntries[i];

	const ImmediateActor& actor = getActor(entry.mActor);
	const ImmediateShape& shape = getShape(entry.mShape);
	const PxTransform& actorPose = getActorGlobalPose(entry.mActor);
	const PxGeometry& geom = shape.mGeom.any();
	const PxTransform& shapeLocalPose = shape.mLocalPose;
	const PxTransform shapePose = actorPose * shapeLocalPose;

#ifdef PHYSX_NEW_PUBLIC_API
	if(PxGeometryQuery::raycast(origin, dir, geom, shapePose, dist, PxHitFlag::eDEFAULT, 1, &pxHit, sizeof(PxGeomRaycastHit), PxGeometryQueryFlag::Enum(0)))
#else
	if(PxGeometryQuery::raycast(origin, dir, geom, shapePose, dist, PxHitFlag::eDEFAULT, 1, &pxHit))
#endif
	{
		if(pxHit.distance<minDist)
		{
			minDist = pxHit.distance;
			hit.mTouchedActor = entry.mActor;
			hit.mTouchedShape = entry.mShape;
			hit.mPos = pxHit.position;
			hit.mNormal = pxHit.normal;
			hit.mDistance = minDist;
			hit.mFaceIndex = pxHit.faceIndex;
			return true;
		}
	}
	return false;
}

bool ImmediateScene::raycastClosest(const PxVec3& origin, const PxVec3& dir, float dist, ImmRaycastHit& hit)
{
	const PxU32 nb = mShapeBounds.size();
	PX_ASSERT(nb==mBPEntries.size());

	float minDist = MAX_FLOAT;
	hit.mTouchedActor = NULL;
	hit.mTouchedShape = NULL;

	PxGeomRaycastHit pxHit;
	for(PxU32 i=0;i<nb;i++)
	{
		raycastClosest(pxHit, minDist, i, origin, dir, dist, hit);
	}
	return minDist!=MAX_FLOAT;
}

