///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_DL_BOX_SHAPE_RENDERER_H
#define PINT_DL_BOX_SHAPE_RENDERER_H

#include "PintDLShapeRenderer.h"

	class PintDLBoxShapeRenderer : public PintDLShapeRenderer
	{
		friend PintDLBoxShapeRenderer* CreatePintDLBoxShapeRenderer(const Point& extents, bool uses_vertex_normals);
								PintDLBoxShapeRenderer(const Point& extents, bool uses_vertex_normals);
		virtual					~PintDLBoxShapeRenderer();

		public:

		// PintShapeRenderer
		virtual	PtrContainer*	GetOwnerContainer()		const	override;
		virtual	const char*		GetClassName()			const	override	{ return "PintDLBoxShapeRenderer";	}
		virtual	void			_Render(const PR& pose)	const	override;
		//~PintShapeRenderer

		struct Data
		{
			inline_	Data(const Point& extents, bool uses_vertex_normals) : mExtents(extents), mUsesVertexNormals(uses_vertex_normals)	{}

			const Point	mExtents;
			const bool	mUsesVertexNormals;
		};

		inline_ bool Equal(const Data& p)	const
		{
			return mData.mExtents==p.mExtents && mData.mUsesVertexNormals==p.mUsesVertexNormals;
		};

				const Data		mData;	// We only store this to implement sharing
		private:
				GLuint			mDisplayListNum2;
	};

	PintDLBoxShapeRenderer* CreatePintDLBoxShapeRenderer(const Point& extents, bool uses_vertex_normals);

#endif