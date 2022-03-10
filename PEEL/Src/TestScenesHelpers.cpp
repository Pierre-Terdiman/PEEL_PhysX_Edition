///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestScenesHelpers.h"
#include "PintShapeRenderer.h"
#include "MyConvex.h"
#include "PintObjectsManager.h"
#include "TestScenes.h"
#include "Cylinder.h"
#include "GLFontRenderer.h"
#include "SupportFile.h"

///////////////////////////////////////////////////////////////////////////////

const char* GetFile(const char* filename, udword& size)
{
	filename = FindPEELFile(filename);
	if(!filename)
		return null;

	size = _GetFileSize(filename);
	return filename;


/*	const char* Tmp0 = _F("../build/%s", filename);
	size = _GetFileSize(Tmp0);
	if(size)
		return Tmp0;

	const char* Tmp1 = _F("./%s", filename);
	size = _GetFileSize(Tmp1);
	if(size)
		return Tmp1;

	return null;*/
}

///////////////////////////////////////////////////////////////////////////////

void CreateSingleTriangleMesh(SurfaceManager& test, float scale, Triangle* tri, bool reverse_winding)
{
	const udword NbFaces = 1;
	const udword NbVerts = 3;

	IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
	bool Status = IS->Init(NbFaces, NbVerts);
	ASSERT(Status);

	Point* Verts = IS->GetVerts();
	Verts[0] = Point(0.0f, 0.0f, 0.0f);
	Verts[1] = Point(scale, 0.0f, 0.0f);
	Verts[2] = Point(scale, 0.0f, scale);
	const Point Center = (Verts[0]+Verts[1]+Verts[2])/3.0f;
	Verts[0] -= Center;
	Verts[1] -= Center;
	Verts[2] -= Center;
	
	if(tri)
	{
		tri->mVerts[0] =  Verts[0];
		tri->mVerts[1] =  Verts[1];
		tri->mVerts[2] =  Verts[2];
	}

	const IndexedTriangle Indices(0, reverse_winding ? 1 : 2, reverse_winding ? 2 : 1);
	IndexedTriangle* F = IS->GetFaces();
	*F = Indices;

	test.RegisterSurface(IS, null, null, null);
}

///////////////////////////////////////////////////////////////////////////////

bool CreateBoxStack(Pint& pint, const PintCaps& caps, const udword nb_stacks, udword nb_base_boxes, bool use_convexes)
{
	if(!caps.mSupportRigidBodySimulation)
		return false;

	const float BoxExtent = 1.0f;

	PINT_BOX_CREATE BoxDesc;
	PINT_CONVEX_CREATE ConvexDesc;

	if(use_convexes)
	{
		AABB Bounds;
		Bounds.SetCenterExtents(Point(0.0f, 0.0f, 0.0f), Point(BoxExtent, BoxExtent, BoxExtent));
		Point Pts[8];
		Bounds.ComputePoints(Pts);

		ConvexDesc.mNbVerts	= 8;
		ConvexDesc.mVerts	= Pts;
		ConvexDesc.mRenderer = CreateConvexRenderer(8, Pts);
	}
	else
	{
		BoxDesc.mExtents	= Point(BoxExtent, BoxExtent, BoxExtent);
		BoxDesc.mRenderer = CreateBoxRenderer(BoxDesc.mExtents);
	}

	PINT_OBJECT_CREATE ObjectDesc;
	if(use_convexes)
		ObjectDesc.SetShape(&ConvexDesc);
	else
		ObjectDesc.SetShape(&BoxDesc);
	ObjectDesc.mMass	= 1.0f;
//	ObjectDesc.mMass	= 0.5f;

	const udword NbStacks = nb_stacks;
	for(udword j=0;j<NbStacks;j++)
	{
		const float CoeffZ = NbStacks==1 ? 0.0f : float(j) - float(NbStacks)*0.5f;
		udword NbBoxes = nb_base_boxes;
		float BoxPosY = BoxExtent;
		while(NbBoxes)
		{
			for(udword i=0;i<NbBoxes;i++)
			{
		//		const float CoeffX = float(i)/float(NbBoxes-1);
				const float CoeffX = float(i) - float(NbBoxes)*0.5f;

		//		ObjectDesc.mPosition.x	= (CoeffX-0.5f) * BoxExtent * 2.0f;
				ObjectDesc.mPosition.x	= CoeffX * BoxExtent * 2.0f;
				ObjectDesc.mPosition.y	= BoxPosY;
				ObjectDesc.mPosition.z	= CoeffZ * BoxExtent * 4.0f;

				//ObjectDesc.mPosition.z += (UnitRandomFloat()-0.5f)*0.01f;

				const PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
				ASSERT(Handle);
			}

			NbBoxes--;
			BoxPosY += BoxExtent*2.0f;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

void CreateBoxContainer(Pint& pint, float box_height, float box_side, float box_depth)
{
	PINT_BOX_CREATE BoxDesc0(box_side, box_height, box_depth);
	BoxDesc0.mRenderer		= CreateBoxRenderer(BoxDesc0.mExtents);

	PINT_BOX_CREATE BoxDesc1(box_depth, box_height, box_side);
	BoxDesc1.mRenderer		= CreateBoxRenderer(BoxDesc1.mExtents);

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc0);
	ObjectDesc.mMass		= 0.0f;

	ObjectDesc.mPosition	= Point(-box_depth, box_height, 0.0f);
	CreatePintObject(pint, ObjectDesc);
	ObjectDesc.mPosition.x	= box_depth;
	CreatePintObject(pint, ObjectDesc);

	ObjectDesc.SetShape(&BoxDesc1);
	ObjectDesc.mPosition	= Point(0.0f, box_height, -box_depth);
	CreatePintObject(pint, ObjectDesc);
	ObjectDesc.mPosition.z	= box_depth;
	CreatePintObject(pint, ObjectDesc);
}

///////////////////////////////////////////////////////////////////////////////

bool CreateSeaOfStaticBoxes(Pint& pint, float amplitude, udword nb_x, udword nb_y, float altitude, bool add_to_scene, PtrContainer* created_objects)
{
	BasicRandom Rnd(42);
	PINT_BOX_CREATE BoxCreate;

	PINT_OBJECT_CREATE ObjectDesc(&BoxCreate);
	ObjectDesc.mMass		= 0.0f;
	ObjectDesc.mAddToWorld	= add_to_scene;

	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)/float(nb_y-1)) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)/float(nb_x-1)) - 0.5f);

			Point Random;
			UnitRandomPt(Random, Rnd);
			const Point Extents = Random + Point(1.0f, 1.0f, 1.0f);

			BoxCreate.mExtents	= Extents;
			BoxCreate.mRenderer	= CreateBoxRenderer(Extents);

			UnitRandomPt(Random, Rnd);
			const Point Center = Random + Point(CoeffX * amplitude, altitude, CoeffY * amplitude);
			ObjectDesc.mPosition	= Center;

			PintActorHandle h = CreatePintObject(pint, ObjectDesc);
			if(created_objects)
				created_objects->AddPtr(h);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CreateSeaOfStaticSpheres(Pint& pint, float amplitude, udword nb_x, udword nb_y, float altitude)
{
	BasicRandom Rnd(42);
	SRand(42);
	PINT_SPHERE_CREATE SphereCreate;
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)/float(nb_y-1)) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)/float(nb_x-1)) - 0.5f);

			Point Random;
			UnitRandomPt(Random, Rnd);
			const float Radius = 1.0f + UnitRandomFloat();

			SphereCreate.mRadius	= Radius;
			SphereCreate.mRenderer	= CreateSphereRenderer(Radius);

			UnitRandomPt(Random, Rnd);
			const Point Center = Random + Point(CoeffX * amplitude, altitude, CoeffY * amplitude);

			PINT_OBJECT_CREATE ObjectDesc(&SphereCreate);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Center;
			CreatePintObject(pint, ObjectDesc);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CreateSeaOfStaticCapsules(Pint& pint, float amplitude, udword nb_x, udword nb_y, float altitude)
{
	BasicRandom Rnd(42);
	SRand(42);
	PINT_CAPSULE_CREATE CapsuleCreate;
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)/float(nb_y-1)) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)/float(nb_x-1)) - 0.5f);

			Point Random;
			UnitRandomPt(Random, Rnd);
			const float Radius = 1.0f + UnitRandomFloat();
			const float HalfHeight = 1.0f + UnitRandomFloat();

			CapsuleCreate.mRadius		= Radius;
			CapsuleCreate.mHalfHeight	= HalfHeight;
			CapsuleCreate.mRenderer		= CreateCapsuleRenderer(Radius, HalfHeight*2.0f);

			UnitRandomPt(Random, Rnd);
			const Point Center = Random + Point(CoeffX * amplitude, altitude, CoeffY * amplitude);

			PINT_OBJECT_CREATE ObjectDesc(&CapsuleCreate);
			ObjectDesc.mMass		= 0.0f;
			ObjectDesc.mPosition	= Center;
			CreatePintObject(pint, ObjectDesc);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool CreateSeaOfStaticConvexes(Pint& pint, const PintCaps& caps, udword nb_x, udword nb_y, float altitude)
{
	if(!caps.mSupportConvexes)
		return false;

	MyConvex C;
//	udword i=2;	// Small convexes
	udword i=4;	// 'Big' convexes
//	udword i=7;
//	udword i=13;
	C.LoadFile(i);

	PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
	ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

	const float Scale = 3.0f;
	for(udword y=0;y<nb_y;y++)
	{
		for(udword x=0;x<nb_x;x++)
		{
			const float xf = (float(x)-float(nb_x)*0.5f)*Scale;
			const float yf = (float(y)-float(nb_y)*0.5f)*Scale;

			const Point pos = Point(xf, altitude, yf);

			PintActorHandle Handle = CreateStaticObject(pint, &ConvexCreate, pos);
			ASSERT(Handle);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

void GenerateCirclePts(udword nb, Point* pts, float scale, float y)
{
	for(udword i=0;i<nb;i++)
	{
		const float Angle = TWOPI*float(i)/float(nb);
		pts[i].x = cosf(Angle)*scale;
		pts[i].y = y;
		pts[i].z = sinf(Angle)*scale;
	}
}

///////////////////////////////////////////////////////////////////////////////

udword GenerateConvex(Point* verts, udword nbInsideCirclePts, udword nbOutsideCirclePts, float scale0, float scale1, float z)
{
	const udword nbVerts = nbInsideCirclePts + nbOutsideCirclePts;
	GenerateCirclePts(nbInsideCirclePts, verts, scale0, 0.0f);
	GenerateCirclePts(nbOutsideCirclePts, verts+nbInsideCirclePts, scale1, z);
	return nbVerts;
}

///////////////////////////////////////////////////////////////////////////////

PintActorHandle CreateSimpleObject(Pint& pint, const PINT_SHAPE_CREATE* shape, float mass, const Point& pos, const Quat* rot, const Point* linVel, const Point* angVel, const char* name)
{
	PINT_OBJECT_CREATE ObjectDesc(shape);
	ObjectDesc.mName			= name;
	ObjectDesc.mMass			= mass;
	ObjectDesc.mPosition		= pos;
	if(rot)
		ObjectDesc.mRotation	= *rot;
	if(linVel)
		ObjectDesc.mLinearVelocity	= *linVel;
	if(angVel)
		ObjectDesc.mAngularVelocity	= *angVel;
	return CreatePintObject(pint, ObjectDesc);
}

///////////////////////////////////////////////////////////////////////////////

PintActorHandle CreateDynamicObject(Pint& pint, const PINT_SHAPE_CREATE* shape, const Point& pos, const Quat* rot, const Point* linVel, const Point* angVel, const char* name)
{
	return CreateSimpleObject(pint, shape, 1.0f, pos, rot, linVel, angVel, name);
}

///////////////////////////////////////////////////////////////////////////////

PintActorHandle CreateStaticObject(Pint& pint, const PINT_SHAPE_CREATE* shape, const Point& pos, const Quat* rot, const char* name)
{
	return CreateSimpleObject(pint, shape, 0.0f, pos, rot, null, null, name);
}

///////////////////////////////////////////////////////////////////////////////

PintActorHandle CreateDynamicBox(Pint& pint, float size_x, float size_y, float size_z, const Point& pos, const Quat* rot, const PINT_MATERIAL_CREATE* material)
{
	PINT_BOX_CREATE BoxDesc(size_x, size_y, size_z);
	BoxDesc.mMaterial	= material;
	BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

	return CreateDynamicObject(pint, &BoxDesc, pos, rot);
}

///////////////////////////////////////////////////////////////////////////////

bool CreateTestScene_ConvexStack_Generic(Pint& pint, udword NbX, udword NbY, udword NbLayers, udword NbInsideCirclePts, udword NbOutsideCirclePts)
{
	const udword TotalNbVerts = NbInsideCirclePts + NbOutsideCirclePts;
	ASSERT(TotalNbVerts<1024);

	Point Pts[1024];
	udword NbPts = GenerateConvex(Pts, NbInsideCirclePts, NbOutsideCirclePts, 2.0f, 3.0f, 2.0f);
	ASSERT(NbPts==TotalNbVerts);

	PINT_CONVEX_CREATE ConvexCreate(TotalNbVerts, Pts);
	ConvexCreate.mRenderer	= CreateConvexRenderer(TotalNbVerts, Pts);

	for(udword j=0;j<NbLayers;j++)
	{
		const float Scale = 8.0f;
		for(udword y=0;y<NbY;y++)
		{
			for(udword x=0;x<NbX;x++)
			{
				const float xf = (float(x)-float(NbX)*0.5f)*Scale;
				const float yf = (float(y)-float(NbY)*0.5f)*Scale;

				const Point pos = Point(xf, 0.0f + 2.0f * float(j), yf);

				PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, pos);
				ASSERT(Handle);
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

void GenerateConvexPile(Pint& pint, udword nb_x, udword nb_y, udword nb_layers, float amplitude, udword nb_random_pts, float scatter)
{
	ASSERT(nb_random_pts<1024);
	Point Pts[1024];
	BasicRandom Rnd(42);
	for(udword i=0;i<nb_random_pts;i++)
	{
		UnitRandomPt(Pts[i], Rnd);
		Pts[i] *= amplitude;
	}

	PINT_CONVEX_CREATE ConvexCreate(nb_random_pts, Pts);
	ConvexCreate.mRenderer	= CreateConvexRenderer(nb_random_pts, Pts);

	const float Scale = scatter;
	for(udword j=0;j<nb_layers;j++)
	{
		for(udword y=0;y<nb_y;y++)
		{
			for(udword x=0;x<nb_x;x++)
			{
				const float xf = (float(x)-float(nb_x)*0.5f)*Scale;
				const float yf = (float(y)-float(nb_y)*0.5f)*Scale;

				const Point pos = Point(xf, amplitude + (amplitude * 2.0f * float(j)), yf);

				PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, pos);
				ASSERT(Handle);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

bool CreateArrayOfDynamicConvexes(Pint& pint, const PINT_CONVEX_CREATE& convex_create, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_y, const Point* offset, const Point* lin_vel)
{
	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			Point Origin(CoeffX * scale_x, altitude, CoeffY * scale_y);
			if(offset)
				Origin += *offset;

			PintActorHandle Handle = CreateDynamicObject(pint, &convex_create, Origin, null, lin_vel);
			ASSERT(Handle);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool GenerateArrayOfShapes(Pint& pint, const PINT_SHAPE_CREATE* shape, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_z, float mass, PintCollisionGroup group, const Point* offset)
{
	PINT_OBJECT_CREATE ObjectDesc(shape);
	ObjectDesc.mMass			= mass;
	ObjectDesc.mCollisionGroup	= group;

	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			Point Origin(CoeffX * scale_x, altitude, CoeffY * scale_z);
			if(offset)
				Origin += *offset;

			ObjectDesc.mPosition	= Origin;
			CreatePintObject(pint, ObjectDesc);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool GenerateArrayOfSpheres(Pint& pint, float radius, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_z, float mass, PintCollisionGroup group, const Point* offset)
{
	PINT_SPHERE_CREATE SphereDesc(radius);
	SphereDesc.mRenderer	= CreateSphereRenderer(radius);

	return GenerateArrayOfShapes(pint, &SphereDesc, nb_x, nb_y, altitude, scale_x, scale_z, mass, group, offset);
}

///////////////////////////////////////////////////////////////////////////////

bool GenerateArrayOfBoxes(Pint& pint, const Point& extents, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_z, float mass, PintCollisionGroup group, const Point* offset)
{
	PINT_BOX_CREATE BoxDesc(extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(extents);

	return GenerateArrayOfShapes(pint, &BoxDesc, nb_x, nb_y, altitude, scale_x, scale_z, mass, group, offset);
}

///////////////////////////////////////////////////////////////////////////////

bool GenerateArrayOfCapsules(Pint& pint, float radius, float half_height, udword nb_x, udword nb_y, float altitude, float scale_x, float scale_z, float mass, PintCollisionGroup group, const Point* offset)
{
	PINT_CAPSULE_CREATE CapsuleDesc(radius, half_height);
	CapsuleDesc.mRenderer	= CreateCapsuleRenderer(radius, half_height*2.0f);

	return GenerateArrayOfShapes(pint, &CapsuleDesc, nb_x, nb_y, altitude, scale_x, scale_z, mass, group, offset);
}

///////////////////////////////////////////////////////////////////////////////

bool GenerateArrayOfConvexes(Pint& pint, const PintCaps& caps, bool is_static, float scale, udword nb_x, udword nb_y, udword convex_id)
{
	if(!caps.mSupportConvexes)
		return false;

	MyConvex C;
	const udword i=convex_id;
//	udword i=2;	// Small convexes
//	udword i=4;	// 'Big' convexes
//	udword i=7;
//	udword i=13;
	C.LoadFile(i);

	PINT_CONVEX_CREATE ConvexCreate(C.mNbVerts, C.mVerts);
	ConvexCreate.mRenderer	= CreateConvexRenderer(ConvexCreate.mNbVerts, ConvexCreate.mVerts);

	const float AltitudeC = 10.0f;
	const float OneOverNbX = OneOverNb(nb_x);
	const float OneOverNbY = OneOverNb(nb_y);
	for(udword y=0;y<nb_y;y++)
	{
		const float CoeffY = 2.0f * ((float(y)*OneOverNbY) - 0.5f);
		for(udword x=0;x<nb_x;x++)
		{
			const float CoeffX = 2.0f * ((float(x)*OneOverNbX) - 0.5f);

			const Point Origin(CoeffX * scale, AltitudeC, CoeffY * scale);

			if(is_static)
			{
				PintActorHandle Handle = CreateStaticObject(pint, &ConvexCreate, Origin);
				ASSERT(Handle);
			}
			else
			{
				PintActorHandle Handle = CreateDynamicObject(pint, &ConvexCreate, Origin);
				ASSERT(Handle);
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool GenerateArrayOfObjects(Pint& pint, const PintCaps& caps, PintShape type, udword type_data, udword nb_x, udword nb_y, float altitude, float scale, float mass)
{
	if(type==PINT_SHAPE_SPHERE)
	{
		const float Radius = 0.5f;
		return GenerateArrayOfSpheres(pint, Radius, nb_x, nb_y, altitude, scale, scale, mass);
	}
	else if(type==PINT_SHAPE_CAPSULE)
	{
		const float Radius = 0.5f;
		const float HalfHeight = 1.0f;
		return GenerateArrayOfCapsules(pint, Radius, HalfHeight, nb_x, nb_y, altitude, scale, scale, mass);
	}
	else if(type==PINT_SHAPE_BOX)
	{
		const float Radius = 0.5f;
		return GenerateArrayOfBoxes(pint, Point(Radius, Radius, Radius), nb_x, nb_y, altitude, scale, scale, mass);
	}
	else if(type==PINT_SHAPE_CONVEX)
	{
		if(!caps.mSupportConvexes)
			return false;
		return GenerateArrayOfConvexes(pint, caps, mass==0.0f, scale, nb_x, nb_y, type_data);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool GenerateGroundObstacles(Pint& pint, float spacing, float size, bool ramp)
{
	const udword NbRows = 20;
	const float Extent = 10.0f;
	const udword Divider = 10;
	const float MaxExtent = Extent/Divider;

	if(1)
	{
		const Point Origin(MaxExtent-Extent, 0.0f, -10.0f);

		PINT_BOX_CREATE BoxDesc(MaxExtent, size, size);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
		for(udword i=0;i<NbRows;i++)
		{
			for(udword j=0;j<Divider;j++)
			{
				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= Origin + Point(float(j)*MaxExtent*2.0f, 0.0f, float(i)*spacing);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}
		}
	}

	if(1)
	{
		const Point Origin(-10.0f, 0.0f, MaxExtent-Extent);

		PINT_BOX_CREATE BoxDesc(size, size, MaxExtent);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
		for(udword i=0;i<NbRows;i++)
		{
			for(udword j=0;j<Divider;j++)
			{
				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mPosition	= Origin + Point(float(i)*spacing, 0.0f, float(j)*MaxExtent*2.0f);
				ObjectDesc.mMass		= 0.0f;
				CreatePintObject(pint, ObjectDesc);
			}
		}
	}

	if(ramp)
	{
		Matrix3x3 Rot;
		Rot.RotX(25.0f*DEGTORAD);

		PINT_BOX_CREATE BoxDesc(10.0f, 1.0f, 10.0f);
		BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

		PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
		ObjectDesc.mPosition	= Point(5.0f, 0.0f, -100.0f);
		ObjectDesc.mRotation	= Rot;
		ObjectDesc.mMass		= 0.0f;
		CreatePintObject(pint, ObjectDesc);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Setup_PotPourri_Raycasts(Pint& pint, const PintCaps& caps, float mass, udword nb_layers, udword nb_x, udword nb_y)
{
	if(!caps.mSupportRaycasts)
		return false;

	const float BoxHeight = 4.0f;
	const float BoxSide = 1.0f;
	const float BoxDepth = 10.0f;

	const float SphereRadius = 0.5f;
	const float CapsuleRadius = 0.3f;
	const float HalfHeight = 0.5f;
	float yy = CapsuleRadius;
	BasicRandom Rnd(42);

	PINT_SPHERE_CREATE SphereDesc(SphereRadius);
	SphereDesc.mRenderer	= CreateSphereRenderer(SphereRadius);

	PINT_BOX_CREATE BoxDesc(CapsuleRadius, CapsuleRadius, CapsuleRadius);
	BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);

	PINT_CAPSULE_CREATE CapsuleDesc(CapsuleRadius, HalfHeight);
	CapsuleDesc.mRenderer	= CreateCapsuleRenderer(CapsuleRadius, HalfHeight*2.0f);

	for(udword k=0;k<nb_layers;k++)
	{
		for(udword y=0;y<nb_y;y++)
		{
			const float CoeffY = float(nb_y) * 0.5f * ((float(y)/float(nb_y-1)) - 0.5f);
			for(udword x=0;x<nb_x;x++)
			{
				const float CoeffX = float(nb_x) * 0.5f * ((float(x)/float(nb_x-1)) - 0.5f);

				const float RandomX = 4.0f * Rnd.RandomFloat();
				const float RandomY = 4.0f * Rnd.RandomFloat();
				const float RandomZ = 4.0f * Rnd.RandomFloat();

				const udword Index = Rnd.Randomize() % 3;

				PINT_OBJECT_CREATE ObjectDesc;
				if(Index==0)
					ObjectDesc.SetShape(&SphereDesc);
				else if(Index==1)
					ObjectDesc.SetShape(&BoxDesc);
				else if(Index==2)
					ObjectDesc.SetShape(&CapsuleDesc);
				ObjectDesc.mMass		= mass;
				ObjectDesc.mPosition.x	= RandomX + CoeffX * (BoxDepth - SphereRadius - BoxSide*2.0f);
				ObjectDesc.mPosition.y	= RandomY + yy;
				ObjectDesc.mPosition.z	= RandomZ + CoeffY * (BoxDepth - SphereRadius - BoxSide*2.0f);

				UnitRandomQuat(ObjectDesc.mRotation, Rnd);

				CreatePintObject(pint, ObjectDesc);
			}
		}
		yy += HalfHeight*2.0f*4.0f;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool Setup_PotPourri_Raycasts(TestBase& test, udword nb_rays, float max_dist)
{
	BasicRandom Rnd(42);
	for(udword i=0;i<nb_rays;i++)
	{
		Point Origin;
		UnitRandomPt(Origin, Rnd);
		Origin *= 30.0f;
		Origin.y += 30.0f;

		Point Dir;
		UnitRandomPt(Dir, Rnd);
		test.RegisterRaycast(Origin, Dir, max_dist);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef REMOVED
static bool CreateRope(	Pint& pint, PintActorHandle articulation, PintActorHandle parent,
						const Point& p0, const Point& p1, udword nb_capsules, float capsule_radius, float capsule_mass,
						/*bool articulation,*/ bool overlapping_links, bool create_distance_constraints, PintActorHandle* handles=null, float* half_height=null, Quat* qq=null)
{
	if(nb_capsules>MAX_LINKS)
		return false;

//	PintArticHandle Articulation = articulation ? pint.CreateArticulation() : null;

	const Point Dir = (p1 - p0).Normalize();

	float Offset;
	float HalfHeight;
	Point Extents;
	if(!overlapping_links)
	{
		const float TotalLength = p0.Distance(p1);
		// <===TotalLength====>
		// (=*=)(=*=)(=*=)(=*=)
		// We divide TotalLength in N equal parts.
		const float LinkLength = TotalLength/float(nb_capsules);
		//    LinkLength = (Radius + HalfHeight)*2
		//<=> HalfHeight = (LinkLength/2) - Radius
		HalfHeight = LinkLength*0.5f - capsule_radius;
		if(HalfHeight<0.0f)
			return false;
		Offset = capsule_radius + HalfHeight;
		Extents = Dir * (capsule_radius + HalfHeight);
	}
	else
	{
/* TODO
		float TotalLength = p0.Distance(p1);
		// <====TotalLength====>
		// (=*=X=*=X=*=X=*=X=*=)
		// Remove start & end radius from total length:
		TotalLength -= capsule_radius*2.0f;
		if(TotalLength<0.0f)
			return false;
		// <===TotalLength===>
		// =*=X=*=X=*=X=*=X=*=

		//    LinkLength = (Radius + HalfHeight)*2
		//<=> HalfHeight = (LinkLength/2) - Radius
*/
	}

//	const Quat qXtoY = ShortestRotation(Point(1.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f));
	const Quat qToDir = ShortestRotation(Point(0.0f, 1.0f, 0.0f), Dir);
	if(qq)
		*qq = qToDir;

	if(half_height)
		*half_height = HalfHeight;

	PINT_CAPSULE_CREATE CapsuleDesc(capsule_radius, HalfHeight);
//	CapsuleDesc.mLocalRot	= qXtoY;
	CapsuleDesc.mRenderer	= CreateCapsuleRenderer(capsule_radius, HalfHeight*2.0f);

	PINT_OBJECT_CREATE ObjectDesc(&CapsuleDesc);
	ObjectDesc.mMass		= capsule_mass;
	ObjectDesc.mRotation	= qToDir;

	PintActorHandle Handles[MAX_LINKS];
	Point Positions[MAX_LINKS];

	Point Pos = p0 + Offset*Dir;

//	udword GroupBit = 0;	###
	for(udword i=0;i<nb_capsules;i++)
	{
		Positions[i] = Pos;
		{
			ObjectDesc.mPosition		= Pos;
//			ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;	###
			if(articulation)
			{
				PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
				ArticulatedDesc.mParent = i ? Handles[i-1] : parent;
				ArticulatedDesc.mLocalPivot0 = Extents;
				ArticulatedDesc.mLocalPivot1 = -Extents;
				Handles[i] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, articulation);
			}
			else
			{
				Handles[i] = CreatePintObject(pint, ObjectDesc);
				if(i)
				{
					PINT_SPHERICAL_JOINT_CREATE Desc;
					Desc.mObject0		= Handles[i-1];
					Desc.mObject1		= Handles[i];
					Desc.mLocalPivot0	= Extents;
					Desc.mLocalPivot1	= -Extents;
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
			if(handles)
				handles[i] = Handles[i];
		}
		Pos += Extents*2.0f;
	}

//	if(Articulation)
//		pint.AddArticulationToScene(Articulation);

	if(1)
	{
		if(0)
		{
			udword i=nb_capsules-2;
			PINT_DISTANCE_JOINT_CREATE Desc;
			Desc.mObject0		= Handles[0];
			Desc.mObject1		= Handles[i];
			Desc.mDistance		= Positions[i].Distance(Positions[0]);
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}

		if(create_distance_constraints)
		{
			for(udword i=0;i<nb_capsules;i++)
			{
				if(i+2<nb_capsules)
				{
					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0		= Handles[i];
					Desc.mObject1		= Handles[i+2];
					Desc.mDistance		= Positions[i].Distance(Positions[i+2]);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}

		if(0)
		{
			for(udword i=1;i<nb_capsules;i++)
			{
				PINT_DISTANCE_JOINT_CREATE Desc;
				Desc.mObject0		= Handles[0];
				Desc.mObject1		= Handles[i];
				Desc.mDistance		= Positions[0].Distance(Positions[i]);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}
	return true;
}

/*static bool CreateRope(Pint& pint, const Point& p0, const Point& p1, udword nb_capsules, float capsule_radius, float capsule_mass, bool articulation, bool overlapping_links, bool create_distance_constraints, PintActorHandle* handles=null, float* half_height=null)
{
//	PintArticHandle Articulation = articulation ? pint.CreateArticulation() : null;
	return CreateRope(pint, articulation, p0, p1, nb_capsules, capsule_radius, capsule_mass, articulation, overlapping_links, create_distance_constraints, handles, half_height);
//	if(Articulation)
//		pint.AddArticulationToScene(Articulation);
}*/
#endif


static bool CreateCapsuleRope(	Pint& pint, PintArticHandle articulation, PintActorHandle parent,
						float capsule_radius, float half_height, float capsule_mass, float capsule_mass_for_inertia,
						const Point& capsule_dir, const Point& start_pos, udword nb_capsules,
						bool create_distance_constraints, PintActorHandle* handles, const PintCollisionGroup* collision_groups)
{
	if(nb_capsules>MAX_LINKS)
		return false;

/*	printf("capsule_radius: %f\n", capsule_radius);
	printf("half_height: %f\n", half_height);
	printf("capsule_mass: %f\n", capsule_mass);
	printf("capsule_mass_for_inertia: %f\n", capsule_mass_for_inertia);
	printf("capsule_dir: %f %f %f\n", capsule_dir.x, capsule_dir.y, capsule_dir.z);
	printf("start_pos: %f %f %f\n", start_pos.x, start_pos.y, start_pos.z);*/

	const Quat qToDir = ShortestRotation(Point(0.0f, 1.0f, 0.0f), capsule_dir);

	PINT_CAPSULE_CREATE CapsuleDesc(capsule_radius, half_height);
	CapsuleDesc.mRenderer	= CreateCapsuleRenderer(capsule_radius, half_height*2.0f);

	PINT_OBJECT_CREATE ObjectDesc(&CapsuleDesc);
	ObjectDesc.mMass			= capsule_mass;
	ObjectDesc.mMassForInertia	= capsule_mass_for_inertia;
	ObjectDesc.mRotation		= qToDir;

	Point CapsulePos = start_pos + capsule_dir*(capsule_radius+half_height);
	const Point CapsuleOffset(0.0f, capsule_radius+half_height, 0.0f);

#ifdef SETUP_ROPE_COLLISION_GROUPS
	udword GroupBit = 0;
#endif

	PintActorHandle Handles[MAX_LINKS];
	Point Positions[MAX_LINKS];
	for(udword i=0;i<nb_capsules;i++)
	{
#ifdef SETUP_ROPE_COLLISION_GROUPS
		ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
#endif
		if(collision_groups)
			ObjectDesc.mCollisionGroup = collision_groups[i];

		ObjectDesc.mPosition = Positions[i] = CapsulePos;
		CapsulePos += capsule_dir*(capsule_radius+half_height)*2.0f;
		if(articulation)
		{
			PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
			ArticulatedDesc.mParent = i ? Handles[i-1] : parent;
			ArticulatedDesc.mLocalPivot0 = CapsuleOffset;
			ArticulatedDesc.mLocalPivot1 = -CapsuleOffset;
			Handles[i] = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, articulation);
		}
		else
			Handles[i] = CreatePintObject(pint, ObjectDesc);
		if(handles)
			handles[i] = Handles[i];
	}

	if(!articulation)
	{
		if(parent)
		{
			PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(parent, Handles[0], CapsuleOffset, -CapsuleOffset));
			ASSERT(JointHandle);
		}

//		for(udword j=0;j<32;j++)
		{
			for(udword i=1;i<nb_capsules;i++)
			{
				PintJointHandle JointHandle = pint.CreateJoint(PINT_SPHERICAL_JOINT_CREATE(Handles[i-1], Handles[i], CapsuleOffset, -CapsuleOffset));
				ASSERT(JointHandle);
			}
		}
	}

	if(1)
	{
		if(0)
		{
			udword i=nb_capsules-2;
			PINT_DISTANCE_JOINT_CREATE Desc;
			Desc.mObject0			= Handles[0];
			Desc.mObject1			= Handles[i];
			Desc.mLimits.mMaxValue	= Positions[i].Distance(Positions[0]);
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}

		if(create_distance_constraints)
		{
			for(udword i=0;i<nb_capsules;i++)
			{
				if(i+2<nb_capsules)
				{
					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0			= Handles[i];
					Desc.mObject1			= Handles[i+2];
					Desc.mLimits.mMaxValue	= Positions[i].Distance(Positions[i+2]);
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}

		if(0)
		{
			for(udword i=1;i<nb_capsules;i++)
			{
				PINT_DISTANCE_JOINT_CREATE Desc;
				Desc.mObject0			= Handles[0];
				Desc.mObject1			= Handles[i];
				Desc.mLimits.mMaxValue	= Positions[0].Distance(Positions[i]);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
		}
	}
	return true;
}

// I don't know what's going on but this function breaks the crane tests in Release
#pragma optimize( "", off )
bool CreateCapsuleRope2(Pint& pint, float& half_height, PintArticHandle articulation, PintActorHandle parent,
						const Point& p0, const Point& p1, udword nb_capsules,
						float capsule_radius, float capsule_mass, float capsule_mass_for_inertia,
						bool /*overlapping_links*/, bool create_distance_constraints, PintActorHandle* handles, const PintCollisionGroup* collision_groups)
{
	const Point CapsuleDir = (p1 - p0).Normalize();

	float Offset;
	float HalfHeight;
//	Point Extents;
//	if(!overlapping_links)
	{
		const float TotalLength = p0.Distance(p1);
//		printf("TotalLength: %f\n", TotalLength);
		// <===TotalLength====>
		// (=*=)(=*=)(=*=)(=*=)
		// We divide TotalLength in N equal parts.
		const float LinkLength = TotalLength/float(nb_capsules);
//		printf("LinkLength: %f\n", LinkLength);
		//    LinkLength = (Radius + HalfHeight)*2
		//<=> HalfHeight = (LinkLength/2) - Radius
		HalfHeight = LinkLength*0.5f - capsule_radius;
//		printf("HalfHeight: %f\n", HalfHeight);
		if(HalfHeight<0.0f)
			return false;
		Offset = capsule_radius + HalfHeight;
//		printf("Offset: %f\n", Offset);
//		Extents = CapsuleDir * (capsule_radius + HalfHeight);
	}

	half_height = HalfHeight;

	const Point& StartPos = p0;// + Extents;
	return CreateCapsuleRope(pint, articulation, parent, capsule_radius, HalfHeight, capsule_mass, capsule_mass_for_inertia, CapsuleDir, StartPos, nb_capsules, create_distance_constraints, handles, collision_groups);
}
#pragma optimize( "", on )

///////////////////////////////////////////////////////////////////////////////

// This one is unfinished & old. Not properly tuned or optimized or anything, etc.
// TODO: refactor with new one in Bulldozer demo
void CreateCaterpillarTrack_OLD(Pint& pint, CaterpillarTrackObjects& objects, const PINT_MATERIAL_CREATE* material, const Point& pos,
								const CylinderMesh& mCylinder, PintShapeRenderer* cylinder_renderer,
								const CylinderMesh& mCylinder2, PintShapeRenderer* cylinder_renderer2,
								udword multiplier)
{
	//### TODO: we really need an editor for all these params

#ifdef REMOVED
	const PINT_MATERIAL_CREATE FrictionlessMaterial(0.0f, 0.0f, 0.0f);
#endif
	const float Radius = 4.0f;
//	const float Altitude = 2.0f;
//	const float Altitude = pos.y;
	const float LinkMass = 1.0f;
	const float GearMass = 10.0f;
	const float SideMass = 4.0f;

	////

	Curve C;
//	const udword NbCurvePts = 4;
	const udword NbCurvePts = 1024;
//	const udword NbCurvePts = 64;
//	const udword NbCurvePts = 16;
	C.Init(NbCurvePts);
	C.mClosed = true;
	for(udword i=0;i<NbCurvePts;i++)
	{
		const float Coeff = float(i)/float(NbCurvePts);
		const float Angle = Coeff*TWOPI;
		const float x = sinf(Angle) * Radius;
		/*const*/ float y = cosf(Angle) * Radius * 0.5f;
		const float LimitP = 0.5f*Radius;
//		const float LimitP = 0.5f*Radius*0.5f;
		const float LimitN = 0.5f*Radius*0.5f;
		if(y>LimitP)
			y=LimitP;
		if(y<-LimitN)
			y=-LimitN;
		C.SetVertex(x, y/*+Altitude*/, 0.0f, i);
	}
	const float Length = C.ComputeLength();

	////

//	const udword NbLinks = 48;
//	const udword NbLinks = 32*2;
	const udword NbLinks = 32;	//
//	const udword NbLinks = 64;
//	const udword NbLinks = 16;
//	const udword NbLinks = 128;
//	const float D = PI*Radius/float(NbLinks);
	const float D = Length*0.5f/float(NbLinks);
//	const Point Extents(D*0.8f, 0.1f, 1.0f);
	const Point Extents(D, 0.1f, 1.0f);
//	const Point ArticulationPos(0.0f, Altitude + Extents.y, 0.0f);
//	const Point ArticulationPos = pos + Point(0.0f, Extents.y, 0.0f);

	//

	// Main plank
	PINT_BOX_CREATE BoxDesc(Extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(BoxDesc.mExtents);
	BoxDesc.mMaterial	= material;
#ifdef REMOVED
	BoxDesc.mMaterial	= &FrictionlessMaterial;
#endif

		// Smaller inside box to catch the gear
		PINT_BOX_CREATE BoxDesc2(Extents.x*0.2f, 0.2f, Extents.z*0.4f);
//		BoxDesc2.mExtents	= Point(Extents.x*0.2f, 0.15f, Extents.z*0.4f);
//		BoxDesc2.mExtents	= Point(Extents.x*0.2f, 0.2f, Extents.z*0.4f);
		BoxDesc2.mRenderer	= CreateBoxRenderer(BoxDesc2.mExtents);
		BoxDesc2.mMaterial	= material;
#ifdef REMOVED
		BoxDesc2.mMaterial	= &FrictionlessMaterial;		
#endif
		BoxDesc2.mLocalPos	= Point(0.0f, -Extents.y - BoxDesc2.mExtents.y, 0.0f);
		BoxDesc.SetNext(&BoxDesc2);

		// Left & right upper boxes (will touch the ground)
		PINT_BOX_CREATE BoxDesc3(Extents.x, 0.4f, 0.1f);
//		BoxDesc3.mExtents	= Point(Extents.x, 0.15f, 0.1f);
//		BoxDesc3.mExtents	= Point(Extents.x, 0.3f, 0.1f);
//			BoxDesc3.mExtents	= Point(Extents.x, 0.4f, 0.1f);
//BoxDesc3.mExtents	= Point(Extents.x, 0.3f, 0.1f);
		BoxDesc3.mRenderer	= CreateBoxRenderer(BoxDesc3.mExtents);
		BoxDesc3.mMaterial	= material;
		BoxDesc3.mLocalPos	= Point(0.0f, BoxDesc3.mExtents.y - Extents.y, Extents.z + BoxDesc3.mExtents.z);
			BoxDesc3.mLocalPos	= Point(0.0f, 0.0f, Extents.z + BoxDesc3.mExtents.z);
//		BoxDesc3.mLocalPos	= Point(0.0f, -BoxDesc3.mExtents.y, Extents.z + BoxDesc3.mExtents.z);
//BoxDesc3.mLocalPos	= Point(0.0f, - BoxDesc3.mExtents.y + Extents.y, Extents.z + BoxDesc3.mExtents.z);
		BoxDesc2.SetNext(&BoxDesc3);

		PINT_BOX_CREATE BoxDesc4(BoxDesc3.mExtents);
		BoxDesc4.mRenderer	= BoxDesc3.mRenderer;
		BoxDesc4.mMaterial	= material;
		BoxDesc4.mLocalPos	= Point(0.0f, BoxDesc3.mLocalPos.y, -BoxDesc3.mLocalPos.z);
		BoxDesc3.SetNext(&BoxDesc4);

	PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
	ObjectDesc.mMass		= LinkMass;
//	ObjectDesc.mPosition	= ArticulationPos;
	ObjectDesc.mMassForInertia = LinkMass*10.0f;

	// There are multiple issues with articulations:
	// - they tend to emulate what a high solver iteration count would do with regular joints. And it turns out
	// the system explodes easily with regular joints & high counts when things are not properly setup (like not physically possible).
	// That's the case here with some settings that make the track impossible to actually wrap around the gears, etc.
	// So using articulations exposes that and explodes.
	// - then it looks like the default friction doesn't work well with this. It works much better with the other friction modes.......
	// Looks like a bug in PhysX we'll have to investigate & fix.
//	PintArticHandle Articulation = pint.CreateArticulation(PINT_ARTICULATION_CREATE());
	PintArticHandle Articulation = null;

	float Angles[128];
	for(udword i=0;i<NbLinks;i++)
	{
		const float Coeff = float(i)/float(NbLinks);
		Point pts[2];
		Point tmp;
		C.GetPoint(tmp, Coeff, pts);
		const float dx = pts[1].x - pts[0].x;
		const float dy = pts[1].y - pts[0].y;
		Angles[i] = atan2f(dy, dx);
	}

	PintActorHandle Objects[128];
	Point Positions[128];
	PintActorHandle Parent = null;
	for(udword i=0;i<NbLinks;i++)
	{
		const udword ii = (i)%NbLinks;

		const float Coeff = float(ii)/float(NbLinks);

		Point pts[2];
		C.GetPoint(ObjectDesc.mPosition, Coeff, pts);

		ObjectDesc.mPosition += pos;

		float Angle2;
		if(0)
		{
			const udword jj = (i+1)%NbLinks;
			const float NextCoeff = float(jj)/float(NbLinks);
			Point NextPt;
			C.GetPoint(NextPt, NextCoeff);
			const float dx = NextPt.x - ObjectDesc.mPosition.x;
			const float dy = NextPt.y - ObjectDesc.mPosition.y;
			Angle2 = atan2f(dy, dx);
		}
		if(1)
		{
			const float dx = pts[1].x - pts[0].x;
			const float dy = pts[1].y - pts[0].y;
			Angle2 = atan2f(dy, dx);
		}
		if(0)
		{
			const udword j = (i-1)%NbLinks;
			const udword k = (i+1)%NbLinks;
			Angle2 = (Angles[i] + Angles[j] + Angles[k])/3.0f;
		}

		Matrix3x3 Rot;
//		Rot.RotZ(-Angle);
		Rot.RotZ(Angle2);
		ObjectDesc.mRotation = Rot;

		if(Articulation)
		{
			PINT_ARTICULATED_BODY_CREATE ArticulatedDesc;
			if(Parent)
			{
				ArticulatedDesc.mParent = Parent;
				ArticulatedDesc.mLocalPivot0 = Point(D, 0.0f, 0.0f);
				ArticulatedDesc.mLocalPivot1 = Point(-D, 0.0f, 0.0f);
			}
			if(0)
			{
				ArticulatedDesc.mX = Point(0.0f, 0.0f, 1.0f);
				ArticulatedDesc.mEnableTwistLimit = false;
				ArticulatedDesc.mTwistLowerLimit = -0.001f;
				ArticulatedDesc.mTwistUpperLimit = 0.001f;
				ArticulatedDesc.mEnableSwingLimit = true;
				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
			}
			else
			{
				ArticulatedDesc.mEnableTwistLimit = true;
				ArticulatedDesc.mTwistLowerLimit = -0.001f;
				ArticulatedDesc.mTwistUpperLimit = 0.001f;
				ArticulatedDesc.mEnableSwingLimit = true;
//				ArticulatedDesc.mSwingYLimit = 0.001f;
				ArticulatedDesc.mSwingZLimit = 0.001f;
//				ArticulatedDesc.mSwingYLimit = FLT_MAX;
				ArticulatedDesc.mSwingYLimit = PI - 0.001f;
			}
			Parent = pint.CreateArticulatedObject(ObjectDesc, ArticulatedDesc, Articulation);
		}
		else
		{
			Parent = CreatePintObject(pint, ObjectDesc);
		}
		Objects[i] = Parent;
		Positions[i] = ObjectDesc.mPosition;
	}
	if(Articulation)
		pint.AddArticulationToScene(Articulation);

	if(Articulation)
	{
		PINT_HINGE_JOINT_CREATE Desc;
		Desc.mObject0		= Objects[0];
		Desc.mObject1		= Objects[NbLinks-1];
		Desc.mLocalPivot0	= Point(-D, 0.0f, 0.0f);
		Desc.mLocalPivot1	= Point(D, 0.0f, 0.0f);
		Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
		Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
		PintJointHandle JointHandle = pint.CreateJoint(Desc);
		ASSERT(JointHandle);
	}
	else
	{
		//### Brute force to get something working. We should tune & optimize this later.
		const bool UseExtraDistanceConstraints = true;
		for(udword j=0;j<multiplier;j++)
		{
			for(udword i=0;i<NbLinks;i++)
			{
				const udword ii = (i)%NbLinks;
				const udword jj = (i+1)%NbLinks;

				PINT_HINGE_JOINT_CREATE Desc;
				Desc.mObject0		= Objects[ii];
				Desc.mObject1		= Objects[jj];
				Desc.mLocalPivot0	= Point(D, 0.0f, 0.0f);
				Desc.mLocalPivot1	= Point(-D, 0.0f, 0.0f);
				Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
				Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
				PintJointHandle JointHandle = pint.CreateJoint(Desc);
				ASSERT(JointHandle);
			}
			if(UseExtraDistanceConstraints)
			{
				for(udword i=0;i<NbLinks;i++)
				{
					const udword ii = (i)%NbLinks;
					const udword jj = (i+2)%NbLinks;

					PINT_DISTANCE_JOINT_CREATE Desc;
					Desc.mObject0			= Objects[ii];
					Desc.mObject1			= Objects[jj];
	//				Desc.mLimits.mMaxValue	= Positions[ii].Distance(Positions[jj]);
					Desc.mLimits.mMaxValue	= Extents.x*2.0f*2.0f;
					PintJointHandle JointHandle = pint.CreateJoint(Desc);
					ASSERT(JointHandle);
				}
			}
		}
	}

//	return;

	const udword NbGears = 3;
	objects.mNbGears = NbGears;
//	const float x = Radius - mCylinder.mRadius - 0.005f;
//	const float x = Radius - mCylinder2.mRadius - 0.1f;
	const float x = Radius - mCylinder2.mRadius - 0.075f;
//	const float x = Radius - mCylinder2.mRadius - 0.05f;
	PintActorHandle GearObject[3];
	const float xs[3] = { x, -x, 0.0f };
//	const float ys[3] = { 0.0f, 0.0f, 0.0f };
//	const float ys[3] = { 0.0f, 0.0f, 1.0f };
	const float ys[3] = { 0.0f, 0.0f, 0.9f };
//	const float ys[3] = { 0.0f, 0.0f, 1.2f };
//	const float ys[3] = { 0.0f, 0.0f, 1.1f };
//	const float ys[3] = { 0.0f, 0.0f, 1.5f };
	for(udword i=0;i<NbGears;i++)
	{
		PINT_CONVEX_CREATE ConvexCreate(mCylinder.mNbVerts, mCylinder.mVerts);
		ConvexCreate.mRenderer	= cylinder_renderer;
		ConvexCreate.mMaterial	= material;

		PINT_CONVEX_CREATE ConvexCreate2(mCylinder2.mNbVerts, mCylinder2.mVerts);
		ConvexCreate2.mRenderer	= cylinder_renderer2;
		ConvexCreate2.mMaterial	= material;
		ConvexCreate2.mLocalPos	= Point(0.0f, 0.0f, mCylinder.mHalfHeight+mCylinder2.mHalfHeight);
		ConvexCreate.SetNext(&ConvexCreate2);

		PINT_CONVEX_CREATE ConvexCreate3 = ConvexCreate2;
		ConvexCreate3.mLocalPos	= Point(0.0f, 0.0f, -mCylinder.mHalfHeight-mCylinder2.mHalfHeight);
		ConvexCreate2.SetNext(&ConvexCreate3);

		PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
		ObjectDesc.mName		= "GearObject";
		ObjectDesc.mMass		= GearMass;
//		ObjectDesc.mPosition	= Point(xs[i], Altitude, 0.0f);
		ObjectDesc.mPosition	= pos + Point(xs[i], ys[i], 0.0f);
//		ObjectDesc.mAngularVelocity	= Point(0.0f, 0.0f, -5.0f);
		GearObject[i] = CreatePintObject(pint, ObjectDesc);
		ASSERT(GearObject[i]);
		objects.mGears[i] = GearObject[i];
	}
	{
		const float Gap = 0.1f;
		PINT_BOX_CREATE SideBoxDesc(x, 0.3f, 0.1f);
		SideBoxDesc.mRenderer	= CreateBoxRenderer(SideBoxDesc.mExtents);
		const float z = mCylinder.mHalfHeight + mCylinder2.mHalfHeight*2.0f + SideBoxDesc.mExtents.z + Gap;
		SideBoxDesc.mLocalPos	= Point(0.0f, 0.0f, z);

		PINT_BOX_CREATE SideBoxDesc2 = SideBoxDesc;
		SideBoxDesc2.mLocalPos	= Point(0.0f, 0.0f, -z);
		SideBoxDesc.SetNext(&SideBoxDesc2);

		PINT_OBJECT_CREATE ObjectDesc(&SideBoxDesc);
		ObjectDesc.mMass		= SideMass;
//		ObjectDesc.mPosition	= Point(0.0f, Altitude, 0.0f);
		ObjectDesc.mPosition	= pos;
		PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle);
		objects.mChassis = Handle;

		for(udword i=0;i<NbGears;i++)
		{
			PINT_HINGE_JOINT_CREATE Desc;
			Desc.mObject0		= GearObject[i];
			Desc.mObject1		= Handle;
			Desc.mLocalPivot0	= Point(0.0f, 0.0f, 0.0f);
			Desc.mLocalPivot1	= Point(xs[i], ys[i], 0.0f);
			Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
			Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
			PintJointHandle JointHandle = pint.CreateJoint(Desc);
			ASSERT(JointHandle);
		}
	}

	if(0)
	{
		PINT_CONVEX_CREATE ConvexCreate(mCylinder.mNbVerts, mCylinder.mVerts);
		ConvexCreate.mRenderer	= cylinder_renderer;
		ConvexCreate.mLocalPos	= Point(x, 0.0f, 0.0f);

		PINT_CONVEX_CREATE ConvexCreate2(mCylinder.mNbVerts, mCylinder.mVerts);
		ConvexCreate2.mRenderer	= cylinder_renderer;
		ConvexCreate2.mLocalPos	= Point(-x, 0.0f, 0.0f);
		ConvexCreate.SetNext(&ConvexCreate2);

		PINT_OBJECT_CREATE ObjectDesc(&ConvexCreate);
		ObjectDesc.mMass		= 20.0f;
//		ObjectDesc.mPosition	= Point(0.0f, Altitude, 0.0f);
		ObjectDesc.mPosition	= pos;
		PintActorHandle Handle = CreatePintObject(pint, ObjectDesc);
		ASSERT(Handle);
	}
}

///////////////////////////////////////////////////////////////////////////////

bool CreateBridges(Pint& pint, const PintCaps& caps, BRIDGES_CREATE& create)
{
	if(!caps.mSupportHingeJoints || !caps.mSupportRigidBodySimulation)
		return false;

	const bool UseFiltering = true;
	if(UseFiltering)
	{
		if(!caps.mSupportCollisionGroups)
			return false;

		const PintDisabledGroups DG(1, 2);
		pint.SetDisabledGroups(1, &DG);
	}

	const bool TestStatic = false;
	const float InertiaCoeff = create.mInertiaCoeff;
	const udword Multiplier = create.mMultiplier;
	const Point& Extents = create.mPlankExtents;
	const udword NbBoxes = create.mNbPlanks;
	const udword NbRows = create.mNbBridges;
	const Point Dir(1.0f, 0.0f, 0.0f);
//	const Point PosOffset = Dir*(Extents.x + 0.1f);
	const Point PosOffset = Dir*Extents.x;

	PINT_BOX_CREATE BoxDesc(Extents);
	BoxDesc.mRenderer	= CreateBoxRenderer(Extents);
//	BoxDesc.mExtents	= Point(2.0f, 0.1f, 2.0f);

	const Point PlatformExtents(Point(Extents.x*10.0f, Extents.y, Extents.z));
	PINT_BOX_CREATE BoxDesc2(PlatformExtents);
	const Point RampExtents(Point(20.0f, Extents.y, Extents.z));
	PINT_BOX_CREATE BoxDesc3(RampExtents);
	if(create.mIncludeSupport)
	{
		BoxDesc2.mRenderer	= CreateBoxRenderer(PlatformExtents);
		BoxDesc3.mRenderer	= CreateBoxRenderer(RampExtents);
	}

	float* Chainette = (float*)StackAlloc(sizeof(float)*(NbBoxes+1));
	float* Dy = (float*)StackAlloc(sizeof(float)*(NbBoxes+1));
	const float Coeff = 20.0f;
	for(udword i=0;i<NbBoxes+1;i++)
	{
		const float f0 = float(i) - float(NbBoxes/2);
		const float f1 = float(i+1) - float(NbBoxes/2);

		Chainette[i] = Coeff*coshf(f0/Coeff);

		Dy[i] = Coeff*(coshf(f1/Coeff) - coshf(f0/Coeff));
	}

	PintActorHandle* Handles = (PintActorHandle*)StackAlloc(sizeof(PintActorHandle)*NbBoxes);
	for(udword j=0;j<NbRows;j++)
	{
		const float RowCoeff = NbRows!=1 ? float(j)/float(NbRows-1) : 0.0f;

		Point Pos = create.mOrigin;
		Pos.z += float(j)*Extents.z*4.0f;

		Point RightPos(0.0f, 0.0f, 0.0f);
		udword GroupBit = 0;
		for(udword i=0;i<NbBoxes;i++)
		{
			Matrix3x3 Rot;
			const float Alpha = atanf(Dy[i]);
			const float CurveCoeff = 0.5f + RowCoeff * 2.5f;
			Rot.RotZ(Alpha*CurveCoeff);
			//Rot.RotZ(0.0f);

			Point R;
			Rot.GetRow(0, R);

			float Mass = create.mPlankMass;
			if(i==0 || i==NbBoxes-1)
			{
				Mass = 0.0f;

				if(create.mIncludeSupport)
				{
					const float DispX = PlatformExtents.x + Extents.x;// - fabsf(cos(Alpha*CurveCoeff)*Extents.y);
					const float DispY = fabsf(sinf(Alpha*CurveCoeff)*Extents.x);
					PINT_OBJECT_CREATE ObjectDesc(&BoxDesc2);
					ObjectDesc.mMass		= Mass;
					ObjectDesc.mPosition	= Pos + Point(i==0 ? -DispX : DispX, DispY, 0.0f);
					CreatePintObject(pint, ObjectDesc);

					const float RampAngle = 15.0f*DEGTORAD;
					const float Sign = i==0 ? -1.0f:1.0f;
					ObjectDesc.SetShape(&BoxDesc3);
					ObjectDesc.mMass		= Mass;
					ObjectDesc.mPosition	+= Point(Sign*(PlatformExtents.x+RampExtents.x), -fabsf(sinf(RampAngle)*RampExtents.x), 0.0f);
					Matrix3x3 RotRamp;
					RotRamp.RotZ(-RampAngle*Sign);
					ObjectDesc.mRotation = RotRamp;
					CreatePintObject(pint, ObjectDesc);
				}
			}

			{
				PINT_OBJECT_CREATE ObjectDesc(&BoxDesc);
				ObjectDesc.mMass			= Mass;
				ObjectDesc.mMassForInertia	= Mass*InertiaCoeff;
					//ObjectDesc.mKinematic		= true;
				if(TestStatic)
					ObjectDesc.mMass			= 0.0f;
				ObjectDesc.mPosition		= Pos;
//				ObjectDesc.mPosition.y		= (Chainette[i] + Chainette[i+1])*0.5f;
//				ObjectDesc.mPosition.y		+= Chainette[i] - 10.0f;

				ObjectDesc.mRotation = Rot;

				const Point LeftPos = Pos - R*Extents.x;
				if(i)
				{
					Pos += RightPos - LeftPos;
					//ObjectDesc.mPosition += RightPos - LeftPos;
					ObjectDesc.mPosition		= Pos;
				}

				ObjectDesc.mCollisionGroup	= 1 + GroupBit;	GroupBit = 1 - GroupBit;
				Handles[i] = CreatePintObject(pint, ObjectDesc);
			}

			RightPos = Pos + R*Extents.x;

//			Pos += PosOffset*2.0f;
			Pos += R*2.0f*Extents.x;
		}

		if(!TestStatic)
		{
			for(udword j=0;j<Multiplier;j++)
			{
				for(udword i=0;i<NbBoxes-1;i++)
				{
					if(1)
					{
						PINT_HINGE_JOINT_CREATE Desc;
						Desc.mObject0		= Handles[i];
						Desc.mObject1		= Handles[i+1];
						Desc.mLocalPivot0	= PosOffset;
						Desc.mLocalPivot1	= -PosOffset;
						Desc.mLocalAxis0	= Point(0.0f, 0.0f, 1.0f);
						Desc.mLocalAxis1	= Point(0.0f, 0.0f, 1.0f);
						PintJointHandle JointHandle = pint.CreateJoint(Desc);
						ASSERT(JointHandle);
					}
					else
					{
						PINT_FIXED_JOINT_CREATE Desc;
						Desc.mObject0		= Handles[i];
						Desc.mObject1		= Handles[i+1];
						Desc.mLocalPivot0	= PosOffset;
						Desc.mLocalPivot1	= -PosOffset;
						PintJointHandle JointHandle = pint.CreateJoint(Desc);
						ASSERT(JointHandle);
					}
				}
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ClampAngularVelocity(Pint& pint, PintActorHandle wheel_parent, PintActorHandle wheel, float max_velocity)
{
	const Point ParentAngularVelocity = pint.GetAngularVelocity(wheel_parent);

	const Point AngularVelocity = pint.GetAngularVelocity(wheel);
//	printf("Angular velocity %d: %f %f %f\n", i, AngularVelocity.x, AngularVelocity.y, AngularVelocity.z);

	Point RelativeVelocity = AngularVelocity - ParentAngularVelocity;

//	RelativeVelocity.x = RelativeVelocity.y = 0.0f;
	bool Clamped = false;
	if(RelativeVelocity.z>max_velocity)
	{
		RelativeVelocity.z = max_velocity;
		Clamped = true;
	}
	else if(RelativeVelocity.z<-max_velocity)
	{
		RelativeVelocity.z = -max_velocity;
		Clamped = true;
	}
	if(Clamped)
		pint.SetAngularVelocity(wheel, RelativeVelocity + ParentAngularVelocity);
	return Clamped;
}

///////////////////////////////////////////////////////////////////////////////

float PrintAngularVelocity(Pint& pint, GLFontRenderer& renderer, PintActorHandle handle, float y, float text_scale)
{
	const Point AngVel = pint.GetAngularVelocity(handle);
	renderer.print(0.0f, y, text_scale, _F("Angular velocity: %.5f | %.5f | %.5f\n", AngVel.x, AngVel.y, AngVel.z));
	return y - text_scale;
}

///////////////////////////////////////////////////////////////////////////////

bool GetHingeTwistAngle(Pint& pint, PintJointHandle handle, float& angle)
{
	Pint_Joint* API = pint.GetJointAPI();
	if(!API)
		return false;

	PintHingeDynamicData Data;
	if(!API->GetHingeDynamicData(handle, Data))
		return false;
	angle = Data.mTwistAngle;
	return true;
}

///////////////////////////////////////////////////////////////////////////////


