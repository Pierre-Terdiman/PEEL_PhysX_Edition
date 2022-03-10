///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICELABEL_H
#define ICELABEL_H

	class ICEGUI_API LabelDesc : public WidgetDesc
	{
		public:
							LabelDesc();
	};

	class ICEGUI_API IceLabel : public IceWidget
	{
		public:
						IceLabel(const LabelDesc& desc);
		virtual			~IceLabel();

		private:
				PREVENT_COPY(IceLabel);
	};

#endif	// ICELABEL_H