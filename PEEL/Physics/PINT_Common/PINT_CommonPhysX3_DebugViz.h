///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_DEBUG_VIZ_H
#define PINT_COMMON_PHYSX3_DEBUG_VIZ_H

	class PintRender;
	void DrawWireframeConvexMesh(PintRender& renderer, const PxConvexMeshGeometry& geometry, const PxTransform& absPose);

#if PHYSX_SUPPORT_VEHICLE5
	typedef PxRaycastBuffer	RaycastResults;
	typedef PxSweepBuffer	SweepResults;
#else
	typedef PxRaycastQueryResult	RaycastResults;
	typedef PxSweepQueryResult		SweepResults;
#endif

	class SQRecorder
	{
		public:
							SQRecorder();
							~SQRecorder();

		inline_	void		SetEnabled(bool b)	{ mEnabled = b;	}
				void		Release();
				void		Reset();

				void		RecordRaycast(const PxVec3& origin, const PxVec3& unitDir, float distance, udword index);
				void		RecordSweep(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, float distance, udword index);

				void		DrawRaycasts(PintRender& renderer, const RaycastResults* results)					const;
				void		DrawSweeps(PintRender& renderer, const SweepResults* results, bool drawGeometry)	const;

		protected:
				Container	mSweeps;
				Container	mRaycasts;
				bool		mEnabled;
	};

#endif

