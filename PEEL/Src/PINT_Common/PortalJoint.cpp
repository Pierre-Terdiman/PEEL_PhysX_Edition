#include "PortalJoint.h"
#include "Extensions\ExtConstraintHelper.h"
#include "PxPhysics.h"
#include "PxPortalJoint.h"
#include <stdio.h>

using namespace physx;
using namespace Ext;

PxPortalJoint* physx::PxPortalJointCreate(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PX_CHECK_AND_RETURN_NULL(localFrame0.isSane(), "PxPortalJointCreate: local frame 0 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(localFrame1.isSane(), "PxPortalJointCreate: local frame 1 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL((actor0 && actor0->is<PxRigidBody>()) || (actor1 && actor1->is<PxRigidBody>()), "PxPortalJointCreate: at least one actor must be dynamic");
	PX_CHECK_AND_RETURN_NULL(actor0 != actor1, "PxPortalJointCreate: actors must be different");

	PortalJoint* j;
	PX_NEW_SERIALIZED(j, PortalJoint)(physics.getTolerancesScale(), actor0, localFrame0, actor1, localFrame1);

	if(j->attach(physics, actor0, actor1))
		return j;

	PX_DELETE(j);
	return NULL;
}

bool PortalJoint::attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1)
{
	mPxConstraint = physics.createConstraint(actor0, actor1, *this, sShaders, sizeof(PortalJointData));
	return mPxConstraint!=NULL;
}

void PortalJoint::exportExtraData(PxSerializationContext& stream) const
{
	if(mData)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mData, sizeof(PortalJointData));
	}
	stream.writeName(mName);
}

void PortalJoint::importExtraData(PxDeserializationContext& context)
{
	if(mData)
		mData = context.readExtraData<PortalJointData, PX_SERIAL_ALIGN>();
	context.readName(mName);
}

void PortalJoint::resolveReferences(PxDeserializationContext& context)
{
	setPxConstraint(resolveConstraintPtr(context, getPxConstraint(), getConnector(), sShaders));	
}

PortalJoint* PortalJoint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	PortalJoint* obj = new (address) PortalJoint(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(PortalJoint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

void PortalJoint::setRelativePose(const PxTransform& pose)
{
	PortalJointData* data = static_cast<PortalJointData*>(mData);
	data->relPose = pose;
	markDirty();
}

static void PortalJointProject(const void* constantBlock, PxTransform& bodyAToWorld, PxTransform& bodyBToWorld, bool projectToA)
{
	const PortalJointData& data = *reinterpret_cast<const PortalJointData*>(constantBlock);
}

static void PortalJointVisualize(PxConstraintVisualizer& viz, const void* constantBlock, const PxTransform& body0Transform, const PxTransform& body1Transform, PxU32 flags)
{
	if(flags & PxConstraintVisualizationFlag::eLOCAL_FRAMES)
	{
		const PortalJointData& data = *reinterpret_cast<const PortalJointData*>(constantBlock);

		// Visualize joint frames
		PxTransform cA2w, cB2w;
		joint::computeJointFrames(cA2w, cB2w, data, body0Transform, body1Transform);
		viz.visualizeJointFrames(cA2w, cB2w);
	}
}

static PxU32 PortalJointSolverPrep(Px1DConstraint* constraints,
	PxVec3& body0WorldOffset,
	PxU32 /*maxConstraints*/,
	PxConstraintInvMassScale& invMassScale,
	const void* constantBlock,
	const PxTransform& bA2w,
	const PxTransform& bB2w,
	bool /*useExtendedLimits*/,
	PxVec3& cA2wOut, PxVec3& cB2wOut)
{
	const PortalJointData& data = *reinterpret_cast<const PortalJointData*>(constantBlock);

	PxTransform cA2w, cB2w;
	joint::ConstraintHelper ch(constraints, invMassScale, cA2w, cB2w, body0WorldOffset, data, bA2w, bB2w);

	cA2wOut = bA2w.p;
	cB2wOut = bB2w.p;

	if (cA2w.q.dot(cB2w.q)<0.0f)
		cB2w.q = -cB2w.q;

	const PxMat33 rot(data.relPose.q);

	const PxTransform expectedPose1 = data.relPose * bA2w;

	struct Local
	{
		static Px1DConstraint& getConstraint(joint::ConstraintHelper& ch)
		{
			Px1DConstraint& c = *ch.getConstraintRow();
			c.minImpulse = -PX_MAX_F32;
			c.maxImpulse = PX_MAX_F32;
			c.velocityTarget = 0.0f;
			c.forInternalUse = 0.0f;
			c.mods.bounce.restitution = 0.0f;
			c.mods.bounce.velocityThreshold = 0.0f;
			return c;
		}
	};

	if(1)
	{
		const PxQuat errorQ = expectedPose1.q * bB2w.q.getConjugate();
		const PxVec3 error(errorQ.x, errorQ.y, errorQ.z);

		for(PxU32 i=0;i<3;i++)
		{
			Px1DConstraint& con = Local::getConstraint(ch);
			con.flags = Px1DConstraintFlag::eOUTPUT_FORCE|Px1DConstraintFlag::eANGULAR_CONSTRAINT;
			con.solveHint = PxConstraintSolveHint::eROTATIONAL_EQUALITY;
			con.angular0 = con.linear0 = con.linear1 = PxVec3(0.0f);
			con.angular0[i] = 1.0f;
			con.angular1 = rot[i];
			con.geometricError = error.dot(rot[i]);
		}
	}

	if(1)
	{
		const PxVec3 error = expectedPose1.p - bB2w.p;

		for(PxU32 i=0;i<3;i++)
		{
			Px1DConstraint& con = Local::getConstraint(ch);
			con.flags = Px1DConstraintFlag::eOUTPUT_FORCE;
			con.solveHint = PxConstraintSolveHint::eEQUALITY;
			con.linear0 = con.angular0 = con.angular1 = PxVec3(0.0f);
			con.linear0[i] = 1.0f;
			con.linear1 = rot[i];
			con.geometricError = error.dot(rot[i]);
		}
	}

	return ch.getCount();
}

PxConstraintShaderTable Ext::PortalJoint::sShaders = { PortalJointSolverPrep, PortalJointProject, PortalJointVisualize, PxConstraintFlag::Enum(0) };

