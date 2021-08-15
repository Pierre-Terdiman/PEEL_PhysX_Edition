///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_CONVEX_INSTANCE_SHAPE_RENDERER_H
#define PINT_CONVEX_INSTANCE_SHAPE_RENDERER_H

#include "PintDLShapeRenderer.h"

#define QUAT2MATRIXSHADER

#ifdef QUAT2MATRIXSHADER
	struct Transform : public Allocateable
	{
		Quat	q;
		Point	p;
		udword	pad;
	};
	ICE_COMPILE_TIME_ASSERT(sizeof(Transform)==sizeof(float)*4*2);
#endif

	class PintConvexInstanceRenderer : public PintDLShapeRenderer
	{
									PREVENT_COPY(PintConvexInstanceRenderer)
		public:
									PintConvexInstanceRenderer(udword nb_verts, const Point* verts);
		virtual						~PintConvexInstanceRenderer();

		// PintShapeRenderer
		virtual	const char*			GetClassName()			const	override	{ return "PintConvexInstanceRenderer";	}
		virtual	void				_Render(const PR& pose)	const	override;
		//~PintShapeRenderer

				void				DrawBatch()	const;
		private:
#ifdef QUAT2MATRIXSHADER
				Transform*			mPoses;
#else
				Matrix4x4*			mMatrices;
#endif
				IndexedTriangle*	mIndexedTris;
				Point*				mSrcVerts;
				udword				mNbVerts;
				udword				mNbTris;

		mutable	udword				mCurrentNbInstances;
		const	udword				mMaxNbInstances;

				GLuint				mVBO;
				GLuint				mVAO;
				GLuint				mEBO;
				GLuint				mIBO;
	};

	void	StartInstanceRender();
	void	EndInstanceRender();
	void	ReleaseAllConvexInstanceRenderers();

#endif