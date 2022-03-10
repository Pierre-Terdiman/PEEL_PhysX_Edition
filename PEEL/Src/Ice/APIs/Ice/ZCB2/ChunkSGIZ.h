
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKSGIZ_H
#define CHUNKSGIZ_H

	#define SGIZ_VERSION	1

	enum ZCB2_SGIZ_Flag
	{
		ZCB2_SGIZ_HEMI	= (1<<0),
	};

	class ZCB2_API SGIZChunk : public HELPChunk
	{
		DECLARE_CHUNK(SGIZChunk, mSGIZCore)

		DECLARE_STD_MEMBER(Radius,	float)

		DECLARE_STD_FLAG(Hemi, ZCB2_SGIZ_HEMI)
	};

#endif // CHUNKSGIZ_H
