///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a shader compiler.
 *	\file		IceShaderCompiler.h
 *	\author		Pierre Terdiman
 *	\date		November, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESHADERCOMPILER_H
#define ICESHADERCOMPILER_H

	enum MicroCodeHeader
	{
		MCH_BOOLEAN_RENDER_STATE	= 1,	//!< Bool command
		MCH_DWORD_RENDER_STATE		= 2,	//!< Dword command
		MCH_TEXTURE_STAGE_STATE		= 3,	//!< Texture state command
		MCH_TEXTURE_TRANSFORM		= 4,	//!< Texture transform command
		MCH_MATERIAL				= 5,	//!< Material command
		MCH_PASS					= 6,	//!< Pass command
		MCH_DYNAMIC					= 7,	//!< Dynamic command
		MCH_END						= 8,	//!< End of code
	};

	class Shader;

	class ICERENDERER_API Technique : public Allocateable
	{
		private:
										Technique();
										~Technique();
		public:
		// Naming
					Technique&			SetName(const char* name)			{ mName = name;		return *this;		}
					const char*			GetName()							{ return mName;							}
		// Passes
		inline_		udword				GetNbPasses()				const	{ return mPasses.GetNbEntries();		}
		inline_		SBID				GetPass(udword i)			const	{ return mPasses.GetEntry(i) & 0xffff;	}
		inline_		BOOL				PassNeedRendering(udword i)	const	{ return mPasses.GetEntry(i)>>16;		}
		inline_		bool				IsValid()					const	{ return GetNbPasses()!=0;				}

		// Pipeline
		inline_		PipelineID			GetPipeline()				const	{ return mPipeline;						}
		inline_		void				SetPipeline(PipelineID id)			{ mPipeline = id;						}

		// Sorting key
//		inline_		TriangleSortKey		GetSortingKey()				const	{ return mSortKey;						}
//		inline_		void				SetSortingKey(TriangleSortKey key)	{ mSortKey = key;						}

					bool				Release(RenderStateManager& rsm);
		// Microcode
					CustomArray&		GetMicroCode()						{ return mMicroCode;					}
		private:
					String				mName;			//!< Technique's name
					CustomArray			mMicroCode;		//!< Technique's microcode
					Container			mPasses;		//!< Contains state-block IDs (one state block/pass)
					PipelineID			mPipeline;		//!< Pipeline hint
//					TriangleSortKey		mSortKey;		//!< Sorting key

		friend		class				Shader;
		friend		class				ShaderCompiler;
	};

	class ICERENDERER_API Shader// : public Allocateable
	{
		public:
										Shader();
		virtual							~Shader();

		inline_		bool				IsValid()					const		{ return mCurrentTech!=INVALID_ID;							}
		// Passes
		inline_		udword				GetNbTechniques()			const		{ return mTechniques.GetNbEntries();						}
		inline_		Technique*			GetTechnique(udword i)		const		{ return (Technique*)mTechniques.GetEntry(i);				}
		inline_		Technique*			GetCurrentTechnique()		const
										{
											if(mCurrentTech==INVALID_ID)	return null;
											return (Technique*)mTechniques.GetEntry(mCurrentTech);
										}

		inline_		PipelineID			GetCurrentPipeline()		const
										{
											if(mCurrentTech==INVALID_ID)	return PIPELINE_OPAQUE;
											Technique* CurTech = (Technique*)mTechniques.GetEntry(mCurrentTech);
											return CurTech->GetPipeline();
										}

					Technique*			AddTechnique();

					void				SelectNextTechnique();
					void				SelectPreviousTechnique();
					bool				SelectTechnique(udword index);

					bool				Release(RenderStateManager& rsm);

					bool				Capture(RenderStateManager& rsm);
		// Renderer-dependent
		virtual		udword				GetTextureBitmap(const String& identifier)	= 0;

		private:
					PtrContainer		mTechniques;		//!< Array of Technique pointers
					udword				mCurrentTech;		//!< Currently selected (validated) technique

		friend		class				ShaderCompiler;
	};

	//! Shader creation structure
	struct ICERENDERER_API SHADERCREATE
	{
		inline_		SHADERCREATE() : mFilename(null), mBuffer(null)	{}

		const char*	mFilename;	//!< file to compile, or null
		const char*	mBuffer;	//!< buffer to compile, or null

		Shader*		mShader;	//!< Destination shader
	};

	class ICERENDERER_API ShaderCompiler : public Allocateable
	{
		public:
									ShaderCompiler();
									~ShaderCompiler();
		// Creation

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Compiles a shader from a file or a buffer.
		 *
		 *	This method just creates the shader's microcode, not any hardware counterpart. This microcode is
		 *	hardware independent, and can be saved to disk for deferred processing. In order to use it, you
		 *	further need to validate the shader and capture it in a state block - or its software equivalent.
		 *
		 *	\param		sc	[in] shader creation structure
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					bool			Create(const SHADERCREATE& sc);

					CustomArray*	GetCurrentCode();

					Shader*			mShader;
					Technique*		mCurrentTechnique;
					MaterialProps	mCurrentMaterial;
					ubyte			mCurrentTextureStage;
//		private:
	};

#endif // ICESHADERCOMPILER_H
