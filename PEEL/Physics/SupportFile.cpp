///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SupportFile.h"

IceFile2::IceFile2(const char* filename/*, const char* access*/)
{
	//### REMOVED
//	CheckFileAccessIsAllowed(filename);

	mBufferLength	= 0;
	mBuffer			= null;
//	mFp				= filename ? fopen(filename, access ? access : "rb") : null;
	mFp				= filename ? fopen(filename, "rb") : null;
	mName			= filename;

	if(mFp)	RegisterResource(filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IceFile2::~IceFile2()
{
	if(mFp)	fclose(mFp);

	ICE_FREE(mBuffer);
}

// Same as GetLine but returning the length as well
// ### refactor with GetLine and put as a utility function somewhere
static bool GetLine(FILE* fp, char* buffer, udword buffer_size, udword* length)
{
	// Checkings
	if(!fp || !buffer || !buffer_size)	return false;

	int c;
	udword i=0;
	do{
		c = fgetc(fp);
		if(c==EOF)
		{
			if(i==0)	return false;	// Nothing in the file
			// Else the line lacks a return character, but it's still technically valid
			buffer[i] = 0;
			if(length)	*length = i;
			return true;
		}
		if((c!=0x0d)&&(c!=0x0a))
		{
			buffer[i++]=(char)c;
			if(i==buffer_size)
			{
				ASSERT(0);
				IceCore::MessageBox(null, "GetLine:: buffer overflow!!", "Error", MB_OK);
				return false;
			}
		}
//	}while(c!=0x0d);
	}while(c!=0x0a);	// Check this one...
	buffer[i] = 0;
	if(length)	*length = i;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Catches a line in an already opened text file.
 *	\param		buffer		[out] the output buffer
 *	\param		buffer_size	[in] size of output buffer
 *	\param		length		[out] size of output text
 *	\return		true if success, else false if no more lines.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IceFile2::GetLine(char* buffer, udword buffer_size, udword* length)
{
	return ::GetLine(mFp, buffer, buffer_size, length);
}

// Check file pointer
#define CHECK_FILE_PTR(err)		if(!mFp)	{ SetIceError("IceFile::CHECK_FILE_PTR: invalid file pointer", null);	return err;}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Loads the whole file in a buffer.
 *	\param		length		[out] the buffer length
 *	\return		the buffer address, or null if failed
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ubyte* IceFile2::Load(udword& length)
{
	CHECK_FILE_PTR(null)

	// Check file size
	length = _GetFileSize(mName);
	if(!length)	{ SetIceError("IceFile::Load: file has zero length", null);	return null;}

	// Get some bytes
	ICE_FREE(mBuffer);
	mBuffer = (ubyte*)ICE_ALLOC(sizeof(ubyte)*length);
	if(!mBuffer)	{ SetIceError("IceFile::Load: out of memory", null);	return null;}

	mBufferLength = length;

	// Read the file
	fread(mBuffer, length, 1, mFp);

	// Close and free
	fclose(mFp);	mFp = null;

	// Return stored bytes
	return mBuffer;
}

ubyte IceFile2::LoadByte()
{
	CHECK_FILE_PTR(0)

	ubyte b;
	fread(&b, sizeof(ubyte), 1, mFp);
	return b;
}

uword IceFile2::LoadWord()
{
	CHECK_FILE_PTR(0)

	uword c;
	fread(&c, sizeof(uword), 1, mFp);
	return c;
}

udword IceFile2::LoadDword()
{
	CHECK_FILE_PTR(0)

	udword d;
	fread(&d, sizeof(udword), 1, mFp);
	return d;
}

float IceFile2::LoadFloat()
{
	CHECK_FILE_PTR(0.0f)

	float f;
	fread(&f, sizeof(float), 1, mFp);
	return f;
}

double IceFile2::LoadDouble()
{
	CHECK_FILE_PTR(0.0)

	double d;
	fread(&d, sizeof(double), 1, mFp);
	return d;
}

const char* IceFile2::LoadString()
{
	CHECK_FILE_PTR(null)

//	ASSERT(!"Fixme!");

	static char Buf[4096];

	// ### not really doing what the proto says!

	{
		udword Length;
		udword Offset=0;
		while(::GetLine(mFp, Buf+Offset, 4096, &Length))
		{
			if(!Length)
			{
				// ############ really hardcoded for KP..........!
				strcpy(Buf+Offset, "§SKIP§");
				Offset += 6;
			}
			else
			{
				Offset += Length;
				if(Offset>=4096)
				{
					IceCore::MessageBox(null, "IceFile::GetString:: buffer overflow!!", "Fatal error", MB_OK);
					return null;
				}
			}
		}
		return Buf;
	}

/*	udword Offset=0;
	ubyte b;
	do
	{
		b = LoadByte();
		Buf[Offset++] = b;
	}
	while(b);

	if(strlen(Buf)>=4096)
	{
		IceCore::MessageBox(null, "IceFile::GetString:: buffer overflow!!", "Fatal error", MB_OK);
	}

	return Buf;*/
}

bool IceFile2::LoadBuffer(void* dest, udword size)
{
	if(!dest || !size) return false;
	CHECK_FILE_PTR(false)

	fread(dest, size, 1, mFp);
	return true;
}

bool IceFile2::Seek(udword pos)
{
	CHECK_FILE_PTR(false)

	return fseek(mFp, pos, SEEK_SET)==0;
}


bool VirtualFile2::ReadFromZip(const char* filename)
{
/*	ArchiveHandle Handle;
	if(!IsInArchives(filename, &Handle))
		return false;

	Pack* p = reinterpret_cast<Pack*>(Handle.mArchive);
	udword i = Handle.mIndex;

	FILE* fp = fopen(p->name, "rb");
	if(!fp)	return false;

	fseek(fp, p->files[i].offset, SEEK_SET);

	mBuffer = (ubyte*)UnZip(fp);
	mCurrentAddress = mBuffer;
	mSize = p->files[i].size;
	fclose(fp);*/
	return true;
/*
	if(!filename)	return false;

	// Look for the file in registered ZIP files
	for(Pack* p=GetArchives(); p; p=p->next)
	{
		// Loop through files in the ZIP
		for(int i=0; i<p->nFiles; i++)
		{
			if(Stricmp(filename, p->files[i].name)==0)
			{
				FILE* fp = fopen(p->name, "rb");
				if(fp) 
				{
					fseek(fp, p->files[i].offset, SEEK_SET);

					mBuffer = (ubyte*)UnZip(fp);
					mCurrentAddress = mBuffer;
					mSize = p->files[i].size;
//					int size=p->files[i].size;
//					printf("%s: %d bytes\n", filename, size);
					fclose(fp);
					return true;
				}
			}
		}
	}
	return false;*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VirtualFile2::VirtualFile2(const VirtualFileHandle& handle) : mSize(0), mBuffer(null), mCurrentAddress(null), mFile(null)
{
/*	if(handle.IsInArchive())
	{
		Pack* p = reinterpret_cast<Pack*>(handle.mHandle.mArchive);
		FILE* fp = fopen(p->name, "rb");
		ASSERT(fp);
		if(fp) 
		{
			PackedFile& PF = p->files[handle.mHandle.mIndex];
			fseek(fp, PF.offset, SEEK_SET);

			mBuffer = (ubyte*)UnZip(fp);
			mCurrentAddress = mBuffer;
			mSize = PF.size;
//			int size=p->files[i].size;
//			printf("%s: %d bytes\n", filename, size);
			fclose(fp);
		}
	}
	else*/
	{
		mFile = ICE_NEW(IceFile2)(handle.mFilename);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VirtualFile2::VirtualFile2(const char* filename) : mSize(0), mBuffer(null), mCurrentAddress(null), mFile(null)
{
	// First, we try to read requested file from registered archives
	if(!ReadFromZip(filename))
	{
		// If the file hasn't been found in the archives, we try to load it directly, as an individual, usual file.
		mFile = ICE_NEW(IceFile2)(filename);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VirtualFile2::~VirtualFile2()
{
	DELETESINGLE(mFile);
	if(mBuffer)	free(mBuffer);	// Allocated in Unzip.cpp
	mCurrentAddress = null;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool VirtualFile2::Seek(udword offset) const
{
	if(mCurrentAddress)
	{
		mCurrentAddress = mBuffer + offset;
		ASSERT(mCurrentAddress<=mBuffer+mSize);
		return true;
	}
	ASSERT(mFile);
	return mFile->Seek(offset);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ubyte VirtualFile2::ReadByte() const
{
	if(mCurrentAddress)
	{
		const ubyte tmp = *mCurrentAddress;
		mCurrentAddress += sizeof(ubyte);
		ASSERT(mCurrentAddress<=mBuffer+mSize);
		return tmp;
	}
	ASSERT(mFile);
	return mFile->LoadByte();
}

uword VirtualFile2::ReadWord() const
{
	if(mCurrentAddress)
	{
		const uword tmp = *reinterpret_cast<uword*>(mCurrentAddress);
		mCurrentAddress += sizeof(uword);
		ASSERT(mCurrentAddress<=mBuffer+mSize);
		return tmp;
	}
	ASSERT(mFile);
	return mFile->LoadWord();
}

udword VirtualFile2::ReadDword() const
{
	if(mCurrentAddress)
	{
		const udword tmp = *reinterpret_cast<udword*>(mCurrentAddress);
		mCurrentAddress += sizeof(udword);
		ASSERT(mCurrentAddress<=mBuffer+mSize);
		return tmp;
	}
	ASSERT(mFile);
	return mFile->LoadDword();
}

float VirtualFile2::ReadFloat() const
{
	if(mCurrentAddress)
	{
		const float tmp = *reinterpret_cast<float*>(mCurrentAddress);
		mCurrentAddress += sizeof(float);
		ASSERT(mCurrentAddress<=mBuffer+mSize);
		return tmp;
	}
	ASSERT(mFile);
	return mFile->LoadFloat();
}

double VirtualFile2::ReadDouble() const
{
	if(mCurrentAddress)
	{
		const double tmp = *reinterpret_cast<double*>(mCurrentAddress);
		mCurrentAddress += sizeof(double);
		ASSERT(mCurrentAddress<=mBuffer+mSize);
		return tmp;
	}
	ASSERT(mFile);
	return mFile->LoadDouble();
}

bool VirtualFile2::ReadBuffer(void* dest, udword size) const
{
	if(mCurrentAddress)
	{
		CopyMemory(dest, mCurrentAddress, size);
		mCurrentAddress += size;
		ASSERT(mCurrentAddress<=mBuffer+mSize);
		return true;
	}
	return mFile->LoadBuffer(dest, size);
}

const char* VirtualFile2::ReadString() const
{
	if(mCurrentAddress)
	{
		const char* StringAddress = (const char*)mCurrentAddress;

		ubyte c;
		do{
			c = ReadByte();
		}
		while(c);

		return StringAddress;
	}
	return mFile->LoadString();
}

bool VirtualFile2::GetLine(char* buffer, udword buffer_size, udword* length) const
{
	if(mCurrentAddress)
	{
		// Adapted from IceFile::GetLine

		int c;
		udword i=0;
		do{
//			c = fgetc(mFp);
			c = *mCurrentAddress++;
//			if(c==EOF)
//			if(mCurrentAddress==mBuffer+mSize)
			if(mCurrentAddress>=mBuffer+mSize)
			{
				if(i==0)	return false;	// Nothing in the file
				// Else the line lacks a return character, but it's still technically valid
				buffer[i] = 0;
				if(length)	*length = i;
				return true;
			}
			if((c!=0x0d)&&(c!=0x0a))
			{
				buffer[i++]=(char)c;
				if(i==buffer_size)
				{
	//				return SetIceError(
					ASSERT(0);
					IceCore::MessageBox(null, "IceFile::GetLine:: buffer overflow!!", "Error", MB_OK);
					return false;
				}
			}
	//	}while(c!=0x0d);
		}while(c!=0x0a);	// Check this one...
		buffer[i] = 0;
		if(length)	*length = i;
		return true;
	}
	return mFile->GetLine(buffer, buffer_size, length);
}

ubyte* VirtualFile2::Load(udword& length) const
{
	if(mCurrentAddress)
	{
		length = mSize;
		return mBuffer;
	}
	return mFile->Load(length);
}




ScriptFile2::ScriptFile2()
{
	// The file parser supports the include command
	ForceEnable(BFC_SUPPORT_INCLUDE);
}

ScriptFile2::~ScriptFile2()
{
}

bool ScriptFile2::Execute(const char* filename)
{
	// Checkings
	if(!filename)				return false;

	// Check the file exists
	VirtualFileHandle FoundFile;
	if(!FindFile(filename, &FoundFile))	return false;

	// Open it as binary
//	IceFile File(FoundFile, "rb");
	VirtualFile2 File(FoundFile);

	// Call the base method
	CommandParser::Execute(filename);

	return Execute(File);
}

bool ScriptFile2::Execute(const VirtualFileHandle& handle)
{
	// Open it as binary
//	IceFile File(FoundFile, "rb");
	VirtualFile2 File(handle);

	// Call the base method
	CommandParser::Execute(handle.mFilename);

	return Execute(File);
}

//#define MAX_SIZE	1024*1024
#define MAX_SIZE	1024*1024*10

bool ScriptFile2::Execute(VirtualFile2& file)
{
	// Loop through all lines of the file
//	char Buffer[4096];	// Current text line ### 4096...
	char* Buffer = new char[MAX_SIZE];
	bool GoAhead = true;
	udword Length;
//	while(GoAhead && file.GetLine(Buffer, 4096, &Length))
	while(GoAhead && file.GetLine(Buffer, MAX_SIZE, &Length))
	{
//		if(Length>=4096)
		if(Length>=MAX_SIZE)
//		if(strlen(Buffer)>=4096)
		{
			IceCore::MessageBox(null, "ScriptFile::Execute:: buffer overflow!!", "Fatal error", MB_OK);
		}

		GoAhead = CommandParse(Buffer);
	}
	// If the script has been interrupted, consider there's been a fatal error
	DELETEARRAY(Buffer);
	return GoAhead;
}

// TODO: these IceCore functions don't work on x64 for some reason (the disassembly is super strange). Redefining them locally solves the problem.

bool /*IceCore::*/_GetPath(const char* filename, String& path)
{
	if(!filename)
		return false;

	const char* Separator0 = strrchr(filename, '\\');
	const char* Separator1 = strrchr(filename, '/');
	if(!Separator0 && !Separator1)
		return false;

	const char* Separator = TMax(Separator0, Separator1);

	const udword Offset = udword(Separator - filename);
	path = filename;
	path.SetAt(0, Offset+1);
	return true;
}

udword /*IceCore::*/_GetFileSize(const char* name)
{
	// Checkings
	if(!name)
		return 0;

	#ifndef SEEK_END
	#define SEEK_END 2
	#endif

	//CheckFileAccessIsAllowed(name);

	FILE* File = fopen(name, "rb");
	if(!File)
	{
		SetIceError("IceCore::GetFileSize: file not found.", null);
		return 0;
	}
	fseek(File, 0, SEEK_END);
	udword eof_ftell = ftell(File);
	fclose(File);
	return eof_ftell;
}
