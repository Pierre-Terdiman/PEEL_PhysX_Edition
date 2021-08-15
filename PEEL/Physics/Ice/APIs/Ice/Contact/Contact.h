///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for Contact library.
 *	\file		Contact.h
 *	\author		Pierre Terdiman
 *	\date		January, 14, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CONTACT_H
#define CONTACT_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation messages
#if defined(CONTACT_EXPORTS)
	#pragma message("----Compiling CONTACT")
#elif !defined(CONTACT_EXPORTS)
	#pragma message("----Using CONTACT")
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Automatic linking
	#ifndef BAN_CONTACT_AUTOLINK
		#ifdef _WIN64
			#ifdef _DEBUG
				#pragma comment(lib, "Contact64_D.lib")
			#else
				#pragma comment(lib, "Contact64.lib")
			#endif
		#else
			#ifdef _DEBUG
				#pragma comment(lib, "Contact_D.lib")
			#else
				#pragma comment(lib, "Contact.lib")
			#endif
		#endif
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor

#ifndef ICE_NO_DLL
	#ifdef CONTACT_EXPORTS
		#define CONTACT_API		__declspec(dllexport)
	#else
		#define CONTACT_API		__declspec(dllimport)
	#endif
#else
		#define CONTACT_API
#endif

	#include "ContactSettings.h"

	namespace Ctc
	{
		// Distances

		#include "CTC_RayAABBDistance.h"
		#include "CTC_PointAABBDistance.h"
		#include "CTC_PointOBBDistance.h"
		#include "CTC_OBBOBBDistance.h"
		#include "CTC_CircleCircleDistance.h"
		#include "CTC_EdgeEdgeDistance.h"
		#include "CTC_SegmentSegmentDistance.h"
		#include "CTC_PointRectangleDistance.h"
		#include "CTC_SegmentRectangleDistance.h"
		#include "CTC_SegmentOBBDistance.h"
		#include "CTC_TriangleTriangleDistance.h"
		#include "CTC_PointTriangleDistance.h"
		#include "CTC_SegmentTriangleDistance.h"
		#include "CTC_TriangleRectangleDistance.h"
		#include "CTC_RectangleRectangleDistance.h"
		#include "CTC_QuadQuadDistance.h"

		// Intersections

		#include "CTC_CapsuleOBBOverlap.h"
		#include "CTC_PointTriangleOverlap.h"
		#include "CTC_PointPolygonOverlap.h"
		#include "CTC_RayAABBOverlap.h"
		#include "CTC_SphereSphereOverlap.h"
		#include "CTC_SphereAABBOverlap.h"
		#include "CTC_SpherePlaneOverlap.h"
		#include "CTC_PointSphereOverlap.h"
		#include "CTC_SphereOBBOverlap.h"
		#include "CTC_DiskPlaneOverlap.h"
		#include "CTC_RayPlaneOverlap.h"
		#include "CTC_RayTriangleOverlap.h"
		#include "CTC_RayPolygonOverlap.h"
		#include "CTC_TriangleTriangleOverlap.h"
		#include "CTC_RaySphereOverlap.h"
		#include "CTC_RayCapsuleOverlap.h"
		#include "CTC_TriangleAABBOverlap.h"
		#include "CTC_PointOBBOverlap.h"
		#include "CTC_PlaneAABBOverlap.h"
		#include "CTC_OBBOBBOverlap.h"
		#include "CTC_SphereConeOverlap.h"
		#include "CTC_TriangleCylinderOverlap.h"
		#include "CTC_TriangleSphereOverlap.h"

		// Contacts

		#include "CTC_Contact.h"
		#include "CTC_PlanePlane.h"
		#include "CTC_PlanePlanePlane.h"
		#include "CTC_Continuous.h"
		#include "CTC_DistanceSolver.h"
		#include "CTC_EdgeEdgeContact.h"
		#include "CTC_PolygonPolygonContact.h"

		// Sweep tests

		#include "CTC_Extrude.h"
		#include "CTC_SweptEllipsoid.h"
		#include "CTC_SweptBox.h"
		#include "CTC_SweepSphereSphere.h"
		#include "CTC_SweepBoxBox.h"
		#include "CTC_SweepBoxTriangle.h"
		#include "CTC_SweepBoxSphere.h"
		#include "CTC_SweepSphereCapsule.h"
		#include "CTC_SweepCapsuleCapsule.h"
		#include "CTC_SweepCapsuleTriangles.h"
		#include "CTC_SweepTriSphere.h"
	}

#endif // CONTACT_H
