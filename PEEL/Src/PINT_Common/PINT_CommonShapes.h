///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_SHAPES_H
#define PINT_COMMON_SHAPES_H

#include "..\Pint.h"

	class PintShapeRenderer;

	struct InternalShapeData
	{
		InternalShapeData(void* engine_shape, const void* engine_material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) :
			mShape			(engine_shape),
			mMaterial		(engine_material),
			mRenderer		(renderer),
			mLocalPose		(local_pose),
			mCollisionGroup	(collision_group)
		{
		}
		void*				mShape;
		const void*			mMaterial;
		PintShapeRenderer*	mRenderer;
		PR					mLocalPose;
		udword				mCollisionGroup;

		inline_ bool Compare(const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)	const
		{
			// TODO: shapes' names are ignored here. Good? Bad?
			// Also this will break if we change the local pose after shape creation.
			if(		mMaterial==material
				&&	mRenderer==renderer
				&&	mLocalPose.mPos==local_pose.mPos
				&&	mLocalPose.mRot==local_pose.mRot
				&&	mCollisionGroup==collision_group
				)
				return true;
			return false;
		}
	};

	struct InternalSphereShapeData : InternalShapeData
	{
		InternalSphereShapeData(float radius, void* shape, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) :
			mRadius				(radius),
			InternalShapeData	(shape, material, renderer, local_pose, collision_group)
		{
		}
		float	mRadius;
	};

	struct InternalBoxShapeData : InternalShapeData
	{
		InternalBoxShapeData(const Point& extents, void* shape, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) :
			mExtents			(extents),
			InternalShapeData	(shape, material, renderer, local_pose, collision_group)
		{
		}
		Point	mExtents;
	};

	struct InternalCapsuleShapeData : InternalShapeData
	{
		InternalCapsuleShapeData(float radius, float half_height, void* shape, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) :
			mRadius				(radius),
			mHalfHeight			(half_height),
			InternalShapeData	(shape, material, renderer, local_pose, collision_group)
		{
		}
		float	mRadius;
		float	mHalfHeight;
	};

	// "Mesh" = convex / triangle / heightfield / etc
	struct InternalMeshShapeData : InternalShapeData
	{
		InternalMeshShapeData(const void* mesh, void* shape, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group) :
			mMesh				(mesh),
			InternalShapeData	(shape, material, renderer, local_pose, collision_group)
		{
		}
		const void*	mMesh;
	};

	template<class T>
	class SharedShapes
	{
		public:

		inline_	udword		GetNbShapes()	const	{ return mShapes.GetNbEntries()/(sizeof(T)/sizeof(udword));	}
		inline_	const T*	GetShapes()		const	{ return reinterpret_cast<const T*>(mShapes.GetEntries());	}

		inline_	udword		RegisterShapeData(const T& data)
							{
								const udword Index = GetNbShapes();
								T* Data = ICE_RESERVE(T, mShapes);
								*Data = data;
								return Index;
							}

				Container	mShapes;
	};

	class SharedSphereShapes : public SharedShapes<InternalSphereShapeData>
	{
		public:
							SharedSphereShapes()	{}
							~SharedSphereShapes()	{}

		inline_	udword		RegisterShape(float radius, void* shape, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)
							{
								return RegisterShapeData(InternalSphereShapeData(radius, shape, material, renderer, local_pose, collision_group));
							}

				void*		FindShape(float radius, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)	const;
	};

	class SharedBoxShapes : public SharedShapes<InternalBoxShapeData>
	{
		public:
							SharedBoxShapes()	{}
							~SharedBoxShapes()	{}

		inline_	udword		RegisterShape(const Point& extents, void* shape, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)
							{
								return RegisterShapeData(InternalBoxShapeData(extents, shape, material, renderer, local_pose, collision_group));
							}

				void*		FindShape(const Point& extents, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)	const;
	};

	class SharedCapsuleShapes : public SharedShapes<InternalCapsuleShapeData>
	{
		public:
							SharedCapsuleShapes()	{}
							~SharedCapsuleShapes()	{}

		inline_	udword		RegisterShape(float radius, float half_height, void* shape, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)
							{
								return RegisterShapeData(InternalCapsuleShapeData(radius, half_height, shape, material, renderer, local_pose, collision_group));
							}

				void*		FindShape(float radius, float half_height, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)	const;
	};

	class SharedMeshShapes : public SharedShapes<InternalMeshShapeData>
	{
		public:
							SharedMeshShapes()	{}
							~SharedMeshShapes()	{}

		inline_	udword		RegisterShape(const void* mesh, void* shape, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)
							{
								return RegisterShapeData(InternalMeshShapeData(mesh, shape, material, renderer, local_pose, collision_group));
							}

				void*		FindShape(const void* mesh, const void* material, PintShapeRenderer* renderer, const PR& local_pose, udword collision_group)	const;
	};

#endif