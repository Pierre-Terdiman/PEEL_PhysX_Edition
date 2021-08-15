///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for special matrices.
 *	\file		IceSpecialMatrix.h
 *	\author		Pierre Terdiman
 *	\date		January, 30, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESPECIALMATRICES_H
#define ICESPECIALMATRICES_H

	// Computes a world matrix.
	ICEMATHS_API void ComputeWorldMatrix(Matrix4x4& mat, const PRS& prs);
	ICEMATHS_API void ComputeWorldMatrix(Matrix4x4& mat, const PR& pr);

	// Computes a shadow matrix
	ICEMATHS_API void ComputeShadowMatrix(Matrix4x4& mat, const Point& light, const Point& p0, const Point& p1, const Point& p2);

	// Computes a sphere map matrix
	ICEMATHS_API void ComputeSphereMapMatrix(Matrix4x4& mat, float scale=0.5f);

	// Computes a self-shadowing matrix
	ICEMATHS_API void ComputeSelfShadowMatrix(Matrix4x4& mat, const Point& light);

	// Computes a rotozoom matrix
	ICEMATHS_API void ComputeRotozoomMatrix(Matrix4x4& mat, float angle, float zoom, float posx, float posy);

	// Computes a reflection matrix
	ICEMATHS_API void ComputeReflectionMatrix(Matrix4x4& mat, const Plane& plane);

#endif // ICESPECIALMATRICES_H

