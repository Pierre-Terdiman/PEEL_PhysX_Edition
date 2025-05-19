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
#if !PEEL_PUBLIC_BUILD
	#include "RenderModelOutdoor.h"
	#include "RenderModelExperiment.h"
	#include "RenderModelWFC.h"
	#include "RenderModelLearn.h"
#endif

void CreateRenderModels(RenderModel** render_models)
{
#if !PEEL_PUBLIC_BUILD
	render_models[RENDER_MODEL_LEARN]			= ICE_NEW(RenderModel_Learn);
	render_models[RENDER_MODEL_OUTDOOR]			= ICE_NEW(RenderModel_Outdoor);
	render_models[RENDER_MODEL_EXPERIMENT]		= ICE_NEW(RenderModel_Experiment);
	render_models[RENDER_MODEL_WFC]				= ICE_NEW(RenderModel_WFC);
#endif
	render_models[RENDER_MODEL_FFP]				= ICE_NEW(RenderModel_FFP);
	render_models[RENDER_MODEL_SIMPLE_SHADER_1]	= ICE_NEW(RenderModel_SimpleShaderType1);
	render_models[RENDER_MODEL_SIMPLE_SHADER_2]	= ICE_NEW(RenderModel_SimpleShaderType2);
	render_models[RENDER_MODEL_MATCAP]			= ICE_NEW(RenderModel_Matcap);
	render_models[RENDER_MODEL_SIMPLE_SHADOWS]	= ICE_NEW(RenderModel_SimpleShadows);
}

