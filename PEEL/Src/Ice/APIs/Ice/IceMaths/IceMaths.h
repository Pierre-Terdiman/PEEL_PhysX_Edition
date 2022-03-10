///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for IceMaths.dll.
 *	\file		IceMaths.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMATHS_H
#define ICEMATHS_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation messages
#if defined(ICEMATHS_EXPORTS)
	#pragma message("----Compiling ICE Maths")
#elif !defined(ICEMATHS_EXPORTS)
	#pragma message("----Using ICE Maths")
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Automatic linking
	#ifndef BAN_ICEMATHS_AUTOLINK
		#ifdef _WIN64
			#ifdef _DEBUG
				#pragma comment(lib, "IceMaths64_D.lib")
			#else
				#pragma comment(lib, "IceMaths64.lib")
			#endif
		#else
			#ifdef _DEBUG
				#pragma comment(lib, "IceMaths_D.lib")
			#else
				#pragma comment(lib, "IceMaths.lib")
			#endif
		#endif
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor

#ifndef ICE_NO_DLL
	#ifdef ICEMATHS_EXPORTS
		#define ICEMATHS_API	__declspec(dllexport)
	#else
		#define ICEMATHS_API	__declspec(dllimport)
	#endif
#else
		#define ICEMATHS_API
#endif

	// Includes
	#include <Math.h>

	#include "IceMathsSettings.h"

	#ifdef COMPILER_VISUAL_CPP
		#pragma intrinsic(memcmp)
		#pragma intrinsic(memcpy)
		#pragma intrinsic(memset)
		#pragma intrinsic(strcat)
		#pragma intrinsic(strcmp)
		#pragma intrinsic(strcpy)
		#pragma intrinsic(strlen)
		#pragma intrinsic(abs)
		#pragma intrinsic(labs)

		#pragma intrinsic(fabs)
		#pragma intrinsic(asin)
		#pragma intrinsic(acos)
		#pragma intrinsic(atan)
		#pragma intrinsic(sinh)
		#pragma intrinsic(cosh)
		#pragma intrinsic(tanh)
		#pragma intrinsic(fmod)
		#pragma intrinsic(pow)
		#pragma intrinsic(exp)
		#pragma intrinsic(sqrt)
		#pragma intrinsic(log)
		#pragma intrinsic(log10)
		#pragma intrinsic(sin)
		#pragma intrinsic(cos)
		#pragma intrinsic(tan)
	#endif

	namespace IceMaths
	{
		// enum for identity constructor flag
		enum IDENTITY
		{
			Idt
		};

ICEMATHS_API void* CreateClass(const char* name);

		// Numerics
		#include "IceApprox.h"
		#include "IceCubeRoot.h"
		#include "IceRoots.h"
		#include "IcePolynomial.h"

		// Basic
		#include "IceAxes.h"
		#include "IceEulerAngles.h"
		#include "IcePoint.h"
		#include "IceHPoint.h"
		#include "IceMatrix3x3.h"
		#include "IceMatrix4x4.h"
		#include "IceSpecialMatrices.h"
		#include "IceMatrixNxN.h"
		#include "IceQuat.h"
		#include "IceAngleAxis.h"
		#include "IcePRS.h"
		#include "IceBasis.h"

		//! A function to access a global 3x3 identity matrix
		FUNCTION ICEMATHS_API const Matrix3x3& Get3x3IdentityMatrix();

		//! A function to access a global 4x4 identity matrix
		FUNCTION ICEMATHS_API const Matrix4x4& Get4x4IdentityMatrix();

		//
		#include "IceCubeIndex.h"
		#include "IceMoment.h"
		#include "IcePlane.h"
		#include "IceTriangle.h"
		#include "IceIndexedTriangle.h"
		#include "IceQuad.h"
		#include "IcePolygon.h"

		// Shapes
		#include "IceCircle.h"
		#include "IceCylinder.h"
		#include "IceRay.h"
		#include "IceSegment.h"
		#include "IceRectangle3.h"
		#include "IceAABB.h"
		#include "IceMiniball.h"
		#include "IceBoundingSphere.h"
		#include "IceOBB.h"
		#include "IceLSS.h"
		#include "IceRSS.h"
		#include "IceTSS.h"

		// Containers
		#include "IceVertices.h"
		#include "IceMatrixPalette.h"
		#include "IceTriangleContainers.h"
		#include "IcePlanes.h"
		#include "IceVisualDebugData.h"

		// Advanced
		#include "IceGaussMap.h"
		#include "IcePRNode.h"
		#include "IceEigen.h"
		#include "IceProgressiveEigen.h"
		#include "IceFFT.h"
		#include "IceVolumeIntegration.h"
		#include "IceMass.h"
		#include "IceInertiaTensor.h"
		#include "IceCatmullRom.h"
		#include "IceSpline.h"
		#include "IceZCurve.h"
		#include "IceIK.h"
		#include "IceFilters.h"
		#include "IceJacobi.h"
		#include "IceScrewing.h"
		#include "IceInterval.h"
		#include "IceInclusion.h"
		#include "IceCollisionResponse.h"
		#include "IceKeyframe.h"
		#include "IceAxisSortHelper.h"

		// Noise
		#include "IceWhiteGaussianNoise.h"
		#include "IcePerlinNoise.h"
		#include "IceFractalBrownianMotion.h"
		#include "IceRidgedFractal.h"
		#include "IceWorleyNoise.h"
		#include "IcePharrNoise.h"
		#include "IceSkinnerNoise.h"

		#include "IceMathsSerialize.h"

		//! remap (0->1) to (0->1) but with derivative continuity
		inline_ float HermiteLerpParameter(const float t)
		{
//			gAssert( t >= - EPSILON && t <= 1.f + EPSILON );
			return (3.0f - 2.0f*t)*t*t;
		}

		//! even better continuity than Hermite lerp, but slower
		inline_ float CosLerpParameter(const float t)
		{
//			gAssert( t >= - EPSILON && t <= 1.f + EPSILON );
			return 0.5f - 0.5f * cosf(t*PI);
		}

		// Returns angle between 0 and PI in radians
		inline_ float SafeACos(float f)
		{
			// Take care of FPU inaccuracies
			if(f>1.0f)	return 0.0f;
			if(f<-1.0f)	return PI;
						return acosf(f);
		}

		// Returns angle between -PI/2 and PI/2 in radians
		inline_ float SafeASin(float f)
		{
			// Take care of FPU inaccuracies
			if(f>1.0f)	return HALFPI;
			if(f<-1.0f)	return -HALFPI;
						return asinf(f);
		}

		// Returns angle between -PI/2 and PI/2 in radians
		inline_ float SafeATan(float f)
		{
			return atanf(f);
		}

		// Returns angle between -PI and PI in radians
		inline_ float SafeATan(float sin, float cos)
		{
			return atan2f(sin, cos);
		}

		// Returns the angle between two (possibly un-normalized) vectors
		inline_ float Angle(const Point& v0, const Point& v1)
		{
			float Cos = v0|v1;					// |v0|*|v1|*Cos(Angle)
			float Sin = (v0^v1).Magnitude();	// |v0|*|v1|*Sin(Angle)
			return SafeATan(Sin, Cos);
		}

		#include "IceMathsAPI.h"

		class ICEMATHS_API DynamicPR
		{
			public:
			inline_	DynamicPR()		{}
			inline_	DynamicPR(const PR& pr, const Point& v, const Point& w)	: mInitPose(pr), mLinearVelocity(v), mAngularVelocity(w)	{}
			inline_	~DynamicPR()	{}

			PR		mInitPose;			//!< Initial pose at start of time interval
			Point	mLinearVelocity;	//!< Linear velocity over the interval
			Point	mAngularVelocity;	//!< Angular velocity over the interval
		};

		// Temp helpers
		ICEMATHS_API void Integrate(PR& dst, const PR& src, const Point& v, const Point& w, float time_step=1.0f);
		ICEMATHS_API void Desintegrate(Point& v, Point& w, const PR& dst, const PR& src, float time_step=1.0f);

		inline_ void Integrate(PR& dst, const DynamicPR& pose, float time_step=1.0f)
		{
			Integrate(dst, pose.mInitPose, pose.mLinearVelocity, pose.mAngularVelocity, time_step);
		}
	}

#endif // ICEMATHS_H
