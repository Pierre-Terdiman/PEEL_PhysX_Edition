///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_BASE_H
#define PINT_COMMON_PHYSX3_BASE_H

	#define SAFE_RELEASE(x)	if(x) { x->release(); x = null; }

//	inline_ Point			ToPoint(const PxVec3& p)	{ return Point(p.x, p.y, p.z);				}
	inline_ const Point&	ToPoint(const PxVec3& p)	{ return (const Point&)p;					}
//	inline_ PxVec3			ToPxVec3(const Point& p)	{ return PxVec3(p.x, p.y, p.z);				}
	inline_ const PxVec3&	ToPxVec3(const Point& p)	{ return (const PxVec3&)p;					}

//	inline_ Quat			ToQuat(const PxQuat& q)		{ return Quat(q.w, q.x, q.y, q.z);			}
//	inline_ PxQuat			ToPxQuat(const Quat& q)		{ return PxQuat(q.p.x, q.p.y, q.p.z, q.w);	}
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.x)==OFFSET_OF(PxQuat, x));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.y)==OFFSET_OF(PxQuat, y));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.z)==OFFSET_OF(PxQuat, z));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, w)==OFFSET_OF(PxQuat, w));
	inline_ const Quat&		ToQuat(const PxQuat& q)		{ return (const Quat&)q;					}
	inline_ const PxQuat&	ToPxQuat(const Quat& q)		{ return (const PxQuat&)q;					}

	inline_	const PxTransform	ToPxTransform(const PR& pose)
	{
		return PxTransform(ToPxVec3(pose.mPos), ToPxQuat(pose.mRot));
	}

	inline_	const PR	ToPR(const PxTransform& pose)
	{
		return PR(ToPoint(pose.p), ToQuat(pose.q));
	}

	inline_ PxSphereGeometry ToPxSphereGeometry(PxTransform& pose, const Sphere& sphere)
	{
		pose = PxTransform(ToPxVec3(sphere.mCenter));
		return PxSphereGeometry(sphere.mRadius);
	}

	inline_ PxBoxGeometry ToPxBoxGeometry(PxTransform& pose, const OBB& box)
	{
		const Quat Q = box.mRot;	// ### SIGH
		pose = PxTransform(ToPxVec3(box.mCenter), ToPxQuat(Q));
		return PxBoxGeometry(ToPxVec3(box.mExtents));
	}

	inline_ PxCapsuleGeometry ToPxCapsuleGeometry(PxTransform& pose, const LSS& capsule)
	{
		// ### optimize this
		const Point Center = (capsule.mP0 + capsule.mP1)*0.5f;
		Point CapsuleAxis = capsule.mP1 - capsule.mP0;
		const float M = CapsuleAxis.Magnitude();
		CapsuleAxis /= M;
		const PxQuat q = PxShortestRotation(PxVec3(1.0f, 0.0f, 0.0f), ToPxVec3(CapsuleAxis));
		pose = PxTransform(ToPxVec3(Center), q);
		return PxCapsuleGeometry(capsule.mRadius, M*0.5f);
	}

#endif