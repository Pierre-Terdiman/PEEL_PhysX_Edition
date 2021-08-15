//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2020 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef PX_RACK_AND_PINION_JOINT_H
#define PX_RACK_AND_PINION_JOINT_H
/** \addtogroup extensions
  @{
*/

#include "extensions/PxJoint.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

	class PxRackAndPinionJoint;

	/**
	\brief Create a rack & pinion Joint.

	\param[in] physics		The physics SDK
	\param[in] actor0		An actor to which the joint is attached. NULL may be used to attach the joint to a specific point in the world frame
	\param[in] localFrame0	The position and orientation of the joint relative to actor0
	\param[in] actor1		An actor to which the joint is attached. NULL may be used to attach the joint to a specific point in the world frame
	\param[in] localFrame1	The position and orientation of the joint relative to actor1

	@see PxRackAndPinionJoint
	*/
	PxRackAndPinionJoint*	PxRackAndPinionJointCreate(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1);

	/**
	\brief A joint that connects an existing revolute joint to an existing prismatic joint,
	and constrains their relative angular/linear velocity and position with respect to each other.

	@see PxRackAndPinionJointCreate PxJoint
	*/
	class PxRackAndPinionJoint : public PxJoint
	{
	public:

		/**
		\brief Set the hinge & prismatic joints connected by the rack & pinion joint.

		The passed hinge joint can be either PxRevoluteJoint or PxD6Joint. It cannot be null.
		The passed prismatic joint can be either PxPrismaticJoint or PxD6Joint. It cannot be null.

		Note that these joints are only used to compute the positional error correction term,
		used to adjust potential drift between jointed actors. The rack & pinion joint can run without
		calling this function, but in that case some visible overlap may develop over time between
		the teeth of the rack & pinion meshes.

		\param[in]	hinge		The hinge joint (pinion)
		\param[in]	prismatic	The prismatic joint (rack)
		\return		true if success
		*/
		virtual	bool		setJoints(PxJoint* hinge, PxJoint* prismatic)	= 0;

		/**
		\brief Set the desired ratio directly.

		\param[in]	ratio	Desired ratio between the hinge and the prismatic.
		*/
		virtual	void		setRatio(float ratio)	= 0;

		/**
		\brief Get the ratio.

		\return		Current ratio
		*/
		virtual	float		getRatio()	const		= 0;

		/**
		\brief Set the desired ratio indirectly.

		This is a simple helper function that computes the ratio from passed data:

		ratio = (PI*2*nbRackTeeth)/(rackLength*nbPinionTeeth)

		\param[in]	nbRackTeeth		Number of teeth on the rack (cannot be zero)
		\param[in]	nbPinionTeeth	Number of teeth on the pinion (cannot be zero)
		\param[in]	rackLength		Length of the rack
		\return		true if success
		*/
		virtual	bool		setData(PxU32 nbRackTeeth, PxU32 nbPinionTeeth, float rackLength)	= 0;

		virtual	void		updateError()				= 0;

		virtual	const char*	getConcreteTypeName() const { return "PxRackAndPinionJoint"; }

	protected:

		PX_INLINE			PxRackAndPinionJoint(PxType concreteType, PxBaseFlags baseFlags) : PxJoint(concreteType, baseFlags) {}

		PX_INLINE			PxRackAndPinionJoint(PxBaseFlags baseFlags) : PxJoint(baseFlags)	{}

		virtual	bool		isKindOf(const char* name) const { return !::strcmp("PxRackAndPinionJoint", name) || PxJoint::isKindOf(name);	}
	};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
