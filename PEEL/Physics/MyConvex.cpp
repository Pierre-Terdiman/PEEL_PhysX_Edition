///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyConvex.h"
#include "Common.h"

MyConvex::MyConvex() :
	mNbVerts	(0),
	mVerts		(NULL),
	mNbPolys	(0),
	mPolys		(NULL)
{
}

MyConvex::~MyConvex()
{
	Release();
}

void MyConvex::Release()
{
	DELETEARRAY(mPolys);
	DELETEARRAY(mVerts);
}

bool MyConvex::LoadFromFile(const char* filename)
{
	if(!filename)
		return false;

//	printf("Trying to load: %s\n", filename);

	FILE* fp = fopen(filename, "rb");
	if(!fp)
		return false;

	fread(&mNbVerts, sizeof(int), 1, fp);

	mVerts = ICE_NEW(Point)[mNbVerts];
	float Scale = 0.0f;
	for(int i=0;i<mNbVerts;i++)
	{
		float vals[3];
		fread(vals, sizeof(float)*3, 1, fp);
		mVerts[i].x = vals[0];
		mVerts[i].y = vals[1];
		mVerts[i].z = vals[2];
		if(fabsf(vals[0])>Scale)	Scale = fabsf(vals[0]);
		if(fabsf(vals[1])>Scale)	Scale = fabsf(vals[1]);
		if(fabsf(vals[2])>Scale)	Scale = fabsf(vals[2]);
	}
	Scale = 1.0f/Scale;
	for(int i=0;i<mNbVerts;i++)
		mVerts[i] *= Scale;

	fread(&mNbPolys, sizeof(int), 1, fp);
	mPolys = ICE_NEW(MyPoly)[mNbPolys];

	for(int i=0;i<mNbPolys;i++)
	{
		fread(&mPolys[i].mNbVerts, sizeof(short), 1, fp);
		mPolys[i].mIndices = new char[mPolys[i].mNbVerts];
		fread(mPolys[i].mIndices, mPolys[i].mNbVerts, 1, fp);
		fread(mPolys[i].mPlane, sizeof(float)*4, 1, fp);
		mPolys[i].mPlane[3] *= Scale;
	}
	fclose(fp);
	return true;
}

bool MyConvex::LoadFile(udword i)
{
	// We have to copy this to a local buffer since FindPEELFile also uses _F()
	char Buffer[64];
	strcpy(Buffer, _F("convex%d.bin", i));
	const char* File = FindPEELFile(Buffer);
	bool status = LoadFromFile(File);
	ASSERT(status);
	if(!status)
		printf("Failing to load convex object!\n");
	return status;
}

void MyConvex::Scale(float s)
{
	for(int i=0;i<mNbVerts;i++)
		mVerts[i] *= s;
}
