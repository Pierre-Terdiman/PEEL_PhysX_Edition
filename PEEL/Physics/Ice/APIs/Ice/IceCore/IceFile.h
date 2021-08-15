///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains file related code.
 *	\file		IceFile.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFILE_H
#define ICEFILE_H

	#define FILE_SAVE	false		//!< Fileselect parameter.
	#define FILE_LOAD	true		//!< Fileselect parameter.

	// File access
	#define FILE_READ				0x1
	#define FILE_WRITE				0x2
	#define	FILE_SHAREREAD			0x4
	#define	FILE_SHAREWRITE			0x8

	// File attrib
	#define FILE_ATTRB_READONLY		0x00000001  
	#define FILE_ATTRB_HIDDEN		0x00000002  
	#define FILE_ATTRB_SYSTEM		0x00000004  
	#define FILE_ATTRB_DIRECTORY	0x00000008  
	#define FILE_ATTRB_ARCHIVE		0x00000010  
	#define FILE_ATTRB_NORMAL		0x00000020  

	// File open flags
	enum FILE_CREATEATTRB
	{
		FILE_CREATE_NEW				= 1,							// Create the file, fails if the same filename already exists
		FILE_CREATE_ALWAYS			= 2,							// Create or open an existing file (with truncation to zero)
		FILE_OPEN_EXISTING			= 3,							// Open an existing file, fails if the file doesn't exist
		FILE_OPEN_ALWAYS			= 4,							// Open an existing file, or create one if it doens't exist

		FILE_OPFORCEDW				= 0x7fffffff
	};

	// File seek flags
	enum FILE_SEEK
	{
		FILE_FROM_BEGIN				= 1,
		FILE_FROM_CURRENT			= 2, 
		FILE_FROM_END				= 4,

		FILE_FROM_FORCE				= 0x7fffffff
	};

	// Misc functions

	//ICECORE_API	bool		FileSelect(const char* type, const char* destname, const char* alias, bool mode, char* filename);

	FUNCTION ICECORE_API	bool		IsFile(const char* string);
	FUNCTION ICECORE_API	udword		GetFileSize(const char* name);
	FUNCTION ICECORE_API	bool		FileExists(const char* filename);
	FUNCTION ICECORE_API	const char*	ChangeExtension(const char* filename, const char* new_ext);
	FUNCTION ICECORE_API	bool		RemoveExtension(String& filename);
	FUNCTION ICECORE_API	const char*	GetExtension(const char* filename, String* extension=null);

#ifdef ICE_OBSOLETE
	FUNCTION ICECORE_API	udword		CreateFile(String& filepathname, udword fileaccess, FILE_CREATEATTRB fileattrib);
	FUNCTION ICECORE_API	bool		CloseFile(udword handle);
	FUNCTION ICECORE_API	udword		SeekFilePtr(udword handle, udword length, FILE_SEEK seek);
	FUNCTION ICECORE_API	bool		WriteFile(udword handle, const void* buffer, udword length, udword* writtenlength=null);
	FUNCTION ICECORE_API	bool		ReadFile(udword handle, void* buffer, udword length, udword* readlength);
	FUNCTION ICECORE_API	udword		GetFilePosition(udword handle);
	//FUNCTION ICECORE_API	udword		GetFileLength(udword handle);
	FUNCTION ICECORE_API	bool		DeleteFile(String& filepathname);
	FUNCTION ICECORE_API	bool		CopyFile(String& existingfile, String& newfile, bool overwrite);
	FUNCTION ICECORE_API	bool		MoveFile(String& existingfile, String& newfile);
	FUNCTION ICECORE_API	udword		GetFileAttributes(String& filepathname);
#endif
	FUNCTION ICECORE_API	const char* GetCurrentDirectory_();

	#define	PATH_MAX	_MAX_PATH

	#define	FINDFILE_DIR			0x01
	#define	FINDFILE_HIDDEN			0x02
	#define	FINDFILE_NORMAL			0x04
	#define	FINDFILE_READONLY		0x08

	class ICECORE_API IceFile : public Allocateable
	{
		public:
//									IceFile(const char* filename, const char* access);
									IceFile(const char* filename);
									~IceFile();

		inline_	bool				IsValid()			const	{ return mFp || mBuffer;	}
		inline_ const String&		GetName()			const	{ return mName;				}
		inline_ const ubyte*		GetBuffer()			const	{ return mBuffer;			}
		inline_ const udword		GetBufferLength()	const	{ return mBufferLength;		}

		// Loading
				ubyte				LoadByte();
				uword				LoadWord();
				udword				LoadDword();
				float				LoadFloat();
				double				LoadDouble();
				const char*			LoadString();
				bool				LoadBuffer(void* dest, udword size);
		// Saving
/*				bool				SaveByte(ubyte data);
				bool				SaveWord(uword data);
				bool				SaveDword(udword data);
				bool				SaveFloat(float data);
				bool				SaveString(const char* data);
				bool				SaveBuffer(const void* src, udword size);*/
		//
				bool				Seek(udword pos);
				bool				GetLine(char* buffer, udword buffer_size, udword* length=null);
				ubyte*				Load(udword& length);
		private:
				String				mName;
				FILE*				mFp;

				ubyte*				mBuffer;
				udword				mBufferLength;
	};

	///////////////////////////////////////////////////////////////////////////

	// Legacy file finder from ~1999, kept in case Flexporter still uses it.
	class ICECORE_API FileFinder : public Allocateable
	{
		public:
									FileFinder(const char* mask);
				bool				FindNext();
				bool				IsValid()	const	{ return (mHandle==(void*)-1)==0; }
		public:
				String				mFile;
				udword				mAttribs;
				udword				mSize;
		private:
				WIN32_FIND_DATA		mFindData;
				HANDLE				mHandle;
	};

	#define FILEFOUND		__ffice__.mFile;

	#define STARTFINDFILES(x)	\
	FileFinder	__ffice__(x);	\
	while(__ffice__.IsValid())	\
	{

	#define ENDFINDFILES		\
		__ffice__.FindNext();	\
	}

	///////////////////////////////////////////////////////////////////////////

	// Revisited file finder for PEEL.
	class ICECORE_API FileFinder2 : public Allocateable
	{
		public:
									FileFinder2(const char* mask);
				bool				FindNext();
		inline_	bool				IsValid()	const	{ return (mHandle!=(void*)-1);	}

		public:
				CHAR				mFile[MAX_PATH];
				udword				mAttribs;
				udword				mSize;
		private:
				WIN32_FIND_DATAA	mFindData;
				HANDLE				mHandle;
		inline_	void				ProcessFile();
	};

	typedef bool (*FileFinderCallback)(const char* filename, const char* path, void* user_data);

	struct ICECORE_API FindFilesParams
	{
		FindFilesParams() :
			mMask				(null)
		{
			mDir[0] = 0;
		}
		const char*	mMask;
		CHAR		mDir[MAX_PATH];
	};

	FUNCTION ICECORE_API void _FindFiles(FindFilesParams& params, FileFinderCallback callback, void* user_data);

	///////////////////////////////////////////////////////////////////////////

	FUNCTION ICECORE_API void UnixToDosName(char* name);
	FUNCTION ICECORE_API void DosToUnixName(char* name);

	enum DriveType
	{
		DRIVE_TYPE_UNKNOWN		= 0,
		DRIVE_TYPE_NO_ROOT_DIR	= 1,
		DRIVE_TYPE_REMOVABLE	= 2,
		DRIVE_TYPE_FIXED		= 3,
		DRIVE_TYPE_REMOTE		= 4,
		DRIVE_TYPE_CDROM		= 5,
		DRIVE_TYPE_RAMDISK		= 6,
	};

	struct Drive
	{
		char	mName;	// 'A' for floppy, 'C' for main HD, etc
		char	mType;	// DriveType enum
	};

	FUNCTION ICECORE_API udword GetAvailableDrives(Drive* drives);
	FUNCTION ICECORE_API void AllowFileAccess(bool flag);
	FUNCTION ICECORE_API bool IsFileAccessAllowed();

#endif // ICEFILE_H


