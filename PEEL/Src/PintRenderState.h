///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PINT_RENDER_STATE_H
#define PINT_RENDER_STATE_H

	void	SetEngineColor(const Point& color);
	void	SetMainColor(const Point& color);
	void	SetMainColor(const RGBAColor& color);
	void	ResetMainColor();
	Point	GetMainColor();

#endif
