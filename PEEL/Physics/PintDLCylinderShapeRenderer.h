///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_DL_CYLINDER_SHAPE_RENDERER_H
#define PINT_DL_CYLINDER_SHAPE_RENDERER_H

#include "PintDLShapeRenderer.h"

	class PintDLCylinderShapeRenderer : public PintDLShapeRenderer
	{
		friend PintDLCylinderShapeRenderer* CreatePintDLCylinderShapeRenderer(float radius, float height);
		PintDLCylinderShapeRenderer(float r, float h);

		public:

		// PintShapeRenderer
		virtual	PtrContainer*	GetOwnerContainer()	const	override;
		virtual	const char*		GetClassName()		const	override	{ return "PintDLCylinderShapeRenderer";	}
		//~PintShapeRenderer

		struct Data
		{
			inline_	Data(float radius, float height) : mRadius(radius), mHeight(height)	{}

			const float	mRadius;
			const float	mHeight;
		};

		inline_ bool Equal(const Data& p)	const
		{
			return mData.mRadius==p.mRadius && mData.mHeight==p.mHeight;
		};

				const Data		mData;	// We only store this to implement sharing
	};

	PintDLCylinderShapeRenderer* CreatePintDLCylinderShapeRenderer(float radius, float height);

#endif