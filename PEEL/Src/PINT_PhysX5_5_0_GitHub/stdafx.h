///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\PINT_Common\PINT_Ice.h"

#include <vector>

#ifndef UINT32_MAX
	#define UINT32_MAX MAX_UDWORD
#endif
#ifndef UINT8_MAX
	#define UINT8_MAX MAX_UBYTE
#endif

#define PX_ENABLE_FEATURES_UNDER_CONSTRUCTION	0

#include "PxPhysicsAPI.h"
#include "foundation/PxFoundation.h"
#include "foundation/PxUtilities.h"

using namespace physx;

// General
#define PHYSX_SUPPORT_SCRATCH_BUFFER						1
#define PHYSX_SUPPORT_CONTACT_NOTIFICATIONS					1
#define PHYSX_SUPPORT_CONTACT_MODIFICATIONS					1
#define PHYSX_SUPPORT_PX_BROADPHASE_TYPE					1
#define PHYSX_SUPPORT_PX_BROADPHASE_ABP						1
#define PHYSX_SUPPORT_PX_BROADPHASE_PABP					1
#define PHYSX_SUPPORT_CPU_DISPATCHER_MODE					1
#define PHYSX_SUPPORT_CUDA_GL_INTEROP						0
// Actors/Rigid bodies
#define PHYSX_SUPPORT_ADAPTIVE_FORCE						0
#define PHYSX_SUPPORT_MAX_DEPEN_VELOCITY					1
#define PHYSX_SUPPORT_TGS									1
#define PHYSX_SUPPORT_SUBSTEPS								1
#define PHYSX_SUPPORT_POINT_FRICTION						1
#define PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION				1
#define	PHYSX_SUPPORT_FRICTION_EVERY_ITERATION				1
#define	PHYSX_SUPPORT_EXTERNAL_FORCES_EVERY_ITERATION		1
#define PHYSX_SUPPORT_STABILIZATION_FLAG					1
#define	PHYSX_SUPPORT_KINE_FILTERING_MODE					1
#define	PHYSX_SUPPORT_GYROSCOPIC_FORCES						1
#define PHYSX_SUPPORT_SOLVER_RESIDUALS						0
#define PHYSX_SUPPORT_COMPLIANT_CONTACTS					1
// Aggregates
#define	PHYSX_SUPPORT_GPU_AGGREGATES						1
// Shapes
#define PHYSX_SUPPORT_SHARED_SHAPES							1
#define PHYSX_DEPRECATED_CREATE_SHAPE						1
#define PHYSX_SUPPORT_TORSION_FRICTION						1
#define PHYSX_SUPPORT_RIGIDACTOREX_CREATE_EXCLUSIVE_SHAPE	1
#define PHYSX_SUPPORT_CUSTOM_GEOMETRY						1
#define PHYSX_SUPPORT_CUSTOM_GEOMETRY_PUBLIC_MEMBERS		0
#define PHYSX_SUPPORT_CONVEX_CORE_GEOMETRY					0
#define PHYSX_SUPPORT_HEIGHTFIELDS							1
#define PHYSX_SUPPORT_HEIGHTFIELD_THICKNESS					0
#define PHYSX_SUPPORT_DIRECT_SHAPE_GET_GEOMETRY				1
// Articulations
#define PHYSX_SUPPORT_ARTICULATIONS							0
#define	PHYSX_SUPPORT_DAMPING_ON_ARTICULATION_LINKS			1
#define PHYSX_SUPPORT_RCA									1
#define PHYSX_SUPPORT_RCA_CFM_SCALE							1
#define PHYSX_SUPPORT_RCA_DOF_SCALE							0
#define PHYSX_SUPPORT_RCA_ARMATURE							1
#define PHYSX_SUPPORT_RCA_NEW_LIMIT_API						1
// Convexes
#define PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT			1
#define PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS					1
// Meshes
#define PHYSX_SUPPORT_DYNAMIC_MESHES						1
#define PHYSX_SUPPORT_DEFORMABLE_MESHES						1
#define PHYSX_SUPPORT_MESH_16BIT_INDICES					PxTriangleMeshFlag::e16_BIT_INDICES
// Cooking
#define PHYSX_SUPPORT_INSERTION_CALLBACK					1
#define PHYSX_SUPPORT_PX_MESH_MIDPHASE						1
#define PHYSX_SUPPORT_PX_MESH_MIDPHASE2						1
#define PHYSX_SUPPORT_PX_MESH_COOKING_HINT					1
#define PHYSX_SUPPORT_QUANTIZED_TREE_OPTION					1
#define PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE		1
#define PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY				1
#define PHYSX_SUPPORT_IMMEDIATE_COOKING						1
// Vehicles
#define PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS				1
#define PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION				1
#define PHYSX_SUPPORT_ANTIROLLBAR							1
#define PHYSX_SUPPORT_LIMIT_SUSPENSION_EXPANSION_VELOCITY	1
#define PHYSX_SUPPORT_NEW_VEHICLE_SUSPENSION_FLAGS			1
#define PHYSX_SUPPORT_STEER_FILTER							1
#define PHYSX_SUPPORT_VEHICLE5								1
// Joints
#define PHYSX_REMOVE_JOINT_32_COMPATIBILITY					1
#define PHYSX_SUPPORT_EXTENDED_LIMITS						1
#define PHYSX_SUPPORT_DISABLE_PREPROCESSING					1
#define PHYSX_SUPPORT_PORTAL_JOINT							1
#define PHYSX_SUPPORT_GEAR_JOINT							1
#define PHYSX_SUPPORT_RACK_JOINT							1
#define PHYSX_SUPPORT_CHAIN_JOINT							0
#define PHYSX_SUPPORT_NEW_JOINT_TYPES						1
#define PHYSX_SUPPORT_JOINT_PROJECTION						0
#define PHYSX_SUPPORT_JOINT_CONTACT_DISTANCE				0
#define PHYSX_SUPPORT_JOINT_PXTRANSFORM32					1
// Scene queries
#define PHYSX_SUPPORT_SQ_UPDATE_MODE						1
#define PHYSX_DEPRECATED_DISTANCE							1
#define PHYSX_SUPPORT_BVH_STRUCTURE							1
#define PHYSX_SUPPORT_SIMD_GUARD_FLAG						1
// CCD
#define PHYSX_SUPPORT_RAYCAST_CCD							1
#define PHYSX_SUPPORT_RAYCAST_CCD_UNREGISTER				1
#define PHYSX_SUPPORT_ANGULAR_CCD							1
// Fluids
#define	PHYSX_SUPPORT_FLUIDS								1


#if PX_SUPPORT_GPU_PHYSX
	#define PHYSX_SUPPORT_GPU					1
	#define PHYSX_SUPPORT_DIRECT_GPU			1
	#define PHYSX_SUPPORT_GPU_NEW_MEMORY_CONFIG	1
	#define PHYSX_SUPPORT_GPU_AGG_MEMORY_CONFIG	1
#endif

#if PHYSX_SUPPORT_GPU
	#define BUILD_GPU_DATA	buildGPUData
#endif
#define PHYSX_NUM_PRIMS_PER_LEAF	numPrimsPerLeaf
//#define PHYSX_SUPPORT_LINEAR_COEFF
#define NEW_D6_API
//#define PHYSX_SUPPORT_CHARACTERS
#define PHYSX_SUPPORT_CHARACTERS2
#define PHYSX_REMOVED_CLIENT_ID

//#define PHYSX_SUPPORT_VEHICLE_FIX
//#define PHYSX_SUPPORT_VEHICLE_XP

#define PHYSX_NEW_PUBLIC_API

#define PX_RACK_AND_PINION_H

#define PxIdtQuat						PxQuat(PxIdentity)
#define	PxConvexFlag_eGPU_COMPATIBLE	PxConvexFlag::eGPU_COMPATIBLE
#define PHYSX_CREATE_AGGREGATE_PARAMS	max_size, max_size, enable_self_collision
#define PHYSX_CONTACT_DISTANCE			contactDistance_deprecated

//#define PHYSX_SUPPORT_PMAP_XP

#include "..\PINT_Common\PINT_CommonPhysX3_Deprecated.h"
