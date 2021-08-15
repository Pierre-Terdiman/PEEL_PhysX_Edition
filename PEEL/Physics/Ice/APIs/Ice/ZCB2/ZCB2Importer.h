#ifndef ZCB2IMPORTER_H
#define ZCB2IMPORTER_H

	class ZCB2_API ZCB2ImportCallback
	{
		public:
						ZCB2ImportCallback()	{}
		virtual			~ZCB2ImportCallback()	{}

		virtual	void	OnImportStart(udword nb_chunks)	= 0;
		virtual	void	OnNewChunk(udword index)		= 0;
	};

	class ZCB2_API ZCB2Importer : public Allocateable
	{
		public:
											ZCB2Importer();
		virtual								~ZCB2Importer();

						bool				Release();

						bool				Import(const char* filename, ZCB2ImportCallback* callback=null);
						bool				Import(const VirtualFile& file, ZCB2ImportCallback* callback=null);

		// Application-dependent import method, called by the importer for known chunks
		virtual			bool				NewChunk(const BaseChunk* chunk)	= 0;

		// Application-dependent import method, called by the importer for unknown chunks
		virtual			void				NewUnknownChunk(udword type, const char* name, const VirtualFile& file)	{}

		// Application-dependent error handler
//		virtual			bool				ZCB2ImportError			(const char* errortext, udword errorcode)	= 0;

		// Application-dependent log method
//		virtual			void				ZCB2Log					(LPSTR fmt, ...)							= 0;

		private:
//						Container			mChunks;
	};

#endif // ZCB2IMPORTER_H
