///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef CONTROL_INTERFACE_H
#define CONTROL_INTERFACE_H

class PintRender;

namespace PEEL	// To avoid conflicts with the ICE ControlInterface class
{
	class ControlInterface
	{
		public:

						ControlInterface		()							{}
		virtual			~ControlInterface		()							{}

		virtual	bool	LeftDownCallback		(const MouseInfo& mouse)	{ return true;	}
		virtual	bool	MiddleDownCallback		(const MouseInfo& mouse)	{ return true;	}
		virtual	bool	RightDownCallback		(const MouseInfo& mouse)	{ return true;	}

		virtual	void	LeftDragCallback		(const MouseInfo& mouse)	{}
		virtual	void	MiddleDragCallback		(const MouseInfo& mouse)	{}
		virtual	void	RightDragCallback		(const MouseInfo& mouse)	{}

		virtual	void	LeftUpCallback			(const MouseInfo& mouse)	{}
		virtual	void	MiddleUpCallback		(const MouseInfo& mouse)	{}
		virtual	void	RightUpCallback			(const MouseInfo& mouse)	{}

		virtual	void	MouseMoveCallback		(const MouseInfo& mouse)	{}

		virtual	void	LeftDblClkCallback		(const MouseInfo& mouse)	{}
		virtual	void	MiddleDblClkCallback	(const MouseInfo& mouse)	{}
		virtual	void	RightDblClkCallback		(const MouseInfo& mouse)	{}

//		virtual	void	MouseWheelCallback		(const MouseInfo& mouse)	{}

		virtual	bool	KeyboardCallback		(unsigned char key, int x, int y, bool down)	{ return false;	}

		virtual	void	RenderCallback			(PintRender&)				{}
		virtual	void	Reset					()							{}
	};
}

	PEEL::ControlInterface*	GetControlInterface();
	void					SetControlInterface(PEEL::ControlInterface* ci);

	void					MouseCallback(int button, int state, int x, int y);
	void					MouseCallback2(int button, int state, int x, int y);
	void					MotionCallback(int x, int y);

	const MouseInfo&		GetMouseInfo();

#endif
