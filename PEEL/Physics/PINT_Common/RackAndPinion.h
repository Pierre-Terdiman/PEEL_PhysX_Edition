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
