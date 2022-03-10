///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_SHAPE_H
#define PINT_COMMON_PHYSX3_SHAPE_H

#include "..\Pint.h"

//#define SHARED_SHAPES_USE_HASH

#ifdef REMOVED


#ifdef SHARED_SHAPES_USE_HASH
	// Jeeez this stuff is as tedious to use as ever
	struct ShapePtr
	{
		inline_	ShapePtr(PxShape* shape=null) : mShape(shape)	{}

		PxShape*	mShape;
	};
#endif

//#ifdef REMOVED
	struct InternalShape
	{
		InternalShape(PxShape* shape, const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mShape			(shape),
			mMaterial		(material),
			mRenderer		(renderer),
			mLocalPose		(local_pose),
			mCollisionGroup	(collision_group)
		{
		}
		PxShape*			mShape;
		const PxMaterial*	mMaterial;
		PintShapeRenderer*	mRenderer;
		PxTransform			mLocalPose;
		PxU16				mCollisionGroup;

		inline_ bool Compare(const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group)	const
		{
/*			if(mMaterial!=&material)
				return false;
			if(mRenderer!=renderer)
				return false;
			if(mLocalPose.p!=local_pose.p)
				return false;
			if(!(mLocalPose.q==local_pose.q))
				return false;
			if(mCollisionGroup!=collision_group)
				return false;
			return true;*/

			// TODO: shapes' names are ignored here. Good? Bad?
			// Also this will break if we change the local pose after shape creation.
			if(		mMaterial==material
				&&	mRenderer==renderer
				&&	mLocalPose.p.x==local_pose.p.x
				&&	mLocalPose.p.y==local_pose.p.y
				&&	mLocalPose.p.z==local_pose.p.z
				&&	mLocalPose.q.x==local_pose.q.x
				&&	mLocalPose.q.y==local_pose.q.y
				&&	mLocalPose.q.z==local_pose.q.z
				&&	mLocalPose.q.w==local_pose.q.w
				&&	mCollisionGroup==collision_group
				)
				return true;
			return false;
		}
	};
//#endif

#ifdef SHARED_SHAPES_USE_HASH
	struct InternalSharedShape
	{
		InternalSharedShape(const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mMaterial		(material),
			mRenderer		(renderer),
			mLocalPose		(local_pose),
			mCollisionGroup	(collision_group)
		{
		}
		const PxMaterial*	mMaterial;
		PintShapeRenderer*	mRenderer;
		PxTransform			mLocalPose;
		PxU16				mCollisionGroup;

		inline_ bool Compare(const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group)	const
		{
/*			if(mMaterial!=&material)
				return false;
			if(mRenderer!=renderer)
				return false;
			if(mLocalPose.p!=local_pose.p)
				return false;
			if(!(mLocalPose.q==local_pose.q))
				return false;
			if(mCollisionGroup!=collision_group)
				return false;
			return true;*/

			// TODO: shapes' names are ignored here. Good? Bad?
			// Also this will break if we change the local pose after shape creation.
			if(		mMaterial==material
				&&	mRenderer==renderer
				&&	mLocalPose.p.x==local_pose.p.x
				&&	mLocalPose.p.y==local_pose.p.y
				&&	mLocalPose.p.z==local_pose.p.z
				&&	mLocalPose.q.x==local_pose.q.x
				&&	mLocalPose.q.y==local_pose.q.y
				&&	mLocalPose.q.z==local_pose.q.z
				&&	mLocalPose.q.w==local_pose.q.w
				&&	mCollisionGroup==collision_group
				)
				return true;
			return false;
		}
	};
#endif

#ifdef SHARED_SHAPES_USE_HASH
	struct InternalSphereShape : InternalSharedShape
#else
	struct InternalSphereShape : InternalShape
#endif
	{
#ifdef SHARED_SHAPES_USE_HASH
		InternalSphereShape(float radius, const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mRadius				(radius),
			InternalSharedShape	(material, renderer, local_pose, collision_group)
#else
		InternalSphereShape(float radius, PxShape* shape, const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mRadius			(radius),
			InternalShape	(shape, material, renderer, local_pose, collision_group)
#endif
		{
		}
		float	mRadius;

#ifdef SHARED_SHAPES_USE_HASH
		PX_FORCE_INLINE	bool operator == (const InternalSphereShape& other) const
		{
			return (other.mRadius == mRadius) && other.Compare(mMaterial, mRenderer, mLocalPose, mCollisionGroup);
		}
#endif
	};

	struct InternalBoxShape : InternalShape
	{
		InternalBoxShape(const PxVec3& extents, PxShape* shape, const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mExtents		(extents),
			InternalShape	(shape, material, renderer, local_pose, collision_group)
		{
		}
		PxVec3	mExtents;
	};

	struct InternalCapsuleShape : InternalShape
	{
		InternalCapsuleShape(float radius, float half_height, PxShape* shape, const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mRadius			(radius),
			mHalfHeight		(half_height),
			InternalShape	(shape, material, renderer, local_pose, collision_group)
		{
		}
		float	mRadius;
		float	mHalfHeight;
	};

	struct InternalConvexShape : InternalShape
	{
		InternalConvexShape(PxConvexMesh* convex_mesh, PxShape* shape, const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mConvexMesh		(convex_mesh),
			InternalShape	(shape, material, renderer, local_pose, collision_group)
		{
		}
		PxConvexMesh*	mConvexMesh;
	};

	struct InternalMeshShape : InternalShape
	{
		InternalMeshShape(PxTriangleMesh* triangle_mesh, PxShape* shape, const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mTriangleMesh	(triangle_mesh),
			InternalShape	(shape, material, renderer, local_pose, collision_group)
		{
		}
		PxTriangleMesh*	mTriangleMesh;
	};

#if PHYSX_SUPPORT_HEIGHTFIELDS
	struct InternalHeightfieldShape : InternalShape
	{
		InternalHeightfieldShape(PxHeightField* heightfield, PxShape* shape, const PxMaterial* material, PintShapeRenderer* renderer, const PxTransform& local_pose, PxU16 collision_group) :
			mHeightfield	(heightfield),
			InternalShape	(shape, material, renderer, local_pose, collision_group)
		{
		}
		PxHeightField*	mHeightfield;
	};
#endif

#ifdef SHARED_SHAPES_USE_HASH
	static uint32_t PxComputeHash(const InternalSphereShape& shape)
	{
		const uint32_t key = IR(shape.mRadius);

		uint32_t k = key;
		k += ~(k << 15);
		k ^= (k >> 10);
		k += (k << 3);
		k ^= (k >> 6);
		k += ~(k << 11);
		k ^= (k >> 16);
		return uint32_t(k);
	}
#endif

#endif	// REMOVED

	class PhysX_ShapeAPI : public Pint_Shape
	{
		public:
									PhysX_ShapeAPI(Pint& pint);
		virtual						~PhysX_ShapeAPI();

		virtual	const char*			GetName(PintShapeHandle handle)				const	override;
		virtual	bool				SetName(PintShapeHandle handle, const char* name)	override;

		virtual	PintShape			GetType(PintShapeHandle handle)				const	override;

		virtual	PR					GetWorldTransform(PintActorHandle actor, PintShapeHandle shape)	const	override;

		virtual	bool				GetWorldBounds(PintActorHandle actor, PintShapeHandle shape, AABB& bounds)	const	override;

		virtual	bool				GetTriangleMeshData(SurfaceInterface& surface, PintShapeHandle handle, bool)	const	override;
		virtual	bool				GetTriangle(Triangle& tri, PintShapeHandle handle, udword index)				const	override;
		virtual	bool				GetIndexedTriangle(IndexedTriangle& tri, PintShapeHandle handle, udword index)	const	override;
		virtual	bool				FindTouchedTriangles(Container& indices, PintSQThreadContext context, PintShapeHandle handle, const PR& pose, const PintSphereOverlapData& sphere)	const	override;
		virtual	bool				Refit(PintShapeHandle shape, PintActorHandle actor);

		virtual	PintShapeRenderer*	GetShapeRenderer(PintShapeHandle handle)	const	override;
	};

#endif