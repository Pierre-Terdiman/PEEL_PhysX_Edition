///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main file for ZCB2 library.
 *	\file		ZCB2.h
 *	\author		Pierre Terdiman
 *	\date		January, 14, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ZCB2_H
#define ZCB2_H

	#include "ZCB2Settings.h"

	namespace ZCB2
	{
		#include "ChunkBase.h"
		#include "ChunkXIDS.h"
		#include "ChunkUBAR.h"
		#include "ChunkROBJ.h"
		#include "ChunkPRS.h"
		#include "ChunkTIME.h"

		#include "ChunkSCEN.h"

		#include "ChunkTXMP.h"
		#include "ChunkMATL.h"

		#include "ChunkCAME.h"
		#include "ChunkFCAM.h"
		#include "ChunkTCAM.h"

		#include "ChunkPNTS.h"
		#include "ChunkFACE.h"
		#include "ChunkMESH.h"

		#include "ChunkLITE.h"
		#include "ChunkSLIT.h"
		#include "ChunkDLIT.h"
		#include "ChunkPLIT.h"

		#include "ChunkHELP.h"
		#include "ChunkBGIZ.h"
		#include "ChunkSGIZ.h"
		#include "ChunkCGIZ.h"
		#include "ChunkSKEL.h"

		#include "ChunkCURV.h"
		#include "ChunkSHAP.h"

		#include "ChunkCTRL.h"
		#include "ChunkSAMP.h"
		#include "ChunkROTA.h"

		#include "ChunkMOVE.h"

		#include "ZCB2Breaker.h"
		#include "ZCB2Exporter.h"
		#include "ZCB2Importer.h"
	}

#endif // ZCB2_H
