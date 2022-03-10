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
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "GUI_Helpers.h"

///////////////////////////////////////////////////////////////////////////////

namespace
{
	class KinematicTestScene : public TestBase
	{
		public:
									KinematicTestScene();
		virtual						~KinematicTestScene();

		virtual	float				GetRenderData(Point& center)	const	{ return 500.0f;					}
		virtual	TestCategory		GetCategory()					const	{ return CATEGORY_KINEMATICS;		}
		virtual void				GetSceneParams(PINT_WORLD_CREATE& desc);
		virtual	bool				CommonSetup();
		virtual	void				CommonRelease();
		virtual	bool				Setup(Pint& pint, const PintCaps& caps);
		virtual	void				Close(Pint& pint);
		virtual	void				CommonUpdate(float dt);
		virtual	udword				Update(Pint& pint, float dt);
		virtual	void				OnObjectReleased(Pint& pint, PintActorHandle removed_object);

				udword				mNbX;
				udword				mNbY;
				udword				mNbLayers;
				float				mScaleX;
				float				mScaleY;
				float				mYScale;
				Point*				mKinePositions;
				PintShapeRenderer*	mBoxRenderer;
				Point				mBoxExtents;
				bool				mAddStaticObjects;
				bool				mAddDynamicObjects;
				bool				mUpdateKinematics;
	};
}

KinematicTestScene::KinematicTestScene() :
	mNbX				(0),
	mNbY				(0),
	mNbLayers			(0),
	mScaleX				(0.0f),
	mScaleY				(0.0f),
	mYScale				(0.0f),
	mKinePositions		(null),
	mBoxRenderer		(null),
	mAddStaticObjects	(false),
	mAddDynamicObjects	(false),
	mUpdateKinematics	(true)
{
	mBoxExtents = Point(1.5f, 0.2f, 1.5f);
}

KinematicTestScene::~KinematicTestScene()
{
}

void KinematicTestScene::GetSceneParams(PINT_WORLD_CREATE& desc)
{
	TestBase::GetSceneParams(desc);
	desc.mGlobalBounds.SetMinMax(Point(-110.0f, -10.0f, -110.0f), Point(110.0f, 10.0f, 110.0f));
	desc.mCamera[0] = PintCameraPose(Point(72.45f, 21.60f, 73.97f), Point(-0.64f, -0.46f, -0.61f));
	desc.mCamera[1] = PintCameraPose(Point(94.85f, 9.49f, 95.52f), Point(-0.65f, -0.44f, -0.62f));
	desc.mCamera[2] = PintCameraPose(Point(105.18f, 58.27f, 74.29f), Point(-0.69f, -0.58f, -0.43f));
	SetDefEnv(desc, false);
}

bool KinematicTestScene::CommonSetup()
{
	mBoxRenderer = CreateBoxRenderer(mBoxExtents);

	mNbX = mNbY = 64;
//	mNbX = mNbY = 4;
//	mNbX = mNbY = 1;
//	mNbX = mNbY = 2;
	mNbLayers = 1;
	mYScale = 0.4f;

	mKinePositions = ICE_NEW(Point)[mNbLayers*mNbX*mNbY];
	return TestBase::CommonSetup();
}

void KinematicTestScene::CommonRelease()
{
	DELETEARRAY(mKinePositions);
	TestBase::CommonRelease();
}

void KinematicTestScene::OnObjectReleased(Pint& pint, PintActorHandle removed_object)
{
	PintActorHandle* Handles = (PintActorHandle*)pint.mUserData;
	if(Handles)
	{
		for(udword i=0;i<mNbLayers*mNbX*mNbY;i++)
		{
			if(Handles[i]==removed_object)
				Handles[i]=null;
		}
	}
}

bool KinematicTestScene::Setup(Pint& pint, const PintCaps& caps)
{
	if(!caps.mSupportRigidBodySimulation || !caps.mSupportKinematics)
		return false;

	PINT_BOX_CREATE BoxDesc(mBoxExtents);
	BoxDesc.mRenderer = mBoxRenderer;

	PintActorHandle* Handles = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*mNbLayers*mNbX*mNbY);
	pint.mUserData = Handles;
	udword Index = 0;
	for(udword j=0;j<mNbLayers;j++)
	{
		for(udword y=0;y<mNbY;y++)
		{
			for(udword x=0;x<mNbX;x++)
			{
				const float xf = (float(x)-float(mNbX)*0.5f)*mScaleX;
				const float yf = (float(y)-float(mNbY)*0.5f)*mScaleY;

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= Point(xf, 0.2f + float(j)*mYScale, yf);
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mKinematic	= true;
				PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
				ASSERT(Handle);
				Handles[Index++] = Handle;
			}
		}
	}

	if(mAddStaticObjects)
	{
		const udword NbX = 128;
		const udword NbY = 128;
		const float Altitude = 0.0f;
		const float Amplitude = 150.0f;

		BasicRandom Rnd(42);

		for(udword y=0;y<NbY;y++)
		{
			const float CoeffY = 2.0f * ((float(y)/float(NbY-1)) - 0.5f);
			for(udword x=0;x<NbX;x++)
			{
				const float CoeffX = 2.0f * ((float(x)/float(NbX-1)) - 0.5f);

				Point Random;
				UnitRandomPt(Random, Rnd);
				const Point Extents = Random + Point(3.0f, 3.0f, 3.0f);

				PINT_BOX_CREATE BoxDesc(Extents);
				BoxDesc.mRenderer = CreateBoxRenderer(Extents);

				UnitRandomPt(Random, Rnd);
				const Point Center = Random + Point(CoeffX * Amplitude, Altitude, CoeffY * Amplitude);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= Center;
				ObjectDesc.mMass		= 0.0f;
				const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
				ASSERT(Handle);
			}
		}
	}
	if(mAddDynamicObjects)
		return GenerateArrayOfBoxes(pint, Point(1.0f, 1.0f, 1.0f), 32, 32, 10.0f, 60.0f, 60.0f);
	return true;
}

void KinematicTestScene::Close(Pint& pint)
{
	PintActorHandle* Handles = (PintActorHandle*)pint.mUserData;
	ICE_FREE(Handles);
	pint.mUserData = null;

	TestBase::Close(pint);
}

void KinematicTestScene::CommonUpdate(float dt)
{
	TestBase::CommonUpdate(dt);

	// Compute new kinematic poses - the same for all engines
	if(mUpdateKinematics)
	{
		udword Index = 0;
		const float Coeff = 0.2f;
		for(udword j=0;j<mNbLayers;j++)
		{
			for(udword y=0;y<mNbY;y++)
			{
				for(udword x=0;x<mNbX;x++)
				{
					const float xf = (float(x)-float(mNbX)*0.5f)*mScaleX;
					const float yf = (float(y)-float(mNbY)*0.5f)*mScaleY;

					const float h = sinf(mCurrentTime*2.0f + float(x)*Coeff + float(y)*Coeff)*2.0f;
					mKinePositions[Index++] = Point(xf, h + 2.0f + float(j)*mYScale, yf);
				}
			}
		}
	}
}

udword KinematicTestScene::Update(Pint& pint, float dt)
{
	if(mUpdateKinematics)
	{
		const PintActorHandle* Handles = (const PintActorHandle*)pint.mUserData;
		if(Handles)
		{
			const Point* Positions = mKinePositions;
			udword Nb = mNbLayers*mNbX*mNbY;
			while(Nb--)
			{
				const PintActorHandle h = *Handles++;
				const Point& Pos = *Positions++;
				if(h)
					pint.SetKinematicPose(h, Pos);
			}
		}
	}
	return 0;
}

#define START_KINEMATIC_TEST(name, desc)										\
	class name : public KinematicTestScene										\
	{																			\
		public:																	\
								name()						{				}	\
		virtual					~name()						{				}	\
		virtual	const char*		GetName()			const	{ return #name;	}	\
		virtual	const char*		GetDescription()	const	{ return desc;	}

static const char* gDesc_KinematicObjects_NoOverlap = "64*64 (non overlapping) kinematic objects. There should be no broadphase entries for kinematics.";
START_KINEMATIC_TEST(KinematicObjects_NoOverlap, gDesc_KinematicObjects_NoOverlap)

	virtual	bool	CommonSetup()
	{
		mScaleX = mScaleY = 3.5f;
		return KinematicTestScene::CommonSetup();
	}

END_TEST(KinematicObjects_NoOverlap)

static const char* gDesc_KinematicObjects_Overlap = "64*64 (overlapping) kinematic objects. There could be broadphase entries for kinematics, memory usage could increase.";
START_KINEMATIC_TEST(KinematicObjects_Overlap, gDesc_KinematicObjects_Overlap)

	virtual	bool	CommonSetup()
	{
		mScaleX = mScaleY = 2.5f;
		return KinematicTestScene::CommonSetup();
	}

END_TEST(KinematicObjects_Overlap)

static const char* gDesc_KinematicObjects_NoOverlap_DynamicObjects = "64*64 (non overlapping) kinematic objects, plus 32*32 dynamic cubes.";
START_KINEMATIC_TEST(KinematicObjects_NoOverlap_DynamicObjects, gDesc_KinematicObjects_NoOverlap_DynamicObjects)

	virtual	bool	CommonSetup()
	{
		mAddDynamicObjects = true;
		mScaleX = mScaleY = 3.5f;
		return KinematicTestScene::CommonSetup();
	}

END_TEST(KinematicObjects_NoOverlap_DynamicObjects)

static const char* gDesc_KinematicObjects_Overlap_DynamicObjects = "64*64 (overlapping) kinematic objects, plus 32*32 dynamic cubes.";
START_KINEMATIC_TEST(KinematicObjects_Overlap_DynamicObjects, gDesc_KinematicObjects_Overlap_DynamicObjects)

	virtual	bool	CommonSetup()
	{
		mAddDynamicObjects = true;
		mScaleX = mScaleY = 2.5f;
		return KinematicTestScene::CommonSetup();
	}

END_TEST(KinematicObjects_Overlap_DynamicObjects)

static const char* gDesc_KinematicObjects_Overlap_StaticObjects = "64*64 (overlapping) kinematic objects, plus 128*128 static cubes. The presence of statics may decrease performance compared to the same scene without statics.";
START_KINEMATIC_TEST(KinematicObjects_Overlap_StaticObjects, gDesc_KinematicObjects_Overlap_StaticObjects)

	virtual	bool	CommonSetup()
	{
		mAddStaticObjects = true;
		mScaleX = mScaleY = 2.5f;
		return KinematicTestScene::CommonSetup();
	}

END_TEST(KinematicObjects_Overlap_StaticObjects)

static const char* gDesc_SleepingKinematics = "128*128 (overlapping) sleeping kinematic objects.";
START_KINEMATIC_TEST(KinematicObjects_Sleeping, gDesc_SleepingKinematics)

	virtual	bool	CommonSetup()
	{
//		mAddStaticObjects = true;
		mUpdateKinematics = false;
		mNbX = mNbY = 128;
		mScaleX = mScaleY = 2.5f;
		return KinematicTestScene::CommonSetup();
	}

END_TEST(KinematicObjects_Sleeping)

///////////////////////////////////////////////////////////////////////////////

#define NB_FRAMES_TO_CAPTURE	100
#define NB_BONES				19
#define OFFSET					8.0f
//#define MAX_NB_CHARACTERS		256
#define MAX_NB_CHARACTERS		1000

	class KinematicCharacterTest : public TestBase
	{
		public:
								KinematicCharacterTest() :
									mNbCharacters				(0),
									mCapturedData				(null),
									mAddStaticObjects			(false),
									mUseAggregates_Characters	(false),
									mUseAggregates_Level		(false),
									mAggregatesSelfCollide		(false),
									mSkipKinePoseUpdate			(false)
								{
									for(udword i=0;i<NB_BONES;i++)
										mBoneRenderer[i] = null;
								}
		virtual					~KinematicCharacterTest()
								{
									DELETEARRAY(mCapturedData);
								}

		virtual	TestCategory	GetCategory()	const		{ return CATEGORY_KINEMATICS;	}

		struct Character
		{
			Point	mPos;
			udword	mFrameIndex;
			float	mTime;
			float	mX, mY;
			Quat	mRot;

			void	Init(BasicRandom& rnd)
			{
				UnitRandomPt(mPos, rnd);
				mPos.y = 0.0f;
				mPos *= 100.0f;

				mFrameIndex = GetRandomNumber(42);
				mTime = rnd.RandomFloat()*42.0f;

				mX = mY = 0.0f;
				ComputePose(mTime - 1.0f/60.0f);
				ComputePose(mTime);
			}

			void	ComputePose(float time)
			{
				Point Prev(mX, 0.0f, mY);

				const float Angle = time*0.4f;
				mX = sinf(Angle)*100.0f;
				mY = cosf(Angle)*100.0f;
		//		mX += 40.0f * sinf(Angle*0.673f) * cosf(1.17f*Angle);
		//		mY += 40.0f * cosf(Angle) * sinf(0.58f*Angle);

				Point NewPos(mX, 0.0f, mY);
				Point Dir = (NewPos - Prev).Normalize();
				Matrix3x3 FromTo;
				FromTo.FromTo(Point(0.0f, 0.0f, 1.0f), Dir);
				mRot = FromTo;
			}

			Matrix4x4	ComputeBoneMatrix(const OBB& src)	const
			{
				Matrix4x4 M0;
				M0 = src.mRot;
				M0.SetTrans(src.mCenter);

				Matrix4x4 M1;
				M1 = mRot;
				M1.SetTrans(Point(mX, OFFSET, mY));

				return M0*M1;
			}

			void	Update()
			{
				mFrameIndex++;
				if(mFrameIndex==42)
		//		if(mFrameIndex==22)
		//		if(mFrameIndex==NB_FRAMES_TO_CAPTURE)
					mFrameIndex = 0;

				mTime += 1.0f/60.0f;

				ComputePose(mTime);
			}
		};

		udword				mNbCharacters;
		OBB*				mCapturedData;
		PintShapeRenderer*	mBoneRenderer[NB_BONES];
		Character			mCharacters[MAX_NB_CHARACTERS];
		bool				mAddStaticObjects;
		bool				mUseAggregates_Characters;
		bool				mUseAggregates_Level;
		bool				mAggregatesSelfCollide;
		mutable	bool		mSkipKinePoseUpdate;

		virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
		{
			TestBase::GetSceneParams(desc);
			desc.mCamera[0] = PintCameraPose(Point(48.77f, 13.24f, 35.55f), Point(-0.84f, -0.03f, -0.54f));
			desc.mCamera[1] = PintCameraPose(Point(224.87f, 207.81f, 145.12f), Point(-0.52f, -0.77f, -0.37f));
			desc.mCamera[2] = PintCameraPose(Point(165.85f, 14.84f, 163.22f), Point(-0.99f, 0.05f, -0.10f));
			SetDefEnv(desc, true);
		}

		virtual	void	CommonRelease()
		{
			DELETEARRAY(mCapturedData);
			TestBase::CommonRelease();
		}

		virtual bool	CommonSetup()
		{
			TestBase::CommonSetup();

			const char* File = FindPEELFile("obbs2.bin");
			if(File)
			{
				FILE* fp = fopen(File, "rb");
				if(fp)
				{
					mCapturedData = ICE_NEW(OBB)[NB_FRAMES_TO_CAPTURE*NB_BONES];
					fread(mCapturedData, NB_FRAMES_TO_CAPTURE*NB_BONES*sizeof(OBB), 1, fp);
					fclose(fp);
				}
			}

			if(mCapturedData)
			{
				const OBB* Src = mCapturedData;
				for(udword i=0;i<NB_BONES;i++)
					mBoneRenderer[i] = CreateBoxRenderer(Src[i].mExtents);
			}

			BasicRandom Rnd(42);
			for(udword i=0;i<MAX_NB_CHARACTERS;i++)
				mCharacters[i].Init(Rnd);
			return true;
		}

		virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)
		{
			PintActorHandle* Handles = (PintActorHandle*)pint.mUserData;
			if(Handles)
			{
				for(udword i=0;i<NB_BONES*mNbCharacters;i++)
				{
					if(Handles[i]==removed_object)
						Handles[i]=null;
				}
			}
		}

		virtual bool	Setup(Pint& pint, const PintCaps& caps)
		{
			if(!caps.mSupportKinematics)
				return false;
			if((mUseAggregates_Characters||mUseAggregates_Level) && !caps.mSupportAggregates)
				return false;

			PintActorHandle* Handles = (PintActorHandle*)ICE_ALLOC(sizeof(PintActorHandle)*NB_BONES*mNbCharacters);
			pint.mUserData = Handles;

			for(udword j=0;j<mNbCharacters;j++)
			{
				PintAggregateHandle Aggregate = mUseAggregates_Characters ? pint.CreateAggregate(NB_BONES, mAggregatesSelfCollide) : null;
				const OBB* Src = mCapturedData;
				for(udword i=0;i<NB_BONES;i++)
				{
					PINT_BOX_CREATE BoxDesc(Src[i].mExtents);
					BoxDesc.mRenderer	= mBoneRenderer[i];

					const Matrix4x4 M = mCharacters[j].ComputeBoneMatrix(Src[i]);

					PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
					ObjectDesc.mPosition	= M.GetTrans();
					ObjectDesc.mPosition	+= mCharacters[j].mPos;
					ObjectDesc.mRotation	= M;
					ObjectDesc.mRotation.Normalize();
					ObjectDesc.mMass		= 1.0f;
					ObjectDesc.mKinematic	= true;
					ObjectDesc.mAddToWorld	= !mUseAggregates_Characters;
					const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
					*Handles++ = Handle;
					if(Aggregate)
						pint.AddToAggregate(Handle, Aggregate);
				}
				if(Aggregate)
					pint.AddAggregateToScene(Aggregate);
			}

			if(mAddStaticObjects)
			{
				const udword NbX = 128;
				const udword NbY = 128;
				const float Altitude = 0.0f;
				const float Amplitude = 200.0f;

				BasicRandom Rnd(42);

				for(udword y=0;y<NbY;y++)
				{
					const float CoeffY = 2.0f * ((float(y)/float(NbY-1)) - 0.5f);
					PintAggregateHandle Aggregate = mUseAggregates_Level ? pint.CreateAggregate(128, false) : null;

					for(udword x=0;x<NbX;x++)
					{
						const float CoeffX = 2.0f * ((float(x)/float(NbX-1)) - 0.5f);

						Point Random;
						UnitRandomPt(Random, Rnd);
						const Point Extents = Random + Point(3.0f, 3.0f, 3.0f);

						PINT_BOX_CREATE BoxDesc(Extents);
						BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

						UnitRandomPt(Random, Rnd);
						const Point Center = Random + Point(CoeffX * Amplitude, Altitude, CoeffY * Amplitude);

						PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
						ObjectDesc.mPosition	= Center;
						ObjectDesc.mMass		= 0.0f;
						ObjectDesc.mAddToWorld	= !mUseAggregates_Level;
						PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
						ASSERT(Handle);
						if(Aggregate)
							pint.AddToAggregate(Handle, Aggregate);
					}
					if(Aggregate)
						pint.AddAggregateToScene(Aggregate);
				}
			}
			return true;
		}

		virtual	void	Close(Pint& pint)
		{
			PintActorHandle* Handles = (PintActorHandle*)pint.mUserData;
			ICE_FREE(Handles);
			pint.mUserData = null;

			TestBase::Close(pint);
		}

		virtual void	CommonUpdate(float dt)
		{
			TestBase::CommonUpdate(dt);

			for(udword i=0;i<mNbCharacters;i++)
				mCharacters[i].Update();
		}

		virtual udword	Update(Pint& pint, float dt)
		{
			if(!mSkipKinePoseUpdate)
			{
				const PintActorHandle* Handles = (const PintActorHandle*)pint.mUserData;
				if(Handles)
				{
					for(udword j=0;j<mNbCharacters;j++)
					{
						const OBB* Src = mCapturedData + mCharacters[j].mFrameIndex*NB_BONES;
						for(udword i=0;i<NB_BONES;i++)
						{
							Matrix4x4 M = mCharacters[j].ComputeBoneMatrix(Src[i]);
							M.m[3][0] += mCharacters[j].mPos.x;
							M.m[3][1] += mCharacters[j].mPos.y;
							M.m[3][2] += mCharacters[j].mPos.z;
							PR Pose = M;
							Pose.mRot.Normalize();
							const PintActorHandle h = *Handles++;
							if(h)
								pint.SetKinematicPose(h, Pose);
						}
					}
				}
			}
			return TestBase::Update(pint, dt);
		}
	};

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_KinematicCharacter = "Kinematic character with or without aggregates, with or without self-collisions, moving in a mockup static world.";

static const char* gDesc_1NoAggregates =
"Single kinematic character. The body parts are kinematic objects\n\
updated each frame from the animation engine. Dynamic objects are\n\
pushed away by the character but this is a one-way interaction.\n\
The character is otherwise a Juggernaut (in X-Men terms!)";

static const char* gDesc_100NoAggregates =
"100 kinematic characters. Same as previous test but with 100\n\
characters. This is an 'empty' scene from the physics engine's point\n\
of view but it still takes time to simulate because all the constantly\n\
moving body parts put a lot of stress on the broadphase.";

static const char* gDesc_100AggregatesSelfCollide =
"100 kinematic characters. Same as previous test but this time each\n\
character is an 'aggregate', i.e. a collection of actors. An aggregate\n\
is a single entry in the broadphase structure instead of one for each\n\
body part before) - so there is less stress on the broadphase.\n\
Self-collisions for the body parts within a single aggregate are\n\
enabled.";

static const char* gDesc_100AggregatesNoSelfCollide =
"100 kinematic characters. Same as previous test, but this time\n\
self-collisions are disabled. The different tests show the impact of\n\
each option on performance.";

static const char* gDesc_200NoAggregates =
"200 kinematic characters - no aggregates. We repeat the same tests\n\
with more objects just to see how the performance scales. Note that\n\
this is a situation where tree-based or alternative broadphases like\n\
MBP in PhysX 3.4 perform a lot better than traditional SAP\n\
approaches.";

static const char* gDesc_200AggregatesSelfCollide =
"200 kinematic characters - aggregates, self-collisions. We repeat the\n\
same tests with more objects just to see how the performance scales.";

static const char* gDesc_200AggregatesNoSelfCollide =
"200 kinematic characters - aggregates, no self-collisions. We repeat\n\
the same tests with more objects just to see how the performance\n\
scales.";

static const char* gDesc_200NoAggregatesLevel =
"200 kinematic characters & level - no aggregates. Same as before\n\
but now we add 128*128 static boxes to simulate a complex game\n\
level. SAP broadphases should struggle a lot.";

static const char* gDesc_200AggregatesSelfCollideLevel =
"200 kinematic characters & level - aggregates, self-collisions.\n\
Aggregates should provide a large speedup if SAP is used, not that\n\
much - but still something - if MBP is used.";

static const char* gDesc_200AggregatesNoSelfCollideLevel =
"200 kinematic characters & level - aggregates, no self-collisions";

static const char* gDesc_200NoAggregatesLevelAggregates =
"200 kinematic characters & level - no aggregate character,\n\
aggregate level. Same as before but now the 128*128 level boxes\n\
are also put in aggregates. So there are 128 static aggregates\n\
containing a set of 128 random boxes each.";

static const char* gDesc_200AggregatesSelfCollideLevelAggregates =
"200 kinematic characters & level - aggregate characters with\n\
self-collisions, aggregate level.";

static const char* gDesc_200AggregatesNoSelfCollideLevelAggregates =
"200 kinematic characters & level - aggregate characters without\n\
self-collisions, aggregate level.";

struct KinePreset
{
	const char*	mDesc;
	udword		mNbCharacters;
	bool		mAddStaticObjects;
	bool		mUseAggregates_Characters;
	bool		mAggregatesSelfCollide;
	bool		mUseAggregates_Level;
};

#define NB_PRESETS	13
static const KinePreset gPreset[NB_PRESETS] = {
	{gDesc_1NoAggregates, 1, false, false, false, false},
	{gDesc_100NoAggregates, 100, false, false, false, false},
	{gDesc_100AggregatesSelfCollide, 100, false, true, true, false},
	{gDesc_100AggregatesNoSelfCollide, 100, false, true, false, false},
	{gDesc_200NoAggregates, 200, false, false, false, false},
	{gDesc_200AggregatesSelfCollide, 200, false, true, true, false},
	{gDesc_200AggregatesNoSelfCollide, 200, false, true, false, false},
	{gDesc_200NoAggregatesLevel, 200, true, false, false, false},
	{gDesc_200AggregatesSelfCollideLevel, 200, true, true, true, false},
	{gDesc_200AggregatesNoSelfCollideLevel, 200, true, true, false, false},
	{gDesc_200NoAggregatesLevelAggregates, 200, true, false, false, true},
	{gDesc_200AggregatesSelfCollideLevelAggregates, 200, true, true, true, true},
	{gDesc_200AggregatesNoSelfCollideLevelAggregates, 200, true, true, false, true},
};

class KinematicCharacter : public KinematicCharacterTest
{
			EditBoxPtr		mEditBox_NbCharacters;
			EditBoxPtr		mEditBox_Desc;
			CheckBoxPtr		mCheckBox_UseAggregates;
			CheckBoxPtr		mCheckBox_AggregateSelfCollision;
			CheckBoxPtr		mCheckBox_Level;
			CheckBoxPtr		mCheckBox_UseAggregatesForLevel;
			CheckBoxPtr		mCheckBox_ProfileKinePoseUpdate;
			CheckBoxPtr		mCheckBox_SkipKinePoseUpdate;
			ComboBoxPtr		mComboBox_Preset;
	public:
							KinematicCharacter()					{									}
	virtual					~KinematicCharacter()					{									}
	virtual	const char*		GetName()						const	{ return "KinematicCharacter";		}
	virtual	const char*		GetDescription()				const	{ return gDesc_KinematicCharacter;	}
	virtual	float			GetRenderData(Point& center)	const	{ return 1000.0f;					}

	virtual	udword			GetProfilingFlags()	const
							{
								mSkipKinePoseUpdate = mCheckBox_SkipKinePoseUpdate ? mCheckBox_SkipKinePoseUpdate->IsChecked() : false;
								const bool Status = mCheckBox_ProfileKinePoseUpdate ? mCheckBox_ProfileKinePoseUpdate->IsChecked() : false;
								return Status ? PROFILING_TEST_UPDATE : PROFILING_SIM_UPDATE;
							}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 350;
		WD.mHeight	= 400;
		WD.mLabel	= "KinematicCharacter";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 80;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		const bool Enabled = true;
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb characters:", &UIElems);
			mEditBox_NbCharacters = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "1", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			mEditBox_NbCharacters->SetEnabled(Enabled);
			y += YStep;

			mCheckBox_UseAggregates = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use aggregates", &UIElems, false, null, null);
			mCheckBox_UseAggregates->SetEnabled(Enabled);
			y += YStep;

			mCheckBox_AggregateSelfCollision = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Aggregate self-collision", &UIElems, false, null, null);
			mCheckBox_AggregateSelfCollision->SetEnabled(Enabled);
			y += YStep;

			mCheckBox_Level = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Add static level mockup", &UIElems, false, null, null);
			mCheckBox_Level->SetEnabled(Enabled);
			y += YStep;

			mCheckBox_UseAggregatesForLevel = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Use aggregates for level", &UIElems, false, null, null);
			mCheckBox_UseAggregatesForLevel->SetEnabled(Enabled);
			y += YStep;

			mCheckBox_ProfileKinePoseUpdate = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Profile kinematic pose updates", &UIElems, false, null, null);
			mCheckBox_ProfileKinePoseUpdate->SetEnabled(Enabled);
			y += YStep;

			mCheckBox_SkipKinePoseUpdate = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Skip kinematic pose updates", &UIElems, false, null, null);
			mCheckBox_SkipKinePoseUpdate->SetEnabled(Enabled);
			y += YStep;
		}
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Presets:", &UIElems);

			class MyComboBox : public IceComboBox
			{
				KinematicCharacter&	mTest;
				public:
								MyComboBox(const ComboBoxDesc& desc, KinematicCharacter& test) :
									IceComboBox(desc),
									mTest(test)	{}
				virtual			~MyComboBox()	{}
				virtual	void	OnComboBoxEvent(ComboBoxEvent event)
				{
					if(event==CBE_SELECTION_CHANGED)
					{
						const udword SelectedIndex = GetSelectedIndex();
						const bool Enabled = SelectedIndex==GetItemCount()-1;

						mTest.mEditBox_NbCharacters->SetEnabled(Enabled);
						mTest.mCheckBox_UseAggregates->SetEnabled(Enabled);
						mTest.mCheckBox_AggregateSelfCollision->SetEnabled(Enabled);
						mTest.mCheckBox_Level->SetEnabled(Enabled);
						mTest.mCheckBox_UseAggregatesForLevel->SetEnabled(Enabled);
//						mTest.mCheckBox_ProfileKinePoseUpdate->SetEnabled(Enabled);
//						mTest.mCheckBox_SkipKinePoseUpdate->SetEnabled(Enabled);						

						if(!Enabled && SelectedIndex<NB_PRESETS)
						{
							mTest.mEditBox_NbCharacters->SetText(_F("%d", gPreset[SelectedIndex].mNbCharacters));
							mTest.mCheckBox_UseAggregates->SetChecked(gPreset[SelectedIndex].mUseAggregates_Characters);
							mTest.mCheckBox_AggregateSelfCollision->SetChecked(gPreset[SelectedIndex].mAggregatesSelfCollide);
							mTest.mCheckBox_Level->SetChecked(gPreset[SelectedIndex].mAddStaticObjects);
							mTest.mCheckBox_UseAggregatesForLevel->SetChecked(gPreset[SelectedIndex].mUseAggregates_Level);
//							mTest.mCheckBox_ProfileKinePoseUpdate->SetChecked(false);
//							mTest.mCheckBox_SkipKinePoseUpdate->SetChecked(false);							
							mTest.mEditBox_Desc->SetMultilineText(gPreset[SelectedIndex].mDesc);
						}
						if(SelectedIndex<NB_PRESETS)
							mTest.mEditBox_Desc->SetMultilineText(gPreset[SelectedIndex].mDesc);
						else
							mTest.mEditBox_Desc->SetMultilineText("User-defined");
						mTest.mMustResetTest = true;
					}
				}
			};

			ComboBoxDesc CBBD;
			CBBD.mID		= 0;
			CBBD.mParent	= UI;
			CBBD.mX			= 4+OffsetX;
			CBBD.mY			= y;
			CBBD.mWidth		= 250;
			CBBD.mHeight	= 20;
			CBBD.mLabel		= "Presets";
			mComboBox_Preset = ICE_NEW(MyComboBox)(CBBD, *this);
			RegisterUIElement(mComboBox_Preset);
			mComboBox_Preset->Add("SingleNoAggregates");
			mComboBox_Preset->Add("100NoAggregates");
			mComboBox_Preset->Add("100AggregatesSelfCollide");
			mComboBox_Preset->Add("100AggregatesNoSelfCollide");
			mComboBox_Preset->Add("200NoAggregates");
			mComboBox_Preset->Add("200AggregatesSelfCollide");
			mComboBox_Preset->Add("200AggregatesNoSelfCollide");
			mComboBox_Preset->Add("200NoAggregatesLevel");
			mComboBox_Preset->Add("200AggregatesSelfCollideLevel");
			mComboBox_Preset->Add("200AggregatesNoSelfCollideLevel");
			mComboBox_Preset->Add("200NoAggregatesLevelAggregates");
			mComboBox_Preset->Add("200AggregatesSelfCollideLevelAggregates");
			mComboBox_Preset->Add("200AggregatesNoSelfCollideLevelAggregates");
			mComboBox_Preset->Add("User-defined");
			mComboBox_Preset->Select(4);
			mComboBox_Preset->SetVisible(true);
			y += YStep;

			y += YStep;
			mEditBox_Desc = helper.CreateEditBox(UI, 0, 4, y, 350-16, 100, "", &UIElems, EDITBOX_TEXT, null);
			mEditBox_Desc->SetReadOnly(true);
			y += 100;

			mComboBox_Preset->OnComboBoxEvent(CBE_SELECTION_CHANGED);
		}

		y += YStep;
		AddResetButton(UI, 4, y, 350-16);

		return null;
	}

	virtual	const char*		GetSubName()	const
	{
		if(mComboBox_Preset)
		{
			const udword SelectedIndex = mComboBox_Preset->GetSelectedIndex();
			if(SelectedIndex==0)
				return "SingleNoAggregates";
			else if(SelectedIndex==1)
				return "100NoAggregates";
			else if(SelectedIndex==2)
				return "100AggregatesSelfCollide";
			else if(SelectedIndex==3)
				return "100AggregatesNoSelfCollide";
			else if(SelectedIndex==4)
				return "200NoAggregates";
			else if(SelectedIndex==5)
				return "200AggregatesSelfCollide";
			else if(SelectedIndex==6)
				return "200AggregatesNoSelfCollide";
			else if(SelectedIndex==7)
				return "200NoAggregatesLevel";
			else if(SelectedIndex==8)
				return "200AggregatesSelfCollideLevel";
			else if(SelectedIndex==9)
				return "200AggregatesNoSelfCollideLevel";
			else if(SelectedIndex==10)
				return "200NoAggregatesLevelAggregates";
			else if(SelectedIndex==11)
				return "200AggregatesSelfCollideLevelAggregates";
			else if(SelectedIndex==12)
				return "200AggregatesNoSelfCollideLevelAggregates";
		}
		return null;
	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		KinematicCharacterTest::GetSceneParams(desc);
		const bool AddStaticObjects = mCheckBox_Level ? mCheckBox_Level->IsChecked() : false;
		SetDefEnv(desc, !AddStaticObjects);
	}

	virtual bool			CommonSetup()
	{
		mNbCharacters = GetInt(mNbCharacters, mEditBox_NbCharacters);
		if(mNbCharacters>MAX_NB_CHARACTERS)
		{
			mNbCharacters = MAX_NB_CHARACTERS;
//			mEditBox_NbCharacters->SetText(_F("%s", MAX_NB_CHARACTERS));
//			mEditBox_NbCharacters->SetText("256");
			mEditBox_NbCharacters->SetText("1000");
		}

		mUseAggregates_Characters = mCheckBox_UseAggregates ? mCheckBox_UseAggregates->IsChecked() : false;
		mAggregatesSelfCollide = mCheckBox_UseAggregates ? mCheckBox_AggregateSelfCollision->IsChecked() : false;
		mAddStaticObjects = mCheckBox_Level ? mCheckBox_Level->IsChecked() : false;
		mUseAggregates_Level = mCheckBox_UseAggregatesForLevel ? mCheckBox_UseAggregatesForLevel->IsChecked() : false;
		return KinematicCharacterTest::CommonSetup();
	}

	virtual bool			Setup(Pint& pint, const PintCaps& caps)
	{
		return KinematicCharacterTest::Setup(pint, caps);
	}

}KinematicCharacter;

///////////////////////////////////////////////////////////////////////////////

//static const float gMainScale = 0.5f;
static const float gMainScale = 1.0f;
//static const float gMainScale = 1.5f;

static const float gLegWidth = 0.4f*gMainScale;
//static const float gLegWidth = 1.0f;
//static const float gLegWidth = 0.1f;

static const float gUpperLength = 6.0f*gMainScale;
static const float gLowerLength = 4.0f*gMainScale;
//static const float gLowerLength = 8.0f*gMainScale;

//static const float gCycleStride = 8.0f*gMainScale;
static const float gCycleStride = 4.0f*gMainScale;
//static const float gCycleStride = 1.0f;
//static const float gCycleStride = 6.0f*gMainScale;
//static const float gCycleStride = 2.0f*gMainScale;

//static const float gWalkSpeed = 1.0f;
static const float gWalkSpeed = 2.0f;
//static const float gWalkSpeed = 4.0f;
//static const float gWalkSpeed = 6.0f;
//static const float gWalkSpeed = 8.0f;
//static const float gWalkSpeed = 16.0f;
static const float gWalkSpeedMultiplier = 0.5f;

//static const float gStepHeight = 1.5f;
static const float gStepHeight = 1.0f;
//static const float gStepHeight = 0.5f;
//static const float gStepHeight = 0.75f;

static const float gProbeLength = 10.0f;
static const float gProbeOffset = 4.0f;

// todo: reimplement
static float SawTooth(float t)
{
	t = fmodf(t, PI*2.0f);

	if(t<0.0f)
		t += PI*2.0f;

	const float HalfPi = PI*0.5f;
	if(t<HalfPi)
		return t / HalfPi;

	if(t<PI)
		return 1.0f - ((t - HalfPi) / HalfPi);

	return -SawTooth(t - PI);
}

static udword CircleCircleIntersection(Point* ip, const Point& center0, float radius0, const Point& center1, float radius1)
{
    // The two circles are |X-C0| = R0 and |X-C1| = R1.  Define U = C1 - C0
    // and V = Perp(U) where Perp(x,y) = (y,-x).  Note that Dot(U,V) = 0 and
    // |V|^2 = |U|^2.  The intersection points X can be written in the form
    // X = C0+s*U+t*V and X = C1+(s-1)*U+t*V.  Squaring the circle equations
    // and substituting these formulas into them yields
    //   R0^2 = (s^2 + t^2)*|U|^2
    //   R1^2 = ((s-1)^2 + t^2)*|U|^2.
    // Subtracting and solving for s yields
    //   s = ((R0^2-R1^2)/|U|^2 + 1)/2
    // Then replace in the first equation and solve for t^2
    //   t^2 = (R0^2/|U|^2) - s^2.
    // In order for there to be solutions, the right-hand side must be
    // nonnegative.  Some algebra leads to the condition for existence of
    // solutions,
    //   (|U|^2 - (R0+R1)^2)*(|U|^2 - (R0-R1)^2) <= 0.
    // This reduces to
    //   |R0-R1| <= |U| <= |R0+R1|.
    // If |U| = |R0-R1|, then the circles are side-by-side and just tangent.
    // If |U| = |R0+R1|, then the circles are nested and just tangent.
    // If |R0-R1| < |U| < |R0+R1|, then the two circles to intersect in two
    // points.

    Point U = center1 - center0;
	float USqrLen = U.SquareMagnitude();
    float R0 = radius0, R1 = radius1;
    float R0mR1 = R0 - R1;
    if(USqrLen == 0.0f && R0mR1 == 0.0f)
    {
        // Circles are the same.
        return 0xffffffff;
    }

    float R0mR1Sqr = R0mR1*R0mR1;
    if(USqrLen < R0mR1Sqr)
    {
        // The circles do not intersect.
        return 0;
    }

    float R0pR1 = R0 + R1;
    float R0pR1Sqr = R0pR1*R0pR1;
    if(USqrLen > R0pR1Sqr)
    {
        // The circles do not intersect.
        return 0;
    }

    if(USqrLen < R0pR1Sqr)
    {
        if(R0mR1Sqr < USqrLen)
        {
            float invUSqrLen = 1.0f / USqrLen;
            float s = 0.5f*((R0*R0 - R1*R1)*invUSqrLen + 1.0f);
            Point tmp = center0 + s*U;

            // In theory, discr is nonnegative.  However, numerical round-off
            // errors can make it slightly negative.  Clamp it to zero.
            float discr = R0*R0*invUSqrLen - s*s;
            if(discr < 0.0f)
            {
                discr = 0.0f;
            }
            float t = sqrtf(discr);
            Point V(U.y, -U.x, 0.0f);
            ip[0] = tmp - t*V;
            ip[1] = tmp + t*V;
            return (t >0.0f ? 2 : 1);
        }
        else
        {
            // |U| = |R0-R1|, circles are tangent.
            ip[0] = center0 + (R0 / R0mR1)*U;
			return 1;
        }
    }
    else
    {
        // |U| = |R0+R1|, circles are tangent.
        ip[0] = center0 + (R0 / R0pR1)*U;
		return 1;
    }
}


	class WalkCycle
	{
							PREVENT_COPY(WalkCycle)
		public:
							WalkCycle(Point& pos, float time, float z, bool drive);
							~WalkCycle();

			void			Update(float elapsed_time, bool is_moving, float current_direction, float walk_speed, float walk_speed_multiplier);
			void			Render(PintRender& render);

			float			mLocalTime;
			float			mCurrentStride;
			Point			mFixedPos;
			Point&			mPos;
			float			mZ;
			bool			mFixed;
			Point			mUpperPos;
			Point			mMiddlePos;
			Point			mLowerPos;
			bool			mDrive;
	};

WalkCycle::WalkCycle(Point& pos, float time, float z, bool drive) :
	mLocalTime		(time),
	mCurrentStride	(0.0f),
	mFixedPos		(Point(0.0f, 0.0f, 0.0f)),
	mPos			(pos),
	mZ				(z),
	mFixed			(false),
	mUpperPos		(Point(0.0f, 0.0f, 0.0f)),
	mMiddlePos		(Point(0.0f, 0.0f, 0.0f)),
	mLowerPos		(Point(0.0f, 0.0f, 0.0f)),
	mDrive			(drive)
{
}

WalkCycle::~WalkCycle()
{
}

void WalkCycle::Update(float elapsed_time, bool is_moving, float current_direction, float walk_speed, float walk_speed_multiplier)
{
	if(1)
	{
		mCurrentStride = gCycleStride;// * walk_speed_multiplier;
		if(!is_moving)
			elapsed_time = 0.0f;
	}

	mLocalTime += elapsed_time * walk_speed * walk_speed_multiplier;

	if(0)
	{
		// "works" but reverse walk is faster than FW walk and there's a bug in there
		if(is_moving)
		{
			mCurrentStride += fabsf(elapsed_time) * 4.0f;
			if(mCurrentStride>gCycleStride)
				mCurrentStride = gCycleStride;
		}
		else
		{
			mCurrentStride -= fabsf(elapsed_time) * 4.0f;
			if(mCurrentStride<0.0f)
				mCurrentStride = 0.0f;
		}
	}

	float CycleStride = mCurrentStride;

	const float Angle = mLocalTime;// * gWalkSpeed;
//	float x = sinf(Angle) * CycleStride;
	float x = SawTooth(Angle) * CycleStride;
	float y = cosf(Angle) * CycleStride * gStepHeight;
	if(y<0.0f)
	{
//		y = 0.0f;

		if(!mFixed)
		{
			mFixed = true;
			mFixedPos = mPos + Point(x, 0.0f, 0.0f);
//			mFixedPos.y = 
		}

		const float Delta = mPos.x + x - mFixedPos.x;
		if(mDrive)
			mPos.x -= Delta;
	}
	else
	{
		mFixed = false;
	}

	const float TotalLength = gUpperLength + gLowerLength;
	const Point UpperPos = mPos + Point(0.0f, TotalLength /*- 1.0f/*- gCycleStride*0.5f*/, mZ);

	Point LowerPos = mPos + Point(x, y, mZ);

	Point d = LowerPos - UpperPos;
	if(d.Magnitude()>TotalLength-0.05f)
	{
		d.Normalize();
		d*=TotalLength-0.05f;
		LowerPos = UpperPos + d;
	}

//	const float Drop = 1.0f;
//	const float Drop = 0.1f;
	const float Drop = 0.0f;
	const Point rayOrigin = LowerPos + Point(0.0f, gProbeOffset, 0.0f);
/*	b2RaycastHit hit = Raycast(mWorld, rayOrigin, LowerPos - Point(0.0f, gProbeLength), 0xffffffff & ~(FILTER_GROUP_LEGS|FILTER_GROUP_PLAYER|FILTER_GROUP_TREE));
	if(hit.mFixture)
	{
//		g_debugDraw.DrawSolidCircle(hit.mPoint, 0.1f, Point(0.0f, 0.0f), b2Color(1.0f, 1.0f, 0.0f));

		const float distance = (hit.mPoint - rayOrigin).Length();
		if(distance<gProbeOffset)
		{
			const float Diff = gProbeOffset - distance;
			mPos.y += Diff;
		}
		else mPos.y -= Drop;
	}
	else*/
		mPos.y -= Drop;


	Point ip[2];
	udword nb = CircleCircleIntersection(ip, UpperPos, gUpperLength, LowerPos, gLowerLength);
	assert(nb);
/*	if(nb>=1)
	g_debugDraw.DrawSolidCircle(ip[0], 0.1f, Point(0.0f, 0.0f), b2Color(1.0f, 1.0f, 1.0f));
	if(nb>=2)
	g_debugDraw.DrawSolidCircle(ip[1], 0.1f, Point(0.0f, 0.0f), b2Color(1.0f, 1.0f, 1.0f));
*/
	udword index = 0;
	if(current_direction>0.0f)
	{
		if(ip[0].x<ip[1].x)
			index = 1;
	}
	else
	{
		if(ip[0].x>ip[1].x)
			index = 1;
	}

	mUpperPos = UpperPos;
	mMiddlePos = ip[index];
	mLowerPos = LowerPos;
}

void WalkCycle::Render(PintRender& render)
{
	{
/*		g_debugDraw.DrawCircle(mPos, gCycleStride, b2Color(1.0f, 0.0f, 0.0f));

		g_debugDraw.DrawSolidCircle(mLowerPos, 0.1f, Point(0.0f, 0.0f), mFixed ? b2Color(0.0f, 0.0f, 1.0f) : b2Color(1.0f, 1.0f, 1.0f));

		g_debugDraw.DrawCircle(mUpperPos, gUpperLength, b2Color(0.0f, 1.0f, 0.0f));
		g_debugDraw.DrawCircle(mLowerPos, gLowerLength, b2Color(0.0f, 1.0f, 0.0f));
		g_debugDraw.DrawSolidCircle(mUpperPos, 0.1f, Point(0.0f, 0.0f), b2Color(1.0f, 1.0f, 1.0f));*/

		render.DrawLine(mLowerPos, mMiddlePos, Point(1.0f, 1.0f, 1.0f));
		render.DrawLine(mUpperPos, mMiddlePos, Point(1.0f, 1.0f, 1.0f));

		render.DrawLine(mLowerPos + Point(0.0f, gProbeOffset, 0.0f), mLowerPos - Point(0.0f, gProbeLength, 0.0f), Point(0.0f, 1.0f, 1.0f));
	}
}

static void GetPoseFromSegment(PR& pose, const Point& p0, const Point& p1)
{
	pose.mPos = (p0+p1)*0.5f;

	const Point Dir = (p0 - p1).Normalize();

	Point Right, Up;
//	ComputeBasis2(Dir, Right, Up);

	Right = Point(0.0f, 0.0f, 1.0f);
	Up = Dir^Right;
	Up.Normalize();

	Matrix3x3 Rot;
	Rot[0] = Up;
	Rot[1] = Dir;
	Rot[2] = Right;
	pose.mRot = Rot;
}

static const char* gDesc_Walker = "Kinematic walker. The dynamic feet are attached to kinematic legs and forced into an 'impossible' situation. \
Ideally the physics engine should not freak out in that case, which might require decreasing the Baumgarte coefficient.";

START_TEST(Walker, CATEGORY_KINEMATICS, gDesc_Walker)

	Point		mPos;
	WalkCycle*	mWalkCycle[2];

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("Walker", 20);
	}

	virtual	void Walker::GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(10.44f, 6.34f, 11.60f), Point(-0.64f, -0.00f, -0.77f));
		SetDefEnv(desc, true);
	}

	virtual	bool Walker::CommonSetup()
	{
		mPos.Zero();
		mPos.y = 0.25f;
		const bool Move = false;
		mWalkCycle[0] = new WalkCycle(mPos, 0.0f, gLegWidth, Move);
		mWalkCycle[1] = new WalkCycle(mPos, PI, -gLegWidth, Move);

		return TestBase::CommonSetup();
	}

	virtual	void Walker::CommonRelease()
	{
		for(udword i=0;i<2;i++)
			DELETESINGLE(mWalkCycle[i]);

		return TestBase::CommonRelease();
	}

	virtual	void Walker::CommonUpdate(float dt)
	{
		const bool is_moving = true;
		const float mCurrentDirection = 1.0f;
		const float mWalkSpeedMultiplier = 1.0f;
//		const float mWalkSpeedMultiplier = 0.5f;
//		const float mWalkSpeedMultiplier = 0.0f;
		for(udword i=0;i<2;i++)
			mWalkCycle[i]->Update(dt, is_moving, mCurrentDirection, gWalkSpeed, mWalkSpeedMultiplier);
		return TestBase::CommonUpdate(dt);
	}

	virtual	void Walker::CommonDebugRender(PintRender& render)
	{
		if(0)
		{
			for(udword i=0;i<2;i++)
			{
				if(mWalkCycle[i])
					mWalkCycle[i]->Render(render);
			}
		}
		return TestBase::CommonDebugRender(render);
	}

	struct PintData
	{
		PintActorHandle	mUpperLeg[2];
		PintActorHandle	mLowerLeg[2];
	};

	virtual	void Walker::Close(Pint& pint)
	{
		PintData* PD = (PintData*)pint.mUserData;
		DELETESINGLE(PD);
		TestBase::Close(pint);
	}

	virtual bool Walker::Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportKinematics)
			return false;

//		mCreateDefaultEnvironment = false;

/*		if(!caps.mSupportCollisionGroups || !caps.mSupportRigidBodySimulation)
			return false;*/

		PintData* PD = new PintData;
		pint.mUserData = PD;

		for(udword i=0;i<2;i++)
		{
			{
				const Point UpperBasePos(mPos.x, mPos.y+gLowerLength+gUpperLength*0.5f, 0.0f);

				PINT_BOX_CREATE BoxDesc(gLegWidth, gUpperLength*0.5f, gLegWidth*0.5f);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= UpperBasePos;
	//			ObjectDesc.mRotation	= M;
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mKinematic	= true;
				PD->mUpperLeg[i] = CreatePintObject(pint, ObjectDesc);
			}
			{
				const Point LowerBasePos(mPos.x, mPos.y+gLowerLength*0.5f, 0.0f);

				PINT_BOX_CREATE BoxDesc(gLegWidth, gLowerLength*0.5f, gLegWidth*0.5f);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= LowerBasePos;
	//			ObjectDesc.mRotation	= M;
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mKinematic	= true;
				PD->mLowerLeg[i] = CreatePintObject(pint, ObjectDesc);
			}
			{
				const Point LowerBasePos(mPos.x, mPos.y+gLowerLength*0.5f, 0.0f);

				PINT_BOX_CREATE BoxDesc(1.0f, gLegWidth, gLegWidth*0.5f);
				BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
				BoxDesc.mMaterial	= &mZeroFrictionMaterial;	// To avoid friction forces resisting the motion of the upper kinematic legs

				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= LowerBasePos;
	//			ObjectDesc.mRotation	= M;
				ObjectDesc.mMass		= 1.0f;
				PintActorHandle h = CreatePintObject(pint, ObjectDesc);

//				PINT_FIXED_JOINT_CREATE Desc;
				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= PD->mLowerLeg[i];
				Desc.mObject1		= h;
				Desc.mLocalPivot0	= Point(0.0f, -gLowerLength*0.5f, 0.0f);
				Desc.mLocalPivot1	= Point(-1.0f, 0.0f, 0.0f);
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
//				Desc.mMaxLimitAngle	= PI/4.0f;
//				Desc.mMinLimitAngle	= 0.0f;
				Desc.mLimits.Set(-PI/4.0f, 0.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
		return true;
	}

	udword Walker::Update(Pint& pint, float dt)
	{
		const PintData* PD = (const PintData*)pint.mUserData;
		for(udword i=0;i<2;i++)
		{
			if(mWalkCycle[i])
			{
				PR Pose;

				GetPoseFromSegment(Pose, mWalkCycle[i]->mUpperPos, mWalkCycle[i]->mMiddlePos);
				pint.SetKinematicPose(PD->mUpperLeg[i], Pose);

				GetPoseFromSegment(Pose, mWalkCycle[i]->mMiddlePos, mWalkCycle[i]->mLowerPos);
				pint.SetKinematicPose(PD->mLowerLeg[i], Pose);
			}
		}
		return TestBase::Update(pint, dt);
	}

END_TEST(Walker)

///////////////////////////////////////////////////////////////////////////////

