///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all Havok plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_Common.h"
#include "PINT_CommonHavok.h"
#include "..\PintShapeRenderer.h"

bool Havok::SetKinematicPose(PintActorHandle handle, const Point& pos)
{
	hkpRigidBody* RB = (hkpRigidBody*)handle;

	hkpKeyFrameUtility::applyHardKeyFrame(ToHkVector4(pos), hkQuaternion::getIdentity(), 60.0f, RB);
	return true;
}

bool Havok::SetKinematicPose(PintActorHandle handle, const PR& pr)
{
	hkpRigidBody* RB = (hkpRigidBody*)handle;

	hkpKeyFrameUtility::applyHardKeyFrame(ToHkVector4(pr.mPos), ToHkQuaternion(pr.mRot), 60.0f, RB);
	return true;
}

static inline_ void FillResultStruct(PintRaycastHit& hit, const hkpClosestCdPointCollector& result, float max_dist)
{
	const hkpRootCdPoint& Hit = result.getHit();
	const hkpCollidable* touched = Hit.m_rootCollidableA;
	hit.mTouchedActor	= (PintActorHandle)(hkpRigidBody*)touched->getOwner();	// ###
	hit.mTouchedShape	= PintShapeHandle(touched);
	hit.mImpact			= ToPoint(Hit.m_contact.getPosition());
	hit.mNormal			= ToPoint(Hit.m_contact.getNormal());
	hit.mDistance		= Hit.m_contact.getDistance() * max_dist;
	hit.mTriangleIndex	= INVALID_ID;
}

udword Havok::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	ASSERT(mPhysicsWorld);

	// Since we're not too concerned with perfect accuracy, we can set the early out
	// distance to give the algorithm a chance to exit more quickly:
//	mPhysicsWorld->getCollisionInput()->m_config->m_iterativeLinearCastEarlyOutDistance = 0.1f;

	hkpClosestCdPointCollector castCollector;
	udword NbHits = 0;
	while(nb--)
	{
		hkpLinearCastInput input;
		input.m_to = ToHkVector4(sweeps->mBox.mCenter + sweeps->mDir*sweeps->mMaxDist);

		const hkpBoxShape BoxShape(ToHkVector4(sweeps->mBox.mExtents), 0.0f);
		const hkTransform Pose(ToHkQuaternion(Quat(sweeps->mBox.mRot)), ToHkVector4(sweeps->mBox.mCenter));
		hkpCollidable Collidable(&BoxShape, &Pose);

		castCollector.reset();
		mPhysicsWorld->linearCast(&Collidable, input, castCollector);

		if(castCollector.hasHit())
		{
			NbHits++;
			FillResultStruct(*dest, castCollector, sweeps->mMaxDist);
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

udword Havok::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	ASSERT(mPhysicsWorld);

	hkpClosestCdPointCollector castCollector;
	udword NbHits = 0;
	while(nb--)
	{
		hkpLinearCastInput input;
		input.m_to = ToHkVector4(sweeps->mSphere.mCenter + sweeps->mDir*sweeps->mMaxDist);

		HAVOK_COLLIDABLE_FROM_SPHERE(sweeps->mSphere)

		castCollector.reset();
		mPhysicsWorld->linearCast(&Collidable, input, castCollector);

		if(castCollector.hasHit())
		{
			NbHits++;
			FillResultStruct(*dest, castCollector, sweeps->mMaxDist);
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

udword Havok::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	ASSERT(mPhysicsWorld);

	hkpClosestCdPointCollector castCollector;
	udword NbHits = 0;
	while(nb--)
	{
		const Point Center = (sweeps->mCapsule.mP0 + sweeps->mCapsule.mP1)*0.5f;

		hkpLinearCastInput input;
		input.m_to = ToHkVector4(Center + sweeps->mDir*sweeps->mMaxDist);

		HAVOK_COLLIDABLE_FROM_CAPSULE(sweeps->mCapsule, Center)

		castCollector.reset();
		mPhysicsWorld->linearCast(&Collidable, input, castCollector);

		if(castCollector.hasHit())
		{
			NbHits++;
			FillResultStruct(*dest, castCollector, sweeps->mMaxDist);
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

PintConvexHandle Havok::CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index)
{
	hkStridedVertices StridedVerts;
	StridedVerts.m_numVertices	= desc.mNbVerts;
	StridedVerts.m_striding		= sizeof(Point);
	StridedVerts.m_vertices		= &desc.mVerts->x;

//	hkpConvexVerticesShape* shape = FindConvexShape(gShareShapes, mConvexShapes, *ConvexCreate, StridedVerts);

	hkpConvexVerticesShape* shape = new hkpConvexVerticesShape(StridedVerts);
	ASSERT(shape);

//	if(desc.mRenderer)
//		shape->setUserData(hkUlong(desc.mRenderer));

	const udword CurrentSize = mConvexObjects.size();
	mConvexObjects.push_back(shape);
	if(index)
		*index = CurrentSize;
	return PintConvexHandle(shape);

/*	btConvexHullShape* shape = new btConvexHullShape(&desc.mVerts->x, desc.mNbVerts, sizeof(Point));
	ASSERT(shape);

	if(desc.mRenderer)
		shape->setUserPointer(desc.mRenderer);

	const udword CurrentSize = mConvexObjects.size();
	mConvexObjects.push_back(shape);
	return CurrentSize;*/
}

/*
			hkStridedVertices StridedVerts;
			StridedVerts.m_numVertices	= ConvexCreate->mNbVerts;
			StridedVerts.m_striding		= sizeof(Point);
			StridedVerts.m_vertices		= &ConvexCreate->mVerts->x;

			hkpConvexVerticesShape* shape = FindConvexShape(gShareShapes, mConvexShapes, *ConvexCreate, StridedVerts);
			refs.pushBack(shape);

//			sphere->addReference();
			hkpConvexTransformShape* convexTransform = new hkpConvexTransformShape(shape, LocalPose);
			ASSERT(convexTransform);
			shapeArray.pushBack(convexTransform);
*/

udword Havok::BatchConvexSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)
{
	ASSERT(mPhysicsWorld);

	hkpClosestCdPointCollector castCollector;
	udword NbHits = 0;
	while(nb--)
	{
		hkpConvexVerticesShape* ConvexShape = mConvexObjects[sweeps->mConvexObjectIndex];

		const Point& center = sweeps->mTransform.mPos;
		const Quat& q = sweeps->mTransform.mRot;

//		const Point Center = (sweeps->mCapsule.mP0 + sweeps->mCapsule.mP1)*0.5f;

		hkpLinearCastInput input;
		input.m_to = ToHkVector4(center + sweeps->mDir*sweeps->mMaxDist);

//		HAVOK_COLLIDABLE_FROM_CAPSULE(sweeps->mCapsule, Center)

//	const hkpConvexShape CapsuleShape(ToHkVector4(capsule.mP0 - center), ToHkVector4(capsule.mP1 - center), capsule.mRadius);
	const hkTransform Pose(ToHkQuaternion(q), ToHkVector4(center));
	const hkpCollidable Collidable(ConvexShape, &Pose);

		castCollector.reset();
		mPhysicsWorld->linearCast(&Collidable, input, castCollector);

		if(castCollector.hasHit())
		{
			NbHits++;
			FillResultStruct(*dest, castCollector, sweeps->mMaxDist);
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

udword Havok::BatchSphereOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)
{
	ASSERT(mPhysicsWorld);

//	hkpFirstCdBodyPairCollector collector;
	hkpFlagCdBodyPairCollector collector;

	udword NbHits = 0;
	while(nb--)
	{
		HAVOK_COLLIDABLE_FROM_SPHERE(overlaps->mSphere)

		collector.reset();
		mPhysicsWorld->getPenetrations(&Collidable, *mPhysicsWorld->getCollisionInput(), collector);
		if(collector.hasHit())
		{
			NbHits++;
			dest->mHit = true;
		}
		else
		{
			dest->mHit = false;
		}

		overlaps++;
		dest++;
	}
	return NbHits;
}

// "Those are the only 3 collectors we provide but you can create your own custom collector,
// it will just need to inherit from hkpCdBodyPairCollector and implement an addCdBodyPair(...) function."

class ObjectsBodyPairCollector : public hkpCdBodyPairCollector
{
	public:
//		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkpAllCdBodyPairCollector);

				ObjectsBodyPairCollector() : mNbHits(0)
				{
//					reset();
				}

				virtual ~ObjectsBodyPairCollector(){}

		virtual void reset()
		{
			mNbHits = 0;
//			m_hits.clear();
			hkpCdBodyPairCollector::reset();
		}

/*		inline const hkArray<hkpRootCdBodyPair>& getHits() const
		{
			return m_hits;
		}*/

		virtual void addCdBodyPair( const hkpCdBody& bodyA, const hkpCdBody& bodyB )
		{
			mNbHits++;
		}

//		hkInplaceArray<hkpRootCdBodyPair, 16> m_hits;
		udword	mNbHits;
};

udword Havok::BatchSphereOverlapObjects(PintSQThreadContext context, udword nb, PintOverlapObjectHit* dest, const PintSphereOverlapData* overlaps)
{
	ASSERT(mPhysicsWorld);

//	hkpFirstCdBodyPairCollector collector;
	hkpAllCdBodyPairCollector collector;
//	ObjectsBodyPairCollector collector;

	udword NbHits = 0;
	while(nb--)
	{
		HAVOK_COLLIDABLE_FROM_SPHERE(overlaps->mSphere)

		collector.reset();
		mPhysicsWorld->getPenetrations(&Collidable, *mPhysicsWorld->getCollisionInput(), collector);
		udword Nb = collector.getHits().getSize();
//		udword Nb = collector.mNbHits;
		NbHits += Nb;
		dest->mNbObjects = Nb;

		overlaps++;
		dest++;
	}
	return NbHits;
}

PR Havok::GetWorldTransform(PintActorHandle handle)
{
	const hkpRigidBody* RB = (const hkpRigidBody*)handle;

	const hkTransform& BodyPose = RB->getTransform();

	return ToPR(BodyPose);
}

void Havok::SetWorldTransform(PintActorHandle handle, const PR& pose)
{
	hkpRigidBody* RB = (hkpRigidBody*)handle;

	hkTransform BodyPose;
	BodyPose.setTranslation(ToHkVector4(pose.mPos));
	BodyPose.setRotation(ToHkQuaternion(pose.mRot));
	RB->setTransform(BodyPose);
}

/*void Havok::ApplyActionAtPoint(PintObjectHandle handle, PintActionType action_type, const Point& action, const Point& pos)
{
	hkpRigidBody* RB = (hkpRigidBody*)handle;
	if(action_type==PINT_ACTION_FORCE)
	{
		RB->applyForce(1.0f/60.0f, ToHkVector4(action), ToHkVector4(pos));
	}
	else if(action_type==PINT_ACTION_IMPULSE)
	{
		RB->applyPointImpulseAsCriticalOperation(ToHkVector4(action), ToHkVector4(pos));
	}
	else ASSERT(0);
}*/

void Havok::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
	hkpRigidBody* RB = (hkpRigidBody*)handle;
	RB->applyPointImpulseAsCriticalOperation(ToHkVector4(world_impulse), ToHkVector4(world_pos));
}

Point Havok::GetAngularVelocity(PintActorHandle handle)
{
	hkpRigidBody* RB = (hkpRigidBody*)handle;
	return ToPoint(RB->getAngularVelocity());
}

void Havok::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
	hkpRigidBody* RB = (hkpRigidBody*)handle;
	RB->setAngularVelocity(ToHkVector4(angular_velocity));
}

float Havok::GetMass(PintActorHandle handle)
{
	const hkpRigidBody* RB = (const hkpRigidBody*)handle;
	return RB->getMass();
}

Point Havok::GetLocalInertia(PintActorHandle handle)
{
	const hkpRigidBody* RB = (const hkpRigidBody*)handle;
	hkMatrix3 LocalInertia;
	RB->getInertiaLocal(LocalInertia);
	return Point(LocalInertia(0,0), LocalInertia(1,1), LocalInertia(2,2));
}








hkpSphereShape* FindSphereShape(bool share_shapes, std::vector<hkpSphereShape*>& shapes, const PINT_SPHERE_CREATE& sphere_create)
{
	const float Radius = sphere_create.mRadius;
	if(share_shapes)
	{
		const int size = shapes.size();
		for(int i=0;i<size;i++)
		{
			hkpSphereShape* CurrentShape = shapes[i];
			if(CurrentShape->getRadius()==Radius)
			{
				CurrentShape->addReference();
				return CurrentShape;
			}
		}
	}

	hkpSphereShape* shape = new hkpSphereShape(Radius);
	ASSERT(shape);
	shapes.push_back(shape);

	if(sphere_create.mRenderer)
		shape->setUserData(hkUlong(sphere_create.mRenderer));

	return shape;
}

hkpCapsuleShape* FindCapsuleShape(bool share_shapes, std::vector<InternalCapsuleShape>& shapes, const PINT_CAPSULE_CREATE& capsule_create)
{
	const float Radius = capsule_create.mRadius;
	const float HalfHeight = capsule_create.mHalfHeight;
	if(share_shapes)
	{
		const int size = shapes.size();
		for(int i=0;i<size;i++)
		{
			const InternalCapsuleShape& CurrentShape = shapes[i];
			if(CurrentShape.mRadius==Radius && CurrentShape.mHalfHeight==HalfHeight)
			{
				CurrentShape.mShape->addReference();
				return CurrentShape.mShape;
			}
		}
	}

	const hkVector4 vertexA(0.0f, HalfHeight, 0.0f);
	const hkVector4 vertexB(0.0f, -HalfHeight, 0.0f);

	hkpCapsuleShape* shape = new hkpCapsuleShape(vertexA, vertexB, capsule_create.mRadius);
	ASSERT(shape);

	InternalCapsuleShape ICS;
	ICS.mShape		= shape;
	ICS.mRadius		= Radius;
	ICS.mHalfHeight	= HalfHeight;
	shapes.push_back(ICS);

	if(capsule_create.mRenderer)
		shape->setUserData(hkUlong(capsule_create.mRenderer));

	return shape;
}

#ifndef HAVOK660
hkpCylinderShape* FindCylinderShape(bool share_shapes, std::vector<InternalCylinderShape>& shapes, const PINT_CYLINDER_CREATE& cylinder_create)
{
	const float Radius = cylinder_create.mRadius;
	const float HalfHeight = cylinder_create.mHalfHeight;
	if(share_shapes)
	{
		const int size = shapes.size();
		for(int i=0;i<size;i++)
		{
			const InternalCylinderShape& CurrentShape = shapes[i];
			if(CurrentShape.mRadius==Radius && CurrentShape.mHalfHeight==HalfHeight)
			{
				CurrentShape.mShape->addReference();
				return CurrentShape.mShape;
			}
		}
	}

	const hkVector4 vertexA(0.0f, HalfHeight, 0.0f);
	const hkVector4 vertexB(0.0f, -HalfHeight, 0.0f);

	hkpCylinderShape* shape = new hkpCylinderShape(vertexA, vertexB, cylinder_create.mRadius);
	ASSERT(shape);

	InternalCylinderShape ICS;
	ICS.mShape		= shape;
	ICS.mRadius		= Radius;
	ICS.mHalfHeight	= HalfHeight;
	shapes.push_back(ICS);

	if(cylinder_create.mRenderer)
		shape->setUserData(hkUlong(cylinder_create.mRenderer));

	return shape;
}
#endif

hkpBoxShape* FindBoxShape(bool share_shapes, std::vector<hkpBoxShape*>& shapes, const PINT_BOX_CREATE& box_create)
{
	const hkVector4 halfExtents = ToHkVector4(box_create.mExtents);
	if(share_shapes)
	{
		const int size = shapes.size();
		for(int i=0;i<size;i++)
		{
			hkpBoxShape* CurrentShape = shapes[i];
			const hkVector4& CurrentExtents = CurrentShape->getHalfExtents();
			if(		CurrentExtents(0)==halfExtents(0)
				&&	CurrentExtents(1)==halfExtents(1)
				&&	CurrentExtents(2)==halfExtents(2))
			{
				CurrentShape->addReference();
				return CurrentShape;
			}
		}
	}
	hkpBoxShape* shape = new hkpBoxShape(halfExtents, 0);
	ASSERT(shape);
	shapes.push_back(shape);

	if(box_create.mRenderer)
		shape->setUserData(hkUlong(box_create.mRenderer));

	return shape;
}

hkpConvexVerticesShape* FindConvexShape(bool share_shapes, std::vector<hkpConvexVerticesShape*>& shapes, const PINT_CONVEX_CREATE& convex_create, const hkStridedVertices& strided_verts)
{
	if(share_shapes && convex_create.mRenderer)
	{
		const int size = shapes.size();
		for(int i=0;i<size;i++)
		{
			hkpConvexVerticesShape* CurrentShape = shapes[i];
			if(CurrentShape->getUserData()==hkUlong(convex_create.mRenderer))
			{
				CurrentShape->addReference();
				return CurrentShape;
			}
		}
	}

	hkpConvexVerticesShape* shape = new hkpConvexVerticesShape(strided_verts);
	ASSERT(shape);
	shapes.push_back(shape);

	if(convex_create.mRenderer)
		shape->setUserData(hkUlong(convex_create.mRenderer));

	return shape;
}

#ifndef HAVOK660
udword Havok_GetAllocatedMemory()
{
	if(1)
	{
		hkMemoryAllocator::MemoryStatistics Stats;
		hkMallocAllocator::m_defaultMallocAllocator->getMemoryStatistics(Stats);
//		return Stats.m_inUse;
		return Stats.m_allocated;
	}

/*	if(0)
	{
		hkMemoryAllocator::MemoryStatistics Stats;
		hkMemorySystem::getInstance().getHeapStatistics(Stats);
//		return Stats.m_allocated;
		return Stats.m_inUse;
	}*/
}

static hkpShape* CreateMeshShape(const PINT_MESH_CREATE& create, HavokMeshFormat format)
{
	// ### share meshes?
	const udword NbVerts = create.mSurface.mNbVerts;
	const Point* V = create.mSurface.mVerts;

	const udword NbTris = create.mSurface.mNbFaces;
	const udword* I = create.mSurface.mDFaces;

	hkpShape* HavokShape=null;
	if(format==HAVOK_BV_COMPRESSED_MESH_SHAPE)
	{
		// "Since this shape stores all of its primitive geometries internally in a compressed format, it cannot be used to reference external geometry."
		hkGeometry geom;
		for(udword i=0;i<NbVerts;i++)
		{
//			geom.m_vertices[i] = hkVector4(V[i].x, V[i].y, V[i].z);
			geom.m_vertices.pushBack(hkVector4(V[i].x, V[i].y, V[i].z));
		}

		for(udword i=0;i<NbTris;i++)
		{
			hkGeometry::Triangle T;
			T.set(I[0], I[1], I[2], -1);
			I+=3;
//			geom.m_triangles[i] = T;
			geom.m_triangles.pushBack(T);
		}
		{
			hkpDefaultBvCompressedMeshShapeCinfo cinfo(&geom);
//			cinfo.m_maxConvexShapeError = 0.0f;
//	hkMemoryAllocator::MemoryStatistics Stats;
//	hkMemorySystem::getInstance().getHeapStatistics(Stats);
//	hkLong before = Stats.m_inUse;
//udword MemoryBefore = Havok_GetAllocatedMemory();
			HavokShape = new hkpBvCompressedMeshShape(cinfo);
//udword MemoryAfter = Havok_GetAllocatedMemory();
//printf("Mesh size: %d\n", (MemoryAfter-MemoryBefore)/1024);
//	hkMemorySystem::getInstance().getHeapStatistics(Stats);
//	hkLong after = Stats.m_inUse;
//	printf("Mesh size: %d\n", after-before);
//			hkpBvCompressedMeshShape* tmp = new hkpBvCompressedMeshShape(cinfo);
//			HavokShape = tmp;
//			int test = tmp->calcSizeForSpu(CalcSizeForSpuInput(), 1000000000);
//			hkBool b = tmp->castRay( const hkpShapeRayCastInput& input, hkpShapeRayCastOutput& results ) const;
		}
	}
	else if(format==HAVOK_EXTENDED_MESH_SHAPE)
	{
		hkpExtendedMeshShape* m_mesh = new hkpExtendedMeshShape;
//		hkpBvCompressedMeshShape* m_mesh = new hkpBvCompressedMeshShape;
		
//		hkpStorageExtendedMeshShape* m_mesh = new hkpStorageExtendedMeshShape;

		// it is common to have a landscape with 0 convex radius (for each triangle)
		// and all moving objects with non zero radius.
//		m_mesh->setRadius(0.0f);

		{
			hkpExtendedMeshShape::TrianglesSubpart part;

			part.m_vertexBase			= &V->x;
			part.m_vertexStriding		= sizeof(Point);
			part.m_numVertices			= NbVerts;

			part.m_indexBase			= I;
			part.m_indexStriding		= sizeof(udword)*3;
			part.m_numTriangleShapes	= NbTris;
			part.m_stridingType			= hkpExtendedMeshShape::INDICES_INT32;

			m_mesh->addTrianglesSubpart(part);
		}

		hkpMoppCompilerInput mci;
		hkpMoppCode* m_code = hkpMoppUtility::buildCode(m_mesh, mci);

		HavokShape = new hkpMoppBvTreeShape(m_mesh, m_code);

		m_code->removeReference();
		m_mesh->removeReference();

		// ### TODO
//		hkResult res = m_mesh->computeWeldingInfo(shape, hkpWeldingUtility::WELDING_TYPE_CLOCKWISE);
//		hkResult res = m_mesh->computeWeldingInfo(shape, hkpWeldingUtility::WELDING_TYPE_ANTICLOCKWISE);
	}
	return HavokShape;
}

hkpShape* CreateMeshShape(const PINT_MESH_CREATE& create, HavokMeshFormat format, std::vector<HavokMeshRender>& meshes, PintShapeRenderer* renderer)
{
	if(/*gShareMeshData &&*/renderer)
	{
		const udword Size = meshes.size();
		for(udword i=0;i<Size;i++)
		{
			const HavokMeshRender& CurrentMesh = meshes[i];
			if(CurrentMesh.mRenderer==renderer)
			{
				return CurrentMesh.mTriangleMesh;
			}
		}
	}

	hkpShape* S = CreateMeshShape(create, format);
	meshes.push_back(HavokMeshRender(S, renderer));

	if(create.mRenderer)
		S->setUserData(hkUlong(create.mRenderer));

	return S;
}

SharedHavok::SharedHavok(const EditableParams& params) :
	mParams			(params),
	mPhysicsWorld	(null),
	mMemoryRouter	(null),
	mStackBuffer	(null),
//	mThreadPool		(null),
	mJobQueue		(null),
	mContext		(null),
	mVdb			(null)
{
}

SharedHavok::~SharedHavok()
{
	ASSERT(!mPhysicsWorld);
	ASSERT(!mVdb);
	ASSERT(!mContext);
	ASSERT(!mJobQueue);
//	ASSERT(!mThreadPool);
	ASSERT(!mStackBuffer);
	ASSERT(!mMemoryRouter);
}

hkpWorld* SharedHavok::SetupWorld(const PINT_WORLD_CREATE& desc, hkJobQueue* job_queue)
{
	// The world cinfo contains global simulation parameters, including gravity, solver settings etc.
	hkpWorldCinfo worldInfo;

	if(desc.mGlobalBounds.IsValid())
	{
		Point m, M;
		desc.mGlobalBounds.GetMin(m);
		desc.mGlobalBounds.GetMax(M);

		worldInfo.m_broadPhaseWorldAabb = hkAabb(ToHkVector4(m), ToHkVector4(M));
	}
	else
	{
		const float Size = mParams.mGlobalBoxSize;
		worldInfo.m_broadPhaseWorldAabb = hkAabb(hkVector4(-Size, -Size, -Size), hkVector4(Size, Size, Size));
	}
	worldInfo.m_broadPhaseType = mParams.mBroadPhaseType;
	worldInfo.m_contactPointGeneration = mParams.mContactPointGeneration;

	worldInfo.m_gravity = ToHkVector4(desc.mGravity);

	worldInfo.m_solverIterations = mParams.mSolverIterationCount;

	// Set the simulation type of the world to multi-threaded.
	if(mParams.mNbThreads>=2)
	{
		worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
	}
	else
	{
		if(mParams.mUseCCD)
			worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_CONTINUOUS;
		else
			worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_DISCRETE;
	}
	if(mParams.mUseCCD)
		worldInfo.m_sizeOfToiEventQueue = mParams.mSizeOfToiEventQueue;

	// Flag objects that fall "out of the world" to be automatically removed - just necessary for this physics scene
	worldInfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_REMOVE_ENTITY;

	worldInfo.m_enableDeactivation = mParams.mEnableSleeping;

	worldInfo.m_collisionTolerance = mParams.mCollisionTolerance;

//	worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_8ITERS_HARD);

	hkpWorld* PhysicsWorld = new hkpWorld(worldInfo);

	// Disable deactivation, so that you can view timers in the VDB. This should not be done in your game.
	PhysicsWorld->m_wantDeactivation = mParams.mEnableSleeping;


	// When the simulation type is SIMULATION_TYPE_MULTITHREADED, in the debug build, the sdk performs checks
	// to make sure only one thread is modifying the world at once to prevent multithreaded bugs. Each thread
	// must call markForRead / markForWrite before it modifies the world to enable these checks.
	PhysicsWorld->markForWrite();


	// Register all collision agents, even though only box - box will be used in this particular example.
	// It's important to register collision agents before adding any entities to the world.
	hkpAgentRegisterUtil::registerAllAgents( PhysicsWorld->getCollisionDispatcher() );

	// We need to register all modules we will be running multi-threaded with the job queue
	if(job_queue)
		PhysicsWorld->registerWithJobQueue(job_queue);

	// Now we have finished modifying the world, release our write marker.
	PhysicsWorld->unmarkForWrite();

	return PhysicsWorld;
}

void SharedHavok::FillRigidBodyInfo(hkpRigidBodyCinfo& info, const PINT_OBJECT_CREATE& desc, hkpShape* shape)
{
	info.m_shape				= shape;
	info.m_position				= ToHkVector4(desc.mPosition);
	info.m_rotation				= ToHkQuaternion(desc.mRotation);
	info.m_solverDeactivation	= mParams.mSolverDeactivation;
	info.m_collisionFilterInfo	= hkpGroupFilter::calcFilterInfo(desc.mCollisionGroup);
	if(desc.mMass!=0.0f)
	{
		info.m_linearDamping	= mParams.mLinearDamping;
		info.m_angularDamping	= mParams.mAngularDamping;

		if(mParams.mUseCCD)
		{
			if(mParams.mCCDQuality==0)
				info.m_qualityType	= HK_COLLIDABLE_QUALITY_DEBRIS;
			else if(mParams.mCCDQuality==1)
				info.m_qualityType	= HK_COLLIDABLE_QUALITY_DEBRIS_SIMPLE_TOI;
			else if(mParams.mCCDQuality==2)
				info.m_qualityType	= HK_COLLIDABLE_QUALITY_MOVING;
			else if(mParams.mCCDQuality==3)
				info.m_qualityType	= HK_COLLIDABLE_QUALITY_CRITICAL;
			else if(mParams.mCCDQuality==4)
				info.m_qualityType	= HK_COLLIDABLE_QUALITY_BULLET;
		}

		if(desc.mKinematic)
			info.m_motionType	= hkpMotion::MOTION_KEYFRAMED;
	}
	else
	{
		info.m_qualityType		= HK_COLLIDABLE_QUALITY_FIXED;
		info.m_motionType		= hkpMotion::MOTION_FIXED;
	}
//	info.m_enableDeactivation	= false;
//	info.m_allowedPenetrationDepth = 1.0f;
}

hkpRigidBody* SharedHavok::CreateObject(const hkpRigidBodyCinfo& info, const PINT_OBJECT_CREATE& create, hkpShape* shape)
{
	hkpRigidBody* rigidBody = new hkpRigidBody(info);

	if(create.mAddToWorld)
	{
		mPhysicsWorld->markForWrite();
		mPhysicsWorld->addEntity(rigidBody);
		mPhysicsWorld->unmarkForWrite();
		rigidBody->removeReference();
	}

	shape->removeReference();

	if(info.m_motionType != hkpMotion::MOTION_FIXED)
	{
		rigidBody->markForWrite();
		rigidBody->setMaxLinearVelocity(mParams.mMaxLinearVelocity);
		rigidBody->setLinearVelocity(ToHkVector4(create.mLinearVelocity));
		rigidBody->setAngularVelocity(ToHkVector4(create.mAngularVelocity));

		if(create.mCOMLocalOffset.IsNonZero())
		{
			hkVector4 LocalCOM = rigidBody->getCenterOfMassLocal();
			LocalCOM.add(ToHkVector4(create.mCOMLocalOffset));
			rigidBody->setCenterOfMassLocal(LocalCOM);
		}

		rigidBody->unmarkForWrite();
	}

	mRigidBodies.Add(udword(rigidBody));

//	hkReal pd = rigidBody->getAllowedPenetrationDepth();

//	rigidBody->setDeactivator( hkpRigidBodyCinfo::DEACTIVATOR_NEVER );

	return rigidBody;
}

bool SharedHavok::ReleaseObject(PintActorHandle handle)
{
	hkpRigidBody* rigidBody = (hkpRigidBody*)handle;
	mRigidBodies.Delete(udword(handle));
		mPhysicsWorld->markForWrite();
//int test0 = rigidBody->getReferenceCount();
		hkBool Status = mPhysicsWorld->removeEntity(rigidBody);
//int test1 = rigidBody->getReferenceCount();
		ASSERT(Status);
		mPhysicsWorld->unmarkForWrite();
//	DELETESINGLE(rigidBody);
//rigidBody->removeReference();
//int test2 = rigidBody->getReferenceCount();
	return true;
}

static void SetupMaterial(hkpRigidBodyCinfo& info, const PINT_MATERIAL_CREATE* material)
{
	if(material)
	{
		info.m_friction		= material->mDynamicFriction;
		info.m_restitution	= material->mRestitution;
	}
	else
	{
		info.m_friction		= 0.5f;
		info.m_restitution	= 0.0f;
	}
}

static void SetupMassProperties(hkpRigidBodyCinfo& info, const hkpShape* shape, const PINT_OBJECT_CREATE& desc)
{
	const float M = desc.mMassForInertia<0.0f ? desc.mMass : desc.mMassForInertia;
	hkpInertiaTensorComputer::setShapeVolumeMassProperties(shape, M, info);
//	printf("Computed/desired mass: %f | %f\n", info.m_mass, desc.mMass);
	info.m_mass = desc.mMass;
}

PintActorHandle SharedHavok::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	udword NbShapes = desc.GetNbShapes();
	if(!NbShapes)
		return null;
	if(NbShapes>1)
		return CreateCompoundObject(desc);

	const bool Legacy = false;

	if(desc.mShapes->mType==PINT_SHAPE_SPHERE)
	{
		const PINT_SPHERE_CREATE* SphereCreate = static_cast<const PINT_SPHERE_CREATE*>(desc.mShapes);

		hkpSphereShape* shape = FindSphereShape(mParams.mShareShapes, mSphereShapes, *SphereCreate);

		hkpRigidBodyCinfo info;
		if(desc.mMass!=0.0f)
		{
			if(Legacy)
			{
				hkMassProperties massProperties;
				hkpInertiaTensorComputer::computeSphereVolumeMassProperties(SphereCreate->mRadius, desc.mMass, massProperties);
				info.m_mass				= massProperties.m_mass;
				info.m_centerOfMass		= massProperties.m_centerOfMass;
				info.m_inertiaTensor	= massProperties.m_inertiaTensor;
				info.m_motionType		= hkpMotion::MOTION_SPHERE_INERTIA;
			//	info.m_motionType		= hkpMotion::MOTION_BOX_INERTIA;
			}
			else
				SetupMassProperties(info, shape, desc);
		}
		FillRigidBodyInfo(info, desc, shape);
		SetupMaterial(info, SphereCreate->mMaterial);
		return PintActorHandle(CreateObject(info, desc, shape));
	}
	else if(desc.mShapes->mType==PINT_SHAPE_BOX)
	{
		const PINT_BOX_CREATE* BoxCreate = static_cast<const PINT_BOX_CREATE*>(desc.mShapes);

		const hkVector4 halfExtents = ToHkVector4(BoxCreate->mExtents);

		hkpBoxShape* shape = FindBoxShape(mParams.mShareShapes, mBoxShapes, *BoxCreate);

		hkpRigidBodyCinfo info;
//		info.m_allowedPenetrationDepth = 1.0f;
		if(desc.mMass!=0.0f)
		{
			if(Legacy)
			{
				hkMassProperties massProperties;
				hkpInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtents, desc.mMass, massProperties);
				info.m_mass				= massProperties.m_mass;
				info.m_centerOfMass		= massProperties.m_centerOfMass;
				info.m_inertiaTensor	= massProperties.m_inertiaTensor;
				info.m_motionType		= hkpMotion::MOTION_BOX_INERTIA;
	//			info.m_motionType		= hkpMotion::MOTION_SPHERE_INERTIA;
			}
			else
				SetupMassProperties(info, shape, desc);
		}
		FillRigidBodyInfo(info, desc, shape);
		SetupMaterial(info, BoxCreate->mMaterial);
		return PintActorHandle(CreateObject(info, desc, shape));
	}
	else if(desc.mShapes->mType==PINT_SHAPE_CAPSULE)
	{
		const PINT_CAPSULE_CREATE* CapsuleCreate = static_cast<const PINT_CAPSULE_CREATE*>(desc.mShapes);

		hkpCapsuleShape* shape = FindCapsuleShape(mParams.mShareShapes, mCapsuleShapes, *CapsuleCreate);

		hkpRigidBodyCinfo info;
//		info.m_allowedPenetrationDepth = 1.0f;
		if(desc.mMass!=0.0f)
		{
			if(Legacy)
			{
				const hkVector4 vectorA(0.0f, CapsuleCreate->mHalfHeight, 0.0f);
				const hkVector4 vectorB(0.0f, -CapsuleCreate->mHalfHeight, 0.0f);

				hkMassProperties massProperties;
				hkpInertiaTensorComputer::computeCapsuleVolumeMassProperties(vectorA, vectorB, CapsuleCreate->mRadius, desc.mMass, massProperties);
				info.m_mass				= massProperties.m_mass;
				info.m_centerOfMass		= massProperties.m_centerOfMass;
				info.m_inertiaTensor	= massProperties.m_inertiaTensor;
				info.m_motionType		= hkpMotion::MOTION_DYNAMIC;
	//			info.m_motionType		= hkpMotion::MOTION_THIN_BOX_INERTIA;
	//			info.m_motionType		= hkpMotion::MOTION_SPHERE_INERTIA;
			}
			else
				SetupMassProperties(info, shape, desc);
		}
		FillRigidBodyInfo(info, desc, shape);
		SetupMaterial(info, CapsuleCreate->mMaterial);
		return PintActorHandle(CreateObject(info, desc, shape));
	}
	else if(desc.mShapes->mType==PINT_SHAPE_CYLINDER)
	{
		const PINT_CYLINDER_CREATE* CylinderCreate = static_cast<const PINT_CYLINDER_CREATE*>(desc.mShapes);

		hkpCylinderShape* shape = FindCylinderShape(mParams.mShareShapes, mCylinderShapes, *CylinderCreate);

		hkpRigidBodyCinfo info;
//		info.m_allowedPenetrationDepth = 1.0f;
		if(desc.mMass!=0.0f)
			SetupMassProperties(info, shape, desc);
		FillRigidBodyInfo(info, desc, shape);
		SetupMaterial(info, CylinderCreate->mMaterial);
		return PintActorHandle(CreateObject(info, desc, shape));
	}
	else if(desc.mShapes->mType==PINT_SHAPE_CONVEX)
	{
		const PINT_CONVEX_CREATE* ConvexCreate = static_cast<const PINT_CONVEX_CREATE*>(desc.mShapes);

		hkStridedVertices StridedVerts;
		StridedVerts.m_numVertices	= ConvexCreate->mNbVerts;
		StridedVerts.m_striding		= sizeof(Point);
		StridedVerts.m_vertices		= &ConvexCreate->mVerts->x;

		hkpConvexVerticesShape* shape = FindConvexShape(mParams.mShareShapes, mConvexShapes, *ConvexCreate, StridedVerts);

		hkpRigidBodyCinfo info;
		if(desc.mMass!=0.0f)
		{
			if(Legacy)
			{
				hkMassProperties massProperties;
				hkpInertiaTensorComputer::computeConvexHullMassProperties(StridedVerts, 0.0f, massProperties);
				info.m_mass				= massProperties.m_mass;
				info.m_centerOfMass		= massProperties.m_centerOfMass;
				info.m_inertiaTensor	= massProperties.m_inertiaTensor;
				info.m_motionType		= hkpMotion::MOTION_DYNAMIC;
			}
			else
				SetupMassProperties(info, shape, desc);
		}
		FillRigidBodyInfo(info, desc, shape);
		SetupMaterial(info, ConvexCreate->mMaterial);
		return PintActorHandle(CreateObject(info, desc, shape));
	}
	else if(desc.mShapes->mType==PINT_SHAPE_MESH)
	{
		const PINT_MESH_CREATE* MeshCreate = static_cast<const PINT_MESH_CREATE*>(desc.mShapes);

		hkpShape* shape = CreateMeshShape(*MeshCreate, mParams.mMeshFormat, mMeshes, MeshCreate->mRenderer);

//		if(MeshCreate->mRenderer)
//			shape->setUserData(hkUlong(MeshCreate->mRenderer));

		hkpRigidBodyCinfo info;
		if(desc.mMass!=0.0f)
		{
			ASSERT(0);
		}
		FillRigidBodyInfo(info, desc, shape);
		SetupMaterial(info, MeshCreate->mMaterial);
		return PintActorHandle(CreateObject(info, desc, shape));
	}
	else ASSERT(0);
	return null;
}

/*
#include <Common/Internal/ConvexHull/hkGeometryUtility.h>
hkpRigidBody* CreateRandomConvexGeometricFromBox(const hkVector4& size, const hkReal mass, const hkVector4& position, const int numVertices, BasicRandom& rnd)
{
	// generate a random convex geometry
	hkArray<hkVector4> vertices;
	{
		hkVector4 halfExtents; halfExtents.setMul4( 0.5f, size );
		vertices.reserve( numVertices );
		for( int i = 0; i < numVertices; i++ )
		{
			hkVector4 xyz;
			{
				xyz(0) = rnd.randomFloat();
				xyz(1) = rnd.randomFloat();
				xyz(2) = rnd.randomFloat();
				xyz(3) = 0.0f;
			}
			xyz.normalize3();
			xyz(0) *= halfExtents(0);
			xyz(1) *= halfExtents(1);
			xyz(2) *= halfExtents(2);

			vertices.pushBack( xyz );
		}
	}

	// convert it to a convex vertices shape
	hkpConvexVerticesShape* cvs = new hkpConvexVerticesShape(vertices);
	
	hkpRigidBodyCinfo convexInfo;

	convexInfo.m_shape = cvs;
	if(mass != 0.0f)
	{
		convexInfo.m_mass = mass;
		hkpInertiaTensorComputer::setShapeVolumeMassProperties(convexInfo.m_shape, convexInfo.m_mass, convexInfo);
		convexInfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;
	}
	else
	{
		convexInfo.m_motionType = hkpMotion::MOTION_FIXED;
	}

	convexInfo.m_rotation.setIdentity();
	convexInfo.m_position = position;

	hkpRigidBody* convexRigidBody = new hkpRigidBody(convexInfo);

	cvs->removeReference();

	return convexRigidBody;
}*/

/*
static void createBrickWall( hkpWorld* world, int height, int length, const hkVector4& position, hkReal gapWidth, hkpConvexShape* box, hkVector4Parameter halfExtents )
{
	hkVector4 posx = position;
	// do a raycast to place the wall
	{
		hkpWorldRayCastInput ray;
		ray.m_from = posx;
		ray.m_to = posx;

		ray.m_from(1) += 20.0f;
		ray.m_to(1)   -= 20.0f;

		hkpWorldRayCastOutput result;
		world->castRay( ray, result );
		posx.setInterpolate4( ray.m_from, ray.m_to, result.m_hitFraction );
	}
	// move the start point
	posx(0) -= ( gapWidth + 2.0f * halfExtents(0) ) * length * 0.5f;
	posx(1) -= halfExtents(1) + box->getRadius();

	hkArray<hkpEntity*> entitiesToAdd;

	for ( int x = 0; x < length; x ++ )		// along the ground
	{
		hkVector4 pos = posx;
		for( int ii = 0; ii < height; ii++ )
		{
			pos(1) += (halfExtents(1) + box->getRadius())* 2.0f;

			hkpRigidBodyCinfo boxInfo;
			boxInfo.m_mass = 10.0f;
			hkMassProperties massProperties;
			hkpInertiaTensorComputer::computeBoxVolumeMassProperties(halfExtents, boxInfo.m_mass, massProperties);

			boxInfo.m_mass = massProperties.m_mass;
			boxInfo.m_centerOfMass = massProperties.m_centerOfMass;
			boxInfo.m_inertiaTensor = massProperties.m_inertiaTensor;
			boxInfo.m_solverDeactivation = boxInfo.SOLVER_DEACTIVATION_MEDIUM;
			boxInfo.m_shape = box;
			//boxInfo.m_qualityType = HK_COLLIDABLE_QUALITY_DEBRIS;
			boxInfo.m_restitution = 0.0f;

			boxInfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;

			{
				boxInfo.m_position = pos;
				hkpRigidBody* boxRigidBody = new hkpRigidBody(boxInfo);
				world->addEntity( boxRigidBody );
				boxRigidBody->removeReference();
			}

			pos(1) += (halfExtents(1) + box->getRadius())* 2.0f;
			pos(0) += halfExtents(0) * 0.6f;
			{
				boxInfo.m_position = pos;
				hkpRigidBody* boxRigidBody = new hkpRigidBody(boxInfo);
				entitiesToAdd.pushBack(boxRigidBody);
			}
			pos(0) -= halfExtents(0) * 0.6f;
		}
		posx(0) += halfExtents(0)* 2.0f + gapWidth;
	}
	world->addEntityBatch( entitiesToAdd.begin(), entitiesToAdd.getSize());

	for (int i=0; i < entitiesToAdd.getSize(); i++){ entitiesToAdd[i]->removeReference(); }
}

static void setupPhysics(hkpWorld* physicsWorld)
{
	//
	//  Create the ground box
	//
	{
		hkVector4 groundRadii(70.0f, 2.0f, 140.0f);
		hkpConvexShape* shape = new hkpBoxShape(groundRadii , 0);

		hkpRigidBodyCinfo ci;
		ci.m_shape			= shape;
		ci.m_motionType		= hkpMotion::MOTION_FIXED;
		ci.m_position		= hkVector4( 0.0f, -2.0f, 0.0f );
		ci.m_qualityType	= HK_COLLIDABLE_QUALITY_FIXED;

		physicsWorld->addEntity(new hkpRigidBody(ci))->removeReference();
		shape->removeReference();
	}

	hkVector4 groundPos( 0.0f, 0.0f, 0.0f );
	hkVector4 posy = groundPos;

	//
	// Create the walls
	//

	int wallHeight = 8;
	int wallWidth  = 8;
	int numWalls = 6;
	hkVector4 boxSize( 1.0f, 0.5f, 0.5f);
	hkpBoxShape* box = new hkpBoxShape( boxSize , 0 );
	box->setRadius( 0.0f );

	hkReal deltaZ = 25.0f;
	posy(2) = -deltaZ * numWalls * 0.5f;

	for ( int y = 0; y < numWalls; y ++ )			// first wall
	{
		createBrickWall( physicsWorld, wallHeight, wallWidth, posy, 0.2f, box, boxSize );
		posy(2) += deltaZ;
	}
	box->removeReference();

	//
	// Create a ball moving towards the walls
	//

	const hkReal radius = 1.5f;
	const hkReal sphereMass = 150.0f;

	hkVector4 relPos( 0.0f,radius + 0.0f, 50.0f );

	hkpRigidBodyCinfo info;
	hkMassProperties massProperties;
	hkpInertiaTensorComputer::computeSphereVolumeMassProperties(radius, sphereMass, massProperties);

	info.m_mass = massProperties.m_mass;
	info.m_centerOfMass  = massProperties.m_centerOfMass;
	info.m_inertiaTensor = massProperties.m_inertiaTensor;
	info.m_shape = new hkpSphereShape( radius );
	info.m_position.setAdd4(posy, relPos );
	info.m_motionType  = hkpMotion::MOTION_BOX_INERTIA;

	info.m_qualityType = HK_COLLIDABLE_QUALITY_BULLET;


	hkpRigidBody* sphereRigidBody = new hkpRigidBody( info );
	g_ball = sphereRigidBody;

	physicsWorld->addEntity( sphereRigidBody );
	sphereRigidBody->removeReference();
	info.m_shape->removeReference();

	hkVector4 vel(  0.0f,4.9f, -100.0f );
	sphereRigidBody->setLinearVelocity( vel );
}
*/

PintActorHandle SharedHavok::CreateCompoundObject(const PINT_OBJECT_CREATE& desc)
{
	// We'll basically have to create a 'List' Shape  (ie. a hkpListShape) in order to have many
	// shapes in the same body. Each element of the list will be a (transformed) hkpShape, ie.
	// a hkpTransformShape (which basically is a (geometry,transformation) pair).
	// The hkpListShape constructor needs a pointer to an array of hkShapes, so we create an array here, and push
	// back the hkTransformShapes as we create them.
	hkInplaceArray<hkpShape*, 32> shapeArray;
	hkInplaceArray<hkpShape*, 32> refs;

	const PINT_SHAPE_CREATE* CurrentShape = desc.mShapes;
	while(CurrentShape)
	{
		ASSERT(!CurrentShape->mMaterial);

		hkTransform LocalPose;
		LocalPose.setTranslation(ToHkVector4(CurrentShape->mLocalPos));
		LocalPose.setRotation(ToHkQuaternion(CurrentShape->mLocalRot));

		if(CurrentShape->mType==PINT_SHAPE_SPHERE)
		{
			const PINT_SPHERE_CREATE* SphereCreate = static_cast<const PINT_SPHERE_CREATE*>(CurrentShape);

			hkpSphereShape* shape = FindSphereShape(mParams.mShareShapes, mSphereShapes, *SphereCreate);
			refs.pushBack(shape);

//			sphere->addReference();
			hkpConvexTransformShape* sphereTransform = new hkpConvexTransformShape(shape, LocalPose);
			ASSERT(sphereTransform);
			shapeArray.pushBack(sphereTransform);
		}
		else if(CurrentShape->mType==PINT_SHAPE_BOX)
		{
			const PINT_BOX_CREATE* BoxCreate = static_cast<const PINT_BOX_CREATE*>(CurrentShape);

//			const hkVector4 halfExtents(ToHkVector4(BoxCreate->mExtents));

			hkpBoxShape* shape = FindBoxShape(mParams.mShareShapes, mBoxShapes, *BoxCreate);
			refs.pushBack(shape);

//			sphere->addReference();
			hkpConvexTransformShape* boxTransform = new hkpConvexTransformShape(shape, LocalPose);
			ASSERT(boxTransform);
			shapeArray.pushBack(boxTransform);
		}
		else if(CurrentShape->mType==PINT_SHAPE_CAPSULE)
		{
			const PINT_CAPSULE_CREATE* CapsuleCreate = static_cast<const PINT_CAPSULE_CREATE*>(CurrentShape);

			hkpCapsuleShape* shape = FindCapsuleShape(mParams.mShareShapes, mCapsuleShapes, *CapsuleCreate);
			refs.pushBack(shape);

//			sphere->addReference();
			hkpConvexTransformShape* capsuleTransform = new hkpConvexTransformShape(shape, LocalPose);
			ASSERT(capsuleTransform);
			shapeArray.pushBack(capsuleTransform);
		}
		else if(CurrentShape->mType==PINT_SHAPE_CYLINDER)
		{
			const PINT_CYLINDER_CREATE* CylinderCreate = static_cast<const PINT_CYLINDER_CREATE*>(CurrentShape);

			hkpCylinderShape* shape = FindCylinderShape(mParams.mShareShapes, mCylinderShapes, *CylinderCreate);
			refs.pushBack(shape);

//			sphere->addReference();
			hkpConvexTransformShape* cylinderTransform = new hkpConvexTransformShape(shape, LocalPose);
			ASSERT(cylinderTransform);
			shapeArray.pushBack(cylinderTransform);
		}
		else if(CurrentShape->mType==PINT_SHAPE_CONVEX)
		{
			const PINT_CONVEX_CREATE* ConvexCreate = static_cast<const PINT_CONVEX_CREATE*>(CurrentShape);

			hkStridedVertices StridedVerts;
			StridedVerts.m_numVertices	= ConvexCreate->mNbVerts;
			StridedVerts.m_striding		= sizeof(Point);
			StridedVerts.m_vertices		= &ConvexCreate->mVerts->x;

			hkpConvexVerticesShape* shape = FindConvexShape(mParams.mShareShapes, mConvexShapes, *ConvexCreate, StridedVerts);
			refs.pushBack(shape);

//			sphere->addReference();
			hkpConvexTransformShape* convexTransform = new hkpConvexTransformShape(shape, LocalPose);
			ASSERT(convexTransform);
			shapeArray.pushBack(convexTransform);
		}
		else ASSERT(0);

		CurrentShape = CurrentShape->mNext;
	}


	// Now we can create the compound body as a hkpListShape

	hkpListShape* listShape;
	{
		listShape = new hkpListShape(shapeArray.begin(), shapeArray.getSize());
		for(int i=0; i<refs.getSize(); ++i)
			refs[i]->removeReference();

		for(int i=0; i<shapeArray.getSize(); ++i)
			shapeArray[i]->removeReference();
	}

	// Create the rigid body
	hkpRigidBodyCinfo info;
	if(desc.mMass!=0.0f)
	{
		// #######
		hkMassProperties massProperties;
		hkpInertiaTensorComputer::computeBoxVolumeMassProperties(hkVector4(1.0f, 1.0f, 1.0f), desc.mMass, massProperties);

		info.m_mass				= massProperties.m_mass;
		info.m_centerOfMass		= massProperties.m_centerOfMass;
		info.m_inertiaTensor	= massProperties.m_inertiaTensor;
		info.m_motionType		= hkpMotion::MOTION_DYNAMIC;
	}
	else
	{
		info.m_restitution		= 0.0f;
	}
	FillRigidBodyInfo(info, desc, listShape);

	return PintActorHandle(CreateObject(info, desc, listShape));
}

PintJointHandle SharedHavok::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	ASSERT(mPhysicsWorld);

	hkpRigidBody* body0 = (hkpRigidBody*)desc.mObject0;
	hkpRigidBody* body1 = (hkpRigidBody*)desc.mObject1;

	hkpConstraintInstance* constraint = null;
	hkpConstraintData* data = null;

	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			const PINT_SPHERICAL_JOINT_CREATE& jc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);

			if(1)
			{
				hkpBallAndSocketConstraintData* bsc = new hkpBallAndSocketConstraintData();
				bsc->setInBodySpace(ToHkVector4(jc.mLocalPivot0), ToHkVector4(jc.mLocalPivot1));

				constraint = new hkpConstraintInstance(body0, body1, bsc);
				data = bsc;

				if(0)
				{
					hkReal inertiaStabilizationFactor;
					hkResult res = data->getInertiaStabilizationFactor(inertiaStabilizationFactor);
					printf("inertiaStabilizationFactor: %f\n", inertiaStabilizationFactor);
				}
			}
			else
			{
/*				hkpBallSocketChainData* chainData = new hkpBallSocketChainData();
				hkpConstraintChainInstance* chainInstance = new hkpConstraintChainInstance( chainData );

				chainInstance->addEntity( entities[0] );
				for (int e = 1; e < entities.getSize(); e++)
				{
					chainData->addConstraintInfoInBodySpace( hkVector4(elemHalfSize, 0.0f, 0.0f), hkVector4( -elemHalfSize, 0.0f, 0.0f) );
					chainInstance->addEntity( entities[e] );
				}
				chainData->removeReference();

				m_world->addConstraint(chainInstance);
				chainInstance->removeReference();*/
			}
		}
		break;

		case PINT_JOINT_HINGE:
		{
			const PINT_HINGE_JOINT_CREATE& jc = static_cast<const PINT_HINGE_JOINT_CREATE&>(desc);

			if(jc.mMinLimitAngle!=MIN_FLOAT || jc.mMaxLimitAngle!=MAX_FLOAT)
			{
				hkpLimitedHingeConstraintData* lhc = new hkpLimitedHingeConstraintData();

				if(!jc.mGlobalAnchor.IsNotUsed() && !jc.mGlobalAxis.IsNotUsed())
				{
					lhc->setInWorldSpace(	body0->getTransform(), body1->getTransform(),
											ToHkVector4(jc.mGlobalAnchor), ToHkVector4(jc.mGlobalAxis));
				}
				else
				{
					Point Right0, Up0;	ComputeBasis(jc.mLocalAxis0, Right0, Up0);
					Point Right1, Up1;	ComputeBasis(jc.mLocalAxis1, Right1, Up1);

					lhc->setInBodySpace(ToHkVector4(jc.mLocalPivot0), ToHkVector4(jc.mLocalPivot1),
										ToHkVector4(jc.mLocalAxis0), ToHkVector4(jc.mLocalAxis1),
										ToHkVector4(Right0), ToHkVector4(Right1));
				}
				lhc->setMinAngularLimit(jc.mMinLimitAngle);
				lhc->setMaxAngularLimit(jc.mMaxLimitAngle);

				constraint = new hkpConstraintInstance(body0, body1, lhc);
				data = lhc;
			}
			else
			{
				hkpHingeConstraintData* hc = new hkpHingeConstraintData();

				if(!jc.mGlobalAnchor.IsNotUsed() && !jc.mGlobalAxis.IsNotUsed())
				{
					hc->setInWorldSpace(body0->getTransform(), body1->getTransform(),
										ToHkVector4(jc.mGlobalAnchor), ToHkVector4(jc.mGlobalAxis));
				}
				else
				{
					hc->setInBodySpace(ToHkVector4(jc.mLocalPivot0), ToHkVector4(jc.mLocalPivot1), ToHkVector4(jc.mLocalAxis0), ToHkVector4(jc.mLocalAxis1));
				}

				constraint = new hkpConstraintInstance(body0, body1, hc);
				data = hc;
			}
		}
		break;

		case PINT_JOINT_FIXED:
		{
			const PINT_FIXED_JOINT_CREATE& jc = static_cast<const PINT_FIXED_JOINT_CREATE&>(desc);

			if(0)	// Emulating fixed joint with limited hinge
			{
				hkpLimitedHingeConstraintData* lhc = new hkpLimitedHingeConstraintData();

				const Point LocalAxis(1,0,0);
				const Point PerpAxis(0,1,0);
				lhc->setInBodySpace(ToHkVector4(jc.mLocalPivot0), ToHkVector4(jc.mLocalPivot1), ToHkVector4(LocalAxis), ToHkVector4(LocalAxis), ToHkVector4(PerpAxis), ToHkVector4(PerpAxis));
				lhc->setMinAngularLimit(0.0f);
				lhc->setMaxAngularLimit(0.0f);
				
				constraint = new hkpConstraintInstance(body0, body1, lhc);
				data = lhc;
			}

			if(1)	// Emulating fixed joint with prismatic
			{
				hkpPrismaticConstraintData* pris = new hkpPrismaticConstraintData(); 

				const Point LocalAxis(1,0,0);
				const Point PerpAxis(0,1,0);
				pris->setInBodySpace(ToHkVector4(jc.mLocalPivot0), ToHkVector4(jc.mLocalPivot1), ToHkVector4(LocalAxis), ToHkVector4(LocalAxis), ToHkVector4(PerpAxis), ToHkVector4(PerpAxis));
				pris->setMaxLinearLimit(0.0f);
				pris->setMinLinearLimit(0.0f);
				
				constraint = new hkpConstraintInstance(body0, body1, pris);
				data = pris;
			}
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			const PINT_PRISMATIC_JOINT_CREATE& jc = static_cast<const PINT_PRISMATIC_JOINT_CREATE&>(desc);

			hkpPrismaticConstraintData* pris = new hkpPrismaticConstraintData(); 

			Point Right0, Up0;	ComputeBasis(jc.mLocalAxis0, Right0, Up0);
			Point Right1, Up1;	ComputeBasis(jc.mLocalAxis1, Right1, Up1);

			pris->setInBodySpace(	ToHkVector4(jc.mLocalPivot0.mPos), ToHkVector4(jc.mLocalPivot1.mPos),
									ToHkVector4(jc.mLocalAxis0), ToHkVector4(jc.mLocalAxis1),
									ToHkVector4(Right0), ToHkVector4(Right1));
//			pris->setMaxLinearLimit(10.0f);
//			pris->setMinLinearLimit(-10.0f);
			
			constraint = new hkpConstraintInstance(body0, body1, pris);
			data = pris;
		}
		break;
	}

	if(data)
	{
//		hkResult res = data->setInertiaStabilizationFactor(1.0f);
	}

	if(constraint)
	{
		mPhysicsWorld->addConstraint(constraint); 	
		constraint->removeReference();
	}
	if(data)
		data->removeReference();

	return PintJointHandle(constraint);
}

#ifdef SUPPORT_HAVOK_ARTICULATION
PintArticHandle SharedHavok::CreateArticulation(const PINT_ARTICULATION_CREATE&)
{
	hkpBallSocketChainData* chainData = new hkpBallSocketChainData();
	hkpConstraintChainInstance* chainInstance = new hkpConstraintChainInstance(chainData);
	return PintArticHandle(chainInstance);
}

PintActorHandle SharedHavok::CreateArticulatedObject(const PINT_OBJECT_CREATE& desc, const PINT_ARTICULATED_BODY_CREATE& create, PintArticHandle articulation)
{
	hkpConstraintChainInstance* chainInstance = (hkpConstraintChainInstance*)articulation;
	hkpBallSocketChainData* chainData = static_cast<hkpBallSocketChainData*>(chainInstance->getData());

	hkpRigidBody* RigidBody = (hkpRigidBody*)CreateObject(desc);

	const udword NbEntities = chainInstance->m_chainedEntities.getSize();
//	printf("Nb0: %d\n", chainInstance->m_chainedEntities.getSize());
	chainInstance->addEntity( RigidBody );
//	printf("Nb1: %d\n", chainInstance->m_chainedEntities.getSize());
	if(NbEntities)
//	if(chainData->getNumConstraintInfos())
		chainData->addConstraintInfoInBodySpace(ToHkVector4(create.mLocalPivot0), ToHkVector4(create.mLocalPivot1));
	return PintActorHandle(RigidBody);
}

bool SharedHavok::AddArticulationToScene(PintArticHandle articulation)
{
	hkpConstraintChainInstance* chainInstance = (hkpConstraintChainInstance*)articulation;
	hkpBallSocketChainData* chainData = static_cast<hkpBallSocketChainData*>(chainInstance->getData());

//	chainData->useStabilizedCode(true);
	chainData->m_tau = 0.6f;
	chainData->removeReference();

	mPhysicsWorld->lock();
		mPhysicsWorld->addConstraint(chainInstance);
		chainInstance->removeReference();
	mPhysicsWorld->unlock();
	return true;
}
#endif

static void DrawLeafShape(PintRender& renderer, const hkpShape* shape, const PR& pose)
{
	ASSERT(shape->getUserData());
	if(shape->getUserData())
	{
		PintShapeRenderer* psr = (PintShapeRenderer*)shape->getUserData();
		psr->Render(pose);
		return;
	}
	
	if(1)
	{
		const hkpShapeType Type = shape->getType();

		if(Type==hkcdShapeType::SPHERE)
		{
			const hkpSphereShape* SphereShape = static_cast<const hkpSphereShape*>(shape);
			const float Radius = SphereShape->getRadius();
			renderer.DrawSphere(Radius, pose);
		}
		else if(Type==hkcdShapeType::BOX)
		{
			const hkpBoxShape* BoxShape = static_cast<const hkpBoxShape*>(shape);
			const hkVector4& Extents = BoxShape->getHalfExtents();
			renderer.DrawBox(ToPoint(Extents), pose);
		}
		else if(Type==hkcdShapeType::CONVEX_VERTICES)
		{
			const hkpConvexVerticesShape* ConvexShape = static_cast<const hkpConvexVerticesShape*>(shape);
			ASSERT(0);
		}
		else ASSERT(0);
	}
}

void SharedHavok::Render(PintRender& renderer, PintRenderPass render_pass)
{
	const udword Size = mRigidBodies.GetNbEntries();
	for(udword i=0;i<Size;i++)
	{
		hkpRigidBody* rigidBody = (hkpRigidBody*)mRigidBodies[i];

		rigidBody->markForRead();

		const hkTransform& BodyPose = rigidBody->getTransform();

		const hkpCollidable* collidable = rigidBody->getCollidable();
		const hkpShape* shape = collidable->getShape();
		const hkpShapeType type = shape->getType();
		if(type==hkcdShapeType::LIST)
		{
			const hkpListShape* listShape = static_cast<const hkpListShape*>(shape);
			const int NbShapes = listShape->getNumChildShapes();
			for(int j=0;j<NbShapes;j++)
			{
				const hkpShape* ChildShape = listShape->getChildShapeInl(j);
				const hkpShapeType ChildShapeType = ChildShape->getType();
				if(ChildShapeType==hkcdShapeType::CONVEX_TRANSFORM)
				{
					const hkpConvexTransformShape* ConvexTransformShape = static_cast<const hkpConvexTransformShape*>(ChildShape);
					const hkpConvexShape* LeafShape = ConvexTransformShape->getChildShape();
					hkTransform LeafTransform;
					ConvexTransformShape->getTransform(&LeafTransform);

					hkTransform Combo;
					Combo.setMul(BodyPose, LeafTransform);

					DrawLeafShape(renderer, LeafShape, ToPR(Combo));
				}
				else ASSERT(0);
			}
		}
		else
		{
			DrawLeafShape(renderer, shape, ToPR(BodyPose));
		}

		rigidBody->unmarkForRead();
	}
}

void SharedHavok::RenderDebugData(PintRender& renderer)
{
}

enum HavokGUIElement
{
	HAVOK_GUI_MAIN,
	//
	HAVOK_GUI_ENABLE_SLEEPING,
	HAVOK_GUI_ENABLE_CCD,
	HAVOK_GUI_SHARE_SHAPES,
	HAVOK_GUI_USE_VDB,
	//
	HAVOK_GUI_NB_THREADS,
	HAVOK_GUI_SOLVER_DEACTIVATION,
	HAVOK_GUI_BP_TYPE,
	HAVOK_GUI_CONTACT_POINT_GENERATION,
	HAVOK_GUI_MESH_FORMAT,
	HAVOK_GUI_CCD_QUALITY,
	//
	HAVOK_GUI_TOI_EVENT_QUEUE_SIZE,
	HAVOK_GUI_SOLVER_BUFFER_SIZE,
	HAVOK_GUI_SOLVER_ITER,
	HAVOK_GUI_LINEAR_DAMPING,
	HAVOK_GUI_ANGULAR_DAMPING,
	HAVOK_GUI_COLLISION_TOLERANCE,
	HAVOK_GUI_GLOBAL_BOX_SIZE,
};

EditableParams::EditableParams() :
	mNbThreads				(0),
	mSolverDeactivation		(hkpRigidBodyCinfo::SOLVER_DEACTIVATION_MEDIUM),
	mBroadPhaseType			(hkpWorldCinfo::BROADPHASE_TYPE_HYBRID),
	mContactPointGeneration	(hkpWorldCinfo::CONTACT_POINT_REJECT_MANY),
	mMeshFormat				(HAVOK_BV_COMPRESSED_MESH_SHAPE),
	mCCDQuality				(4),
	mLinearDamping			(0.1f),
	mAngularDamping			(0.05f),
	mCollisionTolerance		(0.05f),
	mGlobalBoxSize			(10000.0f),
	mMaxLinearVelocity		(10000.0f),
	mSolverIterationCount	(4),
	mSolverBufferSize		(2048),
	mSizeOfToiEventQueue	(1024),
	mEnableSleeping			(false),
	mShareShapes			(true),
	mUseVDB					(false),
	mUseCCD					(false)
{
}

static EditableParams gParams;

const EditableParams& Havok_::GetEditableParams()
{
	return gParams;
}

	struct HavokUI : public Allocateable
	{
						HavokUI(UICallback& callback);
						~HavokUI();

		UICallback&		mCallback;

		Widgets*		mHavokGUI;
		IceComboBox*	mComboBox_NbThreads;
		IceComboBox*	mComboBox_SolverDeactivation;
		IceComboBox*	mComboBox_BroadPhase;
		IceComboBox*	mComboBox_ContactPointGeneration;
		IceComboBox*	mComboBox_MeshFormat;
		IceComboBox*	mComboBox_CCDQuality;
		IceEditBox*		mEditBox_SolverIter;
		IceEditBox*		mEditBox_SolverBufferSize;
		IceEditBox*		mEditBox_SizeOfToiEventQueue;
		IceEditBox*		mEditBox_LinearDamping;
		IceEditBox*		mEditBox_AngularDamping;
		IceEditBox*		mEditBox_CollisionTolerance;
		IceEditBox*		mEditBox_GlobalBoxSize;
	};

HavokUI::HavokUI(UICallback& callback) :
	mCallback							(callback),
	mHavokGUI							(null),
	mComboBox_NbThreads					(null),
	mComboBox_SolverDeactivation		(null),
	mComboBox_BroadPhase				(null),
	mComboBox_ContactPointGeneration	(null),
	mComboBox_MeshFormat				(null),
	mComboBox_CCDQuality				(null),
	mEditBox_SolverIter					(null),
	mEditBox_SolverBufferSize			(null),
	mEditBox_SizeOfToiEventQueue		(null),
	mEditBox_LinearDamping				(null),
	mEditBox_AngularDamping				(null),
	mEditBox_CollisionTolerance			(null),
	mEditBox_GlobalBoxSize				(null)
{
}

HavokUI::~HavokUI()
{
	Common_CloseGUI(mHavokGUI);
}

static HavokUI* gHavokUI = null;

static udword gSolverDeactivationToIndex[] = { 0, 0, 1, 2, 3, 4 };
static hkpRigidBodyCinfo::SolverDeactivation gIndexToSolverDeactivation[] = {
												hkpRigidBodyCinfo::SOLVER_DEACTIVATION_OFF,
												hkpRigidBodyCinfo::SOLVER_DEACTIVATION_LOW,
												hkpRigidBodyCinfo::SOLVER_DEACTIVATION_MEDIUM,
												hkpRigidBodyCinfo::SOLVER_DEACTIVATION_HIGH,
												hkpRigidBodyCinfo::SOLVER_DEACTIVATION_MAX	};

static udword gNbThreadsToIndex[] = { 0, 0, 1, 2, 3 };
static udword gIndexToNbThreads[] = { 0, 2, 3, 4 };

void Havok_::GetOptionsFromGUI()
{
	if(!gHavokUI)
		return;

	if(gHavokUI->mComboBox_NbThreads)
	{
		const udword Index = gHavokUI->mComboBox_NbThreads->GetSelectedIndex();
		ASSERT(Index<sizeof(gIndexToNbThreads)/sizeof(gIndexToNbThreads[0]));
		gParams.mNbThreads = gIndexToNbThreads[Index];
	}

	if(gHavokUI->mComboBox_SolverDeactivation)
	{
		const udword Index = gHavokUI->mComboBox_SolverDeactivation->GetSelectedIndex();
		ASSERT(Index<sizeof(gIndexToSolverDeactivation)/sizeof(gIndexToSolverDeactivation[0]));
		gParams.mSolverDeactivation = gIndexToSolverDeactivation[Index];
	}

	if(gHavokUI->mComboBox_BroadPhase)
	{
		const udword Index = gHavokUI->mComboBox_BroadPhase->GetSelectedIndex();
		gParams.mBroadPhaseType = hkpWorldCinfo::BroadPhaseType(Index);
	}

	if(gHavokUI->mComboBox_ContactPointGeneration)
	{
		const udword Index = gHavokUI->mComboBox_ContactPointGeneration->GetSelectedIndex();
		gParams.mContactPointGeneration = hkpWorldCinfo::ContactPointGeneration(Index);
	}

	if(gHavokUI->mComboBox_MeshFormat)
	{
		const udword Index = gHavokUI->mComboBox_MeshFormat->GetSelectedIndex();
		gParams.mMeshFormat = HavokMeshFormat(Index);
	}

	if(gHavokUI->mComboBox_CCDQuality)
	{
		const udword Index = gHavokUI->mComboBox_CCDQuality->GetSelectedIndex();
		gParams.mCCDQuality = Index;
	}

	Common_GetFromEditBox(gParams.mSolverIterationCount, gHavokUI->mEditBox_SolverIter);
	Common_GetFromEditBox(gParams.mSolverBufferSize, gHavokUI->mEditBox_SolverBufferSize);
	Common_GetFromEditBox(gParams.mSizeOfToiEventQueue, gHavokUI->mEditBox_SizeOfToiEventQueue);
	Common_GetFromEditBox(gParams.mLinearDamping, gHavokUI->mEditBox_LinearDamping, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mAngularDamping, gHavokUI->mEditBox_AngularDamping, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mCollisionTolerance, gHavokUI->mEditBox_CollisionTolerance, 0.0f, FLT_MAX);
	Common_GetFromEditBox(gParams.mGlobalBoxSize, gHavokUI->mEditBox_GlobalBoxSize, 0.0f, FLT_MAX);
}

static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
{
	switch(check_box.GetID())
	{
		case HAVOK_GUI_ENABLE_SLEEPING:
			gParams.mEnableSleeping = checked;
			break;
		case HAVOK_GUI_ENABLE_CCD:
			gParams.mUseCCD = checked;
			break;
		case HAVOK_GUI_SHARE_SHAPES:
			gParams.mShareShapes = checked;
			break;
		case HAVOK_GUI_USE_VDB:
			gParams.mUseVDB = checked;
			break;
	}
}

IceWindow* Havok_::InitSharedGUI(IceWidget* parent, PintGUIHelper& helper, UICallback& callback)
{
	ASSERT(!gHavokUI);
	gHavokUI = ICE_NEW(HavokUI)(callback);

	IceWindow* Main = helper.CreateMainWindow(gHavokUI->mHavokGUI, parent, HAVOK_GUI_MAIN, "Havok options");

	const sdword YStep = 20;
	const sdword YStepCB = 16;
	sdword y = 4;

	{
		const udword CheckBoxWidth = 200;

		helper.CreateCheckBox(Main, HAVOK_GUI_ENABLE_SLEEPING, 4, y, CheckBoxWidth, 20, "Enable sleeping", gHavokUI->mHavokGUI, gParams.mEnableSleeping, gCheckBoxCallback);
		y += YStepCB;

		helper.CreateCheckBox(Main, HAVOK_GUI_ENABLE_CCD, 4, y, CheckBoxWidth, 20, "Enable CCD", gHavokUI->mHavokGUI, gParams.mUseCCD, gCheckBoxCallback);
		y += YStepCB;

		helper.CreateCheckBox(Main, HAVOK_GUI_SHARE_SHAPES, 4, y, CheckBoxWidth, 20, "Share shapes", gHavokUI->mHavokGUI, gParams.mShareShapes, gCheckBoxCallback);
		y += YStepCB;

		helper.CreateCheckBox(Main, HAVOK_GUI_USE_VDB, 4, y, CheckBoxWidth, 20, "Use VDB", gHavokUI->mHavokGUI, gParams.mUseVDB, gCheckBoxCallback);
		y += YStepCB;
	}

	y += YStep;

	const sdword LabelOffsetY = 2;
	{
		const sdword OffsetX = 90;
		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Solver deact.:", gHavokUI->mHavokGUI);
		ComboBoxDesc CBBD;
		CBBD.mID		= HAVOK_GUI_SOLVER_DEACTIVATION;
		CBBD.mParent	= Main;
		CBBD.mX			= 4+OffsetX;
		CBBD.mY			= y;
		CBBD.mWidth		= 256;
		CBBD.mHeight	= 20;
		CBBD.mLabel		= "Solver deactivation";
		gHavokUI->mComboBox_SolverDeactivation = ICE_NEW(IceComboBox)(CBBD);
		gHavokUI->mHavokGUI->Register(gHavokUI->mComboBox_SolverDeactivation);
		gHavokUI->mComboBox_SolverDeactivation->Add("SOLVER_DEACTIVATION_OFF");
		gHavokUI->mComboBox_SolverDeactivation->Add("SOLVER_DEACTIVATION_LOW");
		gHavokUI->mComboBox_SolverDeactivation->Add("SOLVER_DEACTIVATION_MEDIUM");
		gHavokUI->mComboBox_SolverDeactivation->Add("SOLVER_DEACTIVATION_HIGH");
		gHavokUI->mComboBox_SolverDeactivation->Add("SOLVER_DEACTIVATION_MAX");
		ASSERT(gParams.mSolverDeactivation<sizeof(gSolverDeactivationToIndex)/sizeof(gSolverDeactivationToIndex[0]));
		gHavokUI->mComboBox_SolverDeactivation->Select(gSolverDeactivationToIndex[gParams.mSolverDeactivation]);
		gHavokUI->mComboBox_SolverDeactivation->SetVisible(true);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Broadphase:", gHavokUI->mHavokGUI);
		CBBD.mID		= HAVOK_GUI_BP_TYPE;
		CBBD.mY			= y;
		CBBD.mLabel		= "Broadphase";
		gHavokUI->mComboBox_BroadPhase = ICE_NEW(IceComboBox)(CBBD);
		gHavokUI->mHavokGUI->Register(gHavokUI->mComboBox_BroadPhase);
		gHavokUI->mComboBox_BroadPhase->Add("SAP");
		gHavokUI->mComboBox_BroadPhase->Add("Tree");
		gHavokUI->mComboBox_BroadPhase->Add("Hybrid");
		gHavokUI->mComboBox_BroadPhase->Select(gParams.mBroadPhaseType);
		gHavokUI->mComboBox_BroadPhase->SetVisible(true);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Ctc pt gen:", gHavokUI->mHavokGUI);
		CBBD.mID		= HAVOK_GUI_CONTACT_POINT_GENERATION;
		CBBD.mY			= y;
		CBBD.mLabel		= "Contact point generation";
		gHavokUI->mComboBox_ContactPointGeneration = ICE_NEW(IceComboBox)(CBBD);
		gHavokUI->mHavokGUI->Register(gHavokUI->mComboBox_ContactPointGeneration);
		gHavokUI->mComboBox_ContactPointGeneration->Add("CONTACT_POINT_ACCEPT_ALWAYS");
		gHavokUI->mComboBox_ContactPointGeneration->Add("CONTACT_POINT_REJECT_DUBIOUS");
		gHavokUI->mComboBox_ContactPointGeneration->Add("CONTACT_POINT_REJECT_MANY");
		gHavokUI->mComboBox_ContactPointGeneration->Select(gParams.mContactPointGeneration);
		gHavokUI->mComboBox_ContactPointGeneration->SetVisible(true);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Mesh format:", gHavokUI->mHavokGUI);
		CBBD.mID		= HAVOK_GUI_MESH_FORMAT;
		CBBD.mY			= y;
		CBBD.mLabel		= "Mesh format";
		gHavokUI->mComboBox_MeshFormat = ICE_NEW(IceComboBox)(CBBD);
		gHavokUI->mHavokGUI->Register(gHavokUI->mComboBox_MeshFormat);
		gHavokUI->mComboBox_MeshFormat->Add("BV compressed mesh shape");
		gHavokUI->mComboBox_MeshFormat->Add("Extended mesh shape");
		gHavokUI->mComboBox_MeshFormat->Select(gParams.mMeshFormat);
		gHavokUI->mComboBox_MeshFormat->SetVisible(true);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Num threads:", gHavokUI->mHavokGUI);
		CBBD.mID		= HAVOK_GUI_NB_THREADS;
		CBBD.mY			= y;
		CBBD.mLabel		= "Num threads";
		gHavokUI->mComboBox_NbThreads = ICE_NEW(IceComboBox)(CBBD);
		gHavokUI->mHavokGUI->Register(gHavokUI->mComboBox_NbThreads);
		gHavokUI->mComboBox_NbThreads->Add("Single threaded");
		gHavokUI->mComboBox_NbThreads->Add("2 threads");
		gHavokUI->mComboBox_NbThreads->Add("3 threads");
		gHavokUI->mComboBox_NbThreads->Add("4 threads");
		ASSERT(gParams.mNbThreads<sizeof(gNbThreadsToIndex)/sizeof(gNbThreadsToIndex[0]));
		gHavokUI->mComboBox_NbThreads->Select(gNbThreadsToIndex[gParams.mNbThreads]);
		gHavokUI->mComboBox_NbThreads->SetVisible(true);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "CCD Quality:", gHavokUI->mHavokGUI);
		CBBD.mID		= HAVOK_GUI_CCD_QUALITY;
		CBBD.mY			= y;
		CBBD.mLabel		= "CCD quality";
		gHavokUI->mComboBox_CCDQuality = ICE_NEW(IceComboBox)(CBBD);
		gHavokUI->mHavokGUI->Register(gHavokUI->mComboBox_CCDQuality);
		gHavokUI->mComboBox_CCDQuality->Add("HK_COLLIDABLE_QUALITY_DEBRIS");
		gHavokUI->mComboBox_CCDQuality->Add("HK_COLLIDABLE_QUALITY_DEBRIS_SIMPLE_TOI");
		gHavokUI->mComboBox_CCDQuality->Add("HK_COLLIDABLE_QUALITY_MOVING");
		gHavokUI->mComboBox_CCDQuality->Add("HK_COLLIDABLE_QUALITY_CRITICAL");
		gHavokUI->mComboBox_CCDQuality->Add("HK_COLLIDABLE_QUALITY_BULLET");
		gHavokUI->mComboBox_CCDQuality->Select(gParams.mCCDQuality);
		gHavokUI->mComboBox_CCDQuality->SetVisible(true);
		y += YStep;
	}

	y += YStep;

	const sdword EditBoxWidth = 60;
	{
		const sdword LabelSize = 140;
		const sdword OffsetX = 140;
		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelSize, 20, "TOI event queue size:", gHavokUI->mHavokGUI);
		gHavokUI->mEditBox_SizeOfToiEventQueue = helper.CreateEditBox(Main, HAVOK_GUI_TOI_EVENT_QUEUE_SIZE, 4+OffsetX, y, EditBoxWidth, 20, _F("%d", gParams.mSizeOfToiEventQueue), gHavokUI->mHavokGUI, EDITBOX_INTEGER_POSITIVE, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelSize, 20, "Solver buffer size:", gHavokUI->mHavokGUI);
		gHavokUI->mEditBox_SolverBufferSize = helper.CreateEditBox(Main, HAVOK_GUI_SOLVER_BUFFER_SIZE, 4+OffsetX, y, EditBoxWidth, 20, _F("%d", gParams.mSolverBufferSize), gHavokUI->mHavokGUI, EDITBOX_INTEGER_POSITIVE, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelSize, 20, "Num solver iter:", gHavokUI->mHavokGUI);
		gHavokUI->mEditBox_SolverIter = helper.CreateEditBox(Main, HAVOK_GUI_SOLVER_ITER, 4+OffsetX, y, EditBoxWidth, 20, _F("%d", gParams.mSolverIterationCount), gHavokUI->mHavokGUI, EDITBOX_INTEGER_POSITIVE, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelSize, 20, "Linear damping:", gHavokUI->mHavokGUI);
		gHavokUI->mEditBox_LinearDamping = helper.CreateEditBox(Main, HAVOK_GUI_LINEAR_DAMPING, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gParams.mLinearDamping), gHavokUI->mHavokGUI, EDITBOX_FLOAT_POSITIVE, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelSize, 20, "Angular damping:", gHavokUI->mHavokGUI);
		gHavokUI->mEditBox_AngularDamping = helper.CreateEditBox(Main, HAVOK_GUI_ANGULAR_DAMPING, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gParams.mAngularDamping), gHavokUI->mHavokGUI, EDITBOX_FLOAT_POSITIVE, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelSize, 20, "Collision tolerance:", gHavokUI->mHavokGUI);
		gHavokUI->mEditBox_CollisionTolerance = helper.CreateEditBox(Main, HAVOK_GUI_COLLISION_TOLERANCE, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gParams.mCollisionTolerance), gHavokUI->mHavokGUI, EDITBOX_FLOAT_POSITIVE, null);
		y += YStep;

		helper.CreateLabel(Main, 4, y+LabelOffsetY, LabelSize, 20, "World bounds size:", gHavokUI->mHavokGUI);
		gHavokUI->mEditBox_GlobalBoxSize = helper.CreateEditBox(Main, HAVOK_GUI_GLOBAL_BOX_SIZE, 4+OffsetX, y, EditBoxWidth, 20, helper.Convert(gParams.mGlobalBoxSize), gHavokUI->mHavokGUI, EDITBOX_FLOAT_POSITIVE, null);
		y += YStep;
	}

	y += YStep;
	return Main;
}

void Havok_::CloseSharedGUI()
{
	DELETESINGLE(gHavokUI);
}

#endif
