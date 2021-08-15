///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GLUTX_H
#define GLUTX_H

	// GlutX is Glut Extended - with support for new things like double-click, mouse wheel events, etc

	#ifndef GLUTX_NO_DLL
		#ifdef GLUTX_EXPORTS
			#define GLUTX_API	__declspec(dllexport)
		#else
			#define GLUTX_API	__declspec(dllimport)
		#endif
	#else
			#define GLUTX_API
	#endif

	#define GLUTX_FUNCTION		extern "C"

	#define GLUTX_KEY_LEFT		100
	#define GLUTX_KEY_UP		101
	#define GLUTX_KEY_RIGHT		102
	#define GLUTX_KEY_DOWN		103
	#define GLUTX_KEY_PAGE_UP	104
	#define GLUTX_KEY_PAGE_DOWN	105
	#define GLUTX_KEY_HOME		106
	#define GLUTX_KEY_END		107
	#define GLUTX_KEY_INSERT	108

	typedef void	(__cdecl *GlutX_ExitCallback)		(int);
	typedef void	(__cdecl *GlutX_DisplayCallback)	();
	typedef void	(__cdecl *GlutX_IdleCallback)		();
	typedef void	(__cdecl *GlutX_ReshapeCallback)	(int width, int height);
	typedef void	(__cdecl *GlutX_MotionCallback)		(int x, int y);
	typedef void	(__cdecl *GlutX_KeyboardCallback)	(unsigned char key, int x, int y, bool down);
	typedef void	(__cdecl *GlutX_SpecialKeyCallback)	(int key, int x, int y, bool down);
	typedef void	(__cdecl *GlutX_MouseCallback)		(int button, int state, int x, int y);
	typedef void	(__cdecl *GlutX_MouseWheelCallback)	(int x, int y, short wheelValue);
	typedef void	(__cdecl *GlutX_DropFileCallback)	(int x, int y, const char* filename);
	typedef void	(__cdecl *GlutX_RawInputCallback)	(unsigned int, unsigned int);

	GLUTX_FUNCTION	GLUTX_API void	glutxInit(int* argcp, char** argv);
	GLUTX_FUNCTION	GLUTX_API int	glutxGet(unsigned int type);
	GLUTX_FUNCTION	GLUTX_API void	glutxInitDisplayMode(unsigned int mask=0);
	GLUTX_FUNCTION	GLUTX_API void	glutxInitWindowPosition(int x, int y);
	GLUTX_FUNCTION	GLUTX_API void	glutxInitWindowSize(int width, int height);
	GLUTX_FUNCTION	GLUTX_API void	glutxGetWindowSize(int& width, int& height);
	GLUTX_FUNCTION	GLUTX_API int	glutxCreateWindow(const char* title, GlutX_ExitCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxSetWindow(int handle);
	GLUTX_FUNCTION	GLUTX_API void	glutxMainLoop();
	GLUTX_FUNCTION	GLUTX_API void	glutxSwapBuffers();
	GLUTX_FUNCTION	GLUTX_API void	glutxReportErrors();
	GLUTX_FUNCTION	GLUTX_API void	glutxPostRedisplay();
	GLUTX_FUNCTION	GLUTX_API void	glutxSolidSphere(float radius, int slices, int stacks);
	GLUTX_FUNCTION	GLUTX_API void	glutxSolidCube(float size);

	GLUTX_FUNCTION	GLUTX_API void	glutxDisplayFunc	(GlutX_DisplayCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxReshapeFunc	(GlutX_ReshapeCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxIdleFunc		(GlutX_IdleCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxMotionFunc		(GlutX_MotionCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxMouseFunc		(GlutX_MouseCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxMouseFunc2		(GlutX_MouseCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxMouseWheelFunc	(GlutX_MouseWheelCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxKeyboardFunc	(GlutX_KeyboardCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxSpecialKeyFunc	(GlutX_SpecialKeyCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxDropFileFunc	(GlutX_DropFileCallback cb);
	GLUTX_FUNCTION	GLUTX_API void	glutxRawInputFunc	(GlutX_RawInputCallback cb);

#endif