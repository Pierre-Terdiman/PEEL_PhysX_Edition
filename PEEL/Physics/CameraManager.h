///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

	struct PintRaycastData;

	struct CameraData
	{
		CameraData() : mCurrentCameraIndex(0), mNbSceneCameras(0)	{}
		udword			mCurrentCameraIndex;
		udword			mNbSceneCameras;
		PintCameraPose	mCameraPose[PINT_MAX_CAMERA_POSES];

		bool			operator == (const CameraData& other)	const;

		void			Init(const PINT_WORLD_CREATE& desc);
		void			Reset();
		void			SelectNextCamera();
		void			SelectPreviousCamera();
	};

	class CameraManager
	{
		public:
							CameraManager();
							~CameraManager();

				bool		LoadCameraData(const char* filename);
				void		UpdateCameraPose();
				void		GenerateRays(PintRaycastData* rays, udword nb_rays, float max_dist);
				void		Release();

		private:
				udword		mNbFrames;
				udword		mNbCameraPoses;
				Ray*		mCameraPoses;
				bool		TryLoadCameraData(const char* filename);
	};

#endif
