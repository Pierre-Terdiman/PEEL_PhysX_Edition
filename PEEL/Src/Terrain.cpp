///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Terrain.h"
#include "PintGLMeshShapeRenderer.h"
#include "PEEL_Threads.h"

///////////////////////////////////////////////////////////////////////////////

CustomLayer::CustomLayer() : mHeights(null)
{
}

CustomLayer::~CustomLayer()
{
	ICE_FREE(mHeights);
}

bool CustomLayer::Init(const CUSTOMLAYERCREATE& create)
{
	const udword DataSize = create.mWidth*create.mHeight*sizeof(float);
	mHeights = reinterpret_cast<float*>(ICE_ALLOC(DataSize));
	if(create.mHeights)
		CopyMemory(mHeights, create.mHeights, DataSize);
	else
		ZeroMemory(mHeights, DataSize);

	return HeightLayer::Init(create);
}

bool CustomLayer::Update(float* field, udword width, udword height) const
{
	if(!mHeights)
		return false;

	const float* h = mHeights;
	udword NbToGo = width*height;
	while(NbToGo--)
	{
		*field += *h++;
		field++;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

TerrainData::TerrainData() :
	mRenderer	(null),
#ifdef SUPPORT_TERRAIN_TEXTURE
	mTexture	(null),
#endif
	mNbX		(0),
	mNbY		(0)
{
}

TerrainData::~TerrainData()
{
	Release();
}

bool TerrainData::Init(udword nb_x, udword nb_y, float scale_x, float scale_y)
{
	mNbX = nb_x;
	mNbY = nb_y;

	mSurface.MakePlane(nb_x, nb_y);
	mSurface.Flip();
	mSurface.Scale(Point(scale_x, 1.0f, scale_y));

//	Point Center;
//	mSurface.VertexCloud::ComputeLocalGeomCenter(Center);

	if(0)
	{
		SinusLayer* SL = ICE_NEW(SinusLayer);

		SINUSLAYERCREATE Create;
		Create.mWidth		= nb_x;
		Create.mHeight		= nb_y;
		Create.mAmplitude	= 2.0f;
		//
		Create.mXFreq		= 0.1f;
		Create.mYFreq		= 0.1f;
		Create.mPhaseX		= 0.0f;
		Create.mPhaseY		= 0.0f;

		SL->Init(Create);

		AddLayer(SL);

		UpdateVertices(null);
	}

#ifdef SUPPORT_TERRAIN_TEXTURE
		{
			const udword NbFaces = mSurface.GetNbFaces();
			const udword NbVerts = mSurface.GetNbVerts();

			IndexedSurface* UVSurface = mSurface.AddExtraSurface(SURFACE_UVS);
			UVSurface->Init(NbFaces, NbVerts);

			// Copy same topology
			IndexedTriangle* TFaces = (IndexedTriangle*)UVSurface->GetFaces();
			CopyMemory(TFaces, mSurface.GetFaces(), NbFaces*sizeof(IndexedTriangle));

			Point* uvs = (Point*)UVSurface->GetVerts();
			AABB Bounds;
			Bounds.SetEmpty();
			for(udword i=0;i<NbVerts;i++)
			{
				uvs[i] = mSurface.GetVerts()[i];
				Bounds.Extend(uvs[i]);
			}
			Point Offset, Scale;
			Bounds.GetMin(Offset);
			Bounds.GetExtents(Scale);
			Scale *= 2.0f;
			Scale = 1.0f / Scale;
			for(udword i=0;i<NbVerts;i++)
			{
				uvs[i].x = (uvs[i].x + Offset.x) * Scale.x;
				uvs[i].y = (uvs[i].z + Offset.z) * Scale.z;
				uvs[i].z = 0.0f;
			}
		}

		//PintShapeRenderer* Renderer = CreateMeshRenderer(mSurface);

	mRenderer = ICE_NEW(PintGLMeshShapeRendererEx)(mSurface, DL_MESH_DEFORMABLE/*|DL_MESH_USE_DIRECT_DATA*/);
#else
	// ### TODO: CRCs computed for nothing here
	mRenderer = ICE_NEW(PintGLMeshShapeRenderer)(PintSurfaceInterface(mSurface.GetSurfaceInterface()), 0, null);
#endif
	return true;
}

void TerrainData::Release()
{
	const udword NbLayers = mLayers.GetNbEntries();
	for(udword i=0;i<NbLayers;i++)
	{
		HeightLayer* Layer = reinterpret_cast<HeightLayer*>(mLayers.GetEntry(i));
		DELETESINGLE(Layer);
	}
	mLayers.Empty();

	DELETESINGLE(mRenderer);
	mSurface.Clean();

	mNbX = mNbY = 0;
}

void TerrainData::AddLayer(const HeightLayer* layer)
{
	mLayers.AddPtr(layer);
}

bool TerrainData::RemoveLayer(const HeightLayer* layer)
{
	return mLayers.DeleteKeepingOrder(const_cast<HeightLayer*>(layer));
}

// ### Heightfield API to revisit in ICE
void TerrainData::Evaluate(Heightfield& hf) const
{
	hf.Clear(0.0f);

	//HF.GetHeights();
	const udword NbLayers = mLayers.GetNbEntries();
	for(udword i=0;i<NbLayers;i++)
	{
		const HeightLayer* Layer = reinterpret_cast<const HeightLayer*>(mLayers.GetEntry(i));
		Layer->Update(hf);
	}
}

void TerrainData::UpdateVertices(Point* v)
{
	Heightfield HF(mNbX, mNbY);
	Evaluate(HF);

	const float* NewHeights = HF.GetHeights();

	Point* V = mSurface.GetVerts();
	const udword NbVerts = mSurface.GetNbVerts();
	ASSERT(NbVerts==mNbX*mNbY);
	for(udword i=0;i<NbVerts;i++)
	{
		if(v)
			v[i].y = *NewHeights;

		V[i].y = *NewHeights++;
	}
}

void TerrainData::UpdateRenderer()
{
	mRenderer->UpdateVerts(mSurface.GetVerts(), null);
}

///////////////////////////////////////////////////////////////////////////////

#include "PsHashMap.h"

namespace px = physx;
namespace ps = physx::shdfnd;

typedef ps::HashMap<PintActorHandle, TerrainData*>	TerrainCache;

static TerrainCache* gTerrainCache = null;

bool RegisterTerrainData(PintActorHandle owner, TerrainData* terrain_data)
{
	if(!gTerrainCache)
		gTerrainCache = new TerrainCache;

	return gTerrainCache->insert(owner, terrain_data);
}

bool UnregisterTerrainData(PintActorHandle owner)
{
	return gTerrainCache ? gTerrainCache->erase(owner) : false;
}

TerrainData* GetTerrainData(PintActorHandle owner)
{
	if(!gTerrainCache)
		return null;

	const TerrainCache::Entry* e = gTerrainCache->find(owner);
	if(!e)
		return null;

	return e->second;
}

void UnregisterAllTerrainData()
{
	DELETESINGLE(gTerrainCache);
}

///////////////////////////////////////////////////////////////////////////////

static PtrContainer gTerrainData;

TerrainData* CreateTerrainData()
{
	TerrainData* TD = ICE_NEW(TerrainData);
	gTerrainData.AddPtr(TD);
	return TD;
}

void ReleaseAllTerrainData()
{
	const udword Nb = gTerrainData.GetNbEntries();
	for(udword i=0;i<Nb;i++)
	{
		TerrainData* TD = reinterpret_cast<TerrainData*>(gTerrainData.GetEntry(i));
		DELETESINGLE(TD);
	}
	gTerrainData.Empty();
}

///////////////////////////////////////////////////////////////////////////////

/*static*/ void CreateRegularGridTopology(IndexedTriangle* tris, udword nb_faces, udword nbu, udword nbv)
{
	udword k = 0;
	for(udword j=0;j<nbv-1;j++)
	{
		for(udword i=0;i<nbu-1;i++)
		{
			// Create first triangle
			//    0___1
			//    |\  |
			//    |*\ |
			//    |__\|
			// nbu  nbu+1
			tris[k].mRef[0] = i   + j*nbu;
			tris[k].mRef[1] = i+1 + (j+1)*nbu;
			tris[k].mRef[2] = i   + (j+1)*nbu;
			k++;

			// Create second triangle
			//    0___1
			//    |\  |
			//    | \*|
			//    |__\|
			// nbu  nbu+1
			tris[k].mRef[0] = i   + j*nbu;
			tris[k].mRef[1] = i+1 + j*nbu;
			tris[k].mRef[2] = i+1 + (j+1)*nbu;
			k++;
		}
	}
	ASSERT(k==nb_faces);
}

/*static*/ void CreateAlternativeGridTopology(IndexedTriangle* tris, udword nb_faces, udword nbu, udword nbv)
{
	udword k = 0;
	for(udword j=0;j<nbv-1;j++)
	{
		for(udword i=0;i<nbu-1;i++)
		{
			if((i^j)&1)
			{
				// Create first triangle
				//    0___1
				//    |  /|
				//    |*/ |
				//    |/_ |
				// nbu  nbu+1
				tris[k].mRef[0] = i   + j*nbu;
				tris[k].mRef[1] = i+1 + j*nbu;
				tris[k].mRef[2] = i   + (j+1)*nbu;
				k++;

				// Create second triangle
				//    0___1
				//    |  /|
				//    | /*|
				//    |/__|
				// nbu  nbu+1
				tris[k].mRef[0] = i+1 + j*nbu;
				tris[k].mRef[1] = i+1 + (j+1)*nbu;
				tris[k].mRef[2] = i   + (j+1)*nbu;
			}
			else
			{
				// Create first triangle
				//    0___1
				//    |\  |
				//    |*\ |
				//    |__\|
				// nbu  nbu+1
				tris[k].mRef[0] = i   + j*nbu;
				tris[k].mRef[1] = i+1 + (j+1)*nbu;
				tris[k].mRef[2] = i   + (j+1)*nbu;
				k++;

				// Create second triangle
				//    0___1
				//    |\  |
				//    | \*|
				//    |__\|
				// nbu  nbu+1
				tris[k].mRef[0] = i   + j*nbu;
				tris[k].mRef[1] = i+1 + j*nbu;
				tris[k].mRef[2] = i+1 + (j+1)*nbu;
			}
			k++;
		}
	}
	ASSERT(k==nb_faces);
}

//static
void CreateAlternativeGridTopologyLOD(IndexedTriangle* tris, udword nb_faces, udword nbu, udword nbv, udword nbu2, udword nbv2, udword lod)
{
	udword k = 0;
	for(udword jj=0;jj<nbv2-1;jj++)
	{
		for(udword ii=0;ii<nbu2-1;ii++)
		{
			const udword i = ii*lod;
			const udword j = jj*lod;

			const udword ni = i+lod;
			const udword nj = j+lod;

			if((ii^jj)&1)
			{
				// Create first triangle
				//    0___1
				//    |  /|
				//    |*/ |
				//    |/_ |
				// nbu  nbu+1
				tris[k].mRef[0] = i  + j*nbu;
				tris[k].mRef[1] = ni + j*nbu;
				tris[k].mRef[2] = i  + nj*nbu;
				k++;

				// Create second triangle
				//    0___1
				//    |  /|
				//    | /*|
				//    |/__|
				// nbu  nbu+1
				tris[k].mRef[0] = ni + j*nbu;
				tris[k].mRef[1] = ni + nj*nbu;
				tris[k].mRef[2] = i  + nj*nbu;
			}
			else
			{
				// Create first triangle
				//    0___1
				//    |\  |
				//    |*\ |
				//    |__\|
				// nbu  nbu+1
				tris[k].mRef[0] = i  + j*nbu;
				tris[k].mRef[1] = ni + nj*nbu;
				tris[k].mRef[2] = i  + nj*nbu;
				k++;

				// Create second triangle
				//    0___1
				//    |\  |
				//    | \*|
				//    |__\|
				// nbu  nbu+1
				tris[k].mRef[0] = i  + j*nbu;
				tris[k].mRef[1] = ni + j*nbu;
				tris[k].mRef[2] = ni + nj*nbu;
			}
			k++;
		}
	}
	ASSERT(k==nb_faces);
}

bool MakePlane(IndexedSurface& surface, udword nbu, udword nbv, Quad* fit, bool alternative_topology)
{
	SPY_ZONE("MakePlane")

	if(nbu<=1 || nbv<=1)
		return false;

	//alternative_topology = false;

	if(alternative_topology)
	{
		// In this version we want to create a topology like this:
		//    0___1___2___3___4_ _
		//    |\  |  /|\  |  /|
		//    | \ | / | \ | / |
		//    |__\|/__|__\|/__|_ _
		// nbu  nbu+1 ...
		//    |  /|\  |  /|\  |
		//    | / | \ | / | \ |
		//    |/__|__\|/__|__\|_ _
		//
		// This only works if nbu and nbv are odd.

		if(!(nbu&1) || !(nbv&1))
			return false;
	}
	else
	{
		// A "plane" is actually a regular grid, which can be used in many applications:
		// - it can be a height-field
		// - it can be a cloth patch
		// - it can be anything built from a nbu*nbv set of vertices
		//
		// The order in which vertices are created is normalized, so that further routines can assume the underlying plane topology
		// is well-known. (For example it is useful to create springs in cloth simulation).
		//
		// nbu*nbv vertices are created, in the following order:
		//
		//    0___1___2___3___4___5_ _ _
		//    |\  |\  |\  |\  |\  |
		//    | \ | \ | \ | \ | \ |
		//    |__\|__\|__\|__\|__\| _ _ _
		// nbu  nbu+1 ...
		//
		// The grid is centered around the origin. Triangles are created as above. Number of triangles is (nbu-1)*(nbv-1)*2.
		//
		// Extremal vertices are:
		// Upper-left:	0
		// Upper-right:	nbu-1
		// Lower-left:	nbu*(nbv-1)
		// Lower-right:	nbu*(nbv-1) + (nbu-1)
	}

	const udword NbVerts = nbu * nbv;
	const udword NbFaces = (nbu-1)*(nbv-1)*2;
//		const udword lod = 4;
//		const udword nbu2 = (nbu/4)+1;
//		const udword nbv2 = (nbv/4)+1;
//		const udword NbFaces = (nbu2-1)*(nbv2-1)*2;

	if(!surface.Init(NbFaces, NbVerts, null, null, false))
		return false;

	Point* Verts = surface.GetVerts();

	// Create vertices
	if(!fit)
	{
		float size = 10.0f;
		size = 4.0f;
		size = 40.0f;

		// Create a plane/grid centered around the origin, in the XZ plane.
		for(udword y=0;y<nbv;y++)
		{
			for(udword x=0;x<nbu;x++)
			{
				Verts[x+y*nbu] = Point(float(x)-(float(nbu-1)*0.5f), 0.0f, float(y)-(float(nbv-1)*0.5f));
				Verts[x+y*nbu] *= size;
			}
		}
	}
	else
	{
		// Here we create the "plane" within the provided quad. The convention is:
		// - the upper-left vertex maps the first vertex of the quad.
		// - the upper-right vertex maps the second vertex of the quad.
		// - the lower-left vertex maps the third vertex of the quad.
		// - the lower-right vertex maps the fourth vertex of the quad.
		//
		// The input quad is supposed planar...
		//
		for(udword y=0;y<nbv;y++)
		{
			const float ty = float(y)/float(nbv-1);
			for(udword x=0;x<nbu;x++)
			{
				const float tx = float(x)/float(nbu-1);

				const Point p0 = (1.0f-tx)*fit->mVerts[0] + tx*fit->mVerts[1];
				const Point p1 = (1.0f-tx)*fit->mVerts[2] + tx*fit->mVerts[3];

				Verts[x+y*nbu] = (1.0f-ty)*p0 + ty*p1;
			}
		}
	}

	IndexedTriangle* Tris = surface.GetFaces();

	// Create faces
	if(alternative_topology)
	{
		CreateAlternativeGridTopology(Tris, NbFaces, nbu, nbv);
//		CreateAlternativeGridTopologyLOD(Tris, NbFaces, nbu, nbv, nbu2, nbv2, lod);
	}
	else
	{
		CreateRegularGridTopology(Tris, NbFaces, nbu, nbv);
	}

	// Delete now obsolete data structures
	surface.InvalidateTopology();
	surface.InvalidateGeometry();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#define USE_LOCAL_PERLIN

#ifdef USE_LOCAL_PERLIN
static inline_ void PerlinSetup(float val, int& b0, int& b1, float& r0, float& r1)
{
	const float t = val + PERLIN_N;
	const int it = int(t);
	b0 = it & PERLIN_BM;
	b1 = (b0+1) & PERLIN_BM;
	r0 = t - it;
	r1 = r0 - 1.0f;
}

#define	PERLIN_s_curve(t)		( t*t * (3.0f - 2.0f*t) )
#define	PERLIN_lerp(t, a, b)	( a + t*(b - a) )
static inline_ float PERLIN_at3(const float* q, float rx, float ry, float rz)
{
	return rx*q[0] + ry*q[1] + rz*q[2];
}

static inline_ float PerlinCore(const float* q0, const float* q1, float rx0, float rx1, float ry, float rz, float t)
{
	const float u = PERLIN_at3(q0, rx0, ry, rz);
	const float v = PERLIN_at3(q1, rx1, ry, rz);
	return PERLIN_lerp(t, u, v);
}

static inline_ float PERLIN_at2(const float* q, float rx, float rz)
{
	return rx*q[0] + rz*q[2];
}

static inline_ float PerlinCore2D(const float* q0, const float* q1, float rx0, float rx1, float rz, float t)
{
	const float u = PERLIN_at2(q0, rx0, rz);
	const float v = PERLIN_at2(q1, rx1, rz);
	return PERLIN_lerp(t, u, v);
}

	class LocalPerlin : public PerlinNoise
	{
		public:

		float		Compute2(const Point& vector)	const
		{
			if(1)
			{
				// For 2D with vector.y = 0.0f

				int bx0, bx1;	float rx0, rx1;	PerlinSetup(vector.x, bx0, bx1, rx0, rx1);
				int bz0, bz1;	float rz0, rz1;	PerlinSetup(vector.z, bz0, bz1, rz0, rz1);

				const udword i = mP[bx0];
				const udword j = mP[bx1];

				const int b00 = mP[i + 16];
				const int b10 = mP[j + 16];
				const int b01 = mP[i + 17];
				const int b11 = mP[j + 17];

				const float t  = PERLIN_s_curve(rx0);
				const float sz = PERLIN_s_curve(rz0);

				const float a0 = PerlinCore2D(mG3[b00 + bz0], mG3[b10 + bz0], rx0, rx1, rz0, t);
				const float a1 = PerlinCore2D(mG3[b00 + bz1], mG3[b10 + bz1], rx0, rx1, rz1, t);

				return PERLIN_lerp(sz, a0, a1);
			}
			else
			{
				int bx0, bx1;	float rx0, rx1;	PerlinSetup(vector.x, bx0, bx1, rx0, rx1);
				int by0, by1;	float ry0, ry1;	PerlinSetup(vector.y, by0, by1, ry0, ry1);
				int bz0, bz1;	float rz0, rz1;	PerlinSetup(vector.z, bz0, bz1, rz0, rz1);

				const udword i = mP[bx0];
				const udword j = mP[bx1];

				const int b00 = mP[i + by0];
				const int b10 = mP[j + by0];
				const int b01 = mP[i + by1];
				const int b11 = mP[j + by1];

				const float t  = PERLIN_s_curve(rx0);
				const float sy = PERLIN_s_curve(ry0);
				const float sz = PERLIN_s_curve(rz0);

				const float a0 = PerlinCore(mG3[b00 + bz0], mG3[b10 + bz0], rx0, rx1, ry0, rz0, t);
				const float b0 = PerlinCore(mG3[b01 + bz0], mG3[b11 + bz0], rx0, rx1, ry1, rz0, t);
				const float c = PERLIN_lerp(sy, a0, b0);

				const float a1 = PerlinCore(mG3[b00 + bz1], mG3[b10 + bz1], rx0, rx1, ry0, rz1, t);
				const float b1 = PerlinCore(mG3[b01 + bz1], mG3[b11 + bz1], rx0, rx1, ry1, rz1, t);
				const float d = PERLIN_lerp(sy, a1, b1);

				return PERLIN_lerp(sz, c, d);
			}
		}
	};

	class FBM : public FractalBrownianMotion
	{
		public:

		float	Compute2(const Point& vector) const
		{
			const LocalPerlin& perlin = static_cast<const LocalPerlin&>(mNoise);
			const float* ExponentArray = mExponentArray;
			const float Lacunarity = mLacunarity;

			// Initialize vars to proper values
			float Value = 0.0f;

			// Inner loop of spectral construction
			Point v = vector;
			const udword Limit = udword(mOctaves);
			udword i;
			for(i=0;i<Limit;i++)
			{
				Value += perlin.Compute2(v) * ExponentArray[i];
				v *= Lacunarity;
			}

			const float Remainder = mOctaves - Limit;
			if(Remainder)
			{
				// add in ``octaves''  remainder
				// ``i'' and spatial freq. are preset in loop above
				Value += Remainder * perlin.Compute2(v) * ExponentArray[i];
			}
			return Value;
		}
	};
#endif

#ifdef USE_LOCAL_PERLIN
static float Evaluate(const FBM& fbm, const float ScaleY, const float Scale, float x, float z)
#else
static float Evaluate(const FractalBrownianMotion& fbm, const float ScaleY, const float Scale, float x, float z)
#endif
{
#ifdef USE_LOCAL_PERLIN
	float val = ScaleY * fbm.Compute2(Point(x*Scale, 0.0f, z*Scale));
#else
	float val = ScaleY * fbm.Compute(Point(x*Scale, 0.0f, z*Scale));
#endif

	//if(method)
	{
#ifdef USE_LOCAL_PERLIN
	const float Value1 = fbm.Compute2(Point(x*Scale*0.5f, 0.0f, z*Scale*0.5f));
#else
	const float Value1 = fbm.Compute(Point(x*Scale*0.5f, 0.0f, z*Scale*0.5f));
#endif
	val *= Value1 * 2.0f;
	}
	return val;
}

float Terrain_Evaluate(const FractalBrownianMotion& fbm, const float ScaleY, const float Scale, float x, float z)
{
#ifdef USE_LOCAL_PERLIN
	return Evaluate(static_cast<const FBM&>(fbm), ScaleY, Scale, x, z);
#else
	return Evaluate(fbm, ScaleY, Scale, x, z);
#endif
}

namespace
{
	struct Params
	{
		Point*	V;
		Point*	N;
		udword	nbu;
		udword	nbv;
		udword	StartOffset;
		udword	i0;
		udword	i1;
		Point	min;
		Point	max;
		const FractalBrownianMotion* _fbm;
		Point	offset;
	};
}

static int CreateProceduralTerrain_Thread(void* user_data)
{
	SPY_ZONE("CreateProceduralTerrainThread")

	const Params* params = reinterpret_cast<const Params*>(user_data);

	Point* V = params->V;
	Point* N = params->N;
	const Point& min = params->min;
	const Point& max = params->max;
	const Point& offset = params->offset;

#ifdef USE_LOCAL_PERLIN
	const FBM& fbm = static_cast<const FBM&>(*params->_fbm);
#else
	const FractalBrownianMotion& fbm = *params->_fbm;
#endif
	const udword NbU = params->nbu;
	const udword NbV = params->nbv;

	udword Offset = params->StartOffset;
	//const float ScaleY = 1.5f;
	const float ScaleY = 16.0f;
	const float Scale = 0.02f;
	//const float ScaleY = 32.0f;
	//const float Scale = 0.005f;

	const float OneOverNbU = 1.0f / float(NbU-1);
	const float OneOverNbV = 1.0f / float(NbV-1);
	const udword i0 = params->i0;
	const udword i1 = params->i1;
	for(udword i=i0;i<i1;i++)
	{
		const float CoeffX = float(i) * OneOverNbU;
		const float x = min.x + CoeffX * (max.x - min.x);

		for(udword j=0;j<NbV;j++)
		{
			const float CoeffZ = float(j) * OneOverNbV;
			const float z = min.z + CoeffZ * (max.z - min.z);

/*					float val = ScaleY * fbm.Compute(Point(x*Scale, 0.0f, z*Scale));
			//float val = fbm.Compute(Point(z*Scale, 0.0f, x*Scale));

//if(method)
{
const float Value1 = fbm.Compute(Point(x*Scale*0.5f, 0.0f, z*Scale*0.5f));
val *= Value1 * 2.0f;
}*/

			float val = Evaluate(fbm, ScaleY, Scale, x, z);

/*vec3 calcNormal( in vec3 pos, float t )
{
vec2  eps = vec2( 0.001*t, 0.0 );
return normalize( vec3( terrainH(pos.xz-eps.xy) - terrainH(pos.xz+eps.xy),
					2.0*eps.x,
					terrainH(pos.xz-eps.yx) - terrainH(pos.xz+eps.yx) ) );
}*/
			//const float epsilon = 0.001f;
			const float epsilon = 0.1f;
			//const float epsilon = 1.0f;
			//const float epsilon = 10.0f;
			Point n;
			n.x = Evaluate(fbm, ScaleY, Scale, x-epsilon, z) - Evaluate(fbm, ScaleY, Scale, x+epsilon, z);
			n.y = epsilon*2.0f;
			n.z = Evaluate(fbm, ScaleY, Scale, x, z-epsilon) - Evaluate(fbm, ScaleY, Scale, x, z+epsilon);
			n.Normalize();
			//Point n(0.0f, 1.0f, 0.0f);

			V[Offset].y = val;
			//V[Offset].y = CoeffX;
			//V[Offset].y = CoeffZ;
			//V[Offset].y = fbm.Compute(Point(CoeffX, 0.0f, 0.0f));

if(1)
{
	V[Offset].x -= offset.x;
	V[Offset].z -= offset.z;
}

			//RD->mVertices[Offset] = V[Offset];
			//RD->mVertexNormals[Offset] = n;
N[Offset] = n;

			Offset++;
		}
	}

	return 0;
}

static inline_ void SetupParams(Params& params, Point* V, Point* N, udword nbu, udword nbv, const Point& min, const Point& max, const FractalBrownianMotion& _fbm, const Point& offset)
{
	params.V			= V;
	params.N			= N;
	params.nbu			= nbu;
	params.nbv			= nbv;
	params.min			= min;
	params.max			= max;
	params._fbm			= &_fbm;
	params.offset		= offset;
}

void CreateProceduralTerrain(Point* V, Point* N, udword nbu, udword nbv, const Point& min, const Point& max, const FractalBrownianMotion& _fbm, const Point& offset)
{
	SPY_ZONE("CreateProceduralTerrain")

	if(1)
	{
		Params P;
		SetupParams(P, V, N, nbu, nbv, min, max, _fbm, offset);
		P.StartOffset	= 0;
		P.i0			= 0;
		P.i1			= nbu;
		CreateProceduralTerrain_Thread(&P);
	}

	if(0)
	{
		if(0)
		{
			const udword NbVerts = nbu*nbv;
			for(udword i=0;i<NbVerts;i++)
			{
				V[i].x -= offset.x;
				V[i].z -= offset.z;
				V[i].y = 0.0f;
				N[i] = Point(0.0f, 1.0f, 0.0f);
			}
		}

		Params P;
		SetupParams(P, V, N, nbu, nbv, min, max, _fbm, offset);
		P.StartOffset	= 0;
		P.i0			= 0;
		P.i1			= nbu/2;
		//CreateProceduralTerrain_Thread(&P);

		Params P2;
		SetupParams(P2, V, N, nbu, nbv, min, max, _fbm, offset);
		P2.StartOffset	= (nbu/2)*nbv;
		P2.i0			= nbu/2;
		P2.i1			= nbu;
		//CreateProceduralTerrain_Thread(&P);

		PEEL_AddThreadWork(0, CreateProceduralTerrain_Thread, &P);
		PEEL_AddThreadWork(1, CreateProceduralTerrain_Thread, &P2);
		PEEL_StartThreadWork(2);
		PEEL_EndThreadWork(2);
	}

#ifdef REMOVED
#ifdef USE_LOCAL_PERLIN
	const FBM& fbm = static_cast<const FBM&>(_fbm);
#else
	const FractalBrownianMotion& fbm = _fbm;
#endif
	const udword NbU = nbu;
	const udword NbV = nbv;

	udword Offset = 0;
	//const float ScaleY = 1.5f;
	const float ScaleY = 16.0f;
	const float Scale = 0.02f;
	//const float ScaleY = 32.0f;
	//const float Scale = 0.005f;

	for(udword i=0;i<NbU;i++)
	{
		const float CoeffX = float(i)/float(NbU-1);
		const float x = min.x + CoeffX * (max.x - min.x);

		for(udword j=0;j<NbV;j++)
		{
			const float CoeffZ = float(j)/float(NbV-1);
			const float z = min.z + CoeffZ * (max.z - min.z);

/*					float val = ScaleY * fbm.Compute(Point(x*Scale, 0.0f, z*Scale));
			//float val = fbm.Compute(Point(z*Scale, 0.0f, x*Scale));

//if(method)
{
const float Value1 = fbm.Compute(Point(x*Scale*0.5f, 0.0f, z*Scale*0.5f));
val *= Value1 * 2.0f;
}*/

			float val = Evaluate(fbm, ScaleY, Scale, x, z);

/*vec3 calcNormal( in vec3 pos, float t )
{
vec2  eps = vec2( 0.001*t, 0.0 );
return normalize( vec3( terrainH(pos.xz-eps.xy) - terrainH(pos.xz+eps.xy),
					2.0*eps.x,
					terrainH(pos.xz-eps.yx) - terrainH(pos.xz+eps.yx) ) );
}*/
			//const float epsilon = 0.001f;
			const float epsilon = 0.1f;
			//const float epsilon = 1.0f;
			//const float epsilon = 10.0f;
			Point n;
			n.x = Evaluate(fbm, ScaleY, Scale, x-epsilon, z) - Evaluate(fbm, ScaleY, Scale, x+epsilon, z);
			n.y = epsilon*2.0f;
			n.z = Evaluate(fbm, ScaleY, Scale, x, z-epsilon) - Evaluate(fbm, ScaleY, Scale, x, z+epsilon);
			n.Normalize();
			//Point n(0.0f, 1.0f, 0.0f);

			V[Offset].y = val;
			//V[Offset].y = CoeffX;
			//V[Offset].y = CoeffZ;
			//V[Offset].y = fbm.Compute(Point(CoeffX, 0.0f, 0.0f));

V[Offset].x -= offset.x;
V[Offset].z -= offset.z;

			//RD->mVertices[Offset] = V[Offset];
			//RD->mVertexNormals[Offset] = n;
N[Offset] = n;

			Offset++;
		}
	}
#endif
}

