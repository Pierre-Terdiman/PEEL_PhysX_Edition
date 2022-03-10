///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_SQ_H
#define PINT_SQ_H

#include "PintDef.h"
#include "PintRenderPass.h"

	class Pint;
	class PintRender;
	class PintShapeRenderer;
	struct PintRaycastHit;
	struct PintMultipleHits;
	struct PintRaycastData;
	struct PintBoxSweepData;
	struct PintSphereSweepData;
	struct PintCapsuleSweepData;
	struct PintConvexSweepData;
	struct PintSphereOverlapData;
	struct PintBoxOverlapData;
	struct PintCapsuleOverlapData;
	struct PintConvexOverlapData;
	struct PintBooleanHit;

	template <typename Type>
	class Hits
	{
		public:
						Hits() :
							mNbHits		(0),
							mMaxNbHits	(0),
							mHits		(null)
						{
						}
						~Hits()
						{
							ASSERT(!mHits);
						}

			void		Reset()
						{
							mNbHits = 0;
							mMaxNbHits = 0;
							DELETEARRAY(mHits);
						}

			Type*		PrepareQuery(udword nb)
						{
							mNbHits = nb;
							if(nb>mMaxNbHits)
							{
								DELETEARRAY(mHits);
								mHits = ICE_NEW(Type)[nb];
								mMaxNbHits = nb;
							}
							return mHits;
						}

			udword		mNbHits;
			udword		mMaxNbHits;
			Type*		mHits;
	};

	// Manages Pint-dependent SQ-related data
	class PintSQ
	{
		public:
												PintSQ();
												~PintSQ();

				void							Init(Pint* owner);
		// Raycasts
				PintRaycastHit*					PrepareRaycastQuery					(udword nb, const PintRaycastData* data);
				PintBooleanHit*					PrepareRaycastAnyQuery				(udword nb, const PintRaycastData* data);
				PintMultipleHits*				PrepareRaycastAllQuery				(udword nb, const PintRaycastData* data);
		// Sweeps
				PintRaycastHit*					PrepareBoxSweepQuery				(udword nb, const PintBoxSweepData* data);
				PintRaycastHit*					PrepareSphereSweepQuery				(udword nb, const PintSphereSweepData* data);
				PintRaycastHit*					PrepareCapsuleSweepQuery			(udword nb, const PintCapsuleSweepData* data);
				PintRaycastHit*					PrepareConvexSweepQuery				(udword nb, const PintConvexSweepData* data);
		// Overlaps
				PintBooleanHit*					PrepareSphereOverlapAnyQuery		(udword nb, const PintSphereOverlapData* data);
				PintMultipleHits*				PrepareSphereOverlapObjectsQuery	(udword nb, const PintSphereOverlapData* data);
				PintBooleanHit*					PrepareBoxOverlapAnyQuery			(udword nb, const PintBoxOverlapData* data);
				PintMultipleHits*				PrepareBoxOverlapObjectsQuery		(udword nb, const PintBoxOverlapData* data);
				PintBooleanHit*					PrepareCapsuleOverlapAnyQuery		(udword nb, const PintCapsuleOverlapData* data);
				PintMultipleHits*				PrepareCapsuleOverlapObjectsQuery	(udword nb, const PintCapsuleOverlapData* data);
				PintBooleanHit*					PrepareConvexOverlapAnyQuery		(udword nb, const PintConvexOverlapData* data);
				PintMultipleHits*				PrepareConvexOverlapObjectsQuery	(udword nb, const PintConvexOverlapData* data);

				void							Render(PintRender& renderer, PintRenderPass render_pass, bool paused);
				void							RenderBatched(PintRender& renderer, PintRenderPass render_pass, bool paused);
				void							Reset();
				void							ResetHitData();

		inline_	PintSQThreadContext				GetThreadContext()			const	{ return mThreadContext;				}
		inline_	Container&						GetRaycastAllStream()				{ return mRaycastAllStream;				}
		inline_	Container&						GetSphereOverlapObjectsStream()		{ return mSphereOverlapObjectsStream;	}
		inline_	Container&						GetBoxOverlapObjectsStream()		{ return mBoxOverlapObjectsStream;		}
		inline_	Container&						GetCapsuleOverlapObjectsStream()	{ return mCapsuleOverlapObjectsStream;	}
		inline_	Container&						GetConvexOverlapObjectsStream()		{ return mConvexOverlapObjectsStream;	}

		private:
				Pint*							mOwner;
				PintSQThreadContext				mThreadContext;
				//
				Hits<PintRaycastHit>			mRaycasts;
				Hits<PintBooleanHit>			mRaycastsAny;
				Hits<PintMultipleHits>			mRaycastsAll;
				Container						mRaycastAllStream;
				//
				Hits<PintRaycastHit>			mBoxSweeps;
				Hits<PintRaycastHit>			mSphereSweeps;
				Hits<PintRaycastHit>			mCapsuleSweeps;
				Hits<PintRaycastHit>			mConvexSweeps;
				//
				Hits<PintBooleanHit>			mSphereOverlapAny;
				Hits<PintBooleanHit>			mBoxOverlapAny;
				Hits<PintBooleanHit>			mCapsuleOverlapAny;
				Hits<PintBooleanHit>			mConvexOverlapAny;
				Hits<PintMultipleHits>			mSphereOverlapObjects;
				Hits<PintMultipleHits>			mBoxOverlapObjects;
				Hits<PintMultipleHits>			mCapsuleOverlapObjects;
				Hits<PintMultipleHits>			mConvexOverlapObjects;
				Container						mSphereOverlapObjectsStream;
				Container						mBoxOverlapObjectsStream;
				Container						mCapsuleOverlapObjectsStream;
				Container						mConvexOverlapObjectsStream;

		// Following test-related data is only used for rendering results
				const PintRaycastData*			mRaycastData;
				//
				const PintBoxSweepData*			mBoxSweepData;
				const PintSphereSweepData*		mSphereSweepData;
				const PintCapsuleSweepData*		mCapsuleSweepData;
				const PintConvexSweepData*		mConvexSweepData;
				//
				const PintSphereOverlapData*	mSphereOverlapData;
				const PintBoxOverlapData*		mBoxOverlapData;
				const PintCapsuleOverlapData*	mCapsuleOverlapData;
				const PintConvexOverlapData*	mConvexOverlapData;

				Vertices						mBatchedLines_Main;
				Vertices						mBatchedLines_R;
				Vertices						mBatchedLines_G;
				Vertices						mBatchedLines_B;

				struct BatchedConvexData
				{
					PintShapeRenderer*	mRenderer;
					PR					mPose;
					//Point				mColor;
				};
				Container						mBatchedConvexes;

				void							ResetAllDataPointers();

		inline_ void							DrawImpact						(const PintRaycastHit& hit);
				void							DrawRaycastData					(udword nb, const PintRaycastData* data, const PintRaycastHit* hits, const Point& color);
				void							DrawRaycastAnyData				(udword nb, const PintRaycastData* data, const PintBooleanHit* hits, const Point& color);
				void							DrawRaycastAllData				(udword nb, const PintRaycastData* data, const PintMultipleHits* hits, const Point& color);

				void							DrawBoxSweepData				(udword nb, const PintBoxSweepData* data, const PintRaycastHit* hits, const Point& color);
				void							DrawSphereSweepData				(udword nb, const PintSphereSweepData* data, const PintRaycastHit* hits, const Point& color);
				void							DrawCapsuleSweepData			(udword nb, const PintCapsuleSweepData* data, const PintRaycastHit* hits, const Point& color);
				void							DrawConvexSweepData				(udword nb, const PintConvexSweepData* data, const PintRaycastHit* hits, const Point& color);

				void							DrawSphereOverlapAnyData		(udword nb, const PintSphereOverlapData* data, const PintBooleanHit* hits, const Point& color);
				void							DrawSphereOverlapObjectsData	(udword nb, const PintSphereOverlapData* data, const PintMultipleHits* hits, const Point& color);
				void							DrawBoxOverlapAnyData			(udword nb, const PintBoxOverlapData* data, const PintBooleanHit* hits, const Point& color);
				void							DrawBoxOverlapObjectsData		(udword nb, const PintBoxOverlapData* data, const PintMultipleHits* hits, const Point& color);
				void							DrawCapsuleOverlapAnyData		(udword nb, const PintCapsuleOverlapData* data, const PintBooleanHit* hits, const Point& color);
				void							DrawCapsuleOverlapObjectsData	(udword nb, const PintCapsuleOverlapData* data, const PintMultipleHits* hits, const Point& color);
				void							DrawConvexOverlapAnyData		(udword nb, const PintConvexOverlapData* data, const PintBooleanHit* hits, const Point& color);
				void							DrawConvexOverlapObjectsData	(udword nb, const PintConvexOverlapData* data, const PintMultipleHits* hits, const Point& color);
	};

#endif