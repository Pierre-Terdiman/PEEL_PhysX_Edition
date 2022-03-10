#ifndef PX_GEAR_JOINT_H
#define PX_GEAR_JOINT_H
/** \addtogroup extensions
  @{
*/

#include "extensions/PxJoint.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

	class PxGearJoint;

	/**
	\brief Create a gear Joint.

	\param[in] physics		The physics SDK
	\param[in] actor0		An actor to which the joint is attached. NULL may be used to attach the joint to a specific point in the world frame
	\param[in] localFrame0	The position and orientation of the joint relative to actor0
	\param[in] actor1		An actor to which the joint is attached. NULL may be used to attach the joint to a specific point in the world frame
	\param[in] localFrame1	The position and orientation of the joint relative to actor1

	@see PxGearJoint
	*/
	PxGearJoint*	PxGearJointCreate(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1);

	/**
	\brief A joint that connects two existing revolute joints and constrains their relative angular velocity and position with respect to each other.

	@see PxGearJointCreate PxJoint
	*/
	class PxGearJoint : public PxJoint
	{
	public:

		/**
		\brief Set the hinge/revolute joints connected by the gear joint.

		The passed joints can be either PxRevoluteJoint or PxD6Joint. They cannot be null.

		Note that these joints are only used to compute the positional error correction term,
		used to adjust potential drift between jointed actors. The gear joint can run without
		calling this function, but in that case some visible overlap may develop over time between
		the teeth of the gear meshes.

		\param[in]	hinge0		The first hinge joint
		\param[in]	hinge1		The second hinge joint
		\return		true if success
		*/
		virtual	bool		setHinges(PxJoint* hinge0, PxJoint* hinge1)	= 0;

		/**
		\brief Set the desired gear ratio.

		For two gears with n0 and n1 teeth respectively, the gear ratio is n0/n1.

		\param[in]	ratio	Desired ratio between the two hinges.
		*/
		virtual	void		setGearRatio(float ratio)	= 0;

		/**
		\brief Get the gear ratio.

		\return		Current ratio
		*/
		virtual	float		getGearRatio()	const		= 0;

		virtual	void		updateError()				= 0;

		virtual	const char*	getConcreteTypeName() const { return "PxGearJoint"; }

	protected:

		PX_INLINE			PxGearJoint(PxType concreteType, PxBaseFlags baseFlags) : PxJoint(concreteType, baseFlags) {}

		PX_INLINE			PxGearJoint(PxBaseFlags baseFlags) : PxJoint(baseFlags)	{}

		virtual	bool		isKindOf(const char* name) const { return !::strcmp("PxGearJoint", name) || PxJoint::isKindOf(name);	}
	};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
