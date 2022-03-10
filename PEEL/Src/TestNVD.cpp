///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestNVD.h"

#include "PINT_Editor.h"
#include "..\PINT_Common\PINT_Common.h"
#include "USDExport.h"
#include "ZB2Export.h"
#include "PintShapeRenderer.h"
#include "PEEL.h"

extern EditorPlugin* gEditor;
extern Pint* mEdited;

	#include "NVD_ControlInterface.h"
	#include "RenderModel.h"
	#include "NVD.h"
	#include "NVD_Render.h"
	#include "NVD_ClassManager.h"
	#include "NVD_TreeView.h"
	#include "PxTransform.h"
	using namespace physx;
	using namespace Nvd;
	#include "ICE_To_PX.h"

static const bool gUseRenderCaches = true;

static	IceTreeView*	gObjectTreeView = null;
extern	RenderModel*	gCurrentRenderModel;

static	bool	gDrawVisibleBounds			= false;
static	bool	gDrawGrid					= true;
static	bool	gUseObjectNamesInTreeView	= true;

/*static*/	bool	gDisplayInternalData		= true;
/*static*/ NVDControlInterface gNVDControlInterface;

static void RegisterNVDClasses()
{
	NvdCapture* Nvd = getNvdForCapture();
	if(!Nvd)
		return;

	if(0)
	{
		Nvd->createEnum("FakeEnum");
		Nvd->createEnumValue("FakeEnum", "FakeEnum_1", 1);
		Nvd->createEnumValue("FakeEnum", "FakeEnum_2", 2);
		Nvd->createEnumValue("FakeEnum", "FakeEnum_3", 3);
		Nvd->createEnumValue("FakeEnum", "FakeEnum_4", 4);

		Nvd->createFlags("FakeFlags");
		Nvd->createFlagValue("FakeFlags", "FakeFlag_1", 1<<0);
		Nvd->createFlagValue("FakeFlags", "FakeFlag_2", 1<<1);
		Nvd->createFlagValue("FakeFlags", "FakeFlag_3", 1<<2);
		Nvd->createFlagValue("FakeFlags", "FakeFlag_4", 1<<3);

		Nvd->createClass("VirtualObject");
//		Nvd->createProperty("VirtualObject", "BroadPhaseType", "FakeEnum");
		Nvd->createEnumProperty("VirtualObject", "BroadPhaseType", "FakeEnum");
		Nvd->createFlagsProperty("VirtualObject", "MyFlags", "FakeFlags");

		static void* VirtualObject = null;
		Nvd->createInstance(InstanceHandle(&VirtualObject), "VirtualObject");
		Nvd->setPropertyValue_int(InstanceHandle(&VirtualObject), "BroadPhaseType", 2);
		Nvd->setPropertyValue_int(InstanceHandle(&VirtualObject), "MyFlags", (1<<2)|(1<<3));
/*		Nvd->setPropertyValue_int(InstanceHandle(&VirtualObject), "Pipo", 42);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject), "LinearVelocity.x", 1.0f);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject), "LinearVelocity.y", 11111.0f);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject), "LinearVelocity.y", -9.81f);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject), "AngularVelocity.z", 0.2f);*/
	}

	if(0)
	{
		Nvd->createClass("PxVec3");
		Nvd->createProperty("PxVec3", "x", TYPE_FLOAT);
		Nvd->createProperty("PxVec3", "y", TYPE_FLOAT);
		Nvd->createProperty("PxVec3", "z", TYPE_FLOAT);

		Nvd->createClass("VirtualObject");
		Nvd->createProperty("VirtualObject", "LinearVelocity", "PxVec3");
		Nvd->createProperty("VirtualObject", "AngularVelocity", "PxVec3");
		Nvd->createProperty("VirtualObject", "Pipo", TYPE_INT);

		static void* VirtualObject = null;
		Nvd->createInstance(InstanceHandle(&VirtualObject), "VirtualObject");
		Nvd->setPropertyValue_int(InstanceHandle(&VirtualObject), "Pipo", 42);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject), "LinearVelocity.x", 1.0f);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject), "LinearVelocity.y", 11111.0f);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject), "LinearVelocity.y", -9.81f);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject), "AngularVelocity.z", 0.2f);
	}

	if(0)
	{
//		Nvd->createClass("PEEL");
//		Nvd->createClass("Pipo test");
		Nvd->createClass("VirtualObject");
		Nvd->createProperty("VirtualObject", "isSleeping", TYPE_BOOL);
		Nvd->createProperty("VirtualObject", "nb iter", TYPE_INT);
		Nvd->createProperty("VirtualObject", "offset", TYPE_FLOAT);
		Nvd->createProperty("VirtualObject", "name", TYPE_STRING);

		static void* VirtualObject0 = null;
		static void* VirtualObject1 = null;
		Nvd->createInstance(InstanceHandle(&VirtualObject0), "VirtualObject");
		Nvd->createInstance(InstanceHandle(&VirtualObject1), "VirtualObject");

		Nvd->setPropertyValue_bool(InstanceHandle(&VirtualObject0), "isSleeping", false);
		Nvd->setPropertyValue_int(InstanceHandle(&VirtualObject0), "nb iter", 4);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject0), "offset", 0.0f);
		Nvd->setPropertyValue_string(InstanceHandle(&VirtualObject0), "name", "pipo0");

		Nvd->setPropertyValue_bool(InstanceHandle(&VirtualObject1), "isSleeping", true);
		Nvd->setPropertyValue_int(InstanceHandle(&VirtualObject1), "nb iter", 8);
		Nvd->setPropertyValue_float(InstanceHandle(&VirtualObject1), "offset", 10.0f);
		Nvd->setPropertyValue_string(InstanceHandle(&VirtualObject1), "name", "pipo1");
	}

	Nvd->createClass("PEEL Scene", NVD_TYPE_SCENE);
	{
		Nvd->createProperty("PEEL Scene", "Name", TYPE_STRING);
		Nvd->createProperty("PEEL Scene", "Gravity", TYPE_VEC3);
		Nvd->createProperty("PEEL Scene", "NbSimulateCallsPerFrame", TYPE_INT);
		Nvd->createProperty("PEEL Scene", "Timestep", TYPE_FLOAT);
		Nvd->createProperty("PEEL Scene", "CreateDefaultEnvironment", TYPE_BOOL);
		Nvd->createProperty("PEEL Scene", "Materials", TYPE_GROUP);
		Nvd->createProperty("PEEL Scene", "Shapes", TYPE_GROUP);
		Nvd->createProperty("PEEL Scene", "Static actors", TYPE_GROUP);
		Nvd->createProperty("PEEL Scene", "Dynamic actors", TYPE_GROUP);
		Nvd->createProperty("PEEL Scene", "Aggregates", TYPE_GROUP);
		Nvd->createProperty("PEEL Scene", "Articulations", TYPE_GROUP);
		Nvd->createProperty("PEEL Scene", "Joints", TYPE_GROUP);
	}

	Nvd->createClass("Material");
	{
		Nvd->createProperty("Material", "Name", TYPE_STRING);
		Nvd->createProperty("Material", "StaticFriction", TYPE_FLOAT);
		Nvd->createProperty("Material", "DynamicFriction", TYPE_FLOAT);
		Nvd->createProperty("Material", "Restitution", TYPE_FLOAT);
	}

	Nvd->createClass("Actor", NVD_TYPE_ACTOR);
	{
		Nvd->createProperty("Actor", "Name", TYPE_STRING);
		Nvd->createProperty("Actor", "Mass", TYPE_FLOAT);
		Nvd->createProperty("Actor", "GlobalPose", TYPE_RENDER_TRANSFORM);
	}

	Nvd->createClass("Shape", NVD_TYPE_SHAPE);
	{
		Nvd->createEnum("ShapeType");
		{
			Nvd->createEnumValue("ShapeType", "PINT_SHAPE_UNDEFINED", PINT_SHAPE_UNDEFINED);
			Nvd->createEnumValue("ShapeType", "PINT_SHAPE_SPHERE", PINT_SHAPE_SPHERE);
			Nvd->createEnumValue("ShapeType", "PINT_SHAPE_CAPSULE", PINT_SHAPE_CAPSULE);
			Nvd->createEnumValue("ShapeType", "PINT_SHAPE_CYLINDER", PINT_SHAPE_CYLINDER);
			Nvd->createEnumValue("ShapeType", "PINT_SHAPE_BOX", PINT_SHAPE_BOX);
			Nvd->createEnumValue("ShapeType", "PINT_SHAPE_CONVEX", PINT_SHAPE_CONVEX);
			Nvd->createEnumValue("ShapeType", "PINT_SHAPE_MESH", PINT_SHAPE_MESH);
			Nvd->createEnumValue("ShapeType", "PINT_SHAPE_MESH2", PINT_SHAPE_MESH2);
		}

		Nvd->createProperty("Shape", "Name", TYPE_STRING);
		Nvd->createEnumProperty("Shape", "Type", "ShapeType");
		Nvd->createProperty("Shape", "LocalPose", TYPE_RENDER_TRANSFORM);
	}

	Nvd->createClass("TriangleMesh", NVD_TYPE_MESH);
	Nvd->createClass("ConvexMesh", NVD_TYPE_CONVEX);

	Nvd->createClass("Joint");
	{
		Nvd->createEnum("JointType");
		{
			Nvd->createEnumValue("JointType", "PINT_JOINT_UNDEFINED", PINT_JOINT_UNDEFINED);
			Nvd->createEnumValue("JointType", "PINT_JOINT_SPHERICAL", PINT_JOINT_SPHERICAL);
			Nvd->createEnumValue("JointType", "PINT_JOINT_HINGE", PINT_JOINT_HINGE);
			Nvd->createEnumValue("JointType", "PINT_JOINT_HINGE2", PINT_JOINT_HINGE2);
			Nvd->createEnumValue("JointType", "PINT_JOINT_PRISMATIC", PINT_JOINT_PRISMATIC);
			Nvd->createEnumValue("JointType", "PINT_JOINT_FIXED", PINT_JOINT_FIXED);
			Nvd->createEnumValue("JointType", "PINT_JOINT_DISTANCE", PINT_JOINT_DISTANCE);
			Nvd->createEnumValue("JointType", "PINT_JOINT_D6", PINT_JOINT_D6);
			Nvd->createEnumValue("JointType", "PINT_JOINT_GEAR", PINT_JOINT_GEAR);
			Nvd->createEnumValue("JointType", "PINT_JOINT_RACK_AND_PINION", PINT_JOINT_RACK_AND_PINION);
			Nvd->createEnumValue("JointType", "PINT_JOINT_PORTAL", PINT_JOINT_PORTAL);
		}

		Nvd->createProperty("Joint", "Name", TYPE_STRING);
//		Nvd->createProperty("Joint", "LocalPose0", TYPE_STRING);
		Nvd->createEnumProperty("Joint", "Type", "JointType");
//		Nvd->createProperty("Shape", "LocalPose", TYPE_RENDER_TRANSFORM);
	}

	Nvd->createClass("Aggregate");
	{
		Nvd->createProperty("Aggregate", "MaxSize", TYPE_INT);
		Nvd->createProperty("Aggregate", "SelfCollisions", TYPE_BOOL);
	}
}

TestNVD::TestNVD()
{
}

TestNVD::~TestNVD()
{
}

void TestNVD::Init()
{
	NvdCapture* Nvd = getNvd();
	if(Nvd)
		Nvd->open();
}

void TestNVD::Close()
{
	NvdCapture* Nvd = getNvd();
	if(Nvd)
		Nvd->close();
}

void TestNVD::Sync()
{
	NvdCapture* Nvd = getNvdForCapture();
	if(Nvd)
	{
		Nvd->sync();
	}
}

void TestNVD::SetGravity(const Point& gravity)
{
	NvdCapture* Nvd = getNvd();
	if(Nvd)
		Nvd->setPropertyValue_vec3(InstanceHandle(gEditor), "Gravity", ToPxVec3(gravity));
}

void TestNVD::Update()
{
	NvdCapture* Nvd = getNvd();
	if(Nvd)
	{
		if(mEdited)
		{
			const udword NbActors = gEditor->GetNbEditorActors();
			for(udword i=0;i<NbActors;i++)
			{
				const ActorData* CurrentActor = gEditor->GetEditorActor(i);
				const PR NewPose = mEdited->GetWorldTransform(GetNativeActorHandle(CurrentActor));
				Nvd->setPropertyValue_renderTransform(InstanceHandle(CurrentActor), ToPxTransform(NewPose));
			}
		}
//		Nvd->sync();
	}
}
