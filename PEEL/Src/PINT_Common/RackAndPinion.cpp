#include "RackAndPinion.h"
#include "extensions/ExtConstraintHelper.h"
#include "PxPhysics.h"
#include "extensions/PxRevoluteJoint.h"
#include "extensions/PxPrismaticJoint.h"
#include <stdio.h>

using namespace physx;
using namespace Ext;

PxRackAndPinionJoint* physx::PxRackAndPinionJointCreate(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PX_CHECK_AND_RETURN_NULL(localFrame0.isSane(), "PxRackAndPinionJointCreate: local frame 0 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(localFrame1.isSane(), "PxRackAndPinionJointCreate: local frame 1 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL((actor0 && actor0->is<PxRigidBody>()) || (actor1 && actor1->is<PxRigidBody>()), "PxRackAndPinionJointCreate: at least one actor must be dynamic");
	PX_CHECK_AND_RETURN_NULL(actor0 != actor1, "PxRackAndPinionJointCreate: actors must be different");

	RackAndPinionJoint* j;
	PX_NEW_SERIALIZED(j, RackAndPinionJoint)(physics.getTolerancesScale(), actor0, localFrame0, actor1, localFrame1);

	if(j->attachConstraint(physics, actor0, actor1))
		return j;

	PX_DELETE(j);
	return NULL;
}

void RackAndPinionJoint::setRatio(float ratio)
{
	RackAndPinionJointData* data = reinterpret_cast<RackAndPinionJointData*>(mData);
	data->ratio = ratio;
	markDirty();
}

float RackAndPinionJoint::getRatio() const
{
	RackAndPinionJointData* data = reinterpret_cast<RackAndPinionJointData*>(mData);
	return data->ratio;
}

bool RackAndPinionJoint::setData(PxU32 nbRackTeeth, PxU32 nbPinionTeeth, float rackLength)
{
	if(!nbRackTeeth)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxRackAndPinionJoint::setData: nbRackTeeth cannot be zero.");
		return false;
	}

	if(!nbPinionTeeth)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxRackAndPinionJoint::setData: nbPinionTeeth cannot be zero.");
		return false;
	}

	if(rackLength<=0.0f)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxRackAndPinionJoint::setData: rackLength must be positive.");
		return false;
	}

	RackAndPinionJointData* data = reinterpret_cast<RackAndPinionJointData*>(mData);
	data->ratio = (PxTwoPi*nbRackTeeth)/(rackLength*nbPinionTeeth);
	markDirty();
	return true;
}

bool RackAndPinionJoint::setJoints(PxJoint* hinge, PxJoint* prismatic)
{
	RackAndPinionJointData* data = static_cast<RackAndPinionJointData*>(mData);

	if(!hinge || !prismatic)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxRackAndPinionJoint::setJoints: cannot pass null pointers to this function.");
		return false;
	}

	const PxType type0 = hinge->getConcreteType();
	if(type0!=PxJointConcreteType::eREVOLUTE && type0!=PxJointConcreteType::eD6)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxRackAndPinionJoint::setJoints: passed hinge joint must be either a revolute joint or a D6 joint.");
		return false;
	}

	const PxType type1 = prismatic->getConcreteType();
	if(type1!=PxJointConcreteType::ePRISMATIC && type1!=PxJointConcreteType::eD6)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxRackAndPinionJoint::setJoints: passed prismatic joint must be either a prismatic joint or a D6 joint.");
		return false;
	}

	data->hingeJoint = hinge;
	data->prismaticJoint = prismatic;
	markDirty();
	return true;
}

static float angleDiff(float angle0, float angle1)
{
	const float diff = fmodf( angle1 - angle0 + PxPi, PxTwoPi) - PxPi;
	return diff < -PxPi ? diff + PxTwoPi : diff;
}

void RackAndPinionJoint::updateError()
{
	RackAndPinionJointData* data = static_cast<RackAndPinionJointData*>(mData);

	PxJoint* hingeJoint = data->hingeJoint;
	PxJoint* prismaticJoint = data->prismaticJoint;
	if(!hingeJoint || !prismaticJoint)
		return;

	PxRigidActor* rackActor0;
	PxRigidActor* rackActor1;
	getActors(rackActor0, rackActor1);

	float Angle0 = 0.0f;
	float Sign0 = 0.0f;
	{
		const PxType type = hingeJoint->getConcreteType();
		if(type==PxJointConcreteType::eREVOLUTE)
			Angle0 = static_cast<PxRevoluteJoint*>(hingeJoint)->getAngle();
		else if(type==PxJointConcreteType::eD6)
			Angle0 = static_cast<PxD6Joint*>(hingeJoint)->getTwistAngle();

		PxRigidActor* hingeActor0;
		PxRigidActor* hingeActor1;
		hingeJoint->getActors(hingeActor0, hingeActor1);

		if(rackActor0==hingeActor0 || rackActor1==hingeActor0)
			Sign0 = -1.0f;
		else if(rackActor0==hingeActor1 || rackActor1==hingeActor1)
			Sign0 = 1.0f;
		else
			PX_ASSERT(0);
	}

	if(!mInitDone)
	{
		mInitDone = true;
		mPersistentAngle0 = Angle0;
	}

	const float travelThisFrame0 = angleDiff(Angle0, mPersistentAngle0);
	mVirtualAngle0 += travelThisFrame0;
//	printf("mVirtualAngle0: %f\n", mVirtualAngle0);

	mPersistentAngle0 = Angle0;

	float px = 0.0f;
	float Sign1 = 0.0f;
	{
		const PxType type = prismaticJoint->getConcreteType();
		if(type==PxJointConcreteType::ePRISMATIC)
			px = static_cast<PxPrismaticJoint*>(prismaticJoint)->getPosition();
		else if(type==PxJointConcreteType::eD6)
			px = prismaticJoint->getRelativeTransform().p.x;

		PxRigidActor* prismaticActor0;
		PxRigidActor* prismaticActor1;
		prismaticJoint->getActors(prismaticActor0, prismaticActor1);

		if(rackActor0==prismaticActor0 || rackActor1==prismaticActor0)
			Sign1 = -1.0f;
		else if(rackActor0==prismaticActor1 || rackActor1==prismaticActor1)
			Sign1 = 1.0f;
		else
			PX_ASSERT(0);
	}

//	printf("px: %f\n", px);
	data->px = Sign1*px;
	data->vangle = Sign0*mVirtualAngle0;
	markDirty();
}

PxConstraint* RackAndPinionJoint::attachConstraint(PxPhysics& physics, PxRigidActor* actor0, PxRigidActor* actor1)
{
	mPxConstraint = physics.createConstraint(actor0, actor1, *this, sShaders, sizeof(RackAndPinionJointData));
	return mPxConstraint;
}

void RackAndPinionJoint::exportExtraData(PxSerializationContext& stream) const
{
	if(mData)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mData, sizeof(RackAndPinionJointData));
	}
	stream.writeName(mName);
}

void RackAndPinionJoint::importExtraData(PxDeserializationContext& context)
{
	if(mData)
		mData = context.readExtraData<RackAndPinionJointData, PX_SERIAL_ALIGN>();
	context.readName(mName);
}

void RackAndPinionJoint::resolveReferences(PxDeserializationContext& context)
{
	setPxConstraint(resolveConstraintPtr(context, getPxConstraint(), getConnector(), sShaders));

	RackAndPinionJointData* data = static_cast<RackAndPinionJointData*>(mData);
	context.translatePxBase(data->hingeJoint);
	context.translatePxBase(data->prismaticJoint);
}

RackAndPinionJoint* RackAndPinionJoint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	RackAndPinionJoint* obj = PX_PLACEMENT_NEW(address, RackAndPinionJoint(PxBaseFlag::eIS_RELEASABLE));
	address += sizeof(RackAndPinionJoint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

static void RackAndPinionJointProject(const void* /*constantBlock*/, PxTransform& /*bodyAToWorld*/, PxTransform& /*bodyBToWorld*/, bool /*projectToA*/)
{
//	const RackAndPinionJointData& data = *reinterpret_cast<const RackAndPinionJointData*>(constantBlock);
}

static void RackAndPinionJointVisualize(PxConstraintVisualizer& viz, const void* constantBlock, const PxTransform& body0Transform, const PxTransform& body1Transform, PxU32 flags)
{
	if(flags & PxConstraintVisualizationFlag::eLOCAL_FRAMES)
	{
		const RackAndPinionJointData& data = *reinterpret_cast<const RackAndPinionJointData*>(constantBlock);

		// Visualize joint frames
		PxTransform cA2w, cB2w;
		joint::computeJointFrames(cA2w, cB2w, data, body0Transform, body1Transform);
		viz.visualizeJointFrames(cA2w, cB2w);
	}

	if(0)
	{
		const RackAndPinionJointData& data = *reinterpret_cast<const RackAndPinionJointData*>(constantBlock);

		if(0)
		{
			PxTransform cA2w, cB2w;
			joint::computeJointFrames(cA2w, cB2w, data, body0Transform, body1Transform);

			const PxVec3 gearAxis0 = cA2w.q.getBasisVector0();
			const PxVec3 rackPrismaticAxis = cB2w.q.getBasisVector0();
			viz.visualizeLine(cA2w.p, cA2w.p + gearAxis0, 0xff0000ff);
			viz.visualizeLine(cB2w.p, cB2w.p + rackPrismaticAxis, 0xff0000ff);
		}
	}
}

static PxU32 RackAndPinionJointSolverPrep(Px1DConstraint* constraints,
	PxVec3& body0WorldOffset,
	PxU32 /*maxConstraints*/,
	PxConstraintInvMassScale& invMassScale,
	const void* constantBlock,
	const PxTransform& bA2w,
	const PxTransform& bB2w,
	bool /*useExtendedLimits*/,
	PxVec3& cA2wOut, PxVec3& cB2wOut)
{
	const RackAndPinionJointData& data = *reinterpret_cast<const RackAndPinionJointData*>(constantBlock);

	PxTransform cA2w, cB2w;
	joint::ConstraintHelper ch(constraints, invMassScale, cA2w, cB2w, body0WorldOffset, data, bA2w, bB2w);

	cA2wOut = cB2w.p;
	cB2wOut = cB2w.p;

	const PxVec3 gearAxis = cA2w.q.getBasisVector0();
	const PxVec3 rackPrismaticAxis = cB2w.q.getBasisVector0();

	PxVec3 velocity = gearAxis.cross(cB2w.p - cA2w.p);
	const float dp = velocity.dot(rackPrismaticAxis);
	const float Coeff = PxSign(dp);

	Px1DConstraint& con = constraints[0];
	con.linear0 = PxVec3(0.0f);
	con.linear1 = rackPrismaticAxis * data.ratio*Coeff;
	con.angular0 = gearAxis;
	con.angular1 = PxVec3(0.0f);
	con.geometricError = -Coeff*data.px*data.ratio - data.vangle;
	con.minImpulse = -PX_MAX_F32;
	con.maxImpulse = PX_MAX_F32;
	con.velocityTarget = 0.f;
	con.forInternalUse = 0.f;
	con.solveHint = 0;
	con.flags = Px1DConstraintFlag::eOUTPUT_FORCE|Px1DConstraintFlag::eANGULAR_CONSTRAINT;
	con.mods.bounce.restitution = 0.0f;
	con.mods.bounce.velocityThreshold = 0.0f;
	return 1;
}

PxConstraintShaderTable Ext::RackAndPinionJoint::sShaders = { RackAndPinionJointSolverPrep, RackAndPinionJointProject, RackAndPinionJointVisualize, PxConstraintFlag::Enum(0) };

