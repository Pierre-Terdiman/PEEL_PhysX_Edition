///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef MUJOCO_CONVERTER_H
#define MUJOCO_CONVERTER_H

#include "Pint.h"
#include <Core/RockArray.h>

	//### this one is probably not needed anymore
	class MatrixStack
	{
		public:
				MatrixStack();

		void	Transform(Point& pos, Quat& rot);
		void	Push(const Point& pos, const Quat& rot);
		void	Pop();

		//private:
		udword		mNbMatrices;
		Matrix4x4	mMat[64];
	};

	class JointMapingCallback
	{
		public:
		virtual	void	RegisterJointMapping(const char* name, PintActorHandle handle)	= 0;
	};

	// ### we should eventually feed this from an XML parser and use MuJoCo's files directly,
	// but first we'll test it with code - to decouple it from the XML parser.
	// All of this is very WIP.
	class MuJoCoConverter
	{
		public:
						MuJoCoConverter(Pint& pint);
						~MuJoCoConverter();

		Quat			FromEuler(float x, float y, float z)	const;

		bool			StartModel(bool fixed_base);
		void			EndModel();

		void			StartBody(const char* name, const Point& pos, const Quat& rot, float mass);
		void			EndBody();

		void			SetInertia(float mass, const Point& pos, const Quat& rot, const Point& inertia);

		void			AddVisual(PintShapeRenderer* renderer, const Point* pos = null, const Quat* rot = null);
		void			AddSphereShape(float radius, const Point* pos = null, const Quat* rot = null);
		void			AddBoxShape(float ex, float ey, float ez, const Point* pos = null, const Quat* rot = null);
		void			AddCylinderShape(float radius, float half_height, const Point* pos = null, const Quat* rot = null);

		void			SetHingeJoint(const char* name, const Point& axis, float min_limit, float max_limit);

		PintJointHandle	AddEqualityConstraint(const char* name, const char* body0, const char* body1, const Point& anchor);

		Pint&							mPint;
		PintArticHandle					mModel;
		PintAggregateHandle				mSuperAggregate;
		PintAggregateHandle				mAggregate;
		Rock::Array<PintActorHandle>	mBodies;
		Rock::Array<PINT_SHAPE_CREATE*>	mShapes;

		// Redundant data used to create equality constraints
		struct BodyData
		{
			const char*		mName;
			PR				mPose;
			PintActorHandle	mHandle;
		};
		Rock::Array<BodyData>			mBodyData;

		struct BodyAndJoint : public Allocateable
		{
			BodyAndJoint() : mJointName(null), mType(PINT_JOINT_UNDEFINED), mMin(MAX_FLOAT), mMax(MIN_FLOAT)	{}

			PINT_OBJECT_CREATE	mBody;
			const char*			mJointName;
			PintJoint			mType;
			Point				mAxis;
			float				mMin;
			float				mMax;
		};
		BodyAndJoint*			mDesc;
		MatrixStack				mMS;

		void					CreateBody();
		void					AddShape(PINT_SHAPE_CREATE* new_shape);
		void					ReleaseShapes();
		const BodyData*			GetBodyData(const char* name)	const;

		JointMapingCallback*	mJointMapping;
		float					mStiffness;
		float					mDamping;
		bool					mUseRCA;
		bool					mUseRobotAggregate;
		//bool					mUseWorldAggregate;
		bool					mShowCollisionShapes;
		bool					mShowVisualMeshes;
	};

#endif
