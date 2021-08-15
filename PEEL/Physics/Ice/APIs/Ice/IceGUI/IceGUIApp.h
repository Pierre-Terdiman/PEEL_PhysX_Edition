///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an ICE application skeleton.
 *	\file		IceGUIApp.h
 *	\author		Pierre Terdiman
 *	\date		June, 28, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEGUIAPP_H
#define ICEGUIAPP_H

	//! Application's base creation structure
	struct ICEGUI_API ICEGUIAPPCREATE
	{
						ICEGUIAPPCREATE();

		// WinMain parameters, to be passed to the base app
		HINSTANCE		hInstance;			//!< Copied from WinMain
		HINSTANCE		hPrevInstance;		//!< Copied from WinMain
		LPSTR			lpCmdLine;			//!< Copied from WinMain
		int				nCmdShow;			//!< Copied from WinMain
		// Application settings
/*		const char*		mAppName;			//!< Application's name
		const char*		mMenuName;			//!< Possible menu name
		const char*		mScriptName;		//!< Possible script name
		sdword			mPosX;				//!< X position on screen
		sdword			mPosY;				//!< Y position on screen
		sdword			mWidth;				//!< Width or -1 for screen size
		sdword			mHeight;			//!< Height or -1 for screen size
		BOOL			mForceSingleRun;	//!< TRUE: allows only one instance of the app at the same time
		BOOL			mFullscreen;		//!< TRUE: run in fullscreen mode, FALSE: run in windowed mode*/
	};

	class ICEGUI_API IceGUIApp : public Allocateable
	{
		public:
								IceGUIApp();
		virtual					~IceGUIApp();

		virtual	bool			Init(const ICEGUIAPPCREATE& create);
		virtual	int				Run();
		virtual	void			Quit();
		virtual	void			SetIdleWindow(IceWindow* window);

		virtual	IceWindow*		GetMainWindow();

		protected:
		virtual	bool			InitializeScene()		{ return true;	}
		virtual	bool			ProcessCommandLineParam(const String& parameter);

		private:
				PREVENT_COPY(IceGUIApp);

				HINSTANCE		hInstance;			//!< Copied from WinMain
				HINSTANCE		hPrevInstance;		//!< Copied from WinMain
				LPSTR			lpCmdLine;			//!< Copied from WinMain
				int				nCmdShow;			//!< Copied from WinMain
	};

	FUNCTION ICEGUI_API	bool	InitGUI();
	FUNCTION ICEGUI_API	void	CloseGUI();

#endif // ICEGUIAPP_H

