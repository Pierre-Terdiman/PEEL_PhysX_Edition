///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETOGGLEBUTTON_H
#define ICETOGGLEBUTTON_H

	class IceToggleButton;

	// Toggle button callback
	typedef void (*TBCallback)	(const IceToggleButton& button, bool checked, void* user_data);

	class ICEGUI_API ToggleButtonDesc : public ButtonDesc
	{
		public:
							ToggleButtonDesc();

				TBCallback	mCallback;
				bool		mChecked;
	};
	
	class ICEGUI_API IceToggleButton : public IceWidget
	{
		public:
							IceToggleButton(const ToggleButtonDesc& desc);
		virtual				~IceToggleButton();

				void		SetChecked(bool b);
				bool		IsChecked() const;

		virtual	void		OnClick();

		inline_	void		SetCallback(TBCallback callback)	{ mCallback = callback;	}
		inline_	TBCallback	GetCallback()	const				{ return mCallback;		}

				bool		SetImage(const IceBitmap& bitmap);

		private:
				TBCallback	mCallback;

				PREVENT_COPY(IceToggleButton);
	};

#endif	// ICETOGGLEBUTTON_H