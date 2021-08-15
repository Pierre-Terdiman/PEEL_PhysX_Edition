///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_GL_MESH_SHAPE_RENDERER_H
#define PINT_GL_MESH_SHAPE_RENDERER_H

#include "PintShapeRenderer.h"
#include "PintDLMeshShapeRenderer.h"	// For ActiveEdgesRenderer

	class GLMesh;

	class PintGLMeshShapeRenderer : public PintShapeRenderer
	{
		public:
							PintGLMeshShapeRenderer(const PintSurfaceInterface& surface, udword flags, const Point* normals);
		virtual				~PintGLMeshShapeRenderer();

		// PintShapeRenderer
		virtual	const char*	GetClassName()							const	override	{ return "PintGLMeshShapeRenderer";	}
		virtual	void		_Render(const PR& pose)					const	override;
		virtual	bool		UpdateVerts(const Point*, const Point* normals)	override;
		//~PintShapeRenderer

		private:
				GLMesh*		mMesh;
	};

//#ifdef TOSEE
	// A new experiment
	class GLMeshEx;
	class PintGLMeshShapeRendererEx : public PintShapeRenderer
	{
		public:
									PintGLMeshShapeRendererEx(const MultiSurface& multi_surface, udword flags);
		virtual						~PintGLMeshShapeRendererEx();

		// PintShapeRenderer
		virtual	const char*			GetClassName()							const	override{ return "PintGLMeshShapeRendererEx";	}
		virtual	void				_Render(const PR& pose)					const	override;
		virtual	bool				UpdateVerts(const Point*, const Point* normals)	override;
		//~PintShapeRenderer

		private:
				ActiveEdgesRenderer	mActiveEdges;
				GLMeshEx*			mMesh;
	};
//#endif

#endif