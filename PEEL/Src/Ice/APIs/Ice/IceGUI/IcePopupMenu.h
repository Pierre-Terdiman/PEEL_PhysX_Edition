///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPOPUPMENU_H
#define ICEPOPUPMENU_H

	FUNCTION ICEGUI_API HMENU	StartPopupMenu();
	FUNCTION ICEGUI_API bool	AddPopupMenuEntry(HMENU menu, const char* text, udword id, bool checked, bool disabled);
	FUNCTION ICEGUI_API udword	DisplayPopupMenu(HWND hwnd, HMENU menu, sdword xs, sdword ys);
	FUNCTION ICEGUI_API void	DestroyPopupMenu(HMENU menu);

	FUNCTION ICEGUI_API udword	PopupMenuSelection(HWND hwnd, const Strings& entries, sdword xs, sdword ys);

	class ICEGUI_API IcePopupMenu : public IceGUIElement
	{
		public:
							IcePopupMenu();
		virtual				~IcePopupMenu();

				int			Popup(IceWidget* widget, int x, int y);
				void		Add(const char* item, int id);
				void		AddMenu(const char* item, IcePopupMenu* menu);
				void		AddSeparator();
				void		SetEnabled(int id, bool b);
				void		SetChecked(int id, bool b);

				bool		IsEnabled(int id)	const;
				bool		IsChecked(int id)	const;
		private:
				PREVENT_COPY(IcePopupMenu);
	};

#endif // ICEPOPUPMENU_H