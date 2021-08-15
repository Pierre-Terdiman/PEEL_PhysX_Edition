///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEBITMAP_H
#define ICEBITMAP_H

	class ICEGUI_API IceBitmap : public Allocateable
	{
		public:
									IceBitmap();
									~IceBitmap();

				int					CreateEmptyBitmap(HDC hdc, sdword width, sdword height, sdword depth, RGBQUAD* palette=null);
				bool				ClearColor(HDC hdc, ubyte r, ubyte g, ubyte b);

				void				DeleteBitmap();
//				int					SetFromBitmap(const Picture& Bmp, HDC hDC = NULL);
//				void				setFromImage(const Aztec::MImagePtr &newImage, HDC hDC);

				int					Draw(HDC hDC, sdword x, sdword y, sdword w=-1, sdword h=-1);
				int					Draw(HDC hDC, sdword destx, sdword desty, sdword srcx, sdword srcy, sdword w, sdword h);

	inline_		HBITMAP				GetHandle()		const	{ return mhBitmap;	}

		private:
				BITMAPINFOHEADER	mBitmapInfoHeader;
				BYTE*				mCompleteHeader;
				RGBQUAD*			mPalette;
				BYTE*				mPixels;
				HBITMAP				mhBitmap;

				sdword				mWidth;
				sdword				mHeight;
				sdword				mDepth;
				sdword				mDataWidth;

				PREVENT_COPY(IceBitmap);
	};

#endif	// ICEBITMAP_H
