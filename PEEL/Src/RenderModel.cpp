///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RenderModel.h"

extern RenderModel* gCurrentRenderModel;
void SetGlobalPoseRenderModel(const float* m)
{
	if(gCurrentRenderModel)
		gCurrentRenderModel->SetupCurrentModelMatrix(m);
}

#include "RenderModelFFP.h"
#include "RenderModelMatcap.h"
#include "RenderModelSimpleShader.h"
#include "RenderModelSimpleShadows.h"

void CreateRenderModels(RenderModel** render_models)
{
	render_models[RENDER_MODEL_FFP]				= ICE_NEW(RenderModel_FFP);
	render_models[RENDER_MODEL_SIMPLE_SHADER_1]	= ICE_NEW(RenderModel_SimpleShaderType1);
	render_models[RENDER_MODEL_SIMPLE_SHADER_2]	= ICE_NEW(RenderModel_SimpleShaderType2);
	render_models[RENDER_MODEL_MATCAP]			= ICE_NEW(RenderModel_Matcap);
	render_models[RENDER_MODEL_SIMPLE_SHADOWS]	= ICE_NEW(RenderModel_SimpleShadows);
}

