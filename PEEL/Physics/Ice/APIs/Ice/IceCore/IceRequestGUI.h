///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for standard GUI requests.
 *	\file		IceRequestGUI.h
 *	\author		Pierre Terdiman
 *	\date		March, 23, 2008
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEREQUESTGUI_H
#define ICEREQUESTGUI_H

	struct ICECORE_API MessageBoxParams : public Allocateable
	{
						MessageBoxParams();

		const char*		mCaption;		//!< Message box title
		const char*		mMessage;		//!< Message
		const char*		mButtonText;	//!< Button texts, separated by |
	};

	class ICECORE_API GUIProgressBar : public Allocateable
	{
		public:
						GUIProgressBar()	{}
		virtual			~GUIProgressBar()	{}

		virtual	void	SetRange0(udword min, udword max)	= 0;
		virtual	void	SetRange1(udword min, udword max)	= 0;
		virtual	void	SetText0(const char* text)			= 0;
		virtual	void	SetText1(const char* text)			= 0;
		virtual	void	Update0()							= 0;
		virtual	void	Update1()							= 0;
	};

	class ICECORE_API GUIRequestInterface : public Allocateable
	{
		public:
								GUIRequestInterface()	{}
		virtual					~GUIRequestInterface()	{}

		virtual	udword			RequestMessageBox(const MessageBoxParams& params)	= 0;
		virtual	GUIProgressBar*	CreateProgressBar()									= 0;
		virtual	void			DeleteProgressBar(GUIProgressBar*)					= 0;
	};

	FUNCTION ICECORE_API	void SetGUIRequestInterface(GUIRequestInterface* it);
	FUNCTION ICECORE_API	GUIRequestInterface* GetGUIRequestInterface();

#endif // ICEREQUESTGUI_H
