///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PintDLMeshShapeRenderer.h"

extern float gConvexEdgesThreshold;
extern float gChaoticEdgesCoeff;
//extern bool gChaoticEdges;

// A basic display-list-based mesh renderer. Not batched.

///////////////////////////////////////////////////////////////////////////////

ActiveEdgesRenderer::ActiveEdgesRenderer() : mActiveEdgesDL(INVALID_ID)
{
}

ActiveEdgesRenderer::~ActiveEdgesRenderer()
{
	Release();
}

void ActiveEdgesRenderer::Release()
{
	if(mActiveEdgesDL!=INVALID_ID)
	{
		glDeleteLists(mActiveEdgesDL, 1);
		mActiveEdgesDL = INVALID_ID;
	}
}

static inline_ void OutputSegment(const Point& p0, const Point& p1, const Point& right, float coeff)
{
	Point mid = (p0+p1)*0.5f;
	Point p2 = mid + right*coeff*(UnitRandomFloat()-0.5f);

	glVertex3f(p0.x, p0.y, p0.z);
	glVertex3f(p2.x, p2.y, p2.z);

	glVertex3f(p2.x, p2.y, p2.z);
	glVertex3f(p1.x, p1.y, p1.z);
}

void ActiveEdgesRenderer::CreateDL(const SurfaceInterface& surface)
{
	if(1)
	{
		// This is a bit unfortunate but the mesh data coming from zb2 files is not always optimized.
		MeshBuilder2 Builder;

		MBCreate Create;
		Create.NbVerts				= surface.mNbVerts;
		Create.NbFaces				= surface.mNbFaces;
		Create.Verts				= surface.mVerts;
		Create.KillZeroAreaFaces	= true;
		Create.UseW					= false;
		Create.IndexedGeo			= true;

		bool status = Builder.Init(Create);
		ASSERT(status);

		MBFaceData FD;
		udword tmp[3];
		for(udword i=0;i<Create.NbFaces;i++)
		{
			FD.Index	= i;
			if(surface.mDFaces)
			{
				tmp[0] = surface.mDFaces[i*3+0];
				tmp[1] = surface.mDFaces[i*3+1];
				tmp[2] = surface.mDFaces[i*3+2];
			}
			else if(surface.mWFaces)
			{
				tmp[0] = surface.mWFaces[i*3+0];
				tmp[1] = surface.mWFaces[i*3+1];
				tmp[2] = surface.mWFaces[i*3+2];
			}

			FD.VRefs	= tmp;
			status = Builder.AddFace(FD);
			ASSERT(status);
		}

		MBResult Consolidated;
		status = Builder.Build(Consolidated);
		ASSERT(status);

		const udword NbVerts = Consolidated.Geometry.NbVerts;
		const udword NbFaces = Consolidated.Topology.NbFaces;
		const udword* VRefs = Consolidated.Topology.VRefs;
		const udword* VertsRefs = Consolidated.Geometry.VertsRefs;

		const Point* NewVerts = reinterpret_cast<const Point*>(Consolidated.Geometry.Verts);
		IndexedTriangle* NewTris = ICE_NEW(IndexedTriangle)[NbFaces];
		for(udword i=0;i<NbFaces;i++)
		{
			// Refs to "fat" consolidated vertices
			const udword VRef0 = VRefs[i*3+0];
			const udword VRef1 = VRefs[i*3+1];
			const udword VRef2 = VRefs[i*3+2];
			ASSERT(VRef0<NbVerts);
			ASSERT(VRef1<NbVerts);
			ASSERT(VRef2<NbVerts);

			// Refs to reduced set of vertices
			const udword vref0 = VertsRefs[VRef0];
			const udword vref1 = VertsRefs[VRef1];
			const udword vref2 = VertsRefs[VRef2];
			ASSERT(vref0<Consolidated.Geometry.NbGeomPts);
			ASSERT(vref1<Consolidated.Geometry.NbGeomPts);
			ASSERT(vref2<Consolidated.Geometry.NbGeomPts);

			NewTris[i].mRef[0] = vref0;
			NewTris[i].mRef[1] = vref1;
			NewTris[i].mRef[2] = vref2;
		}

		EDGELISTCREATE EC;
		EC.NbFaces		= NbFaces;
		EC.DFaces		= NewTris->mRef;
		EC.FacesToEdges	= true;
		EC.EdgesToFaces	= true;
		EC.Verts		= NewVerts;
		EC.Epsilon		= gConvexEdgesThreshold;

		mActiveEdgesDL = glGenLists(1);
		glNewList(mActiveEdgesDL, GL_COMPILE);

		EdgeList EL;
		if(EL.Init(EC))
		{
			glBegin(GL_LINES);		
				const udword NbEdges = EL.GetNbEdges();
				const EdgeDesc* ED = EL.GetEdgeToTriangles();
				const Edge* Edges = EL.GetEdges();
				for(udword i=0;i<NbEdges;i++)
				{
					if(ED[i].IsActive())
					{
						const Point& p0 = NewVerts[Edges[i].mRef0];
						const Point& p1 = NewVerts[Edges[i].mRef1];
						if(gChaoticEdgesCoeff!=0.0f && ED[i].Count>=1)
						//if(gChaoticEdges && ED[i].Count>=1)
						{
							const udword  Offset = ED[i].Offset;
							udword Tri0 = EL.GetFacesByEdges(Offset);
							//udword Tri1 = EL.GetFacesByEdges(Offset+1);

							Point N;
							NewTris[Tri0].Normal(NewVerts, N);

							Point Right, Up;
							ComputeBasis2(N, Right, Up);

							/*Point mid = (p0+p1)*0.5f;
							Point p2 = mid + Right*0.4f*(UnitRandomFloat()-0.5f);

							glVertex3f(p0.x, p0.y, p0.z);
							glVertex3f(p2.x, p2.y, p2.z);

							glVertex3f(p2.x, p2.y, p2.z);
							glVertex3f(p1.x, p1.y, p1.z);*/

							//OutputSegment(p0, p1, Right, 0.4f);

							const Point m0 = (p0+p1)*0.5f;
							//const Point m00 = (p0+m0)*0.5f;
							//const Point m01 = (m0+p1)*0.5f;

							//OutputSegment(p0, m0, Right, 0.4f);
							//OutputSegment(m0, p1, Right, 0.4f);
							OutputSegment(p0, m0, Right, gChaoticEdgesCoeff);
							OutputSegment(m0, p1, Right, gChaoticEdgesCoeff);
						}
						else
						{
							glVertex3f(p0.x, p0.y, p0.z);
							glVertex3f(p1.x, p1.y, p1.z);
						}
					}
				}
			glEnd();
		}

		glEndList();

		DELETEARRAY(NewTris);
	}
	else
	{
		EDGELISTCREATE EC;
		EC.NbFaces		= surface.mNbFaces;
		EC.DFaces		= surface.mDFaces;
		EC.WFaces		= surface.mWFaces;
		EC.FacesToEdges	= true;
		EC.EdgesToFaces	= true;
		EC.Verts		= surface.mVerts;
		EC.Epsilon		= gConvexEdgesThreshold;

		Vertices	mActiveEdges;
		EdgeList EL;
		if(EL.Init(EC))
		{
			const udword NbEdges = EL.GetNbEdges();
			const EdgeDesc* ED = EL.GetEdgeToTriangles();
			const Edge* Edges = EL.GetEdges();
			for(udword i=0;i<NbEdges;i++)
			{
				if(ED[i].IsActive())
				{
					const Point& p0 = surface.mVerts[Edges[i].mRef0];
					const Point& p1 = surface.mVerts[Edges[i].mRef1];
					mActiveEdges.AddVertex(p0).AddVertex(p1);
				}
			}
		}

	/*	for(udword i=0;i<surface.mNbFaces;i++)
		{
			const udword VRef0 = surface.mDFaces[i*3+0];
			const udword VRef1 = surface.mDFaces[i*3+1];
			const udword VRef2 = surface.mDFaces[i*3+2];
			const Point& p0 = surface.mVerts[VRef0];
			const Point& p1 = surface.mVerts[VRef1];
			const Point& p2 = surface.mVerts[VRef2];
			mActiveEdges.AddVertex(p0).AddVertex(p1);
			mActiveEdges.AddVertex(p1).AddVertex(p2);
			mActiveEdges.AddVertex(p2).AddVertex(p0);
		}*/

		mActiveEdgesDL = glGenLists(1);

		glNewList(mActiveEdgesDL, GL_COMPILE);
			const udword NbSegments = mActiveEdges.GetNbVertices()/2;
			const Point* V = mActiveEdges.GetVertices();
			glBegin(GL_LINES);
			for(udword i=0;i<NbSegments;i++)
			{
				const Point& p0 = *V++;
				const Point& p1 = *V++;
				glVertex3f(p0.x, p0.y, p0.z);
				glVertex3f(p1.x, p1.y, p1.z);
			}
			glEnd();
		glEndList();
	}
}

///////////////////////////////////////////////////////////////////////////////

PintDLMeshShapeRenderer::PintDLMeshShapeRenderer(const PintSurfaceInterface& surface, udword flags) : mData(surface, flags)
{
	CreateDisplayList(surface, flags);
}

PintDLMeshShapeRenderer::PintDLMeshShapeRenderer(const MultiSurface& multi_surface, udword flags) : mData(PintSurfaceInterface(multi_surface.GetSurfaceInterface()), flags)
{
	CreateDisplayList(multi_surface, flags);
}

PintDLMeshShapeRenderer::~PintDLMeshShapeRenderer()
{
}

void PintDLMeshShapeRenderer::CreateDisplayList(const SurfaceInterface& surface, udword flags, const IndexedSurface* UVSurface)
{
	//flags = DL_MESH_USE_ACTIVE_EDGES|DL_MESH_USE_VERTEX_NORMALS|DL_MESH_USE_SMOOTH_NORMALS;

	if(flags & (DL_MESH_USE_DIRECT_DATA|DL_MESH_USE_VERTEX_NORMALS))
		mBaseFlags |= SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS;

	const BOOL active_edges = flags & DL_MESH_USE_ACTIVE_EDGES;
	const BOOL uses_vertex_normals = flags & DL_MESH_USE_VERTEX_NORMALS;
	const BOOL uses_smooth_normals = flags & DL_MESH_USE_SMOOTH_NORMALS;

	if(0)
	{
		AABB Bounds;
		ComputeAABB(Bounds, surface.mVerts, surface.mNbVerts);
		Point Center;	Bounds.GetCenter(Center);
		Point Extents;	Bounds.GetExtents(Extents);
		int stop=1;
	}

	if(active_edges)
		mActiveEdges.CreateDL(surface);

	if(!surface.mNbVerts || !surface.mNbFaces)
	{
		glNewList(mDisplayListNum, GL_COMPILE);
//			glBegin(GL_TRIANGLES);
//			glEnd();
		glEndList();
		return;
	}

	if(flags & DL_MESH_USE_DIRECT_DATA)
	{
		// This codepath just for Mecabricks models
		if(UVSurface)
		{
			ASSERT(surface.mNbFaces == UVSurface->GetNbFaces());
		}

		//printf("TODO: revisit normals in this codepath!\n");
		NORMALSCREATE Create;
		Create.NbVerts		= surface.mNbVerts;
		Create.Verts		= surface.mVerts;
		Create.NbFaces		= surface.mNbFaces;
		Create.dFaces		= surface.mDFaces;
		Create.wFaces		= surface.mWFaces;
		Create.UseAngles	= true;

		SmoothNormals SN;
		SN.Compute(Create);
		const Point* Normals = SN.GetVertexNormals();

		glNewList(mDisplayListNum, GL_COMPILE);
			glBegin(GL_TRIANGLES);

			for(udword i=0;i<surface.mNbFaces;i++)
			{
				udword VRef0, VRef1, VRef2;
				if(surface.mDFaces)
				{
					VRef0 = surface.mDFaces[i*3+0];
					VRef1 = surface.mDFaces[i*3+1];
					VRef2 = surface.mDFaces[i*3+2];
				}
				else if(surface.mWFaces)
				{
					VRef0 = surface.mWFaces[i*3+0];
					VRef1 = surface.mWFaces[i*3+1];
					VRef2 = surface.mWFaces[i*3+2];
				}
				ASSERT(VRef0<surface.mNbVerts);
				ASSERT(VRef1<surface.mNbVerts);
				ASSERT(VRef2<surface.mNbVerts);
/*				const Point& v0 = surface.mVerts[VRef0];
				const Point& v1 = surface.mVerts[VRef1];
				const Point& v2 = surface.mVerts[VRef2];

				if(uses_vertex_normals && !uses_smooth_normals)
				{
					const Point Normal = ((v1-v0)^(v2-v0)).Normalize();
					glNormal3f(Normal.x, Normal.y, Normal.z);
				}*/

				//////////

				if(1)
				{
					const udword VRefs[3] = { VRef0, VRef1, VRef2 };

					for(udword j=0;j<3;j++)
					{
						const udword VRef = VRefs[j];
						{
							const Point& n = Normals[VRef];
							glNormal3f(-n.x, -n.y, -n.z);	// ### note the sign because NORMALSCREATE doesn't do what we need
						}

						if(UVSurface)
						{
							const udword TRef = UVSurface->GetFace(i)->mRef[j];
							ASSERT(TRef<UVSurface->GetNbVerts());
							const Point& UVs = UVSurface->GetVerts()[TRef];
							glTexCoord2f(UVs.x, UVs.y);
						}

						const Point& v = surface.mVerts[VRef];
						glVertex3f(v.x, v.y, v.z);
					}
				}
/*				else
				{

//				if(uses_vertex_normals && uses_smooth_normals)
				{
					const Point& n0 = Normals[VRef0];
					glNormal3f(n0.x, n0.y, n0.z);
				}

				if(UVSurface)
				{
					const udword TRef0 = UVSurface->GetFace(i)->mRef[0];
					ASSERT(TRef0<UVSurface->GetNbVerts());
					const Point& UVs = UVSurface->GetVerts()[TRef0];
					glTexCoord2f(UVs.x, UVs.y);
				}

				glVertex3f(v0.x, v0.y, v0.z);

				//////////

//				if(uses_vertex_normals && uses_smooth_normals)
				{
					const Point& n1 = Normals[VRef1];
					glNormal3f(n1.x, n1.y, n1.z);
				}

				if(UVSurface)
				{
					const udword TRef1 = UVSurface->GetFace(i)->mRef[1];
					ASSERT(TRef1<UVSurface->GetNbVerts());
					const Point& UVs = UVSurface->GetVerts()[TRef1];
					glTexCoord2f(UVs.x, UVs.y);
				}

				glVertex3f(v1.x, v1.y, v1.z);

				//////////

//				if(uses_vertex_normals && uses_smooth_normals)
				{
					const Point& n2 = Normals[VRef2];
					glNormal3f(n2.x, n2.y, n2.z);
				}

				if(UVSurface)
				{
					const udword TRef2 = UVSurface->GetFace(i)->mRef[2];
					ASSERT(TRef2<UVSurface->GetNbVerts());
					const Point& UVs = UVSurface->GetVerts()[TRef2];
					glTexCoord2f(UVs.x, UVs.y);
				}

				glVertex3f(v2.x, v2.y, v2.z);
				}*/
			}
			glEnd();
		glEndList();
		return;
	}

	// Good old class! Ideally we'd reuse ICE's ConsolidatedSurface class but at the moment including the
	// renderer headers gives a lot of "ambiguous symbol" errors. So for now we just replicate the ICE code here.

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

	// Vertex colors
//	Create.NbCVerts;
//	Create.CVerts
//	Create.IndexedColors			= true;

	Create.KillZeroAreaFaces		= true;
	Create.UseW						= false;
	if(uses_vertex_normals && uses_smooth_normals)
	{
		Create.ComputeVNorm				= true;
		Create.ComputeFNorm				= true;
		Create.WeightNormalWithAngles	= true;
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
/*
		FD.CRefs			= ColorSurface		? (udword*)ColorSurface	->GetFace(i)	: null;
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
		FD.TRefs2			= UVSurface2		? (udword*)UVSurface2	->GetFace(i)	: null;
#endif
		FD.SmoothingGroups	= create.mSMG		? create.mSMG[i]						: 1;
		FD.MaterialID		= create.mDMatID	? create.mDMatID[i]	: create.mWMatID	? create.mWMatID[i]	: INVALID_ID;*/

		status = Builder.AddFace(FD);
		ASSERT(status);

		if(0)
		{
			if(VRefs[0]>=surface.mNbVerts)
				printf("Invalid vertex index!\n");
			if(VRefs[1]>=surface.mNbVerts)
				printf("Invalid vertex index!\n");
			if(VRefs[2]>=surface.mNbVerts)
				printf("Invalid vertex index!\n");
		}
	}

	MBResult Consolidated;
	status = Builder.Build(Consolidated);
	ASSERT(status);

	const udword NbVerts = Consolidated.Geometry.NbVerts;
	/*const*/ udword NbFaces = Consolidated.Topology.NbFaces;
	const udword* VRefs = Consolidated.Topology.VRefs;
	const Point* Verts = reinterpret_cast<const Point*>(Consolidated.Geometry.Verts);
	const Point* Normals = reinterpret_cast<const Point*>(Consolidated.Geometry.Normals);
	const udword* VertsRefs = Consolidated.Geometry.VertsRefs;

	const float* TVerts = Consolidated.Geometry.TVerts;
	const udword* TVertsRefs = Consolidated.Geometry.TVertsRefs;

	glNewList(mDisplayListNum, GL_COMPILE);
		glBegin(GL_TRIANGLES);
		while(NbFaces--)
		{
			// Refs to "fat" consolidated vertices
			const udword VRef0 = *VRefs++;
			const udword VRef1 = *VRefs++;
			const udword VRef2 = *VRefs++;
			ASSERT(VRef0<NbVerts);
			ASSERT(VRef1<NbVerts);
			ASSERT(VRef2<NbVerts);

			// Refs to reduced set of vertices
			const udword vref0 = VertsRefs[VRef0];
			const udword vref1 = VertsRefs[VRef1];
			const udword vref2 = VertsRefs[VRef2];
			ASSERT(vref0<Consolidated.Geometry.NbGeomPts);
			ASSERT(vref1<Consolidated.Geometry.NbGeomPts);
			ASSERT(vref2<Consolidated.Geometry.NbGeomPts);

			const Point& v0 = Verts[vref0];
			const Point& v1 = Verts[vref1];
			const Point& v2 = Verts[vref2];

			if(uses_vertex_normals && !uses_smooth_normals)
			{
				const Point Normal = ((v1-v0)^(v2-v0)).Normalize();
				glNormal3f(Normal.x, Normal.y, Normal.z);
			}

			/////

			const Point* Pts[] = { &v0, &v1, &v2 };
			const udword VRefs[] = { VRef0, VRef1, VRef2 };
			for(udword j=0;j<3;j++)
			{
				const udword VRef = VRefs[j];
				if(uses_vertex_normals && uses_smooth_normals)
				{
					const Point& n = Normals[VRef];
					glNormal3f(n.x, n.y, n.z);		
				}

				if(TVertsRefs && TVerts)
				{
					const udword tref = TVertsRefs[VRef];
					ASSERT(tref<Consolidated.Geometry.NbTVerts);
					glTexCoord2f(TVerts[tref*2+0], TVerts[tref*2+1]);
				}

				glVertex3f(Pts[j]->x, Pts[j]->y, Pts[j]->z);
			}
		}
		glEnd();
	glEndList();
}

void PintDLMeshShapeRenderer::CreateDisplayList(const MultiSurface& multi_surface, udword flags)
{
	if(flags & (DL_MESH_USE_DIRECT_DATA|DL_MESH_USE_VERTEX_NORMALS))
		mBaseFlags |= SHAPE_RENDERER_USES_EXPLICIT_VERTEX_NORMALS;

	const SurfaceInterface surface = multi_surface.GetSurfaceInterface();

	const IndexedSurface* UVSurface = multi_surface.GetExtraSurface(SURFACE_UVS);

	CreateDisplayList(surface, flags, UVSurface);
	return;

#ifdef REMOVED
	if(!UVSurface)
	{
		CreateDisplayList(surface, flags);
		return;
	}

	const BOOL uses_active_edges = flags & DL_MESH_USE_ACTIVE_EDGES;
	const BOOL uses_vertex_normals = flags & DL_MESH_USE_VERTEX_NORMALS;

	if(uses_active_edges)
		mActiveEdges.CreateDL(surface);

	if(!surface.mNbVerts || !surface.mNbFaces)
	{
		glNewList(mDisplayListNum, GL_COMPILE);
//			glBegin(GL_TRIANGLES);
//			glEnd();
		glEndList();
		return;
	}

	if(!uses_vertex_normals)
	{
		// Good old class! Ideally we'd reuse ICE's ConsolidatedSurface class but at the moment including the
		// renderer headers gives a lot of "ambiguous symbol" errors. So for now we just replicate the ICE code here.

		MeshBuilder2 Builder;

		MBCreate Create;

		Create.NbVerts					= surface.mNbVerts;
		Create.NbFaces					= surface.mNbFaces;
		Create.Verts					= surface.mVerts;

		Create.NbTVerts					= UVSurface		? UVSurface		->GetNbVerts() : 0;
		Create.TVerts					= UVSurface		? UVSurface		->GetVerts() : null;

//		Create.NbCVerts;
//		Create.CVerts

		Create.KillZeroAreaFaces		= true;
		Create.UseW						= false;
//		Create.ComputeVNorm				= true;
//		Create.ComputeFNorm				= true;
//		Create.ComputeNormInfo			= true;
		Create.IndexedGeo				= true;
		Create.IndexedUVW				= true;
//		Create.IndexedColors			= true;
//		Create.RelativeIndices			= false;
//		Create.IsSkin					= true;
//		Create.WeightNormalWithAngles	= true;
//		Create.OptimizeVertexList		= true;

		bool status = Builder.Init(Create);
		ASSERT(status);

		MBFaceData FD;
		for(udword i=0;i<Create.NbFaces;i++)
		{
			FD.Index	= i;
			FD.VRefs	= (udword*)&surface.mDFaces[i*3];
			FD.TRefs	= UVSurface			? (udword*)UVSurface	->GetFace(i)	: null;
			status = Builder.AddFace(FD);
			ASSERT(status);
/*
		FD.CRefs			= ColorSurface		? (udword*)ColorSurface	->GetFace(i)	: null;
#ifdef MMZ_SUPPORT_SECOND_MAPPING_CHANNEL
		FD.TRefs2			= UVSurface2		? (udword*)UVSurface2	->GetFace(i)	: null;
#endif
		FD.SmoothingGroups	= create.mSMG		? create.mSMG[i]						: 1;
		FD.MaterialID		= create.mDMatID	? create.mDMatID[i]	: create.mWMatID	? create.mWMatID[i]	: INVALID_ID;

		if(!Builder.AddFace(FD))	return SetIceError("ConsolidatedSurface::Consolidate: MeshBuilder AddFace failed!", null);*/

		}

		MBResult Consolidated;
		status = Builder.Build(Consolidated);
		ASSERT(status);

		const udword NbVerts = Consolidated.Geometry.NbVerts;
		/*const*/ udword NbFaces = Consolidated.Topology.NbFaces;
		const udword* VRefs = Consolidated.Topology.VRefs;
		const float* Verts = Consolidated.Geometry.Verts;
		const float* TVerts = Consolidated.Geometry.TVerts;
		const float* Normals = Consolidated.Geometry.Normals;
		const udword* VertsRefs = Consolidated.Geometry.VertsRefs;
		const udword* TVertsRefs = Consolidated.Geometry.TVertsRefs;

		glNewList(mDisplayListNum, GL_COMPILE);
			glBegin(GL_TRIANGLES);
			while(NbFaces--)
			{
				// Refs to "fat" consolidated vertices
				const udword VRef0 = *VRefs++;
				const udword VRef1 = *VRefs++;
				const udword VRef2 = *VRefs++;
				ASSERT(VRef0<NbVerts);
				ASSERT(VRef1<NbVerts);
				ASSERT(VRef2<NbVerts);

				// Refs to reduced set of vertices
				const udword vref0 = VertsRefs[VRef0];
				const udword vref1 = VertsRefs[VRef1];
				const udword vref2 = VertsRefs[VRef2];
				ASSERT(vref0<Consolidated.Geometry.NbGeomPts);
				ASSERT(vref1<Consolidated.Geometry.NbGeomPts);
				ASSERT(vref2<Consolidated.Geometry.NbGeomPts);

				const udword tref0 = TVertsRefs[VRef0];
				const udword tref1 = TVertsRefs[VRef1];
				const udword tref2 = TVertsRefs[VRef2];
				ASSERT(tref0<Consolidated.Geometry.NbTVerts);
				ASSERT(tref1<Consolidated.Geometry.NbTVerts);
				ASSERT(tref2<Consolidated.Geometry.NbTVerts);

				const Point v0(Verts[vref0*3+0], Verts[vref0*3+1], Verts[vref0*3+2]);
				const Point v1(Verts[vref1*3+0], Verts[vref1*3+1], Verts[vref1*3+2]);
				const Point v2(Verts[vref2*3+0], Verts[vref2*3+1], Verts[vref2*3+2]);

				const Point uv0(TVerts[tref0*2+0], TVerts[tref0*2+1], 0.0f);
				const Point uv1(TVerts[tref1*2+0], TVerts[tref1*2+1], 0.0f);
				const Point uv2(TVerts[tref2*2+0], TVerts[tref2*2+1], 0.0f);

				glTexCoord2f(uv0.x, uv0.y);
				glVertex3f(v0.x, v0.y, v0.z);

				glTexCoord2f(uv1.x, uv1.y);
				glVertex3f(v1.x, v1.y, v1.z);

				glTexCoord2f(uv2.x, uv2.y);
				glVertex3f(v2.x, v2.y, v2.z);
			}
			glEnd();
		glEndList();
	}
	else
	{
		glNewList(mDisplayListNum, GL_COMPILE);
			glBegin(GL_TRIANGLES);
			if(UVSurface)
			{
				for(udword i=0;i<surface.mNbFaces;i++)
				{
					const IndexedTriangle* UVTriangle = UVSurface->GetFace(i);

					const udword TRef0 = UVTriangle->mRef[0];
					const udword TRef1 = UVTriangle->mRef[1];
					const udword TRef2 = UVTriangle->mRef[2];
					const Point* TVerts = UVSurface->GetVerts();
					const Point& uv0 = TVerts[TRef0];
					const Point& uv1 = TVerts[TRef1];
					const Point& uv2 = TVerts[TRef2];

					const udword VRef0 = surface.mDFaces[i*3+0];
					const udword VRef1 = surface.mDFaces[i*3+1];
					const udword VRef2 = surface.mDFaces[i*3+2];
					const Point& p0 = surface.mVerts[VRef0];
					const Point& p1 = surface.mVerts[VRef1];
					const Point& p2 = surface.mVerts[VRef2];
	//				const Point Normal = ((p2-p0)^(p1-p0)).Normalize();
					const Point Normal = ((p1-p0)^(p2-p0)).Normalize();
					glNormal3f(Normal.x, Normal.y, Normal.z);
					glTexCoord2f(uv0.x, uv0.y);
					glVertex3f(p0.x, p0.y, p0.z);

					glTexCoord2f(uv1.x, uv1.y);
					glVertex3f(p1.x, p1.y, p1.z);

					glTexCoord2f(uv2.x, uv2.y);
					glVertex3f(p2.x, p2.y, p2.z);
				}
			}
			else
			{
				for(udword i=0;i<surface.mNbFaces;i++)
				{
					const udword VRef0 = surface.mDFaces[i*3+0];
					const udword VRef1 = surface.mDFaces[i*3+1];
					const udword VRef2 = surface.mDFaces[i*3+2];
					const Point& p0 = surface.mVerts[VRef0];
					const Point& p1 = surface.mVerts[VRef1];
					const Point& p2 = surface.mVerts[VRef2];
	//				const Point Normal = ((p2-p0)^(p1-p0)).Normalize();
					const Point Normal = ((p1-p0)^(p2-p0)).Normalize();
					glNormal3f(Normal.x, Normal.y, Normal.z);
					glVertex3f(p0.x, p0.y, p0.z);
					glVertex3f(p1.x, p1.y, p1.z);
					glVertex3f(p2.x, p2.y, p2.z);
				}
			}
			glEnd();
		glEndList();
	}
#endif
}

#include "GLRenderHelpers.h"
extern	bool	gWireframePass;
void PintDLMeshShapeRenderer::_Render(const PR& pose) const
{
	if(gWireframePass && mActiveEdges.mActiveEdgesDL!=INVALID_ID)
	{
		glPushMatrix();
			GLRenderHelpers::SetupGLMatrix(pose);
			glCallList(mActiveEdges.mActiveEdgesDL);
		glPopMatrix();
	}
	else
	{
		PintDLShapeRenderer::_Render(pose);
	}
}

///////////////////////////////////////////////////////////////////////////////

PintDLMeshShapeRenderer2::PintDLMeshShapeRenderer2(const PintSurfaceInterface& surface, udword flags) : PintDLMeshShapeRenderer(surface, flags)
{
}

PintDLMeshShapeRenderer2::PintDLMeshShapeRenderer2(const MultiSurface& multi_surface, udword flags) : PintDLMeshShapeRenderer(multi_surface, flags)
{
}

PintDLMeshShapeRenderer2::~PintDLMeshShapeRenderer2()
{
}

void PintDLMeshShapeRenderer2::Init(const TriSurface& ts)
{
	mActiveEdges.Release();

	glDeleteLists(mDisplayListNum, 1);
	mDisplayListNum = glGenLists(1);


	const udword NbFaces = ts.GetNbFaces();
	Triangle* Faces = ts.GetFaces();

	udword* Tmp = new udword[NbFaces*3];
	for(udword i=0;i<NbFaces*3;i++)
	{
		Tmp[i] = i;
	}

	SurfaceInterface SI;

	SI.mNbVerts		= NbFaces*3;
	SI.mVerts		= Faces->mVerts;
	SI.mNbFaces		= NbFaces;
	SI.mDFaces		= Tmp;
	SI.mWFaces		= null;

	CreateDisplayList(SI, DL_MESH_USE_ACTIVE_EDGES);
	delete [] Tmp;
}

void PintDLMeshShapeRenderer2::_Render(const PR& pose) const
{
	if(gWireframePass && mActiveEdges.mActiveEdgesDL!=INVALID_ID)
	{
		glPushMatrix();
//			GLRenderHelpers::SetupGLMatrix(pose);
			glCallList(mActiveEdges.mActiveEdgesDL);
		glPopMatrix();
	}
	else
	{
//		PintDLShapeRenderer::Render(pose);

		glPushMatrix();
		{
			glCallList(mDisplayListNum);
		}
		glPopMatrix();

	}
}