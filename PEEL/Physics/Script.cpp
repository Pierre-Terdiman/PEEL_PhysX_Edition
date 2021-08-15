///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Script.h"
#include "TestScenes.h"
#include "Sound.h"
#include "PEEL.h"
#include "CameraManager.h"
#include "Tool.h"

CHECK_CONTAINER_ITEM(AutomatedTest)

static AutomatedTests* gAutomatedTests = null;

AutomatedTests* GetAutomatedTests()
{
	return gAutomatedTests;
}

void ReleaseAutomatedTests()
{
	DELETESINGLE(gAutomatedTests);
}

struct ParseContext
{
	ParseContext() :
		mCurrentTest	(null),
		mNbFrames		(0),
		mRendering		(false),
		mRandomizeOrder	(false),
		mTrashCache		(false),
		mSaveExcelFile	(true)
	{
	}

	inline_	AutomatedTest*	AllocateNewTest(PhysicsTest* test)
	{
		AutomatedTest* AT = ICE_RESERVE(AutomatedTest, mTests);
		AT->Init(test);
		return AT;
	}

	AutomatedTest*	mCurrentTest;
	Container		mTests;
	udword			mNbFrames;
	bool			mRendering;
	bool			mRandomizeOrder;
	bool			mTrashCache;
	bool			mSaveExcelFile;
};

AutomatedTests::AutomatedTests(const ParseContext& ctx) :
	mTests			(ctx.mTests),
	mDefaultNbFrames(ctx.mNbFrames),
	mIndex			(0),
	mRendering		(ctx.mRendering),
	mRandomizeOrder	(ctx.mRandomizeOrder),
	mTrashCache		(ctx.mTrashCache),
	mSaveExcelFile	(ctx.mSaveExcelFile)
{
}

AutomatedTests::~AutomatedTests()
{
	AutomatedTest* ATs = reinterpret_cast<AutomatedTest*>(mTests.GetEntries());
	const udword N = sizeof(AutomatedTest)/sizeof(udword);
	const udword NbTests = mTests.GetNbEntries()/N;
	for(udword i=0;i<NbTests;i++)
	{
		DELETESINGLE(ATs[i].mSubScene);
	}
}

bool AutomatedTests::IsValid() const
{
	return mDefaultNbFrames!=0 && mTests.GetNbEntries()!=0;
}

AutomatedTest* AutomatedTests::GetCurrentTest() const
{
	if(!mTests.GetNbEntries())
		return null;

/*	if(mIndex>=mTests.GetNbEntries())
		return null;

	return (PhysicsTest*)mTests.GetEntry(mIndex);*/

	const udword N = sizeof(AutomatedTest)/sizeof(udword);
	const udword NbTests = mTests.GetNbEntries()/N;
	if(mIndex>=NbTests)
		return null;

	AutomatedTest* AT = reinterpret_cast<AutomatedTest*>(mTests.GetEntries());
	return AT + mIndex;
}

AutomatedTest* AutomatedTests::SelectNextTest()
{
	mIndex++;
	return GetCurrentTest();
}

///////////////////////////////////////////////////////////////////////////////

static PhysicsTest* FindTest(const char* name)
{
	const udword NbTests = GetNbTests();
	for(udword i=0;i<NbTests;i++)
	{
		PhysicsTest* CurrentTest = GetTest(i);
		if(strcmp(CurrentTest->GetName(), name)==0)
			return CurrentTest;
	}
	return null;
}

static bool gParseCallback(const char* command, const ParameterBlock& pb, size_t context, void* user_data, const ParameterBlock* cmd)
{
	ParseContext* Context = reinterpret_cast<ParseContext*>(user_data);

	if(pb.GetNbParams()>=2 && pb[0]=="Test")
	{
		const char* TestName = pb[1];
		PhysicsTest* Test = FindTest(TestName);
		if(!Test)
			OutputConsoleError(_F("Unknown test in script:\n%s\n", TestName));
		else
		{
			AutomatedTest* AT = Context->AllocateNewTest(Test);
			if(pb.GetNbParams()==3)
				AT->mNbFrames = (sdword)pb[2];
			else
				AT->mNbFrames = 0;
			Context->mCurrentTest = AT;
		}
	}
	else if(pb.GetNbParams()==3 && pb[0]=="Part")
	{
		const char* TestName = pb[1];
		PhysicsTest* Test = FindTest(TestName);
		if(!Test)
			OutputConsoleError(_F("Unknown test in script:\n%s\n", TestName));
		else
		{
			AutomatedTest* AT = Context->AllocateNewTest(Test);
			AT->mSoundPos = (sdword)pb[2];
			Context->mCurrentTest = AT;
		}
	}
	else if(pb.GetNbParams()==3 && pb[0]=="Music")
	{
		StartSound(pb[1], (sdword)pb[2]);
	}
	else if(pb.GetNbParams()==2 && pb[0]=="NbFrames")
	{
		Context->mNbFrames = (sdword)pb[1];
	}
	else if(pb.GetNbParams()==2 && pb[0]=="NextCamera")
	{
		if(Context->mCurrentTest)
			Context->mCurrentTest->mNextCameraPos = (sdword)pb[1];
		else
			OutputConsoleError("Script error: NextCamera command: no current test available.\n");
	}
	else if(pb.GetNbParams()==2 && pb[0]=="Subscene")
	{
		if(Context->mCurrentTest)
			Context->mCurrentTest->mSubScene = ICE_NEW(String)(pb[1]);
		else
			OutputConsoleError("Script error: Subscene command: no current test available.\n");
	}
	else if(pb.GetNbParams()==2 && pb[0]=="WireframeOverlay")
	{
		if(Context->mCurrentTest)
			Context->mCurrentTest->mWireframeOverlay = (sdword)pb[1];
		else
			OutputConsoleError("Script error: WireframeOverlay command: no current test available.\n");
	}
	else if(pb.GetNbParams()==2 && pb[0]=="Tool")
	{
		if(Context->mCurrentTest)
		{
			if(pb[1]=="Bullet")
				Context->mCurrentTest->mTool = TOOL_BULLET;
			else
				OutputConsoleError("Script error: Tool command: unsupported tool.\n");
		}
		else
			OutputConsoleError("Script error: Tool command: no current test available.\n");
	}
	else if(pb.GetNbParams()==2 && pb[0]=="Rendering")
	{
		if(pb[1]=="true")
			Context->mRendering = true;
		else if(pb[1]=="false")
			Context->mRendering = false;
	}
	else if(pb.GetNbParams()==2 && pb[0]=="RandomizeOrder")
	{
		if(pb[1]=="true")
			Context->mRandomizeOrder = true;
		else if(pb[1]=="false")
			Context->mRandomizeOrder = false;
	}
	else if(pb.GetNbParams()==2 && pb[0]=="TrashCache")
	{
		if(pb[1]=="true")
			Context->mTrashCache = true;
		else if(pb[1]=="false")
			Context->mTrashCache = false;
	}
	else if(pb.GetNbParams()==2 && pb[0]=="SaveExcelFile")
	{
		if(pb[1]=="true")
			Context->mSaveExcelFile = true;
		else if(pb[1]=="false")
			Context->mSaveExcelFile = false;
	}
	else
	{
		OutputConsoleError(_F("Unknown command in script:\n%s\n", command));
	}
	return true;
}

static void StartAutomatedTest(const AutomatedTest& test)
{
	if(test.mSubScene)
		test.mTest->SetDefaultSubscene(test.mSubScene->Get());

	EnableWireframeOverlay(test.mWireframeOverlay!=0);

	ActivateTest(test.mTest);

	// Tool must be selected after "ActivateTest"
	if(test.mTool!=INVALID_ID)
		SelectTool(test.mTool);
}

void RunScript(const char* filename)
{
	ASSERT(filename);

	{
		ReleaseAutomatedTests();

		ScriptFile Parser;
	//	Parser.Enable(BFC_MAKE_LOWER_CASE);
		Parser.Enable(BFC_REMOVE_TABS);
		Parser.Disable(BFC_REMOVE_SEMICOLON);
		Parser.Enable(BFC_DISCARD_COMMENTS);
		Parser.Disable(BFC_DISCARD_UNKNOWNCMDS);
		Parser.Disable(BFC_DISCARD_INVALIDCMDS);
		Parser.Disable(BFC_DISCARD_GLOBALCMDS);

		ParseContext Context;

		Parser.SetUserData(&Context);
		Parser.SetParseCallback(gParseCallback);
		if(Parser.Execute(filename))
		{
			if(Context.mNbFrames && Context.mTests.GetNbEntries())
			{
				gAutomatedTests = ICE_NEW(AutomatedTests)(Context);
			}
		}
	}

	AutomatedTests* AutoTests = GetAutomatedTests();
	if(AutoTests && AutoTests->IsValid())
	{
		EnableRendering(AutoTests->mRendering);
		SetRandomizeOrder(AutoTests->mRandomizeOrder);
		SetTrashCache(AutoTests->mTrashCache);

		AutomatedTest* Test = AutoTests->GetCurrentTest();
		StartAutomatedTest(*Test);
	}
}

static const char* gWantedTest = null;
void StartTest(const char* name)
{
	gWantedTest = name;
}

void TestCSVExport();
void UpdateAutomatedTests(udword frame_nb, bool menu_is_visible)
{
	if(gWantedTest)
	{
		PhysicsTest* t = FindTest(gWantedTest);
		if(!t)
		{
			OutputConsoleError(_F("Unknown test: %s\n", gWantedTest));
		}
		else
		{
			ActivateTest(t);
		}
		gWantedTest = null;
	}

	AutomatedTests* AutoTests = gAutomatedTests;
	if(!AutoTests)
		return;

	AutomatedTest* CurrentTest = AutoTests->GetCurrentTest();
	if(!CurrentTest)
		return;

	const udword SoundPos = GetSoundPos();
	if(SoundPos>CurrentTest->mNextCameraPos)
	{
		GetCamera().SelectNextCamera();
	}

	const udword Limit = MAX(CurrentTest->mNbFrames, AutoTests->mDefaultNbFrames);
	if(frame_nb>Limit || menu_is_visible || SoundPos>CurrentTest->mSoundPos)
	{
		if(AutoTests->mSaveExcelFile)
			TestCSVExport();

		AutomatedTest* NextTest = AutoTests->SelectNextTest();
		if(NextTest)
		{
			StartAutomatedTest(*NextTest);
		}
		else
			exit(0);
	}
}
