///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef CAMERA_H
#define CAMERA_H

#ifdef TOSEE
	class Camera
	{
		public:
								Camera();
								~Camera();

		inline_	void			SetFOV(float fov)			{ mFOV = fov;			}
		inline_	float			GetFOV()			const	{ return mFOV;			}
		inline_	void			SetNearClip(float clip)		{ mNearClip = clip;		}
		inline_	float			GetNearClip()		const	{ return mNearClip;		}
		inline_	void			SetFarClip(float clip)		{ mFarClip = clip;		}
		inline_	float			GetFarClip()		const	{ return mFarClip;		}
		inline_	void			SetSpeed(float speed)		{ mSpeed = speed;		}
		inline_	float			GetSpeed()			const	{ return mSpeed;		}
		inline_	void			SetSensitivity(float s)		{ mSensitivity = s;		}
		inline_	float			GetSensitivity()	const	{ return mSensitivity;	}

		inline_	const Point&	getPos()			const	{ return mWorldPos;		}
		inline_	const Point&	getDir()			const	{ return mWorldDir;		}

		inline_	void			SetPos(const Point& pos)	{ mWorldPos = pos;		}
		inline_	void			SetDir(const Point& dir)	{ mWorldDir = dir;		}

				void			Update(float dx, float dy);

				void			MoveForward(float dt);
				void			MoveBackward(float dt);
				void			MoveRight(float dt);
				void			MoveLeft(float dt);

				void			GetViewMatrix(Matrix4x4& mat);
				void			GetProjMatrix(Matrix4x4& mat);
		private:
				Point			mWorldPos;
				Point			mWorldDir;
				Point			mViewY;
				float			mFOV;
				float			mNearClip;
				float			mFarClip;
				float			mSpeed;
				float			mSensitivity;
	};
#endif

	void			SetCameraSpeed(float speed);
	float			GetCameraSpeed();
	void			SetCameraFOV(float fov);
	float			GetCameraFOV();
	void			SetCameraNearClip(float clip);
	float			GetCameraNearClip();
	void			SetCameraFarClip(float clip);
	float			GetCameraFarClip();
	void			SetCameraSensitivity(float s);
	float			GetCameraSensitivity();

	void			SetCamera(const Point& pos, const Point& dir);
	Point			GetCameraPos();
	Point			GetCameraDir();
	void			ResetCamera();
	void			MoveCameraForward(float dt);
	void			MoveCameraBackward(float dt);
	void			MoveCameraRight(float dt);
	void			MoveCameraLeft(float dt);
	void			RotateCamera(int dx, int dy);
//	void			SetupCameraMatrix(float z_near=1.0f, float z_far=10000.0f);
	void			SetupProjectionMatrix();
	void			SetupModelViewMatrix();
	void			GetModelViewMatrix(Matrix4x4& mat);
	void			GetProjMatrix(Matrix4x4& mat);

	void			BuildFrustumPts(Point* frustum);
	const Plane*	GetFrustumPlanes();

	void			CaptureFrustum();
	void			DrawCapturedFrustum();

	Point			ComputeWorldRay(int xs, int ys);

	class WorldRayComputer
	{
		public:
						WorldRayComputer();

		inline_	Point ComputeWorldRay(int xs, int ys)
		{
			const float u = xs - mWidth;
			const float v = mHeight - ys;

			const Point CamRay(mVTanCoeff*u, mHTanCoeff*v, 1.0f);

			return (mInvView * CamRay).Normalize();
		}

		private:
			Matrix3x3	mInvView;
			float		mWidth;
			float		mHeight;
			float		mHTanCoeff;
			float		mVTanCoeff;
	};

#endif
