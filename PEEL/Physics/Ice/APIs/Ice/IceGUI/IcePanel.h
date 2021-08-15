///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPANEL_H
#define ICEPANEL_H

	class ICEGUI_API PanelDesc : public WindowDesc
	{
		public:
								PanelDesc();
	};

	class ICEGUI_API IcePanel : public IceWindow
	{
		public:
								IcePanel(const PanelDesc& desc);
		virtual					~IcePanel();

		virtual	void			Refresh();
		virtual int				handleEvent(IceGUIEvent* event);

				bool			AddWidget(IceWidget* widget);
				bool			Update();
		private:
				udword			mPanelFlags;
				udword			mOffsetY;
				PtrContainer	mWidgets;
	};

#endif	// ICEPANEL_H