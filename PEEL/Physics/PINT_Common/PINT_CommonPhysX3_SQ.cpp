
udword PhysX::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
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
	const PxSceneQueryFlags sqFlags = GetRaycastQueryFlags();

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
			BatchQuery->raycast(ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, 0, sqFlags, PF, GetSQFilterCallback());
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

			udword NbBatchHits = mScene->raycasts(NbToGo, (const PxVec3*)Origins, (const PxVec3*)Dirs, MaxDists, buf, sqFlags, fd1, GetSQFilterCallback());
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
			if(mScene->raycast((const PxVec3&)(raycasts->mOrigin), (const PxVec3&)(raycasts->mDir), raycasts->mMaxDist, buf, sqFlags, fd1, GetSQFilterCallback(), null
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

class MyQueryFilterCallback : public PxQueryFilterCallback
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
}gQueryFilterCallback;

udword PhysX::BatchRaycastAll(PintSQThreadContext context, udword nb, PintRaycastAllHit* dest, Container& stream, const PintRaycastData* raycasts)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	const PxSceneQueryFlags sqFlags = GetRaycastQueryFlags()|PxSceneQueryFlag::eMESH_MULTIPLE;

	PxRaycastHit hitBuffer[2048];
	const PxU32 hitBufferSize = 2048;
	bool blockingHit;
	udword Offset = 0;
	udword NbHits = 0;
	while(nb--)
	{
		const PxI32 CurrentNbHits = raycastMultiple(mScene, ToPxVec3(raycasts->mOrigin), ToPxVec3(raycasts->mDir), raycasts->mMaxDist, sqFlags,
															hitBuffer, hitBufferSize, blockingHit, PF, GetSQFilterCallback()/*, &gQueryFilterCallback*/);
		NbHits += CurrentNbHits;
		dest->mNbHits = CurrentNbHits;
		dest->mOffset = Offset;
		Offset += CurrentNbHits;

		if(CurrentNbHits)
		{
			PintRaycastHit* buffer = (PintRaycastHit*)stream.Reserve(CurrentNbHits*(sizeof(PintRaycastHit)/sizeof(udword)));
			for(PxI32 i=0;i<CurrentNbHits;i++)
				FillResultStruct(buffer[i], hitBuffer[i]);
		}

		raycasts++;
		dest++;
	}
	return NbHits;
}

udword PhysX::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	const PxSceneQueryFlags sweepQueryFlags = GetSweepQueryFlags(mParams);

	udword NbHits = 0;
	while(nb--)
	{
		PxTransform Pose;
		const PxBoxGeometry Geom = ToPxBoxGeometry(Pose, sweeps->mBox);

		PxSweepHit Hit;
//		if(mScene->sweepAny(PxBoxGeometry(ToPxVec3(sweeps->mBox.mExtents)), Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, sweepQueryFlags, Hit, PF))
		if(sweepSingle(mScene, Geom, Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, sweepQueryFlags, Hit, PF, GetSQFilterCallback()))
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

udword PhysX::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	const PxSceneQueryFlags sweepQueryFlags = GetSweepQueryFlags(mParams);

	udword NbHits = 0;
	while(nb--)
	{
		const PxTransform Pose(ToPxVec3(sweeps->mSphere.mCenter), PxQuat(PxIdentity));

		PxSweepHit Hit;
		if(sweepSingle(mScene, PxSphereGeometry(sweeps->mSphere.mRadius), Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, sweepQueryFlags, Hit, PF, GetSQFilterCallback()))
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

udword PhysX::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	const PxSceneQueryFlags sweepQueryFlags = GetSweepQueryFlags(mParams);

	udword NbHits = 0;
	while(nb--)
	{
		PxTransform Pose;
		const PxCapsuleGeometry Geom = ToPxCapsuleGeometry(Pose, sweeps->mCapsule);

		PxSweepHit Hit;
		if(sweepSingle(mScene, Geom, Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, sweepQueryFlags, Hit, PF, GetSQFilterCallback()))
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

udword PhysX::BatchConvexSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)
{
	ASSERT(mScene);

	const PxQueryFilterData PF = GetSQFilterData();
	const PxSceneQueryFlags sweepQueryFlags = GetSweepQueryFlags(mParams);

	PxConvexMeshGeometry convexGeom;

	udword NbHits = 0;
	while(nb--)
	{
		const PxTransform Pose(ToPxVec3(sweeps->mTransform.mPos), ToPxQuat(sweeps->mTransform.mRot));

		convexGeom.convexMesh = mConvexObjects[sweeps->mConvexObjectIndex];

		PxSweepHit Hit;
		if(sweepSingle(mScene, convexGeom, Pose, ToPxVec3(sweeps->mDir), sweeps->mMaxDist, sweepQueryFlags, Hit, PF, GetSQFilterCallback()))
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
