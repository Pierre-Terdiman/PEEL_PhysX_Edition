///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLShader.h"
#include "SupportFile.h"

GLShader::Loader::Loader(const char* filename) : mData(null)
{
	const udword FileSize = _GetFileSize(filename);
	if(FileSize)
	{
		FILE* fp = fopen(filename, "rb");
		if(fp)
		{
			mData = (char*)ICE_ALLOC(sizeof(char)*(FileSize+1));
			fread(mData, FileSize, 1, fp);
			mData[FileSize] = 0;
			fclose(fp);
		}
	}
}

GLShader::Loader::~Loader()
{
	ICE_FREE(mData);
}

bool GLShader::GetCompileError(GLuint shader, bool print_error)
{
	if(!shader)
		return true;

	GLint Success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &Success);
	if(Success == GL_TRUE)
		return true;

	int infoLogLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if(infoLogLength<=1)
		return true;

	char* log = (char*)malloc(sizeof(char) * infoLogLength);
	int slen;
	glGetShaderInfoLog(shader, infoLogLength, &slen, log);

	if(print_error)
		printf("Shader compilation message: %s\n", log);

	free(log);
	return false;
}

/*
/*GLuint shader, GLenum pname, GLint* param
bool GLShder::CheckCompileStatus(GLuint shader)
{
	GLint* Success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &Success);
	if(Success != GL_TRUE)
	{
		getCompileError(shader);
		deleteShaders();
		return false;
	}
}*/

void GLShader::ReleaseProgram(GLShader::Data& data)
{
	if(data.mVertexShader)
	{
		glDetachShader(data.mProgram, data.mVertexShader);
		glDeleteShader(data.mVertexShader);
	}

	if(data.mFragmentShader)
	{
		glDetachShader(data.mProgram, data.mFragmentShader);
		glDeleteShader(data.mFragmentShader);
	}

	if(data.mGeometryShader)
	{
		glDetachShader(data.mProgram, data.mGeometryShader);
		glDeleteShader(data.mGeometryShader);
	}

	glDeleteProgram(data.mProgram);

	data.Reset();
}

static GLuint CompileShader(udword tag, GLuint prg, udword nb_sources, const char** sources)
{
	const GLuint ShaderHandle = glCreateShader(tag);
	glShaderSource(ShaderHandle, nb_sources, sources, NULL);
	glCompileShader(ShaderHandle);

	GLShader::GetCompileError(ShaderHandle, true);

//	GlslPrintShaderLog(ShaderHandle);
	glAttachShader(prg, ShaderHandle);

	return ShaderHandle;
}

GLShader::Data GLShader::CompileProgram(const char* vsource, const char* fsource, const char* gsource)
{
	printf("Compiling shader.... ");

	Data data;
	data.mProgram = glCreateProgram();

	if(vsource)
		data.mVertexShader = ::CompileShader(GL_VERTEX_SHADER, data.mProgram, 1, &vsource);

	if(fsource)
		data.mFragmentShader = ::CompileShader(GL_FRAGMENT_SHADER, data.mProgram, 1, &fsource);

	if(gsource)
	{
		data.mGeometryShader = ::CompileShader(GL_GEOMETRY_SHADER, data.mProgram, 1, &gsource);

		// hack, force billboard gs mode
		glProgramParameteriEXT ( data.mProgram, GL_GEOMETRY_VERTICES_OUT_EXT, 4 ) ; 
		glProgramParameteriEXT ( data.mProgram, GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS ) ; 
		glProgramParameteriEXT ( data.mProgram, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP ) ; 
	}

	glLinkProgram(data.mProgram);

	// check if program linked
	GLint success = 0;
	glGetProgramiv(data.mProgram, GL_LINK_STATUS, &success);

	if(!success)
	{
		char temp[1024];
		glGetProgramInfoLog(data.mProgram, 1024, 0, temp);
		printf("Failed to link program:\n%s\n", temp);

		ReleaseProgram(data);
	}
	printf("Done.\n");
	return data;
}

GLShader::Data GLShader::CompileMultiFilesShader(udword nb_sources, const char** sources)
{
	GLShader::Data data;

	data.mProgram = glCreateProgram();

	const char** lines = new const char*[nb_sources+1];
	for(udword i=0; i<nb_sources; i++)
		lines[i+1] = sources[i];

	lines[0] = "#version 130\n#define _VERTEX_\n";
	data.mVertexShader = ::CompileShader(GL_VERTEX_SHADER, data.mProgram, nb_sources+1, lines);

	lines[0] = "#version 130\n#define _FRAGMENT_\n";
	data.mFragmentShader = ::CompileShader(GL_FRAGMENT_SHADER, data.mProgram, nb_sources+1, lines);

	glLinkProgram(data.mProgram);

	delete[] lines;

	return data;
}

bool GLShader::SetUniform1f(GLuint program, const char* name, const float value)
{
	const GLint loc = glGetUniformLocation(program, name);
	if(loc >= 0)
	{
		glUniform1f(loc, value);
		return true;
	}
	return false;
}

bool GLShader::SetUniform2f(GLuint program, const char* name, float val0, float val1)
{
	const GLint loc = glGetUniformLocation(program, name);
	if(loc >= 0)
	{
		glUniform2f(loc, val0, val1);
		return true;
	}
	return false;
}

bool GLShader::SetUniform3f(GLuint program, const char* name, float val0, float val1, float val2)
{
	const GLint loc = glGetUniformLocation(program, name);
	if(loc >= 0)
	{
		glUniform3f(loc, val0, val1, val2);
		return true;
	}
	return false;
}

bool GLShader::SetUniform4f(GLuint program, const char* name, float val0, float val1, float val2, float val3)
{
	const GLint loc = glGetUniformLocation(program, name);
	if (loc >= 0)
	{
		glUniform4f(loc, val0, val1, val2, val3);
		return true;
	}
	return false;
}

bool GLShader::SetUniform1i(GLuint program, const char* name, const sdword value)
{
	const GLint loc = glGetUniformLocation(program, name);
	if(loc >= 0)
	{
		glUniform1i(loc, value);
		return true;
	}
	return false;
}
