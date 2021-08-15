
	#define BAN_ICECORE_AUTOLINK
	#define BAN_ICEMATHS_AUTOLINK
	#define BAN_ICEIMAGEWORK_AUTOLINK
	#define BAN_MESHMERIZER_AUTOLINK
	#define BAN_ICECHARACTER_AUTOLINK
	#define BAN_ICERENDERER_AUTOLINK

// Ignore unnecessary headers that don't compile on Win64
#define ICERENDERSTATEMANAGER_H
#define ICESTATEBLOCK_H
#define ICERENDERCORE_H
#define ICERENDERABLESURFACE_H
#define ICEVOIDRENDERER_H

	#include <IceRenderer/IceRendererAFX.h>
	using namespace IceRenderer;

//	#include <IceCharacter/IceCharacterAFX.h>
	#include <IceCharacter/IceCharacter.h>
	using namespace IceCharacter;

#include "ZCB2.h"
