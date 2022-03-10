#ifndef EXT_GEAR_JOINT_H
#define EXT_GEAR_JOINT_H

#include "PxGearJoint.h"
#include "Extensions\ExtJoint.h"
#include "CmUtils.h"

namespace physx
{
struct PxGearJointGeneratedValues;
namespace Ext
{
	struct GearJointData : public JointData
	{
		PxJoint*	hingeJoint0;
		PxJoint*	hingeJoint1;
		float		gearRatio;
		float		error;
	};

	typedef Joint<PxGearJoint, PxGearJointGeneratedValues> GearJointT;

	class GearJoint : public GearJointT
	{
	public:
// PX_SERIALIZATION
									GearJoint(PxBaseFlags baseFlags) : GearJointT(baseFlags) {}
		virtual		void			exportExtraData(PxSerializationContext& context) const;
					void			importExtraData(PxDeserializationContext& context);
					void			resolveReferences(PxDeserializationContext& context);
		static		GearJoint*		createObject(PxU8*& address, PxDeserializationContext& context);
		static		void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

		GearJoint(const PxTolerancesScale& /*scale*/, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1) :
			GearJointT(PxJointConcreteType::eFIXED, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE, actor0, localFrame0, actor1, localFrame1, sizeof(GearJointData), "GearJointData")
		{
			GearJointData* data = static_cast<GearJointData*>(mData);
			data->hingeJoint0 = NULL;
			data->hingeJoint1 = NULL;
			data->gearRatio = 0.0f;
			data->error = 0.0f;

			mVirtualAngle0 = 0.0f;
			mVirtualAngle1 = 0.0f;
			mPersistentAngle0 = 0.0f;
			mPersistentAngle1 = 0.0f;
			mInitDone = false;
		}

		// PxConstraintConnector
		virtual	void* prepareData()
		{
			updateError();
			return mData;
		}
		//~PxConstraintConnector

		// PxGearJoint
		virtual	bool					setHinges(PxJoint* hinge0, PxJoint* hinge1);
		virtual	void					setGearRatio(float ratio);
		virtual	float					getGearRatio()	const;
		//~PxGearJoint

				PxConstraint*			attachConstraint(PxPhysics& physics, PxRigidActor* actor0, PxRigidActor* actor1);
		
		virtual PxConstraintSolverPrep	getPrep() const { return sShaders.solverPrep;  }

	private:
				float					mVirtualAngle0;
				float					mVirtualAngle1;
				float					mPersistentAngle0;
				float					mPersistentAngle1;
				bool					mInitDone;

				void					updateError();

		static PxConstraintShaderTable sShaders;
	};
} // namespace Ext

}

#endif
