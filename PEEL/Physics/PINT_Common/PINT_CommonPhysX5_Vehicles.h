///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX5_VEHICLES_H
#define PINT_COMMON_PHYSX5_VEHICLES_H

#include "extensions/PxSceneQueryExt.h"

	class PintRender;
	class MyBatchQueryExt : public PxBatchQueryExt
	{
		public:
		virtual	void	enableSQRecorder(bool b)								= 0;
		virtual	void	resetSQRecorder()										= 0;
		virtual	void	drawSQRaycasts(PintRender& renderer)					= 0;
		virtual	void	drawSQSweeps(PintRender& renderer, bool drawGeometry)	= 0;
	};

	MyBatchQueryExt* createMyBatchQueryExt(
	const PxScene& scene, PxQueryFilterCallback* queryFilterCallback,
	const PxU32 maxNbRaycasts, const PxU32 maxNbRaycastTouches,
	const PxU32 maxNbSweeps, const PxU32 maxNbSweepTouches,
	const PxU32 maxNbOverlaps, const PxU32 maxNbOverlapTouches);

#endif
