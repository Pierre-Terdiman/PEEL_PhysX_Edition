///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICELISTBOX_H
#define ICELISTBOX_H

	enum ListBoxType
	{
		LISTBOX_NORMAL,
		LISTBOX_MULTISELECTION,
	};

	enum ListBoxEvent
	{
		LBE_SELECTION_CHANGED,
		LBE_DBL_CLK,
	};

	class ICEGUI_API ListBoxDesc : public WidgetDesc
	{
		public:
								ListBoxDesc();

				ListBoxType		mType;
	};

	class ICEGUI_API IceListBox : public IceWidget
	{
		public:
								IceListBox(const ListBoxDesc& desc);
		virtual					~IceListBox();

		virtual	void			OnListboxEvent(ListBoxEvent event);

				void			Add(const char* item);
				void			Select(int index);
				void			Deselect(int index);
				void			Remove(int index);
				void			RemoveAll();
				void			SetItemText(int index, const char* text);
				void			SetItemData(int index, void* data);

				int				GetItemCount()			const;
				int				GetSelectedIndex()		const;
				bool			IsSelected(int index)	const;
				const char*		GetItemText(int index)	const;
				void*			GetItemData(int index)	const;

		private:
				PREVENT_COPY(IceListBox);
	};

#endif	// ICELISTBOX_H