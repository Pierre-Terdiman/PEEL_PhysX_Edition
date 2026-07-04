///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PINT_Box3d.h"

#include "..\PINT_Common\PINT_Ice.h"
//#include "..\PINT_Common\PINT_Common.cpp"

#include "..\PINT_Common\PINT_Common.h"
//#include "..\PINT_Common\PINT_IceAllocatorSwitch.h"
#include "..\PintShapeRenderer.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN64
	#pragma comment(lib, "../../Ice/Lib64/IceCore64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceMaths64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Contact64.lib")
	#pragma comment(lib, "../../Ice/Lib64/Meshmerizer64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceImageWork64.lib")
	#pragma comment(lib, "../../Ice/Lib64/IceGUI64.lib")
//	#pragma comment(lib, "../../Ice/Lib64/IML64.lib")

	#ifdef _DEBUG
		#pragma comment(lib, "../../../../PEEL_Externals/Box3d/Lib/x64/Debug/box3dd.lib")	// CMake debug postfix 'd'
	#else
		#pragma comment(lib, "../../../../PEEL_Externals/Box3d/Lib/x64/Release/box3d.lib")
	#endif
#else
	#pragma comment(lib, "../../Ice/Lib/IceCore.lib")
	#pragma comment(lib, "../../Ice/Lib/IceMaths.lib")
	#pragma comment(lib, "../../Ice/Lib/Contact.lib")
	#pragma comment(lib, "../../Ice/Lib/Meshmerizer.lib")
	#pragma comment(lib, "../../Ice/Lib/IceImageWork.lib")
	#pragma comment(lib, "../../Ice/Lib/IceGUI.lib")
	#pragma comment(lib, "../../Ice/Lib/IML.lib")
#endif

///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include <limits>
#include <thread>

///////////////////////////////////////////////////////////////////////////////

// --- Options (wired to the plugin UI) ---
static bool		gAllowSleeping	= true;
static bool		gEnableCCD		= false;	// continuous collision (bullets): off by default, like other engines
static bool		gRenderDebug	= false;	// debug-draw overlay: off by default (large render overhead)
static udword	gNbThreads		= 0;	// 0 => single-threaded internal scheduler
static udword	gNbSubsteps		= 4;	// Box3d sub-steps per Step
static float	gLinearDamping	= 0.1f;		// PEEL default (matches the PhysX/Jolt plugins)
static float	gAngularDamping	= 0.05f;	// PEEL default
static bool		gShareMeshData	= true;		// cook a triangle mesh once and share the BVH across bodies
static float	gDefaultFriction	= 0.5f;		// PEEL default; used when a shape carries no material
static float	gDefaultRestitution	= 0.0f;		// PEEL default
static float	gContactHertz		= 30.0f;	// Box3d default; contact "spring" frequency (higher = stiffer contacts)
static float	gContactDampingRatio	= 10.0f;	// Box3d default; contact damping (higher = more energy absorbed at contacts)

// --- Conversions between PEEL/Ice math and Box3d math ---
// Point <-> b3Vec3 : both are 3 contiguous floats.
// Quat  <-> b3Quat : Ice Quat is {Point p; float w} (scalar last, ctor takes (w,x,y,z));
//                    Box3d b3Quat is {b3Vec3 v; float s} (scalar last). So field order matches.
static inline b3Vec3	ToB3Vec3(const Point& p)	{ b3Vec3 v; v.x = p.x; v.y = p.y; v.z = p.z; return v;							}
static inline Point		ToPoint (const b3Vec3& v)	{ return Point(v.x, v.y, v.z);													}
static inline b3Quat	ToB3Quat(const Quat& q)		{ b3Quat r; r.v.x = q.p.x; r.v.y = q.p.y; r.v.z = q.p.z; r.s = q.w; return r;	}
static inline Quat		ToQuat  (const b3Quat& q)	{ return Quat(q.s, q.v.x, q.v.y, q.v.z);	/* Ice ctor is (w,x,y,z) */			}

// Cook a PEEL triangle surface into a standalone Box3d mesh (int32 index triples, matching the
// SurfaceInterface layout). The returned b3MeshData* is REFERENCED (not copied) by the mesh
// shape, so the caller must keep it alive for the shape's lifetime and b3DestroyMesh() it after.
static b3MeshData* Box3d_CreateMesh(const SurfaceInterface& s)
{
	if(s.mNbVerts<3 || s.mNbFaces<1)	// Box3d requires >=3 verts and >=1 triangle
		return null;

	std::vector<b3Vec3> Verts(s.mNbVerts);
	for(udword i=0;i<s.mNbVerts;i++)
		Verts[i] = ToB3Vec3(s.mVerts[i]);

	std::vector<int32_t> Indices(size_t(s.mNbFaces)*3);
	for(udword i=0;i<s.mNbFaces;i++)
	{
		if(s.mDFaces)	// 32-bit index buffer
		{
			Indices[i*3+0] = int32_t(s.mDFaces[i*3+0]);
			Indices[i*3+1] = int32_t(s.mDFaces[i*3+1]);
			Indices[i*3+2] = int32_t(s.mDFaces[i*3+2]);
		}
		else			// 16-bit index buffer
		{
			Indices[i*3+0] = int32_t(s.mWFaces[i*3+0]);
			Indices[i*3+1] = int32_t(s.mWFaces[i*3+1]);
			Indices[i*3+2] = int32_t(s.mWFaces[i*3+2]);
		}
	}

	b3MeshDef Def		= {};
	Def.vertices		= Verts.data();
	Def.indices			= Indices.data();
	Def.materialIndices	= null;
	Def.weldTolerance	= 0.0f;
	Def.vertexCount		= int(s.mNbVerts);
	Def.triangleCount	= int(s.mNbFaces);
	Def.weldVertices	= false;
	Def.useMedianSplit	= false;
	Def.identifyEdges	= true;	// shared-edge adjacency for correct concave-edge contacts

	return b3CreateMesh(&Def, null, 0);	// self-contained; Verts/Indices can be freed on return
}

// App-owned heightfield samples returned from CreateHeightfieldObject. Box3d bakes scale into the
// cooked heightfield (b3CreateHeightFieldShape takes no scale) and the U/V scale lives on the shape
// descriptor, so we defer cooking to CreateShape and just stash the raw grid here (mirroring the
// mesh IndexedSurface handle).
struct Box3dHeightfieldData
{
	udword				mNbU;		// columns (-> Z)
	udword				mNbV;		// rows    (-> X)
	float				mMinHeight;
	float				mMaxHeight;
	std::vector<float>	mHeights;	// mNbU*mNbV samples, U (column) fast: mHeights[v*mNbU + u]
};

// Handle returned by CreateMeshObject. Holds the raw geometry (to cook a private mesh per body when
// mesh-data sharing is off) plus, when sharing is on, a single cooked b3MeshData shared by all bodies.
// The shared b3MeshData is owned by Box3dPint::mSharedMeshes (freed in Close after the world), NOT by
// this handle -- so DeleteMeshObject can run before teardown without dangling live shapes.
struct Box3dMeshObject
{
	IndexedSurface	mSurface;
	b3MeshData*		mShared;	// shared cooked mesh when gShareMeshData, else null
};

// Box3d encodes the final hull with uint8 indices, so vertex/face/half-edge counts are hard-capped
// at B3_HULL_LIMIT (255) -- there is no runtime setting to raise it. For a convex that would exceed
// the cap, b3CreateHull returns null; we retry with a progressively smaller vertex budget so the
// object still gets a (slightly coarser) collision hull instead of no shape at all.
static b3HullData* Box3d_CreateHullLimited(const b3Vec3* points, int count)
{
	int budget			= count;	// full detail first; b3CreateHull clamps this to [4, 255]
	b3HullData* Hull	= b3CreateHull(points, count, budget);
	while(!Hull && budget>8)
	{
		budget	= (budget*3)/4;		// shrink the vertex budget ~25% and retry
		Hull	= b3CreateHull(points, count, budget);
	}
	if(Hull && budget<count)
		printf("Box3d plugin: convex hull exceeded Box3d's 255 half-edge limit; simplified (input %d verts, vertex budget reduced to %d) -- collision geometry differs from other engines.\n", count, budget);
	else if(!Hull)
		printf("Box3d plugin: failed to build a convex hull from %d verts; object has no collision shape.\n", count);
	return Hull;
}

///////////////////////////////////////////////////////////////////////////////

Box3d_SceneAPI::Box3d_SceneAPI(Pint& pint) : Pint_Scene(pint)
{
}

Box3d_SceneAPI::~Box3d_SceneAPI()
{
}

// PEEL builds "deferred" actors with mAddToWorld==false -- CreateObject creates them disabled
// (BodyDef.isEnabled=false). AddActors enables them so they start simulating, the Box3d equivalent of
// Jolt's BodyInterface::AddBody. Enabling a dynamic body also brings it into the awake set.
bool Box3d_SceneAPI::AddActors(udword nb_actors, const PintActorHandle* actors)
{
	if(!actors)
		return false;
	while(nb_actors--)
	{
		const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(*actors++);
		if(Actor && !b3Body_IsEnabled(Actor->mBody))
			b3Body_Enable(Actor->mBody);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

Box3d_ActorAPI::Box3d_ActorAPI(Pint& pint) : Pint_Actor(pint)
{
}

Box3d_ActorAPI::~Box3d_ActorAPI()
{
}

// The Pint_Actor API operates on a PintActorHandle, which for this plugin is a Box3dActor*.
static inline b3BodyId GetActorBody(PintActorHandle handle)
{
	return reinterpret_cast<const Box3dActor*>(handle)->mBody;
}

const char* Box3d_ActorAPI::GetName(PintActorHandle handle) const
{
	return null;
}

bool Box3d_ActorAPI::SetName(PintActorHandle handle, const char* name)
{
	return false;
}

bool Box3d_ActorAPI::GetWorldBounds(PintActorHandle handle, AABB& bounds) const
{
	const b3AABB Box = b3Body_ComputeAABB(GetActorBody(handle));
	bounds.mMin = ToPoint(Box.lowerBound);
	bounds.mMax = ToPoint(Box.upperBound);
	return true;
}

void Box3d_ActorAPI::WakeUp(PintActorHandle handle)
{
	b3Body_SetAwake(GetActorBody(handle), true);
}

float Box3d_ActorAPI::GetLinearDamping(PintActorHandle handle) const
{
	return b3Body_GetLinearDamping(GetActorBody(handle));
}

bool Box3d_ActorAPI::SetLinearDamping(PintActorHandle handle, float damping)
{
	b3Body_SetLinearDamping(GetActorBody(handle), damping);
	return true;
}

float Box3d_ActorAPI::GetAngularDamping(PintActorHandle handle) const
{
	return b3Body_GetAngularDamping(GetActorBody(handle));
}

bool Box3d_ActorAPI::SetAngularDamping(PintActorHandle handle, float damping)
{
	b3Body_SetAngularDamping(GetActorBody(handle), damping);
	return true;
}

bool Box3d_ActorAPI::GetLinearVelocity(PintActorHandle handle, Point& linear_velocity, bool world_space) const
{
	linear_velocity = ToPoint(b3Body_GetLinearVelocity(GetActorBody(handle)));	// Box3d velocities are world-space
	return true;
}

bool Box3d_ActorAPI::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity, bool world_space)
{
	b3Body_SetLinearVelocity(GetActorBody(handle), ToB3Vec3(linear_velocity));
	return true;
}

bool Box3d_ActorAPI::GetAngularVelocity(PintActorHandle handle, Point& angular_velocity, bool world_space) const
{
	angular_velocity = ToPoint(b3Body_GetAngularVelocity(GetActorBody(handle)));
	return true;
}

bool Box3d_ActorAPI::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity, bool world_space)
{
	b3Body_SetAngularVelocity(GetActorBody(handle), ToB3Vec3(angular_velocity));
	return true;
}

float Box3d_ActorAPI::GetMass(PintActorHandle handle) const
{
	return b3Body_GetMass(GetActorBody(handle));	// 0 for static bodies
}

bool Box3d_ActorAPI::GetLocalInertia(PintActorHandle handle, Point& inertia) const
{
	const b3Matrix3 I = b3Body_GetLocalRotationalInertia(GetActorBody(handle));
	inertia = Point(I.cx.x, I.cy.y, I.cz.z);	// diagonal of the local inertia tensor
	return true;
}

///////////////////////////////////////////////////////////////////////////////

Box3dPint::Box3dPint() : mSceneAPI(*this), mActorAPI(*this), mHasWorld(false)
{
	mWorld = b3WorldId();	// zero-initialised => invalid until Init()
	mTimestep = 1.0f/60.0f;
	for(int i=0;i<32;i++)
		mGroupMask[i] = ~uint64_t(0);	// every group collides with everything until SetDisabledGroups()
}

Box3dPint::~Box3dPint()
{
}

const char* Box3dPint::GetName() const
{
	// Append the Box3d version + worker-thread count, like the other engines.
	const b3Version Ver = b3GetVersion();
	const unsigned int HardwareThreads = std::thread::hardware_concurrency();
	const int NbThreads = gNbThreads ? int(gNbThreads) : (HardwareThreads>1 ? int(HardwareThreads)-1 : 1);
	return _F("Box3d %d.%d.%d (%dT)", Ver.major, Ver.minor, Ver.revision, NbThreads);
}

const char* Box3dPint::GetUIName() const
{
	// The version is constant at runtime, so build the string once into a persistent buffer
	// (safer than returning a transient _F() result that the UI may hold on to).
	static char sName[32] = {0};
	if(!sName[0])
	{
		const b3Version Ver = b3GetVersion();
		sprintf(sName, "Box3d %d.%d.%d", Ver.major, Ver.minor, Ver.revision);
	}
	return sName;
}

void Box3dPint::GetCaps(PintCaps& caps) const
{
	// Rigid bodies (box/sphere/capsule/convex), compounds, kinematics, triangle meshes,
	// heightfields, cylinders, collision groups, the main joints and scene queries (raycasts,
	// box/sphere/capsule sweeps, sphere overlaps). Not yet: D6/gear/rack joints.
	caps.mSupportRigidBodySimulation	= true;
	caps.mSupportConvexes				= true;
	caps.mSupportCompounds				= true;
	caps.mSupportMeshes					= true;
	caps.mSupportHeightfields			= true;
	caps.mSupportCollisionGroups		= true;
	caps.mSupportCylinders				= true;
	caps.mSupportKinematics				= true;

	caps.mSupportSphericalJoints		= true;
	caps.mSupportHingeJoints			= true;
	caps.mSupportFixedJoints			= true;
	caps.mSupportPrismaticJoints		= true;
	caps.mSupportDistanceJoints			= true;

	caps.mSupportRaycasts				= true;
	caps.mSupportBoxSweeps				= true;
	caps.mSupportSphereSweeps			= true;
	caps.mSupportCapsuleSweeps			= true;
	caps.mSupportSphereOverlaps			= true;
}

void Box3dPint::Init(const PINT_WORLD_CREATE& desc)
{
	b3WorldDef WorldDef		= b3DefaultWorldDef();
	WorldDef.gravity		= ToB3Vec3(desc.mGravity);
	// 0 == automatic: use all hardware threads but one (leave a core for the main thread), like Jolt.
	// Box3d spawns its own worker threads when workerCount>1 (internal scheduler); it clamps to [1, B3_MAX_WORKERS].
	const unsigned int HardwareThreads = std::thread::hardware_concurrency();
	WorldDef.workerCount	= gNbThreads ? gNbThreads : (HardwareThreads>1 ? HardwareThreads-1 : 1);
	WorldDef.enableSleep		= gAllowSleeping;
	WorldDef.enableContinuous	= gEnableCCD;	// continuous collision (CCD) toggle
	WorldDef.contactHertz		= gContactHertz;		// contact softness: higher = stiffer, less energy absorbed
	WorldDef.contactDampingRatio	= gContactDampingRatio;	// overdamped (10) contacts can eat marginal tipping energy

	mTimestep = desc.mTimestep>0.0f ? desc.mTimestep : (1.0f/60.0f);	// for the CCD bullet heuristic

	mWorld		= b3CreateWorld(&WorldDef);
	mHasWorld	= b3World_IsValid(mWorld);

	if(mHasWorld)
	{
		// Persistent static body used as the anchor for joints whose actor is null
		// (Box3d has no "fixed to world" sentinel like Jolt's Body::sFixedToWorld).
		b3BodyDef AnchorDef	= b3DefaultBodyDef();
		AnchorDef.type		= b3_staticBody;
		mWorldBody			= b3CreateBody(mWorld, &AnchorDef);
	}
}

void Box3dPint::Close()
{
	for(Box3dJoint* Joint : mJoints)
		delete Joint;
	mJoints.clear();

	// Destroy the world first: it frees all bodies/shapes/joints. Only then is it safe to free
	// the app-owned mesh data those shapes referenced (b3DestroyWorld does not own it).
	if(mHasWorld)
	{
		b3DestroyWorld(mWorld);
		mHasWorld = false;
	}

	for(Box3dActor* Actor : mActors)
	{
		for(b3MeshData* Mesh : Actor->mMeshes)
			b3DestroyMesh(Mesh);
		for(b3HeightFieldData* HF : Actor->mHeightfields)
			b3DestroyHeightField(HF);
		delete Actor;
	}
	mActors.clear();

	// Shared cooked meshes are owned here (referenced by many bodies); free after the world.
	for(b3MeshData* Mesh : mSharedMeshes)
		b3DestroyMesh(Mesh);
	mSharedMeshes.clear();
}

void Box3dPint::SetGravity(const Point& gravity)
{
	if(mHasWorld)
		b3World_SetGravity(mWorld, ToB3Vec3(gravity));
}

udword Box3dPint::Update(float dt)
{
	if(mHasWorld)
		b3World_Step(mWorld, dt, gNbSubsteps>0 ? int(gNbSubsteps) : 1);
	return udword(b3GetByteCount());
}

Point Box3dPint::GetMainColor()
{
	return Point(225.0f/255.0f, 240.0f/255.0f, 254.0f/255.0f);
}

void Box3dPint::Render(PintRender& renderer, PintRenderPass render_pass)
{
	for(Box3dActor* Actor : mActors)
	{
		if(!renderer.SetCurrentActor(reinterpret_cast<PintActorHandle>(Actor)))
			continue;

		const b3WorldTransform Xform = b3Body_GetTransform(Actor->mBody);
		const PR BodyPose(ToPoint(Xform.p), ToQuat(Xform.q));

		for(const Box3dShapeData& Shape : Actor->mShapes)
		{
			if(!Shape.mRenderer)
				continue;

			// World pose of the shape = body pose composed with the shape's local pose.
			// NB: verify the Ice quaternion multiply order matches (world = Body * Local).
			PR ShapePose;
			ShapePose.mPos = BodyPose.mPos + BodyPose.mRot.Rotate(Shape.mLocalPose.mPos);
			ShapePose.mRot = BodyPose.mRot * Shape.mLocalPose.mRot;

			renderer.DrawShape(Shape.mRenderer, ShapePose);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Debug visualization.
//	Box3d's b3World_Draw only draws shapes through a createDebugShape/DrawShapeFcn caching
//	callback pair (physics_world.c) that we'd have to register on the world -- so instead we
//	iterate the shapes ourselves and draw their ACTUAL physics geometry via PintRender. This is
//	the tool to spot physics-vs-render divergence.
///////////////////////////////////////////////////////////////////////////////

static void Box3d_DrawWireSphere(PintRender& renderer, const Point& center, float radius, const Point& color)
{
	renderer.DrawWireframeSphere(Sphere(center, radius), color);
}

static void Box3d_DrawWireHull(PintRender& renderer, const b3HullData* hull, const b3Transform& xf, const Point& color)
{
	const b3Vec3*			pts			= b3GetHullPoints(hull);
	const b3HullHalfEdge*	edges		= b3GetHullEdges(hull);
	const int				nbHalfEdges	= hull->edgeCount;
	for(int e=0;e<nbHalfEdges;e++)
		if(e < edges[e].twin)	// each undirected edge has two half-edges; draw it once
		{
			const Point a = ToPoint(b3TransformPoint(xf, pts[edges[e].origin]));
			const Point b = ToPoint(b3TransformPoint(xf, pts[edges[edges[e].twin].origin]));
			renderer.DrawLine(a, b, color);
		}
}

static void Box3d_DrawWireAABB(PintRender& renderer, const b3AABB& aabb, const Point& color)
{
	const b3Vec3& lo = aabb.lowerBound;
	const b3Vec3& hi = aabb.upperBound;
	const Point c[8] = {	Point(lo.x,lo.y,lo.z), Point(hi.x,lo.y,lo.z), Point(hi.x,hi.y,lo.z), Point(lo.x,hi.y,lo.z),
							Point(lo.x,lo.y,hi.z), Point(hi.x,lo.y,hi.z), Point(hi.x,hi.y,hi.z), Point(lo.x,hi.y,hi.z) };
	static const int edges[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
	for(int i=0;i<12;i++)
		renderer.DrawLine(c[edges[i][0]], c[edges[i][1]], color);
}

// Draw the ACTUAL cooked heightfield surface, reading Box3d's own stored heights with its exact
// index convention (height_field.c): height(col,row) = minHeight + heightScale*comp[row*columnCount+col],
// column -> local X (scale.x), row -> local Z (scale.z). If this overlay matches the render, the cook
// is right; if it's transposed vs the render, the cook's index order is wrong.
static void Box3d_DrawWireHeightField(PintRender& renderer, const b3HeightFieldData* hf, const b3Transform& xf, const Point& color)
{
	const uint16_t*	comp	= b3GetHeightFieldCompressedHeights(hf);
	const int		cols	= hf->columnCount;	// -> X
	const int		rows	= hf->rowCount;		// -> Z

	for(int r=0;r<rows;r++)
	{
		for(int c=0;c<cols;c++)
		{
			const float h0 = hf->minHeight + hf->heightScale * float(comp[r*cols + c]);
			const Point p0 = ToPoint(b3TransformPoint(xf, b3Vec3{ hf->scale.x*float(c), h0*hf->scale.y, hf->scale.z*float(r) }));
			if(c+1<cols)
			{
				const float h1 = hf->minHeight + hf->heightScale * float(comp[r*cols + (c+1)]);
				renderer.DrawLine(p0, ToPoint(b3TransformPoint(xf, b3Vec3{ hf->scale.x*float(c+1), h1*hf->scale.y, hf->scale.z*float(r) })), color);
			}
			if(r+1<rows)
			{
				const float h2 = hf->minHeight + hf->heightScale * float(comp[(r+1)*cols + c]);
				renderer.DrawLine(p0, ToPoint(b3TransformPoint(xf, b3Vec3{ hf->scale.x*float(c), h2*hf->scale.y, hf->scale.z*float(r+1) })), color);
			}
		}
	}
}

// Draw the cooked triangle mesh as a wireframe (shared edges are drawn twice; fine for debug).
static void Box3d_DrawWireMesh(PintRender& renderer, const b3Mesh& mesh, const b3Transform& xf, const Point& color)
{
	const b3MeshData*		data	= mesh.data;
	const b3Vec3*			verts	= b3GetMeshVertices(data);
	const b3MeshTriangle*	tris	= b3GetMeshTriangles(data);
	const b3Vec3			scale	= mesh.scale;	// per-instance scale baked at shape creation
	const int				nbTris	= data->triangleCount;
	for(int t=0;t<nbTris;t++)
	{
		const b3MeshTriangle& T = tris[t];
		const b3Vec3 v0 = { verts[T.index1].x*scale.x, verts[T.index1].y*scale.y, verts[T.index1].z*scale.z };
		const b3Vec3 v1 = { verts[T.index2].x*scale.x, verts[T.index2].y*scale.y, verts[T.index2].z*scale.z };
		const b3Vec3 v2 = { verts[T.index3].x*scale.x, verts[T.index3].y*scale.y, verts[T.index3].z*scale.z };
		const Point p0 = ToPoint(b3TransformPoint(xf, v0));
		const Point p1 = ToPoint(b3TransformPoint(xf, v1));
		const Point p2 = ToPoint(b3TransformPoint(xf, v2));
		renderer.DrawLine(p0, p1, color);
		renderer.DrawLine(p1, p2, color);
		renderer.DrawLine(p2, p0, color);
	}
}

void Box3dPint::RenderDebugData(PintRender& renderer)
{
	if(!mHasWorld || !gRenderDebug)
		return;

	const Point Color(0.0f, 1.0f, 0.0f);	// green wireframe overlay

	for(Box3dActor* Actor : mActors)
	{
		const b3Transform BodyXf = b3Body_GetTransform(Actor->mBody);

		b3ShapeId Shapes[256];
		const int NbShapes = b3Body_GetShapes(Actor->mBody, Shapes, 256);
		for(int s=0;s<NbShapes;s++)
		{
			const b3ShapeId Shape = Shapes[s];
			switch(b3Shape_GetType(Shape))
			{
				case b3_capsuleShape:
				{
					const b3Capsule c = b3Shape_GetCapsule(Shape);
					const Point p1 = ToPoint(b3TransformPoint(BodyXf, c.center1));
					const Point p2 = ToPoint(b3TransformPoint(BodyXf, c.center2));
					renderer.DrawLine(p1, p2, Color);
					Box3d_DrawWireSphere(renderer, p1, c.radius, Color);
					Box3d_DrawWireSphere(renderer, p2, c.radius, Color);
				}
				break;

				case b3_sphereShape:
				{
					const b3Sphere sp = b3Shape_GetSphere(Shape);
					Box3d_DrawWireSphere(renderer, ToPoint(b3TransformPoint(BodyXf, sp.center)), sp.radius, Color);
				}
				break;

				case b3_hullShape:
					Box3d_DrawWireHull(renderer, b3Shape_GetHull(Shape), BodyXf, Color);
				break;

				case b3_heightShape:
					Box3d_DrawWireHeightField(renderer, b3Shape_GetHeightField(Shape), BodyXf, Color);
				break;

				case b3_meshShape:
					Box3d_DrawWireMesh(renderer, b3Shape_GetMesh(Shape), BodyXf, Color);
				break;

				default:	// compound (and anything else): draw the world-space bounds
					Box3d_DrawWireAABB(renderer, b3Shape_GetAABB(Shape), Color);
				break;
			}
		}
	}
}

void Box3dPint::CreateShape(b3BodyId body, const PINT_SHAPE_CREATE* shape_create, Box3dActor* actor)
{
	const PR LocalPose(shape_create->mLocalPos, shape_create->mLocalRot);

	b3ShapeDef ShapeDef	= b3DefaultShapeDef();
	ShapeDef.density	= 1.0f;
	ShapeDef.filter		= MakeGroupFilter(actor->mGroup);	// PEEL collision group -> category/mask bits
	if(shape_create->mMaterial)
	{
		// Box3d exposes a single friction coefficient (no static/dynamic split).
		ShapeDef.baseMaterial.friction		= shape_create->mMaterial->mDynamicFriction;
		ShapeDef.baseMaterial.restitution	= shape_create->mMaterial->mRestitution;
	}
	else	// no per-shape material: fall back to the global defaults (like the PhysX/Jolt plugins)
	{
		ShapeDef.baseMaterial.friction		= gDefaultFriction;
		ShapeDef.baseMaterial.restitution	= gDefaultRestitution;
	}

	// We bake the local pose into the Box3d geometry (offset/rotated), and keep it in
	// Box3dShapeData too so Render() can place the shape renderer at the same pose.
	b3Transform LocalXform;
	LocalXform.p = ToB3Vec3(LocalPose.mPos);
	LocalXform.q = ToB3Quat(LocalPose.mRot);

	switch(shape_create->mType)
	{
		case PINT_SHAPE_SPHERE:
		{
			const PINT_SPHERE_CREATE* Create = static_cast<const PINT_SPHERE_CREATE*>(shape_create);
			b3Sphere Sphere;
			Sphere.center	= LocalXform.p;		// orientation is irrelevant for a sphere
			Sphere.radius	= Create->mRadius;
			b3CreateSphereShape(body, &ShapeDef, &Sphere);
		}
		break;

		case PINT_SHAPE_CAPSULE:
		{
			const PINT_CAPSULE_CREATE* Create = static_cast<const PINT_CAPSULE_CREATE*>(shape_create);
			// PEEL's capsule renderer is aligned along the local Y axis. (PhysX's native capsule is
			// X and its plugin rotates Y->X to match; Box3d capsules are 2-point, so we just build
			// the axis along Y directly, like the Jolt plugin -- no render-side conversion needed.)
			const b3Vec3 HalfAxis = b3RotateVector(LocalXform.q, ToB3Vec3(Point(0.0f, Create->mHalfHeight, 0.0f)));
			b3Capsule Capsule;
			Capsule.center1	= b3Vec3{ LocalXform.p.x - HalfAxis.x, LocalXform.p.y - HalfAxis.y, LocalXform.p.z - HalfAxis.z };
			Capsule.center2	= b3Vec3{ LocalXform.p.x + HalfAxis.x, LocalXform.p.y + HalfAxis.y, LocalXform.p.z + HalfAxis.z };
			Capsule.radius	= Create->mRadius;
			b3CreateCapsuleShape(body, &ShapeDef, &Capsule);
		}
		break;

		case PINT_SHAPE_BOX:
		{
			const PINT_BOX_CREATE* Create = static_cast<const PINT_BOX_CREATE*>(shape_create);
			const b3BoxHull Hull = b3MakeTransformedBoxHull(Create->mExtents.x, Create->mExtents.y, Create->mExtents.z, LocalXform);
			b3CreateHullShape(body, &ShapeDef, &Hull.base);
		}
		break;

		case PINT_SHAPE_CONVEX:
		{
			const PINT_CONVEX_CREATE* Create = static_cast<const PINT_CONVEX_CREATE*>(shape_create);
			std::vector<b3Vec3> Pts(Create->mNbVerts);
			for(udword i=0;i<Create->mNbVerts;i++)
				Pts[i] = ToB3Vec3(Create->mVerts[i]);
			b3HullData* Hull = Box3d_CreateHullLimited(Pts.data(), int(Create->mNbVerts));
			if(Hull)
			{
				b3CreateTransformedHullShape(body, &ShapeDef, Hull, LocalXform, b3Vec3{1.0f, 1.0f, 1.0f});
				b3DestroyHull(Hull);	// the shape clones the geometry
			}
		}
		break;

		case PINT_SHAPE_CYLINDER:
		{
			// Box3d has no analytic cylinder: build a faceted Y-axis hull (matches PEEL's Y-axis
			// cylinder renderer, like the capsule). 32 sides -> 192 half-edges, under the 255 cap.
			const PINT_CYLINDER_CREATE* Create = static_cast<const PINT_CYLINDER_CREATE*>(shape_create);
			b3HullData* Hull = b3CreateCylinder(2.0f*Create->mHalfHeight, Create->mRadius, -Create->mHalfHeight, 32);
			if(Hull)
			{
				b3CreateTransformedHullShape(body, &ShapeDef, Hull, LocalXform, b3Vec3{1.0f, 1.0f, 1.0f});
				b3DestroyHull(Hull);	// the shape clones the geometry
			}
		}
		break;

		case PINT_SHAPE_MESH:
		{
			const PINT_MESH_CREATE* Create = static_cast<const PINT_MESH_CREATE*>(shape_create);
			ASSERT(b3Body_GetType(body)==b3_staticBody);	// Box3d mesh shapes: static bodies only
			b3MeshData* Mesh = Box3d_CreateMesh(Create->GetSurface());
			if(Mesh)
			{
				actor->mMeshes.push_back(Mesh);	// must outlive the shape (Box3d references, doesn't clone)
				b3CreateMeshShape(body, &ShapeDef, Mesh, b3Vec3{1.0f, 1.0f, 1.0f});
			}
		}
		break;

		case PINT_SHAPE_MESH2:
		{
			const PINT_MESH_CREATE2* Create = static_cast<const PINT_MESH_CREATE2*>(shape_create);
			const Box3dMeshObject* MO = reinterpret_cast<const Box3dMeshObject*>(Create->mTriangleMesh);
			ASSERT(b3Body_GetType(body)==b3_staticBody);	// Box3d mesh shapes: static bodies only
			if(MO && MO->mShared)
			{
				// Mesh-data sharing on: reference the single cooked mesh (owned by mSharedMeshes).
				b3CreateMeshShape(body, &ShapeDef, MO->mShared, b3Vec3{1.0f, 1.0f, 1.0f});
			}
			else if(MO)
			{
				// Sharing off: cook a private mesh for this body (owned by the actor).
				b3MeshData* Mesh = Box3d_CreateMesh(MO->mSurface.GetSurfaceInterface());
				if(Mesh)
				{
					actor->mMeshes.push_back(Mesh);
					b3CreateMeshShape(body, &ShapeDef, Mesh, b3Vec3{1.0f, 1.0f, 1.0f});
				}
			}
		}
		break;

		case PINT_SHAPE_HEIGHTFIELD:
		{
			const PINT_HEIGHTFIELD_CREATE* Create = static_cast<const PINT_HEIGHTFIELD_CREATE*>(shape_create);
			const Box3dHeightfieldData* HF = reinterpret_cast<const Box3dHeightfieldData*>(Create->mHeightfield);
			ASSERT(b3Body_GetType(body)==b3_staticBody);	// Box3d heightfields: static bodies only
			if(HF)
			{
				// PhysX parity: PEEL V (rows) -> X @ mScaleV, PEEL U (cols) -> Z @ mScaleU, height +Y.
				// Box3d: columnCount=countX (column -> X), rowCount=countZ (row -> Z), heights indexed
				// [row*columnCount + column]. So Box3d column <-> PEEL V, Box3d row <-> PEEL U.
				const int	countX	= int(HF->mNbV);	// Box3d columnCount
				const int	countZ	= int(HF->mNbU);	// Box3d rowCount
				const float	Delta	= HF->mMaxHeight - HF->mMinHeight;

				// Offset heights so the field minimum sits at local Y=0 (matches PhysX; the renderer
				// already subtracts mMinHeight). Note the transpose vs PEEL's mHeights[v*mNbU + u]:
				// Box3d cell (col=v, row=u) lives at [u*mNbV + v] = [row*countX + col].
				std::vector<float> Heights(size_t(countX)*size_t(countZ));
				for(int row=0;row<countZ;row++)			// row <-> PEEL U  (-> Z)
					for(int col=0;col<countX;col++)		// col <-> PEEL V  (-> X)
						Heights[row*countX + col] = HF->mHeights[size_t(col)*HF->mNbU + row] - HF->mMinHeight;

				// Cell materials must be valid (0 = solid; 0xFF would be a hole).
				const int cellsX = countX>1 ? countX-1 : 1;
				const int cellsZ = countZ>1 ? countZ-1 : 1;
				std::vector<uint8_t> Materials(size_t(cellsX)*size_t(cellsZ), 0);

				b3HeightFieldDef Def	= {};
				Def.heights				= Heights.data();
				Def.materialIndices		= Materials.data();
				Def.scale				= b3Vec3{ Create->mScaleV, 1.0f, Create->mScaleU };	// x=rowScale, y=1, z=colScale
				Def.countX				= countX;
				Def.countZ				= countZ;
				Def.globalMinimumHeight	= 0.0f;
				Def.globalMaximumHeight	= (Delta>0.0f) ? Delta : 1.0f;	// avoid a degenerate min==max range
				Def.clockwiseWinding	= false;	// flip to true if faces come out inverted

				b3HeightFieldData* Cooked = b3CreateHeightField(&Def);	// copies def arrays; Heights/Materials freeable after
				if(Cooked)
				{
					actor->mHeightfields.push_back(Cooked);	// must outlive the shape (Box3d references it)
					b3CreateHeightFieldShape(body, &ShapeDef, Cooked);
				}
			}
		}
		break;

		default:
			ASSERT(0);	// unknown shape type
		break;
	}

	Box3dShapeData Data;
	Data.mRenderer	= shape_create->mRenderer;
	Data.mLocalPose	= LocalPose;
	actor->mShapes.push_back(Data);
}

PintActorHandle Box3dPint::CreateObject(const PINT_OBJECT_CREATE& desc)
{
	if(!mHasWorld)
		return null;

	const udword NbShapes = desc.GetNbShapes();
	if(!NbShapes)
		return null;

	const bool IsDynamic = desc.mMass!=0.0f;

	// Box3d mesh & heightfield shapes attach to STATIC bodies only, so if this object carries
	// any such shape, force it static regardless of mass.
	class HasMeshShapeCB : public PintShapeEnumerateCallback
	{
		public:
						HasMeshShapeCB() : mHasMesh(false)	{}
		virtual	void	ReportShape(const PINT_SHAPE_CREATE& create, udword, void*)	override
						{
							if(create.mType==PINT_SHAPE_MESH || create.mType==PINT_SHAPE_MESH2 || create.mType==PINT_SHAPE_HEIGHTFIELD)
								mHasMesh = true;
						}
		bool			mHasMesh;
	};
	HasMeshShapeCB MeshCheck;
	desc.GetNbShapes(&MeshCheck);

	b3BodyDef BodyDef		= b3DefaultBodyDef();
	BodyDef.type			= (MeshCheck.mHasMesh || !IsDynamic) ? b3_staticBody : (desc.mKinematic ? b3_kinematicBody : b3_dynamicBody);
	BodyDef.position		= ToB3Vec3(desc.mPosition);
	BodyDef.rotation		= ToB3Quat(desc.mRotation);
	// PT: Erin says we should do this last, see note below
//	BodyDef.linearVelocity	= ToB3Vec3(desc.mLinearVelocity);
//	BodyDef.angularVelocity	= ToB3Vec3(desc.mAngularVelocity);
	BodyDef.enableSleep		= gAllowSleeping;
	BodyDef.linearDamping	= gLinearDamping;	// per-body in Box3d (no global damping); static/kinematic ignore it
	BodyDef.angularDamping	= gAngularDamping;
	// NB: the CCD "bullet" flag is set selectively below (only for genuinely fast bodies), NOT here
	// from gEnableCCD -- see the detailed note at the b3Body_SetBullet() call.
	BodyDef.isEnabled		= desc.mAddToWorld;	// emulate "not added to world" (e.g. aggregated parts)

	const b3BodyId Body = b3CreateBody(mWorld, &BodyDef);

	Box3dActor* Actor = new Box3dActor;
	Actor->mBody = Body;
	Actor->mGroup = desc.mCollisionGroup;
	b3Body_SetUserData(Body, Actor);	// so scene-query hits can recover the PintActorHandle

	// Create & attach every shape (works for single shapes and compounds).
	class ShapeCB : public PintShapeEnumerateCallback
	{
		public:
						ShapeCB(Box3dPint& pint, b3BodyId body, Box3dActor* actor) : mPint(pint), mBody(body), mActor(actor)	{}
		virtual	void	ReportShape(const PINT_SHAPE_CREATE& create, udword index, void* user_data)	override
						{
							mPint.CreateShape(mBody, &create, mActor);
						}
		Box3dPint&		mPint;
		b3BodyId		mBody;
		Box3dActor*		mActor;
	};
	ShapeCB CB(*this, Body, Actor);
	desc.GetNbShapes(&CB);

	// Mass & inertia: Box3d derives them from shape densities; rescale to the requested mass.
	if(IsDynamic)
	{
		b3Body_ApplyMassFromShapes(Body);

		const bool HasCOMOffset = desc.mCOMLocalOffset.IsNonZero();
		if(desc.mMass>0.0f || HasCOMOffset)
		{
			b3MassData MassData = b3Body_GetMassData(Body);
			if(desc.mMass>0.0f && MassData.mass>0.0f)
			{
				const float Scale	= desc.mMass / MassData.mass;
				MassData.mass		= desc.mMass;
				MassData.inertia.cx	= b3Vec3{ MassData.inertia.cx.x*Scale, MassData.inertia.cx.y*Scale, MassData.inertia.cx.z*Scale };
				MassData.inertia.cy	= b3Vec3{ MassData.inertia.cy.x*Scale, MassData.inertia.cy.y*Scale, MassData.inertia.cy.z*Scale };
				MassData.inertia.cz	= b3Vec3{ MassData.inertia.cz.x*Scale, MassData.inertia.cz.y*Scale, MassData.inertia.cz.z*Scale };
			}
			if(HasCOMOffset)
				MassData.center = b3Vec3{ MassData.center.x + desc.mCOMLocalOffset.x, MassData.center.y + desc.mCOMLocalOffset.y, MassData.center.z + desc.mCOMLocalOffset.z };
			b3Body_SetMassData(Body, MassData);

			// b3Body_SetMassData updates invInertiaLocal but NOT invInertiaWorld (the solver refreshes the
			// latter each step). Re-apply the transform so invInertiaWorld is recomputed from the rescaled
			// inertia NOW -- otherwise an impulse applied before the first step (e.g. DoubleDominoEffect's
			// setup-time AddWorldImpulseAtWorldPos) uses the stale pre-rescale inertia and spins the body wildly.
			b3Body_SetTransform(Body, ToB3Vec3(desc.mPosition), ToB3Quat(desc.mRotation));
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Continuous collision (CCD) in Box3d -- important, non-obvious behavior:
	//
	// Box3d has TWO continuous paths (see box3d/src/solver.c):
	//   * A NON-BULLET fast dynamic body only sweeps against STATIC geometry (b3SolveContinuous,
	//     "Continuous collision of dynamic versus static"). It will still tunnel through other
	//     DYNAMIC bodies.
	//   * A BULLET fast body additionally sweeps against dynamic bodies -- BUT the continuous query
	//     explicitly skips other bullets ("Skip bullets"), so bullet-vs-bullet never does CCD.
	//
	// Consequences:
	//   * World enableContinuous alone => CCD only vs static. (A fast object vs a static wall works.)
	//   * Dynamic-vs-dynamic CCD (e.g. a fast box into a stack of resting dynamic boxes) needs the
	//     fast object to be a BULLET and the targets to be NON-bullets.
	//   * So we must NOT flag every dynamic body as a bullet (as enabling CCD on all bodies in
	//     PhysX/Jolt effectively does): that makes every pair bullet-vs-bullet and disables
	//     dynamic-vs-dynamic CCD entirely (this was observed in PEEL's stacking CCD test).
	//
	// Heuristic: flag a body as a bullet only when it is genuinely "fast" at creation -- its initial
	// velocity would carry it more than ~half its smallest extent in one step (same spirit as Box3d's
	// own internal fast-body test). Projectiles (large initial velocity) become bullets; bodies at
	// rest (zero initial velocity, e.g. a stack) stay non-bullets and act as valid CCD targets.
	// Inherent Box3d limitation that remains: two fast bullets aimed at each other still won't do CCD
	// against one another (bullet-vs-bullet is unsupported).
	// ------------------------------------------------------------------------------------------------
	if(gEnableCCD && b3Body_GetType(Body)==b3_dynamicBody)
	{
		const b3AABB Bounds = b3Body_ComputeAABB(Body);
		float MinExtent = Bounds.upperBound.x - Bounds.lowerBound.x;
		const float ExtY = Bounds.upperBound.y - Bounds.lowerBound.y;
		const float ExtZ = Bounds.upperBound.z - Bounds.lowerBound.z;
		if(ExtY<MinExtent)	MinExtent = ExtY;
		if(ExtZ<MinExtent)	MinExtent = ExtZ;

		const float Speed = desc.mLinearVelocity.Magnitude();
		if(Speed*mTimestep > 0.5f*MinExtent)	// moves more than half its smallest size per step
			b3Body_SetBullet(Body, true);
	}

	mActors.push_back(Actor);

	// PT: Erin says:
	// "When you create a body with non-zero angular velocity then shift the center of mass (by adding shapes),
	// this adds some linear velocity. Apparently PhysX and Jolt don't apply this correction. To make the spinning
	// samples work, set body velocity last."
	b3Body_SetLinearVelocity(Actor->mBody, ToB3Vec3(desc.mLinearVelocity));
	b3Body_SetAngularVelocity(Actor->mBody, ToB3Vec3(desc.mAngularVelocity));
	return reinterpret_cast<PintActorHandle>(Actor);
}

bool Box3dPint::ReleaseObject(PintActorHandle handle)
{
	Box3dActor* Actor = reinterpret_cast<Box3dActor*>(handle);
	if(!Actor)
		return false;

	if(b3Body_IsValid(Actor->mBody))
		b3DestroyBody(Actor->mBody);

	// Free app-owned cooked meshes/heightfields AFTER the body/shape that referenced them.
	for(b3MeshData* Mesh : Actor->mMeshes)
		b3DestroyMesh(Mesh);
	for(b3HeightFieldData* HF : Actor->mHeightfields)
		b3DestroyHeightField(HF);

	for(size_t i=0;i<mActors.size();i++)
		if(mActors[i]==Actor)
		{
			mActors[i] = mActors.back();
			mActors.pop_back();
			break;
		}

	delete Actor;
	return true;
}

// Box3d joint frames are relative to the BODY ORIGIN (not the COM), so we use PEEL's local
// pivots verbatim (unlike Jolt, which subtracts the COM). The b3 joint axis conventions are:
// revolute rotates about frame Z, prismatic slides along frame X -- so we rotate the canonical
// axis onto the PINT joint axis when building the frame.
static b3Transform Box3d_MakeAxisFrame(const Point& pivot, const Point& axis, const b3Vec3& canonical)
{
	b3Transform Frame;
	Frame.p = ToB3Vec3(pivot);
	Frame.q = b3ComputeQuatBetweenUnitVectors(canonical, b3Normalize(ToB3Vec3(axis)));
	return Frame;
}

// Build both revolute frames from a COMMON world axis so the joint's reference orientation is identity at
// the rest pose, whatever the bodies' initial relative orientation. Box3d_MakeAxisFrame derives each
// frame's perpendicular basis from that body's LOCAL axis independently; when the bodies are misoriented
// (e.g. ragdoll bones) those bases diverge, giving a wrong limit reference -> a huge violation at t=0 ->
// explosion. This mirrors PhysX's PxSetJointGlobalFrame: one world frame, expressed in each body's space.
static void Box3d_MakeConsistentAxisFrames(b3BodyId bodyA, const Point& pivotA, const Point& axisA,
										   b3BodyId bodyB, const Point& pivotB,
										   const b3Vec3& canonical, b3Transform& frameA, b3Transform& frameB)
{
	const b3Quat qA = b3Body_GetRotation(bodyA);
	const b3Quat qB = b3Body_GetRotation(bodyB);
	const b3Vec3 worldAxis = b3Normalize(b3RotateVector(qA, ToB3Vec3(axisA)));	// common hinge axis, in world
	const b3Quat qW = b3ComputeQuatBetweenUnitVectors(canonical, worldAxis);		// one world frame orientation
	frameA.p = ToB3Vec3(pivotA);
	frameA.q = b3InvMulQuat(qA, qW);	// qA^-1 * qW: the world frame expressed in body A
	frameB.p = ToB3Vec3(pivotB);
	frameB.q = b3InvMulQuat(qB, qW);	// qB^-1 * qW: the world frame expressed in body B
}

// Given frame A (already built from PEEL's local data) and the two bodies' rest orientations, build frame
// B so that worldFrameB == worldFrameA at rest -- i.e. the joint's reference (its limit/cone zero) is
// identity regardless of the bodies' initial relative orientation. Works for any joint type (it just
// matches B's world frame to A's), and keeps A's PEEL-authored frame (e.g. a spherical cone axis).
static b3Transform Box3d_MatchFrameB(b3BodyId bodyA, const b3Transform& frameA, b3BodyId bodyB, const Point& pivotB)
{
	const b3Quat qA = b3Body_GetRotation(bodyA);
	const b3Quat qB = b3Body_GetRotation(bodyB);
	b3Transform frameB;
	frameB.p = ToB3Vec3(pivotB);
	frameB.q = b3InvMulQuat(qB, b3MulQuat(qA, frameA.q));	// qB^-1 * (qA * frameA.q)
	return frameB;
}

static b3Transform Box3d_MakePRFrame(const PR& pr)
{
	b3Transform Frame;
	Frame.p = ToB3Vec3(pr.mPos);
	Frame.q = ToB3Quat(pr.mRot);
	return Frame;
}

static inline b3Quat Box3d_QuatIdentity()
{
	b3Quat q;
	q.v.x = q.v.y = q.v.z = 0.0f;
	q.s = 1.0f;
	return q;
}

// Box3d solves the velocity motor against contacts iteratively; an unbounded motor (the old 1e6)
// can overpower a contact and drive the body through an obstacle -- PhysX/Jolt keep the contact
// dominant. We cap the motor to a bounded acceleration scaled by the driven body's inertia
// (revolute/spherical) or mass (prismatic), so it still spins up quickly but a solid contact stalls it.
static const float BOX3D_MOTOR_ANG_ACCEL = 250.0f;	// rad/s^2
static const float BOX3D_MOTOR_LIN_ACCEL = 250.0f;	// m/s^2

static float Box3d_MeanInertia(b3BodyId body)	// 0 for non-dynamic bodies (treated as immovable)
{
	if(b3Body_GetType(body)!=b3_dynamicBody)
		return 0.0f;
	const b3Matrix3 I = b3Body_GetLocalRotationalInertia(body);
	return (I.cx.x + I.cy.y + I.cz.z) / 3.0f;	// rotation-invariant scalar inertia
}

static float Box3d_ReducePositive(float a, float b)	// reduced (harmonic) value; 0 == immovable (infinite)
{
	if(a>0.0f && b>0.0f)	return (a*b)/(a+b);
	if(a>0.0f)				return a;
	if(b>0.0f)				return b;
	return 1.0f;	// both immovable: harmless fallback
}

// Bounded motor cap: a torque for rotational joints, a force for prismatic.
static float Box3d_MotorCap(PintJoint type, b3BodyId a, b3BodyId b)
{
	if(type==PINT_JOINT_PRISMATIC)
		return BOX3D_MOTOR_LIN_ACCEL * Box3d_ReducePositive(b3Body_GetMass(a), b3Body_GetMass(b));
	return BOX3D_MOTOR_ANG_ACCEL * Box3d_ReducePositive(Box3d_MeanInertia(a), Box3d_MeanInertia(b));
}

// Inertia about the hinge axis LINE through the pivot: aT.I_com.a + m.d_perp^2 (parallel-axis theorem).
// b3Body_GetLocalRotationalInertia is taken about the COM, but a revolute joint pins the pivot, so the
// motor's effective inertia -- and thus the torque it needs to hold the body against gravity -- must be
// taken about the pivot. For a beam hinged at one end this is ~4x the COM inertia.
static float Box3d_HingeAxisInertia(b3BodyId body, const b3Vec3& pivotLocal, const b3Vec3& axisLocal)
{
	if(b3Body_GetType(body)!=b3_dynamicBody)
		return 0.0f;	// static/kinematic: immovable
	const float len2 = axisLocal.x*axisLocal.x + axisLocal.y*axisLocal.y + axisLocal.z*axisLocal.z;
	if(len2<1.0e-12f)
		return 0.0f;
	const float inv = 1.0f/sqrtf(len2);
	const b3Vec3 a = { axisLocal.x*inv, axisLocal.y*inv, axisLocal.z*inv };
	const b3Matrix3 I = b3Body_GetLocalRotationalInertia(body);	// columns cx,cy,cz
	const b3Vec3 Ia = { I.cx.x*a.x + I.cy.x*a.y + I.cz.x*a.z,
						I.cx.y*a.x + I.cy.y*a.y + I.cz.y*a.z,
						I.cx.z*a.x + I.cy.z*a.y + I.cz.z*a.z };
	const float comInertia = a.x*Ia.x + a.y*Ia.y + a.z*Ia.z;	// aT.I.a (inertia about a parallel axis through COM)
	const b3Vec3 com = b3Body_GetLocalCenterOfMass(body);
	const b3Vec3 r = { pivotLocal.x-com.x, pivotLocal.y-com.y, pivotLocal.z-com.z };
	const float rDotA = r.x*a.x + r.y*a.y + r.z*a.z;
	float dPerp2 = (r.x*r.x + r.y*r.y + r.z*r.z) - rDotA*rDotA;	// squared perpendicular distance COM->axis
	if(dPerp2<0.0f)
		dPerp2 = 0.0f;
	return comInertia + b3Body_GetMass(body)*dPerp2;
}

static float Box3d_RevoluteMotorCap(b3BodyId a, const b3Vec3& pivotA, const b3Vec3& axisA,
									b3BodyId b, const b3Vec3& pivotB, const b3Vec3& axisB)
{
	return BOX3D_MOTOR_ANG_ACCEL * Box3d_ReducePositive(Box3d_HingeAxisInertia(a, pivotA, axisA),
														Box3d_HingeAxisInertia(b, pivotB, axisB));
}

PintJointHandle Box3dPint::CreateJoint(const PINT_JOINT_CREATE& desc)
{
	if(!mHasWorld)
		return null;

	Box3dActor* A0 = reinterpret_cast<Box3dActor*>(desc.mObject0);
	Box3dActor* A1 = reinterpret_cast<Box3dActor*>(desc.mObject1);
	const b3BodyId BodyA = A0 ? A0->mBody : mWorldBody;	// null actor -> the static world anchor
	const b3BodyId BodyB = A1 ? A1->mBody : mWorldBody;

	// Generic motor bound (mean COM inertia for rotational joints, mass for prismatic). Revolute joints
	// refine this below to the inertia about the hinge axis THROUGH THE PIVOT (parallel-axis term): that's
	// the effective inertia a cantilever needs to hold itself against gravity, ~4x the COM inertia when
	// hinged at one end. This fixes both HingeJointMotorsZeroVelocity (holds) and HingeJointMotorVsObstacle
	// (still yields to the contact, since a box hinged near its COM keeps a small parallel-axis term).
	float MotorCap = Box3d_MotorCap(desc.mType, BodyA, BodyB);
	
	
	const b3Vec3 AxisZ = { 0.0f, 0.0f, 1.0f };	// revolute axis
	const b3Vec3 AxisX = { 1.0f, 0.0f, 0.0f };	// prismatic axis

	b3JointId Jid = b3JointId();

	switch(desc.mType)
	{
		case PINT_JOINT_SPHERICAL:
		{
			const PINT_SPHERICAL_JOINT_CREATE& jc = static_cast<const PINT_SPHERICAL_JOINT_CREATE&>(desc);
			b3SphericalJointDef d	= b3DefaultSphericalJointDef();
			d.base.bodyIdA			= BodyA;
			d.base.bodyIdB			= BodyB;
			d.base.collideConnected	= false;
			// Keep A's PEEL-authored cone frame; match B so the cone reference is identity at rest even when
			// the bodies are misoriented (same reasoning as the hinge fix).
			d.base.localFrameA		= Box3d_MakePRFrame(jc.mLocalPivot0);
			d.base.localFrameB		= Box3d_MatchFrameB(BodyA, d.base.localFrameA, BodyB, jc.mLocalPivot1.mPos);
			if(jc.mLimits.mMinValue>0.0f && jc.mLimits.mMaxValue>0.0f)	// PEEL: cone limit enabled when both>0
			{
				d.enableConeLimit	= true;
				d.coneAngle			= jc.mLimits.mMaxValue;
			}
			Jid = b3CreateSphericalJoint(mWorld, &d);
		}
		break;

		case PINT_JOINT_HINGE:
		{
			const PINT_HINGE_JOINT_CREATE& jc = static_cast<const PINT_HINGE_JOINT_CREATE&>(desc);
			b3RevoluteJointDef d	= b3DefaultRevoluteJointDef();
			d.base.bodyIdA			= BodyA;
			d.base.bodyIdB			= BodyB;
			d.base.collideConnected	= false;
			// Common-world-axis frames so misoriented bodies (ragdolls) get a correct limit reference.
			Box3d_MakeConsistentAxisFrames(BodyA, jc.mLocalPivot0, jc.mLocalAxis0, BodyB, jc.mLocalPivot1, AxisZ, d.base.localFrameA, d.base.localFrameB);
			MotorCap = Box3d_RevoluteMotorCap(BodyA, ToB3Vec3(jc.mLocalPivot0), ToB3Vec3(jc.mLocalAxis0),
											  BodyB, ToB3Vec3(jc.mLocalPivot1), ToB3Vec3(jc.mLocalAxis1));
			if(jc.mLimits.mMinValue<=jc.mLimits.mMaxValue)	// PEEL: hinge limit enabled when min<=max
			{
				d.enableLimit	= true;
				d.lowerAngle	= jc.mLimits.mMinValue;
				d.upperAngle	= jc.mLimits.mMaxValue;
			}
			if(jc.mUseMotor)
			{
				d.enableMotor		= true;
				d.motorSpeed		= jc.mDriveVelocity;
				d.maxMotorTorque	= MotorCap;
			}
			Jid = b3CreateRevoluteJoint(mWorld, &d);
		}
		break;

		case PINT_JOINT_HINGE2:
		{
			const PINT_HINGE2_JOINT_CREATE& jc = static_cast<const PINT_HINGE2_JOINT_CREATE&>(desc);
			b3RevoluteJointDef d	= b3DefaultRevoluteJointDef();
			d.base.bodyIdA			= BodyA;
			d.base.bodyIdB			= BodyB;
			d.base.collideConnected	= false;
			// The PR's X column is the hinge axis (matches the Jolt plugin); align it to frame Z. Build both
			// frames from the common world axis so misoriented bodies get a correct limit reference (see hinge).
			Box3d_MakeConsistentAxisFrames(BodyA, jc.mLocalPivot0.mPos, jc.mLocalPivot0.mRot.Rotate(Point(1.0f,0.0f,0.0f)),
										   BodyB, jc.mLocalPivot1.mPos, AxisZ, d.base.localFrameA, d.base.localFrameB);
			MotorCap = Box3d_RevoluteMotorCap(BodyA, ToB3Vec3(jc.mLocalPivot0.mPos), ToB3Vec3(jc.mLocalPivot0.mRot.Rotate(Point(1.0f,0.0f,0.0f))),
											  BodyB, ToB3Vec3(jc.mLocalPivot1.mPos), ToB3Vec3(jc.mLocalPivot1.mRot.Rotate(Point(1.0f,0.0f,0.0f))));
			if(jc.mLimits.mMinValue<=jc.mLimits.mMaxValue)
			{
				d.enableLimit	= true;
				d.lowerAngle	= jc.mLimits.mMinValue;
				d.upperAngle	= jc.mLimits.mMaxValue;
			}
			if(jc.mUseMotor)
			{
				d.enableMotor		= true;
				d.motorSpeed		= jc.mDriveVelocity;
				d.maxMotorTorque	= MotorCap;
			}
			Jid = b3CreateRevoluteJoint(mWorld, &d);
		}
		break;

		case PINT_JOINT_PRISMATIC:
		{
			const PINT_PRISMATIC_JOINT_CREATE& jc = static_cast<const PINT_PRISMATIC_JOINT_CREATE&>(desc);
			b3PrismaticJointDef d	= b3DefaultPrismaticJointDef();
			d.base.bodyIdA			= BodyA;
			d.base.bodyIdB			= BodyB;
			d.base.collideConnected	= false;
			if(jc.mLocalAxis0.IsNonZero())	// legacy axis-based API
			{
				d.base.localFrameA	= Box3d_MakeAxisFrame(jc.mLocalPivot0.mPos, jc.mLocalAxis0, AxisX);
				d.base.localFrameB	= Box3d_MakeAxisFrame(jc.mLocalPivot1.mPos, jc.mLocalAxis1, AxisX);
			}
			else							// frame-based API (X column = slider axis)
			{
				d.base.localFrameA	= Box3d_MakePRFrame(jc.mLocalPivot0);
				d.base.localFrameB	= Box3d_MakePRFrame(jc.mLocalPivot1);
			}
			if(jc.mLimits.mMinValue<=jc.mLimits.mMaxValue)
			{
				d.enableLimit		= true;
				d.lowerTranslation	= jc.mLimits.mMinValue;
				d.upperTranslation	= jc.mLimits.mMaxValue;
			}
			// TODO: prismatic spring (jc.mSpring) is intentionally NOT mapped -- left as hard limits
			// only, so the slider moves freely within [min,max].
			// PEEL/PhysX/Jolt use mSpring as a SOFT LIMIT (a spring resisting only at the [min,max]
			// stops: PxJointLinearLimitPair / Jolt mLimitsSpringSettings). Box3d has no soft limit --
			// its prismatic limits are hard and its only spring is a separate DRIVE toward a target
			// translation. Mapping mSpring to that drive (hertz = sqrt(k/m_eff)/2pi) drove stiff
			// suspension springs past Box3d's usable frequency (~0.25/substep-dt), so b3MakeSoft
			// clamped them to rigid and the joint locked up (behaved like a fixed joint). Revisit if
			// Box3d ever gains soft limits, or model the suspension via a b3_wheelJoint instead.
			Jid = b3CreatePrismaticJoint(mWorld, &d);
		}
		break;

		case PINT_JOINT_FIXED:
		{
			const PINT_FIXED_JOINT_CREATE& jc = static_cast<const PINT_FIXED_JOINT_CREATE&>(desc);
			b3WeldJointDef d		= b3DefaultWeldJointDef();
			d.base.bodyIdA			= BodyA;
			d.base.bodyIdB			= BodyB;
			d.base.collideConnected	= false;
			d.base.localFrameA.p	= ToB3Vec3(jc.mLocalPivot0);
			d.base.localFrameA.q	= Box3d_QuatIdentity();
			d.base.localFrameB.p	= ToB3Vec3(jc.mLocalPivot1);
			d.base.localFrameB.q	= Box3d_QuatIdentity();
			// linearHertz/angularHertz default to 0 => maximally rigid.
			Jid = b3CreateWeldJoint(mWorld, &d);
		}
		break;

		case PINT_JOINT_DISTANCE:
		{
			const PINT_DISTANCE_JOINT_CREATE& jc = static_cast<const PINT_DISTANCE_JOINT_CREATE&>(desc);
			b3DistanceJointDef d	= b3DefaultDistanceJointDef();
			d.base.bodyIdA			= BodyA;
			d.base.bodyIdB			= BodyB;
			d.base.collideConnected	= false;
			d.base.localFrameA.p	= ToB3Vec3(jc.mLocalPivot0);
			d.base.localFrameA.q	= Box3d_QuatIdentity();
			d.base.localFrameB.p	= ToB3Vec3(jc.mLocalPivot1);
			d.base.localFrameB.q	= Box3d_QuatIdentity();
			const float MinLen		= jc.mLimits.mMinValue<0.0f ? 0.0f	 : jc.mLimits.mMinValue;	// -1 => that side disabled
			const float MaxLen		= jc.mLimits.mMaxValue<0.0f ? 1.0e6f : jc.mLimits.mMaxValue;
			// Box3d's distance joint is RIGID at 'length' when enableSpring is false, IGNORING the
			// min/max range -- that clamps the bodies to 'length' (0 when the min side is disabled)
			// and pulls them together. For PhysX-style behavior (free within [min,max], hard stops at
			// the ends) enable the spring but with hertz=0 so it exerts no restoring pull: the solver
			// then enforces only the min/max limits. (If min==max, Box3d falls back to a rigid rod at
			// 'length', which is exactly what a fixed-distance joint wants.)
			d.enableSpring			= true;
			d.hertz					= 0.0f;
			d.dampingRatio			= 0.0f;
			d.enableLimit			= true;
			d.minLength				= MinLen;
			d.maxLength				= MaxLen;
			d.length				= MinLen;	// only used in the min==max rigid case
			Jid = b3CreateDistanceJoint(mWorld, &d);
		}
		break;

		default:
			// D6, GEAR, RACK_AND_PINION, CHAIN, PORTAL, UNIVERSAL: no clean Box3d equivalent yet.
			return null;
	}

	if(!b3Joint_IsValid(Jid))
		return null;

	Box3dJoint* Joint	= new Box3dJoint;
	Joint->mJoint		= Jid;
	Joint->mType		= desc.mType;
	Joint->mMotorCap	= MotorCap;
	mJoints.push_back(Joint);
	return reinterpret_cast<PintJointHandle>(Joint);
}

bool Box3dPint::ReleaseJoint(PintJointHandle handle)
{
	Box3dJoint* Joint = reinterpret_cast<Box3dJoint*>(handle);
	if(!Joint)
		return false;

	if(b3Joint_IsValid(Joint->mJoint))
		b3DestroyJoint(Joint->mJoint, true);

	for(size_t i=0;i<mJoints.size();i++)
		if(mJoints[i]==Joint)
		{
			mJoints[i] = mJoints.back();
			mJoints.pop_back();
			break;
		}

	delete Joint;
	return true;
}

b3Filter Box3dPint::MakeGroupFilter(PintCollisionGroup group) const
{
	b3Filter Filter;
	Filter.categoryBits	= uint64_t(1) << group;	// this shape's group bit
	Filter.maskBits		= mGroupMask[group];	// the groups it still collides with
	Filter.groupIndex	= 0;
	return Filter;
}

void Box3dPint::SetDisabledGroups(udword nb_groups, const PintDisabledGroups* groups)
{
	// Update the per-group mask table. Disabling pair (a,b) is symmetric.
	for(udword i=0;i<nb_groups;i++)
	{
		const PintCollisionGroup a = groups[i].mGroup0;
		const PintCollisionGroup b = groups[i].mGroup1;
		if(a>=32 || b>=32)	// guard PintDisabledGroups' 0xffff default sentinel
			continue;
		mGroupMask[a] &= ~(uint64_t(1) << b);
		mGroupMask[b] &= ~(uint64_t(1) << a);
	}

	// PEEL normally calls this before creating objects, but re-apply to any shapes that already
	// exist so we're correct regardless of call order.
	for(Box3dActor* Actor : mActors)
	{
		const b3Filter Filter = MakeGroupFilter(Actor->mGroup);
		b3ShapeId Shapes[256];
		const int NbShapes = b3Body_GetShapes(Actor->mBody, Shapes, 256);
		for(int s=0;s<NbShapes;s++)
			b3Shape_SetFilter(Shapes[s], Filter, true);	// invokeContacts so live contacts re-filter
	}
}

PintMeshHandle Box3dPint::CreateMeshObject(const PINT_MESH_DATA_CREATE& desc, PintMeshIndex* index)
{
	// The handle holds the raw geometry (for per-body cooking when sharing is off). When gShareMeshData
	// is set, we also cook the BVH once here and share it across every body that references this mesh.
	const PintSurfaceInterface& PSI = desc.GetSurface();
	Box3dMeshObject* MO = new Box3dMeshObject;
	MO->mSurface.Init(PSI.mNbFaces, PSI.mNbVerts, PSI.mVerts, (IndexedTriangle*)PSI.mDFaces);
	MO->mShared = null;
	if(gShareMeshData)
	{
		MO->mShared = Box3d_CreateMesh(MO->mSurface.GetSurfaceInterface());
		if(MO->mShared)
			mSharedMeshes.push_back(MO->mShared);	// plugin owns it; freed in Close() after b3DestroyWorld
	}
	if(index)
		*index = 0;
	return PintMeshHandle(MO);
}

bool Box3dPint::DeleteMeshObject(PintMeshHandle handle, const PintMeshIndex* index)
{
	// Only the handle wrapper is freed here. A shared cooked mesh (mShared) is owned by mSharedMeshes
	// and freed in Close() -- after the world/shapes that reference it -- so it never dangles.
	delete reinterpret_cast<Box3dMeshObject*>(handle);
	return true;
}

PintHeightfieldHandle Box3dPint::CreateHeightfieldObject(const PINT_HEIGHTFIELD_DATA_CREATE& desc, PintHeightfieldData& data, PintHeightfieldIndex* index)
{
	// Box3d bakes scale at cook time (and the U/V scale lives on the shape descriptor), so we only
	// stash the raw samples here and cook the b3HeightFieldData per-shape in CreateShape.
	Box3dHeightfieldData* HF = new Box3dHeightfieldData;
	HF->mNbU = desc.mNbU;
	HF->mNbV = desc.mNbV;

	const udword NbSamples = desc.mNbU * desc.mNbV;
	HF->mHeights.resize(NbSamples);

	float MinH =  MAX_FLOAT;
	float MaxH = -MAX_FLOAT;
	for(udword i=0;i<NbSamples;i++)
	{
		const float h	= desc.mHeights ? desc.mHeights[i] : desc.mUniqueValue;
		HF->mHeights[i]	= h;
		if(h<MinH)	MinH = h;
		if(h>MaxH)	MaxH = h;
	}
	if(!NbSamples)
		MinH = MaxH = 0.0f;

	HF->mMinHeight		= MinH;
	HF->mMaxHeight		= MaxH;

	data.mMinHeight		= MinH;
	data.mMaxHeight		= MaxH;
	data.mHeightScale	= 1.0f;	// Box3d handles quantization internally; we feed raw offset heights

	if(index)
		*index = 0;
	return PintHeightfieldHandle(HF);
}

bool Box3dPint::DeleteHeightfieldObject(PintHeightfieldHandle handle, const PintHeightfieldIndex* index)
{
	delete reinterpret_cast<Box3dHeightfieldData*>(handle);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	Scene queries
///////////////////////////////////////////////////////////////////////////////

static inline b3Vec3 ToB3Vec3Scaled(const Point& p, float s)
{
	b3Vec3 v; v.x = p.x*s; v.y = p.y*s; v.z = p.z*s; return v;
}

static void Box3d_FillRaycastHit(PintRaycastHit& dst, b3ShapeId shape, const b3Vec3& point, const b3Vec3& normal, float distance, int tri_index)
{
	dst.mImpact			= ToPoint(point);
	dst.mNormal			= ToPoint(normal);
	dst.mDistance		= distance;
	const b3BodyId Body	= b3Shape_GetBody(shape);
	dst.mTouchedActor	= reinterpret_cast<PintActorHandle>(b3Body_GetUserData(Body));
	dst.mTouchedShape	= null;
	dst.mTriangleIndex	= (tri_index>=0) ? udword(tri_index) : INVALID_ID;
}

namespace
{
	struct AnyHitCtx { bool mHit; };

	struct ClosestHitCtx
	{
		float		mBestFraction;
		b3ShapeId	mShape;
		b3Vec3		mPoint;
		b3Vec3		mNormal;
		int			mTriangleIndex;
		bool		mHit;
	};

	struct OverlapCollectCtx { std::vector<Box3dActor*>* mActors; };

	float Box3d_AnyRayCB(b3ShapeId, b3Pos, b3Vec3, float, uint64_t, int, int, void* ctx)
	{
		static_cast<AnyHitCtx*>(ctx)->mHit = true;
		return 0.0f;	// return 0 => terminate the cast at the first hit
	}

	float Box3d_ClosestCastCB(b3ShapeId shape, b3Pos point, b3Vec3 normal, float fraction, uint64_t, int tri_index, int, void* ctx)
	{
		ClosestHitCtx* c = static_cast<ClosestHitCtx*>(ctx);
		if(fraction < c->mBestFraction)
		{
			c->mBestFraction	= fraction;
			c->mShape			= shape;
			c->mPoint			= point;
			c->mNormal			= normal;
			c->mTriangleIndex	= tri_index;
			c->mHit				= true;
		}
		return fraction;	// clip the cast to this fraction so only closer hits are reported
	}

	bool Box3d_AnyOverlapCB(b3ShapeId, void* ctx)
	{
		static_cast<AnyHitCtx*>(ctx)->mHit = true;
		return false;	// return false => terminate at the first overlap
	}

	bool Box3d_CollectOverlapCB(b3ShapeId shape, void* ctx)
	{
		OverlapCollectCtx* c = static_cast<OverlapCollectCtx*>(ctx);
		const b3BodyId Body = b3Shape_GetBody(shape);
		c->mActors->push_back(static_cast<Box3dActor*>(b3Body_GetUserData(Body)));
		return true;	// keep collecting all overlaps
	}
}

static udword Box3d_SweepProxy(b3WorldId world, const b3ShapeProxy& proxy, const Point& dir, float max_dist, PintRaycastHit& dest)
{
	ClosestHitCtx Ctx;
	Ctx.mBestFraction		= 1.0f;
	Ctx.mHit				= false;
	const b3Vec3 Origin		= { 0.0f, 0.0f, 0.0f };
	const b3Vec3 Translation= ToB3Vec3Scaled(dir, max_dist);
	b3World_CastShape(world, Origin, &proxy, Translation, b3DefaultQueryFilter(), Box3d_ClosestCastCB, &Ctx);
	if(Ctx.mHit)
	{
		Box3d_FillRaycastHit(dest, Ctx.mShape, Ctx.mPoint, Ctx.mNormal, Ctx.mBestFraction*max_dist, Ctx.mTriangleIndex);
		return 1;
	}
	dest.SetNoHit();
	return 0;
}

udword Box3dPint::BatchRaycasts(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintRaycastData* raycasts)
{
	if(!mHasWorld)
		return 0;

	const b3QueryFilter Filter = b3DefaultQueryFilter();
	udword NbHits = 0;
	while(nb--)
	{
		const b3Vec3 Origin			= ToB3Vec3(raycasts->mOrigin);
		const b3Vec3 Translation	= ToB3Vec3Scaled(raycasts->mDir, raycasts->mMaxDist);

		const b3RayResult Result = b3World_CastRayClosest(mWorld, Origin, Translation, Filter);
		if(Result.hit)
		{
			NbHits++;
			Box3d_FillRaycastHit(*dest, Result.shapeId, Result.point, Result.normal, Result.fraction*raycasts->mMaxDist, Result.triangleIndex);
		}
		else
			dest->SetNoHit();

		raycasts++;
		dest++;
	}
	return NbHits;
}

udword Box3dPint::BatchRaycastAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintRaycastData* raycasts)
{
	if(!mHasWorld)
		return 0;

	const b3QueryFilter Filter = b3DefaultQueryFilter();
	udword NbHits = 0;
	while(nb--)
	{
		const b3Vec3 Origin			= ToB3Vec3(raycasts->mOrigin);
		const b3Vec3 Translation	= ToB3Vec3Scaled(raycasts->mDir, raycasts->mMaxDist);

		AnyHitCtx Ctx;
		Ctx.mHit = false;
		b3World_CastRay(mWorld, Origin, Translation, Filter, Box3d_AnyRayCB, &Ctx);
		if(Ctx.mHit)
		{
			NbHits++;
			dest->mHit = true;
		}
		else
			dest->SetNoHit();

		raycasts++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::BatchBoxSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintBoxSweepData* sweeps)
{
	if(!mHasWorld)
		return 0;

	udword NbHits = 0;
	while(nb--)
	{
		// A 3D box proxy is its 8 world-space corners (the header's "4 points" note is 2D-legacy).
		const OBB& Box	= sweeps->mBox;
		const Point Ax	= Box.mRot[0] * Box.mExtents.x;
		const Point Ay	= Box.mRot[1] * Box.mExtents.y;
		const Point Az	= Box.mRot[2] * Box.mExtents.z;
		b3Vec3 Corners[8];
		udword k = 0;
		for(int sx=-1;sx<=1;sx+=2)
			for(int sy=-1;sy<=1;sy+=2)
				for(int sz=-1;sz<=1;sz+=2)
					Corners[k++] = ToB3Vec3(Box.mCenter + Ax*float(sx) + Ay*float(sy) + Az*float(sz));

		b3ShapeProxy Proxy;
		Proxy.points	= Corners;
		Proxy.count		= 8;
		Proxy.radius	= 0.0f;

		NbHits += Box3d_SweepProxy(mWorld, Proxy, sweeps->mDir, sweeps->mMaxDist, *dest);
		sweeps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::BatchSphereSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintSphereSweepData* sweeps)
{
	if(!mHasWorld)
		return 0;

	udword NbHits = 0;
	while(nb--)
	{
		const b3Vec3 Center = ToB3Vec3(sweeps->mSphere.mCenter);
		b3ShapeProxy Proxy;
		Proxy.points	= &Center;
		Proxy.count		= 1;
		Proxy.radius	= sweeps->mSphere.mRadius;

		NbHits += Box3d_SweepProxy(mWorld, Proxy, sweeps->mDir, sweeps->mMaxDist, *dest);
		sweeps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::BatchCapsuleSweeps(PintSQThreadContext context, udword nb, PintRaycastHit* dest, const PintCapsuleSweepData* sweeps)
{
	if(!mHasWorld)
		return 0;

	udword NbHits = 0;
	while(nb--)
	{
		// A capsule proxy is its two endpoints + radius (orientation is implicit).
		const b3Vec3 Pts[2] = { ToB3Vec3(sweeps->mCapsule.mP0), ToB3Vec3(sweeps->mCapsule.mP1) };
		b3ShapeProxy Proxy;
		Proxy.points	= Pts;
		Proxy.count		= 2;
		Proxy.radius	= sweeps->mCapsule.mRadius;

		NbHits += Box3d_SweepProxy(mWorld, Proxy, sweeps->mDir, sweeps->mMaxDist, *dest);
		sweeps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::BatchSphereOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintSphereOverlapData* overlaps)
{
	if(!mHasWorld)
		return 0;

	const b3QueryFilter Filter	= b3DefaultQueryFilter();
	const b3Vec3 Origin			= { 0.0f, 0.0f, 0.0f };
	udword NbHits = 0;
	while(nb--)
	{
		const b3Vec3 Center = ToB3Vec3(overlaps->mSphere.mCenter);
		b3ShapeProxy Proxy;
		Proxy.points	= &Center;
		Proxy.count		= 1;
		Proxy.radius	= overlaps->mSphere.mRadius;

		AnyHitCtx Ctx;
		Ctx.mHit = false;
		b3World_OverlapShape(mWorld, Origin, &Proxy, Filter, Box3d_AnyOverlapCB, &Ctx);
		if(Ctx.mHit)
		{
			NbHits++;
			dest->mHit = true;
		}
		else
			dest->SetNoHit();

		overlaps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::BatchSphereOverlapObjects(PintSQThreadContext context, udword nb, PintMultipleHits* dest, Container& stream, const PintSphereOverlapData* overlaps)
{
	if(!mHasWorld)
		return 0;

	const b3QueryFilter Filter	= b3DefaultQueryFilter();
	const b3Vec3 Origin			= { 0.0f, 0.0f, 0.0f };
	udword NbHits = 0;
	udword Offset = 0;
	std::vector<Box3dActor*> Actors;
	while(nb--)
	{
		const b3Vec3 Center = ToB3Vec3(overlaps->mSphere.mCenter);
		b3ShapeProxy Proxy;
		Proxy.points	= &Center;
		Proxy.count		= 1;
		Proxy.radius	= overlaps->mSphere.mRadius;

		// Box3d reports overlaps one-at-a-time; collect them, then commit to the shared stream.
		Actors.clear();
		OverlapCollectCtx Ctx;
		Ctx.mActors = &Actors;
		b3World_OverlapShape(mWorld, Origin, &Proxy, Filter, Box3d_CollectOverlapCB, &Ctx);

		const udword Nb	= udword(Actors.size());
		NbHits			+= Nb;
		dest->mNbHits	= Nb;
		dest->mOffset	= Offset;
		Offset			+= Nb;
		if(Nb)
		{
			PintOverlapHit* buffer = reinterpret_cast<PintOverlapHit*>(stream.Reserve(Nb*(sizeof(PintOverlapHit)/sizeof(udword))));
			for(udword i=0;i<Nb;i++)
			{
				buffer[i].mTouchedActor	= reinterpret_cast<PintActorHandle>(Actors[i]);
				buffer[i].mTouchedShape	= null;
			}
		}

		overlaps++;
		dest++;
	}
	return NbHits;
}

///////////////////////////////////////////////////////////////////////////////

/*udword Box3dPint::BatchBoxOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintBoxOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::BatchBoxOverlapObjects(PintSQThreadContext context, udword nb, PintOverlapObjectHit* dest, const PintBoxOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::BatchCapsuleOverlapAny(PintSQThreadContext context, udword nb, PintBooleanHit* dest, const PintCapsuleOverlapData* overlaps)
{
	return 0;
}

udword Box3dPint::BatchCapsuleOverlapObjects(PintSQThreadContext context, udword nb, PintOverlapObjectHit* dest, const PintCapsuleOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::FindTriangles_MeshSphereOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintSphereOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::FindTriangles_MeshBoxOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintBoxOverlapData* overlaps)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

udword Box3dPint::FindTriangles_MeshCapsuleOverlap(PintSQThreadContext context, PintActorHandle handle, udword nb, const PintCapsuleOverlapData* overlaps)
{
	return 0;
}*/

///////////////////////////////////////////////////////////////////////////////

PR Box3dPint::GetWorldTransform(PintActorHandle handle)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	const b3WorldTransform Xform = b3Body_GetTransform(Actor->mBody);
	return PR(ToPoint(Xform.p), ToQuat(Xform.q));
}

void Box3dPint::SetWorldTransform(PintActorHandle handle, const PR& pose)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	b3Body_SetTransform(Actor->mBody, ToB3Vec3(pose.mPos), ToB3Quat(pose.mRot));
}

void Box3dPint::AddWorldImpulseAtWorldPos(PintActorHandle handle, const Point& world_impulse, const Point& world_pos)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	b3Body_ApplyLinearImpulse(Actor->mBody, ToB3Vec3(world_impulse), ToB3Vec3(world_pos), true);
}

Point Box3dPint::GetLinearVelocity(PintActorHandle handle)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	return ToPoint(b3Body_GetLinearVelocity(Actor->mBody));
}

void Box3dPint::SetLinearVelocity(PintActorHandle handle, const Point& linear_velocity)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	b3Body_SetLinearVelocity(Actor->mBody, ToB3Vec3(linear_velocity));
}

Point Box3dPint::GetAngularVelocity(PintActorHandle handle)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	return ToPoint(b3Body_GetAngularVelocity(Actor->mBody));
}

void Box3dPint::SetAngularVelocity(PintActorHandle handle, const Point& angular_velocity)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	b3Body_SetAngularVelocity(Actor->mBody, ToB3Vec3(angular_velocity));
}

bool Box3dPint::SetKinematicPose(PintActorHandle handle, const Point& pos)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	const b3Quat Rot = b3Body_GetRotation(Actor->mBody);
	b3Body_SetTransform(Actor->mBody, ToB3Vec3(pos), Rot);
	return true;
}

bool Box3dPint::SetKinematicPose(PintActorHandle handle, const PR& pr)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	// TODO: b3Body_SetTargetTransform() would give proper kinematic contact velocities (needs dt).
	b3Body_SetTransform(Actor->mBody, ToB3Vec3(pr.mPos), ToB3Quat(pr.mRot));
	return true;
}

bool Box3dPint::IsKinematic(PintActorHandle handle)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	return b3Body_GetType(Actor->mBody)==b3_kinematicBody;
}

bool Box3dPint::EnableKinematic(PintActorHandle handle, bool flag)
{
	const Box3dActor* Actor = reinterpret_cast<const Box3dActor*>(handle);
	b3Body_SetType(Actor->mBody, flag ? b3_kinematicBody : b3_dynamicBody);
	return true;
}

bool Box3dPint::SetDriveEnabled(PintJointHandle handle, bool flag)
{
	Box3dJoint* Joint = reinterpret_cast<Box3dJoint*>(handle);
	if(!Joint)
		return false;

	switch(Joint->mType)
	{
		case PINT_JOINT_HINGE:
		case PINT_JOINT_HINGE2:		b3RevoluteJoint_EnableMotor(Joint->mJoint, flag);	break;
		case PINT_JOINT_PRISMATIC:	b3PrismaticJoint_EnableMotor(Joint->mJoint, flag);	break;
		case PINT_JOINT_SPHERICAL:	b3SphericalJoint_EnableMotor(Joint->mJoint, flag);	break;
		default:					return false;
	}
	b3Joint_WakeBodies(Joint->mJoint);
	return true;
}

bool Box3dPint::SetDriveVelocity(PintJointHandle handle, const Point& linear, const Point& angular)
{
	Box3dJoint* Joint = reinterpret_cast<Box3dJoint*>(handle);
	if(!Joint)
		return false;

	switch(Joint->mType)
	{
		case PINT_JOINT_HINGE:
		case PINT_JOINT_HINGE2:
			b3RevoluteJoint_SetMotorSpeed(Joint->mJoint, angular.x);
			b3RevoluteJoint_SetMaxMotorTorque(Joint->mJoint, Joint->mMotorCap);
			break;
		case PINT_JOINT_PRISMATIC:
			b3PrismaticJoint_SetMotorSpeed(Joint->mJoint, linear.x);
			b3PrismaticJoint_SetMaxMotorForce(Joint->mJoint, Joint->mMotorCap);
			break;
		case PINT_JOINT_SPHERICAL:
			b3SphericalJoint_SetMotorVelocity(Joint->mJoint, ToB3Vec3(angular));
			b3SphericalJoint_SetMaxMotorTorque(Joint->mJoint, Joint->mMotorCap);
			break;
		default:
			return false;
	}
	b3Joint_WakeBodies(Joint->mJoint);
	return true;
}

bool Box3dPint::SetDrivePosition(PintJointHandle handle, const PR& pose)
{
	Box3dJoint* Joint = reinterpret_cast<Box3dJoint*>(handle);
	if(!Joint)
		return false;

	// Box3d position control is spring-based (target + hertz/damping), not a hard servo.
	switch(Joint->mType)
	{
		case PINT_JOINT_SPHERICAL:
			b3SphericalJoint_EnableSpring(Joint->mJoint, true);
			b3SphericalJoint_SetTargetRotation(Joint->mJoint, ToB3Quat(pose.mRot));
			break;
		case PINT_JOINT_PRISMATIC:
			b3PrismaticJoint_EnableSpring(Joint->mJoint, true);
			b3PrismaticJoint_SetTargetTranslation(Joint->mJoint, pose.mPos.x);
			break;
		default:
			return false;
	}
	b3Joint_WakeBodies(Joint->mJoint);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static Box3dPint* gBox3d = null;
static void gBox3d_GetOptionsFromGUI(const char*);

void Box3d_Init(const PINT_WORLD_CREATE& desc)
{
	gBox3d_GetOptionsFromGUI(desc.GetTestName());

	ASSERT(!gBox3d);
	gBox3d = ICE_NEW(Box3dPint);
	gBox3d->Init(desc);
}

void Box3d_Close()
{
	if(gBox3d)
	{
		gBox3d->Close();
		delete gBox3d;
		gBox3d = null;
	}
}

Box3dPint* GetBox3d()
{
	return gBox3d;
}

///////////////////////////////////////////////////////////////////////////////

static Widgets*	gBox3dGUI = null;

static IceCheckBox*	gCheckBox_RenderDebug = null;
static IceCheckBox*	gCheckBox_AllowSleeping = null;
static IceCheckBox*	gCheckBox_ShareMeshData = null;
static IceCheckBox*	gCheckBox_AllowShapeSharing = null;
static IceCheckBox*	gCheckBox_CCD = null;
static IceCheckBox*	gCheckBox_BackfaceCulling  = null;

static IceEditBox* gEditBox_NbThreads = null;
static IceEditBox* gEditBox_NbSubsteps = null;
static IceEditBox* gEditBox_TempAllocSize = null;
static IceEditBox* gEditBox_MaxBodies = null;
static IceEditBox* gEditBox_MaxBodyPairs = null;
static IceEditBox* gEditBox_MaxContactConstraints = null;
static IceEditBox* gEditBox_NbBodyMutexes = null;
static IceEditBox* gEditBox_NbPosIter = null;
static IceEditBox* gEditBox_NbVelIter = null;
static IceEditBox* gEditBox_LinearDamping = null;
static IceEditBox* gEditBox_AngularDamping = null;
static IceEditBox* gEditBox_SpeculativeContactDistance = null;
static IceEditBox* gEditBox_PenetrationSlop = null;
static IceEditBox* gEditBox_Baumgarte = null;
static IceEditBox* gEditBox_Friction = null;
static IceEditBox* gEditBox_Restitution = null;
static IceEditBox* gEditBox_ContactHertz = null;
static IceEditBox* gEditBox_ContactDampingRatio = null;

enum Box3dGUIElement
{
	BOX3D_GUI_MAIN,
	//
	BOX3D_GUI_RENDER_DEBUG,
	BOX3D_GUI_ALLOW_SLEEPING,
	BOX3D_GUI_SHARE_MESH_DATA,
	//BOX3D_GUI_ALLOW_SHAPE_SHARING,
	BOX3D_GUI_ENABLE_CCD,
	//BOX3D_GUI_BACKFACE_CULLING,
	//
};

static void gCheckBoxCallback(const IceCheckBox& check_box, bool checked, void* user_data)
{
	const udword id = check_box.GetID();
	switch(id)
	{
		case BOX3D_GUI_RENDER_DEBUG:
			gRenderDebug = checked;	// live toggle
			break;
		case BOX3D_GUI_ALLOW_SLEEPING:
			gAllowSleeping = checked;
			break;
		case BOX3D_GUI_SHARE_MESH_DATA:
			gShareMeshData = checked;
			break;
		/*case BOX3D_GUI_ALLOW_SHAPE_SHARING:
			break;*/
		case BOX3D_GUI_ENABLE_CCD:
			gEnableCCD = checked;
			break;
		/*case BOX3D_GUI_BACKFACE_CULLING:
			break;*/
	}
}

static void gBox3d_GetOptionsFromGUI(const char* test_name)
{
/*
	if(gCheckBox_AllowShapeSharing)
		gAllowShapeSharing = gCheckBox_AllowShapeSharing->IsChecked();
	if(gCheckBox_BackfaceCulling)
		gBackfaceCulling = gCheckBox_BackfaceCulling->IsChecked();

	Common_GetFromEditBox(gNbThreads, gEditBox_NbThreads);
	Common_GetFromEditBox(gNbSubsteps, gEditBox_NbSubsteps);
	Common_GetFromEditBox(gTempAllocSize, gEditBox_TempAllocSize);
	Common_GetFromEditBox(gMaxBodies, gEditBox_MaxBodies);
	Common_GetFromEditBox(gMaxBodyPairs, gEditBox_MaxBodyPairs);
	Common_GetFromEditBox(gMaxContactConstraints, gEditBox_MaxContactConstraints);
	Common_GetFromEditBox(gNbBodyMutexes, gEditBox_NbBodyMutexes);
	Common_GetFromEditBox(gNbPosIter, gEditBox_NbPosIter);
	Common_GetFromEditBox(gNbVelIter, gEditBox_NbVelIter);
	Common_GetFromEditBox(gLinearDamping, gEditBox_LinearDamping, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gAngularDamping, gEditBox_AngularDamping, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gSpeculativeContactDistance, gEditBox_SpeculativeContactDistance, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gPenetrationSlop, gEditBox_PenetrationSlop, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gBaumgarte, gEditBox_Baumgarte, 0.0f, 1.0f);
	Common_GetFromEditBox(gDefaultFriction, gEditBox_Friction, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gDefaultRestitution, gEditBox_Restitution, 0.0f, MAX_FLOAT);
*/
	if(gCheckBox_AllowSleeping)
		gAllowSleeping = gCheckBox_AllowSleeping->IsChecked();
	if(gCheckBox_ShareMeshData)
		gShareMeshData = gCheckBox_ShareMeshData->IsChecked();
	if(gCheckBox_CCD)
		gEnableCCD = gCheckBox_CCD->IsChecked();
	Common_GetFromEditBox(gNbThreads, gEditBox_NbThreads);
	Common_GetFromEditBox(gNbSubsteps, gEditBox_NbSubsteps);
	Common_GetFromEditBox(gLinearDamping, gEditBox_LinearDamping, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gAngularDamping, gEditBox_AngularDamping, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gDefaultFriction, gEditBox_Friction, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gDefaultRestitution, gEditBox_Restitution, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gContactHertz, gEditBox_ContactHertz, 0.0f, MAX_FLOAT);
	Common_GetFromEditBox(gContactDampingRatio, gEditBox_ContactDampingRatio, 0.0f, MAX_FLOAT);

	if(test_name)
		GetPintPlugin()->ApplyTestUIParams(test_name);
}

static IceEditBox* CreateEditBox(PintGUIHelper& helper, IceWindow* parent, sdword& y, const char* label, const char* value, EditBoxFilter filter)
{
	const sdword YStep = 20;
	const sdword LabelOffsetY = 2;
	const sdword EditBoxWidth = 60;
	const sdword LabelWidth = 160;
	const sdword EditBoxX2 = LabelWidth + 10;

	helper.CreateLabel(parent, 4, y+LabelOffsetY, LabelWidth, 20, label, gBox3dGUI);
	IceEditBox* EB = helper.CreateEditBox(parent, INVALID_ID, 4+EditBoxX2, y, EditBoxWidth, 20, value, gBox3dGUI, filter, null);
	y += YStep;
	return EB;
}

IceWindow* Box3d_InitGUI(IceWidget* parent, PintGUIHelper& helper)
{
	IceWindow* Main = helper.CreateMainWindow(gBox3dGUI, parent, BOX3D_GUI_MAIN, "Box3d options");

	const sdword YStep = 20;
	const sdword YStepCB = 16;
	sdword y = 4;

	const sdword OffsetX = 90;
	const sdword LabelOffsetY = 2;
	const sdword EditBoxWidth = 60;
	const udword CheckBoxWidth = 190;
	const sdword LabelWidth = 160;
	const sdword EditBoxX2 = LabelWidth + 10;

/*
	gCheckBox_AllowShapeSharing = helper.CreateCheckBox(Main, Box3d_GUI_ALLOW_SHAPE_SHARING, 4, y, CheckBoxWidth, 20, "Allow shape sharing", gBox3dGUI, gAllowShapeSharing, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_BackfaceCulling = helper.CreateCheckBox(Main, Box3d_GUI_BACKFACE_CULLING, 4, y, CheckBoxWidth, 20, "Backface culling (scene queries)", gBox3dGUI, gBackfaceCulling, gCheckBoxCallback);
	y += YStep;

	gEditBox_NbThreads					= CreateEditBox(helper, Main, y, "Nb threads (0==automatic):", _F("%d", gNbThreads), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbSubsteps					= CreateEditBox(helper, Main, y, "Nb substeps:", _F("%d", gNbSubsteps), EDITBOX_INTEGER_POSITIVE);
	gEditBox_TempAllocSize				= CreateEditBox(helper, Main, y, "Tmp alloc size (Mb):", _F("%d", gTempAllocSize), EDITBOX_INTEGER_POSITIVE);
	gEditBox_MaxBodies					= CreateEditBox(helper, Main, y, "Max bodies:", _F("%d", gMaxBodies), EDITBOX_INTEGER_POSITIVE);
	gEditBox_MaxBodyPairs				= CreateEditBox(helper, Main, y, "Max body pairs:", _F("%d", gMaxBodyPairs), EDITBOX_INTEGER_POSITIVE);
	gEditBox_MaxContactConstraints		= CreateEditBox(helper, Main, y, "Max contact constraints:", _F("%d", gMaxContactConstraints), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbBodyMutexes				= CreateEditBox(helper, Main, y, "Nb body mutexes:", _F("%d", gNbBodyMutexes), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbPosIter					= CreateEditBox(helper, Main, y, "Nb pos iter:", _F("%d", gNbPosIter), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbVelIter					= CreateEditBox(helper, Main, y, "Nb vel iter:", _F("%d", gNbVelIter), EDITBOX_INTEGER_POSITIVE);
	gEditBox_LinearDamping				= CreateEditBox(helper, Main, y, "Linear damping:", helper.Convert(gLinearDamping), EDITBOX_FLOAT_POSITIVE);
	gEditBox_AngularDamping				= CreateEditBox(helper, Main, y, "Angular damping:", helper.Convert(gAngularDamping), EDITBOX_FLOAT_POSITIVE);
	gEditBox_SpeculativeContactDistance	= CreateEditBox(helper, Main, y, "Speculative contact distance:", helper.Convert(gSpeculativeContactDistance), EDITBOX_FLOAT_POSITIVE);
	gEditBox_PenetrationSlop			= CreateEditBox(helper, Main, y, "Max penetration slop:", helper.Convert(gPenetrationSlop), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Baumgarte					= CreateEditBox(helper, Main, y, "Baumgarte:", helper.Convert(gBaumgarte), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Friction					= CreateEditBox(helper, Main, y, "Default friction:", helper.Convert(gDefaultFriction), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Restitution				= CreateEditBox(helper, Main, y, "Default restitution:", helper.Convert(gDefaultRestitution), EDITBOX_FLOAT_POSITIVE);

	auto make_snapshot = [](IceButton& button, void* user_data) { 
		PhysicsScene Scene;
		Scene.FromPhysicsSystem(gPhysicsSystem);
		ofstream Stream;
		Stream.open("snapshot.bin", ofstream::out | ofstream::binary | ofstream::trunc);
		if (Stream.is_open()) 
		{
			StreamOutWrapper Wrapper(Stream);
			Scene.SaveBinaryState(Wrapper, true, false);
		}
	};
	helper.CreateButton(Main, 0, 4, y, 130, 20, "Save snapshot.bin", gBox3dGUI, make_snapshot, nullptr);
	y += YStep;*/

	gCheckBox_RenderDebug = helper.CreateCheckBox(Main, BOX3D_GUI_RENDER_DEBUG, 4, y, CheckBoxWidth, 20, "Render debug data", gBox3dGUI, gRenderDebug, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_AllowSleeping = helper.CreateCheckBox(Main, BOX3D_GUI_ALLOW_SLEEPING, 4, y, CheckBoxWidth, 20, "Allow sleeping", gBox3dGUI, gAllowSleeping, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_ShareMeshData = helper.CreateCheckBox(Main, BOX3D_GUI_SHARE_MESH_DATA, 4, y, CheckBoxWidth, 20, "Share mesh data", gBox3dGUI, gShareMeshData, gCheckBoxCallback);
	y += YStepCB;

	gCheckBox_CCD = helper.CreateCheckBox(Main, BOX3D_GUI_ENABLE_CCD, 4, y, CheckBoxWidth, 20, "Enable CCD", gBox3dGUI, gEnableCCD, gCheckBoxCallback);
	y += YStepCB;

	gEditBox_NbThreads = CreateEditBox(helper, Main, y, "Nb threads (0==automatic):", _F("%d", gNbThreads), EDITBOX_INTEGER_POSITIVE);
	gEditBox_NbSubsteps = CreateEditBox(helper, Main, y, "Nb substeps:", _F("%d", gNbSubsteps), EDITBOX_INTEGER_POSITIVE);
	gEditBox_LinearDamping = CreateEditBox(helper, Main, y, "Linear damping:", helper.Convert(gLinearDamping), EDITBOX_FLOAT_POSITIVE);
	gEditBox_AngularDamping = CreateEditBox(helper, Main, y, "Angular damping:", helper.Convert(gAngularDamping), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Friction = CreateEditBox(helper, Main, y, "Default friction:", helper.Convert(gDefaultFriction), EDITBOX_FLOAT_POSITIVE);
	gEditBox_Restitution = CreateEditBox(helper, Main, y, "Default restitution:", helper.Convert(gDefaultRestitution), EDITBOX_FLOAT_POSITIVE);
	gEditBox_ContactHertz = CreateEditBox(helper, Main, y, "Contact hertz:", helper.Convert(gContactHertz), EDITBOX_FLOAT_POSITIVE);
	gEditBox_ContactDampingRatio = CreateEditBox(helper, Main, y, "Contact damping ratio:", helper.Convert(gContactDampingRatio), EDITBOX_FLOAT_POSITIVE);

	return Main;
}

void Box3d_CloseGUI()
{
	Common_CloseGUI(gBox3dGUI);
	gCheckBox_RenderDebug = null;
	gCheckBox_AllowSleeping = null;
	gCheckBox_ShareMeshData = null;
	gCheckBox_AllowShapeSharing = null;
	gCheckBox_CCD = null;
	gCheckBox_BackfaceCulling = null;
	gEditBox_NbThreads = null;
	gEditBox_NbSubsteps = null;
	gEditBox_TempAllocSize = null;
	gEditBox_MaxBodies = null;
	gEditBox_MaxBodyPairs = null;
	gEditBox_MaxContactConstraints = null;
	gEditBox_NbBodyMutexes = null;
	gEditBox_NbPosIter = null;
	gEditBox_NbVelIter = null;
	gEditBox_LinearDamping = null;
	gEditBox_AngularDamping = null;
	gEditBox_ContactHertz = null;
	gEditBox_ContactDampingRatio = null;
	gEditBox_SpeculativeContactDistance = null;
	gEditBox_PenetrationSlop = null;
	gEditBox_Baumgarte = null;
	gEditBox_Friction = null;
	gEditBox_Restitution = null;
}

///////////////////////////////////////////////////////////////////////////////

class Box3dPlugIn : public PintPlugin
{
	public:
	virtual	IceWindow*	InitGUI(IceWidget* parent, PintGUIHelper& helper)	override	{ return Box3d_InitGUI(parent, helper);	}
	virtual	void		CloseGUI()											override	{ Box3d_CloseGUI();						}

	virtual	void		Init(const PINT_WORLD_CREATE& desc)					override	{ Box3d_Init(desc);						}
	virtual	void		Close()												override	{ Box3d_Close();						}

	virtual	Pint*		GetPint()											override	{ return GetBox3d();					}

	virtual	IceWindow*	InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)	override;
	virtual	void		CloseTestGUI()																					override;
	virtual	const char*	GetTestGUIName()																				override	{ return "Box3d";	}
	virtual	void		ApplyTestUIParams(const char* test_name)														override;
};
static Box3dPlugIn gPlugIn;

PintPlugin*	GetPintPlugin()
{
	return &gPlugIn;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateTabWindow(IceWidget* parent, Widgets& owner)
{
	WindowDesc WD;
	WD.mParent	= parent;
	WD.mX		= 0;
	WD.mY		= 0;
	WD.mType	= WINDOW_DIALOG;
	IceWindow* TabWindow = ICE_NEW(IceWindow)(WD);
	owner.Register(TabWindow);
	return TabWindow;
}

static const sdword YStep = 20;
static const udword CheckBoxWidth = 200;

static IceCheckBox*	gCheckBox_Override = null;
static IceCheckBox*	gCheckBox_Generic0 = null;

static const char* gDesc_CCD =
"By design the CCD tests fail without CCD enabled.\n\
Check this box to make them pass using the\n\
default Box3d CCD algorithm.";

static IceWindow* CreateUI_CCD(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, const char* test_name)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void CCD(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gCheckBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::CCD, null);
	y += YStep;

	bool EnableCCD = true;

	const sdword x2 = x + 10;
	const sdword CheckBoxWidth2 = 220;
	{
		sdword YSaved = y;
		y += 50;

		ASSERT(!gCheckBox_Generic0);
		gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, x2, y, CheckBoxWidth2, 20, "Enable Box3d CCD", &owner, EnableCCD, null, null);
		y += YStep;

		IceEditBox* EB = helper.CreateEditBox(TabWindow, 0, x, YSaved, 260, 80, "", &owner, EDITBOX_TEXT, null);
		EB->SetReadOnly(true);
		EB->SetMultilineText(gDesc_CCD);
		y += 20;
	}

	return TabWindow;
}

static bool IsCCDTest(const char* test_name)
{
	return strncmp(test_name, "CCDTest_", 8)==0;
}

///////////////////////////////////////////////////////////////////////////////

static IceWindow* CreateUI_Dzhanibekov(IceWidget* parent, PintGUIHelper& helper, Widgets& owner, bool enabled)
{
	IceWindow* TabWindow = CreateTabWindow(parent, owner);

	sdword y = 4;
	sdword x = 4;

	struct Override{ static void Gyro(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gCheckBox_Generic0->SetEnabled(checked);
	}};
	ASSERT(!gCheckBox_Override);
	gCheckBox_Override = helper.CreateCheckBox(TabWindow, 0, x, y, 200, 20, "Override main panel settings", &owner, true, Override::Gyro, null);
	y += YStep;

	ASSERT(!gCheckBox_Generic0);
	gCheckBox_Generic0 = helper.CreateCheckBox(TabWindow, 0, 4, y, CheckBoxWidth, 20, "Enable gyroscopic forces", &owner, enabled, null, null);
	y += YStep;
	// Gyro forces are always applied in Box3D so disable the checkbox, but keep the UI so that we can disable angular damping below
	gCheckBox_Generic0->SetEnabled(false);

	return TabWindow;
}

///////////////////////////////////////////////////////////////////////////////

IceWindow* Box3dPlugIn::InitTestGUI(const char* test_name, IceWidget* parent, PintGUIHelper& helper, Widgets& owner)
{
	if(IsCCDTest(test_name))
		return CreateUI_CCD(parent, helper, owner, test_name);

	if(	strcmp(test_name, "Dzhanibekov")==0)
		return CreateUI_Dzhanibekov(parent, helper, owner, true);

	return null;
}

void Box3dPlugIn::CloseTestGUI()
{
	gCheckBox_Override = null;
	gCheckBox_Generic0 = null;
}

void Box3dPlugIn::ApplyTestUIParams(const char* test_name)
{
	const bool ApplySettings = gCheckBox_Override ? gCheckBox_Override->IsChecked() : false;
	if(!ApplySettings)
		return;

	// TODO: we should eventually expose and define all of these directly in the PINT structures

	if(IsCCDTest(test_name))
	{
		gEnableCCD = gCheckBox_Generic0->IsChecked();
		return;
	}

	if(strcmp(test_name, "Dzhanibekov")==0)
	{
		gAngularDamping = 0.0f;
		return;
	}
}
