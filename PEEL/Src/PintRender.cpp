///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintRender.h"
//#include "PintShapeRenderer.h"
#include "GLRenderHelpers.h"
#include "GLRenderStates.h"
#include "GLFontRenderer.h"
#include "Shader.h"
#include "PintRenderState.h"

//### not great
#include "PEEL.h"
#include "DefaultControlInterface.h"

extern udword gScreenWidth;
extern udword gScreenHeight;
extern GLFontRenderer gTexter;

static inline_ void DrawImpact(const PintRaycastHit& hit)
{
	GLRenderHelpers::DrawFrame(hit.mImpact, 1.0f);
	GLRenderHelpers::DrawLine(hit.mImpact, hit.mImpact + hit.mNormal, Point(1.0f, 1.0f, 1.0f));
}

static void DrawWireframeBox(const OBB& box, const Point& offset, const Point& color)
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
		GLRenderHelpers::DrawLine(Pts[VRef0], Pts[VRef1], color);
	}
}

/*static void DrawWireframeSphere(const Sphere& sphere, const Point& offset, const Point& color)
{
	GLRenderHelpers::DrawSphereWireframe(sphere.mRadius, PR(sphere.mCenter + offset, Quat(Idt)), color);
}

static void DrawWireframeCapsule(float r, float h, const PR& pose, const Point& offset, const Point& color)
{
	GLRenderHelpers::DrawCapsuleWireframe(r, h, PR(pose.mPos + offset, pose.mRot), color);
}*/

void DefaultRenderer::Print(float x, float y, float fontSize, const char* string, const Point& color)
{
	const float TextScale = fontSize;// * float(INITIAL_SCREEN_HEIGHT) / float(gScreenHeight);

	gTexter.setColor(color.x, color.y, color.z, 1.0f);
	gTexter.print(x, y, TextScale, string);
}

void DefaultRenderer::DrawLine(const Point& p0, const Point& p1, const Point& color)
{
	GLRenderHelpers::DrawLine(p0, p1, color);
}

void DefaultRenderer::DrawLines(udword nb_lines, const Point* pts, const Point& color)
{
	GLRenderHelpers::DrawLines(nb_lines, pts, color);
}

void DefaultRenderer::DrawLines2D(udword nb_verts, const float* vertices, const Point& color)
{
	GLRenderHelpers::DrawLines2D(vertices, nb_verts, color);
}

void DefaultRenderer::DrawLine2D(float x0, float x1, float y0, float y1, const Point& color)
{
	GLRenderHelpers::DrawLine2D(x0, x1, y0, y1, color);
}

void DefaultRenderer::DrawRectangle2D(float x0, float x1, float y0, float y1, const Point& color, float alpha)
{
	GLRenderHelpers::DrawRectangle(x0, x1, y0, y1, color, color, alpha, gScreenWidth, gScreenHeight, false, 0);
}

void DefaultRenderer::DrawTriangle(const Point& p0, const Point& p1, const Point& p2, const Point& color)
{
	GLRenderHelpers::DrawTriangle(p0, p1, p2, color);
}

void DefaultRenderer::DrawWireframeAABB(udword nb_boxes, const AABB* boxes, const Point& color)
{
	const udword BatchSize = 32;
	Point Tmp[24*BatchSize];

	udword NbRemaining = nb_boxes;

	while(NbRemaining)
	{
		udword NbToGo = NbRemaining;
		if(NbToGo>BatchSize)
			NbToGo = BatchSize;

		udword Index = 0;
		Point* Dst = Tmp;
		for(udword j=0;j<NbToGo;j++)
		{
			const AABB& box = *boxes++;

			Point Pts[8];
			box.ComputePoints(Pts);

			const udword* Edges = box.GetEdges();

			for(udword i=0;i<12;i++)
			{
				*Dst++ = Pts[*Edges++];
				*Dst++ = Pts[*Edges++];
			}
		}
		GLRenderHelpers::DrawLines(12*NbToGo, Tmp, color);

		NbRemaining -= NbToGo;
	}

/*	for(udword j=0;j<nb_boxes;j++)
	{
		const AABB& box = boxes[j];

		Point Pts[8];
		box.ComputePoints(Pts);

		const udword* Edges = box.GetEdges();
		//for(udword i=0;i<12;i++)
		//	GLRenderHelpers::DrawLine(Pts[Edges[i*2]], Pts[Edges[i*2+1]], color);

		for(udword i=0;i<12;i++)
		{
			Tmp[i*2+0] = Pts[Edges[i*2]];
			Tmp[i*2+1] = Pts[Edges[i*2+1]];
		}
		GLRenderHelpers::DrawLines(12, Tmp, color);
	}*/
}

void DefaultRenderer::DrawWireframeOBB(const OBB& box, const Point& color)
{
	DrawWireframeBox(box, Point(0.0f, 0.0f, 0.0f), color);
}

void DefaultRenderer::DrawWireframeSphere(const Sphere& sphere, const Point& color)
{
	GLRenderHelpers::DrawSphereWireframe(sphere.mRadius, PR(sphere.mCenter, Quat(Idt)), color);
}

void DefaultRenderer::DrawSphere(float radius, const PR& pose)
{
	GLRenderHelpers::DrawSphere(radius, pose);
}

void DefaultRenderer::DrawBox(const Point& extents, const PR& pose)
{
	GLRenderHelpers::DrawBox(extents, pose);
}

void DefaultRenderer::DrawCapsule(float r, float h, const PR& pose)
{
	GLRenderHelpers::DrawCapsule(r, h, pose);
}

void DefaultRenderer::DrawSurface(const SurfaceInterface& surface, const PR& pose)
{
	glPushMatrix();
		GLRenderHelpers::SetupGLMatrix(pose);

		glEnableClientState(GL_VERTEX_ARRAY);
//		const Point* Normals = gBatchNormals->GetVertices();
//		if(Normals)
//			glEnableClientState(GL_NORMAL_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(Point), surface.mVerts);
//		if(Normals)
//			glNormalPointer(GL_FLOAT, sizeof(Point), Normals);

		if(surface.mDFaces)
			glDrawElements(GL_TRIANGLES, 3*surface.mNbFaces, GL_UNSIGNED_INT, surface.mDFaces);
		else if(surface.mWFaces)
			glDrawElements(GL_TRIANGLES, 3*surface.mNbFaces, GL_UNSIGNED_SHORT, surface.mWFaces);

//		if(Normals)
//			glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
}

/*void DefaultRenderer::DrawCylinder(float r, float h, const PR& pose)
{
	GLRenderHelpers::DrawCylinder(r, h, pose);
}*/

#include "GLPointRenderer.h"
void DefaultRenderer::DrawPoints(udword nb, const Point* pts, udword stride)
{
	const Point color(1.0f, 1.0f, 1.0f);
	GLPointRenderer::Draw(color, nb, pts, stride);
}

#include "Camera.h"
extern bool gCameraVFC;
const Plane* DefaultRenderer::GetFrustumPlanes()
{
	return gCameraVFC ? ::GetFrustumPlanes() : null;
}

///////////////////////////////////////////////////////////////////////////////

#include "PintVisibilityManager.h"
#include "PintSelectionManager.h"

#include "GLPointRenderer2.h"
#include "PintColorShapeRenderer.h"
#include "PintBatchConvexShapeRenderer.h"
#include "PintConvexInstanceShapeRenderer.h"

//extern	bool	gShadowPass;
extern	bool	gExternalColorControl;

enum HighlightFlags
{
	HIGHLIGHT_SELECTION		= (1<<0),
	HIGHLIGHT_FOCUS_ACTOR	= (1<<1),
	HIGHLIGHT_FOCUS_SHAPE	= (1<<2),
	HIGHLIGHT_FOCUS_TEST	= (1<<3),
	HIGHLIGHT_FOCUS			= HIGHLIGHT_FOCUS_ACTOR | HIGHLIGHT_FOCUS_SHAPE
};

DefaultRenderer::DefaultRenderer() :
	mPint			(null),
	mVisManager		(null),
	mSelManager		(null),
	mFocusActor		(null),
	mFocusShape		(null),
	mFocusShapeActor(null),
	mFrameNumber	(0),
	mPass			(PINT_RENDER_PASS_MAIN),
	mHighlightFlags	(0),
	mRenderEnabled	(true)
{
}

DefaultRenderer::~DefaultRenderer()
{
	Release();
}

void DefaultRenderer::Release()
{
	mSegments.Empty();
	mSelectedShapes.Empty();
	mFocusShapes.Empty();
	mBatchedShapes.Empty();
	mTransparentShapes.Empty();
	mSortKeys.Empty();
	mRS.~RadixSort();
	mFocusActor = null;
	mFocusShape = null;
	mFocusShapeActor = null;
}

static const bool gBatching = false;
//static PintShapeRenderer* gCurrentRenderer = null;
#define XP_XP_XP
#ifdef XP_XP_XP
	static const RGBAColor* gColor = null;
	static const ManagedTexture* gTexture = null;
	static udword gNeedsVertexNormals = INVALID_ID;
#endif

namespace
{
	struct BatchedData
	{
		const PintShapeRenderer*	mRenderer;
		PR							mPose;
	};
	CHECK_CONTAINER_ITEM(BatchedData)

	static const udword gSize = sizeof(BatchedData)/sizeof(udword);
}

void DefaultRenderer::StartRender(Pint* pint, SelectionManager* sel_manager, PintRenderPass render_pass)
{
	SPY_ZONE("DefaultRenderer::StartRender")

	StartBatchConvexRender();
	StartInstanceRender();

	mPint = pint;
//	gCurrentRenderer = null;
#ifdef XP_XP_XP
	gColor = null;
	gTexture = null;
	gNeedsVertexNormals = INVALID_ID;
#endif

	const bool UseSelMan = (render_pass==PINT_RENDER_PASS_MAIN)||(render_pass==PINT_RENDER_PASS_REFLECTIONS);
	if(UseSelMan && sel_manager && sel_manager->HasSelection())
		mSelManager = sel_manager;
	else
		mSelManager = null;

	mVisManager = pint->mVisHelper;
	mRenderEnabled = true;
	mHighlightFlags = 0;
	mSelectedShapes.Reset();
	mFocusShapes.Reset();
	mBatchedShapes.Reset();
	mTransparentShapes.Reset();
	mSortKeys.Reset();
//	mSegments.Reset();

	mPass = render_pass;

	GLRenderStates::SetDefaultRenderStates(render_pass==PINT_RENDER_PASS_REFLECTIONS);
}

static void DrawSelectedShapes(const Container& shapes, const Point& color)
{
	if(shapes.GetNbEntries())
	{
		StartBatchConvexRender();
		StartInstanceRender();

		// Pretend it's a shadow pass so that render states aren't setup
//		const bool Saved = gShadowPass;
//		gShadowPass = true;

		//const Point SavedColor = GetMainColor();
		SetMainColor(color);
		gExternalColorControl = true;

		udword Nb = shapes.GetNbEntries()/gSize;
		const BatchedData* BD = (const BatchedData*)shapes.GetEntries();
		while(Nb--)
		{
			//if(BD->mRenderer!=mCurrentShape)
			{
			// Go to source to bypass textures/colors
//			PintShapeRenderer* Renderer = BD->mRenderer->GetSource();
//			Renderer->Render(BD->mPose);
				SetupMaterial(BD->mRenderer);
				//SetMainColor(color);
				RenderShape(BD->mRenderer, BD->mPose);
				//SetupMaterialAndRenderShape(BD->mRenderer, BD->mPose);
			}
			BD++;
		}

//		gShadowPass = Saved;

		EndBatchConvexRender();
		EndInstanceRender();

		//SetMainColor(SavedColor);
		ResetMainColor();
		gExternalColorControl = false;
	}
}

void DefaultRenderer::EndRender()
{
	SPY_ZONE("DefaultRenderer::EndRender")

	if(gBatching)
	{
		udword NbKeys = mSortKeys.GetNbEntries();
		const udword* Sorted = mRS.Sort(mSortKeys.GetEntries(), NbKeys, RADIX_UNSIGNED).GetRanks();

		ASSERT(mBatchedShapes.GetNbEntries()/gSize==NbKeys);
		const BatchedData* BD = (const BatchedData*)mBatchedShapes.GetEntries();
		while(NbKeys--)
		{
			const udword i = *Sorted++;
			SetupMaterial(BD[i].mRenderer);
			RenderShape(BD[i].mRenderer, BD[i].mPose);
			//SetupMaterialAndRenderShape(BD[i].mRenderer, BD[i].mPose);
		}
	}

	EndBatchConvexRender();

	GLPointRenderer2::DrawBatchedPoints();

	EndInstanceRender();

	// Draw selected shapes
	
	// ### not great
	if(1)
	{
		const float ColorCoeff = GetDefaultControlInterface().IsSelectionLocked() ? 0.5f : 1.0f;
		DrawSelectedShapes(mSelectedShapes, Point(ColorCoeff, 0.0f, 0.0f));
		DrawSelectedShapes(mFocusShapes, Point(1.0f, 1.0f, 0.0f));
		//const Point GulfOrange(252.0f/255.0f, 81.0f/255.0f, 11.0f/255.0f);
		//const Point GulfBlue(106.0f/255.0f, 197.0f/255.0f, 244.0f/255.0f);
		//DrawSelectedShapes(mSelectedShapes, GulfOrange*ColorCoeff);
		//DrawSelectedShapes(mFocusShapes, GulfBlue*ColorCoeff);
	}
	// ### I suppose this doesn't work with instanced transparent objects

	{
		// Render transparent pass immediately for reflections, but delay it (after wire pass) for main
//		if(mPass!=PINT_RENDER_PASS_MAIN && mPass!=PINT_RENDER_PASS_WIREFRAME_OVERLAY)
			RenderTransparent();

		//void RenderTransparent()
		if(mTransparentShapes.GetNbEntries())
		{
			SPY_ZONE("RenderTransparent")
			glDepthMask(FALSE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//  TODO: not sure this is enough, different batched renderers might be transparent with different colors etc so we need to replicate what we do in the opaque pipeline
StartBatchConvexRender();
StartInstanceRender();

			// TODO: sort meshes & faces
			udword Nb = mTransparentShapes.GetNbEntries()/gSize;
			const BatchedData* BD = (const BatchedData*)mTransparentShapes.GetEntries();
			while(Nb--)
			{
				SetupMaterial(BD->mRenderer);
				RenderShape(BD->mRenderer, BD->mPose);
				//SetupMaterialAndRenderShape(BD->mRenderer, BD->mPose);
				BD++;
			}

EndBatchConvexRender();
EndInstanceRender();

			glDisable(GL_BLEND);
			glDepthMask(TRUE);

			mTransparentShapes.Reset();
		}
	}
}

bool DefaultRenderer::SetCurrentActor(PintActorHandle h)
{
	// Warning, handle is a native handle here!
	const bool RenderEnabled = mVisManager->IsRenderable(h);
	mRenderEnabled = RenderEnabled;

	udword Flags = 0;
	if(mPass==PINT_RENDER_PASS_MAIN || mPass==PINT_RENDER_PASS_REFLECTIONS)
	{
		if(RenderEnabled && mSelManager)
		{
			if(mSelManager->IsSelectedNative(mPint, h))
				Flags = HIGHLIGHT_SELECTION;
		}

		if(h==mFocusActor)
			Flags |= HIGHLIGHT_FOCUS_ACTOR;
		else
			Flags &= ~HIGHLIGHT_FOCUS_ACTOR;

		if(h==mFocusShapeActor)
			Flags |= HIGHLIGHT_FOCUS_TEST;
		else
			Flags &= ~(HIGHLIGHT_FOCUS_TEST|HIGHLIGHT_FOCUS_SHAPE);
	}
	mHighlightFlags = Flags;
	return RenderEnabled;
}

bool DefaultRenderer::SetCurrentShape(PintShapeHandle h)
{
	if(mHighlightFlags & HIGHLIGHT_FOCUS_TEST)
	{
		if(h==mFocusShape)
			mHighlightFlags |= HIGHLIGHT_FOCUS_SHAPE;
		else
			mHighlightFlags &= ~HIGHLIGHT_FOCUS_SHAPE;
	}
	return true;
}

inline_ void DefaultRenderer::ProcessShape(PintShapeRenderer* shape, const PR& pose)
{
	if((shape->mBaseFlags & SHAPE_RENDERER_TRANSPARENT) && mPass!=PINT_RENDER_PASS_SHADOW && mPass!=PINT_RENDER_PASS_WIREFRAME_OVERLAY)
	{
		BatchedData* BD = ICE_RESERVE(BatchedData, mTransparentShapes);

		BD->mRenderer = shape;
		BD->mPose = pose;
	}
	else
	{
#ifdef XP_XP_XP
		const RGBAColor* Color = shape->GetColor();
		const ManagedTexture* Texture = shape->GetTexture();
		const udword NeedsVertexNormals = shape->_UseExplicitVertexNormals();

		bool Switch = false;
		if(		Color!=gColor
			||	Texture!=gTexture
			||	NeedsVertexNormals!=gNeedsVertexNormals
			)
		{
			gColor = Color;
			gTexture = Texture;
			gNeedsVertexNormals = NeedsVertexNormals;
			EndBatchConvexRender();
			EndInstanceRender();
			StartBatchConvexRender();
			StartInstanceRender();
		}
#endif

		SetupMaterial(shape);

		// ### this isn't really about a "render switch", since the batched convex renderers are different for each convex for example!
		// But they internally refer to the same shared/global memory buffer.

		/*const bool RenderSwitch = shape->mBaseFlags & SHAPE_RENDERER_USES_BATCHING && shape!=gCurrentRenderer;
		if(RenderSwitch)
		{
			gCurrentRenderer = shape;
			EndBatchConvexRender();
			EndInstanceRender();
			StartBatchConvexRender();
			StartInstanceRender();
		}*/

//EndBatchConvexRender();
//EndInstanceRender();
		RenderShape(shape, pose);
//StartBatchConvexRender();
//StartInstanceRender();

		//SetupMaterialAndRenderShape(shape, pose);
	}
}

void DefaultRenderer::DrawShape(PintShapeRenderer* shape, const PR& pose)
{
	if(!mRenderEnabled)
		return;

	SPY_ZONE("DefaultRenderer::DrawShape")
	ASSERT(shape);

	if(mHighlightFlags)
	{
		Container& Storage = (mHighlightFlags & HIGHLIGHT_FOCUS) ? mFocusShapes : mSelectedShapes;

		BatchedData* BD = ICE_RESERVE(BatchedData, Storage);

		BD->mRenderer = shape;
		BD->mPose = pose;
		return;
	}

	if(gBatching)
	{
		BatchedData* BD = ICE_RESERVE(BatchedData, mBatchedShapes);

		BD->mRenderer = shape;
		BD->mPose = pose;

		mSortKeys.Add(shape->mSortKey);
		return;
	}

	//### doesn't work for collections
	if(0 && (shape->mBaseFlags & SHAPE_RENDERER_TRANSPARENT) && mPass!=PINT_RENDER_PASS_SHADOW && mPass!=PINT_RENDER_PASS_WIREFRAME_OVERLAY)
	{
		BatchedData* BD = ICE_RESERVE(BatchedData, mTransparentShapes);

		BD->mRenderer = shape;
		BD->mPose = pose;
		return;
	}

	if(0)
	{
		SetupMaterial(shape);
		RenderShape(shape, pose);
		//SetupMaterialAndRenderShape(shape, pose);
	}

	// ### could we instead have the RenderCollection call the batching code?
	// - have a new Render vcall in PintShapeRenderer (not _Render, something like PrepareRender)
	// - default implementation calls a DefaultRenderer's function doing the transparent stuff above + the SetupMaterial / RenderShape calls
	// - RenderCollection does its loop and calls the same function on each of its shapes
	// Maybe not really "better" than the current version, it still skips RenderCollection's _Render(). Oh well.
	const PintShapeRenderer::RenderData* RD = shape->GetRenderers();
	if(RD)
	{
		udword NbToGo = shape->GetNbRenderers();
		while(NbToGo--)
		{
			// TODO: this PR stuff is very old and lame.... to revisit
			PR GlobalPose = RD->mPose;
			GlobalPose *= pose;

//			SetupMaterial(RD->mRenderer);
//			RenderShape(RD->mRenderer, GlobalPose);
			ProcessShape(RD->mRenderer, GlobalPose);

			RD++;
		}
	}
	else
	{
//		SetupMaterial(shape);
//		RenderShape(shape, pose);
		ProcessShape(shape, pose);
	}
}

void DefaultRenderer::DrawSQ(PintRenderPass render_pass)
{
	const udword NbVerts = mSegments.GetNbVertices();
	if(NbVerts)
	{
		const Point* V = mSegments.GetVertices();

		glDisable(GL_LIGHTING);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
//		glColor4f(color.x, color.y, color.z, 1.0f);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Point), &V->x);
		glDrawArrays(GL_LINES, 0, NbVerts);
		glDisableClientState(GL_VERTEX_ARRAY);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_LIGHTING);
	}
}
