///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RenderModelSimpleShadows.h"
#include "Camera.h"
#include "ShadowShader.h"
#include "FlatShader.h"
#include "ShadowMap.h"
#include "RenderTarget.h"
#include "GLTexture.h"
#include "TextureManager.h"
#include "Pint.h"
#include "PEEL.h"
#include "GUI_Helpers.h"
#include "Scattering.h"
#include "DefaultEnv.h"

#include "PxTransform.h"
using namespace physx;

static const int gNumLights = 3;
static PxVec3 gLightDirs[gNumLights];
static PxVec3 gLightPos[gNumLights];
static ShadowMap* gShadowMaps[gNumLights];
/*static*/ ShadowShader* gDefaultShader = null;
/*static*/ PEEL::RenderTarget* gReflectedSceneTarget = null;

static FlatShader* gFlatShader = null;

static udword gRenderTargetWidth = 0;
static udword gRenderTargetHeight = 0;

static float	gGroundY = 0.0f;
static GLuint	gGroundTexId = 0;
static GLuint	gEnvTexId = 0;
static GLuint	gObjectTexId = 0;
static SystemTexture*	gEnvST = null;

static int gNumShadows = 3;
static bool gShowReflections = true;
static bool gDrawGroundPlane = true;

static float gReflectionCoeff = 0.4f;
//static float gShadowAmbient = 0.1f;
static float gShadowAmbient = 0.0f;
static float gCurrentPhongPower = 0.0f;
//static float gPhongPower = 3.0f;
static float gPhongPower = 10.0f;
//static float gEnvCoeff = 0.2f;
static float gEnvCoeff = 0.6f;
static float gShadowClipNear = 1.0f;
//static float gShadowClipFar = 100.0f;
static float gShadowClipFar = 500.0f;
static float gLitAreaSize = -1.0f;
//static udword gNbSplits = 1;
static udword gNbSplits = 2;
//static udword gShadowMapRes = 2048;
static udword gShadowMapRes = 4096;
//static udword gShadowMapWidth = 1024;
static udword gShadowMapWidth = 256;
//static udword gShadowMapHeight = 768;
static udword gShadowMapHeight = 256;

		bool	gShadowPass = false;

RenderModel_SimpleShadows::RenderModel_SimpleShadows()
{
}

RenderModel_SimpleShadows::~RenderModel_SimpleShadows()
{
}

bool RenderModel_SimpleShadows::Init()
{
	gDefaultShader = ICE_NEW(ShadowShader);
	const bool status = gDefaultShader->init();

//	for(int i=0; i<gNumLights; i++)
//		gShadowMaps[i] = null;

	const float fov = 60.0f;
	for(int i=0; i<gNumLights; i++)
		gShadowMaps[i] = new ShadowMap(gShadowMapWidth, gShadowMapHeight, fov, i, gShadowMapRes, gNbSplits);

	if(1)
	{
		Picture EnvMap;
		EnvMap.Init(256, 256);
		gCurrentPhongPower = gPhongPower;
		EnvMap.CreatePhongTable(gCurrentPhongPower);
//		EnvMap.MakePerlin();
//		EnvMap.MakePerlin();
		gEnvTexId = GLTexture::createTexture(256, 256, EnvMap.GetPixels(), true);
		gEnvST = CreateSystemTexture(256, 256, gEnvTexId, "Phong");
	}

	if(0)
	{
		gGroundTexId = GLTexture::createSingleColorTexture(256, 256, RGBAPixel(255, 255, 255, 255), true);
		gEnvTexId = GLTexture::createSingleColorTexture(256, 256, RGBAPixel(255, 0, 0, 255), true);
		gObjectTexId = GLTexture::createSingleColorTexture(256, 256, RGBAPixel(0, 0, 255, 255), true);
	}

	if(0)
	{
		Picture Pic(256, 256);
		Pic.MakeColorPicker();
		gObjectTexId = GLTexture::createTexture(256, 256, Pic.GetPixels(), true);
	}

	if(0)
	{
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_DENSITY, 0.0009f);
		glFogf(GL_FOG_START, 20.0f);
		glFogf(GL_FOG_END, 100.0f);
		const Point FogColor(0.2f, 0.2f, 0.2f);
		glFogfv(GL_FOG_COLOR, &FogColor.x);
	}

	if(0)
	{
		gFlatShader = ICE_NEW(FlatShader);
		gFlatShader->init();
	}

	return status;
}

bool RenderModel_SimpleShadows::Close()
{
	DELETESINGLE(gReflectedSceneTarget)

	DELETESINGLE(gFlatShader)
	DELETESINGLE(gDefaultShader)

	for(int i=0; i<gNumLights; i++)
	{
		DELETESINGLE(gShadowMaps[i]);
	}
	return false;
}

bool RenderModel_SimpleShadows::HasSpecialGroundPlane() const
{
	return true;
}

/*bool RenderModel_SimpleShadows::NeedsVertexNormals() const
{
//	return false;
	return true;
}*/

void RenderModel_SimpleShadows::SetGroundPlane(bool b)
{
	gDrawGroundPlane = b;
	gShowReflections = b;
	if(gDefaultShader)
		gDefaultShader->setShowReflection(b);
}

void RenderModel_SimpleShadows::InitScene(const Point& c, float size)
{
	const PxVec3 center(c.x, c.y, c.z);

	if(gLitAreaSize>0.0f)
		size = gLitAreaSize;

	const float dphi = TWOPI / 3.0f;
	const float phi0 = PI / 4.0f;//0.0f;

	const bool moveLightsWithTarget = true;

	//gLightPos[0] = PxVec3(sceneBounds.minimum.x, sceneBounds.maximum.y, sceneBounds.minimum.z);
	gLightPos[0] = PxVec3(size * cosf(phi0 + 0.0f * dphi), size, size * sinf(phi0 + 0.0f * dphi));
	if(moveLightsWithTarget)
		gLightPos[0] += center;
	gLightDirs[0] = gLightPos[0] - center;
	gLightDirs[0].normalize();

	gLightPos[1] = PxVec3(size * cosf(phi0 + 1.0f * dphi), size, size * sinf(phi0 + 1.0f * dphi));
	if(moveLightsWithTarget)
		gLightPos[1] += center;
	gLightDirs[1] = gLightPos[1] - center;
	gLightDirs[1].normalize();

	gLightPos[2] = PxVec3(size * cosf(phi0 + 2.0f * dphi), size, size * sinf(phi0 + 2.0f * dphi));
	if(moveLightsWithTarget)
		gLightPos[2] += center;
	gLightDirs[2] = gLightPos[2] - center;
	gLightDirs[2].normalize();
}

void RenderModel_SimpleShadows::SetupCamera()
{
	SetupProjectionMatrix();
	SetupModelViewMatrix();
}

//void RenderWireframeOverlay(PintRenderPass render_pass);
//void RenderTransparentObjects(PintRenderPass render_pass);

static void RenderScene_(/*SimpleViewerCallbacks& callbacks, */PintRenderPass render_pass, bool useShader)
{
	SPY_ZONE("RenderScene_")

	if(useShader)
	{
		if(gPhongPower!=gCurrentPhongPower)
		{
			Picture EnvMap;
			EnvMap.Init(256, 256);
			gCurrentPhongPower = gPhongPower;
			EnvMap.CreatePhongTable(gCurrentPhongPower);
			GLTexture::releaseTexture(gEnvTexId);
			gEnvTexId = GLTexture::createTexture(256, 256, EnvMap.GetPixels(), true);
			if(gEnvST)
				gEnvST->mGLID = gEnvTexId;
		}

		ShadowShaderProps texMat;
		texMat.init();
		texMat.texId = gObjectTexId;
		texMat.shadowAmbient = gShadowAmbient;
		texMat.environmentMappingCoeff = gEnvCoeff;
		texMat.envTexId = gEnvTexId;

		gDefaultShader->activate(texMat);
	}

	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();

//	callbacks.RenderScene();
	DrawScene(render_pass);

//	glPopMatrix();

//	DrawSQResults(render_pass);

	if(useShader)
		gDefaultShader->deactivate();

	DrawSQResults(render_pass);

/*	if(render_pass==PINT_RENDER_PASS_MAIN)
	{
		RenderWireframeOverlay(render_pass);

	if(useShader)
	{
		ShaderProps texMat;
		texMat.init();
		texMat.texId = gObjectTexId;
		gDefaultShader->activate(texMat);
	}

		RenderTransparentObjects(render_pass);

	if(useShader)
		gDefaultShader->deactivate();
	}
//	if(useShader)
//		gDefaultShader->deactivate();*/
}

static void RenderObjects_(/*SimpleViewerCallbacks& callbacks,*/ PintRenderPass render_pass)
{
	SPY_ZONE("RenderObjects_")

	const bool reflectedOnly = render_pass==PINT_RENDER_PASS_REFLECTIONS;

	// ground plane
	if(!reflectedOnly && gDrawGroundPlane)
	{
		{
			ShadowShaderProps mat;
			mat.init();
			mat.reflectionCoeff = gReflectionCoeff;
			mat.shadowAmbient = gShadowAmbient;
			mat.texId = gGroundTexId;

			gDefaultShader->activate(mat);
		}

		const float size = GetDefaultEnvironmentSize();
		const float y = -0.002f;

		PxVec3 center(0.0f, 0.0f, 0.0f);
		PxVec3 p0 = center + PxVec3(size, y, size);
		PxVec3 p1 = center + PxVec3(-size, y, size);
		PxVec3 p2 = center + PxVec3(-size, y, -size);
		PxVec3 p3 = center + PxVec3(size, y, -size);

		float texScale = 30.0f;
		PxVec3 t0(0.0f, 0.0f, 0.0f);
		PxVec3 t1(texScale, 0.0f, 0.0f);
		PxVec3 t2(texScale, texScale, 0.0f);
		PxVec3 t3(0.0f, texScale, 0.0f);

		glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(t0.x, t0.y); glVertex3f(p0.x, p0.y, p0.z);
		glTexCoord2f(t2.x, t2.y); glVertex3f(p2.x, p2.y, p2.z);
		glTexCoord2f(t1.x, t1.y); glVertex3f(p1.x, p1.y, p1.z);

		glTexCoord2f(t0.x, t0.y); glVertex3f(p0.x, p0.y, p0.z);
		glTexCoord2f(t3.x, t3.y); glVertex3f(p3.x, p3.y, p3.z);
		glTexCoord2f(t2.x, t2.y); glVertex3f(p2.x, p2.y, p2.z);
		glEnd();

		gDefaultShader->deactivate();
	}

	RenderScene_(/*callbacks, */render_pass, true);
}



extern bool gMirror;
extern bool gInvalidPlanes;
extern GLuint g_msaaFbo;
extern bool	gWireframePass;
//static SimpleViewerCallbacks* gCallbacks = null;
static void RenderShadowCasters_()
{
	SPY_ZONE("RenderShadowCasters_")

	gInvalidPlanes = true;
//	if(gCallbacks)
//		RenderScene_(*gCallbacks, false);
		gShadowPass = true;
		//gWireframePass = true;

		if(gFlatShader)
		{
			ShaderProps tmp;
			gFlatShader->activate(tmp);
		}

		glDisable(GL_LIGHTING);
		//glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		if(1)
		{
			RenderScene_(PINT_RENDER_PASS_SHADOW, false);
		}
		else
		{
			glDepthFunc(GL_LESS);	// We want to get the nearest pixels
			glColorMask(0,0,0,0);	// Disable color, it's useless, we only want depth.
			glDepthMask(GL_TRUE);	// Ask z writing

			RenderScene_(PINT_RENDER_PASS_SHADOW, false);

			glDepthFunc(GL_LEQUAL);	// EQUAL should work, too. (Only draw pixels if they are the closest ones)
			glColorMask(1,1,1,1);	// We want color this time
			glDepthMask(GL_FALSE);	// Writing the z component is useless now, we already have it		

			RenderScene_(PINT_RENDER_PASS_SHADOW, false);

			glDepthMask(GL_TRUE);
		}
		glEnable(GL_LIGHTING);

		if(gFlatShader)
			gFlatShader->deactivate();

		//gWireframePass = false;
		gShadowPass = false;
}


void RenderModel_SimpleShadows::Render(udword width, udword height, const Point& cam_pos, const Point& cam_dir, float fov)
{
/*	SetupCamera();	// ### already done I think
	UpdateShaderCameraParams();

	gSimpleShader->setBackLightDir((const PxVec3&)gBackLightDir);

	{
		ShaderProps texMat;
		texMat.init();
		gSimpleShader->activate(texMat);
	}

	DrawScene();

	gSimpleShader->deactivate();*/




//	callbacks.SetupCamera();
	SetupCamera();	// ### already done I think

//		SimpleViewer::UpdateCamera(false);
	UpdateCamera(false);

	const PxVec3& CamPos = (const PxVec3&)cam_pos;
	const PxVec3& CamDir = (const PxVec3&)cam_dir;

	// shadows
//	const float shadowClipNear = 0.001f;
	const float shadowClipNear = gShadowClipNear;
//	const float shadowClipFar = 20.0f;
	const float shadowClipFar = gShadowClipFar;
//	const float shadowClipFar = 200.0f;
//	const float shadowClipFar = 40.0f;

	if(gNumShadows > 0)
	{
		for(int i=0; i<gNumLights; i++)
		{
			SPY_ZONE("makeShadowMap")

			if(i<gNumShadows && gShadowMaps[i])
			{
				if(!gShadowMaps[i])
					gShadowMaps[i] = new ShadowMap(gShadowMapWidth, gShadowMapHeight, fov, i, gShadowMapRes, gNbSplits);

				gDefaultShader->setShadowMap(i, gShadowMaps[i]);

//				gCallbacks = &callbacks;
				gShadowMaps[i]->makeShadowMap(CamPos, CamDir, gLightDirs[i], shadowClipNear, shadowClipFar, &RenderShadowCasters_);
			}
		}
		if(g_msaaFbo)
			glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFbo);
	}

	const Point SunDir = -GetSunDir();
	gDefaultShader->setBackLightDir((const PxVec3&)SunDir);

	for (int i = 0; i < gNumLights; i++)
		gDefaultShader->setSpotLight(i, gLightPos[i], gLightDirs[i]);
	gDefaultShader->setNumShadows(gNumShadows);
	gDefaultShader->setShowReflection(gShowReflections);

	// reflection
	if(gShowReflections)
	{
		SPY_ZONE("Reflections")

		gInvalidPlanes = true;

		if(!gReflectedSceneTarget)
		{
			gRenderTargetWidth = width;
			gRenderTargetHeight = height;
			gReflectedSceneTarget = new PEEL::RenderTarget(width, height);
			gReflectedSceneTarget->beginCapture();
			glViewport(0, 0, width, height);
			gReflectedSceneTarget->endCapture();

			gDefaultShader->setReflectionTexId(gReflectedSceneTarget->getColorTexId());
		}
		else if(gRenderTargetWidth!=width || gRenderTargetHeight!=height)
		{
			gRenderTargetWidth = width;
			gRenderTargetHeight = height;
			gReflectedSceneTarget->resize(width, height);
		}

		gReflectedSceneTarget->beginCapture();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		GLdouble clipEq[4];
		clipEq[0] = 0.0f;
		clipEq[1] = -1.0f;
		clipEq[2] = 0.0f;
		clipEq[3] = gGroundY;
		glEnable(GL_CLIP_PLANE0);
		glClipPlane(GL_CLIP_PLANE0, clipEq);

//			callbacks.SetupCamera();
			SetupCamera();

//			SimpleViewer::UpdateCamera(true);
			UpdateCamera(true);

			RenderObjects_(/*callbacks, */PINT_RENDER_PASS_REFLECTIONS);

		glDisable(GL_CLIP_PLANE0);
		gReflectedSceneTarget->endCapture();

				if(g_msaaFbo)
					glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFbo);
	}

//		callbacks.SetupCamera();
		SetupCamera();

//		SimpleViewer::UpdateCamera(false);
		UpdateCamera(false);

		gInvalidPlanes = true;
//		callbacks.RenderHelpers();

		if(1)
		{
			RenderObjects_(/*callbacks,*/PINT_RENDER_PASS_MAIN);
		}
		else
		{
			glDepthFunc(GL_LESS);	// We want to get the nearest pixels
			glColorMask(0,0,0,0);	// Disable color, it's useless, we only want depth.
			glDepthMask(GL_TRUE);	// Ask z writing

			RenderObjects_(/*callbacks,*/PINT_RENDER_PASS_MAIN);

			glDepthFunc(GL_LEQUAL);	// EQUAL should work, too. (Only draw pixels if they are the closest ones)
			glColorMask(1,1,1,1);	// We want color this time
			glDepthMask(GL_FALSE);	// Writing the z component is useless now, we already have it		

			RenderObjects_(/*callbacks,*/PINT_RENDER_PASS_MAIN);

			glDepthMask(GL_TRUE);
		}
}

/*
void RenderModel_SimpleShadows::UpdateShaderCameraParams()
{
	float modelMatrix[16];
	float projMatrix[16];

	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);

	gSimpleShader->updateCamera(modelMatrix, projMatrix);
}
*/

void RenderModel_SimpleShadows::UpdateCamera(bool mirrored)
{
	if(mirrored)
	{
		PxTransform trans;

		PxVec3 planeN(0.0f, 1.0f, 0.0f);
		PxVec3 planeP(0.0f, gGroundY, 0.0f);
		float np2 = 2.0f*planeN.dot(planeP);

		PxVec3 r0 = PxVec3(1.0f, 0.0f, 0.0f) - 2 * planeN.x*planeN;
		PxVec3 r1 = PxVec3(0.0f, 1.0f, 0.0f) - 2 * planeN.y*planeN;
		PxVec3 r2 = PxVec3(0.0f, 0.0f, 1.0f) - 2 * planeN.z*planeN;
		PxVec3 t = np2*planeN;

		float matGL[16] = {
			r0.x, r1.x, r2.x, 0.0f,
			r0.y, r1.y, r2.y, 0.0f,
			r0.z, r1.z, r2.z, 0.0f,
			t.x, t.y, t.z, 1.0f };

		glMultMatrixf(matGL);
		glEnable(GL_CULL_FACE);	glCullFace(GL_FRONT);
	}
	else
	{
		glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);
	}

	float modelMatrix[16];
	float projMatrix[16];

	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);

	gDefaultShader->updateCamera(modelMatrix, projMatrix);
}

void RenderModel_SimpleShadows::SetupCurrentModelMatrix(const float* m)
{
	if(gDefaultShader)
		gDefaultShader->setWorldMatrix(m);
}

///////////////////////////////////////////////////////////////////////////////

#include "PintDLSphereShapeRenderer.h"
#include "PintDLCapsuleShapeRenderer.h"
#include "PintDLCylinderShapeRenderer.h"
#include "PintDLBoxShapeRenderer.h"
#include "PintDLMeshShapeRenderer.h"
#include "PintColorShapeRenderer.h"
#include "PintBatchConvexShapeRenderer.h"
#include "PintGLMeshShapeRenderer.h"
#include "Sphere.h"
#include "Capsule.h"

PintShapeRenderer* RenderModel_SimpleShadows::_CreateSphereRenderer(float radius, bool geo_sphere)
{
	if(geo_sphere)
	{
		const GeoSphereMesh SM(radius);
		return ICE_NEW(PintConvexBatchRendererCPUTransformNoNormals)(SM.mNbVerts, SM.mVerts);
	}
	else
	{
		const udword NbCirclePts = 12;
		const udword NbRotations = 12;
		const SphereMesh SM(NbCirclePts, NbRotations, radius);
		return ICE_NEW(PintConvexBatchRendererCPUTransformNoNormals)(SM.mNbVerts, SM.mVerts);
	}

	if(geo_sphere)
	{
		const GeoSphereMesh SM(radius);
		return ICE_NEW(PintBatchConvexShapeRenderer)(SM.mNbVerts, SM.mVerts);
	}
	else
	{
		const udword NbCirclePts = 12;
		const udword NbRotations = 12;
		const SphereMesh SM(NbCirclePts, NbRotations, radius);
		return ICE_NEW(PintBatchConvexShapeRenderer)(SM.mNbVerts, SM.mVerts);
	}

	return CreatePintDLSphereShapeRenderer(radius, false, geo_sphere);
}

PintShapeRenderer* RenderModel_SimpleShadows::_CreateCapsuleRenderer(float radius, float height)
{
	if(0)
	{
		const udword NbCirclePts = 16;
		const CapsuleMesh CM(NbCirclePts, radius, height*0.5f, false);
		return ICE_NEW(PintConvexBatchRendererCPUTransformNoNormals)(CM.mNbVerts, CM.mVerts);
	}

	return CreatePintDLCapsuleShapeRenderer(radius, height, false);
}

PintShapeRenderer* RenderModel_SimpleShadows::_CreateCylinderRenderer(float radius, float height)
{
	return CreatePintDLCylinderShapeRenderer(radius, height);
}

PintShapeRenderer* RenderModel_SimpleShadows::_CreateBoxRenderer(const Point& extents)
{
	AABB box;
	box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), extents);

	Point Pts[8];
	box.ComputePoints(Pts);

//	return ICE_NEW(PintBatchConvexShapeRenderer)(8, Pts);
	return ICE_NEW(PintConvexBatchRendererCPUTransformNoNormals)(8, Pts);

//	return ICE_NEW(PintDLBoxShapeRenderer)(extents, true);
}

PintShapeRenderer* RenderModel_SimpleShadows::_CreateConvexRenderer(udword nb_verts, const Point* verts)
{
//	return ICE_NEW(PintBatchConvexShapeRenderer)(nb_verts, verts);
	return ICE_NEW(PintConvexBatchRendererCPUTransformNoNormals)(nb_verts, verts);
}

static inline_ udword GetMeshFlags(bool active_edges, bool direct_data)
{
	udword Flags = 0;//DL_MESH_USE_VERTEX_NORMALS|DL_MESH_USE_SMOOTH_NORMALS;
	if(active_edges)
		Flags |= DL_MESH_USE_ACTIVE_EDGES;
	if(direct_data)
		Flags |= DL_MESH_USE_DIRECT_DATA;
	return Flags;
}

PintShapeRenderer* RenderModel_SimpleShadows::_CreateMeshRenderer(const PintSurfaceInterface& surface, const Point* normals, bool active_edges, bool direct_data)
{
	if(1 && !active_edges && !direct_data)
		return ICE_NEW(PintGLMeshShapeRenderer)(surface, 0, null);
	else
		return CreatePintDLMeshShapeRenderer(surface, GetMeshFlags(active_edges, direct_data));
}

PintShapeRenderer* RenderModel_SimpleShadows::_CreateMeshRenderer(const MultiSurface& multi_surface, bool active_edges, bool direct_data)
{
	if(1)
		return ICE_NEW(PintGLMeshShapeRendererEx)(multi_surface, GetMeshFlags(active_edges, direct_data));
	else
		return CreatePintDLMeshShapeRenderer(multi_surface, GetMeshFlags(active_edges, direct_data));
}

PintShapeRenderer* RenderModel_SimpleShadows::_CreateColorShapeRenderer(PintShapeRenderer* renderer, const RGBAColor& color, const ManagedTexture* texture)
{
	return CreatePintColorShapeRenderer(renderer, color, texture);
}

///////////////////////////////////////////////////////////////////////////////

// UI stuff

static const sdword OffsetX = 90;
static const sdword EditBoxWidth = 60;
static const sdword LabelOffsetY = 2;
static const sdword YStep = 20;
IceWindow* RenderModel_SimpleShadows::InitGUI(IceWidget* parent, Widgets* owner, PintGUIHelper& helper)
{
	sdword y = 4;

	class LocalComboBox : public IceComboBox
	{
		public:
						LocalComboBox(const ComboBoxDesc& desc) : IceComboBox(desc){}

		virtual	void	OnComboBoxEvent(ComboBoxEvent event)
		{
			if(event==CBE_SELECTION_CHANGED)
			{
				const udword Index = GetSelectedIndex();
				gNumShadows = Index;
			}
		}
	};

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Shadows:", owner);
	IceComboBox* CB = CreateComboBox<LocalComboBox>(parent, 0, 94, y, 150, 20, "Shadows", owner, null/*gTooltip_*/);
	CB->Add("No shadows");
	CB->Add("1 shadow");
	CB->Add("2 shadows");
	CB->Add("3 shadows");
	CB->Select(3);

	y += YStep;

	{
		struct Local{ static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data) { gShowReflections = checked;	}};
		helper.CreateCheckBox(parent, 0, 4, y, 200, 20, "Reflections", owner, gShowReflections, Local::gCheckBoxCallback, null);
	}
	y += YStep;

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Reflection coeff:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gReflectionCoeff = GetFloat(gReflectionCoeff, &edit_box);
		}};
		helper.CreateEditBox(parent, 0, 4+10+OffsetX, y, EditBoxWidth, 20, helper.Convert(gReflectionCoeff), owner, EDITBOX_FLOAT, Local::gEBCallback, null);
//		y += YStep;
	}

	helper.CreateLabel(parent, 180+4, y+LabelOffsetY, 90, 20, "Env coeff:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gEnvCoeff = GetFloat(gEnvCoeff, &edit_box);
		}};
		helper.CreateEditBox(parent, 0, 180+4+10+OffsetX, y, EditBoxWidth, 20, helper.Convert(gEnvCoeff), owner, EDITBOX_FLOAT, Local::gEBCallback, null);
		y += YStep;
	}

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Shadow clip near:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gShadowClipNear = GetFloat(gShadowClipNear, &edit_box);
		}};
		helper.CreateEditBox(parent, 0, 4+10+OffsetX, y, EditBoxWidth, 20, helper.Convert(gShadowClipNear), owner, EDITBOX_FLOAT, Local::gEBCallback, null);
//		y += YStep;
	}

	helper.CreateLabel(parent, 180+4, y+LabelOffsetY, 90, 20, "Shadow ambient:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gShadowAmbient = GetFloat(gShadowAmbient, &edit_box);
		}};
		helper.CreateEditBox(parent, 0, 180+4+10+OffsetX, y, EditBoxWidth, 20, helper.Convert(gShadowAmbient), owner, EDITBOX_FLOAT, Local::gEBCallback, null);
		y += YStep;
	}

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Shadow clip far:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gShadowClipFar = GetFloat(gShadowClipFar, &edit_box);
		}};
		helper.CreateEditBox(parent, 0, 4+10+OffsetX, y, EditBoxWidth, 20, helper.Convert(gShadowClipFar), owner, EDITBOX_FLOAT, Local::gEBCallback, null);
//		y += YStep;
	}

	helper.CreateLabel(parent, 180+4, y+LabelOffsetY, 90, 20, "Phong power:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gPhongPower = GetFloat(gPhongPower, &edit_box);
		}};
		helper.CreateEditBox(parent, 0, 180+4+10+OffsetX, y, EditBoxWidth, 20, helper.Convert(gPhongPower), owner, EDITBOX_FLOAT, Local::gEBCallback, null);
		y += YStep;
	}

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Nb splits:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gNbSplits = GetInt(gNbSplits, &edit_box);
			RenderModel_SimpleShadows* RM = (RenderModel_SimpleShadows*)edit_box.GetUserData();
			RM->Close();
			RM->Init();
		}};
		IceEditBox* EB = helper.CreateEditBox(parent, 0, 4+10+OffsetX, y, EditBoxWidth, 20, _F("%d", gNbSplits), owner, EDITBOX_INTEGER_POSITIVE, Local::gEBCallback, null);
		EB->SetUserData(this);
		y += YStep;
	}

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Resolution:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gShadowMapRes = GetInt(gShadowMapRes, &edit_box);
			RenderModel_SimpleShadows* RM = (RenderModel_SimpleShadows*)edit_box.GetUserData();
			RM->Close();
			RM->Init();
		}};
		IceEditBox* EB = helper.CreateEditBox(parent, 0, 4+10+OffsetX, y, EditBoxWidth, 20, _F("%d", gShadowMapRes), owner, EDITBOX_INTEGER_POSITIVE, Local::gEBCallback, null);
		EB->SetUserData(this);
		y += YStep;
	}

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Map width:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gShadowMapWidth = GetInt(gShadowMapWidth, &edit_box);
			RenderModel_SimpleShadows* RM = (RenderModel_SimpleShadows*)edit_box.GetUserData();
			RM->Close();
			RM->Init();
		}};
		IceEditBox* EB = helper.CreateEditBox(parent, 0, 4+10+OffsetX, y, EditBoxWidth, 20, _F("%d", gShadowMapWidth), owner, EDITBOX_INTEGER_POSITIVE, Local::gEBCallback, null);
		EB->SetUserData(this);
		y += YStep;
	}

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Map height:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gShadowMapHeight = GetInt(gShadowMapHeight, &edit_box);
			RenderModel_SimpleShadows* RM = (RenderModel_SimpleShadows*)edit_box.GetUserData();
			RM->Close();
			RM->Init();
		}};
		IceEditBox* EB = helper.CreateEditBox(parent, 0, 4+10+OffsetX, y, EditBoxWidth, 20, _F("%d", gShadowMapHeight), owner, EDITBOX_INTEGER_POSITIVE, Local::gEBCallback, null);
		EB->SetUserData(this);
		y += YStep;
	}

	helper.CreateLabel(parent, 4, y+LabelOffsetY, 90, 20, "Lit area size:", owner);
	{
		struct Local{ static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			gLitAreaSize = GetFloat(gLitAreaSize, &edit_box);
		}};
		helper.CreateEditBox(parent, 0, 4+10+OffsetX, y, EditBoxWidth, 20, helper.Convert(gLitAreaSize), owner, EDITBOX_FLOAT, Local::gEBCallback, null);
		y += YStep;
	}

	return null;
}

