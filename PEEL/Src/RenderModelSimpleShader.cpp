///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RenderModelSimpleShader.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "PEEL.h"
#include "Scattering.h"

static SimpleShader* gSimpleShader = null;

RenderModel_SimpleShader::RenderModel_SimpleShader(bool needs_vertex_normals) :
	mNeedsVertexNormals(needs_vertex_normals)
{
}

RenderModel_SimpleShader::~RenderModel_SimpleShader()
{
}

bool RenderModel_SimpleShader::Init()
{
	glDisable(GL_FOG);

	gSimpleShader = ICE_NEW(SimpleShader)(mNeedsVertexNormals);
	gSimpleShader->Init();
	return true;
}

bool RenderModel_SimpleShader::Close()
{
	DELETESINGLE(gSimpleShader)
	return false;
}

IceWindow* RenderModel_SimpleShader::InitGUI(IceWidget* parent, Widgets* owner, PintGUIHelper& helper)
{
	return null;
}

bool RenderModel_SimpleShader::HasSpecialGroundPlane() const
{
	return false;
}

/*bool RenderModel_SimpleShader::NeedsVertexNormals() const
{
	return mNeedsVertexNormals;
}*/

void RenderModel_SimpleShader::SetGroundPlane(bool b)
{
}

void RenderModel_SimpleShader::InitScene(const Point& center, float size)
{
}

void RenderModel_SimpleShader::SetupCamera()
{
	SetupProjectionMatrix();
	SetupModelViewMatrix();
}

void RenderModel_SimpleShader::Render(udword width, udword height, const Point& cam_pos, const Point& cam_dir, float fov)
{
	SetupCamera();	// ### already done I think
	UpdateShaderCameraParams();

	const Point SunDir = -GetSunDir();

	SimpleShaderProps Mtl;
	Mtl.mBackLightDir = SunDir;
	gSimpleShader->Activate(Mtl);

	DrawScene(PINT_RENDER_PASS_MAIN);
	gSimpleShader->Deactivate();
	DrawSQResults(PINT_RENDER_PASS_MAIN);
}

void RenderModel_SimpleShader::UpdateShaderCameraParams()
{
	float modelMatrix[16];
	float projMatrix[16];

	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);

	gSimpleShader->__UpdateCamera(modelMatrix, projMatrix);
}

///////////////////////////////////////////////////////////////////////////////

#include "PintDLSphereShapeRenderer.h"
#include "PintDLCapsuleShapeRenderer.h"
#include "PintDLCylinderShapeRenderer.h"
#include "PintDLBoxShapeRenderer.h"
#include "PintDLMeshShapeRenderer.h"
#include "PintGLMeshShapeRenderer.h"
#include "PintColorShapeRenderer.h"
#include "PintBatchConvexShapeRenderer.h"
#include "PintConvexInstanceShapeRenderer.h"

PintShapeRenderer* RenderModel_SimpleShader::_CreateSphereRenderer(float radius, bool geo_sphere)
{
	return CreatePintDLSphereShapeRenderer(radius, mNeedsVertexNormals, geo_sphere);
}

PintShapeRenderer* RenderModel_SimpleShader::_CreateCapsuleRenderer(float radius, float height)
{
	return CreatePintDLCapsuleShapeRenderer(radius, height, mNeedsVertexNormals);
}

PintShapeRenderer* RenderModel_SimpleShader::_CreateCylinderRenderer(float radius, float height)
{
	return CreatePintDLCylinderShapeRenderer(radius, height);
}

PintShapeRenderer* RenderModel_SimpleShader::_CreateBoxRenderer(const Point& extents)
{
	return CreatePintDLBoxShapeRenderer(extents, mNeedsVertexNormals);
}

PintShapeRenderer* RenderModel_SimpleShader::_CreateConvexRenderer(udword nb_verts, const Point* verts)
{
	if(mNeedsVertexNormals)
		return ICE_NEW(PintBatchConvexShapeRenderer)(nb_verts, verts);
	else
		return ICE_NEW(PintConvexInstanceRenderer)(nb_verts, verts);
}

static inline_ udword GetMeshFlags(bool active_edges, bool vertex_normals, bool direct_data)
{
	udword Flags = 0;
	if(active_edges)
		Flags |= DL_MESH_USE_ACTIVE_EDGES;
	if(vertex_normals)
		Flags |= DL_MESH_USE_VERTEX_NORMALS;
	if(direct_data)
		Flags |= DL_MESH_USE_DIRECT_DATA;
	return Flags;
}

PintShapeRenderer* RenderModel_SimpleShader::_CreateMeshRenderer(const PintSurfaceInterface& surface, const Point* normals, bool active_edges, bool direct_data)
{
	if(!mNeedsVertexNormals && !active_edges && !direct_data)
		return ICE_NEW(PintGLMeshShapeRenderer)(surface, 0, null);
	else
		return CreatePintDLMeshShapeRenderer(surface, GetMeshFlags(active_edges, mNeedsVertexNormals, direct_data));
}

PintShapeRenderer* RenderModel_SimpleShader::_CreateMeshRenderer(const MultiSurface& multi_surface, bool active_edges, bool direct_data)
{
	return CreatePintDLMeshShapeRenderer(multi_surface, GetMeshFlags(active_edges, mNeedsVertexNormals, direct_data));
}

PintShapeRenderer* RenderModel_SimpleShader::_CreateColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture)
{
	return CreatePintColorShapeRenderer(renderer, color, texture);
}

///////////////////////////////////////////////////////////////////////////////
