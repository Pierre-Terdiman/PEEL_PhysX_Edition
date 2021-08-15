#ifndef ZCB2EXPORTER_H
#define ZCB2EXPORTER_H

	class ZCB2_API ZCB2Exporter : public Allocateable
	{
		public:
								ZCB2Exporter();
								~ZCB2Exporter();

				struct ChunkInfo
				{
					BaseChunk*	mChunk;
					BOOL		mMustDelete;
				};

				bool			Release();

				BaseChunk*		CreateChunk(udword type);

				BaseChunk*		RegisterUserDefinedChunk(BaseChunk*, BOOL transfer_ownership=TRUE);

				bool			Export(const char* filename);
		private:
				Container		mChunkData;
	};

#endif // ZCB2EXPORTER_H
