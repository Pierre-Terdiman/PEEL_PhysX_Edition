///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESCROLLBAR_H
#define ICESCROLLBAR_H

	enum ScrollBarType
	{
		SCROLLBAR_HORIZONTAL,
		SCROLLBAR_VERTICAL,
	};

	enum ScrollBarEvent
	{
		SBE_SCROLL_UP_OR_LEFT,
		SBE_SCROLL_DOWN_OR_RIGHT,
		SBE_PAGE_UP_OR_LEFT,
		SBE_PAGE_DOWN_OR_RIGHT,
		SBE_THUMBPOSITION,
		SBE_THUMBTRACK,
		SBE_TOP_OR_LEFT,
		SBE_BOTTOM_OR_RIGHT,
		SBE_END_SCROLL,

		SBE_UNDEFINED,
	};

	class ICEGUI_API ScrollBarDesc : public WidgetDesc
	{
		public:
								ScrollBarDesc();

				ScrollBarType	mType;
	};


	class ICEGUI_API IceScrollbar : public IceWidget
	{
		public:
								IceScrollbar(const ScrollBarDesc& desc);
		virtual					~IceScrollbar();

		virtual	void			OnScrollbarEvent(ScrollBarEvent event);

				void			SetValue(int ivalue);
				int				GetValue()						const;

				void			SetRange(int min, int max);
				int				GetMinValue()					const;
				int				GetMaxValue()					const;

				void			SetPagesize(int size);
				int				GetPagesize()					const;

		private:
				PREVENT_COPY(IceScrollbar);
	};

#endif	// ICESCROLLBAR_H