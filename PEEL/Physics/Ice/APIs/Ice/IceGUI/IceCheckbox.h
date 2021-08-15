///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECHECKBOX_H
#define ICECHECKBOX_H

	class IceCheckBox;

	// Checkbox callback
	typedef void (*CBCallback)	(const IceCheckBox& check_box, bool checked, void* user_data);

	class ICEGUI_API CheckBoxDesc : public WidgetDesc
	{
		public:
							CheckBoxDesc();

				bool		mChecked;
				bool*		mMirror;
				CBCallback	mCallback;
	};

	class ICEGUI_API IceCheckBox : public IceWidget
	{
		public:
							IceCheckBox(const CheckBoxDesc& desc);
		virtual				~IceCheckBox();

				void		SetChecked(bool b);
				bool		IsChecked()		const;

		virtual	void		OnClick();

		inline_	void		SetCallback(CBCallback callback)	{ mCallback = callback;	}
		inline_	CBCallback	GetCallback()	const				{ return mCallback;		}

		private:
				CBCallback	mCallback;
				bool*		mMirror;
				PREVENT_COPY(IceCheckBox);
	};

#endif	// ICECHECKBOX_H