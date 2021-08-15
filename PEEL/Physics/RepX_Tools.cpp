///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RepX_Tools.h"

///////////////////////////////////////////////////////////////////////////////

namespace
{

struct RepX_ParseContext : public Allocateable
{
				RepX_ParseContext();
				~RepX_ParseContext();

	udword		mLineNumber;
	FILE*		mFP;
	bool		mIsParsingDynamicActors;
};

RepX_ParseContext::RepX_ParseContext() : mLineNumber(0), mFP(null), mIsParsingDynamicActors(false)
{
	mFP = fopen("d:\\test.repx", "w");
}

RepX_ParseContext::~RepX_ParseContext()
{
	if(mFP)
		fclose(mFP);
}

static bool gParseCallback(const char* command, const ParameterBlock& pb, size_t context, void* user_data, const ParameterBlock* cmd)
{
	RepX_ParseContext* Context = (RepX_ParseContext*)user_data;

	Context->mLineNumber++;

	const char* TagStart = strchr(command, '<');

	bool SkipLine = Context->mIsParsingDynamicActors;
	if(TagStart)
	{
		if(_strncoll(TagStart, "<PxRigidDynamic", 15)==0)
		{
			Context->mIsParsingDynamicActors = true;
			SkipLine = true;
		}
		else if(_strncoll(TagStart, "</PxRigidDynamic", 16)==0)
		{
			Context->mIsParsingDynamicActors = false;
			SkipLine = true;
		}
	}

	if(Context->mFP && !SkipLine)
	{
		fwrite(command, strlen(command), 1, Context->mFP);
		const ubyte c = 0xa;
		fwrite(&c, 1, 1, Context->mFP);
	}
	return true;
}

static bool _ProcessRepXFile_RemoveDynamics(const char* filename)
{
	if(!FileExists(filename))
		return false;

	printf("RepX file: %s\n", filename);
	{
		ScriptFile FPS;
		FPS.Disable(BFC_MAKE_LOWER_CASE);
		FPS.Enable(BFC_REMOVE_TABS);
		FPS.Disable(BFC_REMOVE_SEMICOLON);
		FPS.Disable(BFC_DISCARD_COMMENTS);
		FPS.Disable(BFC_DISCARD_UNKNOWNCMDS);
		FPS.Disable(BFC_DISCARD_INVALIDCMDS);
		FPS.Disable(BFC_DISCARD_GLOBALCMDS);

		RepX_ParseContext Context;
		FPS.SetUserData(&Context);
		FPS.SetParseCallback(gParseCallback);
		FPS.Execute(filename);

		printf("%d lines processed\n", Context.mLineNumber);
	}
	return true;
}

}

void ProcessRepXFile_RemoveDynamics(const char* filename)
{
	ASSERT(filename);

	if(!_ProcessRepXFile_RemoveDynamics(_F("../build/%s", filename)))
		if(!_ProcessRepXFile_RemoveDynamics(_F("./%s", filename)))
			printf(_F("Failed to load '%s'\n", filename));
}

///////////////////////////////////////////////////////////////////////////////

