///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for IceCore.dll.
 *	\file		IceCore.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECORE_H
#define ICECORE_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation messages

#ifndef __cplusplus
	#error ICE needs C++ compilation...
#endif

#if defined(ICECORE_EXPORTS)
	#pragma message("----Compiling ICE Core")
#elif !defined(ICECORE_EXPORTS)
	#pragma message("----Using ICE Core")
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Automatic linking
	#ifndef BAN_ICECORE_AUTOLINK
		#ifdef _WIN64
			#ifdef _DEBUG
				#pragma comment(lib, "IceCore64_D.lib")
			#else
				#pragma comment(lib, "IceCore64.lib")
			#endif
		#else
			#ifdef _DEBUG
				#pragma comment(lib, "IceCore_D.lib")
			#else
				#pragma comment(lib, "IceCore.lib")
			#endif
		#endif
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes

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

//#define ICE_USE_STL_VECTOR
#ifdef ICE_USE_STL_VECTOR
	#pragma warning(disable : 4530)
	#pragma warning(disable : 4251)
	#include <vector>
#endif

//#define NETTLE_MEMORY

#ifdef NETTLE_MEMORY
	// Paul Nettle's memory manager
	#include <new>

	#ifndef	WIN32
	#include <unistd.h>
	#endif
#endif

	// First file to include
	#include "IcePreprocessor.h"

//#define ICECORE_TIMEINFO_INTEGER

// External guards are used to release the lib with selected headers only

	#include "IceQuote.h"
#ifdef NETTLE_MEMORY
	#include "MemoryManager/mmgr.h"
#endif

// ICE includes
	#include "IceAssert.h"
	#include "IceTypes.h"
	#include "IceMemoryMacros.h"
#ifndef	_H_MMGR	// Already use this
	#include "IceMemoryManager.h"
	#include "IceOldMM.h"
#endif
	#include "IceDll.h"
	#include "IceCodeSize.h"
	#include "IceCPUID.h"

	namespace IceCore
	{
		#include "IceCoreTypes.h"
		#include "IceCoreErrors.h"
		#include "IceUtils.h"
		#include "IceStats.h"
		#include "IceRandom.h"
		#include "IceLog.h"
		#include "IceFPU.h"
		#include "IceAllocator.h"
		#include "IceFrameAllocator.h"
		#include "IceLinkedList.h"
		#include "IceContainer.h"
		#include "IcePairs.h"
		#include "IcePairManager.h"
		#include "IceStack.h"
		#include "IcePriorityQueue.h"
		#include "IceBitArray.h"
		#include "IceCustomArray.h"
		#include "IceXList.h"
		#include "IceHandleManager.h"
		#include "IceSignature.h"
		#include "IceTrashBuffer.h"
		#include "IceInstrumentTrace.h"
		#include "IceStackTrack.h"
		#include "IceString.h"
		#include "IceStringTable.h"
		#include "IceConstants.h"
		#include "IceError.h"
		#include "IceSymbolicList.h"
		#include "IceKeyboard.h"
		#include "IceFile.h"
		#include "IcePath.h"
		#include "IceHistoryBuffer.h"
		#include "IceCRC.h"
		#include "IceFunctionBinding.h"
		#include "IceSysInfo.h"
		#include "IceQSort.h"
		#include "IceNLogLogN.h"
		#include "IceInsertionSort.h"
		#include "IceRevisitedRadix.h"
		#include "IceRadix3Passes.h"
		#include "IceTimeInfo.h"
		#include "IceTimestampManager.h"
		#include "IceLinearDamper.h"
		#include "IceProfiler.h"
		#include "IcePermutation.h"
		#include "IceFields.h"
		#include "IceFieldDescriptor.h"
		#include "IceFieldController.h"
		#include "IceRTTI.h"
		#include "IceImportExport.h"
		#include "IceRefTracker.h"
		#include "IceCell.h"
		#include "IceMsg.h"
		#include "IceDynaBlock.h"
		#include "IceCustomList.h"
		#include "IceFlagControl.h"
		#include "IceWatcher.h"
		#include "IceKernel.h"
		#include "IceSmartContainer.h"
		#include "IceRegistry.h"
		#include "IceParser.h"
		#include "IceTimer.h"
		#include "IceHNode.h"
		#include "IceHashing.h"
		#include "IceNotifSystem.h"
		#include "IceDataBinder.h"
		#include "IcePublishSubscribe.h"

//			#define FLEXINE_API __declspec(dllimport)
			#define FLEXINE_API ICECORE_API
		#include "FlexineSDK.h"
		#include "IcePluginManager.h"
		#include "IceCallbacks.h"
		#include "IceRequestGUI.h"
		#include "IceRequest.h"
		#include "IceAPI.h"
		#include "IceAPP.h"
		#include "IceGDIApp.h"
		#include "IceHuffman.h"
		#include "IceMouse.h"
		#include "IceThread.h"
		#include "IceAVI.h"
		#include "IceArrayReducer.h"
		#include "IceStream.h"
		#include "IceFileStream.h"
		#include "IceBuffer.h"
		#include "IceSerialize.h"
		#include "IceGenericCache.h"
		#include "IceFilteredValue.h"
		#include "IceArchive.h"
		#include "IceResource.h"
		#include "IceVSync.h"
		#include "IceCoreSerialize.h"

		#include "IceCoreAPI.h"
	}

#endif // ICECORE_H

