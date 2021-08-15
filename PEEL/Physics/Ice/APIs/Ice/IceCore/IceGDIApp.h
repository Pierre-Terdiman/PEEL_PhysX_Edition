///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an ICE GDI application skeleton.
 *	\file		IceGDIApp.h
 *	\author		Pierre Terdiman
 *	\date		June, 28, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEGDIAPP_H
#define ICEGDIAPP_H

#ifndef _WIN64	// This GDI stuff is rather obsolete so don't bother porting it

	struct ICECORE_API ICEGDIAPPCREATE : ICEAPPCREATE
	{
		udword			BitPlanes;
		udword			x,y;
		const char*		Name;
	};

	class ICECORE_API GDIApplication : public IceApp
	{
		public:
										GDIApplication();
		virtual							~GDIApplication();

		override(IceApp)	bool		Init(const ICEAPPCREATE* create);
		override(IceApp)	bool		Run(const ICEAPPCREATE* create);
		override(IceApp)	bool		Close();

		inline_				ubyte*		GetLogicScreen()					{ return mLogic;				}
		inline_				void		ClearScreen()						{ ZeroMemory(mLogic, mNbBytes);	}
		inline_				void		CopyBufferToPhysic(ubyte* buffer)
										{
											IceCore::SetDIBitsToDevice(mScreenDC, 0, 0, mScreenWidth, mScreenHeight, 0, 0, 0, mScreenHeight, buffer, &mBitmapInfos, DIB_RGB_COLORS);
										}
		inline_				void		CopyLogicToPhysic()					{ CopyBufferToPhysic(mLogic);	}

							bool		InitGDI(const char* name, sdword pos_x, sdword pos_y);
							bool		OpenScreen(sdword width, sdword height, sdword bit_planes, sdword pos_x, sdword pos_y, const char* name);
							void		GetEvent(ICE_EVENT& event);
		private:
							BITMAPINFO	mBitmapInfos;
							HDC			mScreenDC;

							sdword		mScreenWidth;
							sdword		mScreenHeight;
							udword		mBitPlanes;
							udword		mNbBytes;
							ubyte*		mLogic;

							HWND		mWinID;
	};

#endif

#endif // ICEGDIAPP_H
