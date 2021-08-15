///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMENU_H
#define ICEMENU_H

	class ICEGUI_API IceMenu : public IceGUIElement
	{
		public:
							IceMenu();
		virtual				~IceMenu();

				void		Add(const char* item, int id);
				void		AddMenu(const char* item, IceMenu* menu);
				void		AddSeparator();
				void		SetEnabled(int id, bool b);
				void		SetChecked(int id, bool b);

				bool		IsEnabled(int id) const;
				bool		IsChecked(int id) const;
		private:
				PREVENT_COPY(IceMenu);
	};

#endif	// ICEMENU_H