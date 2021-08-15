///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for Meshmerizer.dll.
 *	\file		Meshmerizer.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef MESHMERIZER_H
#define MESHMERIZER_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation messages
#if defined(MESHMERIZER_EXPORTS)
	#pragma message("----Compiling ICE Meshmerizer")
#elif !defined(MESHMERIZER_EXPORTS)
	#pragma message("----Using ICE Meshmerizer")
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Automatic linking
	#ifndef BAN_MESHMERIZER_AUTOLINK
		#ifdef _WIN64
			#ifdef _DEBUG
				#pragma comment(lib, "Meshmerizer64_D.lib")
				#pragma comment(lib, "Contact64_D.lib")
			#else
				#pragma comment(lib, "Meshmerizer64.lib")
				#pragma comment(lib, "Contact64.lib")
			#endif
		#else
			#ifdef _DEBUG
				#pragma comment(lib, "Meshmerizer_D.lib")
				#pragma comment(lib, "Contact_D.lib")
			#else
				#pragma comment(lib, "Meshmerizer.lib")
				#pragma comment(lib, "Contact.lib")
			#endif
		#endif
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor
#ifndef ICE_NO_DLL
	#ifdef MESHMERIZER_EXPORTS
		#define MESHMERIZER_API __declspec(dllexport)
	#else
		#define MESHMERIZER_API __declspec(dllimport)
	#endif
#else
		#define MESHMERIZER_API
#endif

	#include "MeshmerizerSettings.h"

	namespace Meshmerizer
	{
		#include "MeshmerizerErrors.h"

		// Default surfaces
		#include "IceObjectInterface.h"

//		#include "IceBunny.h"
//		#include "IceCassiniEgg.h"

		// Bounding volumes
		#include "IceEdgeList.h"

		#include "IceConvexHull.h"
		#include "IceConvexHull2.h"
		#include "IceHullFromPlanes.h"
		#include "IceHillClimbing.h"
		#include "IceCollisionHull.h"
		#include "IceCullingHull.h"
		#include "IceFrustum.h"
		#include "IceEigenAnalysis.h"

		// Data structures
		#include "IceSurfaceInterface.h"
		#include "IceVertexCloud.h"
		#include "IcePointShell.h"
		#include "IceStabbing.h"
		#include "IceActiveEdges.h"
		#include "IceAdjacencies.h"
		#include "IceValency.h"
		#include "IceTriSurface.h"
		#include "IceIndexedSurface.h"
		#include "IceMultiSurface.h"
		#include "IceVertexToFaces.h"
		#include "IceBSP.h"
		#include "IceShape.h"

		#include "IceRoom.h"

		// Mesh consolidation
		#include "IceUVCreator.h"
		#include "IceNormals.h"
		#include "IceListOptimizer.h"
		#include "IceStriper.h"
		#include "IceMeshBuilderResults.h"
		#include "IceMeshBuilder2.h"
		#include "IceQuantize.h"
		#include "IceManifoldMesh.h"
		#include "IceConvexDecomposer.h"
		#include "IceRemoveTJunctions.h"

		// Intersections
		#include "IceBoxPruning.h"
		#include "IceSeparatingAxes.h"

		// Techniques
		#include "IceLODRunner.h"
		#include "IceLODRunner2.h"
		#include "IceSubdivisionHistory.h"
		#include "IceSubdivision.h"
		#include "IceLoop.h"
		#include "IceButterfly.h"
		#include "IceMeshPacker.h"
		#include "IceMeshPacker2.h"
		#include "IceGridButterfly.h"
		#include "IceDualSilhouette.h"
		#include "IceSilhouette.h"
		#include "IceNormalMask.h"
		#include "IceDecal.h"
		#include "IceProgressiveEigenBoxes.h"
		#include "IceSurfaceClipping.h"
		#include "IceHullGaussMaps.h"

		#include "IceInnerBoxes.h"

		#include "IceConvexData.h"

		#include "MeshmerizerAPI.h"
	}

#endif // MESHMERIZER_H
