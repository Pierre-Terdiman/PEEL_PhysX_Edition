///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX_JointUtils.h"

static void normalToTangents(const PxVec3& n, PxVec3& t1, PxVec3& t2)
{
	const PxReal m_sqrt1_2 = PxReal(0.7071067811865475244008443621048490);
	if(fabsf(n.z) > m_sqrt1_2)
	{
		const PxReal a = n.y*n.y + n.z*n.z;
		const PxReal k = PxReal(1.0)/PxSqrt(a);
		t1 = PxVec3(0,-n.z*k,n.y*k);
		t2 = PxVec3(a*k,-n.x*t1.z,n.x*t1.y);
	}
	else 
	{
		const PxReal a = n.x*n.x + n.y*n.y;
		const PxReal k = PxReal(1.0)/PxSqrt(a);
		t1 = PxVec3(-n.y*k,n.x*k,0);
		t2 = PxVec3(-n.z*t1.y,n.z*t1.x,a*k);
	}
	t1.normalize();
	t2.normalize();
}

PxQuat ComputeJointQuat(const PxTransform* pose, const PxVec3& localAxis)
{
//	return PxQuat(PxIdentity);

	//find 2 orthogonal vectors.
	//gotta do this in world space, if we choose them
	//separately in local space they won't match up in worldspace.
	PxVec3 axisw = pose ? pose->rotate(localAxis) : localAxis;
	axisw.normalize();

	PxVec3 normalw, binormalw;
	::normalToTangents(axisw, binormalw, normalw);

	const PxVec3 localNormal = pose ? pose->rotateInv(normalw) : normalw;

	const PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
	PxQuat q(rot);
	q.normalize();

/*		if(q.w<0.0f)
		{
			printf("Negating quat...\n");
			q = -q;
		}*/

	return q;
}
