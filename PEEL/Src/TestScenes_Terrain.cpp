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
#include "PintGLMeshShapeRenderer.h"
#include "PintSQ.h"
#include "Terrain.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_GridMesh = "Grid mesh.";

class TerrainTest : public TestBase
{
			TerrainData		mData;
	public:
							TerrainTest()				{							}
	virtual					~TerrainTest()				{							}
	virtual	const char*		GetName()			const	{ return "Grid mesh";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_GridMesh;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_DISABLED;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-6.42f, 7.49f, 4.86f), Point(0.63f, -0.63f, -0.46f));
		SetDefEnv(desc, false);
	}

	virtual bool	CommonSetup()
	{
		mData.Init(128, 128, 0.01f, 0.01f);

		return TestBase::CommonSetup();
	}

	virtual void	CommonRelease()
	{
		mData.Release();
		TestBase::CommonRelease();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes || !caps.mSupportDeformableMeshes)
			return false;

		if(0)
		{
			PINT_MESH_CREATE MeshCreate;
			MeshCreate.SetSurfaceData(mData.mSurface.GetSurfaceInterface());
			MeshCreate.mRenderer = CreateMeshRenderer(MeshCreate.GetSurface());

			PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
			ObjectDesc.mMass	= 0.0f;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);
			(void)h;
		}
		else
		{
			PINT_MESH_DATA_CREATE MeshDataCreate;
			MeshDataCreate.SetSurfaceData(mData.mSurface.GetSurfaceInterface());
			MeshDataCreate.mDeformable	= true;
			PintMeshHandle mh = pint.CreateMeshObject(MeshDataCreate);

			PINT_MESH_CREATE2 MeshCreate;
			MeshCreate.mTriangleMesh	= mh;
			MeshCreate.mRenderer		= mData.mRenderer;

			PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
			ObjectDesc.mMass		= 0.0f;
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			RegisterTerrainData(h, &mData);
		}
		return true;
	}

END_TEST(TerrainTest)

///////////////////////////////////////////////////////////////////////////////

// Multiple options here:
// - overlap tests
// - contact notifications
// - contact modifications
//
// The contact notif/modif version is less accurate because not all touched triangles will be
// reported (there can be contact reduction going on, etc, so e.g. a large cube on a highly
// tessellated mesh may only report 4 contact triangles at the corners of the cube's base).
// However this works automatically for all objects, contrary to the overlap version that
// requires extra overlap tests for each object that wants to deform the terrain.

static const char* gDesc_DeformableTerrain = "Test deformable terrain";

namespace
{
	class ContactNotifyTest : public TestBase
	{
		protected:
		Vertices	mContacts;
		Vertices	mTriangles;

		virtual	void	CommonRelease()
		{
			mContacts.Empty();
			mTriangles.Empty();
			TestBase::CommonRelease();
		}

		virtual	void	CommonDebugRender(PintRender& render)
		{
			if(0)
			{
				const udword NbPts = mContacts.GetNbVertices()/2;
				//printf("NbPts: %d\n", NbPts);
				const Point* V = mContacts.GetVertices();
				const Point Color(1.0f, 0.0f, 0.0f);
				for(udword i=0;i<NbPts;i++)
				{
					render.DrawLine(V[0], V[1], Color);
					V += 2;
				}
			}
			if(0)
			{
				const udword NbTris = mTriangles.GetNbVertices()/3;
				//printf("NbTris: %d\n", NbTris);
				const Point* V = mTriangles.GetVertices();
				const Point Color(1.0f, 1.0f, 0.0f);
				for(udword i=0;i<NbTris;i++)
				{
					render.DrawTriangle(V[0], V[1], V[2], Color);
					V += 3;
				}
			}

			//if(!IsPaused())
			{
				mContacts.Reset();
				mTriangles.Reset();
			}
		}
	};
}

static const bool gDebugPX2197 = false;
static const bool gUseContactModif = false;
static const float gSphereRadius = 1.0f;

class DeformableTerrain : public ContactNotifyTest, public PintContactModifyCallback
{
	struct PintData	: public Allocateable
	{
		PintData() : mTerrainActor(null), mTerrainShape(null), mSphereActor(null)	{}
		PintActorHandle	mTerrainActor;
		PintShapeHandle	mTerrainShape;
		PintActorHandle	mSphereActor;
	};

			TerrainData		mData;
	public:
							DeformableTerrain()			{									}
	virtual					~DeformableTerrain()		{									}
	virtual	const char*		GetName()			const	{ return "Deformable terrain";		}
	virtual	const char*		GetDescription()	const	{ return gDesc_DeformableTerrain;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_DISABLED;			}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-6.42f, 7.49f, 4.86f), Point(0.63f, -0.63f, -0.46f));
		SetDefEnv(desc, true);

		if(gUseContactModif)
		{
			if(!gDebugPX2197)	//MEGABUG
			desc.mContactModifyCallback = this;
		}
	}

	virtual bool	CommonSetup()
	{
		mData.Init(64, 64, 0.01f, 0.01f);
/*
		//mSurface.MakePlane(256, 256);
		//mSurface.MakePlane(128, 128);
		mSurface.MakePlane(64, 64);
		mSurface.Flip();
		//mSurface.Scale(Point(0.02f, 1.0f, 0.02f));
		mSurface.Scale(Point(0.01f, 1.0f, 0.01f));
		//mSurface.Scale(Point(0.002f, 1.0f, 0.002f));*/

		return TestBase::CommonSetup();
	}

	virtual void	CommonRelease()
	{
		mData.Release();
		ContactNotifyTest::CommonRelease();
		mVertexIndices.Empty();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation || !caps.mSupportMeshes || !caps.mSupportDeformableMeshes)
			return false;
		if(gUseContactModif && !caps.mSupportContactModifications)
			return false;

		PintData* PD = ICE_NEW(PintData);
		pint.mUserData = PD;

		{
			PINT_SPHERE_CREATE ShapeDesc(gSphereRadius);
			ShapeDesc.mRenderer = CreateSphereRenderer(gSphereRadius, true);

			//PINT_BOX_CREATE ShapeDesc(Point(gSphereRadius, gSphereRadius, gSphereRadius));
			//ShapeDesc.mRenderer = CreateBoxRenderer(Point(gSphereRadius, gSphereRadius, gSphereRadius));

			PINT_OBJECT_CREATE ObjectDesc(&ShapeDesc);
			ObjectDesc.mMass		= 1.0f;
			ObjectDesc.mPosition	= Point(0.0f, 4.0f, 0.0f);
			PD->mSphereActor = CreatePintObject(pint, ObjectDesc);
		}

		if(0)
		{
			PINT_MESH_CREATE MeshCreate;
			MeshCreate.SetSurfaceData(mData.mSurface.GetSurfaceInterface());
			MeshCreate.mRenderer = CreateMeshRenderer(MeshCreate.GetSurface());

			PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
			ObjectDesc.mMass	= 0.0f;
			PD->mTerrainActor = CreatePintObject(pint, ObjectDesc);
		}
		else
		{
			PINT_MESH_DATA_CREATE MeshDataCreate;
			MeshDataCreate.SetSurfaceData(mData.mSurface.GetSurfaceInterface());
			MeshDataCreate.mDeformable	= true;
			PintMeshHandle mh = pint.CreateMeshObject(MeshDataCreate);

			PINT_MESH_CREATE2 MeshCreate;
			MeshCreate.mTriangleMesh	= mh;
			MeshCreate.mRenderer		= mData.mRenderer;

			PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition.y	= 1.0f;
			PD->mTerrainActor = CreatePintObject(pint, ObjectDesc);
		}

		{
			Pint_Actor* ActorAPI = pint.GetActorAPI();
			if(ActorAPI && ActorAPI->GetNbShapes(PD->mTerrainActor)==1)
				PD->mTerrainShape = ActorAPI->GetShape(PD->mTerrainActor, 0);
		}

		return true;
	}

	virtual	void	Close(Pint& pint)
	{
		PintData* PD = reinterpret_cast<PintData*>(pint.mUserData);
		DELETESINGLE(PD);
		ContactNotifyTest::Close(pint);
	}

	///////////////////////////////////////////////////////////////////////////

	bool		mModifyContacts;
	Container	mVertexIndices;

	virtual	bool	PrepContactModify(Pint& pint, udword nb_contacts, PintActorHandle h0, PintActorHandle h1, PintShapeHandle s0, PintShapeHandle s1)
	{
		ASSERT(gUseContactModif);
		const PintData* PD = reinterpret_cast<const PintData*>(pint.mUserData);
		mModifyContacts = (h0==PD->mTerrainActor || h1==PD->mTerrainActor);
		return true;
	}

	virtual	ContactModif	ModifyDelayedContact(Pint& pint, const PR& pose0, const PR& pose1, Point& p, Point& n, float& s, udword feature0, udword feature1, udword index)
	{
		ASSERT(gUseContactModif);
		return CONTACT_IGNORE;
	}

	virtual	ContactModif	ModifyContact(Pint& pint, const PR& pose0, const PR& pose1, Point& p, Point& n, float& s, udword feature0, udword feature1)
	{
		ASSERT(gUseContactModif);
		const PintData* PD = reinterpret_cast<const PintData*>(pint.mUserData);
		PintActorHandle TerrainActor = PD->mTerrainActor;

		if(!mModifyContacts || !TerrainActor)
			return CONTACT_AS_IS;

		mContacts.AddVertex(p);
		mContacts.AddVertex(p+n);

		//printf("feature1: %d\n", feature1);

		if(feature1!=INVALID_ID)
		{
			const PintShapeHandle TerrainShape = PD->mTerrainShape;

			Pint_Shape* ShapeAPI = pint.GetShapeAPI();
			if(ShapeAPI && TerrainShape)
			{
				const Matrix4x4 M = pose1;

				Triangle tri;
				bool status = ShapeAPI->GetTriangle(tri, TerrainShape, feature1);
				ASSERT(status);
				mTriangles.AddVertex(tri.mVerts[0]*M);
				mTriangles.AddVertex(tri.mVerts[1]*M);
				mTriangles.AddVertex(tri.mVerts[2]*M);

				IndexedTriangle itri;
				status = ShapeAPI->GetIndexedTriangle(itri, TerrainShape, feature1);
				ASSERT(status);
				mVertexIndices.AddUnique(itri.mRef[0]);
				mVertexIndices.AddUnique(itri.mRef[1]);
				mVertexIndices.AddUnique(itri.mRef[2]);
			}
		}

		return CONTACT_AS_IS;
		//return mIgnore ? CONTACT_IGNORE : CONTACT_AS_IS;
	}

	///////////////////////////////////////////////////////////////////////////

	void	DeformTerrainWithContactModifs(Pint& pint, float dt)
	{
		const PintData* PD = reinterpret_cast<const PintData*>(pint.mUserData);
		PintActorHandle TerrainActor = PD->mTerrainActor;
		PintShapeHandle TerrainShape = PD->mTerrainShape;
		if(!TerrainActor || !TerrainShape)
			return;

		Pint_Shape* API = pint.GetShapeAPI();
		if(!API)
			return;


		if(gDebugPX2197)	//MEGABUG
		{
			const PintShapeHandle TerrainShape = PD->mTerrainShape;

			SurfaceInterface SI;
			if(API->GetTriangleMeshData(SI, TerrainShape, true))	//###
			{
				Point* Pts = const_cast<Point*>(SI.mVerts);
				API->Refit(TerrainShape, TerrainActor);
				(PintGLMeshShapeRenderer*)(mData.mRenderer)->UpdateVerts(Pts, null);
			}

			mVertexIndices.Reset();
		}


		udword NbToGo = mVertexIndices.GetNbEntries();
		//printf("NbToGo: %d\n", NbToGo);
		if(NbToGo)
		{
			SurfaceInterface SI;
			if(API->GetTriangleMeshData(SI, TerrainShape, true))	//###
				UpdateTerrainVertices(API, const_cast<Point*>(SI.mVerts), TerrainActor, TerrainShape);

			mVertexIndices.Reset();
		}
	}

	void	DeformTerrainWithOverlapCalls(Pint& pint, float dt)
	{
		const PintData* PD = reinterpret_cast<const PintData*>(pint.mUserData);
		PintActorHandle TerrainActor = PD->mTerrainActor;
		PintShapeHandle TerrainShape = PD->mTerrainShape;
		if(!TerrainActor || !TerrainShape)
			return;

//		Pint_Actor* ActorAPI = pint.GetActorAPI();
//		if(!ActorAPI)
//			return;

		const PR SpherePose = pint.GetWorldTransform(PD->mSphereActor);

		PintSphereOverlapData Overlap;
		Overlap.mSphere.mCenter = SpherePose.mPos;
		Overlap.mSphere.mRadius = gSphereRadius + 0.2f;	// Enlarge sphere a bit
		Overlap.mSphere.mRadius = gSphereRadius + 0.02f;	// Enlarge sphere a bit

		Pint_Shape* ShapeAPI = pint.GetShapeAPI();
		if(!ShapeAPI)
			return;

		const PR TerrainPose = ShapeAPI->GetWorldTransform(TerrainActor, TerrainShape);

		Container TouchedTriangleIndices;
		if(ShapeAPI->FindTouchedTriangles(TouchedTriangleIndices, pint.mSQHelper->GetThreadContext(), TerrainShape, TerrainPose, Overlap))
		{
			SurfaceInterface SI;
			if(ShapeAPI->GetTriangleMeshData(SI, TerrainShape, true))
			{
				const Matrix4x4 M = TerrainPose;

				const udword NbToucheddTris = TouchedTriangleIndices.GetNbEntries();
				for(udword i=0;i<NbToucheddTris;i++)
				{
					Triangle tri;
					bool status = ShapeAPI->GetTriangle(tri, TerrainShape, TouchedTriangleIndices[i]);
					ASSERT(status);
					mTriangles.AddVertex(tri.mVerts[0]*M);
					mTriangles.AddVertex(tri.mVerts[1]*M);
					mTriangles.AddVertex(tri.mVerts[2]*M);

					IndexedTriangle itri;
					status = ShapeAPI->GetIndexedTriangle(itri, TerrainShape, TouchedTriangleIndices[i]);
					ASSERT(status);
					mVertexIndices.AddUnique(itri.mRef[0]);
					mVertexIndices.AddUnique(itri.mRef[1]);
					mVertexIndices.AddUnique(itri.mRef[2]);
				}

				UpdateTerrainVertices(ShapeAPI, const_cast<Point*>(SI.mVerts), TerrainActor, TerrainShape);
				mVertexIndices.Reset();
			}
		}
	}

	void	UpdateTerrainVertices(Pint_Shape* ShapeAPI, Point* verts, PintActorHandle TerrainActor, PintShapeHandle TerrainShape)
	{
		if(verts)
		{
			const udword NbToGo = mVertexIndices.GetNbEntries();
			for(udword i=0;i<NbToGo;i++)
			{
				const float CurrentY = verts[mVertexIndices[i]].y;
				if(CurrentY>-0.4f)
				{
					//verts[mVertexIndices[i]].y -= 0.01f;
					verts[mVertexIndices[i]].y -= 0.1f;
					//verts[mVertexIndices[i]].y -= 0.4f;
				}
			}
			//mVertexIndices.Reset();

			ShapeAPI->Refit(TerrainShape, TerrainActor);
			(PintGLMeshShapeRenderer*)(mData.mRenderer)->UpdateVerts(verts, null);
		}
	}

	virtual	udword		Update(Pint& pint, float dt)
	{
		if(gUseContactModif)
			DeformTerrainWithContactModifs(pint, dt);
		else
			DeformTerrainWithOverlapCalls(pint, dt);

		return ContactNotifyTest::Update(pint, dt);
	}

END_TEST(DeformableTerrain)

///////////////////////////////////////////////////////////////////////////////

