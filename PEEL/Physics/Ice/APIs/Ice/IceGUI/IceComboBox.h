///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECOMBOBOX_H
#define ICECOMBOBOX_H

	enum ComboBoxEvent
	{
		CBE_SELECTION_CHANGED,
		CBE_DBL_CLK,
	};

	class ICEGUI_API ComboBoxDesc : public WidgetDesc
	{
		public:
							ComboBoxDesc();
	};

	class ICEGUI_API IceComboBox : public IceWidget
	{
		public:
								IceComboBox(const ComboBoxDesc& desc);
		virtual					~IceComboBox();

		virtual	void			OnComboBoxEvent(ComboBoxEvent event);

				void			Add(const char* item);
				void			Select(int index);
				void			Remove(int index);
				void			RemoveAll();

				int				GetItemCount()		const;
				int				GetSelectedIndex()	const;

				void			SetItemData(int index, void* data);
				void*			GetItemData(int index)	const;

		private:
				PREVENT_COPY(IceComboBox);
	};

#endif	// ICECOMBOBOX_H