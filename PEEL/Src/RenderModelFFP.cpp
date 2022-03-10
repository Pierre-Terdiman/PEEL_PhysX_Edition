///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RenderModelFFP.h"
#include "Camera.h"
#include "PEEL.h"
#include "Scattering.h"

RenderModel_FFP::RenderModel_FFP()
{
}

RenderModel_FFP::~RenderModel_FFP()
{
}

static void InitLighting()
{
	glEnable(GL_COLOR_MATERIAL);

	const float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, zero);

	glEnable(GL_LIGHTING);
//	Point Dir(-1.0f, 1.0f, 0.5f);
	//Point Dir(0.0f, 1.0f, 0.0f);
	//Dir.Normalize();
	const Point Dir = GetSunDir();

	const float AmbientValue = 0.3f;
	const float ambientColor0[]		= { AmbientValue, AmbientValue, AmbientValue, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor0);

	const float specularColor0[]	= { 0.0f, 0.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor0);

	const float diffuseColor0[]	= { 0.7f, 0.7f, 0.7f, 0.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor0);

	const float position0[]		= { Dir.x, Dir.y, Dir.z, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, position0);

	glEnable(GL_LIGHT0);

//	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);

	if(0)
	{
		glEnable(GL_FOG);
		//glFogi(GL_FOG_MODE,GL_LINEAR); 
		//glFogi(GL_FOG_MODE,GL_EXP); 
		glFogi(GL_FOG_MODE,GL_EXP2); 
		glFogf(GL_FOG_START, 0.0f);
		glFogf(GL_FOG_END, 100.0f);
		glFogf(GL_FOG_DENSITY, 0.005f);
//		glClearColor(0.2f, 0.2f, 0.2f, 1.0);
		const Point FogColor(0.2f, 0.2f, 0.2f);
		glFogfv(GL_FOG_COLOR, &FogColor.x);
	}
}

bool RenderModel_FFP::Init()
{
	glDisable(GL_FOG);

	InitLighting();

	return true;
}

bool RenderModel_FFP::Close()
{
	return true;
}

IceWindow* RenderModel_FFP::InitGUI(IceWidget* parent, Widgets* owner, PintGUIHelper& helper)
{
	return null;
}

bool RenderModel_FFP::HasSpecialGroundPlane() const
{
	return false;
}

/*bool RenderModel_FFP::NeedsVertexNormals() const
{
	return true;
}*/

void RenderModel_FFP::SetGroundPlane(bool b)
{
}

void RenderModel_FFP::InitScene(const Point& center, float size)
{
//	InitLighting();

	// I think I need to do that each frame to make the light view-indep or something

//	Point Dir(-1.0f, 1.0f, 0.5f);
//	Point Dir(0.0f, 1.0f, 0.0f);
//	Dir.Normalize();
	const Point Dir = GetSunDir();

	const float position0[]		= { Dir.x, Dir.y, Dir.z, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, position0);
}

void RenderModel_FFP::SetupCamera()
{
	SetupProjectionMatrix();
	SetupModelViewMatrix();
}

void RenderModel_FFP::Render(udword width, udword height, const Point& cam_pos, const Point& cam_dir, float fov)
{
	DrawScene(PINT_RENDER_PASS_MAIN);
	DrawSQResults(PINT_RENDER_PASS_MAIN);
}

///////////////////////////////////////////////////////////////////////////////

#include "PintDLSphereShapeRenderer.h"
#include "PintDLCapsuleShapeRenderer.h"
#include "PintDLCylinderShapeRenderer.h"
#include "PintDLBoxShapeRenderer.h"
#include "PintDLConvexShapeRenderer.h"
#include "PintDLMeshShapeRenderer.h"
#include "PintGLMeshShapeRenderer.h"
#include "PintColorShapeRenderer.h"
#include "PintBatchConvexShapeRenderer.h"
//#include "PintPointSpriteSphereShapeRenderer.h"
#include "Sphere.h"

PintShapeRenderer* RenderModel_FFP::_CreateSphereRenderer(float radius, bool geo_sphere)
{
	if(0)
	{
		const udword NbCirclePts = 12;
		const udword NbRotations = 12;
		const SphereMesh SM(NbCirclePts, NbRotations, radius);
		return ICE_NEW(PintBatchConvexShapeRenderer)(SM.mNbVerts, SM.mVerts);
	}

	return CreatePintDLSphereShapeRenderer(radius, true, geo_sphere);
//	return ICE_NEW(PintPointSpriteSphereShapeRenderer)(radius);
}

PintShapeRenderer* RenderModel_FFP::_CreateCapsuleRenderer(float radius, float height)
{
	return CreatePintDLCapsuleShapeRenderer(radius, height, true);
}

PintShapeRenderer* RenderModel_FFP::_CreateCylinderRenderer(float radius, float height)
{
	return CreatePintDLCylinderShapeRenderer(radius, height);
}

PintShapeRenderer* RenderModel_FFP::_CreateBoxRenderer(const Point& extents)
{
//	return ICE_NEW(PintDLBoxShapeRenderer)(extents, true);
	AABB box;
	box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), extents);

	Point Pts[8];
	box.ComputePoints(Pts);

	return ICE_NEW(PintBatchConvexShapeRenderer)(8, Pts);
}

PintShapeRenderer* RenderModel_FFP::_CreateConvexRenderer(udword nb_verts, const Point* verts)
{
	return ICE_NEW(PintBatchConvexShapeRenderer)(nb_verts, verts);
//	return ICE_NEW(PintDLConvexShapeRenderer)(nb_verts, verts);
}

static inline_ udword GetMeshFlags(bool active_edges, bool direct_data)
{
	udword Flags = DL_MESH_USE_VERTEX_NORMALS;
	if(active_edges)
		Flags |= DL_MESH_USE_ACTIVE_EDGES;
	if(direct_data)
		Flags |= DL_MESH_USE_DIRECT_DATA;
	return Flags;
}

PintShapeRenderer* RenderModel_FFP::_CreateMeshRenderer(const PintSurfaceInterface& surface, const Point* normals, bool active_edges, bool direct_data)
{
	// Can't use PintGLMeshShapeRenderer here without passing explicit vertex normals
//	if(!active_edges && !direct_data)
//		return ICE_NEW(PintGLMeshShapeRenderer)(surface);
//	else
		return CreatePintDLMeshShapeRenderer(surface, GetMeshFlags(active_edges, direct_data));
}

PintShapeRenderer* RenderModel_FFP::_CreateMeshRenderer(const MultiSurface& multi_surface, bool active_edges, bool direct_data)
{
	return CreatePintDLMeshShapeRenderer(multi_surface, GetMeshFlags(active_edges, direct_data));
}

PintShapeRenderer* RenderModel_FFP::_CreateColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture)
{
	return CreatePintColorShapeRenderer(renderer, color, texture);
}

///////////////////////////////////////////////////////////////////////////////

