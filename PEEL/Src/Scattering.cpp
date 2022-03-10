///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Scattering.h"

float	gSunPhi = 1.57080f;
float	gSunTheta = 1.25664f;
float	gSunIntensity = 100.0f;
Sun		gSun(gSunTheta, gSunPhi, gSunIntensity);
bool	gRenderSunEnabled = false;
float	gRayleighMultiplier = 0.2f;
float	gMieMultiplier = 0.04f;
float	gScatteringMultiplier = 0.3f;
float	gExtinctionMultiplier = 1.0f;
float	gHGg = 0.67f;

Point GetSunDir()
{
	Point Dir = gSun.GetDirection();
	Dir.Normalize();
	return Dir;
}

HPoint GetSunColorAndIntensity()
{
	return gSun.GetColorAndIntensity();
}

void UpdateSun()
{
	//gSun.SetPhi(fmodf(gSunPhi, TWOPI));
	gSun.SetPhi(gSunPhi);
	//gSun.SetTheta(fmodf(gSunTheta, TWOPI));
	gSun.SetTheta(gSunTheta);
	gSun.SetIntensity(gSunIntensity);
}

static Atmosphere gAtmosphere;

void InitAtmosphere()
{
	gAtmosphere.SetParam(eAtmBetaRayMultiplier, 0.2f);
	gAtmosphere.SetParam(eAtmBetaMieMultiplier, 0.04f);
	gAtmosphere.SetParam(eAtmInscatteringMultiplier, 0.3f);
	gAtmosphere.SetParam(eAtmExtinctionMultiplier, 1.0f);
	gAtmosphere.SetParam(eAtmHGg, 0.67f);
}

#include "GLShader.h"
void SetupAtmosphereShaderParams(GLuint shader_program)
{
	gAtmosphere.SetParam(eAtmBetaRayMultiplier, gRayleighMultiplier);
	gAtmosphere.SetParam(eAtmBetaMieMultiplier, gMieMultiplier);
	gAtmosphere.SetParam(eAtmInscatteringMultiplier, gScatteringMultiplier);
	gAtmosphere.SetParam(eAtmExtinctionMultiplier, gExtinctionMultiplier);
	gAtmosphere.SetParam(eAtmHGg, gHGg);

	HPoint vSunColorIntensity = GetSunColorAndIntensity();
	//printf("vSunColorIntensity: %f %f %f %f\n", vSunColorIntensity.x, vSunColorIntensity.y, vSunColorIntensity.z, vSunColorIntensity.w);
	GLShader::SetUniform3f(shader_program, "CV_SUN_COLOR", vSunColorIntensity.x, vSunColorIntensity.y, vSunColorIntensity.z);
	GLShader::SetUniform1f(shader_program, "sunIntensity", vSunColorIntensity.w);

	float fReflectance = 0.1f;
	Point vDiffuse(0.138f,0.113f, 0.08f); // Taken from soil's relectance spectrum data.
	vDiffuse *= fReflectance; 
	GLShader::SetUniform3f(shader_program, "CV_TERRAIN_REFLECTANCE", vDiffuse.x, vDiffuse.y, vDiffuse.z);

	// Scattering multipliers.
	float fRayMult = gAtmosphere.GetParam(eAtmBetaRayMultiplier);
	float fMieMult = gAtmosphere.GetParam(eAtmBetaMieMultiplier);

	HPoint vBetaR, vBetaDashR, vBetaM, vBetaDashM, vBetaRM, vOneOverBetaRM;

	// Rayleigh
	Point tmp = gAtmosphere.GetBetaRayleigh();
	vBetaR.x = tmp.x;
	vBetaR.y = tmp.y;
	vBetaR.z = tmp.z;
	vBetaR.w = 0.0f;
	vBetaR *= fRayMult;
	//rnd->SetVertexShaderConstantF( CV_BETA_1, (const float*)&vBetaR, 1 );

	tmp = gAtmosphere.GetBetaDashRayleigh();
	vBetaDashR.x = tmp.x;
	vBetaDashR.y = tmp.y;
	vBetaDashR.z = tmp.z;
	vBetaDashR.w = 0.0f;
	vBetaDashR *= fRayMult;
//	rnd->SetVertexShaderConstantF( CV_BETA_DASH_1, (const float*)&vBetaDashR, 1 );
	GLShader::SetUniform3f(shader_program, "CV_BETA_DASH_1", vBetaDashR.x, vBetaDashR.y, vBetaDashR.z);

	// Mie
	tmp = gAtmosphere.GetBetaMie();
	vBetaM.x = tmp.x;
	vBetaM.y = tmp.y;
	vBetaM.z = tmp.z;
	vBetaM.w = 0.0f;
	vBetaM *= fMieMult;
	//rnd->SetVertexShaderConstantF( CV_BETA_2, (const float*)&vBetaM, 1 );

	tmp = gAtmosphere.GetBetaDashMie();
	vBetaDashM.x = tmp.x;
	vBetaDashM.y = tmp.y;
	vBetaDashM.z = tmp.z;
	vBetaDashM.w = 0.0f;
	vBetaDashM *= fMieMult;
//	rnd->SetVertexShaderConstantF( CV_BETA_DASH_2, (const float*)&vBetaDashM, 1 );
	GLShader::SetUniform3f(shader_program, "CV_BETA_DASH_2", vBetaDashM.x, vBetaDashM.y, vBetaDashM.z);

	// Rayleigh + Mie (optimization)
	vBetaRM = vBetaR + vBetaM;
	//rnd->SetVertexShaderConstantF(CV_BETA_1_PLUS_2, (const float*)&vBetaRM, 1);
	GLShader::SetUniform3f(shader_program, "CV_BETA_1_PLUS_2", vBetaRM.x, vBetaRM.y, vBetaRM.z);

	vOneOverBetaRM[0] = 1.0f/vBetaRM[0];
	vOneOverBetaRM[1] = 1.0f/vBetaRM[1];
	vOneOverBetaRM[2] = 1.0f/vBetaRM[2];
	vOneOverBetaRM[3] = 0.0f;
	//rnd->SetVertexShaderConstantF(CV_ONE_OVER_BETA_1_PLUS_2, (const float*)&vOneOverBetaRM, 1);
	GLShader::SetUniform3f(shader_program, "CV_ONE_OVER_BETA_1_PLUS_2", vOneOverBetaRM.x, vOneOverBetaRM.y, vOneOverBetaRM.z);

	// Henyey Greenstein's G value.
	const float g = gAtmosphere.GetParam(eAtmHGg);
//	HPoint vG(1-g*g, 1+g, 2*g, 0.0f);
//	HPoint vG(1+g*g, 1+g, 2*g, 0.0f);
	HPoint vG((1.0f-g)*(1.0f-g), 1.0f + g*g, 2.0f*g, 0.0f);
//	rnd->SetVertexShaderConstantF( CV_HG, (const float*)&vG, 1 );
	GLShader::SetUniform3f(shader_program, "CV_HG", vG.x, vG.y, vG.z);

	// constants.
	const float l2e = 1.0f/logf(2.0f);	// 1.4426951
	GLShader::SetUniform3f(shader_program, "CV_CONSTANTS", 1.0f, l2e, 0.5f);

	// each term (extinction, inscattering multiplier)
	const float fExt = gAtmosphere.GetParam(eAtmExtinctionMultiplier);
	const float fIns = gAtmosphere.GetParam(eAtmInscatteringMultiplier);
	GLShader::SetUniform3f(shader_program, "CV_TERM_MULTIPLIERS", fExt, fIns, 0.0f);
}

