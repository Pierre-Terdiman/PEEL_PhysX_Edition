
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKPLIT_H
#define CHUNKPLIT_H

	#define PLIT_VERSION	1

	class ZCB2_API PLITChunk : public LITEChunk
	{
		DECLARE_CHUNK(PLITChunk, mPLITCore)

		DECLARE_STD_MEMBER(Range,			float)
		DECLARE_STD_MEMBER(Attenuation0,	float)
		DECLARE_STD_MEMBER(Attenuation1,	float)
		DECLARE_STD_MEMBER(Attenuation2,	float)
	};

#endif // CHUNKPLIT_H
