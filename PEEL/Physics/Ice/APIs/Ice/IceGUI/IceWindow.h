///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEWINDOW_H
#define ICEWINDOW_H

	class IceMenuBar;

	enum WindowType
	{
		WINDOW_NORMAL,
		WINDOW_POPUP,
		WINDOW_DIALOG,
		WINDOW_MODALDIALOG
	};

	enum WindowStyle
	{
		WSTYLE_NORMAL			= 0,
		WSTYLE_BORDER			= (1<<0),
		WSTYLE_DLGFRAME			= (1<<1),
		WSTYLE_THICKFRAME		= (1<<2),
		WSTYLE_STATIC_EDGES		= (1<<3),
		WSTYLE_CLIENT_EDGES		= (1<<4),
		WSTYLE_ALWAYS_ON_TOP	= (1<<5),
		WSTYLE_TRANSPARENT		= (1<<6),
		WSTYLE_HIDDEN			= (1<<7),
	};

	class ICEGUI_API WindowDesc : public WidgetDesc
	{
		public:
								WindowDesc();

				WindowType		mType;
				udword			mStyle;
				bool			mCloseAppWithWindow;
	};

	class ICEGUI_API IceWindow : public IceWidget
	{
		public:
								IceWindow(const WindowDesc& desc);
		virtual					~IceWindow();

		virtual int				handleEvent(IceGUIEvent* event);

		// Called at exit
		virtual bool			Closing()			{ return true;	}

				void			SetTimer(int milliSeconds);
				void			SetMenuBar(IceMenuBar* menuBar);
				void			SetDragAndDrop(bool b);

		inline_	bool			CloseAppWithWindow()	const	{ return mCloseAppWithWindow;	}

		private:
				UINT			mTimer;
				bool			mCloseAppWithWindow;
				PREVENT_COPY(IceWindow);
	};

#endif	// ICEWINDOW_H