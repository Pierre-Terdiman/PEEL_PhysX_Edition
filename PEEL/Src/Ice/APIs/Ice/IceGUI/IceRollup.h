///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEROLLUP_H
#define ICEROLLUP_H

	class ICEGUI_API RollupDesc : public WindowDesc
	{
		public:
									RollupDesc();
	};

	class ICEGUI_API IceRollup : public IceWindow
	{
		public:
									IceRollup(const RollupDesc& desc);
		virtual						~IceRollup();

				bool				Init(IceWindow* contents, const char* label, bool open);

		virtual void				redraw();

		virtual	int					handleEvent(IceGUIEvent* event);

		inline_	IceWindow*			GetContents()		{ return mContents;	}
#ifdef INCLUDE_GROUPS
		inline_	IceGroupBox*		GetGroupOpen()		{ return mOpen;		}
		inline_	IceGroupBox*		GetGroupClose()		{ return mClose;	}
#endif
#ifdef INCLUDE_WIDGETS
		inline_	IceWidget*			GetGroupOpen()		{ return mOpen;		}
		inline_	IceWidget*			GetGroupClose()		{ return mClose;	}
#endif
		private:
				IceWindow*			mContents;
				IceToggleButton*	mButton;
#ifdef INCLUDE_GROUPS
				IceGroupBox*		mOpen;
				IceGroupBox*		mClose;
#endif
#ifdef INCLUDE_WIDGETS
				IceWidget*			mOpen;
				IceWidget*			mClose;
#endif

				uword				mYOffsetStart;
				uword				mYOffsetEnd;
				uword				mXOffsetButton;
				uword				mXOffsetGroup;
	};

#endif	// ICEROLLUP_H