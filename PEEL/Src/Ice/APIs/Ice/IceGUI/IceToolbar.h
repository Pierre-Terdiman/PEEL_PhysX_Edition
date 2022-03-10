///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETOOLBAR_H
#define ICETOOLBAR_H

	class ICEGUI_API ToolbarDesc : public WindowDesc
	{
		public:
							ToolbarDesc();
	};

	class ICEGUI_API IceToolbar : public IceWindow
	{
		public:
							IceToolbar(const ToolbarDesc& desc);
		virtual				~IceToolbar();

		private:
				PREVENT_COPY(IceToolbar);
	};

#endif	// ICETOOLBAR_H