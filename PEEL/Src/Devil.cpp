///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Devil.h"

#ifdef DOES_NOT_WORK

..\..\..\PEEL_Externals\DevIL_1.8.0\include
..\..\..\PEEL_Externals\DevIL_1.8.0\lib\x86\Release

#ifndef _WIN64
	#define COMPILE_DEVIL
#endif

#ifdef COMPILE_DEVIL
	// DevIL wrapper copied from '\APPs\#Plugins\SystemPlugs\IceDevil.cpp' (ICE). Probably old and obsolete but it still works.
//	#include ".\DevIL\include\il\il_wrap.h"

	typedef void	ILvoid;
	#undef _UNICODE
//	#define IL_STATIC_LIB
//	#include <IL\il_wrap.h>
	#include <IL/ilut.h>
	#pragma comment(lib, "ilu.lib")
	#pragma comment(lib, "ilut.lib")
#endif

#ifdef COMPILE_DEVIL
static bool InitDone = false;
#endif
bool LoadWithDevil(const char* filename, Picture& pic)
{
#ifdef COMPILE_DEVIL
	if(!InitDone)
	{
		InitDone = true;
		ilInit();
	}

//	ilImage DevilImage;
//	ILboolean b = DevilImage.Load((char*)filename);
//	if(!b)	return false;

		ILuint ImageName;
		ilGenImages(1, &ImageName);
		ilBindImage(ImageName);
		if(!ilLoadImage((char*)filename))
		{
			return false;
		}


//	ILuint W = DevilImage.Width();
//	ILuint H = DevilImage.Height();
//	ILuint BPP = DevilImage.Bitpp();
//	ILubyte* Data = DevilImage.GetData();

		ILuint W = ilGetInteger(IL_IMAGE_WIDTH);
		ILuint H = ilGetInteger(IL_IMAGE_HEIGHT); 
		ILuint BPP = ilGetInteger(IL_IMAGE_BPP); 
		ILenum F = ilGetInteger(IL_IMAGE_FORMAT); 
		ILubyte* Data = ilGetData(); 

	if(F!=IL_RGB && F!=IL_RGBA && F!=IL_BGR && F!=IL_BGRA)
	{
		IceCore::MessageBox(null, "Unsupported format!\nImage will not be loaded.", "Error", MB_OK);
		return false;
	}

	pic.Init(ToWord(W), ToWord(H));
	RGBAPixel* Pixels = pic.GetPixels();
	for(udword y=0;y<H;y++)
	{
		for(udword x=0;x<W;x++)
		{
			if(F==IL_RGB)
			{
				Pixels->R = *Data++;
				Pixels->G = *Data++;
				Pixels->B = *Data++;
				Pixels->A = 0;
			}
			else if(F==IL_RGBA)
			{
				Pixels->R = *Data++;
				Pixels->G = *Data++;
				Pixels->B = *Data++;
				Pixels->A = *Data++;
			}
			else if(F==IL_BGR)
			{
				Pixels->B = *Data++;
				Pixels->G = *Data++;
				Pixels->R = *Data++;
				Pixels->A = 0;
			}
			else if(F==IL_BGRA)
			{
				Pixels->B = *Data++;
				Pixels->G = *Data++;
				Pixels->R = *Data++;
				Pixels->A = *Data++;
			}
			Pixels++;
		}
	}
//	pic.FlipVertical();

	ilDeleteImages(1, &ImageName);
#endif
	return true;
}
#endif

#define USE_DEVIL_1_8_0

typedef void	ILvoid;
#undef _UNICODE
#ifdef USE_DEVIL_1_8_0
	#include "DevIL_1.8.0/include/IL/il.h"
#else
	#include ".\il\il.h"
#endif

static void			(ILAPIENTRY* gFunc_ilInit)			()								= null;
//static ILuint		(ILAPIENTRY* gFunc_ilGenImage)		()								= null;
static void			(ILAPIENTRY* gFunc_ilGenImages)		(ILsizei Num, ILuint* Images)	= null;
static void			(ILAPIENTRY* gFunc_ilBindImage)		(ILuint Image)					= null;
#ifdef USE_DEVIL_1_8_0
static ILboolean	(ILAPIENTRY* gFunc_ilLoadImage)		(ILconst_string FileName)		= null;
#else
static ILboolean	(ILAPIENTRY* gFunc_ilLoadImage)		(const ILstring FileName)		= null;
#endif
static ILubyte*		(ILAPIENTRY* gFunc_ilGetData)		()								= null;
static ILint		(ILAPIENTRY* gFunc_ilGetInteger)	(ILenum Mode)					= null;
//static void			(ILAPIENTRY* gFunc_ilDeleteImage)	(const ILuint Num)				= null;
static ILvoid		(ILAPIENTRY* gFunc_ilDeleteImages)	(ILsizei Num, const ILuint* Images)	= null;

static ILboolean	(ILAPIENTRY* gFunc_ilSetData)		(void* Data)					= null;
static void			(ILAPIENTRY* gFunc_ilSetInteger)	(ILenum Mode, ILint Param)		= null;
static ILboolean	(ILAPIENTRY* gFunc_ilSaveImage)		(ILconst_string FileName)		= null;
static ILboolean	(ILAPIENTRY* gFunc_ilTexImage)		(
  ILuint Width,
  ILuint Height,
  ILuint Depth,
  ILubyte Bpp,
  ILenum Format,
  ILenum Type,
  ILvoid *Data	
	)		= null;

static bool			gOK = false;
static bool			gFirstCall = true;

static	LIBRARY	gHandle = null;

bool InitDevil()
{
	gOK = false;

	if(!IceCore::LoadLibrary_("DevIL.dll", gHandle, false))
//	if(!IceCore::LoadLibrary_("C:/Projects/#PEEL/PEEL_Externals/DevIL_1.8.0/lib/x86/Release/DevIL.dll", gHandle, false))
		return false;

	*(void**)&gFunc_ilInit = IceCore::BindSymbol(gHandle, "ilInit");
	if(!gFunc_ilInit)
		return false;

//	*(void**)&gFunc_ilGenImage = IceCore::BindSymbol(gHandle, "ilGenImage");
//	if(!gFunc_ilGenImage)
//		return false;

	*(void**)&gFunc_ilGenImages = IceCore::BindSymbol(gHandle, "ilGenImages");
	if(!gFunc_ilGenImages)
		return false;

	*(void**)&gFunc_ilBindImage = IceCore::BindSymbol(gHandle, "ilBindImage");
	if(!gFunc_ilBindImage)
		return false;

	*(void**)&gFunc_ilLoadImage = IceCore::BindSymbol(gHandle, "ilLoadImage");
	if(!gFunc_ilLoadImage)
		return false;

	*(void**)&gFunc_ilGetData = IceCore::BindSymbol(gHandle, "ilGetData");
	if(!gFunc_ilGetData)
		return false;

	*(void**)&gFunc_ilGetInteger = IceCore::BindSymbol(gHandle, "ilGetInteger");
	if(!gFunc_ilGetInteger)
		return false;

//	*(void**)&gFunc_ilDeleteImage = IceCore::BindSymbol(gHandle, "ilDeleteImage");
//	if(!gFunc_ilDeleteImage)
//		return false;

	*(void**)&gFunc_ilDeleteImages = IceCore::BindSymbol(gHandle, "ilDeleteImages");
	if(!gFunc_ilDeleteImages)
		return false;

	*(void**)&gFunc_ilSetData = IceCore::BindSymbol(gHandle, "ilSetData");
	if(!gFunc_ilSetData)
		return false;

	*(void**)&gFunc_ilSetInteger = IceCore::BindSymbol(gHandle, "ilSetInteger");
	if(!gFunc_ilSetInteger)
		return false;

	*(void**)&gFunc_ilSaveImage = IceCore::BindSymbol(gHandle, "ilSaveImage");
	if(!gFunc_ilSaveImage)
		return false;

	*(void**)&gFunc_ilTexImage = IceCore::BindSymbol(gHandle, "ilTexImage");
	if(!gFunc_ilTexImage)
		return false;

	(gFunc_ilInit)();

	gOK = true;

	return true;
}

bool CloseDevil()
{
	gOK						= false;
	gFunc_ilInit			= null;
//	gFunc_ilGenImage		= null;
	gFunc_ilGenImages		= null;
	gFunc_ilBindImage		= null;
	gFunc_ilLoadImage		= null;
	gFunc_ilGetData			= null;
	gFunc_ilGetInteger		= null;
//	gFunc_ilDeleteImage		= null;
	gFunc_ilDeleteImages	= null;
	gFunc_ilSetData			= null;
	gFunc_ilSetInteger		= null;
	gFunc_ilSaveImage		= null;
	gFunc_ilTexImage		= null;

	return IceCore::UnloadLibrary(gHandle);
}

bool LoadWithDevil(const char* filename, Picture& pic)
{
	if(gFirstCall)
	{
		gFirstCall = false;
		if(!InitDevil())
			printf("WARNING: DevIL failed to initialize.\n");
	}

	if(!gOK)
		return false;

	//const ILuint ImageName = (gFunc_ilGenImage)();
	ILuint ImageName;
	(gFunc_ilGenImages)(1, &ImageName);
	(gFunc_ilBindImage)(ImageName);
	if(!(gFunc_ilLoadImage)((char*)filename))
		return false;

	const ILuint W = (gFunc_ilGetInteger)(IL_IMAGE_WIDTH);
	const ILuint H = (gFunc_ilGetInteger)(IL_IMAGE_HEIGHT); 
//	const ILuint D = (gFunc_ilGetInteger)(IL_IMAGE_DEPTH); 
//	const ILuint T = (gFunc_ilGetInteger)(IL_IMAGE_TYPE); 
//	const ILuint BPP = (gFunc_ilGetInteger)(IL_IMAGE_BPP); 
	const ILenum F = (gFunc_ilGetInteger)(IL_IMAGE_FORMAT); 
	const ILubyte* Data = (gFunc_ilGetData)(); 

	if(F!=IL_RGB && F!=IL_RGBA && F!=IL_BGR && F!=IL_BGRA)
	{
		IceCore::MessageBox(null, "Unsupported format!\nImage will not be loaded.", "Error", MB_OK);
		return false;
	}

	pic.Init(ToWord(W), ToWord(H));
	RGBAPixel* Pixels = pic.GetPixels();
	if(F==IL_RGBA)
	{
		CopyMemory(Pixels, Data, H*W*sizeof(RGBAPixel));
	}
	else
	{
		for(udword y=0;y<H;y++)
		{
			for(udword x=0;x<W;x++)
			{
				if(F==IL_RGB)
				{
					Pixels->R = *Data++;
					Pixels->G = *Data++;
					Pixels->B = *Data++;
					Pixels->A = 0;
				}
				/*else if(F==IL_RGBA)
				{
					Pixels->R = *Data++;
					Pixels->G = *Data++;
					Pixels->B = *Data++;
					Pixels->A = *Data++;
				}*/
				else if(F==IL_BGR)
				{
					Pixels->B = *Data++;
					Pixels->G = *Data++;
					Pixels->R = *Data++;
					Pixels->A = 0;
				}
				else if(F==IL_BGRA)
				{
					Pixels->B = *Data++;
					Pixels->G = *Data++;
					Pixels->R = *Data++;
					Pixels->A = *Data++;
				}
				Pixels++;
			}
		}
	}
//	(gFunc_ilDeleteImage)(ImageName);
	(gFunc_ilDeleteImages)(1, &ImageName);
	return true;
}

bool SaveWithDevil(const char* filename, const Picture& pic)
{
	if(gFirstCall)
	{
		gFirstCall = false;
		if(!InitDevil())
			printf("WARNING: DevIL failed to initialize.\n");
	}

	if(!gOK)
		return false;

	//const ILuint ImageName = (gFunc_ilGenImage)();
	ILuint ImageName;
	(gFunc_ilGenImages)(1, &ImageName);
	(gFunc_ilBindImage)(ImageName);

/*	(gFunc_ilSetInteger)(IL_IMAGE_WIDTH, pic.GetWidth());
	(gFunc_ilSetInteger)(IL_IMAGE_HEIGHT, pic.GetHeight()); 
	(gFunc_ilSetInteger)(IL_IMAGE_BPP, 8); 
	(gFunc_ilSetInteger)(IL_IMAGE_FORMAT, IL_RGBA); 
	(gFunc_ilSetData)(pic.GetPixels()); */
	(gFunc_ilTexImage)(pic.GetWidth(), pic.GetHeight(), 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pic.GetPixels());

	if(!(gFunc_ilSaveImage)((char*)filename))
		return false;

	(gFunc_ilDeleteImages)(1, &ImageName);
	return true;
}

