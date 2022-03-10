///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RenderModelMatcap.h"
#include "Camera.h"
#include "MatcapShader.h"
#include "PEEL.h"
#include "GLTexture.h"
#include "TextureManager.h"
#include "Scattering.h"

static MatcapShader* gSimpleShader = null;
static GLuint	gEnvTexId = 0;

RenderModel_Matcap::RenderModel_Matcap()
{
}

RenderModel_Matcap::~RenderModel_Matcap()
{
}

bool RenderModel_Matcap::Init()
{
	glDisable(GL_FOG);

	if(1)
	{
		Picture EnvMap;
		EnvMap.Init(256, 256);
		EnvMap.CreatePhongTable();
//		EnvMap.MakePerlin();
//		EnvMap.MakePerlin();
		gEnvTexId = GLTexture::CreateTexture(256, 256, EnvMap.GetPixels(), true);
		SystemTexture* ST = CreateSystemTexture(256, 256, gEnvTexId, "Mapcap");
	}

	gSimpleShader = ICE_NEW(MatcapShader);
	gSimpleShader->Init();
	return true;
}

bool RenderModel_Matcap::Close()
{
	DELETESINGLE(gSimpleShader)
	return false;
}

IceWindow* RenderModel_Matcap::InitGUI(IceWidget* parent, Widgets* owner, PintGUIHelper& helper)
{
	return null;
}

bool RenderModel_Matcap::HasSpecialGroundPlane() const
{
	return false;
}

void RenderModel_Matcap::SetGroundPlane(bool b)
{
}

void RenderModel_Matcap::InitScene(const Point& center, float size)
{
}

void RenderModel_Matcap::SetupCamera()
{
	SetupProjectionMatrix();
	SetupModelViewMatrix();
}

void RenderModel_Matcap::Render(udword width, udword height, const Point& cam_pos, const Point& cam_dir, float fov)
{
	SetupCamera();	// ### already done I think
	UpdateShaderCameraParams();

	const Point SunDir = -GetSunDir();

	MatcapShaderProps Mtl;
	Mtl.mBackLightDir = SunDir;
	Mtl.mEnvTexId = gEnvTexId;
	Mtl.mEnvironmentMappingCoeff = 0.5f;
	gSimpleShader->Activate(Mtl);

	DrawScene(PINT_RENDER_PASS_MAIN);
	gSimpleShader->Deactivate();
	DrawSQResults(PINT_RENDER_PASS_MAIN);
}

void RenderModel_Matcap::UpdateShaderCameraParams()
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
#include "PintColorShapeRenderer.h"
#include "PintBatchConvexShapeRenderer.h"
#include "PintConvexInstanceShapeRenderer.h"
#include "PintGLMeshShapeRenderer.h"

PintShapeRenderer* RenderModel_Matcap::_CreateSphereRenderer(float radius, bool geo_sphere)
{
	return CreatePintDLSphereShapeRenderer(radius, true, geo_sphere);
}

PintShapeRenderer* RenderModel_Matcap::_CreateCapsuleRenderer(float radius, float height)
{
	return CreatePintDLCapsuleShapeRenderer(radius, height, true);
}

PintShapeRenderer* RenderModel_Matcap::_CreateCylinderRenderer(float radius, float height)
{
	return CreatePintDLCylinderShapeRenderer(radius, height);
}

PintShapeRenderer* RenderModel_Matcap::_CreateBoxRenderer(const Point& extents)
{
	return CreatePintDLBoxShapeRenderer(extents, true);
}

PintShapeRenderer* RenderModel_Matcap::_CreateConvexRenderer(udword nb_verts, const Point* verts)
{
	return ICE_NEW(PintBatchConvexShapeRenderer)(nb_verts, verts);
}

static inline_ udword GetMeshFlags(bool active_edges, bool direct_data)
{
	udword Flags = DL_MESH_USE_VERTEX_NORMALS|DL_MESH_USE_SMOOTH_NORMALS;
	if(active_edges)
		Flags |= DL_MESH_USE_ACTIVE_EDGES;
	if(direct_data)
		Flags |= DL_MESH_USE_DIRECT_DATA;
	return Flags;
}

PintShapeRenderer* RenderModel_Matcap::_CreateMeshRenderer(const PintSurfaceInterface& surface, const Point* normals, bool active_edges, bool direct_data)
{
	return ICE_NEW(PintGLMeshShapeRenderer)(surface, DL_MESH_USE_VERTEX_NORMALS|DL_MESH_USE_SMOOTH_NORMALS, normals);
	//return ICE_NEW(PintDLMeshShapeRenderer)(surface, GetMeshFlags(active_edges, direct_data));
}

PintShapeRenderer* RenderModel_Matcap::_CreateMeshRenderer(const MultiSurface& multi_surface, bool active_edges, bool direct_data)
{
	return CreatePintDLMeshShapeRenderer(multi_surface, GetMeshFlags(active_edges, direct_data));
}

PintShapeRenderer* RenderModel_Matcap::_CreateColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture)
{
	return CreatePintColorShapeRenderer(renderer, color, texture);
}

///////////////////////////////////////////////////////////////////////////////
