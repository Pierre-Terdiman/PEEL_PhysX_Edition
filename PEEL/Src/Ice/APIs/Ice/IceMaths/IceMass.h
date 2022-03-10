///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This file contains code to compute the mass of various primitives.
 *	\file		IceMass.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMASS_H
#define ICEMASS_H

	FUNCTION ICEMATHS_API	float ComputeSphereMass(float radius, float density);
	FUNCTION ICEMATHS_API	float ComputeSphereDensity(float radius, float mass);
	FUNCTION ICEMATHS_API	float ComputeBoxMass(const Point& extents, float density);
	FUNCTION ICEMATHS_API	float ComputeBoxDensity(const Point& extents, float mass);
	FUNCTION ICEMATHS_API	float ComputeEllipsoidMass(const Point& extents, float density);
	FUNCTION ICEMATHS_API	float ComputeEllipsoidDensity(const Point& extents, float mass);
	FUNCTION ICEMATHS_API	float ComputeCylinderMass(float r, float l, float density);
	FUNCTION ICEMATHS_API	float ComputeCylinderDensity(float r, float l, float mass);
	FUNCTION ICEMATHS_API	float ComputeConeMass(float r, float l, float density);
	FUNCTION ICEMATHS_API	float ComputeConeDensity(float r, float l, float mass);

#endif	// ICEMASS_H

