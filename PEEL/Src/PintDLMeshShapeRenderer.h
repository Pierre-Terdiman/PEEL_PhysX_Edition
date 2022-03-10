///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_DL_MESH_SHAPE_RENDERER_H
#define PINT_DL_MESH_SHAPE_RENDERER_H

#include "PintDLShapeRenderer.h"
#include "PintSurfaceInterface.h"

	enum DLMeshFlags
	{
		DL_MESH_USE_ACTIVE_EDGES	= (1<<0),
		DL_MESH_USE_VERTEX_NORMALS	= (1<<1),
		DL_MESH_USE_SMOOTH_NORMALS	= (1<<2),
		DL_MESH_USE_DIRECT_DATA		= (1<<3),
		DL_MESH_DEFORMABLE			= (1<<4),
	};

	class ActiveEdgesRenderer
	{
		public:
									ActiveEdgesRenderer();
									~ActiveEdgesRenderer();

				void				CreateDL(const SurfaceInterface& surface);
				void				Release();

				GLuint				mActiveEdgesDL;
	};

	class PintDLMeshShapeRenderer : public PintDLShapeRenderer
	{
		friend class PintDLMeshShapeRenderer2;
		friend PintDLMeshShapeRenderer* CreatePintDLMeshShapeRenderer(const PintSurfaceInterface& surface, udword flags);
		friend PintDLMeshShapeRenderer* CreatePintDLMeshShapeRenderer(const MultiSurface& multi_surface, udword flags);
									PintDLMeshShapeRenderer(const PintSurfaceInterface& surface, udword flags);
									PintDLMeshShapeRenderer(const MultiSurface& multi_surface, udword flags);
		virtual						~PintDLMeshShapeRenderer();

		public:

		// PintShapeRenderer
		virtual	PtrContainer*		GetOwnerContainer()			const	override;
		virtual	const char*			GetClassName()				const	override	{ return "PintDLMeshShapeRenderer";	}
		virtual	void				_Render(const PR& pose)		const	override;
		//~PintShapeRenderer

		struct Data
		{
			inline_	Data(const PintSurfaceInterface& surface, udword flags) :
				mFlags		(flags),
				mNbVerts	(surface.mNbVerts),
				mNbFaces	(surface.mNbFaces),
				mCRC32_Verts(surface.mCRC32_Verts),
				mCRC32_Faces(surface.mCRC32_Faces)
			{}

			const udword	mFlags;
			const udword	mNbVerts;
			const udword	mNbFaces;
			const udword	mCRC32_Verts;
			const udword	mCRC32_Faces;
		};

		inline_ bool Equal(const Data& p)	const
		{
			// We use CRCs to avoid keeping an extra copy of verts & tris data here (and to avoid memcmp). The risk of collisions is probably acceptable?
			return mData.mFlags==p.mFlags
				&& mData.mNbVerts==p.mNbVerts
				&& mData.mNbFaces==p.mNbFaces
				&& mData.mCRC32_Verts==p.mCRC32_Verts
				&& mData.mCRC32_Faces==p.mCRC32_Faces;
		};

				const Data		mData;	// We only store this to implement sharing

		protected:
				void				CreateDisplayList(const SurfaceInterface& surface, udword flags, const IndexedSurface* UVSurface=null);
				void				CreateDisplayList(const MultiSurface& multi_surface, udword flags);

				ActiveEdgesRenderer	mActiveEdges;
	};

	PintDLMeshShapeRenderer* CreatePintDLMeshShapeRenderer(const PintSurfaceInterface& surface, udword flags);
	PintDLMeshShapeRenderer* CreatePintDLMeshShapeRenderer(const MultiSurface& multi_surface, udword flags);

	class PintDLMeshShapeRenderer2 : public PintDLMeshShapeRenderer
	{
		public:
									PintDLMeshShapeRenderer2(const PintSurfaceInterface& surface, udword flags);
									PintDLMeshShapeRenderer2(const MultiSurface& multi_surface, udword flags);
		virtual						~PintDLMeshShapeRenderer2();

		// PintShapeRenderer
		virtual	const char*			GetClassName()			const	override	{ return "PintDLMeshShapeRenderer2";	}
		virtual	void				_Render(const PR& pose)	const	override;
		//~PintShapeRenderer

				//### test!
				void				Init(const TriSurface& ts);
	};

#endif