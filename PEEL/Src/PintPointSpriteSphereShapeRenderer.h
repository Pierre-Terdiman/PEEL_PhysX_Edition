///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_POINT_SPRITE_SPHERE_SHAPE_RENDERER_H
#define PINT_POINT_SPRITE_SPHERE_SHAPE_RENDERER_H

#include "PintShapeRenderer.h"

	class PintPointSpriteSphereShapeRenderer : public PintShapeRenderer
	{
				float	mRadius;
		public:
						PintPointSpriteSphereShapeRenderer(float radius);

		// PintShapeRenderer
		virtual	const char*	GetClassName()			const	override	{ return "PintPointSpriteSphereShapeRenderer";	}
		virtual	void		_Render(const PR& pose)	const	override;
		//~PintShapeRenderer
	};

#endif