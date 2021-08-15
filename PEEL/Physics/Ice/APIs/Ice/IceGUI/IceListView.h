///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICELISTVIEW_H
#define ICELISTVIEW_H

	class ICEGUI_API ListViewDesc : public WidgetDesc
	{
		public:
							ListViewDesc();
	};

	class ICEGUI_API IceListView : public IceWidget
	{
		public:
							IceListView(const ListViewDesc& desc);
		virtual				~IceListView();

				int			Add(const char* label);
				void		Remove(int item);
				void		RemoveAll();
				void		SetLabel(int item, int column, const char* label);
				void		SetUserData(int item, int column, void* user_data);
				void		SetSelected(int item, bool b);
				void		DeselectAll();
				void		SetImageList(void* himagelist);
				void		SetImage(int item, int column, int imagenormal);

				void		InsertTextColumn(int column, int width, char const* label);
				void		InsertImageColumn(int column, int width, int imageindex);

				void		ScrollToItem(int item);

				int			GetItemCount()							const;
				int			GetNumSelected()						const;
				int			GetNextSelectedItem(int startitem = 0)	const;
				const char*	GetLabel( int oitem, int column)		const;
				void*		GetUserData(int item, int column)		const;
				bool		IsSelected(int index)					const;

				void		SetDrawingEnabled(bool draw);

		private:
				HWND		mHWnd;

				PREVENT_COPY(IceListView);
	};

#endif	// ICELISTVIEW_H