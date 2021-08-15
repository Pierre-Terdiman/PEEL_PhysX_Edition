///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RaytracingTest.h"
#include "RaytracingWindow.h"
#include "Pint.h"
#include "PintSQ.h"
#include "Camera.h"
#include "SourceRay.h"
#include "PEEL_Threads.h"

//#define LIGHT_DIR	1.0f, 1.0f, 0.5f
#define LIGHT_DIR	0.0f, 1.0f, 0.0f

static inline_ void SetBackgroundColor(RGBAPixel& pixel)
{
	pixel.R = pixel.G = pixel.B = 51;
	pixel.A = 255;
}

static inline_ void ComputeShading(RGBAPixel& color, const PintRaycastHit& hit,
								   const Point& light_dir, const Point& light_color)
{
	color.R = ubyte((hit.mNormal.x + 1.0f)*0.5f*255.0f);
	color.G = ubyte((hit.mNormal.y + 1.0f)*0.5f*255.0f);
	color.B = ubyte((hit.mNormal.z + 1.0f)*0.5f*255.0f);
	color.A = 255;

	//color.R = ubyte(fabsf(hit.mImpact.x));
	//color.G = ubyte(fabsf(hit.mImpact.y));
	//color.B = ubyte(fabsf(hit.mImpact.z));

	return;	


//	float LDotN = fabsf(light_dir|hit.mNormal);
	float LDotN = light_dir|hit.mNormal;
	if(LDotN<0.0f)
		LDotN = 0.0f;

	float diffuse = 0.3f + 0.7f * LDotN;
	diffuse = TClamp(diffuse, 0.0f, 1.0f);

	color.R = ubyte(light_color.x * diffuse);
	color.G = ubyte(light_color.y * diffuse);
	color.B = ubyte(light_color.z * diffuse);

	color.A = 255;
}

udword RaytracingTest(Picture& pic, Pint& pint, udword& total_time, udword screen_width, udword screen_height, udword nb_rays, float max_dist)
{
	SPY_ZONE("RaytracingTest")

//	max_dist = MAX_FLOAT;
	udword TotalTime = 0;
	udword TotalTime2 = 0;
	udword Nb=0;
	const Point origin = GetCameraPos();

	Point LightColor = pint.GetMainColor();
	LightColor *= 255.0f;

	const udword RAYTRACING_RENDER_WIDTH = nb_rays;
	const udword RAYTRACING_RENDER_HEIGHT = nb_rays;

	ASSERT(pic.GetWidth()==RAYTRACING_RENDER_WIDTH);
	ASSERT(pic.GetHeight()==RAYTRACING_RENDER_HEIGHT);
	RGBAPixel* Pixels = pic.GetPixels();

	const float fScreenWidth = float(screen_width)/float(RAYTRACING_RENDER_WIDTH);
	const float fScreenHeight = float(screen_height)/float(RAYTRACING_RENDER_HEIGHT);

	Point LightDir(LIGHT_DIR);
	LightDir.Normalize();

	if(0)
	{
		FILE* fp = fopen("d:\\tmp\\rays.bin", "wb");
		if(fp)
		{
			const udword TotalNbRays = RAYTRACING_RENDER_HEIGHT * RAYTRACING_RENDER_WIDTH;
			fwrite(&TotalNbRays, sizeof(udword), 1, fp);

			for(udword j=0;j<RAYTRACING_RENDER_HEIGHT;j++)
			{
				const udword yi = udword(fScreenHeight*float(j));
				for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
				{
					const udword xi = udword(fScreenWidth*float(i));

					const Point dir = ComputeWorldRay(xi, yi);

					Source1_Ray_t R;
					R.m_Start.x			= origin.x;
					R.m_Start.y			= origin.y;
					R.m_Start.z			= origin.z;
					R.m_Start.w			= 0.0f;
					R.m_Delta.x			= dir.x * max_dist;
					R.m_Delta.y			= dir.y * max_dist;
					R.m_Delta.z			= dir.z * max_dist;
					R.m_Delta.w			= 0.0f;
					R.m_StartOffset.x	= 0.0f;
					R.m_StartOffset.y	= 0.0f;
					R.m_StartOffset.z	= 0.0f;
					R.m_StartOffset.w	= 0.0f;
					R.m_Extents.x		= 0.0f;
					R.m_Extents.y		= 0.0f;
					R.m_Extents.z		= 0.0f;
					R.m_Extents.w		= 0.0f;
					R.m_pDummy			= null;
					R.m_IsRay			= true;
					R.m_IsSwept			= false;

					fwrite(&R, sizeof(Source1_Ray_t), 1, fp);
				}
			}
			fclose(fp);
		}
	}

	{
		__declspec(align(16))	PintRaycastHit Hits[RAYTRACING_MAX_RENDER_SIZE];
		__declspec(align(16))	PintRaycastData RaycastData[RAYTRACING_MAX_RENDER_SIZE];

		for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
		{
			RaycastData[i].mOrigin = origin;
			RaycastData[i].mMaxDist = max_dist;
		}

//#define COLLECT_BLACK_PIXELS
#ifdef COLLECT_BLACK_PIXELS
		Vertices BlackRays;
#endif
		WorldRayComputer WRC;
		udword Time;
		for(udword j=0;j<RAYTRACING_RENDER_HEIGHT;j++)
		{
			const udword yi = udword(fScreenHeight*float(j));

			{
				StartProfile(Time);
				{
					for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
					{
						const udword xi = udword(fScreenWidth*float(i));
//						RaycastData[i].mDir = ComputeWorldRay(xi, yi);
						RaycastData[i].mDir = WRC.ComputeWorldRay(xi, yi);					
						Hits[i].mTouchedActor = null;
					}
				}
				EndProfile(Time);
				TotalTime2 += Time;
			}

//			_asm	push ebx
			StartProfile(Time);
//			_asm	pop ebx
				pint.BatchRaycasts(pint.mSQHelper->GetThreadContext(), RAYTRACING_RENDER_WIDTH, Hits, RaycastData);
//			_asm	push ebx
			EndProfile(Time);
			TotalTime += Time;
//			_asm	pop ebx

			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				if(Hits[i].mTouchedActor)
				{
					Nb++;
					ComputeShading(*Pixels, Hits[i], LightDir, LightColor);
				}
				else
				{
					SetBackgroundColor(*Pixels);
#ifdef COLLECT_BLACK_PIXELS
					BlackRays.AddVertex(RaycastData[i].mOrigin);
					BlackRays.AddVertex(RaycastData[i].mDir);
#endif
				}
				Pixels++;
			}
		}
#ifdef COLLECT_BLACK_PIXELS
		FILE* fp = fopen("f:\\tmp\\black_rays.bin", "wb");
		if(fp)
		{
			const udword TotalNbRays = BlackRays.GetNbVertices()/2;
			fwrite(&TotalNbRays, sizeof(udword), 1, fp);

			const Point* Data = BlackRays.GetVertices();

			for(udword i=0;i<TotalNbRays;i++)
			{
				Source1_Ray_t R;
				R.m_Start.x			= Data[i*2].x;
				R.m_Start.y			= Data[i*2].y;
				R.m_Start.z			= Data[i*2].z;
				R.m_Start.w			= 0.0f;
				R.m_Delta.x			= Data[i*2+1].x * max_dist;
				R.m_Delta.y			= Data[i*2+1].y * max_dist;
				R.m_Delta.z			= Data[i*2+1].z * max_dist;
				R.m_Delta.w			= 0.0f;
				R.m_StartOffset.x	= 0.0f;
				R.m_StartOffset.y	= 0.0f;
				R.m_StartOffset.z	= 0.0f;
				R.m_StartOffset.w	= 0.0f;
				R.m_Extents.x		= 0.0f;
				R.m_Extents.y		= 0.0f;
				R.m_Extents.z		= 0.0f;
				R.m_Extents.w		= 0.0f;
				R.m_pDummy			= null;
				R.m_IsRay			= true;
				R.m_IsSwept			= false;

				fwrite(&R, sizeof(Source1_Ray_t), 1, fp);
			}
			fclose(fp);
		}
#endif
		total_time = TotalTime;
	}

//	printf("Raygen time: %d\n", TotalTime2/1024);

	return Nb;
}







struct RaytracingParams
{
	Pint*				mPint;
	PintSQThreadContext	mContext;
	RGBAPixel*			mPixels;

	Point				mOrigin;
	Point				mLightDir;
	Point				mLightColor;

	udword				mNbToGo;
	const Point*		mDirs;
	float				mMaxDist;

	udword				RAYTRACING_RENDER_WIDTH;
	udword				RAYTRACING_RENDER_HEIGHT;

	udword				mNb;
	udword				mTotalTime;
};

void ThreadSetup();

static int gRaytracingThread(void* user_data)
{
	SPY_ZONE("gRaytracingThread")

	ThreadSetup();

	RaytracingParams* __restrict Params = (RaytracingParams* __restrict)user_data;

	RGBAPixel* __restrict Dest = Params->mPixels;

	const udword RAYTRACING_RENDER_WIDTH = Params->RAYTRACING_RENDER_WIDTH;
	const udword RAYTRACING_RENDER_HEIGHT = Params->RAYTRACING_RENDER_HEIGHT;

	udword TotalTime = 0;
	udword Nb = 0;

	{
		__declspec(align(16))	PintRaycastHit Hits[RAYTRACING_MAX_RENDER_SIZE];
		__declspec(align(16))	PintRaycastData RaycastData[RAYTRACING_MAX_RENDER_SIZE];

		{
			const Point Origin = Params->mOrigin;
			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				RaycastData[i].mOrigin = Origin;
				RaycastData[i].mMaxDist = Params->mMaxDist;
			}
		}

		udword NbToGo = Params->mNbToGo;
		const Point* __restrict Dirs = Params->mDirs;
		const Point LightDir = Params->mLightDir;
		const Point LightColor = Params->mLightColor;
		while(NbToGo)
		{
			NbToGo -= RAYTRACING_RENDER_WIDTH;
			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				RaycastData[i].mDir = *Dirs++;
				Hits[i].mTouchedActor = null;
			}

//			udword Time;
//			StartProfile(Time);
				Params->mPint->BatchRaycasts(Params->mContext, RAYTRACING_RENDER_WIDTH, Hits, RaycastData);
//			EndProfile(Time);
//			TotalTime += Time;

			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				if(Hits[i].mTouchedActor)
				{
					Nb++;
					ComputeShading(*Dest, Hits[i], LightDir, LightColor);
				}
				else
				{
					SetBackgroundColor(*Dest);
				}
				Dest++;
			}
		}
	}

	Params->mTotalTime = TotalTime;
	Params->mNb = Nb;

	return 1;	// Written as "status" member
}

udword RaytracingTestMT(Picture& pic, Pint& pint, udword& total_time, udword screen_width, udword screen_height, udword nb_rays, float max_dist)
{
	SPY_ZONE("RaytracingTestMT")

	const Point origin = GetCameraPos();

	Point LightColor = pint.GetMainColor();
	LightColor *= 255.0f;

	const udword RAYTRACING_RENDER_WIDTH = nb_rays;
	const udword RAYTRACING_RENDER_HEIGHT = nb_rays;

	ASSERT(pic.GetWidth()==RAYTRACING_RENDER_WIDTH);
	ASSERT(pic.GetHeight()==RAYTRACING_RENDER_HEIGHT);
	RGBAPixel* Pixels = pic.GetPixels();

	const float fScreenWidth = float(screen_width)/float(RAYTRACING_RENDER_WIDTH);
	const float fScreenHeight = float(screen_height)/float(RAYTRACING_RENDER_HEIGHT);

	Point LightDir(LIGHT_DIR);
	LightDir.Normalize();

	const udword NbPixels = RAYTRACING_RENDER_WIDTH*RAYTRACING_RENDER_HEIGHT;

	Point* Dirs = ICE_NEW(Point)[NbPixels];
	{
		WorldRayComputer WRC;
		Point* Dest = Dirs;
		for(udword j=0;j<RAYTRACING_RENDER_HEIGHT;j++)
		{
			const udword yi = udword(fScreenHeight*float(j));
			for(udword i=0;i<RAYTRACING_RENDER_WIDTH;i++)
			{
				const udword xi = udword(fScreenWidth*float(i));
//				const Point dir = ComputeWorldRay(xi, yi);
				const Point dir = WRC.ComputeWorldRay(xi, yi);
				*Dest++ = dir;
			}
		}
	}

#define NB_RT_THREADS	8
	udword Time;
	RaytracingParams Params[NB_RT_THREADS];
	if(0)
	{
		for(udword i=0;i<1;i++)
		{
			Params[i].RAYTRACING_RENDER_WIDTH	= nb_rays;
			Params[i].RAYTRACING_RENDER_HEIGHT	= nb_rays;

			Params[i].mPint			= &pint;
			Params[i].mPixels		= Pixels + i*(NbPixels/NB_RT_THREADS);
			Params[i].mOrigin		= origin,
			Params[i].mLightDir		= LightDir;
			Params[i].mLightColor	= LightColor * (1.0f - float(i)*0.2f);
			Params[i].mNb			= 0;
			Params[i].mTotalTime	= 0;
			Params[i].mNbToGo		= NbPixels/NB_RT_THREADS;
	//Params[i].mNbToGo		= NbPixels;
			Params[i].mDirs			= Dirs + i*(NbPixels/NB_RT_THREADS);
			Params[i].mMaxDist		= max_dist;
		}

		StartProfile(Time);

			IceThread* Thread1 = CreateThread_(gRaytracingThread, &Params[0]);
			WaitThread(Thread1, null);

		EndProfile(Time);
	}
	else
	{
		for(udword i=0;i<NB_RT_THREADS;i++)
		{
			Params[i].RAYTRACING_RENDER_WIDTH	= nb_rays;
			Params[i].RAYTRACING_RENDER_HEIGHT	= nb_rays;

			Params[i].mPint			= &pint;
			Params[i].mContext		= pint.CreateSQThreadContext();
			Params[i].mPixels		= Pixels + i*(NbPixels/NB_RT_THREADS);
			Params[i].mOrigin		= origin,
			Params[i].mLightDir		= LightDir;
			Params[i].mLightColor	= LightColor;// * (1.0 - float(i)*0.2f);
			Params[i].mNb			= 0;
			Params[i].mTotalTime	= 0;
			Params[i].mNbToGo		= NbPixels/NB_RT_THREADS;
	//Params[i].mNbToGo		= NbPixels;
			Params[i].mDirs			= Dirs + i*(NbPixels/NB_RT_THREADS);
			Params[i].mMaxDist		= max_dist;
		}

		StartProfile(Time);
		{
			if(0)
			{
				IceThread* Thread1 = CreateThread_(gRaytracingThread, &Params[0]);
				IceThread* Thread2 = CreateThread_(gRaytracingThread, &Params[1]);
				IceThread* Thread3 = CreateThread_(gRaytracingThread, &Params[2]);
				IceThread* Thread4 = CreateThread_(gRaytracingThread, &Params[3]);
				IceThread* Thread5 = CreateThread_(gRaytracingThread, &Params[4]);
				IceThread* Thread6 = CreateThread_(gRaytracingThread, &Params[5]);
				IceThread* Thread7 = CreateThread_(gRaytracingThread, &Params[6]);
					gRaytracingThread(&Params[7]);
				WaitThread(Thread1, null);
				WaitThread(Thread2, null);
				WaitThread(Thread3, null);
				WaitThread(Thread4, null);
				WaitThread(Thread5, null);
				WaitThread(Thread6, null);
				WaitThread(Thread7, null);
			}
			else
			{
				for(udword i=0;i<NB_RT_THREADS-1;i++)
					PEEL_AddThreadWork(i, gRaytracingThread, &Params[i]);

				PEEL_StartThreadWork(NB_RT_THREADS-1);
					gRaytracingThread(&Params[NB_RT_THREADS-1]);
				PEEL_EndThreadWork(NB_RT_THREADS-1);
			}
		}
		EndProfile(Time);
	}

	DELETEARRAY(Dirs);

	total_time = 0;
	udword Nb = 0;
	for(udword i=0;i<NB_RT_THREADS;i++)
	{
		pint.ReleaseSQThreadContext(Params[i].mContext);

		total_time += Params[i].mTotalTime;
		Nb += Params[i].mNb;
	}
//total_time = T;
total_time = Time;
	return Nb;
}
