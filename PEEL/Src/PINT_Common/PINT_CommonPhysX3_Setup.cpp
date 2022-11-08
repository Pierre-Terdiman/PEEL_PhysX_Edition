///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Pint.h"

#include "..\PINT_Common\PINT_Common.h"

#include "PINT_CommonPhysX3.h"
#include "PINT_CommonPhysX3_Setup.h"

/*class MySimulationFilterCallback : public PxSimulationFilterCallback
{
	public:
	virtual		PxFilterFlags	pairFound(	PxU32 pairID,
											PxFilterObjectAttributes attributes0, PxFilterData filterData0, const PxActor* a0, const PxShape* s0,
											PxFilterObjectAttributes attributes1, PxFilterData filterData1, const PxActor* a1, const PxShape* s1,
											PxPairFlags& pairFlags)
	{
		//printf("pairFound\n");
//		if(mFilterCallback->PairFound(*gPhysX, PintActorHandle(a0), PintShapeHandle(s0), PintActorHandle(a1), PintShapeHandle(s1)))
			return PxFilterFlag::eDEFAULT;
//		else
//			return PxFilterFlag::eSUPPRESS;
	}
	virtual		void			pairLost(	PxU32 pairID,
											PxFilterObjectAttributes attributes0, PxFilterData filterData0,
											PxFilterObjectAttributes attributes1, PxFilterData filterData1,
											bool objectRemoved)
	{
	}
	virtual		bool			statusChange(PxU32& pairID, PxPairFlags& pairFlags, PxFilterFlags& filterFlags)
	{
		return false;
	}

//	PintFilterCallback*	mFilterCallback;

}gSimFilterCallback;*/

void SetupSceneDesc(PxSceneDesc& sceneDesc, const PINT_WORLD_CREATE& desc, const EditableParams& mParams,
					//PxDefaultCpuDispatcher* cpu_dispatcher, PxSimulationEventCallback* secb, PxContactModifyCallback* cmcb)
					PxCpuDispatcher* cpu_dispatcher, PxSimulationEventCallback* secb, PxContactModifyCallback* cmcb)
{
	gFilterShaderExtraPairFlags = 0;

	sceneDesc.gravity					= ToPxVec3(desc.mGravity);
//	PxU32					nbContactDataBlocks;
//	PxU32					maxNbContactDataBlocks;
//	sceneDesc.nbContactDataBlocks		= 0;
//	sceneDesc.maxNbContactDataBlocks	= 1;

	sceneDesc.staticStructure			= mParams.mStaticPruner;
	sceneDesc.dynamicStructure			= mParams.mDynamicPruner;
	sceneDesc.dynamicTreeRebuildRateHint= mParams.mSQDynamicRebuildRateHint;
//	sceneDesc.dynamicTreeSecondaryPruner= PxDynamicTreeSecondaryPruner::eNONE;
//	sceneDesc.dynamicTreeSecondaryPruner= PxDynamicTreeSecondaryPruner::eBUCKET;
//	sceneDesc.dynamicTreeSecondaryPruner= PxDynamicTreeSecondaryPruner::eBVH;

//	sceneDesc.maxNbContactDataBlocks	= PX_MAX_U32;

	SetSceneFlag(sceneDesc, PxSceneFlag::eENABLE_PCM,				mParams.mPCM);
#if PHYSX_SUPPORT_ADAPTIVE_FORCE
	SetSceneFlag(sceneDesc, PxSceneFlag::eADAPTIVE_FORCE,			mParams.mAdaptiveForce);
#endif
	SetSceneFlag(sceneDesc, PxSceneFlag::eENABLE_STABILIZATION,		mParams.mStabilization);
	SetSceneFlag(sceneDesc, PxSceneFlag::eENABLE_ACTIVE_ACTORS,		mParams.mEnableActiveTransforms);
	SetSceneFlag(sceneDesc, PxSceneFlag::eDISABLE_CONTACT_CACHE,	!mParams.mEnableContactCache);
	SetSceneFlag(sceneDesc, PxSceneFlag::eENABLE_CCD,				mParams.mEnableCCD);
#if PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
	SetSceneFlag(sceneDesc, PxSceneFlag::eENABLE_FRICTION_EVERY_ITERATION,	mParams.mFrictionEveryIteration);
#endif
//		SetSceneFlag(sceneDesc, PxSceneFlag::eDEPRECATED_TRIGGER_TRIGGER_REPORTS,				true);
//	if(!gEnableSSE)
//		sceneDesc.flags					|= PxSceneFlag::eDISABLE_SSE;
	if(mParams.mEnableOneDirFriction)
		//sceneDesc.flags					|= PxSceneFlag::eENABLE_ONE_DIRECTIONAL_FRICTION;
		sceneDesc.frictionType			= PxFrictionType::eONE_DIRECTIONAL;
	if(mParams.mEnableTwoDirFriction)
//			sceneDesc.flags					|= PxSceneFlag::eENABLE_TWO_DIRECTIONAL_FRICTION;
		sceneDesc.frictionType			= PxFrictionType::eTWO_DIRECTIONAL;
	sceneDesc.broadPhaseType			= mParams.mBroadPhaseType;
//	sceneDesc.simulationOrder			= PxSimulationOrder::eSOLVE_COLLIDE;
	sceneDesc.ccdMaxPasses				= mParams.mMaxNbCCDPasses;

#if PHYSX_SUPPORT_TGS
	if(mParams.mTGS)
		sceneDesc.solverType			= PxSolverType::eTGS;
#endif

	if(!mParams.mEnableSleeping)
		sceneDesc.wakeCounterResetValue	= 9999999999.0f;

	sceneDesc.cpuDispatcher				= cpu_dispatcher;
	sceneDesc.filterShader				= PhysX3_SimulationFilterShader;
	if(mParams.mEnableCCD)
		gFilterShaderExtraPairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;

	if(desc.mContactModifyCallback || desc.mContactModifyCallback2)
	{
		sceneDesc.contactModifyCallback	= cmcb;
		gFilterShaderExtraPairFlags |= PxPairFlag::eMODIFY_CONTACTS;
	}

	if(mParams.mEnableContactNotif || desc.mContactNotifyCallback)
	{
		sceneDesc.simulationEventCallback	= secb;

		const udword DesiredFlags = desc.mContactNotifyCallback ? desc.mContactNotifyCallback->GetContactFlags() : PintContactNotifyCallback::CONTACT_ALL;
		if(DesiredFlags & PintContactNotifyCallback::CONTACT_FOUND)
			gFilterShaderExtraPairFlags |= PxU32(PxPairFlag::eNOTIFY_TOUCH_FOUND|PxPairFlag::eNOTIFY_TOUCH_CCD);
		if(DesiredFlags & PintContactNotifyCallback::CONTACT_PERSIST)
			gFilterShaderExtraPairFlags |= PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
		if(DesiredFlags & PintContactNotifyCallback::CONTACT_LOST)
			gFilterShaderExtraPairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;

		gFilterShaderExtraPairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
	}

/*		if(desc.mFilterCallback)
		{
			gSimFilterCallback.mFilterCallback = desc.mFilterCallback;
			sceneDesc.filterCallback		= &gSimFilterCallback;
			PhysX3_SetFilterCallback(true);
		}*/

#if PHYSX_SUPPORT_KINE_FILTERING_MODE
	// TODO: consider making this an option in the UI to check performance with/without
	sceneDesc.kineKineFilteringMode = PxPairFilteringMode::eKILL;
	sceneDesc.staticKineFilteringMode = PxPairFilteringMode::eKILL;
//	sceneDesc.kineKineFilteringMode = PxPairFilteringMode::eKEEP;
//	sceneDesc.staticKineFilteringMode = PxPairFilteringMode::eKEEP;
#endif

	sceneDesc.frictionOffsetThreshold	= mParams.mFrictionOffsetThreshold;

	if(mParams.mMaxBiasCoeff>=0.0f)
		sceneDesc.maxBiasCoefficient = mParams.mMaxBiasCoeff;
}

namespace
{
	class MyBroadPhaseCallback : public PxBroadPhaseCallback
	{
		public:
		virtual		void	onObjectOutOfBounds(PxShape& shape, PxActor& actor)	{}
		virtual		void	onObjectOutOfBounds(PxAggregate& aggregate)			{}

	}gBroadPhaseCallback;
}

void SetupBroadphase(const PINT_WORLD_CREATE& desc, const EditableParams& params, PxScene* scene)
{
	ASSERT(scene);

	if(params.mBroadPhaseType==PxBroadPhaseType::eMBP)
	{
		PxVec3 min, max;
		if(desc.mGlobalBounds.IsValid())
		{
			min.x = desc.mGlobalBounds.GetMin(0);
			min.y = desc.mGlobalBounds.GetMin(1);
			min.z = desc.mGlobalBounds.GetMin(2);
			max.x = desc.mGlobalBounds.GetMax(0);
			max.y = desc.mGlobalBounds.GetMax(1);
			max.z = desc.mGlobalBounds.GetMax(2);
		}
		else
		{
			min = PxVec3(-params.mMBPRange);
			max = PxVec3(params.mMBPRange);
		}
		const PxBounds3 globalBounds(min, max);

		PxBounds3 regions[256];
		const PxU32 nbRegions = PxBroadPhaseExt::createRegionsFromWorldBounds(regions, globalBounds, params.mMBPSubdivLevel);
		for(PxU32 i=0;i<nbRegions;i++)
		{
			PxBroadPhaseRegion region;
#if PHYSX_SUPPORT_PX_BROADPHASE_PABP
			region.mBounds = regions[i];
			region.mUserData = reinterpret_cast<void*>(size_t(i));
#else
			region.bounds = regions[i];
			region.userData = reinterpret_cast<void*>(size_t(i));
#endif
			scene->addBroadPhaseRegion(region);
		}

		scene->setBroadPhaseCallback(&gBroadPhaseCallback);
	}
}
