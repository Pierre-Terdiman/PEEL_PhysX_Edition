///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestScenes.h"
//#include "TestScenesHelpers.h"
//#include "Loader_Bin.h"
#include "PintSQ.h"
//#include "PintShapeRenderer.h"
#include "ZB2Import.h"

///////////////////////////////////////////////////////////////////////////////

struct MeshMeshHits : Allocateable
{
	MeshMeshHits(PintActorHandle mesh_handle0, PintActorHandle mesh_handle1) : mMeshHandle0(mesh_handle0), mMeshHandle1(mesh_handle1)	{}

	//PintMultipleHits	mHits;
	Container			mResults;
	PintActorHandle		mMeshHandle0;
	PintActorHandle		mMeshHandle1;
};

class MidphaseMeshMeshTest : public TestBase
{
	public:
							MidphaseMeshMeshTest()		{										}
	virtual					~MidphaseMeshMeshTest()		{										}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_MIDPHASE_MESH_MESH;	}
	virtual	udword			GetProfilingFlags()	const	{ return PROFILING_TEST_UPDATE;			}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(50.00f, 50.00f, 50.00f), Point(-0.44f, -0.57f, -0.69f));
		SetDefEnv(desc, false);
	}

/*	virtual bool	CommonSetup()	override
	{
		TestBase::CommonSetup();
		//LoadMeshesFromFile_(*this, "testzone.bin");
		return true;
	}*/

	virtual	void	Close(Pint& pint)	override
	{
		MeshMeshHits* MMH = reinterpret_cast<MeshMeshHits*>(pint.mUserData);
		DELETESINGLE(MMH);
	}

	virtual	void	DrawDebugInfo(Pint& pint, PintRender& render)	override
	{
		if(!pint.mUserData)
			return;
		const MeshMeshHits* MMH = reinterpret_cast<const MeshMeshHits*>(pint.mUserData);

		const PintActorHandle Handle0 = MMH->mMeshHandle0;
		const Matrix4x4 GlobalPose0 = pint.GetWorldTransform(Handle0);

		const PintActorHandle Handle1 = MMH->mMeshHandle1;
		const Matrix4x4 GlobalPose1 = pint.GetWorldTransform(Handle1);

		const Point Red(1.0f, 0.0f, 0.0f);
		const Point Green(0.0f, 1.0f, 0.0f);
		Pint_Actor* ActorAPI = pint.GetActorAPI();
		if(ActorAPI)
		{
			const PintShapeHandle ShapeHandle0 = ActorAPI->GetShape(Handle0, 0);	// ####
			const PintShapeHandle ShapeHandle1 = ActorAPI->GetShape(Handle1, 0);	// ####

			Pint_Shape* ShapeAPI = pint.GetShapeAPI();
			if(ShapeAPI)
			{
				Triangle tri0;
				Triangle tri1;
				udword NbPairs = MMH->mResults.GetNbEntries()/2;
				const udword* Indices = MMH->mResults.GetEntries();
				while(NbPairs--)
				{
					const udword Index0 = *Indices++;
					const udword Index1 = *Indices++;

					{
						bool Status = ShapeAPI->GetTriangle(tri0, ShapeHandle0, Index0);

						const Point v0 = tri0.mVerts[0] * GlobalPose0;
						const Point v1 = tri0.mVerts[1] * GlobalPose0;
						const Point v2 = tri0.mVerts[2] * GlobalPose0;

						render.DrawLine(v0, v1, Red);
						render.DrawLine(v1, v2, Red);
						render.DrawLine(v2, v0, Red);
					}

					{
						bool Status = ShapeAPI->GetTriangle(tri1, ShapeHandle1, Index1);

						const Point v0 = tri1.mVerts[0] * GlobalPose1;
						const Point v1 = tri1.mVerts[1] * GlobalPose1;
						const Point v2 = tri1.mVerts[2] * GlobalPose1;

						render.DrawLine(v0, v1, Green);
						render.DrawLine(v1, v2, Green);
						render.DrawLine(v2, v0, Green);
					}
				}
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

namespace
{
enum MeshMeshIndex
{
	MESH_MESH_VENUS,
	MESH_MESH_TESSVENUS,
	MESH_MESH_COW_KNOT,
	MESH_MESH_BUNNY,
	MESH_MESH_TESSBUNNY,
	MESH_MESH_BUDDHAS,
	MESH_MESH_MULTI_BUDDHAS,
	MESH_MESH_MEGA_BUDDHAS,
};
}

/*static bool MeshMeshSetup(Pint& pint, SurfaceManager& sm, const PintCaps& caps)
{
	PtrContainer Objects;
	if(!sm.CreateMeshesFromRegisteredSurfaces(pint, caps, null, &Objects))
		return false;

	ASSERT(Objects.GetNbEntries()==2);
	if(!Objects.GetNbEntries())
		return false;
	PintActorHandle MeshHandle0 = PintActorHandle(Objects[0]);
	ASSERT(MeshHandle0);

	PintActorHandle MeshHandle1 = PintActorHandle(Objects[1]);
	ASSERT(MeshHandle1);

	ASSERT(!pint.mUserData);
	pint.mUserData = ICE_NEW(MeshMeshHits)(MeshHandle0, MeshHandle1);

	return true;
}*/

static const char* gDesc_MeshMesh = "MESH-MESH MIDPHASE: test collision detection between arbitrary triangle meshes. Usually the domain of dedicated collision libraries like Opcode. In the PhysX family only PhysX5 supports it.";

class MeshMesh : public MidphaseMeshMeshTest, public ZB2CreationCallback
{
			ComboBoxPtr		mComboBox_Filename;
	public:
							MeshMesh()					{							}
	virtual					~MeshMesh()					{							}
	virtual	const char*		GetName()			const	{ return "MeshMesh";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_MeshMesh;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 300;
		WD.mHeight	= 150;
		WD.mLabel	= "Mesh-vs-mesh scenes";
		WD.mType	= WINDOW_DIALOG;
		IceWindow* UI = ICE_NEW(IceWindow)(WD);
		RegisterUIElement(UI);
		UI->SetVisible(true);

		Widgets& UIElems = GetUIElements();

//		const sdword EditBoxWidth = 60;
		const sdword LabelWidth = 30;
		const sdword OffsetX = LabelWidth + 10;
		const sdword LabelOffsetY = 2;
		const sdword YStep = 20;
		sdword y = 0;
		{
			y += YStep;
			helper.CreateLabel(UI, 4, y+LabelOffsetY, LabelWidth, 20, "File:", &UIElems);
			{
				IceComboBox* CB = CreateComboBox<IceComboBox>(UI, 0, 4+OffsetX, y, 170, 200, "Filename", null, null);
				CB->Add("Venus");
				CB->Add("Tessellated Venus");
				CB->Add("Cow & Knot");
				CB->Add("Bunnies");
				CB->Add("Tessellated Bunnies");
				CB->Add("Buddhas");
				CB->Add("Multi Buddhas");
				CB->Add("Mega Buddhas");
				CB->Select(0);
				mComboBox_Filename = CB;
			}
			RegisterUIElement(mComboBox_Filename);
			y += YStep;
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, 300-16);

		return null;
	}
	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)	override
	{
		const char* Filename = null;
		if(mComboBox_Filename)
		{
			const udword Index = mComboBox_Filename->GetSelectedIndex();
			if(Index==MESH_MESH_VENUS)
				Filename = "MeshMesh_Venus.zb2";
			else if(Index==MESH_MESH_TESSVENUS)
				Filename = "MeshMesh_TessVenus.zb2";
			else if(Index==MESH_MESH_COW_KNOT)
				Filename = "MeshMesh_CowKnot.zb2";
			else if(Index==MESH_MESH_BUNNY)
				Filename = "MeshMesh_Bunny.zb2";
				//Filename = "MeshMesh_Bunny2.zb2";
			else if(Index==MESH_MESH_TESSBUNNY)
				Filename = "MeshMesh_TessBunny.zb2";
			else if(Index==MESH_MESH_BUDDHAS)
				Filename = "MeshMesh_Buddhas.zb2";
			else if(Index==MESH_MESH_MULTI_BUDDHAS)
				Filename = "MeshMesh_MultiBuddhas.zb2";
			else if(Index==MESH_MESH_MEGA_BUDDHAS)
				Filename = "MeshMesh_MegaBuddhas.zb2";	
		}

		if(Filename)
		{
			Filename = FindPEELFile(Filename);
			if(Filename)
			{
				bool status = ImportZB2File(desc, Filename);
				(void)status;

				desc.mCamera[0] = PintCameraPose(Point(11.40f, 10.46f, 20.11f), Point(-0.34f, -0.25f, -0.91f));
				SetDefEnv(desc, false);
			}
		}
	}

	virtual bool	CommonSetup()	override
	{
		TestBase::CommonSetup();
		return true;
	}

	virtual	void	NotifyCreatedObjects(Pint& pint, const ZB2CreatedObjects& objects, ZCB2Factory*)
	{
		ASSERT(objects.mNbActors==2);
		if(objects.mNbActors)
		{
			const PintActorHandle MeshHandle0 = objects.mActors[0].mHandle;
			ASSERT(MeshHandle0);

			const PintActorHandle MeshHandle1 = objects.mActors[1].mHandle;
			ASSERT(MeshHandle1);

			ASSERT(!pint.mUserData);
			pint.mUserData = ICE_NEW(MeshMeshHits)(MeshHandle0, MeshHandle1);
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshMeshOverlaps)
			return false;

		//return MeshMeshSetup(pint, *this, caps);

		return CreateZB2Scene(pint, caps, this);
	}

	virtual udword	Update(Pint& pint, float dt)
	{
		if(!pint.mUserData)
			return 0;

		MeshMeshHits* MMH = reinterpret_cast<MeshMeshHits*>(pint.mUserData);
		const PintActorHandle MeshHandle0 = MMH->mMeshHandle0;
		ASSERT(MeshHandle0);

		const PintActorHandle MeshHandle1 = MMH->mMeshHandle1;
		ASSERT(MeshHandle1);

		return pint.FindTriangles_MeshMeshOverlap(pint.mSQHelper->GetThreadContext(), MeshHandle0, MeshHandle1, MMH->mResults);
	}

END_TEST(MeshMesh)

///////////////////////////////////////////////////////////////////////////////

