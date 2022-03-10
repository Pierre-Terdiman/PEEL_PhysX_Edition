///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for file streams.
 *	\file		IceFileStream.h
 *	\author		Pierre Terdiman
 *	\date		September, 13, 2004
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEFILESTREAM_H
#define ICEFILESTREAM_H

	class ICECORE_API FileReadStream : public ReadStream
	{
		public:
												FileReadStream(const char* filename);
		virtual									~FileReadStream();

		inline_					bool			IsValid()								const	{ return mFp!=null;	}

		override(ReadStream)	bool			Seek(udword offset)						const;

		// Loading API
		override(ReadStream)	ubyte			ReadByte()								const;
		override(ReadStream)	uword			ReadWord()								const;
		override(ReadStream)	udword			ReadDword()								const;
		override(ReadStream)	float			ReadFloat()								const;
		override(ReadStream)	double			ReadDouble()							const;
		override(ReadStream)	bool			ReadBuffer(void* buffer, udword size)	const;

		private:
								FILE*			mFp;
	};

	class ICECORE_API FileWriteStream : public WriteStream
	{
		public:
												FileWriteStream(const char* filename);
		virtual									~FileWriteStream();

		inline_					bool			IsValid()								const	{ return mFp!=null;	}

		// Saving API
		override(WriteStream)	WriteStream&	StoreByte(ubyte b);
		override(WriteStream)	WriteStream&	StoreWord(uword w);
		override(WriteStream)	WriteStream&	StoreDword(udword d);
		override(WriteStream)	WriteStream&	StoreFloat(float f);
		override(WriteStream)	WriteStream&	StoreDouble(double f);
		override(WriteStream)	WriteStream&	StoreBuffer(const void* buffer, udword size);

		private:
								FILE*			mFp;
	};

#endif	// ICEFILESTREAM_H
