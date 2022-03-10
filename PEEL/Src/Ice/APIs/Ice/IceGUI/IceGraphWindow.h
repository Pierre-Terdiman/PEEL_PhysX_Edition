///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEGRAPHWINDOW_H
#define ICEGRAPHWINDOW_H

//#define USE_GRAPH_PIC
#define MAX_NB_KEYS			8

	enum GraphStyle
	{
		GRAPH_ANTIALIASED	= (1<<0),
	};

	class ICEGUI_API GraphDesc : public WindowDesc
	{
		public:
								GraphDesc();

				GraphStyle		mGraphStyle;
				bool			mEnablePopupMenu;
	};

	// Separate class with a private section, to make sure nobody changes the keys without sending a user callback.
	class ICEGUI_API GraphKeys
	{
		public:
										GraphKeys();
		virtual							~GraphKeys();

				bool					InitKeys(udword nb_keys, const Keyframe* keys);

		inline_	udword					GetNbKeys()			const	{ return mNbKeys;			}
		inline_	const Keyframe&			GetKey(udword i)	const	{ return mKeys[i];			}
		inline_	const Keyframe*			GetKeys()			const	{ return mKeys;				}

				bool					CreateKey(const Keyframe& key);
				bool					DeleteKey(udword index);
				void					DeleteAllKeys();
				bool					SetKey(udword index, float x, float value);

		virtual	void					OnModifiedKey()				{}

		private:
				udword					mNbKeys;
				Keyframe				mKeys[MAX_NB_KEYS];
		// Internal methods
				void					SortKeys();
	};

	class ICEGUI_API GraphWindow : public IceWindow, public GraphKeys
	{
		public:
										GraphWindow(const GraphDesc& desc);
		virtual							~GraphWindow();

		virtual	void					redraw ();
		virtual	int						handleEvent(IceGUIEvent* event);

				sdword					ClipX(sdword x)	const;
				sdword					ClipY(sdword y)	const;

				bool					CreateKeyframe(sdword x, sdword y);
				bool					DeleteKeyframe(udword index);
				void					DeleteAllKeyframes();

#ifdef USE_GRAPH_PIC
				Picture					mPic;
		inline_	uword					GetGraphWidth()		const	{ return mPic.GetWidth();	}
		inline_	uword					GetGraphHeight()	const	{ return mPic.GetHeight();	}
#else
				uword					mWidth, mHeight;
				RGBPixel				mBackColor;
		inline_	uword					GetGraphWidth()		const	{ return mWidth;			}
		inline_	uword					GetGraphHeight()	const	{ return mHeight;			}
#endif
		inline_	float					GetDisplayOffsetY()	const	{ return mDisplayOffsetY;	}

//		protected:
				GraphStyle				mGraphStyle;

				sdword					mX, mY;

				float					mDisplayOffsetY;

				udword					mSelected;
				bool					mTrackY;
				bool					mHasMoved;
				bool					mEnablePopupMenu;
	};

#endif	// ICEGRAPHWINDOW_H