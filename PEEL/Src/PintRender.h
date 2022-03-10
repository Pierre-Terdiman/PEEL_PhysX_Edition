///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_RENDER_H
#define PINT_RENDER_H

#include "Pint.h"

	class SelectionManager;
	class DefaultRenderer : public PintRender
	{
		public:
									DefaultRenderer();
		virtual						~DefaultRenderer();

		// PintRender
		virtual	void				Print				(float x, float y, float fontSize, const char* string, const Point& color)	override;

		virtual	void				DrawLine			(const Point& p0, const Point& p1, const Point& color)						override;
		virtual	void				DrawLines			(udword nb_lines, const Point* pts, const Point& color)						override;
		virtual	void				DrawLines2D			(udword nb_verts, const float* vertices, const Point& color)				override;
		virtual	void				DrawLine2D			(float x0, float x1, float y0, float y1, const Point& color)				override;
		virtual	void				DrawRectangle2D		(float x0, float x1, float y0, float y1, const Point& color, float alpha)	override;
		virtual	void				DrawTriangle		(const Point& p0, const Point& p1, const Point& p2, const Point& color)		override;
		virtual	void				DrawWireframeAABB	(udword nb_boxes, const AABB* boxes, const Point& color)					override;
		virtual	void				DrawWireframeOBB	(const OBB& box, const Point& color)										override;
		virtual	void				DrawWireframeSphere	(const Sphere& sphere, const Point& color)									override;

		virtual	void				DrawSphere			(float radius, const PR& pose)												override;
		virtual	void				DrawBox				(const Point& extents, const PR& pose)										override;
		virtual	void				DrawCapsule			(float radius, float height, const PR& pose)								override;
		virtual	void				DrawSurface			(const SurfaceInterface& surface, const PR& pose)							override;
		virtual	void				DrawPoints			(udword nb, const Point* pts, udword stride)								override;

		virtual	const Plane*		GetFrustumPlanes()																				override;

		virtual	bool				SetCurrentActor		(PintActorHandle h)															override;
		virtual	bool				SetCurrentShape		(PintShapeHandle h)															override;
		virtual	void				DrawShape			(PintShapeRenderer* shape, const PR& pose)									override;
		//~PintRender

				void				StartRender			(Pint* pint, SelectionManager* sel_manager, PintRenderPass render_pass);
				void				EndRender			();
		inline_ void				ProcessShape(PintShapeRenderer* shape, const PR& pose);

//				void				DrawCylinder		(float radius, float height, const PR& pose);

				void				DrawSQ(PintRenderPass render_pass);

				void				Release();

				Pint*				mPint;
				VisibilityManager*	mVisManager;
				SelectionManager*	mSelManager;
				PintActorHandle		mFocusActor;
				PintShapeHandle		mFocusShape;
				PintActorHandle		mFocusShapeActor;
				Container			mSelectedShapes;
				Container			mFocusShapes;
				Container			mBatchedShapes;
				Container			mTransparentShapes;
				Container			mSortKeys;
				RadixSort			mRS;
				udword				mFrameNumber;
				PintRenderPass		mPass;
				Vertices			mSegments;
				udword				mHighlightFlags;
				bool				mRenderEnabled;
	};

#endif