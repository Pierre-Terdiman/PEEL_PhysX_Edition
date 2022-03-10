///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PEEL_MENU_BAR_H
#define PEEL_MENU_BAR_H

	enum MenuID
	{
		MENU_UNDEFINED,
		MENU_LOAD,
		MENU_MERGE,
		MENU_SAVE,
		MENU_RUN_SCRIPT,
		MENU_RECENT_FILE_0,
		MENU_RECENT_FILE_1,
		MENU_RECENT_FILE_2,
		MENU_RECENT_FILE_3,
		MENU_RECENT_FILE_4,
		MENU_RECENT_FILE_5,
		MENU_RECENT_FILE_6,
		MENU_RECENT_FILE_7,
		MENU_RECENT_FILE_8,
		MENU_RECENT_FILE_9,

		MENU_ABOUT,

		MENU_HIDE_ALL,
		MENU_SHOW_ALL,
		MENU_SELECT_ALL,
		MENU_SHOW_SELECTION_DIALOG,
		MENU_HIDE_SELECTED,
		MENU_SHOW_SELECTED,
		MENU_HIDE_UNSELECTED,
		MENU_SELECT_HIDDEN,
		MENU_DELETE_SELECTED,
		MENU_EDIT_SELECTED,
		MENU_CANCEL_SELECTION,

		//MENU_CREATE_JOINTS,

		MENU_SCENE_PRINT_DEBUG_INFOS,

		MENU_TOOL,	// Must be last
	};

	class Widgets;

	void CreatePEELMenuBar(IceWindow* parent, Widgets& widgets);
	void ReleasePEELMenuBar();

	void ProcessMenuEvent(udword menu_item_id);
	void UpdateToolsCheckmark();
	void UpdateSelectionItems();
	void UpdateRecentFiles();

#endif
