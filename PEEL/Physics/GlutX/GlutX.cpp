///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdlib.h>
#include "Include\GlutX.h"
#include "GL/glut.h"

///////////////////////////////////////////////////////////////////////////////

void glutxInit(int* argcp, char** argv)								{ glutInit(argcp, argv);												}
int glutxGet(unsigned int type)										{ return glutGet(type);													}
void glutxInitDisplayMode(unsigned int mask)						{ glutInitDisplayMode(mask ? mask : GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);	}
void glutxInitWindowPosition(int x, int y)							{ glutInitWindowPosition(x, y);											}
void glutxInitWindowSize(int width, int height)						{ glutInitWindowSize(width, height);									}
int glutxCreateWindow(const char* title, GlutX_ExitCallback cb)		{ return __glutCreateWindowWithExit(title, cb);							}
void glutxSetWindow(int handle)										{ glutSetWindow(handle);												}
void glutxMainLoop()												{ glutMainLoop();														}
void glutxSwapBuffers()												{ glutSwapBuffers();													}
void glutxReportErrors()											{ glutReportErrors();													}
void glutxPostRedisplay()											{ glutPostRedisplay();													}
void glutxSolidSphere(float radius, int slices, int stacks)			{ glutSolidSphere(radius, slices, stacks);								}
void glutxSolidCube(float size)										{ glutSolidCube(size);													}

void glutxGetWindowSize(int& width, int& height)
{
	width = glutGet(GLUT_WINDOW_WIDTH);
	height = glutGet(GLUT_WINDOW_HEIGHT);
}

void glutxDisplayFunc(GlutX_DisplayCallback cb)						{ glutDisplayFunc(cb);								}
void glutxReshapeFunc(GlutX_ReshapeCallback cb)						{ glutReshapeFunc(cb);								}
void glutxIdleFunc(GlutX_IdleCallback cb)							{ glutIdleFunc(cb);									}
void glutxMotionFunc(GlutX_MotionCallback cb)						{ glutMotionFunc(cb);	glutPassiveMotionFunc(cb);	}
void glutxMouseFunc(GlutX_MouseCallback cb)							{ glutMouseFunc(cb);								}
void glutxMouseFunc2(GlutX_MouseCallback cb)						{ glutMouseFunc2(cb);								}
void glutxMouseWheelFunc(GlutX_MouseWheelCallback cb)				{ glutMouseWheelFunc(cb);							}
void glutxDropFileFunc(GlutX_DropFileCallback cb)					{ glutDropFileFunc(cb);								}
void glutxRawInputFunc(GlutX_RawInputCallback cb)					{ glutRawInputFunc(cb);								}

static GlutX_KeyboardCallback gUserKeyboardCB = NULL;
static void _KeyboardDownCallback(unsigned char key, int x, int y)	{ if(gUserKeyboardCB)	(gUserKeyboardCB)(key, x, y, true);		}
static void _KeyboardUpCallback(unsigned char key, int x, int y)	{ if(gUserKeyboardCB)	(gUserKeyboardCB)(key, x, y, false);	}
void glutxKeyboardFunc(GlutX_KeyboardCallback cb)
{
	gUserKeyboardCB = cb;
	glutKeyboardFunc(::_KeyboardDownCallback);
	glutKeyboardUpFunc(::_KeyboardUpCallback);
}

static GlutX_SpecialKeyCallback gUserSpecialKeyCB = NULL;
static void _SpecialKeyDownCallback(int key, int x, int y)			{ if(gUserSpecialKeyCB)	(gUserSpecialKeyCB)(key, x, y, true);	}
static void _SpecialKeyUpCallback(int key, int x, int y)			{ if(gUserSpecialKeyCB)	(gUserSpecialKeyCB)(key, x, y, false);	}
void glutxSpecialKeyFunc(GlutX_SpecialKeyCallback cb)
{
	gUserSpecialKeyCB = cb;
	glutSpecialFunc(::_SpecialKeyDownCallback);
	glutSpecialUpFunc(::_SpecialKeyUpCallback);
}
