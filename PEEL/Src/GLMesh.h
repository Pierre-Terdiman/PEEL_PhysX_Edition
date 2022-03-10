///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GL_MESH_H
#define GL_MESH_H

	class GLIndexBuffer : public Allocateable
	{
		public:
					GLIndexBuffer();
					~GLIndexBuffer();

		void		Create(udword nb_tris, const udword* indices32, const uword* indices16);
		void		Release();

		void		Select()	const;
		void		Unselect()	const;

		udword		mNbTris;
		GLuint		mIndicesIBO;
		BOOL		mHas16BitIndices;
	};

	class GLVertexBuffer : public Allocateable
	{
		public:
					GLVertexBuffer();
					~GLVertexBuffer();

		void		Create(udword nb_verts, const Point* verts, const Point* vnormals);
		void		Update(udword nb_verts, const Point* verts, const Point* vnormals);
		void		Release();

		void		Select()	const;
		void		Unselect()	const;

		udword		mNbVerts;
		GLuint		mPositionsVBO;
		GLuint		mNormalsVBO;
	};

	class GLIndexBufferCollection : public Allocateable
	{
		public:
										GLIndexBufferCollection();
										~GLIndexBufferCollection();

				void					AddIndexBuffer(const GLIndexBuffer& index_buffer);

		inline_	udword					GetNbIndexBuffers()			const
										{
											return mIndexBuffers.GetNbEntries();
										}
		inline_	const GLIndexBuffer*	GetIndexBuffer(udword i)	const
										{
											if(i<mIndexBuffers.GetNbEntries())
												return reinterpret_cast<const GLIndexBuffer*>(mIndexBuffers[i]);
											return null;
										}
		private:
				PtrContainer			mIndexBuffers;
	};

	class GLMesh;
	class GLMeshEx;

	enum GLMeshType
	{
		GL_MESH_DISPLAY_LIST,
		GL_MESH_VBO,
	};

	namespace OpenGLMesh
	{
		GLMesh*	Create(udword nb_verts, const Point* verts, const Point* vnormals, udword nb_tris, const udword* indices32, const uword* indices16, GLMeshType type);
		void	UpdateVerts(GLMesh* mesh, const Point* verts, const Point* vertex_normals);
		void	Release(GLMesh* mesh);
		void	Draw(GLMesh* mesh);

//#ifdef TOSEE
		GLMeshEx*	Create(const MultiSurface& multi_surface, udword flags, bool b);
		void		UpdateVerts(GLMeshEx* mesh, const Point* verts);
		void		Release(GLMeshEx* mesh);
		void		Draw(GLMeshEx* mesh);
//#endif
	};

#endif
