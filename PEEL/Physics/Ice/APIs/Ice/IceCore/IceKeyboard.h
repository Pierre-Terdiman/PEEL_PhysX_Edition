///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains key codes.
 *	\file		IceKeyboard.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEKEYBOARD_H
#define ICEKEYBOARD_H

	class ICECORE_API KeyToggle
	{
		public:
		inline_			KeyToggle() : mPrevious(false)	{}
		inline_			~KeyToggle()					{}

		inline_	void	Reset()	{ mPrevious = false;	}

		inline_	bool	Update(bool new_value)
						{
							const bool Toggle = (!new_value && mPrevious);
							mPrevious = new_value;
							return Toggle;
						}
		private:
				bool	mPrevious;
	};

	class ICECORE_API KeyboardMapping : public Allocateable
	{
		public:
									KeyboardMapping();
									~KeyboardMapping();

				bool				CreateAccelerator(udword keycode, const char* command);
				bool				CreateAccelerator(const char* key, const char* command);
				void				DeleteAccelerators();
				const String*		GetCommand(udword keycode);
		protected:
				SymbolicList		mList;
				String				mInternal;
	};

	ICECORE_API udword				GetKeyCode(const char* key);
	ICECORE_API const char*			GetKeyName(udword vk_code);
	// For Windows
	ICECORE_API bool				IsKeyPressed(udword key);
	// For a DOS console
	ICECORE_API bool				IsConsoleKeyPressed();
	ICECORE_API int					GetConsoleKeyPressed();

	// Accelerators
	ICECORE_API bool				SetUserMapping(const KeyboardMapping* mapping);
	ICECORE_API KeyboardMapping*	GetUserMapping();
	ICECORE_API bool				CreateAccelerator(udword keycode, const char* command);
	ICECORE_API bool				CreateAccelerator(const char* key, const char* command);
	ICECORE_API void				DeleteAccelerators();
	ICECORE_API const String*		GetCommand(udword keycode);

#endif // ICEKEYBOARD_H
