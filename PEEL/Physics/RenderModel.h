///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

	struct PintSurfaceInterface;
	class PintGUIHelper;
	class Widgets;
	class PintShapeRenderer;
	class ManagedTexture;
	//struct ManagedTexture;

	class RenderModel : public Allocateable
	{
		public:
									RenderModel()	{}
		virtual						~RenderModel()	{}

		virtual	const char*			GetUIName()		const																		= 0;
		virtual	bool				Init()																						= 0;
		virtual	bool				Close()																						= 0;

		virtual	IceWindow*			InitGUI(IceWidget* parent, Widgets* owner, PintGUIHelper& helper)							= 0;

		virtual	bool				HasSpecialGroundPlane()	const																= 0;
//		virtual	bool				NeedsVertexNormals()	const																= 0;
		virtual	void				SetGroundPlane(bool b)																		= 0;
		virtual	void				InitScene(const Point& center, float size)													= 0;
		virtual	void				SetupCamera()																				= 0;
		virtual	void				Render(udword width, udword height, const Point& cam_pos, const Point& cam_dir, float fov)	= 0;

		// TODO: make these private, only possible to call via RenderManager class or something
		virtual	PintShapeRenderer*	_CreateSphereRenderer(float radius, bool geo_sphere)										= 0;
		virtual	PintShapeRenderer*	_CreateCapsuleRenderer(float radius, float height)											= 0;
		virtual	PintShapeRenderer*	_CreateCylinderRenderer(float radius, float height)											= 0;
		virtual	PintShapeRenderer*	_CreateBoxRenderer(const Point& extents)													= 0;
		virtual	PintShapeRenderer*	_CreateConvexRenderer(udword nb_verts, const Point* verts)									= 0;
		virtual	PintShapeRenderer*	_CreateMeshRenderer(const PintSurfaceInterface& surface, const Point* normals, bool active_edges, bool direct_data)	= 0;
		virtual	PintShapeRenderer*	_CreateMeshRenderer(const MultiSurface& multi_surface, bool active_edges, bool direct_data)	= 0;
		virtual	PintShapeRenderer*	_CreateColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture)= 0;

		// WIP
		virtual	void				SetupCurrentModelMatrix(const float* m)														{}
	};

	enum RenderModelType
	{
		RENDER_MODEL_FFP,
		RENDER_MODEL_SIMPLE_SHADER_1,
		RENDER_MODEL_SIMPLE_SHADER_2,
		RENDER_MODEL_MATCAP,
		RENDER_MODEL_SIMPLE_SHADOWS,

		RENDER_MODEL_COUNT
	};

	void CreateRenderModels(RenderModel** render_models);

#endif