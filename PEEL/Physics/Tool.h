///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_H
#define TOOL_H

#include "PintDef.h"

	class ToolInterface;

	enum ToolIndex
	{
		TOOL_PICKING,
		TOOL_ADD_IMPULSE,
		TOOL_SHOOT_OBJECT,
//		TOOL_CAMERA_TRACKING,
		TOOL_TRANSLATE,
		TOOL_ROTATE,
		TOOL_CREATE_OBJECT,
		TOOL_CREATE_JOINT,
		TOOL_CREATE_CHAIN,
		TOOL_CREATE_ROPE,
		TOOL_TERRAIN_EDITOR,
	//	TOOL_CUSTOM_CONTROL,
		TOOL_TEXTURE,
		TOOL_BULLET,

		TOOL_COUNT
	};

	udword			GetNbTools();
	const char*		GetToolName(udword i);
	void			InitTools();
	void			CloseTools();
	void			SelectTool(udword index);
	ToolInterface*	GetCurrentTool();
	ToolIndex		GetCurrentToolIndex();
	ToolInterface*	GetTool(udword index);

	class Pint;
	void			DrawBoxCorners(Pint& pint, PintActorHandle actor, const Point& color);

	#define TOOLS_BOX_CORNERS_COLOR	Point(1.0f, 1.0f, 0.0f)

#endif
