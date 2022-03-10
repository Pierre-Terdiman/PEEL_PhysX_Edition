///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolBullet.h"
#include "GLRenderHelpers.h"
#include "TestScenes.h"

extern	PhysicsTest*	gRunningTest;

///////////////////////////////////////////////////////////////////////////////

static void gBulletRenderCallback(const Point& p0, const Point& p1, void* user_data)
{
	const Point White(1.0f, 1.0f, 1.0f);
	GLRenderHelpers::DrawLine(p0, p1, White);
	//GLRenderHelpers::DrawLine(B->mOldPos, B->mCurPos, pint.GetMainColor());
}

static void gBulletHitCallback(const PintRaycastHit& hit, const Point& start_pos, const Ray& world_ray, void* user_data)
{
	Pint* pint = (Pint*)user_data;
	if(!pint)
		return;

	if(gRunningTest)
		if(!gRunningTest->OnBulletHit(*pint, hit, world_ray))
			pint->AddWorldImpulseAtWorldPos(hit.mTouchedActor, world_ray.mDir*10.0f, hit.mImpact);
}


// Based on the bullet manager from ICE / KP. We could unify all of them at some point.

namespace
{
	struct Bullet : public Allocateable
	{
		inline_		Bullet()	{}
		inline_		~Bullet()	{}

		Point		mStartPos;
		Ray			mRay;
		float		mSpeed;
	};
}

	typedef void (*BulletDeleteCallback)	(const Bullet& bullet, void* user_data, bool lost_bullet);
//	typedef bool (*BulletFilterCallback)	(/*RMMesh* touched,*/ void* user_data);
	typedef void (*BulletHitCallback)		(const PintRaycastHit& hit, const Point& start_pos, const Ray& world_ray, void* user_data);
	typedef void (*BulletRenderCallback)	(const Point& p0, const Point& p1, void* user_data);

///////////////////////////////////////////////////////////////////////////////

class InternalBullet
{
	public:
	Bullet					mData;
	BulletDeleteCallback	mDeleteCallback;
//	BulletFilterCallback	mFilterCallback;
	BulletHitCallback		mHitCallback;
	BulletRenderCallback	mRenderCallback;
	void*					mUserData;

	float					mRenderLength;
	Point					mImpact;
	bool					mHasTouched;
	bool					mFlybyTest;
	bool					mPad0;
	bool					mPad1;
};
CHECK_CONTAINER_ITEM(InternalBullet)

class InternalBulletRender
{
	public:
	Point					mStartPos;
	Point					mOldPos;
	Point					mCurPos;
	BulletRenderCallback	mRenderCallback;
	void*					mUserData;
};
CHECK_CONTAINER_ITEM(InternalBulletRender)

static Container* gBullets = null;
static Container* gRenderInfo = null;

static void DeleteBullet(InternalBullet* B, bool lost_bullet=false)
{
	if(B->mDeleteCallback)
		(B->mDeleteCallback)(B->mData, B->mUserData, lost_bullet);

	// Delete bullet
	const udword Last = (gBullets->GetNbEntries() / (sizeof(InternalBullet)/sizeof(udword))) - 1;
	const InternalBullet* C = reinterpret_cast<const InternalBullet*>(gBullets->GetEntries());
	*B = C[Last];
	gBullets->ForceSize(gBullets->GetNbEntries() - (sizeof(InternalBullet)/sizeof(udword)));
}

bool RegisterBullet(const Bullet& bullet, BulletDeleteCallback delete_callback, /*BulletFilterCallback filter_callback,*/ BulletHitCallback hit_callback, BulletRenderCallback render_callback, void* user_data, float render_length)
{
	if(!gBullets)
		gBullets = ICE_NEW(Container);

	InternalBullet* ib = reinterpret_cast<InternalBullet*>(gBullets->Reserve(sizeof(InternalBullet)/sizeof(udword)));
	ib->mData			= bullet;
	ib->mDeleteCallback	= delete_callback;
//	ib->mFilterCallback	= filter_callback;
	ib->mHitCallback	= hit_callback;
	ib->mRenderCallback	= render_callback;
	ib->mUserData		= user_data;
	ib->mHasTouched		= false;
	ib->mFlybyTest		= false;
	ib->mRenderLength	= render_length;
	return true;
}

static inline_ bool _Raycast(Pint& pint, PintRaycastHit& hit, const Point& origin, const Point& dir, float max_dist)
{
	PintRaycastData tmp;
	tmp.mOrigin		= origin;
	tmp.mDir		= dir;
	tmp.mMaxDist	= max_dist;
	return pint.BatchRaycasts(pint.mSQHelper->GetThreadContext(), 1, &hit, &tmp)!=0;
}

void EvolveBullets(/*RMScene* scene, const TimeInfo& time*/float elapsed_time)
{
	if(gRenderInfo)
		gRenderInfo->Reset();

	if(!gBullets)
		return;

	udword NbBullets = gBullets->GetNbEntries() / (sizeof(InternalBullet)/sizeof(udword));
	if(!NbBullets)
		return;

	//
	//const AABB& SceneBounds = scene->GetGlobalBox();
	AABB SceneBounds;
	SceneBounds.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), Point(1000.0f, 1000.0f, 1000.0f));

	InternalBullet* B = reinterpret_cast<InternalBullet*>(gBullets->GetEntries());
	while(NbBullets--)
	{
		// Make sure lost bullets eventually get deleted
		if(B->mHasTouched || SceneBounds.ContainsPoint(B->mData.mRay.mOrig))
		{
			const float Epsilon = 0.01f;
			if(!B->mHasTouched)
			{
				const float MaxDist = B->mData.mSpeed==0.0f ? MAX_FLOAT : (B->mData.mSpeed + Epsilon)*elapsed_time;

				Pint* pint = (Pint*)B->mUserData;

				PintRaycastHit Hit;
				if(_Raycast(*pint, Hit, B->mData.mRay.mOrig, B->mData.mRay.mDir, MaxDist))
				{
					B->mImpact = B->mData.mRay.mOrig + B->mData.mRay.mDir * Hit.mDistance;	// ### or just Hit.mPosition
					B->mHasTouched = true;
					if(B->mHitCallback)
						(B->mHitCallback)(Hit, B->mData.mStartPos, B->mData.mRay, B->mUserData);
				}
			}

			///////

			bool MustDeleteBullet = false;
			if(B->mHasTouched)
			{
				if(B->mRenderLength==0.0f)
				{
					MustDeleteBullet = true;
				}
				else
				{
					const Point CurPos = B->mImpact;
					const Point VP = B->mData.mRay.mOrig + B->mData.mRay.mDir * (B->mData.mSpeed*elapsed_time);

					float RL = B->mRenderLength;
					const float d = B->mData.mStartPos.Distance(VP);
					if(d<B->mRenderLength)
						RL = d;

					const Point OldPos = VP - RL * B->mData.mRay.mDir;

					const float d0 = B->mData.mStartPos.SquareDistance(CurPos);
					const float d1 = B->mData.mStartPos.SquareDistance(OldPos);
					if(d1>d0)
						MustDeleteBullet = true;
				}
			}

			///////

			InternalBulletRender* ibr = null;
			if(B->mRenderCallback && !MustDeleteBullet)
			{
				if(!gRenderInfo)
					gRenderInfo = ICE_NEW(Container);
				// Save render info
				ibr = reinterpret_cast<InternalBulletRender*>(gRenderInfo->Reserve(sizeof(InternalBulletRender)/sizeof(udword)));
				ibr->mRenderCallback	= B->mRenderCallback;
				ibr->mUserData			= B->mUserData;
				ibr->mStartPos			= B->mData.mStartPos;
				ibr->mOldPos			= B->mData.mRay.mOrig;
				if(B->mHasTouched)
				{
					ibr->mCurPos		= B->mImpact;
					const Point VP = B->mData.mRay.mOrig + B->mData.mRay.mDir * (B->mData.mSpeed*elapsed_time);

					float RL = B->mRenderLength;
					const float d = ibr->mStartPos.Distance(VP);
					if(d<B->mRenderLength)
						RL = d;

					ibr->mOldPos		= VP - RL * B->mData.mRay.mDir;
				}
				else
				{
					ibr->mCurPos		= B->mData.mRay.mOrig + B->mData.mRay.mDir * (B->mData.mSpeed*elapsed_time);

					float RL = B->mRenderLength;
					const float d = ibr->mStartPos.Distance(ibr->mCurPos);
					if(d<B->mRenderLength)
						RL = d;

					ibr->mOldPos		= ibr->mCurPos - RL * B->mData.mRay.mDir;
				}
				// Careful here: the segment can pierce the test plane over many frames (in slow motion for example).
				// But we want to play the sound only once.
//				if(!B->mFlybyTest)
//					B->mFlybyTest = FlybyTest(B->mData.mSkeleton, ibr->mOldPos, ibr->mCurPos, B->mData.mRay.mDir);
			}

			///////

			if(MustDeleteBullet)
			{
				DeleteBullet(B);
			}
			else
			{
				// Update ray for next time
				B->mData.mRay.mOrig += B->mData.mRay.mDir*(B->mData.mSpeed - Epsilon)*elapsed_time;
				B++;
			}
		}
		else
		{
			// Lost bullet
			DeleteBullet(B, true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

ToolBullet::ToolBullet() : mDelay(0), mIsFiring(false)
{
}

ToolBullet::~ToolBullet()
{
	DELETESINGLE(gRenderInfo);
	DELETESINGLE(gBullets);
}

void ToolBullet::Reset(udword pint_index)
{
	DELETESINGLE(gRenderInfo);
	DELETESINGLE(gBullets);
}

void ToolBullet::RightDownCallback(Pint& pint, udword pint_index)
{
	mIsFiring = true;
}

void ToolBullet::RightUpCallback(Pint& pint, udword pint_index)
{
	mIsFiring = false;
}

void ToolBullet::PreRenderCallback()
{
	EvolveBullets(1.0f/60.0f);
}

void ToolBullet::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	if(mDelay)
		mDelay--;

	if(mIsFiring && !mDelay)
	{
		//PintRaycastHit Hit;
		//if(_Raycast(pint, Hit, mOrigin, mDir, 5000.0f))
		{
			const Point StartPos = mOrigin - Point(0.0f, 0.5f, 0.0f);

			//Point Dir = Hit.mImpact - StartPos;
			//Dir.Normalize();
			Point Dir = mDir;

			Bullet b;
			b.mStartPos		= StartPos;
			b.mRay.mOrig	= b.mStartPos;
			b.mRay.mDir		= Dir;
			b.mSpeed		= 40.0f;
			bool status = RegisterBullet(b, null, gBulletHitCallback, gBulletRenderCallback, &pint, 2.0f);

		/*	Bullet b;
			b.mStartPos		= mOrigin;// + Point(0.0f, -4.0f, 0.0f);
			b.mRay.mOrig	= b.mStartPos;
			b.mRay.mDir		= mDir;
			b.mSpeed		= 10.0f;

			bool status = RegisterBullet(b, null, null, null, null, &pint, 1.0f);*/

//			mDelay = 6;
			mDelay = 2;
		}
	}

	if(!gRenderInfo)
		return;

	const udword NbBullets = gRenderInfo->GetNbEntries() / (sizeof(InternalBulletRender)/sizeof(udword));
	if(!NbBullets)
		return;

//	const Point White(1.0f, 1.0f, 1.0f);

	const InternalBulletRender* B = reinterpret_cast<const InternalBulletRender*>(gRenderInfo->GetEntries());
	const InternalBulletRender* Last = B + NbBullets;
	while(B!=Last)
	{
		//GLRenderHelpers::DrawLine(B->mOldPos, B->mCurPos, White);
		//GLRenderHelpers::DrawLine(B->mOldPos, B->mCurPos, pint.GetMainColor());

		if(B->mRenderCallback)
			(B->mRenderCallback)(B->mOldPos, B->mCurPos, B->mUserData);
		B++;
	}
}

///////////////////////////////////////////////////////////////////////////////

#include "GUI_Helpers.h"

void ToolBullet::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
}


