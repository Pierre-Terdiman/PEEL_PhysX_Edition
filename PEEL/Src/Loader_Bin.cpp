///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader_Bin.h"
#include "SurfaceManager.h"
#include "Common.h"
#include "ProgressBar.h"
#include "MeshCleaner.h"

static const bool gUseMeshCleaner = false;

static bool SaveBIN(const char* filename, const IndexedSurface& surface)
{
	const udword NbVerts = surface.GetNbVerts();
	const udword NbFaces = surface.GetNbFaces();
	const Point* Verts = surface.GetVerts();
	const IndexedTriangle* F = surface.GetFaces();

	CustomArray CA;
	CA.Store(udword(1));	// # meshes
	CA.Store(udword(1));	// Collidable
	CA.Store(udword(1));	// Renderable
	CA.Store(NbVerts);
	CA.Store(NbFaces);

	for(udword i=0;i<NbVerts;i++)
	{
		CA.Store(Verts[i].x);
		CA.Store(Verts[i].y);
		CA.Store(Verts[i].z);
	}

	for(udword i=0;i<NbFaces;i++)
	{
		CA.Store(F[i].mRef[0]);
		CA.Store(F[i].mRef[1]);
		CA.Store(F[i].mRef[2]);
	}

	CA.ExportToDisk(filename);
	return true;
}

static void Tessellate(IndexedSurface* IS, udword level, TessellationScheme ts)
{
	while(level--)
	{
		if(ts==TESS_BUTTERFLY)
		{
			ButterflyScheme BS;
			IS->Subdivide(BS);
		}
		else if(ts==TESS_POLYHEDRAL)
		{
			PolyhedralScheme PS;
			IS->Subdivide(PS);
		}
	}
}

static bool LoadBIN(const char* filename, SurfaceManager& test, const Point* scale, bool merge_meshes, bool reset_pivots, udword tessellation, TessellationScheme ts)
{
	IceFile BinFile(filename);
	if(!BinFile.IsValid())
		return false;

	const udword NbMeshes = BinFile.LoadDword();
	printf("-LoadBIN: loading %d meshes...\n", NbMeshes);

	struct Load
	{
		static void Vertices(IndexedSurface& surface, udword nb_verts, IceFile& file, AABB& bounds, const Point* scale)
		{
			Point* Verts = surface.GetVerts();
			file.LoadBuffer(Verts, nb_verts*3*sizeof(float));
			for(udword j=0;j<nb_verts;j++)
			{
//				Verts[j].x = file.LoadFloat();
//				Verts[j].y = file.LoadFloat();
//				Verts[j].z = file.LoadFloat();
				if(scale)
				{
					Verts[j].x *= scale->x;
					Verts[j].y *= scale->y;
					Verts[j].z *= scale->z;
				}

				if(0)
				{
					Matrix3x3 RotX;
					RotX.RotX(HALFPI*0.5f);
					Verts[j] *= RotX;
					Verts[j] += Point(0.1f, -0.2f, 0.3f);
				}

				bounds.Extend(Verts[j]);
			}
		}

		static void Triangles(IndexedSurface& surface, udword nb_faces, IceFile& file)
		{
			IndexedTriangle* F = surface.GetFaces();
/*			for(udword j=0;j<nb_faces;j++)
			{
				F[j].mRef[0] = file.LoadDword();
				F[j].mRef[1] = file.LoadDword();
				F[j].mRef[2] = file.LoadDword();
			}*/
			file.LoadBuffer(F, nb_faces*3*sizeof(udword));
		}
	};

	CreateProgressBar(NbMeshes, _F("Loading %s...", filename));

	AABB GlobalBounds;
	GlobalBounds.SetEmpty();
	udword TotalNbTris = 0;
	udword TotalNbVerts = 0;
	if(!merge_meshes)
	{
		for(udword i=0;i<NbMeshes;i++)
		{
			SetProgress(i);

			const udword Collidable = BinFile.LoadDword();
			(void)Collidable;
			const udword Renderable = BinFile.LoadDword();
			(void)Renderable;

			const udword NbVerts = BinFile.LoadDword();
			const udword NbFaces = BinFile.LoadDword();

//			TotalNbTris += NbFaces;
//			TotalNbVerts += NbVerts;

			IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
			bool Status = IS->Init(NbFaces, NbVerts);
			ASSERT(Status);

			Load::Vertices(*IS, NbVerts, BinFile, GlobalBounds, scale);

			Load::Triangles(*IS, NbFaces, BinFile);

/*			if(tessellation)
			{
				for(udword j=0;j<tessellation;j++)
				{
					if(ts==TESS_BUTTERFLY)
					{
						ButterflyScheme BS;
						IS->Subdivide(BS);
					}
					else if(ts==TESS_POLYHEDRAL)
					{
						PolyhedralScheme PS;
						IS->Subdivide(PS);
					}
				}
			}*/
			if(tessellation)
				Tessellate(IS, tessellation, ts);

			if(gUseMeshCleaner)
			{
				MeshCleaner Cleaner(IS->GetNbVerts(), IS->GetVerts(), IS->GetNbFaces(), IS->GetFaces()->mRef);
				IS->Init(Cleaner.mNbTris, Cleaner.mNbVerts, Cleaner.mVerts, (const IndexedTriangle*)Cleaner.mIndices);
			}

			TotalNbTris += IS->GetNbFaces();
			TotalNbVerts += IS->GetNbVerts();

//			SaveBIN("c:\\TessBunny.bin", *IS);

			if(reset_pivots)
			{
				const Point& LocalCenter = IS->GetLocalGeomCenter();

				IS->Translate(-LocalCenter);

				const PR Pose(LocalCenter, Quat(Idt));
				test.RegisterSurface(IS, null, &Pose, null);
			}
			else
			{
				test.RegisterSurface(IS, null, null, null);
			}
		}
	}
	else
	{
		IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);

		for(udword i=0;i<NbMeshes;i++)
		{
			SetProgress(i);

			const udword Collidable = BinFile.LoadDword();
			(void)Collidable;
			const udword Renderable = BinFile.LoadDword();
			(void)Renderable;

			const udword NbVerts = BinFile.LoadDword();
			const udword NbFaces = BinFile.LoadDword();

			IndexedSurface LocalIS;
			bool Status = LocalIS.Init(NbFaces, NbVerts);
			ASSERT(Status);

			Load::Vertices(LocalIS, NbVerts, BinFile, GlobalBounds, scale);

			Load::Triangles(LocalIS, NbFaces, BinFile);

			IS->Merge(&LocalIS);
		}

/*		if(tessellation)
		{
			for(udword j=0;j<tessellation;j++)
			{
				ButterflyScheme BS;
				IS->Subdivide(BS);
			}
		}*/
		if(tessellation)
			Tessellate(IS, tessellation, ts);

		TotalNbTris = IS->GetNbFaces();
		TotalNbVerts = IS->GetNbVerts();

		test.RegisterSurface(IS, null, null, null);
	}

	ReleaseProgressBar();

	test.SetGlobalBounds(GlobalBounds);

	const udword GrandTotal = sizeof(Point)*TotalNbVerts + sizeof(IndexedTriangle)*TotalNbTris;
	printf("-LoadBIN: loaded %d tris and %d verts, for a total of %d Kb.\n", TotalNbTris, TotalNbVerts, GrandTotal/1024);
	printf("-LoadBIN: min bounds: %f | %f | %f\n", GlobalBounds.GetMin(0), GlobalBounds.GetMin(1), GlobalBounds.GetMin(2));
	printf("-LoadBIN: max bounds: %f | %f | %f\n", GlobalBounds.GetMax(0), GlobalBounds.GetMax(1), GlobalBounds.GetMax(2));
	return true;
}

void LoadMeshesFromFile_(SurfaceManager& test, const char* filename, const Point* scale, bool mergeMeshes, udword tessellation, TessellationScheme ts)
{
	ASSERT(filename);

	const char* File = FindPEELFile(filename);
	if(!File || !LoadBIN(File, test, scale, mergeMeshes, false, tessellation, ts))
		OutputConsoleError(_F("Failed to load '%s'\n", filename));

//	if(!LoadBIN(_F("../build/%s", filename), test, scale, mergeMeshes, tessellation, ts))
//		if(!LoadBIN(_F("./%s", filename), test, scale, mergeMeshes, tessellation, ts))
//			printf(_F("Failed to load '%s'\n", filename));
}

void LoadBinMeshesFromFile(SurfaceManager& test, const char* filename, const BinLoaderSettings& settings)
{
	const char* File = FindPEELFile(filename);
	if(!File || !LoadBIN(File, test, &settings.mScale, settings.mMergeMeshes, settings.mResetPivot, settings.mTessellation, settings.mTS))
		OutputConsoleError(_F("Failed to load '%s'\n", filename));
}
