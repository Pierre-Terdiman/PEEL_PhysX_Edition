///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEEL.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
//#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "MyConvex.h"
//#include "Loader_Bin.h"
#include "GUI_Helpers.h"
#include "Camera.h"
#include "PintSQ.h"
#include "DefaultControlInterface.h"

///////////////////////////////////////////////////////////////////////////////

static const bool gFractureLog = false;

#include ".\Fracture\Fracture.h"
#include ".\Fracture\Convex.h"
#include ".\Fracture\MultiConvex.h"
#include ".\Fracture\FracturePattern.h"

static const char* gDesc_VoronoiFracture = "Stress test for Voronoi-based fractures.";

#define NB_FRACTURED_OBJECTS	4

START_TEST(VoronoiFracture, CATEGORY_FRACTURE, gDesc_VoronoiFracture)

	FracturePattern mFracPattern;
	PtrContainer	mMultiConvexes;
	bool			mRelease;

	virtual	float	GetRenderData(Point& center)	const	{ return 100.0f;	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(19.97f, 5.78f, 19.70f), Point(-0.65f, -0.30f, -0.69f));
		SetDefEnv(desc, true);
		mRelease = false;
	}

	virtual	bool	CommonSetup()
	{
		mRelease = false;
		{
			const int NumCells = 100;
//			mFracPattern.create3dVoronoi(Point(30.0f, 30.0f, 30.0f), NumCells, 10.0f);
//			mFracPattern.create3dVoronoi(Point(10.0f, 10.0f, 10.0f), NumCells, 10.0f);
//			mFracPattern.create3dVoronoi(Point(3.0f, 3.0f, 3.0f), NumCells, 10.0f);
//			mFracPattern.create3dVoronoi(Point(3.0f, 3.0f, 3.0f), NumCells, 5.0f);
			mFracPattern.create3dVoronoi(Point(3.0f, 3.0f, 3.0f), NumCells, 8.0f);
//			mFracPattern.create3dVoronoi(Point(3.0f, 3.0f, 3.0f), NumCells, 2.0f);
//			mFracPattern.create3dVoronoi(2.0f*Point(3.0f, 3.0f, 3.0f), NumCells, 10.0f);
		}

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
		mRelease = true;
		DeleteOwnedObjects<MultiConvex>(mMultiConvexes);
		mFracPattern.clear();
		TestBase::CommonRelease();
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)	
	{
		if(mRelease)
			return;
		const udword Nb = mMultiConvexes.GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			MultiConvex* MC = reinterpret_cast<MultiConvex*>(mMultiConvexes[i]);
			if(MC->DeleteConvex(pint, removed_object))
				return;
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		std::vector<Point> polyPoints;

	//	const float r = 0.5f;
		const float r = 1.0f;
	//	const float r = 2.0f;
		const float h = 2.0f;
	//	const float h = 20.0f;
		const udword numSegs = 30;
		const float dphi = TWOPI / numSegs;
		for(udword i=0; i<numSegs; i++)
			polyPoints.push_back(Point(-r * cosf(i*dphi), 0.0f, -r * sinf(i*dphi)));
		const FractureConvex c(udword(polyPoints.size()), &polyPoints[0], h);

		for(udword j=0;j<NB_FRACTURED_OBJECTS;j++)
		{
			for(udword i=0;i<NB_FRACTURED_OBJECTS;i++)
			{
				const Point Offset(float(i)*4.0f, 0.0f, float(j)*4.0f);

				MultiConvex* MC = ICE_NEW(MultiConvex);
				mMultiConvexes.AddPtr(MC);

				MC->createFromConvex(pint, &c, Point(0.0f, h, 0.0f)+Offset);

				/*
						float vel = 1.0f;
						float dist;
						int convexNr;
						if (MC->rayCast(ray, dist, convexNr))
						{
			//				MC->randomFracture(convexNr, ray, 2, 5.0f);
							MC->patternFracture(convexNr, ray, mFracPattern, 10.0f, vel);
						}*/
				if(1)
				{
			//		float vel = 1.0f;
					float vel = 0.0f;
					Ray R(Point(0.0f, 10.0f, 0.0f)+Offset, Point(0.0f, -1.0f, 0.0f));
//					Ray R(Point(0.0f, -10.0f, 0.0f)+Offset, Point(0.0f, 1.0f, 0.0f));
					MC->patternFracture(pint, 0, R, mFracPattern, 10.0f, vel);
//					MC->randomFracture(pint, 0, R, 2, vel);
				}
			}
		}

/*		if(0)
		{
			const float Altitude = -1.0f;
			CreatePlanarMesh(*this, pint, Altitude);
			mCreateDefaultEnvironment = false;
		}*/

		return true;
	}

END_TEST(VoronoiFracture)

///////////////////////////////////////////////////////////////////////////////

//#define AUTOMATIC_BULLET
#ifdef AUTOMATIC_BULLET
	#include "ToolBullet.h"
#endif

//#include "ConvexHull.h"
static const char* gDesc_VoronoiFracture2 = "Runtime fracture. Press SHIFT + left mouse button to fracture the object. You can also use the 'Bullet' tool to shoot bullets at the scene.";

//START_TEST(VoronoiFracture2, CATEGORY_FRACTURE, gDesc_VoronoiFracture2)

class VoronoiFracture2 : public TestBase, public PintContactNotifyCallback
{
			ComboBoxPtr		mComboBox_ConvexIndex;
			EditBoxPtr		mEditBox_NbCells;
			EditBoxPtr		mEditBox_BiasExp;
			EditBoxPtr		mEditBox_SizeX;
			EditBoxPtr		mEditBox_SizeY;
			EditBoxPtr		mEditBox_SizeZ;
			EditBoxPtr		mEditBox_MinSize;
			CheckBoxPtr		mCheckBox_Kinematic;
#ifdef AUTOMATIC_BULLET
			// Savage hack I know, or is it?
			ToolBullet		mToolBullet;
#endif
	public:
							VoronoiFracture2()			{									}
	virtual					~VoronoiFracture2()			{									}
	virtual	const char*		GetName()			const	{ return "VoronoiFracture2";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_VoronoiFracture2;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_FRACTURE;			}

	PtrContainer	mEngines;
	FracturePattern mFracPattern;
	FracturePattern mFracPattern2;
	FracturePattern mFracPattern3;
	FracturePattern mFracPattern4;
	FractureManager	mFractureManager;

	virtual	float	GetRenderData(Point& center)	const	{ return 50.0f;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 256;
		WD.mHeight	= 256;
		WD.mLabel	= "VoronoiFracture";
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
		{
			y += YStep;
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Convex:", &UIElems);
			mComboBox_ConvexIndex = CreateConvexObjectComboBox(UI, 4+OffsetX, y, true);
			mComboBox_ConvexIndex->Add("Cube");
			mComboBox_ConvexIndex->Select(CONVEX_INDEX_1);
			RegisterUIElement(mComboBox_ConvexIndex);
			y += YStep;
		}

		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Nb cells:", &UIElems);
			mEditBox_NbCells = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "100", &UIElems, EDITBOX_INTEGER_POSITIVE, null, null);
			y += YStep;
		}

		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Bias exp:", &UIElems);
			mEditBox_BiasExp = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "8.0", &UIElems, EDITBOX_FLOAT, null, null);
			y += YStep;
		}

		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Size X:", &UIElems);
			mEditBox_SizeX = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "30.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Size Y:", &UIElems);
			mEditBox_SizeY = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "30.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Size Z:", &UIElems);
			mEditBox_SizeZ = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "30.0", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}
		{
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "Min size:", &UIElems);
			mEditBox_MinSize = helper.CreateEditBox(UI, 1, 4+OffsetX, y, EditBoxWidth, 20, "0.1", &UIElems, EDITBOX_FLOAT_POSITIVE, null, null);
			y += YStep;
		}
		{
			mCheckBox_Kinematic = helper.CreateCheckBox(UI, 0, 4, y, 400, 20, "Start as kinematic", &UIElems, true, null, null);
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 256-16);

		return null;
	}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0]					= PintCameraPose(Point(2.60f, 4.04f, 2.71f), Point(-0.59f, -0.55f, -0.59f));
		desc.mCamera[1]					= PintCameraPose(Point(6.92f, 1.37f, 5.02f), Point(-0.79f, 0.20f, -0.58f));
		desc.mContactNotifyCallback		= this;
		desc.mCreateDefaultEnvironment	= true;
	}

	virtual	bool	CommonSetup()
	{
		const udword NumCells = GetInt(100, mEditBox_NbCells);
		const float BiasExp = GetFloat(8.0f, mEditBox_BiasExp);
		const float SizeX = GetFloat(30.0f, mEditBox_SizeX);
		const float SizeY = GetFloat(30.0f, mEditBox_SizeY);
		const float SizeZ = GetFloat(30.0f, mEditBox_SizeZ);
		mFracPattern.create3dVoronoi(Point(SizeX, SizeY, SizeZ), NumCells, BiasExp, 42);
		mFracPattern2.create3dVoronoi(Point(SizeX, SizeY, SizeZ), NumCells, BiasExp, 1234);
		mFracPattern3.create3dVoronoi(Point(SizeX, SizeY, SizeZ), NumCells, BiasExp, 5678);
		mFracPattern4.create3dVoronoi(Point(SizeX, SizeY, SizeZ), NumCells, BiasExp, 0xdeadbeef);

#ifdef AUTOMATIC_BULLET
		mToolBullet.mDir = Point(-1.0f, 0.0f, 0.0f);
		mToolBullet.mOrigin = Point(10.0f, 2.5f, 0.0f);
#endif

		return TestBase::CommonSetup();
	}

	virtual	void	CommonRelease()
	{
#ifdef AUTOMATIC_BULLET
		mToolBullet.Reset(INVALID_ID);
#endif

		mEngines.Empty();

		mFractureManager.Release();

		mFracPattern4.clear();
		mFracPattern3.clear();
		mFracPattern2.clear();
		mFracPattern.clear();

		TestBase::CommonRelease();
	}

	float	ComputeRoughSize(udword nb_verts, const Point* verts)	const
	{
		AABB ConvexBounds;
		ComputeAABB(ConvexBounds, verts, nb_verts);

		if(0)
		{
			Point Extents;
			ConvexBounds.GetExtents(Extents);
			return Extents.x * Extents.y * Extents.z * 8.0f;
		}

		Point Diagonal;
		ConvexBounds.GetDiagonal(Diagonal);
		if(gFractureLog)
			printf("Rough size: %f\n", Diagonal.Magnitude());

		const float Coeff = 1.0f;
//		const float Coeff = 10.0f;

		return Coeff * Diagonal.Magnitude();
	}

	void	CreateInitialConvex(Pint& pint, udword nb_verts, const Point* verts, udword nb_polygons, const Container& polygon_data, bool kinematic, float h)
	{
		//Convex* c = ICE_NEW(Convex)(nb_verts, verts, nb_polygons, (const int*)polygon_data.GetEntries());
		FractureConvex* c = ICE_NEW(FractureConvex)(nb_verts, verts, nb_polygons, polygon_data.GetEntries());
		Point Offset = c->centerAtZero();
		Offset.y += h;

		const float RoughSize = ComputeRoughSize(nb_verts, verts);

		mFractureManager.RegisterConvex(pint, c, RoughSize, Offset, Quat(Idt), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), kinematic);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

#ifdef AUTOMATIC_BULLET
		mToolBullet.RightDownCallback(pint, INVALID_ID);
#endif

		// ### not always needed in this test
//		if(!caps.mSupportContactNotifications)
//			return false;

		mEngines.AddPtr(&pint);

		ConvexIndex Index = CONVEX_INDEX_0;
		if(mComboBox_ConvexIndex)
			Index = ConvexIndex(mComboBox_ConvexIndex->GetSelectedIndex());

		const bool StartAsKinematic = mCheckBox_Kinematic ? mCheckBox_Kinematic->IsChecked() : false;

		if(Index==CONVEX_INDEX_13+1)
		{
			AABB Box;
			Box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), Point(1.0f, 1.0f, 1.0f));

			Point Pts[8];
			Box.ComputePoints(Pts);

			ConvexHull2* CH = CreateConvexHull2(8, Pts);

			Container tmp;
			const udword NbPolygons = CH->GetNbPolygons();
			for(udword i=0;i<NbPolygons;i++)
			{
				const HullPolygon& mp = CH->GetPolygon(i);
				tmp.Add(udword(mp.mNbVerts));

				for(udword j=0;j<mp.mNbVerts;j++)
				{
					tmp.Add(udword(mp.mVRef[j]));
				}
			}

			CreateInitialConvex(pint, CH->GetNbVerts(), CH->GetVerts(), NbPolygons, tmp, StartAsKinematic, 2.0f);

			DELETESINGLE(CH);
		}
		else
		{
			MyConvex C;
			C.LoadFile(Index);

			Container tmp;
			for(int i=0;i<C.mNbPolys;i++)
			{
				const MyPoly& mp = C.mPolys[i];
				tmp.Add(udword(mp.mNbVerts));
				for(int j=0;j<mp.mNbVerts;j++)
				{
					tmp.Add(udword(mp.mIndices[j]));
				}
			}
			
			CreateInitialConvex(pint, C.mNbVerts, C.mVerts, C.mNbPolys, tmp, StartAsKinematic, 2.0f);
		}
		return true;
	}

	virtual	void	OnObjectReleased(Pint& pint, PintActorHandle removed_object)	
	{
		udword Index;
		FractureConvex* c = mFractureManager.FindConvex(removed_object, &Index);
		if(c)
		{
			c->Release();
			mFractureManager.RemoveLink(Index);
		}
	}

	virtual	bool	BufferContacts()		const	{ return true;			}
	virtual	udword	GetContactFlags()		const	{ return CONTACT_FOUND;	}
	virtual	float	GetContactThreshold()	const	{ return 1000.0f;		}

	virtual	void	OnContact(Pint& pint, udword nb_contacts, const PintContactData* contacts)
	{
//		printf("OnContact\n");

		while(nb_contacts--)
		{
			DoFracture(contacts->mObject0, contacts->mWorldPos, &pint);
			DoFracture(contacts->mObject1, contacts->mWorldPos, &pint);
			contacts++;
		}
	}

//	virtual	bool	MiddleDownCallback(const MouseInfo& mouse)
	virtual	bool	LeftDownCallback(const MouseInfo& mouse)
	{
		if(!IsKeyPressed(VK_SHIFT))
			return true;

		const Point Dir = ComputeWorldRay(int(mouse.mMouseX), int(mouse.mMouseY));
		const Point Pos = GetCameraPos();

		PintRaycastData RD;
		RD.mOrigin	= Pos;
		RD.mDir		= Dir;
		RD.mMaxDist	= 5000.0f;

		const udword NbEngines = mEngines.GetNbEntries();
		for(udword i=0;i<NbEngines;i++)
		{
			Pint* Engine = reinterpret_cast<Pint*>(mEngines[i]);
			PintRaycastHit Hit;
			udword NbHits = Engine->BatchRaycasts(Engine->mSQHelper->GetThreadContext(), 1, &Hit, &RD);
			if(NbHits)
				DoFracture(Hit.mTouchedActor, Hit.mImpact, Engine);
		}

		return false;
	}

	bool	DoFracture(PintActorHandle object, const Point& world_pt, Pint* pint)
	{
		udword Index;
		FractureConvex* c = mFractureManager.FindConvex(object, &Index);
		if(!c)
			return false;

		const float MinSize = GetFloat(0.1f, mEditBox_MinSize);
		const float RoughSize = ComputeRoughSize(c->GetNbVerts(), c->GetVerts());
		if(RoughSize<MinSize)
		{
			if(gFractureLog)
				printf("Too small - cannot fracture\n");
			return false;
		}
		else
		{
			const PR WorldPose = pint->GetWorldTransform(object);
			const Point LocalHit = ComputeLocalPoint(world_pt, WorldPose);

			PtrContainer newConvexes;
			if(1)
			{
				static int count=0;
				if(count==0)
					mFracPattern.getConvexIntersection(c, LocalHit, 1.0f, newConvexes);
				else if(count==1)
					mFracPattern2.getConvexIntersection(c, LocalHit, 1.0f, newConvexes);
				else if(count==2)
					mFracPattern3.getConvexIntersection(c, LocalHit, 1.0f, newConvexes);
				else if(count==3)
					mFracPattern4.getConvexIntersection(c, LocalHit, 1.0f, newConvexes);
				count++;
				if(count==4)
					count=0;
			}
			else
			{
				Point N(1.0f, 1.0f, 1.0f);
				N.Normalize();
				const Plane LocalPlane(LocalHit, N);

				FractureConvex* c0;
				FractureConvex* c1;
				if(c->cut(LocalPlane.n, -LocalPlane.d, c0, c1))
				{
					//newConvexes.push_back(c0);
					newConvexes.AddPtr(c0);
					//newConvexes.push_back(c1);
					newConvexes.AddPtr(c1);
				}
			}

			const udword NbNewConvexes = newConvexes.GetNbEntries();
			if(NbNewConvexes)
			{
				const Point LinVel = pint->GetLinearVelocity(object);
				const Point AngVel = pint->GetAngularVelocity(object);

				const bool KineParent = c->mKinematic;

				c->Release();
				mFractureManager.RemoveLink(Index);
				// Calling ReleasePintObject would call OnObjectReleased again, which
				// would do FindConvex / RemoveLink again. We copy parts of the code
				// here again instead.
				//ReleasePintObject(*pint, object, true);
				{
					GetDefaultControlInterface().mSelMan.RemoveFromSelection(pint, object);

					pint->_ReleaseObject(object);
				}

				for(udword i=0; i<NbNewConvexes; i++)
				{
					FractureConvex* cn = reinterpret_cast<FractureConvex*>(newConvexes[i]);

					if(!cn->GetNbVerts())
					{
						cn->Release();
						continue;
					}

					bool Kinematic;

					float RoughSize = 0.0f;
					{
						AABB ConvexBounds;
						ComputeAABB(ConvexBounds, cn->GetVerts(), cn->GetNbVerts());
						Point Diagonal;
						ConvexBounds.GetDiagonal(Diagonal);
						if(gFractureLog)
							printf("Rough size: %f\n", Diagonal.Magnitude());
						RoughSize = Diagonal.Magnitude();

//						Kinematic = ConvexBounds.ContainsPoint(LocalHit)==0;
//						Kinematic = KineParent ? SphereAABB(LocalHit, 0.2f, ConvexBounds)==0 : false;
						const Sphere S(LocalHit, 1.0f);
						Kinematic = KineParent ? S.Contains(ConvexBounds)==0 : false;
					}
					if(RoughSize<MinSize)
					{
						if(gFractureLog)
							printf("Too small - discarded\n");
						cn->Release();
						continue;
					}

					Point off = cn->centerAtZero();
					Matrix4x4 posei = WorldPose;
						Point Tmp = posei.GetTrans();
						Tmp += off * Matrix3x3(WorldPose.mRot);
						posei.SetTrans(Tmp);
					Point v = Tmp - world_pt;
					v.Normalize();
					v *= 5.0f;

						const Matrix3x3 Rot(posei);
						//ObjectDesc.mLinearVelocity	= v + LinVel * Matrix3x3(posei);
						mFractureManager.RegisterConvex(*pint, cn, RoughSize, posei.GetTrans(), Rot, LinVel * Rot, AngVel * Rot, Kinematic);
				}
			}
		}
		return true;
	}

	virtual	bool	OnBulletHit(Pint& pint, const PintRaycastHit& hit, const Ray& world_ray)
	{
		return DoFracture(hit.mTouchedActor, hit.mImpact, &pint);
	}

#ifdef AUTOMATIC_BULLET
	virtual	void					CommonUpdate(float dt)						override
	{
		//TestBase::CommonUpdate(dt);
		mToolBullet.PreRenderCallback();
	}

	virtual	void					DrawDebugInfo(Pint& pint, PintRender& render)	override
	{
		mToolBullet.RenderCallback(pint, INVALID_ID);
	}

/*	virtual	udword					Update(Pint& pint, float dt)
	{
		//mToolBullet.RenderCallback(pint, INVALID_ID);
		return TestBase::Update(pint, dt);
	}

	virtual	void					CommonRender(PintRender&, PintRenderPass)	override
	{
		//TestBase::CommonRender(dt);
	}

	virtual	void					CommonDebugRender(PintRender&)				override
	{
		//TestBase::CommonUpdate(dt);
	}*/
#endif

END_TEST(VoronoiFracture2)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_VoronoiFracture3 = "Fracturable box stack. Press SHIFT + left mouse button to fracture the object. You can also use the 'Bullet' tool to shoot bullets at the scene.";

class VoronoiFracture3 : public VoronoiFracture2
{
	public:
	virtual	const char*		GetName()			const	{ return "VoronoiFracture3";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_VoronoiFracture3;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_FRACTURE;			}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(1.71f, 3.22f, 3.57f), Point(-0.43f, -0.30f, -0.85f));
		desc.mContactNotifyCallback		= this;
		desc.mCreateDefaultEnvironment	= true;
	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		return CreateOverrideTabControl("Fracturable box stack", 20);
		//return VoronoiFracture2::InitUI(helper);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		// ### not always needed in this test
//		if(!caps.mSupportContactNotifications)
//			return false;

		mEngines.AddPtr(&pint);

		const udword nb_stacks = 1;
		udword nb_base_boxes = 10;
		//bool CreateBoxStack(Pint& pint, const PintCaps& caps, const udword nb_stacks, udword nb_base_boxes)
		{
			const float BoxExtent = 0.2f;
//			const float BoxExtent = 0.4f;
//			const float BoxExtent = 1.0f;
			//PINT_BOX_CREATE BoxDesc(BoxExtent, BoxExtent, BoxExtent);
			//BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);

			AABB Box;
			Box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), Point(BoxExtent, BoxExtent, BoxExtent));

			Point Pts[8];
			Box.ComputePoints(Pts);

			ConvexHull2* CH = CreateConvexHull2(8, Pts);
			const udword NbVerts = CH->GetNbVerts();
			const Point* Verts = CH->GetVerts();

			Container tmp;
			const udword NbPolygons = CH->GetNbPolygons();
			for(udword i=0;i<NbPolygons;i++)
			{
				const HullPolygon& mp = CH->GetPolygon(i);
				tmp.Add(udword(mp.mNbVerts));

				for(udword j=0;j<mp.mNbVerts;j++)
				{
					tmp.Add(udword(mp.mVRef[j]));
				}
			}

			BasicRandom Rnd(42);

			const udword NbStacks = nb_stacks;
			for(udword j=0;j<NbStacks;j++)
			{
				const float CoeffZ = NbStacks==1 ? 0.0f : float(j) - float(NbStacks)*0.5f;
				udword NbBoxes = nb_base_boxes;
				float BoxPosY = BoxExtent;
				while(NbBoxes)
				{
					for(udword i=0;i<NbBoxes;i++)
					{
				//		const float CoeffX = float(i)/float(NbBoxes-1);
						const float CoeffX = float(i) - float(NbBoxes)*0.5f;

						// Perturb position a bit because some engines have FPU accuracy issues with objects
						// exactly stacked on top of each-other.
						const float Noise = Rnd.RandomFloat()*0.001f;

						const Point Pos(CoeffX * BoxExtent * 2.0f, BoxPosY, CoeffZ * BoxExtent * 4.0f + Noise);

						//CreateInitialConvex(pint, CH->GetNbVerts(), CH->GetVerts(), NbPolygons, tmp, false, 0.0f);
							FractureConvex* c = ICE_NEW(FractureConvex)(NbVerts, Verts, NbPolygons, tmp.GetEntries());

							const float RoughSize = ComputeRoughSize(NbVerts, Verts);

							mFractureManager.RegisterConvex(pint, c, RoughSize, Pos, Quat(Idt), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), false);
					}

					NbBoxes--;
					BoxPosY += BoxExtent*2.0f;
				}
			}
			DELETESINGLE(CH);
		}
		return true;
	}


END_TEST(VoronoiFracture3)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_VoronoiFracture4 = "Fracturable tiles. Press SHIFT + left mouse button to fracture the object. You can also use the 'Bullet' tool to shoot bullets at the scene.";

class VoronoiFracture4 : public VoronoiFracture2
{
	public:
	virtual	const char*		GetName()			const	{ return "VoronoiFracture4";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_VoronoiFracture4;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_FRACTURE;			}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(1.71f, 3.22f, 3.57f), Point(-0.43f, -0.30f, -0.85f));
		desc.mContactNotifyCallback		= this;
		desc.mCreateDefaultEnvironment	= true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		// ### not always needed in this test
//		if(!caps.mSupportContactNotifications)
//			return false;

		mEngines.AddPtr(&pint);

		const Point Extents(0.4f, 0.1f, 0.4f);

		AABB Box;
		Box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), Extents);

		Point Pts[8];
		Box.ComputePoints(Pts);

		ConvexHull2* CH = CreateConvexHull2(8, Pts);
		const udword NbVerts = CH->GetNbVerts();
		const Point* Verts = CH->GetVerts();

		Container tmp;
		const udword NbPolygons = CH->GetNbPolygons();
		for(udword i=0;i<NbPolygons;i++)
		{
			const HullPolygon& mp = CH->GetPolygon(i);
			tmp.Add(udword(mp.mNbVerts));

			for(udword j=0;j<mp.mNbVerts;j++)
			{
				tmp.Add(udword(mp.mVRef[j]));
			}
		}

		const udword Nb = 20;
		for(udword j=0;j<Nb;j++)
		{
			//const float CoeffY = float(j)/float(Nb-1);
			const float CoeffZ = float(j);
			for(udword i=0;i<Nb;i++)
			{
				//const float CoeffX = float(i)/float(Nb-1);
				const float CoeffX = float(i);
				const Point Pos(CoeffX * Extents.x * 2.0f, Extents.y, CoeffZ * Extents.z * 2.0f);

				//CreateInitialConvex(pint, CH->GetNbVerts(), CH->GetVerts(), NbPolygons, tmp, false, 0.0f);

				FractureConvex* c = ICE_NEW(FractureConvex)(NbVerts, Verts, NbPolygons, tmp.GetEntries());

				const float RoughSize = ComputeRoughSize(NbVerts, Verts);

				// ### we want static, not kine
				//mFractureManager.RegisterConvex(pint, c, RoughSize, Pos, Quat(Idt), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), true);
				mFractureManager.RegisterConvex(pint, c, 0.0f, Pos, Quat(Idt), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), false);
			}
		}
		DELETESINGLE(CH);
		return true;
	}

END_TEST(VoronoiFracture4)

///////////////////////////////////////////////////////////////////////////////

#include "Voronoi2D.h"

static const char* gDesc_VoronoiFracture5 = "Fracturable walls. Press SHIFT + left mouse button to fracture the object. You can also use the 'Bullet' tool to shoot bullets at the scene.";
static const float gRoomSize = 10.0f;

class VoronoiFracture5 : public VoronoiFracture2, public VoronoiCallback
{
	public:
	virtual	const char*		GetName()			const	{ return "VoronoiFracture5";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_VoronoiFracture5;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_FRACTURE;			}

	virtual void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		//desc.mCamera[0] = PintCameraPose(Point(1.71f, 3.22f, 3.57f), Point(-0.43f, -0.30f, -0.85f));
		desc.mCamera[0] = PintCameraPose(Point(12.39f, 0.93f, 10.75f), Point(-0.72f, 0.30f, -0.63f));
		desc.mContactNotifyCallback		= this;
		desc.mCreateDefaultEnvironment	= true;
	}

	Pint*	mPint;
	virtual	void	OnGeneratedCell(const Point2D& site, udword nb_verts, const Point2D* verts)
	{
		if(!mPint)
			return;

		const float GlobalScale = 1.0f;
		const bool Kine = false;

		if(1)
		{
			// ### this codepath works, the other one doesn't!
			//const float h = 4.0f*GlobalScale;
			const float h = 0.2f*GlobalScale;

			//Point* Pts = ICE_NEW(Point)[nb_verts*2];
			Point* Pts = ICE_NEW(Point)[nb_verts];
			Point Center(0.0f, 0.0f, 0.0f);
			for(udword i=0;i<nb_verts;i++)
			{
				Pts[i] = Point(verts[i].x * GlobalScale, 0.0f, verts[i].y * GlobalScale);
				//Pts[i+nb_verts] = Point(verts[i].x * GlobalScale, h, verts[i].y * GlobalScale);
				Center += Pts[i];
			}
			Center /= float(nb_verts);
			for(udword i=0;i<nb_verts;i++)
			{
				Pts[i] -= Center;
				Pts[i].y += h*0.5f;
			}

			const float RoughSize = 0.0f;//ComputeRoughSize(nb_verts*2, Pts);
			const float WorldSize = gRoomSize;// + h*2.0f;

			{
				const Point Pos = Center + Point(0.0f, h*0.5f, 0.0f);

				FractureConvex* c = ICE_NEW(FractureConvex)(nb_verts, Pts, h);
				mFractureManager.RegisterConvex(*mPint, c, RoughSize, Pos, Quat(Idt), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), Kine);
			}

			{
				Matrix4x4 M;
				M.RotX(PI);
				M.SetTrans(Point(0.0f, WorldSize+h, 0.0f));

				const Point Pos = Center + Point(0.0f, h*0.5f, 0.0f);

				FractureConvex* c = ICE_NEW(FractureConvex)(nb_verts, Pts, h);
				mFractureManager.RegisterConvex(*mPint, c, RoughSize, Pos*M, M, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), Kine);
			}

			{
				Matrix4x4 M;
				M.RotX(HALFPI);
				M.SetTrans(Point(0.0f, WorldSize*0.5f, WorldSize*0.5f));

				const Point Pos = Center + Point(0.0f, h*0.5f, 0.0f);

				FractureConvex* c = ICE_NEW(FractureConvex)(nb_verts, Pts, h);
				mFractureManager.RegisterConvex(*mPint, c, RoughSize, Pos*M, M, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), Kine);
			}

			{
				Matrix4x4 M;
				M.RotX(-HALFPI);
				//M.SetTrans(Point(0.0f, WorldSize*0.5f + h, -WorldSize*0.5f - h));
				M.SetTrans(Point(0.0f, WorldSize*0.5f, -WorldSize*0.5f));

				const Point Pos = Center + Point(0.0f, h*0.5f, 0.0f);

				FractureConvex* c = ICE_NEW(FractureConvex)(nb_verts, Pts, h);
				mFractureManager.RegisterConvex(*mPint, c, RoughSize, Pos*M, M, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), Kine);
			}

			{
				Matrix4x4 M;
				M.RotZ(HALFPI);
				M.SetTrans(Point(WorldSize*0.5f + h, WorldSize*0.5f, 0.0f));

				const Point Pos = Center + Point(0.0f, h*0.5f, 0.0f);

				FractureConvex* c = ICE_NEW(FractureConvex)(nb_verts, Pts, h);
				mFractureManager.RegisterConvex(*mPint, c, RoughSize, Pos*M, M, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), Kine);
			}

			{
				Matrix4x4 M;
				M.RotZ(-HALFPI);
				M.SetTrans(Point(-WorldSize*0.5f - h, WorldSize*0.5f, 0.0f));

				const Point Pos = Center + Point(0.0f, h*0.5f, 0.0f);

				FractureConvex* c = ICE_NEW(FractureConvex)(nb_verts, Pts, h);
				mFractureManager.RegisterConvex(*mPint, c, RoughSize, Pos*M, M, Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), Kine);
			}

			DELETEARRAY(Pts);
			//mPint = null;
			return;
		}

		// ### TODO: figure out why this codepath doesn't work

		Point* Pts = ICE_NEW(Point)[nb_verts*2];
		Point Center(0.0f, 0.0f, 0.0f);
		for(udword i=0;i<nb_verts;i++)
		{
			Pts[i] = Point(verts[i].x, 0.0f, verts[i].y) * GlobalScale;
			Pts[i+nb_verts] = Point(verts[i].x, 10.0f, verts[i].y) * GlobalScale;
			//Pts[i+nb_verts] = Point(0.0f, 10.0f, 0.0f) * GlobalScale;
			Center += Pts[i];
			Center += Pts[i+nb_verts];
		}
		Center /= float(nb_verts*2);
		for(udword i=0;i<nb_verts*2;i++)
			Pts[i] -= Center;

		ConvexHull2* CH = CreateConvexHull2(nb_verts*2, Pts);
		const udword NbVerts = CH->GetNbVerts();
		const Point* Verts = CH->GetVerts();

		Container tmp;
		const udword NbPolygons = CH->GetNbPolygons();
		for(udword i=0;i<NbPolygons;i++)
		{
			const HullPolygon& mp = CH->GetPolygon(i);
			tmp.Add(udword(mp.mNbVerts));

			for(udword j=0;j<mp.mNbVerts;j++)
			{
				//const udword jj = mp.mNbVerts - 1 - j;
				//tmp.Add(udword(mp.mVRef[jj]));
				tmp.Add(udword(mp.mVRef[j]));
			}
		}

//		Center.Zero();
//		CreateInitialConvex(*mPint, NbVerts, Pts, NbPolygons, tmp, false, 0.0f);
		FractureConvex* c = ICE_NEW(FractureConvex)(NbVerts, Pts, NbPolygons, tmp.GetEntries());
		const float RoughSize = ComputeRoughSize(NbVerts, Pts);
		mFractureManager.RegisterConvex(*mPint, c, RoughSize, Center, Quat(Idt), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 0.0f, 0.0f), Kine);
		DELETESINGLE(CH);

		DELETEARRAY(Pts);

		mPint = null;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportRigidBodySimulation)
			return false;

		// ### not always needed in this test
//		if(!caps.mSupportContactNotifications)
//			return false;

		mEngines.AddPtr(&pint);

//		const udword NbCells = 32;
//		const udword NbCells = 64;
		const udword NbCells = 128;
//		const udword NbCells = 4;
		const float ScaleX = gRoomSize;
		const float ScaleZ = gRoomSize;

		mPint = &pint;
		GenerateVoronoiCells(NbCells, ScaleX, ScaleZ, *this);

		return true;
	}

	virtual void	CommonDebugRender(PintRender& renderer)
	{
		if(0)
		{
			const float SizeX = 10.0f*0.5f;
			const float SizeY = 1.0f;
			const float SizeZ = SizeX;

			AABB Bounds;
			Bounds.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), Point(SizeX, SizeY, SizeZ));
			renderer.DrawWireframeAABB(1, &Bounds, Point(1.0f, 0.0f, 0.0f));
		}
	}

END_TEST(VoronoiFracture5)

///////////////////////////////////////////////////////////////////////////////

