///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_BATCH_CONVEX_SHAPE_RENDERER_H
#define PINT_BATCH_CONVEX_SHAPE_RENDERER_H

#include "PintDLShapeRenderer.h"

#define USE_DRAW_ELEMENTS
#define USE_BATCH_RENDER
#ifdef USE_BATCH_RENDER
	#define USE_CPU_TRANSFORM
#endif

	class PintBatchConvexShapeRenderer : public PintDLShapeRenderer
	{
		public:
									PintBatchConvexShapeRenderer(udword nb_verts, const Point* verts);
		virtual						~PintBatchConvexShapeRenderer();

		// PintShapeRenderer
		virtual	const char*			GetClassName()			const	override	{ return "PintBatchConvexShapeRenderer";	}
		virtual	void				_Render(const PR& pose)	const	override;
		//~PintShapeRenderer

		private:
#ifdef USE_DRAW_ELEMENTS
		//		IndexedTriangle16*	mIndexedTris;
				IndexedTriangle*	mIndexedTris;
				Point*				mSrcVerts;
				Point*				mSrcNormals;
	#ifdef USE_CPU_TRANSFORM
		#ifndef USE_BATCH_RENDER
				Point*				mDstVerts;
				Point*				mDstNormals;
		#endif
	#endif
				udword				mTotalNbVerts;
#else
				Triangle*			mTris;
				Triangle*			mNormals;
#endif
				udword				mTotalNbTris;
	};

	class PintConvexBatchRendererCPUTransformNoNormals : public PintDLShapeRenderer
	{
		public:
									PintConvexBatchRendererCPUTransformNoNormals(udword nb_verts, const Point* verts);
		virtual						~PintConvexBatchRendererCPUTransformNoNormals();

		// PintShapeRenderer
		virtual	const char*			GetClassName()			const	override	{ return "PintConvexBatchRendererCPUTransformNoNormals";	}
		virtual	void				_Render(const PR& pose)	const	override;
		//~PintShapeRenderer

		private:
				IndexedTriangle*	mIndexedTris;
				Point*				mSrcVerts;
				udword				mNbVerts;
				udword				mNbTris;
	};



	void	StartBatchConvexRender();
	void	EndBatchConvexRender();
	void	ReleaseBatchConvexRender();

#endif