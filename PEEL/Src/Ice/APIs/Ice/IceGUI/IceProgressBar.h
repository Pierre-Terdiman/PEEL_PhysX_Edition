///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPROGRESSBAR_H
#define ICEPROGRESSBAR_H

	enum ProgressBarType
	{
		PROGRESSBAR_NORMAL,
		PROGRESSBAR_SMOOTH
	};

	class ICEGUI_API ProgressBarDesc : public WidgetDesc
	{
		public:
									ProgressBarDesc();

				ProgressBarType		mStyle;
	};

	class ICEGUI_API IceProgressBar : public IceWidget
	{
		public:
									IceProgressBar(const ProgressBarDesc& desc);
		virtual						~IceProgressBar();

				void				SetValue(int value);
				void				SetTotalSteps(int steps);

				int					GetValue()		const;
				int					GetTotalSteps()	const;
		private:
				int					mValue;
				int					mSteps;

				PREVENT_COPY(IceProgressBar);
	};

#endif	// ICEPROGRESSBAR_H