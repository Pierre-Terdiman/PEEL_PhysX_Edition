///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERADIOBUTTON_H
#define ICERADIOBUTTON_H

	class ICEGUI_API RadioButtonDesc : public WidgetDesc
	{
		public:
							RadioButtonDesc();

				bool		mNewGroup;
	};

	class ICEGUI_API IceRadioButton : public IceWidget
	{
		public:
							IceRadioButton(const RadioButtonDesc& desc);
		virtual				~IceRadioButton();

				void		SetChecked(bool b);

				bool		IsChecked() const;

		private:
				PREVENT_COPY(IceRadioButton);
	};

#endif	// ICERADIOBUTTON_H