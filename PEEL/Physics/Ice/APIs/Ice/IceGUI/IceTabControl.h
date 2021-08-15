///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETABCONTROL_H
#define ICETABCONTROL_H

	enum TabControlStyle
	{
		TABCONTROL_NORMAL		= 0,
		TABCONTROL_BUTTONS		= (1<<0),
		TABCONTROL_FLATBUTTONS	= (1<<1),
		TABCONTROL_MULTILINE	= (1<<2),
	};

	class ICEGUI_API TabControlDesc : public WidgetDesc
	{
		public:
						TabControlDesc();

		TabControlStyle	mStyle;
	};

	class ICEGUI_API IceTabControl : public IceWidget
	{
		public:
						IceTabControl(const TabControlDesc& desc);
		virtual			~IceTabControl();

				void	Add(IceWidget* widget, const char* text);
				void	Remove(int index);
				void	Select(int index);

				int		GetSelectedIndex() const;

		private:
				PREVENT_COPY(IceTabControl);
	};

#endif	// ICETABCONTROL_H