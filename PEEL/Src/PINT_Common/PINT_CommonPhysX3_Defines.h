///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_DEFINES_H
#define PINT_COMMON_PHYSX3_DEFINES_H

// General
#ifndef PHYSX_SUPPORT_SCRATCH_BUFFER
	#error Mandatory define missing: PHYSX_SUPPORT_SCRATCH_BUFFER
#endif
#ifndef PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
	#error Mandatory define missing: PHYSX_SUPPORT_CONTACT_NOTIFICATIONS
#endif
#ifndef PHYSX_SUPPORT_CONTACT_MODIFICATIONS
	#error Mandatory define missing: PHYSX_SUPPORT_CONTACT_MODIFICATIONS
#endif
#ifndef PHYSX_SUPPORT_PX_BROADPHASE_TYPE
	#error Mandatory define missing: PHYSX_SUPPORT_PX_BROADPHASE_TYPE
#endif
#ifndef PHYSX_SUPPORT_PX_BROADPHASE_ABP
	#error Mandatory define missing: PHYSX_SUPPORT_PX_BROADPHASE_ABP
#endif
#ifndef PHYSX_SUPPORT_PX_BROADPHASE_PABP
	#error Mandatory define missing: PHYSX_SUPPORT_PX_BROADPHASE_PABP
#endif

// Actors/Rigid bodies
#ifndef PHYSX_SUPPORT_ADAPTIVE_FORCE
	#error Mandatory define missing: PHYSX_SUPPORT_ADAPTIVE_FORCE
#endif
#ifndef PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
	#error Mandatory define missing: PHYSX_SUPPORT_MAX_DEPEN_VELOCITY
#endif
#ifndef PHYSX_SUPPORT_TGS
	#error Mandatory define missing: PHYSX_SUPPORT_TGS
#endif
#ifndef PHYSX_SUPPORT_SUBSTEPS
	#error Mandatory define missing: PHYSX_SUPPORT_SUBSTEPS
#endif
#ifndef PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
	#error Mandatory define missing: PHYSX_SUPPORT_IMPROVED_PATCH_FRICTION
#endif
#ifndef PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
	#error Mandatory define missing: PHYSX_SUPPORT_FRICTION_EVERY_ITERATION
#endif
#ifndef PHYSX_SUPPORT_STABILIZATION_FLAG
	#error Mandatory define missing: PHYSX_SUPPORT_STABILIZATION_FLAG
#endif
#ifndef PHYSX_SUPPORT_KINE_FILTERING_MODE
	#error Mandatory define missing: PHYSX_SUPPORT_KINE_FILTERING_MODE
#endif
#ifndef PHYSX_SUPPORT_GYROSCOPIC_FORCES
	#error Mandatory define missing: PHYSX_SUPPORT_GYROSCOPIC_FORCES
#endif

// Aggregates
#ifndef PHYSX_SUPPORT_GPU_AGGREGATES
	#error Mandatory define missing: PHYSX_SUPPORT_GPU_AGGREGATES
#endif

// Shapes
#ifndef PHYSX_SUPPORT_SHARED_SHAPES
	#error Mandatory define missing: PHYSX_SUPPORT_SHARED_SHAPES
#endif
#ifndef PHYSX_DEPRECATED_CREATE_SHAPE
	#error Mandatory define missing: PHYSX_DEPRECATED_CREATE_SHAPE
#endif
#ifndef PHYSX_SUPPORT_TORSION_FRICTION
	#error Mandatory define missing: PHYSX_SUPPORT_TORSION_FRICTION
#endif
#ifndef PHYSX_SUPPORT_RIGIDACTOREX_CREATE_EXCLUSIVE_SHAPE
	#error Mandatory define missing: PHYSX_SUPPORT_RIGIDACTOREX_CREATE_EXCLUSIVE_SHAPE
#endif
#ifndef PHYSX_SUPPORT_CUSTOM_GEOMETRY
	#error Mandatory define missing: PHYSX_SUPPORT_CUSTOM_GEOMETRY
#endif
#ifndef PHYSX_SUPPORT_HEIGHTFIELDS
	#error Mandatory define missing: PHYSX_SUPPORT_HEIGHTFIELDS
#endif
#ifndef PHYSX_SUPPORT_HEIGHTFIELD_THICKNESS
	#error Mandatory define missing: PHYSX_SUPPORT_HEIGHTFIELD_THICKNESS
#endif

// Articulations
#ifndef PHYSX_SUPPORT_ARTICULATIONS
	#error Mandatory define missing: PHYSX_SUPPORT_ARTICULATIONS
#endif
#ifndef PHYSX_SUPPORT_DAMPING_ON_ARTICULATION_LINKS
	#error Mandatory define missing: PHYSX_SUPPORT_DAMPING_ON_ARTICULATION_LINKS
#endif
#ifndef PHYSX_SUPPORT_RCA
	#error Mandatory define missing: PHYSX_SUPPORT_RCA
#endif
#ifndef PHYSX_SUPPORT_RCA_CFM_SCALE
	#error Mandatory define missing: PHYSX_SUPPORT_RCA_CFM_SCALE
#endif
#ifndef PHYSX_SUPPORT_RCA_DOF_SCALE
	#error Mandatory define missing: PHYSX_SUPPORT_RCA_DOF_SCALE
#endif
#ifndef PHYSX_SUPPORT_RCA_ARMATURE
	#error Mandatory define missing: PHYSX_SUPPORT_RCA_ARMATURE
#endif
// Convexes
#ifndef PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT
	#error Mandatory define missing: PHYSX_SUPPORT_USER_DEFINED_GAUSSMAP_LIMIT
#endif
#ifndef PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS
	#error Mandatory define missing: PHYSX_SUPPORT_TIGHT_CONVEX_BOUNDS
#endif

// Meshes
#ifndef PHYSX_SUPPORT_DYNAMIC_MESHES
	#error Mandatory define missing: PHYSX_SUPPORT_DYNAMIC_MESHES
#endif
#ifndef PHYSX_SUPPORT_DEFORMABLE_MESHES
	#error Mandatory define missing: PHYSX_SUPPORT_DEFORMABLE_MESHES
#endif

// Cooking
#ifndef PHYSX_SUPPORT_INSERTION_CALLBACK
	#error Mandatory define missing: PHYSX_SUPPORT_INSERTION_CALLBACK
#endif
#ifndef PHYSX_SUPPORT_PX_MESH_MIDPHASE
	#error Mandatory define missing: PHYSX_SUPPORT_PX_MESH_MIDPHASE
#endif
#ifndef PHYSX_SUPPORT_PX_MESH_MIDPHASE2
	#error Mandatory define missing: PHYSX_SUPPORT_PX_MESH_MIDPHASE2
#endif
#ifndef PHYSX_SUPPORT_PX_MESH_COOKING_HINT
	#error Mandatory define missing: PHYSX_SUPPORT_PX_MESH_COOKING_HINT
#endif
#ifndef PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
	#error Mandatory define missing: PHYSX_SUPPORT_PX_MESH_BUILD_STRATEGY
#endif
#ifndef PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
	#error Mandatory define missing: PHYSX_SUPPORT_QUANTIZED_TREE_OPTION
#endif
#ifndef PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE
	#error Mandatory define missing: PHYSX_SUPPORT_DISABLE_ACTIVE_EDGES_PRECOMPUTE
#endif
#ifndef PHYSX_SUPPORT_IMMEDIATE_COOKING
	#error Mandatory define missing: PHYSX_SUPPORT_IMMEDIATE_COOKING
#endif

// Vehicles
#ifndef PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
	#error Mandatory define missing: PHYSX_SUPPORT_VEHICLE_SUSPENSION_SWEEPS
#endif
#ifndef PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
	#error Mandatory define missing: PHYSX_SUPPORT_VEHICLE_SWEEP_INFLATION
#endif
#ifndef PHYSX_SUPPORT_ANTIROLLBAR
	#error Mandatory define missing: PHYSX_SUPPORT_ANTIROLLBAR
#endif
#ifndef PHYSX_SUPPORT_LIMIT_SUSPENSION_EXPANSION_VELOCITY
	#error Mandatory define missing: PHYSX_SUPPORT_LIMIT_SUSPENSION_EXPANSION_VELOCITY
#endif
#ifndef PHYSX_SUPPORT_NEW_VEHICLE_SUSPENSION_FLAGS
	#error Mandatory define missing: PHYSX_SUPPORT_NEW_VEHICLE_SUSPENSION_FLAGS
#endif
#ifndef PHYSX_SUPPORT_STEER_FILTER
	#error Mandatory define missing: PHYSX_SUPPORT_STEER_FILTER
#endif
#ifndef PHYSX_SUPPORT_VEHICLE5
	#error Mandatory define missing: PHYSX_SUPPORT_VEHICLE5
#endif

// Joints
#ifndef PHYSX_REMOVE_JOINT_32_COMPATIBILITY
	#error Mandatory define missing: PHYSX_REMOVE_JOINT_32_COMPATIBILITY
#endif
#ifndef PHYSX_SUPPORT_EXTENDED_LIMITS
	#error Mandatory define missing: PHYSX_SUPPORT_EXTENDED_LIMITS
#endif
#ifndef PHYSX_SUPPORT_DISABLE_PREPROCESSING
	#error Mandatory define missing: PHYSX_SUPPORT_DISABLE_PREPROCESSING
#endif
#ifndef PHYSX_SUPPORT_PORTAL_JOINT
	#error Mandatory define missing: PHYSX_SUPPORT_PORTAL_JOINT
#endif
#ifndef PHYSX_SUPPORT_GEAR_JOINT
	#error Mandatory define missing: PHYSX_SUPPORT_GEAR_JOINT
#endif
#ifndef PHYSX_SUPPORT_RACK_JOINT
	#error Mandatory define missing: PHYSX_SUPPORT_RACK_JOINT
#endif
#ifndef PHYSX_SUPPORT_CHAIN_JOINT
	#error Mandatory define missing: PHYSX_SUPPORT_CHAIN_JOINT
#endif
#ifndef PHYSX_SUPPORT_NEW_JOINT_TYPES
	#error Mandatory define missing: PHYSX_SUPPORT_NEW_JOINT_TYPES
#endif

// Scene queries
#ifndef PHYSX_SUPPORT_SQ_UPDATE_MODE
	#error Mandatory define missing: PHYSX_SUPPORT_SQ_UPDATE_MODE
#endif
#ifndef PHYSX_DEPRECATED_DISTANCE
	#error Mandatory define missing: PHYSX_DEPRECATED_DISTANCE
#endif
#ifndef PHYSX_SUPPORT_BVH_STRUCTURE
	#error Mandatory define missing: PHYSX_SUPPORT_BVH_STRUCTURE
#endif
#ifndef PHYSX_SUPPORT_SIMD_GUARD_FLAG
	#error Mandatory define missing: PHYSX_SUPPORT_SIMD_GUARD_FLAG
#endif

// CCD
#ifndef PHYSX_SUPPORT_RAYCAST_CCD
	#error Mandatory define missing: PHYSX_SUPPORT_RAYCAST_CCD
#endif
#ifndef PHYSX_SUPPORT_RAYCAST_CCD_UNREGISTER
	#error Mandatory define missing: PHYSX_SUPPORT_RAYCAST_CCD_UNREGISTER
#endif
#ifndef PHYSX_SUPPORT_ANGULAR_CCD
	#error Mandatory define missing: PHYSX_SUPPORT_ANGULAR_CCD
#endif

// Fluids
#ifndef PHYSX_SUPPORT_FLUIDS
	#error Mandatory define missing: PHYSX_SUPPORT_FLUIDS
#endif

#endif