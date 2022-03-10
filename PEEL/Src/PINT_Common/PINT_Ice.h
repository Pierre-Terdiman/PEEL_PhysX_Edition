///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_ICE_H
#define PINT_ICE_H

#define _ALLOW_KEYWORD_MACROS

	// Ban autolink to use Release libs in Debug
	#define BAN_ICECORE_AUTOLINK
	#define BAN_ICEMATHS_AUTOLINK
	#define BAN_CONTACT_AUTOLINK
	#define BAN_MESHMERIZER_AUTOLINK
	#define BAN_ICEIMAGEWORK_AUTOLINK
	#define BAN_ICEGUI_AUTOLINK
	#define BAN_ICERENDERER_AUTOLINK
	#define BAN_ICETERRAIN_AUTOLINK

// The old ICE renderer hasn't been ported to x64 and creates warnings,
// so let's just disable the offending headers for now.
	#define ICERENDERSTATEMANAGER_H
	#define ICESTATEBLOCK_H
	#define ICERENDERCORE_H
	#define ICERENDERABLESURFACE_H
	#define ICEVOIDRENDERER_H

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#undef UNICODE

// Windows includes
#ifdef WIN32
	#include <windows.h>		// General purpose Win32 functions
	#include <windowsx.h>
	#include <vfw.h>			// Header File For Video For Windows
	#include <commctrl.h>		// For the toolbar
	#include <commdlg.h>		// For open/save file structures
	#include <shellapi.h>		// For drag & drop
	#include <shlobj.h>			// For browse info
#endif // WIN32

// Standard includes
#ifdef _DEBUG
	#include <crtdbg.h>			// C runtime debug functions
#endif
	#include <stdio.h>			// Standard Input/Output functions
	#include <stdlib.h>
	#include <float.h>
	#include <malloc.h>			// For _alloca
	#include <math.h>

	#include <IceMaths/IceMathsAFX.h>
	using namespace IceMaths;

	#define Interval	IceMaths::Interval

	#include "Meshmerizer\MeshmerizerAFX.h"
	#include "IceImageWork\IceImageWorkAFX.h"
	#include "IceGUI\IceGUIAFX.h"
	#include "IceRenderer\IceRendererAFX.h"
	#include "IceTerrain\IceTerrainAFX.h"
	using namespace Meshmerizer;
	using namespace IceImageWork;
	using namespace IceGUI;
	using namespace IceRenderer;
	using namespace IceTerrain;

	#define ICE_RESERVE(type, container)	reinterpret_cast<type*>((container).Reserve(sizeof(type)/sizeof(udword)));

#endif