///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintSQ.h"
#include "Pint.h"
#include "PintShapeRenderer.h"
#include "GLRenderHelpers.h"
#include "Shader.h"

extern bool gRenderSolidImpactShapes;

PintSQ::PintSQ() :
	mOwner				(null),
	mThreadContext		(null),
	//
	mRaycastData		(null),
	//
	mBoxSweepData		(null),
	mSphereSweepData	(null),
	mCapsuleSweepData	(null),
	mConvexSweepData	(null),
	//
	mSphereOverlapData	(null),
	mBoxOverlapData		(null),
	mCapsuleOverlapData	(null),
	mConvexOverlapData	(null)
{
}

PintSQ::~PintSQ()
{
	ASSERT(!mConvexOverlapData);
	ASSERT(!mCapsuleOverlapData);
	ASSERT(!mBoxOverlapData);
	ASSERT(!mSphereOverlapData);

	ASSERT(!mConvexSweepData);
	ASSERT(!mCapsuleSweepData);
	ASSERT(!mSphereSweepData);
	ASSERT(!mBoxSweepData);

	ASSERT(!mRaycastData);
}

void PintSQ::ResetAllDataPointers()
{
	mRaycastData		= null;

	mBoxSweepData		= null;
	mSphereSweepData	= null;
	mCapsuleSweepData	= null;
	mConvexSweepData	= null;

	mSphereOverlapData	= null;
	mBoxOverlapData		= null;
	mCapsuleOverlapData	= null;
	mConvexOverlapData	= null;
}

void PintSQ::ResetHitData()
{
	mRaycasts.Reset();
	mRaycastsAny.Reset();
	mRaycastsAll.Reset();
	//
	mBoxSweeps.Reset();
	mSphereSweeps.Reset();
	mCapsuleSweeps.Reset();
	mConvexSweeps.Reset();
	//
	mSphereOverlapAny.Reset();
	mBoxOverlapAny.Reset();
	mCapsuleOverlapAny.Reset();
	mConvexOverlapAny.Reset();
	mSphereOverlapObjects.Reset();
	mBoxOverlapObjects.Reset();
	mCapsuleOverlapObjects.Reset();
	mConvexOverlapObjects.Reset();

	mBatchedLines_Main.Empty();
	mBatchedLines_R.Empty();
	mBatchedLines_G.Empty();
	mBatchedLines_B.Empty();
	mBatchedConvexes.Empty();

	mRaycastAllStream.Empty();
	mSphereOverlapObjectsStream.Empty();
	mBoxOverlapObjectsStream.Empty();
	mCapsuleOverlapObjectsStream.Empty();
	mConvexOverlapObjectsStream.Empty();
}

void PintSQ::Reset()
{
	if(mThreadContext)
	{
		mOwner->ReleaseSQThreadContext(mThreadContext);
		mThreadContext = null;
	}
	mOwner = null;

	ResetHitData();

	ResetAllDataPointers();
}

void PintSQ::Init(Pint* owner)
{
	Reset();
	mOwner	= owner;
	mThreadContext = owner->CreateSQThreadContext();
	owner->mSQHelper = this;
}

///////////////////////////////////////////////////////////////////////////////

PintRaycastHit* PintSQ::PrepareRaycastQuery(udword nb, const PintRaycastData* data)
{
//	ResetAllDataPointers();
	mRaycastData = data;
	return mRaycasts.PrepareQuery(nb);
}

PintBooleanHit* PintSQ::PrepareRaycastAnyQuery(udword nb, const PintRaycastData* data)
{
//	ResetAllDataPointers();
	mRaycastData = data;
	return mRaycastsAny.PrepareQuery(nb);
}

PintMultipleHits* PintSQ::PrepareRaycastAllQuery(udword nb, const PintRaycastData* data)
{
//	ResetAllDataPointers();
	mRaycastData = data;
	mRaycastAllStream.Reset();
	return mRaycastsAll.PrepareQuery(nb);
}

///////////////////////////////////////////////////////////////////////////////

PintRaycastHit* PintSQ::PrepareBoxSweepQuery(udword nb, const PintBoxSweepData* data)
{
//	ResetAllDataPointers();
	mBoxSweepData = data;
	return mBoxSweeps.PrepareQuery(nb);
}

PintRaycastHit* PintSQ::PrepareSphereSweepQuery(udword nb, const PintSphereSweepData* data)
{
//	ResetAllDataPointers();
	mSphereSweepData = data;
	return mSphereSweeps.PrepareQuery(nb);
}

PintRaycastHit* PintSQ::PrepareCapsuleSweepQuery(udword nb, const PintCapsuleSweepData* data)
{
//	ResetAllDataPointers();
	mCapsuleSweepData = data;
	return mCapsuleSweeps.PrepareQuery(nb);
}

PintRaycastHit* PintSQ::PrepareConvexSweepQuery(udword nb, const PintConvexSweepData* data)
{
//	ResetAllDataPointers();
	mConvexSweepData = data;
	return mConvexSweeps.PrepareQuery(nb);
}

///////////////////////////////////////////////////////////////////////////////

PintBooleanHit* PintSQ::PrepareSphereOverlapAnyQuery(udword nb, const PintSphereOverlapData* data)
{
//	ResetAllDataPointers();
	mSphereOverlapData = data;
	return mSphereOverlapAny.PrepareQuery(nb);
}

PintMultipleHits* PintSQ::PrepareSphereOverlapObjectsQuery(udword nb, const PintSphereOverlapData* data)
{
//	ResetAllDataPointers();
	mSphereOverlapData = data;
	mSphereOverlapObjectsStream.Reset();
	return mSphereOverlapObjects.PrepareQuery(nb);
}

PintBooleanHit* PintSQ::PrepareBoxOverlapAnyQuery(udword nb, const PintBoxOverlapData* data)
{
//	ResetAllDataPointers();
	mBoxOverlapData = data;
	return mBoxOverlapAny.PrepareQuery(nb);
}

PintMultipleHits* PintSQ::PrepareBoxOverlapObjectsQuery(udword nb, const PintBoxOverlapData* data)
{
//	ResetAllDataPointers();
	mBoxOverlapData = data;
	mBoxOverlapObjectsStream.Reset();
	return mBoxOverlapObjects.PrepareQuery(nb);
}

PintBooleanHit* PintSQ::PrepareCapsuleOverlapAnyQuery(udword nb, const PintCapsuleOverlapData* data)
{
//	ResetAllDataPointers();
	mCapsuleOverlapData = data;
	return mCapsuleOverlapAny.PrepareQuery(nb);
}

PintMultipleHits* PintSQ::PrepareCapsuleOverlapObjectsQuery(udword nb, const PintCapsuleOverlapData* data)
{
//	ResetAllDataPointers();
	mCapsuleOverlapData = data;
	mCapsuleOverlapObjectsStream.Reset();
	return mCapsuleOverlapObjects.PrepareQuery(nb);
}

PintBooleanHit* PintSQ::PrepareConvexOverlapAnyQuery(udword nb, const PintConvexOverlapData* data)
{
//	ResetAllDataPointers();
	mConvexOverlapData = data;
	return mConvexOverlapAny.PrepareQuery(nb);
}

PintMultipleHits* PintSQ::PrepareConvexOverlapObjectsQuery(udword nb, const PintConvexOverlapData* data)
{
//	ResetAllDataPointers();
	mConvexOverlapData = data;
	mConvexOverlapObjectsStream.Reset();
	return mConvexOverlapObjects.PrepareQuery(nb);
}

///////////////////////////////////////////////////////////////////////////////

static inline_ void DrawLine(const Point& p0, const Point& p1, Vertices& buffer)
{
	buffer.AddVertex(p0).AddVertex(p1);
}

inline_ void PintSQ::DrawImpact(const PintRaycastHit& hit)
{
	mBatchedLines_R.AddVertex(hit.mImpact).AddVertex(hit.mImpact + Point(1.0f, 0.0f, 0.0f));
	mBatchedLines_G.AddVertex(hit.mImpact).AddVertex(hit.mImpact + Point(0.0f, 1.0f, 0.0f));
	mBatchedLines_B.AddVertex(hit.mImpact).AddVertex(hit.mImpact + Point(0.0f, 0.0f, 1.0f));
//	DrawLine(hit.mImpact, hit.mImpact + Point(1.0f, 0.0f, 0.0f), mBatchedLines_R);
//	DrawLine(hit.mImpact, hit.mImpact + Point(0.0f, 1.0f, 0.0f), mBatchedLines_G);
//	DrawLine(hit.mImpact, hit.mImpact + Point(0.0f, 0.0f, 1.0f), mBatchedLines_B);

	mBatchedLines_Main.AddVertex(hit.mImpact).AddVertex(hit.mImpact + hit.mNormal);
//	DrawLine(hit.mImpact, hit.mImpact + hit.mNormal, mBatchedLines_Main);
}

static void DrawWireframeBox(const OBB& box, const Point& offset, Vertices& buffer)
{
	Point Pts[8];
	box.ComputePoints(Pts);
	for(udword j=0;j<8;j++)
		Pts[j] += offset;

	const udword* Indices = box.GetEdges();
	for(udword j=0;j<12;j++)
	{
		const udword VRef0 = *Indices++;
		const udword VRef1 = *Indices++;
		DrawLine(Pts[VRef0], Pts[VRef1], buffer);
	}
}

static void DrawCircle(udword nb_segments, const Matrix4x4& matrix, Vertices& buffer, float radius, bool semi_circle)
{
	const float step = TWOPI / float(nb_segments);
	udword segs = nb_segments;
	if(semi_circle)
		segs /= 2;

	for(udword i=0;i<segs;i++)
	{
		udword j=i+1;
		if(j==nb_segments)
			j=0;

		const float angle0 = float(i) * step;
		const float angle1 = float(j) * step;

		const Point p0 = Point(radius * sinf(angle0), radius * cosf(angle0), 0.0f) * matrix;
		const Point p1 = Point(radius * sinf(angle1), radius * cosf(angle1), 0.0f) * matrix;
		DrawLine(p0, p1, buffer);
	}
}

static void DrawWireframeSphere(const Sphere& sphere, const Point& offset, Vertices& buffer)
{
	const udword NbSegments = 14;

	Matrix3x3 M;
	M.Identity();
	const Point R0 = M[0];
	const Point R1 = M[1];
	const Point R2 = M[2];

	Matrix4x4 Rot;
	Rot.SetTrans(sphere.mCenter + offset);

	Rot.SetRow(0, R0);
	Rot.SetRow(1, R1);
	Rot.SetRow(2, R2);
	DrawCircle(NbSegments, Rot, buffer, sphere.mRadius, false);

	Rot.SetRow(0, R1);
	Rot.SetRow(1, R2);
	Rot.SetRow(2, R0);
	DrawCircle(NbSegments, Rot, buffer, sphere.mRadius, false);

	Rot.SetRow(0, R2);
	Rot.SetRow(1, R0);
	Rot.SetRow(2, R1);
	DrawCircle(NbSegments, Rot, buffer, sphere.mRadius, false);
}

static void DrawWireframeCapsule(float r, float h, const PR& pose, const Point& offset, Vertices& buffer)
{
	const udword NbSegments = 14;

	const Point Pos = pose.mPos + offset;

	const Matrix3x3 M = pose.mRot;

	Point p0, p1;

	p1 = M[1];
	p1 *= 0.5f*h;
	p0 = -p1;
	p0 += Pos;
	p1 += Pos;

	const Point c0 = M[0];
	const Point c1 = M[1];
	const Point c2 = M[2];
	DrawLine(p0 + c0*r, p1 + c0*r, buffer);
	DrawLine(p0 - c0*r, p1 - c0*r, buffer);
	DrawLine(p0 + c2*r, p1 + c2*r, buffer);
	DrawLine(p0 - c2*r, p1 - c2*r, buffer);

	Matrix4x4 MM;
	MM.SetRow(0, -c1);
	MM.SetRow(1, -c0);
	MM.SetRow(2, c2);
	MM.SetTrans(p0);
	DrawCircle(NbSegments, MM, buffer, r, true);	//halfcircle -- flipped

	MM.SetRow(0, c1);
	MM.SetRow(1, -c0);
	MM.SetRow(2, c2);
	MM.SetTrans(p1);
	DrawCircle(NbSegments, MM, buffer, r, true);

	MM.SetRow(0, -c1);
	MM.SetRow(1, c2);
	MM.SetRow(2, c0);
	MM.SetTrans(p0);
	DrawCircle(NbSegments, MM, buffer, r, true);//halfcircle -- good

	MM.SetRow(0, c1);
	MM.SetRow(1, c2);
	MM.SetRow(2, c0);
	MM.SetTrans(p1);
	DrawCircle(NbSegments, MM, buffer, r, true);

	MM.SetRow(0, c2);
	MM.SetRow(1, c0);
	MM.SetRow(2, c1);
	MM.SetTrans(p0);
	DrawCircle(NbSegments, MM, buffer, r, false);	//full circle
	MM.SetTrans(p1);
	DrawCircle(NbSegments, MM, buffer, r, false);
}

void PintSQ::DrawRaycastData(udword nb, const PintRaycastData* data, const PintRaycastHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		const Point& Origin = data[i].mOrigin;
		if(hits[i].mTouchedActor)
		{
			mBatchedLines_Main.AddVertex(Origin).AddVertex(hits[i].mImpact);
			DrawImpact(hits[i]);
		}
		else
		{
			const Point EndPoint = data[i].mOrigin + data[i].mDir * data[i].mMaxDist;
			mBatchedLines_Main.AddVertex(Origin).AddVertex(EndPoint);
		}
	}
}

void PintSQ::DrawRaycastAnyData(udword nb, const PintRaycastData* data, const PintBooleanHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		const Point& Origin = data[i].mOrigin;
		const Point EndPoint = data[i].mOrigin + data[i].mDir * data[i].mMaxDist;
		if(hits[i].mHit)
			mBatchedLines_R.AddVertex(Origin).AddVertex(EndPoint);
		else
			mBatchedLines_G.AddVertex(Origin).AddVertex(EndPoint);
	}
}

void PintSQ::DrawRaycastAllData(udword nb, const PintRaycastData* data, const PintMultipleHits* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	const PintRaycastHit* Base = reinterpret_cast<const PintRaycastHit*>(mRaycastAllStream.GetEntries());

	for(udword i=0;i<nb;i++)
	{
		const Point& Origin = data[i].mOrigin;
		const udword NbHits = hits[i].mNbHits;
		if(NbHits)
		{
			const PintRaycastHit* Hits = Base + hits[i].mOffset;
			for(udword j=0;j<NbHits;j++)
			{
				mBatchedLines_Main.AddVertex(Origin).AddVertex(Hits[j].mImpact);
				DrawImpact(Hits[j]);
			}
		}
		else
		{
			const Point EndPoint = data[i].mOrigin + data[i].mDir * data[i].mMaxDist;
			mBatchedLines_Main.AddVertex(Origin).AddVertex(EndPoint);
		}
	}
}

void PintSQ::DrawBoxSweepData(udword nb, const PintBoxSweepData* data, const PintRaycastHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		// Draw solid box at initial position
		const Point& BoxStartPos = data[i].mBox.mCenter;
		const PR Pose(BoxStartPos, Quat(data[i].mBox.mRot));
		glColor4f(color.x, color.y, color.z, 1.0f);
		GLRenderHelpers::DrawBox(data[i].mBox.mExtents, Pose);

		if(hits[i].mTouchedActor)
		{
			// Motion vector
			const Point Delta = hits[i].mDistance * data[i].mDir;

			// Draw motion vector
			DrawLine(BoxStartPos, BoxStartPos + Delta, mBatchedLines_Main);

			// Draw wireframe box at impact position
			if(gRenderSolidImpactShapes)
				GLRenderHelpers::DrawBox(data[i].mBox.mExtents, PR(Pose.mPos+Delta, Pose.mRot));
			else
				DrawWireframeBox(data[i].mBox, Delta, mBatchedLines_Main);

			// Draw impact
			DrawImpact(hits[i]);
		}
		else
		{
			// Motion vector
			const Point Delta = data[i].mMaxDist * data[i].mDir;

			// Draw motion vector
			DrawLine(BoxStartPos, BoxStartPos + Delta, mBatchedLines_Main);

			// Draw wireframe box at end position
			DrawWireframeBox(data[i].mBox, Delta, mBatchedLines_Main);
		}
	}
}

void PintSQ::DrawSphereSweepData(udword nb, const PintSphereSweepData* data, const PintRaycastHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		// Draw solid sphere at initial position
		const Point& SphereStartPos = data[i].mSphere.mCenter;
		const PR Pose(SphereStartPos, Quat(Idt));
		glColor4f(color.x, color.y, color.z, 1.0f);
		GLRenderHelpers::DrawSphere(data[i].mSphere.mRadius, Pose);

		if(hits[i].mTouchedActor)
		{
			// Motion vector
			const Point Delta = hits[i].mDistance * data[i].mDir;

			// Draw motion vector
			DrawLine(SphereStartPos, SphereStartPos + Delta, mBatchedLines_Main);

			// Draw wireframe sphere at impact position
			if(gRenderSolidImpactShapes)
				GLRenderHelpers::DrawSphere(data[i].mSphere.mRadius, PR(Pose.mPos+Delta, Pose.mRot));
			else
				DrawWireframeSphere(data[i].mSphere, Delta, mBatchedLines_Main);

			// Draw impact
			DrawImpact(hits[i]);
		}
		else
		{
			// Motion vector
			const Point Delta = data[i].mMaxDist * data[i].mDir;

			// Draw motion vector
			DrawLine(SphereStartPos, SphereStartPos + Delta, mBatchedLines_Main);

			// Draw wireframe sphere at end position
			DrawWireframeSphere(data[i].mSphere, Delta, mBatchedLines_Main);
		}
	}
}

void PintSQ::DrawCapsuleSweepData(udword nb, const PintCapsuleSweepData* data, const PintRaycastHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		// Draw solid capsule at initial position
		const LSS& Capsule = data[i].mCapsule;
		const Point CapsuleStartPos = (Capsule.mP0 + Capsule.mP1)*0.5f;

		Point CapsuleAxis = Capsule.mP1 - Capsule.mP0;
		const float M = CapsuleAxis.Magnitude();
		CapsuleAxis /= M;
		const Quat q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), CapsuleAxis);

			const Quat qq = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
			Quat qqq = q * qq;

		const PR Pose(CapsuleStartPos, qqq);

		glColor4f(color.x, color.y, color.z, 1.0f);
//		GLRenderHelpers::DrawCapsule(Capsule.mRadius, M*0.5f, Pose);
		GLRenderHelpers::DrawCapsule(Capsule.mRadius, M, Pose);

		if(hits[i].mTouchedActor)
		{
			// Motion vector
			const Point Delta = hits[i].mDistance * data[i].mDir;

			// Draw motion vector
			DrawLine(CapsuleStartPos, CapsuleStartPos + Delta, mBatchedLines_Main);

			// Draw wireframe capsule at impact position
//			DrawWireframeCapsule(Capsule.mRadius, M*0.5f, Pose, Delta, color);
			if(gRenderSolidImpactShapes)
				GLRenderHelpers::DrawCapsule(Capsule.mRadius, M, PR(Pose.mPos + Delta, Pose.mRot));
			else
				DrawWireframeCapsule(Capsule.mRadius, M, Pose, Delta, mBatchedLines_Main);

			// Draw impact
			DrawImpact(hits[i]);
		}
		else
		{
			// Motion vector
			const Point Delta = data[i].mMaxDist * data[i].mDir;

			// Draw motion vector
			DrawLine(CapsuleStartPos, CapsuleStartPos + Delta, mBatchedLines_Main);

			// Draw wireframe capsule at end position
//			DrawWireframeCapsule(Capsule.mRadius, M*0.5f, Pose, Delta, color);
			DrawWireframeCapsule(Capsule.mRadius, M, Pose, Delta, mBatchedLines_Main);
		}
	}
}

void SetUserDefinedPolygonMode();
extern bool	gWireframePass;
static void DrawWireframeConvex(PintShapeRenderer* renderer, const PR& pose, const Point& color)
{
	glColor4f(color.x, color.y, color.z, 1.0f);

	const bool Saved = gWireframePass;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_LIGHTING);
	gWireframePass = true;
		renderer->_Render(pose);
	gWireframePass = Saved;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	SetUserDefinedPolygonMode();
	glEnable(GL_LIGHTING);
}

void PintSQ::DrawConvexSweepData(udword nb, const PintConvexSweepData* data, const PintRaycastHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	// ### this stuff doesn't work, when called the current shader is active and we cannot mix some debug lines
	// in the middle of it without disabling the shader.

	for(udword i=0;i<nb;i++)
	{
		// Draw solid convex at initial position
		const Point& ConvexStartPos = data[i].mTransform.mPos;

		glColor4f(color.x, color.y, color.z, 1.0f);
		RenderShape(data[i].mRenderer, data[i].mTransform);

		if(hits[i].mTouchedActor)
		{
			// Motion vector
			const Point Delta = hits[i].mDistance * data[i].mDir;

			// Draw motion vector
			DrawLine(ConvexStartPos, ConvexStartPos + Delta, mBatchedLines_Main);

			// Draw wireframe convex at impact position
			PR EndPose = data[i].mTransform;
			EndPose.mPos += Delta;
			if(gRenderSolidImpactShapes)
			{
				RenderShape(data[i].mRenderer, EndPose);
			}
			else
			{
			//DrawWireframeConvex(data[i].mRenderer, EndPose, color);
				BatchedConvexData* BCD = ICE_RESERVE(BatchedConvexData, mBatchedConvexes);

				BCD->mRenderer = data[i].mRenderer;
				BCD->mPose = EndPose;
				//BCD->mColor = color;
			}

			// Draw impact
			DrawImpact(hits[i]);
		}
		else
		{
			// Motion vector
			const Point Delta = data[i].mMaxDist * data[i].mDir;

			// Draw motion vector
			DrawLine(ConvexStartPos, ConvexStartPos + Delta, mBatchedLines_Main);

			// Draw wireframe convex at end position
			PR EndPose = data[i].mTransform;
			EndPose.mPos += Delta;
			//DrawWireframeConvex(data[i].mRenderer, EndPose, color);
				BatchedConvexData* BCD = ICE_RESERVE(BatchedConvexData, mBatchedConvexes);

				BCD->mRenderer = data[i].mRenderer;
				BCD->mPose = EndPose;
				//BCD->mColor = color;
		}
	}
}

void PintSQ::DrawSphereOverlapAnyData(udword nb, const PintSphereOverlapData* data, const PintBooleanHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		const Point& SphereStartPos = data[i].mSphere.mCenter;
		const PR Pose(SphereStartPos, Quat(Idt));
/*		glColor4f(color.x, color.y, color.z, 1.0f);
		::DrawLine(SphereStartPos, SphereStartPos + Point(0.0f, 100.0f, 0.0f), color);

		if(hits[i])
		{
			::DrawSphere(data[i].mSphere.mRadius, Pose);
		}
		else
		{
			DrawWireframeSphere(data[i].mSphere, Point(0.0f, 0.0f, 0.0f), color);
		}*/
		DrawWireframeSphere(data[i].mSphere, Point(0.0f, 0.0f, 0.0f), hits[i].mHit ? mBatchedLines_R : mBatchedLines_G);
	}
}

void PintSQ::DrawSphereOverlapObjectsData(udword nb, const PintSphereOverlapData* data, const PintMultipleHits* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		const Point& SphereStartPos = data[i].mSphere.mCenter;
		const PR Pose(SphereStartPos, Quat(Idt));
/*		glColor4f(color.x, color.y, color.z, 1.0f);
		::DrawLine(SphereStartPos, SphereStartPos + Point(0.0f, 100.0f, 0.0f), color);

		if(hits[i])
		{
			::DrawSphere(data[i].mSphere.mRadius, Pose);
		}
		else
		{
			DrawWireframeSphere(data[i].mSphere, Point(0.0f, 0.0f, 0.0f), color);
		}*/

		DrawWireframeSphere(data[i].mSphere, Point(0.0f, 0.0f, 0.0f), hits[i].mNbHits ? mBatchedLines_R : mBatchedLines_G);
	}
}

void PintSQ::DrawBoxOverlapAnyData(udword nb, const PintBoxOverlapData* data, const PintBooleanHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		const Point& BoxStartPos = data[i].mBox.mCenter;
		const PR Pose(BoxStartPos, Quat(data[i].mBox.mRot));
/*		glColor4f(color.x, color.y, color.z, 1.0f);

		if(hits[i])
		{
			::DrawBox(data[i].mBox.mExtents, Pose);
		}
		else
		{
			DrawWireframeBox(data[i].mBox, Point(0.0f, 0.0f, 0.0f), color);
		}*/

		DrawWireframeBox(data[i].mBox, Point(0.0f, 0.0f, 0.0f), hits[i].mHit ? mBatchedLines_R : mBatchedLines_G);
	}
}

void PintSQ::DrawBoxOverlapObjectsData(udword nb, const PintBoxOverlapData* data, const PintMultipleHits* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
		const Point& BoxStartPos = data[i].mBox.mCenter;
		const PR Pose(BoxStartPos, Quat(data[i].mBox.mRot));
/*		glColor4f(color.x, color.y, color.z, 1.0f);

		if(hits[i])
		{
			::DrawBox(data[i].mBox.mExtents, Pose);
		}
		else
		{
			DrawWireframeBox(data[i].mBox, Point(0.0f, 0.0f, 0.0f), color);
		}*/
		DrawWireframeBox(data[i].mBox, Point(0.0f, 0.0f, 0.0f), hits[i].mNbHits ? mBatchedLines_R : mBatchedLines_G);
	}
}

void PintSQ::DrawCapsuleOverlapAnyData(udword nb, const PintCapsuleOverlapData* data, const PintBooleanHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

	for(udword i=0;i<nb;i++)
	{
			// Draw solid capsule at initial position
			const LSS& Capsule = data[i].mCapsule;
			const Point CapsuleStartPos = (Capsule.mP0 + Capsule.mP1)*0.5f;

			Point CapsuleAxis = Capsule.mP1 - Capsule.mP0;
			const float M = CapsuleAxis.Magnitude();
			CapsuleAxis /= M;
			const Quat q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), CapsuleAxis);

				const Quat qq = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
				Quat qqq = q * qq;

			const PR Pose(CapsuleStartPos, qqq);

		DrawWireframeCapsule(Capsule.mRadius, M, Pose, Point(0.0f, 0.0f, 0.0f), hits[i].mHit ? mBatchedLines_R : mBatchedLines_G);
	}
}

void PintSQ::DrawCapsuleOverlapObjectsData(udword nb, const PintCapsuleOverlapData* data, const PintMultipleHits* hits, const Point& color)
{
}

void PintSQ::DrawConvexOverlapAnyData(udword nb, const PintConvexOverlapData* data, const PintBooleanHit* hits, const Point& color)
{
	if(!nb || !data || !hits)
		return;

/*	for(udword i=0;i<nb;i++)
	{
			// Draw solid capsule at initial position
			const LSS& Capsule = data[i].mCapsule;
			const Point CapsuleStartPos = (Capsule.mP0 + Capsule.mP1)*0.5f;

			Point CapsuleAxis = Capsule.mP1 - Capsule.mP0;
			const float M = CapsuleAxis.Magnitude();
			CapsuleAxis /= M;
			const Quat q = ShortestRotation(Point(1.0f, 0.0f, 0.0f), CapsuleAxis);

				const Quat qq = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
				Quat qqq = q * qq;

			const PR Pose(CapsuleStartPos, qqq);

		DrawWireframeCapsule(Capsule.mRadius, M, Pose, Point(0.0f, 0.0f, 0.0f), hits[i].mHit ? mBatchedLines_R : mBatchedLines_G);
	}*/

	for(udword i=0;i<nb;i++)
	{
		// Draw solid convex at initial position
		const Point& ConvexStartPos = data[i].mTransform.mPos;

		glColor4f(color.x, color.y, color.z, 1.0f);
		//RenderShape(data[i].mRenderer, data[i].mTransform);

		//if(hits[i].mTouchedActor)
		if(hits[i].mHit)
		{
			// Draw wireframe convex at impact position
			//glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			RenderShape(data[i].mRenderer, data[i].mTransform);
//			DrawWireframeConvex(data[i].mRenderer, data[i].mTransform, Point(1.0f, 0.0f, 0.0f));
/*				BatchedConvexData* BCD = ICE_RESERVE(BatchedConvexData, mBatchedConvexes);

				BCD->mRenderer = data[i].mRenderer;
				BCD->mPose = data[i].mTransform;
				//BCD->mColor = color;*/
		}
		else
		{
			// Draw wireframe convex at end position
			//glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			//RenderShape(data[i].mRenderer, data[i].mTransform);
//			DrawWireframeConvex(data[i].mRenderer, data[i].mTransform, Point(0.0f, 1.0f, 0.0f));
				BatchedConvexData* BCD = ICE_RESERVE(BatchedConvexData, mBatchedConvexes);

				BCD->mRenderer = data[i].mRenderer;
				BCD->mPose = data[i].mTransform;
				//BCD->mColor = color;
		}
	}

}

void PintSQ::DrawConvexOverlapObjectsData(udword nb, const PintConvexOverlapData* data, const PintMultipleHits* hits, const Point& color)
{
}

void PintSQ::Render(PintRender& renderer, PintRenderPass render_pass, bool paused)
{
	if(!(mOwner->GetFlags() & PINT_IS_ACTIVE))
		return;

	if(render_pass!=PINT_RENDER_PASS_MAIN)
		return;

	const Point Color = mOwner->GetMainColor();

	// New design: we don't render lines immediately, we batch them for later rendering after the current shader has been deactivated.

	DrawRaycastData					(mRaycasts.mNbHits,					mRaycastData,			mRaycasts.mHits,				Color);
	DrawRaycastAnyData				(mRaycastsAny.mNbHits,				mRaycastData,			mRaycastsAny.mHits,				Color);
	DrawRaycastAllData				(mRaycastsAll.mNbHits,				mRaycastData,			mRaycastsAll.mHits,				Color);

	DrawBoxSweepData				(mBoxSweeps.mNbHits,				mBoxSweepData,			mBoxSweeps.mHits,				Color);
	DrawSphereSweepData				(mSphereSweeps.mNbHits,				mSphereSweepData,		mSphereSweeps.mHits,			Color);
	DrawCapsuleSweepData			(mCapsuleSweeps.mNbHits,			mCapsuleSweepData,		mCapsuleSweeps.mHits,			Color);
	DrawConvexSweepData				(mConvexSweeps.mNbHits,				mConvexSweepData,		mConvexSweeps.mHits,			Color);

	DrawSphereOverlapAnyData		(mSphereOverlapAny.mNbHits,			mSphereOverlapData,		mSphereOverlapAny.mHits,		Color);
	DrawSphereOverlapObjectsData	(mSphereOverlapObjects.mNbHits,		mSphereOverlapData,		mSphereOverlapObjects.mHits,	Color);

	DrawBoxOverlapAnyData			(mBoxOverlapAny.mNbHits,			mBoxOverlapData,		mBoxOverlapAny.mHits,			Color);
	DrawBoxOverlapObjectsData		(mBoxOverlapObjects.mNbHits,		mBoxOverlapData,		mBoxOverlapObjects.mHits,		Color);

	DrawCapsuleOverlapAnyData		(mCapsuleOverlapAny.mNbHits,		mCapsuleOverlapData,	mCapsuleOverlapAny.mHits,		Color);
	DrawCapsuleOverlapObjectsData	(mCapsuleOverlapObjects.mNbHits,	mCapsuleOverlapData,	mCapsuleOverlapObjects.mHits,	Color);

	DrawConvexOverlapAnyData		(mConvexOverlapAny.mNbHits,			mConvexOverlapData,		mConvexOverlapAny.mHits,		Color);
	DrawConvexOverlapObjectsData	(mConvexOverlapObjects.mNbHits,		mConvexOverlapData,		mConvexOverlapObjects.mHits,	Color);

	//###TOFIX
//	if(!paused)
//		ResetAllDataPointers();
}

///////////////////////////////////////////////////////////////////////////////

#define PEEL_RED	252.0f/255.0f, 169.0f/255.0f, 133.0f/255.0f
#define PEEL_GREEN	133.0f/255.0f, 202.0f/255.0f, 93.0f/255.0f
#define PEEL_BLUE	111.0f/255.0f, 183.0f/255.0f, 214.0f/255.0f

void PintSQ::RenderBatched(PintRender& renderer, PintRenderPass render_pass, bool paused)
{
	if(render_pass!=PINT_RENDER_PASS_MAIN)
		return;

	for(udword i=0;i<4;i++)
	{
		Vertices* Segments;
		if(i==0)
		{
			Segments = &mBatchedLines_Main;
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
		else if(i==1)
		{
			Segments = &mBatchedLines_R;
//			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
//			glColor4f(253.0f/255.0f, 202.0f/255.0f, 162.0f/255.0f, 1.0f);
			glColor4f(PEEL_RED, 1.0f);
		}
		else if(i==2)
		{
			Segments = &mBatchedLines_G;
//			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
//			glColor4f(181.0f/255.0f, 225.0f/255.0f, 174.0f/255.0f, 1.0f);
			glColor4f(PEEL_GREEN, 1.0f);
		}
		else if(i==3)
		{
			Segments = &mBatchedLines_B;
//			glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
//			glColor4f(154.0f/255.0f, 206.0f/255.0f, 223.0f/255.0f, 1.0f);
			glColor4f(PEEL_BLUE, 1.0f);
		}

		const udword NbVerts = Segments->GetNbVertices();
		if(NbVerts)
		{
			const Point* V = Segments->GetVertices();

			glDisable(GL_LIGHTING);
	//		glColor4f(color.x, color.y, color.z, 1.0f);
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, sizeof(Point), &V->x);
			glDrawArrays(GL_LINES, 0, NbVerts);
			glDisableClientState(GL_VERTEX_ARRAY);
			glEnable(GL_LIGHTING);
		}

		Segments->Reset();
	}

	if(mBatchedConvexes.GetNbEntries())
	{
		const bool Saved = gWireframePass;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_LIGHTING);
		gWireframePass = true;
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		const BatchedConvexData* BCD = (const BatchedConvexData*)mBatchedConvexes.GetEntries();
		udword Nb = mBatchedConvexes.GetNbEntries()/(sizeof(BatchedConvexData)/sizeof(udword));
		while(Nb--)
		{
			//glColor4f(BCD->mColor.x, BCD->mColor.y, BCD->mColor.z, 1.0f);
			RenderShape(BCD->mRenderer, BCD->mPose);
			BCD++;
		}

		gWireframePass = Saved;
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		SetUserDefinedPolygonMode();
		glEnable(GL_LIGHTING);

		mBatchedConvexes.Reset();
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
