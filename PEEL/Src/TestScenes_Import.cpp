///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEEL.h"
#include "PEEL_MenuBar.h"
#include "PintShapeRenderer.h"
#include "TestScenes.h"
#include "TestScenesHelpers.h"
#include "PintObjectsManager.h"
#include "MyConvex.h"
#include "GUI_Helpers.h"
#include "LegoLib\LegoLib.h"
#include "Loader_Obj.h"
#include "ZCB\PINT_ZCB2.h"

//	#include "GUI_ActorEdit.h"

///////////////////////////////////////////////////////////////////////////////

//#define USE_DYNAMIC_CONVEXES
//#define USE_STATIC_MESHES	// Default
//#define USE_STATIC_BOXES	// Doesn't quite work

	enum LegoSceneImportMode
	{
		IMPORT_MODE_DYNAMIC_CONVEXES,
		IMPORT_MODE_STATIC_MESHES,
		IMPORT_MODE_STATIC_BOXES,	// Doesn't quite work
	};

static const char* gDesc_ImportScene = "Import scene from file.";

PhysicsTest* gDragAndDropTest = null;
const char* gDragAndDropName = null;

class ImportSceneFromFile : public TestBase
{
	protected:
			WavefrontDatabase*	mOBJ;
			LegoSceneImportMode mImportMode;
	public:
							ImportSceneFromFile() : mOBJ(null)
							{
								mImportMode = IMPORT_MODE_STATIC_MESHES;

								if(!gDragAndDropTest)
									gDragAndDropTest = this;
							}
	virtual					~ImportSceneFromFile()
							{
							}
	virtual	const char*		GetName()					const	{ return "ImportSceneFromFile";	}
	virtual	const char*		GetDescription()			const
							{
								if(mZB2Factory && mZB2Factory->mScene.mDesc.IsValid())
									return mZB2Factory->mScene.mDesc;

								return gDesc_ImportScene;
							}
	virtual	TestCategory	GetCategory()				const	{ return CATEGORY_INTERNAL;		}

	bool	Import(const char* filename, PINT_WORLD_CREATE& desc)
	{
		bool Success = false;

		const char* Ext = ::GetExtension(filename, null);
		if(!Ext)
		{
			IceCore::MessageBox(0, "Unknown file format. Cannot import.", "Error", MB_OK);
			return false;
		}

		if(strcmp(Ext, "dae")==0)
		{
			Success = true;
			if(0)
			{
				extern IceWindow*	gMainWindow;
				gMainWindow->SetEnabled(false);
				//IceCore::MessageBoxA(null, "test", "test", MB_OK);

				ICEGUIAPPCREATE Create;
				IceGUIApp MyApp;
				bool Status = MyApp.Init(Create);
				(void)Status;

/*				EditActorWindow* Main = CreateActorEditGUI(null, 100, 100);
				Main->SetVisible(true);
				int Result = MyApp.Run();
				DELETESINGLE(Main);*/


		class SelectObjects : public SelectionDialog
		{
			public:
								SelectObjects(const SelectionDialogDesc& desc) : SelectionDialog(desc)	{}
								~SelectObjects()
								{
								}

			virtual		void	OnSelected(void* user_data)
								{
								}
		};

					const udword DialogWidth	= 400;
					const udword DialogHeight	= 300;

					SelectionDialogDesc Desc;
					Desc.mMultiSelection		= true;
			//		Desc.mFilters				= "Plugins";
					Desc.mParent				= null;
					Desc.mX						= 100;
					Desc.mY						= 100;
					Desc.mWidth					= DialogWidth;
					Desc.mHeight				= DialogHeight;
					Desc.mLabel					= "PEEL - Select plugins";
					Desc.mType					= WINDOW_DIALOG;
			//		Desc.mType					= WINDOW_POPUP;
				//	Desc.mStyle					= WSTYLE_DLGFRAME;
					Desc.mStyle					= WSTYLE_HIDDEN;
					Desc.mCloseAppWithWindow	= true;
					Desc.mUserCheckboxes[0]		= "Enable editor";
					Desc.Center();
					SelectObjects* Main = ICE_NEW(SelectObjects)(Desc);
					Main->SetVisible(true);
					int Result = MyApp.Run();
					(void)Result;
					DELETESINGLE(Main);





				gMainWindow->SetEnabled(true);
			}

			InitLegoLib(filename, 0.1f);
			desc.mCamera[0] = PintCameraPose(Point(-4.65f, 15.97f, 17.71f), Point(0.62f, -0.42f, -0.66f));
			SetDefEnv(desc, true);
		}
		else if(strcmp(Ext, "zb2")==0)
		{
			Success = true;
			if(ImportZB2File(desc, filename))
			{
			}
		}
		else if(strcmp(Ext, "obj")==0)
		{
			Success = true;
			DELETESINGLE(mOBJ);
			mOBJ = ICE_NEW(WavefrontDatabase);
			WavefrontLoaderParams Params;
			//Params.mScale	= 1000.0f;
			LoadObj(filename, Params, *mOBJ);
			SetDefEnv(desc, false);
		}
		else
		{
			IceCore::MessageBox(0, "Unsupported file format.", "Error", MB_OK);
		}

		return Success;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		String Filename;
		if(gDragAndDropName)
		{
			Filename.Set(gDragAndDropName);
			gDragAndDropName = null;
		}
		else
		{
			FILESELECTCREATE Create;
			Create.mFilter			= "All Files (*.*)|*.*|ZB2 Files (*.zb2)|*.zb2|DAE Files (*.dae)|*.dae||";
//			Create.mFileName		= "file.zb2";
			Create.mFileName		= "";
			Create.mInitialDir		= GetRoot();
			Create.mCaptionTitle	= "Import scene";
//			Create.mDefExt			= "zb2";
			Create.mDefExt			= "";

			if(!FileselectOpenSingle(Create, Filename))
				return;

			if(!FileExists(Filename))
			{
				IceCore::MessageBox(0, "File not found.", "Error", MB_OK);
				return;
			}
		}

		if(Import(Filename, desc))
		{
			AddToRecentFiles(Filename, "PEEL");
			UpdateRecentFiles();
		}
	}

	virtual bool	Setup(Pint& pint, const PintCaps& caps)
	{
		if(mOBJ)
		{
			const udword NbMeshes = mOBJ->mMeshes.GetNbEntries();
			for(udword i=0;i<NbMeshes;i++)
			{
				const WavefrontMesh* Part = (const WavefrontMesh*)mOBJ->mMeshes[i];

				PINT_MESH_CREATE MeshCreate;
				MeshCreate.SetSurfaceData(Part->GetNbVerts(), Part->GetVerts(), Part->GetNbTris(), Part->GetIndices(), null);
				MeshCreate.mRenderer = CreateMeshRenderer(MeshCreate.GetSurface());

				PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
				ObjectDesc.mMass		= 0.0f;
//				ObjectDesc.mPosition	= Point(float(i)*3.0f, 0.0f, 0.0f);
				ObjectDesc.mPosition	= Part->mPos;
				CreatePintObject(pint, ObjectDesc);
			}
			return true;
		}

		SetupLego(pint, caps);
		CreateZB2Scene(pint, caps);
		return true;
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
		DELETESINGLE(mOBJ);
		TestBase::CommonRelease();
		CloseLegoLib();
	}

	bool	SetupLego(Pint& pint, const PintCaps& caps)
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
				ConvexCreate.mRenderer	= CreateMeshRenderer(PintSurfaceInterface(SI));

				PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
				ObjectDesc.mMass		= 1.0f;
				ObjectDesc.mPosition	= Point(float(i)*3.0f, 4.0f, 0.0f);
				CreatePintObject(pint, ObjectDesc);
			}
		}
		else
		{
			const udword NbParts = GetNbLegoParts();
			printf("%d Lego parts found\n", NbParts);

			const udword NbActors = GetNbLegoActors();
			printf("%d Lego actors found\n", NbActors);

			if(mImportMode==IMPORT_MODE_DYNAMIC_CONVEXES)
			{
				if(NbParts || NbActors)
				{
					if(!caps.mSupportRigidBodySimulation)
						return false;
				}
			}

			if(NbParts)
			{
				if(!caps.mSupportMeshes)
					return false;

				for(udword i=0;i<NbParts;i++)
				{
		//			const LegoPartMesh*	Part = GetLegoPartByID(60477);
					const LegoPartMesh*	Part = GetLegoPartByIndex(i);

					PINT_MESH_CREATE MeshCreate;
					MeshCreate.SetSurfaceData(Part->GetNbVerts(), Part->GetVerts(), Part->GetNbTris(), Part->GetIndices(), null);
					MeshCreate.mRenderer = CreateMeshRenderer(MeshCreate.GetSurface());

					PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
					ObjectDesc.mMass		= 0.0f;
	//				ObjectDesc.mPosition	= Point(float(i)*3.0f, 0.0f, 0.0f);
					ObjectDesc.mPosition	= Part->mPos;
					CreatePintObject(pint, ObjectDesc);
				}
			}

			if(NbActors)
			{
				if(mImportMode==IMPORT_MODE_DYNAMIC_CONVEXES && !caps.mSupportConvexes)
					return false;

				if(mImportMode==IMPORT_MODE_STATIC_MESHES && !caps.mSupportMeshes)
					return false;

				udword NbSharedMeshes = 0;
				for(udword i=0;i<NbActors;i++)
				{
					const LegoActor* Actor = GetLegoActorByIndex(i);

					LegoPartMesh* Mesh = Actor->mMesh;

					PINT_SHAPE_CREATE* ShapeCreate = null;

					PINT_CONVEX_CREATE ConvexShapeCreate;
					if(mImportMode==IMPORT_MODE_DYNAMIC_CONVEXES)
					{
						ConvexShapeCreate.mNbVerts	= Mesh->GetNbVerts();
						ConvexShapeCreate.mVerts	= Mesh->GetVerts();
						ShapeCreate = &ConvexShapeCreate;
					}

					PINT_MESH_CREATE MeshShapeCreate;
					if(mImportMode==IMPORT_MODE_STATIC_MESHES)
					{
	//					MeshShapeCreate.mName				= Mesh->mStringID;	//###
						MeshShapeCreate.SetSurfaceData(Mesh->GetNbVerts(), Mesh->GetVerts(), Mesh->GetNbTris(), Mesh->GetIndices(), null);
						ShapeCreate = &MeshShapeCreate;
					}

#ifdef USE_STATIC_BOXES
					AABB Bounds;
					ComputeAABB(Bounds, Mesh->GetVerts(), Mesh->GetNbVerts());

					Point Extents;
					Bounds.GetExtents(Extents);

					PINT_BOX_CREATE ShapeCreate(Extents);
#endif
					if(Mesh->mUserData && 1)
					{
						PintShapeRenderer* Renderer	= (PintShapeRenderer*)Mesh->mUserData;

						const ManagedTexture* MT = null;
						if(Actor->mMaterial && Actor->mMaterial->mEffect && Actor->mMaterial->mEffect->mImage)
							MT = Actor->mMaterial->mEffect->mImage->mTexture;

						ShapeCreate->mRenderer = CreateColorShapeRenderer(Renderer, Actor->mMaterial->mEffect->mDiffuse, MT);

						NbSharedMeshes++;
					}
					else
					{
						PintShapeRenderer* Renderer;
	//#ifdef USE_DYNAMIC_CONVEXES
						if(0)
						{
							SurfaceInterface SI;
							SI.mNbVerts	= Mesh->GetNbVerts();
							SI.mVerts	= Mesh->GetVerts();
							SI.mNbFaces	= Mesh->GetNbTris();
							SI.mDFaces	= Mesh->GetIndices();

							Renderer	= CreateMeshRenderer(PintSurfaceInterface(SI), null, true);
							ShapeCreate->mRenderer = CreateColorShapeRenderer(Renderer, Actor->mMaterial->mEffect->mDiffuse);
						}
	//#endif
	//#ifdef USE_STATIC_BOXES
						if(0)
						{
							SurfaceInterface SI;
							SI.mNbVerts	= Mesh->GetNbVerts();
							SI.mVerts	= Mesh->GetVerts();
							SI.mNbFaces	= Mesh->GetNbTris();
							SI.mDFaces	= Mesh->GetIndices();

							Renderer	= CreateMeshRenderer(PintSurfaceInterface(SI), null, true);
							ShapeCreate->mRenderer = CreateColorShapeRenderer(Renderer, Actor->mMaterial->mEffect->mDiffuse);
						}
	//#endif
	//#ifdef USE_STATIC_MESHES
						if(1)
						{
//							ManagedTexture MT;
							const ManagedTexture* MT = null;
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
									MT = Actor->mMaterial->mEffect->mImage->mTexture;
								Renderer = CreateMeshRenderer(MS, true, true);
							}
							else
							{
								SurfaceInterface SI;
								SI.mNbVerts	= Mesh->GetNbVerts();
								SI.mVerts	= Mesh->GetVerts();
								SI.mNbFaces	= Mesh->GetNbTris();
								SI.mDFaces	= Mesh->GetIndices();

								// ### TODO: check if CRC is needed here

								Renderer = CreateMeshRenderer(PintSurfaceInterface(SI), null, true, true);
	//							Renderer = CreateMeshRenderer(ShapeCreate.mSurface, true);
							}
		//					PintShapeRenderer* Renderer	= CreateMeshRenderer(MeshCreate.mSurface, true);
//							ShapeCreate.mRenderer		= CreateColorShapeRenderer(Renderer, Actor->mMaterial->mEffect->mDiffuse, &MT);
							ShapeCreate->mRenderer	= CreateColorShapeRenderer(Renderer, Actor->mMaterial->mEffect->mDiffuse, MT);
						}
	//#endif
						Mesh->mUserData		= Renderer;
					}

					PINT_OBJECT_CREATE ObjectDesc(ShapeCreate);
					ObjectDesc.mName		= Mesh->mStringID;
					ObjectDesc.mMass		= 0.0f;
					if(mImportMode==IMPORT_MODE_DYNAMIC_CONVEXES)
						ObjectDesc.mMass	= 1.0f;
					ObjectDesc.mPosition	= Actor->mPose.GetTrans();
					ObjectDesc.mRotation	= Actor->mPose;
					CreatePintObject(pint, ObjectDesc);
				}
				printf("NbSharedMeshes: %d\n", NbSharedMeshes);
			}
		}
		return true;
	}

}ImportSceneFromFile;

///////////////////////////////////////////////////////////////////////////////

static const char* gDesc_LegoStaticScenes = "Lego static scenes.";

struct LegoCarData
{
	const char*		mFilename;
	PintCameraPose	mCameraPose;
};

static LegoCarData gCarData[] = {
	{ "/Lego/#Sunset Track Racer/SunsetTrackRacer.dae", PintCameraPose(Point(8.60f, 8.34f, 10.20f), Point(-0.57f, -0.53f, -0.63f)) },
	{ "/Lego/#Lumenairo©_728_Tempesta/Lumenairo©_728_Tempesta.dae", PintCameraPose(Point(19.58f, 6.45f, -0.37f), Point(-0.55f, -0.52f, -0.66f)) },
	{ "/Lego/#458 Italia GT2/458_Italia_GT2.dae", PintCameraPose(Point(8.87f, 7.31f, 11.71f), Point(-0.51f, -0.52f, -0.69f)) },
	{ "/Lego/#1968 Ford Mustang Fastback/1968_Ford_Mustang_Fastback.dae", PintCameraPose(Point(9.26f, 7.59f, 12.31f), Point(-0.68f, -0.39f, -0.61f)) },
	{ "/Lego/#1974 Porsche 911 Turbo 3.0/1974_Porsche_911_Turbo_3_0.dae", PintCameraPose(Point(9.50f, 8.03f, 12.35f), Point(-0.53f, -0.58f, -0.62f)) },
	{ "/Lego/#2016 Ford GT & 1966 Ford GT40/2016_Ford_GT_&_1966_Ford_GT40.dae", PintCameraPose(Point(-13.05f, 12.06f, 19.17f), Point(0.46f, -0.56f, -0.69f)) },
	{ "/Lego/#2018 Dodge Challenger SRT Demon and 1970 Dodge Charger R_T/Dodge.dae", PintCameraPose(Point(15.99f, 13.01f, 14.44f), Point(-0.62f, -0.49f, -0.62f)) },
	{ "/Lego/#Chevrolet Camaro ZL1 Race Car/Chevrolet_Camaro_ZL1_Race_Car.dae", PintCameraPose(Point(11.62f, 6.89f, 10.95f), Point(-0.64f, -0.41f, -0.65f)) },
	{ "/Lego/#Chevrolet Corvette Z06/Chevrolet Corvette Z06.dae", PintCameraPose(Point(1.02f, 9.29f, 13.38f), Point(0.10f, -0.55f, -0.83f)) },
	{ "/Lego/#Ferrari 488 GT3 Scuderia Corsa/Ferrari_488_GT3_Scuderia_Corsa.dae", PintCameraPose(Point(-9.81f, 5.93f, 10.89f), Point(0.44f, -0.38f, -0.81f)) },
	{ "/Lego/#Ferrari F40 Competizione/Ferrari_F40_Competizione.dae", PintCameraPose(Point(-13.64f, 11.89f, 8.37f), Point(0.61f, -0.70f, -0.36f)) },
	{ "/Lego/#Ford Fiesta M-Sport WRC - reoriented/Ford_Fiesta_M-Sport_WRC.dae", PintCameraPose(Point(-11.20f, 9.58f, 6.50f), Point(0.59f, -0.66f, -0.46f)) },
	{ "/Lego/#Ford Mustang/Ford_Mustang.dae", PintCameraPose(Point(-14.79f, 15.71f, 25.08f), Point(0.59f, -0.47f, -0.66f)) },
	{ "/Lego/#McLaren Senna/McLaren_Senna.dae", PintCameraPose(Point(8.52f, 8.68f, 14.01f), Point(-0.49f, -0.54f, -0.68f)) },
	{ "/Lego/#McLaren P1/McLaren_P1.dae", PintCameraPose(Point(8.54f, 11.91f, 11.41f), Point(-0.51f, -0.69f, -0.51f)) },	
	{ "/Lego/#Porsche 911 GT Finish Line/Porsche_911_GT_Finish_Line.dae", PintCameraPose(Point(-34.30f, 14.58f, -33.83f), Point(0.76f, -0.31f, 0.58f)) },
	{ "/Lego/#Porsche 911 RSR and 911 Turbo 3.0/Porsche_911_RSR_and_911_Turbo_3_0.dae", PintCameraPose(Point(16.34f, 12.80f, -6.43f), Point(-0.75f, -0.53f, 0.38f)) },
	{ "/Lego/#Porsche 919 Hybrid and 917K Pit Lane/Porsche_919_Hybrid_and_917K_Pit_Lane.dae", PintCameraPose(Point(24.20f, 25.61f, 25.39f), Point(-0.56f, -0.65f, -0.51f)) },
	{ "/Lego/#Ferrari Ultimate Garage/Ferrari_Ultimate_Garage.dae", PintCameraPose(Point(-50.37f, 29.04f, 37.05f), Point(0.47f, -0.57f, -0.67f)) },
	{ "/Lego/#First Responder/First Responder.dae", PintCameraPose(Point(17.20f, 19.77f, 15.91f), Point(-0.64f, -0.56f, -0.52f)) },
	{ "/Lego/#Forklift Truck/Forklift_Truck.dae", PintCameraPose(Point(-8.66f, 19.99f, 20.55f), Point(0.48f, -0.54f, -0.69f)) },
	{ "/Lego/#Dune Buggy/Dune_Buggy.dae", PintCameraPose(Point(18.02f, 11.67f, -6.14f), Point(-0.86f, -0.36f, 0.37f)) },
	{ "/Lego/#Rally Car/Rally_Car.dae", PintCameraPose(Point(33.48f, 16.52f, 32.02f), Point(-0.72f, -0.32f, -0.62f)) },
	{ "/Lego/#Classic Go-Kart/Go-Kart.dae", PintCameraPose(Point(11.92f, 15.00f, 11.91f), Point(-0.55f, -0.66f, -0.51f)) },
	{ "/Lego/#New Go-Kart/Go-Kart.dae", PintCameraPose(Point(-9.18f, 13.35f, 10.64f), Point(0.41f, -0.73f, -0.55f)) },
	{ "/Lego/#Quad Bike/Quad Bike.dae", PintCameraPose(Point(13.91f, 8.72f, 11.96f), Point(-0.72f, -0.31f, -0.61f)) },
	{ "/Lego/#Buggy_8048/Buggy.dae", PintCameraPose(Point(19.31f, 18.32f, 19.07f), Point(-0.66f, -0.57f, -0.49f)) },
	{ "/Lego/#Bucket Wheel Excavator/Bucket Wheel Excavator.dae", PintCameraPose(Point(13.91f, 8.72f, 11.96f), Point(-0.72f, -0.31f, -0.61f)) },
};

class LegoStaticMesh : public ImportSceneFromFile
{
			ComboBoxPtr		mComboBox_Filename;

	public:
							LegoStaticMesh()			{									}
	virtual					~LegoStaticMesh()			{									}
	virtual	const char*		GetName()			const	{ return "LegoStaticMesh";			}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_STATIC_SCENE;		}
	virtual	const char*		GetDescription()	const	{ return gDesc_LegoStaticScenes;	}

	virtual	IceTabControl*	InitUI(PintGUIHelper& helper)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 50;
		WD.mY		= 50;
		WD.mWidth	= 450;
		WD.mHeight	= 150;
		WD.mLabel	= "Choose a scene";
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
				IceComboBox* CB = CreateComboBox<IceComboBox>(UI, 0, 4+OffsetX, y, 350, 300, "Filename", null, null);
				CB->Add("Sunset Track Racer (Creator)");
				CB->Add("Lumenairo© 728 Tempesta (MOC)");
				CB->Add("458 Italia GT2 (Speed Champions)");
				CB->Add("1968 Ford Mustang Fastback (Speed Champions)");
				CB->Add("1974 Porsche 911 Turbo 3.0 (Speed Champions)");
				CB->Add("2016 Ford GT & 1966 Ford GT40 (Speed Champions)");
				CB->Add("2018 Dodge Challenger SRT Demon & 1970 Dodge Charger R_T (Speed Champions)");
				CB->Add("Chevrolet Camaro ZL1 Race Car (Speed Champions)");
				CB->Add("Chevrolet Corvette Z06 (Speed Champions)");
				CB->Add("Ferrari 488 GT3 Scuderia Corsa (Speed Champions)");
				CB->Add("Ferrari F40 Competizione (Speed Champions)");
				CB->Add("Ford Fiesta M-Sport WRC (Speed Champions)");
				CB->Add("Ford Mustang (Creator)");
				CB->Add("McLaren Senna (Speed Champions)");
				CB->Add("McLaren P1 (Speed Champions)");
				CB->Add("Porsche 911 GT Finish Line (Speed Champions)");
				CB->Add("Porsche 911 RSR and 911 Turbo 3_0 (Speed Champions)");
				CB->Add("Porsche 919 Hybrid and 917K Pit Lane (Speed Champions)");
				CB->Add("Ferrari Ultimate Garage (Speed Champions)");
				CB->Add("First Responder (Technic)");
				CB->Add("Forklift Truck (Technic)");
				CB->Add("Dune Buggy (Technic)");
				CB->Add("Rally Car (Technic)");
				CB->Add("Classic Go-Kart (Technic)");
				CB->Add("New Go-Kart (Technic)");
				CB->Add("Quad Bike (Technic)");
				CB->Add("Buggy 8048 (Technic)");
				CB->Add("Bucket Wheel Excavator (Technic)");

				CB->Select(0);
				mComboBox_Filename = CB;
			}
			RegisterUIElement(mComboBox_Filename);
			y += YStep;
			y += YStep;
		}

		y += YStep;
		AddResetButton(UI, 4, y, WD.mWidth-16);

		return null;
	}

	virtual	void	GetSceneParams(PINT_WORLD_CREATE& desc)
	{
		TestBase::GetSceneParams(desc);

		udword Index = 0;

		if(mComboBox_Filename)
			Index = mComboBox_Filename->GetSelectedIndex();

		Import(FindPEELFile(gCarData[Index].mFilename), desc);
		desc.mCamera[0] = gCarData[Index].mCameraPose;
	}

}LegoStaticMesh;

static const char* gDesc_LegoExploding = "Lego exploding.";

class LegoExploding : public LegoStaticMesh
{
	public:
							LegoExploding()				{}
	virtual					~LegoExploding()			{}
	virtual	const char*		GetName()			const	{ return "LegoExploding";		}
	virtual	TestCategory	GetCategory()		const	{ return CATEGORY_BEHAVIOR;		}
	virtual	const char*		GetDescription()	const	{ return gDesc_LegoExploding;	}

	virtual	void			GetSceneParams(PINT_WORLD_CREATE& desc)
							{
								mImportMode = IMPORT_MODE_DYNAMIC_CONVEXES;
								LegoStaticMesh::GetSceneParams(desc);
							}
}LegoExploding;
