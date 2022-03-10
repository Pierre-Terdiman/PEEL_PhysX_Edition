///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader_Obj.h"
#include "Common.h"
#include "SupportFile.h"

static void FixMeshIndices(WavefrontMesh& mesh)
{
	udword NbIndices = mesh.GetNbTris()*3;
	udword* Indices = (udword*)mesh.GetIndices();
	udword MinIndex = MAX_UDWORD;
	for(udword i=0;i<NbIndices;i++)
	{
		if(Indices[i]<MinIndex)
			MinIndex = Indices[i];
	}

	for(udword i=0;i<NbIndices;i++)
	{
		Indices[i]-=MinIndex;
	}

	CustomArray CA;
	udword* src = Indices;
	for(udword i=0;i<mesh.GetNbTris();i++)
	{
		const udword index0 = *src++;
		const udword index1 = *src++;
		const udword index2 = *src++;
		CA.StoreASCII(_F("f %d %d %d\n", index0+1, index1+1, index2+1));
	}
	CA.ExportToDisk("D:/tmp/indices.txt", "wb");
}

WavefrontMesh::WavefrontMesh(const char* string_id, udword id) : mID(id), mPos(0.0f, 0.0f, 0.0f), mUserData(null)
{
	mStringID.Set(string_id);
}

WavefrontMesh::~WavefrontMesh()
{
}

void WavefrontMesh::Recenter()
{
	const udword NbVerts = GetNbVerts();
	Point* V = const_cast<Point*>(mVertices.GetVertices());

	const float Coeff = 1.0f / float(NbVerts);
	Point Center(0.0f, 0.0f, 0.0f);
	for(udword i=0;i<NbVerts;i++)
		Center += V[i]*Coeff;
	for(udword i=0;i<NbVerts;i++)
		V[i] -= Center;
	mPos = Center;
}

namespace
{
	struct ParseContext
	{
		ParseContext(const WavefrontLoaderParams& params, WavefrontDatabase& database) :
			mParams			(params),
			mDatabase		(database),
			mCurrentMesh	(null),
			mIndexBase		(0),
			mCurrentID		(0)
		{
		}

		void	FinishMesh()
		{
			if(0)
				FixMeshIndices(*mCurrentMesh);

			printf("Found OBJ mesh with %d verts and %d tris\n", mCurrentMesh->GetNbVerts(), mCurrentMesh->GetNbTris());

			mCurrentMesh->Recenter();
			mDatabase.mMeshes.AddPtr(mCurrentMesh);
			mIndexBase += mCurrentMesh->GetNbVerts();
			mCurrentMesh = null;
		}

		const WavefrontLoaderParams&	mParams;
		WavefrontDatabase&				mDatabase;
		WavefrontMesh*					mCurrentMesh;
		udword							mIndexBase;
		udword							mCurrentID;

		PREVENT_COPY(ParseContext)
	};
}

static bool gParseCallback(const char* command, const ParameterBlock& pb, size_t context, void* user_data, const ParameterBlock* cmd)
{
	ParseContext* Ctx = (ParseContext*)user_data;

//	const udword NbParams = pb.GetNbParams();

	if(pb[0]=="o")
	{
		if(Ctx->mParams.mMergeMeshes)
		{
			if(!Ctx->mCurrentMesh)
				Ctx->mCurrentMesh = ICE_NEW(WavefrontMesh)(null, Ctx->mCurrentID++);
		}
		else
		{
			if(Ctx->mCurrentMesh)
				Ctx->FinishMesh();
			Ctx->mCurrentMesh = ICE_NEW(WavefrontMesh)(null, Ctx->mCurrentID++);
		}
	}

	//if(NbParams==4)
	{
		if(pb[0]=="v")
		{
			if(!Ctx->mCurrentMesh)
				Ctx->mCurrentMesh = ICE_NEW(WavefrontMesh)(null, Ctx->mCurrentID++);

			const float x = float(::atof(pb[1]));
			const float y = float(::atof(pb[2]));
			const float z = float(::atof(pb[3]));

			const float GlobalScale = Ctx->mParams.mScale;
			Point p(x*GlobalScale, y*GlobalScale, z*GlobalScale);

			p = p * Ctx->mParams.mTransform;

			Ctx->mCurrentMesh->mVertices.AddVertex(p);
		}
		else if(pb[0]=="f")
		{
			if(!Ctx->mCurrentMesh)
				Ctx->mCurrentMesh = ICE_NEW(WavefrontMesh)(null, Ctx->mCurrentID++);

			const udword VRef0 = ::atoi(pb[1]);
			const udword VRef1 = ::atoi(pb[2]);
			const udword VRef2 = ::atoi(pb[3]);
			Ctx->mCurrentMesh->mIndices.Add(VRef0-Ctx->mIndexBase-1);
			Ctx->mCurrentMesh->mIndices.Add(VRef1-Ctx->mIndexBase-1);
			Ctx->mCurrentMesh->mIndices.Add(VRef2-Ctx->mIndexBase-1);
		}
	}
	return true;
}

bool LoadObj(const char* name, const WavefrontLoaderParams& params, WavefrontDatabase& database)
{
	const char* Filename = FindPEELFile(name);
	if(!Filename)
		return false;

	void SetDropFile(const char*);
	SetDropFile(name);

	ParseContext Ctx(params, database);

	ScriptFile2 WavefrontParser;
	WavefrontParser.Enable(BFC_MAKE_LOWER_CASE);
	WavefrontParser.Enable(BFC_REMOVE_TABS);
	WavefrontParser.Disable(BFC_REMOVE_SEMICOLON);
	WavefrontParser.Enable(BFC_DISCARD_COMMENTS);
	WavefrontParser.Disable(BFC_DISCARD_UNKNOWNCMDS);
	WavefrontParser.Disable(BFC_DISCARD_INVALIDCMDS);
	WavefrontParser.Disable(BFC_DISCARD_GLOBALCMDS);

	WavefrontParser.SetUserData(&Ctx);
	WavefrontParser.SetParseCallback(gParseCallback);
	WavefrontParser.Execute(Filename);

	if(Ctx.mCurrentMesh)
		Ctx.FinishMesh();

	return true;
}

WavefrontDatabase::~WavefrontDatabase()
{
	Release();
}

void WavefrontDatabase::Release()
{
	udword NbMeshes = mMeshes.GetNbEntries();
	for(udword i=0;i<NbMeshes;i++)
	{
		WavefrontMesh* CurrentMesh = reinterpret_cast<WavefrontMesh*>(mMeshes[i]);
		DELETESINGLE(CurrentMesh);
	}
	mMeshes.Empty();
}
