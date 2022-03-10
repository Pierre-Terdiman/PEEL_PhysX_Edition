///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for the console.
 *	\file		IceConsole.h
 *	\author		Pierre Terdiman
 *	\date		February, 22, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECONSOLE_H
#define ICECONSOLE_H

#ifdef SUPPORT_CONSOLE
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CONSTANTS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	#define CONSOLE_MAX_ROW					60
	#define CONVER							1.000f

	#define	LOGO_Y_POS						-0.02f

	#define CONSOLE_MAX_COMMAND_LENGTH		16
	#define	CONSOLE_MAX_COMMAND_NB			256

	enum ConsoleMode{
		MODE_CONSOLE		= 0,
		MODE_PLAYBACK		= 1,
		MODE_INTERACTIVE	= 2
	};

	struct color_t{
		ubyte	r, g, b;
	};

	struct row_t{
		ubyte		flash;
		color_t		color;
		sbyte		text[60];
	};

	class Console;

	typedef struct cmd_s {
		char	fullcmd[CONSOLE_MAX_COMMAND_LENGTH];
		void	(*function)(class Console* console, char* text, long user_data);
		struct cmd_s* next;
	} cmd_t;

	class Renderer;
	class Texture;

	class ICERENDERER_API Console
	{
	public:
						Console();
						~Console();

		void			SpeechOff()		{mSpeech = false;	}
		void			SpeechOn()		{mSpeech = true;	}

		bool			Create(long width, long height, signed char* font, signed char* logo, Renderer* renderer);
		bool			Draw();

		void			EventProc(UINT uMsg, WPARAM wParam);
		void			AddCmd(Console *, char *, void (*function)(Console *, char *, long));
		void			Out(char *);
		void			SwapEcho()		{ mEcho = !mEcho;			}
		void			FlipRotozoom()	{ mRotozoom = !mRotozoom;	}

		long			ExecCmd(char* cmd, char* param);

		bool			IsDestroyed()	{return mDestroying;	}
		bool			IsActive()		{return mActive;		}

		void			SetUserData(long user_data)	{ mUserData = user_data;}
		void			SetPrompt(const char* text);

	private:

		Renderer*		mRenderer;
		Texture*		mFonts;

		float			mTime;
		float			mAlphaTime;

		long			mUserData;

		long			mWidth;
		long			mHeight;

		row_t			mBuffer[CONSOLE_MAX_ROW];
		long			mNewline;
		long			mCol;
		long			mViewBottom;

		char			mCmdhist[30][60];
		long			mNewcmd;
		long			mNumcmdhist;
		long			mCurcmd;
		long			mNbCmds;

		char			mVer[10];

		cmd_t*			mCmds[CONSOLE_MAX_COMMAND_NB];

		long			mFontW, mFontH;		// characters in a row, column

		ubyte			mPrompt[256];
		ubyte			mLastChar[2];

		bool			mDestroying;
		bool			mActive;
		bool			mInitWasOk;
		bool			mDisappearing;
		bool			mSpeech;
		bool			mEcho;
		bool			mRotozoom;

		ConsoleMode		mMode;

		// Methods
		void			In(long);
		void			CmdHistory(long);
		void			Scroll(long);
		void			Advance();
		void			Alert(char *);
		void			Process();
		void			WriteChar(long row, long col, char letter, udword color);
		void			WriteString(long row, long col, char* string, udword color);
		void			CmdClear();
		void			ExecFile(char *);
		void			Clear();
		void			Destroy();
		void			ResetCol();

		friend	void	BasicCmdDir			(Console* con, char* text, long user_data);
		friend	void	BasicCmdCd			(Console* con, char* text, long user_data);
		friend	void	BasicCmdexit		(Console* con, char* text, long user_data);
		friend	void	BasicCmddestroy		(Console* con, char* text, long user_data);
		friend	void	BasicCmdecho		(Console* con, char* text, long user_data);
		friend	void	BasicCmdcls			(Console* con, char* text, long user_data);
		friend	void	BasicCmdExecFile	(Console* con, char* text, long user_data);
		friend	void	BasicCmdcmdlist		(Console* con, char* text, long user_data);
		friend	void	BasicCmdcmdhist		(Console* con, char* text, long user_data);
		friend	void	BasicCmdSetPrompt	(Console* con, char* text, long user_data);
	};
#endif

#endif	// ICECONSOLE_H