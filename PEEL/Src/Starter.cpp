///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RepX_Tools.h"
#include "CustomICEAllocator.h"
#include <xmmintrin.h>

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN64
	#pragma comment(lib, "../Ice/Lib64/IceCore64.lib")
	#pragma comment(lib, "../Ice/Lib64/IceMaths64.lib")
	#pragma comment(lib, "../Ice/Lib64/Contact64.lib")
	#pragma comment(lib, "../Ice/Lib64/Meshmerizer64.lib")
	#pragma comment(lib, "../Ice/Lib64/IceImageWork64.lib")
	#pragma comment(lib, "../Ice/Lib64/IceCharacter64.lib")
	#pragma comment(lib, "../Ice/Lib64/IceRenderer64.lib")
	#pragma comment(lib, "../Ice/Lib64/IceTerrain64.lib")
	#pragma comment(lib, "../Ice/Lib64/IceGUI64.lib")
	#pragma comment(lib, "../Ice/Lib64/ZCB264.lib")
//	#pragma comment(lib, "../../Ice/Lib64/IML64.lib")
#else
	#pragma comment(lib, "../Ice/Lib/IceCore.lib")
	#pragma comment(lib, "../Ice/Lib/IceMaths.lib")
	#pragma comment(lib, "../Ice/Lib/Contact.lib")
	#pragma comment(lib, "../Ice/Lib/Meshmerizer.lib")
	#pragma comment(lib, "../Ice/Lib/IceImageWork.lib")
	#pragma comment(lib, "../Ice/Lib/IceCharacter.lib")
	#pragma comment(lib, "../Ice/Lib/IceRenderer.lib")
	#pragma comment(lib, "../Ice/Lib/IceTerrain.lib")
	#pragma comment(lib, "../Ice/Lib/IceGUI.lib")
	#pragma comment(lib, "../Ice/Lib/ZCB2.lib")
	//#pragma comment(lib, "../Ice/Lib/IML.lib")
#endif

///////////////////////////////////////////////////////////////////////////////

void ThreadSetup()
{
	_clearfp();

#ifdef _WIN64
	//#define _MCW_ALL _MCW_DN | _MCW_EM | _MCW_IC// | _MCW_RC | _MCW_PC
	#define _MCW_ALL _MCW_DN | _MCW_EM |  _MCW_RC
	_controlfp(_CW_DEFAULT | _DN_FLUSH, _MCW_ALL);
#else
	#define _MCW_ALL _MCW_DN | _MCW_EM | _MCW_IC | _MCW_RC | _MCW_PC
   udword x86_cw;
   udword sse2_cw;
	__control87_2(_CW_DEFAULT | _DN_FLUSH, _MCW_ALL, &x86_cw, &sse2_cw);
#endif

	_clearfp();

	_mm_setcsr(_MM_MASK_MASK | _MM_FLUSH_ZERO_ON | (1 << 6));
}

///////////////////////////////////////////////////////////////////////////////

int PEEL_main(int argc, char** argv);

///////////////////////////////////////////////////////////////////////////////

static Allocator* gIceAllocator = null;

void PEEL_GlobalInit()
{
	ThreadSetup();
	SRand(42);

	if(1)
	{
		// To list leaks, use CreatePEELDefaultAllocator() in Debug, and use aligned allocs in MyAllocatorCallback.
		gIceAllocator = new CustomIceAllocator;
//		gIceAllocator = CreatePEELDefaultAllocator();
//		gIceAllocator = IceCore::CreateDefaultAllocator();
		ASSERT(gIceAllocator);
		IceCore::SetAllocator(*gIceAllocator);
	}

	ICECORECREATE icc;
	icc.mLogFile = false;
	InitIceCore(&icc);
	InitIceMaths();
	InitMeshmerizer();
	InitIceImageWork();
	InitIceGUI();

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
}

void PEEL_GlobalClose()
{
	CloseIceGUI();
	CloseIceImageWork();
	CloseMeshmerizer();
	CloseIceMaths();
	CloseIceCore();

	DELETESINGLE(gIceAllocator);
}

int main(int argc, char** argv)
{
	PEEL_GlobalInit();
	return PEEL_main(argc, argv);
}

