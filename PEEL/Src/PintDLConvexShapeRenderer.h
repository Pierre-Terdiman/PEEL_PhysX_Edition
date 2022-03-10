///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_DL_CONVEX_SHAPE_RENDERER_H
#define PINT_DL_CONVEX_SHAPE_RENDERER_H

#include "PintDLShapeRenderer.h"

#define USE_NEW_CONVEX_HULL
#ifdef USE_NEW_CONVEX_HULL
//	#include "ConvexHull.h"
	typedef	ConvexHull2 Cvh;
#else
	typedef	ConvexHull Cvh;
#endif


	Cvh* CreateConvexHull(udword nb_verts, const Point* verts);
	void DrawHull(const Cvh& hull);

	inline_ void CreateHullDisplayList(GLuint list_handle, Cvh* hull)
	{
		glNewList(list_handle, GL_COMPILE);
			if(hull)
				DrawHull(*hull);
		glEndList();
	}

	class PintDLConvexShapeRenderer : public PintDLShapeRenderer
	{
		public:

		PintDLConvexShapeRenderer(udword nb_verts, const Point* verts);

		// PintShapeRenderer
		virtual	const char*	GetClassName()		const	override	{ return "PintDLConvexShapeRenderer";	}
		//~PintShapeRenderer
	};

#endif