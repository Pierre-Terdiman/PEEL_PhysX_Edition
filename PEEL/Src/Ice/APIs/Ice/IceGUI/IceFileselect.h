///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFILESELECT_H
#define ICEFILESELECT_H

	struct ICEGUI_API FILESELECTCREATE
	{
		String		mFilter;			//!< File filter (ex. "PI1 Files (*.pi1)|*.pi1|All Files (*.*)|*.*||")
		String		mFileName;			//!< Default File Name
		String		mInitialDir;		//!< Initial Directory
		String		mCaptionTitle;		//!< Dialog title
		String		mDefExt;			//!< Default extension
	};

	FUNCTION ICEGUI_API bool FileselectOpenSingle	(const FILESELECTCREATE& create, String& filename);
	FUNCTION ICEGUI_API bool FileselectOpenMultiple	(const FILESELECTCREATE& create, Strings& filenames);
	FUNCTION ICEGUI_API bool FileselectSave			(const FILESELECTCREATE& create, String& filename, bool check_overwrite=false);

#endif // ICEFILESELECT_H