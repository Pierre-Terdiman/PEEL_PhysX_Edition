///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETREEVIEW_H
#define ICETREEVIEW_H

	typedef void* IceTreeViewItem;

	//typedef int __stdcall (*TreeSortFunc)(long lParam1, long lParam2, long lParamSort);

	enum TreeViewEvent
	{
		TVE_SELECTION_CHANGED,
		TVE_CHECKBOX_CHANGED,
		TVE_BEGIN_LABEL_EDIT,
		TVE_END_LABEL_EDIT,
	};

	enum TreeViewStyle
	{
		TREEVIEW_NORMAL			= 0,
		TREEVIEW_CHECKBOXES		= (1<<0),
		TREEVIEW_EDIT_LABELS	= (1<<1),
	};

	class ICEGUI_API TreeViewDesc : public WidgetDesc
	{
		public:
									TreeViewDesc();

				udword				mStyle;
	};

	class ICEGUI_API IceTreeView : public IceWidget
	{
		public:
									IceTreeView		(const TreeViewDesc& desc);
		virtual						~IceTreeView	();

		virtual	udword				OnTreeViewEvent(TreeViewEvent event, void* context);

				IceTreeViewItem*	Add				(IceTreeViewItem* parent, const char* item);
				void				Remove			(IceTreeViewItem* item);
				void				RemoveAll		();
				void				SetLabel		(IceTreeViewItem* item, const char* label);
				void				SetUserData		(IceTreeViewItem* item, void* user_data);
				void				SetOpen			(IceTreeViewItem* item, bool b);
				void				SetSelected		(IceTreeViewItem* item, bool b);
				void				SetImageList	(void* himagelist);
				void				SetImages		(IceTreeViewItem* item, int imagenormal, int imageselected );
				void				MoveItemDown	(IceTreeViewItem* item);
#ifndef _WIN64
				void				SortTree		(IceTreeViewItem* parent, bool recurse, void* func, int parameter);
#endif
				void				ScrollTo		(IceTreeViewItem* item);

				IceTreeViewItem*	GetFirstChild	(IceTreeViewItem* item) const;
				IceTreeViewItem*	GetNextChild	(IceTreeViewItem* item) const;
				IceTreeViewItem*	GetSelectedItem	() const;
				const char*			GetLabel		(IceTreeViewItem* item) const;
				void*				GetUserData		(IceTreeViewItem* item) const;
				bool				IsOpen			(IceTreeViewItem* item) const;
				bool				IsSelected		(IceTreeViewItem* item) const;
				IceTreeViewItem*	GetParent		(IceTreeViewItem* item) const;

		// For TREEVIEW_CHECKBOXES
				void				SetChecked(IceTreeViewItem* item, bool checked);
				bool				IsChecked(IceTreeViewItem* item)		const;

		inline_	bool				GetRemoveAllFlag()	const	{ return mRemoveAll;	}

		private:
				PREVENT_COPY(IceTreeView);

				bool				mRemoveAll;
	};

#endif	// ICETREEVIEW_H