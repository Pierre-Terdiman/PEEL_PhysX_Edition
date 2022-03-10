///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SUPPORT_FILE_H
#define SUPPORT_FILE_H

	// TODO: refactor with ICE version

	class IceFile2 : public Allocateable
	{
		public:
		// Constructor/Destructor
//									IceFile(const char* filename, const char* access);
									IceFile2(const char* filename);
									~IceFile2();

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

	class VirtualFile2 : public ReadStream
	{
		public:
											VirtualFile2(const VirtualFileHandle& handle);
											VirtualFile2(const char* filename);
		virtual								~VirtualFile2();

		inline_					bool		IsValid()	const	{ return mBuffer!=null || mFile!=null;	}
		inline_					udword		GetSize()	const	{ return mSize;							}

		override(ReadStream)	bool		Seek(udword offset)							const;

		// Loading API
		override(ReadStream)	ubyte		ReadByte()									const;
		override(ReadStream)	uword		ReadWord()									const;
		override(ReadStream)	udword		ReadDword()									const;
		override(ReadStream)	float		ReadFloat()									const;
		override(ReadStream)	double		ReadDouble()								const;
		override(ReadStream)	bool		ReadBuffer(void* buffer, udword size)		const;

								const char*	ReadString()								const;
		//
								bool		GetLine(char* buffer, udword buffer_size, udword* length=null)	const;
								ubyte*		Load(udword& length)						const;

		inline_			const	IceFile2*	GetFile()			const	{ return mFile;	}

		private:
								udword		mSize;
								ubyte*		mBuffer;
		mutable					ubyte*		mCurrentAddress;

								IceFile2*	mFile;
		// Internal methods
								bool		ReadFromZip(const char* filename);
	};

	class ScriptFile2 : public CommandParser
	{
		public:
		// Constructor/Destructor
									ScriptFile2();
		virtual						~ScriptFile2();
		// Parsing method
		virtual	bool				Execute(const char* filename);
				bool				Execute(const VirtualFileHandle& handle);
				bool				Execute(VirtualFile2& file);
	};

	bool /*IceCore::*/_GetPath(const char* filename, String& path);
	udword /*IceCore::*/_GetFileSize(const char* name);

#endif
