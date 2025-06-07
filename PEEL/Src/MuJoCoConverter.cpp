///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MuJoCoConverter.h"
#include "PintObjectsManager.h"
#include "PintShapeRenderer.h"
#include "TestScenesHelpers.h"

#include "foundation/PxTransform.h"
using namespace physx;

// TODO: revisit & cleanup all that stuff

///////////////////////////////////////////////////////////////////////////////

MatrixStack::MatrixStack() : mNbMatrices(0)
{
}

void MatrixStack::Transform(Point& pos, Quat& rot)
{
	if(!mNbMatrices)
		return;

	Matrix4x4 Combo;
	Combo.Identity();
/*	for(u32 i=0;i<mNbMatrices;i++)
	{
		Combo *= mMat[i];
	}*/
	Combo = mMat[mNbMatrices-1];

	Matrix4x4 M = rot;
	M.SetTrans(pos);

	Matrix4x4 tmp0 = Combo * M;
	Matrix4x4 tmp1 = M * Combo;
	PR FinalPose = tmp0;
	pos = FinalPose.mPos;
	rot = FinalPose.mRot;
}

void MatrixStack::Push(const Point& pos, const Quat& rot)
{
	Matrix4x4 M = rot;
	M.SetTrans(pos);
	ASSERT(mNbMatrices<64);
	mMat[mNbMatrices++] = M;
}

void MatrixStack::Pop()
{
	mNbMatrices--;
}

///////////////////////////////////////////////////////////////////////////////

// Reminiscent of the MAX-to-D3D conversion in Flexporter
static Point inline_ ToD3D(const Point& p)	{ return Point(p.x, p.z, p.y);				}
//static Quat inline_ ToD3D(const Quat& q)	{ return Quat(q.w, q.p.x, q.p.z, q.p.y);	}
//static Quat inline_ ToD3D(const Quat& q)	{ return Quat(q.w, q.p.x, q.p.z, q.p.y).GetConjugate();	}
static Quat inline_ ToD3D(const Quat& q)
{
	Quat q2(q.w, q.p.x, q.p.z, q.p.y);
	q2.Normalize();
	return q2.GetConjugate();
}

MuJoCoConverter::MuJoCoConverter(Pint& pint) :
	mPint				(pint),
	mModel				(null),
	mSuperAggregate		(null),
	mAggregate			(null),
	mDesc				(null),
	mJointMapping		(null),
	mStiffness			(0.0f),
	mDamping			(0.0f),
	mUseRCA				(true),
	mUseRobotAggregate	(true),
	//mUseWorldAggregate	(false),
	mShowCollisionShapes(false),
	mShowVisualMeshes	(true)
{
}

MuJoCoConverter::~MuJoCoConverter()
{
	ReleaseShapes();
}

Quat MuJoCoConverter::FromEuler(float x, float y, float z) const
{
	// TODO: revisit this

	//EulerAngles EA(x, y, z);
	//Matrix3x3 M = EA;

	Matrix3x3 RX;	RX.RotX(x * DEGTORAD);
	Matrix3x3 RY;	RY.RotY(y * DEGTORAD);
	Matrix3x3 RZ;	RZ.RotZ(z * DEGTORAD);

	const Matrix3x3 M = RX * RY * RZ;
	return M;
}

void MuJoCoConverter::AddShape(PINT_SHAPE_CREATE* new_shape)
{
	if(mShapes.Size())
	{
		PINT_SHAPE_CREATE* Shape = mShapes[mShapes.Size() - 1];
		Shape->SetNext(new_shape);
	}

	mShapes.PushBack(new_shape);
}

void MuJoCoConverter::ReleaseShapes()
{
	const u32 NbShapes = mShapes.Size();
	for(u32 i=0; i<NbShapes; i++)
	{
		PINT_SHAPE_CREATE* CurrentShape = mShapes[i];
		DELETESINGLE(CurrentShape);
	}
	mShapes.Clear();
}

bool MuJoCoConverter::StartModel(bool fixed_base)
{
	if(!mUseRCA)
		return true;

	if(mUseRobotAggregate)
	{
		mAggregate = null;

		PintCaps Caps;
		mPint.GetCaps(Caps);
		if(!Caps.mSupportAggregates)
			printf("Plugin %s does not support aggregates - skipping\n", mPint.GetName());
		else
			mAggregate = mPint.CreateAggregate(128, false);	// ### size? self collision?
	}

	mModel = mPint.CreateRCArticulation(PINT_RC_ARTICULATION_CREATE(fixed_base));
	return mModel != null;
}

void MuJoCoConverter::EndModel()
{
	ASSERT(!mDesc);
	ASSERT(!mBodies.Size());
	if(mUseRCA)
	{
		if(mAggregate)
		{
			/*if(mSuperAggregate)
			{
				mPint.AddRCArticulationToAggregate(mModel, mAggregate);
			}
			else*/
			{
				mPint.AddRCArticulationToAggregate(mModel, mAggregate);
				mPint.AddAggregateToScene(mAggregate);
			}
		}
		else
		{
			if(mSuperAggregate)
			{
				mPint.AddRCArticulationToAggregate(mModel, mSuperAggregate);
			}
			else
			{
				mPint.AddRCArticulationToScene(mModel);
			}
		}
	}
}

void MuJoCoConverter::StartBody(const char* name, const Point& pos, const Quat& rot, float mass)
{
	if(mDesc)
		CreateBody();

	BodyAndJoint* Desc = ICE_NEW(BodyAndJoint);
	Desc->mBody.mName		= name;
	Desc->mBody.mMass		= mass;
	//Desc->mBody.mMass		= 1.0f;
	Desc->mBody.mPosition	= ToD3D(pos);
	Desc->mBody.mRotation	= ToD3D(rot);

	mDesc = Desc;
}

void MuJoCoConverter::SetInertia(float mass, const Point& pos, const Quat& rot, const Point& inertia)
{
	ASSERT(mDesc);
	mDesc->mBody.mMass							= mass;
	mDesc->mBody.mExplicitMassLocalPose.mPos	= ToD3D(pos);
	mDesc->mBody.mExplicitMassLocalPose.mRot	= ToD3D(rot);
	mDesc->mBody.mExplicitInertiaTensor			= ToD3D(inertia);
}

void MuJoCoConverter::SetHingeJoint(const char* name, const Point& axis, float min_limit, float max_limit)
{
	ASSERT(mDesc);
	mDesc->mJointName	= name;
	mDesc->mType		= PINT_JOINT_HINGE;
	mDesc->mAxis		= axis;
	mDesc->mMin			= min_limit;
	mDesc->mMax			= max_limit;
}

void MuJoCoConverter::CreateBody()
{
	if(mShapes.Size())
	{
		PINT_SHAPE_CREATE* Shapes = mShapes[0];
		mDesc->mBody.SetShape(Shapes);
	}

	PintActorHandle ParentHandle = null;
	if(mBodies.Size())
		ParentHandle = mBodies[mBodies.Size() - 1];

	PintActorHandle ActorHandle;
	if(mUseRCA)
	{
		PINT_RC_ARTICULATED_BODY_CREATE ArticulatedDesc;
		ArticulatedDesc.mParent	= ParentHandle;
		//ArticulatedDesc.mLocalPivot0;
		//ArticulatedDesc.mLocalPivot1;
		ArticulatedDesc.mJointType = PINT_JOINT_FIXED;
		ArticulatedDesc.mJointType = PINT_JOINT_HINGE;	// ### only these for now
		ArticulatedDesc.mAxisIndex = X_;
		ArticulatedDesc.mFrictionCoeff = 0.0f;
		//ArticulatedDesc.mFrictionCoeff = 1000.0f;

		ArticulatedDesc.mJointType	= mDesc->mType;
		ArticulatedDesc.mMinLimit	= mDesc->mMin;
		ArticulatedDesc.mMaxLimit	= mDesc->mMax;
		//ArticulatedDesc.mLocalPivot1.mRot = ShortestRotation(mDesc->mAxis, Point(1.0f, 0.0f, 0.0f));

		if(ParentHandle)
		{
			if(mStiffness!=0.0f || mDamping!=0.0f)
			{
				ArticulatedDesc.mMotorFlags = PINT_MOTOR_POSITION;
				ArticulatedDesc.mTargetPos = 0.0f;
				ArticulatedDesc.mMotor.mStiffness = mStiffness;
				ArticulatedDesc.mMotor.mDamping = mDamping;
			}

			const Matrix4x4& ParentM = mMS.mMat[mMS.mNbMatrices-1];
			/*
			Point p = Point(ParentM.GetTrans());
			mCurrentBody->mPosition += p;
			//mMS.Transform(mCurrentBody->mPosition, mCurrentBody->mRotation);
			*/

			//mMS.Transform(mCurrentBody->mPosition, mCurrentBody->mRotation);
			//{
				Matrix4x4 M = mDesc->mBody.mRotation;
				//M.Invert();
				M.SetTrans(mDesc->mBody.mPosition);

				Matrix4x4 tmp0 = ParentM * M;
				Matrix4x4 tmp1 = M * ParentM;

				PR FinalPose = tmp1;
				mDesc->mBody.mPosition = FinalPose.mPos;
				mDesc->mBody.mRotation = FinalPose.mRot;


//Matrix4x4 invM;
//InvertPRMatrix(invM, M);
// JointFrame0 = LocalPivot0 * ChildPose
// JointFrame1 = LocalPivot1 * ParentPose
//LocalPivot0 = inv(ChildPose)
//LocalPivot1 = inv(ParentPose)
Matrix4x4 inv;
InvertPRMatrix(inv, ParentM);
ArticulatedDesc.mLocalPivot0 = tmp1 * inv;
if(1)	// ###needed?
{
InvertPRMatrix(inv, FinalPose);
ArticulatedDesc.mLocalPivot1 = tmp1 * inv;
}
// M = LocalPivot0 * ChildPose <=> LocalPivot0 = M * inv(ChildPose)

//### not sure about that one either
const Quat AxisRot = ShortestRotation(mDesc->mAxis, Point(1.0f, 0.0f, 0.0f));
ArticulatedDesc.mLocalPivot0.mRot *= AxisRot;
ArticulatedDesc.mLocalPivot1.mRot *= AxisRot;



			//}



#ifdef REMOVED
			const Matrix4x4& ParentM = mMS.mMat[mMS.mNbMatrices-1];

			//mMS.Transform(mCurrentBody->mPosition, mCurrentBody->mRotation);
			{
				Matrix4x4 M;// = rot;
				M.Identity();
				M.SetTrans(mCurrentBody->mPosition);

				Matrix4x4 tmp0 = ParentM * M;
				Matrix4x4 tmp1 = M * ParentM;
				PR FinalPose = tmp0;
				mCurrentBody->mPosition = FinalPose.mPos;
				//mCurrentBody->mRotation = FinalPose.mRot;
			}

			{
			//Matrix4x4 ParentM = mMS.mMat[mMS.mNbMatrices-1];
			//ParentM.SetTrans(0.0f, 0.0f, 0.0f, 0.0f);
				Matrix4x4 M = mCurrentBody->mRotation;
				//M.SetTrans(mCurrentBody->mPosition);

				Matrix4x4 tmp0 = ParentM * M;
				Matrix4x4 tmp1 = M * ParentM;
				PR FinalPose = tmp0;
				//mCurrentBody->mPosition = FinalPose.mPos;
				mCurrentBody->mRotation = FinalPose.mRot;
			}

			const Point pt = (Point(ParentM.GetTrans()) + mCurrentBody->mPosition)*0.5f;
			ArticulatedDesc.mLocalPivot0.mPos	= pt - Point(ParentM.GetTrans());
			ArticulatedDesc.mLocalPivot1.mPos	= pt - mCurrentBody->mPosition;

			ArticulatedDesc.mLocalPivot0.mRot = mCurrentBody->mRotation;
			ArticulatedDesc.mLocalPivot1.mRot = mCurrentBody->mRotation;
			if(0)
			{
				Matrix3x3 test;
				test.RotY(45.0f * DEGTORAD);
				ArticulatedDesc.mLocalPivot0.mRot = test;
				ArticulatedDesc.mLocalPivot1.mRot = test;
			}
	//		mCurrentBody->mPosition.Zero();
	//		mCurrentBody->mRotation.Identity();
	//		mCurrentBody->mPosition = Point(10.0f, 0.0f, 0.0f);

			{
	//			ArticulatedDesc.mLocalPivot1.mPos = mCurrentBody->mPosition;
	//			ArticulatedDesc.mLocalPivot1.mRot = mCurrentBody->mRotation;

	//mMS.Transform(mCurrentBody->mPosition, mCurrentBody->mRotation);
	//			ArticulatedDesc.mLocalPivot0.mPos = mCurrentBody->mPosition;
	//mMS.Transform(mCurrentBody->mPosition, mCurrentBody->mRotation);
	//			ArticulatedDesc.mLocalPivot0.mRot = mCurrentBody->mRotation;

	//			mCurrentBody->mPosition.Zero();
	//			mCurrentBody->mRotation.Identity();
			}

	//		ArticulatedDesc.mLocalPivot0.mPos = mCurrentBody->mPosition - Point(ParentM.GetTrans());
			//ArticulatedDesc.mLocalPivot1 = mCurrentBody->mPosition
#endif
		}
		ActorHandle = mPint.CreateRCArticulatedObject(mDesc->mBody, ArticulatedDesc, mModel);

		// We have a mapping here between the joint's name (mDesc->mName) and the corresponding body owning that joint (ActorHandle).
		// We will need this to update the actuators. However the trick is that we have a one-to-N mapping here in PEEL, since the
		// same name corresponds to a different joint/body in each loaded plugin (e.g. PhysX / MuJoCo).
		//printf("%s\n", mDesc->mName);
		if(mJointMapping && mDesc->mJointName)
			mJointMapping->RegisterJointMapping(mDesc->mJointName, ActorHandle);
	}
	else
	{
		if(ParentHandle)
		{
			const Matrix4x4& ParentM = mMS.mMat[mMS.mNbMatrices-1];
			/*
			Point p = Point(ParentM.GetTrans());
			mCurrentBody->mPosition += p;
			//mMS.Transform(mCurrentBody->mPosition, mCurrentBody->mRotation);
			*/

			//mMS.Transform(mCurrentBody->mPosition, mCurrentBody->mRotation);
			{
				Matrix4x4 M = mDesc->mBody.mRotation;
				//M.Invert();
				M.SetTrans(mDesc->mBody.mPosition);

				Matrix4x4 tmp0 = ParentM * M;
				Matrix4x4 tmp1 = M * ParentM;
				PR FinalPose = tmp1;
				mDesc->mBody.mPosition = FinalPose.mPos;
				mDesc->mBody.mRotation = FinalPose.mRot;
			}
		}
		ActorHandle = CreatePintObject(mPint, mDesc->mBody);
	}

	BodyData& Data = mBodyData.Insert();
	Data.mName		= mDesc->mBody.mName;
	Data.mPose		= PR(mDesc->mBody.mPosition, mDesc->mBody.mRotation);
	Data.mHandle	= ActorHandle;

	mMS.Push(mDesc->mBody.mPosition, mDesc->mBody.mRotation);
	DELETESINGLE(mDesc);
	mBodies.PushBack(ActorHandle);

	ReleaseShapes();
}

void MuJoCoConverter::EndBody()
{
	if(mDesc)
		CreateBody();
	mBodies.PopBack();
	mMS.Pop();
}

void MuJoCoConverter::AddVisual(PintShapeRenderer* renderer, const Point* pos, const Quat* rot)
{
	if(!mShowVisualMeshes)
		return;

	const float epsilon = 0.001f;
	//const float epsilon = 0.1f;
	PINT_BOX_CREATE* BoxCreate = ICE_NEW(PINT_BOX_CREATE)(epsilon, epsilon, epsilon);

	BoxCreate->mFlags = PintShapeFlags(SHAPE_FLAGS_DEFAULT | SHAPE_FLAGS_INACTIVE);

	if(pos)
		BoxCreate->mLocalPos = ToD3D(*pos);
	if(rot)
		BoxCreate->mLocalRot = ToD3D(*rot);
	else
	{
//		const Quat q(0.0f, 0.0f, 0.0f, 1.0f);
//		BoxCreate->mLocalRot = ToD3D(q);
	}

	BoxCreate->mRenderer = renderer;
	AddShape(BoxCreate);
}

void MuJoCoConverter::AddSphereShape(float radius, const Point* pos, const Quat* rot)
{
	PINT_SPHERE_CREATE* SphereCreate = ICE_NEW(PINT_SPHERE_CREATE)(radius);
	if(pos)
		SphereCreate->mLocalPos = ToD3D(*pos);
	if(rot)
		SphereCreate->mLocalRot = ToD3D(*rot);

	if(mShowCollisionShapes)
		SphereCreate->mRenderer = CreateRenderer(*SphereCreate);
	else
		SphereCreate->mRenderer = CreateNullRenderer();

	AddShape(SphereCreate);
}

void MuJoCoConverter::AddBoxShape(float ex, float ey, float ez, const Point* pos, const Quat* rot)
{
	if(0)
		return;
	PINT_BOX_CREATE* BoxCreate = ICE_NEW(PINT_BOX_CREATE)(ex, ez, ey);	// "ToD3D"
	if(pos)
		BoxCreate->mLocalPos = ToD3D(*pos);
	if(rot)
		BoxCreate->mLocalRot = ToD3D(*rot);

	if(mShowCollisionShapes)
		BoxCreate->mRenderer = CreateRenderer(*BoxCreate);
	else
		BoxCreate->mRenderer = CreateNullRenderer();

	AddShape(BoxCreate);
}

void MuJoCoConverter::AddCylinderShape(float radius, float half_height, const Point* pos, const Quat* rot)
{
	if(0)
		return;
	Quat q;
	q.Identity();
	if(rot)
		//q = Quat(rot->p.x, rot->p.y, rot->p.z, rot->w);
		q = *rot;
		q.Normalize();

	if(1)
	{
		Matrix3x3 m33;
		m33.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
		q *= m33;
	}
//	if(rot)
//		q *= *rot;
	
	PINT_CYLINDER_CREATE* CylinderCreate = ICE_NEW(PINT_CYLINDER_CREATE)(radius, half_height);
	if(pos)
		CylinderCreate->mLocalPos = ToD3D(*pos);
	if(rot)
		//CylinderCreate->mLocalRot = *rot;
		CylinderCreate->mLocalRot = ToD3D(q);

	if(rot && 0)
	{
		Matrix3x3 m33;
		m33.FromTo(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
		CylinderCreate->mLocalRot *= m33;
	}

	if(mShowCollisionShapes)
		CylinderCreate->mRenderer = CreateRenderer(*CylinderCreate);
	else
		CylinderCreate->mRenderer = CreateNullRenderer();

	AddShape(CylinderCreate);
}

const MuJoCoConverter::BodyData* MuJoCoConverter::GetBodyData(const char* name) const
{
	if(!name)
		return null;
	const u32 Nb = mBodyData.Size();
	for(u32 i=0; i<Nb; i++)
	{
		if(mBodyData[i].mName && strcmp(name, mBodyData[i].mName)==0)
			return &mBodyData[i];
	}
	return null;
}

PintJointHandle MuJoCoConverter::AddEqualityConstraint(const char* name, const char* body0, const char* body1, const Point& anchor)
{
	const MuJoCoConverter::BodyData* Data0 = GetBodyData(body0);
	const MuJoCoConverter::BodyData* Data1 = GetBodyData(body1);
	if(!Data0 || !Data1)
		return null;

	const Point Anchor = ToD3D(anchor);

	PINT_SPHERICAL_JOINT_CREATE Desc;
	Desc.mObject0			= Data0->mHandle;
	Desc.mLocalPivot0.mPos	= Anchor - Data0->mPose.mPos;
	Desc.mObject1			= Data1->mHandle;
	Desc.mLocalPivot1.mPos	= Anchor - Data1->mPose.mPos;
	return mPint.CreateJoint(Desc);
}
