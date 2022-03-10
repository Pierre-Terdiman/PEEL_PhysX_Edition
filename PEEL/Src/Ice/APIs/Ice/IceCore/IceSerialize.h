///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains serialization utilities.
 *	\file		IceSerialize.h
 *	\author		Pierre Terdiman
 *	\date		November, 30, 2004
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESERIALIZE_H
#define ICESERIALIZE_H

	ICECORE_API	void	WriteChunk(ubyte a, ubyte b, ubyte c, ubyte d, CustomArray& array);
	ICECORE_API	void	WriteChunk(ubyte a, ubyte b, ubyte c, ubyte d, WriteStream& stream);
	ICECORE_API	void	ReadChunk(ubyte& a, ubyte& b, ubyte& c, ubyte& d, const CustomArray& array);
	ICECORE_API	void	ReadChunk(ubyte& a, ubyte& b, ubyte& c, ubyte& d, const ReadStream& stream);
	ICECORE_API	udword	ReadDword(bool mismatch, const CustomArray& array);
	ICECORE_API	udword	ReadDword(bool mismatch, const ReadStream& stream);
	ICECORE_API	float	ReadFloat(bool mismatch, const CustomArray& array);
	ICECORE_API	float	ReadFloat(bool mismatch, const ReadStream& stream);
	ICECORE_API	bool	ReadWordBuffer(uword* dest, udword nb, bool mismatch, const ReadStream& stream);
	ICECORE_API	bool	ReadFloatBuffer(float* dest, udword nb, bool mismatch, const ReadStream& stream);
	ICECORE_API	bool	WriteHeader(ubyte a, ubyte b, ubyte c, ubyte d, udword version, WriteStream& stream);
	ICECORE_API	bool	ReadHeader(ubyte a_, ubyte b_, ubyte c_, ubyte d_, udword& version, bool& mismatch, const ReadStream& stream);

	inline_ void Flip(uword& v)
	{
		ubyte* b = (ubyte*)&v;
		ubyte temp = b[0];
		b[0] = b[1];
		b[1] = temp;
	}

	inline_ void Flip(sword& v)
	{
		Flip((uword&)v);
	}

	inline_ void Flip(udword& v)
	{
		ubyte* b = (ubyte*)&v;
		v = b[0]<<24 | b[1]<<16 | b[2]<<8 | b[3];
	}

	inline_ void Flip(sdword& v)
	{
		Flip((udword&)v);
	}

	inline_ void Flip(float& v)
	{
		Flip((udword&)v);
	}

	inline_	bool ReadDwordBuffer(udword* dest, udword nb, bool mismatch, const ReadStream& stream)
	{
		return ReadFloatBuffer((float*)dest, nb, mismatch, stream);
	}

#endif // ICESERIALIZE_H