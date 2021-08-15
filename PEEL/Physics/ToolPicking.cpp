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

///////////////////////////////////////////////////////////////////////////////

static float gPickingForce = 1.0f;
static float gLinearDamping = 5.0f;
static float gAngularDamping = 5.0f;

ToolPicking::ToolPicking() : mIsControllingCamera(false)
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

		if(0)
		{
			PD.mTouchedTriangle.mVerts[0].Zero();
			PD.mTouchedTriangle.mVerts[1].Zero();
			PD.mTouchedTriangle.mVerts[2].Zero();
			if(Hit.mTouchedShape)
			{
				Pint_Shape* ShapeAPI = pint.GetShapeAPI();
				if(ShapeAPI)
				{
					const PintShape Type = ShapeAPI->GetType(Hit.mTouchedShape);
					if(Type==PINT_SHAPE_MESH || Type==PINT_SHAPE_MESH2)
					{
						Triangle Tri;
						if(ShapeAPI->GetTriangle(Tri, Hit.mTouchedShape, Hit.mTriangleIndex))
						{
							PD.mTouchedTriangle = Tri;
						}
					}
				}
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

void ToolPicking::RenderCallback(Pint& pint, udword pint_index)
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
				GLRenderHelpers::DrawLine(NewWorldPt, PD.mDragPoint, pint.GetMainColor());
				if(0)
					GLRenderHelpers::DrawTriangle(PD.mTouchedTriangle.mVerts[0], PD.mTouchedTriangle.mVerts[1], PD.mTouchedTriangle.mVerts[2], Point(1.0f, 1.0f, 0.0f));

				float Mass = 1.0f;
				Pint_Actor* API = pint.GetActorAPI();
				if(API)
					Mass = API->GetMass(PD.mPickedObject);

				const Point action = gPickingForce*Mass*(PD.mDragPoint - NewWorldPt);

				pint.AddWorldImpulseAtWorldPos(PD.mPickedObject, action, NewWorldPt);
				//pint.GetActorAPI()->WakeUp(PD.mPickedObject);
		}

		DrawBoxCorners(pint, PD.mPickedObject, TOOLS_BOX_CORNERS_COLOR);
	}
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
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	const sdword x = 4;

	struct Local
	{
		static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
		{
			const udword ID = edit_box.GetID();
			if(ID==0)
				gPickingForce = GetFloat(gPickingForce, &edit_box);
			else if(ID==1)
				gLinearDamping = GetFloat(gLinearDamping, &edit_box);
			else if(ID==2)
				gAngularDamping = GetFloat(gAngularDamping, &edit_box);
		}
	};

	sdword y = 0;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Picking force:", &owner);
	helper.CreateEditBox(parent, 0, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gPickingForce), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_PickingForce);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Linear damping:", &owner);
	helper.CreateEditBox(parent, 1, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gLinearDamping), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_LinearDamping);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 90, 20, "Angular damping:", &owner);
	helper.CreateEditBox(parent, 2, x+OffsetX, y, EditBoxWidth, 20, helper.Convert(gAngularDamping), &owner, EDITBOX_FLOAT_POSITIVE, Local::gEBCallback, gTooltip_AngularDamping);
	y += YStep;
}
