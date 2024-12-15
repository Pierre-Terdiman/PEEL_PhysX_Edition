///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FlatShader.h"
#include "GLShader.h"

///////////////////////////////////////////////////////////////////////////////

static const char* simpleVertexProgram = "#version 130\n" STRINGIFY(

void main()
{
	gl_Position = ftransform();
}
);

///////////////////////////////////////////////////////////////////////////////

static const char* simpleFragmentProgram = "#version 130\n" STRINGIFY(

void main()
{
	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
);

///////////////////////////////////////////////////////////////////////////////

FlatShader::FlatShader()
{
}

FlatShader::~FlatShader()
{
}

bool FlatShader::Init()
{
	const char* vsProg = simpleVertexProgram;
	const char* psProg = simpleFragmentProgram;
	return LoadShaderCode(vsProg, psProg);
}


