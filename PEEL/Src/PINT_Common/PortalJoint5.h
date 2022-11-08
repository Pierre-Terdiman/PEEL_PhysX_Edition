#ifndef PORTAL_JOINT_5_H
#define PORTAL_JOINT_5_H

// Revisited implementation after changes in PhysX 5

#include "PxPortalJoint.h"
#include "Extensions\ExtJoint.h"
//#include "CmUtils.h"

namespace physx
{
struct PxPortalJointGeneratedValues;
namespace Ext
{
	struct PortalJointData : public JointData
	{
		PxTransform	relPose;
	};

	typedef JointT<PxPortalJoint, PortalJointData, PxPortalJointGeneratedValues> PortalJointT;

	class PortalJoint : public PortalJointT
	{
	public:
// PX_SERIALIZATION
								PortalJoint(PxBaseFlags baseFlags) : PortalJointT(baseFlags) {}
				void			resolveReferences(PxDeserializationContext& context);
		static	PortalJoint*	createObject(PxU8*& address, PxDeserializationContext& context)	{ return createJointObject<PortalJoint>(address, context);	}
		static	void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

								PortalJoint(const PxTolerancesScale& /*scale*/, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1);
		// PxPortalJoint
		virtual	void			setRelativePose(const PxTransform& pose)	PX_OVERRIDE;
		//~PxPortalJoint

		// PxConstraintConnector
		virtual PxConstraintSolverPrep getPrep() const	PX_OVERRIDE;
		//~PxConstraintConnector
	};
} // namespace Ext

}

#endif
