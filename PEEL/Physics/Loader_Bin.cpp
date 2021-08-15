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

static const bool gUseMeshCleaner = false;

struct Indices
{
	udword mRef[3];

	inline_ bool operator!=(const Indices&v) const	{ return mRef[0] != v.mRef[0] || mRef[1] != v.mRef[1] || mRef[2] != v.mRef[2]; }
};

static inline_ udword getHashValue(const Point& v)
{
	const udword* h = (const udword*)(&v.x);
	const udword f = (h[0]+h[1]*11-(h[2]*17)) & 0x7fffffff;	// avoid problems with +-0
	return (f>>22)^(f>>12)^(f);
}

static inline_ udword getHashValue(const Indices& v)
{
//	const udword* h = v.mRef;
//	const udword f = (h[0]+h[1]*11-(h[2]*17)) & 0x7fffffff;	// avoid problems with +-0
//	return (f>>22)^(f>>12)^(f);

	udword a = v.mRef[0];
	udword b = v.mRef[1];
	udword c = v.mRef[2];
	a=a-b;  a=a-c;  a=a^(c >> 13);
	b=b-c;  b=b-a;  b=b^(a << 8); 
	c=c-a;  c=c-b;  c=c^(b >> 13);
	a=a-b;  a=a-c;  a=a^(c >> 12);
	b=b-c;  b=b-a;  b=b^(a << 16);
	c=c-a;  c=c-b;  c=c^(b >> 5);
	a=a-b;  a=a-c;  a=a^(c >> 3);
	b=b-c;  b=b-a;  b=b^(a << 10);
	c=c-a;  c=c-b;  c=c^(b >> 15);
	return c;
}

class MeshCleaner
{
	public:
		MeshCleaner(udword nbVerts, const Point* verts, udword nbTris, const udword* indices);
		~MeshCleaner();

		udword	mNbVerts;
		udword	mNbTris;
		Point*	mVerts;
		udword*	mIndices;
		udword*	mRemap;
};

MeshCleaner::MeshCleaner(udword nbVerts, const Point* srcVerts, udword nbTris, const udword* srcIndices)
{
	Point* cleanVerts = (Point*)ICE_ALLOC(sizeof(Point)*nbVerts);
	ASSERT(cleanVerts);
//	memcpy(cleanVerts, srcVerts, nbVerts*sizeof(Point));

	udword* indices = (udword*)ICE_ALLOC(sizeof(udword)*nbTris*3);

	udword* remapTriangles = (udword*)ICE_ALLOC(sizeof(udword)*nbTris);

	const float meshWeldTolerance = 0.01f;

	udword* vertexIndices = null;
	if(meshWeldTolerance!=0.0f)
	{
		vertexIndices = (udword*)ICE_ALLOC(sizeof(udword)*nbVerts);
		const float weldTolerance = 1.0f / meshWeldTolerance;
		// snap to grid
		for(udword i=0; i<nbVerts; i++)
		{
			vertexIndices[i] = i;
			cleanVerts[i] = Point(	floorf(srcVerts[i].x*weldTolerance + 0.5f),
									floorf(srcVerts[i].y*weldTolerance + 0.5f),
									floorf(srcVerts[i].z*weldTolerance + 0.5f));
		}
	}
	else
	{
		memcpy(cleanVerts, srcVerts, nbVerts*sizeof(Point));
	}

	const udword maxNbElems = TMax(nbTris, nbVerts);
	const udword hashSize = NextPowerOfTwo(maxNbElems);
	const udword hashMask = hashSize-1;
	udword* hashTable = (udword*)ICE_ALLOC(sizeof(udword)*(hashSize + maxNbElems));
	ASSERT(hashTable);
	memset(hashTable, 0xff, hashSize * sizeof(udword));
	udword* const next = hashTable + hashSize;

	udword* remapVerts = (udword*)ICE_ALLOC(sizeof(udword)*nbVerts);
	memset(remapVerts, 0xff, nbVerts * sizeof(udword));

	for(udword i=0;i<nbTris*3;i++)
	{
		const udword vref = srcIndices[i];
		if(vref<nbVerts)
			remapVerts[vref] = 0;
	}

	udword nbCleanedVerts = 0;
	for(udword i=0;i<nbVerts;i++)
	{
		if(remapVerts[i]==0xffffffff)
			continue;

		const Point& v = cleanVerts[i];
		const udword hashValue = getHashValue(v) & hashMask;
		udword offset = hashTable[hashValue];

		while(offset!=0xffffffff && cleanVerts[offset]!=v)
			offset = next[offset];

		if(offset==0xffffffff)
		{
			remapVerts[i] = nbCleanedVerts;
			cleanVerts[nbCleanedVerts] = v;
			if(vertexIndices)
				vertexIndices[nbCleanedVerts] = i;
			next[nbCleanedVerts] = hashTable[hashValue];
			hashTable[hashValue] = nbCleanedVerts++;
		}
		else remapVerts[i] = offset;
	}

	udword nbCleanedTris = 0;
	for(udword i=0;i<nbTris;i++)
	{
		udword vref0 = *srcIndices++;
		udword vref1 = *srcIndices++;
		udword vref2 = *srcIndices++;
		if(vref0>=nbVerts || vref1>=nbVerts || vref2>=nbVerts)
			continue;

		// PT: you can still get zero-area faces when the 3 vertices are perfectly aligned
		const Point& p0 = srcVerts[vref0];
		const Point& p1 = srcVerts[vref1];
		const Point& p2 = srcVerts[vref2];
		const float area2 = ((p0 - p1)^(p0 - p2)).SquareMagnitude();
		if(area2==0.0f)
			continue;

		vref0 = remapVerts[vref0];
		vref1 = remapVerts[vref1];
		vref2 = remapVerts[vref2];
		if(vref0==vref1 || vref1==vref2 || vref2==vref0)
			continue;

		indices[nbCleanedTris*3+0] = vref0;
		indices[nbCleanedTris*3+1] = vref1;
		indices[nbCleanedTris*3+2] = vref2;
		nbCleanedTris++;
	}
	ICE_FREE(remapVerts);

	udword nbToGo = nbCleanedTris;
	nbCleanedTris = 0;
	memset(hashTable, 0xff, hashSize * sizeof(udword));

	Indices* I = reinterpret_cast<Indices*>(indices);
	bool idtRemap = true;
	for(udword i=0;i<nbToGo;i++)
	{
		const Indices& v = I[i];
		const udword hashValue = getHashValue(v) & hashMask;
		udword offset = hashTable[hashValue];

		while(offset!=0xffffffff && I[offset]!=v)
			offset = next[offset];

		if(offset==0xffffffff)
		{
			remapTriangles[nbCleanedTris] = i;
			if(i!=nbCleanedTris)
				idtRemap = false;
			I[nbCleanedTris] = v;
			next[nbCleanedTris] = hashTable[hashValue];
			hashTable[hashValue] = nbCleanedTris++;
		}
	}
	ICE_FREE(hashTable);

	if(vertexIndices)
	{
		for(udword i=0;i<nbCleanedVerts;i++)
			cleanVerts[i] = srcVerts[vertexIndices[i]];
		ICE_FREE(vertexIndices);
	}
	mNbVerts	= nbCleanedVerts;
	mNbTris		= nbCleanedTris;
	mVerts		= cleanVerts;
	mIndices	= indices;
	if(idtRemap)
	{
		ICE_FREE(remapTriangles);
		mRemap	= NULL;
	}
	else
	{
		mRemap	= remapTriangles;
	}
}

MeshCleaner::~MeshCleaner()
{
	ICE_FREE(mRemap);
	ICE_FREE(mIndices);
	ICE_FREE(mVerts);
}


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
