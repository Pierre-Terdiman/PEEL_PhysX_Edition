#include "GearJoint.h"
#include "extensions/ExtConstraintHelper.h"
#include "PxPhysics.h"
#include "extensions/PxRevoluteJoint.h"
#include <stdio.h>

using namespace physx;
using namespace Ext;

PxGearJoint* physx::PxGearJointCreate(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PX_CHECK_AND_RETURN_NULL(localFrame0.isSane(), "PxGearJointCreate: local frame 0 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(localFrame1.isSane(), "PxGearJointCreate: local frame 1 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL((actor0 && actor0->is<PxRigidBody>()) || (actor1 && actor1->is<PxRigidBody>()), "PxGearJointCreate: at least one actor must be dynamic");
	PX_CHECK_AND_RETURN_NULL(actor0 != actor1, "PxGearJointCreate: actors must be different");

	GearJoint* j;
	PX_NEW_SERIALIZED(j, GearJoint)(physics.getTolerancesScale(), actor0, localFrame0, actor1, localFrame1);

	if(j->attachConstraint(physics, actor0, actor1))
		return j;

	PX_DELETE(j);
	return NULL;
}

bool GearJoint::setHinges(PxJoint* hinge0, PxJoint* hinge1)
{
	GearJointData* data = static_cast<GearJointData*>(mData);

	if(!hinge0 || !hinge1)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxGearJoint::setHinges: cannot pass null pointers to this function.");
		return false;
	}

	const PxType type0 = hinge0->getConcreteType();
	if(type0!=PxJointConcreteType::eREVOLUTE && type0!=PxJointConcreteType::eD6)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxGearJoint::setHinges: passed joint must be either a revolute joint or a D6 joint.");
		return false;
	}

	const PxType type1 = hinge1->getConcreteType();
	if(type1!=PxJointConcreteType::eREVOLUTE && type1!=PxJointConcreteType::eD6)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxGearJoint::setHinges: passed joint must be either a revolute joint or a D6 joint.");
		return false;
	}

	data->hingeJoint0 = hinge0;
	data->hingeJoint1 = hinge1;
	markDirty();
	return true;
}

void GearJoint::setGearRatio(float ratio)
{
	GearJointData* data = static_cast<GearJointData*>(mData);
	data->gearRatio = ratio;
	markDirty();
}

float GearJoint::getGearRatio() const
{
	const GearJointData* data = static_cast<const GearJointData*>(mData);
	return data->gearRatio;
}

static float angleDiff(float angle0, float angle1)
{
	const float diff = fmodf( angle1 - angle0 + PxPi, PxTwoPi) - PxPi;
	return diff < -PxPi ? diff + PxTwoPi : diff;
}

void GearJoint::updateError()
{
	GearJointData* data = static_cast<GearJointData*>(mData);

	PxJoint* hingeJoint0 = data->hingeJoint0;
	PxJoint* hingeJoint1 = data->hingeJoint1;
	if(!hingeJoint0 || !hingeJoint1)
		return;

	PxRigidActor* gearActor0;
	PxRigidActor* gearActor1;
	getActors(gearActor0, gearActor1);

	float Angle0 = 0.0f;
	float Sign0 = 0.0f;
	{
		const PxType type = hingeJoint0->getConcreteType();
		if(type==PxJointConcreteType::eREVOLUTE)
			Angle0 = static_cast<PxRevoluteJoint*>(hingeJoint0)->getAngle();
		else if(type==PxJointConcreteType::eD6)
			Angle0 = static_cast<PxD6Joint*>(hingeJoint0)->getTwistAngle();

		PxRigidActor* hingeActor0;
		PxRigidActor* hingeActor1;
		hingeJoint0->getActors(hingeActor0, hingeActor1);

		if(gearActor0==hingeActor0 || gearActor1==hingeActor0)
			Sign0 = -1.0f;
		else if(gearActor0==hingeActor1 || gearActor1==hingeActor1)
			Sign0 = 1.0f;
		else
			PX_ASSERT(0);
	}

	float Angle1 = 0.0f;
	float Sign1 = 0.0f;
	{
		const PxType type = hingeJoint1->getConcreteType();
		if(type==PxJointConcreteType::eREVOLUTE)
			Angle1 = -static_cast<PxRevoluteJoint*>(hingeJoint1)->getAngle();
		else if(type==PxJointConcreteType::eD6)
			Angle1 = -static_cast<PxD6Joint*>(hingeJoint1)->getTwistAngle();

		PxRigidActor* hingeActor0;
		PxRigidActor* hingeActor1;
		hingeJoint1->getActors(hingeActor0, hingeActor1);

		if(gearActor0==hingeActor0 || gearActor1==hingeActor0)
			Sign1 = -1.0f;
		else if(gearActor0==hingeActor1 || gearActor1==hingeActor1)
			Sign1 = 1.0f;
		else
			PX_ASSERT(0);
	}

	if(!mInitDone)
	{
		mInitDone = true;
		mPersistentAngle0 = Angle0;
		mPersistentAngle1 = Angle1;
	}

	const float travelThisFrame0 = angleDiff(Angle0, mPersistentAngle0);
	const float travelThisFrame1 = angleDiff(Angle1, mPersistentAngle1);
	mVirtualAngle0 += travelThisFrame0;
	mVirtualAngle1 += travelThisFrame1;

//	printf("travelThisFrame0: %f\n", travelThisFrame0);
//	printf("travelThisFrame1: %f\n", travelThisFrame1);
//	printf("ratio: %f\n", travelThisFrame1/travelThisFrame0);
	mPersistentAngle0 = Angle0;
	mPersistentAngle1 = Angle1;

	const float error = Sign0*mVirtualAngle0*data->gearRatio - Sign1*mVirtualAngle1;
//	printf("error: %f\n", error);

	data->error = error;
	markDirty();
}

PxConstraint* GearJoint::attachConstraint(PxPhysics& physics, PxRigidActor* actor0, PxRigidActor* actor1)
{
	mPxConstraint = physics.createConstraint(actor0, actor1, *this, sShaders, sizeof(GearJointData));
	return mPxConstraint;
}

void GearJoint::exportExtraData(PxSerializationContext& stream) const
{
	if(mData)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mData, sizeof(GearJointData));
	}
	stream.writeName(mName);
}

void GearJoint::importExtraData(PxDeserializationContext& context)
{
	if(mData)
		mData = context.readExtraData<GearJointData, PX_SERIAL_ALIGN>();
	context.readName(mName);
}

void GearJoint::resolveReferences(PxDeserializationContext& context)
{
	setPxConstraint(resolveConstraintPtr(context, getPxConstraint(), getConnector(), sShaders));

	GearJointData* data = static_cast<GearJointData*>(mData);
	context.translatePxBase(data->hingeJoint0);
	context.translatePxBase(data->hingeJoint1);
}

GearJoint* GearJoint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	GearJoint* obj = PX_PLACEMENT_NEW(address, GearJoint(PxBaseFlag::eIS_RELEASABLE));
	address += sizeof(GearJoint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

static void GearJointProject(const void* /*constantBlock*/, PxTransform& /*bodyAToWorld*/, PxTransform& /*bodyBToWorld*/, bool /*projectToA*/)
{
//	const GearJointData& data = *reinterpret_cast<const GearJointData*>(constantBlock);
}

static const bool gVizJointFrames = true;
static const bool gVizGearAxes = false;

static void GearJointVisualize(PxConstraintVisualizer& viz, const void* constantBlock, const PxTransform& body0Transform, const PxTransform& body1Transform, PxU32 flags)
{
	if(flags & PxConstraintVisualizationFlag::eLOCAL_FRAMES)
	{
		const GearJointData& data = *reinterpret_cast<const GearJointData*>(constantBlock);

		// Visualize joint frames
		PxTransform cA2w, cB2w;
		joint::computeJointFrames(cA2w, cB2w, data, body0Transform, body1Transform);
		if(gVizJointFrames)
			viz.visualizeJointFrames(cA2w, cB2w);

		if(gVizGearAxes)
		{
			const PxVec3 gearAxis0 = cA2w.rotate(PxVec3(1.0f, 0.0f, 0.0f)).getNormalized();
			const PxVec3 gearAxis1 = cB2w.rotate(PxVec3(1.0f, 0.0f, 0.0f)).getNormalized();
			viz.visualizeLine(body0Transform.p+gearAxis0, body0Transform.p, 0xff0000ff);
			viz.visualizeLine(body1Transform.p+gearAxis1, body1Transform.p, 0xff0000ff);
		}
	}
}

static PxU32 GearJointSolverPrep(Px1DConstraint* constraints,
	PxVec3& body0WorldOffset,
	PxU32 /*maxConstraints*/,
	PxConstraintInvMassScale& invMassScale,
	const void* constantBlock,
	const PxTransform& bA2w,
	const PxTransform& bB2w,
	bool /*useExtendedLimits*/,
	PxVec3& cA2wOut, PxVec3& cB2wOut)
{
	const GearJointData& data = *reinterpret_cast<const GearJointData*>(constantBlock);

	PxTransform cA2w, cB2w;
	joint::ConstraintHelper ch(constraints, invMassScale, cA2w, cB2w, body0WorldOffset, data, bA2w, bB2w);

	cA2wOut = cB2w.p;
	cB2wOut = cB2w.p;

	const PxVec3 gearAxis0 = cA2w.q.getBasisVector0();
	const PxVec3 gearAxis1 = cB2w.q.getBasisVector0();

	Px1DConstraint& con = constraints[0];
	con.linear0 = PxVec3(0.0f);
	con.linear1 = PxVec3(0.0f);
	con.angular0 = gearAxis0*data.gearRatio;
	con.angular1 = -gearAxis1;
	con.geometricError = -data.error;
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

PxConstraintShaderTable Ext::GearJoint::sShaders = { GearJointSolverPrep, GearJointProject, GearJointVisualize, PxConstraintFlag::Enum(0) };

