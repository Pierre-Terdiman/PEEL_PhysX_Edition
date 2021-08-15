///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains processor detection code. Comes from Coder Corner.
 *	\file		IceCPUID.h
 *	\author		Pierre Terdiman, original code by Rohan Coelho and Maher Hawash
 *	\date		August, 15, 1998
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECPUID_H
#define ICECPUID_H

#ifndef _WIN64
	struct CPUIDINFO{
		udword		cbSize;
		udword		dwCpuId;
		bool		fGenuineIntel;
		ubyte		UCache;			// Unified Cache Model
		ubyte		DCache;			// Data Cache Model
		ubyte		ICache;			// Instruction Cache Model
		ubyte		DTLB;			// DataTLB Model
		ubyte		ITLB;			// Instrction TLB Model
	};

	#pragma pack(1)

	// For EAX = 0
	struct CPUID0{
		udword		nMaxLevel;
		ubyte		OemString[12];
	};

	// For EAX = 1
	/*
	Bit		Name	Description When Flag = 1 Comments
	0		FPU		Floating-point unit on-chip The processor contains an FPU that supports 
					the Intel 387 floating-point instruction set.
	1		VME		Virtual Mode Extension The processor supports extensions to virtual-8086 mode.
	2		DE		Debugging Extension The processor supports I/O breakpoints, includ-ing 
					the CR4.DE bit for enabling debug exten-sions and optional trapping of 
					access to the DR4 and DR5 registers.
	3		PSE		Page Size Extension The processor supports 4-Mbyte pages.
	4		TSC		Time Stamp Counter The RDTSC instruction is supported including the 
					CR4.TSD bit for access/privilege control.
	5		MSR		Model Specific Registers Model Specific Registers are implemented with 
					the RDMSR, WRMSR instructions.
	6		PAE		Physical Address Extension Physical addresses greater than 32 bits are supported.
	7		MCE		Machine Check Exception Machine Check Exception, Exception 18, and the CR4.MCE 
					enable bit are supported.
	8		CX8		CMPXCHG8 Instruction Supported The compare and exchange 8 bytes instruction
					is supported.
	9		APIC	Local APIC Supported The processor contains a local APIC.
	10-11	Reserved (see note)1
	12		MTRR	Memory Type Range Registers The Processor supports the Memory Type Range 
					Registers specifically the MTRR_CAP register.
	13		PGE		Page Global Enable The global bit in the PDE’s and PTE’s and the CR4.PGE 
					enable bit are supported.
	14		MCA		Machine Check Architecture The Machine Check Architecture is supported, 
					specifically the MCG_CAP register.
	15		CMOV	Conditional Move Instruction Supported The processor supports CMOVcc, and if 
					the FPU feature flag (bit 0) is also set, supports the FCMOVcc and FCOMI instructions.
	16-31	Reserved (see note)1
	*/	

	struct CPUID1{
		// Signature Register (EAX)
		udword		SigStepping:4;
		udword		SigModel:4;
		udword		SigFamily:4;
		udword		SigType:2;
		udword		SigReserved:16;

		// Reserved (EBX, ECX)
		udword		IntelReserved1;
		udword		IntelReserved2;

		// Feature Register (EDX)
		udword		fFpu:1;
		udword		fVirtualMode:1;
		udword		fDebug:1;
		udword		fPageSize:1;
		udword		fTSCounter:1;
		udword		fModelRegister:1;	
		udword		fPhysicalAddress:1;
		udword		fMachineCheckException:1;
		udword		fCmpXchg8:1;
		udword		fAPIC:1;
		udword		fReserved:2;
		udword		fMemoryTypeReg:1;
		udword		fPageGlobalEnable:1;
		udword		fMachineCheck:1;
		udword		fCMOV:1;
		udword		fReserved2:7;
		udword		fMMX:1;
		udword		fReserved3:8;
	};

	// For EAX = 2
	struct CPUID2{
		ubyte		nMaxCacheLevel;
		ubyte		bCacheInfo[15];
	};

	struct CPUIDDATA : public CPUID0, public CPUID1
	{
//		CPUID2		id2[];
	};

	#pragma pack()

	// Constant Values..

	// Processor Signature Types.. ->SigType
	enum {
		TYPE_OEMORIGINAL=0,
		TYPE_OVERDRIVE,
		TYPE_DUALPROCESSOR,
		TYPE_RESERVED
	};

	// Processor Signature Family..  ->SigFamily
	enum {
		CPU_I386 = 3,
		CPU_I486,
		CPU_PENTIUM,
		CPU_PENTIUMPRO
	};

	// Cache Descriptor Informaton..
	enum {
		CACHE_NULL=	0,
		ITLB_4KPAGE_4WAY_64ENTRIES,
		ITLB_4MPAGE_4WAY_4ENTRIES,
		DTLB_4KPAGE_4WAY_64ENTRIES,
		DTLB_4MPAGE_4WAY_8ENTRIES,

		ICACHE_8K_4WAY_32BYTELINE=6,

		DCACHE_8K_2WAY_32BYTELINE=0x0a,

		UCACHE_128K_4WAY_32BYTELINE=0x41,
		UCACHE_256K_4WAY_32BYTELINE,
		UCACHE_512K_4WAY_32BYTELINE
	};

	ICECORE_API bool	DetectProcessor();
	ICECORE_API bool	CpuIdSupported();
	ICECORE_API udword	GetCpuId(CPUIDINFO* id);
	ICECORE_API udword	GetProcessorType();
	ICECORE_API bool	HasFPU();
	ICECORE_API bool	HasRDTSC();
	ICECORE_API bool	HasCMOV();
	ICECORE_API bool	HasFCOMI();
	ICECORE_API bool	HasMMX();
#endif

#endif // ICECPUID_H
