///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tool.h"
#include "ToolTerrainEditor.h"
#include "Camera.h"
#include "Terrain.h"
#include "TextureManager.h"
#include "GLRenderHelpers.h"
#include "GLTexture.h"
#include "GUI_PosEdit.h"
#include "PintObjectsManager.h"
#include "PintGLMeshShapeRenderer.h"

/*
				|Supported		|Supports cache	|Supports XYZ API
Layer			|
================|=========================================================
Bitmap			|<no, todo>		|-				|-
Cellular		|Yes			|Yes			|-
Clamp			|Yes			|No				|Yes
Crater			|No				|-				|-
Erosion			|No				|-				|-
fbm				|Yes			|Yes			|-
Filter			|No				|-				|-
Multiply-Add	|Yes			|No				|Yes
Midpoint		|No				|-				|-
Power			|Yes			|No				|Yes
Ridged			|Yes			|Yes			|-
Sinus			|Yes			|No but could	|-
Spectral		|No				|-				|-
Turbulence		|Yes			|Yes			|-

Constant		|<no, obsolete>	|-				|-
Scale			|<no, obsolete>	|-				|-


TODO:
- textures:
	- add option to disable texture
	- higher res textures
	- option to automatically update the texture => refine this
	- try elevated code to generate the texture
	- compute a small preview texture first?
	- palette editor to find better colors, esp that green...
	- edit sun position
	- output normal map, use in shader?
	- envmap should only be enabled on the snowy parts
	- generate detail texture?

- rendering:
	- smooth shading => needs to be able to recompute normals at runtime
	- disable shadowcaster for terrain?
	- add option to drop shading as well (to see just the texture)
	- could we put a big detail texture on just mipmap 0

- features:
	- load/save
	* enable ops
	- finish adding "difficult" layers (spectral, etc)

- performance:
	- skip update if new value is the same as old value
	- per-layer cache => finish it for all layers
	- multiple threads
	- async
		- text on screen when computing async

- try porting the ICE version and the CUDA version
- add button to setup each layer to "good" values



- option to lock a layer
- option to replicate the current settings on N tiles
- smooth layer?
- make sure everything can tile, and everything can have a start offset
	- i.e. we probably need to make everything a function f(x, y, z)
	- and then use that in the editor, instead of layers?
- capture params of one tile, apply them to all tiles?
- option to invert/negate a layer => just a scale by -1 ?

* terrain editor bug: after deleting a sinus grid and recreating a grid the sinus layer is still here on selection
* select terrain tile automatically after creation
* option to disable a layer
* enable cache
* texture
* isn't the scale layer redundant with that amplitude param?
* expose amplitude(s) to UI
* we need a start offset for the 3d-position-based noises
*/

namespace
{
	enum EditBoxID
	{
		EDIT_BOX_TILE_NB,
		EDIT_BOX_TILE_SIZE,
		EDIT_BOX_TILE_SCALE,
		//
		CHECK_BOX_ENABLE_LAYER,
		//
		EDIT_BOX_MADD_LAYER_ALTITUDE,
		EDIT_BOX_MADD_LAYER_AMPLITUDE,
		//
		//EDIT_BOX_POWER_LAYER_AMPLITUDE,
		EDIT_BOX_POWER_LAYER_FACTOR,
		//
		EDIT_BOX_CLAMP_LAYER_HEIGHT,
		CHECK_BOX_CLAMP_LAYER_BOOL,
		//
#ifdef OBSOLETE
		EDIT_BOX_CONSTANT_LAYER_HEIGHT,
		EDIT_BOX_SCALE_LAYER_SCALE,
#endif
		//
		EDIT_BOX_SINUS_LAYER_AMPLITUDE,
		EDIT_BOX_SINUS_LAYER_XFREQ,
		EDIT_BOX_SINUS_LAYER_YFREQ,
		EDIT_BOX_SINUS_LAYER_PHASEX,
		EDIT_BOX_SINUS_LAYER_PHASEY,
		//
		EDIT_BOX_FBM_LAYER_AMPLITUDE,
		EDIT_BOX_FBM_LAYER_XFREQ,
		EDIT_BOX_FBM_LAYER_YFREQ,
		EDIT_BOX_FBM_LAYER_PHASEX,
		EDIT_BOX_FBM_LAYER_PHASEY,
		EDIT_BOX_FBM_LAYER_Z,
		EDIT_BOX_FBM_LAYER_FRACTAL_INC,
		EDIT_BOX_FBM_LAYER_LACUNARITY,
		EDIT_BOX_FBM_LAYER_OCTAVES,
		//
		EDIT_BOX_TURB_LAYER_AMPLITUDE,
		EDIT_BOX_TURB_LAYER_XFREQ,
		EDIT_BOX_TURB_LAYER_YFREQ,
		EDIT_BOX_TURB_LAYER_PHASEX,
		EDIT_BOX_TURB_LAYER_PHASEY,
		EDIT_BOX_TURB_LAYER_Z,
		EDIT_BOX_TURB_LAYER_OCTAVES,
		//
		EDIT_BOX_RIDGED_LAYER_AMPLITUDE,
		EDIT_BOX_RIDGED_LAYER_XFREQ,
		EDIT_BOX_RIDGED_LAYER_YFREQ,
		EDIT_BOX_RIDGED_LAYER_PHASEX,
		EDIT_BOX_RIDGED_LAYER_PHASEY,
		EDIT_BOX_RIDGED_LAYER_Z,
		EDIT_BOX_RIDGED_LAYER_FRACTAL_INC,
		EDIT_BOX_RIDGED_LAYER_LACUNARITY,
		EDIT_BOX_RIDGED_LAYER_OCTAVES,
		EDIT_BOX_RIDGED_LAYER_OFFSET,
		EDIT_BOX_RIDGED_LAYER_GAIN,

		EDIT_BOX_CELLULAR_LAYER_AMPLITUDE,
		EDIT_BOX_CELLULAR_LAYER_PHASEX,
		EDIT_BOX_CELLULAR_LAYER_PHASEY,
		EDIT_BOX_CELLULAR_LAYER_SCALEX,
		EDIT_BOX_CELLULAR_LAYER_SCALEY,
		EDIT_BOX_CELLULAR_LAYER_Z,

//		EDIT_BOX_MIDPOINT_LAYER_AMPLITUDE,
		EDIT_BOX_MIDPOINT_LAYER_DISP,
		EDIT_BOX_MIDPOINT_LAYER_SMOOTHNESS,

//		EDIT_BOX_CRATER_LAYER_AMPLITUDE,
		EDIT_BOX_CRATER_LAYER_NB,

		CHECK_BOX_ENABLE_TEXTURE,
		CHECK_BOX_AUTO_TEXTURE_UPDATE,
	};
}

static udword gTileNb = 1;
static udword gTileSize = 128;
static float gTileScale = 0.01f;
//
static const float gDefault_MaddLayer_Altitude = 0.0f;
static const float gDefault_MaddLayer_Amplitude = 1.0f;

static const float gDefault_PowerLayer_Factor = 0.0f;

static const float gDefault_ClampLayer_Height = 0.0f;
static const bool gDefault_ClampLayer_Bool = false;

#ifdef OBSOLETE
static const float gDefault_ConstantLayer_Height = 0.0f;
static const float gDefault_ScaleLayer_Scale = 0.0f;
#endif

static const float gDefault_SinusLayer_Amplitude = 1.0f;
static const float gDefault_SinusLayer_XFreq = 0.0f;
static const float gDefault_SinusLayer_YFreq = 0.0f;
static const float gDefault_SinusLayer_PhaseX = 0.0f;
static const float gDefault_SinusLayer_PhaseY = 0.0f;

static const float gDefault_fbmLayer_Amplitude = 1.0f;
static const float gDefault_fbmLayer_XFreq = 0.0f;
static const float gDefault_fbmLayer_YFreq = 0.0f;
static const float gDefault_fbmLayer_PhaseX = 0.0f;
static const float gDefault_fbmLayer_PhaseY = 0.0f;
static const float gDefault_fbmLayer_Z = 0.0f;
static const float gDefault_fbmLayer_FractalInc = 0.0f;
static const float gDefault_fbmLayer_Lacunarity = 0.0f;
static const float gDefault_fbmLayer_Octaves = 0.0f;

static const float gDefault_TurbLayer_Amplitude = 1.0f;
static const float gDefault_TurbLayer_XFreq = 0.0f;
static const float gDefault_TurbLayer_YFreq = 0.0f;
static const float gDefault_TurbLayer_PhaseX = 0.0f;
static const float gDefault_TurbLayer_PhaseY = 0.0f;
static const float gDefault_TurbLayer_Z = 0.0f;
static const float gDefault_TurbLayer_Octaves = 0.0f;

static const float gDefault_RidgedLayer_Amplitude = 1.0f;
static const float gDefault_RidgedLayer_XFreq = 0.0f;
static const float gDefault_RidgedLayer_YFreq = 0.0f;
static const float gDefault_RidgedLayer_PhaseX = 0.0f;
static const float gDefault_RidgedLayer_PhaseY = 0.0f;
static const float gDefault_RidgedLayer_Z = 0.0f;
static const float gDefault_RidgedLayer_FractalInc = 0.0f;
static const float gDefault_RidgedLayer_Lacunarity = 0.0f;
static const float gDefault_RidgedLayer_Octaves = 0.0f;
static const float gDefault_RidgedLayer_Offset = 0.0f;
static const float gDefault_RidgedLayer_Gain = 0.0f;

static const float gDefault_CellularLayer_Amplitude = 1.0f;
static const float gDefault_CellularLayer_PhaseX = 0.0f;
static const float gDefault_CellularLayer_PhaseY = 0.0f;
static const float gDefault_CellularLayer_ScaleX = 0.0f;
static const float gDefault_CellularLayer_ScaleY = 0.0f;
static const float gDefault_CellularLayer_Z = 0.0f;

static const float gDefault_MidpointLayer_Amplitude = 1.0f;
static const float gDefault_MidpointLayer_Disp = 0.0f;
static const float gDefault_MidpointLayer_Smoothness = 1.0f;

static const float gDefault_CraterLayer_Amplitude = 1.0f;
static const udword gDefault_CraterLayer_Nb = 0;

static bool gTexture_Enabled = true;
static bool gTexture_AutoUpdate = true;

///////////////////////////////////////////////////////////////////////////////

static const char* gLayerNames[] = {
	"Bitmap",
	"Cellular",
	"Clamp",
	"Constant",
	"Crater",
	"Erosion",
	"fbm",
	"Filter",
	"Midpoint",
	"Power",
	"Ridged",
	"Scale",
	"Sinus",
	"Spectral",
	"Turbulence",
	"Multiply-Add",
};

namespace
{
	struct LayerData
	{
		const char*		mName;
		IceWindow*		mParams;
		HeightLayerType	mType;
	};

	static LayerData gLayers[] = 
	{
		{ "Multiply-Add", null, HEIGHT_LAYER_TYPE_MADD },
		{ "Power", null, HEIGHT_LAYER_TYPE_POWER },
		{ "Clamp", null, HEIGHT_LAYER_TYPE_CLAMP },
#ifdef OBSOLETE
		{ "Constant", null, HEIGHT_LAYER_TYPE_CONSTANT },
		{ "Scale", null, HEIGHT_LAYER_TYPE_SCALE },
#endif
		{ "Sinus", null, HEIGHT_LAYER_TYPE_SINUS },
		{ "fbm", null, HEIGHT_LAYER_TYPE_FBM },
		{ "Turbulence", null, HEIGHT_LAYER_TYPE_TURBULENCE },
		{ "Ridged", null, HEIGHT_LAYER_TYPE_RIDGED },
		{ "Cellular", null, HEIGHT_LAYER_TYPE_CELLULAR },
		{ "Midpoint", null, HEIGHT_LAYER_TYPE_MIDPOINT },
		{ "Crater", null, HEIGHT_LAYER_TYPE_CRATER },
	};
	static const udword gNbLayers = sizeof(gLayers)/sizeof(gLayers[0]);

	class LayerComboBox : public IceComboBox
	{
		public:
									LayerComboBox(const ComboBoxDesc& desc) : IceComboBox(desc), mOwner(null)	{}
		virtual						~LayerComboBox()															{}

		virtual	void				OnComboBoxEvent(ComboBoxEvent event)
		{
			if(event==CBE_SELECTION_CHANGED)
			{
				ASSERT(mOwner);
				//mOwner->OnComboBoxLayerSelected();
			}
		}
				ToolTerrainEditor*	mOwner;
	};

	class OpComboBox : public IceComboBox
	{
		public:
									OpComboBox(const ComboBoxDesc& desc) : IceComboBox(desc), mOwner(null)	{}
		virtual						~OpComboBox()															{}

		virtual	void				OnComboBoxEvent(ComboBoxEvent event)
		{
			if(event==CBE_SELECTION_CHANGED)
			{
				ASSERT(mOwner);
				HeightLayer* SelectedLayer = mOwner->GetSelectedLayer();
				SelectedLayer->SetOp(HeightLayerOp(GetSelectedIndex()));
				mOwner->UpdateTerrain();
			}
		}
				ToolTerrainEditor*	mOwner;
	};

	class LayerListBox : public IceListBox
	{
		public:
						LayerListBox(const ListBoxDesc& desc, ToolTerrainEditor* owner) : IceListBox(desc), mOwner(owner)	{}
		virtual			~LayerListBox()																						{}

		virtual	void	OnListboxEvent(ListBoxEvent event)
		{
			if(event==LBE_SELECTION_CHANGED)
			{
				ASSERT(mOwner);
				mOwner->OnListBoxLayerSelected();
			}
			else if(event==LBE_DBL_CLK)
			{
			}
		}

				ToolTerrainEditor*	mOwner;
	};

	static void gAddLayerCB(IceButton& button, void* user_data)
	{
		ToolTerrainEditor* tte = reinterpret_cast<ToolTerrainEditor*>(user_data);
		if(button.GetID()==0)
			tte->AddLayer();
		else if(button.GetID()==1)
			tte->RemoveLayer();
		else
			ASSERT(0);
	}
}

///////////////////////////////////////////////////////////////////////////////

ToolTerrainEditor::ToolTerrainEditor() :
	mSelectedTerrain		(null),
	mSelectedTerrainActor	(null),
	mSelectedTerrainShape	(null),
	mSelectedEngine			(null),
	mPendingAction			(NONE),
	mModifyTerrain			(false)
{
}

ToolTerrainEditor::~ToolTerrainEditor()
{
}

void ToolTerrainEditor::Reset(udword pint_index)
{
	ResetSelectedPointers();
	UpdateUI();

	mPendingAction			= NONE;
	mModifyTerrain			= false;
//	if(mLayerComboBox)
//		mLayerComboBox->RemoveAll();
//	if(mLayerListBox)
//		mLayerListBox->RemoveAll();
}

void ToolTerrainEditor::OnObjectReleased(Pint& pint, PintActorHandle removed_object)
{
	UnregisterTerrainData(removed_object);

	if(mSelectedTerrainActor==removed_object)
	{
		ResetSelectedPointers();
		UpdateUI();
	}
}

void ToolTerrainEditor::MouseMoveCallback(const MouseInfo& mouse)
{
	const int x = mouse.mMouseX;	mX = x;
	const int y = mouse.mMouseY;	mY = y;
	mDir = ComputeWorldRay(x, y);
	mOrigin = GetCameraPos();
}

void ToolTerrainEditor::KeyboardCallback(Pint& pint, udword pint_index, unsigned char key, bool down)
{
}

void ToolTerrainEditor::RightDownCallback(Pint& pint, udword pint_index)
{
//	mModifyTerrain = true;

	TerrainData* PreviouslySelected = mSelectedTerrain;
	ResetSelectedPointers();

	PintRaycastHit Hit;
	if(!mDir.IsZero() && Raycast(pint, Hit, mOrigin, mDir))
	{
		const PintActorHandle TerrainActor = Hit.mTouchedActor;
		TerrainData* TD = GetTerrainData(TerrainActor);
		if(TD)
		{
			// ### won't work for N plugins
			SetSelectedPointers(TD, TerrainActor, Hit.mTouchedShape, &pint);
		}
	}

	if(PreviouslySelected!=mSelectedTerrain)
		UpdateUI();
}

void ToolTerrainEditor::RightDragCallback(Pint& pint, udword pint_index)
{
}

void ToolTerrainEditor::RightUpCallback(Pint& pint, udword pint_index)
{
	mModifyTerrain = false;
}

static CustomLayer* gTEST = null;

void ToolTerrainEditor::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	if(mPendingAction==CREATE_GRID)
		CreateGridMesh(pint);
//	else if(mPendingAction==CREATE_TEXTURE)
//		CreateTexture(pint);

	if(1)
		return;


	PintRaycastHit Hit;
	if(mDir.IsZero() || !Raycast(pint, Hit, mOrigin, mDir))
		return;

	Pint_Shape* ShapeAPI = pint.GetShapeAPI();
	if(!ShapeAPI)
		return;

	const PintActorHandle TerrainActor = Hit.mTouchedActor;
	TerrainData* TD = GetTerrainData(TerrainActor);
	if(!TD)
		return;

	PintSphereOverlapData Overlap;
	Overlap.mSphere.mCenter = Hit.mImpact;
	Overlap.mSphere.mRadius = 1.0f;

	GLRenderHelpers::DrawFrame(Hit.mImpact, 1.0f);
	GLRenderHelpers::DrawSphereWireframe(Overlap.mSphere.mRadius, PR(Overlap.mSphere.mCenter, Quat(Idt)), Point(1.0f, 0.0f, 0.0f));

	const PintShapeHandle TerrainShape = Hit.mTouchedShape;

	const PintShape Type = ShapeAPI->GetType(TerrainShape);
	if(Type!=PINT_SHAPE_MESH && Type!=PINT_SHAPE_MESH2)
		return;

//	PintShapeRenderer* TouchedShapeRenderer = ShapeAPI->GetShapeRenderer(TerrainShape);
//	if(!TouchedShapeRenderer || strcmp(TouchedShapeRenderer->GetClassName(), "PintGLMeshShapeRenderer")!=0)
//		return;

	const PR TerrainPose = ShapeAPI->GetWorldTransform(TerrainActor, TerrainShape);

	Container TouchedTriangleIndices;
	if(!ShapeAPI->FindTouchedTriangles(TouchedTriangleIndices, pint.mSQHelper->GetThreadContext(), TerrainShape, TerrainPose, Overlap))
		return;

	SurfaceInterface SI;
	if(!ShapeAPI->GetTriangleMeshData(SI, TerrainShape, true))
		return;

	Container VertexIndices;
	const Matrix4x4 M = TerrainPose;

	const udword NbToucheddTris = TouchedTriangleIndices.GetNbEntries();
	for(udword i=0;i<NbToucheddTris;i++)
	{
		if(0)
		{
			Triangle T;
			if(ShapeAPI->GetTriangle(T, TerrainShape, TouchedTriangleIndices[i]))
			{
				GLRenderHelpers::DrawTriangle(T.mVerts[0], T.mVerts[1], T.mVerts[2], Point(1.0f, 0.0f, 0.0f));
			}
		}
		else
		{
			IndexedTriangle T;
			if(ShapeAPI->GetIndexedTriangle(T, TerrainShape, TouchedTriangleIndices[i]))						
			{
				const Point p0 = SI.mVerts[T.mRef[0]] * M;
				const Point p1 = SI.mVerts[T.mRef[1]] * M;
				const Point p2 = SI.mVerts[T.mRef[2]] * M;

//				GLRenderHelpers::DrawTriangle(SI.mVerts[T.mRef[0]], SI.mVerts[T.mRef[1]], SI.mVerts[T.mRef[2]], Point(1.0f, 0.0f, 0.0f));
				GLRenderHelpers::DrawLine(p0, p1, Point(1.0f, 0.0f, 0.0f));
				GLRenderHelpers::DrawLine(p1, p2, Point(1.0f, 0.0f, 0.0f));
				GLRenderHelpers::DrawLine(p2, p0, Point(1.0f, 0.0f, 0.0f));

				//V[T.mRef[0]].y += 0.01f;
				//V[T.mRef[1]].y += 0.01f;
				//V[T.mRef[2]].y += 0.01f;
				VertexIndices.AddUnique(T.mRef[0]);
				VertexIndices.AddUnique(T.mRef[1]);
				VertexIndices.AddUnique(T.mRef[2]);
			}
		}
	}

	if(!mModifyTerrain)
		return;

/*	static bool init=false;
	if(!init)
	{
		init = true;
		const float Amplitude = 40.0f;
		const float FractalIncrement = 1.0f;
		const float Lacunarity = 2.17f;
		const float Octaves = 12.0f;
		FractalBrownianMotion fbm(FractalIncrement, Lacunarity, Octaves);
		const float rf_FractalIncrement	= 1.3f;
		const float rf_Octaves			= 8.0f;
		const float rf_Offset			= 10.0f;
		const float rf_Gain				= 2.0f;
		const float rf_Amplitude		= 100.0f;
		const float rf_Offset2 = 15560.0f;
		const udword TextureFactor = 16;

		Point* V = const_cast<Point*>(SI.mVerts);
		for(udword i=0;i<SI.mNbVerts;i++)
		{
			const Point& wp = V[i];
			float w = Amplitude * 5.0f * fbm.Compute(wp*0.02f) * fbm.Compute(wp*0.01f);	// This one is pretty cool
			V[i].y += w;
		}
	}*/

	if(1)
	{
float* h = gTEST->GetHeights();
		Point* V = const_cast<Point*>(SI.mVerts);
		for(udword i=0;i<VertexIndices.GetNbEntries();i++)
		{
			Point Delta = V[VertexIndices[i]]*M - Hit.mImpact;
			Delta.y = 0.0f;
			float m = Delta.Magnitude();
			if(m<=Overlap.mSphere.mRadius)
			{
				//GLRenderHelpers::DrawFrame(V[VertexIndices[i]], 1.0f);

				const float d = 1.0f - (m/Overlap.mSphere.mRadius);

	//			V[VertexIndices[i]].y -= d * 0.02f;
	//			V[VertexIndices[i]].y += d * 0.06f;
				//V[VertexIndices[i]].y += d * 0.1f;
	//			V[VertexIndices[i]].y += d * w;
				//printf("%f\n", d);
h[VertexIndices[i]] += d * 0.1f;
			}
		}
TD->UpdateVertices(V);

		ShapeAPI->Refit(TerrainShape, Hit.mTouchedActor);
	}




	//(PintGLMeshShapeRenderer*)(TouchedShapeRenderer)->UpdateVerts(V);
//	TD->UpdateVertices();
	TD->UpdateRenderer();
}

void ToolTerrainEditor::PostRenderCallback()
{
	mPendingAction = NONE;
}

bool ToolTerrainEditor::CreateGridMesh(Pint& pint)
{
	{
		PintCaps caps;
		pint.GetCaps(caps);

		if(!caps.mSupportMeshes || !caps.mSupportDeformableMeshes)
			return false;
	}

	Pint_Actor* ActorAPI = pint.GetActorAPI();

	// ### 40.0f is a hardcoded coeff in ICE (MakePlane function)
	const float Coeff = (float(gTileSize-1)*0.5f) * 40.0f * gTileScale;

	const float TotalLength = float(gTileNb-1)*Coeff*2.0f;
	const Point Origin(-TotalLength*0.5f, 0.0f, -TotalLength*0.5f);

	bool ShouldUpdateUI = false;

	for(udword j=0;j<gTileNb;j++)
	{
		for(udword i=0;i<gTileNb;i++)
		{
			//### share this one
			TerrainData* TD = CreateTerrainData();
			TD->Init(gTileSize, gTileSize, gTileScale, gTileScale);
			if(0)
			{
				CustomLayer* CL = ICE_NEW(CustomLayer);
				CUSTOMLAYERCREATE CLC;
				CLC.mAmplitude	= 1.0f;
				CLC.mWidth		= gTileSize;
				CLC.mHeight		= gTileSize;
				CLC.mHeights	= null;
				CL->Init(CLC);
				TD->AddLayer(CL);
				gTEST = CL;
			}

#ifdef SUPPORT_TERRAIN_TEXTURE
		const udword TexWidth = 2048;
		const udword TexHeight = 2048;
		Picture P(TexWidth, TexHeight);
		//P.MakeVoronoi();
	P.MakeAlphaGrid(4);
	//P.AlphaToColor();
	const Point MainColor = pint.GetMainColor();
	RGBAPixel* Pix = P.GetPixels();
	for(udword k=0;k<TexWidth*TexHeight;k++)
	{
		if(Pix[k].A!=255)
		{
			Pix[k].R = ubyte(MainColor.x*255.0f);
			Pix[k].G = ubyte(MainColor.y*255.0f);
			Pix[k].B = ubyte(MainColor.z*255.0f);
			Pix[k].A = PIXEL_OPAQUE;
		}
		else
		{
			Pix[k].R = 0;
			Pix[k].G = 0;
			Pix[k].B = 0;
			Pix[k].A = PIXEL_OPAQUE;
		}
	}


		const ManagedTexture* MT = CreateManagedTexture(TexWidth, TexHeight, P.GetPixels(), null);
		TD->mTexture = MT;
#endif
			PintMeshHandle mh;
			{
				PINT_MESH_DATA_CREATE MeshDesc;
				MeshDesc.SetSurfaceData(TD->mSurface.GetSurfaceInterface());
				MeshDesc.mDeformable = true;
				mh = pint.CreateMeshObject(MeshDesc);
			}

//			PINT_MESH_DATA_CREATE MeshDataCreate;
//			MeshDataCreate.mSurface		= TD->mSurface.GetSurfaceInterface();
//			MeshDataCreate.mDeformable	= true;
//			PintMeshHandle mh = pint.CreateMeshObject(MeshDataCreate);

			PINT_MESH_CREATE2 MeshCreate;
			MeshCreate.mTriangleMesh	= mh;
			MeshCreate.mRenderer		= TD->mRenderer;
#ifdef SUPPORT_TERRAIN_TEXTURE
			MeshCreate.mRenderer		= CreateColorShapeRenderer(TD->mRenderer, RGBAColor(0.0f, 1.0f, 0.0f), MT);
#endif
			PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
			ObjectDesc.mMass		= 0.0f;
			// ### Boundary edges won't match exactly
			ObjectDesc.mPosition	= Origin+Point(float(i)*Coeff*2.0f, 0.0f, float(j)*Coeff*2.0f);
			PintActorHandle h = CreatePintObject(pint, ObjectDesc);

			RegisterTerrainData(h, TD);

			// Auto select
			if(ActorAPI)
			{
				if(ActorAPI->GetNbShapes(h)==1)
				{
					SetSelectedPointers(TD, h, ActorAPI->GetShape(h, 0), &pint);
					ShouldUpdateUI = true;
				}
			}
		}
	}

	if(ShouldUpdateUI)
		UpdateUI();

	return true;
}

/*bool ToolTerrainEditor::CreateTexture(Pint& pint)
{
	return true;
}*/

static void FillUI(const HeightLayer& layer);
void ToolTerrainEditor::OnListBoxLayerSelected()
{
	const HeightLayer* SelectedLayer = GetSelectedLayer();

	if(mOpComboBox)
	{
		mOpComboBox->SetEnabled(SelectedLayer->SupportsOp());
		mOpComboBox->Select(SelectedLayer->GetOp());
	}

	const HeightLayerType Type = SelectedLayer->GetType();
//	udword LayerIndex = INVALID_ID;
	for(udword i=0;i<gNbLayers;i++)
	{
/*		if(gLayers[i].mType==Type)
		{
			mLayerComboBox->Select(i);
			OnComboBoxLayerSelected();
			//TODO: fill UI layer values from existing selected layer
			break;
		}*/

		const bool IsSelected = gLayers[i].mType==Type;

		if(gLayers[i].mParams)
			gLayers[i].mParams->SetVisible(IsSelected);

//		if(IsSelected)
//		{
//			ASSERT(LayerIndex==INVALID_ID);
//			LayerIndex = i;
//		}
	}

//	ASSERT(LayerIndex!=INVALID_ID);
	FillUI(*SelectedLayer);
}

void ToolTerrainEditor::OnComboBoxLayerSelected()
{
	ASSERT(mLayerComboBox.mWidget);
	const udword Selected = mLayerComboBox->GetSelectedIndex();
	for(udword i=0;i<gNbLayers;i++)
	{
		if(gLayers[i].mParams)
			gLayers[i].mParams->SetVisible(i==Selected);
	}
}

void ToolTerrainEditor::UpdateUI()
{
	const bool IsTerrainSelected = mSelectedTerrain!=null;

	if(mAddLayer)
		mAddLayer->SetEnabled(IsTerrainSelected);

	if(mRemoveLayer)
		mRemoveLayer->SetEnabled(IsTerrainSelected);

	if(mLayerListBox)
	{
		mLayerListBox->RemoveAll();
		mLayerListBox->SetEnabled(IsTerrainSelected);
	}

	if(mSelectedTerrain && mLayerListBox)
	{
		const udword NbLayers = mSelectedTerrain->mLayers.GetNbEntries();
		for(udword i=0;i<NbLayers;i++)
		{
			const HeightLayer* Layer = reinterpret_cast<const HeightLayer*>(mSelectedTerrain->mLayers.GetEntry(i));
			const HeightLayerType Type = Layer->GetType();
			//ASSERT(Type!=HEIGHT_LAYER_TYPE_UNDEFINED);
			if(Type!=HEIGHT_LAYER_TYPE_UNDEFINED)
			{
				const udword Index = mLayerListBox->GetItemCount();
				mLayerListBox->Add(gLayerNames[Type]);
				mLayerListBox->SetItemData(Index, const_cast<HeightLayer*>(Layer));	//###
			}
			else
				mLayerListBox->Add("CustomLayer");
		}
	}

	for(udword i=0;i<gNbLayers;i++)
	{
		if(gLayers[i].mParams)
			gLayers[i].mParams->SetVisible(false);
	}
}

void ToolTerrainEditor::AddLayer()
{
	ASSERT(mSelectedTerrain);
	if(!mSelectedTerrain)
		return;

	const udword SelectedLayerIndex = mLayerComboBox->GetSelectedIndex();
	const LayerData& SelectedLayer = gLayers[SelectedLayerIndex];

	HeightLayer* NewLayer = null;
	switch(SelectedLayer.mType)
	{
		case HEIGHT_LAYER_TYPE_MADD:
		{
			MaddLayer* NL = ICE_NEW(MaddLayer);

			MADDLAYERCREATE Create;
			Create.mWidth		= mSelectedTerrain->mNbX;
			Create.mHeight		= mSelectedTerrain->mNbY;
			Create.mAmplitude	= gDefault_MaddLayer_Amplitude;
			//
			Create.mAltitude	= gDefault_MaddLayer_Altitude;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		case HEIGHT_LAYER_TYPE_POWER:
		{
			PowerLayer* NL = ICE_NEW(PowerLayer);

			POWERLAYERCREATE Create;
			Create.mWidth		= mSelectedTerrain->mNbX;
			Create.mHeight		= mSelectedTerrain->mNbY;
			Create.mAmplitude	= 1.0f;//gDefault_PowerLayer_Amplitude;
			//
			Create.mPowerFactor	= gDefault_PowerLayer_Factor;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		case HEIGHT_LAYER_TYPE_CLAMP:
		{
			ClampLayer* NL = ICE_NEW(ClampLayer);

			CLAMPLAYERCREATE Create;
			Create.mWidth			= mSelectedTerrain->mNbX;
			Create.mHeight			= mSelectedTerrain->mNbY;
			Create.mAmplitude		= gDefault_ClampLayer_Height;
			//
			Create.mCutGreaterThan	= gDefault_ClampLayer_Bool;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

#ifdef OBSOLETE
		case HEIGHT_LAYER_TYPE_CONSTANT:
		{
			ConstantLayer* NL = ICE_NEW(ConstantLayer);

			CONSTANTLAYERCREATE Create;
			Create.mWidth			= mSelectedTerrain->mNbX;
			Create.mHeight			= mSelectedTerrain->mNbY;
			Create.mAmplitude		= 1.0f;
			//
			Create.mConstantHeight	= gDefault_ConstantLayer_Height;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		case HEIGHT_LAYER_TYPE_SCALE:
		{
			ScaleLayer* NL = ICE_NEW(ScaleLayer);

			SCALELAYERCREATE Create;
			Create.mWidth			= mSelectedTerrain->mNbX;
			Create.mHeight			= mSelectedTerrain->mNbY;
			Create.mAmplitude		= 1.0f;
			//
			Create.mScaleFactor		= gDefault_ScaleLayer_Scale;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;
#endif

		case HEIGHT_LAYER_TYPE_SINUS:
		{
			SinusLayer* NL = ICE_NEW(SinusLayer);

			SINUSLAYERCREATE Create;
			Create.mWidth		= mSelectedTerrain->mNbX;
			Create.mHeight		= mSelectedTerrain->mNbY;
			Create.mAmplitude	= gDefault_SinusLayer_Amplitude;
			//
			Create.mXFreq		= gDefault_SinusLayer_XFreq;
			Create.mYFreq		= gDefault_SinusLayer_YFreq;
			Create.mPhaseX		= gDefault_SinusLayer_PhaseX;
			Create.mPhaseY		= gDefault_SinusLayer_PhaseY;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		case HEIGHT_LAYER_TYPE_FBM:
		{
			FBMLayer* NL = ICE_NEW(FBMLayer);

			FBMLAYERCREATE Create;
			Create.mWidth		= mSelectedTerrain->mNbX;
			Create.mHeight		= mSelectedTerrain->mNbY;
			Create.mAmplitude	= gDefault_fbmLayer_Amplitude;
			//
			Create.mOffsetX				= gDefault_fbmLayer_PhaseX;
			Create.mOffsetX				= gDefault_fbmLayer_PhaseY;
			Create.mXFreq				= gDefault_fbmLayer_XFreq;
			Create.mYFreq				= gDefault_fbmLayer_YFreq;
			Create.mFractalIncrement	= gDefault_fbmLayer_FractalInc;
			Create.mLacunarity			= gDefault_fbmLayer_Lacunarity;
			Create.mOctaves				= gDefault_fbmLayer_Octaves;
			Create.mZ					= gDefault_fbmLayer_Z;

			NL->Init(Create);
			NewLayer = NL;

			//NL->SetCompatibility(false);
		}
		break;

		case HEIGHT_LAYER_TYPE_TURBULENCE:
		{
			TurbulenceLayer* NL = ICE_NEW(TurbulenceLayer);

			TURBULENCELAYERCREATE Create;
			Create.mWidth		= mSelectedTerrain->mNbX;
			Create.mHeight		= mSelectedTerrain->mNbY;
			Create.mAmplitude	= gDefault_TurbLayer_Amplitude;
			//
			Create.mOffsetX		= gDefault_TurbLayer_PhaseX;
			Create.mOffsetX		= gDefault_TurbLayer_PhaseY;
			Create.mXFreq		= gDefault_TurbLayer_XFreq;
			Create.mYFreq		= gDefault_TurbLayer_YFreq;
			Create.mOctaves		= gDefault_TurbLayer_Octaves;
			Create.mZ			= gDefault_TurbLayer_Z;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		case HEIGHT_LAYER_TYPE_RIDGED:
		{
			RidgedFractalLayer* NL = ICE_NEW(RidgedFractalLayer);

			RIDGEDLAYERCREATE Create;
			Create.mWidth		= mSelectedTerrain->mNbX;
			Create.mHeight		= mSelectedTerrain->mNbY;
			Create.mAmplitude	= gDefault_RidgedLayer_Amplitude;
			//
			Create.mOffsetX				= gDefault_RidgedLayer_PhaseX;
			Create.mOffsetX				= gDefault_RidgedLayer_PhaseY;
			Create.mXFreq				= gDefault_RidgedLayer_XFreq;
			Create.mYFreq				= gDefault_RidgedLayer_YFreq;
			Create.mFractalIncrement	= gDefault_RidgedLayer_FractalInc;
			Create.mLacunarity			= gDefault_RidgedLayer_Lacunarity;
			Create.mOctaves				= gDefault_RidgedLayer_Octaves;
			Create.mOffset				= gDefault_RidgedLayer_Offset;
			Create.mGain				= gDefault_RidgedLayer_Gain;
			Create.mZ					= gDefault_RidgedLayer_Z;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		case HEIGHT_LAYER_TYPE_CELLULAR:
		{
			CellularLayer* NL = ICE_NEW(CellularLayer);

			CELLULARLAYERCREATE Create;
			Create.mWidth		= mSelectedTerrain->mNbX;
			Create.mHeight		= mSelectedTerrain->mNbY;
			Create.mAmplitude	= gDefault_CellularLayer_Amplitude;
			//
			Create.mOffsetX		= gDefault_CellularLayer_PhaseX;
			Create.mOffsetX		= gDefault_CellularLayer_PhaseY;
			Create.mScaleX		= gDefault_CellularLayer_ScaleX;
			Create.mScaleY		= gDefault_CellularLayer_ScaleY;
			Create.mZ			= gDefault_CellularLayer_Z;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		case HEIGHT_LAYER_TYPE_MIDPOINT:
		{
			MidpointLayer* NL = ICE_NEW(MidpointLayer);

			MIDPOINTLAYERCREATE Create;
			Create.mWidth			= mSelectedTerrain->mNbX;
			Create.mHeight			= mSelectedTerrain->mNbY;
			Create.mAmplitude		= gDefault_MidpointLayer_Amplitude;
			//
			Create.mDisplacement	= gDefault_MidpointLayer_Disp;
			Create.mSmoothness		= gDefault_MidpointLayer_Smoothness;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		case HEIGHT_LAYER_TYPE_CRATER:
		{
			CraterLayer* NL = ICE_NEW(CraterLayer);

			CRATERLAYERCREATE Create;
			Create.mWidth			= mSelectedTerrain->mNbX;
			Create.mHeight			= mSelectedTerrain->mNbY;
			Create.mAmplitude		= gDefault_CraterLayer_Amplitude;
			//
			Create.mNbCraters		= gDefault_CraterLayer_Nb;

			NL->Init(Create);
			NewLayer = NL;
		}
		break;

		default:
			ASSERT(0);
			break;
	};

	if(NewLayer)
	{
		NewLayer->SetCacheEnabled(true);

		mSelectedTerrain->AddLayer(NewLayer);

		if(0)
		{
			const udword Index = mLayerListBox->GetItemCount();
			mLayerListBox->Add("Sinus");	// ### should return an index
			mLayerListBox->SetItemData(Index, NewLayer);
		}
		UpdateUI();
		UpdateTerrain();
	}
}

void ToolTerrainEditor::RemoveLayer()
{
	ASSERT(mSelectedTerrain);
	ASSERT(mLayerListBox.mWidget);
	if(!mSelectedTerrain || !mLayerListBox)
		return;

	const int SelectedIndex = mLayerListBox->GetSelectedIndex();
	//printf("SelectedIndex: %d\n", SelectedIndex);
	if(SelectedIndex<0)
		return;

	HeightLayer* Layer = reinterpret_cast<HeightLayer*>(mLayerListBox->GetItemData(SelectedIndex));
	mSelectedTerrain->RemoveLayer(Layer);
	DELETESINGLE(Layer);
	UpdateUI();
	UpdateTerrain();
}

void ToolTerrainEditor::UpdateTerrain()
{
	Pint_Shape* ShapeAPI = mSelectedEngine->GetShapeAPI();
	ASSERT(ShapeAPI);

	SurfaceInterface SI;
	if(ShapeAPI->GetTriangleMeshData(SI, mSelectedTerrainShape, true))
	{
		Point* V = const_cast<Point*>(SI.mVerts);
		mSelectedTerrain->UpdateVertices(V);
		ShapeAPI->Refit(mSelectedTerrainShape, mSelectedTerrainActor);
		mSelectedTerrain->UpdateRenderer();

		if(gTexture_AutoUpdate)
			UpdateTexture();
	}
}

void ToolTerrainEditor::UpdateTexture()
{
#ifdef SUPPORT_TERRAIN_TEXTURE
	const TerrainData* TD = mSelectedTerrain;
	if(!TD)
		return;

	if(!TD->mTexture)
		return;

	const udword TexFactor = 1;
	const udword TexWidth = TD->mNbX * TexFactor;
	const udword TexHeight = TD->mNbY * TexFactor;

	Heightfield HF(TexWidth, TexHeight);
	TD->Evaluate(HF);

	TERRAINTEXTURECREATE ttc;
	ttc.mField			= HF.GetHeights();
	ttc.mWidth			= TexWidth;
	ttc.mHeight			= TexHeight;
	ttc.mRenderWater	= false;
	ttc.mSeaLevel		= 0.0f;

	const float MinMax[2] = { -200.0f, 200.0f };
//	ttc.mMinMax	= MinMax;

	TerrainTexture TT;
	TT.Create(ttc);

	if(TD->mTexture)
	{
		GLTexture::releaseTexture(TD->mTexture->mGLID);
		TD->mTexture->mGLID = GLTexture::createTexture(TT.GetWidth(), TT.GetHeight(), TT.GetPixels(), true);
	}
/*	else
	{
		const ManagedTexture* MT = CreateManagedTexture(TT.GetWidth(), TT.GetHeight(), TT.GetPixels(), "TerrainTexture");
	}*/
#endif
}

HeightLayer* ToolTerrainEditor::GetSelectedLayer() const
{
	ASSERT(mLayerListBox.mWidget);
	const udword Selected = mLayerListBox->GetSelectedIndex();
	if(Selected==INVALID_ID)
		return null;

	ASSERT(mSelectedTerrain);
	//### could also use the UI object's user data here
	HeightLayer* SelectedLayer = mSelectedTerrain->GetLayer(Selected);
	ASSERT(SelectedLayer);
	return SelectedLayer;
}

///////////////////////////////////////////////////////////////////////////////

#include "GUI_Helpers.h"

namespace
{
	enum TabIndex
	{
		TAB_TILES,
		TAB_LAYERS,
		TAB_TEXTURE,
		TAB_COUNT,
	};
}

static void CreateTilesTab(PintGUIHelper& helper, IceWindow* tab, Widgets& owner, ToolTerrainEditor* tte)
{
	const sdword x = 4;
	sdword y = 4;
	const sdword OffsetX = 60;
	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;

	struct Callbacks
	{
		static void CreateGrid(IceButton& button, void* user_data)
		{
			ToolTerrainEditor* tte = reinterpret_cast<ToolTerrainEditor*>(user_data);
			tte->mPendingAction = ToolTerrainEditor::CREATE_GRID;
		}

		static void gEditBox(const IceEditBox& edit_box, udword param, void* user_data)
		{
			const udword ID = edit_box.GetID();
			if(ID==EDIT_BOX_TILE_NB)
				gTileNb = GetInt(gTileNb, &edit_box);
			else if(ID==EDIT_BOX_TILE_SIZE)
				gTileSize = GetInt(gTileSize, &edit_box);
			else if(ID==EDIT_BOX_TILE_SCALE)
				gTileScale = GetFloat(gTileScale, &edit_box);
		}
	};

//	y += YStep;
	y += 10;

	helper.CreateLabel(tab, x, y+LabelOffsetY, 50, 20, "Tile nb:", &owner);
	helper.CreateEditBox(tab, EDIT_BOX_TILE_NB, x+OffsetX, y, EditBoxWidth, 20, _F("%d", gTileNb), &owner, EDITBOX_INTEGER_POSITIVE, Callbacks::gEditBox, null);
	y += YStep;

	helper.CreateLabel(tab, x, y+LabelOffsetY, 50, 20, "Tile size:", &owner);
	helper.CreateEditBox(tab, EDIT_BOX_TILE_SIZE, x+OffsetX, y, EditBoxWidth, 20, _F("%d", gTileSize), &owner, EDITBOX_INTEGER_POSITIVE, Callbacks::gEditBox, null);
	y += YStep;

	helper.CreateLabel(tab, x, y+LabelOffsetY, 50, 20, "Tile scale:", &owner);
	helper.CreateEditBox(tab, EDIT_BOX_TILE_SCALE, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gTileScale), &owner, EDITBOX_FLOAT_POSITIVE, Callbacks::gEditBox, null);
	y += YStep;

//	IceButton* B = helper.CreateButton(parent, 0, x, y, 100, 20, "Button", &owner, Local::ButtonCB, null);
	{
		ButtonDesc BD;
		BD.mParent	= tab;
		BD.mX		= x;
		BD.mY		= y;
		BD.mWidth	= 150;
		BD.mHeight	= 20;
		BD.mStyle	= BUTTON_NORMAL;
		BD.mLabel	= "Create grid";
		IceButton* B = ICE_NEW(IceButton)(BD);
		B->SetCallback(Callbacks::CreateGrid);
		B->SetUserData(tte);
		owner.Register(B);
	}
}

static IceCheckBox* gCheckBox_EnableLayer = null;

static IceEditBox* gEditBox_MaddLayer_Altitude = null;
static IceEditBox* gEditBox_MaddLayer_Amplitude = null;

static IceEditBox* gEditBox_PowerLayer_Factor = null;

static IceEditBox* gEditBox_ClampLayer_Height = null;
static IceCheckBox* gCheckBox_ClampLayer_Bool = null;

#ifdef OBSOLETE
static IceEditBox* gEditBox_ConstantLayer_Height = null;
static IceEditBox* gEditBox_ScaleLayer_Scale = null;
#endif

static IceEditBox* gEditBox_SinusLayer_Amplitude = null;
static IceEditBox* gEditBox_SinusLayer_XFreq = null;
static IceEditBox* gEditBox_SinusLayer_YFreq = null;
static IceEditBox* gEditBox_SinusLayer_PhaseX = null;
static IceEditBox* gEditBox_SinusLayer_PhaseY = null;

static IceEditBox* gEditBox_fbmLayer_Amplitude = null;
static IceEditBox* gEditBox_fbmLayer_XFreq = null;
static IceEditBox* gEditBox_fbmLayer_YFreq = null;
static IceEditBox* gEditBox_fbmLayer_PhaseX = null;
static IceEditBox* gEditBox_fbmLayer_PhaseY = null;
static IceEditBox* gEditBox_fbmLayer_Z = null;
static IceEditBox* gEditBox_fbmLayer_FractalInc = null;
static IceEditBox* gEditBox_fbmLayer_Lacunarity = null;
static IceEditBox* gEditBox_fbmLayer_Octaves = null;

static IceEditBox* gEditBox_TurbLayer_Amplitude = null;
static IceEditBox* gEditBox_TurbLayer_XFreq = null;
static IceEditBox* gEditBox_TurbLayer_YFreq = null;
static IceEditBox* gEditBox_TurbLayer_PhaseX = null;
static IceEditBox* gEditBox_TurbLayer_PhaseY = null;
static IceEditBox* gEditBox_TurbLayer_Z = null;
static IceEditBox* gEditBox_TurbLayer_Octaves = null;

static IceEditBox* gEditBox_RidgedLayer_Amplitude = null;
static IceEditBox* gEditBox_RidgedLayer_XFreq = null;
static IceEditBox* gEditBox_RidgedLayer_YFreq = null;
static IceEditBox* gEditBox_RidgedLayer_PhaseX = null;
static IceEditBox* gEditBox_RidgedLayer_PhaseY = null;
static IceEditBox* gEditBox_RidgedLayer_Z = null;
static IceEditBox* gEditBox_RidgedLayer_FractalInc = null;
static IceEditBox* gEditBox_RidgedLayer_Lacunarity = null;
static IceEditBox* gEditBox_RidgedLayer_Octaves = null;
static IceEditBox* gEditBox_RidgedLayer_Offset = null;
static IceEditBox* gEditBox_RidgedLayer_Gain = null;

static IceEditBox* gEditBox_CellularLayer_Amplitude = null;
static IceEditBox* gEditBox_CellularLayer_PhaseX = null;
static IceEditBox* gEditBox_CellularLayer_PhaseY = null;
static IceEditBox* gEditBox_CellularLayer_ScaleX = null;
static IceEditBox* gEditBox_CellularLayer_ScaleY = null;
static IceEditBox* gEditBox_CellularLayer_Z = null;

static IceEditBox* gEditBox_MidpointLayer_Amplitude = null;
static IceEditBox* gEditBox_MidpointLayer_Disp = null;
static IceEditBox* gEditBox_MidpointLayer_Smoothness = null;

static IceEditBox* gEditBox_CraterLayer_Amplitude = null;
static IceEditBox* gEditBox_CraterLayer_Nb = null;

static IceCheckBox* gCheckBox_EnableTexture = null;
static IceCheckBox* gCheckBox_AutoTextureUpdate = null;

static void FillUI(const HeightLayer& layer)
{
	ASSERT(gCheckBox_EnableLayer);		gCheckBox_EnableLayer->SetChecked(layer.IsLayerEnabled());

	switch(layer.GetType())
	{
		case HEIGHT_LAYER_TYPE_MADD:
		{
			const MaddLayer& L = static_cast<const MaddLayer&>(layer);
			ASSERT(gEditBox_MaddLayer_Amplitude);	gEditBox_MaddLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_MaddLayer_Altitude);	gEditBox_MaddLayer_Altitude->SetText(_F("%f", L.GetAltitude()));
		}
		break;

		case HEIGHT_LAYER_TYPE_POWER:
		{
			const PowerLayer& L = static_cast<const PowerLayer&>(layer);
			//ASSERT(gEditBox_PowerLayer_Amplitude);	gEditBox_PowerLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_PowerLayer_Factor);			gEditBox_PowerLayer_Factor->SetText(_F("%f", L.GetPowerFactor()));
		}
		break;

		case HEIGHT_LAYER_TYPE_CLAMP:
		{
			const ClampLayer& L = static_cast<const ClampLayer&>(layer);
			ASSERT(gEditBox_ClampLayer_Height);		gEditBox_ClampLayer_Height->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gCheckBox_ClampLayer_Bool);		gCheckBox_ClampLayer_Bool->SetChecked(L.GetCutGreaterThan());
		}
		break;

#ifdef OBSOLETE
		case HEIGHT_LAYER_TYPE_CONSTANT:
		{
			const ConstantLayer& L = static_cast<const ConstantLayer&>(layer);
			ASSERT(gEditBox_ConstantLayer_Height);	gEditBox_ConstantLayer_Height->SetText(_F("%f", L.GetConstantHeight()));
		}
		break;

		case HEIGHT_LAYER_TYPE_SCALE:
		{
			const ScaleLayer& L = static_cast<const ScaleLayer&>(layer);
			ASSERT(gEditBox_ScaleLayer_Scale);	gEditBox_ScaleLayer_Scale->SetText(_F("%f", L.GetScaleFactor()));
		}
		break;
#endif

		case HEIGHT_LAYER_TYPE_SINUS:
		{
			const SinusLayer& L = static_cast<const SinusLayer&>(layer);
			ASSERT(gEditBox_SinusLayer_Amplitude);	gEditBox_SinusLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_SinusLayer_XFreq);		gEditBox_SinusLayer_XFreq->SetText(_F("%f", L.GetXFreq()));
			ASSERT(gEditBox_SinusLayer_YFreq);		gEditBox_SinusLayer_YFreq->SetText(_F("%f", L.GetYFreq()));
			ASSERT(gEditBox_SinusLayer_PhaseX);		gEditBox_SinusLayer_PhaseX->SetText(_F("%f", L.GetPhaseX()));
			ASSERT(gEditBox_SinusLayer_PhaseY);		gEditBox_SinusLayer_PhaseY->SetText(_F("%f", L.GetPhaseY()));
		}
		break;

		case HEIGHT_LAYER_TYPE_FBM:
		{
			const FBMLayer& L = static_cast<const FBMLayer&>(layer);
			ASSERT(gEditBox_fbmLayer_Amplitude);	gEditBox_fbmLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_fbmLayer_PhaseX);		gEditBox_fbmLayer_PhaseX->SetText(_F("%f", L.GetOffsetX()));
			ASSERT(gEditBox_fbmLayer_PhaseY);		gEditBox_fbmLayer_PhaseY->SetText(_F("%f", L.GetOffsetY()));
			ASSERT(gEditBox_fbmLayer_XFreq);		gEditBox_fbmLayer_XFreq->SetText(_F("%f", L.GetXFreq()));
			ASSERT(gEditBox_fbmLayer_YFreq);		gEditBox_fbmLayer_YFreq->SetText(_F("%f", L.GetYFreq()));
			ASSERT(gEditBox_fbmLayer_Z);			gEditBox_fbmLayer_Z->SetText(_F("%f", L.GetZ()));
			ASSERT(gEditBox_fbmLayer_FractalInc);	gEditBox_fbmLayer_FractalInc->SetText(_F("%f", L.GetFBM().GetFractalIncrement()));
			ASSERT(gEditBox_fbmLayer_Lacunarity);	gEditBox_fbmLayer_Lacunarity->SetText(_F("%f", L.GetFBM().GetLacunarity()));
			ASSERT(gEditBox_fbmLayer_Octaves);		gEditBox_fbmLayer_Octaves->SetText(_F("%f", L.GetFBM().GetOctaves()));
		}
		break;

		case HEIGHT_LAYER_TYPE_TURBULENCE:
		{
			const TurbulenceLayer& L = static_cast<const TurbulenceLayer&>(layer);
			ASSERT(gEditBox_TurbLayer_Amplitude);	gEditBox_TurbLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_TurbLayer_PhaseX);		gEditBox_TurbLayer_PhaseX->SetText(_F("%f", L.GetOffsetX()));
			ASSERT(gEditBox_TurbLayer_PhaseY);		gEditBox_TurbLayer_PhaseY->SetText(_F("%f", L.GetOffsetY()));
			ASSERT(gEditBox_TurbLayer_XFreq);		gEditBox_TurbLayer_XFreq->SetText(_F("%f", L.GetXFreq()));
			ASSERT(gEditBox_TurbLayer_YFreq);		gEditBox_TurbLayer_YFreq->SetText(_F("%f", L.GetYFreq()));
			ASSERT(gEditBox_TurbLayer_Z);			gEditBox_TurbLayer_Z->SetText(_F("%f", L.GetZ()));
			ASSERT(gEditBox_TurbLayer_Octaves);		gEditBox_TurbLayer_Octaves->SetText(_F("%f", L.GetOctaves()));
		}
		break;

		case HEIGHT_LAYER_TYPE_RIDGED:
		{
			const RidgedFractalLayer& L = static_cast<const RidgedFractalLayer&>(layer);
			ASSERT(gEditBox_RidgedLayer_Amplitude);		gEditBox_RidgedLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_RidgedLayer_PhaseX);		gEditBox_RidgedLayer_PhaseX->SetText(_F("%f", L.GetOffsetX()));
			ASSERT(gEditBox_RidgedLayer_PhaseY);		gEditBox_RidgedLayer_PhaseY->SetText(_F("%f", L.GetOffsetY()));
			ASSERT(gEditBox_RidgedLayer_XFreq);			gEditBox_RidgedLayer_XFreq->SetText(_F("%f", L.GetXFreq()));
			ASSERT(gEditBox_RidgedLayer_YFreq);			gEditBox_RidgedLayer_YFreq->SetText(_F("%f", L.GetYFreq()));
			ASSERT(gEditBox_RidgedLayer_Z);				gEditBox_RidgedLayer_Z->SetText(_F("%f", L.GetZ()));
			ASSERT(gEditBox_RidgedLayer_FractalInc);	gEditBox_RidgedLayer_FractalInc->SetText(_F("%f", L.GetRidgedFractal().GetFractalIncrement()));
			ASSERT(gEditBox_RidgedLayer_Lacunarity);	gEditBox_RidgedLayer_Lacunarity->SetText(_F("%f", L.GetRidgedFractal().GetLacunarity()));
			ASSERT(gEditBox_RidgedLayer_Octaves);		gEditBox_RidgedLayer_Octaves->SetText(_F("%f", L.GetRidgedFractal().GetOctaves()));
			ASSERT(gEditBox_RidgedLayer_Offset);		gEditBox_RidgedLayer_Offset->SetText(_F("%f", L.GetRidgedFractal().GetOffset()));
			ASSERT(gEditBox_RidgedLayer_Gain);			gEditBox_RidgedLayer_Gain->SetText(_F("%f", L.GetRidgedFractal().GetGain()));
		}
		break;

		case HEIGHT_LAYER_TYPE_CELLULAR:
		{
			const CellularLayer& L = static_cast<const CellularLayer&>(layer);
			ASSERT(gEditBox_CellularLayer_Amplitude);	gEditBox_CellularLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_CellularLayer_PhaseX);		gEditBox_CellularLayer_PhaseX->SetText(_F("%f", L.GetOffsetX()));
			ASSERT(gEditBox_CellularLayer_PhaseY);		gEditBox_CellularLayer_PhaseY->SetText(_F("%f", L.GetOffsetY()));
			ASSERT(gEditBox_CellularLayer_ScaleX);		gEditBox_CellularLayer_ScaleX->SetText(_F("%f", L.GetScaleX()));
			ASSERT(gEditBox_CellularLayer_ScaleY);		gEditBox_CellularLayer_ScaleY->SetText(_F("%f", L.GetScaleY()));
			ASSERT(gEditBox_CellularLayer_Z);			gEditBox_CellularLayer_Z->SetText(_F("%f", L.GetZ()));
		}
		break;

		case HEIGHT_LAYER_TYPE_MIDPOINT:
		{
			const MidpointLayer& L = static_cast<const MidpointLayer&>(layer);
//			ASSERT(gEditBox_MidpointLayer_Amplitude);	gEditBox_MidpointLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_MidpointLayer_Disp);		gEditBox_MidpointLayer_Disp->SetText(_F("%f", L.GetDisplacement()));
			ASSERT(gEditBox_MidpointLayer_Smoothness);	gEditBox_MidpointLayer_Smoothness->SetText(_F("%f", L.GetSmoothness()));
		}
		break;

		case HEIGHT_LAYER_TYPE_CRATER:
		{
			const CraterLayer& L = static_cast<const CraterLayer&>(layer);
//			ASSERT(gEditBox_CraterLayer_Amplitude);		gEditBox_CraterLayer_Amplitude->SetText(_F("%f", L.GetAmplitude()));
			ASSERT(gEditBox_CraterLayer_Nb);			gEditBox_CraterLayer_Nb->SetText(_F("%d", L.GetNbCraters()));
		}
		break;

		default:
		{
			ASSERT(0);
		}
		break;
	}
}

/*#define UPDATE_BOOL_VALUE(def, layer_type, layer_class, layer_func)	\
	const float Value = GetFloat(def, &edit_box);					\
	ASSERT(SelectedLayer->GetType()==layer_type);					\
	static_cast<layer_class*>(SelectedLayer)->layer_func(Value);	\
	tte->UpdateTerrain();
*/

static void gLayers_CheckBox_CB(const IceCheckBox& check_box, bool checked, void* user_data)
{
	ToolTerrainEditor* tte = reinterpret_cast<ToolTerrainEditor*>(user_data);
	ASSERT(tte);

	HeightLayer* SelectedLayer = tte->GetSelectedLayer();
	if(SelectedLayer)
	{
		const udword ID = check_box.GetID();
		switch(ID)
		{
			case CHECK_BOX_ENABLE_LAYER:
			{
				SelectedLayer->SetLayerEnabled(check_box.IsChecked());
				tte->UpdateTerrain();
			}
			break;

			case CHECK_BOX_CLAMP_LAYER_BOOL:
			{
			//	UPDATE_BOOL_VALUE(gDefault_ClampLayer_Bool, HEIGHT_LAYER_TYPE_CLAMP, ClampLayer, SetCutGreaterThan)

				const bool Value = check_box.IsChecked();
				ASSERT(SelectedLayer->GetType()==HEIGHT_LAYER_TYPE_CLAMP);
				static_cast<ClampLayer*>(SelectedLayer)->SetCutGreaterThan(Value);
				tte->UpdateTerrain();
			}
			break;
		}
	}
}

#define UPDATE_FLOAT_VALUE(def, layer_type, layer_class, layer_func)\
	const float Value = GetFloat(def, &edit_box);					\
	ASSERT(SelectedLayer->GetType()==layer_type);					\
	static_cast<layer_class*>(SelectedLayer)->layer_func(Value);	\
	tte->UpdateTerrain();

#define UPDATE_INT_VALUE(def, layer_type, layer_class, layer_func)	\
	const udword Value = GetInt(def, &edit_box);					\
	ASSERT(SelectedLayer->GetType()==layer_type);					\
	static_cast<layer_class*>(SelectedLayer)->layer_func(Value);	\
	tte->UpdateTerrain();

static void gLayers_EditBox_CB(const IceEditBox& edit_box, udword param, void* user_data)
{
	ToolTerrainEditor* tte = reinterpret_cast<ToolTerrainEditor*>(user_data);
	ASSERT(tte);

	HeightLayer* SelectedLayer = tte->GetSelectedLayer();

	const udword ID = edit_box.GetID();
	switch(ID)
	{
		case EDIT_BOX_MADD_LAYER_AMPLITUDE:		{ UPDATE_FLOAT_VALUE(gDefault_MaddLayer_Amplitude, HEIGHT_LAYER_TYPE_MADD, MaddLayer, SetAmplitude)					}	break;
		case EDIT_BOX_MADD_LAYER_ALTITUDE:		{ UPDATE_FLOAT_VALUE(gDefault_MaddLayer_Altitude, HEIGHT_LAYER_TYPE_MADD, MaddLayer, SetAltitude)					}	break;

		//case EDIT_BOX_POWER_LAYER_AMPLITUDE:	{ UPDATE_FLOAT_VALUE(gDefault_PowerLayer_Amplitude, HEIGHT_LAYER_TYPE_POWER, PowerLayer, SetAmplitude)				}	break;
		case EDIT_BOX_POWER_LAYER_FACTOR:		{ UPDATE_FLOAT_VALUE(gDefault_PowerLayer_Factor, HEIGHT_LAYER_TYPE_POWER, PowerLayer, SetPowerFactor)				}	break;

		case EDIT_BOX_CLAMP_LAYER_HEIGHT:		{ UPDATE_FLOAT_VALUE(gDefault_ClampLayer_Height, HEIGHT_LAYER_TYPE_CLAMP, ClampLayer, SetAmplitude)					}	break;
//		case CHECK_BOX_CLAMP_LAYER_BOOL:		{ UPDATE_BOOL_VALUE(gDefault_ClampLayer_Bool, HEIGHT_LAYER_TYPE_CLAMP, ClampLayer, SetCutGreaterThan)				}	break;

#ifdef OBSOLETE
		case EDIT_BOX_CONSTANT_LAYER_HEIGHT:	{ UPDATE_FLOAT_VALUE(gDefault_ConstantLayer_Height, HEIGHT_LAYER_TYPE_CONSTANT, ConstantLayer, SetConstantHeight)	}	break;
		case EDIT_BOX_SCALE_LAYER_SCALE:		{ UPDATE_FLOAT_VALUE(gDefault_ScaleLayer_Scale, HEIGHT_LAYER_TYPE_SCALE, ScaleLayer, SetScaleFactor)				}	break;
#endif

		case EDIT_BOX_SINUS_LAYER_AMPLITUDE:	{ UPDATE_FLOAT_VALUE(gDefault_SinusLayer_Amplitude, HEIGHT_LAYER_TYPE_SINUS, SinusLayer, SetAmplitude)				}	break;
		case EDIT_BOX_SINUS_LAYER_XFREQ:		{ UPDATE_FLOAT_VALUE(gDefault_SinusLayer_XFreq, HEIGHT_LAYER_TYPE_SINUS, SinusLayer, SetXFreq)						}	break;
		case EDIT_BOX_SINUS_LAYER_YFREQ:		{ UPDATE_FLOAT_VALUE(gDefault_SinusLayer_YFreq, HEIGHT_LAYER_TYPE_SINUS, SinusLayer, SetYFreq)						}	break;
		case EDIT_BOX_SINUS_LAYER_PHASEX:		{ UPDATE_FLOAT_VALUE(gDefault_SinusLayer_PhaseX, HEIGHT_LAYER_TYPE_SINUS, SinusLayer, SetPhaseX)					}	break;
		case EDIT_BOX_SINUS_LAYER_PHASEY:		{ UPDATE_FLOAT_VALUE(gDefault_SinusLayer_PhaseY, HEIGHT_LAYER_TYPE_SINUS, SinusLayer, SetPhaseY)					}	break;

		case EDIT_BOX_FBM_LAYER_AMPLITUDE:		{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_Amplitude, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetAmplitude)					}	break;
		case EDIT_BOX_FBM_LAYER_XFREQ:			{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_XFreq, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetXFreq)							}	break;
		case EDIT_BOX_FBM_LAYER_YFREQ:			{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_YFreq, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetYFreq)							}	break;
		case EDIT_BOX_FBM_LAYER_PHASEX:			{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_PhaseX, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetOffsetX)							}	break;
		case EDIT_BOX_FBM_LAYER_PHASEY:			{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_PhaseY, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetOffsetY)							}	break;
		case EDIT_BOX_FBM_LAYER_Z:				{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_Z, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetZ)									}	break;
		case EDIT_BOX_FBM_LAYER_FRACTAL_INC:	{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_FractalInc, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetFractalIncrement)			}	break;
		case EDIT_BOX_FBM_LAYER_LACUNARITY:		{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_Lacunarity, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetLacunarity)					}	break;
		case EDIT_BOX_FBM_LAYER_OCTAVES:		{ UPDATE_FLOAT_VALUE(gDefault_fbmLayer_Octaves, HEIGHT_LAYER_TYPE_FBM, FBMLayer, SetOctaves)						}	break;

		case EDIT_BOX_TURB_LAYER_AMPLITUDE:		{ UPDATE_FLOAT_VALUE(gDefault_TurbLayer_Amplitude, HEIGHT_LAYER_TYPE_TURBULENCE, TurbulenceLayer, SetAmplitude)		}	break;
		case EDIT_BOX_TURB_LAYER_XFREQ:			{ UPDATE_FLOAT_VALUE(gDefault_TurbLayer_XFreq, HEIGHT_LAYER_TYPE_TURBULENCE, TurbulenceLayer, SetXFreq)				}	break;
		case EDIT_BOX_TURB_LAYER_YFREQ:			{ UPDATE_FLOAT_VALUE(gDefault_TurbLayer_YFreq, HEIGHT_LAYER_TYPE_TURBULENCE, TurbulenceLayer, SetYFreq)				}	break;
		case EDIT_BOX_TURB_LAYER_PHASEX:		{ UPDATE_FLOAT_VALUE(gDefault_TurbLayer_PhaseX, HEIGHT_LAYER_TYPE_TURBULENCE, TurbulenceLayer, SetOffsetX)			}	break;
		case EDIT_BOX_TURB_LAYER_PHASEY:		{ UPDATE_FLOAT_VALUE(gDefault_TurbLayer_PhaseY, HEIGHT_LAYER_TYPE_TURBULENCE, TurbulenceLayer, SetOffsetY)			}	break;
		case EDIT_BOX_TURB_LAYER_Z:				{ UPDATE_FLOAT_VALUE(gDefault_TurbLayer_Z, HEIGHT_LAYER_TYPE_TURBULENCE, TurbulenceLayer, SetZ)						}	break;
		case EDIT_BOX_TURB_LAYER_OCTAVES:		{ UPDATE_FLOAT_VALUE(gDefault_TurbLayer_Octaves, HEIGHT_LAYER_TYPE_TURBULENCE, TurbulenceLayer, SetOctaves)			}	break;

		case EDIT_BOX_RIDGED_LAYER_AMPLITUDE:	{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_Amplitude, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetAmplitude)			}	break;
		case EDIT_BOX_RIDGED_LAYER_XFREQ:		{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_XFreq, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetXFreq)					}	break;
		case EDIT_BOX_RIDGED_LAYER_YFREQ:		{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_YFreq, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetYFreq)					}	break;
		case EDIT_BOX_RIDGED_LAYER_PHASEX:		{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_PhaseX, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetOffsetX)					}	break;
		case EDIT_BOX_RIDGED_LAYER_PHASEY:		{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_PhaseY, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetOffsetY)					}	break;
		case EDIT_BOX_RIDGED_LAYER_Z:			{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_Z, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetZ)							}	break;
		case EDIT_BOX_RIDGED_LAYER_FRACTAL_INC:	{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_FractalInc, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetFractalIncrement)	}	break;
		case EDIT_BOX_RIDGED_LAYER_LACUNARITY:	{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_Lacunarity, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetLacunarity)			}	break;
		case EDIT_BOX_RIDGED_LAYER_OCTAVES:		{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_Octaves, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetOctaves)				}	break;
		case EDIT_BOX_RIDGED_LAYER_OFFSET:		{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_Offset, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetOffset)					}	break;
		case EDIT_BOX_RIDGED_LAYER_GAIN:		{ UPDATE_FLOAT_VALUE(gDefault_RidgedLayer_Gain, HEIGHT_LAYER_TYPE_RIDGED, RidgedFractalLayer, SetGain)						}	break;

		case EDIT_BOX_CELLULAR_LAYER_AMPLITUDE:	{ UPDATE_FLOAT_VALUE(gDefault_CellularLayer_Amplitude, HEIGHT_LAYER_TYPE_CELLULAR, CellularLayer, SetAmplitude)		}	break;
		case EDIT_BOX_CELLULAR_LAYER_PHASEX:	{ UPDATE_FLOAT_VALUE(gDefault_CellularLayer_PhaseX, HEIGHT_LAYER_TYPE_CELLULAR, CellularLayer, SetOffsetX)			}	break;
		case EDIT_BOX_CELLULAR_LAYER_PHASEY:	{ UPDATE_FLOAT_VALUE(gDefault_CellularLayer_PhaseY, HEIGHT_LAYER_TYPE_CELLULAR, CellularLayer, SetOffsetY)			}	break;
		case EDIT_BOX_CELLULAR_LAYER_SCALEX:	{ UPDATE_FLOAT_VALUE(gDefault_CellularLayer_ScaleX, HEIGHT_LAYER_TYPE_CELLULAR, CellularLayer, SetScaleX)			}	break;
		case EDIT_BOX_CELLULAR_LAYER_SCALEY:	{ UPDATE_FLOAT_VALUE(gDefault_CellularLayer_ScaleY, HEIGHT_LAYER_TYPE_CELLULAR, CellularLayer, SetScaleY)			}	break;
		case EDIT_BOX_CELLULAR_LAYER_Z:			{ UPDATE_FLOAT_VALUE(gDefault_CellularLayer_Z, HEIGHT_LAYER_TYPE_CELLULAR, CellularLayer, SetZ)						}	break;

//		case EDIT_BOX_MIDPOINT_LAYER_AMPLITUDE:	{ UPDATE_FLOAT_VALUE(gDefault_MidpointLayer_Amplitude, HEIGHT_LAYER_TYPE_MIDPOINT, MidpointLayer, SetAmplitude)		}	break;
		case EDIT_BOX_MIDPOINT_LAYER_DISP:		{ UPDATE_FLOAT_VALUE(gDefault_MidpointLayer_Disp, HEIGHT_LAYER_TYPE_MIDPOINT, MidpointLayer, SetDisplacement)		}	break;
		case EDIT_BOX_MIDPOINT_LAYER_SMOOTHNESS:{ UPDATE_FLOAT_VALUE(gDefault_MidpointLayer_Smoothness, HEIGHT_LAYER_TYPE_MIDPOINT, MidpointLayer, SetSmoothness)	}	break;

//		case EDIT_BOX_CRATER_LAYER_AMPLITUDE:	{ UPDATE_FLOAT_VALUE(gDefault_CraterLayer_Amplitude, HEIGHT_LAYER_TYPE_CRATER, CraterLayer, SetAmplitude)	}	break;
		case EDIT_BOX_CRATER_LAYER_NB:			{ UPDATE_INT_VALUE(gDefault_CraterLayer_Nb, HEIGHT_LAYER_TYPE_CRATER, CraterLayer, SetNbCraters)			}	break;

		default:	ASSERT(0);	break;
	};

#ifdef REMOVED
	if(ID==EDIT_BOX_CONSTANT_LAYER_HEIGHT)
	{
		const float Value = GetFloat(gDefault_ConstantLayer_Height, &edit_box);
		ASSERT(SelectedLayer->GetType()==HEIGHT_LAYER_TYPE_CONSTANT);
		static_cast<ConstantLayer*>(SelectedLayer)->SetConstantHeight(Value);
		tte->UpdateTerrain();
	}
	else if(ID==EDIT_BOX_SINUS_LAYER_XFREQ)
	{
		const float Value = GetFloat(gDefault_SinusLayer_XFreq, &edit_box);
		ASSERT(SelectedLayer->GetType()==HEIGHT_LAYER_TYPE_SINUS);
		static_cast<SinusLayer*>(SelectedLayer)->SetXFreq(Value);
		tte->UpdateTerrain();
	}
	else if(ID==EDIT_BOX_SINUS_LAYER_YFREQ)
	{
		const float Value = GetFloat(gDefault_SinusLayer_YFreq, &edit_box);
		ASSERT(SelectedLayer->GetType()==HEIGHT_LAYER_TYPE_SINUS);
		static_cast<SinusLayer*>(SelectedLayer)->SetYFreq(Value);
		tte->UpdateTerrain();
	}
	else if(ID==EDIT_BOX_SINUS_LAYER_PHASEX)
	{
		const float Value = GetFloat(gDefault_SinusLayer_PhaseX, &edit_box);
		ASSERT(SelectedLayer->GetType()==HEIGHT_LAYER_TYPE_SINUS);
		static_cast<SinusLayer*>(SelectedLayer)->SetPhaseX(Value);
		tte->UpdateTerrain();
	}
	else if(ID==EDIT_BOX_SINUS_LAYER_PHASEY)
	{
		const float Value = GetFloat(gDefault_SinusLayer_PhaseY, &edit_box);
		ASSERT(SelectedLayer->GetType()==HEIGHT_LAYER_TYPE_SINUS);
		static_cast<SinusLayer*>(SelectedLayer)->SetPhaseY(Value);
		tte->UpdateTerrain();
	}
	else
		ASSERT(0);
#endif
}

static void CreateLayersTab(PintGUIHelper& helper, IceWindow* tab, Widgets& owner, ToolTerrainEditor* tte)
{
	const sdword x = 4;
	sdword y = 4;

	const sdword YStep = 20;
	const sdword LabelWidth = 100;
	const sdword LabelOffsetY = 2;
	const sdword OffsetX = 60;
	const sdword EditBoxWidth = 60;

	helper.CreateLabel(tab, x, y+LabelOffsetY, LabelWidth, 20, "Layers:", &owner);
	LayerComboBox* LCB = CreateComboBox<LayerComboBox>(tab, 0, x+OffsetX, y, 100, 20, "Layers", &owner, null);
	LCB->mOwner = tte;
	LCB->SetVisible(true);
	tte->mLayerComboBox = LCB;

	{
		ListBoxDesc LBD;
		LBD.mParent		= tab;
		LBD.mX			= x + 280;
		LBD.mY			= y;
		LBD.mWidth		= 150;
		LBD.mHeight		= 200;
		LBD.mLabel		= "List box";
		LayerListBox* LLB	= ICE_NEW(LayerListBox)(LBD, tte);
		owner.Register(LLB);
		tte->mLayerListBox = LLB;

		tte->mAddLayer = helper.CreateButton(tab, 0, x + 280, y + LBD.mHeight + 8, 130, 20, "Add layer", &owner, gAddLayerCB, tte);
		tte->mRemoveLayer = helper.CreateButton(tab, 1, x + 280, y + LBD.mHeight + 8 + 20, 130, 20, "Remove layer", &owner, gAddLayerCB, tte);
	}

	{
		y += YStep;
		helper.CreateLabel(tab, x, y+LabelOffsetY, LabelWidth, 20, "Op:", &owner);
		OpComboBox* OCB = CreateComboBox<OpComboBox>(tab, 0, x+OffsetX, y, 100, 20, "Op", &owner, null);
		OCB->Add("Set");
		OCB->Add("Add");
		OCB->Add("Multiply");
		OCB->Select(1);
		OCB->mOwner = tte;
		OCB->SetVisible(true);
		OCB->SetEnabled(false);
		tte->mOpComboBox = OCB;
	}

	{
		y += YStep;
		gCheckBox_EnableLayer = helper.CreateCheckBox(tab, CHECK_BOX_ENABLE_LAYER, x, y, EditBoxWidth, 20, "Enable:", &owner, true, gLayers_CheckBox_CB, null);
		gCheckBox_EnableLayer->SetUserData(tte);
	}

	y += YStep;

	IceEditBox* LayerBox;
	{
		const sdword ToolSpecificAreaWidth = 260;
		const sdword ToolSpecificAreaHeight = 260;

		LayerBox = helper.CreateEditBox(tab, 0, 4, y, ToolSpecificAreaWidth, ToolSpecificAreaHeight, "=== Layer-specific settings ===", &owner, EDITBOX_TEXT, null);
		LayerBox->SetReadOnly(true);
		//LayerBox->SetVisible(true);
		y += ToolSpecificAreaHeight;
	}

	for(udword i=0;i<gNbLayers;i++)
	{
		LCB->Add(gLayers[i].mName);

		WindowDesc WD;
		WD.mParent	= LayerBox;
		WD.mX		= 4;
		WD.mY		= 20;
		WD.mWidth	= 240;
		WD.mHeight	= 240;
		WD.mLabel	= "Params";
		WD.mType	= WINDOW_DIALOG;
		//WD.mStyle	= WSTYLE_BORDER;
		IceWindow* Params = ICE_NEW(IceWindow)(WD);
		owner.Register(Params);
		Params->SetVisible(false);
		gLayers[i].mParams = Params;
	}
	LCB->Select(0);

	udword Index=0;

	{
		IceWindow* Params = gLayers[Index++].mParams;
		//Params->SetVisible(true);

		const sdword x2 = 4;
		sdword y2 = 4;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
		gEditBox_MaddLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_MADD_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_MaddLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_MaddLayer_Amplitude->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Altitude:", &owner);
		gEditBox_MaddLayer_Altitude = helper.CreateEditBox(Params, EDIT_BOX_MADD_LAYER_ALTITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_MaddLayer_Altitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_MaddLayer_Altitude->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;
		//Params->SetVisible(true);

		const sdword x2 = 4;
		sdword y2 = 4;

//		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
//		gEditBox_PowerLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_POWER_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_PowerLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
//		gEditBox_PowerLayer_Amplitude->SetUserData(tte);
//		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Factor:", &owner);
		gEditBox_PowerLayer_Factor = helper.CreateEditBox(Params, EDIT_BOX_POWER_LAYER_FACTOR, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_PowerLayer_Factor), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_PowerLayer_Factor->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;
		//Params->SetVisible(true);

		const sdword x2 = 4;
		sdword y2 = 4;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Height:", &owner);
		gEditBox_ClampLayer_Height = helper.CreateEditBox(Params, EDIT_BOX_CLAMP_LAYER_HEIGHT, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_ClampLayer_Height), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_ClampLayer_Height ->SetUserData(tte);
		y2 += YStep;

		gCheckBox_ClampLayer_Bool = helper.CreateCheckBox(Params, CHECK_BOX_CLAMP_LAYER_BOOL, x2+OffsetX, y2, EditBoxWidth, 20, "Bool:", &owner, gDefault_ClampLayer_Bool, gLayers_CheckBox_CB, null);
		gCheckBox_ClampLayer_Bool->SetUserData(tte);
		y2 += YStep;
	}

#ifdef OBSOLETE
	{
		IceWindow* Params = gLayers[Index++].mParams;
		//Params->SetVisible(true);

		const sdword x2 = 4;
		sdword y2 = 4;
		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Height:", &owner);
		gEditBox_ConstantLayer_Height = helper.CreateEditBox(Params, EDIT_BOX_CONSTANT_LAYER_HEIGHT, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_ConstantLayer_Height), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_ConstantLayer_Height->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;
		//Params->SetVisible(true);

		const sdword x2 = 4;
		sdword y2 = 4;
		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Scale:", &owner);
		gEditBox_ScaleLayer_Scale = helper.CreateEditBox(Params, EDIT_BOX_SCALE_LAYER_SCALE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_ScaleLayer_Scale), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_ScaleLayer_Scale->SetUserData(tte);
		y2 += YStep;
	}
#endif

	{
		IceWindow* Params = gLayers[Index++].mParams;

		const sdword x2 = 4;
		sdword y2 = 4;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
		gEditBox_SinusLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_SINUS_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_SinusLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_SinusLayer_Amplitude->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "X freq:", &owner);
		gEditBox_SinusLayer_XFreq = helper.CreateEditBox(Params, EDIT_BOX_SINUS_LAYER_XFREQ, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_SinusLayer_XFreq), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_SinusLayer_XFreq->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Y freq:", &owner);
		gEditBox_SinusLayer_YFreq = helper.CreateEditBox(Params, EDIT_BOX_SINUS_LAYER_YFREQ, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_SinusLayer_YFreq), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_SinusLayer_YFreq->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase X:", &owner);
		gEditBox_SinusLayer_PhaseX = helper.CreateEditBox(Params, EDIT_BOX_SINUS_LAYER_PHASEX, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_SinusLayer_PhaseX), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_SinusLayer_PhaseX->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase Y:", &owner);
		gEditBox_SinusLayer_PhaseY = helper.CreateEditBox(Params, EDIT_BOX_SINUS_LAYER_PHASEY, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_SinusLayer_PhaseY), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_SinusLayer_PhaseY->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;

		const sdword x2 = 4;
		sdword y2 = 4;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
		gEditBox_fbmLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_Amplitude->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "X freq:", &owner);
		gEditBox_fbmLayer_XFreq = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_XFREQ, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_XFreq), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_XFreq->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Y freq:", &owner);
		gEditBox_fbmLayer_YFreq = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_YFREQ, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_YFreq), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_YFreq->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase X:", &owner);
		gEditBox_fbmLayer_PhaseX = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_PHASEX, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_PhaseX), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_PhaseX->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase Y:", &owner);
		gEditBox_fbmLayer_PhaseY = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_PHASEY, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_PhaseY), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_PhaseY->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Z:", &owner);
		gEditBox_fbmLayer_Z = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_Z, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_Z), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_Z->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Fractal inc:", &owner);
		gEditBox_fbmLayer_FractalInc = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_FRACTAL_INC, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_FractalInc), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_FractalInc->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Lacunarity:", &owner);
		gEditBox_fbmLayer_Lacunarity = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_LACUNARITY, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_Lacunarity), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_Lacunarity->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Octaves:", &owner);
		gEditBox_fbmLayer_Octaves = helper.CreateEditBox(Params, EDIT_BOX_FBM_LAYER_OCTAVES, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_fbmLayer_Octaves), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_fbmLayer_Octaves->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;

		const sdword x2 = 4;
		sdword y2 = 4;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
		gEditBox_TurbLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_TURB_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_TurbLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_TurbLayer_Amplitude->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "X freq:", &owner);
		gEditBox_TurbLayer_XFreq = helper.CreateEditBox(Params, EDIT_BOX_TURB_LAYER_XFREQ, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_TurbLayer_XFreq), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_TurbLayer_XFreq->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Y freq:", &owner);
		gEditBox_TurbLayer_YFreq = helper.CreateEditBox(Params, EDIT_BOX_TURB_LAYER_YFREQ, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_TurbLayer_YFreq), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_TurbLayer_YFreq->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase X:", &owner);
		gEditBox_TurbLayer_PhaseX = helper.CreateEditBox(Params, EDIT_BOX_TURB_LAYER_PHASEX, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_TurbLayer_PhaseX), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_TurbLayer_PhaseX->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase Y:", &owner);
		gEditBox_TurbLayer_PhaseY = helper.CreateEditBox(Params, EDIT_BOX_TURB_LAYER_PHASEY, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_TurbLayer_PhaseY), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_TurbLayer_PhaseY->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Z:", &owner);
		gEditBox_TurbLayer_Z = helper.CreateEditBox(Params, EDIT_BOX_TURB_LAYER_Z, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_TurbLayer_Z), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_TurbLayer_Z->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Octaves:", &owner);
		gEditBox_TurbLayer_Octaves = helper.CreateEditBox(Params, EDIT_BOX_TURB_LAYER_OCTAVES, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_TurbLayer_Octaves), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_TurbLayer_Octaves->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;

		const sdword x2 = 4;
		sdword y2 = 4;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
		gEditBox_RidgedLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_Amplitude->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "X freq:", &owner);
		gEditBox_RidgedLayer_XFreq = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_XFREQ, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_XFreq), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_XFreq->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Y freq:", &owner);
		gEditBox_RidgedLayer_YFreq = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_YFREQ, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_YFreq), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_YFreq->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase X:", &owner);
		gEditBox_RidgedLayer_PhaseX = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_PHASEX, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_PhaseX), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_PhaseX->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase Y:", &owner);
		gEditBox_RidgedLayer_PhaseY = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_PHASEY, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_PhaseY), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_PhaseY->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Z:", &owner);
		gEditBox_RidgedLayer_Z = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_Z, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_Z), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_Z->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Fractal inc:", &owner);
		gEditBox_RidgedLayer_FractalInc = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_FRACTAL_INC, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_FractalInc), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_FractalInc->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Lacunarity:", &owner);
		gEditBox_RidgedLayer_Lacunarity = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_LACUNARITY, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_Lacunarity), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_Lacunarity->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Octaves:", &owner);
		gEditBox_RidgedLayer_Octaves = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_OCTAVES, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_Octaves), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_Octaves->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Offset:", &owner);
		gEditBox_RidgedLayer_Offset = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_OFFSET, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_Offset), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_Offset->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Gain:", &owner);
		gEditBox_RidgedLayer_Gain = helper.CreateEditBox(Params, EDIT_BOX_RIDGED_LAYER_GAIN, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_RidgedLayer_Gain), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_RidgedLayer_Gain->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;

		const sdword x2 = 4;
		sdword y2 = 4;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
		gEditBox_CellularLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_CELLULAR_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_CellularLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_CellularLayer_Amplitude->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase X:", &owner);
		gEditBox_CellularLayer_PhaseX = helper.CreateEditBox(Params, EDIT_BOX_CELLULAR_LAYER_PHASEX, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_CellularLayer_PhaseX), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_CellularLayer_PhaseX->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Phase Y:", &owner);
		gEditBox_CellularLayer_PhaseY = helper.CreateEditBox(Params, EDIT_BOX_CELLULAR_LAYER_PHASEY, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_CellularLayer_PhaseY), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_CellularLayer_PhaseY->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Scale X:", &owner);
		gEditBox_CellularLayer_ScaleX = helper.CreateEditBox(Params, EDIT_BOX_CELLULAR_LAYER_SCALEX, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_CellularLayer_ScaleX), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_CellularLayer_ScaleX->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Scale Y:", &owner);
		gEditBox_CellularLayer_ScaleY = helper.CreateEditBox(Params, EDIT_BOX_CELLULAR_LAYER_SCALEY, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_CellularLayer_ScaleY), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_CellularLayer_ScaleY->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Z:", &owner);
		gEditBox_CellularLayer_Z = helper.CreateEditBox(Params, EDIT_BOX_CELLULAR_LAYER_Z, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_CellularLayer_Z), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_CellularLayer_Z->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;

		const sdword x2 = 4;
		sdword y2 = 4;

//		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
//		gEditBox_MidpointLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_MIDPOINT_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_MidpointLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
//		gEditBox_MidpointLayer_Amplitude->SetUserData(tte);
//		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Displacement:", &owner);
		gEditBox_MidpointLayer_Disp = helper.CreateEditBox(Params, EDIT_BOX_MIDPOINT_LAYER_DISP, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_MidpointLayer_Disp), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_MidpointLayer_Disp->SetUserData(tte);
		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Smoothness:", &owner);
		gEditBox_MidpointLayer_Smoothness = helper.CreateEditBox(Params, EDIT_BOX_MIDPOINT_LAYER_SMOOTHNESS, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_MidpointLayer_Smoothness), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
		gEditBox_MidpointLayer_Smoothness->SetUserData(tte);
		y2 += YStep;
	}

	{
		IceWindow* Params = gLayers[Index++].mParams;

		const sdword x2 = 4;
		sdword y2 = 4;

//		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Amplitude:", &owner);
//		gEditBox_CraterLayer_Amplitude = helper.CreateEditBox(Params, EDIT_BOX_CRATER_LAYER_AMPLITUDE, x2+OffsetX, y2, EditBoxWidth, 20, helper.Convert(gDefault_CraterLayer_Amplitude), &owner, EDITBOX_FLOAT, gLayers_EditBox_CB, null);
//		gEditBox_CraterLayer_Amplitude->SetUserData(tte);
//		y2 += YStep;

		helper.CreateLabel(Params, x2, y2+LabelOffsetY, 100, 20, "Nb craters:", &owner);
		gEditBox_CraterLayer_Nb = helper.CreateEditBox(Params, EDIT_BOX_CRATER_LAYER_NB, x2+OffsetX, y2, EditBoxWidth, 20, _F("%d", gDefault_CraterLayer_Nb), &owner, EDITBOX_INTEGER_POSITIVE, gLayers_EditBox_CB, null);
		gEditBox_CraterLayer_Nb->SetUserData(tte);
		y2 += YStep;
	}
}

static void CreateTextureTab(PintGUIHelper& helper, IceWindow* tab, Widgets& owner, ToolTerrainEditor* tte)
{
	const sdword x = 4;
	sdword y = 4;
	const sdword OffsetX = 100;
	const sdword EditBoxWidth = 60;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;

	struct Callbacks
	{
		// ### TODO: refactor this code with UpdateTexture function
		static void CreateTexture(IceButton& button, void* user_data)
		{
			ToolTerrainEditor* tte = reinterpret_cast<ToolTerrainEditor*>(user_data);
			//tte->mPendingAction = ToolTerrainEditor::CREATE_TEXTURE;

			if(tte->mSelectedTerrain)
			{
				const TerrainData* TD = tte->mSelectedTerrain;

				Heightfield HF(TD->mNbX, TD->mNbY);
				TD->Evaluate(HF);

				TERRAINTEXTURECREATE ttc;
				ttc.mField			= HF.GetHeights();
				ttc.mWidth			= TD->mNbX;
				ttc.mHeight			= TD->mNbY;
				ttc.mRenderWater	= false;
				ttc.mSeaLevel		= 0.0f;

				TerrainTexture TT;
				TT.Create(ttc);

#ifdef SUPPORT_TERRAIN_TEXTURE
				if(TD->mTexture)
				{
					GLTexture::releaseTexture(TD->mTexture->mGLID);
					TD->mTexture->mGLID = GLTexture::createTexture(TT.GetWidth(), TT.GetHeight(), TT.GetPixels(), true);
				}
				else
#endif
				{
					const ManagedTexture* MT = CreateManagedTexture(TT.GetWidth(), TT.GetHeight(), TT.GetPixels(), "TerrainTexture");
				}
				//GLTexture::createTexture(TT.GetWidth(), TT.GetHeight(), TT.GetPixels(), true);
				//const ManagedTexture* MT = CreateManagedTexture(TT.GetWidth(), TT.GetHeight(), TT.GetPixels(), "TerrainTexture");
			}
		}

/*		static void gEditBox(const IceEditBox& edit_box, udword param, void* user_data)
		{
			const udword ID = edit_box.GetID();
			if(ID==EDIT_BOX_TILE_NB)
				gTileNb = GetInt(gTileNb, &edit_box);
			else if(ID==EDIT_BOX_TILE_SIZE)
				gTileSize = GetInt(gTileSize, &edit_box);
			else if(ID==EDIT_BOX_TILE_SCALE)
				gTileScale = GetFloat(gTileScale, &edit_box);
		}*/

		static void gCheckBox_CB(const IceCheckBox& check_box, bool checked, void* user_data)
		{
			ToolTerrainEditor* tte = reinterpret_cast<ToolTerrainEditor*>(user_data);
			ASSERT(tte);

			const udword ID = check_box.GetID();
			switch(ID)
			{
				case CHECK_BOX_ENABLE_TEXTURE:
				{
					gTexture_Enabled = check_box.IsChecked();
				}
				break;

				case CHECK_BOX_AUTO_TEXTURE_UPDATE:
				{
					gTexture_AutoUpdate = check_box.IsChecked();
				}
				break;
			}
		}

	};

//	y += YStep;
	y += 10;

/*	helper.CreateLabel(tab, x, y+LabelOffsetY, 100, 20, "Tile nb:", &owner);
	helper.CreateEditBox(tab, EDIT_BOX_TILE_NB, x+OffsetX, y, EditBoxWidth, 20, _F("%d", gTileNb), &owner, EDITBOX_INTEGER_POSITIVE, Callbacks::gEditBox, null);
	y += YStep;

	helper.CreateLabel(tab, x, y+LabelOffsetY, 100, 20, "Tile size:", &owner);
	helper.CreateEditBox(tab, EDIT_BOX_TILE_SIZE, x+OffsetX, y, EditBoxWidth, 20, _F("%d", gTileSize), &owner, EDITBOX_INTEGER_POSITIVE, Callbacks::gEditBox, null);
	y += YStep;

	helper.CreateLabel(tab, x, y+LabelOffsetY, 100, 20, "Tile scale:", &owner);
	helper.CreateEditBox(tab, EDIT_BOX_TILE_SCALE, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gTileScale), &owner, EDITBOX_FLOAT_POSITIVE, Callbacks::gEditBox, null);
	y += YStep;*/

//	IceButton* B = helper.CreateButton(parent, 0, x, y, 100, 20, "Button", &owner, Local::ButtonCB, null);
	{
		ButtonDesc BD;
		BD.mParent	= tab;
		BD.mX		= x;
		BD.mY		= y;
		BD.mWidth	= 150;
		BD.mHeight	= 20;
		BD.mStyle	= BUTTON_NORMAL;
		BD.mLabel	= "Create texture";
		IceButton* B = ICE_NEW(IceButton)(BD);
		B->SetCallback(Callbacks::CreateTexture);
		B->SetUserData(tte);
		owner.Register(B);
	}

	if(0)
	{
		gCheckBox_EnableTexture = helper.CreateCheckBox(tab, CHECK_BOX_ENABLE_TEXTURE, x, y, EditBoxWidth, 20, "Enable:", &owner, gTexture_Enabled, Callbacks::gCheckBox_CB, null);
		gCheckBox_EnableTexture->SetUserData(tte);
		y += YStep;

		gCheckBox_AutoTextureUpdate = helper.CreateCheckBox(tab, CHECK_BOX_AUTO_TEXTURE_UPDATE, x, y, EditBoxWidth, 20, "Auto update:", &owner, gTexture_AutoUpdate, Callbacks::gCheckBox_CB, null);
		gCheckBox_AutoTextureUpdate->SetUserData(tte);
		y += YStep;
	}
}

void ToolTerrainEditor::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword x = 4;
	sdword y = 4;

	if(1)
	{
		IceWindow* Tabs[TAB_COUNT];

		TabControlDesc TCD;
		TCD.mParent	= parent;
		TCD.mX		= 4;
		TCD.mY		= y;
		TCD.mWidth	= 460;
		TCD.mHeight	= 360;
		//TCD.mStyle	= WSTYLE_BORDER;
		IceTabControl* TabControl = ICE_NEW(IceTabControl)(TCD);
		owner.Register(TabControl);

		for(udword i=0;i<TAB_COUNT;i++)
		{
			WindowDesc WD;
			WD.mParent	= parent;
			WD.mX		= 0;
			WD.mY		= 0;
			WD.mWidth	= 410;
			WD.mHeight	= 350;
			WD.mLabel	= "Tab";
			WD.mType	= WINDOW_DIALOG;
			//WD.mStyle	= WSTYLE_BORDER;
			//WD.mStyle	= WSTYLE_CLIENT_EDGES;
			IceWindow* Tab = ICE_NEW(IceWindow)(WD);
			owner.Register(Tab);
			Tab->SetVisible(false);
			Tabs[i] = Tab;
		}
		TabControl->Add(Tabs[TAB_TILES], "Tiles");
		TabControl->Add(Tabs[TAB_LAYERS], "Layers");
		TabControl->Add(Tabs[TAB_TEXTURE], "Texture");

		CreateTilesTab(helper, Tabs[TAB_TILES], owner, this);
		CreateLayersTab(helper, Tabs[TAB_LAYERS], owner, this);
		CreateTextureTab(helper, Tabs[TAB_TEXTURE], owner, this);
	}

	UpdateUI();
}
