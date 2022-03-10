///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Pint.h"
#include "CameraManager.h"
#include "Camera.h"

void CameraData::Init(const PINT_WORLD_CREATE& desc)
{
	mCurrentCameraIndex = 0;
	mNbSceneCameras = 0;
	mCameraPose[0] = desc.mCamera[0];
	PintCameraPose Default;
	for(udword i=1;i<PINT_MAX_CAMERA_POSES;i++)
	{
		const PintCameraPose& CP = desc.mCamera[i];
		mCameraPose[i] = CP;
		if(CP!=desc.mCamera[i-1] && CP!=Default)
			mNbSceneCameras++;
	}
}

void CameraData::Reset()
{
	mCurrentCameraIndex = 0;
	SetCamera(mCameraPose[0].mPos, mCameraPose[0].mDir);
}

void CameraData::SelectNextCamera()
{
	if(mNbSceneCameras)
	{
		if(mCurrentCameraIndex!=mNbSceneCameras)
			mCurrentCameraIndex++;
		SetCamera(mCameraPose[mCurrentCameraIndex].mPos, mCameraPose[mCurrentCameraIndex].mDir);
	}
}

void CameraData::SelectPreviousCamera()
{
	if(mNbSceneCameras)
	{
		if(mCurrentCameraIndex)
			mCurrentCameraIndex--;
		SetCamera(mCameraPose[mCurrentCameraIndex].mPos, mCameraPose[mCurrentCameraIndex].mDir);
	}
}

bool CameraData::operator == (const CameraData& other) const
{
	if(mNbSceneCameras!=other.mNbSceneCameras)
		return false;

	for(udword i=0;i<PINT_MAX_CAMERA_POSES;i++)
	{
		if(!(mCameraPose[i]==other.mCameraPose[i]))
			return false;
	}
	return true;
}






CameraManager::CameraManager() : mNbFrames(0), mNbCameraPoses(0), mCameraPoses(null)
{
}

CameraManager::~CameraManager()
{
	Release();
}

void CameraManager::Release()
{
	DELETEARRAY(mCameraPoses);
	mNbFrames = mNbCameraPoses = 0;
}

bool CameraManager::TryLoadCameraData(const char* filename)
{
	IceFile BinFile(filename);
	if(!BinFile.IsValid())
		return false;

	const udword NbCameraPoses = BinFile.LoadDword();
	printf("Loading %d camera poses...\n", NbCameraPoses);
	mCameraPoses = ICE_NEW(Ray)[NbCameraPoses];
	mNbCameraPoses = NbCameraPoses;
	for(udword i=0;i<NbCameraPoses;i++)
	{
		BinFile.LoadBuffer(&mCameraPoses[i].mOrig, sizeof(Point));
		BinFile.LoadBuffer(&mCameraPoses[i].mDir, sizeof(Point));
	}
	return true;
}

bool CameraManager::LoadCameraData(const char* filename)
{
	ASSERT(filename);

	const char* File = FindPEELFile(filename);
	if(!File || !TryLoadCameraData(File))
	{
		printf(_F("Failed to load '%s'\n", filename));
		return false;
	}

/*	if(!TryLoadCameraData(_F("../build/%s", filename)))
	{
		if(!TryLoadCameraData(_F("./%s", filename)))
		{
			printf(_F("Failed to load '%s'\n", filename));
			return false;
		}
	}*/
	return true;
}

void CameraManager::UpdateCameraPose()
{
	if(!mNbCameraPoses)
		return;

	SetCamera(mCameraPoses[mNbFrames].mOrig, mCameraPoses[mNbFrames].mDir);
	glMatrixMode(GL_PROJECTION);
//	SetupCameraMatrix();
	SetupProjectionMatrix();
	SetupModelViewMatrix();

	mNbFrames++;
	if(mNbFrames==mNbCameraPoses)
		mNbFrames=0;
}

extern udword gScreenWidth;
extern udword gScreenHeight;

void CameraManager::GenerateRays(PintRaycastData* rays, udword nb_rays, float max_dist)
{
	const float fScreenWidth = float(gScreenWidth)/float(nb_rays);
	const float fScreenHeight = float(gScreenHeight)/float(nb_rays);
	const Point Origin = GetCameraPos();

	udword Offset=0;
	for(udword j=0;j<nb_rays;j++)
	{
		const udword yi = udword(fScreenHeight*float(j));
		for(udword i=0;i<nb_rays;i++)
		{
			const udword xi = udword(fScreenWidth*float(i));
			rays[Offset].mOrigin = Origin;
			rays[Offset].mDir = ComputeWorldRay(xi, yi);
			rays[Offset].mMaxDist = max_dist;
			Offset++;
		}
	}
}