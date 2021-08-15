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
#include "Loader_Bin.h"
#include "MyConvex.h"
#include "TextureManager.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_EmptyScene = "Empty scene. Use this to measure the operating overhead of each engine.";

START_TEST(EmptyScene, CATEGORY_UNDEFINED, gDesc_EmptyScene)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(30.38f, 16.45f, 31.28f), Point(-0.58f, -0.48f, -0.65f));
		desc.mCamera[0] = PintCameraPose(Point(30.38f, 16000.45f, 31.28f), Point(-0.58f, -0.48f, -0.65f));
		//desc.mCamera[0] = PintCameraPose(Point(11.19f, 0.57f, 9.77f), Point(-0.38f, -0.81f, -0.45f));
		//desc.mCamera[0] = PintCameraPose(Point(-1524286.38f, -189248.67f, -1175734.13f), Point(0.79f, 0.16f, 0.59f));
		//desc.mCamera[0] = PintCameraPose(Point(-1524286.38f, 5000000.0f, -1175734.13f), Point(0.79f, 0.16f, 0.59f));
		desc.mCamera[0] = PintCameraPose(Point(30.38f, 16000.45f, 31.28f), Point(0.57f, -0.62f, 0.53f));
		desc.mCamera[0] = PintCameraPose(Point(-1608341.13f, -3888308.50f, 5651855.00f), Point(0.09f, 0.93f, -0.36f));

		desc.mCamera[0] = PintCameraPose(Point(2213434.00f, -1216752.50f, 3018044.00f), Point(0.66f, -0.32f, 0.69f));

		SetDefEnv(desc, false);
	}

	virtual	bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return true;
	}

END_TEST(EmptyScene)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DisplayConvexes = "Display standard PEEL convexes. This is just to test rendering options.";

START_TEST(DisplayConvexes, CATEGORY_UNDEFINED, gDesc_DisplayConvexes)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-4.65f, 15.97f, 17.71f), Point(0.62f, -0.42f, -0.66f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes)
			return false;

		for(udword i=0;i<14;i++)
		{
			MyConvex C;
			C.LoadFile(i);

			PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
			ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &ConvexCreate;
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Point(float(i)*3.0f, 4.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(DisplayConvexes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DisplayAllShapes = "Display standard shapes. This is just to test rendering options.";

START_TEST(DisplayAllShapes, CATEGORY_UNDEFINED, gDesc_DisplayAllShapes)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.62f, 5.24f, -5.53f), Point(0.60f, -0.58f, 0.54f));
		SetDefEnv(desc, true);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		const Point Scale(0.1f, 0.1f, 0.1f);
		LoadMeshesFromFile_(*this, "bunny.bin", &Scale);
//		mCreateDefaultEnvironment = false;
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportConvexes || !caps.mSupportMeshes)
			return false;

		const float IncX = 3.0f;
		float x = 2.5f;
		{
			const float Radius = 1.0f;
			PINT_SPHERE_CREATE Create(Radius);
			Create.mRenderer	= CreateSphereRenderer(Create.mRadius);
			CreateStaticObject(pint, &Create, Point(x, Radius, 0.0f), null, "Sphere");
			x += IncX;

			Create.mRenderer	= CreateSphereRenderer(Create.mRadius, true);
			CreateStaticObject(pint, &Create, Point(x, Radius, 0.0f), null, "GeoSphere");
			x += IncX;
		}

		{
			const float Size = 1.0f;
			PINT_BOX_CREATE Create(Point(Size, Size, Size));
			Create.mRenderer	= CreateBoxRenderer(Create.mExtents);
			CreateStaticObject(pint, &Create, Point(x, Size, 0.0f), null, "Box");
			x += IncX;
		}

		{
			const float Radius = 0.5f;
			const float HalfHeight = 0.5f;
			PINT_CAPSULE_CREATE Create(Radius, HalfHeight);
			Create.mRenderer	= CreateCapsuleRenderer(Radius, HalfHeight*2.0f);
			CreateStaticObject(pint, &Create, Point(x, Radius+HalfHeight, 0.0f), null, "Capsule");
			x += IncX;
		}

		{
			MyConvex C;
			C.LoadFile(2);

			PINT_CONVEX_CREATE Create(C.mNbVerts, C.mVerts);
			Create.mRenderer	= CreateConvexRenderer(Create.mNbVerts, Create.mVerts);

			CreateStaticObject(pint, &Create, Point(x, 1.0f, 0.0f), null, "Convex");
			x += IncX;
		}
		return CreateMeshesFromRegisteredSurfaces(pint, caps, null, null, "Mesh");
	}

END_TEST(DisplayAllShapes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_MeshInstancing = "Mesh instancing test.";

START_TEST(MeshInstancing, CATEGORY_UNDEFINED, gDesc_MeshInstancing)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.97f, 2.69f, -3.01f), Point(0.56f, -0.40f, 0.72f));
		SetDefEnv(desc, true);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		const Point Scale(0.1f, 0.1f, 0.1f);
		LoadMeshesFromFile_(*this, "bunny.bin", &Scale);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes)
			return false;

//		return CreateMeshesFromRegisteredSurfaces(pint, caps, *this, null, null, "Mesh");

		if(!GetNbRegisteredSurfaces())
			return false;
		const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);

		const IndexedSurface* IS = SD->mSurface;
		PintShapeRenderer* Renderer = SD->mRenderer;

		PINT_MESH_CREATE MeshDesc;
		MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
		MeshDesc.mRenderer	= Renderer;

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &MeshDesc;
		ObjectDesc.mMass		= 0.0f;

		{
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}

		{
			ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}

		{
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 3.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(MeshInstancing)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ColoredMeshInstancing = "Colored mesh instancing test.";

START_TEST(ColoredMeshInstancing, CATEGORY_UNDEFINED, gDesc_ColoredMeshInstancing)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.97f, 2.69f, -3.01f), Point(0.56f, -0.40f, 0.72f));
		SetDefEnv(desc, true);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		const Point Scale(0.1f, 0.1f, 0.1f);
		LoadMeshesFromFile_(*this, "bunny.bin", &Scale);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes)
			return false;

//		return CreateMeshesFromRegisteredSurfaces(pint, caps, *this, null, null, "Mesh");

		if(!GetNbRegisteredSurfaces())
			return false;
		const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);

		const IndexedSurface* IS = SD->mSurface;
		PintShapeRenderer* Renderer = SD->mRenderer;

		{
			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(1.0f, 0.0f, 0.0f));

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &MeshDesc;
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 1.0f, 0.0f));

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &MeshDesc;
			ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 0.0f, 1.0f));

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &MeshDesc;
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 3.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(ColoredMeshInstancing)

static const char* gDesc_ColoredMeshInstancing2 = "Colored mesh instancing test 2.";

START_TEST(ColoredMeshInstancing2, CATEGORY_UNDEFINED, gDesc_ColoredMeshInstancing2)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.97f, 2.69f, -3.01f), Point(0.56f, -0.40f, 0.72f));
		SetDefEnv(desc, true);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		const Point Scale(0.1f, 0.1f, 0.1f);
		LoadMeshesFromFile_(*this, "bunny.bin", &Scale);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes)
			return false;

//		return CreateMeshesFromRegisteredSurfaces(pint, caps, *this, null, null, "Mesh");

		if(!GetNbRegisteredSurfaces())
			return false;
		const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);

		const IndexedSurface* IS = SD->mSurface;
		PintShapeRenderer* Renderer = SD->mRenderer;

		PintMeshHandle MeshHandle;
		{
			PINT_MESH_DATA_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
			MeshHandle = pint.CreateMeshObject(MeshDesc);
		}

			PINT_MESH_CREATE2 MeshDesc(MeshHandle);
		{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(1.0f, 0.0f, 0.0f));

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &MeshDesc;
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 1.0f, 0.0f));

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &MeshDesc;
			ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 0.0f, 1.0f));

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes		= &MeshDesc;
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 3.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(ColoredMeshInstancing2)

///////////////////////////////////////////////////////////////////////////////

static bool MakeVoronoi(Picture& pic, udword nb_seeds, udword flags=PIXEL_FULL)
{
	if(!nb_seeds)
		return false;
	if(!flags)
		return true;

	const uword mWidth = pic.GetWidth();
	const uword mHeight = pic.GetHeight();
	RGBAPixel* mData = pic.GetPixels();

	sdword*		Seeds	= (sdword*)ICE_ALLOC_TMP(sizeof(sdword)*nb_seeds*2);	CHECKALLOC(Seeds);
	RGBAPixel*	Colors	= ICE_NEW_TMP(RGBAPixel[nb_seeds]);						CHECKALLOC(Colors);

	// Random seeds & colors
	const ubyte BaseColor = 128;
	for(udword i=0;i<nb_seeds;i++)
	{
		Seeds[i*2+0] = rand() % mWidth;
		Seeds[i*2+1] = rand() % mHeight;
		Colors[i].R = BaseColor + ubyte(rand() % (235 - BaseColor));
		Colors[i].G = BaseColor + ubyte(rand() % (235 - BaseColor));
		Colors[i].B = BaseColor + ubyte(rand() % (235 - BaseColor));
		Colors[i].A = BaseColor + ubyte(rand() % (235 - BaseColor));
	}

	udword* Pixels = (udword*)mData;
	udword p=0;
	for(sdword y=0;y<sdword(mHeight);y++)
	{
		for(sdword x=0;x<sdword(mWidth);x++)
		{
			float MinDist = MAX_FLOAT;
			udword j=0;
			for(udword i=0;i<nb_seeds;i++)
			{
				const float dx = float(x - Seeds[i*2+0]);
				const float dy = float(y - Seeds[i*2+1]);
				const float Dist = dx*dx + dy*dy;
				if(Dist<MinDist)
				{
					MinDist = Dist;
					j = i;
				}
			}
			if(flags&PIXEL_R)	mData[p].R = Colors[j].R;
			if(flags&PIXEL_G)	mData[p].G = Colors[j].G;
			if(flags&PIXEL_B)	mData[p].B = Colors[j].B;
			if(flags&PIXEL_A)	mData[p].A = Colors[j].A;
			p++;
		}
	}

	DELETEARRAY(Colors);
	ICE_FREE(Seeds);

	return true;
}

static const char* gDesc_TexturedMesh = "Textured mesh test.";

START_TEST(TexturedMesh, CATEGORY_UNDEFINED, gDesc_TexturedMesh)

	virtual	float	GetRenderData(Point& center)	const	{ return 20.0f;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.97f, 2.69f, -3.01f), Point(0.56f, -0.40f, 0.72f));
		SetDefEnv(desc, true);
	}

	virtual bool	CommonSetup()
	{
		TestBase::CommonSetup();
		const Point Scale(0.1f, 0.1f, 0.1f);
		LoadMeshesFromFile_(*this, "bunny.bin", &Scale);
		return true;
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportMeshes)
			return false;

//		return CreateMeshesFromRegisteredSurfaces(pint, caps, *this, null, null, "Mesh");

		if(!GetNbRegisteredSurfaces())
			return false;
		const SurfaceManager::SurfaceData* SD = GetSurfaceData(0);

		const IndexedSurface* IS = SD->mSurface;

		MultiSurface MS;
		MS.IndexedSurface::Init(IS->GetNbFaces(), IS->GetNbVerts(), IS->GetVerts(), IS->GetFaces());
		// Based on RMMesh::CreateSphereMapUVs
		{
			IndexedSurface* UVSurface = MS.AddExtraSurface(SURFACE_UVS);
			UVSurface->Init(IS->GetNbFaces(), IS->GetNbVerts());

			// Copy same topology
			IndexedTriangle* TFaces = (IndexedTriangle*)UVSurface->GetFaces();
			CopyMemory(TFaces, IS->GetFaces(), IS->GetNbFaces()*sizeof(IndexedTriangle));

			Point* uvs = (Point*)UVSurface->GetVerts();
			ComputeSphereMapModel(IS->GetNbVerts(), IS->GetVerts(), uvs);
		}

		PintShapeRenderer* Renderer = CreateMeshRenderer(MS);
//		PintShapeRenderer* Renderer = CreateMeshRenderer(IS->GetSurfaceInterface());

		const udword TexWidth = 512;
		const udword TexHeight = 512;
//		Picture P(TexWidth, TexHeight);
//		P.MakeVoronoi();
		Picture P(TexWidth, TexHeight);
		::MakeVoronoi(P, 64);
		const ManagedTexture* MT = CreateManagedTexture(TexWidth, TexHeight, P.GetPixels(), null);

		if(1)
		{
			PintMeshHandle MeshHandle;
			{
				PINT_MESH_DATA_CREATE MeshDesc;
				MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
				MeshHandle = pint.CreateMeshObject(MeshDesc);
			}

				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
			{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(1.0f, 0.0f, 0.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &MeshDesc;
				ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 1.0f, 0.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &MeshDesc;
				ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 0.0f, 1.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &MeshDesc;
				ObjectDesc.mPosition	= Point(0.0f, 0.0f, 3.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}
		}
		else
		{
			{
				PINT_MESH_CREATE MeshDesc;
				MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(1.0f, 0.0f, 0.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &MeshDesc;
				ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			{
				PINT_MESH_CREATE MeshDesc;
				MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 1.0f, 0.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &MeshDesc;
				ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			{
				PINT_MESH_CREATE MeshDesc;
				MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 0.0f, 1.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &MeshDesc;
				ObjectDesc.mPosition	= Point(0.0f, 0.0f, 3.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}
		}
		return true;
	}

END_TEST(TexturedMesh)

///////////////////////////////////////////////////////////////////////////////

#include "Voronoi2D.h"

static const char* gDesc_TestVoronoi2D = "Test voronoi 2D";

class Voronoi2D : public TestBase, public VoronoiCallback
{
	public:
							Voronoi2D()					{								}
	virtual					~Voronoi2D()				{								}
	virtual	const char*		GetName()			const	{ return "Voronoi2D";			}
	virtual	const char*		GetDescription()	const	{ return gDesc_TestVoronoi2D;	}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_UNDEFINED;	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.97f, 2.69f, -3.01f), Point(0.56f, -0.40f, 0.72f));
		SetDefEnv(desc, true);
	}

	Pint*	mPint;
	virtual	void	OnGeneratedCell(const Point2D& site, udword nb_verts, const Point2D* verts)
	{
		Point* Pts = ICE_NEW(Point)[nb_verts*2];
		for(udword i=0;i<nb_verts;i++)
		{
			Pts[i] = Point(verts[i].x, 0.0f, verts[i].y);
			Pts[i+nb_verts] = Point(verts[i].x, 1.0f, verts[i].y);
		}

		PINT_CONVEX_CREATE ConvexCreate;
		ConvexCreate.mNbVerts	= nb_verts*2;
		ConvexCreate.mVerts		= Pts;
		ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

		PINT_OBJECT_CREATE ObjectDesc;
		ObjectDesc.mShapes		= &ConvexCreate;
		ObjectDesc.mMass		= 0.0f;
		//ObjectDesc.mPosition	= pos;
		PintActorHandle Handle = CreatePintObject(*mPint, ObjectDesc);
		ASSERT(Handle);

		DELETEARRAY(Pts);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		const udword NbCells = 32;
		const float ScaleX = 10.0f;
		const float ScaleZ = 10.0f;

		mPint = &pint;
		GenerateVoronoiCells(NbCells, ScaleX, ScaleZ, *this);

		return true;
	}

	virtual	void	CommonDebugRender(PintRender& renderer)
	{
		if(0)
			TestVoronoi2D(renderer);
	}

END_TEST(Voronoi2D)

///////////////////////////////////////////////////////////////////////////////

#ifdef REMOVED
static const char* gDesc_DisplayLegoParts = "Display LEGO parts. This is just to test rendering options.";

START_TEST(DisplayLegoParts, CATEGORY_UNDEFINED, gDesc_DisplayLegoParts)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		InitLegoLib();
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-4.65f, 15.97f, 17.71f), Point(0.62f, -0.42f, -0.66f));

//		mCreateDefaultEnvironment = false;
		SetDefEnv(desc, true);
	}

	virtual	void	CommonRelease()
	{
		const udword NbActors = GetNbLegoActors();
		for(udword i=0;i<NbActors;i++)
		{
			LegoActor* Actor = const_cast<LegoActor*>(GetLegoActorByIndex(i));
			Actor->mUserData = null;
			LegoPartMesh* Mesh = Actor->mMesh;
			Mesh->mUserData = null;
		}
		TestBase::CommonRelease();
		CloseLegoLib();
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(0)
		{
			if(!caps.mSupportConvexes)
				return false;

			const udword NbParts = GetNbLegoParts();

			for(udword i=0;i<NbParts;i++)
			{
	//			const LegoPartMesh*	Part = GetLegoPartByID(60477);
				const LegoPartMesh*	Part = GetLegoPartByIndex(i);

				SurfaceInterface SI;
				SI.mNbVerts	= Part->GetNbVerts();
				SI.mVerts	= Part->GetVerts();
				SI.mNbFaces	= Part->GetNbTris();
				SI.mDFaces	= Part->GetIndices();

				PINT_CONVEX_CREATE ConvexCreate(Part->GetNbVerts(), Part->GetVerts());
				ConvexCreate.mRenderer	= CreateMeshRenderer(SI);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &ConvexCreate;
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mPosition	= Point(float(i)*3.0f, 4.0f, 0.0f);
				CreatePintObject(pint, ObjectDesc);
			}
		}
		else
		{
			if(!caps.mSupportMeshes)
				return false;

			const udword NbParts = GetNbLegoParts();
			for(udword i=0;i<NbParts;i++)
			{
	//			const LegoPartMesh*	Part = GetLegoPartByID(60477);
				const LegoPartMesh*	Part = GetLegoPartByIndex(i);

				PINT_MESH_CREATE MeshCreate;
				MeshCreate.mSurface.mNbVerts	= Part->GetNbVerts();
				MeshCreate.mSurface.mVerts		= Part->GetVerts();
				MeshCreate.mSurface.mNbFaces	= Part->GetNbTris();
				MeshCreate.mSurface.mDFaces		= Part->GetIndices();
				MeshCreate.mRenderer			= CreateMeshRenderer(MeshCreate.mSurface);

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &MeshCreate;
				ObjectDesc.mMass		= 0.0f;
//				ObjectDesc.mPosition	= Point(float(i)*3.0f, 0.0f, 0.0f);
				ObjectDesc.mPosition	= Part->mPos;
				CreatePintObject(pint, ObjectDesc);
			}

//#define USE_DYNAMIC_CONVEXES
			const udword NbActors = GetNbLegoActors();
			udword NbSharedMeshes = 0;
			for(udword i=0;i<NbActors;i++)
			{
				const LegoActor* Actor = GetLegoActorByIndex(i);

				LegoPartMesh* Mesh = Actor->mMesh;

#ifdef USE_DYNAMIC_CONVEXES
				SurfaceInterface SI;
				SI.mNbVerts	= Mesh->GetNbVerts();
				SI.mVerts	= Mesh->GetVerts();
				SI.mNbFaces	= Mesh->GetNbTris();
				SI.mDFaces	= Mesh->GetIndices();

				PINT_CONVEX_CREATE MeshCreate(Mesh->GetNbVerts(), Mesh->GetVerts());
#else
				PINT_MESH_CREATE MeshCreate;
				MeshCreate.mSurface.mNbVerts	= Mesh->GetNbVerts();
				MeshCreate.mSurface.mVerts		= Mesh->GetVerts();
				MeshCreate.mSurface.mNbFaces	= Mesh->GetNbTris();
				MeshCreate.mSurface.mDFaces		= Mesh->GetIndices();
#endif
				if(Mesh->mUserData && 1)
				{
					PintShapeRenderer* Renderer	= (PintShapeRenderer*)Mesh->mUserData;
					MeshCreate.mRenderer		= CreateColorShapeRenderer(Renderer, Actor->mMaterial->mEffect->mDiffuse);

					NbSharedMeshes++;
				}
				else
				{
#ifdef USE_DYNAMIC_CONVEXES
					PintShapeRenderer* Renderer	= CreateMeshRenderer(SI, true);
					MeshCreate.mRenderer		= CreateColorShapeRenderer(Renderer, Actor->mMaterial->mEffect->mDiffuse);
#else
					PintShapeRenderer* Renderer;
					GLuint texId = 0;
					if(Mesh->GetNbTVerts())
					{
						MultiSurface MS;
						MS.IndexedSurface::Init(Mesh->GetNbTris(), Mesh->GetNbVerts(), Mesh->GetVerts(), (const IndexedTriangle*)Mesh->GetIndices());
						{
							IndexedSurface* UVSurface = MS.AddExtraSurface(SURFACE_UVS);
							UVSurface->Init(Mesh->GetNbTTris(), Mesh->GetNbTVerts());

							IndexedTriangle* TFaces = (IndexedTriangle*)UVSurface->GetFaces();
							CopyMemory(TFaces, Mesh->GetTIndices(), Mesh->GetNbTTris()*sizeof(IndexedTriangle));

							Point* uvs = (Point*)UVSurface->GetVerts();
							CopyMemory(uvs, Mesh->GetTVerts(), Mesh->GetNbTVerts()*sizeof(Point));
						}

						if(Actor->mMaterial && Actor->mMaterial->mEffect && Actor->mMaterial->mEffect->mImage)
							texId = Actor->mMaterial->mEffect->mImage->mTexId;
						Renderer = CreateMeshRenderer(MS, true);
					}
					else
					{
						Renderer = CreateMeshRenderer(MeshCreate.mSurface, true);
					}
//					PintShapeRenderer* Renderer	= CreateMeshRenderer(MeshCreate.mSurface, true);
					MeshCreate.mRenderer		= CreateColorShapeRenderer(Renderer, Actor->mMaterial->mEffect->mDiffuse, texId);
#endif
					Mesh->mUserData				= Renderer;
				}

				PINT_OBJECT_CREATE ObjectDesc;
				ObjectDesc.mShapes		= &MeshCreate;
#ifdef USE_DYNAMIC_CONVEXES
				ObjectDesc.mMass		= 1.0f;
#else
				ObjectDesc.mMass		= 0.0f;
#endif
				ObjectDesc.mPosition	= Actor->mPose.GetTrans();
				ObjectDesc.mRotation	= Actor->mPose;
				CreatePintObject(pint, ObjectDesc);
			}
			printf("NbSharedMeshes: %d\n", NbSharedMeshes);
		}
		return true;
	}

END_TEST(DisplayLegoParts)
#endif

///////////////////////////////////////////////////////////////////////////////

//PHYSICS_TEST_DYNAMIC(DynamicCompound, CATEGORY_UNDEFINED, null)
#ifdef REMOVED
	class DynamicCompound : public TestBase
	{
		public:
									DynamicCompound()		{								}
		virtual						~DynamicCompound()		{								}
		virtual	const char*			GetName()				{ return "DynamicCompound";		}
		virtual	TestCategory		GetCategory()	const	{ return CATEGORY_UNDEFINED;	}
		virtual	bool				Setup(Pint& pint, const PintCaps& caps);
		virtual	void				CommonUpdate(float dt);
		virtual	udword				Update(Pint& pint, float dt);

				PintActorHandle		mShapes[3];
	}DynamicCompound;

bool DynamicCompound::Setup(Pint& pint, const PintCaps& caps)
{
	if(!caps.mSupportCompounds)
		return false;

//	const Point MainBodyExtents(3.0f, 1.0f, 3.0f);
	const Point MainBodyExtents(3.0f, 1.0f, 1.0f);
	const Point WingExtents(1.0f, 0.1f, 4.0f);

	PINT_SPHERE_CREATE SphereDesc;
	SphereDesc.mRadius		= 1.0f;
	SphereDesc.mRenderer	= CreateSphereRenderer(1.0f);

	PINT_BOX_CREATE BoxDesc[3];
	BoxDesc[0].mExtents		= MainBodyExtents;
	BoxDesc[1].mExtents		= WingExtents;
	BoxDesc[1].mLocalPos	= Point(MainBodyExtents.x + WingExtents.x, 0.0f, 0.0f);
	BoxDesc[2].mExtents		= WingExtents;
	BoxDesc[2].mLocalPos	= Point(-MainBodyExtents.x - WingExtents.x, 0.0f, 0.0f);
	BoxDesc[0].mRenderer	= CreateBoxRenderer(BoxDesc[0].mExtents);
	BoxDesc[1].mRenderer	= CreateBoxRenderer(BoxDesc[1].mExtents);
	BoxDesc[2].mRenderer	= CreateBoxRenderer(BoxDesc[2].mExtents);
	BoxDesc[0].mNext = &BoxDesc[1];
	BoxDesc[1].mNext = &BoxDesc[2];

	SphereDesc.mNext = &BoxDesc[1];

	PINT_OBJECT_CREATE ObjectDesc;
//	ObjectDesc.mShapes		= BoxDesc;
	ObjectDesc.mShapes		= &SphereDesc;
	ObjectDesc.mMass		= 1.0f;
	ObjectDesc.mPosition	= Point(0.0f, MainBodyExtents.y, 0.0f);
	PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
	ASSERT(Handle);

	udword NbShapes = pint.GetShapes(mShapes, Handle);
	ASSERT(NbShapes==3);
	return true;
}

void DynamicCompound::CommonUpdate(float dt)
{
	mCurrentTime += dt;
}

udword DynamicCompound::Update(Pint& pint, float dt)
{
	Matrix3x3 Rot;
	Rot.RotX(mCurrentTime*1.0f);
	const Quat Q = Rot;

/*	for(udword i=1;i<3;i++)
	{
		pint.SetLocalRot(mShapes[i], Q);
	}*/
	pint.SetLocalRot(mShapes[0], Q);
	return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////

/*
START_TEST(ProxyShapes, CATEGORY_UNDEFINED, null)

bool ProxyShapes::Setup(Pint& pint, const PintCaps& caps)
{
	const Point Extents(10.0f, 10.0f, 10.0f);
	const udword NbX = 32;
	const udword NbY = 32;
	const float Altitude = 10.0f;
	const float Scale = 20.0f;

	PINT_BOX_CREATE BoxDesc(Extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(Extents);

	for(udword y=0;y<NbY;y++)
	{
		const float CoeffY = float(y) - float(NbY-1)*0.5f;
		for(udword x=0;x<NbX;x++)
		{
			const float CoeffX = float(x) - float(NbX-1)*0.5f;

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mShapes			= &BoxDesc;
			ObjectDesc.mMass			= 0.0f;
			ObjectDesc.mPosition.x		= CoeffX * Scale;
			ObjectDesc.mPosition.y		= Altitude;
			ObjectDesc.mPosition.z		= CoeffY * Scale;
			CreatePintObject(pint, ObjectDesc);
		}
	}

	mCreateDefaultEnvironment = false;

	return true;
}
*/

///////////////////////////////////////////////////////////////////////////////
