///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef FLAT_SHADER_H
#define FLAT_SHADER_H

#include "Shader.h"

	class FlatShader : public PEEL::Shader
	{
						PREVENT_COPY(FlatShader)
		public:
						FlatShader();
		virtual			~FlatShader();

				bool	Init();
	};

#endif