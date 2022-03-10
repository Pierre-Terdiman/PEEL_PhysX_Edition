///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEEL_MenuBar.h"
#include "GLRenderHelpers.h"
#include "Tool.h"
	#include "ToolPicking.h"
	#include "ToolShootObject.h"
	#include "ToolAddImpulse.h"
	#include "ToolTranslate.h"
	#include "ToolRotate.h"
	#include "ToolObjectCreate.h"
	#include "ToolCreateJoint.h"
	#include "ToolCreateRope.h"
	#include "ToolTerrainEditor.h"
	#include "ToolTexture.h"
	#include "ToolBullet.h"

udword GetNbTools()
{
	return TOOL_COUNT;
}

static const char* gToolNames[] = 
{
	"Picking",
	"Add impulse",
	"Shoot object",
	"Translate object",
	"Rotate object",
	"Create object",
	"Create joint",
	"Create chain",
	"Create rope",
	"Terrain editor",
	"Texture",
	"Bullet",
};

ICE_COMPILE_TIME_ASSERT(TOOL_COUNT==ARRAYSIZE(gToolNames));

const char* GetToolName(udword i)
{
	if(i>=TOOL_COUNT)
		return null;
	return gToolNames[i];
}

static	ToolIndex		gCurrentToolIndex = TOOL_PICKING;
static	ToolInterface*	gTools[TOOL_COUNT] = {};
static	ToolInterface*	gCurrentTool = null;

ToolInterface* GetCurrentTool()
{
	return gCurrentTool;
}

ToolIndex GetCurrentToolIndex()
{
	return gCurrentToolIndex;
}

ToolInterface* GetTool(udword index)
{
	if(index>=TOOL_COUNT)
		return null;
	return gTools[index];
}

void SelectTool(udword index)
{
	if(index>=TOOL_COUNT)
		return;
	gCurrentToolIndex = ToolIndex(index);
	for(udword i=0;i<TOOL_COUNT;i++)
	{
		IceWindow*	GetToolOptionWindow(udword i);
		IceWindow* ToolWindow = GetToolOptionWindow(i);
		if(ToolWindow)
			ToolWindow->SetVisible(i==gCurrentToolIndex);
	}

	if(gCurrentTool)
		gCurrentTool->Deselect();
	gCurrentTool = gTools[index];
	if(gCurrentTool)
		gCurrentTool->Select();
	UpdateToolsCheckmark();
}

void InitTools()
{
	gTools[TOOL_PICKING]			= ICE_NEW(ToolPicking);
	gTools[TOOL_ADD_IMPULSE]		= ICE_NEW(ToolAddImpulse);
	gTools[TOOL_SHOOT_OBJECT]		= ICE_NEW(ToolShootObject);
	gTools[TOOL_TRANSLATE]			= ICE_NEW(ToolTranslate);
	gTools[TOOL_ROTATE]				= ICE_NEW(ToolRotate);
	gTools[TOOL_CREATE_OBJECT]		= ICE_NEW(ToolCreateObject);
	gTools[TOOL_CREATE_JOINT]		= ICE_NEW(ToolCreateJoint);
	gTools[TOOL_CREATE_CHAIN]		= ICE_NEW(ToolCreateChain);
	gTools[TOOL_CREATE_ROPE]		= ICE_NEW(ToolCreateRope);
	gTools[TOOL_TERRAIN_EDITOR]		= ICE_NEW(ToolTerrainEditor);
	gTools[TOOL_TEXTURE]			= ICE_NEW(ToolTexture);
	gTools[TOOL_BULLET]				= ICE_NEW(ToolBullet);

	SelectTool(0);
}

void CloseTools()
{
	for(udword i=0;i<TOOL_COUNT;i++)
		DELETESINGLE(gTools[i]);
}

void DrawBoxCorners(Pint& pint, PintActorHandle actor, const Point& color)
{
	Pint_Actor* API = pint.GetActorAPI();
	if(API)
	{
		AABB Bounds;
		if(API->GetWorldBounds(actor, Bounds))
			GLRenderHelpers::DrawBoxCorners(Bounds, color, 0.2f);
	}
}
