///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_SCENES_VEHICLES_BASE_H
#define TEST_SCENES_VEHICLES_BASE_H

#include "TestScenes.h"

	class VehicleInput : public TestBase
	{
		struct CameraData
		{
			CameraData() : mUpOffset(0.0f), mDistToTarget(0.0f), mSharpnessPos(0.2f), mSharpnessRot(0.2f)	{}

			float	mUpOffset;
			float	mDistToTarget;
			float	mSharpnessPos;
			float	mSharpnessRot;
		};

		public:
									VehicleInput();
		virtual						~VehicleInput();

		// PhysicsTest
		virtual bool				CommonSetup()	override;
		virtual	bool				SpecialKeyCallback(int key, int x, int y, bool down)	override;
		virtual	bool				KeyboardCallback(unsigned char key, int x, int y, bool down)	override;
		virtual	void				LostFocus()	override;
		virtual	udword				GetFlags()	const	override;
		virtual	float				GetRenderData(Point& center)	const	override
									{
										center = mCameraTarget;
										//return 1000.0f;
										return 200.0f;
									}
		//~PhysicsTest

		// GamepadInterface
		virtual	void				OnButtonEvent(udword button_id, bool down)	override;
		virtual	void				OnAnalogButtonEvent(udword button_id, ubyte old_value, ubyte new_value)	override;
		virtual	void				OnAxisEvent(udword axis_id, float value)	override;
		//~GamepadInterface

				PINT_VEHICLE_INPUT	mInput;
				Point				mFilteredCameraPos;
				Point				mFilteredCameraDir;
				Point				mFilteredObjectDir;
				Point				mCameraTarget;
				CameraData			mCamera;
				float				mExtraDistanceScale;
		private:
				udword				mCameraMode;
				float				mCameraRotateInputY;
				float				mCameraRotateInputZ;
				float				mCameraRotateAngleY;
				float				mCameraRotateAngleZ;
				float				mExtraDistance;
		public:
		inline_	void				SetCameraMode(udword index)
									{
										ASSERT(index<mNbCameraModes);
										mCameraMode = index;
									}
		inline_	void				NextCameraMode()
									{
										if(mCameraMode<mNbCameraModes-1)
											mCameraMode++;
										else
											mCameraMode = 0;
									}
		inline_	void				PreviousCameraMode()
									{
										if(mCameraMode)
											mCameraMode--;
										else
											mCameraMode = mNbCameraModes-1;
									}
				bool				mControlCamera;
				bool				mCameraStyle;
		static	const udword		mNbCameraModes;

				void				UpdateCamera(Pint& pint, PintActorHandle focus_object);

		inline_	void				CameraRotateLeft(bool b)	{ mCameraRotateInputY = b ? -0.5f : 0.0f;	}
		inline_	void				CameraRotateRight(bool b)	{ mCameraRotateInputY = b ? 0.5f : 0.0f;	}
		inline_	void				CameraRotateUp(bool b)		{ mCameraRotateInputZ = b ? 0.5f : 0.0f;	}
		inline_	void				CameraRotateDown(bool b)	{ mCameraRotateInputZ = b ? -0.5f : 0.0f;	}
	};

#endif