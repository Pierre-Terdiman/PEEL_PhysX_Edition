///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_COMMON_PHYSX3_MESH_H
#define PINT_COMMON_PHYSX3_MESH_H

	class PintShapeRenderer;
	struct PintSurfaceInterface;

	class MeshManager
	{
		public:
								MeshManager();
		virtual					~MeshManager();

				PxConvexMesh*	CreateConvexMesh(const Point* verts, udword nb_verts, PxConvexFlags flags, PintShapeRenderer* renderer, bool share_meshes);
				PxTriangleMesh*	CreateTriangleMesh(const PintSurfaceInterface& surface, PintShapeRenderer* renderer, bool deformable, bool share_meshes, bool dynamic);
				void			ReleaseMeshes();

		private:
		virtual	PxConvexMesh*	CreatePhysXConvex(udword nb_verts, const Point* verts, PxConvexFlags flags)			= 0;
		virtual	PxTriangleMesh*	CreatePhysXMesh(const PintSurfaceInterface& surface, bool deformable, bool dynamic)	= 0;

			struct ConvexRender
			{
				ConvexRender(PxConvexMesh* convexMesh, PintShapeRenderer* renderer) :
					mConvexMesh	(convexMesh),
					mRenderer	(renderer)
				{
				}
				PxConvexMesh*			mConvexMesh;
				PintShapeRenderer*		mRenderer;
			};
			std::vector<ConvexRender>	mConvexes;

			struct MeshRender
			{
				MeshRender(PxTriangleMesh* triMesh, PintShapeRenderer* renderer) :
					mTriangleMesh	(triMesh),
					mRenderer		(renderer)
				{
				}
				PxTriangleMesh*			mTriangleMesh;
				PintShapeRenderer*		mRenderer;
			};
			std::vector<MeshRender>		mMeshes;
	};

	class MeshObjectManager
	{
		public:
								MeshObjectManager() : mFirstFree(INVALID_ID)	{}
								~MeshObjectManager()							{}

				udword			AddObject(void* object);
				bool			DeleteObject(void* object, const udword* index);

				PtrContainer	mObjects;
				udword			mFirstFree;
	};

#endif