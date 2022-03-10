///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESLIDER_H
#define ICESLIDER_H

	enum SliderType
	{
		SLIDER_HORIZONTAL,
		SLIDER_VERTICAL
	};

	enum SliderEvent
	{
	};

	class ICEGUI_API SliderDesc : public WidgetDesc
	{
		public:
							SliderDesc();

				SliderType	mStyle;
	};

	class ICEGUI_API IceSlider : public IceWidget
	{
		public:
							IceSlider(const SliderDesc& desc);
		virtual				~IceSlider();

		virtual	void		OnSliderEvent(SliderEvent event)		{}

				void		SetValue(float value);
				void		SetRange(float min, float max, int ticks = 100);
				void		SetSteps(int line, int page);

				float		GetValue()					const;
				float		GetTrackValue(int ivalue)	const;
				float		GetMinValue()				const;
				float		GetMaxValue()				const;
				int			GetLineStep()				const;
				int			GetPageStep()				const;

				void		SetIntValue(int value);
				void		SetIntRange(int min, int max);
				int			GetIntValue()				const;

		private:
				float		mMin;
				float		mMax;
				int			mTicks;

				PREVENT_COPY(IceSlider);
	};

#endif	// ICESLIDER_H