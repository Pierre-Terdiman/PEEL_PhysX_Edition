///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <delayimp.h>

//#ifdef PHYSX_SUPPORT_GPU
//#include "Gpu\PxGpu.h"
//#endif

#define USE_DELAY_LOADING
#ifdef USE_DELAY_LOADING
#pragma comment(lib, "delayimp")

FARPROC WINAPI delayHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
    switch(dliNotify)
	{
        case dliStartProcessing:
        case dliNotePreGetProcAddress:
        case dliFailLoadLib:
        case dliFailGetProc:
        case dliNoteEndProcessing:
            break;

        case dliNotePreLoadLibrary:
			{
			printf("  Remapping %s\n", pdli->szDll);

    		if(strcmp(pdli->szDll, "PhysX_32.dll")==0)
#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXPROFILE_32_5_1_0.dll");
#else
	#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXDEBUG_32_5_1_0.dll");
	#else
				return (FARPROC)::LoadLibraryA("PhysX_32_5_1_0.dll");
	#endif
#endif

    		if(strcmp(pdli->szDll, "PhysXCommon_32.dll")==0)
#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXCommonPROFILE_32_5_1_0.dll");
#else
	#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXCommonDEBUG_32_5_1_0.dll");
	#else
				return (FARPROC)::LoadLibraryA("PhysXCommon_32_5_1_0.dll");
	#endif
#endif

    		if(strcmp(pdli->szDll, "PhysXCooking_32.dll")==0)
#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXCookingPROFILE_32_5_1_0.dll");
#else
	#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXCookingDEBUG_32_5_1_0.dll");
	#else
				return (FARPROC)::LoadLibraryA("PhysXCooking_32_5_1_0.dll");
	#endif
#endif

    		if(strcmp(pdli->szDll, "PhysXFoundation_32.dll")==0)
#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXFoundationPROFILE_32_5_1_0.dll");
#else
	#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXFoundationDEBUG_32_5_1_0.dll");
	#else
				return (FARPROC)::LoadLibraryA("PhysXFoundation_32_5_1_0.dll");
	#endif
#endif

#ifdef PHYSX_SUPPORT_GPU
    		if(strcmp(pdli->szDll, "PhysXGpu_32.dll")==0)
	#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXGpuPROFILE_32_5_1_0.dll");
	#else
		#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXGpuDEBUG_32_5_1_0.dll");
		#else
				return (FARPROC)::LoadLibraryA("PhysXGpu_32_5_1_0.dll");
		#endif
	#endif
#endif

    		if(strcmp(pdli->szDll, "PhysX_64.dll")==0)
#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXPROFILE_64_5_1_0.dll");
#else
	#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXDEBUG_64_5_1_0.dll");
	#else
				return (FARPROC)::LoadLibraryA("PhysX_64_5_1_0.dll");
	#endif
#endif

    		if(strcmp(pdli->szDll, "PhysXCommon_64.dll")==0)
#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXCommonPROFILE_64_5_1_0.dll");
#else
	#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXCommonDEBUG_64_5_1_0.dll");
	#else
				return (FARPROC)::LoadLibraryA("PhysXCommon_64_5_1_0.dll");
	#endif
#endif

    		if(strcmp(pdli->szDll, "PhysXCooking_64.dll")==0)
#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXCookingPROFILE_64_5_1_0.dll");
#else
	#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXCookingDEBUG_64_5_1_0.dll");
	#else
				return (FARPROC)::LoadLibraryA("PhysXCooking_64_5_1_0.dll");
	#endif
#endif

    		if(strcmp(pdli->szDll, "PhysXFoundation_64.dll")==0)
#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXFoundationPROFILE_64_5_1_0.dll");
#else
	#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXFoundationDEBUG_64_5_1_0.dll");
	#else
				return (FARPROC)::LoadLibraryA("PhysXFoundation_64_5_1_0.dll");
	#endif
#endif

#ifdef PHYSX_SUPPORT_GPU
    		if(strcmp(pdli->szDll, "PhysXGpu_64.dll")==0)
	#ifdef PEEL_PROFILE
				return (FARPROC)::LoadLibraryA("PhysXGpuPROFILE_64_5_1_0.dll");
	#else
		#ifdef _DEBUG
				return (FARPROC)::LoadLibraryA("PhysXGpuDEBUG_64_5_1_0.dll");
		#else
				return (FARPROC)::LoadLibraryA("PhysXGpu_64_5_1_0.dll");
		#endif
	#endif
#endif
			}
            break;

        default :
            return NULL;
    }
    return NULL;
}

#ifndef DELAYIMP_INSECURE_WRITABLE_HOOKS
const
#endif
PfnDliHook __pfnDliNotifyHook2 = delayHook;
#endif

#ifdef USE_DELAY_LOADING
#include "common/windows/PxWindowsDelayLoadHook.h"
//#include "pvd/windows/PxWindowsPvdDelayLoadHook.h"

	class MyDelayLoadHook : public PxDelayLoadHook
#ifdef PHYSX_SUPPORT_GPU
		, public PxGpuLoadHook
#endif
	{
	public:
		MyDelayLoadHook()
		{
			PxSetPhysXDelayLoadHook(this);
			PxSetPhysXCookingDelayLoadHook(this);
			PxSetPhysXCommonDelayLoadHook(this);
//			PxPvdSetFoundationDelayLoadHook(this);
#ifdef PHYSX_SUPPORT_GPU
			PxSetPhysXGpuLoadHook(this);
//			PxSetPhysXGpuDelayLoadHook(this);
#endif
		}
		virtual ~MyDelayLoadHook() {}

		/////

		virtual const char* getPhysXCommonDllName() const
		{
#ifndef WIN64
			printf("Remapping PhysXCommon_32.dll\n");
	#ifdef PEEL_PROFILE
			return "PhysXCommonPROFILE_32_5_1_0.dll";
	#else
		#ifdef _DEBUG
			return "PhysXCommonDEBUG_32_5_1_0.dll";
		#else
			return "PhysXCommon_32_5_1_0.dll";
		#endif
	#endif
#else
			printf("Remapping PhysXCommon_64.dll\n");
	#ifdef PEEL_PROFILE
			return "PhysXCommonPROFILE_64_5_1_0.dll";
	#else
		#ifdef _DEBUG
			return "PhysXCommonDEBUG_64_5_1_0.dll";
		#else
			return "PhysXCommon_64_5_1_0.dll";
		#endif
	#endif
#endif
		}

		/////

		virtual const char* getPhysXFoundationDllName() const
		{
#ifndef WIN64
			printf("Remapping PhysXFoundation\n");
	#ifdef PEEL_PROFILE
			return "PhysXFoundationPROFILE_32_5_1_0.dll";
	#else
		#ifdef _DEBUG
			return "PhysXFoundationDEBUG_32_5_1_0.dll";
		#else
			return "PhysXFoundation_32_5_1_0.dll";
		#endif
	#endif
#else
			printf("Remapping PhysXFoundation\n");
	#ifdef PEEL_PROFILE
			return "PhysXFoundationPROFILE_64_5_1_0.dll";
	#else
		#ifdef _DEBUG
			return "PhysXFoundationDEBUG_64_5_1_0.dll";
		#else
			return "PhysXFoundation_64_5_1_0.dll";
		#endif
	#endif
#endif
		}

		/////

/*
		virtual const char* getPxPvdSDKDllName() const
		{
			printf("Remapping PxPvdSDK_32.dll\n");
			return "PxPvdSDK_32_5_1_0.dll";
		}*/

#ifdef PHYSX_SUPPORT_GPU
		virtual const char* getPhysXGpuDllName() const
		{
	#ifndef WIN64
			printf("Remapping PhysXGpu_32.dll\n");
		#ifdef PEEL_PROFILE
			return "PhysXGpuPROFILE_32_5_1_0.dll";
		#else
			#ifdef _DEBUG
			return "PhysXGpuDEBUG_32_5_1_0.dll";
			#else
			return "PhysXGpu_32_5_1_0.dll";
			#endif
		#endif
	#else
			printf("Remapping PhysXGpu_64.dll\n");
		#ifdef PEEL_PROFILE
			return "PhysXGpuPROFILE_64_5_1_0.dll";
		#else
			#ifdef _DEBUG
			return "PhysXGpuDEBUG_64_5_1_0.dll";
			#else
			return "PhysXGpu_64_5_1_0.dll";
			#endif
		#endif
	#endif
		}
#endif

	}gMyDelayHook;
#endif