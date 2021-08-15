///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEBUTTON_H
#define ICEBUTTON_H

	enum ButtonStyle
	{
		BUTTON_NORMAL	= 0,
		BUTTON_FLAT		= (1<<0),
		BUTTON_PUSHLIKE	= (1<<1),
	};

	class IceButton;

	// Button callback
	typedef void (*BCallback)	(IceButton& button, void* user_data);

	class ICEGUI_API ButtonDesc : public WidgetDesc
	{
		public:
							ButtonDesc();

				ButtonStyle	mStyle;
				BCallback	mCallback;
	};

	class ICEGUI_API IceButton : public IceWidget
	{
		public:
								IceButton(const ButtonDesc& desc);
		virtual					~IceButton();

		virtual	void			OnClick();

		inline_	void			SetCallback(BCallback callback)	{ mCallback = callback;	}
		inline_	BCallback		GetCallback()			const	{ return mCallback;		}

				bool			SetImage(const IceBitmap& bitmap);
				bool			EnableBitmap();
				bool			SetBitmapColor(ubyte r, ubyte g, ubyte b);
		inline_	const RGBPixel&	GetBitmapColor()		const	{ return mColor;		}

				void			GetText(String& text)		const;
				void			SetText(const char* text)	const;

				void			SetButtonDown(bool button_down);
		inline_	bool			GetButtonDown()				const	{ return mButtonDown;	}
		private:
				BCallback		mCallback;
				IceBitmap*		mBitmap;
				RGBPixel		mColor;
				bool			mButtonDown;

				PREVENT_COPY(IceButton);
	};

#endif	// ICEBUTTON_H