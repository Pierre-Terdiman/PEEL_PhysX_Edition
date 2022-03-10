///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader_RepX.h"
#include "SurfaceManager.h"
#include "Pint.h"
#include "PintShapeRenderer.h"
#include "ProgressBar.h"
#include "PintObjectsManager.h"

//#define VALVE_ROTATE45

/*
- we can't use PINT classes directly since the RepX format uses additional data (e.g. IDs) that are not available in PINT classes. So we'd need
companion classes for each PINT class, at which point it's just easier to keep a single class and not bother with PINT at all.
*/

namespace
{

enum CurrentData
{
	DATA_UNDEFINED,
	//
	DATA_DWORD,
	DATA_PR,
	//
	DATA_VERTICES,
	DATA_INDICES,
};

#define MAX_DEPTH	8

///////////////////////////////////////////////////////////////////////////////

struct SurfaceData : public Allocateable
{
						SurfaceData();

	udword				mID;
	IndexedSurface*		mSurface;
	PintShapeRenderer*	mRenderer;
};

SurfaceData::SurfaceData() : mID(INVALID_ID), mSurface(null), mRenderer(null)
{
}

///////////////////////////////////////////////////////////////////////////////

// Let's try a GeometryUnion kind of thing for now
struct ShapeData : public Allocateable
{
						ShapeData();

	PintShape			mType;
	// Shared data
	PR					mLocalPose;
	ShapeData*			mNext;
//	PintShapeRenderer*	mRenderer;
	// Mesh data:
	udword				mMeshID;
};

ShapeData::ShapeData() : mType(PINT_SHAPE_UNDEFINED), mNext(null), /*mRenderer(null),*/ mMeshID(INVALID_ID)
{
	mLocalPose.Identity();
}

///////////////////////////////////////////////////////////////////////////////

struct ActorData : public Allocateable
{
					ActorData();

	PR				mGlobalPose;
	ShapeData*		mShape;
};

ActorData::ActorData() : mShape(null)
{
	mGlobalPose.Identity();
}

}

///////////////////////////////////////////////////////////////////////////////

struct RepX_ParseContext : public Allocateable
{
	RepX_ParseContext(PtrContainer& surfaces, float scale, bool z_is_up) :
		mLineNumber		(0),
		mExpectedNbLines(0),
		mCurrentProgress(0),
		mSurfaces		(surfaces),
		mCurrentSurface	(null),
		mCurrentMeshID	(INVALID_ID),
		mCurrentData	(DATA_UNDEFINED),
		mCurrentActor	(null),
		mCurrentShape	(null),
		mNbTargetDword	(0),
		mNbTargetPR		(0),
		mScale			(scale),
		mSwapYZ			(z_is_up),
		mGeometry		(false)
		{
		}

	~RepX_ParseContext();

	udword				mLineNumber;
	udword				mExpectedNbLines;
	udword				mCurrentProgress;
	PtrContainer&		mSurfaces;

	Container			mSurfaceData;
	PtrContainer		mActorData;
	PtrContainer		mShapeData;

	IndexedSurface*		mCurrentSurface;
	Container			mCurrentVerts;
	Container			mCurrentIndices;
	udword				mCurrentMeshID;

	CurrentData			mCurrentData;

	ActorData*			mCurrentActor;
	ShapeData*			mCurrentShape;

	udword				mNbTargetDword;
	udword				mTargetDword[MAX_DEPTH];

	udword				mNbTargetPR;
	PR					mTargetPR[MAX_DEPTH];

	float				mScale;
	bool				mSwapYZ;

	bool				mGeometry;

	const SurfaceData*	GetSurfaceByID(udword id)	const;
	bool				ProcessTriangleMeshTag(const char* tag);
	bool				ProcessGeometryTag(const char* tag);
	bool				ProcessShapeTag(const char* tag);
	bool				ProcessActorTag(const char* tag);
	bool				ProcessTag(const char* tag);
	void				ProcessData(const char* data);
	void				CreateRenderers();

	PREVENT_COPY(RepX_ParseContext)
};

RepX_ParseContext::~RepX_ParseContext()
{
	ASSERT(!mNbTargetPR);
	ASSERT(!mNbTargetDword);
	ASSERT(!mGeometry);
	ASSERT(!mCurrentSurface);
	ASSERT(!mCurrentActor);
	ASSERT(!mCurrentShape);
	ASSERT(mCurrentData==DATA_UNDEFINED);

	if(0)
	{
		const udword Nb = mActorData.GetNbEntries();
		printf("%d actors:\n", Nb);
		for(udword i=0;i<Nb;i++)
		{
			ActorData* Data = (ActorData*)mActorData.GetEntry(i);
			udword NbShapes = 0;
			ShapeData* Current = Data->mShape;
			while(Current)
			{
				NbShapes++;
				Current = Current->mNext;
			}
			printf("  %d shapes\n", NbShapes);
		}
	}

	{
		const udword Nb = mShapeData.GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			ShapeData* Data = (ShapeData*)mShapeData.GetEntry(i);
			DELETESINGLE(Data);
		}
	}

	{
		const udword Nb = mActorData.GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			ActorData* Data = (ActorData*)mActorData.GetEntry(i);
			DELETESINGLE(Data);
		}
	}
}

void RepX_ParseContext::CreateRenderers()
{
	const udword Nb = mShapeData.GetNbEntries();
	CreateProgressBar(Nb, _F("Creating %d renderers...", Nb));
	for(udword i=0;i<Nb;i++)
	{
		SetProgress(i);
		const ShapeData* CurrentShape = (const ShapeData*)mShapeData.GetEntry(i);

		if(CurrentShape->mType == PINT_SHAPE_MESH)
		{
			const SurfaceData* SD = GetSurfaceByID(CurrentShape->mMeshID);
			ASSERT(SD);
			if(SD)
				const_cast<SurfaceData*>(SD)->mRenderer = CreateMeshRenderer(PintSurfaceInterface(SD->mSurface->GetSurfaceInterface()));
		}
	}
	ReleaseProgressBar();
}

const SurfaceData* RepX_ParseContext::GetSurfaceByID(udword id) const
{
	const udword NbSurfaces = mSurfaceData.GetNbEntries()/(sizeof(SurfaceData)/sizeof(udword));
	const SurfaceData* Data = (const SurfaceData*)mSurfaceData.GetEntries();
	for(udword i=0;i<NbSurfaces;i++)
	{
		if(Data[i].mID==id)
		{
			return &Data[i];
		}
	}
	return null;
}

static inline_ bool _IsFinite(float f)
{
	return (0 == ((_FPCLASS_SNAN | _FPCLASS_QNAN | _FPCLASS_NINF | _FPCLASS_PINF) & _fpclass(f) ));
}

// Process tags related to <PxTriangleMesh>
bool RepX_ParseContext::ProcessTriangleMeshTag(const char* tag)
{
	ASSERT(mCurrentSurface);

	if(strcmp(tag, "</PxTriangleMesh>")==0)
	{
		const udword NbVerts = mCurrentVerts.GetNbEntries()/3;
		/*const*/ udword NbFaces = mCurrentIndices.GetNbEntries()/3;

		Point* V = (Point*)mCurrentVerts.GetEntries();
		IndexedTriangle* I = (IndexedTriangle*)mCurrentIndices.GetEntries();

		if(mSwapYZ)
		{
#ifdef VALVE_ROTATE45
			Matrix3x3 Rot;
			Rot.RotX(45.0f * DEGTORAD);
#endif
			for(udword i=0;i<NbVerts;i++)
			{
				TSwap(V[i].y, V[i].z);
#ifdef VALVE_ROTATE45
				V[i] *= Rot;
#endif
			}

			for(udword i=0;i<NbFaces;i++)
			{
				TSwap(I[i].mRef[1], I[i].mRef[2]);
			}
		}

		if(0)
		{
			float dmax = MIN_FLOAT;
			udword NbRemoved = 0;
			udword i = 0;
			//for(udword i=0;i<NbFaces;i++)
			while(i<NbFaces)
			{
				const udword i0 = I[i].mRef[0];
				const udword i1 = I[i].mRef[1];
				const udword i2 = I[i].mRef[2];
				const Point& p0 = V[i0];
				const Point& p1 = V[i1];
				const Point& p2 = V[i2];
				float d0 = p0.Distance(p1);
				float d1 = p1.Distance(p2);
				float d2 = p2.Distance(p0);
				float d = MAX(d0, d1);
				d = MAX(d, d2);
				if(d>dmax)
				{
					dmax = d;
				}
				if(d>50.0f)
//				if(d>100.0f)
				{
					I[i] = I[--NbFaces];
					NbRemoved++;
				}
				else i++;
			}
			i=0;
		}

		const bool Status = mCurrentSurface->Init(NbFaces, NbVerts, V, I);
		ASSERT(Status);

		SurfaceData* Memory = ICE_RESERVE(SurfaceData, mSurfaceData);
		Memory->mID = mCurrentMeshID;
		Memory->mSurface = mCurrentSurface;
//		printf("Surface %d with %d verts and %d faces\n", mCurrentMeshID, NbVerts, NbFaces);
		Memory->mRenderer = null;

		mCurrentSurface = null;
		mCurrentVerts.Reset();
		mCurrentIndices.Reset();
		mCurrentMeshID = INVALID_ID;
	}
	else if(strcmp(tag, "<Id>")==0)
	{
		mCurrentData = DATA_DWORD;
		mNbTargetDword++;
	}
	else if(strcmp(tag, "</Id>")==0)
	{
		ASSERT(mNbTargetDword);
		mNbTargetDword--;
		mCurrentMeshID = mTargetDword[mNbTargetDword];
		mCurrentData = DATA_UNDEFINED;
	}
	else if(strcmp(tag, "<Points>")==0)
	{
		mCurrentData = DATA_VERTICES;
	}
	else if(strcmp(tag, "</Points>")==0)
	{
		mCurrentData = DATA_UNDEFINED;
	}
	else if(strcmp(tag, "<Triangles>")==0)
	{
		mCurrentData = DATA_INDICES;
	}
	else if(strcmp(tag, "</Triangles>")==0)
	{
		mCurrentData = DATA_UNDEFINED;
	}
	return true;
}

// Process tags related to <PxRigidStatic>
bool RepX_ParseContext::ProcessActorTag(const char* tag)
{
	ASSERT(mCurrentActor);

	// If we're already parsing a shape, limit our search to supported shape tags
	if(mCurrentShape)
		return ProcessShapeTag(tag);

	if(strcmp(tag, "</PxRigidStatic>")==0)
	{
		mActorData.AddPtr(mCurrentActor);
		mCurrentActor = null;
	}
	else if(strcmp(tag, "<GlobalPose>")==0)
	{
		mCurrentData = DATA_PR;
		mNbTargetPR++;
	}
	else if(strcmp(tag, "</GlobalPose>")==0)
	{
		ASSERT(mNbTargetPR);
		mNbTargetPR--;
		mCurrentActor->mGlobalPose = mTargetPR[mNbTargetPR];
		mCurrentData = DATA_UNDEFINED;
	}
	else if(strcmp(tag, "<PxShape>")==0)
	{
		mCurrentShape = ICE_NEW(ShapeData);
	}
	return true;
}

// Process tags related to <PxShape>
bool RepX_ParseContext::ProcessShapeTag(const char* tag)
{
	ASSERT(mCurrentActor);
	ASSERT(mCurrentShape);

	// If we're already parsing a geometry, limit our search to supported geometry tags
	if(mGeometry)
		return ProcessGeometryTag(tag);

	if(strcmp(tag, "</PxShape>")==0)
	{
		mCurrentShape->mNext = mCurrentActor->mShape;
		mCurrentActor->mShape = mCurrentShape;

/*		if(mCurrentShape->mType==PINT_SHAPE_UNDEFINED)
		{
			int stop=1;
		}*/

		mShapeData.AddPtr(mCurrentShape);
		mCurrentShape = null;
	}
	else if(strcmp(tag, "<LocalPose>")==0)
	{
		mCurrentData = DATA_PR;
		mNbTargetPR++;
	}
	else if(strcmp(tag, "</LocalPose>")==0)
	{
		ASSERT(mNbTargetPR);
		mNbTargetPR--;
		mCurrentShape->mLocalPose = mTargetPR[mNbTargetPR];
		mCurrentData = DATA_UNDEFINED;
	}
	else if(strcmp(tag, "<Geometry>")==0)
	{
		mGeometry = true;
	}
	return true;
}

// Process tags related to <Geometry>
bool RepX_ParseContext::ProcessGeometryTag(const char* tag)
{
	ASSERT(mCurrentActor);
	ASSERT(mCurrentShape);
	ASSERT(mGeometry);

	if(strcmp(tag, "</Geometry>")==0)
	{
		mGeometry = false;
	}
	else if(strcmp(tag, "<PxTriangleMeshGeometry>")==0)
	{
		ASSERT(mCurrentShape->mType==PINT_SHAPE_UNDEFINED);
		mCurrentShape->mType=PINT_SHAPE_MESH;
	}
	else if(strcmp(tag, "</PxTriangleMeshGeometry>")==0)
	{
		ASSERT(mCurrentShape->mType==PINT_SHAPE_MESH);

		// ### missing local pose!
/*		if(mCurrentID!=INVALID_ID)
		{
			const udword NbSurfaces = mSurfaceData.GetNbEntries()/(sizeof(SurfaceData)/sizeof(udword));
			const SurfaceData* Data = (const SurfaceData*)mSurfaceData.GetEntries();
			for(udword i=0;i<NbSurfaces;i++)
			{
				if(Data[i].mID==mCurrentID)
				{
					const Matrix4x4 M = mCurrentActorPose;
					const udword NbVerts = Data[i].mSurface->GetNbVerts();
					Point* V = Data[i].mSurface->GetVerts();
					for(udword j=0;j<NbVerts;j++)
					{
						V[j] *= M;
					}
					break;
				}
			}
// ########### meshes are instanced! So we can't just multiply the vertices like this
// ########### swap missing on local/global poses
//printf("%d\n", mCurrentID);
			mCurrentID=INVALID_ID;
		}*/
	}
	else if(strcmp(tag, "<TriangleMesh>")==0)
	{
		ASSERT(mCurrentShape->mType==PINT_SHAPE_MESH);
		mCurrentData = DATA_DWORD;
		mNbTargetDword++;
	}
	else if(strcmp(tag, "</TriangleMesh>")==0)
	{
		ASSERT(mCurrentShape->mType==PINT_SHAPE_MESH);
		ASSERT(mNbTargetDword);
		mNbTargetDword--;
		mCurrentShape->mMeshID = mTargetDword[mNbTargetDword];
		mCurrentData = DATA_UNDEFINED;
	}
	return true;
}

bool RepX_ParseContext::ProcessTag(const char* tag)
{
	// If we're already parsing an actor, limit our search to supported actor tags
	if(mCurrentActor)
		return ProcessActorTag(tag);

	// If we're already parsing a triangle mesh, limit our search to supported triangle mesh tags
	if(mCurrentSurface)
		return ProcessTriangleMeshTag(tag);

	// Otherwise look for supported entry tags.
	if(strcmp(tag, "<PxRigidStatic>")==0)
	{
		mCurrentActor = ICE_NEW(ActorData);
	}
	else if(strcmp(tag, "<PxTriangleMesh>")==0)
	{
		IndexedSurface* IS = ICE_NEW(TrackedIndexedSurface);
		mSurfaces.AddPtr(IS);
		mCurrentSurface = IS;
	}
	return true;
}

static bool GetPose(Quat& rot, Point& pos, const char* data, float scale)
{
	ParameterBlock PB(data);
	const udword NbParams = PB.GetNbParams();
	ASSERT(NbParams==7);
	if(NbParams!=7)
		return false;

	rot.p.x = float(::atof(PB[0]));
	rot.p.y = float(::atof(PB[1]));
	rot.p.z = float(::atof(PB[2]));
	rot.w = float(::atof(PB[3]));
	pos.x = float(::atof(PB[4]) * scale);
	pos.y = float(::atof(PB[5]) * scale);
	pos.z = float(::atof(PB[6]) * scale);
	return true;
}

void RepX_ParseContext::ProcessData(const char* _data)
{
	if(mCurrentData==DATA_UNDEFINED)
		return;

	char data[1024];
	udword i=0;
	while(*_data)
	{
		const char c = *_data++;
		if(c!=ASCII_TAB)
			data[i++] = c;
	}
	data[i] = 0;

	if(mCurrentData==DATA_DWORD)
	{
		ParameterBlock PB(data);
		const udword NbParams = PB.GetNbParams();
		ASSERT(NbParams==1);
		ASSERT(mNbTargetDword);
		mTargetDword[mNbTargetDword-1] = (udword)(sdword)PB[0];
	}
	else if(mCurrentData==DATA_VERTICES)
	{
		ParameterBlock PB(data);
		const udword NbParams = PB.GetNbParams();
		for(udword i=0;i<NbParams;i++)
		{
//			const float f = PB[i];
			const float f = float(::atof(PB[i]));
			ASSERT(_IsFinite(f));
			ASSERT(f<20000.0f);
			mCurrentVerts.Add(f*mScale);
		}
	}
	else if(mCurrentData==DATA_INDICES)
	{
		ParameterBlock PB(data);
		const udword NbParams = PB.GetNbParams();
		for(udword i=0;i<NbParams;i++)
		{
			const sdword d = PB[i];
			mCurrentIndices.Add(udword(d));
		}
	}
	else if(mCurrentData==DATA_PR)
	{
		PR tmp;
		GetPose(tmp.mRot, tmp.mPos, data, mScale);

		ASSERT(mNbTargetPR);
		mTargetPR[mNbTargetPR-1] = tmp;
	}
}

//static bool gParseCallback(const char* command, const ParameterBlock& pb, udword context, void* user_data, const ParameterBlock* cmd)
static bool gParseCallback(const char* command, void* user_data)
{
	RepX_ParseContext* Context = (RepX_ParseContext*)user_data;

/*	if(Context->mLineNumber==298425)
	{
		int stop=1;
	}*/

	if(Context->mExpectedNbLines)
	{
		const udword CurrentProgress = udword(100.0f * float(Context->mLineNumber)/float(Context->mExpectedNbLines));
		if(CurrentProgress!=Context->mCurrentProgress)
		{
			Context->mCurrentProgress = CurrentProgress;
			SetProgress(CurrentProgress);
		}
	}
	Context->mLineNumber++;

	const char* TagStart = strchr(command, '<');
	if(TagStart)
	{
		if(TagStart!=command)
		{
			*const_cast<char*>(TagStart) = 0;
			Context->ProcessData(command);
			*const_cast<char*>(TagStart) = '<';
		}
ProcessMore:
		char CleanedTag[256];
		const char* TagEnd = TagStart;
		udword i=0;
		while(*TagEnd!='>')
		{
			const char c = *TagEnd++;
			if(c!=' ')
				CleanedTag[i++] = c;
		}
		CleanedTag[i] = *TagEnd++;
		CleanedTag[i+1] = 0;
		ASSERT(i<256);
		Context->ProcessTag(CleanedTag);

		const bool Eol = *TagEnd==0;
		if(!Eol)
		{
			TagStart = strchr(TagEnd, '<');
			if(TagStart)
			{
				*const_cast<char*>(TagStart) = 0;
				Context->ProcessData(TagEnd);
				*const_cast<char*>(TagStart) = '<';
				goto ProcessMore;
			}
			else Context->ProcessData(TagEnd);
		}
	}
	else Context->ProcessData(command);

	return true;
}

static bool LoadRepX_Obsolete(const char* filename, PtrContainer& surfaces, float scale, bool z_is_up)
{
	if(!FileExists(filename))
		return false;

	CreateProgressBar(100, "Parsing RepX file...");
	{
		ScriptFile FPS;
		FPS.Enable(BFC_MAKE_LOWER_CASE);
		FPS.Enable(BFC_REMOVE_TABS);
		FPS.Disable(BFC_REMOVE_SEMICOLON);
		FPS.Enable(BFC_DISCARD_COMMENTS);
		FPS.Disable(BFC_DISCARD_UNKNOWNCMDS);
		FPS.Disable(BFC_DISCARD_INVALIDCMDS);
		FPS.Disable(BFC_DISCARD_GLOBALCMDS);

		RepX_ParseContext Context(surfaces, scale, z_is_up);
		FPS.SetUserData(&Context);
//		FPS.SetParseCallback(gParseCallback);
		FPS.SetRawParseCallback(gParseCallback);
		FPS.Execute(filename);
	}
	ReleaseProgressBar();
	return true;
}

void LoadRepXFile_Obsolete(PtrContainer& surfaces, const char* filename, float scale, bool z_is_up)
{
	ASSERT(filename);

	const char* File = FindPEELFile(filename);
	if(!File || !LoadRepX_Obsolete(File, surfaces, scale, z_is_up))
//	if(!File || !LoadRepX_Obsolete(_F("../build/%s", filename), test, scale, z_is_up))
		printf(_F("Failed to load '%s'\n", filename));

//	if(!LoadRepX_Obsolete(_F("../build/%s", filename), test, scale, z_is_up))
//		if(!LoadRepX_Obsolete(_F("./%s", filename), test, scale, z_is_up))
//			printf(_F("Failed to load '%s'\n", filename));
}

///////////////////////////////////////////////////////////////////////////////

	class RepXScene : public Allocateable
	{
		public:
							RepXScene(float scale, bool z_is_up);
							~RepXScene();

		PtrContainer		mSurfaces;
		RepX_ParseContext	mContext;

		PREVENT_COPY(RepXScene)
	};

RepXScene::RepXScene(float scale, bool z_is_up) : mContext(mSurfaces, scale, z_is_up)
{
}

RepXScene::~RepXScene()
{
	DeleteOwnedObjects<IndexedSurface>(mSurfaces);
}

void* CreateRepXContext(const char* filename, float scale, bool z_is_up)
{
	ASSERT(filename);

/*	const char* Filename = _F("../build/%s", filename);
	if(!FileExists(Filename))
	{
		Filename = _F("./%s", filename);
		if(!FileExists(Filename))
		{
			printf(_F("Failed to load '%s'\n", filename));
			return null;
		}
	}*/
	const char* Filename = FindPEELFile(filename);
	if(!Filename)
	{
		printf(_F("Failed to load '%s'\n", filename));
		return null;
	}

	char Buffer[1024];
	strcpy(Buffer, Filename);
	strcat(Buffer, ".meta");

	udword Count = 0;
	{
		IceFile MetaFile(Buffer);
		if(MetaFile.IsValid())
		{
			udword Version  = MetaFile.LoadDword();
			(void)Version;
			Count  = MetaFile.LoadDword();
		}
	}

	RepXScene* Scene = ICE_NEW(RepXScene)(scale, z_is_up);
	Scene->mContext.mExpectedNbLines = Count;

	CreateProgressBar(100, "Parsing RepX file...");
	{
		ScriptFile FPS;
		FPS.Enable(BFC_MAKE_LOWER_CASE);
		FPS.Enable(BFC_REMOVE_TABS);
		FPS.Disable(BFC_REMOVE_SEMICOLON);
		FPS.Enable(BFC_DISCARD_COMMENTS);
		FPS.Disable(BFC_DISCARD_UNKNOWNCMDS);
		FPS.Disable(BFC_DISCARD_INVALIDCMDS);
		FPS.Disable(BFC_DISCARD_GLOBALCMDS);

		FPS.SetUserData(&Scene->mContext);
//		FPS.SetParseCallback(gParseCallback);
		FPS.SetRawParseCallback(gParseCallback);
		FPS.Execute(Filename);
	}
	ReleaseProgressBar();

	if(Scene->mContext.mLineNumber!=Count)
	{
		CustomArray CA;
		CA.Store(udword(0)).Store(Scene->mContext.mLineNumber);
		CA.ExportToDisk(Buffer, "wb");
	}

	Scene->mContext.CreateRenderers();

	return Scene;
}

void ReleaseRepXContext(void* repx_context)
{
	RepXScene* Scene = (RepXScene*)repx_context;
	DELETESINGLE(Scene);
}

bool AddToPint(Pint& pint, void* repx_context)
{
	RepXScene* Scene = (RepXScene*)repx_context;
	if(!Scene)
		return false;

//	if(Regular)
	{
		// Parse all actors, create PINT counterpart

		DWORD time = TimeGetTime();

		const udword MAX_DESC = 16;
		udword NbMeshShapes = 0;
		PINT_MESH_CREATE MeshDesc[MAX_DESC];

		const udword Nb = Scene->mContext.mActorData.GetNbEntries();
		CreateProgressBar(Nb, _F("%s: creating %d actors", pint.GetName(), Nb));
		for(udword i=0;i<Nb;i++)
		{
			SetProgress(i);

			const ActorData* Data = (const ActorData*)Scene->mContext.mActorData.GetEntry(i);

			NbMeshShapes = 0;

			PINT_OBJECT_CREATE ObjectDesc;
			ObjectDesc.mPosition	= Data->mGlobalPose.mPos;
			ObjectDesc.mRotation	= Data->mGlobalPose.mRot;
			ObjectDesc.mMass		= 0.0f;

			ShapeData* CurrentShape = Data->mShape;
			while(CurrentShape)
			{
				if(CurrentShape->mType==PINT_SHAPE_MESH)
				{
					ASSERT(NbMeshShapes<MAX_DESC);

					const SurfaceData* SD = Scene->mContext.GetSurfaceByID(CurrentShape->mMeshID);
					ASSERT(SD);

					MeshDesc[NbMeshShapes].SetSurfaceData(SD->mSurface->GetSurfaceInterface());
					MeshDesc[NbMeshShapes].mRenderer = SD->mRenderer;//CreateMeshRenderer(MeshDesc[NbMeshShapes].mSurface);

					MeshDesc[NbMeshShapes].mLocalPos = CurrentShape->mLocalPose.mPos;
					MeshDesc[NbMeshShapes].mLocalRot = CurrentShape->mLocalPose.mRot;

					MeshDesc[NbMeshShapes].SetNext(ObjectDesc.GetFirstShape());
					ObjectDesc.SetShape(&MeshDesc[NbMeshShapes]);
					NbMeshShapes++;
				}
//				else ASSERT(0);

				CurrentShape = CurrentShape->mNext;
			}

			CreatePintObject(pint, ObjectDesc);
		}
		ReleaseProgressBar();

		time = TimeGetTime() - time;
		printf("Mesh creation time: %d (%s)\n", time, pint.GetName());
	}
	return true;
}
