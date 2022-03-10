///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEWIDGET_H
#define ICEWIDGET_H

	enum WidgetType
	{
		WIDGET_UNDEFINED,
		WIDGET_BUTTON,
		WIDGET_CHECKBOX,
		WIDGET_COMBOBOX,
		WIDGET_GROUPBOX,
		WIDGET_LABEL,
		WIDGET_EDITBOX,
		WIDGET_LISTBOX,
		WIDGET_MENU,
		WIDGET_MENUBAR,
		WIDGET_POPUPMENU,
		WIDGET_PROGRESSBAR,
		WIDGET_RADIOBUTTON,
		WIDGET_SLIDER,
		WIDGET_SCROLLBAR,
		WIDGET_TABCONTROL,
		WIDGET_TOGGLEBUTTON,
		WIDGET_TREEVIEW,
		WIDGET_WINDOW,
		WIDGET_LISTVIEW,
		WIDGET_ROLLUP,
		WIDGET_PANEL,
		WIDGET_TOOLBAR,
	};

	class IceWidget;
	class IceGUIEvent;
	class IceBitmap;

	class ICEGUI_API IceGUIElement : public Allocateable
	{
		public:
								IceGUIElement();
		virtual					~IceGUIElement();

		inline_	HWND			GetHandle()					const	{ return mHWnd;				}
		inline_	WidgetType		GetType()					const	{ return mType;				}
		inline_	IceWidget*		GetParent()					const	{ return mParent;			}

		inline_	void*			GetUserData()				const	{ return mUserData;			}
		inline_	void			SetUserData(void* user_data)		{ mUserData = user_data;	}

		virtual	bool			CanClose()							{ return true;				}
		virtual	void			OnDelete()							{							}

		protected:
				HWND			mHWnd;
				WidgetType		mType;
				IceWidget*		mParent;
				void*			mUserData;

		inline_	void			SetHandle(HWND handle)				{ mHWnd = handle;			}
		inline_	void			SetType(WidgetType type)			{ mType = type;				}
		inline_	void			SetParent(IceWidget* parent)		{ mParent = parent;			}

				PREVENT_COPY(IceGUIElement);
	};

	enum WidgetFlag
	{
		WF_MOVABLE_CLIENT_AREA		= (1<<0),
		WF_SET_FOCUS_ON_ACTIVATE	= (1<<1),
		WF_STYLE_BORDER				= (1<<2),	// Style uses WS_BORDER // Only for labels / buttons / checkboxes so far
	};

	class ICEGUI_API WidgetDesc : public Allocateable
	{
		public:
								WidgetDesc();

				void			Center();
				void			LocateAtCursorPos();

				IceWidget*		mParent;
				udword			mFlags;			//!< Combination of WidgetFlag
				sdword			mX;
				sdword			mY;
				sdword			mWidth;
				sdword			mHeight;
				udword			mID;
				const char*		mLabel;
				void*			mUserData;
				bool			mVisible;
				bool			mEnabled;
	};

	class ICEGUI_API IceWidget : public IceGUIElement
	{
		public:
								IceWidget(const WidgetDesc& desc);
		virtual					~IceWidget();

		// Called just before deletion during shutdown
		// Closing of application aborted if any control returns false
		virtual	void			OnDelete();
		virtual	void			Refresh();
		virtual int				handleEvent(IceGUIEvent* event)		{ return 0;		}
		virtual void			redraw()							{				}
		virtual bool			PaintBackground()					{ return true;	}
		virtual	void			MenuEvent(udword menu_item_id)		{				}

				void			SetNewParent(IceWidget* parent);

				void			SetBounds(sdword x, sdword y, sdword width, sdword height);
				void			SetLabel(const char* label);

		virtual	void			SetVisible(bool b);
				void			SetPosition(sdword x, sdword y);
				void			SetSize(sdword width, sdword height);
				void			SetEnabled(bool b);
				void			SetID(udword id);

				void			SetFocus();

				bool			AddToolTip(const char* text);

//bool			SetImageGeneric(const IceBitmap& bitmap);
//const IceBitmap*	mBitmap;

		inline_	udword			GetFlags()					const	{ return mFlags;	}

				int				x()							const;
				int				y()							const;
				int				w()							const;
				int				h()							const;
				int				w2()						const;
				int				h2()						const;
				void			GetOffset(POINT& offset)	const;
				const char*		GetLabel()					const;
				bool			IsVisible()					const;
				bool			IsEnabled()					const;
				udword			GetID()						const;

		protected:
				udword			mFlags;
		// Internal methods
				void			PostInitWidget(const WidgetDesc& desc);
		private:
//			    HBRUSH			mBackgroundBrush;

				PREVENT_COPY(IceWidget);
	};

#endif	// ICEWIDGET_H