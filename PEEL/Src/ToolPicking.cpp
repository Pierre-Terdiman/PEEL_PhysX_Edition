///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Tool.h"
#include "ToolPicking.h"
#include "GLRenderHelpers.h"
#include "Camera.h"
#include "PintObjectsManager.h"

#include "MeshCleaner.h"
#include "PintShapeRenderer.h"

///////////////////////////////////////////////////////////////////////////////

static float gPickingForce = 1.0f;
static float gLinearDamping = 5.0f;
static float gAngularDamping = 5.0f;
static bool gDrawTouchedTriangle = false;
//static bool gDebugMode = false;
static float gDebugRadius = 0.0f;

ToolPicking::ToolPicking() : mIsControllingCamera(false), mDebugPint(null)
{
}

ToolPicking::~ToolPicking()
{
}

void ToolPicking::Reset(udword pint_index)
{
	if(pint_index==INVALID_ID)
	{
		for(udword i=0;i<MAX_NB_ENGINES;i++)
		{
			mPickData[i].Reset();
			mTrackedData[i].Reset();
		}
	}
	else
	{
		ASSERT(pint_index<MAX_NB_ENGINES)
		mPickData[pint_index].Reset();
		mTrackedData[pint_index].Reset();
	}
}

void ToolPicking::OnObjectReleased(Pint& pint, PintActorHandle removed_object)
{
	for(udword i=0;i<MAX_NB_ENGINES;i++)
	{
		PickingData& PD = mPickData[i];
		if(PD.mPickedObject==removed_object)
		{
			PD.Reset();
			//break;
		}

		TrackingData& TD = mTrackedData[i];
		if(TD.mTrackedObject==removed_object)
		{
			TD.Reset();
			//break;
		}
	}
}

void ToolPicking::KeyboardCallback(Pint& pint, udword pint_index, unsigned char key, bool down)
{
	if(down)
	{
		if(key==127)
		{
			ASSERT(pint_index<MAX_NB_ENGINES)
			PickingData& PD = mPickData[pint_index];

			if(PD.mPickedObject)
			{
				ReleasePintObject(pint, PD.mPickedObject, true);
				PD.Reset();
			}
		}
	}
}

static bool FindTouchedTriangle(Triangle& touched_tri, Pint& pint, const PintRaycastHit& hit)
{
	if(hit.mTouchedShape)
	{
		Pint_Shape* ShapeAPI = pint.GetShapeAPI();
		if(ShapeAPI)
		{
			const PintShape Type = ShapeAPI->GetType(hit.mTouchedShape);
			if(Type==PINT_SHAPE_MESH || Type==PINT_SHAPE_MESH2)
				return ShapeAPI->GetTriangle(touched_tri, hit.mTouchedShape, hit.mTriangleIndex);
		}
	}
	return false;
}

void ToolPicking::RightDownCallback(Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	PickingData& PD = mPickData[pint_index];

	PintRaycastHit Hit;
	if(Raycast(pint, Hit, mOrigin, mDir) && !IsDefaultEnv(pint, Hit.mTouchedActor))
	{
		printf("Picked object: 0x%x (%s)\n", size_t(Hit.mTouchedActor), pint.GetName());

		Pint_Actor* API = pint.GetActorAPI();
		if(API)
		{
			const char* Name = API->GetName(Hit.mTouchedActor);
			if(Name)
				printf("  Name: %s\n", Name);

			const float Mass = API->GetMass(Hit.mTouchedActor);
			printf("  Mass: %f\n", Mass);

			Point LocalInertia;
			API->GetLocalInertia(Hit.mTouchedActor, LocalInertia);
			printf("  Local inertia: %f | %f | %f\n", LocalInertia.x, LocalInertia.y, LocalInertia.z);
		}

		PD.mDragPoint = Hit.mImpact;
		printf("  Impact point: %f | %f | %f\n", Hit.mImpact.x, Hit.mImpact.y, Hit.mImpact.z);

		const PR WorldTransform = pint.GetWorldTransform(Hit.mTouchedActor);

/*		// TODO: rewrite this clumsy stuff
			const Matrix4x4 M = WorldTransform;
			Matrix4x4 InvM;
			InvertPRMatrix(InvM, M);

		PD.mLocalPoint = Hit.mImpact * InvM;*/
		PD.mLocalPoint = ComputeLocalPoint(Hit.mImpact, WorldTransform);
/*#ifdef _DEBUG
		const Point NewWorldPt = PD.mLocalPoint * M;
		int stop=1;
#endif*/
		PD.mPickedObject = Hit.mTouchedActor;
		PD.mDistance = Hit.mDistance;
		PD.mImpact = Hit.mImpact;
		if(API)
		{
			PD.mLinearDamping = API->GetLinearDamping(Hit.mTouchedActor);
			PD.mAngularDamping = API->GetAngularDamping(Hit.mTouchedActor);
			API->SetLinearDamping(PD.mPickedObject, gLinearDamping);
			API->SetAngularDamping(PD.mPickedObject, gAngularDamping);
		}

		if(gDrawTouchedTriangle)
		{
			PD.mTouchedTriangle.mVerts[0].Zero();
			PD.mTouchedTriangle.mVerts[1].Zero();
			PD.mTouchedTriangle.mVerts[2].Zero();

			Triangle Tri;
			if(FindTouchedTriangle(Tri, pint, Hit))
				PD.mTouchedTriangle = Tri;
		}

		//if(gDebugMode)
		if(gDebugRadius>0.0f)
		{
			Triangle Tri;
			if(FindTouchedTriangle(Tri, pint, Hit))
			{
				PintSphereOverlapData Data;
				Tri.Center(Data.mSphere.mCenter);
				//Data.mSphere.mRadius = Tri.MaxEdgeLength()*2.0f;
				Data.mSphere.mRadius = gDebugRadius;
				mSphere = Data.mSphere;

				mHits.SetNoHit();
				mStream.Reset();
				pint.FindTriangles_MeshSphereOverlap(pint.mSQHelper->GetThreadContext(), Hit.mTouchedActor, 1, &mHits, mStream, &Data);
			}
		}
	}
	else
	{
		PD.Reset();
	}
}

void ToolPicking::RightDragCallback(Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	PickingData& PD = mPickData[pint_index];
	if(PD.mPickedObject)
	{
		const Point NewPos = mOrigin + mDir * PD.mDistance;
		PD.mDragPoint = NewPos;
	}
}

void ToolPicking::RightUpCallback(Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	PickingData& PD = mPickData[pint_index];
	if(PD.mPickedObject)
	{
		Pint_Actor* API = pint.GetActorAPI();
		if(API)
		{
			API->SetLinearDamping(PD.mPickedObject, PD.mLinearDamping);
			API->SetAngularDamping(PD.mPickedObject, PD.mAngularDamping);
		}
	}
	mPickData[pint_index].Reset();
}

void ToolPicking::RightDblClkCallback(Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)
	TrackingData& TD = mTrackedData[pint_index];

	PintRaycastHit Hit;
	if(Raycast(pint, Hit, mOrigin, mDir) && !IsDefaultEnv(pint, Hit.mTouchedActor))
	{
		printf("Picked object: %d\n", size_t(Hit.mTouchedActor));
		if(Hit.mTouchedActor==TD.mTrackedObject)
			TD.mTrackedObject = null;
		else
			TD.mTrackedObject = Hit.mTouchedActor;
//		gTrackedEngine = gEngines[i].mEngine;
	}
	else
	{
		TD.mTrackedObject = null;
//		gTrackedEngine = null;
	}
}

void ToolPicking::PreRenderCallback()
{
	mIsControllingCamera = false;
}

void ToolPicking::RenderCallback(PintRender& render, Pint& pint, udword pint_index)
{
	ASSERT(pint_index<MAX_NB_ENGINES)

	TrackingData& TD = mTrackedData[pint_index];
//	if(gTrackedObject/* && gTrackedEngine*/)
	if(TD.mTrackedObject/* && gTrackedEngine*/)
	{
		const PR Pose = pint.GetWorldTransform(TD.mTrackedObject);
		const Point CamPos = GetCameraPos();
		const Point Dir = (Pose.mPos - CamPos).Normalize();
		SetCamera(CamPos, Dir);
		mIsControllingCamera = true;
	}

	PickingData& PD = mPickData[pint_index];
	if(PD.mPickedObject)
	{
		const Point& Pt = PD.mImpact;
//		DrawFrame(Pt);

//		DrawLine(Pt, gEngines[i].mDragPoint, gEngines[i].mEngine->GetMainColor(), 1.0f);

		{
			const PR WorldTransform = pint.GetWorldTransform(PD.mPickedObject);

			// TODO: we need a dt here

			// TODO: rewrite this clumsy stuff
				const Matrix4x4 M = WorldTransform;
				const Point NewWorldPt = PD.mLocalPoint * M;

				GLRenderHelpers::DrawFrame(NewWorldPt, GetFrameSize());
				//GLRenderHelpers::DrawFatFrame(NewWorldPt, GetFrameSize());
				GLRenderHelpers::DrawLine(NewWorldPt, PD.mDragPoint, pint.GetMainColor());
				if(gDrawTouchedTriangle)
				{
					GLRenderHelpers::DrawTriangle(PD.mTouchedTriangle.mVerts[0], PD.mTouchedTriangle.mVerts[1], PD.mTouchedTriangle.mVerts[2], Point(1.0f, 1.0f, 0.0f));
					GLRenderHelpers::DrawTriangle(PD.mTouchedTriangle.mVerts[0], PD.mTouchedTriangle.mVerts[2], PD.mTouchedTriangle.mVerts[1], Point(1.0f, 1.0f, 0.0f));
				}

				float Mass = 1.0f;
				Pint_Actor* API = pint.GetActorAPI();
				if(API)
					Mass = API->GetMass(PD.mPickedObject);

				const Point action = gPickingForce*Mass*(PD.mDragPoint - NewWorldPt);

				pint.AddWorldImpulseAtWorldPos(PD.mPickedObject, action, NewWorldPt);
				//pint.GetActorAPI()->WakeUp(PD.mPickedObject);
		}

		DrawBoxCorners(pint, PD.mPickedObject, TOOLS_BOX_CORNERS_COLOR);

		if(gDebugRadius>0.0f)
		//if(gDebugMode)
		{
			render.DrawWireframeSphere(mSphere, Point(1.0f, 1.0f, 1.0f));

			const PintActorHandle Handle = PD.mPickedObject;
			const PR WorldPose = pint.GetWorldTransform(Handle);
			mDebugMeshPose = WorldPose;
			const Matrix4x4 GlobalPose = WorldPose;

			const Point Color(1.0f, 1.0f, 1.0f);
			Pint_Actor* ActorAPI = pint.GetActorAPI();
			if(ActorAPI)
			{
				PintShapeHandle ShapeHandle = ActorAPI->GetShape(Handle, 0);	// ####

				Pint_Shape* ShapeAPI = pint.GetShapeAPI();
				if(ShapeAPI)
				{
					Triangle tri;
					const udword NbHits = mHits.mNbHits;
					mDebugMeshVertices.Reset();
					mDebugPint = &pint;

					for(udword i=0;i<NbHits;i++)
					{
						bool Status = ShapeAPI->GetTriangle(tri, ShapeHandle, mStream[i]);

						mDebugMeshVertices.AddVertex(tri.mVerts[0]).AddVertex(tri.mVerts[1]).AddVertex(tri.mVerts[2]);

						const Point v0 = tri.mVerts[0] * GlobalPose;
						const Point v1 = tri.mVerts[1] * GlobalPose;
						const Point v2 = tri.mVerts[2] * GlobalPose;

						render.DrawLine(v0, v1, Color);
						render.DrawLine(v1, v2, Color);
						render.DrawLine(v2, v0, Color);
					}
				}
			}
		}
	}
}

void ToolPicking::CreateDebugMesh()
{
	if(!mDebugPint)
		return;
	const udword NbVertices = mDebugMeshVertices.GetNbVertices();
	const udword NbTris = NbVertices/3;

	IndexedSurface IS;
	const Point* V = mDebugMeshVertices.GetVertices();
	IS.Init(NbTris, NbTris*3, V, null, false);
	IS.CreateDefaultTopology();

	//const float WeldTolerance = 0.05f;
	//const float WeldTolerance = 0.1f;
	MeshCleaner Cleaner(IS.GetNbVerts(), IS.GetVerts(), IS.GetNbFaces(), IS.GetFaces()->mRef/*, WeldTolerance*/);
	IS.Init(Cleaner.mNbTris, Cleaner.mNbVerts, Cleaner.mVerts, (const IndexedTriangle*)Cleaner.mIndices);

	const udword NewNbTris = IS.GetNbFaces();
	const Point* NewVertices = IS.GetVerts();
	const IndexedTriangle* NewTris = IS.GetFaces();

	PINT_MESH_CREATE MeshCreate;
	MeshCreate.SetSurfaceData(IS.GetSurfaceInterface());
	MeshCreate.mRenderer = CreateMeshRenderer(MeshCreate.GetSurface());

	PINT_OBJECT_CREATE ObjectDesc(&MeshCreate);
	ObjectDesc.mMass		= 0.0f;
	ObjectDesc.mPosition	= mDebugMeshPose.mPos;
	ObjectDesc.mRotation	= mDebugMeshPose.mRot;
	CreatePintObject(*mDebugPint, ObjectDesc);
}

bool ToolPicking::IsControllingCamera() const
{
	return mIsControllingCamera;
}

///////////////////////////////////////////////////////////////////////////////

#include "GUI_Helpers.h"

static const char* gTooltip_PickingForce = "Coeff multiplier for picking force";
static const char* gTooltip_LinearDamping = "Linear damping applied to picked object (while picking)";
static const char* gTooltip_AngularDamping = "Angular damping applied to picked object (while picking)";

void ToolPicking::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword OffsetX = 90;
	const sdword EditBoxWidth = 60;
	const sdword ButtonWidth = 120;
	const sdword CheckBoxWidth = 140;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	const sdword x = 4;

	struct Local
	{
		static void EditBoxCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			const udword ID = edit_box.GetID();
			if(ID==0)
				gPickingForce = GetFloat(gPickingForce, &edit_box);
			else if(ID==1)
				gLinearDamping = GetFloat(gLinearDamping, &edit_box);
			else if(ID==2)
				gAngularDamping = GetFloat(gAngularDamping, &edit_box);
			else if(ID==3)
				gDebugRadius = GetFloat(gDebugRadius, &edit_box);
		}

		static void CheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
		{
			if(check_box.GetID()==0)
				gDrawTouchedTriangle = checked;
//			else if(check_box.GetID()==1)
//				gDebugMode = checked;
		}

		static void ButtonCallback(IceButton& button, void* user_data)
		{
			((ToolPicking*)user_data)->CreateDebugMesh();
		}
	};

	sdword y = 0;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Picking force:", &owner);
	helper.CreateEditBox(parent, 0, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gPickingForce), &owner, EDITBOX_FLOAT_POSITIVE, Local::EditBoxCallback, gTooltip_PickingForce);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Linear damping:", &owner);
	helper.CreateEditBox(parent, 1, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gLinearDamping), &owner, EDITBOX_FLOAT_POSITIVE, Local::EditBoxCallback, gTooltip_LinearDamping);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Angular damping:", &owner);
	helper.CreateEditBox(parent, 2, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gAngularDamping), &owner, EDITBOX_FLOAT_POSITIVE, Local::EditBoxCallback, gTooltip_AngularDamping);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Debug radius:", &owner);
	helper.CreateEditBox(parent, 3, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gDebugRadius), &owner, EDITBOX_FLOAT_POSITIVE, Local::EditBoxCallback/*, gTooltip_AngularDamping*/);
	y += YStep;

	helper.CreateCheckBox(parent, 0, x, y, CheckBoxWidth, 20, "Draw touched triangle", &owner, gDrawTouchedTriangle, Local::CheckBoxCallback/*, gTooltip_AngularDamping*/);
	y += YStep;

//	helper.CreateCheckBox(parent, 1, x, y, CheckBoxWidth, 20, "Debug mode", &owner, gDebugMode, Local::CheckBoxCallback/*, gTooltip_AngularDamping*/);
//	y += YStep;

	IceButton* b = helper.CreateButton(parent, 0, x, y, ButtonWidth, 20, "Create debug mesh", &owner, Local::ButtonCallback, this/*, const char* tooltip*/);
	y += YStep;
}
