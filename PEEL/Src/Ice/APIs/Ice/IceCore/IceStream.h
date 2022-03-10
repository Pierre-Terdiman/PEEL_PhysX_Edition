///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for generic stream interfaces.
 *	\file		IceStream.h
 *	\author		Pierre Terdiman
 *	\date		September, 13, 2004
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESTREAM_H
#define ICESTREAM_H

	class ICECORE_API ReadStream : public Allocateable
	{
		public:
									ReadStream()			{}
		virtual						~ReadStream()			{}

		virtual		bool			Seek(udword offset)						const	= 0;

		// Loading API
		virtual		ubyte			ReadByte()								const	= 0;
		virtual		uword			ReadWord()								const	= 0;
		virtual		udword			ReadDword()								const	= 0;
		virtual		float			ReadFloat()								const	= 0;
		virtual		double			ReadDouble()							const	= 0;
		virtual		bool			ReadBuffer(void* buffer, udword size)	const	= 0;

		inline_		bool			ReadBool()								const	{ return ReadByte()!=0;	}
	};

	class ICECORE_API WriteStream : public Allocateable
	{
		public:
									WriteStream()			{}
		virtual						~WriteStream()			{}

		// Saving API
		virtual		WriteStream&	StoreByte(ubyte b)								= 0;
		virtual		WriteStream&	StoreWord(uword w)								= 0;
		virtual		WriteStream&	StoreDword(udword d)							= 0;
		virtual		WriteStream&	StoreFloat(float f)								= 0;
		virtual		WriteStream&	StoreDouble(double f)							= 0;
		virtual		WriteStream&	StoreBuffer(const void* buffer, udword size)	= 0;

		inline_		WriteStream&	StoreBool(bool b)		{ return StoreByte(ubyte(b));	}
		inline_		WriteStream&	StoreByte(sbyte b)		{ return StoreByte(ubyte(b));	}
		inline_		WriteStream&	StoreWord(sword w)		{ return StoreWord(uword(w));	}
		inline_		WriteStream&	StoreDword(sdword d)	{ return StoreDword(udword(d));	}
	};

#endif	// ICESTREAM_H
