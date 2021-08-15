///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_CommonPhysX3_DebugViz.h"
#include "PINT_CommonPhysX3_Base.h"
#include "../Pint.h"

void DrawWireframeConvexMesh(PintRender& renderer, const PxConvexMeshGeometry& geometry, const PxTransform& absPose)
{
	const PxConvexMesh* convexMesh = geometry.convexMesh;

	const PxVec3* vertices = convexMesh->getVertices();
	const PxU8* indexBuffer = convexMesh->getIndexBuffer();
	const PxU32 nbPolygons = convexMesh->getNbPolygons();

	const PxMat44 m44(PxMat33(absPose.q) * geometry.scale.toMat33(), absPose.p);

	const Point color(1.0f, 0.0f, 1.0f);

	for(PxU32 i=0; i<nbPolygons; i++)
	{
		PxHullPolygon polygon;
		convexMesh->getPolygonData(i, polygon);

		const PxU32 pnbVertices = polygon.mNbVerts;

		PxVec3 begin = m44.transform(vertices[indexBuffer[0]]);	// PT: transform it only once before the loop starts
		for(PxU32 j=1; j<pnbVertices; j++)
		{
			const PxVec3 end = m44.transform(vertices[indexBuffer[j]]);
			renderer.DrawLine(ToPoint(begin), ToPoint(end), color);
			begin = end;
		}
		const PxVec3 first = m44.transform(vertices[indexBuffer[0]]);
		renderer.DrawLine(ToPoint(begin), ToPoint(first), color);

		indexBuffer += pnbVertices;
	}
}

namespace
{
	struct DebugRaycast
	{
		Point	mOrigin;
		Point	mDir;
		float	mMaxDist;
		udword	mIndex;
	};
	CHECK_CONTAINER_ITEM(DebugRaycast);

	struct DebugSweep
	{
		//const PxGeometry*	mGeometry;
		PxGeometryHolder	mHolder;
		PxTransform			mPose;
		Point				mOrigin;
		Point				mDir;
		float				mMaxDist;
		udword				mIndex;
	};
	CHECK_CONTAINER_ITEM(DebugSweep);
}

SQRecorder::SQRecorder() : mEnabled(false)
{
}

SQRecorder::~SQRecorder()
{
}

void SQRecorder::Release()
{
	mRaycasts.Empty();
	mSweeps.Empty();
}

void SQRecorder::Reset()
{
	mRaycasts.Reset();
	mSweeps.Reset();
}

void SQRecorder::RecordRaycast(const PxVec3& origin, const PxVec3& unitDir, float distance, udword index)
{
	if(mEnabled)
	{
		DebugRaycast* Record = (DebugRaycast*)mRaycasts.Reserve(sizeof(DebugRaycast)/sizeof(udword));
		Record->mOrigin		= ToPoint(origin);
		Record->mDir		= ToPoint(unitDir);
		Record->mMaxDist	= distance;
		Record->mIndex		= index;
	}
}

void SQRecorder::RecordSweep(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, float distance, udword index)
{
	if(mEnabled)
	{
		DebugSweep* Record = (DebugSweep*)mSweeps.Reserve(sizeof(DebugSweep)/sizeof(udword));
		//Record->mGeometry	= &geometry;
		Record->mHolder.storeAny(geometry);
		Record->mPose		= pose;
		Record->mOrigin		= ToPoint(pose.p);
		Record->mDir		= ToPoint(unitDir);
		Record->mMaxDist	= distance;
		Record->mIndex		= index;
	}
}

void SQRecorder::DrawRaycasts(PintRender& renderer, const RaycastResults* results)	const
{
	if(!mEnabled)
		return;

	const DebugRaycast* Records = (const DebugRaycast*)mRaycasts.GetEntries();
	udword NbRaycasts = mRaycasts.GetNbEntries()/(sizeof(DebugRaycast)/sizeof(udword));
	while(NbRaycasts--)
	{
		const DebugRaycast& Record = *Records++;
		const RaycastResults& Current = results[Record.mIndex];

		const float Scale = 0.1f;
		renderer.DrawLine(Record.mOrigin, Record.mOrigin + Point(Scale, 0.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		renderer.DrawLine(Record.mOrigin, Record.mOrigin + Point(0.0f, Scale, 0.0f), Point(0.0f, 1.0f, 0.0f));
		renderer.DrawLine(Record.mOrigin, Record.mOrigin + Point(0.0f, 0.0f, Scale), Point(0.0f, 0.0f, 1.0f));

		if(Current.hasBlock)
		{
			const PxRaycastHit& hit = Current.block;
			renderer.DrawLine(Record.mOrigin, ToPoint(hit.position), Point(1.0f, 0.0f, 0.0f));
			renderer.DrawLine(ToPoint(hit.position), ToPoint(hit.position+hit.normal), Point(0.0f, 1.0f, 1.0f));
		}
		else
		{
			renderer.DrawLine(Record.mOrigin, Record.mOrigin + Record.mDir*Record.mMaxDist, Point(0.0f, 1.0f, 0.0f));
		}
	}
}

void SQRecorder::DrawSweeps(PintRender& renderer, const SweepResults* results, bool drawGeometry)	const
{
	if(!mEnabled)
		return;

	const DebugSweep* Records = (const DebugSweep*)mSweeps.GetEntries();
	udword NbSweeps = mSweeps.GetNbEntries()/(sizeof(DebugSweep)/sizeof(udword));
	if(0)
		printf("\n");
	while(NbSweeps--)
	{
		const DebugSweep& Record = *Records++;
		const SweepResults& Current = results[Record.mIndex];

		if(0)
		{
			printf("MaxDist: %f\n", Record.mMaxDist);
			if(Current.hasBlock)
				printf("HitDist: %f\n", Current.block.distance);
			else
				printf("(no hit)\n");
		}

		const float Scale = 0.1f;
		renderer.DrawLine(Record.mOrigin, Record.mOrigin + Point(Scale, 0.0f, 0.0f), Point(1.0f, 0.0f, 0.0f));
		renderer.DrawLine(Record.mOrigin, Record.mOrigin + Point(0.0f, Scale, 0.0f), Point(0.0f, 1.0f, 0.0f));
		renderer.DrawLine(Record.mOrigin, Record.mOrigin + Point(0.0f, 0.0f, Scale), Point(0.0f, 0.0f, 1.0f));

		if(Current.hasBlock)
		{
			const PxSweepHit& hit = Current.block;
			renderer.DrawLine(Record.mOrigin, ToPoint(hit.position), Point(1.0f, 0.0f, 0.0f));
			renderer.DrawLine(ToPoint(hit.position), ToPoint(hit.position+hit.normal), Point(0.0f, 1.0f, 1.0f));
		}
		else
		{
			//renderer.DrawLine(Record.mOrigin, Record.mOrigin + Record.mDir*Record.mMaxDist, Point(0.0f, 1.0f, 0.0f));
			if(Current.nbTouches)
			{
				for(PxU32 i=0;i<Current.nbTouches;i++)
				{
					const PxSweepHit& hit = Current.touches[i];
					renderer.DrawLine(Record.mOrigin, ToPoint(hit.position), Point(1.0f, 0.0f, 0.0f));
					renderer.DrawLine(ToPoint(hit.position), ToPoint(hit.position+hit.normal), Point(0.0f, 1.0f, 1.0f));
				}
			}
			else
			{
				renderer.DrawLine(Record.mOrigin, Record.mOrigin + Record.mDir*Record.mMaxDist, Point(0.0f, 1.0f, 0.0f));
			}
		}

		if(drawGeometry)
		{
			PxTransform targetPose = Record.mPose;

			if(Current.hasBlock)
				targetPose.p += ToPxVec3(Record.mDir) * Current.block.distance;
			else
				targetPose.p += ToPxVec3(Record.mDir) * Record.mMaxDist;

			const PxGeometry& geom = Record.mHolder.any();
			if(geom.getType()==PxGeometryType::eCONVEXMESH)
			{
				const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom);

				DrawWireframeConvexMesh(renderer, convexGeom, targetPose);
				//visualizeConvexMesh(convexGeom, renderer, Record.mPose);
				//printf("%f %f %f\n", Record.mPose.p.x, Record.mPose.p.y, Record.mPose.p.z);
			}
			else if(geom.getType()==PxGeometryType::eSPHERE)
			{
				const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);

				const Sphere S(ToPoint(targetPose.p), sphereGeom.radius);
				renderer.DrawWireframeSphere(S, Point(1.0f, 0.0f, 1.0f));

				//visualizeConvexMesh(convexGeom, renderer, targetPose);
			}
		}
	}
}
