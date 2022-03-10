///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "TestScenes_VehiclesBase.h"
#include "PintObjectsManager.h"
#include "MyConvex.h"
#include "Camera.h"
#include "GUI_Helpers.h"
#include "Loader_Bin.h"
#include "TerrainRegionManager.h"

//#define USE_TEXTURE
#ifdef USE_TEXTURE
	#include "Devil.h"
	#include "GLTexture.h"
	#include "TextureManager.h"
#endif

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_CCT = "CCT - Basic character controller test.";

class CCT : public VehicleInput
{
	protected:
	MouseFilter	mFilter;
	float	mPitch;
	float	mYaw;
	float	mSpeed;
	public:
							CCT() : mPitch(0.0f), mYaw(0.0f), mSpeed(10.0f)
							{
							}
	virtual					~CCT()						{						}
	virtual	const char*		GetName()			const	{ return "CCT";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_CCT;		}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_CCT;	}

	virtual	udword			GetFlags() const
							{
								return TEST_FLAGS_USE_CURSOR_KEYS;
							}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(11.05f, 6.34f, 10.78f), Point(-0.67f, -0.32f, -0.67f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();

//		LoadMeshesFromFile_(*this, "Archipelago.bin");
//		LoadMeshesFromFile_(*this, "Terrain.bin");
		LoadMeshesFromFile_(*this, "TestZone.bin");
//		LoadMeshesFromFile_(*this, "kp.bin");
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		Pint_Character* API = pint.GetCharacterAPI();
		if(!caps.mSupportCharacters || !API)
			return false;

		if(!CreateMeshesFromRegisteredSurfaces(pint, caps))
			return false;

		const float Scale = 0.75f;
		PINT_CHARACTER_CREATE Create;
		Create.mCapsule.mRadius = 0.5f*Scale;
		Create.mCapsule.mHalfHeight = 0.5f*Scale;
		Create.mCapsule.mRenderer = CreateCapsuleRenderer(Create.mCapsule.mRadius, Create.mCapsule.mHalfHeight*2.0f);
//		Create.mPosition = Point(0.0f, Create.mCapsule.mRadius + Create.mCapsule.mHalfHeight, 0.0f);
		Create.mPosition = Point(0.0f, 5.0f*Scale, 0.0f);
//		Create.mPosition = Point(225.14f, 756.96f, -67.39f);

		const PintCharacterHandle h = API->CreateCharacter(Create);
		pint.mUserData = h;
		return true;
	}

	virtual	void	CommonUpdate(float dt)
	{
		TestBase::CommonUpdate(dt);

		mCamera.mUpOffset		= 2.0f;
		mCamera.mDistToTarget	= 10.0f;

		extern sdword gDeltaMouseX;
		extern sdword gDeltaMouseY;

		float DeltaMouseX = float(gDeltaMouseX);
		float DeltaMouseY = float(gDeltaMouseY);
		mFilter.Apply(DeltaMouseX, DeltaMouseY);

		const float Coeff = 0.004f;
		mPitch -= DeltaMouseY*Coeff;
		mYaw += DeltaMouseX*Coeff;

		const float mPitchMin	= -HALFPI*0.95f;
		const float	mPitchMax	= HALFPI*0.95f;
		if(mPitch<mPitchMin)	mPitch = mPitchMin;
		if(mPitch>mPitchMax)	mPitch = mPitchMax;
	}

	virtual	udword	Update(Pint& pint, float dt)
	{
		Pint_Character* API = pint.GetCharacterAPI();
		const PintCharacterHandle h = PintCharacterHandle(pint.mUserData);
		if(h && API)
		{
			Point Disp(0.0f, -9.81f, 0.0f);
			const float Speed = mSpeed;
			Point Dir = GetCameraDir();
			Dir.y = 0.0f;
			Dir.Normalize();
			Point Right, Up;
			ComputeBasis(Dir, Right, Up);

			if(mInput.mKeyboard_Accelerate)
			{
				Disp += Speed*Dir;
			}
			if(mInput.mKeyboard_Brake)
			{
				Disp -= Speed*Dir;
			}
			if(mInput.mKeyboard_Left)
			{
				Disp += Speed*Right;
			}
			if(mInput.mKeyboard_Right)
			{
				Disp -= Speed*Right;
			}

			Disp *= dt;

			API->MoveCharacter(h, Disp);

			if(0)
			{
				Point CamPos = GetCameraPos();
				Disp.y = 0.0f;
				CamPos += Disp;
				SetCamera(CamPos, Dir);
			}

//			UpdateCamera(pint, pint.GetCharacterActor(h));

//			const EulerAngles EA(mYaw, mPitch);
//			const Matrix3x3 M(EA);

			Matrix3x3 RotX;	RotX.RotX(mPitch+HALFPI);
			Matrix3x3 RotY;	RotY.RotY(mYaw);
//			Matrix3x3 M = RotY*RotX;
			Matrix3x3 M = RotX*RotY;

//M.RotYX(mYaw, mPitch+HALFPI);
//M.RotYX(mPitch+HALFPI, mYaw);

			const PR Pose = pint.GetWorldTransform(API->GetCharacterActor(h));

			const Point& Target = Pose.mPos;
			const Point NewDir = M[1];
//			const Point NewDir = M[2];
//			printf("%f %f %f\n", NewDir.x, NewDir.y, NewDir.z);

//			Point CamPos = GetCameraPos();
//			Disp.y = 0.0f;
//			CamPos += Disp;
//			SetCamera(CamPos, Dir);
			const float DistToTarget = 4.0f;
			SetCamera(Target - NewDir*DistToTarget, NewDir);
//			SetCamera(Target - Dir*10.0f, NewDir);
		}

		return TestBase::Update(pint, dt);
	}

	virtual	void	MouseMoveCallback(const MouseInfo& mouse)
	{
//		printf("%d %d\n", mouse.mRelMouseX, mouse.mRelMouseY);
//		const float Coeff = 0.01f;
//		mPitch -= float(mouse.mRelMouseY)*Coeff;
//		mYaw += float(mouse.mRelMouseX)*Coeff;
	}

	virtual	bool	RepositionMouse()	const
	{
		return true;
	}

	virtual	void	DrawDebugInfo(Pint& pint, PintRender& render)
	{
		if(1)
		{
			AABB CCTBounds;
			if(GetCCTBounds(pint, CCTBounds))
				render.DrawWireframeAABB(1, &CCTBounds, Point(1.0f, 0.0f, 0.0f));
		}
		else
		{
			Pint_Actor* ActorAPI = pint.GetActorAPI();
			if(!ActorAPI)
				return;

			Pint_Character* API = pint.GetCharacterAPI();
			if(!API)
				return;

			const PintCharacterHandle h = PintCharacterHandle(pint.mUserData);
			if(!h)
				return;

			const PintActorHandle CCTActor = API->GetCharacterActor(h);

			AABB CCTBounds;
			if(ActorAPI->GetWorldBounds(CCTActor, CCTBounds))
				render.DrawWireframeAABB(1, &CCTBounds, Point(1.0f, 0.0f, 0.0f));
		}
	}

	bool GetCCTBounds(Pint& pint, AABB& bounds) const
	{
		Pint_Actor* ActorAPI = pint.GetActorAPI();
		if(!ActorAPI)
			return false;

		Pint_Character* API = pint.GetCharacterAPI();
		if(!API)
			return false;

		const PintCharacterHandle h = PintCharacterHandle(pint.mUserData);
		if(!h)
			return false;

		const PintActorHandle CCTActor = API->GetCharacterActor(h);

		return ActorAPI->GetWorldBounds(CCTActor, bounds);
	}

END_TEST(CCT)

///////////////////////////////////////////////////////////////////////////////

