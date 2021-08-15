///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_BULLET_H
#define PINT_COMMON_BULLET_H

	#define	BULLET_MAIN_COLOR	Point(1.0f, 1.0f, 0.5f)

	inline_ Point			ToPoint(const btVector3& p)		{ return Point(p.x(), p.y(), p.z());			}
	inline_ Quat			ToQuat(const btQuaternion& q)	{ return Quat(q.getW(), q.x(), q.y(), q.z());	}
	inline_ btVector3		ToBtVector3(const Point& p)		{ return btVector3(p.x, p.y, p.z);				}
	inline_ btQuaternion	ToBtQuaternion(const Quat& q)	{ return btQuaternion(q.p.x, q.p.y, q.p.z, q.w);}
	inline_ PR				ToPR(const btTransform& pose)
	{
		const btVector3 p = pose.getOrigin();
		const btQuaternion q = pose.getRotation();
		return PR(ToPoint(p), ToQuat(q));
	}

	// A special version to keep track of the triangle index
	struct MyClosestConvexResultCallback : public btCollisionWorld::ConvexResultCallback
	{
		MyClosestConvexResultCallback(const btVector3& convexFromWorld, const btVector3& convexToWorld) : m_convexFromWorld(convexFromWorld), m_convexToWorld(convexToWorld), m_hitCollisionObject(0)
		{
#ifdef _DEBUG
			mNbCalls = 0;
#endif
		}

		btVector3	m_convexFromWorld;//used to calculate hitPointWorld from hitFraction
		btVector3	m_convexToWorld;

		btVector3	m_hitNormalWorld;
		btVector3	m_hitPointWorld;
		const btCollisionObject*	m_hitCollisionObject;
		
		udword		m_TriangleIndex;

#ifdef _DEBUG
		udword		mNbCalls;
#endif

		virtual	btScalar	addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
		{
#ifdef _DEBUG
			mNbCalls++;
#endif
			//caller already does the filter on the m_closestHitFraction
			ASSERT(convexResult.m_hitFraction <= m_closestHitFraction);

			if(convexResult.m_localShapeInfo)
				m_TriangleIndex	= convexResult.m_localShapeInfo->m_triangleIndex;
			else
				m_TriangleIndex	= INVALID_ID;

			m_closestHitFraction = convexResult.m_hitFraction;
			m_hitCollisionObject = convexResult.m_hitCollisionObject;
			if(normalInWorldSpace)
			{
				m_hitNormalWorld = convexResult.m_hitNormalLocal;
			}
			else
			{
				///need to transform normal into worldspace
				m_hitNormalWorld = m_hitCollisionObject->getWorldTransform().getBasis()*convexResult.m_hitNormalLocal;
			}
			m_hitPointWorld = convexResult.m_hitPointLocal;
			return convexResult.m_hitFraction;
		}
	};

	inline_ void FillResultStruct(PintRaycastHit& hit, const MyClosestConvexResultCallback& result, float max_dist)
	{
		hit.mTouchedActor	= (PintActorHandle)btRigidBody::upcast(result.m_hitCollisionObject);
		hit.mTouchedShape	= null;
		hit.mImpact			= ToPoint(result.m_hitPointWorld);
		hit.mNormal			= ToPoint(result.m_hitNormalWorld);
	//	hit.mDistance		= origin.Distance(hit.mImpact);
		hit.mDistance		= result.m_closestHitFraction * max_dist;
		hit.mTriangleIndex	= result.m_TriangleIndex;
	}

	inline_	udword 	RemapCollisionGroup(udword group)	{ return group+2;	}

#endif