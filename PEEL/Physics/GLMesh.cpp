///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLMesh.h"
#include "GLShader.h"
#include "Camera.h"

///////////////////////////////////////////////////////////////////////////////

static inline_ GLuint CreateArrayBuffer(udword nb_verts, const Point* verts)
{
	GLuint id = 0;
	if(verts)
	{
		glGenBuffers(1, &id);
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, nb_verts*sizeof(Point), verts, GL_STATIC_DRAW);
	}
	return id;
}

static inline_ void UpdateVertexBuffer(udword nb_verts, const Point* verts, const Point* vertex_normals, GLuint pos_id, GLuint nrm_id)
{
	if(verts)
	{
		glBindBuffer(GL_ARRAY_BUFFER, pos_id);
		glBufferData(GL_ARRAY_BUFFER, nb_verts*sizeof(Point), verts, GL_STATIC_DRAW);
	}
	if(vertex_normals)
	{
		glBindBuffer(GL_ARRAY_BUFFER, nrm_id);
		glBufferData(GL_ARRAY_BUFFER, nb_verts*sizeof(Point), vertex_normals, GL_STATIC_DRAW);
	}

	if(verts || vertex_normals)
		glBindBuffer(GL_ARRAY_BUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////////

GLIndexBuffer::GLIndexBuffer() : mNbTris(0), mIndicesIBO(0), mHas16BitIndices(FALSE)
{
}

GLIndexBuffer::~GLIndexBuffer()
{
	Release();
}

void GLIndexBuffer::Release()
{
	if(mIndicesIBO)
	{
		glDeleteBuffers(1, &mIndicesIBO);
		mIndicesIBO = 0;
	}
}

void GLIndexBuffer::Create(udword nb_tris, const udword* indices32, const uword* indices16)
{
	mNbTris = nb_tris;
	glGenBuffers(1, &mIndicesIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesIBO);
	if(indices32)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nb_tris*3*sizeof(udword), indices32, GL_STATIC_DRAW);
		mHas16BitIndices = FALSE;
	}
	else if(indices16)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nb_tris*3*sizeof(uword), indices16, GL_STATIC_DRAW);
		mHas16BitIndices = TRUE;
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLIndexBuffer::Select() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesIBO);
}

void GLIndexBuffer::Unselect() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	
}

///////////////////////////////////////////////////////////////////////////////

GLVertexBuffer::GLVertexBuffer() : mNbVerts(0), mPositionsVBO(0), mNormalsVBO(0)
{
}

GLVertexBuffer::~GLVertexBuffer()
{
	Release();
}

void GLVertexBuffer::Release()
{
	if(mNormalsVBO)
	{
		glDeleteBuffers(1, &mNormalsVBO);
		mNormalsVBO = 0;
	}

	if(mPositionsVBO)
	{
		glDeleteBuffers(1, &mPositionsVBO);
		mPositionsVBO = 0;
	}
}

void GLVertexBuffer::Create(udword nb_verts, const Point* verts, const Point* vnormals)
{
	mNbVerts = nb_verts;
	mPositionsVBO = CreateArrayBuffer(nb_verts, verts);

	mNormalsVBO = CreateArrayBuffer(nb_verts, vnormals);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLVertexBuffer::Update(udword nb_verts, const Point* verts, const Point* vnormals)
{
	ASSERT(nb_verts==mNbVerts);
	UpdateVertexBuffer(nb_verts, verts, vnormals, mPositionsVBO, mNormalsVBO);
}

void GLVertexBuffer::Select() const
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, mPositionsVBO);
	glVertexPointer(3, GL_FLOAT, sizeof(Point), 0);

	if(mNormalsVBO)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, mNormalsVBO);
		glNormalPointer(GL_FLOAT, sizeof(Point), 0);
	}
}

void GLVertexBuffer::Unselect() const
{
	glDisableClientState(GL_VERTEX_ARRAY);
	if(mNormalsVBO)
		glDisableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////////

GLIndexBufferCollection::GLIndexBufferCollection()
{
}

GLIndexBufferCollection::~GLIndexBufferCollection()
{
}

void GLIndexBufferCollection::AddIndexBuffer(const GLIndexBuffer& index_buffer)
{
	mIndexBuffers.AddPtr(&index_buffer);
}

///////////////////////////////////////////////////////////////////////////////

class GLMesh : public Allocateable
{
	public:
	GLMeshType	mType;
};

class GLMeshDL : public GLMesh
{
	public:
		inline_	GLMeshDL()	{ mType = GL_MESH_DISPLAY_LIST;	}

	GLuint		mDisplayListNum;
};

class GLMeshVBO : public GLMesh
{
	public:
		inline_	GLMeshVBO()	{ mType = GL_MESH_VBO;	}

	udword		mNbVerts;
	udword		mNbTris;
	GLuint		mPositionsVBO;
	GLuint		mNormalsVBO;
	GLuint		mIndicesIBO;
	BOOL		mHas16BitIndices;
};

static GLMesh* CreateMesh_DL(udword nb_verts, const Point* verts, const Point* vnormals, udword nb_tris, const udword* indices32, const uword* indices16)
{
	GLMeshDL* M = ICE_NEW(GLMeshDL);

	M->mDisplayListNum = glGenLists(1);

	glNewList(M->mDisplayListNum, GL_COMPILE);
		glBegin(GL_TRIANGLES);
		while(nb_tris--)
		{
			udword VRef0, VRef1, VRef2;
			if(indices32)
			{
				VRef0 = *indices32++;
				VRef1 = *indices32++;
				VRef2 = *indices32++;
			}
			else if(indices16)
			{
				VRef0 = *indices16++;
				VRef1 = *indices16++;
				VRef2 = *indices16++;
			}

			const Point& p0 = verts[VRef0];
			const Point& p1 = verts[VRef1];
			const Point& p2 = verts[VRef2];

			if(vnormals)
			{
				const Point& n0 = vnormals[VRef0];
				const Point& n1 = vnormals[VRef1];
				const Point& n2 = vnormals[VRef2];

				glNormal3f(n0.x, n0.y, n0.z);
				glVertex3f(p0.x, p0.y, p0.z);

				glNormal3f(n1.x, n1.y, n1.z);
				glVertex3f(p1.x, p1.y, p1.z);

				glNormal3f(n2.x, n2.y, n2.z);
				glVertex3f(p2.x, p2.y, p2.z);
			}
			else
			{
				glVertex3f(p0.x, p0.y, p0.z);
				glVertex3f(p1.x, p1.y, p1.z);
				glVertex3f(p2.x, p2.y, p2.z);
			}
		}

/*					for(udword i=0;i<nb_tris;i++)
					{
						const udword VRef0 = indices[i*3+0];
						const udword VRef1 = indices[i*3+1];
						const udword VRef2 = indices[i*3+2];
						const Point& p0 = verts[VRef0];
						const Point& p1 = verts[VRef1];
						const Point& p2 = verts[VRef2];
	//					const Point Normal = ((p2-p0)^(p1-p0)).Normalize();
						const Point Normal = ((p1-p0)^(p2-p0)).Normalize();
						glNormal3f(Normal.x, Normal.y, Normal.z);
						glVertex3f(p0.x, p0.y, p0.z);
						glVertex3f(p1.x, p1.y, p1.z);
						glVertex3f(p2.x, p2.y, p2.z);
					}*/


		glEnd();
	glEndList();

	return M;
}

static GLMesh* CreateMesh_VBO(udword nb_verts, const Point* verts, const Point* vnormals, udword nb_tris, const udword* indices32, const uword* indices16)
{
	GLMeshVBO* M = ICE_NEW(GLMeshVBO);

	M->mNbVerts	= nb_verts;
	M->mNbTris	= nb_tris;

	M->mPositionsVBO = CreateArrayBuffer(nb_verts, verts);

	M->mNormalsVBO = CreateArrayBuffer(nb_verts, vnormals);

	glGenBuffers(1, &M->mIndicesIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, M->mIndicesIBO);
	if(indices32)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nb_tris*3*sizeof(udword), indices32, GL_STATIC_DRAW);
		M->mHas16BitIndices = FALSE;
	}
	else if(indices16)
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nb_tris*3*sizeof(uword), indices16, GL_STATIC_DRAW);
		M->mHas16BitIndices = TRUE;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return M;
}

GLMesh* OpenGLMesh::Create(udword nb_verts, const Point* verts, const Point* vnormals, udword nb_tris, const udword* indices32, const uword* indices16, GLMeshType type)
{
	if(type==GL_MESH_DISPLAY_LIST)
		return CreateMesh_DL(nb_verts, verts, vnormals, nb_tris, indices32, indices16);
	else if(type==GL_MESH_VBO)
		return CreateMesh_VBO(nb_verts, verts, vnormals, nb_tris, indices32, indices16);
	else
		return null;
}

void OpenGLMesh::Release(GLMesh* m)
{
	if(!m)
		return;

	if(m->mType==GL_MESH_DISPLAY_LIST)
	{
		GLMeshDL* mesh = static_cast<GLMeshDL*>(m);
		glDeleteLists(mesh->mDisplayListNum, 1);
		DELETESINGLE(mesh);
	}
	else if(m->mType==GL_MESH_VBO)
	{
		GLMeshVBO* mesh = static_cast<GLMeshVBO*>(m);
		glDeleteBuffers(1, &mesh->mPositionsVBO);
		glDeleteBuffers(1, &mesh->mNormalsVBO);
		glDeleteBuffers(1, &mesh->mIndicesIBO);
		DELETESINGLE(mesh);
	}
}

void OpenGLMesh::Draw(GLMesh* m/*, const Matrix44& xform, const Vec3& color*/)
{
	if(!m)
		return;

	if(m->mType==GL_MESH_DISPLAY_LIST)
	{
		GLMeshDL* mesh = static_cast<GLMeshDL*>(m);
		glCallList(mesh->mDisplayListNum);
	}
	else if(m->mType==GL_MESH_VBO)
	{
		GLMeshVBO* mesh = static_cast<GLMeshVBO*>(m);
/*		GLint program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &program);

		if (program)
			glUniformMatrix4fv( glGetUniformLocation(program, "objectTransform"), 1, false, xform);*/

//		glColor3fv(color);
//		glSecondaryColor3fv(color);

		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->mPositionsVBO);
		glVertexPointer(3, GL_FLOAT, sizeof(Point), 0);

		if(mesh->mNormalsVBO)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->mNormalsVBO);
			glNormalPointer(GL_FLOAT, sizeof(Point), 0);
		}
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mIndicesIBO);
		if(mesh->mHas16BitIndices)
			glDrawElements(GL_TRIANGLES, mesh->mNbTris*3, GL_UNSIGNED_SHORT, 0);
		else
			glDrawElements(GL_TRIANGLES, mesh->mNbTris*3, GL_UNSIGNED_INT, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		if(mesh->mNormalsVBO)
			glDisableClientState(GL_NORMAL_ARRAY);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);	

//		if (program)
//			glUniformMatrix4fv(glGetUniformLocation(program, "objectTransform"), 1, false, Matrix44::kIdentity);
	}
}

void OpenGLMesh::UpdateVerts(GLMesh* m, const Point* verts, const Point* vertex_normals)
{
	if(!m)
		return;

	if(m->mType==GL_MESH_DISPLAY_LIST)
	{
		ASSERT(0);
	}
	else if(m->mType==GL_MESH_VBO)
	{
		GLMeshVBO* mesh = static_cast<GLMeshVBO*>(m);
		UpdateVertexBuffer(mesh->mNbVerts, verts, vertex_normals, mesh->mPositionsVBO, mesh->mNormalsVBO);
	}
}








//#ifdef TOSEE

class GLMeshEx : public Allocateable
{
	public:

	udword		mNbVerts;
	udword		mNbTris;
	GLuint		mPositionsVBO;
	GLuint		mTexCoordsVBO;
	GLuint		mNormalsVBO;
	GLuint		mIndicesIBO;
};

// TODO: use flags
GLMeshEx* OpenGLMesh::Create(const MultiSurface& multi_surface, udword /*flags*/, bool b)
{
	const SurfaceInterface surface = multi_surface.GetSurfaceInterface();
	const IndexedSurface* UVSurface = multi_surface.GetExtraSurface(SURFACE_UVS);

	if(UVSurface)
	{
		ASSERT(surface.mNbFaces == UVSurface->GetNbFaces());
	}

	if(b)	// For textured deformable terrain, don't remap anything
	{
		if(UVSurface)
		{
			ASSERT(surface.mNbVerts == UVSurface->GetNbVerts());
		}
		const udword NbVerts = surface.mNbVerts;
		const udword NbFaces = surface.mNbFaces;

		GLMeshEx* M = ICE_NEW(GLMeshEx);

		M->mNbVerts	= NbVerts;
		M->mNbTris	= NbFaces;

		M->mPositionsVBO = CreateArrayBuffer(NbVerts, surface.mVerts);

		if(UVSurface)
		{
			glGenBuffers(1, &M->mTexCoordsVBO);
			glBindBuffer(GL_ARRAY_BUFFER, M->mTexCoordsVBO);
			float* UVs = new float[NbVerts*2];
			for(udword i=0;i<NbVerts;i++)
			{
				const Point& uv = UVSurface->GetVerts()[i];
				UVs[i*2+0] = uv.x;
				UVs[i*2+1] = uv.y;
			}
			glBufferData(GL_ARRAY_BUFFER, NbVerts*sizeof(float)*2, UVs, GL_STATIC_DRAW);
			DELETEARRAY(UVs);
		}
		else
			M->mTexCoordsVBO = 0;

		/*if(Normals)
		{
			glGenBuffers(1, &M->mNormalsVBO);
			glBindBuffer(GL_ARRAY_BUFFER, M->mNormalsVBO);
			glBufferData(GL_ARRAY_BUFFER, NbVerts*sizeof(Point), _VNormals.GetEntries(), GL_STATIC_DRAW);
		}
		else*/
			M->mNormalsVBO = 0;

		if(0)
		{
			NORMALSCREATE NCreate;
			NCreate.NbVerts		= surface.mNbVerts;
			NCreate.Verts		= surface.mVerts;
			NCreate.NbFaces		= surface.mNbFaces;
			NCreate.dFaces		= surface.mDFaces;
			NCreate.wFaces		= surface.mWFaces;
			NCreate.UseAngles	= true;

			SmoothNormals SN;
			SN.Compute(NCreate);
			const Point* Normals = SN.GetVertexNormals();

			glGenBuffers(1, &M->mNormalsVBO);
			glBindBuffer(GL_ARRAY_BUFFER, M->mNormalsVBO);
			glBufferData(GL_ARRAY_BUFFER, NbVerts*sizeof(Point), Normals, GL_STATIC_DRAW);
		}

		glGenBuffers(1, &M->mIndicesIBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, M->mIndicesIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, NbFaces*3*sizeof(udword), surface.mDFaces, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return M;
	}





	//printf("TODO: revisit normals in this codepath!\n");

	NORMALSCREATE NCreate;
	NCreate.NbVerts		= surface.mNbVerts;
	NCreate.Verts		= surface.mVerts;
	NCreate.NbFaces		= surface.mNbFaces;
	NCreate.dFaces		= surface.mDFaces;
	NCreate.wFaces		= surface.mWFaces;
	NCreate.UseAngles	= true;

	SmoothNormals SN;
	SN.Compute(NCreate);
	const Point* Normals = SN.GetVertexNormals();

		MeshBuilder2 Builder;

		MBCreate Create;
		// Main surface
		Create.NbVerts		= surface.mNbVerts;
		Create.NbFaces		= surface.mNbFaces;
		Create.Verts		= surface.mVerts;
		Create.IndexedGeo	= true;

		// UVs
		if(UVSurface)
		{
			Create.NbTVerts		= UVSurface->GetNbVerts();
			Create.TVerts		= UVSurface->GetVerts();
			Create.IndexedUVW	= true;
		}

		// Store explicit normals in vertex colors
		if(Normals)
		{
			Create.NbCVerts			= surface.mNbVerts;
			Create.CVerts			= Normals;
			Create.IndexedColors	= true;
		}

		Create.KillZeroAreaFaces	= true;
		Create.UseW					= false;
		{
			Create.ComputeVNorm				= false;
			Create.ComputeFNorm				= false;
			Create.WeightNormalWithAngles	= false;
		}
	//	Create.ComputeNormInfo			= true;
	//	Create.RelativeIndices			= false;
	//	Create.IsSkin					= true;
	//	Create.OptimizeVertexList		= true;

		bool status = Builder.Init(Create);
		ASSERT(status);

		MBFaceData FD;
		for(udword i=0;i<Create.NbFaces;i++)
		{
			FD.Index	= i;

			// ### we don't need these 2 cases
			udword VRefs[3];
			if(surface.mDFaces)
			{
				VRefs[0] = surface.mDFaces[i*3+0];
				VRefs[1] = surface.mDFaces[i*3+1];
				VRefs[2] = surface.mDFaces[i*3+2];
			}
			else if(surface.mWFaces)
			{
				VRefs[0] = surface.mWFaces[i*3+0];
				VRefs[1] = surface.mWFaces[i*3+1];
				VRefs[2] = surface.mWFaces[i*3+2];
			}
			FD.VRefs	= VRefs;

			if(UVSurface)
				FD.TRefs	= UVSurface->GetFace(i)->mRef;

			if(Normals)
				FD.CRefs	= VRefs;
	/*
			FD.CRefs			= ColorSurface		? (udword*)ColorSurface	->GetFace(i)	: null;
	#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
			FD.TRefs2			= UVSurface2		? (udword*)UVSurface2	->GetFace(i)	: null;
	#endif
			FD.SmoothingGroups	= create.mSMG		? create.mSMG[i]						: 1;
			FD.MaterialID		= create.mDMatID	? create.mDMatID[i]	: create.mWMatID	? create.mWMatID[i]	: INVALID_ID;*/

			status = Builder.AddFace(FD);
			ASSERT(status);
		}


	MBResult Consolidated;
	status = Builder.Build(Consolidated);
	ASSERT(status);

	const udword NbVerts = Consolidated.Geometry.NbVerts;
	const udword NbFaces = Consolidated.Topology.NbFaces;
	const udword* VRefs = Consolidated.Topology.VRefs;
	const Point* Verts = reinterpret_cast<const Point*>(Consolidated.Geometry.Verts);
	const udword* VertsRefs = Consolidated.Geometry.VertsRefs;

	const float* TVerts = Consolidated.Geometry.TVerts;
	const udword* TVertsRefs = Consolidated.Geometry.TVertsRefs;

	const Point* CVerts = reinterpret_cast<const Point*>(Consolidated.Geometry.CVerts);
	const udword* CVertsRefs = Consolidated.Geometry.ColorRefs;

	Container _Verts;
	Container _UVs;
	Container _VNormals;
	{
		Point* DstVerts = reinterpret_cast<Point*>(_Verts.Reserve(NbVerts*(sizeof(Point)/sizeof(udword))));
		float* DstUVs = reinterpret_cast<float*>(_UVs.Reserve(NbVerts*2));
		Point* DstNormals = reinterpret_cast<Point*>(_VNormals.Reserve(NbVerts*(sizeof(Point)/sizeof(udword))));

		for(udword i=0;i<NbVerts;i++)
		{
			const udword VRef = VertsRefs[i];
			ASSERT(VRef<Consolidated.Geometry.NbGeomPts);
			*DstVerts++ = Verts[VRef];

			if(UVSurface)
			{
				const udword TRef = TVertsRefs[i];
				ASSERT(TRef<Consolidated.Geometry.NbTVerts);
				*DstUVs++ = TVerts[TRef*2+0];
				*DstUVs++ = TVerts[TRef*2+1];
			}

			if(Normals)
			{
				const udword CRef = CVertsRefs[i];
				ASSERT(CRef<Consolidated.Geometry.NbColorVerts);
				*DstNormals++ = -CVerts[CRef];	// ### note the sign because NORMALSCREATE doesn't do what we need
			}
		}
	}

	GLMeshEx* M = ICE_NEW(GLMeshEx);

	M->mNbVerts	= NbVerts;
	M->mNbTris	= NbFaces;

	M->mPositionsVBO = CreateArrayBuffer(NbVerts, reinterpret_cast<const Point*>(_Verts.GetEntries()));

	if(UVSurface)
	{
		glGenBuffers(1, &M->mTexCoordsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, M->mTexCoordsVBO);
		glBufferData(GL_ARRAY_BUFFER, NbVerts*sizeof(float)*2, _UVs.GetEntries(), GL_STATIC_DRAW);
	}
	else
		M->mTexCoordsVBO = 0;

	if(Normals)
	{
		glGenBuffers(1, &M->mNormalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, M->mNormalsVBO);
		glBufferData(GL_ARRAY_BUFFER, NbVerts*sizeof(Point), _VNormals.GetEntries(), GL_STATIC_DRAW);
	}
	else
		M->mNormalsVBO = 0;

	glGenBuffers(1, &M->mIndicesIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, M->mIndicesIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, NbFaces*3*sizeof(udword), VRefs, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return M;
}

void OpenGLMesh::Release(GLMeshEx* mesh)
{
	if(!mesh)
		return;

	glDeleteBuffers(1, &mesh->mPositionsVBO);
	glDeleteBuffers(1, &mesh->mTexCoordsVBO);
	glDeleteBuffers(1, &mesh->mNormalsVBO);
	glDeleteBuffers(1, &mesh->mIndicesIBO);
	DELETESINGLE(mesh);
}

void OpenGLMesh::Draw(GLMeshEx* mesh)
{
	if(!mesh)
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->mPositionsVBO);
	glVertexPointer(3, GL_FLOAT, sizeof(Point), 0);

	if(mesh->mTexCoordsVBO)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->mTexCoordsVBO);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, 0);
	}

	if(mesh->mNormalsVBO)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->mNormalsVBO);
		glNormalPointer(GL_FLOAT, sizeof(Point), 0);
	}
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mIndicesIBO);

	glDrawElements(GL_TRIANGLES, mesh->mNbTris*3, GL_UNSIGNED_INT, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	if(mesh->mNormalsVBO)
		glDisableClientState(GL_NORMAL_ARRAY);
	if(mesh->mTexCoordsVBO)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);	
}

void OpenGLMesh::UpdateVerts(GLMeshEx* mesh, const Point* verts)
{
	if(!mesh)
		return;

	UpdateVertexBuffer(mesh->mNbVerts, verts, null, mesh->mPositionsVBO, mesh->mNormalsVBO);

/*	if(mesh->mNormalsVBO)
	{
		NORMALSCREATE NCreate;
		NCreate.NbVerts		= surface.mNbVerts;
		NCreate.Verts		= surface.mVerts;
		NCreate.NbFaces		= surface.mNbFaces;
		NCreate.dFaces		= surface.mDFaces;
		NCreate.wFaces		= surface.mWFaces;
		NCreate.UseAngles	= true;

		SmoothNormals SN;
		SN.Compute(NCreate);
		const Point* Normals = SN.GetVertexNormals();

		glGenBuffers(1, &M->mNormalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, M->mNormalsVBO);
		glBufferData(GL_ARRAY_BUFFER, NbVerts*sizeof(Point), Normals, GL_STATIC_DRAW);
	}*/
}

//#endif








/*void DrawGpuMeshInstances(GpuMesh* m, const Matrix44* xforms, int n, const Vec3& color)
{
	if (m)
	{
		GLint program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &program);

		GLint param = glGetUniformLocation(program, "objectTransform");

		glVerify(glColor3fv(color));
		glVerify(glSecondaryColor3fv(color));

		glVerify(glEnableClientState(GL_VERTEX_ARRAY));
		glVerify(glBindBuffer(GL_ARRAY_BUFFER, m->mPositionsVBO));
		glVerify(glVertexPointer(3, GL_FLOAT, sizeof(float)*3, 0));	

		glVerify(glEnableClientState(GL_NORMAL_ARRAY));
		glVerify(glBindBuffer(GL_ARRAY_BUFFER, m->mNormalsVBO));
		glVerify(glNormalPointer(GL_FLOAT, sizeof(float)*3, 0));
		
		glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->mIndicesIBO));

		for (int i=0; i < n; ++i)
		{
			if (program)
				glUniformMatrix4fv( param, 1, false, xforms[i]);

			glVerify(glDrawElements(GL_TRIANGLES, m->mNumFaces*3, GL_UNSIGNED_INT, 0));
		}

		glVerify(glDisableClientState(GL_VERTEX_ARRAY));
		glVerify(glDisableClientState(GL_NORMAL_ARRAY));

		glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}
}
*/


static const char* testVertexShader = "#version 330 core\n" STRINGIFY(
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in mat4 instanceMatrix;


	uniform mat4 projection;
	uniform mat4 view;

	out vec4 varyPos;

	void main()
	{
//		gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
//		gl_Position = vec4(aPos, 1.0);
		gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
//		gl_Position = projection * view * vec4(aPos, 1.0);

		varyPos = view * vec4(aPos, 1.0);
	}
);

static const char* testPixelShader = "#version 330 core\n" STRINGIFY(
	out vec4 FragColor;
	in vec4 varyPos;

	void main()
	{
		vec3 dx = dFdx(varyPos.xyz);
		vec3 dy = dFdy(varyPos.xyz);
		vec3 normal = normalize(cross(dx, dy));

//		float diffuse = (0.3 + 0.7 * max(dot(normal, -parallelLightDir),0.0));
	//	float diffuse = max(dot(normal, -parallelLightDir), 0.0);
//		diffuse = clamp(diffuse, 0.0, 1.0);
//		vec4 color = gl_Color * diffuse;

		FragColor = vec4(normal, 1.0f);
//		FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
	} 
);

void ModernTest()
{
	if(1)
		return;
	static GLuint VBO = 0;
	static GLuint VAO = 0;
	static GLuint EBO = 0;
	static GLuint IBO = 0;
	static GLuint Prg = 0;
	static bool InitDone = false;

	static float vertices[] = {
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, 0.5f,
		 0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f,  0.5f, 0.5f
	};
	static udword indices[] = {
		0, 1, 3,
		3, 1, 2,
		4, 7, 5,
		5, 7, 6,
		0, 4, 1,
		1, 4, 5,
		7, 3, 6,
		6, 3, 2,
		0, 3, 4,
		4, 3, 7,
		5, 6, 1,
		1, 6, 2
	};
	const udword NbTris = sizeof(indices)/(sizeof(udword)*3);

	static Matrix4x4* Matrices = null;
#define NB_INSTANCES	100

	if(!InitDone)
	{
		InitDone = true;
		Prg = GLShader::CompileProgram(testVertexShader, testPixelShader, null);

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glGenBuffers(1, &IBO);


		// 1. bind Vertex Array Object
		glBindVertexArray(VAO);
		// 2. copy our vertices array in a buffer for OpenGL to use
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// 3. copy our index array in a element buffer for OpenGL to use
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		// 4. then set the vertex attributes pointers
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0); 
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
		glBindVertexArray(0); 

//		glDeleteVertexArrays(1, &VAO);
//		glDeleteBuffers(1, &VBO);
//		 glDeleteBuffers(1, &EBO);

		Matrices = ICE_NEW(Matrix4x4)[NB_INSTANCES*NB_INSTANCES];
		for(udword j=0;j<NB_INSTANCES;j++)
		{
			for(udword i=0;i<NB_INSTANCES;i++)
			{
				Matrices[i+j*NB_INSTANCES].Identity();
				Matrices[i+j*NB_INSTANCES].SetTrans(Point(float(i)*2.0f, 0.0f, float(j)*2.0f));
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, IBO);
		glBufferData(GL_ARRAY_BUFFER, NB_INSTANCES*NB_INSTANCES * sizeof(Matrix4x4), Matrices, GL_DYNAMIC_DRAW);
 
			glBindVertexArray(VAO);
			// vertex Attributes
			GLsizei vec4Size = 4*4;//sizeof(glm::vec4);
			const udword baseLoc = 1;
			glEnableVertexAttribArray(baseLoc+0);	glVertexAttribPointer(baseLoc+0, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4), (void*)0);
			glEnableVertexAttribArray(baseLoc+1);	glVertexAttribPointer(baseLoc+1, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4), (void*)(vec4Size));
			glEnableVertexAttribArray(baseLoc+2);	glVertexAttribPointer(baseLoc+2, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4), (void*)(2 * vec4Size));
			glEnableVertexAttribArray(baseLoc+3);	glVertexAttribPointer(baseLoc+3, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4), (void*)(3 * vec4Size));

			glVertexAttribDivisor(baseLoc+0, 1);
			glVertexAttribDivisor(baseLoc+1, 1);
			glVertexAttribDivisor(baseLoc+2, 1);
			glVertexAttribDivisor(baseLoc+3, 1);

			glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0); 
	}

	glUseProgram(Prg);

	// We must set these after selecting the prg
	{
		Matrix4x4 MV;
		GetModelViewMatrix(MV);

		udword loc = glGetUniformLocation(Prg, "view");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &MV.m[0][0]);
	}

	{
		Matrix4x4 P;
		GetProjMatrix(P);

		udword loc = glGetUniformLocation(Prg, "projection");
		glUniformMatrix4fv(loc, 1, GL_FALSE, &P.m[0][0]);
	}

	{
		glBindBuffer(GL_ARRAY_BUFFER, IBO);

		static float Time = 0.0f;
		Time += 0.1f;
		Matrix3x3 M;
		M.RotYX(Time, Time*1.17f);

		for(udword j=0;j<NB_INSTANCES;j++)
		{
			for(udword i=0;i<NB_INSTANCES;i++)
			{
				Matrices[i+j*NB_INSTANCES] = M;
				Matrices[i+j*NB_INSTANCES].SetTrans(Point(float(i)*2.0f, 0.0f, float(j)*2.0f));
			}
		}

		glBufferData(GL_ARRAY_BUFFER, NB_INSTANCES*NB_INSTANCES * sizeof(Matrix4x4), Matrices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); 
	}

	glBindVertexArray(VAO);
//	glDrawArrays(GL_TRIANGLES, 0, 3);
//	glDrawElements(GL_TRIANGLES, NbTris*3, GL_UNSIGNED_INT, 0);
	glDrawElementsInstanced(GL_TRIANGLES, NbTris*3, GL_UNSIGNED_INT, 0, NB_INSTANCES*NB_INSTANCES);

//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
	glBindVertexArray(0);

}

