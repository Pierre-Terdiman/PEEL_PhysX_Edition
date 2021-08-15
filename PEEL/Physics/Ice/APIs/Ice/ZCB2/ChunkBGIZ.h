
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKBGIZ_H
#define CHUNKBGIZ_H

	#define BGIZ_VERSION	1

	class ZCB2_API BGIZChunk : public HELPChunk
	{
		DECLARE_CHUNK(BGIZChunk, mBGIZCore)

		DECLARE_STD_MEMBER(Length,	float)
		DECLARE_STD_MEMBER(Width,	float)
		DECLARE_STD_MEMBER(Height,	float)
	};

#endif // CHUNKBGIZ_H
