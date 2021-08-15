///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_TERRAIN_TILE_RENDERER_H
#define PINT_TERRAIN_TILE_RENDERER_H

#include "PintShapeRenderer.h"
#include "GLMesh.h"

	class PintTerrainTileRenderer : public PintShapeRenderer
	{
		public:
								PintTerrainTileRenderer(GLIndexBufferCollection& index_buffers, udword nb_verts, const Point* verts, const Point* vnormals);
		virtual					~PintTerrainTileRenderer();

		// PintShapeRenderer
		virtual	const char*		GetClassName()							const	override	{ return "PintTerrainTileRenderer";	}
		virtual	void			_Render(const PR& pose)					const	override;
		virtual	bool			UpdateVerts(const Point*, const Point* normals)	override;
		//~PintShapeRenderer

		private:
				GLIndexBufferCollection&	mIndexBuffers;
				GLVertexBuffer				mVertexBuffer;
		public:
				udword						mLOD;
	};

#endif