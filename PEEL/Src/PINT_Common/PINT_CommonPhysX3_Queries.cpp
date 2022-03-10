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
#include "PINT_CommonPhysX3_Queries.h"

udword SharedPhysX::BatchRaycastAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	PxQueryFilterCallback* FCB = GetSQFilterCallback();

PxSceneQueryFilterData fdAny = PF;
fdAny.flags |= PxQueryFlag::eANY_HIT;
#ifndef PHYSX_REMOVED_CLIENT_ID
fdAny.clientId = PX_DEFAULT_CLIENT;
#endif
PxRaycastBuffer buf;

	udword NbHits = 0;
//	PxRaycastHit Hit;
	while(nb--)
	{
		//const bool b = raycastAny(mScene, ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, Hit, PF, FCB);
const bool b = mScene->raycast(ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, buf, PxHitFlag::eMESH_ANY, fdAny, FCB, NULL
#if PHYSX_SUPPORT_SIMD_GUARD_FLAG
	, PxGeometryQueryFlag::Enum(0)
#endif
	   );

		NbHits += b;
		dest->mHit = b;
		raycasts++;
		dest++;
	}
	return NbHits;
}

udword SharedPhysX::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	ASSERT(mScene);

#ifdef SETUP_FILTERING
	PxFilterData fd;
	SampleVehicleSetupVehicleShapeQueryFilterData(&fd);
	const PxSceneQueryFilterData PF(fd, PxSceneQueryFilterFlag::eDYNAMIC | PxSceneQueryFilterFlag::eSTATIC);
#else
	const PxQueryFilterData PF = GetSQFilterData();
#endif
//	const PxSceneQueryFilterData PF(PxFilterData(0, 0, 0, 0), PxSceneQueryFilterFlag::eDYNAMIC | PxSceneQueryFilterFlag::eSTATIC);
	const PxSceneQueryFlags sqFlags = GetRaycastQueryFlags(mParams);
	PxQueryFilterCallback* FCB = GetSQFilterCallback();

//#define RAYCAST_XP
//#define USE_RAYCASTS_API

#ifndef PHYSX_SUPPORT_VEHICLE5
	if(0)
	{
//		static
			PxBatchQuery* BatchQuery = null;
//		static
			PxBatchQueryDesc BatchQueryDesc(nb, 0, 0);
		if(!BatchQuery)
		{
			BatchQueryDesc.queryMemory.userRaycastResultBuffer	= new PxRaycastQueryResult[nb];
			BatchQueryDesc.queryMemory.userRaycastTouchBuffer	= new PxRaycastHit[nb];
			BatchQueryDesc.queryMemory.raycastTouchBufferSize	= nb;

			BatchQuery = mScene->createBatchQuery(BatchQueryDesc);
		}

		udword NbToGo = nb;
		while(nb--)
		{
			PxRaycastHit Hit;
			BatchQuery->raycast(ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, 0, sqFlags, PF, FCB);
			raycasts++;
		}

		BatchQuery->execute();
		BatchQuery->release();

		udword NbHits = 0;
		for(udword i=0;i<NbToGo;i++)
		{
			if(BatchQueryDesc.queryMemory.userRaycastResultBuffer[i].getNbAnyHits())
			{
				NbHits++;
//				ASSERT(BatchQueryDesc.queryMemory.userRaycastResultBuffer[i].queryStatus==PxBatchQueryStatus::eSUCCESS);
//				ASSERT(BatchQueryDesc.queryMemory.userRaycastResultBuffer[i].nbHits==1);
				FillResultStruct(*dest, BatchQueryDesc.queryMemory.userRaycastResultBuffer[i].getAnyHit(0));
			}
			else
			{
				dest->SetNoHit();
			}
			dest++;
		}
		DELETEARRAY(BatchQueryDesc.queryMemory.userRaycastTouchBuffer);
		DELETEARRAY(BatchQueryDesc.queryMemory.userRaycastResultBuffer);
		return NbHits;
	}
	else
#endif
	{
#ifdef USE_RAYCASTS_API
		PxQueryFilterData fd1 = PF;
	#ifndef PHYSX_REMOVED_CLIENT_ID
		fd1.clientId = PX_DEFAULT_CLIENT;
	#endif

	#define BATCH_SIZE	32
		Point	Origins[BATCH_SIZE];
		Point	Dirs[BATCH_SIZE];
		float	MaxDists[BATCH_SIZE];
		PxRaycastBuffer buf[BATCH_SIZE];

		udword NbTotalHits = 0;

		while(nb)
		{
			udword NbToGo = nb;
			if(NbToGo>BATCH_SIZE)
				NbToGo = BATCH_SIZE;
			for(udword i=0;i<NbToGo;i++)
			{
				Origins[i] = raycasts->mOrigin;
				Dirs[i] = raycasts->mDir;
				MaxDists[i] = raycasts->mMaxDist;
				buf[i].hasBlock = false;
				raycasts++;
			}

			udword NbBatchHits = mScene->raycasts(NbToGo, (const PxVec3*)Origins, (const PxVec3*)Dirs, MaxDists, buf, sqFlags, fd1, FCB);
			NbTotalHits += NbBatchHits;
			nb-=NbToGo;
			for(udword i=0;i<NbToGo;i++)
			{
				if(buf[i].hasBlock)
					FillResultStruct(*dest, buf[i].block);
				else
					dest->SetNoHit();
				dest++;
			}
		}
		return NbTotalHits;
#else
	#ifndef RAYCAST_XP
		PxRaycastBuffer buf;
	#endif
		PxQueryFilterData fd1 = PF;
	#ifndef PHYSX_REMOVED_CLIENT_ID
		fd1.clientId = PX_DEFAULT_CLIENT;
	#endif

//PxSceneQueryCache Cache;
		udword NbHits = 0;
		while(nb--)
		{
//bool blockingHit;
//PxRaycastHit HitBuffer[256];
//if(mScene->raycastMultiple(ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, sqFlags, HitBuffer, 256, blockingHit, PF))

//			if(mScene->raycast(ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, buf, sqFlags, fd1))
	#ifdef RAYCAST_XP
			PxRaycastHit Hit;
			if(mScene->raycastClosest((const PxVec3&)(raycasts->mOrigin), (const PxVec3&)(raycasts->mDir), raycasts->mMaxDist, Hit, sqFlags, fd1, null))
	#else
			if(mScene->raycast((const PxVec3&)(raycasts->mOrigin), (const PxVec3&)(raycasts->mDir), raycasts->mMaxDist, buf, sqFlags, fd1, FCB, null
		#if PHYSX_SUPPORT_SIMD_GUARD_FLAG
				, PxGeometryQueryFlag::Enum(0)
		#endif
				))
	#endif
//			hit = buf.block;
//			if(buf.hasBlock)
/*			PxRaycastHit Hit;
			if(raycastSingle(mScene, ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, sqFlags, Hit, PF
//, null, Cache.shape ? &Cache : null
									))*/
			{
				NbHits++;
//				FillResultStruct(*dest, Hit);
	#ifdef RAYCAST_XP
				FillResultStruct(*dest, Hit);
	#else
				FillResultStruct(*dest, buf.block);
	#endif
//FillResultStruct(*dest, HitBuffer[0]);
//Cache.shape	= Hit.shape;
//Cache.actor	= Hit.actor;
			}
			else
			{
				dest->SetNoHit();
			}

			raycasts++;
			dest++;
		}
		return NbHits;
#endif
	}
}

/*class MyQueryFilterCallback : public PxQueryFilterCallback
{
public:
	virtual PxQueryHitType::Enum preFilter(
		const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags)
	{
		return PxQueryHitType::eTOUCH;
	}

	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit)
	{
		return PxQueryHitType::eTOUCH;
	}
	virtual ~MyQueryFilterCallback() {}
}gQueryFilterCallback;*/

udword SharedPhysX::BatchRaycastAll(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintRaycastData* raycasts)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	const PxSceneQueryFlags sqFlags = GetRaycastQueryFlags(mParams)|PxSceneQueryFlag::eMESH_MULTIPLE;
	PxQueryFilterCallback* FCB = GetSQFilterCallback();

	PxRaycastHit hitBuffer[2048];
	const PxU32 hitBufferSize = 2048;
	bool blockingHit;
	udword Offset = 0;
	udword NbHits = 0;
	while(nb--)
	{
		const PxI32 CurrentNbHits = raycastMultiple(mScene, ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, sqFlags,
															hitBuffer, hitBufferSize, blockingHit, PF, FCB/*, &gQueryFilterCallback*/);
		NbHits += CurrentNbHits;
		dest->mNbHits = CurrentNbHits;
		dest->mOffset = Offset;
		Offset += CurrentNbHits;

		if(CurrentNbHits)
		{
			PintRaycastHit* buffer = reinterpret_cast<PintRaycastHit*>(stream.Reserve(CurrentNbHits*(sizeof(PintRaycastHit)/sizeof(udword))));
			for(PxI32 i=0;i<CurrentNbHits;i++)
				FillResultStruct(buffer[i], hitBuffer[i]);
		}

		raycasts++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

template<class PintOverlapDataT, class PxGeometryT, class AdapterT>
static inline_ udword _BatchOverlapAny(PxScene* scene, PintSQThreadContext context, udword nb,
									   PintBooleanHit* restrict_ dest, const PintOverlapDataT* restrict_ overlaps,
									   const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCallback)
{
	ASSERT(scene);

	udword NbHits = 0;
	PxOverlapHit Hit;
	while(nb--)
	{
		PxTransform Pose;
		const PxGeometryT Geom = AdapterT::ToPxGeometry(Pose, overlaps);

		const bool b = overlapAny(scene, Geom, Pose, Hit, filterData, filterCallback);
		NbHits += b;
		dest->mHit = b;
		overlaps++;
		dest++;
	}
	return NbHits;
}

udword SharedPhysX::BatchSphereOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)
{
	struct IceToPx{static inline_ PxSphereGeometry ToPxGeometry(PxTransform& pose, const PintSphereOverlapData* overlaps)
	{
		return ToPxSphereGeometry(pose, overlaps->mSphere);
	}};
	return _BatchOverlapAny<PintSphereOverlapData, PxSphereGeometry, IceToPx>(mScene, context, nb, dest, overlaps, GetSQFilterData(), GetSQFilterCallback());
}

udword SharedPhysX::BatchBoxOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)
{
	struct IceToPx{static inline_ PxBoxGeometry ToPxGeometry(PxTransform& pose, const PintBoxOverlapData* overlaps)
	{
		return ToPxBoxGeometry(pose, overlaps->mBox);
	}};
	return _BatchOverlapAny<PintBoxOverlapData, PxBoxGeometry, IceToPx>(mScene, context, nb, dest, overlaps, GetSQFilterData(), GetSQFilterCallback());
}

udword SharedPhysX::BatchCapsuleOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)
{
	struct IceToPx{static inline_ PxCapsuleGeometry ToPxGeometry(PxTransform& pose, const PintCapsuleOverlapData* overlaps)
	{
		return ToPxCapsuleGeometry(pose, overlaps->mCapsule);
	}};
	return _BatchOverlapAny<PintCapsuleOverlapData, PxCapsuleGeometry, IceToPx>(mScene, context, nb, dest, overlaps, GetSQFilterData(), GetSQFilterCallback());
}

udword SharedPhysX::BatchConvexOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintConvexOverlapData* overlaps)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	PxQueryFilterCallback* FCB = GetSQFilterCallback();

	PxConvexMeshGeometry convexGeom;
//	convexGeom.meshFlags = PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;

	udword NbHits = 0;
	PxOverlapHit Hit;
	while(nb--)
	{
		const PxTransform Pose(ToPxTransform(overlaps->mTransform));

		convexGeom.convexMesh = reinterpret_cast<PxConvexMesh*>(mConvexObjects.mObjects[overlaps->mConvexObjectIndex]);

		const bool b = overlapAny(mScene, convexGeom, Pose, Hit, PF, FCB);
		NbHits += b;
		dest->mHit = b;
		overlaps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

// TODO: templatize this
udword SharedPhysX::BatchSphereOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	PxQueryFilterCallback* FCB = GetSQFilterCallback();

	udword Offset = 0;
	udword NbHits = 0;
	PxOverlapHit Hits[4096];
	while(nb--)
	{
		PxTransform Pose;
		const PxSphereGeometry Geom = ToPxSphereGeometry(Pose, overlaps->mSphere);

		const PxI32 Nb = overlapMultiple(mScene, Geom, Pose, Hits, 4096, PF, FCB);
		NbHits += Nb;
		dest->mNbHits = Nb;
		dest->mOffset = Offset;
		Offset += Nb;

		if(Nb)
		{
			PintOverlapHit* buffer = reinterpret_cast<PintOverlapHit*>(stream.Reserve(Nb*(sizeof(PintOverlapHit)/sizeof(udword))));
			for(PxI32 i=0;i<Nb;i++)
			{
				buffer[i].mTouchedActor	= PintActorHandle(Hits[i].actor);
				buffer[i].mTouchedShape	= PintShapeHandle(Hits[i].shape);
			}
		}

		overlaps++;
		dest++;
	}
	return NbHits;
}

// TODO: templatize this
udword SharedPhysX::BatchBoxOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	PxQueryFilterCallback* FCB = GetSQFilterCallback();

	udword Offset = 0;
	udword NbHits = 0;
	PxOverlapHit Hits[4096];
	while(nb--)
	{
		PxTransform Pose;
		const PxBoxGeometry Geom = ToPxBoxGeometry(Pose, overlaps->mBox);

		const PxI32 Nb = overlapMultiple(mScene, Geom, Pose, Hits, 4096, PF, FCB);
		NbHits += Nb;
		dest->mNbHits = Nb;
		dest->mOffset = Offset;
		Offset += Nb;

		if(Nb)
		{
			PintOverlapHit* buffer = reinterpret_cast<PintOverlapHit*>(stream.Reserve(Nb*(sizeof(PintOverlapHit)/sizeof(udword))));
			for(PxI32 i=0;i<Nb;i++)
			{
				buffer[i].mTouchedActor	= PintActorHandle(Hits[i].actor);
				buffer[i].mTouchedShape	= PintShapeHandle(Hits[i].shape);
			}
		}

		overlaps++;
		dest++;
	}
	return NbHits;
}

// TODO: templatize this
udword SharedPhysX::BatchCapsuleOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)
{
/*	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	PxQueryFilterCallback* FCB = GetSQFilterCallback();

	udword NbHits = 0;
	PxOverlapHit Hits[4096];
	while(nb--)
	{
		const PxTransform Pose(ToPxVec3(overlaps->mSphere.mCenter));

		const PxI32 Nb = mScene->overlapMultiple(PxSphereGeometry(overlaps->mSphere.mRadius), Pose, Hits, 4096, PF, FCB);
		NbHits += Nb;
		dest->mNbObjects = Nb;

		overlaps++;
		dest++;
	}
	return NbHits;*/
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<class PintOverlapDataT, class PxGeometryT, class AdapterT>
static inline_ udword _FindTriangles_MeshOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* restrict_ dest, Container& stream, const PintOverlapDataT* restrict_ overlaps)
{
	PxRigidActor* RigidActor = GetActorFromHandle(handle);
	if(!RigidActor)
		return 0;

	if(RigidActor->getNbShapes()!=1)
		return 0;

	PxShape* meshShape = null;
	RigidActor->getShapes(&meshShape, 1);
	if(!meshShape)
		return 0;

	PxTriangleMeshGeometry meshGeom;
	if(!meshShape->getTriangleMeshGeometry(meshGeom))
		return 0;

#ifdef IS_PHYSX_3_2
	const PxTransform meshPose = PxShapeExt::getGlobalPose(*meshShape);
#else
	const PxTransform meshPose = PxShapeExt::getGlobalPose(*meshShape, *RigidActor);
#endif
	PxU32 Results[8192];
	udword NbTouchedTriangles = 0;
	const PxU32 startIndex = 0;
	udword Offset = 0;
	while(nb--)
	{
		PxTransform Pose;
		const PxGeometryT Geom = AdapterT::ToPxGeometry(Pose, overlaps);

		bool Overflow = false;
		const PxU32 Nb = PxMeshQuery::findOverlapTriangleMesh(Geom, Pose, meshGeom, meshPose, Results, 8192, startIndex, Overflow);
		ASSERT(!Overflow);

		NbTouchedTriangles += Nb;
		dest->mNbHits = Nb;
		dest->mOffset = Offset;
		Offset += Nb;

		if(Nb)
		{
			udword* buffer = stream.Reserve(Nb);
			CopyMemory(buffer, Results, sizeof(udword)*Nb);
		}

		overlaps++;
		dest++;
	}
	return NbTouchedTriangles;
}

///////////////////////////////////////////////////////////////////////////////

udword SharedPhysX::FindTriangles_MeshSphereOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)
{
	struct IceToPx{static inline_ PxSphereGeometry ToPxGeometry(PxTransform& pose, const PintSphereOverlapData* overlaps)
	{
		return ToPxSphereGeometry(pose, overlaps->mSphere);
	}};
	return _FindTriangles_MeshOverlap<PintSphereOverlapData, PxSphereGeometry, IceToPx>(context, handle, nb, dest, stream, overlaps);
}

udword SharedPhysX::FindTriangles_MeshBoxOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintBoxOverlapData* overlaps)
{
	struct IceToPx{static inline_ PxBoxGeometry ToPxGeometry(PxTransform& pose, const PintBoxOverlapData* overlaps)
	{
		return ToPxBoxGeometry(pose, overlaps->mBox);
	}};
	return _FindTriangles_MeshOverlap<PintBoxOverlapData, PxBoxGeometry, IceToPx>(context, handle, nb, dest, stream, overlaps);
}

udword SharedPhysX::FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, PintMultipleHits* dest, Container& stream, const PintCapsuleOverlapData* overlaps)
{
	struct IceToPx{static inline_ PxCapsuleGeometry ToPxGeometry(PxTransform& pose, const PintCapsuleOverlapData* overlaps)
	{
		return ToPxCapsuleGeometry(pose, overlaps->mCapsule);
	}};
	return _FindTriangles_MeshOverlap<PintCapsuleOverlapData, PxCapsuleGeometry, IceToPx>(context, handle, nb, dest, stream, overlaps);
}

///////////////////////////////////////////////////////////////////////////////

template<class PintSweepDataT, class PxGeometryT, class AdapterT>
static inline_ udword _BatchSweeps(	PxScene* scene, PintSQThreadContext context, udword nb,
									PintRaycastHit* restrict_ dest, const PintSweepDataT* restrict_ sweeps,
									PxSceneQueryFlags sweepQueryFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCallback)
{
	ASSERT(scene);

	const float Inflation = 0.0f;

	udword NbHits = 0;
	PxSweepHit Hit;
	while(nb--)
	{
		PxTransform Pose;
		const PxGeometryT Geom = AdapterT::ToPxGeometry(Pose, sweeps);

		if(sweepSingle(scene, Geom, Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, sweepQueryFlags, Hit, filterData, filterCallback
			, null, PX_DEFAULT_CLIENT, Inflation
			))
		{
			NbHits++;
			FillResultStruct(*dest, Hit);
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

udword SharedPhysX::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	struct IceToPx{static inline_ PxBoxGeometry ToPxGeometry(PxTransform& pose, const PintBoxSweepData* sweeps)
	{
		return ToPxBoxGeometry(pose, sweeps->mBox);
	}};
	return _BatchSweeps<PintBoxSweepData, PxBoxGeometry, IceToPx>(mScene, context, nb, dest, sweeps, GetSweepQueryFlags(mParams), GetSQFilterData(), GetSQFilterCallback());
}

udword SharedPhysX::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	struct IceToPx{static inline_ PxSphereGeometry ToPxGeometry(PxTransform& pose, const PintSphereSweepData* sweeps)
	{
		return ToPxSphereGeometry(pose, sweeps->mSphere);
	}};
	return _BatchSweeps<PintSphereSweepData, PxSphereGeometry, IceToPx>(mScene, context, nb, dest, sweeps, GetSweepQueryFlags(mParams), GetSQFilterData(), GetSQFilterCallback());
}

udword SharedPhysX::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	struct IceToPx{static inline_ PxCapsuleGeometry ToPxGeometry(PxTransform& pose, const PintCapsuleSweepData* sweeps)
	{
		return ToPxCapsuleGeometry(pose, sweeps->mCapsule);
	}};
	return _BatchSweeps<PintCapsuleSweepData, PxCapsuleGeometry, IceToPx>(mScene, context, nb, dest, sweeps, GetSweepQueryFlags(mParams), GetSQFilterData(), GetSQFilterCallback());
}

udword SharedPhysX::BatchConvexSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	const PxSceneQueryFlags sweepQueryFlags = GetSweepQueryFlags(mParams);
	PxQueryFilterCallback* FCB = GetSQFilterCallback();

	PxConvexMeshGeometry convexGeom;

	udword NbHits = 0;
	PxSweepHit Hit;
	while(nb--)
	{
		const PxTransform Pose(ToPxVec3(sweeps->mTransform.mPos), ToPxQuat(sweeps->mTransform.mRot));

		convexGeom.convexMesh = reinterpret_cast<PxConvexMesh*>(mConvexObjects.mObjects[sweeps->mConvexObjectIndex]);

		if(sweepSingle(mScene, convexGeom, Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, sweepQueryFlags, Hit, PF, FCB))
		{
			NbHits++;
			FillResultStruct(*dest, Hit);
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

///////////////////////////////////////////////////////////////////////////////
