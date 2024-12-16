///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX_JOINT_UTILS_H
#define PINT_COMMON_PHYSX_JOINT_UTILS_H

	PxQuat ComputeJointQuat(const PxTransform* pose, const PxVec3& localAxis);

#endif

