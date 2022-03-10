///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ControlInterface.h"

// Check that Glut's buttons are compatible with ICE
ICE_COMPILE_TIME_ASSERT(MBT_LEFT==0);
ICE_COMPILE_TIME_ASSERT(MBT_MIDDLE==1);
ICE_COMPILE_TIME_ASSERT(MBT_RIGHT==2);

static PEEL::ControlInterface* gControlInterface = null;

void SetControlInterface(PEEL::ControlInterface* ci)
{
	gControlInterface = ci;
}

PEEL::ControlInterface* GetControlInterface()
{
	return gControlInterface;
}

static MouseInfo gMouseInfo;

const MouseInfo& GetMouseInfo()
{
	return gMouseInfo;
}

//static int gTimestamp = 0;

/*static*/ void MouseCallback(int button, int state, int x, int y)
{
	SPY_ZONE("MouseCallback")
	//printf("MouseCallback %d\n", gTimestamp++);

	{
		gMouseInfo.mMouseX = x;
		gMouseInfo.mMouseY = y;
		if(state==0)
			gMouseInfo.mMouseButton[button] = true;
		else
			gMouseInfo.mMouseButton[button] = false;
	}

	if(gControlInterface)
	{
		if(state==0)
		{
			if(button==MBT_LEFT)
				gControlInterface->LeftDownCallback(gMouseInfo);
			else if(button==MBT_MIDDLE)
				gControlInterface->MiddleDownCallback(gMouseInfo);
			else if(button==MBT_RIGHT)
				gControlInterface->RightDownCallback(gMouseInfo);
		}
		else
		{
			if(button==MBT_LEFT)
				gControlInterface->LeftUpCallback(gMouseInfo);
			else if(button==MBT_MIDDLE)
				gControlInterface->MiddleUpCallback(gMouseInfo);
			else if(button==MBT_RIGHT)
				gControlInterface->RightUpCallback(gMouseInfo);
		}
	}
}

void MouseCallback2(int button, int state, int x, int y)
{
	SPY_ZONE("MouseCallback2")
	//printf("MouseCallback2 %d\n", gTimestamp++);

	gMouseInfo.mMouseX = x;
	gMouseInfo.mMouseY = y;

//	printf("MouseCallback2 %d|%d|%d|%d\n", button, state, x, y);
	if(gControlInterface)
	{
		if(button==MBT_LEFT)
			gControlInterface->LeftDblClkCallback(gMouseInfo);
		else if(button==MBT_MIDDLE)
			gControlInterface->MiddleDblClkCallback(gMouseInfo);
		else if(button==MBT_RIGHT)
			gControlInterface->RightDblClkCallback(gMouseInfo);
	}
}

/*static*/ void MotionCallback(int x, int y)
{
	SPY_ZONE("MotionCallback")

//	gMouseInfo.mMouseMove = true;
	const int dx = gMouseInfo.mMouseX - x;
	const int dy = gMouseInfo.mMouseY - y;
	gMouseInfo.mRelMouseX = dx;
	gMouseInfo.mRelMouseY = dy;
	//printf("MotionCallback %d (%d %d)\n", gTimestamp++, dx, dy);

	// Skip callback if motion is (0;0), i.e. we didn't really drag. This happens after losing focus,
	// when clicking on the window again - making us miss a selection event. Fix is to test dx && dy here.
	if(gControlInterface && (dx || dy))
	{
		if(gMouseInfo.mLeftMouseButton)		gControlInterface->LeftDragCallback(gMouseInfo);
		if(gMouseInfo.mMiddleMouseButton)	gControlInterface->MiddleDragCallback(gMouseInfo);
		if(gMouseInfo.mRightMouseButton)	gControlInterface->RightDragCallback(gMouseInfo);
		/* Always call MouseMove */			gControlInterface->MouseMoveCallback(gMouseInfo);
	}

	// Update old mouse position for next frame *after* the callbacks return, else they don't
	// have access to previous mouse position...
	gMouseInfo.mOldMouseX = gMouseInfo.mMouseX;
	gMouseInfo.mOldMouseY = gMouseInfo.mMouseY;
	gMouseInfo.mMouseX = x;
	gMouseInfo.mMouseY = y;
}
