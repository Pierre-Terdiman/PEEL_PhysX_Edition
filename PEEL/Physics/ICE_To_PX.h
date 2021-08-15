
	inline_ const Point&	ToPoint(const PxVec3& p)	{ return (const Point&)p;	}
	inline_ const PxVec3&	ToPxVec3(const Point& p)	{ return (const PxVec3&)p;	}
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.x)==OFFSET_OF(PxQuat, x));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.y)==OFFSET_OF(PxQuat, y));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, p.z)==OFFSET_OF(PxQuat, z));
	ICE_COMPILE_TIME_ASSERT(OFFSET_OF(Quat, w)==OFFSET_OF(PxQuat, w));
	inline_ const Quat&		ToQuat(const PxQuat& q)		{ return (const Quat&)q;	}
	inline_ const PxQuat&	ToPxQuat(const Quat& q)		{ return (const PxQuat&)q;	}

	inline_	const PxTransform	ToPxTransform(const PR& pose)
	{
		return PxTransform(ToPxVec3(pose.mPos), ToPxQuat(pose.mRot));
	}

	inline_	const PR	ToPR(const PxTransform& pose)
	{
		return PR(ToPoint(pose.p), ToQuat(pose.q));
	}
