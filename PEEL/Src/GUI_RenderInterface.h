///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GUI_RENDER_INTERFACE_H
#define GUI_RENDER_INTERFACE_H

	class Pint;
	class PintRender;

	class GUI_RenderInterface
	{
		public:
						GUI_RenderInterface		()													{}
		virtual			~GUI_RenderInterface	()													{}

		virtual	void	PreRenderCallback		()													{}
		virtual	void	RenderCallback			(PintRender& render, Pint& pint, udword pint_index)	{}
		virtual	void	PostRenderCallback		()													{}
		virtual	void	FinalRenderCallback		()													{}
	};

#endif
