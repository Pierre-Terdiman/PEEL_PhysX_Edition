///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains source code for card detection.
 *	\file		IceDetectCard.h
 *	\author		Pierre Terdiman
 *	\date		July, 07, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEDETECTCARD_H
#define ICEDETECTCARD_H

	//! List of IHV codes
	enum IHVCode
	{
		IHV_UNKNOWN						= 0,
		IHV_ATI_RAGEPRO,
		IHV_ATI_RAGE128PRO_GL,
		IHV_ATI_RAGE128PRO_VR,
		IHV_ATI_RAGE128PRO_ULTRA,
		IHV_ATI_RAGE128_GL,
		IHV_ATI_RAGE128_VR,
		IHV_ATI_RAGE128_4X,
		IHV_ATI_RAGEII,
		IHV_ATI_RAGEII_PLUS,
		IHV_ATI_RAGEIIC_PCI,
		IHV_ATI_RAGEIIC_AGP,
		IHV_ATI_RAGEXC,
		IHV_ATI_RAGEXL,
		IHV_ATI_RAGELT,
		IHV_ATI_RAGELTPRO,
		IHV_ATI_RAGE_MOBILITY,
		IHV_ATI_RAGE128_MOBILITYM3,
		IHV_ATI_RAGE128_MOBILITYM4,
		IHV_ATI_RADEON,
		IHV_ATI_RADEON_VE,
		IHV_ATI_RADEON_MOBILITY_M6,
		IHV_ATI_RADEON_MOBILITY_7500,
		IHV_ATI_RADEON_7500,
		IHV_ATI_RADEON_8500,
		IHV_3DFX_VOODOO,
		IHV_3DFX_VOODOO2,
		IHV_3DFX_VOODOOBANSHEE,
		IHV_3DFX_VOODOO3,
		IHV_3DFX_VOODOO_5500_AGP,
		IHV_3DFX_VOODOORUSH,			// Alliance AT25/AT3D based reference board
		IHV_3DFX_VOODOORUSHM,			// Macronix based reference board
		IHV_INTEL_i740,
		IHV_INTEL_i810,
		IHV_INTEL_i810E,
		IHV_INTEL_i815,
		IHV_MATROX_MILLENIUM,			// STORM
		IHV_MATROX_MYSTIQUE,			// HURRICANE
		IHV_MATROX_MILLENIUM_II_PCI,	// MISTRAL_PCI
		IHV_MATROX_MILLENIUM_II_AGP,	// MISTRAL_AGP
		IHV_MATROX_G100_PCI,			// TWISTER PCI
		IHV_MATROX_G100_AGP,			// TWISTER AGP
		IHV_MATROX_G200_PCI,			// ECLIPSE PCI
		IHV_MATROX_G200_AGP,			// ECLIPSE AGP
		IHV_MATROX_G400_G450,			// TOUCAN
		IHV_MATROX_G550,
		IHV_NVIDIA_RIVATNT,
		IHV_NVIDIA_RIVATNT2ULTRA,
		IHV_NVIDIA_RIVATNT2VANTA,
		IHV_NVIDIA_RIVATNT2M64,
		IHV_NVIDIA_RIVATNT2,
		IHV_NVIDIA_GEFORCE256,
		IHV_NVIDIA_GEFORCE2_GTS,		// ELSA GLADIAC NVIDIA GEFORCE2 GTS REV A
		IHV_NVIDIA_GEFORCE2_MX,			// 3D Prophet
		IHV_NVIDIA_RIVA128,
		IHV_NVIDIA_NV1,
		IHV_NVIDIA_TNT2_ALADDIN,
		IHV_NVIDIA_GEFORCEDDR,			// NV10
		IHV_NVIDIA_QUADRO,				// NV10
		IHV_NVIDIA_GEFORCE2_ULTRA,
		IHV_NVIDIA_QUADRO2_PRO,
		IHV_NVIDIA_GEFORCE3,
		IHV_NVIDIA_QUADRO4_XGL_900,
		IHV_HERCULES_GEFORCE3_TI500,	// Hercules 3D prophet Titanium 500
		IHV_GUILLEMOT_TNTVANTA,			// Maxi Gamer Phoenix 2
		IHV_GUILLEMOT_TNT2,				// Maxi Gamer Xentor
		IHV_GUILLEMOT_TNT2ULTRA,		// Maxi Gamer Xentor 32
		IHV_GUILLEMOT_TNT2M64,			// Maxi Gamer Cougar
		IHV_GUILLEMOT_TNT2M64V,			// Maxi Gamer Gougar Video Edition
		IHV_GUILLEMOT_GEFORCE256,		// Maxi Gamer 3D Prophet
		IHV_3DLABS_300SX,
		IHV_3DLABS_300SXRO2,
		IHV_3DLABS_DELTA,
		IHV_3DLABS_500TX,
		IHV_3DLABS_PERMEDIA,
		IHV_3DLABS_P2,
		IHV_3DLABS_MX,
		IHV_3DLABS_GAMMA,
		IHV_3DLABS_PS2_ST,
		IHV_3DLABS_P3,
		IHV_3DLABS_R3,
		IHV_3DLABS_P4,
		IHV_3DLABS_R4,
		IHV_3DLABS_G2,
		IHV_3DLABS_TI_P1,				// Texas Instruments Permedia
		IHV_3DLABS_TI_P2,				// Texas Instruments Permedia 2
		IHV_3DLABS_TI_P2A,				// Texas Instruments Permedia 2
		IHV_CIRRUSLOGIC_5446,
		IHV_CIRRUSLOGIC_5462,
		IHV_CIRRUSLOGIC_5464,
		IHV_CIRRUSLOGIC_5465,
		IHV_CIRRUSLOGIC_6729,
		IHV_CIRRUSLOGIC_4610,
		IHV_CIRRUSLOGIC_4614,
		IHV_RENDITION_V1000,
		IHV_RENDITION_V2100_V2200,
		IHV_S3_SAVAGE4,
		IHV_S3_SAVAGEMX,
		IHV_S3_SAVAGE200,
		IHV_S3_VIRGE_VX,
		IHV_S3_VIRGE_MX_MV,
		IHV_S3_VIRGE_MX_PLUS,
		IHV_S3_VIRGE_MX,
		IHV_S3_VIRGE_GX2,
		IHV_S3_VIRGE_DX_GX,
		IHV_S3_TRIO64V2_DX_GX,
		IHV_S3_TRIO64UV_PLUS,
		IHV_S3_AURORA64V_PLUS,
		IHV_S3_AURORA128,
		IHV_POWERVR_KYRO,

		IHV_LAST,

		IHV_FORCE_DWORD					= 0x7fffffff
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	A function to detect the 3D card. Usage:
	 *	
	 *	// Detect the 3D card
	 *	DDDEVICEIDENTIFIER2 devIdent;
	 *	mDirectDraw->GetDeviceIdentifier(&devIdent, 0);
	 *	
	 *	// Catch IDs
	 *	DWORD VID = devIdent.dwVendorId;
	 *	DWORD DID = devIdent.dwDeviceId;
	 *	DWORD Rev = devIdent.dwRevision;
	 *	
	 *	IHVCode Code = Detect3DCard(VID, DID, Rev);
	 *	
	 *	\fn			Detect3DCard(udword vid, udword did, udword rev)
	 *	\return		IHVCode enum value
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICERENDERER_API IHVCode Detect3DCard(DWORD vid, DWORD did, DWORD rev);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	A function to translate the 3D card code into a string.
	 *	\fn			GetCardName(IHVCode code)
	 *	\return		card name
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FUNCTION ICERENDERER_API const char* GetCardName(IHVCode code);

#endif // ICEDETECTCARD_H
