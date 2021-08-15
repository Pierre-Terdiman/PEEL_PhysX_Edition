///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_NULL_RENDERER_H
#define PINT_NULL_RENDERER_H

#include "PintShapeRenderer.h"

	class NullRenderer : public PintShapeRenderer
	{
		public:
							NullRenderer() : PintShapeRenderer(/*SHAPE_RENDERER_UNDEFINED,*/ 0)	{}
		virtual				~NullRenderer()													{}

		// PintShapeRenderer
		virtual	const char*	GetClassName()			const	override	{ return "NullRenderer";	}
		virtual	udword		GetNbRenderers()		const	override	{ return 0;					}
		virtual	void		_Render(const PR& pose)	const	override	{}
		//~PintShapeRenderer
	};

#endif