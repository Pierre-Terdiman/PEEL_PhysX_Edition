///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for OBB-OBB intersection
 *	\file		CTC_OBBOBBOverlap.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCOBBOBBOVERLAP_H
#define CTCOBBOBBOVERLAP_H

	enum SepAxis
	{
		SEP_AXIS_OVERLAP,

		SEP_AXIS_A0,
		SEP_AXIS_A1,
		SEP_AXIS_A2,

		SEP_AXIS_B0,
		SEP_AXIS_B1,
		SEP_AXIS_B2,

		SEP_AXIS_A0_CROSS_B0,
		SEP_AXIS_A0_CROSS_B1,
		SEP_AXIS_A0_CROSS_B2,

		SEP_AXIS_A1_CROSS_B0,
		SEP_AXIS_A1_CROSS_B1,
		SEP_AXIS_A1_CROSS_B2,

		SEP_AXIS_A2_CROSS_B0,
		SEP_AXIS_A2_CROSS_B1,
		SEP_AXIS_A2_CROSS_B2,

		SEP_AXIS_FORCE_DWORD	= 0x7fffffff
	};

	// OBB-OBB intersection
	CONTACT_API int OBBOBBOverlap(float B[3][3], float T[3], float _a[3], float b[3]);

	CONTACT_API bool OBBOBBOverlap(const Point& e0, const Point& c0, const Matrix3x3& r0, const Point& e1, const Point& c1, const Matrix3x3& r1, bool full_test=true);

	CONTACT_API int OBBOBBOverlap(Point& res_normal, Point& res_point, float& res_penetration,
		const Point& c0, const Point& e0, const Matrix3x3& r0,
		const Point& c1, const Point& e1, const Matrix3x3& r1
		);

	CONTACT_API int testintersect_box_box(Point hitpos[16], float penedist[16], Point* hitnrm,
		const Point& c0, const Point& e0, const Matrix3x3& r0,
		const Point& c1, const Point& e1, const Matrix3x3& r1
		);

	inline_ int OBBOBBOverlap(Point& res_normal, Point& res_point, float& res_penetration, const OBB& box0, const OBB& box1)
	{
		return Ctc::OBBOBBOverlap(res_normal, res_point, res_penetration,
			box0.mCenter, box0.mExtents, box0.mRot,
			box1.mCenter, box1.mExtents, box1.mRot
			);
	}

	inline_ int testintersect_box_box(Point hitpos[16], float penedist[16], Point* hitnrm, const OBB& box0, const OBB& box1)
	{
		return Ctc::testintersect_box_box(hitpos, penedist, hitnrm,
			box0.mCenter, box0.mExtents, box0.mRot,
			box1.mCenter, box1.mExtents, box1.mRot
			);
	}

	inline_ bool OBBOBBOverlap(const OBB& obb0, const OBB& obb1, bool full_test=true)
	{
		return Ctc::OBBOBBOverlap(
			obb0.mExtents, obb0.mCenter, obb0.mRot,
			obb1.mExtents, obb1.mCenter, obb1.mRot,
			full_test);
	}

	inline_ bool OBBAABBOverlap(const OBB& obb, const AABB& aabb)
	{
		Point Center;
		Point Extents;
		aabb.GetCenter(Center);
		aabb.GetExtents(Extents);
		Matrix3x3 Idt;	Idt.Identity();
		return Ctc::OBBOBBOverlap(obb.mExtents, obb.mCenter, obb.mRot, Extents, Center, Idt, true);
	}

	CONTACT_API SepAxis SeparatingAxis(const Point& e0, const Point& c0, const Matrix3x3& r0, const Point& e1, const Point& c1, const Matrix3x3& r1, bool full_test=true);

	inline_ SepAxis SeparatingAxis(const OBB& obb0, const OBB& obb1, bool full_test=true)
	{
		return Ctc::SeparatingAxis(
			obb0.mExtents, obb0.mCenter, obb0.mRot,
			obb1.mExtents, obb1.mCenter, obb1.mRot,
			full_test);
	}

	CONTACT_API int OBBOBBContact(Point hitpos[16], float penedist[16], Point& hitnrm, const OBB& obb1, const OBB& obb2);

#endif // CTCOBBOBBOVERLAP_H
