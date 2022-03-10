///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PEEL_MenuBar.h"
#include "PEEL.h"
#include "PintGUIHelper.h"
#include "Script.h"
#include "Tool.h"
#include "PINT_Editor.h"
#include "DefaultControlInterface.h"
#include "GUI_ActorEdit.h"
#include "GUI_ActorSelection.h"
#include "PintShapeRenderer.h"
#include "TextureManager.h"

static SymbolicStrList* gRecentFiles = null;

static IceMenu* gMenu_Tools = null;
void UpdateToolsCheckmark()
{
	if(gMenu_Tools)
	{
		const udword NbTools = GetNbTools();
		const ToolIndex CurrentToolIndex = GetCurrentToolIndex();
		for(udword i=0;i<NbTools;i++)
			gMenu_Tools->SetChecked(MENU_TOOL+i, i==CurrentToolIndex);
	}
}

static IceMenu* gMenu_Selection = null;
void UpdateSelectionItems()
{
	if(gMenu_Selection)
	{
		SelectionAction Enabled;
		GetDefaultControlInterface().GetSelectionActionAvailability(Enabled);

		gMenu_Selection->SetEnabled(MENU_HIDE_ALL, Enabled.mHideAll);
		gMenu_Selection->SetEnabled(MENU_SHOW_ALL, Enabled.mShowAll);
		gMenu_Selection->SetEnabled(MENU_SELECT_ALL, Enabled.mSelectAll);
		//gMenu_Selection->SetEnabled(MENU_SHOW_SELECTION_DIALOG, Enabled.mShowSelectionDialog);

		gMenu_Selection->SetEnabled(MENU_HIDE_SELECTED, Enabled.mHideSelection);
		gMenu_Selection->SetEnabled(MENU_SHOW_SELECTED, Enabled.mShowSelection);
		gMenu_Selection->SetEnabled(MENU_HIDE_UNSELECTED, Enabled.mHideUnselected);
		gMenu_Selection->SetEnabled(MENU_DELETE_SELECTED, Enabled.mDeleteSelection);
		gMenu_Selection->SetEnabled(MENU_CANCEL_SELECTION, Enabled.mCancelSelection);
		gMenu_Selection->SetEnabled(MENU_EDIT_SELECTED, Enabled.mEditSelection);
		gMenu_Selection->SetEnabled(MENU_SELECT_HIDDEN, Enabled.mSelectHidden);

		//bool	mDisableSelection;
		//bool	mEnableSelection;
	}
}

static IceMenu* gMenu_File = null;
void UpdateRecentFiles()
{
	if(!gMenu_File)
		return;

	for(udword i=0;i<10;i++)
	{
		// TODO: add missing function to ICE
//		gMenu_File->Remove(MENU_RECENT_FILE_0+i);
		RemoveMenu((HMENU)gMenu_File->GetHandle(), MENU_RECENT_FILE_0+i, MF_BYCOMMAND);
	}

	DELETESINGLE(gRecentFiles);
	gRecentFiles = ICE_NEW(SymbolicStrList);
	const udword NbRecentFiles = GetRecentFiles(*gRecentFiles, "PEEL");
	for(udword i=0;i<NbRecentFiles;i++)
	{
		String FileName;
		if(gRecentFiles->GetSymbolicValue(i, FileName))
		{
			RemovePath(FileName);
			gMenu_File->Add(FileName, MENU_RECENT_FILE_0+i);
		}
	}
}

void CreatePEELMenuBar(IceWindow* parent, Widgets& widgets)
{
	ASSERT(parent);

	IceMenu* Menu_File = ICE_NEW(IceMenu);
	{
		gMenu_File = Menu_File;

		widgets.Register(Menu_File);

		Menu_File->Add("Load (drag-and-drop)", MENU_LOAD);
		Menu_File->Add("Merge (drag-and-drop + Ctrl)", MENU_MERGE);
		Menu_File->Add("Save", MENU_SAVE);
//		Menu_File->SetEnabled(MENU_SAVE, false);
		Menu_File->AddSeparator();
		Menu_File->Add("Run script", MENU_RUN_SCRIPT);
		Menu_File->AddSeparator();

		UpdateRecentFiles();
	}

	IceMenu* Menu_Tools = ICE_NEW(IceMenu);
	{
		gMenu_Tools = Menu_Tools;
		widgets.Register(Menu_Tools);

		const udword NbTools = GetNbTools();
		for(udword i=0;i<NbTools;i++)
			Menu_Tools->Add(GetToolName(i), MENU_TOOL+i);

		UpdateToolsCheckmark();
	}

	IceMenu* Menu_Selection = ICE_NEW(IceMenu);
	{
		gMenu_Selection = Menu_Selection;
		widgets.Register(Menu_Selection);
		Menu_Selection->Add("Hide all", MENU_HIDE_ALL);
		Menu_Selection->Add("Show all", MENU_SHOW_ALL);
		Menu_Selection->Add("Select all (Ctrl+A)", MENU_SELECT_ALL);
		//Menu_Selection->Add("Show selection dialog", MENU_SHOW_SELECTION_DIALOG);
		Menu_Selection->AddSeparator();
		Menu_Selection->Add("Hide selected (H)", MENU_HIDE_SELECTED);
		Menu_Selection->Add("Show selected (J)", MENU_SHOW_SELECTED);
		Menu_Selection->Add("Hide unselected", MENU_HIDE_UNSELECTED);
		Menu_Selection->Add("Select hidden", MENU_SELECT_HIDDEN);
		//AddPopupMenuEntry(Menu, "Disable selected", SELECTION_DISABLE, false, NothingSelected);
		//AddPopupMenuEntry(Menu, "Enable selected", SELECTION_ENABLE, false, NothingSelected);
		Menu_Selection->Add("Delete selected (Del)", MENU_DELETE_SELECTED);
		Menu_Selection->Add("Edit selected (Left dbl-clk on object)", MENU_EDIT_SELECTED);
		Menu_Selection->Add("Cancel selection", MENU_CANCEL_SELECTION);
	}

/*	IceMenu* Menu_Create = ICE_NEW(IceMenu);
	{
		widgets.Register(Menu_Create);
		Menu_Create->Add("Create joints", MENU_CREATE_JOINTS);
	}*/

	IceMenu* Menu_Scene = ICE_NEW(IceMenu);
	{
		widgets.Register(Menu_Scene);
		Menu_Scene->Add("Print Debug infos", MENU_SCENE_PRINT_DEBUG_INFOS);
	}

	IceMenu* Menu_Help = ICE_NEW(IceMenu);
	{
		widgets.Register(Menu_Help);
		Menu_Help->Add("About...", MENU_ABOUT);
	}

	IceMenuBar* MB = ICE_NEW(IceMenuBar)(parent);
	{
		widgets.Register(MB);
		MB->AddMenu("File", Menu_File);
		MB->AddMenu("Tool", Menu_Tools);
		MB->AddMenu("Selection", Menu_Selection);
//		MB->AddMenu("Create", Menu_Create);
		MB->AddMenu("Scene", Menu_Scene);
		MB->AddMenu("Help", Menu_Help);
	}

	parent->SetMenuBar(MB);
}

void ReleasePEELMenuBar()
{
	DELETESINGLE(gRecentFiles);
}

void ProcessMenuEvent(udword menu_item_id)
{
//	printf("Menu event: %d\n", menu_item_id);

	DefaultControlInterface& dci = GetDefaultControlInterface();

	if(menu_item_id>=MENU_RECENT_FILE_0 && menu_item_id<=MENU_RECENT_FILE_9)
	{
		if(gRecentFiles)
		{
			String FileName;
			if(gRecentFiles->GetSymbolicValue(menu_item_id-MENU_RECENT_FILE_0, FileName))
				ImportFile(FileName);
		}
	}
	else if(menu_item_id>=MENU_TOOL)
	{
		const udword NbTools = GetNbTools();
		const ToolIndex CurrentToolIndex = GetCurrentToolIndex();
		const udword Choice = menu_item_id - MENU_TOOL;
		if(Choice<NbTools)
		{
			if(Choice!=CurrentToolIndex)
			{
				SelectTool(Choice);
				UpdateToolComboBox();
				//UpdateToolsCheckmark();
			}
		}
	}
	else if(menu_item_id==MENU_LOAD)
	{
		ImportFile(null);
	}
	else if(menu_item_id==MENU_MERGE)
	{
		// ### refactor
		FILESELECTCREATE Create;
		Create.mFilter			= "ZB2 Files (*.zb2)|*.zb2||";
		Create.mFileName		= "";
		Create.mInitialDir		= GetRoot();
		Create.mCaptionTitle	= "Merge file";
		Create.mDefExt			= "zb2";

		String Filename;
		if(FileselectOpenSingle(Create, Filename))
		{			
			if(!FileExists(Filename))
			{
				IceCore::MessageBox(0, "File not found.", "Error", MB_OK);
			}
			else
			{
				MergeFile(Filename);
			}
		}
	}
	else if(menu_item_id==MENU_SAVE)
	{
		if(GetEditor())
			ExportScene();
		else
			IceCore::MessageBox(0, "Export only available in editor mode.\n\nEnable the editor checkbox when starting PEEL.", "Feature not available", MB_OK);
	}
	else if(menu_item_id==MENU_RUN_SCRIPT)
	{
		FILESELECTCREATE Create;
		Create.mFilter			= "Text files (*.txt)|*.txt|All Files (*.*)|*.*||";
		Create.mFileName		= "TestScript.txt";
		Create.mInitialDir		= GetRoot();
		Create.mCaptionTitle	= "Select script file";
		Create.mDefExt			= "txt";

		String ScriptFilename;
		if(!FileselectOpenSingle(Create, ScriptFilename))
			return;

		RunScript(ScriptFilename);
	}
	else if(menu_item_id==MENU_ABOUT)
	{
		IceCore::MessageBox(0, "-= PEEL v2.0 =-\n\nCreated by Pierre Terdiman (2012-2020)\n\nUses the ICE framework - Pierre Terdiman (1999-2020)", "About PEEL", MB_OK);
	}
	else if(menu_item_id==MENU_HIDE_ALL)
	{
		dci.HideAll();
	}
	else if(menu_item_id==MENU_SHOW_ALL)
	{
		dci.ShowAll();
	}
	else if(menu_item_id==MENU_SELECT_ALL)
	{
		dci.SelectAll(true);
	}
	else if(menu_item_id==MENU_SHOW_SELECTION_DIALOG)
	{
		ShowActorSelectionWindow(null);
	}
	else if(menu_item_id==MENU_HIDE_SELECTED)
	{
		dci.SetSelectionVisibility(false);
	}
	else if(menu_item_id==MENU_SHOW_SELECTED)
	{
		dci.SetSelectionVisibility(true);
	}
	else if(menu_item_id==MENU_HIDE_UNSELECTED)
	{
		dci.HideUnselected();
	}
	else if(menu_item_id==MENU_SELECT_HIDDEN)
	{
		dci.SelectHidden();
	}
	else if(menu_item_id==MENU_DELETE_SELECTED)
	{
		dci.mSelMan.DeleteSelected();
	}
	else if(menu_item_id==MENU_EDIT_SELECTED)
	{
		dci.ShowEditActorWindow();
	}
	else if(menu_item_id==MENU_CANCEL_SELECTION)
	{
		dci.mSelMan.CancelSelection();
		UI_HideEditWindows();
	}
/*	else if(menu_item_id==MENU_CREATE_JOINTS)
	{
		dci.CreateJoints();
	}*/
	else if(menu_item_id==MENU_SCENE_PRINT_DEBUG_INFOS)
	{
		const udword NbShapeRenderers = GetNbShapeRenderers();
		OutputConsoleInfo(_F("NbShapeRenderers: %d\n", NbShapeRenderers));
		for(udword i=0;i<NbShapeRenderers;i++)
		{
			PintShapeRenderer* R = GetShapeRenderer(i);
			OutputConsoleInfo(_F("%d: %s\n", i, R->GetClassNameA()));
		}
		const udword NbManagedTextures = GetNbManagedTextures();
		OutputConsoleInfo(_F("NbManagedTextures: %d\n", NbManagedTextures));
	}
}

