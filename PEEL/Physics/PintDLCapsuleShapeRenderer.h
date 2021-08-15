///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_DL_CAPSULE_SHAPE_RENDERER_H
#define PINT_DL_CAPSULE_SHAPE_RENDERER_H

#include "PintDLShapeRenderer.h"

	class PintDLCapsuleShapeRenderer : public PintDLShapeRenderer
	{
		friend PintDLCapsuleShapeRenderer* CreatePintDLCapsuleShapeRenderer(float r, float h, bool uses_vertex_normals);
		PintDLCapsuleShapeRenderer(float r, float h, bool uses_vertex_normals);

		public:

		// PintShapeRenderer
		virtual	PtrContainer*	GetOwnerContainer()	const	override;
		virtual	const char*		GetClassName()		const	override	{ return "PintDLCapsuleShapeRenderer";	}
		//~PintShapeRenderer

		struct Data
		{
			inline_	Data(float radius, float height, bool uses_vertex_normals) : mRadius(radius), mHeight(height), mUsesVertexNormals(uses_vertex_normals)	{}

			const float	mRadius;
			const float	mHeight;
			const bool	mUsesVertexNormals;
		};

		inline_ bool Equal(const Data& p)	const
		{
			return mData.mRadius==p.mRadius && mData.mHeight==p.mHeight && mData.mUsesVertexNormals==p.mUsesVertexNormals;
		};

				const Data		mData;	// We only store this to implement sharing
	};

	PintDLCapsuleShapeRenderer* CreatePintDLCapsuleShapeRenderer(float radius, float height, bool uses_vertex_normals);

#endif