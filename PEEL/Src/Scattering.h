///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SCATTERING_H
#define SCATTERING_H

	extern float	gSunPhi;
	extern float	gSunTheta;
	extern float	gSunIntensity;
	extern Sun		gSun;
	extern bool		gRenderSunEnabled;
	extern float	gRayleighMultiplier;
	extern float	gMieMultiplier;
	extern float	gScatteringMultiplier;
	extern float	gExtinctionMultiplier;
	extern float	gHGg;

	void	UpdateSun();
	Point	GetSunDir();
	HPoint	GetSunColorAndIntensity();

	void	InitAtmosphere();
	void	SetupAtmosphereShaderParams(GLuint shader_program);

#endif
