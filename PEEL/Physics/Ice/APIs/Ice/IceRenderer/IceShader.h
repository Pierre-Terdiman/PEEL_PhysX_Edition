///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains shaders.
 *	\file		IceShader.h
 *	\author		Pierre Terdiman
 *	\date		November, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESHADER_H
#define ICESHADER_H

#ifdef TOFIX
	class Shader;

	class ICERENDERER_API Technique
	{
		private:
												Technique();
												~Technique();
		public:
		// Name
						Technique&				SetName(const String& name)			{ mName.Set(name);	return *this;	}
						String*					GetName()							{ return &mName;					}
		// Passes
		__forceinline	udword					GetNbPasses()				const	{ return mPasses.GetNbEntries();	}
		__forceinline	udword					GetPasse(udword i)			const	{ return mPasses.GetEntry(i);		}

		private:
						String					mName;
						Container				mPasses;

		friend class Shader;
	};

	class ICERENDERER_API ShaderParam
	{
		public:
		// Constructors/Destructor
												ShaderParam()					{}
												~ShaderParam()					{}

						String					mDecl;
						ucell					mValue;
	};

	typedef bool	(*SHADER_REQUEST_CALLBACK)		(ShaderParam& param, udword user_data);
	typedef bool	(*SHADER_COMMAND_CALLBACK)		(const String& command, udword user_data);

	//! A shader
	class ICERENDERER_API Shader
	{
		public:
												Shader();
		virtual									~Shader();

		// Creation
						bool					Create(Renderer* rd, const char* filename);

		// Callback control
		__forceinline	Shader&					SetParamUserData(udword data)							{ mParamUserData	= data;			return *this;	}
		__forceinline	Shader&					SetParamCallback(SHADER_REQUEST_CALLBACK callback)		{ mParamCallback	= callback;		return *this;	}
		__forceinline	Shader&					SetCommandUserData(udword data)							{ mCommandUserData	= data;			return *this;	}
		__forceinline	Shader&					SetCommandCallback(SHADER_COMMAND_CALLBACK callback)	{ mCommandCallback	= callback;		return *this;	}

		// Passes
		__forceinline	udword					GetNbTechniques()			const		{ return mTechniques.GetNbEntries();			}
		__forceinline	Technique*				GetTechnique(udword i)		const		{ return (Technique*)mTechniques.GetEntry(i);	}

		// Parameters
		__forceinline	udword					GetNbParams()				const		{ return mParams.GetNbEntries();				}
		__forceinline	ShaderParam*			GetParam(udword i)			const		{ return (ShaderParam*)mParams.GetEntry(i);		}

		private:
		static			udword					mParamUserData;		//!< User-defined data
		static			udword					mCommandUserData;	//!< User-defined data
		static			SHADER_REQUEST_CALLBACK	mParamCallback;		//!< Shader parameters callback
		static			SHADER_COMMAND_CALLBACK	mCommandCallback;	//!< Shader command callback

						Container				mTechniques;		//!< Array of Technique pointers
						Container				mParams;			//!< Array of ShaderParam pointers
	};
#endif

#endif // ICESHADER_H
