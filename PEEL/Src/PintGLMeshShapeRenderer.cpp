///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintSurfaceInterface.h"
#include "PintGLMeshShapeRenderer.h"
#include "GLRenderHelpers.h"
#include "GLMesh.h"

// A GL-mesh-based mesh renderer. Not batched.

PintGLMeshShapeRenderer::PintGLMeshShapeRenderer(const PintSurfaceInterface& surface, udword flags, const Point* normals)
{
//	const GLMeshType type = GL_MESH_DISPLAY_LIST;
	const GLMeshType type = GL_MESH_VBO;

	if(normals)
	{
		mBaseFlags |= SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS;
		mMesh = OpenGLMesh::Create(surface.mNbVerts, surface.mVerts, normals, surface.mNbFaces, surface.mDFaces, surface.mWFaces, type);
	}
	else if((flags & DL_MESH_USE_VERTEX_NORMALS))
	{
		Point* Normals = ICE_NEW(Point)[surface.mNbVerts];
		BuildSmoothNormals(surface.mNbFaces, surface.mNbVerts, surface.mVerts, surface.mDFaces, surface.mWFaces, Normals, true);

		mBaseFlags |= SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS;
		mMesh = OpenGLMesh::Create(surface.mNbVerts, surface.mVerts, Normals, surface.mNbFaces, surface.mDFaces, surface.mWFaces, type);

		DELETEARRAY(Normals);
	}
	else
	{
		mMesh = OpenGLMesh::Create(surface.mNbVerts, surface.mVerts, null, surface.mNbFaces, surface.mDFaces, surface.mWFaces, type);
	}
}

PintGLMeshShapeRenderer::~PintGLMeshShapeRenderer()
{
	OpenGLMesh::Release(mMesh);
}

void PintGLMeshShapeRenderer::_Render(const PR& pose) const
{
	glPushMatrix();
	{
		GLRenderHelpers::SetupGLMatrix(pose);
		OpenGLMesh::Draw(mMesh);
	}
	glPopMatrix();
}

bool PintGLMeshShapeRenderer::UpdateVerts(const Point* pts, const Point* normals)
{
	OpenGLMesh::UpdateVerts(mMesh, pts, null);
	return true;
}









//#ifdef TOSEE

PintGLMeshShapeRendererEx::PintGLMeshShapeRendererEx(const MultiSurface& multi_surface, udword flags)
{
	if(flags & DL_MESH_USE_ACTIVE_EDGES)
		mActiveEdges.CreateDL(multi_surface.GetSurfaceInterface());

	const bool Deformable = (flags & DL_MESH_DEFORMABLE)!=0;

	mMesh = OpenGLMesh::Create(multi_surface, flags, Deformable);

	if(flags & (DL_MESH_USE_DIRECT_DATA|DL_MESH_USE_VERTEX_NORMALS))
		mBaseFlags |= SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS;
}

PintGLMeshShapeRendererEx::~PintGLMeshShapeRendererEx()
{
	OpenGLMesh::Release(mMesh);
}

extern	bool	gWireframePass;
void PintGLMeshShapeRendererEx::_Render(const PR& pose) const
{
	glPushMatrix();
	GLRenderHelpers::SetupGLMatrix(pose);
	if(gWireframePass && mActiveEdges.mActiveEdgesDL!=INVALID_ID)
		glCallList(mActiveEdges.mActiveEdgesDL);
	else
		OpenGLMesh::Draw(mMesh);
	glPopMatrix();
}

bool PintGLMeshShapeRendererEx::UpdateVerts(const Point* pts, const Point* normals)
{
	OpenGLMesh::UpdateVerts(mMesh, pts);
	return true;
}

//#endif
