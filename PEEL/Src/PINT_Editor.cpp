///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PINT_Editor.h"
#include "..\PINT_Common\PINT_Common.h"
#include "USDExport.h"
#include "ZB2Export.h"
#include "PintShapeRenderer.h"
#include "PEEL.h"
#include "PintObjectsManager.h"

#pragma warning(disable:4355)	// 'this' : used in base member initializer list

/*
- make it work for cylinders
- materials aren't purged on release so I guess unused materials can get exported
	- but isn't it the same for convex/mesh meshes? esp mesh objects?
- can you share materials/shapes whose names are different?
* missing EditorObject for materials / joints
	- they will all be exported even if !mExportable/!mAddToDatabase
	- materials stats won't be updated on release
- data sharing
- hashmap to optimize:
	- the raycast object remapping
	- the material/etc sharing
- add extra info to ActorData2 like e.g. "ActorHasJoints" ? To check/help when releasing object
- release object: update stats, what do we do with shapes that aren't used anymore?
*/

#ifdef PEEL_COMPILE_NVD
	#define TEST_NVD
#endif

#ifdef TEST_NVD
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
#endif

///////////////////////////////////////////////////////////////////////////////

static	const bool gShareData		= true;

static	bool	gUpdateUI			= false;
static	bool	gDrawSceneBounds	= false;

static		PintPlugin*	mEditedPlugin	= null;
/*static*/	Pint*		mEdited			= null;

/*static*/ EditorPlugin* gEditor = null;

void SetEditedPlugin(PintPlugin* edited)
{
	mEditedPlugin = edited;
}

///////////////////////////////////////////////////////////////////////////////

Stats::Stats() :
	mNbStatics			(0),
	mNbDynamics			(0),
	mNbStaticCompounds	(0),
	mNbDynamicCompounds	(0),
	//
	mNbJoints			(0),
	mNbSphericalJoints	(0),
	mNbHingeJoints		(0),
	mNbPrismaticJoints	(0),
	mNbFixedJoints		(0),
	mNbDistanceJoints	(0),
	mNbD6Joints			(0),
	mNbGearJoints		(0),
	mNbRackJoints		(0),
	mNbPortalJoints		(0),
	//
	mTotalNbVerts		(0),
	mTotalNbTris		(0),
	//
	mNbAggregates		(0),
	mNbArticulations	(0),
	mNbRCArticulations	(0),
	mNbVehicles			(0),
	mNbCharacters		(0)
{
}

///////////////////////////////////////////////////////////////////////////////

EditorObject::EditorObject(Type type) :
	mEditorType		(type),
	mNativeHandle	(null),
	mRefCount		(1),
	mExportable		(true)
{
}

EditorActor::EditorActor() :
	EditorObject	(ACTOR),
	mIsStatic		(false),
	mIsCompound		(false)
{
	mInitPos.Zero();
	mInitRot.Identity();
}

EditorActor::~EditorActor()
{
}

EditorShape::EditorShape() :
	EditorObject	(SHAPE),
	mShapeType		(PINT_SHAPE_UNDEFINED),
	mNbTris			(0),
	mNbVerts		(0)
{
}

EditorShape::~EditorShape()
{
}

EditorJoint::EditorJoint() :
	EditorObject	(JOINT)
{
}

EditorJoint::~EditorJoint()
{
}

EditorMesh::EditorMesh() :
	EditorObject	(MESH)
{
}

EditorMesh::~EditorMesh()
{
}

EditorMaterial::EditorMaterial() :
	EditorObject	(MATERIAL)
{
}

EditorMaterial::~EditorMaterial()
{
}

EditorAggregate::EditorAggregate() :
	EditorObject	(AGGREGATE)
{
}

EditorAggregate::~EditorAggregate()
{
}

EditorCharacter::EditorCharacter() :
	EditorObject	(CHARACTER)
{
	dummy = null;
}

EditorCharacter::~EditorCharacter()
{
}

EditorVehicle::EditorVehicle() :
	EditorObject	(VEHICLE)
{
	dummy = null;
}

EditorVehicle::~EditorVehicle()
{
}

///////////////////////////////////////////////////////////////////////////////

/*bool EditorSceneAPI::SetGravity(const Point& gravity)
{
}

Point EditorSceneAPI::GetGravity() const
{
}*/

bool EditorSceneAPI::AddActors(udword nb_actors, const PintActorHandle* actors)
{
	const EditorPlugin& editor = static_cast<const EditorPlugin&>(mPint);
	if(!mEdited)
		return false;

	Pint_Scene* API = mEdited->GetSceneAPI();
	if(!API)
		return false;

	PintActorHandle* NativeActors = reinterpret_cast<PintActorHandle*>(ICE_ALLOC(sizeof(PintActorHandle)*nb_actors));
	for(udword i=0;i<nb_actors;i++)
	{
		const ActorData* Data = HandleToActorData(actors[i]);
		NativeActors[i] = GetNativeActorHandle(Data);
	}

	const bool b = API->AddActors(nb_actors, NativeActors);

	ICE_FREE(NativeActors);

	return b;
}

void EditorSceneAPI::GetActors(Reporter& reporter) const
{
	const EditorPlugin& editor = static_cast<const EditorPlugin&>(mPint);
	const udword NbActors = editor.GetNbEditorActors();
	for(udword i=0;i<NbActors;i++)
	{
		const ActorData* CurrentActor = HandleToActorData(editor.GetEditorActor(i));
		reporter.ReportObject(PintActorHandle(CurrentActor));
	}
}

void EditorSceneAPI::Cull(udword nb_planes, const Plane* planes, Reporter& reporter) const
{
	const EditorPlugin& editor = static_cast<const EditorPlugin&>(mPint);
	if(!mEdited)
		return;

	Pint_Scene* API = mEdited->GetSceneAPI();
	if(!API)
		return;

	class LocalReport : public Reporter
	{
//		PREVENT_COPY(LocalReport)
		public:

			LocalReport(Reporter& reporter, const EditorPlugin& editor) : mReporter(reporter), mEditor(editor)	{}

		virtual	bool	ReportObject(PintActorHandle engine_handle)
		{
/*			// TODO: optimize that loop
			for(udword j=0;j<mEditor.mEditorActors.GetNbEntries();j++)
			{
				ActorData* CurrentActor = HandleToActorData(mEditor.mEditorActors.GetEntry(j));
				if(GetNativeActorHandle(CurrentActor)==engine_handle)
				{
					mReporter.ReportObject(PintActorHandle(CurrentActor));
					break;
				}
			}*/
			mReporter.ReportObject(mEditor.RemapActorHandle(engine_handle));
			return true;
		}

		Reporter&			mReporter;
		const EditorPlugin&	mEditor;
	};

	API->Cull(nb_planes, planes, LocalReport(reporter, editor));
}

///////////////////////////////////////////////////////////////////////////////

// derived value = computed by native engines but not explicitly defined by users, i.e. not available in ActorData structures.
// We can use "pass through" macros then, that just re-route calls to underlying native engine.
#define PASS_THROUGH_GET_ACTOR_API(ret)							\
	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;	\
	if(!API)													\
		return ret;

#define PASS_THROUGH_GET_DERIVED_VALUE_NOPARAM(func, ret)		\
	PASS_THROUGH_GET_ACTOR_API(ret)								\
	const ActorData* AD = HandleToActorData(handle);			\
	return API->func(GetNativeActorHandle(AD));

#define PASS_THROUGH_GET_DERIVED_VALUE(func, param, ret)		\
	PASS_THROUGH_GET_ACTOR_API(ret)								\
	const ActorData* AD = HandleToActorData(handle);			\
	return API->func(GetNativeActorHandle(AD), param);

#define PASS_THROUGH_SET_DERIVED_VALUE(func, param, ret)		\
	PASS_THROUGH_GET_ACTOR_API(ret)								\
	ActorData* AD = HandleToActorData(handle);					\
	return API->func(GetNativeActorHandle(AD), param);


const char* EditorActorAPI::GetName(PintActorHandle handle) const
{
	const ActorData* AD = HandleToActorData(handle);
	const char* Name = AD->mName.Get();

	if(mEdited)
	{
		Pint_Actor* API = mEdited->GetActorAPI();
		if(API)
		{
			const char* NativeName = API->GetName(GetNativeActorHandle(AD));
			if(!Name)
			{
				ASSERT(!NativeName);
			}
			else
			{
				ASSERT(strcmp(Name, NativeName)==0);
			}
		}
	}

	return Name;
}

bool EditorActorAPI::SetName(PintActorHandle handle, const char* name)
{
	ActorData* AD = HandleToActorData(handle);
	AD->mName = name;

	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;
	if(!API)
		return false;
	return API->SetName(GetNativeActorHandle(AD), name);
}

udword EditorActorAPI::GetNbShapes(PintActorHandle handle) const
{
	const ActorData* AD = HandleToActorData(handle);
	return AD->mShapes.GetNbEntries();
}

PintShapeHandle EditorActorAPI::GetShape(PintActorHandle handle, udword index) const
{
	const ActorData* AD = HandleToActorData(handle);
	return PintShapeHandle(AD->mShapes[index]);
}

udword EditorActorAPI::GetNbJoints(PintActorHandle handle) const
{
	const ActorData* AD = HandleToActorData(handle);
	return AD->mJoints.GetNbEntries();
}

PintJointHandle EditorActorAPI::GetJoint(PintActorHandle handle, udword index) const
{
	const ActorData* AD = HandleToActorData(handle);
	const udword Nb = AD->mJoints.GetNbEntries();
	if(index>=Nb)
		return null;
	return PintJointHandle(AD->mJoints.GetEntry(index));
}

bool EditorActorAPI::GetWorldBounds(PintActorHandle handle, AABB& bounds) const
{
	PASS_THROUGH_GET_DERIVED_VALUE(GetWorldBounds, bounds, false)
}

void EditorActorAPI::WakeUp(PintActorHandle handle)
{
	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;
	if(!API)
		return;

	ActorData* AD = HandleToActorData(handle);
	API->WakeUp(GetNativeActorHandle(AD));
}

bool EditorActorAPI::SetGravityFlag(PintActorHandle handle, bool flag)
{
	PASS_THROUGH_SET_DERIVED_VALUE(SetGravityFlag, flag, false)
}

bool EditorActorAPI::SetDebugVizFlag(PintActorHandle handle, bool flag)
{
	PASS_THROUGH_SET_DERIVED_VALUE(SetDebugVizFlag, flag, false)
}

bool EditorActorAPI::SetSimulationFlag(PintActorHandle handle, bool flag)
{
	PASS_THROUGH_SET_DERIVED_VALUE(SetSimulationFlag, flag, false)
}

float EditorActorAPI::GetLinearDamping(PintActorHandle handle) const
{
	PASS_THROUGH_GET_DERIVED_VALUE_NOPARAM(GetLinearDamping, false)
}

bool EditorActorAPI::SetLinearDamping(PintActorHandle handle, float damping)
{
	PASS_THROUGH_SET_DERIVED_VALUE(SetLinearDamping, damping, false)
}

float EditorActorAPI::GetAngularDamping(PintActorHandle handle) const
{
	PASS_THROUGH_GET_DERIVED_VALUE_NOPARAM(GetAngularDamping, false)
}

bool EditorActorAPI::SetAngularDamping(PintActorHandle handle, float damping)
{
	PASS_THROUGH_SET_DERIVED_VALUE(SetAngularDamping, damping, false)
}

bool EditorActorAPI::GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space) const
{
	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;
	if(!API)
		return false;

	const ActorData* AD = HandleToActorData(handle);
	return API->GetLinearVelocity(GetNativeActorHandle(AD), linear_velocity, world_space);
}

bool EditorActorAPI::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)
{
	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;
	if(!API)
		return false;

	const ActorData* AD = HandleToActorData(handle);
	return API->SetLinearVelocity(GetNativeActorHandle(AD), linear_velocity, world_space);
}

bool EditorActorAPI::GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space) const
{
	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;
	if(!API)
		return false;

	const ActorData* AD = HandleToActorData(handle);
	return API->GetAngularVelocity(GetNativeActorHandle(AD), angular_velocity, world_space);
}

bool EditorActorAPI::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)
{
	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;
	if(!API)
		return false;

	const ActorData* AD = HandleToActorData(handle);
	return API->SetAngularVelocity(GetNativeActorHandle(AD), angular_velocity, world_space);
}

float EditorActorAPI::GetMass(PintActorHandle handle) const
{
	// TODO: check masses are the same in editor & native structs?
	const ActorData* AD = HandleToActorData(handle);
	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;
	if(!API)
		return AD->mMass;
	return API->GetMass(GetNativeActorHandle(AD));
}

bool EditorActorAPI::SetMass(PintActorHandle handle, float mass)
{
	ActorData* AD = HandleToActorData(handle);
	AD->mMass = mass;

	Pint_Actor* API = mEdited ? mEdited->GetActorAPI() : null;
	if(!API)
		return false;

	return API->SetMass(GetNativeActorHandle(AD), mass);
}

bool EditorActorAPI::GetLocalInertia(PintActorHandle handle, Point& inertia) const
{
	PASS_THROUGH_GET_DERIVED_VALUE(GetLocalInertia, inertia, false)
}

bool EditorActorAPI::SetLocalInertia(PintActorHandle handle, const Point& inertia)
{
	PASS_THROUGH_SET_DERIVED_VALUE(SetLocalInertia, inertia, false)
}

bool EditorActorAPI::GetCMassLocalPose(PintActorHandle handle, PR& pose) const
{
	PASS_THROUGH_GET_DERIVED_VALUE(GetCMassLocalPose, pose, false)
}

bool EditorActorAPI::SetCMassLocalPose(PintActorHandle handle, const PR& pose)
{
	PASS_THROUGH_SET_DERIVED_VALUE(SetCMassLocalPose, pose, false)
}

///////////////////////////////////////////////////////////////////////////////

const char* EditorShapeAPI::GetName(PintShapeHandle handle) const
{
	const ShapeData* SD = HandleToShapeData(handle);
	return SD->mName.Get();

/*	Pint_Shape* API = mEdited ? mEdited->GetShapeAPI() : null;
	if(!API)
		return null;
	const ActorData* AD = HandleToActorData(handle);
	return API->GetShape(GetNativeActorHandle(AD), index);*/
}

bool EditorShapeAPI::SetName(PintShapeHandle handle, const char* name)
{
	ShapeData* SD = HandleToShapeData(handle);
	SD->mName = name;

	Pint_Shape* API = mEdited ? mEdited->GetShapeAPI() : null;
	if(!API)
		return false;
	return API->SetName(GetNativeShapeHandle(SD), name);
}

PintShape EditorShapeAPI::GetType(PintShapeHandle handle) const
{
	const ShapeData* SD = HandleToShapeData(handle);
	return SD->mType;
}

PR EditorShapeAPI::GetWorldTransform(PintActorHandle actor, PintShapeHandle shape) const
{
	Pint_Shape* API = mEdited ? mEdited->GetShapeAPI() : null;
	if(!API)
		return PR(Idt);

	const ActorData* AD = HandleToActorData(actor);
	const ShapeData* SD = HandleToShapeData(shape);

	return API->GetWorldTransform(GetNativeActorHandle(AD), GetNativeShapeHandle(SD));
}

bool EditorShapeAPI::GetWorldBounds(PintActorHandle actor, PintShapeHandle shape, AABB& bounds) const
{
	Pint_Shape* API = mEdited ? mEdited->GetShapeAPI() : null;
	if(!API)
		return false;

	const ActorData* AD = HandleToActorData(actor);
	const ShapeData* SD = HandleToShapeData(shape);

	return API->GetWorldBounds(GetNativeActorHandle(AD), GetNativeShapeHandle(SD), bounds);
}

bool EditorShapeAPI::GetTriangle(Triangle& tri, PintShapeHandle handle, udword index) const
{
	Pint_Shape* API = mEdited ? mEdited->GetShapeAPI() : null;
	if(!API)
		return false;

	const ShapeData* SD = HandleToShapeData(handle);
	return API->GetTriangle(tri, GetNativeShapeHandle(SD), index);
}

bool EditorShapeAPI::GetIndexedTriangle(IndexedTriangle& tri, PintShapeHandle handle, udword index) const
{
	Pint_Shape* API = mEdited ? mEdited->GetShapeAPI() : null;
	if(!API)
		return false;

	const ShapeData* SD = HandleToShapeData(handle);
	return API->GetIndexedTriangle(tri, GetNativeShapeHandle(SD), index);
}

PintShapeRenderer* EditorShapeAPI::GetShapeRenderer(PintShapeHandle handle) const
{
	Pint_Shape* API = mEdited ? mEdited->GetShapeAPI() : null;
	if(!API)
		return null;

	const ShapeData* SD = HandleToShapeData(handle);

	return API->GetShapeRenderer(GetNativeShapeHandle(SD));
}

///////////////////////////////////////////////////////////////////////////////

const char* EditorJointAPI::GetName(PintJointHandle handle) const
{
	const JointData* JD = HandleToJointData(handle);
	return JD->mName.Get();
}

bool EditorJointAPI::SetName(PintJointHandle handle, const char* name)
{
	JointData* JD = HandleToJointData(handle);
	JD->mName = name;

	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;
	return API->SetName(GetNativeJointHandle(JD), name);
}

PintJoint EditorJointAPI::GetType(PintJointHandle handle) const
{
	const JointData* JD = HandleToJointData(handle);
	return JD->mType;
}

bool EditorJointAPI::GetActors(PintJointHandle handle, PintActorHandle& actor0, PintActorHandle& actor1) const
{
	const JointData* JD = HandleToJointData(handle);
	actor0 = PintActorHandle(JD->mObject0);
	actor1 = PintActorHandle(JD->mObject1);
	return true;
}

bool EditorJointAPI::SetActors(PintJointHandle handle, PintActorHandle actor0, PintActorHandle actor1)
{
	JointData* JD = HandleToJointData(handle);
	ActorData* OldActor0 = JD->mObject0;
	ActorData* OldActor1 = JD->mObject1;

	ActorData* NewActor0 = HandleToActorData(actor0);
	ActorData* NewActor1 = HandleToActorData(actor1);

	if(OldActor0 != NewActor0)
	{
		if(OldActor0)
			OldActor0->mJoints.Delete(JD);

		if(NewActor0)
			NewActor0->mJoints.AddPtr(JD);

		JD->mObject0 = NewActor0;
	}

	if(OldActor1 != NewActor1)
	{
		if(OldActor1)
			OldActor1->mJoints.Delete(JD);

		if(NewActor1)
			NewActor1->mJoints.AddPtr(JD);

		JD->mObject1 = NewActor1;
	}

	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	return API->SetActors(GetNativeJointHandle(JD), GetNativeActorHandle(NewActor0), GetNativeActorHandle(NewActor1));
}

bool EditorJointAPI::GetFrames(PintJointHandle handle, PR* frame0, PR* frame1) const
{
	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	const JointData* JD = HandleToJointData(handle);
	return API->GetFrames(GetNativeJointHandle(JD), frame0, frame1);
}

bool EditorJointAPI::SetFrames(PintJointHandle handle, const PR* frame0, const PR* frame1)
{
	JointData* JD = HandleToJointData(handle);
	JD->SetFrames(frame0, frame1);

	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	return API->SetFrames(GetNativeJointHandle(JD), frame0, frame1);
}

bool EditorJointAPI::GetLimits(PintJointHandle handle, PintLimits& limits, udword index) const
{
	const JointData* JD = HandleToJointData(handle);

	// Read from editor structures first (they better preserve invalid limits)
	if(JD->GetLimits(limits, index))
		return true;

	// Else try from physics plugin
	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	return API->GetLimits(GetNativeJointHandle(JD), limits, index);
}

bool EditorJointAPI::SetLimits(PintJointHandle handle, const PintLimits& limits, udword index)
{
	JointData* JD = HandleToJointData(handle);
	JD->SetLimits(limits, index);

	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	return API->SetLimits(GetNativeJointHandle(JD), limits, index);
}

bool EditorJointAPI::GetSpring(PintJointHandle handle, PintSpring& spring) const
{
	// ### return from editor structs?

	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	const JointData* JD = HandleToJointData(handle);
	return API->GetSpring(GetNativeJointHandle(JD), spring);
}

bool EditorJointAPI::SetSpring(PintJointHandle handle, const PintSpring& spring)
{
	JointData* JD = HandleToJointData(handle);
	JD->SetSpring(spring);

	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	return API->SetSpring(GetNativeJointHandle(JD), spring);
}

bool EditorJointAPI::GetGearRatio(PintJointHandle handle, float& ratio) const
{
	// ### return from editor structs?

	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	const JointData* JD = HandleToJointData(handle);
	return API->GetGearRatio(GetNativeJointHandle(JD), ratio);
}

bool EditorJointAPI::SetGearRatio(PintJointHandle handle, float ratio)
{
	JointData* JD = HandleToJointData(handle);
	JD->SetGearRatio(ratio);

	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	return API->SetGearRatio(GetNativeJointHandle(JD), ratio);
}

bool EditorJointAPI::GetHingeDynamicData(PintJointHandle handle, PintHingeDynamicData& data) const
{
	// Dynamic/runtime data isn't stored in editor structures so we must go to the edited plugin
	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	const JointData* JD = HandleToJointData(handle);
	const PintJointHandle h = GetNativeJointHandle(JD);
	return API->GetHingeDynamicData(h, data);
}

bool EditorJointAPI::GetD6DynamicData(PintJointHandle handle, PintD6DynamicData& data) const
{
	// Dynamic/runtime data isn't stored in editor structures so we must go to the edited plugin
	Pint_Joint* API = mEdited ? mEdited->GetJointAPI() : null;
	if(!API)
		return false;

	const JointData* JD = HandleToJointData(handle);
	const PintJointHandle h = GetNativeJointHandle(JD);
	return API->GetD6DynamicData(h, data);
}

///////////////////////////////////////////////////////////////////////////////

PintVehicleHandle EditorVehicleAPI::CreateVehicle(PintVehicleData& data, const PINT_VEHICLE_CREATE& vehicle)
{
	// TODO: what should we do here if the edited plugin doesn't support the feature?
	EditorPlugin& editor = static_cast<EditorPlugin&>(mPint);

	Pint_Vehicle* API = mEdited ? mEdited->GetVehicleAPI() : null;
	if(!API)
		return null;

	editor.mStats.mNbVehicles++;
	gUpdateUI = true;

	EditorVehicle* Object = ICE_NEW(EditorVehicle);

	const PintVehicleHandle EngineHandle = API->CreateVehicle(data, vehicle);
	Object->mNativeHandle = EngineHandle;

	Object->dummy = editor.CreateDummyEditorActor(data.mChassisActor);
	data.mChassisActor = Object->dummy;

	ASSERT(!"Missing remap for shapes here");

	VehicleData* NewVehicle = ICE_NEW(VehicleData);
	NewVehicle->mUserData = Object;
	editor.mEditorVehicles.AddPtr(NewVehicle);

	return PintVehicleHandle(NewVehicle);
}

bool EditorVehicleAPI::SetVehicleInput(PintVehicleHandle vehicle, const PINT_VEHICLE_INPUT& input)
{
	Pint_Vehicle* API = mEdited ? mEdited->GetVehicleAPI() : null;
	if(!API)
		return false;

	const VehicleData* Vehicle = HandleToVehicleData(vehicle);

	return API->SetVehicleInput(GetNativeVehicleHandle(Vehicle), input);
}

PintActorHandle EditorVehicleAPI::GetVehicleActor(PintVehicleHandle h) const
{
	const VehicleData* Vehicle = HandleToVehicleData(h);
	return ((const EditorVehicle*)Vehicle->mUserData)->dummy;
}

bool EditorVehicleAPI::GetVehicleInfo(PintVehicleHandle vehicle, PintVehicleInfo& info) const
{
	Pint_Vehicle* API = mEdited ? mEdited->GetVehicleAPI() : null;
	if(!API)
		return false;

	const VehicleData* Vehicle = HandleToVehicleData(vehicle);

	return API->GetVehicleInfo(GetNativeVehicleHandle(Vehicle), info);
}

/*		virtual	bool				ResetVehicleData(PintVehicleHandle vehicle);
		virtual	bool				AddActor(PintVehicleHandle vehicle, PintActorHandle actor);
		virtual	bool				AddShape(PintVehicleHandle vehicle, const PINT_SHAPE_CREATE& create);*/

///////////////////////////////////////////////////////////////////////////////

PintCharacterHandle EditorCharacterAPI::CreateCharacter(const PINT_CHARACTER_CREATE& create)
{
	// TODO: what should we do here if the edited plugin doesn't support the feature?
	EditorPlugin& editor = static_cast<EditorPlugin&>(mPint);

	Pint_Character* API = mEdited ? mEdited->GetCharacterAPI() : null;
	if(!API)
		return null;

	editor.mStats.mNbCharacters++;
	gUpdateUI = true;

	EditorCharacter* Object = ICE_NEW(EditorCharacter);

	const PintCharacterHandle EngineHandle = API->CreateCharacter(create);
	Object->mNativeHandle = EngineHandle;

	const PintActorHandle KineActorEngineHandle = API->GetCharacterActor(EngineHandle);
	Object->dummy = editor.CreateDummyEditorActor(KineActorEngineHandle);

	CharacterData* NewCharacter = ICE_NEW(CharacterData);
	NewCharacter->mUserData = Object;
	editor.mEditorCharacters.AddPtr(NewCharacter);

#ifdef TEST_NVD
	ASSERT(0);
#endif
	return PintCharacterHandle(NewCharacter);
}

PintActorHandle EditorCharacterAPI::GetCharacterActor(PintCharacterHandle h)
{
	const CharacterData* Character = HandleToCharacterData(h);
	return ((const EditorCharacter*)Character->mUserData)->dummy;
}

udword EditorCharacterAPI::MoveCharacter(PintCharacterHandle h, const Point& disp)
{
	Pint_Character* API = mEdited ? mEdited->GetCharacterAPI() : null;
	if(!API)
		return 0;

	const CharacterData* Character = HandleToCharacterData(h);

	return API->MoveCharacter(GetNativeCharacterHandle(Character), disp);
}

///////////////////////////////////////////////////////////////////////////////

EditorPlugin::EditorPlugin() :
	mSceneAPI		(*this),
	mActorAPI		(*this),
	mShapeAPI		(*this),
	mJointAPI		(*this),
	mVehicleAPI		(*this),
	mCharacterAPI	(*this),
	mEditorScene	(null),
	mCollisionGroups(null)
{
	gUpdateUI = false;
}

EditorPlugin::~EditorPlugin()
{
}

const char* EditorPlugin::GetName() const
{
//	if(mEdited)
//		return mEdited->GetName();
	return "Editor";
}

const char*	EditorPlugin::GetUIName() const
{
//	if(mEdited)
//		return mEdited->GetUIName();
	return "Editor Database";
}

void EditorPlugin::GetCaps(PintCaps& caps) const
{
//	if(mEdited)
//		mEdited->GetCaps(caps);

	// We virtually support everything - we want to gather the stats for everything.
	caps.mSupportRigidBodySimulation	= true;
	caps.mSupportCylinders				= true;
	caps.mSupportConvexes				= true;
	caps.mSupportMeshes					= true;
	caps.mSupportDeformableMeshes		= true;
	caps.mSupportContactNotifications	= true;
	caps.mSupportContactModifications	= true;
	caps.mSupportMassForInertia			= true;
	caps.mSupportKinematics				= true;
	caps.mSupportCollisionGroups		= true;
	caps.mSupportCompounds				= true;
	caps.mSupportAggregates				= true;
	//
	caps.mSupportSphericalJoints		= true;
	caps.mSupportHingeJoints			= true;
	caps.mSupportFixedJoints			= true;
	caps.mSupportPrismaticJoints		= true;
	caps.mSupportDistanceJoints			= true;
	caps.mSupportD6Joints				= true;
	caps.mSupportGearJoints				= true;
	caps.mSupportRackJoints				= true;
	caps.mSupportPortalJoints			= true;
	caps.mSupportMCArticulations		= true;
	caps.mSupportRCArticulations		= true;
	//
	caps.mSupportRaycasts				= true;
	//
	caps.mSupportBoxSweeps				= true;
	caps.mSupportSphereSweeps			= true;
	caps.mSupportCapsuleSweeps			= true;
	caps.mSupportConvexSweeps			= true;
	//
	caps.mSupportSphereOverlaps			= true;
	caps.mSupportBoxOverlaps			= true;
	caps.mSupportCapsuleOverlaps		= true;
	caps.mSupportConvexOverlaps			= true;
	caps.mSupportMeshMeshOverlaps		= true;
	//
	caps.mSupportVehicles				= true;
	caps.mSupportCharacters				= true;
}

udword EditorPlugin::GetFlags() const
{
	if(mEdited)
		return mEdited->GetFlags();

	return 0;
}

//PintSceneHandle EditorPlugin::Init(const PINT_WORLD_CREATE& desc)
void EditorPlugin::Init(const PINT_WORLD_CREATE& desc)
{
//	if(mEdited)
//		mEdited->Init(desc);
//	return;

#ifdef TEST_NVD
	NvdCapture* Nvd = getNvd();
	if(Nvd)
		Nvd->open();
#endif

	gUpdateUI = true;

	String SceneDescFromUI;
	if(UI_GetSceneDesc(SceneDescFromUI))
		mEditorScene = ICE_NEW(SceneData)(desc, SceneDescFromUI);
	else
		mEditorScene = ICE_NEW(SceneData)(desc, null);
//	return mEditorScene;
}

void EditorPlugin::SetGravity(const Point& gravity)
{
	if(mEdited)
		mEdited->SetGravity(gravity);
//	return;

	gUpdateUI = true;

	ASSERT(mEditorScene);
	mEditorScene->mGravity = gravity;

#ifdef TEST_NVD
	NvdCapture* Nvd = getNvd();
	if(Nvd)
		Nvd->setPropertyValue_vec3(InstanceHandle(gEditor), "Gravity", ToPxVec3(gravity));
#endif
}

void EditorPlugin::SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)
{
	if(mEdited)
		mEdited->SetDisabledGroups(nb_groups, groups);
//	return;

	gUpdateUI = true;

	ASSERT(!mCollisionGroups);
	mCollisionGroups = ICE_NEW(CollisionGroupsData)(nb_groups, groups);
}

static inline_ void DecreaseStats(EditorObject* editor_object, Stats::Shared& stats)
{
	if(editor_object->mExportable)
	{
		ASSERT(stats.mNbRef);
		stats.mNbRef--;
		if(!editor_object->mRefCount)
		{
			ASSERT(stats.mNbUnique);
			stats.mNbUnique--;
		}
	}
}

template<class T>
static void DeleteObject(T* data)
{
	if(data->mUserData)
	{
		EditorObject* Object = (EditorObject*)data->mUserData;
		DELETESINGLE(Object);
	}
	DELETESINGLE(data);
}

static bool DeleteShape(EditorShape* editor_shape, ShapeData* shape_data, PtrContainer& editor_shapes, Stats& stats, Stats::Shared* shape_stats)
{
	ASSERT(editor_shape->mRefCount);
	editor_shape->mRefCount--;

	if(shape_stats)
		DecreaseStats(editor_shape, *shape_stats);

	if(editor_shape->mRefCount)
		return false;

	if(shape_data->mMaterial)
	{
		EditorMaterial* Object = (EditorMaterial*)shape_data->mMaterial->mUserData;
		ASSERT(Object->GetType()==EditorObject::MATERIAL);
		ASSERT(Object->mRefCount);
		Object->mRefCount--;
		// TODO: delete editor material if refcount is zero?
		DecreaseStats(Object, stats.mMaterials);
	}

	// TODO: optimize this O(n) call
	if(editor_shapes.Delete(shape_data))
		DeleteObject(shape_data);
	return true;
}

static void DeleteActor(ActorData* data, EditorPlugin& editor, Stats& stats)
{
	if(data->mUserData)
	{
		EditorActor* Object = (EditorActor*)data->mUserData;
		ASSERT(Object->GetType()==EditorObject::ACTOR);

		const udword NbShapes = data->mShapes.GetNbEntries();
		for(udword i=0;i<NbShapes;i++)
		{
			ShapeData* CurrentShape = (ShapeData*)data->mShapes.GetEntry(i);
			EditorShape* ShapeObject = (EditorShape*)CurrentShape->mUserData;
			if(ShapeObject)
			{
				ASSERT(ShapeObject->GetType()==EditorObject::SHAPE);
				if(ShapeObject->mShapeType==PINT_SHAPE_SPHERE)
				{
					DeleteShape(ShapeObject, CurrentShape, editor.GetEditorSphereShapes(), stats, &stats.mSphereShapes);
				}
				else if(ShapeObject->mShapeType==PINT_SHAPE_CAPSULE)
				{
					DeleteShape(ShapeObject, CurrentShape, editor.GetEditorCapsuleShapes(), stats, &stats.mCapsuleShapes);
				}
				else if(ShapeObject->mShapeType==PINT_SHAPE_CYLINDER)
				{
					DeleteShape(ShapeObject, CurrentShape, editor.GetEditorCylinderShapes(), stats, &stats.mCylinderShapes);
				}
				else if(ShapeObject->mShapeType==PINT_SHAPE_BOX)
				{
					DeleteShape(ShapeObject, CurrentShape, editor.GetEditorBoxShapes(), stats, &stats.mBoxShapes);
				}
				else if(ShapeObject->mShapeType==PINT_SHAPE_CONVEX)
				{
					DeleteShape(ShapeObject, CurrentShape, editor.GetEditorConvexShapes(), stats, &stats.mConvexShapes);
				}
				else if(ShapeObject->mShapeType==PINT_SHAPE_MESH)
				{
					if(ShapeObject->mExportable)
					{
//						ASSERT(stats.mNbMeshShapes);
//						stats.mNbMeshShapes--;
						stats.mTotalNbTris -= ShapeObject->mNbTris;
						stats.mTotalNbVerts -= ShapeObject->mNbVerts;
					}
					DeleteShape(ShapeObject, CurrentShape, editor.GetEditorMeshShapes(), stats, &stats.mMeshShapes);
				}
				else if(ShapeObject->mShapeType==PINT_SHAPE_MESH2)
				{
					if(ShapeObject->mExportable)
					{
//						ASSERT(stats.mNbMeshShapes);
//						stats.mNbMeshShapes--;
						stats.mTotalNbTris -= ShapeObject->mNbTris;
						stats.mTotalNbVerts -= ShapeObject->mNbVerts;
					}
					DeleteShape(ShapeObject, CurrentShape, editor.GetEditorMeshShapes2(), stats, &stats.mMeshShapes);
				}
				else ASSERT(0);
			}
		}

		if(Object->mExportable)
		{
			const bool IsCompound = NbShapes>1;
			ASSERT(Object->mIsCompound==IsCompound);
			if(Object->mIsStatic)
			{
				ASSERT(data->mMass==0.0f);
				ASSERT(stats.mNbStatics);
				stats.mNbStatics--;
				if(Object->mIsCompound)
				{
					ASSERT(stats.mNbStaticCompounds);
					stats.mNbStaticCompounds--;
				}
			}
			else
			{
				ASSERT(data->mMass!=0.0f);
				ASSERT(stats.mNbDynamics);
				stats.mNbDynamics--;
				if(Object->mIsCompound)
				{
					ASSERT(stats.mNbDynamicCompounds);
					stats.mNbDynamicCompounds--;
				}
			}
		}
		DELETESINGLE(Object);
	}
	DELETESINGLE(data);
}

template<class T>
static void DeleteObjects(PtrContainer& shapes)
{
	const udword Nb = shapes.GetNbEntries();
	for(udword i=0;i<Nb;i++)
	{
		T* Current = (T*)shapes.GetEntry(i);
		DeleteObject(Current);
	}
	shapes.Empty();
}

void EditorPlugin::Close()
{
//	if(mEdited)
//		mEdited->Close();
//	return;

	DeleteObjects<ActorData>(mEditorActors);
	DeleteObjects<MaterialData>(mEditorMaterials);
	DeleteObjects<SphereShapeData>(mEditorSphereShapes);
	DeleteObjects<CapsuleShapeData>(mEditorCapsuleShapes);
	DeleteObjects<CylinderShapeData>(mEditorCylinderShapes);
	DeleteObjects<BoxShapeData>(mEditorBoxShapes);
	DeleteObjects<ConvexShapeData>(mEditorConvexShapes);
	DeleteObjects<MeshShapeData>(mEditorMeshShapes);
	DeleteObjects<MeshShapeData2>(mEditorMeshShapes2);
	DeleteObjects<JointData>(mEditorJoints);
	DeleteObjects<MeshData>(mEditorMeshes);
	DeleteObjects<AggregateData>(mEditorAggregates);
	DeleteObjects<CharacterData>(mEditorCharacters);
	DeleteObjects<VehicleData>(mEditorVehicles);
	DELETESINGLE(mCollisionGroups);
	DELETESINGLE(mEditorScene);

#ifdef TEST_NVD
	NvdCapture* Nvd = getNvd();
	if(Nvd)
		Nvd->close();
#endif
}

udword EditorPlugin::Update(float dt)
{
#ifdef TEST_NVD
	NvdCapture* Nvd = getNvdForCapture();
	if(Nvd)
	{
		Nvd->sync();
	}
#endif

	udword Ret = 0;
	if(mEdited)
		Ret = mEdited->Update(dt);

	if(gUpdateUI)
	{
		gUpdateUI = false;
		UpdateUI();
	}

#ifdef TEST_NVD
	if(Nvd)
	{
		if(mEdited)
		{
			const udword NbActors = mEditorActors.GetNbEntries();
			for(udword i=0;i<NbActors;i++)
			{
				const ActorData* CurrentActor = HandleToActorData(mEditorActors.GetEntry(i));

				const PR NewPose = mEdited->GetWorldTransform(GetNativeActorHandle(CurrentActor));
				Nvd->setPropertyValue_renderTransform(InstanceHandle(CurrentActor), ToPxTransform(NewPose));
			}
		}
//		Nvd->sync();
	}
#endif

	return Ret;
}

Point EditorPlugin::GetMainColor()
{
	if(mEdited)
		return mEdited->GetMainColor();

	return Point(0.5f, 0.5f, 0.5f);
}

#ifdef TEST_NVD
class WireframeOverlayRenderInterface : public NvdRenderInterface
{
	public:
					WireframeOverlayRenderInterface()	{}

	virtual	void	renderHandle(const PxTransform& pose, RenderHandle handle, WeakHandle wh)
	{
		PintShapeRenderer* renderer = reinterpret_cast<PintShapeRenderer*>(handle);
		renderer->_Render(ToPR(pose));
	}

	virtual	void	render(const px::PxTransform& pose)											{ ASSERT(0);	}
	virtual	void	renderPlane(const px::PxTransform& pose)									{ ASSERT(0);	}
	virtual	void	renderSphere(const px::PxTransform& pose, float radius)						{ ASSERT(0);	}
	virtual	void	renderCapsule(const px::PxTransform& pose, float halfHeight, float radius)	{ ASSERT(0);	}
	virtual	void	renderBox(const px::PxTransform& pose, const px::PxVec3& extents)			{ ASSERT(0);	}
	virtual	void	renderMesh(const px::PxTransform& pose, const SurfaceInterface&)			{ ASSERT(0);	}
	virtual	void	renderConvex(const px::PxTransform& pose, const NvdConvexInterface&)		{ ASSERT(0);	}
	virtual	void	renderBounds(const PxBounds3& bounds)										{ ASSERT(0);	}
};

class BoundsRenderInterface : public NvdRenderInterface
{
	public:
					BoundsRenderInterface(PintRender& renderer) : mRenderer(renderer)	{}

	virtual	void	renderHandle(const PxTransform& pose, RenderHandle handle, WeakHandle wh)	{ ASSERT(0);	}
	virtual	void	render(const px::PxTransform& pose)											{ ASSERT(0);	}
	virtual	void	renderPlane(const px::PxTransform& pose)									{ ASSERT(0);	}
	virtual	void	renderSphere(const px::PxTransform& pose, float radius)						{ ASSERT(0);	}
	virtual	void	renderCapsule(const px::PxTransform& pose, float halfHeight, float radius)	{ ASSERT(0);	}
	virtual	void	renderBox(const px::PxTransform& pose, const px::PxVec3& extents)			{ ASSERT(0);	}
	virtual	void	renderMesh(const px::PxTransform& pose, const SurfaceInterface&)			{ ASSERT(0);	}
	virtual	void	renderConvex(const px::PxTransform& pose, const NvdConvexInterface&)		{ ASSERT(0);	}
	virtual	void	renderBounds(const px::PxBounds3& bounds)
	{
		const AABB& Box = reinterpret_cast<const AABB&>(bounds);
		mRenderer.DrawWireframeAABB(1, &Box, Point(1.0f, 1.0f, 1.0f));
	}

	PintRender&		mRenderer;
};


class PEELRenderInterface : public NvdRenderInterface
{
	public:
					PEELRenderInterface(PintRender& renderer) : mRenderer(renderer)	{}
	virtual			~PEELRenderInterface()
					{
						renderSelected();
					}

			struct SelectedRenderItem
			{
				PintShapeRenderer*	mRenderer;
				PxTransform			mPose;
			};
			//#### assert size

			Container	mSelectedRenderData;

			void	renderSelected()
			{
				// ### do that better
				void EndBatchConvexRender();
				void StartBatchConvexRender();

				EndBatchConvexRender();
				StartBatchConvexRender();

				const SelectedRenderItem* SRI = (const SelectedRenderItem*)mSelectedRenderData.GetEntries();
				udword Nb = mSelectedRenderData.GetNbEntries()/(sizeof(SelectedRenderItem)/sizeof(udword));
				glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
				while(Nb--)
				{
					SRI->mRenderer->_Render(ToPR(SRI->mPose));
					SRI++;
				}
//				EndBatchConvexRender();
			}


	virtual	void	renderHandle(const PxTransform& pose, RenderHandle handle, WeakHandle wh)
	{
		PintShapeRenderer* renderer = reinterpret_cast<PintShapeRenderer*>(handle);
	
		if(gNVDControlInterface.mSelected && gNVDControlInterface.mSelected->contains(wh))
		{
			SelectedRenderItem* SRI = (SelectedRenderItem*)mSelectedRenderData.Reserve(sizeof(SelectedRenderItem)/sizeof(udword));
			SRI->mRenderer = renderer;
			SRI->mPose = pose;
		}
		else
		{
//			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			renderer->_Render(ToPR(pose));
		}
	}

	virtual	void	render(const PxTransform& pose)
	{
		return;
		const Point v0 = ToPoint(pose.q.getBasisVector0());
		const Point v1 = ToPoint(pose.q.getBasisVector1());
		const Point v2 = ToPoint(pose.q.getBasisVector2());

		const Point P = ToPoint(pose.p);

		mRenderer.DrawLine(P, P+v0, Point(1.0f, 0.0f, 0.0f));
		mRenderer.DrawLine(P, P+v1, Point(0.0f, 1.0f, 0.0f));
		mRenderer.DrawLine(P, P+v2, Point(0.0f, 0.0f, 1.0f));
	}

	virtual	void	renderPlane(const px::PxTransform& pose)
	{
		return;
//		mRenderer.DrawBox(Point(0.001f, 1000.0f, 1000.0f), ToPR(pose));

		const float gridSize = 200.0f;
		const float minX = -gridSize;
		const float maxX = gridSize;
		const float minZ = -gridSize;
		const float maxZ = gridSize;
		const float dX = 1.0f;
		const float dZ = 1.0f;

		const Point color(1.0f, 1.0f, 1.0f);

		float currentX = minX;
		while(currentX<=maxX)
		{
			mRenderer.DrawLine(Point(currentX, 0.0f, minZ), Point(currentX, 0.0f, maxZ), color);
			currentX += dX;
		}

		float currentZ = minZ;
		while(currentZ<=maxZ)
		{
			mRenderer.DrawLine(Point(minX, 0.0f, currentZ), Point(maxX, 0.0f, currentZ), color);
			currentZ += dZ;
		}
	}

	virtual	void	renderSphere(const PxTransform& pose, float radius)
	{
		mRenderer.DrawSphere(radius, ToPR(pose));
	}

	virtual	void	renderCapsule(const PxTransform& pose, float halfHeight, float radius)
	{
		mRenderer.DrawCapsule(radius, halfHeight*2.0f, ToPR(pose));
	}

	virtual	void	renderBox(const PxTransform& pose, const PxVec3& extents)
	{
		mRenderer.DrawBox(ToPoint(extents), ToPR(pose));
	}

	virtual	void	renderConvex(const PxTransform& pose, const NvdConvexInterface& data)
	{
		const Point color(0.5f, 0.5f, 0.5f);

		const PxVec3* vertexBuffer = data.mVerts;
		const PxU8* indexBuffer = data.mIndices;
		const PxU32 polygonCount = data.mNbPolys;
		for(PxU32 i=0; i<polygonCount; i++)
		{
//			PxHullPolygon data;
//			mesh.getPolygonData(i, data);

			const PxU32 vertexCount = data.mVertexCounts[i];
//			const PxU32 vertexCount = data.mNbVerts;

			const PxVec3& v0 = vertexBuffer[indexBuffer[0]];
			const PxVec3 p0 = pose.transform(v0);
			for(PxU32 j=0; j<vertexCount-2; j++)
			{
				const PxVec3& v1 = vertexBuffer[indexBuffer[j+1]];
				const PxVec3& v2 = vertexBuffer[indexBuffer[j+2]];

				//addTriangle(globalPose.transform(v0), globalPose.transform(v1), globalPose.transform(v2), color);
				const PxVec3 p1 = pose.transform(v1);
				const PxVec3 p2 = pose.transform(v2);
				mRenderer.DrawTriangle(ToPoint(p0), ToPoint(p1), ToPoint(p2), color);
			}
			indexBuffer += vertexCount;
		}
	}

	virtual	void	renderMesh(const PxTransform& pose, const SurfaceInterface& surface)
	{
		if(1)
		{
			mRenderer.DrawSurface(surface, ToPR(pose));
		}
		else
		{
			const Point color(0.5f, 0.5f, 0.5f);

			const PxVec3* verts = reinterpret_cast<const PxVec3*>(surface.mVerts);
			if(surface.mDFaces)
			{
				const PxU32* indices = surface.mDFaces;
				for(PxU32 i=0;i<surface.mNbFaces;i++)
				{
					const PxU32 ref0 = *indices++;
					const PxU32 ref1 = *indices++;
					const PxU32 ref2 = *indices++;
					const PxVec3 p0 = pose.transform(verts[ref0]);
					const PxVec3 p1 = pose.transform(verts[ref1]);
					const PxVec3 p2 = pose.transform(verts[ref2]);
					mRenderer.DrawTriangle(ToPoint(p0), ToPoint(p1), ToPoint(p2), color);
				}
			}
			else if(surface.mWFaces)
			{
				const PxU16* indices = surface.mWFaces;
				for(PxU32 i=0;i<surface.mNbFaces;i++)
				{
					const PxU16 ref0 = *indices++;
					const PxU16 ref1 = *indices++;
					const PxU16 ref2 = *indices++;
					const PxVec3 p0 = pose.transform(verts[ref0]);
					const PxVec3 p1 = pose.transform(verts[ref1]);
					const PxVec3 p2 = pose.transform(verts[ref2]);
					mRenderer.DrawTriangle(ToPoint(p0), ToPoint(p1), ToPoint(p2), color);
				}
			}
		}
	}

	virtual	void	renderBounds(const PxBounds3& bounds)
	{
		ASSERT(0);
	}

	PintRender&		mRenderer;
	PintRenderPass	mRenderPass;
};
#endif

void EditorPlugin::Render(PintRender& renderer, PintRenderPass render_pass)
{
#ifdef TEST_NVD
//	printf("\nRender pass: %d\n", int(render_pass));
	NvdCapture* Nvd = getNvd();
	if(Nvd && !Nvd->isCaptureEnabled())
	{
		if(render_pass==PINT_RENDER_PASS_REFLECTIONS)
			return;

		NvdDatabase* NvDb = getNvdDatabase();
		if(NvDb)
		{
			if(render_pass==PINT_RENDER_PASS_WIREFRAME_OVERLAY)
			{
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
				NvDb->render(WireframeOverlayRenderInterface(), renderer.GetFrustumPlanes());
			}
			else
			{
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				NvDb->render(PEELRenderInterface(renderer), renderer.GetFrustumPlanes());
			}
		}
		return;
	}
#endif
	if(1)
	{
		if(mEdited)
			mEdited->Render(renderer, render_pass);
		return;
	}

/*	if(0)
	{
		const udword NbActors = mEditorActors.GetNbEntries();
		for(udword i=0;i<NbActors;i++)
		{
			const ActorData* CurrentActor = HandleToActorData(mEditorActors.GetEntry(i));

			PR Pose;
			Pose.mPos = CurrentActor->mPosition;
			Pose.mRot = CurrentActor->mRotation;

			const udword NbShapes = CurrentActor->mShapes.GetNbEntries();
			for(udword j=0;j<NbShapes;j++)
			{
				const ShapeData* CurrentShape = (const ShapeData*)CurrentActor->mShapes.GetEntry(j);
				if(CurrentShape->mRenderer)
				{
					// TODO: optimize this
					PR LocalPose;
					LocalPose.mPos = CurrentShape->mLocalPos;
					LocalPose.mRot = CurrentShape->mLocalRot;

					PR ShapePose = LocalPose;
					ShapePose *= Pose;

					PintShapeRenderer* Renderer = CurrentShape->mRenderer;
					Renderer->_Render(ShapePose);
				}
			}
		}
	}*/
}

void EditorPlugin::RenderDebugData(PintRender& renderer)
{
#ifdef TEST_NVD
//	printf("EditorPlugin::RenderDebugData\n");
	NvdCapture* Nvd = getNvd();
	if(Nvd && !Nvd->isCaptureEnabled())
	{
		NvdDatabase* NvDb = getNvdDatabase();
		if(NvDb)
		{
			if(gDrawGrid)
			{
				// TODO: refactor with Grid.cpp
				const float gridSize = 200.0f;
				const float minX = -gridSize;
				const float maxX = gridSize;
				const float minZ = -gridSize;
				const float maxZ = gridSize;
				const float dX = 1.0f;
				const float dZ = 1.0f;

				const Point color(1.0f, 1.0f, 1.0f);

				float currentX = minX;
				while(currentX<=maxX)
				{
					renderer.DrawLine(Point(currentX, 0.0f, minZ), Point(currentX, 0.0f, maxZ), color);
					currentX += dX;
				}

				float currentZ = minZ;
				while(currentZ<=maxZ)
				{
					renderer.DrawLine(Point(minX, 0.0f, currentZ), Point(maxX, 0.0f, currentZ), color);
					currentZ += dZ;
				}
			}

			if(gDrawVisibleBounds)
				NvDb->renderVisibleBounds(BoundsRenderInterface(renderer));
		}
		return;
	}
#endif

	if(mEdited)
		mEdited->RenderDebugData(renderer);
//	return;

	if(gDrawSceneBounds && mEditorScene)
	{
		if(mEditorScene->mGlobalBounds.IsValid())
			renderer.DrawWireframeAABB(1, &mEditorScene->mGlobalBounds, Point(1.0f, 0.0f, 0.0f));
	}
}

PR EditorPlugin::GetWorldTransform(PintActorHandle handle)
{
	const ActorData* Data = HandleToActorData(handle);

	if(mEdited)
		return mEdited->GetWorldTransform(GetNativeActorHandle(Data));

	return PR(Data->mPosition, Data->mRotation);
}

void EditorPlugin::SetWorldTransform(PintActorHandle handle, const PR& pose)
{
	ActorData* Data = HandleToActorData(handle);

	if(mEdited)
		mEdited->SetWorldTransform(GetNativeActorHandle(Data), pose);

	Data->mPosition = pose.mPos;
	Data->mRotation = pose.mRot;
}

#ifdef DEPRECATED
const char* EditorPlugin::GetName(PintActorHandle handle)
{
	const ActorData* Data = HandleToActorData(handle);

	const char* Name = Data->mName.Get();

	if(mEdited)
	{
		Pint_Actor* API = mEdited->GetActorAPI();
		if(API)
		{
			const char* NativeName = API->GetName(GetNativeActorHandle(Data));
			if(!Name)
			{
				ASSERT(!NativeName);
			}
			else
			{
				ASSERT(strcmp(Name, NativeName)==0);
			}
		}
	}

	return Name;
}

bool EditorPlugin::SetName(PintActorHandle handle, const char* name)
{
	ActorData* Data = HandleToActorData(handle);

	Data->mName.Set(name);

	if(mEdited)
	{
		Pint_Actor* API = mEdited->GetActorAPI();
		if(API)
			return API->SetName(GetNativeActorHandle(Data), name);
	}
	return true;
}
#endif

PintConvexHandle EditorPlugin::CreateConvexObject(const PINT_CONVEX_DATA_CREATE& desc, PintConvexIndex* index)
{
	NotImplemented("CreateConvexObject");
//	if(mEdited)
//		return mEdited->CreateConvexObject(desc, index);
	return null;
}

PintMeshHandle EditorPlugin::CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)
{
	gUpdateUI = true;

	EditorMesh* Object = ICE_NEW(EditorMesh);
	//###TODO: move this to EditorMesh ctor
	Object->mExportable = true;//desc.mAddToDatabase;

	// Debatable: we set the refcount to zero for these guys, because they're going to be referenced
	// by mesh shapes later, at which point we'll increase the refcount. Unclear what's best here.
	Object->mRefCount = 0;

	const PintMeshHandle EngineHandle = mEdited ? mEdited->CreateMeshObject(desc, index) : null;
	Object->mNativeHandle = EngineHandle;

	MeshData* NewMesh = ICE_NEW(MeshData)(desc);
	NewMesh->mUserData = Object;
	mEditorMeshes.AddPtr(NewMesh);

#ifdef TEST_NVD
	NvdCapture* Nvd = getNvdForCapture();
	if(Nvd)
	{
//		Nvd->createMesh(InstanceHandle(NewMesh));
		Nvd->createInstance(InstanceHandle(NewMesh), "TriangleMesh");
//		Nvd->addLink(InstanceHandle(this), InstanceHandle(NewActor));
		const SurfaceInterface&	SI = desc.GetSurface();
		Nvd->setMeshData(InstanceHandle(NewMesh), SI.mNbVerts, reinterpret_cast<const PxVec3*>(SI.mVerts), SI.mNbFaces, SI.mDFaces, false);
	}
#endif

	return PintMeshHandle(NewMesh);
}

MaterialData* EditorPlugin::FindMaterial(const PINT_MATERIAL_CREATE& create) const
{
	struct Materials
	{
		static inline_ bool AreEqual(const MaterialData& data, const PINT_MATERIAL_CREATE& create)
		{
			// ###TODO: what about the names?
			if(data.mStaticFriction!=create.mStaticFriction)
				return false;
			if(data.mDynamicFriction!=create.mDynamicFriction)
				return false;
			if(data.mRestitution!=create.mRestitution)
				return false;
			return true;
		}
	};

	// We cannot use PINT_MATERIAL_CREATE pointers for detecting similar materials,
	// as these structures are often created on the stack and reused with different values.
	const udword Nb = mEditorMaterials.GetNbEntries();
	for(udword i=0;i<Nb;i++)
	{
		MaterialData* Current = (MaterialData*)mEditorMaterials.GetEntry(i);

		EditorMaterial* Object = (EditorMaterial*)Current->mUserData;
		ASSERT(Object->GetType()==EditorObject::MATERIAL);

		if(Object->mExportable && Materials::AreEqual(*Current, create))
			return Current;
	}
	return null;
}

const MaterialData* EditorPlugin::RegisterMaterial(const PINT_MATERIAL_CREATE& create, bool add_to_database)
{
	if(add_to_database)
		mStats.mMaterials.mNbRef++;

	MaterialData* NewMaterial = gShareData ? FindMaterial(create) : null;
	if(NewMaterial)
	{
		EditorMaterial* Object = (EditorMaterial*)NewMaterial->mUserData;
		ASSERT(Object->GetType()==EditorObject::MATERIAL);
		Object->mRefCount++;
	}
	else
	{
		if(add_to_database)
			mStats.mMaterials.mNbUnique++;

		NewMaterial = ICE_NEW(MaterialData)(create);
		mEditorMaterials.AddPtr(NewMaterial);

		EditorMaterial* Object = ICE_NEW(EditorMaterial);
		Object->mExportable = add_to_database;
		NewMaterial->mUserData = Object;
#ifdef TEST_NVD
		NvdCapture* Nvd = getNvdForCapture();
		if(Nvd)
		{
			const InstanceHandle h = InstanceHandle(NewMaterial);
			Nvd->createInstance(h, "Material");

			if(create.mName)
				Nvd->setPropertyValue_string(h, "Name", create.mName);

			Nvd->setPropertyValue_float(h, "StaticFriction", create.mStaticFriction);
			Nvd->setPropertyValue_float(h, "DynamicFriction", create.mDynamicFriction);
			Nvd->setPropertyValue_float(h, "Restitution", create.mRestitution);

			Nvd->addLink(InstanceHandle(gEditor), h, "Materials");
		}
#endif
	}
	return NewMaterial;
}

static void SetupNewShape(ShapeData* new_shape, const PINT_SHAPE_CREATE* src, const MaterialData* material, bool add_to_database)
{
	EditorShape* ShapeObject = ICE_NEW(EditorShape);
	ShapeObject->mExportable = add_to_database;
	ShapeObject->mShapeType = src->mType;
	if(new_shape->mType==PINT_SHAPE_MESH)
	{
		const MeshShapeData* MSD = static_cast<const MeshShapeData*>(new_shape);
		ShapeObject->mNbTris = MSD->mNbTris;
		ShapeObject->mNbVerts = MSD->mNbVerts;
	}
	else if(new_shape->mType==PINT_SHAPE_MESH2)
	{
		const MeshShapeData2* MSD = static_cast<const MeshShapeData2*>(new_shape);
		ShapeObject->mNbTris = MSD->mMeshData->mNbTris;
		ShapeObject->mNbVerts = MSD->mMeshData->mNbVerts;
	}
//	ShapeObject->mNativeHandle = <not available>;
	new_shape->mUserData = ShapeObject;
	new_shape->mMaterial = material;
#ifdef TEST_NVD
	NvdCapture* Nvd = getNvdForCapture();
	if(Nvd)
	{
		const InstanceHandle h = InstanceHandle(new_shape);

//		Nvd->createInstance(h, NVD_TYPE_SHAPE);
		Nvd->createInstance(h, "Shape");
//		Nvd->addLink(InstanceHandle(gEditor), h, "Shapes");

		if(material)
			Nvd->addLink(h, InstanceHandle(material));

		if(src->mName)
			Nvd->setPropertyValue_string(h, "Name", src->mName);

		Nvd->setPropertyValue_int(h, "Type", src->mType);

		Nvd->setPropertyValue_renderTransform(h, PxTransform(ToPxVec3(src->mLocalPos), ToPxQuat(src->mLocalRot)));

		// TODO: this should ideally be in the ctors in PhysicsData.cpp but hey.... I know, this is fugly stuff here
		if(new_shape->mType==PINT_SHAPE_SPHERE)
		{
			const SphereShapeData* SSD = static_cast<const SphereShapeData*>(new_shape);
			Nvd->setSphereData(h, SSD->mRadius);
		}
		else if(new_shape->mType==PINT_SHAPE_CAPSULE)
		{
			const CapsuleShapeData* CSD = static_cast<const CapsuleShapeData*>(new_shape);
			Nvd->setCapsuleData(h, CSD->mHalfHeight, CSD->mRadius, 1);
		}
		else if(new_shape->mType==PINT_SHAPE_BOX)
		{
			const BoxShapeData* BSD = static_cast<const BoxShapeData*>(new_shape);
//			Nvd->createObject(h, NVD_TYPE_SHAPE);
			Nvd->setBoxData(h, ToPxVec3(BSD->mExtents));
		}
		else if(new_shape->mType==PINT_SHAPE_MESH)
		{
			const MeshShapeData* MSD = static_cast<const MeshShapeData*>(new_shape);
			// TODO: double check all this
			if(!Nvd->instanceExists(InstanceHandle(MSD->mRenderer)))
			{
//				Nvd->createMesh(InstanceHandle(MSD->mRenderer));
				Nvd->createInstance(InstanceHandle(MSD->mRenderer), "TriangleMesh");
				Nvd->setMeshData(InstanceHandle(MSD->mRenderer),
											MSD->mNbVerts, reinterpret_cast<const PxVec3*>(MSD->mVerts),
											MSD->mNbTris, MSD->mTris, false);
			}
			Nvd->addLink(h, InstanceHandle(MSD->mRenderer));
		}
		else if(new_shape->mType==PINT_SHAPE_MESH2)
		{
			const MeshShapeData2* MSD = static_cast<const MeshShapeData2*>(new_shape);
//			Nvd->createObject(h, NVD_TYPE_SHAPE);
//			Nvd->setBoxData(h, ToPxVec3(BSD->mExtents));
			Nvd->addLink(h, InstanceHandle(MSD->mMeshData));
		}
		else if(new_shape->mType==PINT_SHAPE_CONVEX)
		{
			const ConvexShapeData* CSD = static_cast<const ConvexShapeData*>(new_shape);
			// TODO: double check all this
			if(!Nvd->instanceExists(InstanceHandle(CSD->mRenderer)))
			{
//				Nvd->createMesh(InstanceHandle(MSD->mRenderer));
				Nvd->createInstance(InstanceHandle(CSD->mRenderer), "ConvexMesh");

				// TODO: find a way to NOT recompute the hull here
//				ConvexHull2* CH = CreateConvexHull2(CSD->mNbVerts, CSD->mVerts);
					//ConvexHull2* CreateConvexHull2b(udword nb_verts, const Point* verts);
					//ConvexHull2* CH = CreateConvexHull2b(CSD->mNbVerts, CSD->mVerts);
				ConvexHull2* CH = CreateConvexHull2(CSD->mNbVerts, CSD->mVerts);	// ICE
				if(CH)
				{
					ps::Array<PxU8> vertexCounts;
					ps::Array<PxU8> indices;

					const udword NbPolys = CH->GetNbPolygons();
					for(udword i=0;i<NbPolys;i++)
					{
						const HullPolygon& polygon = CH->GetPolygon(i);
						vertexCounts.pushBack(ToByte(polygon.mNbVerts));
						for(udword j=0;j<polygon.mNbVerts;j++)
						{
							indices.pushBack(ToByte(polygon.mVRef[j]));
						}
					}

					Nvd->setConvexData(	InstanceHandle(CSD->mRenderer), CH->GetNbVerts(),
										reinterpret_cast<const PxVec3*>(CH->GetVerts()), CH->GetNbPolygons(),
										vertexCounts.begin(), indices.size(), indices.begin());

					DELETESINGLE(CH);
				}
			}
			Nvd->addLink(h, InstanceHandle(CSD->mRenderer));
		}
		else
			ASSERT(0);
	}
#endif
}

static inline_ bool AreBaseShapesEqual(const ShapeData& data, const PINT_SHAPE_CREATE& create, const MaterialData* material)
{
	// The passed MaterialData has been created from the passed PINT_SPHERE_CREATE structure already, and
	// that pointer has already been shared (potentially). So all we need to do here is compare the pointers.
	if(data.mMaterial!=material)
		return false;

	// ###TODO: what about the names?

	{
		// Necessary for cases where collision shapes don't match the rendered prim...
		if(data.mRenderer != create.mRenderer)
			return false;
	}

	if(data.mRenderer && create.mRenderer)
	{
		const RGBAColor* color0 = data.mRenderer->GetColor();
		const RGBAColor* color1 = create.mRenderer->GetColor();
		if((color0 && !color1) || (!color0 && color1))
			return false;
		if(color0 && color1)
		{
			if(*color0 != *color1)
				return false;
		}
	}

	if(data.mLocalPos!=create.mLocalPos)
		return false;
	if(data.mLocalRot!=create.mLocalRot)
		return false;
	return true;
}

template<class T0, class T1, class T2>
static T0* FindShape(const T1& create, const PtrContainer& shapes, const MaterialData* material)
{
	const udword Nb = shapes.GetNbEntries();
	for(udword i=0;i<Nb;i++)
	{
		T0* Current = (T0*)shapes.GetEntry(i);

		EditorShape* Object = (EditorShape*)Current->mUserData;
		ASSERT(Object->GetType()==EditorObject::SHAPE);

		if(Object->mExportable && T2::AreEqual(*Current, create, material))
			return Current;
	}
	return null;
}

template<class T0, class T1>
const T0* RegisterShape(T0* new_shape, const T1& create, PtrContainer& shapes, const MaterialData* material, Stats::Shared& stats, bool add_to_database)
{
	if(add_to_database)
		stats.mNbRef++;

	if(new_shape)
	{
		EditorShape* Object = (EditorShape*)new_shape->mUserData;
		ASSERT(Object->GetType()==EditorObject::SHAPE);
		Object->mRefCount++;
	}
	else
	{
		if(add_to_database)
			stats.mNbUnique++;

		new_shape = ICE_NEW(T0)(create);
		shapes.AddPtr(new_shape);

		SetupNewShape(new_shape, &create, material, add_to_database);
	}
	return new_shape;
}

SphereShapeData* EditorPlugin::FindSphereShape(const PINT_SPHERE_CREATE& create, const MaterialData* material) const
{
	struct Shapes
	{
		static inline_ bool AreEqual(const SphereShapeData& data, const PINT_SPHERE_CREATE& create, const MaterialData* material)
		{
			if(data.mRadius!=create.mRadius)
				return false;
			return AreBaseShapesEqual(data, create, material);
		}
	};

	return FindShape<SphereShapeData, PINT_SPHERE_CREATE, Shapes>(create, mEditorSphereShapes, material);
}

const SphereShapeData* EditorPlugin::RegisterSphereShape(const PINT_SPHERE_CREATE& create, const MaterialData* material, bool add_to_database)
{
	SphereShapeData* NewShape = gShareData ? FindSphereShape(create, material) : null;
	return RegisterShape(NewShape, create, mEditorSphereShapes, material, mStats.mSphereShapes, add_to_database);
}

CapsuleShapeData* EditorPlugin::FindCapsuleShape(const PINT_CAPSULE_CREATE& create, const MaterialData* material) const
{
	struct Shapes
	{
		static inline_ bool AreEqual(const CapsuleShapeData& data, const PINT_CAPSULE_CREATE& create, const MaterialData* material)
		{
			if(data.mRadius!=create.mRadius)
				return false;
			if(data.mHalfHeight!=create.mHalfHeight)
				return false;
			return AreBaseShapesEqual(data, create, material);
		}
	};

	return FindShape<CapsuleShapeData, PINT_CAPSULE_CREATE, Shapes>(create, mEditorCapsuleShapes, material);
}

const CapsuleShapeData* EditorPlugin::RegisterCapsuleShape(const PINT_CAPSULE_CREATE& create, const MaterialData* material, bool add_to_database)
{
	CapsuleShapeData* NewShape = gShareData ? FindCapsuleShape(create, material) : null;
	return RegisterShape(NewShape, create, mEditorCapsuleShapes, material, mStats.mCapsuleShapes, add_to_database);
}

CylinderShapeData* EditorPlugin::FindCylinderShape(const PINT_CYLINDER_CREATE& create, const MaterialData* material) const
{
	struct Shapes
	{
		static inline_ bool AreEqual(const CylinderShapeData& data, const PINT_CYLINDER_CREATE& create, const MaterialData* material)
		{
			if(data.mRadius!=create.mRadius)
				return false;
			if(data.mHalfHeight!=create.mHalfHeight)
				return false;
			return AreBaseShapesEqual(data, create, material);
		}
	};

	return FindShape<CylinderShapeData, PINT_CYLINDER_CREATE, Shapes>(create, mEditorCylinderShapes, material);
}

const CylinderShapeData* EditorPlugin::RegisterCylinderShape(const PINT_CYLINDER_CREATE& create, const MaterialData* material, bool add_to_database)
{
	CylinderShapeData* NewShape = gShareData ? FindCylinderShape(create, material) : null;
	return RegisterShape(NewShape, create, mEditorCylinderShapes, material, mStats.mCylinderShapes, add_to_database);
}

BoxShapeData* EditorPlugin::FindBoxShape(const PINT_BOX_CREATE& create, const MaterialData* material) const
{
	struct Shapes
	{
		static inline_ bool AreEqual(const BoxShapeData& data, const PINT_BOX_CREATE& create, const MaterialData* material)
		{
			if(data.mExtents!=create.mExtents)
				return false;
			return AreBaseShapesEqual(data, create, material);
		}
	};

	return FindShape<BoxShapeData, PINT_BOX_CREATE, Shapes>(create, mEditorBoxShapes, material);
}

const BoxShapeData* EditorPlugin::RegisterBoxShape(const PINT_BOX_CREATE& create, const MaterialData* material, bool add_to_database)
{
	BoxShapeData* NewShape = gShareData ? FindBoxShape(create, material) : null;
	return RegisterShape(NewShape, create, mEditorBoxShapes, material, mStats.mBoxShapes, add_to_database);
}

ConvexShapeData* EditorPlugin::FindConvexShape(const PINT_CONVEX_CREATE& create, const MaterialData* material) const
{
	struct Shapes
	{
		static inline_ bool AreEqual(const ConvexShapeData& data, const PINT_CONVEX_CREATE& create, const MaterialData* material)
		{
			//###TODO: this legacy shape used the renderer pointer to detect sharing, which isn't great.
			//  We could actually compare the vertex data or use a checksum instead.
			if(data.mRenderer!=create.mRenderer)
				return false;
			return AreBaseShapesEqual(data, create, material);
		}
	};

	return FindShape<ConvexShapeData, PINT_CONVEX_CREATE, Shapes>(create, mEditorConvexShapes, material);
}

const ConvexShapeData* EditorPlugin::RegisterConvexShape(const PINT_CONVEX_CREATE& create, const MaterialData* material, bool add_to_database)
{
	ConvexShapeData* NewShape = gShareData ? FindConvexShape(create, material) : null;
	return RegisterShape(NewShape, create, mEditorConvexShapes, material, mStats.mConvexShapes, add_to_database);
}

MeshShapeData* EditorPlugin::FindMeshShape(const PINT_MESH_CREATE& create, const MaterialData* material) const
{
	struct Shapes
	{
		static inline_ bool AreEqual(const MeshShapeData& data, const PINT_MESH_CREATE& create, const MaterialData* material)
		{
			//###TODO: this legacy shape used the renderer pointer to detect sharing, which isn't great.
			//  We could actually compare the vertex data or use a checksum instead.
			if(data.mRenderer!=create.mRenderer)
				return false;
			return AreBaseShapesEqual(data, create, material);
		}
	};

	return FindShape<MeshShapeData, PINT_MESH_CREATE, Shapes>(create, mEditorMeshShapes, material);
}

const MeshShapeData* EditorPlugin::RegisterMeshShape(const PINT_MESH_CREATE& create, const MaterialData* material, bool add_to_database)
{
	MeshShapeData* NewShape = gShareData ? FindMeshShape(create, material) : null;
	return RegisterShape(NewShape, create, mEditorMeshShapes, material, mStats.mMeshShapes, add_to_database);
}

MeshShapeData2* EditorPlugin::FindMeshShape2(const PINT_MESH_CREATE2& create, const MaterialData* material) const
{
	struct Shapes
	{
		static inline_ bool AreEqual(const MeshShapeData2& data, const PINT_MESH_CREATE2& create, const MaterialData* material)
		{
			if(data.mMeshData!=reinterpret_cast<MeshData*>(create.mTriangleMesh))
				return false;
			return AreBaseShapesEqual(data, create, material);
		}
	};

	return FindShape<MeshShapeData2, PINT_MESH_CREATE2, Shapes>(create, mEditorMeshShapes2, material);
}

const MeshShapeData2* EditorPlugin::RegisterMeshShape2(const PINT_MESH_CREATE2& create, const MaterialData* material, bool add_to_database)
{
	MeshShapeData2* NewShape = gShareData ? FindMeshShape2(create, material) : null;
	return RegisterShape(NewShape, create, mEditorMeshShapes2, material, mStats.mMeshShapes, add_to_database);
}

PintActorHandle EditorPlugin::CreateDummyEditorActor(PintActorHandle native_handle)
{
	// For CCT internal actors, vehicle chassis, etc

	EditorActor* Object = ICE_NEW(EditorActor);
	Object->mExportable = false;

	PINT_OBJECT_CREATE desc;
	ActorData* NewActor = ICE_NEW(ActorData)(desc);
	NewActor->mUserData = Object;
	mEditorActors.AddPtr(NewActor);

//	Object->mInitPos = NewActor->mPosition;
//	Object->mInitRot = NewActor->mRotation;

	Object->mNativeHandle = native_handle;

	return PintActorHandle(NewActor);
}

namespace
{
	struct FixupRecord
	{
		PINT_MESH_CREATE2*	mMeshCreate;
		PintMeshHandle		mOriginalHandle;
	};

	struct ShapeCreationParams
	{
		ActorData*	mActor;
		Container	mFixupRecords;
		bool		mAddToDatabase;
	};
}

void EditorPlugin::ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)
{
	ShapeCreationParams* Params = reinterpret_cast<ShapeCreationParams*>(user_data);
	const bool AddToDatabase = Params->mAddToDatabase;

	const PINT_SHAPE_CREATE* CurrentShape = &create;

	const MaterialData* NewMaterial = null;
	if(CurrentShape->mMaterial)
		NewMaterial = RegisterMaterial(*CurrentShape->mMaterial, AddToDatabase);

	ShapeData* NewShape = null;
	if(CurrentShape->mType==PINT_SHAPE_SPHERE)
	{
		NewShape = const_cast<SphereShapeData*>(RegisterSphereShape(*static_cast<const PINT_SPHERE_CREATE*>(CurrentShape), NewMaterial, AddToDatabase));
	}
	else if(CurrentShape->mType==PINT_SHAPE_CAPSULE)
	{
		NewShape = const_cast<CapsuleShapeData*>(RegisterCapsuleShape(*static_cast<const PINT_CAPSULE_CREATE*>(CurrentShape), NewMaterial, AddToDatabase));
	}
	else if(CurrentShape->mType==PINT_SHAPE_CYLINDER)
	{
/*		if(AddToDatabase)
			mStats.mNbCylinderShapes++;
		NewShape = ICE_NEW(CylinderShapeData)(*static_cast<const PINT_CYLINDER_CREATE*>(CurrentShape));
		mEditorCylinderShapes.AddPtr(NewShape);
		SetupNewShape(NewShape, CurrentShape, NewMaterial, AddToDatabase);*/
		NewShape = const_cast<CylinderShapeData*>(RegisterCylinderShape(*static_cast<const PINT_CYLINDER_CREATE*>(CurrentShape), NewMaterial, AddToDatabase));
	}
	else if(CurrentShape->mType==PINT_SHAPE_BOX)
	{
		NewShape = const_cast<BoxShapeData*>(RegisterBoxShape(*static_cast<const PINT_BOX_CREATE*>(CurrentShape), NewMaterial, AddToDatabase));
	}
	else if(CurrentShape->mType==PINT_SHAPE_CONVEX)
	{
		NewShape = const_cast<ConvexShapeData*>(RegisterConvexShape(*static_cast<const PINT_CONVEX_CREATE*>(CurrentShape), NewMaterial, AddToDatabase));
	}
	else if(CurrentShape->mType==PINT_SHAPE_MESH)
	{
		const PINT_MESH_CREATE* MeshCreate = static_cast<const PINT_MESH_CREATE*>(CurrentShape);
		if(AddToDatabase)
		{
//			mStats.mNbMeshShapes++;
			const SurfaceInterface& SI = MeshCreate->GetSurface();
			mStats.mTotalNbVerts += SI.mNbVerts;
			mStats.mTotalNbTris += SI.mNbFaces;
		}
//		NewShape = ICE_NEW(MeshShapeData)(*MeshCreate);
//		mEditorMeshShapes.AddPtr(NewShape);
//		SetupNewShape(NewShape, CurrentShape, NewMaterial, AddToDatabase);
		NewShape = const_cast<MeshShapeData*>(RegisterMeshShape(*MeshCreate, NewMaterial, AddToDatabase));
#ifdef TEST_NVD
//		NvdCapture* Nvd = getNvdForCapture();
//		if(Nvd && !Nvd->instanceExists(NewShape))
//		{
//		}
#endif
	}
	else if(CurrentShape->mType==PINT_SHAPE_MESH2)
	{
		const PINT_MESH_CREATE2* MeshCreate = static_cast<const PINT_MESH_CREATE2*>(CurrentShape);

		const MeshData* Data = (const MeshData*)MeshCreate->mTriangleMesh;
		ASSERT(Data);
		if(AddToDatabase)
		{
//			mStats.mNbMeshShapes++;
			mStats.mTotalNbVerts += Data->mNbVerts;
			mStats.mTotalNbTris += Data->mNbTris;
		}
/*
		MeshShapeData2* MSD = ICE_NEW(MeshShapeData2)(*MeshCreate);
//		MSD->mMeshData = Data;	//###do that in ctor
		NewShape = MSD;
		mEditorMeshShapes2.AddPtr(NewShape);*/
		NewShape = const_cast<MeshShapeData2*>(RegisterMeshShape2(*MeshCreate, NewMaterial, AddToDatabase));
		
		ASSERT(Data->mUserData);
		EditorMesh* Object = (EditorMesh*)Data->mUserData;
//		Object->mRefCount++;	//###TODO: this & decref on release
//		SetupNewShape(NewShape, CurrentShape, NewMaterial, AddToDatabase);

						FixupRecord* Record = ICE_RESERVE(FixupRecord, Params->mFixupRecords);
						Record->mMeshCreate	= const_cast<PINT_MESH_CREATE2*>(MeshCreate);
						Record->mOriginalHandle	= MeshCreate->mTriangleMesh;

		const_cast<PINT_MESH_CREATE2*>(MeshCreate)->mTriangleMesh = PintMeshHandle(Object->mNativeHandle);	//###TODO: undo this before returning, else can crash
	}
	else ASSERT(0);

	if(NewShape)
	{
		ActorData*NewActor = Params->mActor;
		NewActor->mShapes.AddPtr(NewShape);

#ifdef TEST_NVD
		NvdCapture* Nvd = getNvdForCapture();
		if(Nvd)
		{
			Nvd->addLink(InstanceHandle(NewActor), InstanceHandle(NewShape));
		}
#endif
	}
}

PintActorHandle EditorPlugin::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	// Discard actors with zero shapes. We could keep them though if needed.
	const udword NbShapes = desc.GetNbShapes();
	if(!NbShapes)
	{
		ASSERT(!"Found actor with no shapes?");
		return null;
	}

	gUpdateUI = true;

	EditorActor* Object = ICE_NEW(EditorActor);
	Object->mExportable = desc.mAddToDatabase;

	// Update actor-related stats
	if(desc.mAddToDatabase)
	{
		const bool IsCompound = NbShapes>1;
		Object->mIsCompound = IsCompound;
		if(desc.mMass==0.0f)
		{
			Object->mIsStatic = true;
			mStats.mNbStatics++;
			if(IsCompound)
				mStats.mNbStaticCompounds++;
		}
		else
		{
			Object->mIsStatic = false;
			mStats.mNbDynamics++;
			if(IsCompound)
				mStats.mNbDynamicCompounds++;
		}
	}

	ActorData* NewActor = ICE_NEW(ActorData)(desc);
	NewActor->mUserData = Object;
	mEditorActors.AddPtr(NewActor);

	Object->mInitPos = NewActor->mPosition;
	Object->mInitRot = NewActor->mRotation;

#ifdef TEST_NVD
	NvdCapture* Nvd = getNvdForCapture();
	if(Nvd)
	{
		const InstanceHandle h = InstanceHandle(NewActor);

//		Nvd->createInstance(h, NVD_TYPE_ACTOR);
		Nvd->createInstance(h, "Actor");

		if(desc.mName)
			Nvd->setPropertyValue_string(h, "Name", desc.mName);

		Nvd->setPropertyValue_renderTransform(h, PxTransform(ToPxVec3(desc.mPosition), ToPxQuat(desc.mRotation)));
		Nvd->setPropertyValue_float(h, "Mass", desc.mMass);
		if(desc.mAddToWorld)
			Nvd->addLink(InstanceHandle(this), h, desc.mMass==0.0f ? "Static actors" : "Dynamic actors");
	}
#endif

	ShapeCreationParams SCP;
	SCP.mActor			= NewActor;
	SCP.mAddToDatabase	= desc.mAddToDatabase;
	desc.GetNbShapes(this, &SCP);

	const PintActorHandle EngineHandle = mEdited ? mEdited->_CreateObject(desc) : null;
	Object->mNativeHandle = EngineHandle;

	// Setup native handles for shapes. Bit tricky.
	if(mEdited && EngineHandle)
	{
		Pint_Actor* API = mEdited->GetActorAPI();
		if(API)
		{
			udword NbNativeShapes = API->GetNbShapes(EngineHandle);

			if(NbNativeShapes==NewActor->mShapes.GetNbEntries())
			{
				for(udword i=0;i<NbNativeShapes;i++)
				{
					const ShapeData* SD = (const ShapeData*)NewActor->mShapes.GetEntry(i);			
					EditorShape* ShapeObject = (EditorShape*)SD->mUserData;
					if(ShapeObject)
						ShapeObject->mNativeHandle = API->GetShape(EngineHandle, i);
				}
			}
		}
	}

			const udword NbRecords = SCP.mFixupRecords.GetNbEntries()/(sizeof(FixupRecord)/sizeof(udword));
			const FixupRecord* Records = (const FixupRecord*)SCP.mFixupRecords.GetEntries();
			for(udword i=0;i<NbRecords;i++)
			{
				Records[i].mMeshCreate->mTriangleMesh = Records[i].mOriginalHandle;
			}

	return PintActorHandle(NewActor);
}

bool EditorPlugin::ReleaseObject(PintActorHandle handle)
{
	ActorData* Data = HandleToActorData(handle);

#ifdef TEST_NVD
	NvdCapture* Nvd = getNvdForCapture();
	if(Nvd)
		Nvd->releaseInstance(InstanceHandle(Data));
#endif

	if(mEdited)
		/*return*/ mEdited->_ReleaseObject(GetNativeActorHandle(Data));

/*	for(udword j=0;j<mEditorActors.GetNbEntries();j++)
	{
		ActorData* Current = HandleToActorData(mEditorActors.GetEntry(j));
		if(Current==Data)
		{
			mEditorActors.DeleteIndex(j);
			DeleteObject(Current, &mStats);
			break;
		}
	}*/
	if(mEditorActors.Delete(Data))
		DeleteActor(Data, *this, mStats);

	// TODO: do we need to purge orphan joints?

	gUpdateUI = true;
	return false;
}

PintJointHandle EditorPlugin::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	PintJointHandle EngineHandle = null;
	if(mEdited)
	{
		// To create the native joint we must replace the engine-level handles in the descriptor with native handles.
		// In particular, for all joints we need to replace the two actor handles temporarily.
		// TODO: maybe find a way to make this less ugly

		const PintActorHandle SavedActorHandle0 = desc.mObject0;
		const PintActorHandle SavedActorHandle1 = desc.mObject1;

		PINT_JOINT_CREATE& desc2 = const_cast<PINT_JOINT_CREATE&>(desc);

		const ActorData* Data0 = HandleToActorData(desc.mObject0);
		const ActorData* Data1 = HandleToActorData(desc.mObject1);
		if(Data0)
			desc2.mObject0 = GetNativeActorHandle(Data0);
		if(Data1)
			desc2.mObject1 = GetNativeActorHandle(Data1);

		// Special joints have additional handles that also need fixing
		if(desc.mType==PINT_JOINT_GEAR)
		{
			PINT_GEAR_JOINT_CREATE& gjc = static_cast<PINT_GEAR_JOINT_CREATE&>(desc2);

			const PintJointHandle SavedJointHandle0 = gjc.mHinge0;
			const PintJointHandle SavedJointHandle1 = gjc.mHinge1;

			const JointData* Hinge0 = HandleToJointData(gjc.mHinge0);
			const JointData* Hinge1 = HandleToJointData(gjc.mHinge1);
			if(Hinge0)
				gjc.mHinge0 = GetNativeJointHandle(Hinge0);
			if(Hinge1)
				gjc.mHinge1 = GetNativeJointHandle(Hinge1);

			EngineHandle = mEdited->CreateJoint(desc2);

			// We need to restore modified pointers for the rest of the function
			gjc.mHinge0 = SavedJointHandle0;
			gjc.mHinge1 = SavedJointHandle1;
		}
		else if(desc.mType==PINT_JOINT_RACK_AND_PINION)
		{
			PINT_RACK_AND_PINION_JOINT_CREATE& gjc = static_cast<PINT_RACK_AND_PINION_JOINT_CREATE&>(desc2);

			const PintJointHandle SavedJointHandle0 = gjc.mHinge;
			const PintJointHandle SavedJointHandle1 = gjc.mPrismatic;

			const JointData* Hinge = HandleToJointData(gjc.mHinge);
			const JointData* Prismatic = HandleToJointData(gjc.mPrismatic);
			if(Hinge)
				gjc.mHinge = GetNativeJointHandle(Hinge);
			if(Prismatic)
				gjc.mPrismatic = GetNativeJointHandle(Prismatic);

			EngineHandle = mEdited->CreateJoint(desc2);

			// We need to restore modified pointers for the rest of the function
			gjc.mHinge = SavedJointHandle0;
			gjc.mPrismatic = SavedJointHandle1;
		}
		else
			EngineHandle = mEdited->CreateJoint(desc2);

		// We need to restore modified pointers for the rest of the function
		desc2.mObject0 = SavedActorHandle0;
		desc2.mObject1 = SavedActorHandle1;
	}

	if(!EngineHandle)
		return null;

	mStats.mNbJoints++;

	JointData* NewJoint = null;
	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			mStats.mNbSphericalJoints++;
			const PINT_SPHERICAL_JOINT_CREATE& jc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(SphericalJointData)(jc);
		}
		break;
		case PINT_JOINT_HINGE:
		{
			mStats.mNbHingeJoints++;
			const PINT_HINGE_JOINT_CREATE& jc = static_cast<const PINT_HINGE_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(HingeJointData)(jc);
		}
		break;
		case PINT_JOINT_HINGE2:
		{
			mStats.mNbHingeJoints++;
			const PINT_HINGE2_JOINT_CREATE& jc = static_cast<const PINT_HINGE2_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(Hinge2JointData)(jc);
		}
		break;
		case PINT_JOINT_PRISMATIC:
		{
			mStats.mNbPrismaticJoints++;
			const PINT_PRISMATIC_JOINT_CREATE& jc = static_cast<const PINT_PRISMATIC_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(PrismaticJointData)(jc);
		}
		break;
		case PINT_JOINT_FIXED:
		{
			mStats.mNbFixedJoints++;
			const PINT_FIXED_JOINT_CREATE& jc = static_cast<const PINT_FIXED_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(FixedJointData)(jc);
		}
		break;
		case PINT_JOINT_DISTANCE:
		{
			mStats.mNbDistanceJoints++;
			const PINT_DISTANCE_JOINT_CREATE& jc = static_cast<const PINT_DISTANCE_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(DistanceJointData)(jc);
		}
		break;
		case PINT_JOINT_D6:
		{
			mStats.mNbD6Joints++;
			const PINT_D6_JOINT_CREATE& jc = static_cast<const PINT_D6_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(D6JointData)(jc);
		}
		break;

		case PINT_JOINT_GEAR:
		{
			mStats.mNbGearJoints++;
			const PINT_GEAR_JOINT_CREATE& jc = static_cast<const PINT_GEAR_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(GearJointData)(jc);
		}
		break;

		case PINT_JOINT_RACK_AND_PINION:
		{
			mStats.mNbRackJoints++;
			const PINT_RACK_AND_PINION_JOINT_CREATE& jc = static_cast<const PINT_RACK_AND_PINION_JOINT_CREATE&>(desc);
			NewJoint = ICE_NEW(RackJointData)(jc);
		}
		break;

		case PINT_JOINT_PORTAL:
		{
			mStats.mNbPortalJoints++;
		}
		break;
	}

	ASSERT(NewJoint);

	if(NewJoint)
	{
		EditorJoint* Object = ICE_NEW(EditorJoint);
		Object->mNativeHandle = EngineHandle;
		NewJoint->mUserData = Object;

		mEditorJoints.AddPtr(NewJoint);

		ActorData* Data0 = HandleToActorData(desc.mObject0);
		ActorData* Data1 = HandleToActorData(desc.mObject1);
		if(Data0)
			Data0->mJoints.AddPtr(NewJoint);
		if(Data1)
			Data1->mJoints.AddPtr(NewJoint);

#ifdef TEST_NVD
		NvdCapture* Nvd = getNvdForCapture();
		if(Nvd)
		{
			const InstanceHandle h = InstanceHandle(NewJoint);

			Nvd->createInstance(h, "Joint");

			if(desc.mName)
				Nvd->setPropertyValue_string(h, "Name", desc.mName);

			Nvd->setPropertyValue_int(h, "Type", desc.mType);

			//Nvd->setPropertyValue_renderTransform(InstanceHandle(NewActor), PxTransform(ToPxVec3(desc.mPosition), ToPxQuat(desc.mRotation)));
			Nvd->addLink(InstanceHandle(this), h, "Joints");
			Nvd->addLink(h, InstanceHandle(desc.mObject0));
			Nvd->addLink(h, InstanceHandle(desc.mObject1));

//			Nvd->addLink(InstanceHandle(desc.mObject0), h);
//			Nvd->addLink(InstanceHandle(desc.mObject1), h);
		}
#endif
	}

	gUpdateUI = true;
	return PintJointHandle(NewJoint);
}

bool EditorPlugin::ReleaseJoint(PintJointHandle handle)
{
	ASSERT(handle);
	JointData* Data = HandleToJointData(handle);

#ifdef TEST_NVD
	ASSERT(0);
#endif

	if(mEdited)
		/*return*/ mEdited->ReleaseJoint(GetNativeJointHandle(Data));

	// If Data is not null then we must have created "NewJoint" and it must be in mEditorJoints
	bool Found = mEditorJoints.Delete(Data);
	ASSERT(Found);

	mStats.mNbJoints--;
	switch(Data->mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			mStats.mNbSphericalJoints--;
		}
		break;
		case PINT_JOINT_HINGE:
		{
			mStats.mNbHingeJoints--;
		}
		break;
		case PINT_JOINT_HINGE2:
		{
			mStats.mNbHingeJoints--;
		}
		break;
		case PINT_JOINT_PRISMATIC:
		{
			mStats.mNbPrismaticJoints--;
		}
		break;
		case PINT_JOINT_FIXED:
		{
			mStats.mNbFixedJoints--;
		}
		break;
		case PINT_JOINT_DISTANCE:
		{
			mStats.mNbDistanceJoints--;
		}
		break;
		case PINT_JOINT_D6:
		{
			mStats.mNbD6Joints--;
		}
		break;

		case PINT_JOINT_GEAR:
		{
			mStats.mNbGearJoints--;
		}
		break;

		case PINT_JOINT_RACK_AND_PINION:
		{
			mStats.mNbRackJoints--;
		}
		break;

		case PINT_JOINT_PORTAL:
		{
			mStats.mNbPortalJoints--;
		}
		break;
	}

	ActorData* ActorData0 = Data->mObject0;
	if(ActorData0)
		ActorData0->mJoints.Delete(Data);
	ActorData* ActorData1 = Data->mObject1;
	if(ActorData1)
		ActorData1->mJoints.Delete(Data);

	if(Data->mUserData)
	{
		EditorJoint* Object = reinterpret_cast<EditorJoint*>(Data->mUserData);
		ASSERT(Object->GetType()==EditorObject::JOINT);
		DELETESINGLE(Object);
	}

	DELETESINGLE(Data);

	gUpdateUI = true;
	return false;
}


/*

PintArticHandle StatsPint::CreateArticulation(const PINT_ARTICULATION_CREATE&)
{
	mStats.mNbArticulations++;
	mUpdateUI = true;
	return null;
}

PintActorHandle StatsPint::CreateArticulatedObject(const PINT_OBJECT_CREATE&, const PINT_ARTICULATED_BODY_CREATE&, PintArticHandle articulation)
{
	mStats.mNbDynamics++;
	mUpdateUI = true;
	return null;
}

*/

#ifdef DEPRECATED
bool EditorPlugin::GetBounds(PintActorHandle handle, AABB& bounds)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
	{
		Pint_Actor* API = mEdited->GetActorAPI();
		if(API)
			return API->GetBounds(GetNativeActorHandle(Data), bounds);
	}

	return false;
}
#endif

#ifdef TOSEE
void EditorPlugin::UpdateNonProfiled(float dt)
{
	if(mEdited)
		mEdited->UpdateNonProfiled(dt);
}
#endif

bool EditorPlugin::SetSQFlag(PintActorHandle actor, bool flag)
{
	if(!mEdited)
		return false;
	const ActorData* Data = HandleToActorData(actor);
	return mEdited->SetSQFlag(GetNativeActorHandle(Data), flag);
}

/*bool EditorPlugin::SetSQFlag(PintActorHandle actor, PintShapeHandle shape, bool flag)
{
}*/

bool EditorPlugin::ResetSQFilters()
{
	return mEdited ? mEdited->ResetSQFilters() : false;
}

PintActorHandle EditorPlugin::RemapActorHandle(PintActorHandle native_handle) const
{
	if(!native_handle)
		return null;

	// TODO: optimize that loop
	for(udword j=0;j<mEditorActors.GetNbEntries();j++)
	{
		const ActorData* CurrentActor = HandleToActorData(mEditorActors.GetEntry(j));
		if(GetNativeActorHandle(CurrentActor)==native_handle)
			return PintActorHandle(CurrentActor);
	}

	ASSERT(!"Engine handle not found in internal structure");
	return null;
}

PintActorHandle EditorPlugin::RemapActorAndShapeHandles(PintActorHandle native_handle, PintShapeHandle& shape_handle) const
{
	if(!native_handle)
		return null;

	// TODO: optimize that loop
	for(udword j=0;j<mEditorActors.GetNbEntries();j++)
	{
		const ActorData* CurrentActor = HandleToActorData(mEditorActors.GetEntry(j));
		if(GetNativeActorHandle(CurrentActor)==native_handle)
		{
			if(0 && shape_handle)
			{
				bool FoundShape = false;
				const udword NbShapes = CurrentActor->mShapes.GetNbEntries();
				for(udword i=0;i<NbShapes;i++)
				{
					const ShapeData* CurrentShape = HandleToShapeData(CurrentActor->mShapes.GetEntry(i));
					if(GetNativeShapeHandle(CurrentShape)==shape_handle)
					{
						shape_handle = PintShapeHandle(CurrentShape);
						FoundShape = true;
						break;
					}
				}
				ASSERT(FoundShape && "Engine handle not found in internal structure");
			}
			return PintActorHandle(CurrentActor);
		}
	}

	ASSERT(!"Engine handle not found in internal structure");
	return null;
}

udword EditorPlugin::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	if(!mEdited)
		return 0;

	const udword Nb = mEdited->BatchRaycasts(context, nb, dest, raycasts);
	for(udword i=0;i<Nb;i++)
	{
/*		PintActorHandle EngineHandle = dest[i].mTouchedActor;
		if(!EngineHandle)
			continue;
		// TODO: optimize that loop
		for(udword j=0;j<mEditorActors.GetNbEntries();j++)
		{
			ActorData* CurrentActor = HandleToActorData(mEditorActors.GetEntry(j));
			if(GetNativeHandle(CurrentActor)==EngineHandle)
			{
				dest[i].mTouchedActor = PintActorHandle(CurrentActor);
				break;
			}
		}
		ASSERT(dest[i].mTouchedActor!=EngineHandle);	// this can now happen for "default environment" plane
		*/
		dest[i].mTouchedActor = RemapActorAndShapeHandles(dest[i].mTouchedActor, dest[i].mTouchedShape);
	}
	return Nb;
}

#ifdef TOSEE
udword EditorPlugin::BatchRaycastAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)
{
	if(mEdited)
		return mEdited->BatchRaycastAny(context, nb, dest, raycasts);
	return 0;
}

udword EditorPlugin::BatchRaycastAll(PintSQThreadContext context, udword nb, PintMultipleHits* dest, const PintRaycastData* raycasts)
{
	if(mEdited)
		return mEdited->BatchRaycastAll(context, nb, dest, raycasts);
	return 0;
}

udword EditorPlugin::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	if(mEdited)
		return mEdited->BatchBoxSweeps(context, nb, dest, sweeps);
	return 0;
}

udword EditorPlugin::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	if(mEdited)
		return mEdited->BatchSphereSweeps(context, nb, dest, sweeps);
	return 0;
}
#endif

udword EditorPlugin::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	if(!mEdited)
		return 0;

	const udword Nb = mEdited->BatchCapsuleSweeps(context, nb, dest, sweeps);
	for(udword i=0;i<Nb;i++)
		dest[i].mTouchedActor = RemapActorAndShapeHandles(dest[i].mTouchedActor, dest[i].mTouchedShape);
	return Nb;
}

#ifdef TOSEE
udword EditorPlugin::BatchSphereOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->BatchSphereOverlapAny(context, nb, dest, overlaps);
	return 0;
}

udword EditorPlugin::BatchSphereOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, const PintSphereOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->BatchSphereOverlapObjects(context, nb, dest, overlaps);
	return 0;
}

udword EditorPlugin::BatchBoxOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->BatchBoxOverlapAny(context, nb, dest, overlaps);
	return 0;
}

udword EditorPlugin::BatchBoxOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, const PintBoxOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->BatchBoxOverlapObjects(context, nb, dest, overlaps);
	return 0;
}

udword EditorPlugin::BatchCapsuleOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->BatchCapsuleOverlapAny(context, nb, dest, overlaps);
	return 0;
}

udword EditorPlugin::BatchCapsuleOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, const PintCapsuleOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->BatchCapsuleOverlapObjects(context, nb, dest, overlaps);
	return 0;
}

udword EditorPlugin::FindTriangles_MeshSphereOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintSphereOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->FindTriangles_MeshSphereOverlap(context, handle, nb, overlaps);
	return 0;
}

udword EditorPlugin::FindTriangles_MeshBoxOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintBoxOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->FindTriangles_MeshBoxOverlap(context, handle, nb, overlaps);
	return 0;
}

udword EditorPlugin::FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintCapsuleOverlapData* overlaps)
{
	if(mEdited)
		return mEdited->FindTriangles_MeshCapsuleOverlap(context, handle, nb, overlaps);
	return 0;
}
#endif

udword EditorPlugin::FindTriangles_MeshMeshOverlap(PintSQThreadContext context, PintActorHandle handle0, PintActorHandle handle1, Container& results)
{
	if(!mEdited)
		return 0;

	const ActorData* Data0 = HandleToActorData(handle0);
	const ActorData* Data1 = HandleToActorData(handle1);

	return mEdited->FindTriangles_MeshMeshOverlap(context, GetNativeActorHandle(Data0), GetNativeActorHandle(Data1), results);
}

void EditorPlugin::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		mEdited->AddWorldImpulseAtWorldPos(GetNativeActorHandle(Data), world_impulse, world_pos);
}

void EditorPlugin::AddLocalTorque(PintActorHandle handle, const Point& local_torque)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		mEdited->AddLocalTorque(GetNativeActorHandle(Data), local_torque);
}

Point EditorPlugin::GetLinearVelocity(PintActorHandle handle)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		return mEdited->GetLinearVelocity(GetNativeActorHandle(Data));
	return Point(0.0f, 0.0f, 0.0f);	// TODO: what should we return here?
}

void EditorPlugin::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		mEdited->SetLinearVelocity(GetNativeActorHandle(Data), linear_velocity);
}

Point EditorPlugin::GetAngularVelocity(PintActorHandle handle)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		return mEdited->GetAngularVelocity(GetNativeActorHandle(Data));
	return Point(0.0f, 0.0f, 0.0f);	// TODO: what should we return here?
}

void EditorPlugin::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		mEdited->SetAngularVelocity(GetNativeActorHandle(Data), angular_velocity);
}


#ifdef TOSEE

udword EditorPlugin::GetShapes(PintActorHandle* shapes, PintActorHandle handle)
{
	if(mEdited)
		return mEdited->GetShapes(shapes, handle);
	return 0;
}

void EditorPlugin::SetLocalRot(PintActorHandle handle, const Quat& q)
{
	if(mEdited)
		mEdited->SetLocalRot(handle, q);
}
#endif

#ifdef DEPRECATED
float EditorPlugin::GetLinearDamping(PintActorHandle handle)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		return mEdited->GetLinearDamping(GetNativeActorHandle(Data));
	return 0.0f;	// TODO: what should we return here?
}

float EditorPlugin::GetAngularDamping(PintActorHandle handle)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		return mEdited->GetAngularDamping(GetNativeActorHandle(Data));
	return 0.0f;	// TODO: what should we return here?
}

void EditorPlugin::SetLinearDamping(PintActorHandle handle, float damping)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		mEdited->SetLinearDamping(GetNativeActorHandle(Data), damping);
}

void EditorPlugin::SetAngularDamping(PintActorHandle handle, float damping)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		mEdited->SetAngularDamping(GetNativeActorHandle(Data), damping);
}
#endif

bool EditorPlugin::SetKinematicPose(PintActorHandle handle, const Point& pos)
{
	ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		return mEdited->SetKinematicPose(GetNativeActorHandle(Data), pos);
	return false;
}

bool EditorPlugin::SetKinematicPose(PintActorHandle handle, const PR& pr)
{
	ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		return mEdited->SetKinematicPose(GetNativeActorHandle(Data), pr);
	return false;
}

bool EditorPlugin::IsKinematic(PintActorHandle handle)
{
	const ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		return mEdited->IsKinematic(GetNativeActorHandle(Data));
	return false;
}

bool EditorPlugin::EnableKinematic(PintActorHandle handle, bool flag)
{
	ActorData* Data = HandleToActorData(handle);
	if(mEdited)
		return mEdited->EnableKinematic(GetNativeActorHandle(Data), flag);
	return false;
}

PintSQThreadContext EditorPlugin::CreateSQThreadContext()
{
	if(mEdited)
		return mEdited->CreateSQThreadContext();
	return null;
}

void EditorPlugin::ReleaseSQThreadContext(PintSQThreadContext context)
{
	if(mEdited)
		mEdited->ReleaseSQThreadContext(context);
}

#ifdef TOSEE
udword EditorPlugin::BatchConvexSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintConvexSweepData* sweeps)
{
	if(mEdited)
		return mEdited->BatchConvexSweeps(context, nb, dest, sweeps);
	return 0;
}
#endif

PintAggregateHandle EditorPlugin::CreateAggregate(udword max_size, bool enable_self_collision)
{
	gUpdateUI = true;
	mStats.mNbAggregates++;

	EditorAggregate* Object = ICE_NEW(EditorAggregate);

	const PintAggregateHandle EngineHandle = mEdited ? mEdited->CreateAggregate(max_size, enable_self_collision) : null;
	Object->mNativeHandle = EngineHandle;

	AggregateData* NewAggregate = ICE_NEW(AggregateData)(max_size, enable_self_collision);
	NewAggregate->mUserData = Object;
	mEditorAggregates.AddPtr(NewAggregate);

#ifdef TEST_NVD
	NvdCapture* Nvd = getNvdForCapture();
	if(Nvd)
	{
		const InstanceHandle h = InstanceHandle(NewAggregate);

		Nvd->createInstance(h, "Aggregate");

		Nvd->setPropertyValue_int(h, "MaxSize", max_size);
		Nvd->setPropertyValue_bool(h, "SelfCollisions", enable_self_collision);
	}
#endif
	return PintAggregateHandle(NewAggregate);
}

bool EditorPlugin::AddToAggregate(PintActorHandle object, PintAggregateHandle aggregate)
{
	AggregateData* Aggregate = reinterpret_cast<AggregateData*>(aggregate);
	const ActorData* Actor = HandleToActorData(object);

	PintAggregateHandle NativeAggregate = PintAggregateHandle(GetNativeHandle(Aggregate));
	PintActorHandle NativeActor = GetNativeActorHandle(Actor);

	if(mEdited && mEdited->AddToAggregate(NativeActor, NativeAggregate))
	{
		Aggregate->mActors.AddPtr(Actor);
#ifdef TEST_NVD
		NvdCapture* Nvd = getNvdForCapture();
		if(Nvd)
		{
			Nvd->addLink(InstanceHandle(Aggregate), InstanceHandle(Actor));
		}
#endif
		return true;
	}
	return false;
}

bool EditorPlugin::AddAggregateToScene(PintAggregateHandle aggregate)
{
	AggregateData* Aggregate = reinterpret_cast<AggregateData*>(aggregate);
	PintAggregateHandle NativeAggregate = PintAggregateHandle(GetNativeHandle(Aggregate));

	Aggregate->mAddedToScene = true;

	if(mEdited && mEdited->AddAggregateToScene(NativeAggregate))
	{
#ifdef TEST_NVD
		NvdCapture* Nvd = getNvdForCapture();
		if(Nvd)
		{
			const InstanceHandle h = InstanceHandle(Aggregate);
			Nvd->addLink(InstanceHandle(this), h, "Aggregates");
		}
#endif
		return true;
	}
	return false;
}

#ifdef TOSEE
PintArticHandle EditorPlugin::CreateArticulation(const PINT_ARTICULATION_CREATE& desc)
{
	if(mEdited)
		return mEdited->CreateArticulation(desc);
	return 0;
}

PintActorHandle EditorPlugin::CreateArticulatedObject(const PINT_OBJECT_CREATE& oc, const PINT_ARTICULATED_BODY_CREATE& bc, PintArticHandle articulation)
{
	if(mEdited)
		return mEdited->CreateArticulatedObject(oc, bc, articulation);
	return 0;
}

bool EditorPlugin::AddArticulationToScene(PintArticHandle articulation)
{
	if(mEdited)
		return mEdited->AddArticulationToScene(articulation);
	return false;
}

void EditorPlugin::SetArticulatedMotor(PintActorHandle object, const PINT_ARTICULATED_MOTOR_CREATE& motor)
{
	if(mEdited)
		mEdited->SetArticulatedMotor(object, motor);
}

PintArticHandle EditorPlugin::CreateRCArticulation(const PINT_RC_ARTICULATION_CREATE& create)
{
	if(mEdited)
		return mEdited->CreateRCArticulation(create);
	return 0;
}

PintActorHandle EditorPlugin::CreateRCArticulatedObject(const PINT_OBJECT_CREATE& oc, const PINT_RC_ARTICULATED_BODY_CREATE& bc, PintArticHandle articulation)
{
	if(mEdited)
		return mEdited->CreateRCArticulatedObject(oc, bc, articulation);
	return 0;
}

bool EditorPlugin::AddRCArticulationToScene(PintArticHandle articulation)
{
	if(mEdited)
		return mEdited->AddRCArticulationToScene(articulation);
	return false;
}

bool EditorPlugin::AddRCArticulationToAggregate(PintArticHandle articulation, PintActorHandle aggregate)
{
	if(mEdited)
		return mEdited->AddRCArticulationToAggregate(articulation, aggregate);
	return false;
}

bool EditorPlugin::SetRCADriveEnabled(PintActorHandle handle, bool flag)
{
	if(mEdited)
		return mEdited->SetRCADriveEnabled(handle, flag);
	return false;
}

bool EditorPlugin::SetRCADriveVelocity(PintActorHandle handle, float velocity)
{
	if(mEdited)
		return mEdited->SetRCADriveVelocity(handle, velocity);
	return false;
}
#endif

#ifdef NOT_WORKING
PintVehicleHandle EditorPlugin::CreateVehicle(PintVehicleData& data, const PINT_VEHICLE_CREATE& vehicle)
{
	mStats.mNbVehicles++;
	gUpdateUI = true;

	// TODO: corresponding editor class
	// BUG: same issue as for CCT, the chassis object misses a wrapper

	if(mEdited)
		return mEdited->CreateVehicle(data, vehicle);
	return 0;
}

void EditorPlugin::SetVehicleInput(PintVehicleHandle vehicle, const PINT_VEHICLE_INPUT& input)
{
	if(mEdited)
		mEdited->SetVehicleInput(vehicle, input);
}

bool EditorPlugin::GetVehicleInfo(PintVehicleHandle vehicle, PintVehicleInfo& info)
{
	if(mEdited)
		return mEdited->GetVehicleInfo(vehicle, info);
	return false;
}
#endif

bool EditorPlugin::SetDriveEnabled(PintJointHandle handle, bool flag)
{
	if(mEdited)
	{
		const JointData* JD = HandleToJointData(handle);
		const PintJointHandle h = GetNativeJointHandle(JD);
		return mEdited->SetDriveEnabled(h, flag);
	}
	return false;
}

bool EditorPlugin::SetDrivePosition(PintJointHandle handle, const PR& pose)
{
	if(mEdited)
	{
		const JointData* JD = HandleToJointData(handle);
		const PintJointHandle h = GetNativeJointHandle(JD);
		return mEdited->SetDrivePosition(h, pose);
	}
	return false;
}

bool EditorPlugin::SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)
{
	if(mEdited)
	{
		const JointData* JD = HandleToJointData(handle);
		const PintJointHandle h = GetNativeJointHandle(JD);
		return mEdited->SetDriveVelocity(h, linear, angular);
	}
	return false;
}

PR EditorPlugin::GetDrivePosition(PintJointHandle handle)
{
	if(mEdited)
	{
		const JointData* JD = HandleToJointData(handle);
		const PintJointHandle h = GetNativeJointHandle(JD);
		return mEdited->GetDrivePosition(h);
	}
	return PR(Idt);
}

#ifdef TOSEE
void EditorPlugin::SetGearJointError(PintJointHandle handle, float error)
{
	if(mEdited)
		return mEdited->SetGearJointError(handle, error);
}
#endif

Pint_Scene* EditorPlugin::GetSceneAPI()
{
	return &mSceneAPI;
}

Pint_Actor* EditorPlugin::GetActorAPI()
{
	return &mActorAPI;
}

Pint_Shape* EditorPlugin::GetShapeAPI()
{
	return &mShapeAPI;
}

Pint_Joint* EditorPlugin::GetJointAPI()
{
	return &mJointAPI;
}

Pint_Vehicle* EditorPlugin::GetVehicleAPI()
{
	return &mVehicleAPI;
}

Pint_Character* EditorPlugin::GetCharacterAPI()
{
	return &mCharacterAPI;
}



#ifdef TOSEE
void EditorPlugin::TestNewFeature()
{
	if(mEdited)
		return mEdited->TestNewFeature();
}
#endif



static void gEditor_GetOptionsFromGUI();

void Editor_Init(const PINT_WORLD_CREATE& desc)
{
#ifdef TEST_NVD
	initNvd();
#endif

	if(mEditedPlugin)
	{
		mEditedPlugin->Init(desc);
		mEdited = mEditedPlugin->GetPint();
	}

	gEditor_GetOptionsFromGUI();

	ASSERT(!gEditor);
	gEditor = ICE_NEW(EditorPlugin);
	gEditor->Init(desc);

#ifdef TEST_NVD
	RegisterNVDClasses();

	NvdCapture* Nvd = getNvdForCapture();
	if(Nvd)
	{
		Nvd->createInstance(InstanceHandle(gEditor), "PEEL Scene");
		Nvd->setPropertyValue_string(InstanceHandle(gEditor), "Name", desc.GetTestName());
		Nvd->setPropertyValue_vec3(InstanceHandle(gEditor), "Gravity", ToPxVec3(desc.mGravity));
		Nvd->setPropertyValue_int(InstanceHandle(gEditor), "NbSimulateCallsPerFrame", desc.mNbSimulateCallsPerFrame);
		Nvd->setPropertyValue_float(InstanceHandle(gEditor), "Timestep", desc.mTimestep);
		Nvd->setPropertyValue_bool(InstanceHandle(gEditor), "CreateDefaultEnvironment", desc.mCreateDefaultEnvironment);
	}
#endif
}

void Editor_Close()
{
	if(mEditedPlugin)
		mEditedPlugin->Close();

	if(gEditor)
	{
		gEditor->Close();
		delete gEditor;
		gEditor = null;
	}

#ifdef TEST_NVD
	closeNvd();
#endif
}

EditorPlugin* GetEditor()
{
	return gEditor;
}

///////////////////////////////////////////////////////////////////////////////

static Widgets*		gEditorGUI = null;
static IceCheckBox*	gCheckBox_DrawSceneBounds = null;
static IceEditBox*	gEditBox_Stats = null;

void EditorPlugin::UpdateUI()
{
/*	if(gCheckBox_DrawSceneBounds)
		gCheckBox_DrawSceneBounds->SetEnabled(gEnableEditor);
	if(gEditBox_Stats)
		gEditBox_Stats->SetEnabled(gEnableEditor);*/

	if(gCheckBox_DrawSceneBounds && mEditorScene)
	{
		if(!mEditorScene->mGlobalBounds.IsValid())
		{
			gCheckBox_DrawSceneBounds->SetEnabled(false);
			gCheckBox_DrawSceneBounds->SetChecked(false);
		}
		else
		{
			gCheckBox_DrawSceneBounds->SetEnabled(true);
		}
	}

	if(gEditBox_Stats)
	{
		CustomArray CA;
		if(mEditorScene)
		{
			const Stats& stats = mStats;
			const udword TotalNbActors =	stats.mNbStatics + stats.mNbDynamics;
			const udword TotalNbUniqueShapes =	stats.mBoxShapes.mNbUnique
										+	stats.mSphereShapes.mNbUnique
										+	stats.mCapsuleShapes.mNbUnique
										+	stats.mCylinderShapes.mNbUnique
										+	stats.mConvexShapes.mNbUnique
										+	stats.mMeshShapes.mNbUnique;

			const udword TotalNbRefShapes =	stats.mBoxShapes.mNbRef
										+	stats.mSphereShapes.mNbRef
										+	stats.mCapsuleShapes.mNbRef
										+	stats.mCylinderShapes.mNbRef
										+	stats.mConvexShapes.mNbRef
										+	stats.mMeshShapes.mNbRef;

			CA.StoreASCII(_F("Gravity:   %.4f  |  %.4f  |  %.4f\n\n", mEditorScene->mGravity.x, mEditorScene->mGravity.y, mEditorScene->mGravity.z));
			CA.StoreASCII(_F("%d unique materials (%d referenced)\n", stats.mMaterials.mNbUnique, stats.mMaterials.mNbRef));
			CA.StoreASCII(_F("%d aggregates\n", stats.mNbAggregates));
			CA.StoreASCII(_F("%d articulations\n", stats.mNbArticulations));
			CA.StoreASCII(_F("%d vehicles\n", stats.mNbVehicles));
			CA.StoreASCII(_F("%d characters\n", stats.mNbCharacters));
			CA.StoreASCII(_F("\n%d actors:\n", TotalNbActors));
			CA.StoreASCII(_F("    %d static actors:\n", stats.mNbStatics));
			CA.StoreASCII(_F("        %d singles / %d compounds\n", stats.mNbStatics - stats.mNbStaticCompounds, stats.mNbStaticCompounds));
			CA.StoreASCII(_F("    %d dynamic actors:\n", stats.mNbDynamics));
			CA.StoreASCII(_F("        %d singles / %d compounds\n", stats.mNbDynamics - stats.mNbDynamicCompounds, stats.mNbDynamicCompounds));
			CA.StoreASCII(_F("\n%d joints:\n", stats.mNbJoints));
			CA.StoreASCII(_F("    %d spherical joints\n", stats.mNbSphericalJoints));
			CA.StoreASCII(_F("    %d hinge joints\n", stats.mNbHingeJoints));
			CA.StoreASCII(_F("    %d prismatic joints\n", stats.mNbPrismaticJoints));
			CA.StoreASCII(_F("    %d fixed joints\n", stats.mNbFixedJoints));
			CA.StoreASCII(_F("    %d distance joints\n", stats.mNbDistanceJoints));
			CA.StoreASCII(_F("    %d D6 joints\n", stats.mNbD6Joints));
			CA.StoreASCII(_F("    %d gear joints\n", stats.mNbGearJoints));
			CA.StoreASCII(_F("    %d rack joints\n", stats.mNbRackJoints));
			CA.StoreASCII(_F("    %d portal joints\n", stats.mNbPortalJoints));
			CA.StoreASCII(_F("\n%d unique shapes: (%d referenced)\n", TotalNbUniqueShapes, TotalNbRefShapes));
			CA.StoreASCII(_F("    %d unique box shapes (%d referenced)\n", stats.mBoxShapes.mNbUnique, stats.mBoxShapes.mNbRef));
			CA.StoreASCII(_F("    %d unique sphere shapes (%d referenced)\n", stats.mSphereShapes.mNbUnique, stats.mSphereShapes.mNbRef));
			CA.StoreASCII(_F("    %d unique capsule shapes (%d referenced)\n", stats.mCapsuleShapes.mNbUnique, stats.mCapsuleShapes.mNbRef));
			CA.StoreASCII(_F("    %d unique cylinder shapes (%d referenced)\n", stats.mCylinderShapes.mNbUnique, stats.mCylinderShapes.mNbRef));
			CA.StoreASCII(_F("    %d unique convex shapes (%d referenced)\n", stats.mConvexShapes.mNbUnique, stats.mConvexShapes.mNbRef));
			CA.StoreASCII(_F("    %d unique mesh shapes (%d referenced)\n", stats.mMeshShapes.mNbUnique, stats.mMeshShapes.mNbRef));
			CA.StoreASCII(_F("        %d total nb verts\n", stats.mTotalNbVerts));
			CA.StoreASCII(_F("        %d total nb tris\n", stats.mTotalNbTris));

			CA.StoreASCII(_F("\n%d mesh objects\n", mEditorMeshes.GetNbEntries()));
		}
		CA.Store(ubyte(0));

		gEditBox_Stats->SetText((const char*)CA.Collapse());
	}
}

enum EditorGUIElement
{
	EDITOR_GUI_MAIN,
	EDITOR_GUI_DRAW_SCENE_BOUNDS,
	EDITOR_GUI_STATS,
#ifdef TEST_NVD
	EDITOR_GUI_CURRENT_FRAME,
	EDITOR_GUI_STOP_CAPTURE,
	EDITOR_GUI_LOAD_CAPTURE,

	EDITOR_GUI_DRAW_VISIBLE_BOUNDS,
	EDITOR_GUI_DRAW_GRID,
	EDITOR_GUI_OBJECT_NAMES_TREEVIEW,
	EDITOR_GUI_DISPLAY_INTERNAL_DATA,
#endif
};

static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
{
	const udword id = check_box.GetID();
	switch(id)
	{
		case EDITOR_GUI_DRAW_SCENE_BOUNDS:
			gDrawSceneBounds = checked;
			gUpdateUI = true;
			break;
#ifdef TEST_NVD
		case EDITOR_GUI_DRAW_VISIBLE_BOUNDS:
			gDrawVisibleBounds = checked;
			gUpdateUI = true;
			break;
		case EDITOR_GUI_DRAW_GRID:
			gDrawGrid = checked;
			gUpdateUI = true;
			break;
		case EDITOR_GUI_OBJECT_NAMES_TREEVIEW:
			gUseObjectNamesInTreeView = checked;
			gUpdateUI = true;
			break;
		case EDITOR_GUI_DISPLAY_INTERNAL_DATA:
			gDisplayInternalData = checked;
			gUpdateUI = true;
			break;
#endif
	}
}

static void gEditor_GetOptionsFromGUI()
{
}

static const char* gTooltip_DrawSceneBounds		= "Visualize scene bounds passed by each test to plug-in's init function";

void ExportScene(String* string)
{
	EditorPlugin* Editor = static_cast<EditorPlugin*>(GetEditorPlugin()->GetPint());
	ASSERT(Editor);

	SceneData* EditorSceneData = Editor->GetEditorSceneData();
	if(!EditorSceneData)
		return;

	// TODO: move this to ICE's import-export plugin mechanism

	if(0)
	{
		ExportUSD(*Editor, null);
		return;
	}

	FILESELECTCREATE Create;
	Create.mFilter			= "ZB2 Files (*.zb2)|*.zb2|USDA Files (*.usda)|*.usda|All Files (*.*)|*.*||";
//	Create.mFileName		= Editor->mEditorScene->mName;
	Create.mFileName		= GetFilenameForExport("zb2");
	Create.mInitialDir		= "";
	Create.mCaptionTitle	= "Export scene";
	Create.mDefExt			= "zb2";

	String Filename;
	if(!FileselectSave(Create, Filename, true))
		return;

	const char* Ext = GetExtension(Filename);
	if(!Ext)
	{
		IceCore::MessageBox(0, "Unknown file format. Cannot export.", "Error", MB_OK);
		return;
	}

	if(strcmp(Ext, "usda")==0)
	{
		ExportUSD(*Editor, &Filename);
		return;
	}

	// Update scene desc, which might have been modified
	if(EditorSceneData)
		UI_GetSceneDesc(EditorSceneData->mDesc);

	ExportZB2(*Editor, Filename);

	if(string)
		*string = Filename;
}

#ifdef REMOVED
static void gExportButtonCallback(IceButton& /*button*/, void* /*user_data*/)
{
	ExportScene();
}
#endif

#ifdef TEST_NVD
	// In the following function we must skip the CreateXXXRenderer() functions because they also
	// register the renderers to the system etc, which we don't want here. So we go through the
	// render model directly, bypassing the extra management layer.
	class PEELRenderAdapter : public NvdRenderAdapter
	{
		public:

		virtual	RenderHandle	createSphereRenderer(float radius)
		{
			return gUseRenderCaches ? gCurrentRenderModel->_CreateSphereRenderer(radius, false) : null;
		}

		virtual	RenderHandle	createCapsuleRenderer(float halfHeight, float radius)
		{
			return gUseRenderCaches ? gCurrentRenderModel->_CreateCapsuleRenderer(radius, halfHeight*2.0f) : null;
		}

		virtual	RenderHandle	createBoxRenderer(const PxVec3& extents)
		{
			return gUseRenderCaches ? gCurrentRenderModel->_CreateBoxRenderer(ToPoint(extents)) : null;
		}

		virtual	RenderHandle	createConvexRenderer(const NvdConvexInterface& data)
		{
			return gUseRenderCaches ? gCurrentRenderModel->_CreateConvexRenderer(data.mNbVerts, reinterpret_cast<const Point*>(data.mVerts)) : null;
		}

		virtual	RenderHandle	createMeshRenderer(const SurfaceInterface& surface)
		{
			// TODO: revisit active edges param
			return gUseRenderCaches ? gCurrentRenderModel->_CreateMeshRenderer(PintSurfaceInterface(surface), null, false, false) : null;
		}

		virtual	void			releaseRenderer(RenderHandle handle)
		{
			PintShapeRenderer* renderer = reinterpret_cast<PintShapeRenderer*>(handle);
			renderer->Release();
		}
	}gPEELRenderAdapter;

	class FrameSlider : public IceSlider
	{
		public:
								FrameSlider(/*MainWindow* main_window, */const SliderDesc& desc/*, udword start_frame*/);
		virtual					~FrameSlider();

		virtual	void			OnSliderEvent(SliderEvent event);

				void			SetEditBox(IceEditBox* edit_box);

				udword			SelectFrame(udword frame_index, bool update_tree_view, bool update_slider_pos);

				//MainWindow*	mMainWindow;
				//udword		mCurrentFrame;

				SceneTreeView*	mSceneTreeView;
				ObjectTreeView*	mObjectTreeView;
		protected:
				IceEditBox*		mEditBox;
		public:
				udword			mMaxNbFrames;
	};

FrameSlider::FrameSlider(/*MainWindow* main_window,*/ const SliderDesc& desc/*, udword start_frame*/) :
	IceSlider		(desc)/*, mMainWindow(main_window),  mCurrentFrame(start_frame)*/,
	mSceneTreeView	(null),
	mObjectTreeView	(null),
	mEditBox		(null),
	mMaxNbFrames	(0)
{
}

FrameSlider::~FrameSlider()
{
}

void FrameSlider::SetEditBox(IceEditBox* edit_box)
{
	mEditBox	= edit_box;
	ASSERT(!edit_box->GetUserData());
	edit_box->SetUserData(this);
}

/*void FrameSlider::SelectFrame(udword frame_index)
{
	if(mCurrentFrame!=frame_index)
	{
		mCurrentFrame = frame_index;
		ExtractFrameData(frame_index, mMainWindow);
	}
}*/

udword FrameSlider::SelectFrame(udword frame_index, bool update_tree_view, bool update_slider_pos)
{
//	if(frame_index>mMaxNbFrames)
//		frame_index = mMaxNbFrames;

	if(update_slider_pos)
		SetIntValue(frame_index);

	NvdDatabase* NvdDb = getNvdDatabase();
	if(NvdDb)
	{
		{
			uint64_t time = __rdtsc();
			NvdDb->selectFrame(frame_index);
			time = __rdtsc() - time;
			printf("SelectFrame: %d\n", uint32_t(time/1024));
		}

		if(update_tree_view && mSceneTreeView)
		{
			uint64_t time = __rdtsc();
			NvdDb->populateSceneTreeView(*mSceneTreeView, gUseObjectNamesInTreeView);
			time = __rdtsc() - time;
			printf("populateSceneTreeView: %d\n", uint32_t(time/1024));

#ifdef USE_CUSTOM_TREEVIEW
			mSceneTreeView->redraw();
#endif
			if(mObjectTreeView)
				mObjectTreeView->RemoveAll();
		}
	}

	return frame_index;
}

void FrameSlider::OnSliderEvent(SliderEvent event)
{
/*	const bool hasMoved = (event==SBE_SCROLL_UP_OR_LEFT)
						||(event==SBE_SCROLL_DOWN_OR_RIGHT)
						||(event==SBE_THUMBTRACK);

		SBE_SCROLL_UP_OR_LEFT,
		SBE_SCROLL_DOWN_OR_RIGHT,
		SBE_PAGE_UP_OR_LEFT,
		SBE_PAGE_DOWN_OR_RIGHT,
		SBE_THUMBPOSITION,
		SBE_THUMBTRACK,
		SBE_TOP_OR_LEFT,
		SBE_BOTTOM_OR_RIGHT,
		SBE_END_SCROLL,
*/
//	printf("Slider event %d\n", event);
//	return;

//	printf("Slider value: %d\n", GetIntValue());
//	printf("Slider value: %f\n", GetValue());
//	printf("Slider page step: %d\n", GetPageStep());
//	printf("Slider line step: %d\n", GetLineStep());

	udword NewFrame = GetIntValue();
//	SelectFrame(NewFrame);
	printf("Frame: %d\n", NewFrame);

	if(mEditBox)
		mEditBox->SetText(_F("%d", NewFrame));

	NewFrame = SelectFrame(NewFrame, event==SBE_END_SCROLL, false);

//	if(mEditBox)
//		mEditBox->SetText(_F("%d", NewFrame));
}

static void InitCaptureReplay()
{
	GetControlInterface()->Reset();
	SetControlInterface(&gNVDControlInterface);
	gNVDControlInterface.Init();

	NvdCapture* Nvd = getNvd();
	if(Nvd)
	{
		gCurrentRenderModel->SetGroundPlane(false);

		Nvd->enableCapture(false);
	}
}

static void InitCaptureReplayUI(FrameSlider* FS, PxU32 nb_frames)
{
//	gNVDControlInterface.Init(FS->mSceneTreeView, FS->mObjectTreeView);
	gNVDControlInterface.mObjectTreeView = FS->mObjectTreeView;
	gNVDControlInterface.mSceneTreeView = FS->mSceneTreeView;

	// TODO: missing "GetIntRange" in ICE

	if(nb_frames)
	{
		FS->SetIntRange(0, nb_frames-1);
		FS->mMaxNbFrames = nb_frames-1;
	}
	else
	{
		FS->SetIntRange(0, 0);
		FS->mMaxNbFrames = 0;
	}
	FS->SetIntValue(0);
//	FS->mCurrentFrame = 0;
//	FS->SetVisible(nb_frames!=0);
	FS->SetEnabled(true);

	FS->OnSliderEvent(SliderEvent(SBE_END_SCROLL));
}

static void gStopCaptureButtonCallback(IceButton& button, void* user_data)
{
	FrameSlider* FS = (FrameSlider*)user_data;

	NvdCapture* Nvd = getNvd();
	if(Nvd)
	{
		InitCaptureReplay();

		NvdDatabase* NvdDb = getNvdDatabase();
		if(NvdDb)
		{
			PxU32 NbFrames = 0;
			if(NvdDb->init(*Nvd, NbFrames, &gPEELRenderAdapter))
			{
				InitCaptureReplayUI(FS, NbFrames);
			}
		}
	}
}

static void gLoadCaptureButtonCallback(IceButton& button, void* user_data)
{
	FrameSlider* FS = (FrameSlider*)user_data;

	NvdCapture* Nvd = getNvd();
	if(Nvd)
	{
		String Filename;
		{
			FILESELECTCREATE Create;
			Create.mFilter			= "NVD Files (*.nvd)|*.nvd|All Files (*.*)|*.*||";
			Create.mFileName		= "Capture.nvd";
//			Create.mInitialDir		= *gRoot;
			Create.mInitialDir		= "d:\\Nvd";
			Create.mCaptionTitle	= "Import capture";
			Create.mDefExt			= "nvd";

			if(!FileselectOpenSingle(Create, Filename))
				return;
		}

		void TestEmpty();
		TestEmpty();

		InitCaptureReplay();

		NvdDatabase* NvdDb = getNvdDatabase();
		if(NvdDb)
		{
//			VirtualFile VF("d:/Nvd/Capture.nvd");
			VirtualFile VF(Filename);
			udword Length;
			const ubyte* Data = VF.Load(Length);

			PxU32 NbFrames = 0;
			if(NvdDb->init(Data, Length, NbFrames, &gPEELRenderAdapter))
			{
				InitCaptureReplayUI(FS, NbFrames);
			}
		}
	}
}
#endif

IceWindow* Editor_InitGUI(IceWidget* parent, PintGUIHelper& helper)
{
	if(mEditedPlugin)
		mEditedPlugin->InitGUI(parent, helper);

	IceWindow* Main = helper.CreateMainWindow(gEditorGUI, parent, EDITOR_GUI_MAIN, "Editor options");

	const sdword YStep = 20;
	const sdword YStepCB = 16;
	sdword y = 4;

	{
		const udword CheckBoxWidth = 200;

		gCheckBox_DrawSceneBounds = helper.CreateCheckBox(Main, EDITOR_GUI_DRAW_SCENE_BOUNDS, 4, y, CheckBoxWidth, 20, "Draw scene bounds (if available)", gEditorGUI, gDrawSceneBounds, gCheckBoxCallback, gTooltip_DrawSceneBounds);
		y += YStepCB;

		y += YStepCB;
	}

#ifdef REMOVED
	{
//		y += YStep*2;
		helper.CreateButton(Main, 0, 4, y, 300, 20, "Export", gEditorGUI, gExportButtonCallback, null/*helper.GetRootDirectory()*/);
		y += YStep*2;
	}
#endif

	// Tab control
	enum TabIndex
	{
		TAB_STATS,
#ifdef TEST_NVD
		TAB_NVD,
		TAB_NVD_SETTINGS,
#endif
		TAB_COUNT,
	};
	IceWindow* Tabs[TAB_COUNT];
	{
		const sdword MainHeight = 650;

		TabControlDesc TCD;
		TCD.mParent	= Main;
		TCD.mX		= 0;
		TCD.mY		= y;
//		TCD.mWidth	= MainWidth - WD.mX - BorderSize;
//		TCD.mHeight	= MainHeight - BorderSize*2;
		TCD.mWidth	= 500;
		TCD.mHeight	= MainHeight;
		IceTabControl* TabControl = ICE_NEW(IceTabControl)(TCD);
		gEditorGUI->Register(TabControl);

		for(udword i=0;i<TAB_COUNT;i++)
		{
			WindowDesc WD;
			WD.mParent	= Main;
			WD.mX		= 0;
			WD.mY		= 0;
			WD.mWidth	= 500;
			WD.mHeight	= MainHeight;
			WD.mLabel	= "Tab";
			WD.mType	= WINDOW_DIALOG;
			IceWindow* Tab = ICE_NEW(IceWindow)(WD);
			gEditorGUI->Register(Tab);
			Tab->SetVisible(true);
			Tabs[i] = Tab;
		}
		TabControl->Add(Tabs[TAB_STATS], "Stats");
#ifdef TEST_NVD
		TabControl->Add(Tabs[TAB_NVD], "NVD");
		TabControl->Add(Tabs[TAB_NVD_SETTINGS], "Settings");
#endif

		// TAB_STATS
		{
			IceWindow* TabWindow = Tabs[TAB_STATS];
			const sdword YStart = 4;
			sdword y = YStart;

			const sdword OffsetX = 180;
			const sdword LabelOffsetY = 2;

			{
		//		helper.CreateLabel(Main, 4, y+LabelOffsetY, 90, 20, "Summary:", gStatsGUI);
				gEditBox_Stats = helper.CreateEditBox(TabWindow, EDITOR_GUI_STATS, 4, y, 300, 550, "", gEditorGUI, EDITBOX_TEXT, null);
				gEditBox_Stats->SetReadOnly(true);
			}

			y += YStep;
		}

#ifdef TEST_NVD
		const sdword EditBoxWidth = 60;

		// TAB_NVD
		{
			IceWindow* TabWindow = Tabs[TAB_NVD];
			const sdword YStart = 4;
			sdword y = YStart;

			const sdword OffsetX = 180;
			const sdword LabelOffsetY = 2;

			const sdword YStep = 20;
			const sdword YStepCB = 16;

			FrameSlider* FS = null;
			{
//				if(!mSlider)
				{
					SliderDesc Desc;
					Desc.mParent	= TabWindow;
					Desc.mWidth		= 400;
					Desc.mHeight	= 20;
					Desc.mX			= 4;
					Desc.mY			= y;
					FrameSlider* SD = ICE_NEW(FrameSlider)(Desc);
					SD->SetEnabled(false);
					FS = SD;
					SD->SetSteps(1, 1);
					SD->SetIntRange(0, 0);
					SD->AddToolTip("Current frame");
					gEditorGUI->Register(SD);
					//SD->SetIntRange(0, 100);

					{
						const sdword EditBoxX = Desc.mX + Desc.mWidth + 10;
						struct Local
						{
							static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
							{
								FrameSlider* FS = reinterpret_cast<FrameSlider*>(edit_box.GetUserData());
								udword DesiredFrame = edit_box.GetInt();
								if(DesiredFrame>FS->mMaxNbFrames)
								{
									DesiredFrame = FS->mMaxNbFrames;
									const_cast<IceEditBox&>(edit_box).SetText(_F("%d", DesiredFrame));
								}

								FS->SelectFrame(DesiredFrame, true, true);
							}
						};

//						SD->SetEditBox(helper.CreateEditBox(TabWindow, EDITOR_GUI_CURRENT_FRAME, EditBoxX, y, EditBoxWidth, 20, "0", gEditorGUI, EDITBOX_INTEGER_POSITIVE, Local::gEBCallback, null));
						{
							IceEditBox* EB = CreateSpinBox(TabWindow, EditBoxX, y, 0, EDITOR_GUI_CURRENT_FRAME);
							EB->SetVisible(true);
							EB->SetCallback(Local::gEBCallback);
							if(gEditorGUI)
								gEditorGUI->Register(EB);
							SD->SetEditBox(EB);
						}
					}

					y += YStepCB*2;
				}

/*				if(nb_frames)
					mSlider->SetIntRange(0, nb_frames-1);
				else
					mSlider->SetIntRange(0, 0);
				mSlider->SetIntValue(0);
				mSlider->mCurrentFrame = 0;
				mSlider->SetVisible(nb_frames!=0);*/
			}

/*			{
				const udword CheckBoxWidth = 200;

				helper.CreateCheckBox(Main, EDITOR_GUI_DRAW_SCENE_BOUNDS, 4, y, CheckBoxWidth, 20, "Draw scene bounds", gEditorGUI, gDrawSceneBounds, gCheckBoxCallback, gTooltip_DrawSceneBounds);
				y += YStepCB;

				y += YStepCB;
			}*/

			if(1)
			{
				helper.CreateButton(TabWindow, EDITOR_GUI_STOP_CAPTURE, 4, y, 400, 20, "Stop capture", gEditorGUI, gStopCaptureButtonCallback, FS/*helper.GetRootDirectory()*/);
				y += YStep*2;
			}

			if(1)
			{
				helper.CreateButton(TabWindow, EDITOR_GUI_LOAD_CAPTURE, 4, y, 400, 20, "Load capture", gEditorGUI, gLoadCaptureButtonCallback, FS/*helper.GetRootDirectory()*/);
				y += YStep*2;
			}

			SceneTreeView* STV = null;
			if(1)
			{
				y += YStep;
#ifdef USE_ICE_TREEVIEW
				TreeViewDesc TVD;
				TVD.mStyle		= TREEVIEW_NORMAL;
				TVD.mStyle		= TREEVIEW_CHECKBOXES;
#else
				WindowDesc TVD;
//				TVD.mStyle		= WSTYLE_CLIENT_EDGES;
#endif
				TVD.mParent		= TabWindow;
				TVD.mX			= 4;
				TVD.mY			= y;
				TVD.mWidth		= 400;
				TVD.mHeight		= 200;
				TVD.mHeight		= 16*16+2;
				TVD.mLabel		= "Scene tree view";
				STV = ICE_NEW(SceneTreeView)(TVD);
				STV->SetVisible(true);
				gEditorGUI->Register(STV);

//				IceTreeViewItem* RootItem = STV->Add(null, "Root");
//				IceTreeViewItem* Item0 = STV->Add(RootItem, "Item0");
//				IceTreeViewItem* Item1 = STV->Add(RootItem, "Item1");
//				IceTreeViewItem* Item2 = STV->Add(RootItem, "Item2");

				FS->mSceneTreeView = STV;
				y += TVD.mHeight;
			}

			if(1)
			{
				y += YStep;
				TreeViewDesc TVD;
				TVD.mStyle		= TREEVIEW_NORMAL;
//				TVD.mStyle		= TREEVIEW_CHECKBOXES;				
				TVD.mParent		= TabWindow;
				TVD.mX			= 4;
				TVD.mY			= y;
				TVD.mWidth		= 400;
				TVD.mHeight		= 200;
				TVD.mLabel		= "Object tree view";
				ObjectTreeView* TV = ICE_NEW(ObjectTreeView)(TVD);
				TV->SetVisible(true);
				gEditorGUI->Register(TV);
				STV->mObjectTreeView = TV;

				if(0)
				{

				IceTreeViewItem* RootItem = TV->Add(null, "Root");
/*				IceTreeViewItem* Item0 = TV->Add(RootItem, "Item0");
				IceTreeViewItem* Item1 = TV->Add(RootItem, "Item1");
				IceTreeViewItem* Item2 = TV->Add(RootItem, "Item2");*/

				udword tgt = TimeGetTime();
				udword Index = 0;
				for(udword i=0;i<2000;i++)
				{
//					IceTreeViewItem* Group = TV->Add(RootItem, _F("Group%d", i));
					IceTreeViewItem* Group = TV->Add(RootItem, "pipo");
					for(udword j=0;j<256;j++)
					{
//						IceTreeViewItem* Item0 = TV->Add(Group, _F("Item%d", Index++));
						IceTreeViewItem* Item0 = TV->Add(Group, "pipo");
					}
				}
				tgt = TimeGetTime() - tgt;
				printf("tgt: %d\n", tgt);
				}

				FS->mObjectTreeView = TV;
				gObjectTreeView = TV;
				y += TVD.mHeight;
			}

			y += YStep;
			TabWindow->SetVisible(false);
		}

		// TAB_NVD_SETTINGS
		{
			IceWindow* TabWindow = Tabs[TAB_NVD_SETTINGS];
			const sdword YStart = 4;
			sdword y = YStart;

			const sdword OffsetX = 180;
			const sdword LabelOffsetY = 2;

			const sdword YStep = 20;
			const sdword YStepCB = 16;

			const udword CheckBoxWidth = 200;
			{
				/*gCheckBox_DrawSceneBounds =*/ helper.CreateCheckBox(TabWindow, EDITOR_GUI_DRAW_VISIBLE_BOUNDS, 4, y, CheckBoxWidth, 20, "Draw all bounds", gEditorGUI, gDrawVisibleBounds, gCheckBoxCallback, null/*gTooltip_DrawSceneBounds*/);
				y += YStepCB;
			}
			{
				/*gCheckBox_DrawSceneBounds =*/ helper.CreateCheckBox(TabWindow, EDITOR_GUI_DRAW_GRID, 4, y, CheckBoxWidth, 20, "Draw grid", gEditorGUI, gDrawGrid, gCheckBoxCallback, null/*gTooltip_DrawSceneBounds*/);
				y += YStepCB;
			}
			{
				/*gCheckBox_DrawSceneBounds =*/ helper.CreateCheckBox(TabWindow, EDITOR_GUI_OBJECT_NAMES_TREEVIEW, 4, y, CheckBoxWidth, 20, "Use object names in tree-view", gEditorGUI, gUseObjectNamesInTreeView, gCheckBoxCallback, null/*gTooltip_DrawSceneBounds*/);
				y += YStepCB;
			}
			{
				/*gCheckBox_DrawSceneBounds =*/ helper.CreateCheckBox(TabWindow, EDITOR_GUI_DISPLAY_INTERNAL_DATA, 4, y, CheckBoxWidth, 20, "Display internal data in tree-view", gEditorGUI, gDisplayInternalData, gCheckBoxCallback, null/*gTooltip_DrawSceneBounds*/);
				y += YStepCB;
			}

			y += YStep;
			TabWindow->SetVisible(false);
		}
#endif

	}
	return Main;
}

void Editor_CloseGUI()
{
	if(mEditedPlugin)
		mEditedPlugin->CloseGUI();

//	Common_CloseGUI(gEditorGUI);
	DELETESINGLE(gEditorGUI);
	gCheckBox_DrawSceneBounds = null;
	gEditBox_Stats = null;
}

///////////////////////////////////////////////////////////////////////////////

class EditorPlugIn : public PintPlugin
{
	public:
	virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper)	{ return Editor_InitGUI(parent, helper);	}
	virtual	void		CloseGUI()											{ Editor_CloseGUI();						}
	virtual	void		Init(const PINT_WORLD_CREATE& desc)					{ Editor_Init(desc);						}
	virtual	void		Close()												{ Editor_Close();							}
	virtual	Pint*		GetPint()											{ return GetEditor();						}

	virtual	IceWindow*	InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
	{
		return mEditedPlugin ? mEditedPlugin->InitTestGUI(test_name, parent, helper, owner) : null;
	}
	virtual	void		CloseTestGUI()
	{
		if(mEditedPlugin)
			mEditedPlugin->CloseTestGUI();
	}
	virtual	const char*	GetTestGUIName()
	{
		return mEditedPlugin ? mEditedPlugin->GetTestGUIName() : null;
	}
	virtual	void		ApplyTestUIParams(const char* test_name)
	{
		if(mEditedPlugin)
			mEditedPlugin->ApplyTestUIParams(test_name);
	}
};
static EditorPlugIn gPlugIn;

PintPlugin*	GetEditorPlugin()
{
	return &gPlugIn;
}

bool IsEditor(Pint* pint)
{
	return pint==gEditor;
}

void EditorPlugin::HideActors(udword nb, PintActorHandle* handles)
{
	ASSERT(!"Not implemented");
/*	ActorData* Data = (ActorData*)handle;

	if(mEdited)
	{
		PintActorHandle NativeHandle = GetNativeHandle(Data);
	}*/
}

void EditorPlugin::ResetPoses(udword nb, PintActorHandle* handles)
{
	const bool ResetVelocities = true;	// Reset velocities as well.

	while(nb--)
	{
		const PintActorHandle handle = *handles++;
		const ActorData* Data = HandleToActorData(handle);
		ASSERT(Data->mUserData);
		const EditorActor* Object = reinterpret_cast<const EditorActor*>(Data->mUserData);
		ASSERT(Object->GetType()==EditorObject::ACTOR);
		// ### what's the difference between Object->mInitPos and Data->mPosition ?
		SetWorldTransform(handle, PR(Object->mInitPos, Object->mInitRot));
		//SetWorldTransform(handle, PR(Data->mPosition, Data->mRotation));

		if(ResetVelocities)
		{
			SetLinearVelocity(handle, Data->mLinearVelocity);
			SetAngularVelocity(handle, Data->mAngularVelocity);
		}
	}
}

static void GetCurrentActorPose(PR& pose, Pint* edited, const PintActorHandle handle, const ActorData* data)
{
	// TODO: it's unclear whether we should use the init pose or not?
	if(edited)
		pose = edited->GetWorldTransform(handle);
	else
		pose = PR(data->mPosition, data->mRotation);
}

///////////////////////////////////////////////////////////////////////////////

	class DefaultMerge : public MergeInterface
	{
		public:
		virtual	PINT_SHAPE_CREATE*	CreateShape(const ShapeData* shape_data);
	};

PINT_SHAPE_CREATE* DefaultMerge::CreateShape(const ShapeData* shape_data)
{
	PINT_SHAPE_CREATE* dst = null;

	switch(shape_data->mType)
	{
#define IMPLEMENT_SHAPE_TYPE(EditorShapeClass, PintShapeClass, type)			\
const EditorShapeClass* Src = static_cast<const EditorShapeClass*>(shape_data);	\
PintShapeClass* NewShape = ICE_NEW(PintShapeClass);								\
ASSERT(NewShape->mType == type);												\
dst = NewShape;

		case PINT_SHAPE_SPHERE:
		{
			IMPLEMENT_SHAPE_TYPE(SphereShapeData, PINT_SPHERE_CREATE, PINT_SHAPE_SPHERE)
			NewShape->mRadius = Src->mRadius;
		}break;

		case PINT_SHAPE_CAPSULE:
		{
			IMPLEMENT_SHAPE_TYPE(CapsuleShapeData, PINT_CAPSULE_CREATE, PINT_SHAPE_CAPSULE)
			NewShape->mRadius = Src->mRadius;
			NewShape->mHalfHeight = Src->mHalfHeight;
		}break;

		case PINT_SHAPE_CYLINDER:
		{
			IMPLEMENT_SHAPE_TYPE(CylinderShapeData, PINT_CYLINDER_CREATE, PINT_SHAPE_CYLINDER)
			NewShape->mRadius = Src->mRadius;
			NewShape->mHalfHeight = Src->mHalfHeight;
		}break;

		case PINT_SHAPE_BOX:
		{
			IMPLEMENT_SHAPE_TYPE(BoxShapeData, PINT_BOX_CREATE, PINT_SHAPE_BOX)
			NewShape->mExtents = Src->mExtents;
		}break;

		case PINT_SHAPE_CONVEX:
		{
			IMPLEMENT_SHAPE_TYPE(ConvexShapeData, PINT_CONVEX_CREATE, PINT_SHAPE_CONVEX)
			NewShape->mNbVerts = Src->mNbVerts;
			NewShape->mVerts = Src->mVerts;
		}break;

		case PINT_SHAPE_MESH:
		{
			IMPLEMENT_SHAPE_TYPE(MeshShapeData, PINT_MESH_CREATE, PINT_SHAPE_MESH)
			NewShape->SetSurfaceData(Src->mNbVerts, Src->mVerts, Src->mNbTris, Src->mTris->mRef, null);
		}break;

		default:
		{
			ASSERT(!"Unknown type");
		}break;
	};
#undef IMPLEMENT_SHAPE_TYPE
	return dst;
}

///////////////////////////////////////////////////////////////////////////////

	class MakeSphere : public MergeInterface
	{
		public:
		virtual	PINT_SHAPE_CREATE*	CreateShape(const ShapeData* shape_data);
	};

PINT_SHAPE_CREATE* MakeSphere::CreateShape(const ShapeData* shape_data)
{
	PINT_SHAPE_CREATE* dst = null;

	switch(shape_data->mType)
	{
#define IMPLEMENT_SHAPE_TYPE(EditorShapeClass)									\
const EditorShapeClass* Src = static_cast<const EditorShapeClass*>(shape_data);	\
PINT_SPHERE_CREATE* NewShape = ICE_NEW(PINT_SPHERE_CREATE);						\
dst = NewShape;
		case PINT_SHAPE_SPHERE:
		{
			IMPLEMENT_SHAPE_TYPE(SphereShapeData)
			NewShape->mRadius = Src->mRadius;
		}break;

		case PINT_SHAPE_CAPSULE:
		{
			IMPLEMENT_SHAPE_TYPE(CapsuleShapeData)
			NewShape->mRadius = Src->mHalfHeight + Src->mRadius;
		}break;

		case PINT_SHAPE_CYLINDER:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(CylinderShapeData)
		}break;

		case PINT_SHAPE_BOX:
		{
			IMPLEMENT_SHAPE_TYPE(BoxShapeData)
			NewShape->mRadius = Src->mExtents.Magnitude();
		}break;

		case PINT_SHAPE_CONVEX:
		{
			IMPLEMENT_SHAPE_TYPE(ConvexShapeData)
			const Sphere BoundingSphere(Src->mNbVerts, Src->mVerts);
			NewShape->mRadius = BoundingSphere.mRadius;
			// I think I need to relocate renderers for this to work fine
//			NewShape->mLocalPos = BoundingSphere.mCenter;
		}break;

		case PINT_SHAPE_MESH:
		{
			IMPLEMENT_SHAPE_TYPE(MeshShapeData)

			const Sphere BoundingSphere(Src->mNbVerts, Src->mVerts);
			NewShape->mRadius = BoundingSphere.mRadius;
			// I think I need to relocate renderers for this to work fine
//			NewShape->mLocalPos = BoundingSphere.mCenter;
		}break;

		default:
		{
			ASSERT(!"Unknown type");
		}break;
	};
#undef IMPLEMENT_SHAPE_TYPE
	return dst;
}

///////////////////////////////////////////////////////////////////////////////

	class MakeBox : public MergeInterface
	{
		public:
		virtual	PINT_SHAPE_CREATE*	CreateShape(const ShapeData* shape_data);
	};

PINT_SHAPE_CREATE* MakeBox::CreateShape(const ShapeData* shape_data)
{
	PINT_SHAPE_CREATE* dst = null;

	switch(shape_data->mType)
	{
#define IMPLEMENT_SHAPE_TYPE(EditorShapeClass)									\
const EditorShapeClass* Src = static_cast<const EditorShapeClass*>(shape_data);	\
PINT_BOX_CREATE* NewShape = ICE_NEW(PINT_BOX_CREATE);							\
dst = NewShape;
		case PINT_SHAPE_SPHERE:
		{
			IMPLEMENT_SHAPE_TYPE(SphereShapeData)
			NewShape->mExtents = Point(Src->mRadius, Src->mRadius, Src->mRadius);
		}break;

		case PINT_SHAPE_CAPSULE:
		{
			IMPLEMENT_SHAPE_TYPE(CapsuleShapeData)
			NewShape->mExtents = Point(Src->mRadius, Src->mRadius+Src->mHalfHeight, Src->mRadius);
		}break;

		case PINT_SHAPE_CYLINDER:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(CylinderShapeData)
		}break;

		case PINT_SHAPE_BOX:
		{
			IMPLEMENT_SHAPE_TYPE(BoxShapeData)
			NewShape->mExtents = Src->mExtents;
		}break;

		case PINT_SHAPE_CONVEX:
		{
			IMPLEMENT_SHAPE_TYPE(ConvexShapeData)
			AABB Bounds;
			ComputeAABB(Bounds, Src->mVerts, Src->mNbVerts);	
			Bounds.GetExtents(NewShape->mExtents);
			//###TODO: deal with center
		}break;

		case PINT_SHAPE_MESH:
		{
			IMPLEMENT_SHAPE_TYPE(MeshShapeData)
			AABB Bounds;
			ComputeAABB(Bounds, Src->mVerts, Src->mNbVerts);	
			Bounds.GetExtents(NewShape->mExtents);
			//###TODO: deal with center
		}break;

		default:
		{
			ASSERT(!"Unknown type");
		}break;
	};
#undef IMPLEMENT_SHAPE_TYPE
	return dst;
}

///////////////////////////////////////////////////////////////////////////////

	class MakeConvex : public MergeInterface
	{
		public:
		virtual	PINT_SHAPE_CREATE*	CreateShape(const ShapeData* shape_data);
	};

PINT_SHAPE_CREATE* MakeConvex::CreateShape(const ShapeData* shape_data)
{
	PINT_SHAPE_CREATE* dst = null;

	switch(shape_data->mType)
	{
#define IMPLEMENT_SHAPE_TYPE(EditorShapeClass)									\
const EditorShapeClass* Src = static_cast<const EditorShapeClass*>(shape_data);	\
PINT_CONVEX_CREATE* NewShape = ICE_NEW(PINT_CONVEX_CREATE);						\
dst = NewShape;

		case PINT_SHAPE_SPHERE:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(SphereShapeData)
		}break;

		case PINT_SHAPE_CAPSULE:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(CapsuleShapeData)
		}break;

		case PINT_SHAPE_CYLINDER:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(CylinderShapeData)
		}break;

		case PINT_SHAPE_BOX:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(BoxShapeData)

			AABB Box;
			Box.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), Src->mExtents);

			Point Pts[8];
			Box.ComputePoints(Pts);

			NewShape->mNbVerts = 8;
			NewShape->mVerts = Pts;
		}break;

		case PINT_SHAPE_CONVEX:
		{
			IMPLEMENT_SHAPE_TYPE(ConvexShapeData)
			NewShape->mNbVerts = Src->mNbVerts;
			NewShape->mVerts = Src->mVerts;
		}break;

		case PINT_SHAPE_MESH:
		{
			IMPLEMENT_SHAPE_TYPE(MeshShapeData)
			NewShape->mNbVerts = Src->mNbVerts;
			NewShape->mVerts = Src->mVerts;
		}break;

		default:
		{
			ASSERT(!"Unknown type");
		}break;
	};
#undef IMPLEMENT_SHAPE_TYPE
	return dst;
}

///////////////////////////////////////////////////////////////////////////////

	class MakeMesh : public MergeInterface
	{
		public:
		virtual	PINT_SHAPE_CREATE*	CreateShape(const ShapeData* shape_data);
	};

PINT_SHAPE_CREATE* MakeMesh::CreateShape(const ShapeData* shape_data)
{
	PINT_SHAPE_CREATE* dst = null;

	switch(shape_data->mType)
	{
#define IMPLEMENT_SHAPE_TYPE(EditorShapeClass)									\
const EditorShapeClass* Src = static_cast<const EditorShapeClass*>(shape_data);	\
PINT_MESH_CREATE* NewShape = ICE_NEW(PINT_MESH_CREATE);							\
dst = NewShape;

		case PINT_SHAPE_SPHERE:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(SphereShapeData)
		}break;

		case PINT_SHAPE_CAPSULE:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(CapsuleShapeData)
		}break;

		case PINT_SHAPE_CYLINDER:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(CylinderShapeData)
		}break;

		case PINT_SHAPE_BOX:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(BoxShapeData)
		}break;

		case PINT_SHAPE_CONVEX:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(ConvexShapeData)
		}break;

		case PINT_SHAPE_MESH:
		{
			ASSERT(!"Not implemented");
			return null;
			IMPLEMENT_SHAPE_TYPE(MeshShapeData)
		}break;

		default:
		{
			ASSERT(!"Unknown type");
		}break;
	};
#undef IMPLEMENT_SHAPE_TYPE
	return dst;
}

///////////////////////////////////////////////////////////////////////////////

static void ComputeAveragePosAndMass(float& mass, Point& pos, Pint* pint, udword nb, const PintActorHandle* handles, Quat* first_rot=null)
{
	float Mass = 0.0f;
	Point Pos(0.0f, 0.0f, 0.0f);

	const float Coeff = 1.0f/float(nb);
	for(udword i=0;i<nb;i++)
	{
		const PintActorHandle handle = handles[i];
		const ActorData* Data = HandleToActorData(handle);
		const PintActorHandle NativeActorHandle = GetNativeActorHandle(Data);

		PR ActorPR;
		GetCurrentActorPose(ActorPR, pint, NativeActorHandle, Data);

		Pos += ActorPR.mPos*Coeff;
		Mass += Data->mMass*Coeff;

		if(i==0 && first_rot)
			*first_rot = ActorPR.mRot;
	}

	mass = Mass;
	pos = Pos;
}

///////////////////////////////////////////////////////////////////////////////

// Creates compound
PintActorHandle EditorPlugin::MergeActors(udword nb, const PintActorHandle* handles, MergeInterface& merge_interface, const MergeParams& params)
{
	PINT_OBJECT_CREATE ActorDesc;

	// Name
	if(params.mName)
	{
		ActorDesc.mName = params.mName;
	}
	else
	{
		if(nb==1)
			ActorDesc.mName = HandleToActorData(*handles)->mName.Get();
		else
			ActorDesc.mName	= "(Merged actors)";
	}

	// If needed, derive compound position & mass from existing actors
	float Mass = 0.0f;
	Point Pos(0.0f, 0.0f, 0.0f);
	if(params.mMass<0.0f || !params.mPose)
	{
		/*const float Coeff = 1.0f/float(nb);
		for(udword i=0;i<nb;i++)
		{
			const PintActorHandle handle = handles[i];
			const ActorData* Data = HandleToActorData(handle);
			const PintActorHandle NativeActorHandle = GetNativeActorHandle(Data);

			PR ActorPR;
			GetCurrentActorPose(ActorPR, mEdited, NativeActorHandle, Data);

			Pos += ActorPR.mPos*Coeff;
			Mass += Data->mMass*Coeff;

			if(i==0)
				ActorDesc.mRotation = ActorPR.mRot;
		}*/

		ComputeAveragePosAndMass(Mass, Pos, mEdited, nb, handles, &ActorDesc.mRotation);
	}

	// Setup mass
	if(params.mMass>=0.0f)
		ActorDesc.mMass = params.mMass;
	else
		ActorDesc.mMass = Mass;

	// Setup pose
	if(params.mPose)
	{
		ActorDesc.mPosition = params.mPose->mPos;
		ActorDesc.mRotation = params.mPose->mRot;
	}
	else
		ActorDesc.mPosition = Pos;

	const Matrix4x4 ActorDescMat44 = PR(ActorDesc.mPosition, ActorDesc.mRotation);
	Matrix4x4 ActorDescMat44Inv;
	InvertPRMatrix(ActorDescMat44Inv, ActorDescMat44);

	// Shapes
	{
		PINT_SHAPE_CREATE* LastShape = null;
		for(udword i=0;i<nb;i++)
		{
			const PintActorHandle handle = handles[i];
			const ActorData* Data = HandleToActorData(handle);
			const PintActorHandle NativeActorHandle = GetNativeActorHandle(Data);

			PR ActorPR;
			GetCurrentActorPose(ActorPR, mEdited, NativeActorHandle, Data);

			const Matrix4x4 ActorMat44 = ActorPR;

			const udword NbShapes = Data->mShapes.GetNbEntries();
			for(udword j=0;j<NbShapes;j++)
			{
				const ShapeData* Current = (const ShapeData*)Data->mShapes.GetEntry(j);

				PINT_SHAPE_CREATE* dst = merge_interface.CreateShape(Current);

				if(dst)
				{
					const Matrix4x4 Local44 = PR(Current->mLocalPos, Current->mLocalRot);
					const Matrix4x4 Combo = Local44 * ActorMat44;
					if(0)
					{
	//					dst->mLocalPos += Combo.GetTrans();
						dst->mLocalPos = Combo.GetTrans();
						dst->mLocalPos -= ActorDesc.mPosition;
						dst->mLocalRot = Matrix3x3(Combo);
					}
					else
					{
						const Matrix4x4 Final = Combo * ActorDescMat44Inv;
						dst->mLocalPos = Final.GetTrans();
						dst->mLocalRot = Matrix3x3(Final);
					}

					dst->mName		= Current->mName.Get();
				//	dst->mMaterial	= Current->mMaterial;
					dst->mRenderer	= Current->mRenderer;
					dst->SetNext(null);

					LastShape = ActorDesc.AddShape(dst, LastShape);
				}
			}
		}
	}

// TODO:
//	ActorDesc.mRotation;
//	ActorDesc.mCOMLocalOffset;
//	ActorDesc.mLinearVelocity;
//	ActorDesc.mAngularVelocity;
//	ActorDesc.mMassForInertia;
//	ActorDesc.mCollisionGroup;
//	ActorDesc.mKinematic;
//	ActorDesc.mAddToWorld;
//	ActorDesc.mAddToDatabase;

	const PintActorHandle MergedActor = CreateObject(ActorDesc);

	DeletePintObjectShapes(ActorDesc);

	return MergedActor;
}

PintActorHandle EditorPlugin::CreateSingleConvex(udword nb, const PintActorHandle* handles)
{
	PINT_OBJECT_CREATE ActorDesc;
	ActorDesc.mName	= "(Merged actors)";
	if(nb==1)
	{
		const ActorData* Data = HandleToActorData(*handles);
		ActorDesc.mName = Data->mName.Get();
	}

//	ActorDesc.mShapes;
//	ActorDesc.mPosition;
//	ActorDesc.mRotation;
//	ActorDesc.mCOMLocalOffset;
//	ActorDesc.mLinearVelocity;
//	ActorDesc.mAngularVelocity;
//	ActorDesc.mMass;
//	ActorDesc.mMassForInertia;
//	ActorDesc.mCollisionGroup;
//	ActorDesc.mKinematic;
//	ActorDesc.mAddToWorld;
//	ActorDesc.mAddToDatabase;

/*	if(1)
	{
		const float Coeff = 1.0f/float(nb);
		for(udword i=0;i<nb;i++)
		{
			const PintActorHandle handle = handles[i];
			const ActorData* Data = HandleToActorData(handle);
			const PintActorHandle NativeActorHandle = GetNativeActorHandle(Data);

			PR ActorPR;
			GetCurrentActorPose(ActorPR, mEdited, NativeActorHandle, Data);

			ActorDesc.mPosition += ActorPR.mPos*Coeff;
			ActorDesc.mMass += Data->mMass*Coeff;

	//		ASSERT(Data->mUserData);
	//		const EditorActor* Object = (const EditorActor*)Data->mUserData;
	//		ASSERT(Object->mEditorType==EditorObject::ACTOR);
	//		SetWorldTransform(handle, PR(Object->mInitPos, Object->mInitRot));
		}
	}*/
	ComputeAveragePosAndMass(ActorDesc.mMass, ActorDesc.mPosition, mEdited, nb, handles);

	Vertices ConvexVerts;
	PintRendererCollection* RC = static_cast<PintRendererCollection*>(CreateRendererCollection());

	{
#ifdef REMOVED
		PINT_SHAPE_CREATE* LastShape = null;
#endif
		for(udword i=0;i<nb;i++)
		{
			const PintActorHandle handle = handles[i];
			const ActorData* Data = HandleToActorData(handle);
			const PintActorHandle NativeActorHandle = GetNativeActorHandle(Data);

	//		ASSERT(Data->mUserData);
	//		const EditorActor* Object = (const EditorActor*)Data->mUserData;
	//		ASSERT(Object->mEditorType==EditorObject::ACTOR);

			PR ActorPR;
			GetCurrentActorPose(ActorPR, mEdited, NativeActorHandle, Data);

			const Matrix4x4 ActorMat44 = ActorPR;

			const udword NbShapes = Data->mShapes.GetNbEntries();
			for(udword j=0;j<NbShapes;j++)
			{
				const ShapeData* Current = (const ShapeData*)Data->mShapes.GetEntry(j);

					const Matrix4x4 Local44 = PR(Current->mLocalPos, Current->mLocalRot);
					const Matrix4x4 Combo = Local44 * ActorMat44;

				if(Current->mType==PINT_SHAPE_MESH)
				{
					const MeshShapeData* MeshData = static_cast<const MeshShapeData*>(Current);
					const udword NbVerts = MeshData->mNbVerts;
					for(udword k=0;k<NbVerts;k++)
					{
						Point p = MeshData->mVerts[k] * Combo;
						p -= ActorDesc.mPosition;

						ConvexVerts.AddVertex(p);
					}
				}
				else if(Current->mType==PINT_SHAPE_CONVEX)
				{
					const ConvexShapeData* ConvexData = static_cast<const ConvexShapeData*>(Current);
					const udword NbVerts = ConvexData->mNbVerts;
					for(udword k=0;k<NbVerts;k++)
					{
						Point p = ConvexData->mVerts[k] * Combo;
						p -= ActorDesc.mPosition;

						ConvexVerts.AddVertex(p);
					}
				}
				else
					ASSERT(0);

//				RC->mShapeRenderers.AddPtr(Current->mRenderer);

				Point p = Combo.GetTrans();
				p -= ActorDesc.mPosition;
				const PR LocalPose(p, Matrix3x3(Combo));
				RC->AddRenderer(Current->mRenderer, LocalPose);

#ifdef REMOVED
				PINT_SHAPE_CREATE* dst = merge_interface.CreateShape(Current);

				if(dst)
				{
					dst->mName		= Current->mName.Get();

					dst->mLocalPos = Combo.GetTrans();
					dst->mLocalPos -= ActorDesc.mPosition;
					dst->mLocalRot = Matrix3x3(Combo);

				//	dst->mMaterial	= Current->mMaterial;
					dst->mRenderer	= Current->mRenderer;
					dst->mNext		= null;

					if(!ActorDesc.mShapes)
						ActorDesc.SetShape(dst);
					if(LastShape)
						LastShape->mNext	= dst;
					LastShape = dst;
				}
#endif
			}
		}
	}

//	if(mass)
//		ActorDesc.mMass = *mass;

	const PINT_CONVEX_CREATE ConvexShape(ConvexVerts.GetNbVertices(), ConvexVerts.GetVertices());
	ConvexShape.mRenderer = RC;//CreateConvexRenderer(ConvexVerts.GetNbVertices(), ConvexVerts.GetVertices());
	ActorDesc.SetShape(&ConvexShape);

	const PintActorHandle MergedActor = CreateObject(ActorDesc);

#ifdef REMOVED
	{
		const PINT_SHAPE_CREATE* CurrentShape = ActorDesc.mShapes;
		while(CurrentShape)
		{
			const PINT_SHAPE_CREATE* NextShape = CurrentShape->mNext;
			DELETESINGLE(CurrentShape);
			CurrentShape = NextShape;
		}
	}
#endif




#ifdef REMOVED

	{
//		PINT_SHAPE_CREATE* LastShape = null;
		for(udword i=0;i<nb;i++)
		{
			const PintActorHandle handle = handles[i];
			const ActorData* Data = HandleToActorData(handle);
			const PintActorHandle NativeActorHandle = GetNativeHandle(Data);

	//		ASSERT(Data->mUserData);
	//		const EditorActor* Object = (const EditorActor*)Data->mUserData;
	//		ASSERT(Object->mEditorType==EditorObject::ACTOR);

			PR ActorPR;
			GetCurrentActorPose(ActorPR, mEdited, NativeActorHandle, Data);

			const Matrix4x4 ActorMat44 = ActorPR;

			const udword NbShapes = Data->mShapes.GetNbEntries();
			for(udword j=0;j<NbShapes;j++)
			{
				const ShapeData* Current = (const ShapeData*)Data->mShapes.GetEntry(j);

					const Matrix4x4 Local44 = PR(Current->mLocalPos, Current->mLocalRot);
					const Matrix4x4 Combo = Local44 * ActorMat44;

//				PINT_SHAPE_CREATE* dst = merge_interface.CreateShape(Current);
				if(Current->mType==PINT_SHAPE_MESH)
				{
					const MeshShapeData* MeshData = static_cast<const MeshShapeData*>(Current);
					const udword NbVerts = MeshData->mNbVerts;
					for(udword k=0;k<NbVerts;k++)
					{
						Point p = MeshData->mVerts[k] * Combo;
						p -= ActorDesc.mPosition;

						ConvexVerts.AddVertex(p);
					}
				}
				else
					ASSERT(0);

/*				if(dst)
				{
					dst->mName		= Current->mName.Get();

					dst->mLocalPos = Combo.GetTrans();
					dst->mLocalPos -= ActorDesc.mPosition;
					dst->mLocalRot = Matrix3x3(Combo);

				//	dst->mMaterial	= Current->mMaterial;
					dst->mRenderer	= Current->mRenderer;
					dst->mNext		= null;

					if(!ActorDesc.mShapes)
						ActorDesc.SetShape(dst);
					if(LastShape)
						LastShape->mNext	= dst;
					LastShape = dst;
				}*/
			}
		}
	}

//	if(mass)
//		ActorDesc.mMass = *mass;

	const PINT_CONVEX_CREATE ConvexShape(ConvexVerts.GetNbVertices(), ConvexVerts.GetVertices());
	ActorDesc.SetShape(&ConvexShape);

	const PintActorHandle MergedActor = CreateObject(ActorDesc);

/*	{
		const PINT_SHAPE_CREATE* CurrentShape = ActorDesc.mShapes;
		while(CurrentShape)
		{
			const PINT_SHAPE_CREATE* NextShape = CurrentShape->mNext;
			DELETESINGLE(CurrentShape);
			CurrentShape = NextShape;
		}
	}*/
#endif

	return MergedActor;
}


PintActorHandle EditorPlugin::CreateSingleMesh(udword nb, const PintActorHandle* handles)
{
	PINT_OBJECT_CREATE ActorDesc;
	ActorDesc.mName	= "(Merged actors)";
	if(nb==1)
	{
		const ActorData* Data = HandleToActorData(*handles);
		ActorDesc.mName = Data->mName.Get();
	}

	ComputeAveragePosAndMass(ActorDesc.mMass, ActorDesc.mPosition, mEdited, nb, handles);

	Vertices Verts;
	Container Indices;
	udword Offset = 0;

	PintRendererCollection* RC = static_cast<PintRendererCollection*>(CreateRendererCollection());

	{
		for(udword i=0;i<nb;i++)
		{
			const PintActorHandle handle = handles[i];
			const ActorData* Data = HandleToActorData(handle);
			const PintActorHandle NativeActorHandle = GetNativeActorHandle(Data);

			PR ActorPR;
			GetCurrentActorPose(ActorPR, mEdited, NativeActorHandle, Data);

			const Matrix4x4 ActorMat44 = ActorPR;

			const udword NbShapes = Data->mShapes.GetNbEntries();
			for(udword j=0;j<NbShapes;j++)
			{
				const ShapeData* Current = (const ShapeData*)Data->mShapes.GetEntry(j);

					const Matrix4x4 Local44 = PR(Current->mLocalPos, Current->mLocalRot);
					const Matrix4x4 Combo = Local44 * ActorMat44;

				if(Current->mType==PINT_SHAPE_MESH)
				{
					const MeshShapeData* MeshData = static_cast<const MeshShapeData*>(Current);

					const udword NbVerts = MeshData->mNbVerts;
					for(udword k=0;k<NbVerts;k++)
					{
						Point p = MeshData->mVerts[k] * Combo;
						p -= ActorDesc.mPosition;

						Verts.AddVertex(p);
					}

					const udword NbTris = MeshData->mNbTris;
					for(udword k=0;k<NbTris;k++)
					{
						const IndexedTriangle& T = MeshData->mTris[k];

						Indices.Add(Offset + T.mRef[0]);
						Indices.Add(Offset + T.mRef[1]);
						Indices.Add(Offset + T.mRef[2]);
					}

					Offset += NbVerts;
				}
				else
					ASSERT(0);

				Point p = Combo.GetTrans();
				p -= ActorDesc.mPosition;
				const PR LocalPose(p, Matrix3x3(Combo));
				RC->AddRenderer(Current->mRenderer, LocalPose);
			}
		}
	}

	PINT_MESH_CREATE MeshCreate;
	MeshCreate.SetSurfaceData(Verts.GetNbVertices(), Verts.GetVertices(), Indices.GetNbEntries()/3, Indices.GetEntries(), null);
	MeshCreate.mRenderer = RC;
	ActorDesc.SetShape(&MeshCreate);

	const PintActorHandle MergedActor = CreateObject(ActorDesc);
	return MergedActor;
}


#ifdef TOSEE
PintActorHandle EditorPlugin::ConvexDecomp(PintActorHandle handle)
{

	const PintActorHandle handle = handles[i];
	const ActorData* Data = HandleToActorData(handle);
	const PintActorHandle NativeActorHandle = GetNativeActorHandle(Data);

	PR ActorPR;
	GetCurrentActorPose(ActorPR, mEdited, NativeActorHandle, Data);

	const Matrix4x4 ActorMat44 = ActorPR;

	const udword NbShapes = Data->mShapes.GetNbEntries();
	for(udword j=0;j<NbShapes;j++)
	{
		const ShapeData* Current = (const ShapeData*)Data->mShapes.GetEntry(j);

		const Matrix4x4 Local44 = PR(Current->mLocalPos, Current->mLocalRot);
		const Matrix4x4 Combo = Local44 * ActorMat44;

		if(Current->mType==PINT_SHAPE_MESH)
		{
			const MeshShapeData* MeshData = static_cast<const MeshShapeData*>(Current);
			const udword NbVerts = MeshData->mNbVerts;
			for(udword k=0;k<NbVerts;k++)
			{
				Point p = MeshData->mVerts[k] * Combo;
				p -= ActorDesc.mPosition;

				ConvexVerts.AddVertex(p);
			}
		}
	}

	return h;
}
#endif

void EditorPlugin::GetAllActors(PtrContainer& actors)
{
	//### we could return the array directly I think
	const udword NbActors = mEditorActors.GetNbEntries();
	for(udword i=0;i<NbActors;i++)
	{
		const ActorData* CurrentActor = HandleToActorData(mEditorActors.GetEntry(i));
		actors.AddPtr(CurrentActor);
	}
}

///////////////////////////////////////////////////////////////////////////////

void Editor_HideActors(udword nb, PintActorHandle* handles)
{
	if(gEditor)
		gEditor->HideActors(nb, handles);
}

void Editor_ResetPoses(udword nb, PintActorHandle* handles)
{
	if(gEditor)
		gEditor->ResetPoses(nb, handles);
}

PintActorHandle Editor_CreateCompound(udword nb, PintActorHandle* handles, const MergeParams& params)
{
	if(gEditor)
		return gEditor->MergeActors(nb, handles, DefaultMerge(), params);
	return null;
}

PintActorHandle Editor_CreateSingleConvex(udword nb, PintActorHandle* handles)
{
	if(gEditor)
		return gEditor->CreateSingleConvex(nb, handles);
	return null;
}

PintActorHandle Editor_CreateSingleMesh(udword nb, PintActorHandle* handles)
{
	if(gEditor)
		return gEditor->CreateSingleMesh(nb, handles);
	return null;
}

void Editor_GetAllActors(PtrContainer& actors)
{
	if(gEditor)
		gEditor->GetAllActors(actors);
}

PintActorHandle Editor_MakeStatic(PintActorHandle h)
{
	//### joints?
	if(gEditor)
		return gEditor->MergeActors(1, &h, DefaultMerge(), MergeParams(null, 0.0f));
	return null;
}

PintActorHandle Editor_MakeDynamic(PintActorHandle h)
{
	//### joints?
	if(gEditor)
		return gEditor->MergeActors(1, &h, DefaultMerge(), MergeParams(null, 1.0f));
	return null;
}

/*PintActorHandle Editor_MakeKinematic(PintActorHandle h)
{
	//### joints?
	if(gEditor)
		return gEditor->MergeActors(1, &h, DefaultMerge(), MergeParams(null, 1.0f, null, true));
	return null;
}*/

PintActorHandle Editor_MakeSphere(PintActorHandle h)
{
	if(gEditor)
		return gEditor->MergeActors(1, &h, MakeSphere(), MergeParams());
	return null;
}

PintActorHandle Editor_MakeBox(PintActorHandle h)
{
	if(gEditor)
		return gEditor->MergeActors(1, &h, MakeBox(), MergeParams());
	return null;
}

PintActorHandle Editor_MakeConvex(PintActorHandle h)
{
	if(gEditor)
		return gEditor->MergeActors(1, &h, MakeConvex(), MergeParams());
	return null;
}

PintActorHandle Editor_MakeMesh(PintActorHandle h)
{
	if(gEditor)
		return gEditor->MergeActors(1, &h, MakeMesh(), MergeParams());
	return null;
}

#ifdef TOSEE
PintActorHandle Editor_MakeConvexDecomp(PintActorHandle h)
{
	if(gEditor)
		return gEditor->ConvexDecomp(h);
	return null;
}
#endif

PintActorHandle Editor_GetNativeHandle(PintActorHandle h)
{
	if(gEditor)
	{
		const ActorData* Data = HandleToActorData(h);
		return GetNativeActorHandle(Data);
	}
	return h;
	//return null;
}

PintShapeHandle Editor_GetNativeHandle(PintShapeHandle h)
{
	if(gEditor)
	{
		const ShapeData* Data = HandleToShapeData(h);
		return GetNativeShapeHandle(Data);
	}
	return h;
	//return null;
}
