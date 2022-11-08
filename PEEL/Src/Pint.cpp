///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Pint.h"

///////////////////////////////////////////////////////////////////////////////

// The caps tell PEEL which features are supported by each PINT plug-in, in a
// way similar to the old D3D caps for graphics cards. The goal is simply to
// disable certain tests for engines that do not support a specific feature, or
// for engines whose plugin does not yet expose a supported feature. Without
// this, some tests can either crash or behave badly simply because the feature
// is not properly exposed yet - giving users the wrong impression.
PintCaps::PintCaps() :

	// True for physics engines, i.e. the ones supporting basic rigid-body simulation.
	// False for collision libraries, e.g. supporting only "scene queries" (like Opcode).
	// It is assumed that engines returning true here support basic collision primitives
	// like boxes, spheres and capsules. More advanced primitives have dedicated caps below.
	mSupportRigidBodySimulation	(false),

	// True for libraries supporting implicit cylinder shapes (as opposed to tessellated convexes).
	// These are typically defined by a radius and a (half) height.
	mSupportCylinders			(false),

	// True for libraries supporting convex objects.
	// These are typically defined by a set of vertices.
	mSupportConvexes			(false),

	// True for libraries supporting triangles meshes.
	// These are typically defined by a set of vertices and a set of triangle indices.
	mSupportMeshes				(false),

	// True for libraries supporting dynamic triangles meshes, i.e. rigid body simulation
	// and contact generation on triangle mesh shapes.
	mSupportDynamicMeshes		(false),

	// True for libraries supporting deformable triangles meshes. This is not the same as
	// a dynamic mesh, as this can be supported for static meshes like terrains.
	mSupportDeformableMeshes	(false),

	// True for libraries supporting heightfields.
	mSupportHeightfields		(false),

	// True for libraries that can return collision/contact data to users.
	mSupportContactNotifications(false),

	// True for libraries that can return collision/contact data to users, and let them modify that data.
	mSupportContactModifications(false),

	// True for physics engines supporting tweaked masses for inertia computations, i.e. computing
	// the shape's inertia tensor with a mass different from the actual final mass of the object.
	mSupportMassForInertia		(false),

	// True for libraries supporting kinematic objects,
	// i.e. driven by "setPosition" kind of calls.
	mSupportKinematics			(false),

	// True for libraries supporting basic filtering groups.
	mSupportCollisionGroups		(false),

	// True for libraries supporting "compound" or "composite" objects,
	// i.e. objects/actors made of multiple shapes.
	mSupportCompounds			(false),

	// True for libraries supporting "aggregates",
	// i.e. a set of otherwise independent actors grouped together in a single broadphase entity.
	mSupportAggregates			(false),

	// True for libraries supporting spherical joints (a.k.a. point-to-point constraint a.k.a. ball-and-socket)
	mSupportSphericalJoints		(false),

	// True for libraries supporting hinge joints (a.k.a. revolute joints)
	mSupportHingeJoints			(false),

	// True for libraries supporting fixed joints
	mSupportFixedJoints			(false),

	// True for libraries supporting prismatic joints (a.k.a. slider constraints)
	mSupportPrismaticJoints		(false),

	// True for libraries supporting distance joints
	mSupportDistanceJoints		(false),

	// True for libraries supporting D6 joints
	mSupportD6Joints			(false),

	// True for libraries supporting gear joints
	mSupportGearJoints			(false),

	// True for libraries supporting rack & pinion joints
	mSupportRackJoints			(false),

	// True for libraries supporting "portal joints"
	mSupportPortalJoints		(false),

	// True for libraries supporting "MC articulations" (MC = Maximal Coordinates),
	// i.e. generally speaking, a feature dedicated to articulated objects.
	// There is no simple definition that easily maps to existing implementations in different engines so this is experimental.
	mSupportMCArticulations		(false),

	// True for libraries supporting "RC articulations" (RC = Reduced Coordinates),
	// - PhysX before version 4 supports mSupportMCArticulations only.
	// - PhysX after version 4 supports both mSupportMCArticulations & mSupportRCArticulations.
	// Merging the two bools would either prevent the PEEL 1.1 articulation scenes from running on old PhysX plugins, or it would
	// make them crash when they try to run the scenes designed for the new articulations/interface.
	mSupportRCArticulations		(false),

	// True for libraries supporting raycasting (part of "scene queries")
	mSupportRaycasts			(false),

	// True for libraries supporting box sweeps (part of "scene queries")
	mSupportBoxSweeps			(false),

	// True for libraries supporting sphere sweeps (part of "scene queries")
	mSupportSphereSweeps		(false),

	// True for libraries supporting capsule sweeps (part of "scene queries")
	mSupportCapsuleSweeps		(false),

	// True for libraries supporting convex sweeps (part of "scene queries")
	mSupportConvexSweeps		(false),

	// True for libraries supporting sphere overlaps (part of "scene queries")
	mSupportSphereOverlaps		(false),

	// True for libraries supporting box overlaps (part of "scene queries")
	mSupportBoxOverlaps			(false),

	// True for libraries supporting capsule overlaps (part of "scene queries")
	mSupportCapsuleOverlaps		(false),

	// True for libraries supporting convex overlaps (part of "scene queries")
	mSupportConvexOverlaps		(false),

	// True for libraries supporting mesh-vs-mesh overlaps (part of "scene queries")
	mSupportMeshMeshOverlaps	(false),

	// True for libraries supporting a module dedicated to vehicle simulation
	// (as opposed to vehicles made with regular rigid bodies and joints, which would use the above caps).
	// There is no simple definition that easily maps to existing implementations in different engines so this is experimental.
	mSupportVehicles			(false),

	// True for libraries supporting a module dedicated to character controllers.
	// There is no simple definition that easily maps to existing implementations in different engines so this is experimental.
	mSupportCharacters			(false)
{
}

///////////////////////////////////////////////////////////////////////////////

static const char* gShapeTypeName[] = 
{
	"Undefined",
	"Sphere",
	"Capsule",
	"Cylinder",
	"Box",
	"Convex",
	"Mesh",
	"Mesh2",
};

const char* GetPintShapeName(PintShape type)
{
	return gShapeTypeName[type];
}

static const char* gJointTypeName[] = 
{
	"Undefined",
	"Spherical",
	"Hinge",
	"Hinge2",
	"Prismatic",
	"Fixed",
	"Distance",
	"D6",
	"Gear",
	"Rack and pinion",
	"Chain",
	"Portal",
};

const char* GetPintJointName(PintJoint type)
{
	return gJointTypeName[type];
}

///////////////////////////////////////////////////////////////////////////////

Point ComputeLocalPoint(const Point& hit, const PR& world_pose)
{
	// TODO: rewrite this clumsy stuff
	const Matrix4x4 M = world_pose;
	Matrix4x4 InvM;
	InvertPRMatrix(InvM, M);

	return hit * InvM;
}