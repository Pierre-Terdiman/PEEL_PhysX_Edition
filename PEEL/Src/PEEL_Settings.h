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
	#define PEEL_PUBLIC_BUILD	1

	// Support sound or not. You need FMOD installed in the proper folder to support sound.
	// Note that demo scripts won't work without sound support, as they're synchronized to the music.
	#define PEEL_SOUND	1

	// Use fracture code from ICE or local files.
	#define PEEL_USE_ICE_FRACTURE	1

	// Use NVD or not. NVD is a custom PVD.
	#define PEEL_COMPILE_NVD	0

	#define PEEL_USE_MSAA	1

	// Use SPY or not. SPY is a custom profiler.
	#define PEEL_USE_SPY	0

#endif
