///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextureManager.h"
#include "GLTexture.h"

extern	bool			gUseEditor;
static	PtrContainer*	gManagedTextures = null;
static	PtrContainer*	gSystemTextures = null;
static	udword			gMT_Timestamp = 0;
static	udword			gST_Timestamp = 0;

///////////////////////////////////////////////////////////////////////////////

ManagedTexture::ManagedTexture(udword width, udword height, const RGBAPixel* pixels, const char* filename, udword index) :
	mWidth		(width),
	mHeight		(height),
	mGLID		(0),
	mID			(index),
	mExportID	(INVALID_ID)
{
	if(filename)
		mFilename.Set(filename);

	mGLID = GLTexture::CreateTexture(width, height, pixels, true);

	mSource = gUseEditor ? ICE_NEW(Picture)(ToWord(width), ToWord(height), pixels) : null;
}

ManagedTexture::~ManagedTexture()
{
	DELETESINGLE(mSource);
	GLTexture::ReleaseTexture(mGLID);
	mGLID = 0;
	mExportID = INVALID_ID;
	mWidth = mHeight = 0;
}

///////////////////////////////////////////////////////////////////////////////

udword GetNbManagedTextures()
{
	return gManagedTextures ? gManagedTextures->GetNbEntries() : 0;
}

const ManagedTexture* GetManagedTexture(udword index)
{
	if(index<GetNbManagedTextures())
		return reinterpret_cast<const ManagedTexture*>(gManagedTextures->GetEntry(index));
	return null;
}

const ManagedTexture* CreateManagedTexture(udword width, udword height, const RGBAPixel* pixels, const char* filename)
{
	if(!gManagedTextures)
		gManagedTextures = ICE_NEW(PtrContainer);

	const udword Index = GetNbManagedTextures();
	ManagedTexture* MT = ICE_NEW(ManagedTexture)(width, height, pixels, filename, Index);
	gManagedTextures->AddPtr(MT);
	gMT_Timestamp++;
	return MT;
}

bool ReleaseManagedTexture(const ManagedTexture* texture)
{
	if(!texture || !gManagedTextures)
		return false;
	const udword Nb = GetNbManagedTextures();
	if(texture->mID>=Nb)
		return false;

	ManagedTexture** Ptrs = (ManagedTexture**)gManagedTextures->GetEntries();
	ManagedTexture* T = (ManagedTexture*)Ptrs[texture->mID];
	if(T!=texture)
		return false;

	if(T->mID!=Nb-1)
	{
		Ptrs[T->mID] = Ptrs[Nb-1];
		Ptrs[T->mID]->mID = T->mID;
	}
	gManagedTextures->ForceSize(Nb-1);
	DELETESINGLE(texture);
	gMT_Timestamp++;
	return true;
}

void ReleaseManagedTextures()
{
	if(gManagedTextures)
	{
		const udword Nb = gManagedTextures->GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			ManagedTexture* T = reinterpret_cast<ManagedTexture*>(gManagedTextures->GetEntry(i));
			DELETESINGLE(T);
		}
		DELETESINGLE(gManagedTextures);
	}
	gMT_Timestamp++;
}

udword GetManagedTexturesTimestamp()
{
	return gMT_Timestamp;
}

///////////////////////////////////////////////////////////////////////////////

SystemTexture* CreateSystemTexture(udword width, udword height, GLuint id, const char* name)
{
	SystemTexture* ST = ICE_NEW(SystemTexture);
	ST->mWidth	= width;
	ST->mHeight	= height;
	ST->mGLID	= id;
	ST->mName.Set(name);

	if(!gSystemTextures)
		gSystemTextures = ICE_NEW(PtrContainer);
	gSystemTextures->AddPtr(ST);

	gST_Timestamp++;
	return ST;
}

udword GetNbSystemTextures()
{
	return gSystemTextures ? gSystemTextures->GetNbEntries() : 0;
}

const SystemTexture* GetSystemTexture(udword index)
{
	if(index<GetNbSystemTextures())
		return reinterpret_cast<const SystemTexture*>(gSystemTextures->GetEntry(index));
	return null;
}

void ReleaseSystemTextures()
{
	if(gSystemTextures)
	{
		const udword Nb = gSystemTextures->GetNbEntries();
		for(udword i=0;i<Nb;i++)
		{
			SystemTexture* T = reinterpret_cast<SystemTexture*>(gSystemTextures->GetEntry(i));
			DELETESINGLE(T);
		}
		DELETESINGLE(gSystemTextures);
	}
	gST_Timestamp++;
}

udword GetSystemTexturesTimestamp()
{
	return gST_Timestamp;
}

