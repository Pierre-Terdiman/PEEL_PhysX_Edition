///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PEEL_SETTINGS_H
#define PEEL_SETTINGS_H

	// Public or private builds. Public builds remove support for tests that include (private) customer-provided data.
	#define PEEL_PUBLIC_BUILD

	// Support sound or not. You need FMOD installed in the proper folder to support sound.
	// Note that demo scripts won't work without sound support, as they're synchronized to the music.
	#define PEEL_SOUND

	// Use fracture code from ICE or local files.
	#define PEEL_USE_ICE_FRACTURE

	// Use NVD or not. NVD is a custom PVD.
	//#define PEEL_COMPILE_NVD

	#define PEEL_USE_MSAA

#ifndef _WIN64
	// Use SPY or not. SPY is a custom profiler.
	//#define PEEL_USE_SPY
#endif

#endif
