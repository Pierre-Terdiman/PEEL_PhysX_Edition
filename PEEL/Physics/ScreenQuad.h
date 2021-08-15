///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef SCREEN_QUAD_H
#define SCREEN_QUAD_H

#include "GLShader.h"

	class PintGUIHelper;
	class Widgets;

	class ScreenQuadMode : public Allocateable
	{
		public:
							ScreenQuadMode();
		virtual				~ScreenQuadMode();

		virtual	const char*	GetUIName()		const								= 0;

		virtual	void		Apply(udword screen_width, udword screen_height)	= 0;

		virtual	void		CreateUI(PintGUIHelper& helper, Widgets* widgets, IceWindow* parent)	{}
	};

	class ShaderBasedScreenQuad : public ScreenQuadMode
	{
		public:
								ShaderBasedScreenQuad();
		virtual					~ShaderBasedScreenQuad();

				GLShader::Data	mProgram;
	};

	void Draw2DScreenQuad(udword screen_width, udword screen_height);

	enum ScreenQuadType
	{
		SCREEN_QUAD_BACK_COLOR,
		SCREEN_QUAD_GRADIENT,
		SCREEN_QUAD_COLOR_SPHERE,

		SCREEN_QUAD_COUNT
	};

#endif
