///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for plane-AABB intersection
 *	\file		CTC_PlaneAABBOverlap.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCPLANEAABBOVERLAP_H
#define CTCPLANEAABBOVERLAP_H

	// Possible types of overlap
	enum PlaneAABBCode
	{
		PLANEAABB_INCLUSION		= -1,
		PLANEAABB_INTERSECT		= 0,
		PLANEAABB_EXCLUSION		= 1,

		PLANEAABB_FORCE_DWORD	= 0x7fffffff
	};

	// Plane-AABB intersection
	CONTACT_API PlaneAABBCode	PlaneAABBOverlap(const Plane& plane, const Point& min, const Point& max);
	CONTACT_API PlaneAABBCode	PlaneAABBOverlap2(const Plane& plane, const Point& min, const Point& max);
	CONTACT_API PlaneAABBCode	PlaneAABBOverlap3(const Plane& plane, const Point& min, const Point& max);
	CONTACT_API PlaneAABBCode	PlaneAABBOverlap4(const Plane& plane, const Point& min, const Point& max);

	inline_ PlaneAABBCode PlaneAABBOverlap(const Plane& plane, const AABB& aabb)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::PlaneAABBOverlap(plane, Box->mMin, Box->mMax);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::PlaneAABBOverlap(plane, Min, Max);
#endif
	}

	inline_ PlaneAABBCode PlaneAABBOverlap2(const Plane& plane, const AABB& aabb)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::PlaneAABBOverlap2(plane, Box->mMin, Box->mMax);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::PlaneAABBOverlap2(plane, Min, Max);
#endif
	}

	inline_ PlaneAABBCode PlaneAABBOverlap3(const Plane& plane, const AABB& aabb)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::PlaneAABBOverlap3(plane, Box->mMin, Box->mMax);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::PlaneAABBOverlap3(plane, Min, Max);
#endif
	}

	inline_ PlaneAABBCode PlaneAABBOverlap4(const Plane& plane, const AABB& aabb)
	{
#ifdef ICE_USE_MINMAX
		const ShadowAABB* Box = (const ShadowAABB*)&aabb;
		return Ctc::PlaneAABBOverlap4(plane, Box->mMin, Box->mMax);
#else
		Point Min, Max;
		aabb.GetMin(Min);
		aabb.GetMax(Max);
		return Ctc::PlaneAABBOverlap4(plane, Min, Max);
#endif
	}

	// Following code from Umbra/dPVS.

	//------------------------------------------------------------------------
	//
	// Function:        DPVS::intersectAABBFrustum()
	//
	// Description:     Determines whether an AABB intersects a frustum
	//
	// Parameters:      a           = reference to AABB (defined by min & max vectors)
	//                  p           = array of pre-normalized clipping planes
	//                  outClipMask = output clip mask (if function returns 'true')
	//                  inClipMask  = input clip mask (indicates which planes are active)
	//
	// Returns:         true if AABB intersects the frustum, false otherwise
	//
	//                  Intersection of AABB and a frustum. The frustum may 
	//                  contain 0-32 planes (active planes are defined by inClipMask). 
	//                  If AABB intersects the frustum, an output clip mask is returned 
	//                  as well (indicating which planes are crossed by the AABB). This 
	//                  information can be used to optimize testing of child nodes or 
	//                  objects inside the nodes (pass value as 'inClipMask' next time).
	//
	//                  This is a variant of the classic "fast" AABB/frustum 
	//                  intersection tester. AABBs that are not culled away by any single 
	//                  plane are classified as "intersecting" even though the AABB may 
	//                  actually be outside the convex volume formed by the planes.
	//------------------------------------------------------------------------

	inline_ bool PlanesAABBOverlap(const AABB& a, const Plane* p, udword& out_clip_mask, udword in_clip_mask)
	{
		//------------------------------------------------------------------------
		// Convert the AABB from (min,max) form into (center,half-diagonal).
		// Note that we could get rid of these six subtractions and three
		// multiplications if the AABB was originally expressed in (center,
		// half-diagonal) form.
		//------------------------------------------------------------------------

		Point m;	a.GetCenter(m);			// get center of AABB ((min+max)*0.5f)
		Point d;	a.GetMax(d); d-=m;		// get positive half-diagonal (max - center)

		//------------------------------------------------------------------------
		// Evaluate through all active frustum planes. We determine the relation 
		// between the AABB and a plane by using the concept of "near" and "far"
		// vertices originally described by Zhang (and later by Möller). Our
		// variant here uses 3 fabs ops, 6 muls, 7 adds and two floating point
		// comparisons per plane. The routine early-exits if the AABB is found
		// to be outside any of the planes. The loop also constructs a new output
		// clip mask. Most FPUs have a native single-cycle fabsf() operation.
		//------------------------------------------------------------------------

		udword Mask				= 1;			// current mask index (1,2,4,8,..)
		udword TmpOutClipMask	= 0;			// initialize output clip mask into empty. 

		while(Mask<=in_clip_mask)				// keep looping while we have active planes left...
		{
			if(in_clip_mask & Mask)				// if clip plane is active, process it..
			{               
				const float NP = d.x*fabsf(p->n.x) + d.y*fabsf(p->n.y) + d.z*fabsf(p->n.z);
				const float MP = m.x*p->n.x + m.y*p->n.y + m.z*p->n.z + p->d;

				if(NP < MP)						// near vertex behind the clip plane... 
					return false;				// .. so there is no intersection..
				if((-NP) < MP)					// near and far vertices on different sides of plane..
					TmpOutClipMask |= Mask;		// .. so update the clip mask...
			}
			Mask+=Mask;							// mk = (1<<plane)
			p++;								// advance to next plane
		}

		out_clip_mask = TmpOutClipMask;			// copy output value (temp used to resolve aliasing!)
		return true;							// indicate that AABB intersects frustum
	}

	inline_ bool PlanesAABBOverlap(const AABB& a, const Plane** pp, udword& out_clip_mask, udword in_clip_mask)
	{
		//------------------------------------------------------------------------
		// Convert the AABB from (min,max) form into (center,half-diagonal).
		// Note that we could get rid of these six subtractions and three
		// multiplications if the AABB was originally expressed in (center,
		// half-diagonal) form.
		//------------------------------------------------------------------------

		Point m;	a.GetCenter(m);			// get center of AABB ((min+max)*0.5f)
		Point d;	a.GetMax(d); d-=m;		// get positive half-diagonal (max - center)

		//------------------------------------------------------------------------
		// Evaluate through all active frustum planes. We determine the relation 
		// between the AABB and a plane by using the concept of "near" and "far"
		// vertices originally described by Zhang (and later by Möller). Our
		// variant here uses 3 fabs ops, 6 muls, 7 adds and two floating point
		// comparisons per plane. The routine early-exits if the AABB is found
		// to be outside any of the planes. The loop also constructs a new output
		// clip mask. Most FPUs have a native single-cycle fabsf() operation.
		//------------------------------------------------------------------------

		udword Mask				= 1;			// current mask index (1,2,4,8,..)
		udword TmpOutClipMask	= 0;			// initialize output clip mask into empty. 

		while(Mask<=in_clip_mask)				// keep looping while we have active planes left...
		{
			const Plane* p = *pp++;

			if(in_clip_mask & Mask)				// if clip plane is active, process it..
			{               
				const float NP = d.x*fabsf(p->n.x) + d.y*fabsf(p->n.y) + d.z*fabsf(p->n.z);
				const float MP = m.x*p->n.x + m.y*p->n.y + m.z*p->n.z + p->d;

				if(NP < MP)						// near vertex behind the clip plane... 
					return false;				// .. so there is no intersection..
				if((-NP) < MP)					// near and far vertices on different sides of plane..
					TmpOutClipMask |= Mask;		// .. so update the clip mask...
			}
			Mask+=Mask;							// mk = (1<<plane)
//			p++;								// advance to next plane
		}

		out_clip_mask = TmpOutClipMask;			// copy output value (temp used to resolve aliasing!)
		return true;							// indicate that AABB intersects frustum
	}

#endif // CTCPLANEAABBOVERLAP_H
