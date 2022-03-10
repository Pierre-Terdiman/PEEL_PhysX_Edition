///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintTerrainTileRenderer.h"
#include "GLRenderHelpers.h"

PintTerrainTileRenderer::PintTerrainTileRenderer(GLIndexBufferCollection& index_buffers, udword nb_verts, const Point* verts, const Point* vnormals) : mIndexBuffers(index_buffers), mLOD(0)
{
	SPY_ZONE("PintTerrainTileRenderer ctor")

	mVertexBuffer.Create(nb_verts, verts, vnormals);
	if(vnormals)
		mBaseFlags |= SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS;
}

PintTerrainTileRenderer::~PintTerrainTileRenderer()
{
	SPY_ZONE("PintTerrainTileRenderer dtor")

	mVertexBuffer.Release();
}

void PintTerrainTileRenderer::_Render(const PR& pose) const
{
	glPushMatrix();
	{
		GLRenderHelpers::SetupGLMatrix(pose);

		mVertexBuffer.Select();

		const GLIndexBuffer* IndexBuffer = mIndexBuffers.GetIndexBuffer(mLOD);
		IndexBuffer->Select();

		if(IndexBuffer->mHas16BitIndices)
			glDrawElements(GL_TRIANGLES, IndexBuffer->mNbTris*3, GL_UNSIGNED_SHORT, 0);
		else
			glDrawElements(GL_TRIANGLES, IndexBuffer->mNbTris*3, GL_UNSIGNED_INT, 0);

		mVertexBuffer.Unselect();
		IndexBuffer->Unselect();
	}
	glPopMatrix();
}

bool PintTerrainTileRenderer::UpdateVerts(const Point* verts, const Point* normals)
{
	SPY_ZONE("PintTerrainTileRenderer UpdateVerts")

	mVertexBuffer.Update(mVertexBuffer.mNbVerts, verts, normals);
	return true;
}
