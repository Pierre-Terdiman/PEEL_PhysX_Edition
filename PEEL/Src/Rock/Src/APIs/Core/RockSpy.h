#ifndef ROCK_SPY_H
#define ROCK_SPY_H

#include <Core/RockSettings.h>

#ifdef ROCK_USE_SPY
	#include "../../../Rock_Externals/Spy/SpyClient.h"

	#define SPY_ZONE(Label)	Spy::Zone __SpyZone(Label);
	#define SPY_SYNC		Spy::Sync();
	#define SPY_INIT		Spy::Init();
	#define SPY_CLOSE		Spy::Close();
#else
	#define SPY_ZONE(Label)
	#define SPY_SYNC
	#define SPY_INIT
	#define SPY_CLOSE
#endif

#endif
