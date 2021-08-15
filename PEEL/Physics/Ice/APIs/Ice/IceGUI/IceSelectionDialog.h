///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESELECTIONDIALOG_H
#define ICESELECTIONDIALOG_H

#define MAX_NB_FILTERS			10
#define	MAX_NB_USER_CHECKBOXES	4

	struct ICEGUI_API SelectionDialogDesc : WindowDesc
	{
		SelectionDialogDesc() :
			mFilters(null),
			mMultiSelection(true)
		{
			ZeroMemory(mUserCheckboxes, MAX_NB_USER_CHECKBOXES*sizeof(const char*));
		}

		const char*				mFilters;
		bool					mMultiSelection;
		const char*				mUserCheckboxes[MAX_NB_USER_CHECKBOXES];
	};

	class ICEGUI_API SelectionDialog : public IceWindow
	{
		public:
							SelectionDialog(const SelectionDialogDesc& desc);
		virtual				~SelectionDialog();

		virtual int			handleEvent(IceGUIEvent* event);

		// Called once if user clicks on "Select".
		virtual		bool	OnSelect()					{ return true;	}
		// If "OnSelect" returns true, called once per selected item.
		virtual		void	OnSelected(void* user_data)	{}

					udword	Add(const char* name, udword group, void* user_data, BOOL selected);
					udword	Update();
					void	Resize();

		protected:

		class SelectionButton : public IceButton
		{
			public:
								SelectionButton(const ButtonDesc& desc) : IceButton(desc), mDlg(null)	{}
			virtual				~SelectionButton()														{}

			virtual	void		OnClick();

			SelectionDialog*	mDlg;
		};

		class SelectionCheckBox : public IceCheckBox
		{
			public:
								SelectionCheckBox(const CheckBoxDesc& desc) : IceCheckBox(desc)	{}
			virtual				~SelectionCheckBox()											{}

			virtual	void		OnClick();

			SelectionDialog*	mDlg;
		};

		class SelectionWindow : public IceListBox
		{
			public:
								SelectionWindow(const ListBoxDesc& desc) : IceListBox(desc), mMain(null)	{}
			virtual				~SelectionWindow()															{}

			virtual	void		OnListboxEvent(ListBoxEvent event);

			SelectionDialog*	mMain;
		};

		SelectionWindow*	mSelectionWindow;
		SelectionButton*	mSelectButton;
		SelectionButton*	mCancelButton;
		SelectionButton*	mSelectAllButton;
		SelectionButton*	mSelectNoneButton;
		SelectionButton*	mSelectInvertButton;
		SelectionButton*	mFilterAllButton;
		SelectionButton*	mFilterNoneButton;
		SelectionButton*	mFilterInvertButton;
		udword				mNbFilters;
		SelectionCheckBox*	mFilters[MAX_NB_FILTERS];
		SelectionCheckBox*	mSortByName;
		SelectionCheckBox*	mUserDefined[MAX_NB_USER_CHECKBOXES];
		IceGroupBox*		mGroupBox;
		Container			mItems;
		udword				mMaxLength;
		bool				mInitialUpdate;

		friend class SelectionButton;
		friend class SelectionWindow;
	};

#endif	// ICESELECTIONDIALOG_H