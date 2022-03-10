///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef RENDER_MODEL_SIMPLE_SHADOWS_H
#define RENDER_MODEL_SIMPLE_SHADOWS_H

#include "RenderModel.h"

	class RenderModel_SimpleShadows : public RenderModel
	{
		public:
									RenderModel_SimpleShadows();
		virtual						~RenderModel_SimpleShadows();

		// RenderModel
		virtual	const char*			GetUIName()		const	override	{ return "Simple shadows";	}
		virtual	bool				Init()	override;
		virtual	bool				Close()	override;
		virtual	IceWindow*			InitGUI(IceWidget* parent, Widgets* owner, PintGUIHelper& helper)	override;
		virtual	bool				HasSpecialGroundPlane()	const	override;
//			virtual	bool				NeedsVertexNormals()	const;
		virtual	void				SetGroundPlane(bool b)	override;
		virtual	void				InitScene(const Point& center, float size)	override;
		virtual	void				SetupCamera()	override;
		virtual	void				Render(udword width, udword height, const Point& cam_pos, const Point& cam_dir, float fov)	override;

		virtual	PintShapeRenderer*	_CreateSphereRenderer(float radius, bool geo_sphere)	override;
		virtual	PintShapeRenderer*	_CreateCapsuleRenderer(float radius, float height)	override;
		virtual	PintShapeRenderer*	_CreateCylinderRenderer(float radius, float height)	override;
		virtual	PintShapeRenderer*	_CreateBoxRenderer(const Point& extents)	override;
		virtual	PintShapeRenderer*	_CreateConvexRenderer(udword nb_verts, const Point* verts)	override;
		virtual	PintShapeRenderer*	_CreateMeshRenderer(const PintSurfaceInterface& surface, const Point* normals, bool active_edges, bool direct_data)	override;
		virtual	PintShapeRenderer*	_CreateMeshRenderer(const MultiSurface& multi_surface, bool active_edges, bool direct_data)	override;
		virtual	PintShapeRenderer*	_CreateColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture)	override;

		virtual	void				SetupCurrentModelMatrix(const float* m)	override;
		//~RenderModel

		private:
//				void				UpdateShaderCameraParams();
				void				UpdateCamera(bool mirrored);
	};

#endif