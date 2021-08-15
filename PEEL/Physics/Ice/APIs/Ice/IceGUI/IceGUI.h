///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for IceGUI.dll.
 *	\file		IceGUI.h
 *	\author		Pierre Terdiman
 *	\date		November, 30, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEGUI_H
#define ICEGUI_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation messages
#if defined(ICEGUI_EXPORTS)
	#pragma message("----Compiling ICE GUI")
#elif !defined(ICEGUI_EXPORTS)
	#pragma message("----Using ICE GUI")
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Automatic linking
	#ifndef BAN_ICEGUI_AUTOLINK
		#ifdef _WIN64
			#ifdef _DEBUG
				#pragma comment(lib, "IceGUI64_D.lib")
			#else
				#pragma comment(lib, "IceGUI64.lib")
			#endif
		#else
			#ifdef _DEBUG
				#pragma comment(lib, "IceGUI_D.lib")
			#else
				#pragma comment(lib, "IceGUI.lib")
			#endif
		#endif
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preprocessor

#ifndef ICE_NO_DLL
	#ifdef ICEGUI_EXPORTS
		#define ICEGUI_API		__declspec(dllexport)
	#else
		#define ICEGUI_API		__declspec(dllimport)
	#endif
#else
		#define ICEGUI_API
#endif

	#undef	DeleteBitmap

	// Includes
	namespace IceGUI
	{
		#include "IceGUIAPI.h"
		#include "IceGUIUtils.h"
		#include "IceFileselect.h"
		#include "IceWidget.h"
		#include "IceGUIEvent.h"
		#include "IceBitmap.h"
		#include "IceWindow.h"
		#include "IceListView.h"
		#include "IceButton.h"
		#include "IceCheckBox.h"
		#include "IceComboBox.h"
		#include "IceGroupBox.h"
		#include "IceGUIApp.h"
		#include "IceLabel.h"
		#include "IceEditBox.h"
		#include "IceListBox.h"
		#include "IceMenu.h"
		#include "IceMenuBar.h"
		#include "IcePopupMenu.h"
		#include "IceProgressBar.h"
		#include "IceRadioButton.h"
		#include "IceSlider.h"
		#include "IceScrollbar.h"
		#include "IceSpinBox.h"
		#include "IceTabControl.h"
		#include "IceToggleButton.h"
		#include "IceTreeView.h"
		#include "IceRollup.h"
		#include "IcePanel.h"
		#include "IceToolbar.h"
		#include "IceColorPicker.h"
		#include "IceColorPicker2.h"
		#include "IceGraphWindow.h"
		#include "IceParticleEditor.h"
		#include "IceVectorEditBox.h"
		#include "IceSelectionDialog.h"
	}

#endif // ICEGUI_H

