
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKSLIT_H
#define CHUNKSLIT_H

	#define SLIT_VERSION	1

	class ZCB2_API SLITChunk : public LITEChunk
	{
		DECLARE_CHUNK(SLITChunk, mSLITCore)

		DECLARE_STD_MEMBER(Range,			float)
		DECLARE_STD_MEMBER(Falloff,			float)
		DECLARE_STD_MEMBER(Attenuation0,	float)
		DECLARE_STD_MEMBER(Attenuation1,	float)
		DECLARE_STD_MEMBER(Attenuation2,	float)
		DECLARE_STD_MEMBER(Theta,			float)
		DECLARE_STD_MEMBER(Phi,				float)
		DECLARE_STD_MEMBER(TDist,			float)
	};

#endif // CHUNKSLIT_H
