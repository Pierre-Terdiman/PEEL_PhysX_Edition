///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEEDITBOX_H
#define ICEEDITBOX_H

	enum EditBoxType
	{
		EDITBOX_NORMAL,
		EDITBOX_READ_ONLY,
		EDITBOX_PASSWORD,
	};

	enum EditBoxStyle
	{
		EDITBOX_ALIGNED_RIGHT	= (1<<0),
		EDITBOX_ALIGNED_CENTER	= (1<<1),
	};

	enum EditBoxFilter
	{
		EDITBOX_TEXT,
		EDITBOX_INTEGER,
		EDITBOX_INTEGER_POSITIVE,
		EDITBOX_FLOAT,
		EDITBOX_FLOAT_POSITIVE,
	};

	class IceEditBox;

	// EditBox callback
	typedef void (*EBCallback)	(const IceEditBox& edit_box, udword param, void* user_data);

	class ICEGUI_API EditBoxDesc : public WidgetDesc
	{
		public:
								EditBoxDesc();

				EditBoxType		mType;
				EditBoxFilter	mFilter;
				udword			mStyle;
				udword			mMaxLength;
				EBCallback		mCallback;
	};

	class ICEGUI_API IceEditBox : public IceWidget
	{
		public:
								IceEditBox(const EditBoxDesc& desc);
		virtual					~IceEditBox();

				void			SetMaxLength(int max);
				int				GetMaxLength()					const;

				void			SetReadOnly(bool flag);

				void			GetText(char* pOut, int len)	const;
				void			SetText(const char* text);
				void			SetMultilineText(const char* text);

				bool			GetTextAsFloat(float& value)	const;
				bool			GetTextAsInt(sdword& value)		const;
				bool			GetTextAsString(String& text)	const;

		// New implementations
				float			GetFloat()	const;
				sdword			GetInt()	const;

		inline_	void			SetCallback(EBCallback callback)		{ mCallback = callback;	}
		inline_	EBCallback		GetCallback()					const	{ return mCallback;		}

		inline_	EditBoxFilter	GetFilter()						const	{ return mFilter;		}

		virtual	void			OnReturn();

		virtual	bool			FilterKey(udword key)			const;

				enum CallbackParam
				{
					CB_PARAM_UNDEFINED,
					CB_PARAM_RETURN,
					CB_PARAM_FOCUS_LOST,
				};

		private:
				EBCallback		mCallback;
				EditBoxFilter	mFilter;
		public:	// ### "temp"
				CallbackParam	mParam;

				PREVENT_COPY(IceEditBox);
	};

#endif	// ICEEDITBOX_H