///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_ZCB.h"
#include "Common.h"

// ### refactor
template<class T>
static void DeleteObjects(PtrContainer& objects)
{
	const udword Nb = objects.GetNbEntries();
	for(udword i=0;i<Nb;i++)
	{
		T* Current = reinterpret_cast<T*>(objects.GetEntry(i));
		//DeleteObject(Current);
		DELETESINGLE(Current);
	}
	objects.Empty();
}


ZCBFactory::ZCBFactory()
{
}

ZCBFactory::~ZCBFactory()
{
//	DeleteObjects<TrackedIndexedSurface>(mMeshes);
}

static void _Log(const char* text)
{
	printf(text);
}

bool ZCBFactory::NewScene(const ZCBSceneInfo& scene)
{
	_Log("NewScene\n");
	return true;
}

bool ZCBFactory::NewCamera(const ZCBCameraInfo& camera)
{
	_Log(_F("NewCamera (%s)\n", camera.mName));
	return true;
}

bool ZCBFactory::NewLight(const ZCBLightInfo& light)
{
	_Log(_F("NewLight (%s)\n", light.mName));
	return true;
}

bool ZCBFactory::NewMaterial(const ZCBMaterialInfo& material)
{
	_Log(_F("NewMaterial (%s)\n", material.mName));
	return true;
}

bool ZCBFactory::NewTexture(const ZCBTextureInfo& texture)
{
	_Log(_F("NewTexture (%s)\n", texture.mName));
	return true;
}

bool ZCBFactory::NewMesh(const ZCBMeshInfo& mesh)
{
	_Log(_F("NewMesh (%s)\n", mesh.mName));

	IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
	IS->Init(mesh.mNbFaces, mesh.mNbVerts, mesh.mVerts, reinterpret_cast<const IndexedTriangle*>(mesh.mFaces));
	IS->Flip();
//	mMeshes.AddPtr(IS);

	PR Pose = mesh.mPrs;
//	Pose.p = mesh.mPrs.mPos;
//	Pose.mRot

	RegisterSurface(IS, null, &Pose);


	return true;
}

bool ZCBFactory::NewShape(const ZCBShapeInfo& shape)
{
	_Log(_F("NewShape (%s)\n", shape.mName));
	return true;
}

bool ZCBFactory::NewHelper(const ZCBHelperInfo& helper)
{
	_Log(_F("NewHelper (%s)\n", helper.mName));
	return true;
}

bool ZCBFactory::NewController(const ZCBControllerInfo& controller)
{
	_Log("NewController\n");
	return true;
}

bool ZCBFactory::NewMotion(const ZCBMotionInfo& motion)
{
	_Log("NewMotion\n");
	return true;
}

bool ZCBFactory::ZCBImportError(const char* error_text, udword error_code)
{
	_Log(error_text);
	return true;
}

void ZCBFactory::ZCBLog(LPSTR fmt, ...)
{
}


#include "Pint.h"
#include "PintObjectsManager.h"
#include "PintShapeRenderer.h"

bool CreateZCBScene(Pint& pint, const PintCaps& caps, ZCBFactory& factory)
{
/*	const udword NbMeshes = factory.mMeshes.GetNbEntries();
	for(udword i=0;i<NbMeshes;i++)
	{
		const IndexedSurface* IS = reinterpret_cast<const IndexedSurface*>(factory.mMeshes.GetEntry(i));

		PINT_MESH_CREATE MeshDesc;
		MeshDesc.mSurface	= IS->GetSurfaceInterface();
		MeshDesc.mRenderer	= CreateMeshRenderer(MeshDesc.mSurface);
		//MeshDesc.mMaterial	= SD[i].mMaterial ? SD[i].mMaterial : material;

		PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
		//ObjectDesc.mName		= name;
		//ObjectDesc.mPosition	= SD[i].mPose.mPos;
		//ObjectDesc.mRotation	= SD[i].mPose.mRot;
		ObjectDesc.mMass		= 0.0f;
		PintActorHandle h = CreatePintObject(pint, ObjectDesc);
	}*/
	return factory.CreateMeshesFromRegisteredSurfaces(pint, caps, null, null, null);

	return true;
}


