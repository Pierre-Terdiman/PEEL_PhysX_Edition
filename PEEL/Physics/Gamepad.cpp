///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Gamepad.h"

#define PEEL_SUPPORT_GAMEPAD
//#define GAMEPAD_LOG

#ifdef PEEL_SUPPORT_GAMEPAD
	#include "Common.h"
	#include <XInput.h>
	//#pragma comment(lib, "xinput.lib")

	typedef DWORD (WINAPI *LPXINPUTGETSTATE)(DWORD, XINPUT_STATE*);
	typedef DWORD (WINAPI *LPXINPUTGETCAPABILITIES)(DWORD,DWORD,XINPUT_CAPABILITIES*);

	static HMODULE					gXInputLibrary = null;
	static LPXINPUTGETSTATE			gXInputGetState = null;
	static LPXINPUTGETCAPABILITIES	gXInputGetCapabilities = null;
	static bool						gGamePadConnected = false;
	static udword					gConnectedPad = 0;

	static const udword				MAX_GAMEPADS = 4;
	static const udword				MAX_GAMEPAD_AXES = 4;
	static XINPUT_STATE				gLastInputState[MAX_GAMEPADS];
	static int						gLastAxisData[MAX_GAMEPADS][MAX_GAMEPAD_AXES];
#endif

bool InitGamepads()
{
#ifdef PEEL_SUPPORT_GAMEPAD
	gXInputLibrary			= null;
	gXInputGetState			= null;
	gXInputGetCapabilities	= null;

	static const udword xInputLibCount = 4;
	static const char* xInputLibs[xInputLibCount] = { "xinput1_4.dll" ,
													  "xinput1_3.dll",
	                                                  "xinput1_2.dll",
	                                                  "xinput1_1.dll" };
	for(udword i=0; i<xInputLibCount; i++)
	{
		gXInputLibrary = LoadLibraryA(xInputLibs[i]);
		if(gXInputLibrary)
			break;
	}

	if(!gXInputLibrary)
	{
		OutputConsoleError("Could not load XInput library. Gamepads support not available.\n");
		return false;
	}

	gXInputGetState			= (LPXINPUTGETSTATE)GetProcAddress(gXInputLibrary, "XInputGetState");
	gXInputGetCapabilities	= (LPXINPUTGETCAPABILITIES)GetProcAddress(gXInputLibrary, "XInputGetCapabilities");
	ASSERT(gXInputGetState && "Error loading XInputGetState function.");
	ASSERT(gXInputGetCapabilities && "Error loading XInputGetCapabilities function.");

	gGamePadConnected = false;
	gConnectedPad = 0;

	for(udword p=0;p<MAX_GAMEPADS;p++)
	{
		ZeroMemory(&gLastInputState[p], sizeof(XINPUT_STATE));
		for(udword i=0;i<MAX_GAMEPAD_AXES;i++)
		{
			gLastAxisData[p][i]=0;
		}
	}
	return true;
#else
	return false;
#endif
}

void ReleaseGamepads()
{
#ifdef PEEL_SUPPORT_GAMEPAD
	if(gXInputLibrary) 
	{
		FreeLibrary(gXInputLibrary);
		gXInputLibrary = null;
		gXInputLibrary = null;
		gXInputGetState = null;
		gXInputGetCapabilities = null;
		gGamePadConnected = false;
		gConnectedPad = 0;
	}
#endif
}

void ProcessGamepads(GamepadInterface& gpint)
{
	SPY_ZONE("ProcessGamepads")

#ifdef PEEL_SUPPORT_GAMEPAD

	if(!gXInputLibrary || !gXInputGetState || !gXInputGetCapabilities)
		return;

	static sdword disConnected[4] = {1, 2, 3, 4};

	for(udword p=0;p<MAX_GAMEPADS;p++)
	{
		if((--disConnected[p]) == 0)
		{
			XINPUT_STATE inputState;
			DWORD state = (gXInputGetState)(p, &inputState);
			if(state == ERROR_DEVICE_NOT_CONNECTED)
			{
				disConnected[p] = 4;

				if(gGamePadConnected && (gConnectedPad == p))
				{
					gGamePadConnected = false;
					gConnectedPad = 0;
					for(udword k=0;k<MAX_GAMEPADS;k++)
					{
						XINPUT_STATE inputStateDisc;
						DWORD stateDisc = (gXInputGetState)(k, &inputStateDisc);
						if(stateDisc == ERROR_SUCCESS)
						{
							gConnectedPad = k;
							gGamePadConnected = true;
							break;
						}
					}
				}
			}
			else if(state == ERROR_SUCCESS)
			{
				if(!gGamePadConnected)
				{
					gGamePadConnected = true;
					gConnectedPad = p;
				}

				disConnected[p] = 1; //force to test next time
				XINPUT_CAPABILITIES caps;
				(gXInputGetCapabilities)(p, XINPUT_FLAG_GAMEPAD, &caps);

				{
					// Process buttons
					const WORD lastWButtons	= gLastInputState[p].Gamepad.wButtons;
					const WORD currWButtons	= inputState.Gamepad.wButtons;

					const WORD buttonsDown	= currWButtons & ~lastWButtons;
					const WORD buttonsUp	=  ~currWButtons & lastWButtons;
//					const WORD buttonsHeld	= currWButtons & lastWButtons;

					for(int i=0;i<14;i++)
					{
						// order has to match struct GamepadControls
						static const WORD buttonMasks[]={
							XINPUT_GAMEPAD_DPAD_UP,
							XINPUT_GAMEPAD_DPAD_DOWN,
							XINPUT_GAMEPAD_DPAD_LEFT,
							XINPUT_GAMEPAD_DPAD_RIGHT,
							XINPUT_GAMEPAD_START,
							XINPUT_GAMEPAD_BACK,
							XINPUT_GAMEPAD_LEFT_THUMB,
							XINPUT_GAMEPAD_RIGHT_THUMB, 
							XINPUT_GAMEPAD_Y,
							XINPUT_GAMEPAD_A,
							XINPUT_GAMEPAD_X,
							XINPUT_GAMEPAD_B,
							XINPUT_GAMEPAD_LEFT_SHOULDER,
							XINPUT_GAMEPAD_RIGHT_SHOULDER,
						};

						if (buttonsDown & buttonMasks[i])
						{
#ifdef GAMEPAD_LOG
							printf("Gamepad button %d true\n", i);
#endif
							gpint.OnButtonEvent(i, true);
						}
						else if(buttonsUp & buttonMasks[i])
						{
#ifdef GAMEPAD_LOG
							printf("Gamepad button %d false\n", i);
#endif
							gpint.OnButtonEvent(i, false);
						}
					}

					// PT: I think we do the 2 last ones separately because they're listed in GamepadControls but not in buttonMasks...
					{
						const BYTE oldTriggerVal = gLastInputState[p].Gamepad.bRightTrigger;
						const BYTE newTriggerVal = inputState.Gamepad.bRightTrigger;
						if(oldTriggerVal || newTriggerVal)
						{
#ifdef GAMEPAD_LOG
							printf("Gamepad right trigger %d %d\n", oldTriggerVal, newTriggerVal);
#endif
							gpint.OnAnalogButtonEvent(1, oldTriggerVal, newTriggerVal);
						}
					}
					{
						const BYTE oldTriggerVal = gLastInputState[p].Gamepad.bLeftTrigger;
						const BYTE newTriggerVal = inputState.Gamepad.bLeftTrigger;
						if(oldTriggerVal || newTriggerVal)
						{
#ifdef GAMEPAD_LOG
							printf("Gamepad left trigger %d %d\n", oldTriggerVal, newTriggerVal);
#endif
							gpint.OnAnalogButtonEvent(0, oldTriggerVal, newTriggerVal);
						}
					}
				}		

				const int axisData[] = {inputState.Gamepad.sThumbRX, inputState.Gamepad.sThumbRY, inputState.Gamepad.sThumbLX, inputState.Gamepad.sThumbLY };
				for(udword i=0;i<MAX_GAMEPAD_AXES;i++)
				{
					if(axisData[i] != gLastAxisData[p][i])
					{
						int data = axisData[i];
						if(1)
						{
							if(abs(data) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
							{
								data = 0;
							}
						}
//						m_sf_app->onGamepadAxis( i, ((float)data)/SHRT_MAX);
						float Value = (float(data))/SHRT_MAX;
/*						if(Value>1.0f)
							Value = 1.0f;
						if(Value<-1.0f)
							Value = -1.0f;
						if(fabsf(Value)>0.2f)	// ### TODO: expose that threshold*/
						{
						//if(data)
#ifdef GAMEPAD_LOG
							printf("Gamepad axis %d %f\n", i, Value);
#endif
							gpint.OnAxisEvent(i, Value);
						}
					}
					gLastAxisData[p][i] = axisData[i];
				}
				gLastInputState[p] = inputState;
			}
		}
	}
#endif
}
