///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Base chunk for ZB2 format.
 *	\file		ChunkBase.h
 *	\author		Pierre Terdiman
 *	\date		August, 29, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CHUNKBASE_H
#define CHUNKBASE_H

	class ZCB2_API ChunkCore : public Allocateable
	{
		public:
		inline_					ChunkCore()	: mVersion(0), mFlags(0)	{}
		inline_					~ChunkCore()							{}

		inline_	bool			Export(CustomArray& array)	const
								{
									array
										.Store(mVersion)
										.Store(mFlags);
									return true;
								}

		inline_	bool			Import(const VirtualFile& file)
								{
									mVersion	= file.ReadWord();
									mFlags		= file.ReadWord();
									return true;
								}

		inline_	void			Enable(uword flag)				{ mFlags |= flag;			}
		inline_	void			Disable(uword flag)				{ mFlags &= ~flag;			}
		inline_	bool			IsSet(uword flag)		const	{ return (mFlags&flag)!=0;	}

				uword			mVersion;
				uword			mFlags;
	};

	class ZCB2_API ChunkProtocol
	{
		public:
								ChunkProtocol()					{}
		virtual					~ChunkProtocol()				{}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Validates the chunk by checking currently set parameters.
		 *	\return		null if ok, else a text describing the problem
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	const char*		Validate()						{ return null;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Exports the chunk to disk.
		 *	\param		array	[out] destination array
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool			Export(CustomArray& array)	= 0;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Imports the chunk from disk.
		 *	\param		file	[in] input file
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		virtual	bool			Import(const VirtualFile& file)	= 0;
	};

	// Contains data exported in the chunk descriptors.
	class ZCB2_API DescriptorInfo : public Allocateable
	{
		public:
								DescriptorInfo() : mChunkType(0), mID(INVALID_ID)	{}
		virtual					~DescriptorInfo()									{}

		// Data access
		inline_	udword			GetChunkType()			const	{ return mChunkType;	}
		inline_	const char*		GetName()				const	{ return mName.Get();	}
		inline_	udword			GetID()					const	{ return mID;			}

		// Chunk definition
		inline_	void			SetName(const char* name)		{ mName = name;			}
		inline_	void			SetID(udword id)				{ mID = id;				}

		protected:
				udword			mChunkType;
		private:
				String			mName;
				udword			mID;
	};

	#define DECLARE_CHUNK(current_class, core)											\
		private:			ChunkCore	core;											\
		public:																			\
							current_class();											\
		virtual				~current_class();											\
		virtual bool		Export(CustomArray& array);									\
		virtual bool		Import(const VirtualFile& file);							\
		virtual const char*	Validate();													\
		const	ChunkCore&	GetCore()	const	{ return core;	}						\
				ChunkCore&	GetCore()			{ return core;	}

	#define DECLARE_STD_MEMBER(name, type)												\
		private:		type		m##name;											\
		public:																			\
		inline_	void		Set##name(const type& value)	{ m##name	= value;	}	\
		inline_	const type&	Get##name()				const	{ return m##name;		}

	#define DECLARE_STD_FLAG(name, flag)												\
		bool	IsSet##name()	const	{ return GetCore().IsSet(flag);	}				\
		void	Set##name(bool b)														\
		{																				\
			if(b)	GetCore().Enable(flag);												\
			else	GetCore().Disable(flag);											\
		}

	#define DECLARE_SUBCHUNK(name, chunk_type)											\
		private:		chunk_type	m##name;											\
		public:																			\
		inline_	const	chunk_type&	Get##name()	const	{ return m##name;	}			\
		inline_			chunk_type&	Get##name()			{ return m##name;	}

	#define DECLARE_STD_ARRAY(name, type)												\
		private:	udword		mNb##name;												\
				type*			m##name;												\
		public:																			\
		inline_	udword			GetNb##name()	const	{ return mNb##name;	}			\
		inline_	const type*		Get##name()		const	{ return m##name;	}			\
				bool			SetNb##name(udword nb)									\
				{																		\
					if(!nb)	return false;												\
					mNb##name	= nb;													\
					m##name		= (type*)ICE_ALLOC(sizeof(type)*nb);					\
					CHECKALLOC(m##name);												\
					return true;														\
				}																		\
				bool			Set##name(udword nb, const type* values)				\
				{																		\
					if(!nb || !values)		return false;								\
					if(!SetNb##name(nb))	return false;								\
					CopyMemory(m##name, values, nb*sizeof(type));						\
					return true;														\
				}																		\
				bool			Set##name(udword i, const type& value)					\
				{																		\
					if(i>=mNb##name || !m##name)	return false;						\
					m##name[i] = value;													\
					return true;														\
				}

	#define DECLARE_STD_ICE_ARRAY(name, type)											\
		private:	udword		mNb##name;												\
				type*			m##name;												\
		public:																			\
		inline_	udword			GetNb##name()	const	{ return mNb##name;	}			\
		inline_	const type*		Get##name()		const	{ return m##name;	}			\
				bool			SetNb##name(udword nb)									\
				{																		\
					if(!nb)	return false;												\
					mNb##name	= nb;													\
					m##name		= ICE_NEW(type)[nb];									\
					CHECKALLOC(m##name);												\
					return true;														\
				}																		\
				bool			Set##name(udword nb, const type* values)				\
				{																		\
					if(!nb || !values)		return false;								\
					if(!SetNb##name(nb))	return false;								\
					CopyMemory(m##name, values, nb*sizeof(type));						\
					return true;														\
				}																		\
				bool			Set##name(udword i, const type& value)					\
				{																		\
					if(i>=mNb##name || !m##name)	return false;						\
					m##name[i] = value;													\
					return true;														\
				}

	class ZCB2_API BaseChunk : public DescriptorInfo, public ChunkProtocol
	{
		DECLARE_CHUNK(BaseChunk, mBaseCore)
	};

#endif // CHUNKBASE_H
