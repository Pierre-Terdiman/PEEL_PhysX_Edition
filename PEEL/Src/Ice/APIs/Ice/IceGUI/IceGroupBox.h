///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEGROUPBOX_H
#define ICEGROUPBOX_H

	class ICEGUI_API GroupBoxDesc : public WidgetDesc
	{
		public:
							GroupBoxDesc();
	};

	class ICEGUI_API IceGroupBox : public IceWidget
	{
		public:
							IceGroupBox(const GroupBoxDesc& desc);
		virtual				~IceGroupBox();

		inline_	IceWindow*	GetSupport()	{ return mSupport;	}
		virtual	void		SetVisible(bool b);

				void		GetText(String& text)		const;
				void		SetText(const char* text)	const;
		private:
				IceWindow*	mSupport;

				PREVENT_COPY(IceGroupBox);
	};

#endif	// ICEGROUPBOX_H