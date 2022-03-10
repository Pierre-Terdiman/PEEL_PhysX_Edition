#ifndef PORTAL_JOINT_H
#define PORTAL_JOINT_H

#include "PxPortalJoint.h"
#include "Extensions\ExtJoint.h"
#include "CmUtils.h"

namespace physx
{
struct PxPortalJointGeneratedValues;
namespace Ext
{
	struct PortalJointData : public JointData
	{
		PxTransform	relPose;
	};

	typedef Joint<PxPortalJoint, PxPortalJointGeneratedValues> PortalJointT;

	class PortalJoint : public PortalJointT
	{
	public:
// PX_SERIALIZATION
									PortalJoint(PxBaseFlags baseFlags) : PortalJointT(baseFlags) {}
		virtual		void			exportExtraData(PxSerializationContext& context) const;
					void			importExtraData(PxDeserializationContext& context);
					void			resolveReferences(PxDeserializationContext& context);
		static		PortalJoint*	createObject(PxU8*& address, PxDeserializationContext& context);
		static		void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

		PortalJoint(const PxTolerancesScale& /*scale*/, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1) :
			PortalJointT(PxJointConcreteType::eFIXED, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE, actor0, localFrame0, actor1, localFrame1, sizeof(PortalJointData), "PortalJointData")
		{
			PortalJointData* data = static_cast<PortalJointData*>(mData);

			data->relPose = PxTransform(PxIdentity);
		}

		virtual	void	setRelativePose(const PxTransform& pose);

		bool			attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1);
		
		static const PxConstraintShaderTable& getConstraintShaderTable() { return sShaders; }

		virtual PxConstraintSolverPrep getPrep() const { return sShaders.solverPrep;  }

	private:

		static PxConstraintShaderTable sShaders;

		PX_FORCE_INLINE PortalJointData& data() const				
		{	
			return *static_cast<PortalJointData*>(mData);
		}
	};
} // namespace Ext

}

#endif
