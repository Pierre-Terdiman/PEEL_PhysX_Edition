///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains mouse-related code.
 *	\file		IceMouse.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEMOUSE_H
#define ICEMOUSE_H

	enum MouseButton
	{
		MBT_LEFT	= 0,
		MBT_MIDDLE	= 1,
		MBT_RIGHT	= 2
	};

	enum MouseFlag
	{
		MOUSE_LEFT_DOWN			= (1<<0),
		MOUSE_MIDDLE_DOWN		= (1<<1),
		MOUSE_RIGHT_DOWN		= (1<<2),
		MOUSE_LEFT_UP			= (1<<3),
		MOUSE_MIDDLE_UP			= (1<<4),
		MOUSE_RIGHT_UP			= (1<<5),
		MOUSE_LEFT_DRAG			= (1<<6),
		MOUSE_MIDDLE_DRAG		= (1<<7),
		MOUSE_RIGHT_DRAG		= (1<<8),
		MOUSE_LEFT_DBL_CLK		= (1<<9),
		MOUSE_MIDDLE_DBL_CLK	= (1<<10),
		MOUSE_RIGHT_DBL_CLK		= (1<<11),
		MOUSE_MOTION			= (1<<12),	// MOUSE_MOVE was already defined in MAX
		MOUSE_WHEEL				= (1<<13),

		MOUSE_ALL_FLAGS			= 0xffffffff
	};

	class ICECORE_API MouseInfo
	{
		public:
								MouseInfo();
								~MouseInfo();
		// Helpers
				bool			ValidateAction(ubyte mbid, bool in_place)	const;

		// Mouse data
				union
				{
					struct
					{
						bool	mLeftMouseButton;
						bool	mMiddleMouseButton;
						bool	mRightMouseButton;
					};
					bool		mMouseButton[3];
				};
				sdword			mMouseX;		//!< Absolute x position
				sdword			mMouseY;		//!< Absolute y position
				sdword			mOldMouseX;		//!< Previous absolute x position
				sdword			mOldMouseY;		//!< Previous absolute y position
				sdword			mRelMouseX;		//!< Delta x
				sdword			mRelMouseY;		//!< Delta y
				sword			mWheelValue;	//!< Wheel increment/decrement (1,2,3... or -1,-2,-3....)
				bool			mMouseMove;		//!< Motion flag
		private:
		mutable	bool			mMBDown[3];
		mutable	sdword			mMX[3], mMY[3];
	};

	class ICECORE_API MouseFilter
	{
		public:
								MouseFilter();
								MouseFilter(udword length, float weight_modifier);
								~MouseFilter();

		inline_		float		GetWeightModifier()						const	{ return mWeightModifier;		}
		inline_		void		SetWeightModifier(float modifier)				{ mWeightModifier = modifier;	}

		inline_		udword		GetHistoryBufferLength()				const	{ return mHistoryBufferLength;	}
					bool		SetHistoryBufferLength(udword length);

					void		Apply(float& delta_mouse_x, float& delta_mouse_y);
		private:
					float		mWeightModifier;
					udword		mHistoryBufferLength;
					float*		mHistoryBufferX;
					float*		mHistoryBufferY;
	};

	// Cursor control
	FUNCTION ICECORE_API void	HideCursor();
	FUNCTION ICECORE_API void	DisplayCursor();
	FUNCTION ICECORE_API void	HideCursorIfNeeded();
	FUNCTION ICECORE_API void	RepositionMouse(HWND handle, udword render_width, udword render_height, sdword& delta_mouse_x, sdword& delta_mouse_y);

	struct ICECORE_API ScreenPoint
	{
		inline_ void Reset()	{ x = y = 0; }
		sdword	x;
		sdword	y;
	};

	struct ICECORE_API ScreenRect
	{
		void Reset()
		{
			mLeft	= 0;
			mTop	= 0;
			mRight	= 0;
			mBottom	= 0;
		}

		bool IsEmpty()	const
		{
			return ((mLeft==mRight) || (mBottom==mTop));
		}

		void Reorganize()
		{
			if(mLeft>mRight)	TSwap(mLeft, mRight);
			if(mTop>mBottom)	TSwap(mTop, mBottom);
		}

		sdword	mLeft;
		sdword	mTop;
		sdword	mRight;
		sdword	mBottom;
	};

	// Window stuff
	FUNCTION ICECORE_API void	GetWindowCenter(HWND handle, udword render_width, udword render_height, ScreenPoint& pt);

#endif // ICEMOUSE_H