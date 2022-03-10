///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Foundation.h"
#include "DefaultControlInterface.h"
#include "Tool.h"
	#include "ToolInterface.h"
//	#include "ToolPicking.h"
//	#include "ToolCameraTracking.h"
//	#include "ToolShootBox.h"
//	#include "ToolAddImpulse.h"
//	#include "ToolObjectPlacement.h"
#include "GLRenderHelpers.h"
#include "GLRenderStates.h"
#include "GLFontRenderer.h"
#include "GLPointRenderer.h"
#include "GLPointRenderer2.h"
#include "GLVSync.h"
#include "PintEngineData.h"
#include "PintRender.h"
#include "TestScenes.h"
#include "Camera.h"
#include "TrashCache.h"
#include "TitleWindow.h"
#include "RaytracingWindow.h"
#include "RaytracingTest.h"
#include "Script.h"
#include "TestSelector.h"
#include "GUI_Helpers.h"
#include "GUI_ActorEdit.h"
#include "GUI_ActorSelection.h"
#include "GUI_JointEdit.h"
#include "GUI_CompoundEdit.h"
#include "GUI_PosEdit.h"
#include "RenderModel.h"
#include "ScreenQuad.h"
	#include "ScreenQuad_BackColor.h"
	#include "ScreenQuad_ColorGradient.h"
	#include "ScreenQuad_ColorSphere.h"
#include "TextureManager.h"
#include "RenderTarget.h"
#include "PintColorShapeRenderer.h"
#include "PintBatchConvexShapeRenderer.h"
#include "PintConvexInstanceShapeRenderer.h"
#include "ZCB\PINT_ZCB2.h"
#include "Sound.h"
#include "LegoLib\LegoLib.h"
#include "PINT_Editor.h"
#include "PEEL_MenuBar.h"
#include "PEEL_Threads.h"
#include "PEEL.h"
#include "Grid.h"
#include "RoundCorners.h"
#include "DisplayTexture.h"
#include "Devil.h"
#include "DefaultEnv.h"
#include "Terrain.h"
#include "Profiling.h"
#include "PintRenderState.h"
#include "Scattering.h"
#include "SupportFile.h"
#include "GLScaledCylinder.h"
#include "GLTexture.h"

#ifdef PEEL_USE_SPY
	#ifdef _DEBUG
		//#pragma comment(lib, "Spy/SpyClient_D.lib")
		#pragma comment(lib, "../Spy/SpyClient.lib")
	#else
		#pragma comment(lib, "../Spy/SpyClient.lib")
	#endif
#endif

//#define TEST_COLOR
#ifdef TEST_COLOR
	static Point gTestColor(0.0f, 0.0f, 0.0f);
#endif

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

extern bool gFreezeFrustum;
extern String gVehicleFilename;

//static const char* gAppTitle = "PEEL (Physics Engine Evaluation Lab)";
static const char* gAppTitle = "PEEL (Physics Engine Evaluation Lab) - PhysX Edition";
static const char* gVersion = "2.0";

static Point gDefaultGravity(0.0f, -9.81f, 0.0f);

/*static*/ bool gUseEditor = false;

static DefaultControlInterface* gDefaultControlInterface = null;

DefaultControlInterface& GetDefaultControlInterface()
{
	return *gDefaultControlInterface;
}

void SetDefaultControlInterface()
{
	SetControlInterface(gDefaultControlInterface);
}

enum RaytracingTestValue
{
	RAYTRACING_TEST_DISABLED,
	RAYTRACING_TEST_ST,
	RAYTRACING_TEST_MT,
};
static	RaytracingTestValue	gRaytracingTest = RAYTRACING_TEST_DISABLED;
static	int					gMainHandle = 0;
static	HWND				gWindowHandle = 0;
HWND GetWindowHandle()
{
	return gWindowHandle;
}

static	bool				gFog = false;
static	bool				gAutoCameraMove = false;
static	bool				gPaused = false;
bool IsPaused()				{ return gPaused;	}
void SetPause(bool paused)	{ gPaused = paused;	}

static	bool				gShowFPS = true;
static	bool				gShowInfos = true;
static	bool				gOneFrame = false;
/*static*/	bool			gMenuIsVisible = true;
static	bool				gHelpIsVisible = false;
static	int					gCurrentTest = 0;
/*static*/	PhysicsTest*	gRunningTest = null;
static	PhysicsTest*		gCandidateTest = null;
static	bool				gRandomizeOrder = false;
static	bool				gTrashCache = false;
static	bool				gEnableVSync = true;
static	bool				gCommaSeparator = false;
static	bool				gDisplayInfoMessages = false;
static	bool				gDisplayMessage = false;
static	float				gDisplayMessageDelay = 0.0f;
static	udword				gDisplayMessageType = 0;

static	bool				gUseFatFrames = false;
static	bool				gSymmetricFrames = false;
static	float				gFrameSize = 1.0f;
float GetFrameSize()			{ return gFrameSize;		}
bool GetSymmetricFrames()		{ return gSymmetricFrames;	}
bool UseFatFrames()				{ return gUseFatFrames;		}

void SetRandomizeOrder(bool b)	{ gRandomizeOrder = b;	}
void SetTrashCache(bool b)		{ gTrashCache = b;		}

static bool gBlitToScreen = true;
//static	float				gRTDistance = MAX_FLOAT;	// Test how different engines react to "infinite" rays
static	float				gRTDistance = 5000.0f;
float GetPickingDistance()
{
	return gRTDistance;
}

static	const Point			gMenuClearColor(0.2f, 0.2f, 0.2f);

static	bool				gRoundCorners = true;
static	udword				gRoundCornersSize = 64;

static	ProfilingUnits		gProfilingUnits = PROFILING_UNITS_RDTSC;
static	UserProfilingMode	gUserProfilingMode = USER_PROFILING_DEFAULT;

enum SQRaycastMode
{
	SQ_RAYCAST_CLOSEST,
	SQ_RAYCAST_ANY,
	SQ_RAYCAST_ALL,
};
static	SQRaycastMode	gSQRaycastMode = SQ_RAYCAST_CLOSEST;
//extern bool gRaycastClosest;
extern udword gRaycastMode;

bool gRenderSolidImpactShapes = true;

enum RaytracingResolution
{
	RT_WINDOW_64,
	RT_WINDOW_128,
	RT_WINDOW_256,
	RT_WINDOW_512,
	RT_WINDOW_768,
};
static	RaytracingResolution	gRaytracingResolution = RT_WINDOW_128;

///////////////////////////////////////////////////////////////////////////////

static	IceWindow*			gToolOptions[TOOL_COUNT] = {};
IceWindow*	GetToolOptionWindow(udword i)	{ return gToolOptions[i];	}
	
/*	static	udword				gRenderModelIndex = 0;*/

///////////////////////////////////////////////////////////////////////////////
// Camera tab

static	float	gCameraSpeedEdit = 20.0f;
static	float	gCameraNearClipEdit = 1.0f;
static	float	gCameraFarClipEdit = 10000.0f;
static	float	gCameraFOVEdit = 60.0f;
static	float	gCameraSensitivityEdit = 10.0f;
static	bool	gCameraFreezeFrustum = false;
		bool	gCameraVFC = true;

///////////////////////////////////////////////////////////////////////////////

/*	template<class T, const int MAX_COUNT>
	class ModeManager
	{
		public:
			ModeManager() : mSelected(0)
			{
				for(udword i=0;i<MAX_COUNT;i++)
					mMode[i] = null;
			}

		T*			mMode[MAX_COUNT];
		udword		mSelected;
	};*/

///////////////////////////////////////////////////////////////////////////////
// Render tab
static	bool	gWireframeEnabled = false;
static	bool	gWireframeOverlayEnabled = true;
static	bool	gGLFinish = false;
static	bool	gRenderEnabled = true;
/*static*/ void EnableRendering(bool b);
static void EnableWireframe(bool b);
//static	float	gWireframeOverlayCoeff = 1.005f;
static	float	gWireframeOverlayCoeff = 1.002f;
///*static*/	float	gConvexEdgesThreshold = 0.1f;
/*static*/	float	gConvexEdgesThreshold = 1.0f;
//bool gChaoticEdges = false;
float gChaoticEdgesCoeff = 0.0f;
static	HPoint	gAmbientColor(0.0f, 0.0f, 0.0f);

static	Point	gWireColor(0.0f, 0.0f, 0.0f);
Point	gPlaneColor(0.5f, 0.5f, 0.5f);
		udword	gBatchSize = 10000;
		bool	gWireframePass = false;

#ifdef PEEL_USE_MSAA
	static int g_msaaSamples = 0;
	static udword gMSAA_Index = 0;
	/*static*/ GLuint g_msaaFbo = 0;
	static GLuint g_msaaColorBuf = 0;
	static GLuint g_msaaDepthBuf = 0;
#endif

static	IceWindow*		gRenderModelOptions[RENDER_MODEL_COUNT] = {};
static	RenderModel*	gRenderModel[RENDER_MODEL_COUNT] = {};
		RenderModel*	gCurrentRenderModel = null;
static	RenderModelType	gRenderModelIndex = RENDER_MODEL_COUNT;
static	bool			SelectRenderModel(udword index)
{
	if(index>=RENDER_MODEL_COUNT)
		return false;
	gRenderModelIndex = RenderModelType(index);
	for(udword i=0;i<RENDER_MODEL_COUNT;i++)
	{
		if(gRenderModelOptions[i])
			gRenderModelOptions[i]->SetVisible(i==index);
	}

	if(gCurrentRenderModel)
		gCurrentRenderModel->Close();
	gCurrentRenderModel = gRenderModel[index];
	const bool status = gCurrentRenderModel->Init();
	//printf("Render model status: %d\n", status);

	if(gRunningTest && !gMenuIsVisible)
		gRunningTest->mMustResetTest = true;
	return status;
}
static	void	InitRenderModels()
{
	CreateRenderModels(gRenderModel);
	SelectRenderModel(RENDER_MODEL_FFP);
}
static	void	CloseRenderModels()
{
	for(udword i=0;i<RENDER_MODEL_COUNT;i++)
		DELETESINGLE(gRenderModel[i]);
}

///////////////////////////////////////////////////////////////////////////////
// Screen quad tab
static	const Point	gScreenQuadTab_BottomColor(1.0f, 1.0f, 1.0f);
static	const Point	gScreenQuadTab_TopColor(0.1f, 0.1f, 0.2f);
static	IceWindow*		gScreenQuadOptions[SCREEN_QUAD_COUNT] = {};
static	ScreenQuadMode*	gScreenQuadModes[SCREEN_QUAD_COUNT] = {};
static	ScreenQuadType	gScreenQuadModeIndex = SCREEN_QUAD_COUNT;

const ScreenQuadColorSphere* GetScreenQuadColorSphere()
{
	return static_cast<ScreenQuadColorSphere*>(gScreenQuadModes[SCREEN_QUAD_COLOR_SPHERE]);
}

static inline_ ScreenQuadMode* GetCurrentScreenQuadMode()
{
	return gScreenQuadModes[gScreenQuadModeIndex];
}

static void SelectScreenQuadMode(udword index)
{
	if(index>=SCREEN_QUAD_COUNT)
		return;
	gScreenQuadModeIndex = ScreenQuadType(index);
	for(udword i=0;i<SCREEN_QUAD_COUNT;i++)
	{
		if(gScreenQuadOptions[i])
			gScreenQuadOptions[i]->SetVisible(i==index);
	}
}

static void InitScreenQuadModes()
{
	gScreenQuadModes[SCREEN_QUAD_BACK_COLOR]					= ICE_NEW(ScreenQuadBackColor)(gMenuClearColor);
	gScreenQuadModes[SCREEN_QUAD_GRADIENT]						= ICE_NEW(ScreenQuadColorGradient)(gScreenQuadTab_TopColor, gScreenQuadTab_BottomColor);
	gScreenQuadModes[SCREEN_QUAD_COLOR_SPHERE]					= ICE_NEW(ScreenQuadColorSphere);

	SelectScreenQuadMode(SCREEN_QUAD_BACK_COLOR);
}

static void CloseScreenQuadModes()
{
	for(udword i=0;i<SCREEN_QUAD_COUNT;i++)
		DELETESINGLE(gScreenQuadModes[i]);
}

///////////////////////////////////////////////////////////////////////////////
// Sun tab

///////////////////////////////////////////////////////////////////////////////
// Color picker tab

	enum EditedColor
	{
		COLOR_PICKER_UNDEFINED,
		COLOR_PICKER_WIRE_COLOR,
		COLOR_PICKER_PLANE_COLOR,
	};
static	EditedColor	gEditedColor = COLOR_PICKER_UNDEFINED;

///////////////////////////////////////////////////////////////////////////////

#define	INITIAL_SCREEN_WIDTH	768
#define	INITIAL_SCREEN_HEIGHT	768
#define	MAIN_GUI_POS_X			0
#define	MAIN_GUI_POS_Y			0	//42
#define	GUI_HIDDEN_POS_X		-4096
#define	GUI_HIDDEN_POS_Y		-4096
#define	MAIN_WINDOW_POS_X		100
#define	MAIN_WINDOW_POS_Y		100
//#define	MAIN_WINDOW_POS_X		MAIN_GUI_POS_X+512+4
//#define	MAIN_WINDOW_POS_Y		MAIN_GUI_POS_Y

/*static*/ GLFontRenderer		gTexter;
/*static*/ udword				gScreenWidth	= INITIAL_SCREEN_WIDTH;
/*static*/ udword				gScreenHeight	= INITIAL_SCREEN_HEIGHT;
static FPS						gFPS;

/*static*/ udword				gFrameNb = 0;

static void StartFrame_MSAA()
{
	SPY_ZONE("StartFrame MSAA")

#ifdef PEEL_USE_MSAA
	if(g_msaaSamples)
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, g_msaaFbo);
#endif
}

static void EndFrame_MSAA()
{
	SPY_ZONE("EndFrame MSAA")

#ifdef PEEL_USE_MSAA
	if(g_msaaFbo && g_msaaSamples)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, g_msaaFbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
		glBlitFramebuffer(0, 0, gScreenWidth, gScreenHeight, 0, 0, gScreenWidth, gScreenHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
#endif
}

static void Release_MSAA()
{
#ifdef PEEL_USE_MSAA
	if(g_msaaFbo)
	{
		glDeleteFramebuffers(1, &g_msaaFbo);
		g_msaaFbo = 0;
	}
	if(g_msaaColorBuf)
	{
		glDeleteRenderbuffers(1, &g_msaaColorBuf);
		g_msaaColorBuf = 0;
	}
	if(g_msaaDepthBuf)
	{
		glDeleteRenderbuffers(1, &g_msaaDepthBuf);
		g_msaaDepthBuf = 0;
	}
#endif
}

static void Resize_MSAA()
{
#ifdef PEEL_USE_MSAA
	if(g_msaaSamples)
	{
		const int width = gScreenWidth;
		const int height = gScreenHeight;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		Release_MSAA();

		int samples;
//		glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
//		printf("%d max samples for MSAA\n");
		samples = g_msaaSamples;

		glGenFramebuffers(1, &g_msaaFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFbo);

		glGenRenderbuffers(1, &g_msaaColorBuf);
		glBindRenderbuffer(GL_RENDERBUFFER, g_msaaColorBuf);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);

		glGenRenderbuffers(1, &g_msaaDepthBuf);
		glBindRenderbuffer(GL_RENDERBUFFER, g_msaaDepthBuf);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_msaaDepthBuf);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, g_msaaColorBuf);

		glCheckFramebufferStatus(GL_FRAMEBUFFER);

		glEnable(GL_MULTISAMPLE);
	}
#endif
}

static void Select_MSAA(udword index)
{
#ifdef PEEL_USE_MSAA
	if(index==gMSAA_Index)
		return;
	gMSAA_Index = index;
	Release_MSAA();
	if(index==0)
		g_msaaSamples = 0;
	else if(index==1)
		g_msaaSamples = 2;
	else if(index==2)
		g_msaaSamples = 4;
	else if(index==3)
		g_msaaSamples = 8;
	else if(index==4)
		g_msaaSamples = 16;
	Resize_MSAA();
#endif
}

const char* GetFilenameForExport(const char* extension, const char* target)
{
	String Path = ".\\";

	if(target)
	{
		const char* Target = FindPEELFile(target);
		if(Target)
		{
			bool status = _GetPath(Target, Path);
		}
	}

	if(!gRunningTest)
		return _F("%sexported.%s", Path.Get(), extension);

	const char* Filename;
	const char* SubName = gRunningTest->GetSubName();
	if(SubName)
		Filename = _F("%s%s_%s.%s", Path.Get(), gRunningTest->GetName(), SubName, extension);
	else
		Filename = _F("%s%s.%s", Path.Get(), gRunningTest->GetName(), extension);
	return Filename;
}

static char gCmdLineScript[256] = {0};

struct CursorKeysState
{
	CursorKeysState() : mUp(false), mDown(false), mLeft(false), mRight(false)	{}
	void	Reset()
	{
		mUp = mDown = mLeft = mRight = false;
	}
	bool	mUp;
	bool	mDown;
	bool	mLeft;
	bool	mRight;
};

static CursorKeysState gState;

void TestCSVExport();
static void PEEL_InitGUI();
static void PEEL_CloseGUI();
static void gPEEL_GetOptionsFromGUI();
static void gPEEL_PollRadioButtons();

static EngineData	gEngines[MAX_NB_ENGINES];
static udword		gNbEngines = 0;
#define FOR_EACH_ENGINE(i)	for(udword i=0;i<gNbEngines;i++)

udword				GetNbEngines()				{ return gNbEngines;												}
bool				ValidateEngine(udword i)	{ return gEngines[i].mEnabled && gEngines[i].mSupportsCurrentTest;	}
Pint*				GetEngine(udword i)			{ return gEngines[i].mEngine;										}
//VisibilityManager*	GetVisHelper(udword i)		{ return gEngines[i].mVisHelper;									}

static udword				gNbSimulateCallsPerFrame = 1;
static float				gTimestep = 1.0f/60.0f;

class RaytracingWindow;
static PintPlugin*			gPlugIns[MAX_NB_ENGINES];
static RaytracingWindow*	gRaytracingWindows[MAX_NB_ENGINES] = {0};
static udword				gNbPlugIns = 0;

static String*				gRoot = null;	// I am Groot!
const char* GetRoot()
{
	return gRoot ? gRoot->Get() : null;
}

static CameraData gCamera;

CameraData& GetCamera()
{
	return gCamera;
}

typedef PintPlugin* (*GetPintPlugin)	();

PintGUIHelper* GetGUIHelper();
static PintGUIHelper& gGUIHelper = *GetGUIHelper();

static PintPlugin* LoadPlugIn(const char* filename)
{
	LIBRARY LibHandle;
	if(!IceCore::LoadLibrary_(filename, LibHandle, true) || !LibHandle)
	{
		printf("WARNING: plugin %s failed to load.\n", filename);
		return null;
	}

	GetPintPlugin func = (GetPintPlugin)BindSymbol(LibHandle, "GetPintPlugin");
	if(!func)
	{
		printf("WARNING: plugin %s is invalid.\n", filename);
		UnloadLibrary(LibHandle);
		return null;
	}
	return (func)();
}

static bool InitAll(PhysicsTest* test)
{
	const Point GravityFromUI = UI_GetGravity();

	PINT_WORLD_CREATE Desc;
//	Desc.mGravity	= gDefaultGravity;
	Desc.mGravity	= GravityFromUI;

	if(test)
	{
		bool MustResetCamera = false;

		static PhysicsTest* PreviousTest = null;
		if(test!=PreviousTest)
		{
			if(PreviousTest)
			{
				for(udword i=0;i<gNbPlugIns;i++)
					gPlugIns[i]->CloseTestGUI();

				PreviousTest->CloseUI();
			}

			IceTabControl* TestTabControl = test->InitUI(gGUIHelper);
			PreviousTest = test;

			if(TestTabControl)
			{
				// If a tab-control is returned, the test itself lets *plugins* define their own per-test UI.
				IceWidget* Parent = TestTabControl->GetParent();
				const char* TestName = test->GetName();
				for(udword i=0;i<gNbPlugIns;i++)
				{
					IceWindow* Main = gPlugIns[i]->InitTestGUI(TestName, Parent, gGUIHelper, test->GetUIElements());
					if(Main)
						TestTabControl->Add(Main, gPlugIns[i]->GetTestGUIName());
				}
			}

			// We only reset the camera when changing scene. That way we let users re-start the test while
			// focusing on a specific part of the scene. They can always press 'C' to manually reset the
			// camera anyway.
			MustResetCamera = true;

			// Set focus back to render window, not to break people's habit of browsing the test scenes
			IceCore::SetFocus(gWindowHandle);
		}

		// We must get the scene params after initializing the UI
		test->GetSceneParams(Desc);

		if(GravityFromUI!=Desc.mGravity)
			UI_SetGravity(Desc.mGravity);

		UI_SetSceneDesc(test->GetDescription());

		class Access : public PINT_WORLD_CREATE
		{
			public:
			void SetNames(const char* name/*, const char* desc*/)
			{
				mTestName = name;
//				mTestDesc = desc;
			}
		};
		static_cast<Access&>(Desc).SetNames(test->GetName()/*, test->GetDescription()*/);

		//### crude test - autodetect changes in camera data & reset camera if we found any
		if(!MustResetCamera)
		{
			CameraData Tmp;
			Tmp.Init(Desc);
			if(!(Tmp==gCamera))
				MustResetCamera = true;
		}

		if(MustResetCamera)
		{
			gCamera.Init(Desc);
			gCamera.Reset();
		}

//		if(test->GetName())
//			glutSetWindowTitle(test->GetName());
	}

	for(udword i=0;i<gNbPlugIns;i++)
		gPlugIns[i]->Init(Desc);

/*	if(gUseEditor && gNbPlugIns>1)
	{
		ASSERT(gPlugIns[0]==GetEditorPlugin());
		SetEditedPlugin(gPlugIns[1]->GetPint());
	}*/

	gNbSimulateCallsPerFrame = Desc.mNbSimulateCallsPerFrame;
	gTimestep = Desc.mTimestep;
	ASSERT(gTimestep>=0.0f);

	gNbEngines = 0;
	for(udword i=0;i<gNbPlugIns;i++)
	{
		ASSERT(gNbEngines!=MAX_NB_ENGINES);
		Pint* Engine = gPlugIns[i]->GetPint();
		ASSERT(Engine);
		gEngines[gNbEngines++].Init(Engine);
	}
	gPEEL_GetOptionsFromGUI();

//	GLPointRenderer::Init();
//	if(gCurrentRenderModel)
//		gCurrentRenderModel->Init();

	return Desc.mCreateDefaultEnvironment;
}

void UI_HideEditWindows()
{
	HideEditActorWindow();
	HideItemSelectionWindow();
	HideEditJointWindow();
	HideCompoundCreateWindow();
}

static void ResetToolAndInterfaces()
{
	UI_HideEditWindows();

	ToolInterface* CurrentTool = GetCurrentTool();
	if(CurrentTool)
		CurrentTool->Reset(INVALID_ID);
	gState.Reset();

	GetControlInterface()->Reset();

	UI_SetGravity(gDefaultGravity);

	GLRenderStates::SetDefaultCullMode();
	GLRenderStates::SetDefaultRenderStates();
}

static void CloseRunningTest()
{
	if(gRunningTest)
	{
		FOR_EACH_ENGINE(i)
		{
			ASSERT(gEngines[i].mEngine);
			gRunningTest->Close(*gEngines[i].mEngine);
		}
		gRunningTest->CommonRelease();
	}

	ResetToolAndInterfaces();
}

static void ResetSQHelpersHitData()
{
	FOR_EACH_ENGINE(i)
		gEngines[i].mSQHelper.ResetHitData();
}

static void CloseAll()
{
	void SetDropFile(const char*);
	SetDropFile(null);
	CloseRunningTest();

	FOR_EACH_ENGINE(i)
		gEngines[i].Reset();

	for(udword i=0;i<gNbPlugIns;i++)
		gPlugIns[i]->Close();

	ReleaseAllShapeRenderers();
	ReleaseAllConvexInstanceRenderers();
	ReleaseBatchConvexRender();
	ReleaseTransparent();
	ReleaseManagedTextures();
	UnregisterAllTerrainData();
	ReleaseAllTerrainData();

//	if(gCurrentRenderModel)
//		gCurrentRenderModel->Close();

//	GLPointRenderer::Close();

	extern float gZCB2_RenderScale;
	gZCB2_RenderScale = 1.0f;

//	glutSetWindowTitle(gAppTitle);
}

static void ResetTimers()
{
	gFrameNb = 0;
	FOR_EACH_ENGINE(i)
		gEngines[i].mTiming.ResetTimings();
}

/*static*/ void ActivateTest(PhysicsTest* test)
{
	if(test)
		gCandidateTest = test;

	if(gCandidateTest)
	{
#ifdef PEEL_PUBLIC_BUILD
		if(gCandidateTest->IsPrivate())
		{
			gDisplayMessage = true;
			gDisplayMessageType = 1;
		}
		else
#endif
		{
			CloseAll();
			const bool CreateDefaultEnv = InitAll(gCandidateTest);

			gRunningTest = gCandidateTest;
			ResetTimers();
			gRunningTest->CommonSetup();

			FOR_EACH_ENGINE(i)
			{
				ASSERT(gEngines[i].mEngine);
				gEngines[i].mSupportsCurrentTest = gRunningTest->Init(*gEngines[i].mEngine, CreateDefaultEnv);
			}

			gMenuIsVisible = false;
		}
	}
	if(gDisplayInfoMessages)
	{
		const udword NbShapeRenderers = GetNbShapeRenderers();
		OutputConsoleInfo(_F("NbShapeRenderers: %d\n", NbShapeRenderers));
		const udword NbManagedTextures = GetNbManagedTextures();
		OutputConsoleInfo(_F("NbManagedTextures: %d\n", NbManagedTextures));
	}
}

static Vertices* gCameraData = null;
static Point gLastCamPos(0.0f, 0.0f, 0.0f);
static Point gLastCamDir(0.0f, 0.0f, 0.0f);
static void RecordCameraPose()
{
	if(!gCameraData)
		gCameraData = ICE_NEW(Vertices);

	const Point CamPos = GetCameraPos();
	const Point CamDir = GetCameraDir();
	if(CamPos!=gLastCamPos || CamDir!=gLastCamDir)
	{
		gLastCamPos = CamPos;
		gLastCamDir = CamDir;
		gCameraData->AddVertex(CamPos);
		gCameraData->AddVertex(CamDir);
	}
}

static void SaveCameraData()
{
	if(!gCameraData)
		return;
	FILE* fp = fopen("d:\\tmp\\camera_data.bin", "wb");
//	FILE* fp = fopen("f:\\tmp\\camera_data.bin", "wb");
	if(fp)
	{
		const udword TotalNbPoses = gCameraData->GetNbVertices()/2;
		fwrite(&TotalNbPoses, sizeof(udword), 1, fp);

		const Point* V = gCameraData->GetVertices();
		fwrite(V, sizeof(Point)*TotalNbPoses*2, 1, fp);
		fclose(fp);
	}
	DELETESINGLE(gCameraData);
}

static void CopyCameraPoseToClipboard()
{
	const Point CamPos = GetCameraPos();
	const Point CamDir = GetCameraDir();
	const char* Text = _F("desc.mCamera[0] = PintCameraPose(Point(%.2ff, %.2ff, %.2ff), Point(%.2ff, %.2ff, %.2ff));", CamPos.x, CamPos.y, CamPos.z, CamDir.x, CamDir.y, CamDir.z);
	CopyToClipboard(Text);
	printf("Camera pose copied to clipboard\n");
}

static void KeyboardCallback(unsigned char key, int x, int y, bool down)
{
	SPY_ZONE("KeyboardCallback")

	GetControlInterface()->KeyboardCallback(key, x, y, down);

	bool KeyHasBeenHandled = false;
	if(!gMenuIsVisible)
		KeyHasBeenHandled = gRunningTest->KeyboardCallback(key, x, y, down);

	ToolInterface* CurrentTool = GetCurrentTool();
	if(CurrentTool)
	{
		FOR_EACH_ENGINE(i)
		{
			if(!gEngines[i].mEnabled || !gEngines[i].mSupportsCurrentTest)
				continue;

			CurrentTool->KeyboardCallback(*gEngines[i].mEngine, i, key, down);
		}
	}

	if(!down || KeyHasBeenHandled)
		return;

	switch(key)
	{
		case 27:
			exit(0);
			break;

		case 13:
			if(!gMenuIsVisible)
			{
				gMenuIsVisible = true;
				Point CameraData[2];
				CameraData[0] = GetCameraPos();
				CameraData[1] = GetCameraDir();
//				Save("PEEL", "Autosaved", "CameraData", CameraData, sizeof(Point)*2);
				//CopyToClipboard();

				ResetToolAndInterfaces();
			}
			else
			{
				ActivateTest();
			}
			break;

		case ' ':
			CopyCameraPoseToClipboard();
			break;
		case 'c':
		case 'C':
			gCamera.Reset();
			break;
		case 'f':
		case 'F':
			gShowFPS = !gShowFPS;
			break;
		case 'i':
		case 'I':
			gShowInfos = !gShowInfos;
			break;
		case '+':
//			if(!down)
				gCamera.SelectNextCamera();
			break;
		case '-':
//			if(!down)
				gCamera.SelectPreviousCamera();
			break;
		case 'p':
		case 'P':
			gPaused = !gPaused;
			gOneFrame = false;
			break;
		case 'o':
		case 'O':
			gOneFrame = true;
			gPaused = false;
			break;
		case 'r':
		case 'R':
			EnableRendering(!gRenderEnabled);
			break;
		case 'h':
		case 'H':
			if(gMenuIsVisible)
				gHelpIsVisible = !gHelpIsVisible;
			else
				ProcessMenuEvent(MENU_HIDE_SELECTED);
			break;
		case 'j':
		case 'J':
			if(gMenuIsVisible)
				gHelpIsVisible = !gHelpIsVisible;
			else
				ProcessMenuEvent(MENU_SHOW_SELECTED);
			break;
		case 's':
		case 'S':
			TestCSVExport();
			break;
		case 'w':
		case 'W':
			EnableWireframe(!gWireframeEnabled);
			break;
		case 'x':
		case 'X':
			EnableWireframeOverlay(!gWireframeOverlayEnabled);
			break;
#ifdef REMOVED
		case 'f':
		case 'F':
			{
/*				if(1)
				{
					gRenderModelIndex++;
					if(gRenderModelIndex==RENDER_MODEL_COUNT)
						gRenderModelIndex = 0;
					SelectRenderModel();
				}*/

				if(0)
				{
					static bool b = false;
					b = !b;
					printf("Current: %d\n", b);
					glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, b);
					glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, b);
				}

/*				gFog = !gFog;
				if(gFog)
				{
					glEnable(GL_FOG);
					//glFogi(GL_FOG_MODE,GL_LINEAR); 
					//glFogi(GL_FOG_MODE,GL_EXP); 
					glFogi(GL_FOG_MODE,GL_EXP2); 
					glFogf(GL_FOG_START, 0.0f);
					glFogf(GL_FOG_END, 100.0f);
					glFogf(GL_FOG_DENSITY, 0.005f);
			//		glClearColor(0.2f, 0.2f, 0.2f, 1.0);
					const Point FogColor(0.2f, 0.2f, 0.2f);
					glFogfv(GL_FOG_COLOR, &FogColor.x);
				}
				else
					glDisable(GL_FOG);*/
			}
			break;
#endif
		case 'l':
		case 'L':
			GetDefaultControlInterface().LockSelection(!GetDefaultControlInterface().IsSelectionLocked());
			break;
		case 'u':
		case 'U':
			gAutoCameraMove = !gAutoCameraMove;
			if(!gAutoCameraMove)
				SaveCameraData();
			break;
/*		case 'u':
		case 'U':
			RecordCameraPose();
			break;
		case 'i':
		case 'I':
			SaveCameraData();
			break;*/
			break;
	}
}

static void SpecialKeyCallback(int key, int x, int y, bool down)
{
	SPY_ZONE("SpecialKeyCallback")

	if(down)
	{
		// Function keys are used to enable/disable each engine
		ToolInterface* CurrentTool = GetCurrentTool();
		FOR_EACH_ENGINE(i)
		{
			if(key==i+1)
			{
				gEngines[i].mEnabled = !gEngines[i].mEnabled;
				if(CurrentTool)
					CurrentTool->Reset(i);
			}
		}
	}

	if(!gMenuIsVisible)
	{
		const bool KeyHasBeenHandled = gRunningTest->SpecialKeyCallback(key, x, y, down);
		if(!KeyHasBeenHandled)
		{
			if(!down)
			{
				if(key==GLUTX_KEY_PAGE_UP)
					gCamera.SelectNextCamera();
				else if(key==GLUTX_KEY_PAGE_DOWN)
					gCamera.SelectPreviousCamera();
			}

			const udword Flags = gRunningTest->GetFlags();
			if(!(Flags & TEST_FLAGS_USE_CURSOR_KEYS))
			{
				switch(key)
				{
					case GLUTX_KEY_UP:		gState.mUp = down; break;
					case GLUTX_KEY_DOWN:	gState.mDown = down; break;
					case GLUTX_KEY_LEFT:	gState.mLeft = down; break;
					case GLUTX_KEY_RIGHT:	gState.mRight = down; break;
				}
			}
		}
	}
	else
	{
		if(down)
			gCurrentTest = TestSelectionKeyboardCallback(key, gCurrentTest);
	}
}

static void PrintTimings()
{
	SPY_ZONE("PrintTimings")

//	const float TextScale = 0.02f * float(INITIAL_SCREEN_HEIGHT) / float(gScreenHeight);
	const float TextScale = 0.0175f * float(INITIAL_SCREEN_HEIGHT) / float(gScreenHeight);
	float y = 1.0f - TextScale;
	gTexter.setColor(1.0f, 1.0f, 1.0f, 1.0f);

	if(gRunningTest && gShowInfos)
	{
		gTexter.print(0.0f, y, TextScale, _F("Test: %s (%d camera views available)\n", gRunningTest->GetName(), gCamera.mNbSceneCameras+1));
		y -= TextScale;
	}

	if(gShowInfos)
	{
		FOR_EACH_ENGINE(i)
		{
			ASSERT(gEngines[i].mEngine);
			Pint* Engine = gEngines[i].mEngine;
			if(!(Engine->GetFlags() & PINT_IS_ACTIVE))
				continue;
#ifdef TEST_COLOR
			const Point MainColor = gTestColor;
#else
			const Point MainColor = Engine->GetMainColor();
#endif
			gTexter.setColor(MainColor.x, MainColor.y, MainColor.z, 1.0f);
			if(gEngines[i].mEnabled)
			{
				if(gEngines[i].mSupportsCurrentTest)
				{
					const PintTiming& Timing = gEngines[i].mTiming;
					if(Timing.mCurrentTestResult == INVALID_ID)
					{
						gTexter.print(0.0f, y, TextScale, _F("%s: %d (Avg: %d)(Worst: %d)(%d Kb)\n",
							Engine->GetName(), Timing.mCurrentTime, Timing.GetAvgTime(), Timing.mWorstTime, Timing.mCurrentMemory/1024));
					}
					else
					{
						gTexter.print(0.0f, y, TextScale, _F("%s: %d (Avg: %d)(Worst: %d)(%d Kb)(Val: %d)\n",
							Engine->GetName(), Timing.mCurrentTime, Timing.GetAvgTime(), Timing.mWorstTime, Timing.mCurrentMemory/1024, Timing.mCurrentTestResult));
					}
				}
				else
				{
					gTexter.print(0.0f, y, TextScale, _F("%s: (unsupported/not exposed)\n", Engine->GetName()));
				}
			}
			else
				gTexter.print(0.0f, y, TextScale, _F("%s: (disabled)\n", Engine->GetName()));
			y -= TextScale;
		}
	}

	gTexter.setColor(1.0f, 1.0f, 1.0f, 1.0f);
	if(gShowInfos)
	{
		if(gPaused)
			gTexter.print(0.0f, y, TextScale, "(Paused)");
		y -= TextScale;

		if(gShowFPS)
			gTexter.print(0.0f, y, TextScale, _F("Frame: %d", gFrameNb));
		y -= TextScale;

		if(gShowFPS)
			gTexter.print(0.0f, y, TextScale, _F("FPS: %.02f", gFPS.GetFPS()));
		y -= TextScale * 2.0f;
	}

	if(gRunningTest && !gMenuIsVisible)
	{
		FOR_EACH_ENGINE(i)
		{
			if(gEngines[i].mEnabled && gEngines[i].mSupportsCurrentTest)
			{
				ASSERT(gEngines[i].mEngine);
				Pint* Engine = gEngines[i].mEngine;

#ifdef TEST_COLOR
				const Point MainColor = gTestColor;
#else
				const Point MainColor = Engine->GetMainColor();
#endif
				gTexter.setColor(MainColor.x, MainColor.y, MainColor.z, 1.0f);

				y = gRunningTest->DrawDebugText(*Engine, gTexter, y, TextScale);
			}
		}
	}

	if(gDisplayMessage)
	{
		gDisplayMessage = false;
		gDisplayMessageDelay = 1.0f;
	}
	if(gDisplayMessageDelay>0.0f)
	{
		y -= TextScale * 2.0f;
		gTexter.setColor(1.0f, 0.0f, 0.0f, 1.0f);
		if(gDisplayMessageType==0)
			gTexter.print(0.0f, y, TextScale, "Saving results...");
		else
			gTexter.print(0.0f, y, TextScale, "This private test is disabled in public builds.");
	}
	gTexter.setColor(1.0f, 1.0f, 1.0f, 1.0f);
}

static void FormatText(const char* text, udword nb_col, ParameterBlock& pb)
{
//	for(udword i=0;i<nb_col;i++)	printf("*");
//	printf("\n");

	String S(text);
	S.Replace(' ', 1);

	char* Buffer = (char*)S.Get();

	udword Total = udword(strlen(Buffer));
	udword PreviousSpace = INVALID_ID;
	udword CurrentLength = 0;
	udword c = 0;

//	while(Buffer[c])
	while(c<Total)
	{
		// Look for space
		udword InitialPos = c;
		while(Buffer[c] && Buffer[c]!=1)
			c++;
		udword WordLength = c - InitialPos;

		if(CurrentLength+WordLength<=nb_col)
		{
			// Add current word to current line
			PreviousSpace = c;
			c++;
			CurrentLength += WordLength+1;
		}
		else
		{
			ASSERT(PreviousSpace!=INVALID_ID);
			Buffer[PreviousSpace] = ' ';
			PreviousSpace = INVALID_ID;

			c = InitialPos;
			CurrentLength = 0;
		}
	}

	pb.Create(S);
	udword NbParams = pb.GetNbParams();
	for(udword i=0;i<NbParams;i++)
	{
		pb.GetParam(i).Replace(1, ' ');
//		printf("%s\n", pb.GetParam(i));
	}
}

static DefaultRenderer* gPintRender = null;
void SetFocusActor(PintActorHandle h)
{
	gPintRender->mFocusActor = h ? Editor_GetNativeHandle(h) : null;
}

void SetFocusShape(PintActorHandle ah, PintShapeHandle sh)
{
	gPintRender->mFocusShape = sh ? Editor_GetNativeHandle(sh) : null;
	gPintRender->mFocusShapeActor = ah ? Editor_GetNativeHandle(ah) : null;
}

void SetUserDefinedPolygonMode()
{
	if(gWireframeEnabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

static void DrawTestDebugInfo()
{
	SPY_ZONE("DrawTestDebugInfo")

	if(gRunningTest)
	{
		gRunningTest->CommonDebugRender(*gPintRender);

		FOR_EACH_ENGINE(i)
		{
			if(gEngines[i].mEnabled && gEngines[i].mSupportsCurrentTest)
			{
				ASSERT(gEngines[i].mEngine);
				Pint* Engine = gEngines[i].mEngine;
#ifdef TEST_COLOR
				const Point MainColor = gTestColor;
#else
				const Point MainColor = Engine->GetMainColor();
#endif
				glColor3f(MainColor.x, MainColor.y, MainColor.z);
				gRunningTest->DrawDebugInfo(*Engine, *gPintRender);
			}
		}
	}
}

// Called from render models with a shader active
void DrawScene(PintRenderPass render_pass)
{
	SPY_ZONE("DrawScene")

	gPintRender->mSegments.Reset();

	FOR_EACH_ENGINE(i)
	{
		if(gEngines[i].mEnabled && gEngines[i].mSupportsCurrentTest)
		{
			ASSERT(gEngines[i].mEngine);
			Pint* Engine = gEngines[i].mEngine;
#ifdef TEST_COLOR
			const Point MainColor = gTestColor;
#else
			const Point MainColor = Engine->GetMainColor();
#endif
			SetEngineColor(MainColor);
			gPintRender->StartRender(gEngines[i].mEngine, &gDefaultControlInterface->mSelMan, render_pass);

				if(gRunningTest)
					gRunningTest->CommonRender(*gPintRender, render_pass);

				Engine->Render(*gPintRender, render_pass);
				gEngines[i].mSQHelper.Render(*gPintRender, render_pass, gPaused);
			gPintRender->EndRender();

/*			if(0)
			{
				glEnable(GL_CULL_FACE);	glCullFace(GL_FRONT);

					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//					glColor3f(0.0f, 0.0f, 0.0f);
					glColor3f(MainColor.x*0.5f, MainColor.y*0.5f, MainColor.z*0.5f);

					SetupProjectionMatrix(1.005f);
					SetupModelViewMatrix();

					glDisable(GL_LIGHTING);
					StartRender(*gPintRender);
						Engine->Render(*gPintRender);
					EndRender(*gPintRender);
					glEnable(GL_LIGHTING);

					SetupProjectionMatrix();
					SetupModelViewMatrix();

				glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);
			}*/
		}
	}
}

// Called from render models without a shader active
void DrawSQResults(PintRenderPass render_pass)
{
	SPY_ZONE("DrawSQResults")

	FOR_EACH_ENGINE(i)
	{
		if(gEngines[i].mEnabled && gEngines[i].mSupportsCurrentTest)
		{
			ASSERT(gEngines[i].mEngine);
			Pint* Engine = gEngines[i].mEngine;

			SetEngineColor(Engine->GetMainColor());

//			gEngines[i].mSQHelper.Render2(*gPintRender, render_pass, gPaused);
			gEngines[i].mSQHelper.RenderBatched(*gPintRender, render_pass, gPaused);
		}
	}
//	gPintRender->DrawSQ(render_pass);
}

// TODO: use ICE version
float ComputeConstantScale(const Point& pos, const Matrix4x4& view, const Matrix4x4& proj)
{
	const Point ppcam0 = pos * view;
	Point ppcam1 = ppcam0;
	ppcam1.x += 1.0f;

	const float l1 = 1.0f /	(ppcam0.x*proj.m[0][3] + ppcam0.y*proj.m[1][3] + ppcam0.z*proj.m[2][3] + proj.m[3][3]);
	const float c1 =		(ppcam0.x*proj.m[0][0] + ppcam0.y*proj.m[1][0] + ppcam0.z*proj.m[2][0] + proj.m[3][0])*l1;
	const float l2 = 1.0f /	(ppcam1.x*proj.m[0][3] + ppcam1.y*proj.m[1][3] + ppcam1.z*proj.m[2][3] + proj.m[3][3]);
	const float c2 =		(ppcam1.x*proj.m[0][0] + ppcam1.y*proj.m[1][0] + ppcam1.z*proj.m[2][0] + proj.m[3][0])*l2;
	const float CorrectScale = 1.0f / (c2 - c1);
	return CorrectScale / float(gScreenWidth);
}

/*static*/ void RenderWireframeOverlay(/*PintRenderPass render_pass*/)
{
	SPY_ZONE("RenderWireframeOverlay")

	if(!gWireframeOverlayEnabled)
		return;

/*	{
		glLineWidth(0.5f);
		SetUserDefinedPolygonMode();
	}*/

//	glEnable(GL_POINT_SMOOTH);
//	glEnable(GL_LINE_SMOOTH);
//	glEnable(GL_POLYGON_SMOOTH);

//	glDepthFunc(GL_LEQUAL);
//	glDepthFunc(GL_LESS);

//	glPolygonOffset(0.0f, 1.0f);
//	glEnable(GL_POLYGON_OFFSET_POINT);
//	glEnable(GL_POLYGON_OFFSET_LINE);
//	glEnable(GL_POLYGON_OFFSET_FILL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_LIGHTING);
	const float CurrentNearClip = GetCameraNearClip();
	SetCameraNearClip(CurrentNearClip*gWireframeOverlayCoeff);
	SetupProjectionMatrix();
	SetupModelViewMatrix();
	gWireframePass = true;
	SetMainColor(gWireColor);
	FOR_EACH_ENGINE(i)
	{
		if(gEngines[i].mEnabled && gEngines[i].mSupportsCurrentTest)
		{
			ASSERT(gEngines[i].mEngine);
			Pint* Engine = gEngines[i].mEngine;
			gPintRender->StartRender(gEngines[i].mEngine, &gDefaultControlInterface->mSelMan, PINT_RENDER_PASS_WIREFRAME_OVERLAY);
				Engine->Render(*gPintRender, PINT_RENDER_PASS_WIREFRAME_OVERLAY);
			gPintRender->EndRender();
		}
	}
	gWireframePass = false;
	SetCameraNearClip(CurrentNearClip);
	SetupProjectionMatrix();
	SetupModelViewMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LIGHTING);

//	glDisable(GL_POLYGON_OFFSET_POINT);
//	glDisable(GL_POLYGON_OFFSET_LINE);
//	glDisable(GL_POLYGON_OFFSET_FILL);
}

#ifdef REMOVED
/*static*/ void RenderTransparentObjects(PintRenderPass render_pass)
{
/*	FOR_EACH_ENGINE(i)
	{
		if(gEngines[i].mEnabled && gEngines[i].mSupportsCurrentTest)
		{
			ASSERT(gEngines[i].mEngine);
			Pint* Engine = gEngines[i].mEngine;
			gPintRender->StartRender(gEngines[i].mEngine, &gDefaultControlInterface.mSelMan, PINT_RENDER_PASS_WIREFRAME_OVERLAY);
				Engine->Render(*gPintRender, PINT_RENDER_PASS_WIREFRAME_OVERLAY);
			gPintRender->EndRender();
		}
	}*/

	// TODO: this isn't going to work for N engines
	RenderTransparent();
}
#endif

static void RenderDebugData()
{
	SPY_ZONE("RenderDebugData")

	glDisable(GL_LIGHTING);
	FOR_EACH_ENGINE(i)
	{
		if(gEngines[i].mEnabled && gEngines[i].mSupportsCurrentTest)
		{
			ASSERT(gEngines[i].mEngine);
			Pint* Engine = gEngines[i].mEngine;
			Engine->RenderDebugData(*gPintRender);
		}
	}

	glEnable(GL_LIGHTING);
}

static void RunRaytractingTest()
{
	if(0 && gRaytracingTest==RAYTRACING_TEST_DISABLED)
	{
		FOR_EACH_ENGINE(i)
		{
			if(gEngines[i].mEnabled)
			{
				ASSERT(gEngines[i].mEngine);
				//Pint* Engine = gEngines[i].mEngine;

				if(gRaytracingWindows[i])
				{
					gRaytracingWindows[i]->CreatePic();
					gRaytracingWindows[i]->redraw();
				}
			}
		}
	}

	if(gRaytracingTest!=RAYTRACING_TEST_DISABLED && !gPaused)
	{
		SPY_ZONE("RaytracingTest")

		Permutation P;
		P.Init(gNbEngines);
		if(gRandomizeOrder)
			P.Random(gNbEngines*2);
		else
			P.Identity();

		uword NbRays = 0;
		if(gRaytracingResolution==RT_WINDOW_64)
			NbRays = 64;
		else if(gRaytracingResolution==RT_WINDOW_128)
			NbRays = 128;
		else if(gRaytracingResolution==RT_WINDOW_256)
			NbRays = 256;
		else if(gRaytracingResolution==RT_WINDOW_512)
			NbRays = 512;
		else if(gRaytracingResolution==RT_WINDOW_768)
			NbRays = 768;

		const uword RAYTRACING_RENDER_WIDTH = NbRays;
		const uword RAYTRACING_RENDER_HEIGHT = NbRays;

		for(udword ii=0;ii<gNbEngines;ii++)
		{
			const udword i = P[ii];

			if(gEngines[i].mEnabled)
			{
				ASSERT(gEngines[i].mEngine);
				Pint* Engine = gEngines[i].mEngine;

				if(gRaytracingWindows[i])
				{
					Picture& Target = gRaytracingWindows[i]->mTarget;
					Target.Init(RAYTRACING_RENDER_WIDTH, RAYTRACING_RENDER_HEIGHT);

					udword NbHits;
					udword TotalTime;
					if(gRaytracingTest==RAYTRACING_TEST_ST)
						NbHits = RaytracingTest(Target, *Engine, TotalTime, gScreenWidth, gScreenHeight, NbRays, gRTDistance);
					else //if(gRaytracingTest==RAYTRACING_TEST_MT)
						NbHits = RaytracingTestMT(Target, *Engine, TotalTime, gScreenWidth, gScreenHeight, NbRays, gRTDistance);

					{
						SPY_ZONE("printf")
						printf("%s: %d hits, time = %d\n", Engine->GetName(), NbHits, TotalTime/1024);
					}

					{
						SPY_ZONE("Update picture")
						gRaytracingWindows[i]->mPic = Target;
						gRaytracingWindows[i]->mPic.Stretch(RAYTRACING_DISPLAY_WIDTH, RAYTRACING_DISPLAY_HEIGHT);
						gRaytracingWindows[i]->redraw();
					}
				}
/*				else
				{
					udword TotalTime;
					udword NbHits = RaytracingTest(*Engine, TotalTime, gScreenWidth, gScreenHeight);
					printf("%s: %d hits, time = %d\n", Engine->GetName(), NbHits, TotalTime/1024);
				}*/
			}
		}
		{
			SPY_ZONE("printf")
			printf("\n");
		}
	}
}

sdword gDeltaMouseX;
sdword gDeltaMouseY;

static sdword gRectSelX0 = -1;
static sdword gRectSelX1 = -1;
static sdword gRectSelY0 = -1;
static sdword gRectSelY1 = -1;
void SetRectangleSelection(sdword x0, sdword x1, sdword y0, sdword y1)
{
	gRectSelX0 = x0;
	gRectSelX1 = y0;
	gRectSelY0 = x1;
	gRectSelY1 = y1;
}

static udword gLastTime = 0;
//extern udword gNbRenderedVerts;
static void RenderCallback()
{
//	printf("\nRenderCallback\n");
/*	{
//		char tmp[1024];
		udword val;
		StartProfile_RDTSC(val);
			udword val2;
			StartProfile_RDTSC(val2);
			EndProfile_RDTSC(val2);
//			sprintf(tmp, "%d", gLastTime);
		EndProfile_RDTSC(val);
		printf("Cost: %d (%d)\n", val, val2);
	}*/

	// Avoid GL errors when frame buffer is null
	if(!gScreenWidth || !gScreenHeight)
		return;

#ifdef PEEL_USE_SPY
	{
		Spy::Zone Spy_RenderCallback("RenderCallback");
#endif

//	printf("%d\n", IsKeyPressed(VK_CONTROL));

	ProcessGamepads(*gDefaultControlInterface);

//	extern udword gScreenWidth;
//	extern udword gScreenHeight;
//	if(!gMenuIsVisible)
//		glutWarpPointer(gScreenWidth/2, gScreenHeight/2);

//	gMouseInfo.mMouseMove = false;

	if(!gMenuIsVisible && gRunningTest && gRunningTest->RepositionMouse())
	{
		if(IceCore::GetFocus()==gWindowHandle)
			RepositionMouse(gWindowHandle, gScreenWidth, gScreenHeight, gDeltaMouseX, gDeltaMouseY);
	}

/*	if(0)
	{
		printf("gNbRenderedVerts: %d\n", gNbRenderedVerts);
		gNbRenderedVerts = 0;
	}*/

	{
		UpdateAutomatedTests(gFrameNb, gMenuIsVisible);

		// Automatically reset the test if its "mMustResetTest" member has been set to true.
		if(gRunningTest && gRunningTest->mMustResetTest)
		{
			gRunningTest->mMustResetTest = false;
			ActivateTest(gRunningTest);
		}
	}

	gFPS.Update();
	gPintRender->mFrameNumber++;

	UpdateSun();

	udword CurrentTime = timeGetTime();
	udword ElapsedTime = CurrentTime - gLastTime;
	gLastTime = CurrentTime;
	const float Delta = float(ElapsedTime)*0.001f;
	if(gDisplayMessageDelay>0.0f)
	{
		gDisplayMessageDelay -= Delta;
		if(gDisplayMessageDelay<0.0f)
			gDisplayMessageDelay = 0.0f;
	}

	if(!gMenuIsVisible)
	{
		SPY_ZONE("Simulate")

		for(udword i=0;i<gNbSimulateCallsPerFrame;i++)
			Simulate(gTimestep, gProfilingUnits, gUserProfilingMode, gRunningTest, gNbEngines , gEngines, gRandomizeOrder, gPaused, gTrashCache);
	}

//	RunRaytractingTest();

	{
		SetCameraSpeed(gCameraSpeedEdit);
		SetCameraFOV(gCameraFOVEdit);
		SetCameraSensitivity(gCameraSensitivityEdit);
		SetCameraNearClip(gCameraNearClipEdit);
		SetCameraFarClip(gCameraFarClipEdit);

		if(gState.mUp)
			MoveCameraForward(Delta);
		if(gState.mDown)
			MoveCameraBackward(Delta);
		if(gState.mLeft)
			MoveCameraRight(Delta);
		if(gState.mRight)
			MoveCameraLeft(Delta);
	}

	StartFrame_MSAA();

	if(gMenuIsVisible)
	{
		SPY_ZONE("glClearColor & glClear")
		glClearColor(gMenuClearColor.x, gMenuClearColor.y, gMenuClearColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}
	else
	{
		SPY_ZONE("ScreenQuadMode shader")
		ScreenQuadMode*	SQM = GetCurrentScreenQuadMode();
		if(SQM)
			SQM->Apply(gScreenWidth, gScreenHeight);
	}

	// This position is better for object placement tool (or is it really?)
//	gMyControlInterface.RenderCallback();

//	glDisable(GL_CULL_FACE);
//	glEnable(GL_CULL_FACE);	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);	glCullFace(GL_BACK);

	if(gCurrentRenderModel)
		gCurrentRenderModel->SetupCamera();

	// This position is better for selection rendering
//	gMyControlInterface.RenderCallback();

	const bool BlitRT = gBlitToScreen && gRaytracingTest!=RAYTRACING_TEST_DISABLED;

	if(gRenderEnabled && !gMenuIsVisible && !BlitRT)
	{
		GetControlInterface()->RenderCallback(*gPintRender);

		if(gCurrentRenderModel)
		{
			{
				SPY_ZONE("InitScene")
				Point RenderCenter(0.0f, 1.0f, 0.0f);
				float RenderSize = 200.0f;
				if(gRunningTest)
					RenderSize = gRunningTest->GetRenderData(RenderCenter);
				gCurrentRenderModel->InitScene(RenderCenter, RenderSize);
			}

			DrawTestDebugInfo();

		// Render wireframe overlay first, to avoid issues with transparent objects
		RenderWireframeOverlay();
		{
			glLineWidth(0.5f);
			SetUserDefinedPolygonMode();
		}

			{
				SPY_ZONE("gCurrentRenderModel Render")
				gCurrentRenderModel->Render(gScreenWidth, gScreenHeight, GetCameraPos(), GetCameraDir(), GetCameraFOV());
//				SimpleViewer::Render(gCallbacks, gScreenWidth, gScreenHeight, GetCameraPos(), GetCameraDir(), 60.0f);
			}
		}

//		RenderWireframeOverlay();
//		RenderTransparentObjects();

		if(gRenderSunEnabled)
		{
			const Point CamPos = GetCameraPos();
			const Point CamDir = GetCameraDir();
			const Point Target = CamPos + CamDir*10.0f;

			const float SunDistance = 5000.0f;
			const float SunRadius = 100.0f;

			const Point SunPos = Target + gSun.GetDirection() * SunDistance;

			glMatrixMode(GL_MODELVIEW);
			PR SunPose(Idt);
			SunPose.mPos = SunPos;
			gPintRender->DrawSphere(SunRadius, SunPose);

			gPintRender->DrawLine(SunPos, Target, Point(1.0f, 1.0f, 0.0f));
		}

		RenderDebugData();

		if(0)
			RenderGrid(*gPintRender);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// TODO; weird one-frame line bug when this one is enabled
	//GLRenderHelpers::DrawFrame(Point(0.0f, 0.0f, 0.0f), 1.0f);

	if(gFreezeFrustum && !gMenuIsVisible)
		DrawCapturedFrustum();

	if(0)
	{
		glColor3f(1.0f, 0.5f, 0.25f);

		PR pose;
		pose.mPos	= Point(0.0f, 0.0f, 0.0f);
		pose.mRot	= Quat(Idt);
		GLRenderHelpers::DrawCapsule(1.0f, 4.0f, pose);
	}

	if(!gMenuIsVisible && 0)
	{
		static PEEL::RenderTarget* RT = null;
		if(!RT)
		{
			RT = new PEEL::RenderTarget(gScreenWidth, gScreenHeight);
			RT->BeginCapture();
				glViewport(0, 0, gScreenWidth, gScreenHeight);
			RT->EndCapture();
		}

		RT->BeginCapture();
		{
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			if(gCurrentRenderModel)
				gCurrentRenderModel->Render(gScreenWidth, gScreenHeight, GetCameraPos(), GetCameraDir(), GetCameraFOV());
		}
		RT->EndCapture();
//		if(g_msaaFbo)
//			glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFbo);

		udword texID = RT->GetColorTexId();//GetDepthTexId();

		glBindTexture(GL_TEXTURE_2D, texID);
	//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glDisable(GL_CULL_FACE);
		const Point color(1.0f, 1.0f, 1.0f);
		const float Alpha = 1.0f;
		GLRenderHelpers::DrawRectangle(	0.0f, 1.0f, 0.0f, 1.0f, color, color, Alpha, gScreenWidth, gScreenHeight, false, GLRenderHelpers::SQT_TEXTURING);

		glDisable(GL_TEXTURE_2D);
	}

	EndFrame_MSAA();

	if(!gMenuIsVisible && gRoundCorners)
		DrawRoundCorners(gScreenWidth, gScreenHeight, gRoundCornersSize);

	{
		ToolInterface* CurrentTool = GetCurrentTool();
		if(CurrentTool)
			CurrentTool->FinalRenderCallback();
	}

	if(0)
		DisplayTexture(gScreenWidth, gScreenHeight);

	if(BlitRT && !gMenuIsVisible)
	{
		RunRaytractingTest();

		for(udword i=0;i<gNbEngines;i++)
		{
			if(gEngines[i].mEnabled)
			{
				ASSERT(gEngines[i].mEngine);
				Pint* Engine = gEngines[i].mEngine;

				if(gRaytracingWindows[i])
				{
					const Picture& Target = gRaytracingWindows[i]->mTarget;
					if(Target.GetPixels())
					{
						static udword gWidth = 0;
						static udword gHeight = 0;
						static GLuint gTexID = 0;

						const udword CurrentWidth = Target.GetWidth();
						const udword CurrentHeight = Target.GetHeight();
						if(gTexID)
						{
							if(CurrentWidth!=gWidth || CurrentHeight!=gHeight)
							{
								GLTexture::ReleaseTexture(gTexID);
								gTexID = 0;
							}
						}

						if(!gTexID)
							gTexID = GLTexture::CreateTexture(CurrentWidth, CurrentHeight, Target.GetPixels(), false);
						else
							GLTexture::UpdateTexture(gTexID, CurrentWidth, CurrentHeight, Target.GetPixels(), false);
						GLTexture::BlitTextureToScreen(gTexID);
					}
				}
			}
		}
	}

	if(0)
	{
		BasicRandom Rnd;
		for(int i=0;i<1000;i++)
		{
			Point tmp;
			UnitRandomPt(tmp, Rnd);
			GLRenderHelpers::DrawLine(Point(0.0f, 0.0f, 0.0f), tmp, tmp);
		}
	}

	if(gMenuIsVisible)
	{
		const float TextScale = 0.02f * float(INITIAL_SCREEN_HEIGHT) / float(gScreenHeight);
//		const float TextScale = 0.0175f * float(INITIAL_SCREEN_HEIGHT) / float(gScreenHeight);
		const float x = 0.0f;
		float YLast;
		gCandidateTest = RenderTestSelector(gTexter, x, TextScale, gCurrentTest, YLast);
		gTexter.setColor(1.0f, 1.0f, 1.0f, 1.0f);

		float y = 1.0f - TextScale * 11.0f;
		if(gHelpIsVisible)
		{
			gTexter.print(x, y, TextScale, "Press H to Hide help");													y -= TextScale;
			y -= TextScale;

			const float HelpTextScale = TextScale * 0.8f;
//			const float HelpTextScale = TextScale;

			gTexter.print(x, y, HelpTextScale, "In menu mode (this screen):");											y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press arrow up/down to select a test, left/right to skip a category");	y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press Return to run selected test (and enter 'simulation mode')");		y -= HelpTextScale;

			y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "In simulation mode:");													y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press Return to go back to the menu mode");								y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press arrow keys to move the camera");									y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press PAGEUP/PAGEDOWN or +/- (on the keypad) to switch cameras");		y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press C to reset the Camera");											y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press O to step One frame");											y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press R to disable Rendering");											y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press W to enable Wireframe mode");										y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press X to enable Wireframe Overlay mode");								y -= HelpTextScale;
//			gTexter.print(x, y, HelpTextScale, "Use the mouse and left mouse button to move the camera");				y -= HelpTextScale;
//			gTexter.print(x, y, HelpTextScale, "Use the mouse and right mouse button for picking");						y -= HelpTextScale;

			y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "At any time:");															y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press F1->Fx to enable/disable engines");								y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press P to enable/disable Pause");										y -= HelpTextScale;
			gTexter.print(x, y, HelpTextScale, "Press S to Save results as an Excel file");								y -= HelpTextScale;
		}
		else
		{
			gTexter.print(x, y, TextScale, "Press H for Help");
		}

		if(gCandidateTest)
		{
			const float PanelSize = TextScale*13.0f;
			float ystart = YLast + TextScale*10.0f + PanelSize;	// 10 here depends on number of lines in selector
			y = ystart - TextScale*2.0f;

			//const Point BackColor(1.0f, 0.5f, 0.2f);
			//const Point BackColor(0.2f, 0.5f, 1.0f);
			//const Point BackColor = gWireColor;
			const Point BackColor(38.0f/255.0f, 109.0f/255.0f, 103.0f/255.0f);
			//const Point BackColor(66.0f/255.0f, 142.0f/255.0f, 136.0f/255.0f);

			const float XOffset = 0.01f;
			GLRenderHelpers::DrawRectangle(XOffset, 1.0f-XOffset, ystart, ystart - PanelSize, BackColor, Point(0.0f, 0.0f, 0.0f), 0.5f, gScreenWidth, gScreenHeight, true, GLRenderHelpers::SQT_DISABLED);

			const char* TestDescription = gCandidateTest->GetDescription();
			if(!TestDescription)
			{
				static const char* NoDesc = "(No description available)";
				TestDescription = NoDesc;
			}

			ParameterBlock pb;
			//FormatText(TestDescription, 84, pb);
			FormatText(TestDescription, 75, pb);
			const udword NbParams = pb.GetNbParams();

			for(udword i=0;i<NbParams;i++)
			{
				gTexter.print(x+XOffset*2.0f, y, TextScale, pb[i].Get());
				y -= TextScale;
			}
		}
	}
	else
	{
		if(gRectSelX0 != gRectSelX1 && gRectSelY0!=gRectSelY1 && gScreenWidth!=0.0f && gScreenHeight!=0.0f)
		{
			const Point Color(1.0f, 1.0f, 1.0f);
			const float Alpha = 0.25f;
			const float x0 = float(gRectSelX0) / float(gScreenWidth);
			const float x1 = float(gRectSelX1) / float(gScreenWidth);
			const float y0 = 1.0f - float(gRectSelY0) / float(gScreenHeight);
			const float y1 = 1.0f - float(gRectSelY1) / float(gScreenHeight);
			GLRenderHelpers::DrawRectangle(x0, x1, y0, y1, Color, Color, Alpha, gScreenWidth, gScreenHeight, true, GLRenderHelpers::SQT_DISABLED);
		}
	}

	if(0)
	{
		GLRenderHelpers::DrawLine2D(0.01f, 1.0f-0.01f, 1.0f-0.01f, 0.01f, Point(1.0f, 1.0f, 0.0f));
	}

	PrintTimings();

	{
		if(gGLFinish)
		{
			SPY_ZONE("glFinish")
			glFinish();
		}

		{
			SPY_ZONE("glutxSwapBuffers")
			glutxSwapBuffers();
		}

		{
			SPY_ZONE("glutxReportErrors")
			glutxReportErrors();
		}
	}

	if(!gBlitToScreen)
		RunRaytractingTest();

	if(gOneFrame)
		gPaused = true;

	gPEEL_PollRadioButtons();

	if(gAutoCameraMove)
	{
		MoveCameraForward(Delta);
		RecordCameraPose();
	}

#ifdef PEEL_USE_SPY
	}
	Spy::Sync();
#endif
}

static void ReshapeCallback(int width, int height)
{
//	printf("ReshapeCallback: %d %d\n", width, height);
	glViewport(0, 0, width, height);
	gTexter.setScreenResolution(width, height);
	GLPointRenderer::SetScreenResolution(width, height);
	gScreenWidth = width;
	gScreenHeight = height;
	Resize_MSAA();
}

static void IdleCallback()
{
	SPY_ZONE("IdleCallback")

	glutxPostRedisplay();
}

void MoveTestSelection(udword key, udword nb)
{
	for(udword i=0;i<nb;i++)
		gCurrentTest = TestSelectionKeyboardCallback(key, gCurrentTest);
}

static void MouseWheelCallback(int x, int y, short wheelValue)
{
	SPY_ZONE("MouseWheelCallback")

//	printf("CHECKPOINT %d\n", wheelValue);

	const udword Key = wheelValue>0 ? GLUTX_KEY_UP : GLUTX_KEY_DOWN;
	const udword Nb = abs(wheelValue);

	if(gMenuIsVisible)
	{
//		for(udword i=0;i<Nb;i++)
//			gCurrentTest = TestSelectionKeyboardCallback(Key, gCurrentTest);
		MoveTestSelection(Key, Nb);
	}
	else
	{

		const MouseInfo& MI = GetMouseInfo();
		if(MI.mMouseButton[MBT_LEFT])
		{
			float CamSpeed = GetCameraSpeed();
			if(Key==GLUTX_KEY_UP)
				CamSpeed *= 2.0f;
			else
				CamSpeed *= 0.5f;
			SetCameraSpeed(CamSpeed);
			UI_UpdateCameraSpeed();
		}
		else
		{
			const float Delta = 4.0f*1.0f/60.0f;
			for(udword i=0;i<Nb;i++)
			{
				if(Key==GLUTX_KEY_UP)
					MoveCameraForward(Delta);
				else
					MoveCameraBackward(Delta);
			}
		}
	}
}

void PEEL_GlobalClose();

static void gCleanup()
{
	{
		Save("PEEL", "Autosaved", "CurrentTest", gCurrentTest);
	}

//	CloseLegoLib();

	ReleaseGamepads();

	Release_MSAA();

	GLPointRenderer2::Close();
	GLPointRenderer::Close();
	ReleaseScaledCylinders();

	if(gCurrentRenderModel)
		gCurrentRenderModel->Close();

	timeBeginPeriod(1);
	printf("Exiting...\n");
	if(gRunningTest)
		gRunningTest->CloseUI();
	CloseAll();
	ReleaseAutomatedTests();
	gVehicleFilename.Reset();

	DELETESINGLE(gRoot);

	for(udword i=0;i<gNbPlugIns;i++)
	{
		gPlugIns[i]->CloseGUI();
	}
	PEEL_CloseGUI();

	CloseTools();
	CloseScreenQuadModes();
	CloseRenderModels();
	gPintRender->Release();
	ReleaseSystemTextures();

	DELETESINGLE(gDefaultControlInterface);
	DELETESINGLE(gPintRender);

	releaseFoundation();

	ReleasePEELMenuBar();
	PEEL_ReleaseThreads();
#ifdef PEEL_USE_SPY
	Spy::Close();
#endif
	PEEL_GlobalClose();

	CloseDevil();

	OutputConsoleInfo("PEEL closed successfully.\n");
}

static void RegisterPlugIn(const char* filename)
{
	PintPlugin* pp = LoadPlugIn(filename);
	if(pp)
	{
		printf(_F("Plugin found: %s\n", filename));
		gPlugIns[gNbPlugIns++] = pp;
	}
}

static char gBuildFolder[MAX_PATH];
static char gCurrentFile[MAX_PATH];

static char* gFolders[] = {
	"../Media/%s",
	"../Media/#Private/Customers/%s",
	"../Media/#Private/Debug/%s",
	"../Media/#Scripts/%s",
	"../Media/#Shaders/%s",
	"../Media/#Sounds/%s",
	"../Media/#VehicleData/%s",
	"../Media/#Vehicles/%s",
	"../Media/NutsAndBolts/%s",
	"../Media/Zcb/%s",
	"../Excel/%s",
//	"../build/%s",
//	"../build/Customers/%s",
//	"../build/Data/%s",
	"./%s",
//	"./Customers/%s",
//	"./Data/%s",
//	"../Data/%s",
	"%s",
	"../Private/Bruneton/%s",
	null
};

static String gDropFile;

void SetDropFile(const char* name)
{
	gDropFile = name;
}

const char* FindPEELFile(const char* filename)
{
	if(filename==gCurrentFile)
	{
		if(FileExists(filename))
			return filename;
		ASSERT(0);
	}

	const char* Candidate = null;
	udword i=0;
	do
	{
		const char* NextCandidate = gFolders[i] ? _F(gFolders[i], filename) : null;
//		printf("NextCandidate: %s\n", NextCandidate);
		Candidate = NextCandidate;
		if(!Candidate)
			break;
//			NextCandidate = _F("%s%s", gBuildFolder, filename);

		strcpy(gCurrentFile, NextCandidate);
		if(FileExists(gCurrentFile))
			return gCurrentFile;
		i++;
	}while(Candidate);

	if(!Candidate)
	{
		i=0;
		do
		{
			const char* NextCandidate = gFolders[i] ? _F(gFolders[i], filename) : null;
			Candidate = NextCandidate;
			if(Candidate)
			{
				strcpy(gCurrentFile, gBuildFolder);
				strcat(gCurrentFile, Candidate);

				if(FileExists(gCurrentFile))
					return gCurrentFile;
				i++;
			}
		}while(Candidate);
	}

	if(gDropFile.Get())
	{
		String Path;
		if(_GetPath(gDropFile, Path))
		{
			strcpy(gCurrentFile, Path);
			strcat(gCurrentFile, filename);
			if(FileExists(gCurrentFile))
				return gCurrentFile;
		}
	}

	{
		OutputConsoleError(_F("File not found: %s\n", filename));
	}
	return null;
}

void TestEmpty()
{
	const udword NbTests = GetNbTests();
	for(udword i=0;i<NbTests;i++)
	{
		PhysicsTest* Test = GetTest(i);
		if(strcmp(Test->GetName(), "EmptyScene")==0)
		{
			gCandidateTest = Test;
			ActivateTest(gCandidateTest);
			break;
		}
	}
}

void SetupBuildFolder(int argc, char** argv)
{
	if(argc && argv)
	{
		strcpy(gBuildFolder, *argv);
		char* Build64 = strstr(gBuildFolder, "Build64");
		if(Build64)
		{
			Build64[8]=0;
		}
		else
		{
			char* Build = strstr(gBuildFolder, "Build");
			if(Build)
			{
				Build[6]=0;
			}
			else
			{
				gBuildFolder[0] = 0;
			}
		}
	}
}

static udword AnalyzeCommandLine(int argc, char** argv)
{
/*	if(argc && argv)
	{
		strcpy(gBuildFolder, *argv);
		char* Build64 = strstr(gBuildFolder, "Build64");
		if(Build64)
		{
			Build64[8]=0;
		}
		else
		{
			char* Build = strstr(gBuildFolder, "Build");
			if(Build)
			{
				Build[6]=0;
			}
			else
			{
				gBuildFolder[0] = 0;
			}
		}
	}*/
	SetupBuildFolder(argc, argv);

	if(argc<=1)
		return 0;

	argc--;
	argv++;

	udword NbPlugins = 0;

	while(argc)
	{
		argc--;
		const char* Command = *argv++;

		if(Command[0]=='/' && Command[1]=='/')
			break;

		if(Command[0]=='-' && Command[1]=='r')
		{
			// The UI will be created after this point, in the proper state
			gRenderEnabled = false;
		}
		else if(Command[0]=='-' && Command[1]=='p')
		{
			if(argc)
			{
				argc--;
				const char* Filename = *argv++;
				RegisterPlugIn(Filename);
				NbPlugins++;
			}
		}
		else if(Command[0]=='-' && Command[1]=='t')
		{
			if(argc)
			{
				argc--;
				const char* TestName = *argv++;

				const udword NbTests = GetNbTests();
				for(udword i=0;i<NbTests;i++)
				{
					PhysicsTest* Test = GetTest(i);
					if(strcmp(Test->GetName(), TestName)==0)
					{
						gCandidateTest = Test;
						break;
					}
				}
			}
		}
		else if(Command[0]=='-' && Command[1]=='s')
		{
			if(argc)
			{
				argc--;
				const char* ScriptFilename = *argv++;
				strcpy(gCmdLineScript, ScriptFilename);
			}
		}
	}
	return NbPlugins;
}

///////////////////////////////////////////////////////////////////////////////

static std::vector<char> m_RawInputMessageData; // Buffer

static void OnRawInput(int inForeground, HRAWINPUT hRawInput)
{
    UINT dataSize;
    GetRawInputData(
      hRawInput, RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

    if(dataSize == 0)
      return;
    if(dataSize > m_RawInputMessageData.size())
        m_RawInputMessageData.resize(dataSize);

    void* dataBuf = &m_RawInputMessageData[0];
    GetRawInputData(
      hRawInput, RID_INPUT, dataBuf, &dataSize, sizeof(RAWINPUTHEADER));

    const RAWINPUT *raw = (const RAWINPUT*)dataBuf;
    if (raw->header.dwType == RIM_TYPEMOUSE)
    {
        HANDLE deviceHandle = raw->header.hDevice;

        const RAWMOUSE& mouseData = raw->data.mouse;

        USHORT flags = mouseData.usButtonFlags;
        short wheelDelta = (short)mouseData.usButtonData;
        LONG x = mouseData.lLastX, y = mouseData.lLastY;

        wprintf(
            L"Mouse: Device=0x%08X, Flags=%04x, WheelDelta=%d, X=%d, Y=%d\n",
            deviceHandle, flags, wheelDelta, x, y);
    }
}

/*

    deviceHandle is a handle that lets you distinguish which mouse generated this event.
    flags is the main type of event. It may be zero or more of following flags:
    RI_MOUSE_LEFT_BUTTON_DOWN
    RI_MOUSE_LEFT_BUTTON_UP
    RI_MOUSE_MIDDLE_BUTTON_DOWN
    RI_MOUSE_MIDDLE_BUTTON_UP
    RI_MOUSE_RIGHT_BUTTON_DOWN
    RI_MOUSE_RIGHT_BUTTON_UP
    RI_MOUSE_BUTTON_1_DOWN
    RI_MOUSE_BUTTON_1_UP
    RI_MOUSE_BUTTON_2_DOWN
    RI_MOUSE_BUTTON_2_UP
    RI_MOUSE_BUTTON_3_DOWN
    RI_MOUSE_BUTTON_3_UP
    RI_MOUSE_BUTTON_4_DOWN
    RI_MOUSE_BUTTON_4_UP
    RI_MOUSE_BUTTON_5_DOWN
    RI_MOUSE_BUTTON_5_UP
    RI_MOUSE_WHEEL
    wheelDelta, applicable when RI_MOUSE_WHEEL is set, gives positive or negative delta of mouse wheel. On my system it's always -120 or +120.
    x, y is relative or absolute, depending on the (mouseData.usFlags & MOUSE_MOVE_ABSOLUTE). But in my tests it's always relative, like (-3, 0), on every USB mouse, as well as touchpad on my laptop.
    There is also mouseData.ulRawButtons available with bits corresponding to current state of all mouse buttons, but works on my PC and not on my laptop (it is always 0).

Mouse: Device=0x0001003D, Flags=0001, WheelDelta=0, X=0, Y=0
Mouse: Device=0x0001003D, Flags=0002, WheelDelta=0, X=0, Y=0
Mouse: Device=0x0001003D, Flags=0000, WheelDelta=0, X=-1, Y=0
Mouse: Device=0x0001003D, Flags=0004, WheelDelta=0, X=0, Y=0
Mouse: Device=0x0001003D, Flags=0008, WheelDelta=0, X=0, Y=0
Mouse: Device=0x0001003D, Flags=0400, WheelDelta=120, X=0, Y=0
Mouse: Device=0x0001003D, Flags=0400, WheelDelta=-120, X=0, Y=0
Mouse: Device=0x0001003D, Flags=0000, WheelDelta=0, X=1, Y=0
Mouse: Device=0x0001003D, Flags=0000, WheelDelta=0, X=1, Y=0
Mouse: Device=0x0001003D, Flags=0000, WheelDelta=0, X=2, Y=-1
Mouse: Device=0x0001003D, Flags=0000, WheelDelta=0, X=1, Y=-1
*/

static void glutXRawInputFunc(udword wparam, udword lparam)
{
//	WPARAM wparam, LPARAM lparam
	OnRawInput(GET_RAWINPUT_CODE_WPARAM(wparam) == RIM_INPUT, (HRAWINPUT)lparam);
}

void ImportFile(const char* filename)
{
	const bool ShouldMerge = !gMenuIsVisible && IsKeyPressed(VK_CONTROL);
	if(ShouldMerge)
	{
		MergeFile(filename);
		return;
	}

	extern PhysicsTest* gDragAndDropTest;
	extern const char* gDragAndDropName;
	if(gDragAndDropTest)
	{
		gDragAndDropName = filename;
		ActivateTest(gDragAndDropTest);
	}
}

static void glutDropFile(int x, int y, const char* filename)
{
/*	extern PhysicsTest* gDragAndDropTest;
	extern const char* gDragAndDropName;
	if(gDragAndDropTest)
	{
		gDragAndDropName = filename;
		ActivateTest(gDragAndDropTest);
	}*/
	ImportFile(filename);
}

///////////////////////////////////////////////////////////////////////////////

//void TestSpy();
static void SetBestRenderMode();

int PEEL_main(int argc, char** argv)
{
	initFoundation();
	PEEL_InitThreads();

/*	if(0)
	{
		TestSpy();
		gCleanup();
		return 0;
	}*/

	if(0)
	{
		String CurrentDir;
		GetCurrentDir(CurrentDir);
		printf("Current dir: %s\n", CurrentDir.Get());
	}

	ICEGUIAPPCREATE Create;
	IceGUIApp MyApp;
	bool Status = MyApp.Init(Create);
	(void)Status;

	InitTests();
	{
		udword Value;
		if(Load("PEEL", "Autosaved", "CurrentTest", Value))
		{
			if(Value<GetNbTests())
				gCurrentTest = Value;
		}
	}

#ifdef PEEL_USE_SPY
	Spy::Init();
#endif

	if(0)
	{
		RAWINPUTDEVICE device;
		device.usUsagePage = 0x01;
		device.usUsage = 0x02;
		device.dwFlags = 0;
		device.hwndTarget = 0;
		RegisterRawInputDevices(&device, 1, sizeof device);

		UINT numDevices;
		GetRawInputDeviceList(
		  NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
		if(numDevices == 0) return 0;

		std::vector<RAWINPUTDEVICELIST> deviceList(numDevices);
		GetRawInputDeviceList(
		  &deviceList[0], &numDevices, sizeof(RAWINPUTDEVICELIST));

		std::vector<wchar_t> deviceNameData;
		//wstring deviceName;
		for(UINT i = 0; i < numDevices; ++i)
		{
		  const RAWINPUTDEVICELIST& device = deviceList[i];
		  if(device.dwType == RIM_TYPEMOUSE)
		  {
			wprintf(L"Mouse: Handle=0x%08X\n", device.hDevice);

			UINT dataSize;
			GetRawInputDeviceInfo(
			  device.hDevice, RIDI_DEVICENAME, null, &dataSize);
			if(dataSize)
			{
			  deviceNameData.resize(dataSize);
			  UINT result = GetRawInputDeviceInfo(
				device.hDevice, RIDI_DEVICENAME, &deviceNameData[0], &dataSize);
			  if(result != UINT_MAX)
			  {
		//        deviceName.assign(deviceNameData.begin(), deviceNameData.end());
		//        wprintf(L"  Name=%s\n", deviceName.c_str());
			  }
			}

			RID_DEVICE_INFO deviceInfo;
			deviceInfo.cbSize = sizeof deviceInfo;
			dataSize = sizeof deviceInfo;
			UINT result = GetRawInputDeviceInfo(
			  device.hDevice, RIDI_DEVICEINFO, &deviceInfo, &dataSize);
			if(result != UINT_MAX)
			{
			  assert(deviceInfo.dwType == RIM_TYPEMOUSE);
			  wprintf(
				L"  Id=%u, Buttons=%u, SampleRate=%u, HorizontalWheel=%s\n",
				deviceInfo.mouse.dwId,
				deviceInfo.mouse.dwNumberOfButtons,
				deviceInfo.mouse.dwSampleRate,
				deviceInfo.mouse.fHasHorizontalWheel ? L"1" : L"0");
			}
		  }
		}
	}

	gPintRender = ICE_NEW(DefaultRenderer);
//	gDefaultControlInterface = ICE_NEW(DefaultControlInterface);
	gDefaultControlInterface = new DefaultControlInterface;

	glutxInit(&argc, argv);
	glutxInitWindowSize(gScreenWidth, gScreenHeight);
	// ### Glut is drunk
	glutxInitWindowPosition(MAIN_WINDOW_POS_X, MAIN_WINDOW_POS_Y);
//	glutxInitWindowPosition(MAIN_WINDOW_POS_X, MAIN_WINDOW_POS_Y+13+MAIN_WINDOW_POS_Y);
	glutxInitDisplayMode();
	struct Exit{ static void Function(int){exit(0);}};
	gMainHandle = glutxCreateWindow(gAppTitle, Exit::Function);
	glutxSetWindow(gMainHandle);
//	glutPositionWindow(MAIN_WINDOW_POS_X, MAIN_WINDOW_POS_Y+13+MAIN_WINDOW_POS_Y);

	SetDefaultControlInterface();

	glutxDisplayFunc(RenderCallback);
	glutxReshapeFunc(ReshapeCallback);
	glutxIdleFunc(IdleCallback);
	glutxKeyboardFunc(KeyboardCallback);
	glutxSpecialKeyFunc(SpecialKeyCallback);
	glutxMouseFunc(MouseCallback);
	glutxMotionFunc(::MotionCallback);
	::MotionCallback(0,0);
//	glutFullScreen();
	//###ZAPPYGLUT
	glutxDropFileFunc(glutDropFile);
	glutxRawInputFunc(glutXRawInputFunc);
	glutxMouseFunc2(MouseCallback2);
	glutxMouseWheelFunc(MouseWheelCallback);
	//###ZAPPYGLUT

	if(1)
	{
		printf("GL_VENDOR = %s\n", glGetString(GL_VENDOR));
		printf("GL_RENDERER = %s\n", glGetString(GL_RENDERER));
		printf("GL_VERSION = %s\n", glGetString(GL_VERSION));
		printf("GL_SHADING_LANGUAGE_VERSION = %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
//		printf("GL_EXTENSIONS = %s\n", glGetString(GL_EXTENSIONS));
	}

	// We'll need the window handle to set the focus back to the render window
	gWindowHandle = WindowFromDC(wglGetCurrentDC());

	// Setup default render states
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	// Setup lighting - we need white lights not to interfere with the plugins' main colors
	glEnable(GL_LIGHTING);

	//glColorMask(0, 0, 0, 0);

//	const float ambientColor0[]	= { 0.0f, 0.0f, 0.0f, 0.0f };
	const float diffuseColor0[]	= { 1.0f, 1.0f, 1.0f, 0.0f };
//	const float specularColor0[]	= { 0.0f, 0.0f, 0.0f, 0.0f };
//	const float position0[]		= { 100.0f, 100.0f, 400.0f, 1.0f };
//	const float position0[]		= { 0.0f, 10.0f, 50.0f, 1.0f };
	Point Dir(-1.0f, 1.0f, 0.5f);
	Dir.Normalize();
	const float position0[]		= { Dir.x, Dir.y, Dir.z, 0.0f };
	if(0)
	{
//		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor0);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor0);
//		glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor0);
		glLightfv(GL_LIGHT0, GL_POSITION, position0);
		glEnable(GL_LIGHT0);
	}

	if(0)
	{
//		const float ambientColor1[]	= { 0.0f, 0.0f, 0.0f, 0.0f };
		const float diffuseColor1[]	= { 0.8f, 0.8f, 0.8f, 0.0f };
//		const float specularColor1[]	= { 0.0f, 0.0f, 0.0f, 0.0f };
//		const float position1[]		= { 0.0f, -10.0f, -50.0f, 1.0f };
		const float position1[]		= { -Dir.x, Dir.y, -Dir.z, 0.0f };
//		glLightfv(GL_LIGHT1, GL_AMBIENT, ambientColor1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseColor1);
//		glLightfv(GL_LIGHT1, GL_SPECULAR, specularColor1);
		glLightfv(GL_LIGHT1, GL_POSITION, position1);
		glEnable(GL_LIGHT1);
	}
	glEnable(GL_NORMALIZE);

	if(0)
	{
		glEnable(GL_FOG);
		//glFogi(GL_FOG_MODE,GL_LINEAR); 
		//glFogi(GL_FOG_MODE,GL_EXP); 
		glFogi(GL_FOG_MODE,GL_EXP2); 
		glFogf(GL_FOG_START, 0.0f);
		glFogf(GL_FOG_END, 1000.0f);
		glFogf(GL_FOG_DENSITY, 0.005f);
//		glClearColor(0.2f, 0.2f, 0.2f, 1.0);
		const Point FogColor(0.2f, 0.2f, 0.2f);
		glFogfv(GL_FOG_COLOR, &FogColor.x);
	}

	if(0)
	{
		// Setup Fog
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_DENSITY, 0.0009f);
		glFogf(GL_FOG_START, 20.0f);
		glFogf(GL_FOG_END, 100.0f);
	}

	glewInit();

	InitAtmosphere();
	InitRenderModels();
	InitScreenQuadModes();
	InitTools();

	EnableGLExtensions();
	GL_SelectVSYNC(gEnableVSync);

	InitScaledCylinders();
	GLPointRenderer::Init();
	GLPointRenderer2::Init();
	GLPointRenderer::SetScreenResolution(gScreenWidth, gScreenHeight);

	GLRenderStates::SetDefaultCullMode();

	gTexter.init();
	gTexter.setScreenResolution(gScreenWidth, gScreenHeight);
	gTexter.setColor(1.0f, 1.0f, 1.0f, 1.0f);

	{
		ASSERT(argc && argv);
		const String ExeFilename = *argv;

		gRoot = ICE_NEW(String);
		// ### compiler screw up right here, for some reason using GetPath crashes on x64 even though it's the same implementation
		IceCore::GetPath2(ExeFilename, *gRoot);
	}

	gCandidateTest = null;

/*	if(gUseEditor)
	{
		gPlugIns[gNbPlugIns++] = GetEditorPlugin();
	}*/

	const udword NbLoadedPlugs = AnalyzeCommandLine(argc, argv);
	if(!NbLoadedPlugs)
	{
		class SelectObjects : public SelectionDialog
		{
			public:
								SelectObjects(const SelectionDialogDesc& desc) : SelectionDialog(desc)	{}
								~SelectObjects()
								{
									if(mUserDefined[0])
									{
										gUseEditor = mUserDefined[0]->IsChecked();
//										printf("Editor: %d\n", mUserDefined[0]->IsChecked());
									}
								}

			virtual		void	OnSelected(void* user_data)
								{
	//								printf("Selected: %s\n", user_data);
									RegisterPlugIn((const char*)user_data);
								}
		};

		ICEGUIAPPCREATE Create;
		IceGUIApp MyApp;
		bool Status = MyApp.Init(Create);
		(void)Status;

		const udword DialogWidth	= 400;
		const udword DialogHeight	= 300;

		SelectionDialogDesc Desc;
		Desc.mMultiSelection		= true;
//		Desc.mFilters				= "Plugins";
		Desc.mParent				= null;
		Desc.mX						= 100;
		Desc.mY						= 100;
		Desc.mWidth					= DialogWidth;
		Desc.mHeight				= DialogHeight;
		Desc.mLabel					= "PEEL - Select plugins";
		Desc.mType					= WINDOW_DIALOG;
//		Desc.mType					= WINDOW_POPUP;
	//	Desc.mStyle					= WSTYLE_DLGFRAME;
		Desc.mStyle					= WSTYLE_HIDDEN;
		Desc.mCloseAppWithWindow	= true;
		Desc.mUserCheckboxes[0]		= "Enable editor";
		Desc.Center();
		SelectObjects* Main = ICE_NEW(SelectObjects)(Desc);

		Strings ss;
		if(1)
		{
			struct Local
			{
				static bool FindFilesCB(const char* filename, const char* path, void* user_data)
				{
					ASSERT(filename);
//					if(filename[0] && filename[0]!='P')	return false;
//					if(filename[1] && filename[1]!='I')	return false;
//					if(filename[2] && filename[2]!='N')	return false;
//					if(filename[3] && filename[3]!='T')	return false;
//					if(filename[4] && filename[4]!='_')	return false;

					// ### hardcoded and lame, but oh well
					const char* Dot = strchr(filename, '.');
					const bool IsDebug = Dot ? (Dot[-1]=='D' && Dot[-2]=='_') : false;
#ifdef _DEBUG
					const bool MustAdd = IsDebug;
#else
					const bool MustAdd = !IsDebug;
#endif
					if(MustAdd)
					{
						Strings* ss = (Strings*)user_data;
		//				Main->Add(FF.mFile, 0, NULL, TRUE);
						// ### TODO: make AddString return the string directly
						ss->AddString(filename);

/*						char buffer[MAX_PATH];
						strcpy(buffer, path);
						strcat(buffer, filename);
						ASSERT(strlen(buffer)<MAX_PATH);
						ss->AddString(buffer);*/
					}

					return true;
				}
			};

			FindFilesParams FFP;
			FFP.mMask = "PINT_*.dll";
//			FFP.mMask = "*.*";

//			String Root;
//			Status = _GetPath(ExeFilename, Root);
//			strcpy(FFP.mDir, Root.Get());
			strcpy(FFP.mDir, gRoot->Get());

			_FindFiles(FFP, Local::FindFilesCB, &ss);
		}

		udword NbS = ss.GetNbStrings();
		for(udword i=0;i<NbS;i++)
		{
			const String* CurrentS = ss.GetStrings()[i];
			Main->Add(CurrentS->Get(), 0, (void*)CurrentS->Get(), TRUE);
		}

		Main->Update();
		Main->Resize();
		Main->SetVisible(true);

		int Result = MyApp.Run();
		(void)Result;

		DELETESINGLE(Main);
	}

	const bool TestEditor = gUseEditor;
	if(TestEditor && gNbPlugIns)
	{
		if(gNbPlugIns!=1)
		{
			IceCore::MessageBoxA(null, "Editor behavior is undefined with more than 1 plugin selected!", "WARNING", MB_OK);
		}
		gPlugIns[gNbPlugIns++] = GetEditorPlugin();
//		PintPlugin* Saved = gPlugIns[0];
//		gPlugIns[0] = GetEditorPlugin();
//		SetEditedPlugin(Saved);
	}

//	printf("InitAll\n");
	InitAll(null);

//	InitLegoLib();
//	printf("InitGamepads\n");
	InitGamepads();

	atexit(gCleanup);

//	printf("PEEL_InitGUI\n");
	PEEL_InitGUI();

	if(TestEditor && gNbPlugIns)
	{
		gNbPlugIns--;
//		gPlugIns[gNbPlugIns++] = GetEditorPlugin();
		PintPlugin* Saved = gPlugIns[0];
		gPlugIns[0] = GetEditorPlugin();
		SetEditedPlugin(Saved);
	}

	timeBeginPeriod(1);

//	printf("SetBestRenderMode\n");
	if(1)
		SetBestRenderMode();

	if(gCandidateTest)
		ActivateTest();

	// Render one frame now to force shader compilation before running the scripts
	RenderCallback();

	if(gCmdLineScript[0])
		RunScript(FindPEELFile(gCmdLineScript));

//	printf("glutxMainLoop\n");
	glutxMainLoop();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

namespace
{
	enum TabIndex
	{
		TAB_MAIN,
		TAB_SCENE,
		TAB_TOOL,
		TAB_CAMERA,
		TAB_RAYTRACING,
		TAB_RENDER,
		TAB_SCREEN_QUAD,
		TAB_SUN,
		TAB_COUNT,
	};
}

static Widgets*		gMainGUI = null;
IceWindow*	gMainWindow = null;
//static IceWindow*	gMainWindow = null;

static EditPosWindow*	gEditBox_Gravity = null;
static IceEditBox*		gEditBox_SceneDesc = null;

static IceEditBox*	gEditBox_WireframeOverlayCoeff = null;
static IceEditBox*	gEditBox_ConvexEdgesThreshold = null;
static IceEditBox*	gEditBox_BatchSize = null;
static IceEditBox*	gEditBox_CameraSpeed = null;
static IceEditBox*	gEditBox_CameraNearClip = null;
static IceEditBox*	gEditBox_CameraFarClip = null;
static IceEditBox*	gEditBox_CameraFOV = null;
static IceEditBox*	gEditBox_CameraSensitivity = null;
static IceEditBox*	gEditBox_RaytracingDistance = null;
static IceEditBox*	gEditBox_SunPhi = null;
static IceEditBox*	gEditBox_SunTheta = null;
static IceEditBox*	gEditBox_SunIntensity = null;
static IceEditBox*	gEditBox_RayleighMultiplier = null;
static IceEditBox*	gEditBox_MieMultiplier = null;
static IceEditBox*	gEditBox_ScatteringMultiplier = null;
static IceEditBox*	gEditBox_ExtinctionMultiplier = null;
static IceEditBox*	gEditBox_HGg = null;

static IceComboBox*	gComboBox_CurrentTool = null;
static IceComboBox*	gComboBox_ProfilingUnits = null;
static IceComboBox*	gComboBox_UserProfilingMode = null;
static IceComboBox*	gComboBox_SQRaycastMode = null;
static IceComboBox*	gComboBox_RaytracingResolution = null;
static IceComboBox*	gComboBox_RenderModel = null;
static IceComboBox*	gComboBox_ScreenQuad = null;
static IceComboBox*	gComboBox_MSAA = null;

//static IceComboBox*	gComboBox_FogType = null;
static IceRadioButton*	gRadioButton_RT_Disabled = null;
static IceRadioButton*	gRadioButton_RT_ST = null;
static IceRadioButton*	gRadioButton_RT_MT = null;
static IceCheckBox*	gCheckBox_RT_Blit = null;

static IceEditBox*	gEditBox_RoundCornersSize = null;

static IceEditBox*	gEditBox_FrameSize = null;

/*static*/ IceWindow*	gColorPicker = null;

static IceCheckBox*	gCheckBox_Render = null;
static IceCheckBox*	gCheckBox_Wireframe = null;
static IceCheckBox*	gCheckBox_WireframeOverlay = null;
static IceCheckBox*	gCheckBox_GLFinish = null;

static IceCheckBox*	gCheckBox_RenderSun = null;

/*static*/ void EnableRendering(bool b)
{
	if(gRenderEnabled!=b)
	{
		gRenderEnabled = b;
		if(gCheckBox_Render)
			gCheckBox_Render->SetChecked(b);
	}
}

static void EnableWireframe(bool b)
{
	if(gWireframeEnabled!=b)
	{
		gWireframeEnabled = b;
		if(gCheckBox_Wireframe)
			gCheckBox_Wireframe->SetChecked(b);
	}
}

void EnableWireframeOverlay(bool b)
{
	if(gWireframeOverlayEnabled!=b)
	{
		gWireframeOverlayEnabled = b;
		if(gCheckBox_WireframeOverlay)
			gCheckBox_WireframeOverlay->SetChecked(b);
	}
}


enum MainGUIElement
{
	MAIN_GUI_MAIN,
	//
	MAIN_GUI_SCENE_GRAVITY,
	MAIN_GUI_SCENE_DESC,
	//
	MAIN_GUI_CAMERA_SPEED,
	MAIN_GUI_CAMERA_NEAR_CLIP,
	MAIN_GUI_CAMERA_FAR_CLIP,
	MAIN_GUI_CAMERA_FOV,
	MAIN_GUI_CAMERA_SENSITIVITY,
	MAIN_GUI_RAYTRACING_DISTANCE,
	MAIN_GUI_JOINT_TYPE,
	//
	MAIN_GUI_CURRENT_TOOL,
	MAIN_GUI_PROFILING_UNITS,
	MAIN_GUI_USER_PROFILING_MODE,
	MAIN_GUI_SQ_RAYCAST_MODE,
	MAIN_GUI_RAYTRACING_RESOLUTION,
	MAIN_GUI_ROUND_CORNERS_SIZE,
	MAIN_GUI_FRAME_SIZE,
	//
	MAIN_GUI_RAYTRACING_TEST_DISABLED,
	MAIN_GUI_RAYTRACING_TEST_ST,
	MAIN_GUI_RAYTRACING_TEST_MT,
	//
	MAIN_GUI_SCREEN_QUAD,
	MAIN_GUI_WIREFRAME_OVERLAY_COEFF,
	MAIN_GUI_CONVEX_EDGES_THRESHOLD,
	MAIN_GUI_BATCH_SIZE,
	MAIN_GUI_MSAA,
	MAIN_GUI_RENDER_MODEL,
	MAIN_GUI_FOG_TYPE,
	//
	MAIN_GUI_SUN_PHI,
	MAIN_GUI_SUN_THETA,
	MAIN_GUI_SUN_INTENSITY,
	MAIN_GUI_RAYLEIGH_MULTIPLIER,
	MAIN_GUI_MIE_MULTIPLIER,
	MAIN_GUI_SCATTERING_MULTIPLIER,
	MAIN_GUI_EXTINCTION_MULTIPLIER,
	MAIN_GUI_HG,

	//
	MAIN_GUI_DUMMY,
};

static volatile int trashSum = 0;
static int gTrashCacheThread(void* user_data)
{
	while(1)
	{
		printf("I'm trashing your cache, hahaha!\n");

		{
			const int size = 1024*1024*64;
			void* buf_ = ICE_ALLOC(size);
			volatile int* buf = (int*)buf_;
			for (int j = 0; j < size/sizeof(buf[0]); j++)
			{
				trashSum += buf[j];
				buf[j] = 0xdeadbeef;
			}
			ICE_FREE(buf_);
		}
	}
	return 1;
}

static const char* gTooltip_CurrentTool				= "Define what happens when pressing the right mouse button in simulation mode";
static const char* gTooltip_CameraSpeed				= "Camera's displacement speed when using the arrow keys";
static const char* gTooltip_CameraNearClip			= "Camera's Z near clip value";
static const char* gTooltip_CameraFarClip			= "Camera's Z far clip value";
static const char* gTooltip_CameraFOV				= "Camera's field of view in degrees";
static const char* gTooltip_CameraSensitivity		= "Camera's sensitivity";
static const char* gTooltip_RDTSC					= "Define profiling units in K-Cycles (checked) or ms (unchecked)";
static const char* gTooltip_RecordMemory			= "Profile performance (unchecked) or memory usage (checked)";
static const char* gTooltip_RaytracingTest			= "Raytrace current scene from current camera pose, using scene-level raycast calls. Results are dumped to console.";
static const char* gTooltip_RandomizeOrder			= "Randomize order in which physics engines are updated";
static const char* gTooltip_TrashCache				= "Trash cache after each simulation";
static const char* gTooltip_VSYNC					= "Enable/disable v-sync";
static const char* gTooltip_CommaSeparator			= "Use ',' or ';' as separator character in saved Excel files";
static const char* gTooltip_InfoMessages			= "Print info messages to console (or hide them)";
static const char* gTooltip_RaycastMode				= "Desired mode for SQ raycast tests. 'Closest' returns one closest hit, 'Any' returns the first hit and early exits, 'All' collects all hits touched by the ray.";
static const char* gTooltip_WireframeOverlayCoeff	= "Projection matrix tweak coeff for wireframe overlay (X key)";
static const char* gTooltip_BatchSize				= "Render batch size";
static const char* gTooltip_MSAA					= "Number of samples for Multi Sample Anti Aliasing";
static const char* gTooltip_RenderModel				= "Render style / pipeline / codepath";
static const char* gTooltip_ScreenQuad				= "Screen quad shader";
static const char* gTooltip_FrameSize				= "Size of debug render frame in various tools";

static void gPEEL_PollRadioButtons()
{
	if(gRadioButton_RT_Disabled && gRadioButton_RT_Disabled->IsChecked())
		gRaytracingTest = RAYTRACING_TEST_DISABLED;
	else if(gRadioButton_RT_ST && gRadioButton_RT_ST->IsChecked())
		gRaytracingTest = RAYTRACING_TEST_ST;
	else if(gRadioButton_RT_MT && gRadioButton_RT_MT->IsChecked())
		gRaytracingTest = RAYTRACING_TEST_MT;
}

void UI_UpdateCameraSpeed()
{
	if(gEditBox_CameraSpeed)
	{
		gCameraSpeedEdit = GetCameraSpeed();
		gEditBox_CameraSpeed->SetText(_F("%f", gCameraSpeedEdit));
	}
}

// Items included here are changed when the test is reset
static void gPEEL_GetOptionsFromGUI()
{
	gRoundCornersSize		= GetInt(gRoundCornersSize, gEditBox_RoundCornersSize);
	gCameraSpeedEdit		= GetFloat(gCameraSpeedEdit, gEditBox_CameraSpeed);
	gCameraNearClipEdit		= GetFloat(gCameraNearClipEdit, gEditBox_CameraNearClip);
	gCameraFarClipEdit		= GetFloat(gCameraFarClipEdit, gEditBox_CameraFarClip);
	gCameraFOVEdit			= GetFloat(gCameraFOVEdit, gEditBox_CameraFOV);
	gCameraSensitivityEdit	= GetFloat(gCameraSensitivityEdit, gEditBox_CameraSensitivity);
	gWireframeOverlayCoeff	= GetFloat(gWireframeOverlayCoeff, gEditBox_WireframeOverlayCoeff);
	gConvexEdgesThreshold	= GetFloat(gConvexEdgesThreshold, gEditBox_ConvexEdgesThreshold);
	gBatchSize				= GetInt(gBatchSize, gEditBox_BatchSize);
	gRTDistance				= GetFloat(gRTDistance, gEditBox_RaytracingDistance);

	if(gComboBox_CurrentTool)
	{
		const udword SelectedIndex = gComboBox_CurrentTool->GetSelectedIndex();
		if(SelectedIndex != GetCurrentToolIndex())
			SelectTool(SelectedIndex);
	}

	if(gComboBox_ProfilingUnits)
	{
		const udword Index = gComboBox_ProfilingUnits->GetSelectedIndex();
		gProfilingUnits = ProfilingUnits(Index);
	}

	if(gComboBox_UserProfilingMode)
	{
		const udword Index = gComboBox_UserProfilingMode->GetSelectedIndex();
		gUserProfilingMode = UserProfilingMode(Index);
	}

	if(gComboBox_SQRaycastMode)
	{
		const udword Index = gComboBox_SQRaycastMode->GetSelectedIndex();
		gSQRaycastMode = SQRaycastMode(Index);
		//gRaycastClosest = gSQRaycastMode==SQ_RAYCAST_CLOSEST;
		gRaycastMode = Index;
	}

	if(gComboBox_RaytracingResolution)
	{
		const udword Index = gComboBox_RaytracingResolution->GetSelectedIndex();
		gRaytracingResolution = RaytracingResolution(Index);
	}

	if(gComboBox_RenderModel)
	{
		const udword SelectedIndex = gComboBox_RenderModel->GetSelectedIndex();
		if(SelectedIndex != gRenderModelIndex)
			SelectRenderModel(SelectedIndex);
	}

	if(gComboBox_ScreenQuad)
	{
		const udword SelectedIndex = gComboBox_ScreenQuad->GetSelectedIndex();
		if(SelectedIndex != gScreenQuadModeIndex)
			SelectScreenQuadMode(SelectedIndex);
	}

	if(gComboBox_MSAA)
	{
		const udword SelectedIndex = gComboBox_MSAA->GetSelectedIndex();
		Select_MSAA(SelectedIndex);
	}

/*	if(gComboBox_FogType)
	{
		const udword SelectedIndex = gComboBox_FogType->GetSelectedIndex();
//		Select_MSAA(SelectedIndex);
	}*/
}

// Items included here can be changed while the test is running
static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
{
//	printf("gEBCallback\n");
	const udword ID = edit_box.GetID();
	if(ID==MAIN_GUI_CAMERA_SPEED)
		gCameraSpeedEdit = GetFloat(gCameraSpeedEdit, &edit_box);
	else if(ID==MAIN_GUI_CAMERA_NEAR_CLIP)
		gCameraNearClipEdit = GetFloat(gCameraNearClipEdit, &edit_box);
	else if(ID==MAIN_GUI_CAMERA_FAR_CLIP)
		gCameraFarClipEdit = GetFloat(gCameraFarClipEdit, &edit_box);
	else if(ID==MAIN_GUI_CAMERA_FOV)
		gCameraFOVEdit = GetFloat(gCameraFOVEdit, &edit_box);
	else if(ID==MAIN_GUI_CAMERA_SENSITIVITY)
		gCameraSensitivityEdit = GetFloat(gCameraSensitivityEdit, &edit_box);
	else if(ID==MAIN_GUI_RAYTRACING_DISTANCE)
		gRTDistance = GetFloat(gRTDistance, &edit_box);
	else if(ID==MAIN_GUI_ROUND_CORNERS_SIZE)
		gRoundCornersSize = GetInt(gRoundCornersSize, &edit_box);
	else if(ID==MAIN_GUI_FRAME_SIZE)
		gFrameSize = GetFloat(gFrameSize, &edit_box);
	else if(ID==MAIN_GUI_WIREFRAME_OVERLAY_COEFF)
		gWireframeOverlayCoeff = GetFloat(gWireframeOverlayCoeff, &edit_box);
	else if(ID==MAIN_GUI_BATCH_SIZE)
		gBatchSize = GetInt(gBatchSize, &edit_box);
	else if(ID==MAIN_GUI_SUN_THETA)
		gSunTheta = GetFloat(gSunTheta, &edit_box);
	else if(ID==MAIN_GUI_SUN_PHI)
		gSunPhi = GetFloat(gSunPhi, &edit_box);
	else if(ID==MAIN_GUI_SUN_INTENSITY)
		gSunIntensity = GetFloat(gSunIntensity, &edit_box);
	else if(ID==MAIN_GUI_RAYLEIGH_MULTIPLIER)
		gRayleighMultiplier = GetFloat(gRayleighMultiplier, &edit_box);
	else if(ID==MAIN_GUI_MIE_MULTIPLIER)
		gMieMultiplier = GetFloat(gMieMultiplier, &edit_box);
	else if(ID==MAIN_GUI_SCATTERING_MULTIPLIER)
		gScatteringMultiplier = GetFloat(gScatteringMultiplier, &edit_box);
	else if(ID==MAIN_GUI_EXTINCTION_MULTIPLIER)
		gExtinctionMultiplier = GetFloat(gExtinctionMultiplier, &edit_box);
	else if(ID==MAIN_GUI_HG)
		gHGg = GetFloat(gHGg, &edit_box);
}

class ToolComboBox : public IceComboBox
{
	public:
					ToolComboBox(const ComboBoxDesc& desc) : IceComboBox(desc){}

	virtual	void	OnComboBoxEvent(ComboBoxEvent event)
	{
		if(event==CBE_SELECTION_CHANGED)
		{
			const ToolIndex SelectedTool = ToolIndex(GetSelectedIndex());
			if(SelectedTool!=GetCurrentToolIndex())
				SelectTool(SelectedTool);
//			printf("OnComboBoxEvent: %d\n", gCurrentTool);
		}
	}
};

class MultiComboBox : public IceComboBox
{
	public:
					MultiComboBox(const ComboBoxDesc& desc) : IceComboBox(desc){}

	virtual	void	OnComboBoxEvent(ComboBoxEvent event)
	{
		if(event==CBE_SELECTION_CHANGED)
		{
			const udword ID = GetID();
			const udword SelectedIndex = GetSelectedIndex();
			if(ID==MAIN_GUI_RAYTRACING_RESOLUTION)
			{
				gRaytracingResolution = RaytracingResolution(SelectedIndex);
			}
			else if(ID==MAIN_GUI_RENDER_MODEL)
			{
				if(SelectedIndex != gRenderModelIndex)
					SelectRenderModel(SelectedIndex);
			}
			else if(ID==MAIN_GUI_SCREEN_QUAD)
			{
				if(SelectedIndex != gScreenQuadModeIndex)
					SelectScreenQuadMode(SelectedIndex);
			}
			else if(ID==MAIN_GUI_MSAA)
			{
				Select_MSAA(SelectedIndex);
			}
			else if(ID==MAIN_GUI_SQ_RAYCAST_MODE)
			{
				gSQRaycastMode = SQRaycastMode(GetSelectedIndex());
				//gRaycastClosest = gSQRaycastMode==SQ_RAYCAST_CLOSEST;
				gRaycastMode = GetSelectedIndex();
				ResetSQHelpersHitData();
			}
		}
	}
};

/*static void gButtonCallbackScript(IceButton& button, void* user_data)
{
	FILESELECTCREATE Create;
	Create.mFilter			= "Text files (*.txt)|*.txt|All Files (*.*)|*.*||";
	Create.mFileName		= "TestScript.txt";
	Create.mInitialDir		= *gRoot;
	Create.mCaptionTitle	= "Select script file";
	Create.mDefExt			= "txt";

	String ScriptFilename;
	if(!FileselectOpenSingle(Create, ScriptFilename))
		return;

	RunScript(ScriptFilename);
}*/

/*static void gButtonCallbackScene(IceButton& button, void* user_data)
{
	FILESELECTCREATE Create;
	Create.mFilter			= "ZB2 Files (*.zb2)|*.zb2|All Files (*.*)|*.*||";
	Create.mFileName		= "scene.zb2";
	Create.mInitialDir		= *gRoot;
	Create.mCaptionTitle	= "Import scene";
	Create.mDefExt			= "zb2";

	String Filename;
	if(!FileselectOpenSingle(Create, Filename))
		return;

	ZCB2Factory Loader;
	bool status = Loader.Import(Filename);
}*/

static const sdword OffsetX = 90;
static const sdword EditBoxWidth = 60;
static const sdword LabelOffsetY = 2;
static const sdword YStep = 20;

static void CreateMainTab(IceWindow* tab)
{
	sdword y = 4;
	{
		struct RandomizeOrder{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gRandomizeOrder = checked; }};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Randomize physics engines order", gMainGUI, gRandomizeOrder, RandomizeOrder::Lambda, gTooltip_RandomizeOrder);
		y += YStep;

		struct TrashCache{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gTrashCache = checked; }};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Trash cache", gMainGUI, gTrashCache, TrashCache::Lambda, gTooltip_TrashCache);
		y += YStep;

		struct VSync{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gEnableVSync = checked; GL_SelectVSYNC(checked);	}};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "VSYNC", gMainGUI, gEnableVSync, VSync::Lambda, gTooltip_VSYNC);
		y += YStep;

		struct CommaSeparator{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gCommaSeparator = checked; }};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Use comma separator", gMainGUI, gCommaSeparator, CommaSeparator::Lambda, gTooltip_CommaSeparator);
		y += YStep;

		struct InfoMessages{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gDisplayInfoMessages = checked; }};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Print info messages to console", gMainGUI, gCommaSeparator, InfoMessages::Lambda, gTooltip_InfoMessages);
		y += YStep;

		struct RoundCorners{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gRoundCorners = checked; }};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Round corners", gMainGUI, gRoundCorners, RoundCorners::Lambda, null/*gTooltip_*/);
		y += YStep;

		gGUIHelper.CreateLabel(tab, 0, y+LabelOffsetY, 90, 20, "Corners size:", gMainGUI);
		gEditBox_RoundCornersSize = gGUIHelper.CreateEditBox(tab, MAIN_GUI_ROUND_CORNERS_SIZE, OffsetX, y, EditBoxWidth, 20, _F("%d", gRoundCornersSize), gMainGUI, EDITBOX_INTEGER_POSITIVE, gEBCallback, null/*gTooltip_PickingForce*/);
		y += YStep;

		gGUIHelper.CreateLabel(tab, 0, y+LabelOffsetY, 90, 20, "Frame size:", gMainGUI);
		gEditBox_FrameSize = gGUIHelper.CreateEditBox(tab, MAIN_GUI_FRAME_SIZE, OffsetX, y, EditBoxWidth, 20, _F("%f", gFrameSize), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, gTooltip_FrameSize);
		y += YStep;

		struct SymmetricFrames{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gSymmetricFrames = checked; }};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Symmetric frames", gMainGUI, gSymmetricFrames, SymmetricFrames::Lambda, null/*gTooltip_*/);
		y += YStep;

		struct FatFrames{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gUseFatFrames = checked; }};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Use fat frames", gMainGUI, gUseFatFrames, FatFrames::Lambda, null/*gTooltip_*/);
		y += YStep;
	}

	{
		gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Profiling units:", gMainGUI);
		gComboBox_ProfilingUnits = CreateComboBox<IceComboBox>(tab, MAIN_GUI_PROFILING_UNITS, 4+OffsetX, y, 150, 20, "Profiling units", gMainGUI, null);
		gComboBox_ProfilingUnits->Add("K-Cycles (RDTSC)");
		gComboBox_ProfilingUnits->Add("ms (timeGetTime)");
		gComboBox_ProfilingUnits->Add("us (QPC)");
		gComboBox_ProfilingUnits->Select(gProfilingUnits);
		y += YStep;
	}
	{
		gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Profiling mode:", gMainGUI);
		gComboBox_UserProfilingMode = CreateComboBox<IceComboBox>(tab, MAIN_GUI_USER_PROFILING_MODE, 4+OffsetX, y, 150, 20, "Profiling mode", gMainGUI, null);
		gComboBox_UserProfilingMode->Add("Per-test default");
		gComboBox_UserProfilingMode->Add("Sim update");
		gComboBox_UserProfilingMode->Add("Test update");
		gComboBox_UserProfilingMode->Add("Combined");
		gComboBox_UserProfilingMode->Select(gUserProfilingMode);
		y += YStep;
	}
	{
		gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Raycast mode:", gMainGUI);
		gComboBox_SQRaycastMode = CreateComboBox<MultiComboBox>(tab, MAIN_GUI_SQ_RAYCAST_MODE, 4+OffsetX, y, 150, 20, "SQ raycast mode", gMainGUI, gTooltip_RaycastMode);
		gComboBox_SQRaycastMode->Add("Raycast closest");
		gComboBox_SQRaycastMode->Add("Raycast any");
		gComboBox_SQRaycastMode->Add("Raycast all");
		gComboBox_SQRaycastMode->Select(gSQRaycastMode);
		y += YStep;

		struct SolidImpactShapes{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gRenderSolidImpactShapes = checked; }};
		gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Draw solid impact shapes", gMainGUI, gRenderSolidImpactShapes, SolidImpactShapes::Lambda, null/*gTooltip_*/);
		y += YStep;
	}
}

Point UI_GetGravity()
{
	return gEditBox_Gravity ? gEditBox_Gravity->GetPos() : gDefaultGravity;
}

void UI_SetGravity(const Point& gravity)
{
	if(gEditBox_Gravity)
		gEditBox_Gravity->SetPos(gravity);
}

bool UI_GetSceneDesc(String& text)
{
	if(!gEditBox_SceneDesc)
		return false;

	gEditBox_SceneDesc->GetTextAsString(text);
	return true;
}

static bool IsCarriageReturn(char c, udword i, udword length, const char* text)
{
	if(c==ASCII_CARRIAGE_RETURN && (i+1)<length && text[i+1]==ASCII_NEXT_LINE)
		return true;
	return false;
}

static bool HasCarriageReturn(const char* text, udword length)
{
	for(udword i=0;i<length;i++)
	{
		const char c = text[i];
		if(IsCarriageReturn(c, i, length, text))
			return true;
	}
	return false;
}

void UI_SetSceneDesc(const char* text)
{
//	if(gEditBox_SceneDesc)
//		gEditBox_SceneDesc->SetMultilineText(text);

	if(!gEditBox_SceneDesc)
		return;

	const udword Length = udword(strlen(text));
	if(!Length)
	{
		gEditBox_SceneDesc->SetText("");
		return;
	}

	const bool HasCR = HasCarriageReturn(text, Length);

	// ### doesn't work well because all characters don't have the same width
	const udword NbCharsPerLine = 80;

	char* Buffer = (char*)ICE_ALLOC(1+sizeof(char)*Length*2);
	udword Offset = 0;
	udword NbChars = 0;
	udword i=0;
	while(i<Length)
	{
		const char c = text[i];

		if(HasCR && IsCarriageReturn(c, i, Length, text))
		{
			NbChars = 0;
			Buffer[Offset++] = ASCII_CARRIAGE_RETURN;
			Buffer[Offset++] = ASCII_NEXT_LINE;
			i+=2;
		}
		else
		{
			bool GotoNextLine = false;
			if(!HasCR && c==' ')
			{
				for(udword j=i+1;j<Length;j++)
				{
					if(text[j]==' ' || IsCarriageReturn(text[j], j, Length, text))
					{
						udword NbChars2 = j-i-1;
						GotoNextLine = (NbChars + NbChars2)>NbCharsPerLine;
						break;
					}
				}

				if(GotoNextLine)
				{
					NbChars = 0;
					Buffer[Offset++] = ASCII_CARRIAGE_RETURN;
					Buffer[Offset++] = ASCII_NEXT_LINE;
				}
			}

			if(!GotoNextLine)
			{
				Buffer[Offset++] = c;
				NbChars++;
			}

			i++;
		}
	}
	Buffer[Offset] = 0;
	gEditBox_SceneDesc->SetText(Buffer);
	ICE_FREE(Buffer);


//	int maxLength = gEditBox_SceneDesc->GetMaxLength();
//	printf("Length: %d\n", Length);
//	printf("maxLength: %d\n", maxLength);
}

namespace
{
class GravityManager : public EditBoxInterface
{
	public:
	virtual	void	ChangeNotification()
	{
		const Point G = gEditBox_Gravity->GetPos();
		gDefaultGravity = G;
		//printf("%f %f %f\n", G.x, G.y, G.z);
		for(udword i=0;i<gNbPlugIns;i++)
		{
			Pint* Engine = gPlugIns[i]->GetPint();
			Engine->SetGravity(G);
		}
	}
}gGravityManager;
}

static void gColorPickerButtonCallback(IceButton& button, void* user_data);

static void CreateSceneTab(IceWindow* tab)
{
	sdword y = 4;
	sdword x = 4;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Default gravity:", gMainGUI);
	{
		WindowDesc WD;
		WD.mParent	= tab;
		WD.mID		= MAIN_GUI_SCENE_GRAVITY;
		WD.mX		= x+OffsetX;
		WD.mY		= y;
		WD.mWidth	= 90;
		WD.mHeight	= 70;
		WD.mType	= WINDOW_NORMAL;
		WD.mStyle	= WSTYLE_BORDER;
//		WD.mStyle	= WSTYLE_CLIENT_EDGES;
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
		gEditBox_Gravity = ICE_NEW(EditPosWindow)(WD, &gGravityManager);
		gEditBox_Gravity->SetPos(gDefaultGravity);
		gMainGUI->Register(gEditBox_Gravity);

		y += 80;
	}

	struct AddDefaultEnv{ static void Lambda(IceButton& button, void* user_data)	{
		for(udword i=0;i<gNbPlugIns;i++)
		{
			Pint* Engine = gPlugIns[i]->GetPint();
			if(Engine)
				SetupDefaultEnvironment(*Engine, true);
		}
	}};
	gGUIHelper.CreateButton(tab, 0, 200, 8, 130, 20, "Add default env", gMainGUI, AddDefaultEnv::Lambda, null);

	struct RemoveDefaultEnv{ static void Lambda(IceButton& button, void* user_data)	{
		for(udword i=0;i<gNbPlugIns;i++)
		{
			Pint* Engine = gPlugIns[i]->GetPint();
			if(Engine)
				ReleaseDefaultEnvironment(*Engine);
		}
	}};
	gGUIHelper.CreateButton(tab, 0, 200, 28, 130, 20, "Remove default env", gMainGUI, RemoveDefaultEnv::Lambda, null);

	gGUIHelper.CreateButton(tab, COLOR_PICKER_PLANE_COLOR, 200, 48, 130, 20, "Change plane color", gMainGUI, gColorPickerButtonCallback, null, "Change reflective plane color");

	//gGUIHelper.CreateEditBox(tab, 0, 4, y, EditBoxWidth, 20, "Scene description", gMainGUI, EDITBOX_TEXT, null, null);
	{
		struct Local
		{
			static void gEBCallback(const IceEditBox& edit_box, udword param, void* user_data)
			{
				//String text;
				//edit_box.GetTextAsString(text);
				printf("gEBCallback: %d\n", param);

/*				if(param!=IceEditBox::CB_PARAM_RETURN)	
					return;

				EditPosWindow* EPW = reinterpret_cast<EditPosWindow*>(user_data);
				ASSERT(EPW);

				//printf("gEBCallback\n");
				const udword ID = edit_box.GetID();
				if(ID==X_)
					EPW->mEdited.x = GetFloat(EPW->mEdited.x, &edit_box);
				else if(ID==Y_)
					EPW->mEdited.y = GetFloat(EPW->mEdited.y, &edit_box);
				else if(ID==Z_)
					EPW->mEdited.z = GetFloat(EPW->mEdited.z, &edit_box);
				EPW->mHasEdit = true;*/
			}
		};

		EditBoxDesc EBD;
		EBD.mID			= MAIN_GUI_SCENE_DESC;
		EBD.mParent		= tab;
		EBD.mX			= 4;
		EBD.mY			= y;
		EBD.mWidth		= 400;
		EBD.mHeight		= 300;
		EBD.mLabel		= "Scene description";
		EBD.mFilter		= EDITBOX_TEXT;
//		EBD.mCallback	= Local::gEBCallback;
		gEditBox_SceneDesc = ICE_NEW(IceEditBox)(EBD);
		gEditBox_SceneDesc->SetVisible(true);
//		if(owner)
			gMainGUI->Register(gEditBox_SceneDesc);

//		if(tooltip)
//			EB->AddToolTip(tooltip);

		gEditBox_SceneDesc->SetMaxLength(8192);
	}

	tab->SetVisible(false);
}

void UpdateToolComboBox()
{
	gComboBox_CurrentTool->Select(GetCurrentToolIndex());
}

static void CreateToolTab(IceWindow* tab)
{
	sdword y = 4;
	{
		gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Current tool:", gMainGUI);
		gComboBox_CurrentTool = CreateComboBox<ToolComboBox>(tab, MAIN_GUI_CURRENT_TOOL, 4+OffsetX, y, 150, 200, "Current tool", gMainGUI, gTooltip_CurrentTool);
		const udword NbTools = GetNbTools();
		for(udword i=0;i<NbTools;i++)
			gComboBox_CurrentTool->Add(GetToolName(i));
//		gComboBox_CurrentTool->Select(GetCurrentToolIndex());
		UpdateToolComboBox();
		y += YStep;
	}

	y += YStep;
	const sdword ToolSpecificAreaWidth = 480;
	const sdword ToolSpecificAreaHeight = 400;

	for(udword i=0;i<TOOL_COUNT;i++)
	{
		sdword y2 = y;
		y2 += YStep;

		WindowDesc WD;
		WD.mParent	= tab;
		WD.mX		= 8;
		WD.mY		= y2;
		WD.mWidth	= 470;
		WD.mHeight	= 370;
		WD.mType	= WINDOW_DIALOG;
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
		//WD.mStyle	= WSTYLE_BORDER;

		IceWindow* OptionWindow = ICE_NEW(IceWindow)(WD);
		gMainGUI->Register(OptionWindow);
		OptionWindow->SetVisible(true);
		gToolOptions[i] = OptionWindow;
//		gRenderModel[i]->InitGUI(OptionWindow, gMainGUI, gGUIHelper);

		ToolInterface* T = GetTool(i);
		if(T)
			T->CreateUI(gGUIHelper, OptionWindow, *gMainGUI);
	}
		if(1)
		{
			IceEditBox* tmp = gGUIHelper.CreateEditBox(tab, MAIN_GUI_DUMMY, 2, y, ToolSpecificAreaWidth, ToolSpecificAreaHeight, "=== Tool-specific settings ===", gMainGUI, EDITBOX_TEXT, null);
//			tmp->AddToolTip(gTooltip_RaytracingTest);
			tmp->SetReadOnly(true);
			y += ToolSpecificAreaHeight;
		}
	tab->SetVisible(false);
}

static void CreateCameraTab(IceWindow* tab)
{
	sdword y = 4;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Camera speed:", gMainGUI);
	gEditBox_CameraSpeed = gGUIHelper.CreateEditBox(tab, MAIN_GUI_CAMERA_SPEED, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gCameraSpeedEdit), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, gTooltip_CameraSpeed);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Camera near clip:", gMainGUI);
	gEditBox_CameraNearClip = gGUIHelper.CreateEditBox(tab, MAIN_GUI_CAMERA_NEAR_CLIP, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gCameraNearClipEdit), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, gTooltip_CameraNearClip);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Camera far clip:", gMainGUI);
	gEditBox_CameraFarClip = gGUIHelper.CreateEditBox(tab, MAIN_GUI_CAMERA_FAR_CLIP, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gCameraFarClipEdit), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, gTooltip_CameraFarClip);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Camera FOV:", gMainGUI);
	gEditBox_CameraFOV = gGUIHelper.CreateEditBox(tab, MAIN_GUI_CAMERA_FOV, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gCameraFOVEdit), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, gTooltip_CameraFOV);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Camera sensitivity:", gMainGUI);
	gEditBox_CameraSensitivity = gGUIHelper.CreateEditBox(tab, MAIN_GUI_CAMERA_SENSITIVITY, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gCameraSensitivityEdit), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, gTooltip_CameraSensitivity);
	y += YStep;

	struct FreezeFrustum{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)
	{
		gCameraFreezeFrustum = checked;
		gFreezeFrustum = checked;
		if(gFreezeFrustum)
			CaptureFrustum();
	}};
	gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "Freeze frustum", gMainGUI, gCameraFreezeFrustum, FreezeFrustum::Lambda, null);
	y += YStep;

	struct VFC{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gCameraVFC = checked; }};
	gGUIHelper.CreateCheckBox(tab, 0, 4, y, 200, 20, "VFC", gMainGUI, gCameraVFC, VFC::Lambda, null);
	y += YStep;

	struct ResetCamera{ static void Lambda(IceButton& button, void* user_data)	{ gCamera.Reset(); }};
	gGUIHelper.CreateButton(tab, 0, 4, y, 130, 20, "Reset view", gMainGUI, ResetCamera::Lambda, null);
	y += YStep;

	struct NextCamera{ static void Lambda(IceButton& button, void* user_data)	{ gCamera.SelectNextCamera(); }};
	gGUIHelper.CreateButton(tab, 0, 4, y, 130, 20, "Select next camera", gMainGUI, NextCamera::Lambda, null);
	y += YStep;

	struct PreviousCamera{ static void Lambda(IceButton& button, void* user_data)	{ gCamera.SelectPreviousCamera(); }};
	gGUIHelper.CreateButton(tab, 0, 4, y, 130, 20, "Select previous camera", gMainGUI, PreviousCamera::Lambda, null);
	y += YStep;

	struct CamPoseToClipboard{ static void Lambda(IceButton& button, void* user_data)	{ CopyCameraPoseToClipboard();  }};
	gGUIHelper.CreateButton(tab, 0, 4, y, 130, 20, "Cam pose to clipboard", gMainGUI, CamPoseToClipboard::Lambda, null);
	y += YStep;

	tab->SetVisible(false);
}

static void CreateRaytraceTab(IceWindow* tab)
{
	sdword y = 4;
	if(1)
	{
		y = 4;
//		y = TITLE_HEIGHT + 70 + 10;
//		y += YStep;
		const sdword saved = y;
		y += YStep;

		const sdword x = 20;

		{
			struct Blit{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gBlitToScreen = checked; }};
			gCheckBox_RT_Blit = gGUIHelper.CreateCheckBox(tab, 0, x, y, 80, 20, "Blit to screen", gMainGUI, gBlitToScreen, Blit::Lambda, null);
			y += YStep;
		}

		RadioButtonDesc RBD;
		RBD.mNewGroup	= true;
		RBD.mID			= MAIN_GUI_RAYTRACING_TEST_DISABLED;
		RBD.mParent		= tab;
//		RBD.mX			= 10;
		RBD.mX			= x;
//		RBD.mX			= 310;
		RBD.mY			= y;
		RBD.mWidth		= 100;
		RBD.mHeight		= 20;
		RBD.mLabel		= "Disabled";
		gRadioButton_RT_Disabled = ICE_NEW(IceRadioButton)(RBD);
		gMainGUI->Register(gRadioButton_RT_Disabled);
		//gRadioButton_RT_Disabled->AddToolTip(gTooltip_RaytracingTest);
		y += YStep;

		RBD.mNewGroup	= false;
		RBD.mID			= MAIN_GUI_RAYTRACING_TEST_ST;
		RBD.mY			= y;
		RBD.mLabel		= "Single threaded";
		gRadioButton_RT_ST = ICE_NEW(IceRadioButton)(RBD);
		gMainGUI->Register(gRadioButton_RT_ST);
		//gRadioButton_RT_ST->AddToolTip(gTooltip_RaytracingTest);
		y += YStep;

		RBD.mNewGroup	= false;
		RBD.mID			= MAIN_GUI_RAYTRACING_TEST_MT;
		RBD.mY			= y;
		RBD.mLabel		= "Multi threaded";
		gRadioButton_RT_MT = ICE_NEW(IceRadioButton)(RBD);
		gMainGUI->Register(gRadioButton_RT_MT);
		//gRadioButton_RT_MT->AddToolTip(gTooltip_RaytracingTest);
		y += YStep;

		{
			gGUIHelper.CreateLabel(tab, x, y+LabelOffsetY, 50, 20, "Max dist:", gMainGUI);
			gEditBox_RaytracingDistance = gGUIHelper.CreateEditBox(tab, MAIN_GUI_RAYTRACING_DISTANCE, x+50, y, EditBoxWidth, 20, gGUIHelper.Convert(gRTDistance), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback/*, gTooltip_CameraSpeed*/);
		}

		{
			y += YStep;
			//gGUIHelper.CreateLabel(MainOptions, 4, y+LabelOffsetY, 90, 20, "RT resolution:", gMainGUI);
			gComboBox_RaytracingResolution = CreateComboBox<MultiComboBox>(tab, MAIN_GUI_RAYTRACING_RESOLUTION, x, y, 150, 20, "RT resolution", gMainGUI, null);
			gComboBox_RaytracingResolution->Add("64x64 rays");
			gComboBox_RaytracingResolution->Add("128*128 rays");
			gComboBox_RaytracingResolution->Add("256*256 rays");
			gComboBox_RaytracingResolution->Add("512*512 rays");
			gComboBox_RaytracingResolution->Add("768*768 rays");
			gComboBox_RaytracingResolution->Select(gRaytracingResolution);
			y += YStep;
		}

//		IceEditBox* tmp = gGUIHelper.CreateEditBox(tab, MAIN_GUI_DUMMY, 4, saved, 120, 100, "=== Raytracing test ===", gMainGUI, EDITBOX_TEXT, null);
//		IceEditBox* tmp = gGUIHelper.CreateEditBox(tab, MAIN_GUI_DUMMY, 296, saved, 180, 140, "=== Raytracing test ===", gMainGUI, EDITBOX_TEXT, null);
		IceEditBox* tmp = gGUIHelper.CreateEditBox(tab, MAIN_GUI_DUMMY, x+296-310, saved, 180, 160, "=== Raytracing test ===", gMainGUI, EDITBOX_TEXT, null, gTooltip_RaytracingTest);
//		tmp->AddToolTip(gTooltip_RaytracingTest);
		tmp->SetReadOnly(true);
	}
	tab->SetVisible(false);

	if(0)
	{
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= 0;
		WD.mY		= 0;
		WD.mWidth	= 512;
		WD.mHeight	= 80;
		WD.mLabel	= "Loading... Please wait";
		WD.mType	= WINDOW_DIALOG;
//		WD.mType	= WINDOW_POPUP;
		WD.mStyle	= WSTYLE_ALWAYS_ON_TOP|WSTYLE_CLIENT_EDGES;
		WD.Center();
		IceWindow* TestWindow = ICE_NEW(IceWindow)(WD);
		gMainGUI->Register(TestWindow);
		TestWindow->SetVisible(true);

		ProgressBarDesc PBD;
//		PBD.mID			= MAIN_GUI_CURRENT_TOOL;
		PBD.mParent		= TestWindow;
		PBD.mX			= 20;
		PBD.mY			= 10;
		PBD.mWidth		= 512-20*2;
		PBD.mHeight		= 20;
//		PBD.mLabel		= "Current tool";
//		PBD.mStyle		= PROGRESSBAR_SMOOTH;
		IceProgressBar* ProgressBar = ICE_NEW(IceProgressBar)(PBD);
		gMainGUI->Register(ProgressBar);
		ProgressBar->SetVisible(true);
		ProgressBar->SetTotalSteps(100);
		ProgressBar->SetValue(40);
	}
}

static void CreateRenderTab(IceWindow* tab)
{
	sdword y = 4;

	// We don't need to update the UI in the UI callback itself, so just modify gRenderEnabled directly
	struct Render{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gRenderEnabled = checked; }};
	gCheckBox_Render = gGUIHelper.CreateCheckBox(tab, 0, 4, y, 80, 20, "Render", gMainGUI, gRenderEnabled, Render::Lambda, null);
//	y += YStep;

	struct Wireframe{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gWireframeEnabled = checked; }};
	gCheckBox_Wireframe = gGUIHelper.CreateCheckBox(tab, 0, 4+100, y, 80, 20, "Wireframe", gMainGUI, gWireframeEnabled, Wireframe::Lambda, null);
//	y += YStep;

	struct WireframeOverlay{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gWireframeOverlayEnabled = checked; }};
	gCheckBox_WireframeOverlay = gGUIHelper.CreateCheckBox(tab, 0, 4+200, y, 80, 20, "Overlay", gMainGUI, gWireframeOverlayEnabled, WireframeOverlay::Lambda, null);

	struct GLFinish{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gGLFinish = checked; }};
	gCheckBox_GLFinish = gGUIHelper.CreateCheckBox(tab, 0, 4+300, y, 80, 20, "GLFinish", gMainGUI, gGLFinish, GLFinish::Lambda, null);
	y += YStep;

	{
	const udword OffsetX2 = 0;
	gGUIHelper.CreateLabel(tab, 4+OffsetX2, y+LabelOffsetY, 90, 20, "Wireframe overlay:", gMainGUI);
	gEditBox_WireframeOverlayCoeff = gGUIHelper.CreateEditBox(tab, MAIN_GUI_WIREFRAME_OVERLAY_COEFF, 4+OffsetX+OffsetX2, y, EditBoxWidth, 20, gGUIHelper.Convert(gWireframeOverlayCoeff), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, gTooltip_WireframeOverlayCoeff);
	}

	{
	const udword LocalLabelWidth = 130;
	const udword OffsetX2 = 200;
	gGUIHelper.CreateLabel(tab, 4+OffsetX2, y+LabelOffsetY, LocalLabelWidth, 20, "Convex edges threshold:", gMainGUI);
	gEditBox_ConvexEdgesThreshold = gGUIHelper.CreateEditBox(tab, MAIN_GUI_CONVEX_EDGES_THRESHOLD, 4+LocalLabelWidth+OffsetX2, y, EditBoxWidth, 20, gGUIHelper.Convert(gConvexEdgesThreshold), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, null/*gTooltip_WireframeOverlayCoeff*/);
	}

	y += YStep;

	{
	const udword OffsetX2 = 0;
	gGUIHelper.CreateLabel(tab, 4+OffsetX2, y+LabelOffsetY, 90, 20, "Batch size:", gMainGUI);
	gEditBox_BatchSize = gGUIHelper.CreateEditBox(tab, MAIN_GUI_BATCH_SIZE, 4+OffsetX+OffsetX2, y, EditBoxWidth, 20, _F("%d", gBatchSize), gMainGUI, EDITBOX_INTEGER_POSITIVE, gEBCallback, gTooltip_BatchSize);
	}
	y += YStep;

	{
		sdword y2 = 10;
		gGUIHelper.CreateButton(tab, COLOR_PICKER_WIRE_COLOR, 300, 50, 130, 20, "Change wire color", gMainGUI, gColorPickerButtonCallback, null, "Change wireframe color");
		y2 += 30;
	}

/*	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Wireframe overlay:", gMainGUI);
	gEditBox_WireframeOverlayCoeff = gGUIHelper.CreateEditBox(tab, MAIN_GUI_WIREFRAME_OVERLAY_COEFF, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gWireframeOverlayCoeff), gMainGUI, EDITBOX_FLOAT_POSITIVE, gEBCallback, gTooltip_WireframeOverlayCoeff);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Batch size:", gMainGUI);
	gEditBox_BatchSize = gGUIHelper.CreateEditBox(tab, MAIN_GUI_BATCH_SIZE, 4+OffsetX, y, EditBoxWidth, 20, _F("%d", gBatchSize), gMainGUI, EDITBOX_INTEGER_POSITIVE, gEBCallback, gTooltip_BatchSize);
	y += YStep;*/

/*	{
		y += YStep;
		gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Fog type:", gMainGUI);
		gComboBox_FogType = CreateComboBox<MultiComboBox>(tab, MAIN_GUI_FOG_TYPE, 94, y, 150, 20, "Fog type", gMainGUI, gTooltip_FogType);
		gComboBox_FogType->Add("Disabled");
		gComboBox_FogType->Add("Linear");
		gComboBox_FogType->Add("Exp");
		gComboBox_FogType->Add("Exp2");
		gComboBox_FogType->Select(gFogType_Index);
	}

	if(0)
	{
		glFogi(GL_FOG_MODE,GL_EXP2); 
		glFogf(GL_FOG_START, 0.0f);
		glFogf(GL_FOG_END, 100.0f);
		glFogf(GL_FOG_DENSITY, 0.005f);
//		glClearColor(0.2f, 0.2f, 0.2f, 1.0);
		const Point FogColor(0.2f, 0.2f, 0.2f);
		glFogfv(GL_FOG_COLOR, &FogColor.x);
	}*/

#ifdef PEEL_USE_MSAA
	{
//		y += YStep;
		gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "MSAA:", gMainGUI);
		gComboBox_MSAA = CreateComboBox<MultiComboBox>(tab, MAIN_GUI_MSAA, 94, y, 150, 20, "MSAA", gMainGUI, gTooltip_MSAA);
		gComboBox_MSAA->Add("Disabled");
		gComboBox_MSAA->Add("MSAA 2X");
		gComboBox_MSAA->Add("MSAA 4X");
		gComboBox_MSAA->Add("MSAA 8X");
		//gComboBox_MSAA->Add("MSAA 16X");
		gComboBox_MSAA->Select(gMSAA_Index);
	}
#endif

	{
		y += YStep;
		gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Render model:", gMainGUI);
		gComboBox_RenderModel = CreateComboBox<MultiComboBox>(tab, MAIN_GUI_RENDER_MODEL, 94, y, 150, 20, "Render model", gMainGUI, gTooltip_RenderModel);
		for(udword i=0;i<RENDER_MODEL_COUNT;i++)
			gComboBox_RenderModel->Add(gRenderModel[i]->GetUIName());
		gComboBox_RenderModel->Select(gRenderModelIndex);
//		y += YStep;
	}

	const udword Height = 200;
	y += YStep*2;
	const sdword x = 16;

	for(udword i=0;i<RENDER_MODEL_COUNT;i++)
	{
		WindowDesc WD;
		WD.mParent	= tab;
		WD.mX		= x;
		WD.mY		= y+20;
		WD.mWidth	= 320;
		WD.mHeight	= Height;
		WD.mType	= WINDOW_DIALOG;
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
//		WD.mStyle	= WSTYLE_BORDER;

		IceWindow* OptionWindow = ICE_NEW(IceWindow)(WD);
		gMainGUI->Register(OptionWindow);
		OptionWindow->SetVisible(true);
		gRenderModelOptions[i] = OptionWindow;

		gRenderModel[i]->InitGUI(OptionWindow, gMainGUI, gGUIHelper);
	}

	{
		IceEditBox* tmp = gGUIHelper.CreateEditBox(tab, MAIN_GUI_DUMMY, 4, y, 350, 230, "===== Render model options =====", gMainGUI, EDITBOX_TEXT, null);
		tmp->SetReadOnly(true);
	}

	tab->SetVisible(false);
}

static void CreateScreenQuadTab(IceWindow* tab)
{
	sdword y = 4;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Screen quad:", gMainGUI);
	gComboBox_ScreenQuad = CreateComboBox<MultiComboBox>(tab, MAIN_GUI_SCREEN_QUAD, 94, y, 200, 20, "Screen quad", gMainGUI, gTooltip_ScreenQuad);
	for(udword i=0;i<SCREEN_QUAD_COUNT;i++)
		gComboBox_ScreenQuad->Add(gScreenQuadModes[i]->GetUIName());
	gComboBox_ScreenQuad->Select(gScreenQuadModeIndex);
	y += YStep;

	const udword Height = 250;
	y += YStep*2;
	const sdword x = 16;

	for(udword i=0;i<SCREEN_QUAD_COUNT;i++)
	{
		WindowDesc WD;
		WD.mParent	= tab;
		WD.mX		= x;
		WD.mY		= y;
		WD.mWidth	= 300;
		WD.mHeight	= Height;
		WD.mType	= WINDOW_DIALOG;
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
//		WD.mStyle	= WSTYLE_BORDER;
		// The label doesn't matter in this case
//		WD.mLabel	= "BackColor";
//		WD.mLabel	= "Gradient";
//		WD.mLabel	= "Color sphere";

		IceWindow* OptionWindow = ICE_NEW(IceWindow)(WD);
		gMainGUI->Register(OptionWindow);
		OptionWindow->SetVisible(true);
		gScreenQuadOptions[i] = OptionWindow;

		gScreenQuadModes[i]->CreateUI(gGUIHelper, gMainGUI, OptionWindow);
	}

	{
		IceEditBox* tmp = gGUIHelper.CreateEditBox(tab, MAIN_GUI_DUMMY, 4, 4+30, 350, 300, "===== Screen quad options =====", gMainGUI, EDITBOX_TEXT, null);
		tmp->SetReadOnly(true);
	}

	tab->SetVisible(false);
}

static void CreateSunTab(IceWindow* tab)
{
	sdword y = 4;

	struct RenderSun{ static void Lambda(const IceCheckBox& check_box, bool checked, void* user_data)	{ gRenderSunEnabled = checked; }};
	gCheckBox_RenderSun = gGUIHelper.CreateCheckBox(tab, 0, 4, y, 80, 20, "Render sun", gMainGUI, gRenderSunEnabled, RenderSun::Lambda, null);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Phi:", gMainGUI);
	gEditBox_SunPhi = gGUIHelper.CreateEditBox(tab, MAIN_GUI_SUN_PHI, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gSunPhi), gMainGUI, EDITBOX_FLOAT, gEBCallback, null);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Theta:", gMainGUI);
	gEditBox_SunTheta = gGUIHelper.CreateEditBox(tab, MAIN_GUI_SUN_THETA, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gSunTheta), gMainGUI, EDITBOX_FLOAT, gEBCallback, null);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Intensity:", gMainGUI);
	gEditBox_SunIntensity = gGUIHelper.CreateEditBox(tab, MAIN_GUI_SUN_INTENSITY, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gSunIntensity), gMainGUI, EDITBOX_FLOAT, gEBCallback, null);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Rayleigh multiplier:", gMainGUI);
	gEditBox_RayleighMultiplier = gGUIHelper.CreateEditBox(tab, MAIN_GUI_RAYLEIGH_MULTIPLIER, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gRayleighMultiplier), gMainGUI, EDITBOX_FLOAT, gEBCallback, null);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Mie multiplier:", gMainGUI);
	gEditBox_MieMultiplier = gGUIHelper.CreateEditBox(tab, MAIN_GUI_MIE_MULTIPLIER, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gMieMultiplier), gMainGUI, EDITBOX_FLOAT, gEBCallback, null);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Scattering multiplier:", gMainGUI);
	gEditBox_ScatteringMultiplier = gGUIHelper.CreateEditBox(tab, MAIN_GUI_SCATTERING_MULTIPLIER, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gScatteringMultiplier), gMainGUI, EDITBOX_FLOAT, gEBCallback, null);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "Extinction multiplier:", gMainGUI);
	gEditBox_ExtinctionMultiplier = gGUIHelper.CreateEditBox(tab, MAIN_GUI_EXTINCTION_MULTIPLIER, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gExtinctionMultiplier), gMainGUI, EDITBOX_FLOAT, gEBCallback, null);
	y += YStep;

	gGUIHelper.CreateLabel(tab, 4, y+LabelOffsetY, 90, 20, "HGg:", gMainGUI);
	gEditBox_HGg = gGUIHelper.CreateEditBox(tab, MAIN_GUI_HG, 4+OffsetX, y, EditBoxWidth, 20, gGUIHelper.Convert(gHGg), gMainGUI, EDITBOX_FLOAT, gEBCallback, null);
	y += YStep;

	tab->SetVisible(false);
}

namespace
{
	class MyColorPickerCallback : public ColorPickerCallback
	{
		public:
			MyColorPickerCallback() : mCB(null)	{}
		ColorPickerCallback*	mCB;

		virtual	void	OnNewColorSelected(ubyte r, ubyte g, ubyte b)
		{
			if(mCB)
			{
				mCB->OnNewColorSelected(r, g, b);
				return;
			}

			const Point NewColor(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f);

			if(gEditedColor==COLOR_PICKER_WIRE_COLOR)
			{
#ifdef TEST_COLOR
				gTestColor = NewColor;
#else
				gWireColor = NewColor;
#endif
			}

			if(gEditedColor==COLOR_PICKER_PLANE_COLOR)
			{
				gPlaneColor = NewColor;
			}

			if(0)
			{
				gAmbientColor.x = NewColor.x;
				gAmbientColor.y = NewColor.y;
				gAmbientColor.z = NewColor.z;
				gAmbientColor.w = 0.0f;
				glLightfv(GL_LIGHT0, GL_AMBIENT, &gAmbientColor.x);
			}
		}
	};
}
static MyColorPickerCallback gColorPickerCallback;

static void gColorPickerButtonCallback(IceButton& button, void* user_data)
{
	gColorPickerCallback.mCB = null;
	gEditedColor = EditedColor(button.GetID());
	if(user_data)
		gColorPicker->SetLabel((const char*)user_data);
	gColorPicker->SetVisible(true);
}

void SetupColorPicker(const char* name, ColorPickerCallback* callback)
{
	if(gColorPicker)
	{
		gColorPickerCallback.mCB = callback;
		gColorPicker->SetLabel(name);
		gColorPicker->SetVisible(true);
		gColorPicker->SetFocus();
	}
}

	class PEELOptions : public IceWindow
	{
		public:
						PEELOptions(const WindowDesc& desc) : IceWindow(desc)	{}
		virtual			~PEELOptions()											{}

		virtual	void	MenuEvent(udword menu_item_id)
		{
			ProcessMenuEvent(menu_item_id);
		}

		virtual int		handleEvent(IceGUIEvent* evt)
		{
/*			if(evt->mType==EVENT_MOUSE_MOVE)
			{
				printf("EVENT_MOUSE_MOVE %d %d\n", evt->mMouse->mMouseX, evt->mMouse->mMouseY);
//				MouseWheelCallback(0, 0, event->mHeight);
			}
			else*/ if(evt->mType==EVENT_MOUSE_WHEEL)
			{
//				MouseWheelCallback(0, 0, event->mHeight);
			}
			return IceWindow::handleEvent(evt);
		}
	};

static void PEEL_InitGUI()
{
	IceWidget* Parent = null;
	IceTabControl* TabControl = null;
	if(1)
	{
		gMainGUI = ICE_NEW(Widgets);

		const udword BorderSize = 2;
		const udword MainWidth = 512;
		const udword MainHeight = INITIAL_SCREEN_HEIGHT+27;

		// Main window
		WindowDesc WD;
		WD.mParent	= null;
		WD.mX		= GUI_HIDDEN_POS_X;
		WD.mY		= GUI_HIDDEN_POS_Y;
		WD.mWidth	= MainWidth;
		WD.mHeight	= MainHeight;
		WD.mLabel	= gAppTitle;
		WD.mType	= WINDOW_DIALOG;
		
//		WD.mStyle	= WSTYLE_STATIC_EDGES;
		IceWindow* OptionsWindow = ICE_NEW(PEELOptions)(WD);
		gMainWindow = OptionsWindow;

		gMainGUI->Register(OptionsWindow);
		CreatePEELMenuBar(OptionsWindow, *gMainGUI);
		OptionsWindow->SetVisible(true);

		// Create tab control
		{
			TabControlDesc TCD;
			TCD.mParent	= OptionsWindow;
			TCD.mX		= 0;
			TCD.mY		= 0;
			TCD.mWidth	= MainWidth - BorderSize*2;
			TCD.mHeight	= MainHeight - BorderSize*2;
			TabControl = ICE_NEW(IceTabControl)(TCD);
			gMainGUI->Register(TabControl);
		}
		Parent = OptionsWindow;
		ASSERT(TabControl->GetParent()==OptionsWindow);

		sdword y = TITLE_HEIGHT + 20;

		IceWindow* MainOptions;
		{
			WindowDesc WD;
			WD.mParent	= OptionsWindow;
			WD.mX		= 100;
			WD.mY		= 100;
			WD.mWidth	= 256;
			WD.mHeight	= 300;
			WD.mLabel	= "Main options";
		//	WD.mType	= WINDOW_NORMAL;
			WD.mType	= WINDOW_DIALOG;
		//	WD.mType	= WINDOW_MODALDIALOG;
		//	WD.mStyle	=
			MainOptions = ICE_NEW(IceWindow)(WD);
			gMainGUI->Register(MainOptions);
			MainOptions->SetVisible(true);
			TabControl->Add(MainOptions, "Generic");

			CreateTitleWindow(*gMainGUI, MainOptions, 40, 0);

			{
				static const char* Date = __DATE__ " at " __TIME__;
				gGUIHelper.CreateLabel(MainOptions, 4, y, 300, 20, _F("Version %s - Build: %s", gVersion, Date), gMainGUI);
				y += YStep;
//				y += YStep;
			}

			{
//				y += YStep;
//				y += YStep * 10;

//				IceButton* gButton_ExecuteScript = gGUIHelper.CreateButton(MainOptions, 0, 20, y, 450, 20, "Execute script", gMainGUI, gButtonCallbackScript, null);
//				y += YStep;
//				y += YStep;
//				y += 30;

//				gGUIHelper.CreateButton(MainOptions, 0, 20, y, 450, 20, "Import scene", gMainGUI, gButtonCallbackScene, null);
//				y += 30;
			}
		}
		{
			IceWindow* Tabs[TAB_COUNT];

			TabControlDesc TCD;
			TCD.mParent	= MainOptions;
			TCD.mX		= 2;
			TCD.mY		= y;
			TCD.mWidth	= 495;
			TCD.mHeight	= 480;
			IceTabControl* TabControl2 = ICE_NEW(IceTabControl)(TCD);
			gMainGUI->Register(TabControl2);

			for(udword i=0;i<TAB_COUNT;i++)
			{
				WindowDesc WD;
				WD.mParent	= MainOptions;
				WD.mX		= 0;
				WD.mY		= 0;
				WD.mWidth	= 50;
				WD.mHeight	= 410;
				WD.mLabel	= "Tab";
				WD.mType	= WINDOW_DIALOG;
				IceWindow* Tab = ICE_NEW(IceWindow)(WD);
				gMainGUI->Register(Tab);
				Tab->SetVisible(true);
				Tabs[i] = Tab;
			}
			TabControl2->Add(Tabs[TAB_MAIN], "Main");
			TabControl2->Add(Tabs[TAB_SCENE], "Scene");
			TabControl2->Add(Tabs[TAB_TOOL], "Tool");
			TabControl2->Add(Tabs[TAB_CAMERA], "Camera");
			TabControl2->Add(Tabs[TAB_RAYTRACING], "Raytracing");
			TabControl2->Add(Tabs[TAB_RENDER], "Render");
			TabControl2->Add(Tabs[TAB_SCREEN_QUAD], "Screen quad");
			TabControl2->Add(Tabs[TAB_SUN], "Sun");

			CreateMainTab(Tabs[TAB_MAIN]);
			CreateSceneTab(Tabs[TAB_SCENE]);
			CreateToolTab(Tabs[TAB_TOOL]);
			CreateCameraTab(Tabs[TAB_CAMERA]);
			CreateRaytraceTab(Tabs[TAB_RAYTRACING]);
			CreateRenderTab(Tabs[TAB_RENDER]);
			CreateScreenQuadTab(Tabs[TAB_SCREEN_QUAD]);
			CreateSunTab(Tabs[TAB_SUN]);

			{
				IceWindow* cp = CreateColorPicker2(null, GUI_HIDDEN_POS_X, GUI_HIDDEN_POS_Y, &gColorPickerCallback);
				gMainGUI->Register(cp);
				cp->SetVisible(false);
				cp->SetPosition(4, 4);
				gColorPicker = cp;
			}
			{
				EditActorWindow* ui = CreateActorEditGUI(null, GUI_HIDDEN_POS_X, GUI_HIDDEN_POS_Y);
				ui->SetVisible(false);
				ui->SetPosition(4, 4);
			}
			{
				ItemSelectionWindow* ui = CreateItemSelectionWindow(null, GUI_HIDDEN_POS_X, GUI_HIDDEN_POS_Y);
				ui->SetVisible(false);
				ui->SetPosition(4, 4);
			}
			{
				CompoundCreateWindow* ui = CreateCompoundCreateGUI(null, GUI_HIDDEN_POS_X, GUI_HIDDEN_POS_Y);
				ui->SetVisible(false);
				ui->SetPosition(4, 4);
			}
			{
				EditJointWindow* ui = CreateJointEditGUI(null, GUI_HIDDEN_POS_X, GUI_HIDDEN_POS_Y);
				ui->SetVisible(false);
				ui->SetPosition(4, 4);
			}	
		}
		OptionsWindow->SetPosition(MAIN_GUI_POS_X, MAIN_GUI_POS_Y);
	}

	for(udword i=0;i<gNbPlugIns;i++)
	{
		IceWindow* Main = gPlugIns[i]->InitGUI(Parent, gGUIHelper);
		if(Main)
		{
			Pint* Engine = gPlugIns[i]->GetPint();
			const udword Flags = Engine->GetFlags();
			if(Flags & PINT_HAS_RAYTRACING_WINDOW)
			{
				const udword RaytracingWindowWidth = RAYTRACING_DISPLAY_WIDTH + 24*2;
				const udword RaytracingWindowHeight = RAYTRACING_DISPLAY_HEIGHT + 24*2+8;
				gRaytracingWindows[i] = CreateRaytracingWindow(*gMainGUI, Main, 512 - RaytracingWindowWidth - 20, 768 - RaytracingWindowHeight - 50, RaytracingWindowWidth, RaytracingWindowHeight);
			}
			if(TabControl)
				TabControl->Add(Main, Engine->GetUIName());
		}
	}
}

static void PEEL_CloseGUI()
{
	DELETESINGLE(gMainGUI);

	gMainWindow = null;

	gColorPicker = null;
	CloseJointEditGUI();
	CloseCompoundCreateGUI();
	CloseActorEditGUI();
	CloseItemSelectionWindow();

	gCheckBox_Render = null;
	gCheckBox_RenderSun = null;
	gCheckBox_Wireframe = null;
	gCheckBox_WireframeOverlay = null;
	gCheckBox_GLFinish = null;

	gEditBox_WireframeOverlayCoeff = null;
	gEditBox_ConvexEdgesThreshold = null;
	gEditBox_BatchSize = null;
	gEditBox_CameraSpeed = null;
	gEditBox_CameraNearClip = null;
	gEditBox_CameraFarClip = null;
	gEditBox_CameraFOV = null;
	gEditBox_CameraSensitivity = null;
	gEditBox_RaytracingDistance = null;
	gEditBox_SunPhi = null;
	gEditBox_SunTheta = null;
	gEditBox_SunIntensity = null;
	gEditBox_RayleighMultiplier = null;
	gEditBox_MieMultiplier = null;
	gEditBox_ScatteringMultiplier = null;
	gEditBox_ExtinctionMultiplier = null;
	gEditBox_HGg = null;

	gComboBox_CurrentTool = null;
	gComboBox_ProfilingUnits = null;
	gComboBox_UserProfilingMode = null;
	gComboBox_SQRaycastMode = null;
	gComboBox_RaytracingResolution = null;
	gComboBox_RenderModel = null;
	gComboBox_ScreenQuad = null;
	gComboBox_MSAA = null;

	gRadioButton_RT_Disabled = null;
	gRadioButton_RT_ST = null;
	gRadioButton_RT_MT = null;
	gCheckBox_RT_Blit = null;

	gEditBox_RoundCornersSize = null;
	gEditBox_FrameSize = null;
	gEditBox_Gravity = null;
	gEditBox_SceneDesc = null;
}

static void SetBestRenderMode()
{
	// Must be done after initializing the UI
	int index = RENDER_MODEL_SIMPLE_SHADOWS;
	if(!SelectRenderModel(index))
	{
		index = RENDER_MODEL_SIMPLE_SHADER_2;
		if(!SelectRenderModel(index))
		{
			index = RENDER_MODEL_SIMPLE_SHADER_1;
			if(!SelectRenderModel(index))
			{
				index = RENDER_MODEL_FFP;
				SelectRenderModel(index);
			}
		}
	}

	SelectScreenQuadMode(SCREEN_QUAD_COLOR_SPHERE);
	Select_MSAA(3);
	if(gComboBox_RenderModel)
		gComboBox_RenderModel->Select(index);
	if(gComboBox_ScreenQuad)
		gComboBox_ScreenQuad->Select(SCREEN_QUAD_COLOR_SPHERE);
	if(gComboBox_MSAA)
		gComboBox_MSAA->Select(3);
}

void TestCSVExport()
{
	if(!gRunningTest)
		return;

	const char* Filename = GetFilenameForExport("csv", "csv.txt");

	FILE* globalFile = fopen(Filename, "w");
	if(!globalFile)
		return;

	if(gProfilingUnits==PROFILING_UNITS_RDTSC)
		fprintf_s(globalFile, "%s (K-Cycles)\n\n", gRunningTest->GetName());
	else if(gProfilingUnits==PROFILING_UNITS_TIME_GET_TIME)
		fprintf_s(globalFile, "%s (ms)\n\n", gRunningTest->GetName());
	else if(gProfilingUnits==PROFILING_UNITS_QPC)
		fprintf_s(globalFile, "%s (us)\n\n", gRunningTest->GetName());
	else ASSERT(0);

	const udword NbFrames = gFrameNb<=MAX_NB_RECORDED_FRAMES ? gFrameNb : MAX_NB_RECORDED_FRAMES;

	for(udword b=0;b<gNbEngines;b++)
	{
		if(!gEngines[b].mEnabled || !gEngines[b].mSupportsCurrentTest || !(gEngines[b].mEngine->GetFlags() & PINT_IS_ACTIVE))
			continue;

		if(gCommaSeparator)
			fprintf_s(globalFile, "%s, ", gEngines[b].mEngine->GetName());
		else
			fprintf_s(globalFile, "%s; ", gEngines[b].mEngine->GetName());

		for(udword i=0;i<NbFrames;i++)
		{
			if(gCommaSeparator)
				fprintf_s(globalFile, "%d, ", gEngines[b].mTiming.mRecorded[i].mTime);
			else
				fprintf_s(globalFile, "%d; ", gEngines[b].mTiming.mRecorded[i].mTime);
		}
		fprintf_s(globalFile, "\n");
	}

	fprintf_s(globalFile, "\n\n");

	fprintf_s(globalFile, "Memory usage (Kb):\n\n");

	for(udword b=0;b<gNbEngines;b++)
	{
		if(!gEngines[b].mEnabled || !gEngines[b].mSupportsCurrentTest || !(gEngines[b].mEngine->GetFlags() & PINT_IS_ACTIVE))
			continue;

		if(gCommaSeparator)
			fprintf_s(globalFile, "%s, ", gEngines[b].mEngine->GetName());
		else
			fprintf_s(globalFile, "%s; ", gEngines[b].mEngine->GetName());

		for(udword i=0;i<NbFrames;i++)
		{
			if(gCommaSeparator)
				fprintf_s(globalFile, "%d, ", gEngines[b].mTiming.mRecorded[i].mUsedMemory/1024);
			else
				fprintf_s(globalFile, "%d; ", gEngines[b].mTiming.mRecorded[i].mUsedMemory/1024);
		}
		fprintf_s(globalFile, "\n");
	}

	fclose(globalFile);
	gDisplayMessage = true;
	gDisplayMessageType = 0;
}
