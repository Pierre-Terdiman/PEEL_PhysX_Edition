///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCSWEEPBOXSPHERE_H
#define CTCSWEEPBOXSPHERE_H

	CONTACT_API	bool SweepBoxSphere(const OBB& box, const Sphere& sphere, const Point& dir, float length, float& min_dist, Point& normal);
	CONTACT_API	bool SweepCapsuleBox(const LSS& lss, const OBB& box, const Point& dir, float length, float& min_dist, Point& normal);

#endif // CTCSWEEPBOXSPHERE_H

