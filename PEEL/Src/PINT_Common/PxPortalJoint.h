#ifndef PX_PORTAL_JOINT_H
#define PX_PORTAL_JOINT_H
/** \addtogroup extensions
  @{
*/

#include "extensions/PxJoint.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

	class PxPortalJoint;

	PxPortalJoint*	PxPortalJointCreate(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1);

	// PT: an unofficial portal joint experiment
	class PxPortalJoint : public PxJoint
	{
	public:

		virtual	void		setRelativePose(const PxTransform& pose)	= 0;

		virtual	const char*	getConcreteTypeName() const { return "PxPortalJoint"; }

	protected:

		PX_INLINE			PxPortalJoint(PxType concreteType, PxBaseFlags baseFlags) : PxJoint(concreteType, baseFlags) {}

		PX_INLINE			PxPortalJoint(PxBaseFlags baseFlags) : PxJoint(baseFlags)	{}

		virtual	bool		isKindOf(const char* name) const { return !::strcmp("PxPortalJoint", name) || PxJoint::isKindOf(name);	}
	};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
