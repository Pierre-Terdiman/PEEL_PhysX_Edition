///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "USDExport.h"
#include "PINT_Editor.h"
#include "PintDLConvexShapeRenderer.h"
#include "PEEL.h"
#include "Devil.h"
#include "SupportFile.h"

#include "PxTransform.h"
#include "PxMat33.h"
using namespace physx;
#include "ICE_To_PX.h"

#include "Zcb2_RenderData.h"

//#define HACK_HACK_HACK

static const bool gUseInstancing = false;
static const bool gUseActorNames = true;
static const bool gExportJoints = true;
static const bool gDiscardImplicitPoses = false;

/*
Updated TODO:
- derive scene params (TGS etc) from test
- revisit actor/shape export to make smaller hierarchies
- check units of angular velocity export
- cleanup old USD files on drive
- make meshes work again
	- actually they work, it's just the new mesh2 stuff that is entirely ignored
- proper export of hinge drives
	- damping values etc don't match, resulting velocity very different
- revisit gear joints
- revisit rack joints
- revisit d6 joints => rot limits not exprted
- check remaining schema end of file
- make it work again for lego scenes with textures
- make it work without recompiling for point instancers
- revisit duplication of shape data


TODO:
- Lego stuff:
- transparent sphere, hide objects touching it, to see the inside of the cars
- make an anim with bricks moving one by one to their final spot?

- collisionConvex is a child of the shape but collisionBox isn't?

- joints:
	- don't export values if default
	- finish that stuff
- kinematics!
- vehicles!
- revisit indentation & line breaks in convex & mesh exports
- do we need to export "doubleSided" both for rigid body & shapes?
- use "# " for comments
- do we need to replicate the attributes like size/radius in the collision shape?
=> looks like yes
=> but then do we need that with convex verts as well?????????????????????
- export bounding box? doesn't seem to be properly recomputed for spheres??
- try Y=up now that it's the default in Graphene
- indentation
- separate physics & graphics (not always the same shapes)
- instances & sharing
- can we compute normals in the shader instead of duplicating verts?
*/

static const bool gAnimXP = false;
static const bool gExportPhysics = true;
static const bool gFlipYZ = false;
static udword gIndent = 0;

#ifdef REMOVED_OLD
static const char* gCollisionShapeAPI = "CollisionShapeAPI";
#else
//static const char* gCollisionShapeAPI = "CollisionAPI";
static const char* gCollisionShapeAPI = "PhysicsCollisionAPI";
#endif

static const char* gPhysXCollisionAPI = "PhysxCollisionAPI";

static void StoreQuotedString(CustomArray& ca, const char* str)
{
	ca.Store(char('"'));
	ca.StoreASCII(str);
	ca.Store(char('"'));
}

	class USDExporter
	{
		public:
					USDExporter();
					~USDExporter();

		void		ExportHeader();
		void		Indent();
		void		StartDef(const char* type, const char* name);
		void		StartPrependDef(const char* type, const char* name, const char* api0, const char* api1=null, const char* api2=null, const char* api3=null, const char* api4=null, const char* api5=null, const char* api6=null);
		void		EndDef();
		void		ExportColor(const Point& color);
		void		StoreXFormOrder(bool pos, bool rot, bool scale);
		void		ExportPose(const Point* pos, const Quat* rot, const Point* scale, bool flip);
		void		ExportConvexHull(const ConvexShapeData* data);
		void		ExportMesh(const MeshShapeData* data);
		void		ExportMesh(const RenderDataChunk* parent, const MeshRenderDataChunk* data);
		void		ExportCommonPhysicsShapeParams();
		void		ExportShape(const ShapeData* shape, udword index);
		void		ExportActor(const EditorPlugin& editor, const ActorData* actor, const char* name, const Point* color, bool is_instanced, bool export_lin_vel, bool export_ang_vel);
		void		ExportSceneData(const SceneData& scene_data);
		void		ExportMaterials(const EditorPlugin& editor, const char* path);
		void		ExportRenderSources(const PtrContainer& shapes, udword& material_id, const char* path);
		void		ExportRenderSource(RenderDataChunk* RenderSource, udword& material_id, const char* path);
		bool		ExportRenderMeshes(const ShapeData* data);

		inline_	void	FormatCheck(udword& nb, udword limit, udword i)
		{
		//	if(nb==128 && i!=limit)
			if(nb==64 && i!=limit)
			{
				nb = 0;
				mArray.StoreASCII(", \n");
				Indent();
			}
		}

		CustomArray	mArray;
	};

USDExporter::USDExporter()
{
}

USDExporter::~USDExporter()
{
}

void USDExporter::Indent()
{
	for(udword i=0;i<gIndent;i++)
//		mArray.StoreASCII("    ");	// Hardcoded to 4 spaces
//		mArray.Store("\t");
		mArray.Store(char(ASCII_TAB));
}

void USDExporter::ExportHeader()
{
	gIndent = 0;
	mArray.StoreASCII("#usda 1.0\n\n");
	mArray.StoreASCII("# This file has been exported from PEEL.\n\n");
	mArray.StoreASCII("(\n");
	gIndent++;
		Indent();	mArray.StoreASCII("defaultPrim = ");
		StoreQuotedString(mArray, "World");
		mArray.StoreASCII("\n");

		Indent();	mArray.StoreASCII("metersPerUnit = 1\n");
		Indent();	mArray.StoreASCII("startTimeCode = 0\n");
		Indent();	mArray.StoreASCII("endTimeCode = 100\n");
		Indent();	mArray.StoreASCII("timeCodesPerSecond = 60\n");

		Indent();	mArray.StoreASCII("upAxis = ");
		if(gFlipYZ)
			StoreQuotedString(mArray, "Z");
		else
			StoreQuotedString(mArray, "Y");
		mArray.StoreASCII("\n)\n");
	gIndent--;
}

void USDExporter::StartDef(const char* type, const char* name)
{
	mArray.StoreASCII("\n");

	Indent();

	// ### don't use _F here
//	mArray.StoreASCII(_F("def %s ", type));
	mArray.StoreASCII("def ");
	mArray.StoreASCII(type);
	mArray.StoreASCII(" ");

	StoreQuotedString(mArray, name);
	mArray.StoreASCII("\n");
	Indent();
	mArray.StoreASCII("{\n");
	gIndent++;
}

void USDExporter::StartPrependDef(const char* type, const char* name, const char* api0, const char* api1, const char* api2, const char* api3, const char* api4, const char* api5, const char* api6)
{
	if(!gExportPhysics)
	{
		StartDef(type, name);
		return;
	}

	mArray.StoreASCII("\n");

	Indent();

	mArray.StoreASCII("def ");
	mArray.StoreASCII(type);
	mArray.StoreASCII(" ");

	StoreQuotedString(mArray, name);
	mArray.StoreASCII(" (\n");
	gIndent++;

	Indent();
	mArray.StoreASCII("prepend apiSchemas = [");
	StoreQuotedString(mArray, api0);
	if(api1)
	{
		mArray.StoreASCII(", ");
		StoreQuotedString(mArray, api1);
		if(api2)
		{
			mArray.StoreASCII(", ");
			StoreQuotedString(mArray, api2);
			if(api3)
			{
				mArray.StoreASCII(", ");
				StoreQuotedString(mArray, api3);
				if(api4)
				{
					mArray.StoreASCII(", ");
					StoreQuotedString(mArray, api4);
					if(api5)
					{
						mArray.StoreASCII(", ");
						StoreQuotedString(mArray, api5);
						if(api6)
						{
							mArray.StoreASCII(", ");
							StoreQuotedString(mArray, api6);
						}
					}
				}
			}
		}
	}
	mArray.StoreASCII("]\n");
	gIndent--;
	Indent();
	mArray.StoreASCII(")\n");
	Indent();
	mArray.StoreASCII("{\n");
	gIndent++;
}

void USDExporter::EndDef()
{
	gIndent--;
	Indent();
	mArray.StoreASCII("}\n");
}

void USDExporter::ExportColor(const Point& color)
{
	Indent();

	// Gamma
	const float Gamma = 2.2f;
	const float R = powf(color.x, Gamma);
	const float G = powf(color.y, Gamma);
	const float B = powf(color.z, Gamma);

	mArray.StoreASCII(_F("color3f[] primvars:displayColor = [(%f, %f, %f)]\n", R, G, B));
}

void USDExporter::StoreXFormOrder(bool pos, bool rot, bool scale)
{
	if(!pos && !rot && !scale)
		return;

	Indent();
	mArray.StoreASCII("uniform token[] xformOpOrder = [");

	bool needComa = false;
	if(pos)
	{
		StoreQuotedString(mArray, "xformOp:translate");
		needComa = true;
	}

	if(rot)
	{
		if(needComa)
			mArray.StoreASCII(", ");
		StoreQuotedString(mArray, "xformOp:orient");
		needComa = true;
	}

	if(scale)
	{
		if(needComa)
			mArray.StoreASCII(", ");
		StoreQuotedString(mArray, "xformOp:scale");
	}

	mArray.StoreASCII("]\n");
}

static Point GetConvertedPos(const Point& pos, bool flip)
{
	if(gFlipYZ && flip)
	{
		Matrix3x3 Transfo;
		Transfo.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

		return pos * Transfo;
	}
	else
	{
		return pos;
	}
}

static Quat GetConvertedRot(const Quat& rot, bool flip)
{
	if(gFlipYZ && flip)
	{
		Matrix3x3 Transfo;
		Transfo.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

		Matrix3x3 M0 = rot;
		Matrix3x3 M = M0 * Transfo;

		return M;
	}
	else
	{
		return rot;
	}
}

void USDExporter::ExportPose(const Point* pos, const Quat* rot, const Point* scale, bool flip)
{
	Matrix3x3 Transfo;
	Transfo.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

//	Matrix4x4 W;
//	W.Identity();
//	if(rot)
//		W = *rot;
//	if(pos)
//		W.SetTrans(*pos);
//
//	Matrix4x4 T44 = Transfo;
////	Matrix4x4 Combo = W * T44;
	//Matrix4x4 Combo = T44 * W;

	if(pos)
	{
/*		if(pos->IsZero())
		{
			pos = null;
		}
		else*/
		{
			Point p;

			//### hardcoded Y/Z flip
			if(gFlipYZ && flip)
			{
	//			ca.StoreASCII(_F("double3 xformOp:translate = (%f, %f, %f)\n", pos->x, pos->z, pos->y));

				p = (*pos) * Transfo;
//				p = Combo.GetTrans();

//				ca.StoreASCII(_F("double3 xformOp:translate = (%f, %f, %f)\n", p.x, p.y, p.z));
			}
			else
			{
				p = *pos;
//				ca.StoreASCII(_F("double3 xformOp:translate = (%f, %f, %f)\n", pos->x, pos->y, pos->z));
			}

			if(gDiscardImplicitPoses && p.IsZero())
				pos = null;
			else
			{
				Indent();
//				mArray.StoreASCII(_F("double3 xformOp:translate = (%f, %f, %f)\n", p.x, p.y, p.z));
				mArray.StoreASCII(_F("float3 xformOp:translate = (%f, %f, %f)\n", p.x, p.y, p.z));
			}
		}
	}

	if(rot)
	{
/*		if(rot->IsIdentity())
		{
			rot = null;
		}
		else*/
		{
			Quat q;

			if(gFlipYZ && flip)
			{
				Matrix3x3 M0 = *rot;
				Matrix3x3 M = M0 * Transfo;
//				Matrix3x3 M = Transfo * M0;

				q = M;
//					Quat q = Combo;
//				ca.StoreASCII(_F("quatf xformOp:orient = (%f, %f, %f, %f)\n", q.w, q.p.x, q.p.y, q.p.z));

				Point p0(1.0f, 2.0f, 3.0f);
				Point p0b = p0 * M;
				Matrix3x3 M2 = q;
				Point p1(1.0f, 2.0f, 3.0f);


	#ifdef REMOVED
				//### hardcoded Y/Z flip - this one to check!
		/*		Matrix3x3 Rot = *rot;
				const udword index0 = 1;
				const udword index1 = 2;
				const Point r1 = Rot[index0];
				const Point r2 = Rot[index1];
				Rot[index0] = r2;
				Rot[index1] = r1;
				const Quat q = Rot;*/

				Matrix3x3 M0 = *rot;

				Matrix3x3 Rot;
		//		Rot.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
				Rot.FromTo(Point(0.0f, 0.0f, 1.0f), Point(0.0f, 1.0f, 0.0f));

				Matrix3x3 M1 = M0 * Rot;
		//		Matrix3x3 M1 = Rot * M0;

				const Quat q = M1;

				ca.StoreASCII(_F("quatf xformOp:orient = (%f, %f, %f, %f)\n", q.w, q.p.x, q.p.y, q.p.z));
	#endif
			}
			else
			{
				q = *rot;
//				ca.StoreASCII(_F("quatf xformOp:orient = (%f, %f, %f, %f)\n", rot->w, rot->p.x, rot->p.y, rot->p.z));
			}

			if(gDiscardImplicitPoses && q.IsIdentity())
				rot = null;
			else
			{
				Indent();
				mArray.StoreASCII(_F("quatf xformOp:orient = (%f, %f, %f, %f)\n", q.w, q.p.x, q.p.y, q.p.z));
			}
		}
	}

	if(scale)
	{
		//###TODO: do we need the flip here?
		Indent();
//		mArray.StoreASCII(_F("double3 xformOp:scale = (%f, %f, %f)\n", scale->x, scale->y, scale->z));
		mArray.StoreASCII(_F("float3 xformOp:scale = (%f, %f, %f)\n", scale->x, scale->y, scale->z));
	}

	StoreXFormOrder(pos!=0, rot!=0, scale!=0);
}

void USDExporter::ExportConvexHull(const ConvexShapeData* data)
{
	Cvh* CHull = CreateConvexHull(data->mNbVerts, data->mVerts);
	ASSERT(CHull);

//	const udword NbVerts = CHull->GetNbVerts();
	const Point* ConvexVerts = CHull->GetVerts();

/*	if(0)
	{
		// Path: export as mesh

		//### TODO: export polygons, not triangles, and also proper normals
		{
			udword TotalNbTris = 0;
			const udword NbPolys = CHull->GetNbPolygons();
			for(udword i=0;i<NbPolys;i++)
			{
				const HullPolygon& PolygonData = CHull->GetPolygon(i);
				const udword NbVertsInPoly = PolygonData.mNbVerts;
				const udword NbTris = NbVertsInPoly - 2;
				TotalNbTris += NbTris;
			}

			ca.StoreASCII("int[] faceVertexCounts = [");
			for(udword i=0;i<TotalNbTris;i++)
			{
				if(i==0)
					ca.StoreASCII("3");
				else
					ca.StoreASCII(", 3");
			}
			ca.StoreASCII("]\n");
		}

		{
			ca.StoreASCII("int[] faceVertexIndices = [");
			const udword NbPolys = CHull->GetNbPolygons();
			for(udword j=0;j<NbPolys;j++)
			{
				const HullPolygon& PolygonData = CHull->GetPolygon(j);
				const udword NbVertsInPoly = PolygonData.mNbVerts;
				const udword NbTris = NbVertsInPoly - 2;
				const udword* Indices = PolygonData.mVRef;

				udword Offset = 1;
				for(udword i=0;i<NbTris;i++)
				{
					const udword VRef0b = Indices[0];
					const udword VRef1b = Indices[Offset];
					const udword VRef2b = Indices[Offset+1];

					if(i==0 && j==0)
						ca.StoreASCII(_F("%d, %d, %d", VRef0b, VRef1b, VRef2b));
					else
						ca.StoreASCII(_F(", %d, %d, %d", VRef0b, VRef1b, VRef2b));

					Offset++;
				}
			}
			ca.StoreASCII("]\n");
		}

		ca.StoreASCII("point3f[] points = [");
		for(udword i=0;i<NbVerts;i++)
		{
			const Point& v = ConvexVerts[i];
			if(i==0)
				ca.StoreASCII(_F("(%f, %f, %f)", v.x, v.y, v.z));
			else
				ca.StoreASCII(_F(", (%f, %f, %f)", v.x, v.y, v.z));
		}
		ca.StoreASCII("]\n");
	}
	else*/
	{
		// Path: export as convex

		{
			Indent();	mArray.StoreASCII("int[] faceVertexCounts = [");
			const udword NbPolys = CHull->GetNbPolygons();
			for(udword i=0;i<NbPolys;i++)
			{
				const HullPolygon& PolygonData = CHull->GetPolygon(i);
				const udword NbVertsInPoly = PolygonData.mNbVerts;

				if(i)
					mArray.StoreASCII(", ");
				mArray.StoreASCII(_F("%d", NbVertsInPoly));
			}
			mArray.StoreASCII("]\n");
		}

		Indent();	mArray.StoreASCII("int[] faceVertexIndices = [");
		Vertices V;
		Vertices N;
		if(1)
		{
			// Dup vertices
			udword Index = 0;
			const udword NbPolys = CHull->GetNbPolygons();
			for(udword j=0;j<NbPolys;j++)
			{
				const HullPolygon& PolygonData = CHull->GetPolygon(j);
				const udword NbVertsInPoly = PolygonData.mNbVerts;
//				const udword NbTris = NbVertsInPoly - 2;
				const udword* Indices = PolygonData.mVRef;

				for(udword i=0;i<NbVertsInPoly;i++)
				{
					const udword VRef = Indices[i];
					const Point& v = ConvexVerts[VRef];
					V.AddVertex(v);
					N.AddVertex(PolygonData.mPlane.n);

					if(i==0 && j==0)
//						ca.StoreASCII(_F("%d", VRef));
						mArray.StoreASCII(_F("%d", Index++));
					else
//						ca.StoreASCII(_F(", %d", VRef));
						mArray.StoreASCII(_F(", %d", Index++));
				}
			}
		}
/*		else
		{
			const udword NbPolys = CHull->GetNbPolygons();
			for(udword j=0;j<NbPolys;j++)
			{
				const HullPolygon& PolygonData = CHull->GetPolygon(j);
				const udword NbVertsInPoly = PolygonData.mNbVerts;
				const udword NbTris = NbVertsInPoly - 2;
				const udword* Indices = PolygonData.mVRef;

				for(udword i=0;i<NbVertsInPoly;i++)
				{
					const udword VRef = Indices[i];

					if(i==0 && j==0)
						ca.StoreASCII(_F("%d", VRef));
					else
						ca.StoreASCII(_F(", %d", VRef));
				}
			}
		}*/
		mArray.StoreASCII("]\n");

		if(1)
		{
//Matrix3x3 Transfo;
//Transfo.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

			{
			Indent();	mArray.StoreASCII("normal3f[] normals = [");
			const udword TotalNbVerts = N.GetNbVertices();
			const Point* Pts = N.GetVertices();
			for(udword i=0;i<TotalNbVerts;i++)
			{
//				Point v = Pts[i];
				const Point& v = Pts[i];

//					if(gFlipYZ)
//						v *= Transfo;

				if(i)
					mArray.StoreASCII(", ");
				mArray.StoreASCII(_F("(%f, %f, %f)", v.x, v.y, v.z));
			}
			mArray.StoreASCII("]\n");
			}



			{
			Indent();	mArray.StoreASCII("point3f[] points = [");
			const udword TotalNbVerts = V.GetNbVertices();
			const Point* Pts = V.GetVertices();
			for(udword i=0;i<TotalNbVerts;i++)
			{
//				Point v = Pts[i];
				const Point& v = Pts[i];

//					if(gFlipYZ)
//						v *= Transfo;

				if(i)
					mArray.StoreASCII(", ");
				mArray.StoreASCII(_F("(%f, %f, %f)", v.x, v.y, v.z));
			}
			mArray.StoreASCII("]\n");
			}
		}
/*		else
		{
			ca.StoreASCII("point3f[] points = [");
			for(udword i=0;i<NbVerts;i++)
			{
				const Point& v = ConvexVerts[i];
				if(i==0)
					ca.StoreASCII(_F("(%f, %f, %f)", v.x, v.y, v.z));
				else
					ca.StoreASCII(_F(", (%f, %f, %f)", v.x, v.y, v.z));
			}
			ca.StoreASCII("]\n");
		}*/
	}
	DELETESINGLE(CHull);
}

void USDExporter::ExportMesh(const MeshShapeData* data)
{
	{
		Indent();	mArray.StoreASCII("int[] faceVertexCounts = [");
		udword Nb = 0;
		for(udword i=0;i<data->mNbTris;i++)
		{
			if(Nb==0)
				mArray.StoreASCII("3");
			else
				mArray.StoreASCII(", 3");

			Nb++;
			FormatCheck(Nb, data->mNbTris-1, i);
		}
		mArray.StoreASCII("]\n");
	}

	{
		Indent();	mArray.StoreASCII("int[] faceVertexIndices = [");
		udword Nb = 0;
		for(udword j=0;j<data->mNbTris;j++)
		{
			const IndexedTriangle& T = data->mTris[j];

			if(Nb==0)
				mArray.StoreASCII(_F("%d, %d, %d", T.mRef[0], T.mRef[1], T.mRef[2]));
			else
				mArray.StoreASCII(_F(", %d, %d, %d", T.mRef[0], T.mRef[1], T.mRef[2]));

			Nb++;
			FormatCheck(Nb, data->mNbTris-1, j);
		}
		mArray.StoreASCII("]\n");
	}

//	Matrix3x3 Transfo;
//	Transfo.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

	Indent();	mArray.StoreASCII("point3f[] points = [");
	udword Nb = 0;
	for(udword i=0;i<data->mNbVerts;i++)
	{
		const Point& v = data->mVerts[i];

//		if(gFlipYZ)
//			v *= Transfo;

		if(Nb==0)
			mArray.StoreASCII(_F("(%f, %f, %f)", v.x, v.y, v.z));
		else
			mArray.StoreASCII(_F(", (%f, %f, %f)", v.x, v.y, v.z));

		Nb++;
		FormatCheck(Nb, data->mNbVerts-1, i);
	}
	mArray.StoreASCII("]\n");
}

void USDExporter::ExportMesh(const RenderDataChunk* parent, const MeshRenderDataChunk* data)
{
	if(parent->GetID()!=INVALID_ID)
	{
		Indent();
		mArray.StoreASCII(_F("rel material:binding = </Materials/material%d>\n", parent->GetID()));

		const ManagedTexture* MT = parent->GetManagedTexture();
		if(MT)
		{
//			const char* 
		}
	}


//	{
		const udword NbTris = data->GetFaces().GetNbFaces();
		const udword NbVerts = data->GetVertices().GetNbPts();
		{
			Indent();	mArray.StoreASCII("int[] faceVertexCounts = [");
			udword Nb = 0;
			for(udword i=0;i<NbTris;i++)
			{
				if(Nb==0)
					mArray.StoreASCII("3");
				else
					mArray.StoreASCII(", 3");

				Nb++;
				FormatCheck(Nb, NbTris-1, i);
			}
			mArray.StoreASCII("]\n");
		}

		{
			Indent();	mArray.StoreASCII("int[] faceVertexIndices = [");
			udword Nb = 0;
			const udword* DFaces = data->GetFaces().GetDFaces();
			for(udword j=0;j<NbTris;j++)
			{
				const udword VRef0 = *DFaces++;
				const udword VRef1 = *DFaces++;
				const udword VRef2 = *DFaces++;

				if(Nb==0)
					mArray.StoreASCII(_F("%d, %d, %d", VRef0, VRef1, VRef2));
				else
					mArray.StoreASCII(_F(", %d, %d, %d", VRef0, VRef1, VRef2));

				Nb++;
				FormatCheck(Nb, NbTris-1, j);
			}
			mArray.StoreASCII("]\n");
		}

	//	Matrix3x3 Transfo;
	//	Transfo.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));

		Indent();	mArray.StoreASCII("point3f[] points = [");
		udword Nb = 0;
		for(udword i=0;i<NbVerts;i++)
		{
			const Point& v = data->GetVertices().GetPts()[i];

	//		if(gFlipYZ)
	//		v *= Transfo;

			if(Nb==0)
				mArray.StoreASCII(_F("(%f, %f, %f)", v.x, v.y, v.z));
//				mArray.StoreASCII(_F("Point(%ff, %ff, %ff)", v.x, v.y, v.z));
			else
				mArray.StoreASCII(_F(", (%f, %f, %f)", v.x, v.y, v.z));
//				mArray.StoreASCII(_F(", Point(%ff, %ff, %ff)", v.x, v.y, v.z));

			Nb++;
			FormatCheck(Nb, NbVerts-1, i);
		}
		mArray.StoreASCII("]\n");
//	}





	const udword NbTFaces = data->GetTFaces().GetNbFaces();
	const udword NbUVs = data->GetUVs().GetNbPts();
	if(NbTFaces && NbUVs)
	{
		Indent();	mArray.StoreASCII("float2[] primvars:st = [");
		udword Nb = 0;
		for(udword i=0;i<NbUVs;i++)
		{
			const Point& v = data->GetUVs().GetPts()[i];

			if(Nb==0)
				mArray.StoreASCII(_F("(%f, %f)", v.x, 1.0f-v.y));
			else
				mArray.StoreASCII(_F(", (%f, %f)", v.x, 1.0f-v.y));

			Nb++;
			FormatCheck(Nb, NbUVs-1, i);
		}
		mArray.StoreASCII("] (\n");
//		mArray.StoreASCII("]\n");
//return;
		gIndent++;
			Indent();	mArray.StoreASCII("customData = {\n");
			Indent();	mArray.StoreASCII("    dictionary Maya = {\n");
			Indent();	mArray.StoreASCII("        int UVSetIndex = 0\n");
			Indent();	mArray.StoreASCII("    }\n");
			Indent();	mArray.StoreASCII("}\n");
			Indent();	mArray.StoreASCII("interpolation = ");
			StoreQuotedString(mArray, "faceVarying");
			mArray.StoreASCII("\n");
		gIndent--;
		Indent();	mArray.StoreASCII(")\n");

		{
			Indent();	mArray.StoreASCII("int[] primvars:st:indices = [");
			udword Nb = 0;
			const udword* TFaces = data->GetTFaces().GetDFaces();
			for(udword j=0;j<NbTFaces;j++)
			{
				const udword VRef0 = *TFaces++;
				const udword VRef1 = *TFaces++;
				const udword VRef2 = *TFaces++;

				if(Nb==0)
					mArray.StoreASCII(_F("%d, %d, %d", VRef0, VRef1, VRef2));
				else
					mArray.StoreASCII(_F(", %d, %d, %d", VRef0, VRef1, VRef2));

				Nb++;
				FormatCheck(Nb, NbTFaces-1, j);
			}
			mArray.StoreASCII("]\n");
		}
	}
}

static bool CanExportRenderMeshes(const ShapeData* data)
{
	if(data->mRenderer && data->mRenderer->mRenderSource)
	{
		RenderDataChunk* RenderSource = data->mRenderer->mRenderSource;

		const udword ChunkType = RenderSource->GetChunkType();
		if(ChunkType==MeshRenderDataType)
		{
			return true;
		}
		else if(ChunkType==ColorRenderDataType)
		{
			RenderDataChunk* Child = RenderSource->GetChildRenderChunk();
			const udword ChildChunkType = Child->GetChunkType();
			if(ChildChunkType==MeshRenderDataType)
			{
				return true;
			}
		}
		else if(ChunkType==RenderCollectionType)
		{
			RenderDataChunkCollection* RDCChunk = static_cast<RenderDataChunkCollection*>(RenderSource);
			const PR* LocalPoses = (const PR*)RDCChunk->mLocalPoses.GetEntries();
			const udword NbChunks = RDCChunk->mRenderDataChunks.GetNbEntries();
			for(udword i=0;i<NbChunks;i++)
			{
				RenderDataChunk* RenderSource = (RenderDataChunk*)RDCChunk->mRenderDataChunks[i];

				const udword ChunkType = RenderSource->GetChunkType();
				if(ChunkType==ColorRenderDataType)
				{
					const ColorRenderDataChunk* crdc = static_cast<const ColorRenderDataChunk*>(RenderSource);

					RenderDataChunk* Child = RenderSource->GetChildRenderChunk();
					const udword ChildChunkType = Child->GetChunkType();
					if(ChildChunkType==MeshRenderDataType)
					{
						// ####
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool USDExporter::ExportRenderMeshes(const ShapeData* data)
{
/*	struct Local
	{
		static bool ExportRenderDataChunk(RenderDataChunk* chunk)
		{
			const udword Type = chunk->GetChunkType();
			if(Type==MeshRenderDataType)
			{
				const MeshRenderDataChunk* mrdc = static_cast<const MeshRenderDataChunk*>(chunk);
				ExportMesh(RenderSource, mrdc);
				return true;
			}
			return false;
		}
	};*/

	bool skip = false;

	if(data->mRenderer && data->mRenderer->mRenderSource)
	{
		RenderDataChunk* RenderSource = data->mRenderer->mRenderSource;

		const udword ChunkType = RenderSource->GetChunkType();
		if(ChunkType==MeshRenderDataType)
		{
			const MeshRenderDataChunk* mrdc = static_cast<const MeshRenderDataChunk*>(RenderSource);
			{
				ExportMesh(RenderSource, mrdc);
				skip = true;
			}
		}
		else if(ChunkType==ColorRenderDataType)
		{
			RenderDataChunk* Child = RenderSource->GetChildRenderChunk();
			const udword ChildChunkType = Child->GetChunkType();
			if(ChildChunkType==MeshRenderDataType)
			{
				const MeshRenderDataChunk* mrdc = static_cast<const MeshRenderDataChunk*>(Child);
				{
					ExportMesh(RenderSource, mrdc);
					skip = true;
				}
			}
		}
		else if(ChunkType==RenderCollectionType)
		{
			RenderDataChunkCollection* RDCChunk = static_cast<RenderDataChunkCollection*>(RenderSource);
			const PR* LocalPoses = (const PR*)RDCChunk->mLocalPoses.GetEntries();
			const udword NbChunks = RDCChunk->mRenderDataChunks.GetNbEntries();
			for(udword i=0;i<NbChunks;i++)
			{
				RenderDataChunk* RenderSource = (RenderDataChunk*)RDCChunk->mRenderDataChunks[i];

				const udword ChunkType = RenderSource->GetChunkType();
				if(ChunkType==ColorRenderDataType)
				{
					const ColorRenderDataChunk* crdc = static_cast<const ColorRenderDataChunk*>(RenderSource);

					RenderDataChunk* Child = RenderSource->GetChildRenderChunk();
					const udword ChildChunkType = Child->GetChunkType();
					if(ChildChunkType==MeshRenderDataType)
					{
						const MeshRenderDataChunk* mrdc = static_cast<const MeshRenderDataChunk*>(Child);


//			const MeshShapeData* Data = static_cast<const MeshShapeData*>(shape);

			static int count=0;
			const char* ShapeName = _F("testName%d", count++);

#ifdef HACK_HACK_HACK
			StartPrependDef("Mesh", ShapeName, gCollisionShapeAPI);
#else
			StartDef("Mesh", ShapeName);
#endif	
				ExportPose(&LocalPoses[i].mPos, &LocalPoses[i].mRot, null, false);

				const Point c(crdc->mColor.R, crdc->mColor.G, crdc->mColor.B);
				ExportColor(c);
				
				Indent();	mArray.StoreASCII("uniform bool doubleSided = 0\n");
				
							ExportMesh(crdc, mrdc);
//							ExportMesh(RenderSource, mrdc);
							skip = true;


#ifdef HACK_HACK_HACK

				if(gExportPhysics)
				{
					Indent();	mArray.StoreASCII("uniform token approximationShape = ");
					StoreQuotedString(mArray, "convexHull");
					mArray.StoreASCII("\n");
				}
#endif

			EndDef();

					}
				}



			}
		}
		else
		{
			ASSERT(!"Unhandled chunk type in ExportRenderMeshes");
		}
	}

	return skip;
}

/*        float contactOffset = 2
        float minTorsionalPatchRadius = 0
        float restOffset = 0
        float torsionalPatchRadius = 0*/

void USDExporter::ExportCommonPhysicsShapeParams()
{
/*	ExportPose(&shape->mLocalPos, &shape->mLocalRot, null, false);

	Indent();	mArray.StoreASCII("bool isCollider\n");
	Indent();	mArray.StoreASCII("uniform token purpose = ");
	StoreQuotedString(mArray, "guide");
	mArray.StoreASCII("\n");*/

	Indent();	mArray.StoreASCII("bool physics:collisionEnabled = true\n");
//	Indent();	mArray.StoreASCII("rel physics:simulationOwner = <...>\n");

	Indent();	mArray.StoreASCII("float physxCollision:contactOffset = 0.02\n");
	Indent();	mArray.StoreASCII("float physxCollision:restOffset = 0.0\n");
	Indent();	mArray.StoreASCII("float physxCollision:minTorsionalPatchRadius = 0.0\n");
	Indent();	mArray.StoreASCII("float physxCollision:torsionalPatchRadius = 0.0\n");
}

void USDExporter::ExportShape(const ShapeData* shape, udword index)
{
	const char* ShapeName = _F("Shape%d", index);

	switch(shape->mType)
	{
		case PINT_SHAPE_SPHERE:
		{
			const SphereShapeData* Data = static_cast<const SphereShapeData*>(shape);
/*
def Sphere "Sphere"
{
	color3f[] primvars:displayColor = [(0, 0, 1)]
	float3[] extent = [(-2, -2, -2), (2, 2, 2)]
	double radius = 2
	bool doubleSided = true
}*/
			StartDef("Sphere", ShapeName);

				Indent();	mArray.StoreASCII(_F("double radius = %f\n", Data->mRadius));

				ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);

				const RGBAColor* Color = Data->mRenderer->GetColor();
				if(Color)
				{
					const Point c(Color->R, Color->G, Color->B);
					ExportColor(c);

//					ca.StoreASCII(_F("float[] primvars:displayOpacity = [(%f, %f, %f)]\n", color.x, color.y, color.z));
				}

				//### TODO: collision shape exported *within* Sphere, is that correct?
				if(gExportPhysics)
				{
					//### name should use index
//					StoreCollisionShape(ca, "Sphere", "collisionSphere");
				}

			EndDef();


			//### copied from Box
			//### TODO: collision shape exported after Box, is that correct?
			if(gExportPhysics)
			{
//				StoreCollisionShape(ca, "Cube", "collisionBox");

				const char* CollisionShapeName = _F("collisionSphere%d", index);

				StartPrependDef("Sphere", CollisionShapeName, gCollisionShapeAPI, gPhysXCollisionAPI);

					Indent();	mArray.StoreASCII(_F("double radius = %f\n", Data->mRadius));

					ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);

					//Indent();	mArray.StoreASCII("bool isCollider\n");
					//Indent();	mArray.StoreASCII("uniform token purpose = ");
					//StoreQuotedString(mArray, "guide");
					//mArray.StoreASCII("\n");

					ExportCommonPhysicsShapeParams();
				EndDef();
			}
		}
		break;

		case PINT_SHAPE_CAPSULE:
		{
			const CapsuleShapeData* Data = static_cast<const CapsuleShapeData*>(shape);

			// TODO: check this export, refactor pose export etc with other shapes

			Matrix3x3 ytoz;
			ytoz.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
			const Quat qytoz = ytoz;

			const Quat q = Data->mLocalRot * qytoz;
//			const Quat q = qytoz * Data->mLocalRot;

//			const float Height = (Data->mHalfHeight - Data->mRadius)*2.0f;
			const float Height = Data->mHalfHeight*2.0f;

			StartDef("Capsule", ShapeName);

				Indent();	mArray.StoreASCII(_F("double radius = %f\n", Data->mRadius));
				Indent();	mArray.StoreASCII(_F("double height = %f\n", Height));
/*				mArray.StoreASCII("uniform token[] axis = [");
//				if(gFlipYZ)
//					StoreQuotedString(mArray, "Z");
//				else
					StoreQuotedString(mArray, "Y");
				mArray.StoreASCII("]\n\n");*/

				//### TODO: missing for other shapes
//				exportPose(ca, &Data->mLocalPos, &Data->mLocalRot, false);
				// ### doesn't seem to work for local pose
//				ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);
				ExportPose(&Data->mLocalPos, &q, null, false);

				const RGBAColor* Color = Data->mRenderer->GetColor();
				if(Color)
				{
					const Point c(Color->R, Color->G, Color->B);
					ExportColor(c);

//					ca.StoreASCII(_F("float[] primvars:displayOpacity = [(%f, %f, %f)]\n", color.x, color.y, color.z));
				}

				//### TODO: collision shape exported *within* Sphere, is that correct?
				if(gExportPhysics)
				{
//					StoreCollisionShape(ca, "Sphere", "collisionSphere");
				}

			EndDef();


			if(gExportPhysics)
			{
//				StoreCollisionShape(ca, "Cube", "collisionBox");

				const char* CollisionShapeName = _F("collisionCapsule%d", index);

				StartPrependDef("Capsule", CollisionShapeName, gCollisionShapeAPI, gPhysXCollisionAPI);

					Indent();	mArray.StoreASCII(_F("double radius = %f\n", Data->mRadius));
					Indent();	mArray.StoreASCII(_F("double height = %f\n", Height));

					//ca.StoreASCII(_F("double size = 2\n"));
//					ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);
					ExportPose(&Data->mLocalPos, &q, null, false);

					// TODO: refactor this with other shapes
					//Indent();	mArray.StoreASCII("bool isCollider\n");
					//Indent();	mArray.StoreASCII("uniform token purpose = ");
					//StoreQuotedString(mArray, "guide");
					//mArray.StoreASCII("\n");

					ExportCommonPhysicsShapeParams();
				EndDef();
			}
		}
		break;

		case PINT_SHAPE_CYLINDER:
		{
			const CylinderShapeData* Data = static_cast<const CylinderShapeData*>(shape);

			// TODO: check this export, refactor pose export etc with other shapes

			Matrix3x3 ytoz;
			ytoz.FromTo(Point(0.0f, 1.0f, 0.0f), Point(0.0f, 0.0f, 1.0f));
			const Quat qytoz = ytoz;

			const Quat q = Data->mLocalRot * qytoz;

			const float Height = Data->mHalfHeight*2.0f;

			StartDef("Cylinder", ShapeName);

				Indent();	mArray.StoreASCII(_F("double radius = %f\n", Data->mRadius));
				Indent();	mArray.StoreASCII(_F("double height = %f\n", Height));

				ExportPose(&Data->mLocalPos, &q, null, false);

				const RGBAColor* Color = Data->mRenderer->GetColor();
				if(Color)
				{
					const Point c(Color->R, Color->G, Color->B);
					ExportColor(c);
				}

			EndDef();

			if(gExportPhysics)
			{
				const char* CollisionShapeName = _F("collisionCylinder%d", index);

				StartPrependDef("Cylinder", CollisionShapeName, gCollisionShapeAPI, gPhysXCollisionAPI);

					Indent();	mArray.StoreASCII(_F("double radius = %f\n", Data->mRadius));
					Indent();	mArray.StoreASCII(_F("double height = %f\n", Height));

					ExportPose(&Data->mLocalPos, &q, null, false);

					ExportCommonPhysicsShapeParams();

					Indent();	mArray.StoreASCII("bool physics:customGeometryEnabled = true\n");
					Indent();	mArray.StoreASCII("custom bool refinementEnableEnabled = true\n");
					Indent();	mArray.StoreASCII("custom int refinementLevel = 2\n");

				EndDef();
			}
		}
		break;

		case PINT_SHAPE_BOX:
		{
			const BoxShapeData* Data = static_cast<const BoxShapeData*>(shape);

			const bool CanSkip = CanExportRenderMeshes(Data);

			if(CanSkip)
			{
				StartDef("Mesh", ShapeName);

					ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);

					const RGBAColor* Color = Data->mRenderer->GetColor();
					if(Color)
					{
						const Point c(Color->R, Color->G, Color->B);
						ExportColor(c);

	//					ca.StoreASCII(_F("float[] primvars:displayOpacity = [(%f, %f, %f)]\n", color.x, color.y, color.z));
					}

					Indent();	mArray.StoreASCII("uniform bool doubleSided = 0\n");

					const bool skip = ExportRenderMeshes(Data);
					ASSERT(skip);
				EndDef();
			}
			else
			{

//			const bool skip = ExportRenderMeshes(Data);
//			if(!skip)
//			{

			StartDef("Cube", ShapeName);

				Indent();	mArray.StoreASCII(_F("double size = 2\n"));

	/*
				//### hardcoded Y/Z flip
	//			ca.StoreASCII(_F("double3 xformOp:scale = (%f, %f, %f)\n", Data->mExtents.x, Data->mExtents.z, Data->mExtents.y));
				ca.StoreASCII(_F("double3 xformOp:scale = (%f, %f, %f)\n", Data->mExtents.x, Data->mExtents.y, Data->mExtents.z));
				ca.StoreASCII("uniform token[] xformOpOrder = [");
				StoreQuotedString(ca, "xformOp:scale");
				ca.StoreASCII("]\n");*/

					ExportPose(&Data->mLocalPos, &Data->mLocalRot, &Data->mExtents, false);

				const RGBAColor* Color = Data->mRenderer->GetColor();
				if(Color)
				{
					const Point c(Color->R, Color->G, Color->B);
					ExportColor(c);

//					ca.StoreASCII(_F("float[] primvars:displayOpacity = [(%f, %f, %f)]\n", color.x, color.y, color.z));
				}

	/*
					int[] faceVertexCounts = [4, 4, 4, 4, 4, 4]
					int[] faceVertexIndices = [0, 1, 3, 2, 4, 5, 7, 6, 10, 11, 13, 12, 14, 15, 9, 8, 17, 23, 21, 19, 22, 16, 18, 20]
					normal3f[] normals = [(0, 0, 1), (0, 0, 1), (0, 0, 1), (0, 0, 1), (0, 0, -1), (0, 0, -1), (0, 0, -1), (0, 0, -1), (0, -1, 0), (0, -1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0), (0, -1, 0), (0, -1, 0), (-1, 0, 0), (1, 0, 0), (-1, 0, 0), (1, 0, 0), (-1, 0, 0), (1, 0, 0), (-1, 0, 0), (1, 0, 0)]
					point3f[] points = [(-1, -1, 1), (1, -1, 1), (-1, 1, 1), (1, 1, 1), (-1, 1, -1), (1, 1, -1), (-1, -1, -1), (1, -1, -1), (-1, -1, 1), (1, -1, 1), (-1, 1, 1), (1, 1, 1), (-1, 1, -1), (1, 1, -1), (-1, -1, -1), (1, -1, -1), (-1, -1, 1), (1, -1, 1), (-1, 1, 1), (1, 1, 1), (-1, 1, -1), (1, 1, -1), (-1, -1, -1), (1, -1, -1)]
	*/

	//			ca.StoreASCII(_F("double radius = %f\n", Data->mRadius));

			EndDef();
			}

			//### TODO: collision shape exported after Box, is that correct?
			if(gExportPhysics)
			{
//				StoreCollisionShape(ca, "Cube", "collisionBox");

				const char* CollisionShapeName = _F("collisionBox%d", index);

				StartPrependDef("Cube", CollisionShapeName, gCollisionShapeAPI, gPhysXCollisionAPI);

					Indent();	mArray.StoreASCII(_F("double size = 2\n"));

					//ca.StoreASCII(_F("double size = 2\n"));
					ExportPose(&Data->mLocalPos, &Data->mLocalRot, &Data->mExtents, false);

					// TODO: refactor this with other shapes
					//Indent();	mArray.StoreASCII("bool isCollider\n");
					//Indent();	mArray.StoreASCII("uniform token purpose = ");
					//StoreQuotedString(mArray, "guide");
					//mArray.StoreASCII("\n");

					ExportCommonPhysicsShapeParams();
				EndDef();
			}

/*        def Cube "collisionBox" (
        )
        {
            bool isCollider
            uniform token purpose = "guide"
            float3 xformOp:scale = (1, 1, 1)
            uniform token[] xformOpOrder = ["xformOp:scale"]
        }*/
		}
		break;

		case PINT_SHAPE_CONVEX:
		{
			const ConvexShapeData* Data = static_cast<const ConvexShapeData*>(shape);

#ifdef REMOVED_OLD
			StartDef("Mesh", ShapeName);
#else
			if(gExportPhysics)
				StartPrependDef("Mesh", ShapeName, gCollisionShapeAPI, gPhysXCollisionAPI, "PhysicsMeshCollisionAPI");
			else
				StartPrependDef("Mesh", ShapeName, gCollisionShapeAPI, gPhysXCollisionAPI);
#endif

				ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);

				const RGBAColor* Color = Data->mRenderer->GetColor();
				if(Color)
				{
					const Point c(Color->R, Color->G, Color->B);
					ExportColor(c);

//					ca.StoreASCII(_F("float[] primvars:displayOpacity = [(%f, %f, %f)]\n", color.x, color.y, color.z));
				}

//				ca.StoreASCII(_F("double radius = %f\n", Data->mRadius));

			Indent();	mArray.StoreASCII("uniform bool doubleSided = 0\n");
//			CA.StoreASCII("int[] faceVertexCounts = [4]\n");
//			CA.StoreASCII("int[] faceVertexIndices = [0, 1, 2, 3]\n");
//			CA.StoreASCII("normal3f[] normals = [(1, 0, 0), (1, 0, 0), (1, 0, 0), (1, 0, 0)]\n");
//			CA.StoreASCII(_F("point3f[] points = [(0, -%f, -%f), (0, %f, -%f), (0, %f, %f), (0, -%f, %f)]\n", Size, Size, Size, Size, Size, Size, Size, Size));

/*			bool skip = false;
			if(1)
			{
				if(Data->mRenderer && Data->mRenderer->mRenderSource)
				{
					udword ChunkType = Data->mRenderer->mRenderSource->GetChunkType();
					if(ChunkType==ColorRenderDataType)
					{
						RenderDataChunk* Child = Data->mRenderer->mRenderSource->GetChildRenderChunk();
						udword ChildChunkType = Child->GetChunkType();
						if(ChildChunkType==MeshRenderDataType)
						{
							const MeshRenderDataChunk* mrdc = static_cast<const MeshRenderDataChunk*>(Child);
							{
								ExportMesh(Data->mRenderer->mRenderSource, mrdc);
								skip = true;
							}
						}
					}
				}
			}*/
			const bool skip = ExportRenderMeshes(Data);

			if(!skip)
				ExportConvexHull(Data);

				if(gExportPhysics)
				{
#ifdef REMOVED_OLD
					const char* CollisionShapeName = _F("collisionConvex%d", index);

#ifdef REMOVED_OLD
						StartPrependDef("Mesh", CollisionShapeName, gCollisionShapeAPI);
#else
						StartPrependDef("ConvexMesh", CollisionShapeName, gCollisionShapeAPI);
#endif

						ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);

				// TODO: refactor this with other shapes
					Indent();	mArray.StoreASCII("bool isCollider\n");
					Indent();	mArray.StoreASCII("uniform token purpose = ");
					StoreQuotedString(mArray, "guide");
					mArray.StoreASCII("\n");

#ifdef REMOVED_OLD
#else
//						ExportConvexHull(Data);

//					Indent();	mArray.StoreASCII("uniform token approximationShape = ");
//					StoreQuotedString(mArray, "convexHull");
//					mArray.StoreASCII("\n");
#endif
					EndDef();
#else
					//Indent();	mArray.StoreASCII("uniform token approximationShape = ");
					Indent();	mArray.StoreASCII("uniform token physics:approximation = ");
					StoreQuotedString(mArray, "convexHull");
					mArray.StoreASCII("\n");
#endif
					ExportCommonPhysicsShapeParams();
				}

			EndDef();

/*
    def Mesh "smallBlueCube"
    {
		uniform bool doubleSided = 1
		float3[] extent = [(-0.5, -0.5, -0.5), (0.5, 0.5, 0.5)]
		int[] faceVertexCounts = [4, 4, 4, 4, 4, 4]
		int[] faceVertexIndices = [0, 1, 3, 2, 4, 5, 7, 6, 10, 11, 13, 12, 14, 15, 9, 8, 17, 23, 21, 19, 22, 16, 18, 20]
		normal3f[] normals = [(0, 0, 1), (0, 0, 1), (0, 0, 1), (0, 0, 1), (0, 0, -1), (0, 0, -1), (0, 0, -1), (0, 0, -1), (0, -1, 0), (0, -1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0), (0, -1, 0), (0, -1, 0), (-1, 0, 0), (1, 0, 0), (-1, 0, 0), (1, 0, 0), (-1, 0, 0), (1, 0, 0), (-1, 0, 0), (1, 0, 0)]
		point3f[] points = [(-0.5, -0.5, 0.5), (0.5, -0.5, 0.5), (-0.5, 0.5, 0.5), (0.5, 0.5, 0.5), (-0.5, 0.5, -0.5), (0.5, 0.5, -0.5), (-0.5, -0.5, -0.5), (0.5, -0.5, -0.5), (-0.5, -0.5, 0.5), (0.5, -0.5, 0.5), (-0.5, 0.5, 0.5), (0.5, 0.5, 0.5), (-0.5, 0.5, -0.5), (0.5, 0.5, -0.5), (-0.5, -0.5, -0.5), (0.5, -0.5, -0.5), (-0.5, -0.5, 0.5), (0.5, -0.5, 0.5), (-0.5, 0.5, 0.5), (0.5, 0.5, 0.5), (-0.5, 0.5, -0.5), (0.5, 0.5, -0.5), (-0.5, -0.5, -0.5), (0.5, -0.5, -0.5)]
		color3f[] primvars:displayColor = [(0, 0, 1)]
	}

*/
		}
		break;

		case PINT_SHAPE_MESH:
		{
			const MeshShapeData* Data = static_cast<const MeshShapeData*>(shape);

#ifdef HACK_HACK_HACK
			StartDef("Mesh", ShapeName);
#else
	#ifdef REMOVED_OLD
			StartDef("Mesh", ShapeName);
	#else
			StartPrependDef("Mesh", ShapeName, gCollisionShapeAPI, gPhysXCollisionAPI);
	#endif
#endif

				ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);

				const RGBAColor* Color = Data->mRenderer->GetColor();
				if(Color)
				{
					const Point c(Color->R, Color->G, Color->B);
					ExportColor(c);

//					ca.StoreASCII(_F("float[] primvars:displayOpacity = [(%f, %f, %f)]\n", color.x, color.y, color.z));
				}

				Indent();	mArray.StoreASCII("uniform bool doubleSided = 0\n");


/*			bool skip = false;
			if(1)
			{
				if(Data->mRenderer && Data->mRenderer->mRenderSource)
				{
					udword ChunkType = Data->mRenderer->mRenderSource->GetChunkType();
					if(ChunkType==ColorRenderDataType)
					{
						RenderDataChunk* Child = Data->mRenderer->mRenderSource->GetChildRenderChunk();
						udword ChildChunkType = Child->GetChunkType();
						if(ChildChunkType==MeshRenderDataType)
						{
							const MeshRenderDataChunk* mrdc = static_cast<const MeshRenderDataChunk*>(Child);
							{
								ExportMesh(Data->mRenderer->mRenderSource, mrdc);
								skip = true;
							}
						}
					}
				}
			}*/
			const bool skip = ExportRenderMeshes(Data);

			if(!skip)
				ExportMesh(Data);

				if(gExportPhysics)
				{
#ifdef REMOVED_OLD
					const char* CollisionShapeName = _F("collisionMesh%d", index);

						StartPrependDef("Mesh", CollisionShapeName, gCollisionShapeAPI);

						ExportPose(&Data->mLocalPos, &Data->mLocalRot, null, false);

				// TODO: refactor this with other shapes
					Indent();	mArray.StoreASCII("bool isCollider\n");
					Indent();	mArray.StoreASCII("uniform token purpose = ");
					StoreQuotedString(mArray, "guide");
//					ca.StoreASCII("\n}\n");
					mArray.StoreASCII("\n");
					EndDef();
#else
#endif
					ExportCommonPhysicsShapeParams();
				}

			EndDef();
		}
		break;

		default:
		{
			ASSERT(0);
		}
		break;
	}
}

void USDExporter::ExportActor(const EditorPlugin& editor, const ActorData* actor, const char* name, const Point* color, bool is_instanced, bool export_lin_vel, bool export_ang_vel)
{
/*	if(gExportPhysics && actor->mMass!=0.0f)
	{
		StartPrependDef("Xform", name, "PhysicsRigidBodyAPI", "PhysicsMassAPI", "PhysxRigidBodyAPI");
		Indent();	mArray.StoreASCII("bool physics:rigidBodyEnabled = true\n");	// "PhysicsRigidBodyAPI"
	}
	else
	{
		StartDef("Xform", name);
	}*/

	// Revisited after talking to Ales
	if(gExportPhysics)
	{
		StartPrependDef("Xform", name, "PhysicsRigidBodyAPI", "PhysicsMassAPI", "PhysxRigidBodyAPI");
		Indent();
		if(actor->mMass!=0.0f)
			mArray.StoreASCII("bool physics:rigidBodyEnabled = true\n");
		else
			mArray.StoreASCII("bool physics:rigidBodyEnabled = false\n");
	}
	else
	{
		StartDef("Xform", name);
	}

	if(!is_instanced)
		ExportPose(&actor->mPosition, &actor->mRotation, null, true);

	if(color)
		ExportColor(*color);
	Indent();	mArray.StoreASCII("uniform bool doubleSided = 0\n");

	if(gExportPhysics)
	{
		if(!is_instanced)
		{
			Indent();	mArray.StoreASCII(_F("uniform int actorID = %d\n", actor->mID));
		}

		if(actor->mMass==0.0f)
		{
			if(!gAnimXP)
			{
			}
		}
		else
		{
			// "PhysicsMassAPI"
			if(0)	// ########## needs a dialog option
			{
				const Pint_Actor* ActorAPI = const_cast<EditorPlugin&>(editor).GetActorAPI();
				if(ActorAPI)
				{
					PintActorHandle h = PintActorHandle(actor);

					{
						const float Mass = ActorAPI->GetMass(h);

						Indent();	mArray.StoreASCII(_F("float physics:mass = %f\n", Mass));
						if(actor->mMassForInertia>=0.0f)
						{
							Indent();	mArray.StoreASCII(_F("uniform float massForInertia = %f\n", actor->mMassForInertia));
						}
					}

					{
						Point LocalInertia;
						ActorAPI->GetLocalInertia(h, LocalInertia);

						Indent();	mArray.StoreASCII(_F("vector3f physics:diagonalInertia = (%f, %f, %f)\n", LocalInertia.x, LocalInertia.y, LocalInertia.z));
					}

					{
						PR MassPose;
						ActorAPI->GetCMassLocalPose(h, MassPose);

						Indent();	mArray.StoreASCII(_F("vector3f physics:centerOfMass = (%f, %f, %f)\n", MassPose.mPos.x, MassPose.mPos.y, MassPose.mPos.z));
						Indent();	mArray.StoreASCII(_F("quatf physics:principalAxes = (%f, %f, %f, %f)\n", MassPose.mRot.w, MassPose.mRot.p.x, MassPose.mRot.p.y, MassPose.mRot.p.z));
					}

				}
			}
			else
			{
				Indent();	mArray.StoreASCII(_F("float physics:mass = %f\n", actor->mMass));
				if(actor->mMassForInertia>=0.0f)
				{
					Indent();	mArray.StoreASCII(_F("uniform float massForInertia = %f\n", actor->mMassForInertia));
				}

				if(!actor->mCOMLocalOffset.IsZero())
				{
					Indent();	mArray.StoreASCII(_F("vector3f physics:centerOfMass = (%f, %f, %f)\n", actor->mCOMLocalOffset.x, actor->mCOMLocalOffset.y, actor->mCOMLocalOffset.z));
				}

				// float3 physics:diagonalInertia = (0.0, 0.0, 0.0)
				// quatf physics:principalAxes = (1, 0, 0, 0)
			}

			if(export_lin_vel && actor->mLinearVelocity.IsNonZero())
			{
				Point v = actor->mLinearVelocity;
				if(gFlipYZ)
					TSwap(v.y, v.z);

				Indent();	mArray.StoreASCII(_F("vector3f physics:velocity = (%f, %f, %f)\n", v.x, v.y, v.z));
			}

			if(export_ang_vel && actor->mAngularVelocity.IsNonZero())
			{
				Point v = actor->mAngularVelocity * RADTODEG;
				if(gFlipYZ)
					TSwap(v.y, v.z);

				Indent();	mArray.StoreASCII(_F("vector3f physics:angularVelocity = (%f, %f, %f)\n", v.x, v.y, v.z));
			}

			// TODO: take these from UI
			Indent();	mArray.StoreASCII("float physxRigidBody:linearDamping = 0.1\n");
			Indent();	mArray.StoreASCII("float physxRigidBody:angularDamping = 0.05\n");
			Indent();	mArray.StoreASCII("float physxRigidBody:maxAngularVelocity = 5729\n");	// 100 * RADTODEG
			Indent();	mArray.StoreASCII("float physxRigidBody:maxLinearVelocity = inf\n");
			Indent();	mArray.StoreASCII("int physxRigidBody:solverPositionIterationCount = 4\n");
			Indent();	mArray.StoreASCII("int physxRigidBody:solverVelocityIterationCount = 1\n");
			Indent();	mArray.StoreASCII("float physxRigidBody:sleepThreshold = 0.05\n");
			Indent();	mArray.StoreASCII("int physxRigidBody:lockedRotAxis = 0\n");
			Indent();	mArray.StoreASCII("int physxRigidBody:lockedPosAxis = 0\n");
			Indent();	mArray.StoreASCII("bool physxRigidBody:enableSpeculativeCCD = false\n");
			Indent();	mArray.StoreASCII("bool physxRigidBody:enableCCD = false\n");
			Indent();
			if(actor->mKinematic)
				mArray.StoreASCII("bool physics:kinematicEnabled = true\n");
			else
				mArray.StoreASCII("bool physics:kinematicEnabled = false\n");

// TODO: export mesh render shape for dynamic convexes in Lego scene
			Indent();	mArray.StoreASCII("float physxRigidBody:maxDepenetrationVelocity = 3\n");

			// rel physics:simulationOwner = <...>
			// uniform bool physics:startsAsleep = false
			// float physxRigidBody:stabilizationThreshold = 0.00001
			// float physxRigidBody:maxContactImpulse = inf
			// bool physxRigidBody:retainAccelerations = false
			// bool physxRigidBody:disableGravity = false
		}
	}

	const udword NbShapes = actor->mShapes.GetNbEntries();
	for(udword j=0;j<NbShapes;j++)
	{
		const ShapeData* CurrentShape = (const ShapeData*)actor->mShapes.GetEntry(j);
		ExportShape(CurrentShape, j);
	}

	EndDef();
}

void USDExporter::ExportSceneData(const SceneData& scene_data)
{
	if(gExportPhysics)
	{
		StartPrependDef("PhysicsScene", "physicsScene", "PhysxSceneAPI");
		{
			{
				Point Gravity = scene_data.mGravity;
				if(gFlipYZ)
					TSwap(Gravity.y, Gravity.z);
				const float GravityMagnitude = Gravity.Magnitude();
				Gravity.Normalize();
				Indent();	mArray.StoreASCII(_F("vector3f physics:gravityDirection = (%f, %f, %f)\n", Gravity.x, Gravity.y, Gravity.z));
				Indent();	mArray.StoreASCII(_F("float physics:gravityMagnitude = %f\n", GravityMagnitude));
			}

			// TODO: take these values from current test!
			Indent();	mArray.StoreASCII("float physxScene:bounceThreshold = 2.0\n");
			Indent();	mArray.StoreASCII("float physxScene:frictionOffsetThreshold = 0.04\n");
			Indent();	mArray.StoreASCII("bool physxScene:enableCCD = false\n");					// TODO: not always false!
			Indent();	mArray.StoreASCII("bool physxScene:enableEnhancedDeterminism = false\n");
			Indent();	mArray.StoreASCII("bool physxScene:enableGPUDynamics = false\n");
			Indent();	mArray.StoreASCII("bool physxScene:enableStabilization = true\n");
			Indent();	mArray.StoreASCII("uint physxScene:timeStepsPerSecond = 60\n");				// TODO: not always!
			Indent();	mArray.StoreASCII("bool physxScene:invertCollisionGroupFilter = false\n");

			Indent();
			mArray.StoreASCII("uniform token physxScene:collisionSystem = ");
			StoreQuotedString(mArray, "PCM");
			mArray.StoreASCII("\n");

			// TODO: not always MBP!
			Indent();
			mArray.StoreASCII("uniform token physxScene:broadphaseType = ");
			StoreQuotedString(mArray, "MBP");
			mArray.StoreASCII("\n");

			// TODO: not always patch!
			Indent();
			mArray.StoreASCII("uniform token physxScene:frictionType = ");
			StoreQuotedString(mArray, "patch");
			mArray.StoreASCII("\n");

			// TODO: not always PGS!
			Indent();
			mArray.StoreASCII("uniform token physxScene:solverType = ");
			StoreQuotedString(mArray, "PGS");
			mArray.StoreASCII("\n");

/*
	uint physxScene:gpuTempBufferCapacity = 16777216
	uint physxScene:gpuMaxRigidContactCount = 524288
	uint physxScene:gpuMaxRigidPatchCount = 81920
	uint physxScene:gpuHeapCapacity = 67108864
	uint physxScene:gpuFoundLostPairsCapacity = 262144
	uint physxScene:gpuFoundLostAggregatePairsCapacity = 1024
	uint physxScene:gpuTotalAggregatePairsCapacity = 1024
	uint physxScene:gpuMaxSoftBodyContacts = 1048576
	uint physxScene:gpuMaxParticleContacts = 1048576 
	uint physxScene:gpuMaxNumPartitions = 8
*/
		}
		EndDef();
	}

	// Export plane if needed
	if(scene_data.mCreateDefaultEnvironment)
	{
		// The hardcoded PEEL plane is:
		//	PINT_BOX_CREATE BoxDesc(400.0f, 10.0f, 400.0f);
		//	ObjectDesc.mPosition.x		= 0.0f;
		//	ObjectDesc.mPosition.y		= -10.0f;
		//	ObjectDesc.mPosition.z		= 0.0f;

		const float Size = 400.0f;

//#ifdef REMOVED_OLD
		StartDef("Mesh", "StaticPlane");
//#else
//		StartDef("Cube", "StaticPlane");
//#endif
			// TODO: why the weird orientation?
			// Beware, this one is for Z up
			ExportColor(Point(0.5f, 0.5f, 0.5f));
			Indent();	mArray.StoreASCII("uniform bool doubleSided = 0\n");
//#ifdef REMOVED_OLD
			Indent();	mArray.StoreASCII("int[] faceVertexCounts = [4]\n");
			Indent();	mArray.StoreASCII("int[] faceVertexIndices = [0, 1, 2, 3]\n");
			if(gFlipYZ)
			{
				Indent();	mArray.StoreASCII("normal3f[] normals = [(1, 0, 0), (1, 0, 0), (1, 0, 0), (1, 0, 0)]\n");
//				Indent();	mArray.StoreASCII(_F("point3f[] points = [(0, -%f, -%f), (0, %f, -%f), (0, %f, %f), (0, -%f, %f)]\n", Size, Size, Size, Size, Size, Size, Size, Size));
				Indent();	mArray.StoreASCII(_F("point3f[] points = [(-%f, -%f, 0), (%f, -%f, 0), (%f, %f, 0), (-%f, %f, 0)]\n", Size, Size, Size, Size, Size, Size, Size, Size));
//				Indent();	mArray.StoreASCII("quatf xformOp:orient = (0.70710677, 0, -0.70710677, 0)\n");
//				StoreXFormOrder(false, true, false);
			}
			else
			{
				Indent();	mArray.StoreASCII("normal3f[] normals = [(0, 1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0)]\n");
				Indent();	mArray.StoreASCII(_F("point3f[] points = [(-%f, 0, -%f), (%f, 0, -%f), (%f, 0, %f), (-%f, 0, %f)]\n", Size, Size, Size, Size, Size, Size, Size, Size));
			}
/*#else
			Indent();	mArray.StoreASCII(_F("double size = 2\n"));

			const Point Scale(Size, Size, 4.0f);
			const Point Pos(0.0f, 0.0f, -4.0f);
			ExportPose(&Pos, null, &Scale, false);
#endif*/

			if(gExportPhysics)
			{
//#ifdef REMOVED_OLD
				//StartDef("StaticBody", "staticBody");
				//EndDef();

				StartPrependDef("Plane", "collisionPlane", gCollisionShapeAPI, gPhysXCollisionAPI);
//					Indent();	mArray.StoreASCII("bool isCollider\n");
			        Indent();	mArray.StoreASCII("uniform token axis = ");
					StoreQuotedString(mArray, gFlipYZ ? "Z" : "Y");
					mArray.StoreASCII("\n");

					//Indent();	mArray.StoreASCII("uniform token purpose = ");
					//StoreQuotedString(mArray, "guide");
					//mArray.StoreASCII("\n");

					ExportCommonPhysicsShapeParams();
				EndDef();
/*#else
				StartPrependDef("Cube", "collisionPlane", gCollisionShapeAPI);

					Indent();	mArray.StoreASCII(_F("double size = 2\n"));
					ExportPose(&Pos, null, &Scale, false);

					Indent();	mArray.StoreASCII("bool isCollider\n");
					Indent();	mArray.StoreASCII("uniform token purpose = ");
					StoreQuotedString(CA, "guide");
					mArray.StoreASCII("\n");
				EndDef();
#endif*/
			}

		EndDef();
	}
}

static void ResetRenderSourceIDs()
{
	const udword NbRenderSources = GetNbRenderSources();
	for(udword i=0;i<NbRenderSources;i++)
	{
		RenderDataChunk* chunk = GetRenderSource(i);
		chunk->SetID(INVALID_ID);
	}
}

static void OutputString(CustomArray& custom_array, const char* string, udword length)
{
	for(udword i=0;i<length;i++)
		custom_array.Store(ubyte(string[i]));
}

static bool Replace(char* buffer, const char* tag, const char* replacement, udword& DstOffset, bool add_underscore)
{
	char* Suffix = strstr(buffer, tag);
	if(!Suffix)
		return false;

	while(Suffix)
	{
		buffer[DstOffset] = 0;

		char* PostSuffix = Suffix + strlen(tag);
		const char* BranchSuffix = replacement;
		if(BranchSuffix)
		{
			char PostSuffixCopy[4096];
			strcpy(PostSuffixCopy, PostSuffix);

			if(add_underscore)
				*Suffix++ = '_';

			while(*BranchSuffix)
				*Suffix++ = *BranchSuffix++;

			BranchSuffix = PostSuffixCopy;
			while(*BranchSuffix)
				*Suffix++ = *BranchSuffix++;
		}
		else
		{
			while(*PostSuffix)
				*Suffix++ = *PostSuffix++;
		}
		*Suffix = 0;
		DstOffset = udword(strlen(buffer));

		Suffix = strstr(buffer, tag);
	}
	return true;
}

static void CopyTexture(const char* filename, const char* texture_name, const char* path)
{
	{
		//###### only works for png
		const char* foundfile = FindPEELFile(filename);
		ASSERT(foundfile);
		if(foundfile)
		{
			const char* dstFilename = _F("%s%s.png", path, texture_name);
			CopyFile(foundfile, dstFilename, FALSE);
		}
	}
	{
		const char* filename = FindPEELFile("template.mdl");
		ASSERT(filename);
		if(filename)
		{
			IceFile F(filename);
			udword Length;
			const char* Data = (const char*)F.Load(Length);

			CustomArray CA;

			char buffer[4096];
			udword SrcOffset=0;
			while(SrcOffset!=Length)
			{
				ASSERT(SrcOffset<Length);

				udword DstOffset=0;
				while((Data[SrcOffset]!=13 && Data[SrcOffset+1]!=10) && SrcOffset!=Length)
				{
					ASSERT(DstOffset<4096);
					buffer[DstOffset++] = Data[SrcOffset++];
				};
				if(SrcOffset!=Length)
				{
					buffer[DstOffset++] = 13;
					buffer[DstOffset++] = 10;
					ASSERT(DstOffset<4096);
					SrcOffset += 2;
				}

				Replace(buffer, "<TEXTURE_NAME>", texture_name, DstOffset, false);

				OutputString(CA, buffer, DstOffset);
			};
//			CA.ExportToDisk(mFilename1, "wb");

			const char* dstFilename = _F("%smat_%s.mdl", path, texture_name);
			CA.ExportToDisk(dstFilename, "wb");
		}
	}
}

static const char* CreateTexture(const ManagedTexture& mt, String& str, const char* path)
{
	static udword GenID = 0;
	str = _F("GenName%d", ++GenID);
	const char* texture_name = str.Get();

	// <create file here>
	{
		const char* filename = _F("%s%s.png", path, texture_name);
		bool status = SaveWithDevil(filename, mt.mSource);
		(void)status;
	}

	{
		const char* filename = FindPEELFile("template.mdl");
		ASSERT(filename);
		if(filename)
		{
			IceFile F(filename);
			udword Length;
			const char* Data = (const char*)F.Load(Length);

			CustomArray CA;

			char buffer[4096];
			udword SrcOffset=0;
			while(SrcOffset!=Length)
			{
				ASSERT(SrcOffset<Length);

				udword DstOffset=0;
				while((Data[SrcOffset]!=13 && Data[SrcOffset+1]!=10) && SrcOffset!=Length)
				{
					ASSERT(DstOffset<4096);
					buffer[DstOffset++] = Data[SrcOffset++];
				};
				if(SrcOffset!=Length)
				{
					buffer[DstOffset++] = 13;
					buffer[DstOffset++] = 10;
					ASSERT(DstOffset<4096);
					SrcOffset += 2;
				}

				Replace(buffer, "<TEXTURE_NAME>", texture_name, DstOffset, false);

				OutputString(CA, buffer, DstOffset);
			};
//			CA.ExportToDisk(mFilename1, "wb");

			const char* dstFilename = _F("%smat_%s.mdl", path, texture_name);
			CA.ExportToDisk(dstFilename, "wb");
		}
	}


	return texture_name;
}

/*
    def Material "OmniGlass"
    {
        token outputs:mdl:displacement.connect = </Materials/OmniGlass/OmniGlass.outputs:out>
        token outputs:mdl:surface.connect = </Materials/OmniGlass/OmniGlass.outputs:out>
        token outputs:mdl:volume.connect = </Materials/OmniGlass/OmniGlass.outputs:out>
		token outputs:surface.connect = None

        def Shader "OmniGlass" (
            kind = "Material"
        )
        {
			uniform token info:implementationSource = "sourceAsset"
			uniform asset info:mdl:sourceAsset = @./OmniGlass.mdl@
			uniform token info:mdl:sourceAsset:subIdentifier = "OmniGlass"
			token outputs:out
        }
    }
*/

void USDExporter::ExportRenderSource(RenderDataChunk* RenderSource, udword& material_id, const char* path)
{
	const ManagedTexture* MT = RenderSource->GetManagedTexture();
	if(MT)
	{
		// Examples:
		// path				C:\Projects\#PEEL\USD_Test\tmp\
		// TextureFilename	maps/diffuse/6636d21.png
		// TextureName		6636d21

		//printf("...found managed texture %s\n", MT->mFilename.Get());
		StartDef("Material", _F("material%d", material_id));
		{
			String tmp;
			const char* TextureName = null;

			const char* TextureFilename = MT->mFilename.Get();
			if(TextureFilename)
			{
				tmp.Set(TextureFilename);
				RemovePath(tmp);

				sdword offset = tmp.ReverseFind('.');
				if(offset!=-1)
					tmp.SetAt(0, offset);
				TextureName = tmp.Get();

				CopyTexture(TextureFilename, TextureName, path);
			}
			else
			{
				TextureName = CreateTexture(*MT, tmp, path);
			}

			if(0)
			{
/*
	def Material "material0"
	{
		token outputs:surface.connect = </Materials/material0/shader0.outputs:out>
		def Shader "shader0"
		(
			kind = "Material"
		)
		{
			uniform token info:id = "mdlMaterial"
			custom asset module = @mat_64391d3.mdl@
			custom string name = "mat_64391d3"
			token outputs:out
			custom string shaderType = "mdl_OmniBasic"
		}
	}
*/
				Indent();	mArray.StoreASCII(_F("token outputs:surface.connect = </Materials/material%d/shader%d.outputs:out>\n", material_id, material_id));

				Indent();	mArray.StoreASCII("def Shader ");
							StoreQuotedString(mArray, _F("shader%d", material_id));
							mArray.StoreASCII("\n");

				Indent();	mArray.StoreASCII("(\n");
				gIndent++;
					Indent();	mArray.StoreASCII("kind = ");
								StoreQuotedString(mArray, "Material");
								mArray.StoreASCII("\n");
				gIndent--;
				Indent();	mArray.StoreASCII(")\n");
				Indent();	mArray.StoreASCII("{\n");
				gIndent++;

					Indent();	mArray.StoreASCII("uniform token info:id = ");
								StoreQuotedString(mArray, "mdlMaterial");
								mArray.StoreASCII("\n");

					Indent();	mArray.StoreASCII(_F("custom asset module = @mat_%s.mdl@\n", TextureName));

					Indent();	mArray.StoreASCII("custom string name = ");
								StoreQuotedString(mArray, _F("mat_%s", TextureName));
								mArray.StoreASCII("\n");

					Indent();	mArray.StoreASCII("token outputs:out\n");

					Indent();	mArray.StoreASCII("custom string shaderType = ");
								StoreQuotedString(mArray, "mdl_OmniBasic");
								mArray.StoreASCII("\n");

				gIndent--;
				Indent();	mArray.StoreASCII("}\n");
			}
			else
			{
/*
    def Material "material0"
    {
        token outputs:mdl:displacement.connect = </Materials/material0/shader0.outputs:out>
        token outputs:mdl:surface.connect = </Materials/material0/shader0.outputs:out>
        token outputs:mdl:volume.connect = </Materials/material0/shader0.outputs:out>
        token outputs:surface.connect = None

        def Shader "shader0" (
            kind = "Material"
        )
        {
            uniform token info:implementationSource = "sourceAsset"
            uniform asset info:mdl:sourceAsset = @mat_64391d3.mdl@
            uniform token info:mdl:sourceAsset:subIdentifier = "mat_64391d3"
            token outputs:out
        }
    }

*/
				Indent();	mArray.StoreASCII(_F("token outputs:mdl:displacement.connect = </Materials/material%d/shader%d.outputs:out>\n", material_id, material_id));
				Indent();	mArray.StoreASCII(_F("token outputs:mdl:surface.connect = </Materials/material%d/shader%d.outputs:out>\n", material_id, material_id));
				Indent();	mArray.StoreASCII(_F("token outputs:mdl:volume.connect = </Materials/material%d/shader%d.outputs:out>\n", material_id, material_id));
				Indent();	mArray.StoreASCII("token outputs:surface.connect = None\n");

				Indent();	mArray.StoreASCII("def Shader ");
							StoreQuotedString(mArray, _F("shader%d", material_id));
							mArray.StoreASCII("\n");

				Indent();	mArray.StoreASCII("(\n");
				gIndent++;
					Indent();	mArray.StoreASCII("kind = ");
								StoreQuotedString(mArray, "Material");
								mArray.StoreASCII("\n");
				gIndent--;
				Indent();	mArray.StoreASCII(")\n");
				Indent();	mArray.StoreASCII("{\n");
				gIndent++;

					Indent();	mArray.StoreASCII("uniform token info:implementationSource = ");
								StoreQuotedString(mArray, "sourceAsset");
								mArray.StoreASCII("\n");

					Indent();	mArray.StoreASCII(_F("uniform asset info:mdl:sourceAsset = @mat_%s.mdl@\n", TextureName));

					Indent();	mArray.StoreASCII("uniform token info:mdl:sourceAsset:subIdentifier = ");
								StoreQuotedString(mArray, _F("mat_%s", TextureName));
								mArray.StoreASCII("\n");

					Indent();	mArray.StoreASCII("token outputs:out\n");

				gIndent--;
				Indent();	mArray.StoreASCII("}\n");
			}
		}
		EndDef();
		RenderSource->SetID(material_id++);
	}
}

void USDExporter::ExportRenderSources(const PtrContainer& shapes, udword& material_id, const char* path)
{
	const udword NbShapes = shapes.GetNbEntries();
	for(udword i=0;i<NbShapes;i++)
	{
		const ShapeData* Data = reinterpret_cast<const ShapeData*>(shapes[i]);
		if(Data->mRenderer && Data->mRenderer->mRenderSource)
		{
			RenderDataChunk* RenderSource = Data->mRenderer->mRenderSource;

			if(RenderSource->GetChunkType()==RenderCollectionType)
			{
				RenderDataChunkCollection* RDCChunk = static_cast<RenderDataChunkCollection*>(RenderSource);
				const udword NbChunks = RDCChunk->mRenderDataChunks.GetNbEntries();
				for(udword j=0;j<NbChunks;j++)
				{
					RenderDataChunk* RenderSource = (RenderDataChunk*)RDCChunk->mRenderDataChunks[j];
					ExportRenderSource(RenderSource, material_id, path);
				}
			}
			else
				ExportRenderSource(RenderSource, material_id, path);
		}
	}
}

void USDExporter::ExportMaterials(const EditorPlugin& editor, const char* path)
{
	ResetRenderSourceIDs();

	udword MaterialID = 0;

	StartDef("Scope", "Materials");
		ExportRenderSources(editor.GetEditorSphereShapes(), MaterialID, path);
		ExportRenderSources(editor.GetEditorCapsuleShapes(), MaterialID, path);
		ExportRenderSources(editor.GetEditorCylinderShapes(), MaterialID, path);
		ExportRenderSources(editor.GetEditorBoxShapes(), MaterialID, path);
		ExportRenderSources(editor.GetEditorConvexShapes(), MaterialID, path);
		ExportRenderSources(editor.GetEditorMeshShapes(), MaterialID, path);
		ExportRenderSources(editor.GetEditorMeshShapes2(), MaterialID, path);
	EndDef();
}

static bool Compatible(const ShapeData* shape0, const ShapeData* shape1)
{
	const PintShape Type = shape0->mType;
	if(Type!=shape1->mType)
		return false;

	if(shape0->mLocalPos != shape1->mLocalPos)
		return false;

	if(shape0->mLocalRot != shape1->mLocalRot)
		return false;

	// mMaterial ? mRenderer ?

	switch(Type)
	{
		case PINT_SHAPE_SPHERE:
		{
			const SphereShapeData* Data0 = static_cast<const SphereShapeData*>(shape0);
			const SphereShapeData* Data1 = static_cast<const SphereShapeData*>(shape1);
			if(Data0->mRadius != Data1->mRadius)
				return false;
		}
		break;

		case PINT_SHAPE_CAPSULE:
		{
			const CapsuleShapeData* Data0 = static_cast<const CapsuleShapeData*>(shape0);
			const CapsuleShapeData* Data1 = static_cast<const CapsuleShapeData*>(shape1);

			if(Data0->mRadius != Data1->mRadius)
				return false;
			if(Data0->mHalfHeight != Data1->mHalfHeight)
				return false;
		}
		break;

		case PINT_SHAPE_BOX:
		{
			const BoxShapeData* Data0 = static_cast<const BoxShapeData*>(shape0);
			const BoxShapeData* Data1 = static_cast<const BoxShapeData*>(shape1);

			if(Data0->mExtents != Data1->mExtents)
				return false;
		}
		break;

		case PINT_SHAPE_CONVEX:
		{
			const ConvexShapeData* Data0 = static_cast<const ConvexShapeData*>(shape0);
			const ConvexShapeData* Data1 = static_cast<const ConvexShapeData*>(shape1);

			const udword NbVerts = Data0->mNbVerts;
			if(NbVerts!=Data1->mNbVerts)
				return false;

			// TODO: ouch - can we use the renderer pointer here?
			for(udword i=0;i<NbVerts;i++)
			{
				if(Data0->mVerts[i] != Data1->mVerts[i])
					return false;
			}
		}
		break;

		case PINT_SHAPE_MESH:
		{
//			const MeshShapeData* Data0 = static_cast<const MeshShapeData*>(shape0);
//			const MeshShapeData* Data1 = static_cast<const MeshShapeData*>(shape1);

			// TODO: support this
//			if(Data0->mRenderer != Data1->mRenderer)
//				return false;
			return false;
		}
		break;

		default:
		{
			ASSERT(0);
		}
		break;
	}
	return true;
}

// Checks if two actors can be put into the same "point instancer"
static bool Compatible(const ActorData* actor0, const ActorData* actor1)
{
	const udword NbShapes = actor0->mShapes.GetNbEntries();
	if(NbShapes!=actor1->mShapes.GetNbEntries())
		return false;

	for(udword i=0;i<NbShapes;i++)
	{
		const ShapeData* Shape0 = (const ShapeData*)actor0->mShapes.GetEntry(i);
		const ShapeData* Shape1 = (const ShapeData*)actor1->mShapes.GetEntry(i);
		// TODO: minor subtlety here: we assume that the shapes are in the same order, but technically two compound actors
		// could have their shapes listed in a different order and still be compatible!
		if(!Compatible(Shape0, Shape1))
			return false;
	}

	if(actor0->mMass != actor1->mMass)
		return false;

	// TODO: take more parameters into account here?

	return true;
}

static Container gEvilTmpContainer;

static const char* GetValidUSDName(const String& name, udword id, const char* unnamed_prefix)
{
	const char* Name;
	if(gUseActorNames && name.Get())
	{
		Name = _F("%s_%d", name.Get(), id);

		// Have to remove 'invalid' USD characters. This code is so bad.
		String tmp;
		if(Name[0]>='0' && Name[0]<='9')
			tmp.Set("_");
		tmp += Name;
		tmp.Replace(' ', '_');
		tmp.Replace('-', '_');
		tmp.Replace('.', '_');
		tmp.Replace('(', '_');
		tmp.Replace(')', '_');
		strcpy(const_cast<char*>(Name), tmp.Get());
	}
	else
		Name = _F("%s_%d", unnamed_prefix, id);
	return Name;
}

static const char* GetActorName(const ActorData* actor)
{
	if(!actor)
		return null;

	if(actor->mName.Get() && strcmp(actor->mName.Get(), "GearObject")==0)
		gEvilTmpContainer.Add(actor->mID);

	return GetValidUSDName(actor->mName, actor->mID, "UnnamedActor");
}

static const char* GetJointName(const JointData* joint)
{
	if(!joint)
		return null;

	return GetValidUSDName(joint->mName, joint->mID, "UnnamedJoint");
}

static void ExportAllActors(const EditorPlugin& editor, USDExporter& exporter)
{
	const Point Color = const_cast<EditorPlugin&>(editor).GetMainColor();
	const udword NbActors = editor.GetNbEditorActors();
	udword ID = 0;
	for(udword i=0;i<NbActors;i++)
	{
		const ActorData* Current = editor.GetEditorActor(i);
		if(CanExport(Current))
		{
			Current->mID = ID++;

			const char* Name = GetActorName(Current);

			exporter.ExportActor(editor, Current, Name, &Color, false, true, true);
		}
	}
}

static void ExportAllJoints(const EditorPlugin& editor, USDExporter& Exporter, CustomArray& CA)
{
	if(gExportJoints)
	{
		struct Local
		{
			static void ExportCommonJointParams(USDExporter& Exporter, CustomArray& CA, const JointData* Current,
				const Point& local_pos0, const Point& local_pos1)
			{
				if(Current->mObject0)
				{
					const char* Name0 = GetActorName(Current->mObject0);
					Exporter.Indent();
					CA.StoreASCII("rel physics:body0 = </World/").StoreASCII(Name0).StoreASCII(">\n");
				}

				if(Current->mObject1)
				{
					const char* Name1 = GetActorName(Current->mObject1);
					Exporter.Indent();
					CA.StoreASCII("rel physics:body1 = </World/").StoreASCII(Name1).StoreASCII(">\n");
				}

				Exporter.Indent();	CA.StoreASCII("bool physics:jointEnabled = true\n");
				Exporter.Indent();	CA.StoreASCII("bool physics:collisionEnabled = false\n");
				Exporter.Indent();	CA.StoreASCII("bool physics:excludeFromArticulation = false\n");
				Exporter.Indent();	CA.StoreASCII("float physics:breakForce = inf\n");
				Exporter.Indent();	CA.StoreASCII("float physics:breakTorque = inf\n");

				Exporter.Indent();	CA.StoreASCII("bool physxJoint:enableProjection = false\n");
				Exporter.Indent();	CA.StoreASCII("float physxJoint:jointFriction = 0.0\n");
				Exporter.Indent();	CA.StoreASCII("float physxJoint:maxJointVelocity = 1000000.0\n");

				Exporter.Indent();	CA.StoreASCII(_F("point3f physics:localPos0 = (%f, %f, %f)\n", local_pos0.x, local_pos0.y, local_pos0.z));
				Exporter.Indent();	CA.StoreASCII(_F("point3f physics:localPos1 = (%f, %f, %f)\n", local_pos1.x, local_pos1.y, local_pos1.z));
			}

			static void normalToTangents(const PxVec3& n, PxVec3& t1, PxVec3& t2)
			{
				const PxReal m_sqrt1_2 = PxReal(0.7071067811865475244008443621048490);
				if(fabsf(n.z) > m_sqrt1_2)
				{
					const PxReal a = n.y*n.y + n.z*n.z;
					const PxReal k = PxReal(1.0)/PxSqrt(a);
					t1 = PxVec3(0,-n.z*k,n.y*k);
					t2 = PxVec3(a*k,-n.x*t1.z,n.x*t1.y);
				}
				else 
				{
					const PxReal a = n.x*n.x + n.y*n.y;
					const PxReal k = PxReal(1.0)/PxSqrt(a);
					t1 = PxVec3(-n.y*k,n.x*k,0);
					t2 = PxVec3(-n.z*t1.y,n.z*t1.x,a*k);
				}
				t1.normalize();
				t2.normalize();
			}

			static PxQuat ComputeJointQuat(const ActorData* actor, const PxVec3& localAxis)
			{
				PxTransform globalPose(PxIdentity);
				if(actor)
					globalPose = PxTransform(ToPxVec3(actor->mPosition), ToPxQuat(actor->mRotation));

				//find 2 orthogonal vectors.
				//gotta do this in world space, if we choose them
				//separately in local space they won't match up in worldspace.
				PxVec3 axisw = globalPose.rotate(localAxis);
				axisw.normalize();

				PxVec3 normalw, binormalw;
				normalToTangents(axisw, binormalw, normalw);

				const PxVec3 localNormal = globalPose.rotateInv(normalw);

				const PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
				PxQuat q(rot);
				q.normalize();
				return q;
			}

			static bool NeedsHingeLimits(float min_limit, float max_limit)
			{
				const bool HasLimits = min_limit<=max_limit;
				if(!HasLimits)
					return false;

				if(min_limit==MIN_FLOAT && max_limit==MAX_FLOAT)
					return false;
				return true;
			}

			static void ExportHingeLimits(USDExporter& Exporter, CustomArray& CA, float min_limit, float max_limit)
			{
				ASSERT(NeedsHingeLimits(min_limit, max_limit));

				Exporter.Indent();
				if(min_limit<MIN_FLOAT*0.5f)
					CA.StoreASCII("float physics:lowerLimit = -inf\n");
				else
					CA.StoreASCII(_F("float physics:lowerLimit = %f\n", min_limit * RADTODEG));

				Exporter.Indent();
				if(max_limit>MAX_FLOAT*0.5f)
					CA.StoreASCII("float physics:upperLimit = inf\n");
				else
					CA.StoreASCII(_F("float physics:upperLimit = %f\n", max_limit * RADTODEG));

				// TODO: revisit this
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:bounceThreshold = 0.0\n");
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:restitution = 0.0\n");
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:contactDistance = 0.0\n");
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:damping = 0.0\n");
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:stiffness = 0.0\n");
			}

			static void ExportHingeDrive(USDExporter& Exporter, CustomArray& CA, float target_vel)
			{
				// TODO: take values from test
				Exporter.Indent();	CA.StoreASCII("float drive:angular:physics:damping = 100000.0\n");
				Exporter.Indent();	CA.StoreASCII("float drive:angular:physics:stiffness = 0.0\n");
				Exporter.Indent();	CA.StoreASCII(_F("float drive:angular:physics:targetVelocity = %f\n", - target_vel * RADTODEG));
				Exporter.Indent();	CA.StoreASCII("float drive:angular:physics:targetPosition = 0.0\n");
				Exporter.Indent();	CA.StoreASCII("float drive:angular:physics:maxForce = inf\n");
				Exporter.Indent();	CA.StoreASCII("uniform token drive:angular:physics:type = ");
									StoreQuotedString(CA, "force");	// or "acceleration"
									CA.StoreASCII("\n");
			}

			static bool ConvertLimitsPEEL2Kit(float& min_limit, float& max_limit)
			{
				if(min_limit<max_limit)
				{
					// If min<max limit then DOF is limited both on the PEEL & Kit sides, nothing to do.
					return true;
				}
				else if(min_limit==max_limit)
				{
					// If min==max limit then DOF is locked in PEEL, must use min>max in Kit
					min_limit = 1.0f;
					max_limit = -1.0f;
					return true;
				}
				else if(min_limit>max_limit)
				{
					// If min>max limit then DOF is free in PEEL, must use INF values in Kit or simply not create the limit.
					return false;
				}
				return false;
			}
		};

		const udword NbJoints = editor.GetNbEditorJoints();
		for(udword i=0;i<NbJoints;i++)
		{
			const JointData* Current = editor.GetEditorJoint(i);
			Current->mID = i;

			{
				switch(Current->mType)
				{
					case PINT_JOINT_SPHERICAL:
					{
						const SphericalJointData* jd = static_cast<const SphericalJointData*>(Current);

						Exporter.StartPrependDef("PhysicsSphericalJoint", GetJointName(Current), "PhysxJointAPI");
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0.mPos, jd->mLocalPivot1.mPos);

							const Quat& q0 = jd->mLocalPivot0.mRot;
							const Quat& q1 = jd->mLocalPivot1.mRot;

							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot0 = (%f, %f, %f, %f)\n", q0.w, q0.p.x, q0.p.y, q0.p.z));
							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot1 = (%f, %f, %f, %f)\n", q1.w, q1.p.x, q1.p.y, q1.p.z));

							Exporter.Indent();	CA.StoreASCII("uniform token physics:axis = ");
												StoreQuotedString(CA, "X");
												CA.StoreASCII("\n");

							if(IsSphericalLimitEnabled(jd->mLimits))
							{
								Exporter.Indent();	CA.StoreASCII(_F("float physics:coneAngle0Limit = %f\n", jd->mLimits.mMinValue * RADTODEG));
								Exporter.Indent();	CA.StoreASCII(_F("float physics:coneAngle1Limit = %f\n", jd->mLimits.mMaxValue * RADTODEG));
							}
							else
							{
								Exporter.Indent();	CA.StoreASCII("float physics:coneAngle0Limit = -1.0\n");
								Exporter.Indent();	CA.StoreASCII("float physics:coneAngle1Limit = -1.0\n");
							}
						}
						Exporter.EndDef();
					}
					break;

					case PINT_JOINT_DISTANCE:
					{
						const DistanceJointData* jd = static_cast<const DistanceJointData*>(Current);

						Exporter.StartPrependDef("PhysicsDistanceJoint", GetJointName(Current), "PhysxJointAPI");
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0, jd->mLocalPivot1);

							Exporter.Indent();	CA.StoreASCII("quatf physics:localRot0 = (1, 0, 0, 0)\n");
							Exporter.Indent();	CA.StoreASCII("quatf physics:localRot1 = (1, 0, 0, 0)\n");

							Exporter.Indent();	CA.StoreASCII(_F("float physics:minDistance = %f\n", jd->mLimits.mMinValue));
							Exporter.Indent();	CA.StoreASCII(_F("float physics:maxDistance = %f\n", jd->mLimits.mMaxValue));
						}
						Exporter.EndDef();
					}
					break;

					case PINT_JOINT_FIXED:
					{
						const FixedJointData* jd = static_cast<const FixedJointData*>(Current);

						Exporter.StartPrependDef("PhysicsFixedJoint", GetJointName(Current), "PhysxJointAPI");
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0, jd->mLocalPivot1);

							Exporter.Indent();	CA.StoreASCII("quatf physics:localRot0 = (1, 0, 0, 0)\n");
							Exporter.Indent();	CA.StoreASCII("quatf physics:localRot1 = (1, 0, 0, 0)\n");
						}
						Exporter.EndDef();
					}
					break;

					case PINT_JOINT_HINGE:
					{
						const HingeJointData* jd = static_cast<const HingeJointData*>(Current);

						const bool HasLimits = Local::NeedsHingeLimits(jd->mLimits.mMinValue, jd->mLimits.mMaxValue);
						const char* LimitsAPI = HasLimits ? "PhysxLimitAPI:angular" : null;

						const bool hasDrive =	(Current->mObject0 && gEvilTmpContainer.Contains(Current->mObject0->mID))
											||	(Current->mObject1 && gEvilTmpContainer.Contains(Current->mObject1->mID));

						if(hasDrive || jd->mUseMotor)
							Exporter.StartPrependDef("PhysicsRevoluteJoint", GetJointName(Current), "PhysxJointAPI", "PhysicsDriveAPI:angular", LimitsAPI);
						else
							Exporter.StartPrependDef("PhysicsRevoluteJoint", GetJointName(Current), "PhysxJointAPI", LimitsAPI);
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0, jd->mLocalPivot1);

							const PxQuat q0 = Local::ComputeJointQuat(jd->mObject0, ToPxVec3(jd->mLocalAxis0));
							const PxQuat q1 = Local::ComputeJointQuat(jd->mObject1, ToPxVec3(jd->mLocalAxis1));

							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot0 = (%f, %f, %f, %f)\n", q0.w, q0.x, q0.y, q0.z));
							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot1 = (%f, %f, %f, %f)\n", q1.w, q1.x, q1.y, q1.z));

							Exporter.Indent();	CA.StoreASCII("uniform token physics:axis = ");
												StoreQuotedString(CA, "X");
												CA.StoreASCII("\n");

							if(hasDrive || jd->mUseMotor)
								Local::ExportHingeDrive(Exporter, CA, jd->mUseMotor ? jd->mDriveVelocity : 5.0f);

							if(HasLimits)
								Local::ExportHingeLimits(Exporter, CA, jd->mLimits.mMinValue, jd->mLimits.mMaxValue);
						}
						Exporter.EndDef();
					}
					break;

					case PINT_JOINT_HINGE2:
					{
						const Hinge2JointData* jd = static_cast<const Hinge2JointData*>(Current);

						const bool HasLimits = Local::NeedsHingeLimits(jd->mLimits.mMinValue, jd->mLimits.mMaxValue);
						const char* LimitsAPI = HasLimits ? "PhysxLimitAPI:angular" : null;

						const bool hasDrive =	(Current->mObject0 && gEvilTmpContainer.Contains(Current->mObject0->mID))
											||	(Current->mObject1 && gEvilTmpContainer.Contains(Current->mObject1->mID));

						if(hasDrive || jd->mUseMotor)
							Exporter.StartPrependDef("PhysicsRevoluteJoint", GetJointName(Current), "PhysxJointAPI", "PhysicsDriveAPI:angular", LimitsAPI);
						else
							Exporter.StartPrependDef("PhysicsRevoluteJoint", GetJointName(Current), "PhysxJointAPI", LimitsAPI);
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0.mPos, jd->mLocalPivot1.mPos);

							const Quat& q0 = jd->mLocalPivot0.mRot;
							const Quat& q1 = jd->mLocalPivot1.mRot;

							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot0 = (%f, %f, %f, %f)\n", q0.w, q0.p.x, q0.p.y, q0.p.z));
							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot1 = (%f, %f, %f, %f)\n", q1.w, q1.p.x, q1.p.y, q1.p.z));

							Exporter.Indent();	CA.StoreASCII("uniform token physics:axis = ");
												StoreQuotedString(CA, "X");
												CA.StoreASCII("\n");

							if(hasDrive || jd->mUseMotor)
								Local::ExportHingeDrive(Exporter, CA, jd->mUseMotor ? jd->mDriveVelocity : 1.0f);

							if(HasLimits)
								Local::ExportHingeLimits(Exporter, CA, jd->mLimits.mMinValue, jd->mLimits.mMaxValue);
						}
						Exporter.EndDef();
					}
					break;

					case PINT_JOINT_PRISMATIC:
					{
						const PrismaticJointData* jd = static_cast<const PrismaticJointData*>(Current);

						const bool hasDrive = false;

						const bool HasLimits = IsPrismaticLimitEnabled(jd->mLimits);
						const char* LimitsAPI = HasLimits ? "PhysxLimitAPI:linear" : null;

						if(hasDrive/* || jd->mUseMotor*/)
							Exporter.StartPrependDef("PhysicsPrismaticJoint", GetJointName(Current), "PhysxJointAPI", "PhysicsDriveAPI:linear", LimitsAPI);
						else
							Exporter.StartPrependDef("PhysicsPrismaticJoint", GetJointName(Current), "PhysxJointAPI", LimitsAPI);
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0.mPos, jd->mLocalPivot1.mPos);

							if(jd->mLocalAxis0.IsNonZero() && jd->mLocalAxis1.IsNonZero())
							{
								const PxQuat q0 = Local::ComputeJointQuat(jd->mObject0, ToPxVec3(jd->mLocalAxis0));
								const PxQuat q1 = Local::ComputeJointQuat(jd->mObject1, ToPxVec3(jd->mLocalAxis1));

								Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot0 = (%f, %f, %f, %f)\n", q0.w, q0.x, q0.y, q0.z));
								Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot1 = (%f, %f, %f, %f)\n", q1.w, q1.x, q1.y, q1.z));
							}
							else
							{
								const PxQuat q0 = ToPxQuat(jd->mLocalPivot0.mRot);
								const PxQuat q1 = ToPxQuat(jd->mLocalPivot1.mRot);

								Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot0 = (%f, %f, %f, %f)\n", q0.w, q0.x, q0.y, q0.z));
								Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot1 = (%f, %f, %f, %f)\n", q1.w, q1.x, q1.y, q1.z));
							}

							Exporter.Indent();	CA.StoreASCII("uniform token axis = ");
												StoreQuotedString(CA, "X");
												CA.StoreASCII("\n");

							if(hasDrive)
							{
								ASSERT(0);
							}

							if(HasLimits)
							{
								//Exporter.Indent();	CA.StoreASCII(_F("float physics:lowerLimit = %f\n", jd->mMinLimit));
								Exporter.Indent();
								if(jd->mLimits.mMinValue<MIN_FLOAT*0.5f)
									CA.StoreASCII("float physics:lowerLimit = -inf\n");
								else
									CA.StoreASCII(_F("float physics:lowerLimit = %f\n", jd->mLimits.mMinValue));

								//Exporter.Indent();	CA.StoreASCII(_F("float physics:upperLimit = %f\n", jd->mMaxLimit));
								Exporter.Indent();
								if(jd->mLimits.mMaxValue>MAX_FLOAT*0.5f)
									CA.StoreASCII("float physics:upperLimit = inf\n");
								else
									CA.StoreASCII(_F("float physics:upperLimit = %f\n", jd->mLimits.mMaxValue));

//
/*
				// TODO: revisit this
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:bounceThreshold = 0\n");
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:restitution = 0\n");
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:contactDistance = 1e20\n");
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:damping = 0\n");
				Exporter.Indent();	CA.StoreASCII("float physxLimit:angular:stiffness = 0\n");
*/
								Exporter.Indent();	CA.StoreASCII("float physxLimit:linear:bounceThreshold = 0.0\n");
								Exporter.Indent();	CA.StoreASCII("float physxLimit:linear:restitution = 0.0\n");
								Exporter.Indent();	CA.StoreASCII("float physxLimit:linear:contactDistance = 0.0\n");
								Exporter.Indent();	CA.StoreASCII(_F("float physxLimit:linear:damping = %f\n", jd->mSpring.mDamping));
								Exporter.Indent();	CA.StoreASCII(_F("float physxLimit:linear:stiffness = %f\n", jd->mSpring.mStiffness));
							}
						}
						Exporter.EndDef();
					}
					break;

					case PINT_JOINT_GEAR:
					{
						const GearJointData* jd = static_cast<const GearJointData*>(Current);

						Exporter.StartPrependDef("PhysxPhysicsGearJoint", GetJointName(Current), "PhysxJointAPI");
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0.mPos, jd->mLocalPivot1.mPos);

							const Quat& q0 = jd->mLocalPivot0.mRot;
							const Quat& q1 = jd->mLocalPivot1.mRot;

							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot0 = (%f, %f, %f, %f)\n", q0.w, q0.p.x, q0.p.y, q0.p.z));
							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot1 = (%f, %f, %f, %f)\n", q1.w, q1.p.x, q1.p.y, q1.p.z));

							Exporter.Indent();	CA.StoreASCII(_F("float physics:gearRatio = %f\n", jd->mGearRatio));

							if(jd->mHinge0)
							{
								const char* Name = GetJointName(jd->mHinge0);
								//const char* Name = _F("Joint%d", jd->mHinge0->mID);
								Exporter.Indent();
								CA.StoreASCII("rel physics:hinge0 = </World/").StoreASCII(Name).StoreASCII(">\n");
							}

							if(jd->mHinge1)
							{
								const char* Name = GetJointName(jd->mHinge1);
								//const char* Name = _F("Joint%d", jd->mHinge1->mID);
								Exporter.Indent();
								CA.StoreASCII("rel physics:hinge1 = </World/").StoreASCII(Name).StoreASCII(">\n");
							}
						}
						Exporter.EndDef();
					}
					break;

					case PINT_JOINT_RACK_AND_PINION:
					{
						const RackJointData* jd = static_cast<const RackJointData*>(Current);

						Exporter.StartPrependDef("PhysxPhysicsRackAndPinionJoint", GetJointName(Current), "PhysxJointAPI");
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0.mPos, jd->mLocalPivot1.mPos);

							const Quat& q0 = jd->mLocalPivot0.mRot;
							const Quat& q1 = jd->mLocalPivot1.mRot;

							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot0 = (%f, %f, %f, %f)\n", q0.w, q0.p.x, q0.p.y, q0.p.z));
							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot1 = (%f, %f, %f, %f)\n", q1.w, q1.p.x, q1.p.y, q1.p.z));

							const float Ratio = (TWOPI*RADTODEG*jd->mNbRackTeeth)/(jd->mRackLength*jd->mNbPinionTeeth);
							Exporter.Indent();	CA.StoreASCII(_F("float physics:ratio = %f\n", Ratio));

							if(jd->mHinge)
							{
								const char* Name = GetJointName(jd->mHinge);
								//const char* Name = _F("Joint%d", jd->mHinge->mID);
								Exporter.Indent();
								CA.StoreASCII("rel physics:hinge = </World/").StoreASCII(Name).StoreASCII(">\n");
							}

							if(jd->mPrismatic)
							{
								const char* Name = GetJointName(jd->mPrismatic);
								//const char* Name = _F("Joint%d", jd->mPrismatic->mID);
								Exporter.Indent();
								CA.StoreASCII("rel physics:prismatic = </World/").StoreASCII(Name).StoreASCII(">\n");
							}
						}
						Exporter.EndDef();
					}
					break;

					case PINT_JOINT_D6:
					{
						const D6JointData* jd = static_cast<const D6JointData*>(Current);

/*
				// TODO: export these ones
				float	mMinTwist;
				float	mMaxTwist;
				float	mMaxSwingY;
				float	mMaxSwingZ;
				udword	mMotorFlags;
				float	mMotorStiffness;
				float	mMotorDamping;
*/

						ASSERT(jd->mMinTwist==jd->mMaxTwist);
						ASSERT(jd->mMaxSwingY==0.0f);
						ASSERT(jd->mMaxSwingZ==0.0f);

						AABB ConvertedLinearLimits = jd->mLinearLimits;
						const bool MustExportLimitX = Local::ConvertLimitsPEEL2Kit(ConvertedLinearLimits.mMin.x, ConvertedLinearLimits.mMax.x);
						const bool MustExportLimitY = Local::ConvertLimitsPEEL2Kit(ConvertedLinearLimits.mMin.y, ConvertedLinearLimits.mMax.y);
						const bool MustExportLimitZ = Local::ConvertLimitsPEEL2Kit(ConvertedLinearLimits.mMin.z, ConvertedLinearLimits.mMax.z);

						const char* LimitAPIs[] = { null, null, null, null, null, null };

						udword NbLimitAPIs = 0;
						if(MustExportLimitX)
							LimitAPIs[NbLimitAPIs++] = "PhysicsLimitAPI:transX";
						if(MustExportLimitY)
							LimitAPIs[NbLimitAPIs++] = "PhysicsLimitAPI:transY";
						if(MustExportLimitZ)
							LimitAPIs[NbLimitAPIs++] = "PhysicsLimitAPI:transZ";
						LimitAPIs[NbLimitAPIs++] = "PhysicsLimitAPI:rotX";
						LimitAPIs[NbLimitAPIs++] = "PhysicsLimitAPI:rotY";
						LimitAPIs[NbLimitAPIs++] = "PhysicsLimitAPI:rotZ";

						Exporter.StartPrependDef("PhysicsJoint", GetJointName(Current), "PhysxJointAPI", LimitAPIs[0], LimitAPIs[1], LimitAPIs[2], LimitAPIs[3], LimitAPIs[4], LimitAPIs[5]);
						{
							Local::ExportCommonJointParams(Exporter, CA, Current, jd->mLocalPivot0.mPos, jd->mLocalPivot1.mPos);

							const Quat& q0 = jd->mLocalPivot0.mRot;
							const Quat& q1 = jd->mLocalPivot1.mRot;

							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot0 = (%f, %f, %f, %f)\n", q0.w, q0.p.x, q0.p.y, q0.p.z));
							Exporter.Indent();	CA.StoreASCII(_F("quatf physics:localRot1 = (%f, %f, %f, %f)\n", q1.w, q1.p.x, q1.p.y, q1.p.z));

							if(MustExportLimitX)
							{
								Exporter.Indent();	CA.StoreASCII(_F("float limit:transX:physics:high = %f\n", ConvertedLinearLimits.mMax.x));
								Exporter.Indent();	CA.StoreASCII(_F("float limit:transX:physics:low = %f\n", ConvertedLinearLimits.mMin.x));
							}

							if(MustExportLimitY)
							{
								Exporter.Indent();	CA.StoreASCII(_F("float limit:transY:physics:high = %f\n", ConvertedLinearLimits.mMax.y));
								Exporter.Indent();	CA.StoreASCII(_F("float limit:transY:physics:low = %f\n", ConvertedLinearLimits.mMin.y));
							}

							if(MustExportLimitZ)
							{
								Exporter.Indent();	CA.StoreASCII(_F("float limit:transZ:physics:high = %f\n", ConvertedLinearLimits.mMax.z));
								Exporter.Indent();	CA.StoreASCII(_F("float limit:transZ:physics:low = %f\n", ConvertedLinearLimits.mMin.z));
							}

							Exporter.Indent();	CA.StoreASCII("float limit:rotX:physics:high = -1\n");
							Exporter.Indent();	CA.StoreASCII("float limit:rotX:physics:low = 1\n");
							Exporter.Indent();	CA.StoreASCII("float limit:rotY:physics:high = -1\n");
							Exporter.Indent();	CA.StoreASCII("float limit:rotY:physics:low = 1\n");
							Exporter.Indent();	CA.StoreASCII("float limit:rotZ:physics:high = -1\n");
							Exporter.Indent();	CA.StoreASCII("float limit:rotZ:physics:low = 1\n");
						}
						Exporter.EndDef();
					}
					break;

					default:
					{
						ASSERT(0);
					}
					break;
				}
			}
		}
	}
}

static void ExportInstanced(const EditorPlugin& editor, USDExporter& Exporter, CustomArray& CA)
{
	//####leaks!!!!
	PtrContainer Instanced;
	PtrContainer NonInstanced;

	const udword NbActors = editor.GetNbEditorActors();
	for(udword i=0;i<NbActors;i++)
	{
		const ActorData* Current = editor.GetEditorActor(i);
		if(!CanExport(Current))
			continue;

		bool Status = false;
		{
			const udword NbInstanced = Instanced.GetNbEntries();
			for(udword j=0;j<NbInstanced;j++)
			{
				PtrContainer* PointInstancer = (PtrContainer*)Instanced.GetEntry(j);
				ASSERT(PointInstancer->GetNbEntries());
				const ActorData* Candidate = (const ActorData*)PointInstancer->GetEntry(0);
				if(Compatible(Candidate, Current))
				{
					PointInstancer->AddPtr(Current);
					Status = true;
					break;
				}
			}
		}

		if(!Status)
		{
			const udword NbNonInstanced = NonInstanced.GetNbEntries();
			for(udword j=0;j<NbNonInstanced;j++)
			{
				const ActorData* Candidate = (const ActorData*)NonInstanced.GetEntry(j);
				if(Compatible(Candidate, Current))
				{
					PtrContainer* PointInstancer = ICE_NEW(PtrContainer);
					PointInstancer->AddPtr(Candidate);
					PointInstancer->AddPtr(Current);
					Instanced.AddPtr(PointInstancer);
					NonInstanced.DeleteIndex(j);
					Status = true;
					break;
				}
			}
		}

		if(!Status)
			NonInstanced.AddPtr(Current);
	}

	const Point Color = const_cast<EditorPlugin&>(editor).GetMainColor();

	// Export instanced
	{
		const udword NbInstanced = Instanced.GetNbEntries();
		udword ID = 0;
		for(udword i=0;i<NbInstanced;i++)
		{
			const PtrContainer* CurrentInstancer = (const PtrContainer*)Instanced.GetEntry(i);

			Exporter.StartDef("PointInstancer", _F("pointinstancer%d", ID));
			{
				const udword NbActors = CurrentInstancer->GetNbEntries();

				// Check if we need to export velocities
				bool UsesLinVel = false;
				bool UsesAngVel = false;
				for(udword j=0;j<NbActors;j++)
				{
					const ActorData* CurrentActor = (const ActorData*)CurrentInstancer->GetEntry(j);
					if(CurrentActor->mLinearVelocity.IsNonZero())
						UsesLinVel = true;
					if(CurrentActor->mAngularVelocity.IsNonZero())
						UsesAngVel = true;
					if(UsesLinVel && UsesAngVel)
						break;
				}

				// Export velocities
				if(UsesLinVel)
				{
					Exporter.Indent();	CA.StoreASCII("vector3f[] velocities = [");
					for(udword j=0;j<NbActors;j++)
					{
						const ActorData* CurrentActor = (const ActorData*)CurrentInstancer->GetEntry(j);

						const Point p = GetConvertedPos(CurrentActor->mLinearVelocity, true);
						if(j)
							CA.StoreASCII(", ");
						CA.StoreASCII(_F("(%f, %f, %f)", p.x, p.y, p.z));
					}
					CA.StoreASCII("]\n");
				}

				if(UsesAngVel)
				{
					Exporter.Indent();	CA.StoreASCII("vector3f[] angularVelocities = [");
					for(udword j=0;j<NbActors;j++)
					{
						const ActorData* CurrentActor = (const ActorData*)CurrentInstancer->GetEntry(j);

						const Point p = GetConvertedPos(CurrentActor->mAngularVelocity, true) * RADTODEG;
						if(j)
							CA.StoreASCII(", ");
						CA.StoreASCII(_F("(%f, %f, %f)", p.x, p.y, p.z));
					}
					CA.StoreASCII("]\n");
				}

				// Export orientations
				Exporter.Indent();	CA.StoreASCII("quath[] orientations = [");
				for(udword j=0;j<NbActors;j++)
				{
					const ActorData* CurrentActor = (const ActorData*)CurrentInstancer->GetEntry(j);
					const Quat q = GetConvertedRot(CurrentActor->mRotation, true);
					if(j)
						CA.StoreASCII(", ");
					CA.StoreASCII(_F("(%f, %f, %f, %f)", q.w, q.p.x, q.p.y, q.p.z));
				}
				CA.StoreASCII("]\n");

				// Export positions
				Exporter.Indent();	CA.StoreASCII("point3f[] positions = [");
				for(udword j=0;j<NbActors;j++)
				{
					const ActorData* CurrentActor = (const ActorData*)CurrentInstancer->GetEntry(j);
					const Point p = GetConvertedPos(CurrentActor->mPosition, true);
					if(j)
						CA.StoreASCII(", ");
					CA.StoreASCII(_F("(%f, %f, %f)", p.x, p.y, p.z));
				}
				CA.StoreASCII("]\n");

				// Export proto indices
				Exporter.Indent();	CA.StoreASCII("int[] protoIndices = [");
				for(udword j=0;j<NbActors;j++)
				{
//						const ActorData* CurrentActor = (const ActorData*)CurrentInstancer->GetEntry(j);
					if(j)
						CA.StoreASCII(", ");
					CA.StoreASCII("0");
				}
				CA.StoreASCII("]\n");

			    Exporter.Indent();	CA.StoreASCII(_F("prepend rel prototypes = </World/pointinstancer%d/ProtoActor>\n", ID++));

				const ActorData* Current = (const ActorData*)CurrentInstancer->GetEntry(0);
				Exporter.ExportActor(editor, Current, "ProtoActor", &Color, true, !UsesLinVel, !UsesAngVel);

			}
			Exporter.EndDef();
		}
	}

	// Export non-instanced
	{
		const udword NbNonInstanced = NonInstanced.GetNbEntries();
		udword ID = 0;
		for(udword i=0;i<NbNonInstanced;i++)
		{
			const ActorData* Current = (const ActorData*)NonInstanced.GetEntry(i);

			Current->mID = ID++;

			const char* Name = GetActorName(Current);

			Exporter.ExportActor(editor, Current, Name, &Color, false, true, true);
		}
	}
}

void ExportUSD2(const EditorPlugin& editor, const String* filename);

void ExportUSD(const EditorPlugin& editor, const String* filename)
{
	if(1)
	{
		ExportUSD2(editor, filename);
		return;
	}

	String Filename;
	if(!filename)
	{
		FILESELECTCREATE Create;
		Create.mFilter			= "USDA Files (*.usda)|*.usda|All Files (*.*)|*.*||";
	//	Create.mFileName		= editor.mEditorScene->mName;
		Create.mFileName		= GetFilenameForExport("usda");
		Create.mInitialDir		= "";
		Create.mCaptionTitle	= "Export scene";
		Create.mDefExt			= "usda";

		if(!FileselectSave(Create, Filename, true))
			return;
	}
	else
	{
		Filename = *filename;
	}

	String Path = Filename;
	bool status = _GetPath(Filename, Path);
	ASSERT(status);

	USDExporter Exporter;
	CustomArray& CA = Exporter.mArray;

	Exporter.ExportHeader();

	const SceneData* EditorSceneData = editor.GetEditorSceneData();
	if(EditorSceneData)
		Exporter.ExportSceneData(*EditorSceneData);

	Exporter.ExportMaterials(editor, Path);

	// Export all actors
	if(gUseInstancing)
	{
		ExportInstanced(editor, Exporter, CA);
	}
	else
	{
		ExportAllActors(editor, Exporter);
	}

	if(gExportPhysics)
	{
		ExportAllJoints(editor, Exporter, CA);
	}

	CA.ExportToDisk(Filename, "wb");

	gEvilTmpContainer.Empty();
}





void ExportUSD2(const EditorPlugin& editor, const String* filename)
{
	String Filename;
	if(!filename)
	{
		FILESELECTCREATE Create;
		Create.mFilter			= "USDA Files (*.usda)|*.usda|All Files (*.*)|*.*||";
	//	Create.mFileName		= editor.mEditorScene->mName;
		Create.mFileName		= GetFilenameForExport("usda");
		Create.mInitialDir		= "";
		Create.mCaptionTitle	= "Export scene";
		Create.mDefExt			= "usda";

		if(!FileselectSave(Create, Filename, true))
			return;
	}
	else
	{
		Filename = *filename;
	}

	String Path = Filename;
	bool status = _GetPath(Filename, Path);
	ASSERT(status);

	USDExporter Exporter;
	CustomArray& CA = Exporter.mArray;

	Exporter.ExportHeader();

	Exporter.ExportMaterials(editor, Path);

	Exporter.StartDef("Xform", "World");
	{
		const SceneData* EditorSceneData = editor.GetEditorSceneData();
		if(EditorSceneData)
			Exporter.ExportSceneData(*EditorSceneData);

		if(gUseInstancing)
		{
			ExportInstanced(editor, Exporter, CA);
		}
		else
		{
			ExportAllActors(editor, Exporter);
		}

		if(gExportPhysics)
		{
			ExportAllJoints(editor, Exporter, CA);
		}
	}
	Exporter.EndDef();

	CA.ExportToDisk(Filename, "wb");

	gEvilTmpContainer.Empty();
}

