///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestScenes_VehiclesBase.h"
#include "Camera.h"

	#include "PxMath.h"
	#include "PxVec3.h"
	#include "PxTransform.h"
	#include "PxMat33.h"
	using namespace physx;
	#include "ICE_To_Px.h"

///////////////////////////////////////////////////////////////////////////////

VehicleInput::VehicleInput() :
	mFilteredCameraPos	(Point(0.0f, 0.0f, 0.0f)),
	mFilteredCameraDir	(Point(0.0f, 0.0f, 0.0f)),
	mFilteredObjectDir	(Point(0.0f, 0.0f, 0.0f)),
	mCameraTarget		(Point(0.0f, 0.0f, 0.0f)),
	mExtraDistanceScale	(1.0f),
	mCameraMode			(0),
	mCameraRotateInputY	(0.0f),
	mCameraRotateInputZ	(0.0f),
	mCameraRotateAngleY	(0.0f),
	mCameraRotateAngleZ	(0.33f),
	mExtraDistance		(0.0f),
	mControlCamera		(false),
	mCameraStyle		(false)
{
}

VehicleInput::~VehicleInput()
{
}

bool VehicleInput::CommonSetup()
{
	mInput.Reset();
	return TestBase::CommonSetup();
}

bool VehicleInput::KeyboardCallback(unsigned char key, int x, int y, bool down)
{
	switch(key)
	{
		case '5':
		{
			mCameraRotateInputY	= 0.0f;
			mCameraRotateInputZ	= 0.0f;
			mCameraRotateAngleY	= 0.0f;
			mCameraRotateAngleZ	= 0.33f;
			mExtraDistance		= 0.0f;
			mExtraDistanceScale	= 1.0f;
			return true;
		}
		break;

		case '4':
		{
			CameraRotateLeft(down);
			return true;
		}
		break;

		case '6':
		{
			CameraRotateRight(down);
			return true;
		}
		break;

		case '2':
		{
			CameraRotateDown(down);
			return true;
		}
		break;

		case '8':
		{
			CameraRotateUp(down);
			return true;
		}
		break;

		case '7':
		{
			if(down)
				mExtraDistance += 0.5f * mExtraDistanceScale;
			return true;
		}
		break;

		case '9':
		{
			if(down)
				mExtraDistance -= 0.5f * mExtraDistanceScale;
			return true;
		}
		break;

		case '+':
		{
			if(down)
				NextCameraMode();
			return true;
		}
		break;

		case '-':
		{
			if(down)
				PreviousCameraMode();
			return true;
		}
		break;
	}
	return false;
}

bool VehicleInput::SpecialKeyCallback(int key, int x, int y, bool down)
{
	switch(key)
	{
		case GLUTX_KEY_UP:
		{
			mInput.mKeyboard_Accelerate = down;
			return true;
		}
		break;
		case GLUTX_KEY_DOWN:
		{
			mInput.mKeyboard_Brake = down;
			return true;
		}
		break;
		case GLUTX_KEY_LEFT:
		{
			mInput.mKeyboard_Left = down;
			return true;
		}
		break;
		case GLUTX_KEY_RIGHT:
		{
			mInput.mKeyboard_Right = down;
			return true;
		}
		break;
		case GLUTX_KEY_INSERT:
		{
			mInput.mKeyboard_HandBrake = down;
			return true;
		}
		break;
		case GLUTX_KEY_PAGE_UP:
		{
			if(down)
				NextCameraMode();
			return true;
		}
		break;
		case GLUTX_KEY_PAGE_DOWN:
		{
			if(down)
				PreviousCameraMode();
			return true;
		}
		break;
	}
	return TestBase::SpecialKeyCallback(key, x, y, down);
}

void VehicleInput::OnButtonEvent(udword button_id, bool down)
{
	//printf("OnButtonEvent: button: %d down: %d\n", button_id, down);

	if(button_id==0)
	{
		if(down)
			mExtraDistance -= 0.5f * mExtraDistanceScale;
	}
	else if(button_id==1)
	{
		if(down)
			mExtraDistance += 0.5f * mExtraDistanceScale;
	}
	else if(button_id==7)
	{
		mCameraRotateInputY	= 0.0f;
		mCameraRotateInputZ	= 0.0f;
		mCameraRotateAngleY	= 0.0f;
		mCameraRotateAngleZ	= 0.33f;
		mExtraDistance		= 0.0f;
		mExtraDistanceScale	= 1.0f;
	}
	else if(button_id==11)
	{
		mInput.mGamepad_Handbrake = down;
	}
}

void VehicleInput::OnAnalogButtonEvent(udword button_id, ubyte old_value, ubyte new_value)
{
	//printf("OnAnalogButtonEvent: button: %d value: %d\n", button_id, new_value);
	if(button_id==0)
	{
		mInput.mGamepad_Brake = float(new_value)/255.0f;
	}
	else if(button_id==1)
	{
		mInput.mGamepad_Accel = float(new_value)/255.0f;
	}
}

void VehicleInput::OnAxisEvent(udword axis_id, float value)
{
	//printf("OnAxisEvent: axis: %d value: %f\n", axis_id, value);
	if(axis_id==0)
	{
		mCameraRotateInputY = value * 0.75f;
	}
	else if(axis_id==1)
	{
		mCameraRotateInputZ = value * 0.75f;
	}
	else if(axis_id==2)
	{
		mInput.mGamepad_Steer = -value;
	}
}

void VehicleInput::LostFocus()
{
	mInput.Reset();
}

const udword VehicleInput::mNbCameraModes = 6;

static inline_ float fsel(float a, float b, float c)
{
	return (a >= 0.0f) ? b : c;
}

void VehicleInput::UpdateCamera(Pint& pint, PintActorHandle focus_object)
{
	if(!focus_object)
		return;

	if(!(pint.GetFlags() & PINT_IS_ACTIVE))
		return;

	// Won't work well for more than 1 engine at a time of course
	const PR Pose = pint.GetWorldTransform(focus_object);
//	printf("%f\n", Pose.mPos.y);
//	const Point CamPos = GetCameraPos();
//	const Point Dir = (Pose.mPos - CamPos).Normalize();
//	SetCamera(CamPos, Dir);

	if(mCameraStyle)
	{
		mCameraTarget = Pose.mPos;

		const float dtime = 1.0f/60.0f;
		const float mMaxCameraRotateSpeed = 5.0f;

		float camDist = mCamera.mDistToTarget + mExtraDistance;
		float cameraYRotExtra = 0.0f;

		mCameraRotateAngleY += mCameraRotateInputY * mMaxCameraRotateSpeed * dtime;
		mCameraRotateAngleY = fsel(mCameraRotateAngleY-10.0f*PI, mCameraRotateAngleY-10.0f*PI, fsel(-mCameraRotateAngleY-10.0f*PI, mCameraRotateAngleY + 10.0f*PI, mCameraRotateAngleY));
		mCameraRotateAngleZ += mCameraRotateInputZ * mMaxCameraRotateSpeed * dtime;
		mCameraRotateAngleZ = PxClamp(mCameraRotateAngleZ, -PI*0.05f, PI*0.45f);

		PxVec3 cameraDir = PxVec3(0,0,1)*PxCos(mCameraRotateAngleY+cameraYRotExtra) + PxVec3(1,0,0)*PxSin(mCameraRotateAngleY+cameraYRotExtra);

		cameraDir=cameraDir*PxCos(mCameraRotateAngleZ)-PxVec3(0,1,0)*PxSin(mCameraRotateAngleZ);

		PxTransform carChassisTransfm = ToPxTransform(Pose);

		PxVec3 target = carChassisTransfm.p;
//		target.y+=0.5f;
		target.y+=mCamera.mUpOffset;

/*const Matrix3x3 M(Pose.mRot);
Point D = M[2];
D.y = 0.0f;
D.Normalize();
Point Right, Up;
ComputeBasis(D, Right, Up);
Matrix3x3 Pose2*/


const Matrix3x3 M(Pose.mRot);
Point D;
//printf("mCameraMode: %d\n", mCameraMode);
const udword Index = mCameraMode;
if(Index==0)
{
	D = M[0];
	D.y = 0.0f;
	D.Normalize();
}
else if(Index==1)
{
	D = M[2];
	D.y = 0.0f;
	D.Normalize();
}
else if(Index==2)
{
	D = -M[0];
	D.y = 0.0f;
	D.Normalize();
}
else if(Index==3)
{
	D = -M[2];
	D.y = 0.0f;
	D.Normalize();
}
else if(Index==4)
{
	D = M[1];
}
else if(Index==5)
{
	D = -M[1];
}
/*
PxVec3 D = carChassisTransfm.q.getBasisVector2();
D.y = 0.0f;
D.normalize();*/

//PxVec3 right, up;
//physx::shdfnd::computeBasis(D, right, up);
Point Right, Up;
//ComputeBasis(ToPoint(D), Right, Up);
ComputeBasis(D, Right, Up);
//PxMat33 mm(ToPxVec3(Right), ToPxVec3(Up), D);
PxMat33 mm(ToPxVec3(Right), ToPxVec3(Up), ToPxVec3(D));
const PxVec3 direction = mm.transform(cameraDir);


//		const PxVec3 direction = carChassisTransfm.q.rotate(cameraDir);

			{
				FeedbackFilter(direction.x, mFilteredObjectDir.x, mCamera.mSharpnessRot);
				FeedbackFilter(direction.y, mFilteredObjectDir.y, mCamera.mSharpnessRot);
				FeedbackFilter(direction.z, mFilteredObjectDir.z, mCamera.mSharpnessRot);
			}

//		PxVec3 position = target-direction*camDist;
		PxVec3 position = target-ToPxVec3(mFilteredObjectDir)*camDist;

			FeedbackFilter(position.x, mFilteredCameraPos.x, mCamera.mSharpnessPos);
			FeedbackFilter(position.y, mFilteredCameraPos.y, mCamera.mSharpnessPos);
			FeedbackFilter(position.z, mFilteredCameraPos.z, mCamera.mSharpnessPos);

	const Point Dir = (ToPoint(target) - ToPoint(position)).Normalize();
			FeedbackFilter(Dir.x, mFilteredCameraDir.x, mCamera.mSharpnessPos);
			FeedbackFilter(Dir.y, mFilteredCameraDir.y, mCamera.mSharpnessPos);
			FeedbackFilter(Dir.z, mFilteredCameraDir.z, mCamera.mSharpnessPos);
//			FeedbackFilter(direction.x, mFilteredCameraDir.x, mCamera.mSharpnessPos);
//			FeedbackFilter(direction.y, mFilteredCameraDir.y, mCamera.mSharpnessPos);
//			FeedbackFilter(direction.z, mFilteredCameraDir.z, mCamera.mSharpnessPos);

			Point Tmp = mFilteredCameraDir;
			Tmp.Normalize();

			SetCamera(mFilteredCameraPos, Tmp);
//		SetCamera(ToPoint(position), ToPoint(direction));
		return;
	}




	Point D(0.0f, 0.0f, 0.0f);

	const Point Target = Pose.mPos;
	mCameraTarget = Target;

	float Up = mCamera.mUpOffset;

	const Matrix3x3 M(Pose.mRot);

	//printf("mCameraMode: %d\n", mCameraMode);
	const udword Index = mCameraMode;
	if(Index==0)
	{
		D = M[0];
		D.y = 0.0f;
		D.Normalize();
	}
	else if(Index==1)
	{
		D = M[2];
		D.y = 0.0f;
		D.Normalize();
	}
	else if(Index==2)
	{
		D = -M[0];
		D.y = 0.0f;
		D.Normalize();
	}
	else if(Index==3)
	{
		D = -M[2];
		D.y = 0.0f;
		D.Normalize();
	}
	else if(Index==4)
	{
		Up = 0.0f;
		D = Point(1.0f, 0.0f, 0.0f);
	}
	else if(Index==5)
	{
		D = Point(1.0f, 0.0f, 0.0f);
	}


	{
		const float SharpnessRot = mCamera.mSharpnessRot;
		FeedbackFilter(D.x, mFilteredObjectDir.x, SharpnessRot);
		FeedbackFilter(D.y, mFilteredObjectDir.y, SharpnessRot);
		FeedbackFilter(D.z, mFilteredObjectDir.z, SharpnessRot);
	}

	//const Point CamPos = Pose.mPos + Point(0.0f, Up, 0.0f) - D*mCamera.mDistToTarget;
	const Point CamPos = Pose.mPos + Point(0.0f, Up, 0.0f) - mFilteredObjectDir*(mCamera.mDistToTarget + mExtraDistance);

	const Point Dir = (Target - CamPos).Normalize();

	const float SharpnessPos = mCamera.mSharpnessPos;
	FeedbackFilter(CamPos.x, mFilteredCameraPos.x, SharpnessPos);
	FeedbackFilter(CamPos.y, mFilteredCameraPos.y, SharpnessPos);
	FeedbackFilter(CamPos.z, mFilteredCameraPos.z, SharpnessPos);
	const float SharpnessRot = mCamera.mSharpnessPos;
	//const float SharpnessRot = mCamera.mSharpnessRot;
	FeedbackFilter(Dir.x, mFilteredCameraDir.x, SharpnessRot);
	FeedbackFilter(Dir.y, mFilteredCameraDir.y, SharpnessRot);
	FeedbackFilter(Dir.z, mFilteredCameraDir.z, SharpnessRot);

	Point Tmp = mFilteredCameraDir;
	Tmp.Normalize();

	SetCamera(mFilteredCameraPos, Tmp);
}

udword VehicleInput::GetFlags() const
{
	return mControlCamera ? TEST_FLAGS_USE_CURSOR_KEYS : TEST_FLAGS_DEFAULT;
}

///////////////////////////////////////////////////////////////////////////////
