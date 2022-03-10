#ifndef EXT_RACK_AND_PINION_JOINT_H
#define EXT_RACK_AND_PINION_JOINT_H

#include "PxRackAndPinionJoint.h"
#include "Extensions\ExtJoint.h"
#include "CmUtils.h"

namespace physx
{
struct PxRackAndPinionJointGeneratedValues;
namespace Ext
{
	struct RackAndPinionJointData : public JointData
	{
		PxJoint*	hingeJoint;
		PxJoint*	prismaticJoint;
		float		ratio;
		float		px;
		float		vangle;
	};

	typedef Joint<PxRackAndPinionJoint, PxRackAndPinionJointGeneratedValues> RackAndPinionJointT;

	class RackAndPinionJoint : public RackAndPinionJointT
	{
	public:
// PX_SERIALIZATION
										RackAndPinionJoint(PxBaseFlags baseFlags) : RackAndPinionJointT(baseFlags) {}
		virtual		void				exportExtraData(PxSerializationContext& context) const;
					void				importExtraData(PxDeserializationContext& context);
					void				resolveReferences(PxDeserializationContext& context);
		static		RackAndPinionJoint*	createObject(PxU8*& address, PxDeserializationContext& context);
		static		void				getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

		RackAndPinionJoint(const PxTolerancesScale& /*scale*/, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1) :
			RackAndPinionJointT(PxJointConcreteType::eFIXED, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE, actor0, localFrame0, actor1, localFrame1, sizeof(RackAndPinionJointData), "RackAndPinionJointData")
		{
			RackAndPinionJointData* data = static_cast<RackAndPinionJointData*>(mData);
			data->hingeJoint = NULL;
			data->prismaticJoint = NULL;
			data->ratio = 1.0f;
			data->px = 0.0f;
			data->vangle = 0.0f;

			mVirtualAngle0 = 0.0f;
			mPersistentAngle0 = 0.0f;
			mInitDone = false;
		}

		// PxConstraintConnector
		virtual	void* prepareData()
		{
			updateError();
			return mData;
		}
		//~PxConstraintConnector

		// PxRackAndPinionJoint
		virtual	bool					setJoints(PxJoint* hinge, PxJoint* prismatic);
		virtual	void					setRatio(float ratio);
		virtual	float					getRatio()	const;
		virtual	bool					setData(PxU32 nbRackTeeth, PxU32 nbPinionTeeth, float rackLength);
		//~PxRackAndPinionJoint

				PxConstraint*			attachConstraint(PxPhysics& physics, PxRigidActor* actor0, PxRigidActor* actor1);
		
		virtual PxConstraintSolverPrep	getPrep() const { return sShaders.solverPrep;  }

	private:
				float					mVirtualAngle0;
				float					mPersistentAngle0;
				bool					mInitDone;

				void					updateError();

		static PxConstraintShaderTable sShaders;
	};
} // namespace Ext

}

#endif
