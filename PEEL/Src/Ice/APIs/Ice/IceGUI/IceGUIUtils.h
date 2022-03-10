///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEGUIUTILS_H
#define ICEGUIUTILS_H

	// Bitmap access callback
	typedef void (*BitmapAccess)	(ubyte* pixels, udword width, udword height, udword bit_count, void* user_data);

	FUNCTION ICEGUI_API void WritePixel(ubyte* dest, udword bit_count, ubyte r, ubyte g, ubyte b);
	FUNCTION ICEGUI_API void ReadPixel(const ubyte* dest, udword bit_count, ubyte& r, ubyte& g, ubyte& b);
	FUNCTION ICEGUI_API bool ClearWindow(HWND hwnd, const RGBPixel* color=null, BitmapAccess callback=null, void* user_data=null);
	FUNCTION ICEGUI_API bool CopyPictureToWindow(HWND hwnd, const Picture& pic, bool alpha, BitmapAccess callback=null, void* user_data=null);
	FUNCTION ICEGUI_API bool CopyPaletteToWindow(HWND hwnd, const RGBPalette& palette, BitmapAccess callback=null, void* user_data=null);
	FUNCTION ICEGUI_API bool FillWindowWithGradient(HWND hwnd, const RGBAPixel& start, const RGBAPixel& end, bool vertical, BitmapAccess callback=null, void* user_data=null);
	FUNCTION ICEGUI_API bool FillGradientWindow(HWND hwnd, const RGBAPixel& color, bool vertical, BitmapAccess callback=null, void* user_data=null);

	inline_ void PutPixel(ubyte* pixels, udword x, udword y, udword pixel_size, udword width, udword height, udword bit_count, ubyte r=0, ubyte g=0, ubyte b=0)
	{
//		if(x>=0 && x<width && y>=0 && y<height)
		if(x<width && y<height)
			WritePixel(pixels + (x+y*width)*pixel_size, bit_count, r, g, b);
	}

	inline_ void GetPixel(const ubyte* pixels, udword x, udword y, udword pixel_size, udword width, udword height, udword bit_count, ubyte& r, ubyte& g, ubyte& b)
	{
//		if(x>=0 && x<width && y>=0 && y<height)
		if(x<width && y<height)
			ReadPixel(pixels + (x+y*width)*pixel_size, bit_count, r, g, b);
	}

	FUNCTION ICEGUI_API void DrawWuLine	(ubyte* pixels, udword pixel_size, udword width, udword height, udword bit_count, int X0, int Y0, int X1, int Y1, ubyte r, ubyte g, ubyte b);
	FUNCTION ICEGUI_API void DrawLine	(ubyte* pixels, udword pixel_size, udword width, udword height, udword bit_count, int x1, int y1, int x2, int y2, ubyte r, ubyte g, ubyte b);

#endif // ICEGUIUTILS_H
