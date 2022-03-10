///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEGUIEVENT_H
#define ICEGUIEVENT_H

	enum EventType
	{
		EVENT_COMMAND,
		EVENT_DRAG_AND_DROP,
		EVENT_MOVE,
		EVENT_SIZE,
		EVENT_TIMER,
		EVENT_IDLE,
		EVENT_SHOW,
		EVENT_HIDE,
		EVENT_MOUSE_UP,
		EVENT_MOUSE_DOWN,
		EVENT_MOUSE_MOVE,
		EVENT_MOUSE_DRAG,
		EVENT_MOUSE_WHEEL,
		EVENT_KEY_UP,
		EVENT_KEY_DOWN,
		EVENT_FOCUS,
		EVENT_ACTIVATE,
		EVENT_NC_MOUSE_UP,
		EVENT_NC_MOUSE_DOWN,
		EVENT_NC_MOUSE_MOVE,
		EVENT_NC_MOUSE_DRAG,
		EVENT_CLOSE,
		EVENT_MOUSE_DBLCLK,
//		EVENT_SET_CURSOR,
		EVENT_MOUSE_LEAVE,

		EVENT_FORCE_DWORD	=	0x7fffffff
	};

	class IceWidget;

	class IceGUIEvent : public Allocateable
	{
		public:

	enum
	{
		MouseLeftButton		= 1,
		MouseRightButton	= 2,
		MouseMiddleButton	= 4
	};

	enum
	{
		KeyCtrl				= 1,
		KeyShift			= 2
	};

	enum
	{
		RightClicked		= 1,
		DoubleClicked		= 2
	};

					IceGUIEvent(EventType type=EVENT_FORCE_DWORD) :
						mType		(type),
						mWidget		(null),
						mCommand	(0),
						mWidth		(0),
						mHeight		(0),
						mX			(0),
						mY			(0),
						mButtons	(0),
						mKey		(0),
						mModifiers	(0),
						mFlags		(0),
						mFilename	(null),
						mMouse		(null)
						{}
					~IceGUIEvent()
						{}

		EventType		mType;
		IceWidget*		mWidget;
		int				mCommand;
		int				mWidth, mHeight;
		int				mX, mY, mButtons;
		int				mKey;
		int				mModifiers;
		int				mFlags;
		const char*		mFilename;

		MouseInfo*		mMouse;

		private:
					PREVENT_COPY(IceGUIEvent);
	};

#endif	// ICEGUIEVENT_H
