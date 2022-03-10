///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_DL_SPHERE_SHAPE_RENDERER_H
#define PINT_DL_SPHERE_SHAPE_RENDERER_H

#include "PintDLShapeRenderer.h"

	class PintDLSphereShapeRenderer : public PintDLShapeRenderer
	{
		friend PintDLSphereShapeRenderer* CreatePintDLSphereShapeRenderer(float radius, bool uses_vertex_normals, bool geo_sphere);
							PintDLSphereShapeRenderer(float radius, bool uses_vertex_normals, bool geo_sphere);
		public:

		// PintShapeRenderer
		virtual	PtrContainer*	GetOwnerContainer()	const	override;
		virtual	const char*		GetClassName()		const	override	{ return "PintDLSphereShapeRenderer";	}
		//~PintShapeRenderer

		struct Data
		{
			inline_	Data(float radius, bool uses_vertex_normals, bool geo_sphere) : mRadius(radius), mUsesVertexNormals(uses_vertex_normals), mGeoSphere(geo_sphere){}

			const float	mRadius;
			const bool	mUsesVertexNormals;
			const bool	mGeoSphere;
		};

		inline_ bool Equal(const Data& p)	const
		{
			return mData.mRadius==p.mRadius && mData.mUsesVertexNormals==p.mUsesVertexNormals && mData.mGeoSphere==p.mGeoSphere;
		};

				const Data		mData;	// We only store this to implement sharing
	};

	PintDLSphereShapeRenderer* CreatePintDLSphereShapeRenderer(float radius, bool uses_vertex_normals, bool geo_sphere);

#endif