///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX5 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX5_Vehicles.h"
#include "PINT_CommonPhysX3_DebugViz.h"

// This is a custom version of PhysX 5's (legacy) batch query ext, that records & renders scene queries

#ifdef PHYSX_SUPPORT_VEHICLE_XP
static const bool gExperiment = true;
static const bool gLogXP = false;

static PxSweepHit gHits[4];
static PxTransform gVehWheelTransform[4];
#endif

void recordIgnoredPoint(const PxTransform& vehWheelTransform,
						const PxRigidActor* actor, const PxShape* shape, const PxVec3& pt, const PxVec3& normal, float sep, PxU32 faceIndex, PxU32 index)
{
#ifdef PHYSX_SUPPORT_VEHICLE_XP
	ASSERT(index<4);

	//if(sep<=0.0f)
	if(!gHits[index].actor || sep<gHits[index].distance)
	{
		gVehWheelTransform[index]	= vehWheelTransform;
		gHits[index].actor		= (PxRigidActor*)actor;
		gHits[index].shape		= (PxShape*)shape;
		gHits[index].position	= pt;
		gHits[index].normal		= normal;
		gHits[index].distance	= sep;
		gHits[index].faceIndex	= faceIndex;
	}
#endif
}

void resetIgnoredPoints()
{
#ifdef PHYSX_SUPPORT_VEHICLE_XP
	for(PxU32 i=0;i<4;i++)
	{
		gHits[i].actor = NULL;
		gHits[i].shape = NULL;
	}
#endif
}

namespace
{
	struct Raycast
	{
		PxVec3 origin;
		PxVec3 unitDir;
		PxReal distance;
		PxHitFlags hitFlags;
		PxQueryFilterData filterData;
		const PxQueryCache* cache;
	};

	struct Sweep
	{
		PxGeometryHolder geometry;
		PxTransform pose;
		PxVec3 unitDir;
		PxReal distance;
		PxHitFlags hitFlags;
		PxQueryFilterData filterData;
		const PxQueryCache* cache;
		PxReal inflation;
	};

	struct Overlap
	{
		PxGeometryHolder geometry;
		PxTransform pose;
		PxQueryFilterData filterData;
		const PxQueryCache* cache;
	};

	template<typename HitType>
	struct NpOverflowBuffer : PxHitBuffer<HitType>
	{
		bool overflow;
		bool processCalled;
		PxU32 saveNbTouches;
		NpOverflowBuffer(HitType* hits, PxU32 count) : PxHitBuffer<HitType>(hits, count), overflow(false), processCalled(false), saveNbTouches(0)
		{
		}

		virtual PxAgain processTouches(const HitType* /*hits*/, PxU32 /*count*/)
		{
			if (processCalled)
				return false;
			saveNbTouches = this->nbTouches;
			processCalled = true;
			return true;
		}

		virtual void finalizeQuery()
		{
			if (processCalled)
			{
				overflow = (this->nbTouches > 0);
				this->nbTouches = saveNbTouches;
			}
		}
	};

	class ExtBatchQuery : public MyBatchQueryExt
	{
		// Keep track of casts for debug viz
		SQRecorder		mRecorder;

	public:

		virtual	void	enableSQRecorder(bool b)
		{
			mRecorder.SetEnabled(b);
		}

		virtual	void	resetSQRecorder()
		{
			mRecorder.Reset();
		}

		virtual	void	drawSQRaycasts(PintRender& renderer)
		{
			mRecorder.DrawRaycasts(renderer, mRaycasts.mBuffers);
		}

		virtual	void	drawSQSweeps(PintRender& renderer, bool drawGeometry)
		{
			mRecorder.DrawSweeps(renderer, mSweeps.mBuffers, drawGeometry);
		}

		ExtBatchQuery(
			const PxScene& scene,
			PxQueryFilterCallback* queryFilterCallback,
			PxRaycastBuffer* raycastBuffers, Raycast* raycastQueries, const PxU32 maxNbRaycasts, PxRaycastHit* raycastTouches, const PxU32 maxNbRaycastTouches,
			PxSweepBuffer* sweepBuffers, Sweep* sweepQueries, const PxU32 maxNbSweeps, PxSweepHit* sweepTouches, const PxU32 maxNbSweepTouches,
			PxOverlapBuffer* overlapBuffers, Overlap* overlapQueries, const PxU32 maxNbOverlaps, PxOverlapHit* overlapTouches, const PxU32 maxNbOverlapTouches);

		~ExtBatchQuery() {}

		virtual void release();

		virtual PxRaycastBuffer* raycast(
			const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, const PxU16 maxNbTouches,
			PxHitFlags hitFlags = PxHitFlags(PxHitFlag::eDEFAULT),
			const PxQueryFilterData& filterData = PxQueryFilterData(),
			const PxQueryCache* cache = NULL);

		virtual PxSweepBuffer* sweep(
			const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance, const PxU16 maxNbTouches,
			PxHitFlags hitFlags = PxHitFlags(PxHitFlag::eDEFAULT),
			const PxQueryFilterData& filterData = PxQueryFilterData(),
			const PxQueryCache* cache = NULL,
			const PxReal inflation = 0.f);

		virtual PxOverlapBuffer* overlap(
			const PxGeometry& geometry, const PxTransform& pose, PxU16 maxNbTouches = 0,
			const PxQueryFilterData& filterData = PxQueryFilterData(),
			const PxQueryCache* cache = NULL);

		virtual void execute();

	private:

		PX_NOCOPY(ExtBatchQuery)

		template<typename HitType, typename QueryType> struct Query
		{
			PxHitBuffer<HitType>* mBuffers;
			QueryType* mQueries;
			PxU32 mMaxNbBuffers;
			HitType* mTouches;
			PxU32 mMaxNbTouches;

			PxU32 mBufferTide;

			Query()
				: mBuffers(NULL),
				mQueries(NULL),
				mMaxNbBuffers(0),
				mTouches(NULL),
				mMaxNbTouches(0),
				mBufferTide(0)
			{
			}

			Query(PxHitBuffer<HitType>* buffers, QueryType* queries, const PxU32 maxNbBuffers, HitType* touches, const PxU32 maxNbTouches)
				: mBuffers(buffers),
				mQueries(queries),
				mMaxNbBuffers(maxNbBuffers),
				mTouches(touches),
				mMaxNbTouches(maxNbTouches),
				mBufferTide(0)
			{
				for (PxU32 i = 0; i < mMaxNbBuffers; i++)
				{
					mBuffers[i].hasBlock = false;
					mBuffers[i].nbTouches = 0;
				}
			}

			PxHitBuffer<HitType>* addQuery(const QueryType& query, const PxU32 maxNbTouches)
			{
				if ((mBufferTide + 1) > mMaxNbBuffers)
				{
					//Ran out of queries.
					return NULL;
				}

				PxHitBuffer<HitType>* buffer = mBuffers + mBufferTide;
				buffer->touches = NULL;
				buffer->maxNbTouches = maxNbTouches;
				buffer->hasBlock = false;
				buffer->nbTouches = 0xffffffff;

				mQueries[mBufferTide] = query;

				mBufferTide++;

				return buffer;
			}

			static void performQuery(const PxScene& scene, const Raycast& query, NpOverflowBuffer<PxRaycastHit>& hitBuffer, PxQueryFilterCallback* qfcb, SQRecorder& recorder, PxU32 index)
			{
				scene.raycast(
					query.origin, query.unitDir, query.distance,
					hitBuffer,
					query.hitFlags,
					query.filterData, qfcb,
					query.cache);

				recorder.RecordRaycast(query.origin, query.unitDir, query.distance, index);
			}

			static void performQuery(const PxScene& scene, const Sweep& query, NpOverflowBuffer<PxSweepHit>& hitBuffer, PxQueryFilterCallback* qfcb, SQRecorder& recorder, PxU32 index)
			{
				bool status = scene.sweep(
					query.geometry.any(), query.pose, query.unitDir, query.distance,
					hitBuffer,
					query.hitFlags,
					query.filterData, qfcb,
					query.cache,
					query.inflation);

				/*if(status && hitBuffer.hasBlock && hitBuffer.block.distance==0.0f)
				{
					status = scene.sweep(
						PxSphereGeometry(0.001f), query.pose, query.unitDir, query.distance,
						hitBuffer,
						query.hitFlags,
						query.filterData, qfcb,
						query.cache,
						query.inflation);
				}*/

					//#############
	/*			if(hitBuffer.hasBlock)
				{
					hitBuffer.block.normal = PxVec3(0.0f, 1.0f, 0.0f);
				}*/

#ifdef PHYSX_SUPPORT_VEHICLE_XP
				if(gExperiment)
				{
					if(gLogXP)
						printf("sweep impact dist: %f\n", hitBuffer.block.distance);
					if(gHits[index].actor)
					{
						if(gLogXP)
							printf("contact pen depth: %f\n", gHits[index].distance);

						//const PxVec3& sweepStart = query.pose.p;
						//float dp = sweepStart.dot(query.unitDir);

						const PxVec3& delta2 = gVehWheelTransform[index].p - query.pose.p;
						float dp = delta2.dot(query.unitDir);
						if(gLogXP)
							printf("dp0:               %f\n", dp);

						const PxVec3& ttt = gHits[index].normal*gHits[index].distance;
						dp -= ttt.dot(query.unitDir);


/*						const PxVec3& newPos = gHits[index].position - gHits[index].normal*gHits[index].distance;	//###

/*						const PxVec3& delta = gHits[index].position - query.pose.p;
						//const PxVec3& delta = newPos - query.pose.p;
						float dp = delta.dot(query.unitDir) - 0.5f;
						printf("dp:                %f\n", dp);*/

						//printf("%f %f %f\n", gHits[index].position.x, gHits[index].position.y, gHits[index].position.z);
						//printf("%f %f %f\n", hitBuffer.block.position.x, hitBuffer.block.position.y, hitBuffer.block.position.z);

						//float ddd = hitBuffer.block.distance;
						//float delta = gHits[index].distance - ddd;
						hitBuffer.hasBlock = true;
						hitBuffer.block = gHits[index];
							//hitBuffer.block.normal = gHits[index].normal;
							//hitBuffer.block.position = gHits[index].position - gHits[index].normal*gHits[index].distance;	//###
						//hitBuffer.block.distance += 0.8f;
						//hitBuffer.block.distance = 0.8f - hitBuffer.block.distance;
						//hitBuffer.block.distance += dp;
						hitBuffer.block.distance = dp;
						//hitBuffer.block.distance = dp0;
						//hitBuffer.block.distance = dp - hitBuffer.block.distance;
						//hitBuffer.block.distance = dp + gHits[index].distance;
						if(gLogXP)
							printf("fake sweep dist:   %f\n\n", hitBuffer.block.distance);
					}
					else
					{
						// ### looks like sometimes we miss contacts that we should have received
						hitBuffer.hasBlock = false;
					}
					return;
				}
#endif

				recorder.RecordSweep(query.geometry.any(), query.pose, query.unitDir, query.distance, index);
			}

			static void performQuery(const PxScene& scene, const Overlap& query, NpOverflowBuffer<PxOverlapHit>& hitBuffer, PxQueryFilterCallback* qfcb, SQRecorder& recorder, PxU32 index)
			{
				scene.overlap(
					query.geometry.any(), query.pose,
					hitBuffer,
					query.filterData, qfcb,
					query.cache);
			}

			void execute(const PxScene& scene, PxQueryFilterCallback* qfcb, SQRecorder& recorder)
			{
				PxU32 touchesTide = 0;
				for (PxU32 i = 0; i < mBufferTide; i++)
				{
					PX_ASSERT(0xffffffff == mBuffers[i].nbTouches);
					PX_ASSERT(0xffffffff != mBuffers[i].maxNbTouches);
					PX_ASSERT(!mBuffers[i].touches);

					bool noTouchesRemaining = false;
					if (mBuffers[i].maxNbTouches > 0)
					{
						if (touchesTide >= mMaxNbTouches)
						{
							//No resources left.
							mBuffers[i].maxNbTouches = 0;
							mBuffers[i].touches = NULL;
							noTouchesRemaining = true;
						}
						else if ((touchesTide + mBuffers[i].maxNbTouches) > mMaxNbTouches)
						{
							//Some resources left but not enough to match requested number.
							//This might be enough but it depends on the number of hits generated by the query.
							mBuffers[i].maxNbTouches = mMaxNbTouches - touchesTide;
							mBuffers[i].touches = mTouches + touchesTide;
						}
						else
						{
							//Enough resources left to match request.
							mBuffers[i].touches = mTouches + touchesTide;
						}
					}

					bool overflow = false;
					{
						PX_ALIGN(16, NpOverflowBuffer<HitType> overflowBuffer)(mBuffers[i].touches, mBuffers[i].maxNbTouches);
						performQuery(scene, mQueries[i], overflowBuffer, qfcb, recorder, i);
						overflow = overflowBuffer.overflow || noTouchesRemaining;
						mBuffers[i].hasBlock = overflowBuffer.hasBlock;
						mBuffers[i].block = overflowBuffer.block;
						mBuffers[i].nbTouches = overflowBuffer.nbTouches;
					}

					if(overflow)
					{
						mBuffers[i].maxNbTouches = 0xffffffff;
					}
					touchesTide += mBuffers[i].nbTouches;
				}

				mBufferTide = 0;
			}
		};

		const PxScene& mScene;
		PxQueryFilterCallback* mQueryFilterCallback;

		Query<PxRaycastHit, Raycast> mRaycasts;
		Query<PxSweepHit, Sweep> mSweeps;
		Query<PxOverlapHit, Overlap> mOverlaps;
	};

	template<typename HitType>
	class ExtBatchQueryDesc
	{
	public:
		ExtBatchQueryDesc(const PxU32 maxNbResults, const PxU32 maxNbTouches)
			: mResults(NULL),
			  mMaxNbResults(maxNbResults),
			  mTouches(NULL),
			  mMaxNbTouches(maxNbTouches)
		{
		}
		ExtBatchQueryDesc(PxHitBuffer<HitType>* results, const PxU32 maxNbResults, HitType* touches, PxU32 maxNbTouches)
			: mResults(results),
			  mMaxNbResults(maxNbResults),
			  mTouches(touches),
			  mMaxNbTouches(maxNbTouches)
		{
		}

		PX_FORCE_INLINE PxHitBuffer<HitType>* getResults() const { return mResults; }
		PX_FORCE_INLINE PxU32 getNbResults() const { return mMaxNbResults; }
		PX_FORCE_INLINE HitType* getTouches() const { return mTouches; }
		PX_FORCE_INLINE PxU32 getNbTouches() const { return mMaxNbTouches; }

	private:
		PxHitBuffer<HitType>* mResults;
		PxU32 mMaxNbResults;
		HitType* mTouches;
		PxU32 mMaxNbTouches;
	};

	template <typename HitType, typename QueryType>
	PxU32 computeByteSize(const ExtBatchQueryDesc<HitType>& queryDesc)
	{
		PxU32 byteSize = 0;
		if (queryDesc.getNbResults() > 0)
		{
			byteSize += sizeof(QueryType)*queryDesc.getNbResults();
			if (!queryDesc.getResults())
			{
				byteSize += sizeof(PxHitBuffer<HitType>)*queryDesc.getNbResults() + sizeof(HitType)*queryDesc.getNbTouches();
			}
		}
		return byteSize;
	}

	template <typename HitType, typename QueryType> PxU8* parseDesc
	(PxU8* bufIn, const ExtBatchQueryDesc<HitType>& queryDesc,
		PxHitBuffer<HitType>*& results,
		QueryType*& queries,
		PxU32& maxBufferSize,
		HitType*& touches,
		PxU32& maxNbTouches)
	{
		PxU8* bufOut = bufIn;

		results = queryDesc.getResults();
		queries = NULL;
		maxBufferSize = queryDesc.getNbResults();
		touches = queryDesc.getTouches();
		maxNbTouches = queryDesc.getNbTouches();

		if (maxBufferSize > 0)
		{
			queries = reinterpret_cast<QueryType*>(bufOut);
			bufOut += sizeof(QueryType)*maxBufferSize;

			if (!results)
			{
				results = reinterpret_cast<PxHitBuffer<HitType>*>(bufOut);
				for (PxU32 i = 0; i < maxBufferSize; i++)
				{
					PX_PLACEMENT_NEW(results + i, PxHitBuffer<HitType>);
				}
				bufOut += sizeof(PxHitBuffer<HitType>)*maxBufferSize;

				if (maxNbTouches > 0)
				{
					touches = reinterpret_cast<HitType*>(bufOut);
					bufOut += sizeof(HitType)*maxNbTouches;
				}
			}
		}

		return bufOut;
	}

	MyBatchQueryExt* create
	(const PxScene& scene, PxQueryFilterCallback* queryFilterCallback,
	 const ExtBatchQueryDesc<PxRaycastHit>& raycastDesc, const ExtBatchQueryDesc<PxSweepHit>& sweepDesc, const ExtBatchQueryDesc<PxOverlapHit>& overlapDesc)
	{
		const PxU32 byteSize =
			sizeof(ExtBatchQuery) +
			computeByteSize<PxRaycastHit, Raycast>(raycastDesc) +
			computeByteSize<PxSweepHit, Sweep>(sweepDesc) +
			computeByteSize<PxOverlapHit, Overlap>(overlapDesc);

		PxAllocatorCallback& allocator = *PxGetAllocatorCallback();

		PxU8* buf = reinterpret_cast<PxU8*>(allocator.allocate(byteSize, "NpBatchQueryExt", __FILE__, __LINE__));
		PX_CHECK_AND_RETURN_NULL(buf, "PxCreateBatchQueryExt - alllocation failed");
		ExtBatchQuery* bq = reinterpret_cast<ExtBatchQuery*>(buf);
		buf += sizeof(ExtBatchQuery);

		PxHitBuffer<PxRaycastHit>* raycastBuffers = NULL;
		Raycast* raycastQueries = NULL;
		PxU32 maxNbRaycasts = 0;
		PxRaycastHit* raycastTouches = NULL;
		PxU32 maxNbRaycastTouches = 0;
		buf = parseDesc<PxRaycastHit, Raycast>(buf, raycastDesc, raycastBuffers, raycastQueries, maxNbRaycasts, raycastTouches, maxNbRaycastTouches);

		PxHitBuffer<PxSweepHit>* sweepBuffers = NULL;
		Sweep* sweepQueries = NULL;
		PxU32 maxNbSweeps = 0;
		PxSweepHit* sweepTouches = NULL;
		PxU32 maxNbSweepTouches = 0;
		buf = parseDesc<PxSweepHit, Sweep>(buf, sweepDesc, sweepBuffers, sweepQueries, maxNbSweeps, sweepTouches, maxNbSweepTouches);

		PxHitBuffer<PxOverlapHit>* overlapBuffers = NULL;
		Overlap* overlapQueries = NULL;
		PxU32 maxNbOverlaps = 0;
		PxOverlapHit* overlapTouches = NULL;
		PxU32 maxNbOverlapTouches = 0;
		buf = parseDesc<PxOverlapHit, Overlap>(buf, overlapDesc, overlapBuffers, overlapQueries, maxNbOverlaps, overlapTouches, maxNbOverlapTouches);

		PX_ASSERT((reinterpret_cast<PxU8*>(bq) + byteSize) == buf);

		PX_PLACEMENT_NEW(bq, ExtBatchQuery)(
			scene, queryFilterCallback,
			raycastBuffers, raycastQueries, maxNbRaycasts, raycastTouches, maxNbRaycastTouches,
			sweepBuffers, sweepQueries, maxNbSweeps, sweepTouches, maxNbSweepTouches,
			overlapBuffers, overlapQueries, maxNbOverlaps, overlapTouches, maxNbOverlapTouches);

		return bq;
	}

	ExtBatchQuery::ExtBatchQuery
	(const PxScene& scene, PxQueryFilterCallback* queryFilterCallback,
	 PxRaycastBuffer* raycastBuffers, Raycast* raycastQueries, const PxU32 maxNbRaycasts, PxRaycastHit* raycastTouches, const PxU32 maxNbRaycastTouches,
	 PxSweepBuffer* sweepBuffers, Sweep* sweepQueries, const PxU32 maxNbSweeps, PxSweepHit* sweepTouches, const PxU32 maxNbSweepTouches,
	 PxOverlapBuffer* overlapBuffers, Overlap* overlapQueries, const PxU32 maxNbOverlaps, PxOverlapHit* overlapTouches, const PxU32 maxNbOverlapTouches)
		: mScene(scene),
		  mQueryFilterCallback(queryFilterCallback)
	{
		typedef Query<PxRaycastHit, Raycast> QueryRaycast;
		typedef Query<PxSweepHit, Sweep> QuerySweep;
		typedef Query<PxOverlapHit, Overlap> QueryOverlap;
		PX_PLACEMENT_NEW(&mRaycasts, QueryRaycast)(raycastBuffers, raycastQueries, maxNbRaycasts, raycastTouches, maxNbRaycastTouches);
		PX_PLACEMENT_NEW(&mSweeps, QuerySweep)(sweepBuffers, sweepQueries, maxNbSweeps, sweepTouches, maxNbSweepTouches);
		PX_PLACEMENT_NEW(&mOverlaps, QueryOverlap)(overlapBuffers, overlapQueries, maxNbOverlaps, overlapTouches, maxNbOverlapTouches);
	}

	void ExtBatchQuery::release()
	{
		mRecorder.Release();
		PxGetAllocatorCallback()->deallocate(this);
	}

	PxRaycastBuffer* ExtBatchQuery::raycast
	(const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, 
	 const PxU16 maxNbTouches,
	 PxHitFlags hitFlags,
	 const PxQueryFilterData& filterData,
	 const PxQueryCache* cache)
	{
		const PxQueryFilterData qfd(filterData.data, filterData.flags | PxQueryFlag::eBATCH_QUERY_LEGACY_BEHAVIOUR);
		const Raycast raycast = { origin, unitDir, distance, hitFlags, qfd, cache };
		PxRaycastBuffer* buffer = mRaycasts.addQuery(raycast, maxNbTouches);
		PX_CHECK_MSG(buffer, "PxBatchQueryExt::raycast - number of raycast() calls exceeds maxNbRaycasts. query discarded");
		return buffer;
	}

	PxSweepBuffer* ExtBatchQuery::sweep
	(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance, 
	 const PxU16 maxNbTouches,
	 PxHitFlags hitFlags,
	 const PxQueryFilterData& filterData,
	 const PxQueryCache* cache, 
	 const PxReal inflation)
	{
		const PxQueryFilterData qfd(filterData.data, filterData.flags | PxQueryFlag::eBATCH_QUERY_LEGACY_BEHAVIOUR);
		const Sweep sweep = { geometry, pose, unitDir, distance, hitFlags, qfd, cache, inflation};
		PxSweepBuffer* buffer = mSweeps.addQuery(sweep, maxNbTouches);
		PX_CHECK_MSG(buffer, "PxBatchQueryExt::sweep - number of sweep() calls exceeds maxNbSweeps. query discarded");
		return buffer;
	}

	PxOverlapBuffer* ExtBatchQuery::overlap
	(const PxGeometry& geometry, const PxTransform& pose, PxU16 maxNbTouches,
	 const PxQueryFilterData& filterData,
	 const PxQueryCache* cache)
	{
		const PxQueryFilterData qfd(filterData.data, filterData.flags | PxQueryFlag::eBATCH_QUERY_LEGACY_BEHAVIOUR);
		const Overlap overlap = { geometry, pose, qfd, cache};
		PxOverlapBuffer* buffer = mOverlaps.addQuery(overlap, maxNbTouches);
		PX_CHECK_MSG(buffer, "PxBatchQueryExt::overlap - number of overlap() calls exceeds maxNbOverlaps. query discarded");
		return buffer;
	}

	void ExtBatchQuery::execute()
	{
		mRaycasts.execute(mScene, mQueryFilterCallback, mRecorder);
		mSweeps.execute(mScene, mQueryFilterCallback, mRecorder);
		mOverlaps.execute(mScene, mQueryFilterCallback, mRecorder);
	}
}

MyBatchQueryExt* createMyBatchQueryExt(
const PxScene& scene, PxQueryFilterCallback* queryFilterCallback,
const PxU32 maxNbRaycasts, const PxU32 maxNbRaycastTouches,
const PxU32 maxNbSweeps, const PxU32 maxNbSweepTouches,
const PxU32 maxNbOverlaps, const PxU32 maxNbOverlapTouches)
{
	PX_CHECK_AND_RETURN_NULL(!((0 != maxNbRaycastTouches) && (0 == maxNbRaycasts)),
		"PxCreateBatchQueryExt - maxNbRaycastTouches is non-zero but maxNbRaycasts is zero");
	PX_CHECK_AND_RETURN_NULL(!((0 != maxNbSweepTouches) && (0 == maxNbSweeps)),
		"PxCreateBatchQueryExt - maxNbSweepTouches is non-zero but maxNbSweeps is zero");
	PX_CHECK_AND_RETURN_NULL(!((0 != maxNbOverlapTouches) && (0 == maxNbOverlaps)),
		"PxCreateBatchQueryExt - maxNbOverlaps is non-zero but maxNbOverlaps is zero");

	return create(scene, queryFilterCallback,
		ExtBatchQueryDesc<PxRaycastHit>(maxNbRaycasts, maxNbRaycastTouches),
		ExtBatchQueryDesc<PxSweepHit>(maxNbSweeps, maxNbSweepTouches),
		ExtBatchQueryDesc<PxOverlapHit>(maxNbOverlaps, maxNbOverlapTouches));
}
