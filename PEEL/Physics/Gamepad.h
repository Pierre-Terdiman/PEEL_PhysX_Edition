///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef GAMEPAD_H
#define GAMEPAD_H

	bool	InitGamepads();
	void	ReleaseGamepads();

	class GamepadInterface
	{
		public:
		virtual	void	OnButtonEvent(udword button_id, bool down)								= 0;
		virtual	void	OnAnalogButtonEvent(udword button_id, ubyte old_value, ubyte new_value)	= 0;
		virtual	void	OnAxisEvent(udword axis_id, float value)								= 0;
	};

	void	ProcessGamepads(GamepadInterface&);

#endif
