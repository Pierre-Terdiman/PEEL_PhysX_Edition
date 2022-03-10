///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX3 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX3_Shape.h"
#include "PINT_CommonPhysX3.h"

PhysX_ShapeAPI::PhysX_ShapeAPI(Pint& pint) : Pint_Shape(pint)
{
}

PhysX_ShapeAPI::~PhysX_ShapeAPI()
{
}

const char* PhysX_ShapeAPI::GetName(PintShapeHandle handle) const
{
	const PxShape* Shape = reinterpret_cast<const PxShape*>(handle);
	ASSERT(Shape);
	return Shape->getName();
}

bool PhysX_ShapeAPI::SetName(PintShapeHandle handle, const char* name)
{
	SharedPhysX& physx = static_cast<SharedPhysX&>(mPint);
	PxShape* Shape = reinterpret_cast<PxShape*>(handle);
	physx.SetShapeName(Shape, name);
	return true;
}

PintShape PhysX_ShapeAPI::GetType(PintShapeHandle handle) const
{
	PxShape* Shape = reinterpret_cast<PxShape*>(handle);
	ASSERT(Shape);

	const PxGeometryType::Enum type = Shape->getGeometryType();
	switch(type)
	{
		case PxGeometryType::eSPHERE:		return PINT_SHAPE_SPHERE;
		case PxGeometryType::eCAPSULE:		return PINT_SHAPE_CAPSULE;
		case PxGeometryType::eBOX:			return PINT_SHAPE_BOX;
		case PxGeometryType::eCONVEXMESH:	return PINT_SHAPE_CONVEX;
		case PxGeometryType::eTRIANGLEMESH:	return PINT_SHAPE_MESH;

//		case PxGeometryType::ePLANE:
//		case PxGeometryType::ePARTICLESYSTEM:
//		case PxGeometryType::eTETRAHEDRONMESH:
//		case PxGeometryType::eHEIGHTFIELD:
	}
	return PINT_SHAPE_UNDEFINED;
}

PR PhysX_ShapeAPI::GetWorldTransform(PintActorHandle actor, PintShapeHandle shape) const
{
	PxShape* Shape = reinterpret_cast<PxShape*>(shape);
	ASSERT(Shape);

#if PHYSX_SUPPORT_SHARED_SHAPES
	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(actor);
	ASSERT(Actor || Shape->getActor());
	const PxTransform Pose = PxShapeExt::getGlobalPose(*Shape, Actor ? *Actor : *Shape->getActor());
#else
	const PxTransform Pose = PxShapeExt::getGlobalPose(*Shape);
#endif

	return ToPR(Pose);
}

bool PhysX_ShapeAPI::GetWorldBounds(PintActorHandle actor, PintShapeHandle shape, AABB& bounds) const
{
	const PxRigidActor* Actor = reinterpret_cast<const PxRigidActor*>(actor);
	ASSERT(Actor);

	PxShape* Shape = reinterpret_cast<PxShape*>(shape);
	ASSERT(Shape);

#ifdef IS_PHYSX_3_2
	const PxBounds3 pxBounds = Shape->getWorldBounds();
#else
	const PxBounds3 pxBounds = PxShapeExt::getWorldBounds(*Shape, *Actor);
#endif
	bounds.mMin = ToPoint(pxBounds.minimum);
	bounds.mMax = ToPoint(pxBounds.maximum);
	return true;
}

bool PhysX_ShapeAPI::GetTriangleMeshData(SurfaceInterface& surface, PintShapeHandle handle, bool b) const
{
	PxShape* Shape = reinterpret_cast<PxShape*>(handle);
	ASSERT(Shape);

	PxTriangleMeshGeometry meshGeom;
	if(!Shape->getTriangleMeshGeometry(meshGeom))
		return false;

	PxTriangleMesh* mesh = meshGeom.triangleMesh;
	if(!mesh)
		return false;

	surface.mNbVerts	= mesh->getNbVertices();
	surface.mNbFaces	= mesh->getNbTriangles();
	// TODO: there's no difference between these two calls really, unify
#if PHYSX_SUPPORT_DEFORMABLE_MESHES
	if(b)
		surface.mVerts		= reinterpret_cast<const Point*>(mesh->getVerticesForModification());
	else
#endif
		surface.mVerts		= reinterpret_cast<const Point*>(mesh->getVertices());
	const void* indices = mesh->getTriangles();
	if(mesh->getTriangleMeshFlags() & PHYSX_SUPPORT_MESH_16BIT_INDICES)
		surface.mWFaces	= reinterpret_cast<const uword*>(indices);
	else
		surface.mDFaces	= reinterpret_cast<const udword*>(indices);

	return true;
}

static PxTriangleMesh* GetTriangleIndices(IndexedTriangle& tri, PintShapeHandle handle, udword index)
{
	PxShape* Shape = reinterpret_cast<PxShape*>(handle);
	ASSERT(Shape);

	PxTriangleMeshGeometry meshGeom;
	if(!Shape->getTriangleMeshGeometry(meshGeom))
		return null;

	PxTriangleMesh* mesh = meshGeom.triangleMesh;
	if(!mesh)
		return null;

	const PxU32 nbTris = mesh->getNbTriangles();
	if(index>=nbTris)
		return null;

	const PxTriangleMeshFlags flags = mesh->getTriangleMeshFlags();
	if(flags & PHYSX_SUPPORT_MESH_16BIT_INDICES)
	{
		const PxU16* indices = reinterpret_cast<const PxU16*>(mesh->getTriangles());
		tri.mRef[0] = indices[index*3+0];
		tri.mRef[1] = indices[index*3+1];
		tri.mRef[2] = indices[index*3+2];
	}
	else
	{
		const PxU32* indices = reinterpret_cast<const PxU32*>(mesh->getTriangles());
		tri.mRef[0] = indices[index*3+0];
		tri.mRef[1] = indices[index*3+1];
		tri.mRef[2] = indices[index*3+2];
	}
	return mesh;
}

bool PhysX_ShapeAPI::GetTriangle(Triangle& tri, PintShapeHandle handle, udword index)	const
{
	IndexedTriangle VRefs;
	PxTriangleMesh* mesh = GetTriangleIndices(VRefs, handle, index);
	if(!mesh)
		return false;
	const udword vref0 = VRefs.mRef[0];
	const udword vref1 = VRefs.mRef[1];
	const udword vref2 = VRefs.mRef[2];

	const PxU32 nbVerts = mesh->getNbVertices();
	PX_ASSERT(vref0<nbVerts);
	PX_ASSERT(vref1<nbVerts);
	PX_ASSERT(vref2<nbVerts);

	const PxVec3* verts = mesh->getVertices();
	tri.mVerts[0] = ToPoint(verts[vref0]);
	tri.mVerts[1] = ToPoint(verts[vref1]);
	tri.mVerts[2] = ToPoint(verts[vref2]);
	return true;
}

bool PhysX_ShapeAPI::GetIndexedTriangle(IndexedTriangle& tri, PintShapeHandle handle, udword index)	const
{
	return GetTriangleIndices(tri, handle, index)!=0;
}

// TODO: refactor with findOverlapTriangleMesh
bool PhysX_ShapeAPI::FindTouchedTriangles(Container& indices, PintSQThreadContext context, PintShapeHandle handle, const PR& pose, const PintSphereOverlapData& overlap) const
{
	PxShape* Shape = reinterpret_cast<PxShape*>(handle);
	ASSERT(Shape);

	PxTriangleMeshGeometry meshGeom;
	if(!Shape->getTriangleMeshGeometry(meshGeom))
		return false;

	PxTriangleMesh* mesh = meshGeom.triangleMesh;
	if(!mesh)
		return false;

	const PxTransform meshPose = ToPxTransform(pose);

	udword* Results = indices.Reserve(8192);

	const PxU32 startIndex = 0;
	const PxTransform Pose(ToPxVec3(overlap.mSphere.mCenter), PxIdtQuat);

	bool Overflow = false;
	const PxU32 Nb = PxMeshQuery::findOverlapTriangleMesh(PxSphereGeometry(overlap.mSphere.mRadius), Pose, meshGeom, meshPose, Results, 8192, startIndex, Overflow);
	ASSERT(!Overflow);
	indices.ForceSize(Nb);
	return true;
}

static const bool gDebugPX2197 = false;

bool PhysX_ShapeAPI::Refit(PintShapeHandle shape, PintActorHandle actor)
{
#if PHYSX_SUPPORT_DEFORMABLE_MESHES
	PxShape* Shape = reinterpret_cast<PxShape*>(shape);
	ASSERT(Shape);

	PxTriangleMeshGeometry meshGeom;
	if(!Shape->getTriangleMeshGeometry(meshGeom))
		return false;

	PxTriangleMesh* mesh = meshGeom.triangleMesh;
	if(!mesh)
		return false;

	if(!gDebugPX2197)	//MEGABUG
		mesh->refitBVH();

	PxRigidActor* Actor = reinterpret_cast<PxRigidActor*>(actor);

	if(1)
	{
		if(!gDebugPX2197)
		{
		// Hack to update the SQ structures
	Shape->acquireReference();
		Actor->detachShape(*Shape);
		Shape->setGeometry(meshGeom);
		Actor->attachShape(*Shape);
	Shape->release();
		}

		// Reset filtering to tell the broadphase about the new mesh bounds.
		Actor->getScene()->resetFiltering(*Actor);
	}
	return true;
#else
	return false;
#endif
}

PintShapeRenderer* PhysX_ShapeAPI::GetShapeRenderer(PintShapeHandle handle) const
{
	PxShape* Shape = reinterpret_cast<PxShape*>(handle);
	ASSERT(Shape);

	// This one is "temporary" and/or questionable. We just need a quick way to map
	// a shape to its renderers, and it's unclear whose responsability it is to do that.
	// Right now we store the renderers in the shapes' user-data but it's arbitrary and
	// a plugin-specific decision, so it seems natural to ask the plugins about the
	// shape-to-renderer mapping, but it's unclear / clumsy / need more work.
	return reinterpret_cast<PintShapeRenderer*>(Shape->userData);
}
