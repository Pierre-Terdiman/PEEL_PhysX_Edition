///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX5 plug-ins, so put only the code here that is "the same" for all versions.

/*
- release notes & manual (PX-2266)
- query system misses a function to remove a pruner (PX-2267)
- think about MT calls there, like MT batched updates (PX-2268)
- need a pruner of pruners, i.e. pruner bounds - not so easy to update etc (PX-2269)
- need to re-implement compound pruner, maybe using a custom geometry (PX-2270)
- need to revisit leaf-level / midphase API.....big change (PX-2271)
    - isn't there an inconsistency between raycasts & others w.r.t. the "midphase" query, i.e. why do we support multiple hits per mesh natively for raycasts but not for overlaps?
- expose build strategy for pruners (PX-2272) *** and #objects / node

- make sure we cannot call buildStep N times before a commit => actually maybe that's ok
- can we support a mode where we update the transforms without rebuilding anything? => via getPayloadData probably
- move lock and flushShapes to a derived class for people who want to use this?
- unify sync & update calls?
- we don't have a generic traversal function on pruners => maybe revisit visualize for this
- adapter redundant with validatePayload but not easy to unify. Maybe revisit markForUpdate and pass the bounds there if needed => or pass adapter to all funcs that need it
- fix profile zone names in PhysX
- optimize: don't iterate over all pruners for all ops
- optimize: skip tree if it only has a few objects?
- use this for a tree-based BP in immediate mode
- unify companion pruner enums in physx

- do we need PX_SIMD_GUARD => naaah up to users
- eventually move most of the _sweepClosest code to Gu

PEEL:
- finish re-routing all queries
- we probably also miss the PxFilterData filtering
- make the system work with vehicles
- templatize that stuff a bit
*/

//#include "stdafx.h"

#include "PINT_CommonPhysX5_CustomSQ.h"

#include "GuAABBPruner.h"
#include "GuQuerySystem.h"
#include "PxShape.h"
#include "PxRigidActor.h"

using namespace physx;
using namespace Gu;

#pragma warning(disable:4355)	// 'this' : used in base member initializer list

#define EXT_PRUNER_EPSILON	0.005f

// In this file we work with PxShape/PxRigidActor pointers in the payload
static PX_FORCE_INLINE void setPayload(PrunerPayload& pp, const PxShape* shape, const PxRigidActor* actor)
{
	pp.data[0] = size_t(shape);
	pp.data[1] = size_t(actor);
}

static PX_FORCE_INLINE PxShape*			getShapeFromPayload(const PrunerPayload& payload)	{ return reinterpret_cast<PxShape*>(payload.data[0]);				}
static PX_FORCE_INLINE PxRigidActor*	getActorFromPayload(const PrunerPayload& payload)	{ return reinterpret_cast<PxRigidActor*>(payload.data[1]);			}
static PX_FORCE_INLINE bool				isDynamicActor(const PxRigidActor& actor)			{ return actor.getConcreteType() != PxConcreteType::eRIGID_STATIC;	}

PX_FORCE_INLINE void getActorShape(const PrunerPayload& payload, PxActorShape& actorShape)
{
	actorShape.actor = getActorFromPayload(payload);
	actorShape.shape = getShapeFromPayload(payload);
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class MyPrunerFilter : public PrunerFilter
	{
		const PxQueryFlags	mFlags;
		public:
						MyPrunerFilter(const PxQueryFilterData& filterData) : mFlags(filterData.flags)	{}
		virtual	bool	processPruner(PxU32 prunerIndex)	const
		{
			if(prunerIndex==0)
				return mFlags & PxQueryFlag::eSTATIC;
			if(prunerIndex==1)
				return mFlags & PxQueryFlag::eDYNAMIC;
			PX_ASSERT(0);
			return false;
		}
	};
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	// We use PxSceneQuerySystem as a hook into PhysX but we could eventually bypass that entirely. It's a custom SQ system: we only re-implement here what we need in PEEL.
	// - the commitUpdates() call is only needed for edge cases like moving objects in pause mode
	// - the filter callback is needed for the editor's hide/show feature (hidden objects should be ignored by SQ)
	// PEEL_CSQS = PEEL_CustomSceneQuerySystem
	class PEEL_CSQS : public PxSceneQuerySystem, public Adapter, public PxUserAllocated
	{
		public:
												PEEL_CSQS(PxU64 contextID, Pruner* staticPruner, Pruner* dynamicPruner, PxU32 dynamicTreeRebuildRateHint, PxSceneQueryUpdateMode::Enum mode);
		virtual									~PEEL_CSQS()	{}

		// Adapter
		virtual	const PxGeometry&				getGeometry(const PrunerPayload& payload)	const
												{
													PxShape* shape = getShapeFromPayload(payload);
													return shape->getGeometry();
												}
		//~Adapter

		virtual	void							release();
		virtual	void							acquireReference()	{ mRefCount++;	}
		virtual	void							addSQShape(	const PxRigidActor& actor, const PxShape& shape, const PxBounds3& bounds,
															const PxTransform& transform, const PxSQCompoundHandle* compoundHandle, bool hasPruningStructure);
		virtual	void							removeSQShape(const PxRigidActor& actor, const PxShape& shape);
		virtual	void							updateSQShape(const PxRigidActor& actor, const PxShape& shape, const PxTransform& transform);
		virtual	void							finalizeUpdates();
		virtual	void							visualize(PxU32 prunerIndex, PxRenderOutput& out)	const;
		virtual	PxSQPrunerHandle				getHandle(const PxRigidActor& actor, const PxShape& shape, PxU32& prunerIndex)	const;
		virtual	void							sync(PxU32 prunerIndex, const PxSQPrunerHandle* handles, const PxU32* indices, const PxBounds3* bounds,
													const PxTransform32* transforms, PxU32 count, const PxBitMap& ignoredIndices);

		virtual	void							flushMemory()												{ mQuerySystem.flushMemory();							}
		virtual	void							setDynamicTreeRebuildRateHint(PxU32 dynTreeRebuildRateHint)	{ mDynamicTreeRebuildHint = dynTreeRebuildRateHint;		}
		virtual	PxU32							getDynamicTreeRebuildRateHint()	const						{ return mDynamicTreeRebuildHint;						}
		virtual	PxSceneQueryUpdateMode::Enum	getUpdateMode()					const						{ return mSceneQueryUpdateMode;							}
		virtual	void							setUpdateMode(PxSceneQueryUpdateMode::Enum mode)			{ mSceneQueryUpdateMode = mode;							}
		virtual	PxU32							getStaticTimestamp()			const						{ return mQuerySystem.getStaticTimestamp();				}

		virtual	bool							raycast(const PxVec3&, const PxVec3&, const PxReal, PxRaycastCallback&, PxHitFlags, const PxQueryFilterData&, PxQueryFilterCallback*,
														const PxQueryCache*, PxGeometryQueryFlags)	const								{ PX_ASSERT(!"Not supported");	return false;	}
		virtual	bool							sweep(	const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance, PxSweepCallback& buf, PxHitFlags hitFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB,
														const PxQueryCache*, const PxReal, PxGeometryQueryFlags)	const
		{
			// ####
//			PX_ASSERT(!"Not supported");
//			return false;
			buf.hasBlock = _sweepClosest(geometry, pose, unitDir, distance, buf.block, hitFlags, filterData, filterCB);
			return buf.hasBlock;
		}
		virtual	bool							overlap(const PxGeometry&, const PxTransform&, PxOverlapCallback&, const PxQueryFilterData&, PxQueryFilterCallback*,
														const PxQueryCache*, PxGeometryQueryFlags)	const								{ PX_ASSERT(!"Not supported");	return false;	}
		virtual	void							preallocate(PxU32 prunerIndex, PxU32 nbShapes)											{ PX_ASSERT(!"Not supported");					}
		virtual	PxSQCompoundHandle				addSQCompound(const PxRigidActor&, const PxShape**, const PxBVH&, const PxTransform*)	{ PX_ASSERT(!"Not supported");	return 0;		}
		virtual	void							removeSQCompound(PxSQCompoundHandle)													{ PX_ASSERT(!"Not supported");					}
		virtual	void							updateSQCompound(PxSQCompoundHandle, const PxTransform&)								{ PX_ASSERT(!"Not supported");					}
		virtual	void							merge(const PxPruningStructure&)														{ PX_ASSERT(!"Not supported");					}
		virtual	void							shiftOrigin(const PxVec3&)																{ PX_ASSERT(!"Not supported");					}
		virtual	void							flushUpdates()																			{ PX_ASSERT(!"Not supported");					}
		virtual	PxSQBuildStepHandle				prepareSceneQueryBuildStep(PxU32 prunerIndex)											{ PX_ASSERT(!"Not supported");	return false;	}
		virtual	void							sceneQueryBuildStep(PxSQBuildStepHandle handle)											{ PX_ASSERT(!"Not supported");					}
		virtual	void							forceRebuildDynamicTree(PxU32 prunerIndex)												{ PX_ASSERT(!"Not supported");					}

				bool							_raycastAny(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
															const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const;

				bool							_raycastClosest(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
																PxRaycastHit& hit, PxHitFlags hitFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const;

				bool							_overlapAny(const PxGeometry& geometry, const PxTransform& pose,
															const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const;

				PxU32							_overlapAll(const PxGeometry& geometry, const PxTransform& pose,
															PxOverlapHit* hits, PxU32 nb, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)	const;

				bool							_sweepClosest(	const PxGeometry& geometry, const PxTransform& pose,
																const PxVec3& unitDir, const PxReal distance, PxSweepHit& hit, PxHitFlags hitFlags,
																const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const;

				// We only need to call commitUpdates() for edge cases where e.g. we move objects in Pause mode, and the regular SQ update is not called.
				PX_FORCE_INLINE	void			commitUpdates()	const	{ const_cast<QuerySystem&>(mQuerySystem).commitUpdates();	}

				ActorShapeMap					mDatabase;
				QuerySystem						mQuerySystem;
				CachedFuncs						mCachedFuncs;
				PxSceneQueryUpdateMode::Enum    mSceneQueryUpdateMode;
				PxU32							mDynamicTreeRebuildHint;
				PxU32							mRefCount;
	};
}

///////////////////////////////////////////////////////////////////////////////

PEEL_CSQS::PEEL_CSQS(PxU64 contextID, Pruner* staticPruner, Pruner* dynamicPruner, PxU32 dynamicTreeRebuildRateHint, PxSceneQueryUpdateMode::Enum mode) :
	mQuerySystem			(contextID, EXT_PRUNER_EPSILON, *this),
	mSceneQueryUpdateMode	(mode),
	mDynamicTreeRebuildHint	(dynamicTreeRebuildRateHint),
	mRefCount				(1)
{
	mQuerySystem.addPruner(staticPruner, 0);
	mQuerySystem.addPruner(dynamicPruner, 0);
}

void PEEL_CSQS::release()
{
	mRefCount--;
	if(!mRefCount)
	{
		PX_DELETE_THIS;
	}
}

void PEEL_CSQS::addSQShape(const PxRigidActor& actor, const PxShape& shape, const PxBounds3& bounds, const PxTransform& transform, const PxSQCompoundHandle* compoundHandle, bool hasPruningStructure)
{
	PX_ASSERT(!compoundHandle || *compoundHandle==0xffffffff);
	PX_ASSERT(!hasPruningStructure);

	PrunerPayload payload;
	setPayload(payload, &shape, &actor);

	const bool isDynamic = isDynamicActor(actor);
	const PxU32 prunerIndex = PxU32(isDynamic);

	const ActorShapeData prunerData = mQuerySystem.addPrunerShape(payload, prunerIndex, isDynamic, transform, &bounds);

	const PxU32 actorIndex = actor.getInternalActorIndex();
	PX_ASSERT(actorIndex!=0xffffffff);
	mDatabase.add(actorIndex, &actor, &shape, prunerData);
}

void PEEL_CSQS::removeSQShape(const PxRigidActor& actor, const PxShape& shape)
{
	const PxU32 actorIndex = actor.getInternalActorIndex();
	PX_ASSERT(actorIndex!=0xffffffff);

	ActorShapeData data;
	const bool status = mDatabase.remove(actorIndex, &actor, &shape, &data);

	mQuerySystem.removePrunerShape(data, NULL);
}

void PEEL_CSQS::updateSQShape(const PxRigidActor& actor, const PxShape& shape, const PxTransform& transform)
{
	const PxU32 actorIndex = actor.getInternalActorIndex();
	PX_ASSERT(actorIndex!=0xffffffff);

	const ActorShapeData shapeHandle = mDatabase.find(actorIndex, &actor, &shape);

	mQuerySystem.updatePrunerShape(shapeHandle, false, transform);
}

void PEEL_CSQS::visualize(PxU32 prunerIndex, PxRenderOutput& out) const
{
/*	const PxU32 primaryColor = 0xffff0000;
	const PxU32 secondaryColor = 0xff00ff00;
	const PxU32 nbPruners = mQuerySystem.getNbPruners();
	for(PxU32 i=0;i<nbPruners;i++)
	{
		const Pruner* p = mQuerySystem.getPruner(i);
		p->visualize(out, primaryColor, secondaryColor);
	}*/
}

PxSQPrunerHandle PEEL_CSQS::getHandle(const PxRigidActor& actor, const PxShape& shape, PxU32& prunerIndex) const
{
	const PxU32 actorIndex = actor.getInternalActorIndex();
	PX_ASSERT(actorIndex!=0xffffffff);

	const ActorShapeData prunerData = mDatabase.find(actorIndex, &actor, &shape);

	prunerIndex = getPrunerIndex(getPrunerInfo(prunerData));
	return PxSQPrunerHandle(getPrunerHandle(prunerData));
}

void PEEL_CSQS::sync(PxU32 prunerIndex, const PxSQPrunerHandle* handles, const PxU32* indices, const PxBounds3* bounds, const PxTransform32* transforms, PxU32 count, const PxBitMap& ignoredIndices)
{
	PX_ASSERT(!ignoredIndices.count());
	mQuerySystem.sync(prunerIndex, handles, indices, bounds, transforms, count);
}

void PEEL_CSQS::finalizeUpdates()
{
	switch(mSceneQueryUpdateMode)
	{
	case PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_ENABLED:		mQuerySystem.update(true, true);	break;
	case PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_DISABLED:	mQuerySystem.update(true, false);	break;
	case PxSceneQueryUpdateMode::eBUILD_DISABLED_COMMIT_DISABLED:	mQuerySystem.update(false, false);	break;
	}
}

namespace
{
	// We re-route this custom filter CB to what we need from the traditional PhysX filter callback. No need to test for prefilter/postfilter flags here, we know
	// what we need in PEEL and we call our own code directly anyway. In fact we could skip the PxQueryFilterCallback object entirely and just re-do here in our
	// PrunerFilterCallback what we did there (in SharedPhysX::preFilter). But reusing PxQueryFilterCallback allows us to keep sharing that code with previous
	// PhysX versions etc.
	struct PEEL_CustomFilterCB : PrunerFilterCallback
	{
		PxQueryFilterCallback*	mFilterCB;

		PX_FORCE_INLINE	PEEL_CustomFilterCB(PxQueryFilterCallback* filterCB) : mFilterCB(filterCB)	{}

		virtual	const PxGeometry*	validatePayload(const PrunerPayload& payload, PxHitFlags& hitFlags)
		{
			PxShape* shape = getShapeFromPayload(payload);

			if(mFilterCB)
			{
				PxRigidActor* actor = getActorFromPayload(payload);
				// We know that we don't use PxFilterData in SharedPhysX::preFilter so we don't bother
				if(mFilterCB->preFilter(PxFilterData(), shape, actor, hitFlags)==PxQueryHitType::eNONE)
					return NULL;
			}

			return &shape->getGeometry();
		}
	};
}

bool PEEL_CSQS::_raycastAny(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const
{
	commitUpdates();

	PEEL_CustomFilterCB csb(filterCB);
	DefaultPrunerRaycastAnyCallback pcb(csb, mCachedFuncs.mCachedRaycastFuncs, origin, unitDir, distance);

	const MyPrunerFilter pf(filterData);

	float d = distance;
	mQuerySystem.raycast(origin, unitDir, d, pcb, &pf);

	return pcb.mFoundHit;
}

bool PEEL_CSQS::_raycastClosest(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, PxRaycastHit& hit, PxHitFlags hitFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const
{
	commitUpdates();

	PEEL_CustomFilterCB csb(filterCB);
	DefaultPrunerRaycastClosestCallback pcb(csb, mCachedFuncs.mCachedRaycastFuncs, origin, unitDir, distance, hitFlags);

	const MyPrunerFilter pf(filterData);
	mQuerySystem.raycast(origin, unitDir, pcb.mClosestHit.distance, pcb, &pf);

	if(pcb.mFoundHit)
	{
		static_cast<PxGeomRaycastHit&>(hit) = pcb.mClosestHit;
		getActorShape(pcb.mClosestPayload, hit);
	}
	return pcb.mFoundHit;
}

bool PEEL_CSQS::_overlapAny(const PxGeometry& geometry, const PxTransform& pose, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const
{
	commitUpdates();

	struct CB : public DefaultPrunerOverlapCallback
	{
		bool	mFoundHit;

		CB(PrunerFilterCallback& filterCB, const GeomOverlapTable* funcs, const PxGeometry& geometry, const PxTransform& pose) : DefaultPrunerOverlapCallback(filterCB, funcs, geometry, pose), mFoundHit(false)	{}

		virtual	bool	reportHit(const PrunerPayload& payload)
		{
			mFoundHit = true;
			return false;
		}
	};

	PEEL_CustomFilterCB csb(filterCB);
	CB pcb(csb, mCachedFuncs.mCachedOverlapFuncs, geometry, pose);

	const MyPrunerFilter pf(filterData);
	const ShapeData queryVolume(geometry, pose, 0.0f);
	mQuerySystem.overlap(queryVolume, pcb, &pf);

	return pcb.mFoundHit;
}

PxU32 PEEL_CSQS::_overlapAll(const PxGeometry& geometry, const PxTransform& pose, PxOverlapHit* hits, PxU32 nb, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const
{
	commitUpdates();

	struct CB : public DefaultPrunerOverlapCallback
	{
		PxU32	mNbHits;

		CB(PrunerFilterCallback& filterCB, const GeomOverlapTable* funcs, const PxGeometry& geometry, const PxTransform& pose) : DefaultPrunerOverlapCallback(filterCB, funcs, geometry, pose), mNbHits(0){}

		virtual	bool	reportHit(const PrunerPayload& payload)
		{
			mNbHits++;
			return true;
		}
	};

	PEEL_CustomFilterCB csb(filterCB);
	CB pcb(csb, mCachedFuncs.mCachedOverlapFuncs, geometry, pose);

	const MyPrunerFilter pf(filterData);
	const ShapeData queryVolume(geometry, pose, 0.0f);
	mQuerySystem.overlap(queryVolume, pcb, &pf);

	return pcb.mNbHits;
}

template<class CallbackT>
static bool _sweepClosestT(PxSweepHit& hit, const QuerySystem& sqs, const Gu::GeomSweepFuncs& funcs, const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, float distance, PxHitFlags hitFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	PEEL_CustomFilterCB csb(filterCB);

	const ShapeData queryVolume(geometry, pose, 0.0f);	// ### inflation

	CallbackT pcb(csb, funcs, geometry, pose, queryVolume, unitDir, distance, hitFlags, false);

	const MyPrunerFilter pf(filterData);
	sqs.sweep(queryVolume, unitDir, pcb.mClosestHit.distance, pcb, &pf);

	if(pcb.mFoundHit)
	{
		static_cast<PxGeomSweepHit&>(hit) = pcb.mClosestHit;
		getActorShape(pcb.mClosestPayload, hit);
	}
	return pcb.mFoundHit;
}

bool PEEL_CSQS::_sweepClosest(	const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance, PxSweepHit& hit, PxHitFlags hitFlags,
								const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB) const
{
	commitUpdates();

	switch(geometry.getType())
	{
		case PxGeometryType::eSPHERE:		{ return _sweepClosestT<DefaultPrunerSphereSweepCallback>(hit, mQuerySystem, mCachedFuncs.mCachedSweepFuncs, geometry, pose, unitDir, distance, hitFlags, filterData, filterCB);	}
		case PxGeometryType::eCAPSULE:		{ return _sweepClosestT<DefaultPrunerCapsuleSweepCallback>(hit, mQuerySystem, mCachedFuncs.mCachedSweepFuncs, geometry, pose, unitDir, distance, hitFlags, filterData, filterCB);	}
		case PxGeometryType::eBOX:			{ return _sweepClosestT<DefaultPrunerBoxSweepCallback>(hit, mQuerySystem, mCachedFuncs.mCachedSweepFuncs, geometry, pose, unitDir, distance, hitFlags, filterData, filterCB);		}
		case PxGeometryType::eCONVEXMESH:	{ return _sweepClosestT<DefaultPrunerConvexSweepCallback>(hit, mQuerySystem, mCachedFuncs.mCachedSweepFuncs, geometry, pose, unitDir, distance, hitFlags, filterData, filterCB);	}
		default:							{ PX_ASSERT(0);	return false;	}
	}
}

///////////////////////////////////////////////////////////////////////////////

static PEEL_CSQS* gCSQS = NULL;

PxSceneQuerySystem* CreatePEELCustomSceneQuerySystem()
{
	const PxU64 contextID = 'PEEL';
	Pruner* staticPruner = PX_NEW(AABBPruner)(true, contextID, COMPANION_PRUNER_INCREMENTAL);
	Pruner* dynamicPruner = PX_NEW(AABBPruner)(true, contextID, COMPANION_PRUNER_INCREMENTAL);

	gCSQS = PX_NEW(PEEL_CSQS)(contextID, staticPruner, dynamicPruner, 100, PxSceneQueryUpdateMode::eBUILD_ENABLED_COMMIT_ENABLED);
	return gCSQS;
}

bool PEEL_RaycastClosest(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, PxRaycastHit& hit, PxHitFlags hitFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	return gCSQS->_raycastClosest(origin, unitDir, distance, hit, hitFlags, filterData, filterCB);
}

bool PEEL_RaycastAny(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	return gCSQS->_raycastAny(origin, unitDir, distance, filterData, filterCB);
}

bool PEEL_OverlapAny(const PxGeometry& geometry, const PxTransform& pose, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	return gCSQS->_overlapAny(geometry, pose, filterData, filterCB);
}

PxU32 PEEL_OverlapAll(const PxGeometry& geometry, const PxTransform& pose, PxOverlapHit* hits, PxU32 nb, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	return gCSQS->_overlapAll(geometry, pose, hits, nb, filterData, filterCB);
}

bool PEEL_SweepClosest(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance, PxSweepHit& hit, PxHitFlags hitFlags, const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCB)
{
	return gCSQS->_sweepClosest(geometry, pose, unitDir, distance, hit, hitFlags, filterData, filterCB);
}
