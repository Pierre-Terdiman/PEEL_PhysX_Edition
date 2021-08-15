///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GLSHADER_H
#define GLSHADER_H

	namespace GLShader
	{
		class Loader
		{
			public:
			Loader(const char* filename);
			~Loader();

			char*	mData;
		};

		struct Data
		{
			inline_	Data()
			{
				Reset();
			}

			inline_	void	Reset()
			{
				mProgram = mVertexShader = mFragmentShader = mGeometryShader = 0;
			}

			operator GLuint()	const
			{
				return mProgram;
			}

			GLuint	mProgram;
			GLuint	mVertexShader;
			GLuint	mFragmentShader;
			GLuint	mGeometryShader;
		};

		Data	CompileProgram(const char* vsource, const char* fsource, const char* gsource);
		Data	CompileMultiFilesShader(udword nb_sources, const char** sources);
		bool	GetCompileError(GLuint shader, bool print_error);
		void	ReleaseProgram(Data& prg);

		bool	SetUniform1f(GLuint program, const char* name, const float value);
		bool	SetUniform2f(GLuint program, const char* name, float val0, float val1);
		bool	SetUniform3f(GLuint program, const char* name, float val0, float val1, float val2);
		bool	SetUniform4f(GLuint program, const char* name, float val0, float val1, float val2, float val3);
		bool	SetUniform1i(GLuint program, const char* name, const sdword value);
	}

#endif
