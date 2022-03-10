#ifndef ZCB2BREAKER_H
#define ZCB2BREAKER_H

	#define ZCB2_VERSION	1

	class ZCB2_API ChunkDescriptor : public Allocateable
	{
		public:
					ChunkDescriptor()
					{
						mChunkType			= 0;
						mID					= 0;
						mFlags				= 0;
						mNameOffset			= 0;
						mVisibilityOffset	= 0;
						mStartOffset		= 0;
						mChunkSize			= 0;
						mFilenameOffset		= 0;
					}
					~ChunkDescriptor()	{}

		udword		mChunkType;
		udword		mID;
		udword		mFlags;
		udword		mNameOffset;
		udword		mVisibilityOffset;
		// Chunk location
		udword		mStartOffset;
		udword		mChunkSize;
		udword		mFilenameOffset;
	};

#endif // ZCB2BREAKER_H
