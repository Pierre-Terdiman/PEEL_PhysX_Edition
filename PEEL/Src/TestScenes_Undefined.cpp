///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "Loader_Bin.h"
#include "MyConvex.h"
#include "TextureManager.h"
#include "GUI_Helpers.h"
#include "GUI_PosEdit.h"
#include "ZB2Import.h"

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_EmptyScene = "Empty scene. Use this to measure the operating overhead of each engine.";

START_TEST(EmptyScene, CATEGORY_UNDEFINED, gDesc_EmptyScene)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);
	}

	virtual	bool	Setup(Pint& pint, const PintCaps& caps)
	{
		return true;
	}

END_TEST(EmptyScene)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_EmptySceneAfterLargeRelease = "Empty scene after releasing a large amount of objects. Exposes issues in engines still parsing empty structures that should have been resized. WARNING: creates 4 million objects, takes a while.";

START_TEST(EmptySceneAfterLargeRelease, CATEGORY_UNDEFINED, gDesc_EmptySceneAfterLargeRelease)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		SetDefEnv(desc, false);
	}

	virtual	bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(!caps.mSupportRigidBodySimulation)
			return false;

		const float Radius = 0.1f;
		PINT_SPHERE_CREATE Create(Radius);
		Create.mRenderer	= CreateSphereRenderer(Create.mRadius);

		const float IncX = Radius * 3.0f;
		float x = 0.0f;

		const udword Nb = 4*1024*1024;

		PintActorHandle* Objects = ICE_ALLOCATE(PintActorHandle, Nb);

		printf("Creating %d objects\n", Nb);
		for(udword i=0;i<Nb;i++)
		{
			//Objects[i] = CreateStaticObject(pint, &Create, Point(x, Radius, 0.0f));
			Objects[i] = CreateDynamicObject(pint, &Create, Point(x, Radius, 0.0f));
			x += IncX;
		}

		printf("Simulating\n");
		pint.Update(1.0f/60.0f);

		printf("Releasing %d objects\n", Nb);
		for(udword i=0;i<Nb;i++)
		{
			pint._ReleaseObject(Objects[Nb-1-i]);
		}

		ICE_FREE(Objects);

		printf("Simulating\n");
		pint.Update(1.0f/60.0f);

		return true;
	}

END_TEST(EmptySceneAfterLargeRelease)

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
			ConvexCreate.mRenderer	= CreateRenderer(ConvexCreate);

			PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Point(float(i)*3.0f, 4.0f, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(DisplayConvexes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_DisplayAllShapes = "Display standard shapes. This is just to test rendering options. Note that it will only create cylinders, convex meshes and triangle meshes for plugins that do support them.";

START_TEST(DisplayAllShapes, CATEGORY_UNDEFINED, gDesc_DisplayAllShapes)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.25f, 6.64f, -4.21f), Point(0.63f, -0.60f, 0.49f));
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
			Create.mRenderer	= CreateRenderer(Create);
			CreateStaticObject(pint, &Create, Point(x, Size, 0.0f), null, "Box");
			x += IncX;
		}

		{
			const float Radius = 0.5f;
			const float HalfHeight = 0.5f;
			PINT_CAPSULE_CREATE Create(Radius, HalfHeight);
			Create.mRenderer	= CreateRenderer(Create);
			CreateStaticObject(pint, &Create, Point(x, Radius+HalfHeight, 0.0f), null, "Capsule");
			x += IncX;
		}

		if(caps.mSupportCylinders)
		{
			const float Radius = 0.5f;
			const float HalfHeight = 0.5f;
			PINT_CYLINDER_CREATE Create(Radius, HalfHeight);
			Create.mRenderer	= CreateRenderer(Create);
			CreateStaticObject(pint, &Create, Point(x, HalfHeight, 0.0f), null, "Cylinder");
		}
		x += IncX;

		if(caps.mSupportConvexes)
		{
			MyConvex C;
			C.LoadFile(2);

			PINT_CONVEX_CREATE Create(C.mNbVerts, C.mVerts);
			Create.mRenderer	= CreateRenderer(Create);

			CreateStaticObject(pint, &Create, Point(x, 1.0f, 0.0f), null, "Convex");
		}
		x += IncX;

		if(caps.mSupportMeshes)
			CreateMeshesFromRegisteredSurfaces(pint, caps, null, null, "Mesh");
		return true;
	}

END_TEST(DisplayAllShapes)

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_ShapeSharingTest = "Shape sharing test.";

START_TEST(ShapeSharingTest, CATEGORY_UNDEFINED, gDesc_ShapeSharingTest)

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);
		desc.mCamera[0] = PintCameraPose(Point(-1.62f, 5.24f, -5.53f), Point(0.60f, -0.58f, 0.54f));
		SetDefEnv(desc, true);
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(1)
		{
			const float Radius = 1.0f;
			PINT_SPHERE_CREATE SphereCreate(Radius);
			SphereCreate.mRenderer	= CreateSphereRenderer(Radius);

			const float Size = 0.75f;
			PINT_BOX_CREATE BoxCreate(Point(Size, Size, Size));
			BoxCreate.mRenderer	= CreateBoxRenderer(BoxCreate.mExtents);

			PINT_OBJECT_CREATE ObjectDesc(&SphereCreate);
			ObjectDesc.mPosition	= Point(0.0f, Radius, 0.0f);
			CreatePintObject(pint, ObjectDesc);

			//BoxCreate.mNext = &SphereCreate;
			//SphereCreate.mNext = null;
			BoxCreate.SetNext(null);
			SphereCreate.SetNext(&BoxCreate);

			ObjectDesc.SetShape(&SphereCreate);
			ObjectDesc.mPosition	= Point(2.0f, Radius, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}

		if(0)
		{
			const float Radius = 1.0f;
			PINT_SPHERE_CREATE SphereCreate(Radius);
			SphereCreate.mRenderer	= CreateSphereRenderer(Radius);

			const float Size = 0.75f;
			PINT_BOX_CREATE BoxCreate(Point(Size, Size, Size));
			BoxCreate.mRenderer	= CreateBoxRenderer(BoxCreate.mExtents);

			BoxCreate.SetNext(null);
			SphereCreate.SetNext(&BoxCreate);

			PINT_OBJECT_CREATE ObjectDesc(&SphereCreate);
			ObjectDesc.mPosition	= Point(0.0f, Radius, 0.0f);
			CreatePintObject(pint, ObjectDesc);

			BoxCreate.SetNext(&SphereCreate);
			SphereCreate.SetNext(null);

			ObjectDesc.SetShape(&BoxCreate);
			ObjectDesc.mPosition	= Point(2.0f, Radius, 0.0f);
			CreatePintObject(pint, ObjectDesc);
		}
		return true;
	}

END_TEST(ShapeSharingTest)

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

		PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
		ObjectDesc.mMass	= 0.0f;

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

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 1.0f, 0.0f));

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
			PINT_MESH_CREATE MeshDesc;
			MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 0.0f, 1.0f));

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
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

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 1.0f, 0.0f));

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
			ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
			ObjectDesc.mMass		= 0.0f;
			CreatePintObject(pint, ObjectDesc);
		}

		{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
			MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 0.0f, 1.0f));

			PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
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

				PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
				ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 1.0f, 0.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
				ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			{
//				PINT_MESH_CREATE2 MeshDesc(MeshHandle);
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 0.0f, 1.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
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

				PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
				ObjectDesc.mPosition	= Point(0.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			{
				PINT_MESH_CREATE MeshDesc;
				MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 1.0f, 0.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
				ObjectDesc.mPosition	= Point(2.0f, 0.0f, 0.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}

			{
				PINT_MESH_CREATE MeshDesc;
				MeshDesc.SetSurfaceData(IS->GetSurfaceInterface());
				MeshDesc.mRenderer	= CreateColorShapeRenderer(Renderer, RGBAColor(0.0f, 0.0f, 1.0f), MT);

				PINT_OBJECT_CREATE ObjectDesc(&MeshDesc);
				ObjectDesc.mPosition	= Point(0.0f, 0.0f, 3.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}
		}
		return true;
	}

END_TEST(TexturedMesh)

///////////////////////////////////////////////////////////////////////////////
