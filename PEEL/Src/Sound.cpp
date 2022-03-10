///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Sound.h"

#ifdef PEEL_SOUND
	#ifdef _WIN64
	static char* FMOD_DLL_Name = "fmod64.dll";
	#else
	static char* FMOD_DLL_Name = "fmod.dll";
	#endif
	#include <fmodapi375win/api/inc/fmoddyn.h>
	static FMOD_INSTANCE* gFMOD = null;
	static const int gFrequency	= 44100;
	static udword gChannel		= INVALID_ID;

#ifdef _WIN64
static FMOD_INSTANCE *FMOD_CreateInstance64(char *dllName)
{
    FMOD_INSTANCE *instance;

    instance = (FMOD_INSTANCE *)calloc(sizeof(FMOD_INSTANCE), 1);
    if (!instance)
    {
        return NULL;
    }

    instance->module = LoadLibrary(dllName);
    if (!instance->module)
    {
        free(instance);
        return NULL;
    }

    F_GETPROC(FSOUND_SetOutput, "FSOUND_SetOutput");
    F_GETPROC(FSOUND_SetDriver, "FSOUND_SetDriver");
    F_GETPROC(FSOUND_SetMixer, "FSOUND_SetMixer");
    F_GETPROC(FSOUND_SetBufferSize, "FSOUND_SetBufferSize");
    F_GETPROC(FSOUND_SetHWND, "FSOUND_SetHWND");
    F_GETPROC(FSOUND_SetMinHardwareChannels, "FSOUND_SetMinHardwareChannels");
    F_GETPROC(FSOUND_SetMaxHardwareChannels, "FSOUND_SetMaxHardwareChannels");
    F_GETPROC(FSOUND_SetMemorySystem, "FSOUND_SetMemorySystem");
    F_GETPROC(FSOUND_Init, "FSOUND_Init");
    F_GETPROC(FSOUND_Close, "FSOUND_Close");
    F_GETPROC(FSOUND_Update, "FSOUND_Update");
    F_GETPROC(FSOUND_SetSFXMasterVolume, "FSOUND_SetSFXMasterVolume");
    F_GETPROC(FSOUND_SetPanSeperation, "FSOUND_SetPanSeperation");
    F_GETPROC(FSOUND_SetSpeakerMode, "FSOUND_SetSpeakerMode");
    F_GETPROC(FSOUND_GetError, "FSOUND_GetError");
    F_GETPROC(FSOUND_GetVersion, "FSOUND_GetVersion");
    F_GETPROC(FSOUND_GetOutput, "FSOUND_GetOutput");
    F_GETPROC(FSOUND_GetOutputHandle, "FSOUND_GetOutputHandle");
    F_GETPROC(FSOUND_GetDriver, "FSOUND_GetDriver");
    F_GETPROC(FSOUND_GetMixer, "FSOUND_GetMixer");
    F_GETPROC(FSOUND_GetNumDrivers, "FSOUND_GetNumDrivers");
    F_GETPROC(FSOUND_GetDriverName, "FSOUND_GetDriverName");
    F_GETPROC(FSOUND_GetDriverCaps, "FSOUND_GetDriverCaps");
    F_GETPROC(FSOUND_GetOutputRate, "FSOUND_GetOutputRate");
    F_GETPROC(FSOUND_GetMaxChannels, "FSOUND_GetMaxChannels");
    F_GETPROC(FSOUND_GetMaxSamples, "FSOUND_GetMaxSamples");
    F_GETPROC(FSOUND_GetSpeakerMode, "FSOUND_GetSpeakerMode");
    F_GETPROC(FSOUND_GetSFXMasterVolume, "FSOUND_GetSFXMasterVolume");
    F_GETPROC(FSOUND_GetNumHWChannels, "FSOUND_GetNumHWChannels");
    F_GETPROC(FSOUND_GetChannelsPlaying, "FSOUND_GetChannelsPlaying");
    F_GETPROC(FSOUND_GetCPUUsage, "FSOUND_GetCPUUsage");
    F_GETPROC(FSOUND_GetMemoryStats, "FSOUND_GetMemoryStats");
    F_GETPROC(FSOUND_Sample_Load, "FSOUND_Sample_Load");
    F_GETPROC(FSOUND_Sample_Alloc, "FSOUND_Sample_Alloc");
    F_GETPROC(FSOUND_Sample_Free, "FSOUND_Sample_Free");
    F_GETPROC(FSOUND_Sample_Upload, "FSOUND_Sample_Upload");
    F_GETPROC(FSOUND_Sample_Lock, "FSOUND_Sample_Lock");
    F_GETPROC(FSOUND_Sample_Unlock, "FSOUND_Sample_Unlock");
    F_GETPROC(FSOUND_Sample_SetMode, "FSOUND_Sample_SetMode");
    F_GETPROC(FSOUND_Sample_SetLoopPoints, "FSOUND_Sample_SetLoopPoints");
    F_GETPROC(FSOUND_Sample_SetDefaults, "FSOUND_Sample_SetDefaults");
    F_GETPROC(FSOUND_Sample_SetDefaultsEx, "FSOUND_Sample_SetDefaultsEx");
    F_GETPROC(FSOUND_Sample_SetMinMaxDistance, "FSOUND_Sample_SetMinMaxDistance");
    F_GETPROC(FSOUND_Sample_SetMaxPlaybacks, "FSOUND_Sample_SetMaxPlaybacks");
    F_GETPROC(FSOUND_Sample_Get, "FSOUND_Sample_Get");
    F_GETPROC(FSOUND_Sample_GetName, "FSOUND_Sample_GetName");
    F_GETPROC(FSOUND_Sample_GetLength, "FSOUND_Sample_GetLength");
    F_GETPROC(FSOUND_Sample_GetLoopPoints, "FSOUND_Sample_GetLoopPoints");
    F_GETPROC(FSOUND_Sample_GetDefaults, "FSOUND_Sample_GetDefaults");
    F_GETPROC(FSOUND_Sample_GetDefaultsEx, "FSOUND_Sample_GetDefaultsEx");
    F_GETPROC(FSOUND_Sample_GetMode, "FSOUND_Sample_GetMode");
    F_GETPROC(FSOUND_Sample_GetMinMaxDistance, "FSOUND_Sample_GetMinMaxDistance");
    F_GETPROC(FSOUND_PlaySound, "FSOUND_PlaySound");
    F_GETPROC(FSOUND_PlaySoundEx, "FSOUND_PlaySoundEx");
    F_GETPROC(FSOUND_StopSound, "FSOUND_StopSound");
    F_GETPROC(FSOUND_SetFrequency, "FSOUND_SetFrequency");
    F_GETPROC(FSOUND_SetVolume, "FSOUND_SetVolume");
    F_GETPROC(FSOUND_SetVolumeAbsolute, "FSOUND_SetVolumeAbsolute");
    F_GETPROC(FSOUND_SetPan, "FSOUND_SetPan");
    F_GETPROC(FSOUND_SetSurround, "FSOUND_SetSurround");
    F_GETPROC(FSOUND_SetMute, "FSOUND_SetMute");
    F_GETPROC(FSOUND_SetPriority, "FSOUND_SetPriority");
    F_GETPROC(FSOUND_SetReserved, "FSOUND_SetReserved");
    F_GETPROC(FSOUND_SetPaused, "FSOUND_SetPaused");
    F_GETPROC(FSOUND_SetLoopMode, "FSOUND_SetLoopMode");
    F_GETPROC(FSOUND_SetCurrentPosition, "FSOUND_SetCurrentPosition");
    F_GETPROC(FSOUND_3D_SetAttributes, "FSOUND_3D_SetAttributes");
    F_GETPROC(FSOUND_3D_SetMinMaxDistance, "FSOUND_3D_SetMinMaxDistance");
    F_GETPROC(FSOUND_IsPlaying, "FSOUND_IsPlaying");
    F_GETPROC(FSOUND_GetFrequency, "FSOUND_GetFrequency");
    F_GETPROC(FSOUND_GetVolume, "FSOUND_GetVolume");
    F_GETPROC(FSOUND_GetAmplitude, "FSOUND_GetAmplitude");
    F_GETPROC(FSOUND_GetPan, "FSOUND_GetPan");
    F_GETPROC(FSOUND_GetSurround, "FSOUND_GetSurround");
    F_GETPROC(FSOUND_GetMute, "FSOUND_GetMute");
    F_GETPROC(FSOUND_GetPriority, "FSOUND_GetPriority");
    F_GETPROC(FSOUND_GetReserved, "FSOUND_GetReserved");
    F_GETPROC(FSOUND_GetPaused, "FSOUND_GetPaused");
    F_GETPROC(FSOUND_GetLoopMode, "FSOUND_GetLoopMode");
    F_GETPROC(FSOUND_GetCurrentPosition, "FSOUND_GetCurrentPosition");
    F_GETPROC(FSOUND_GetCurrentSample, "FSOUND_GetCurrentSample");
    F_GETPROC(FSOUND_GetCurrentLevels, "FSOUND_GetCurrentLevels");
    F_GETPROC(FSOUND_GetNumSubChannels, "FSOUND_GetNumSubChannels");
    F_GETPROC(FSOUND_GetSubChannel, "FSOUND_GetSubChannel");
    F_GETPROC(FSOUND_3D_GetAttributes, "FSOUND_3D_GetAttributes");
    F_GETPROC(FSOUND_3D_GetMinMaxDistance, "FSOUND_3D_GetMinMaxDistance");
    F_GETPROC(FSOUND_3D_Listener_SetCurrent, "FSOUND_3D_Listener_SetCurrent");
    F_GETPROC(FSOUND_3D_Listener_SetAttributes, "FSOUND_3D_Listener_SetAttributes");
    F_GETPROC(FSOUND_3D_Listener_GetAttributes, "FSOUND_3D_Listener_GetAttributes");
    F_GETPROC(FSOUND_3D_SetDopplerFactor, "FSOUND_3D_SetDopplerFactor");
    F_GETPROC(FSOUND_3D_SetDistanceFactor, "FSOUND_3D_SetDistanceFactor");
    F_GETPROC(FSOUND_3D_SetRolloffFactor, "FSOUND_3D_SetRolloffFactor");
    F_GETPROC(FSOUND_FX_Enable, "FSOUND_FX_Enable");
    F_GETPROC(FSOUND_FX_Disable, "FSOUND_FX_Disable");
    F_GETPROC(FSOUND_FX_SetChorus, "FSOUND_FX_SetChorus");
    F_GETPROC(FSOUND_FX_SetCompressor, "FSOUND_FX_SetCompressor");
    F_GETPROC(FSOUND_FX_SetDistortion, "FSOUND_FX_SetDistortion");
    F_GETPROC(FSOUND_FX_SetEcho, "FSOUND_FX_SetEcho");
    F_GETPROC(FSOUND_FX_SetFlanger, "FSOUND_FX_SetFlanger");
    F_GETPROC(FSOUND_FX_SetGargle, "FSOUND_FX_SetGargle");
    F_GETPROC(FSOUND_FX_SetI3DL2Reverb, "FSOUND_FX_SetI3DL2Reverb");
    F_GETPROC(FSOUND_FX_SetParamEQ, "FSOUND_FX_SetParamEQ");
    F_GETPROC(FSOUND_FX_SetWavesReverb, "FSOUND_FX_SetWavesReverb");
    F_GETPROC(FSOUND_Stream_Open, "FSOUND_Stream_Open");
    F_GETPROC(FSOUND_Stream_Create, "FSOUND_Stream_Create");
    F_GETPROC(FSOUND_Stream_Play, "FSOUND_Stream_Play");
    F_GETPROC(FSOUND_Stream_PlayEx, "FSOUND_Stream_PlayEx");
    F_GETPROC(FSOUND_Stream_Stop, "FSOUND_Stream_Stop");
    F_GETPROC(FSOUND_Stream_Close, "FSOUND_Stream_Close");
    F_GETPROC(FSOUND_Stream_SetEndCallback, "FSOUND_Stream_SetEndCallback");
    F_GETPROC(FSOUND_Stream_SetSyncCallback, "FSOUND_Stream_SetSyncCallback");
    F_GETPROC(FSOUND_Stream_GetSample, "FSOUND_Stream_GetSample");
    F_GETPROC(FSOUND_Stream_CreateDSP, "FSOUND_Stream_CreateDSP");
    F_GETPROC(FSOUND_Stream_SetBufferSize, "FSOUND_Stream_SetBufferSize");
    F_GETPROC(FSOUND_Stream_SetPosition, "FSOUND_Stream_SetPosition");
    F_GETPROC(FSOUND_Stream_GetPosition, "FSOUND_Stream_GetPosition");
    F_GETPROC(FSOUND_Stream_SetTime, "FSOUND_Stream_SetTime");
    F_GETPROC(FSOUND_Stream_GetTime, "FSOUND_Stream_GetTime");
    F_GETPROC(FSOUND_Stream_GetLength, "FSOUND_Stream_GetLength");
    F_GETPROC(FSOUND_Stream_GetLengthMs, "FSOUND_Stream_GetLengthMs");
    F_GETPROC(FSOUND_Stream_SetMode, "FSOUND_Stream_SetMode");
    F_GETPROC(FSOUND_Stream_GetMode, "FSOUND_Stream_GetMode");
    F_GETPROC(FSOUND_Stream_SetSubStream, "FSOUND_Stream_SetSubStream");
    F_GETPROC(FSOUND_Stream_GetNumSubStreams, "FSOUND_Stream_GetNumSubStreams");
    F_GETPROC(FSOUND_Stream_SetSubStreamSentence, "FSOUND_Stream_SetSubStreamSentence");
    F_GETPROC(FSOUND_Stream_SetLoopPoints, "FSOUND_Stream_SetLoopPoints");
    F_GETPROC(FSOUND_Stream_SetLoopCount, "FSOUND_Stream_SetLoopCount");
    F_GETPROC(FSOUND_Stream_AddSyncPoint, "FSOUND_Stream_AddSyncPoint");
    F_GETPROC(FSOUND_Stream_DeleteSyncPoint, "FSOUND_Stream_DeleteSyncPoint");
    F_GETPROC(FSOUND_Stream_GetNumSyncPoints, "FSOUND_Stream_GetNumSyncPoints");
    F_GETPROC(FSOUND_Stream_GetSyncPoint, "FSOUND_Stream_GetSyncPoint");
    F_GETPROC(FSOUND_Stream_GetSyncPointInfo, "FSOUND_Stream_GetSyncPointInfo");
    F_GETPROC(FSOUND_Stream_GetOpenState, "FSOUND_Stream_GetOpenState");
    F_GETPROC(FSOUND_Stream_GetNumTagFields, "FSOUND_Stream_GetNumTagFields");
    F_GETPROC(FSOUND_Stream_GetTagField, "FSOUND_Stream_GetTagField");
    F_GETPROC(FSOUND_Stream_FindTagField, "FSOUND_Stream_FindTagField");
    F_GETPROC(FSOUND_Stream_Net_SetProxy, "FSOUND_Stream_Net_SetProxy");
    F_GETPROC(FSOUND_Stream_Net_GetLastServerStatus, "FSOUND_Stream_Net_GetLastServerStatus");
    F_GETPROC(FSOUND_Stream_Net_SetBufferProperties, "FSOUND_Stream_Net_SetBufferProperties");
    F_GETPROC(FSOUND_Stream_Net_GetBufferProperties, "FSOUND_Stream_Net_GetBufferProperties");
    F_GETPROC(FSOUND_Stream_Net_SetMetadataCallback, "FSOUND_Stream_Net_SetMetadataCallback");
    F_GETPROC(FSOUND_Stream_Net_GetStatus, "FSOUND_Stream_Net_GetStatus");
    F_GETPROC(FSOUND_CD_Play, "FSOUND_CD_Play");
    F_GETPROC(FSOUND_CD_SetPlayMode, "FSOUND_CD_SetPlayMode");
    F_GETPROC(FSOUND_CD_Stop, "FSOUND_CD_Stop");
    F_GETPROC(FSOUND_CD_SetPaused, "FSOUND_CD_SetPaused");
    F_GETPROC(FSOUND_CD_SetVolume, "FSOUND_CD_SetVolume");
    F_GETPROC(FSOUND_CD_SetTrackTime, "FSOUND_CD_SetTrackTime");
    F_GETPROC(FSOUND_CD_OpenTray, "FSOUND_CD_OpenTray");
    F_GETPROC(FSOUND_CD_GetPaused, "FSOUND_CD_GetPaused");
    F_GETPROC(FSOUND_CD_GetTrack, "FSOUND_CD_GetTrack");
    F_GETPROC(FSOUND_CD_GetNumTracks, "FSOUND_CD_GetNumTracks");
    F_GETPROC(FSOUND_CD_GetVolume, "FSOUND_CD_GetVolume");
    F_GETPROC(FSOUND_CD_GetTrackLength, "FSOUND_CD_GetTrackLength");
    F_GETPROC(FSOUND_CD_GetTrackTime, "FSOUND_CD_GetTrackTime");
    F_GETPROC(FSOUND_DSP_Create, "FSOUND_DSP_Create");
    F_GETPROC(FSOUND_DSP_Free, "FSOUND_DSP_Free");
    F_GETPROC(FSOUND_DSP_SetPriority, "FSOUND_DSP_SetPriority");
    F_GETPROC(FSOUND_DSP_GetPriority, "FSOUND_DSP_GetPriority");
    F_GETPROC(FSOUND_DSP_SetActive, "FSOUND_DSP_SetActive");
    F_GETPROC(FSOUND_DSP_GetActive, "FSOUND_DSP_GetActive");
    F_GETPROC(FSOUND_DSP_GetClearUnit, "FSOUND_DSP_GetClearUnit");
    F_GETPROC(FSOUND_DSP_GetSFXUnit, "FSOUND_DSP_GetSFXUnit");
    F_GETPROC(FSOUND_DSP_GetMusicUnit, "FSOUND_DSP_GetMusicUnit");
    F_GETPROC(FSOUND_DSP_GetClipAndCopyUnit, "FSOUND_DSP_GetClipAndCopyUnit");
    F_GETPROC(FSOUND_DSP_GetFFTUnit, "FSOUND_DSP_GetFFTUnit");
    F_GETPROC(FSOUND_DSP_MixBuffers, "FSOUND_DSP_MixBuffers");
    F_GETPROC(FSOUND_DSP_ClearMixBuffer, "FSOUND_DSP_ClearMixBuffer");
    F_GETPROC(FSOUND_DSP_GetBufferLength, "FSOUND_DSP_GetBufferLength");
    F_GETPROC(FSOUND_DSP_GetBufferLengthTotal, "FSOUND_DSP_GetBufferLengthTotal");
    F_GETPROC(FSOUND_DSP_GetSpectrum, "FSOUND_DSP_GetSpectrum");
    F_GETPROC(FSOUND_Reverb_SetProperties, "FSOUND_Reverb_SetProperties");
    F_GETPROC(FSOUND_Reverb_GetProperties, "FSOUND_Reverb_GetProperties");
    F_GETPROC(FSOUND_Reverb_SetChannelProperties, "FSOUND_Reverb_SetChannelProperties");
    F_GETPROC(FSOUND_Reverb_GetChannelProperties, "FSOUND_Reverb_GetChannelProperties");
    F_GETPROC(FSOUND_Record_SetDriver, "FSOUND_Record_SetDriver");
    F_GETPROC(FSOUND_Record_GetNumDrivers, "FSOUND_Record_GetNumDrivers");
    F_GETPROC(FSOUND_Record_GetDriverName, "FSOUND_Record_GetDriverName");
    F_GETPROC(FSOUND_Record_GetDriver, "FSOUND_Record_GetDriver");
    F_GETPROC(FSOUND_Record_StartSample, "FSOUND_Record_StartSample");
    F_GETPROC(FSOUND_Record_Stop, "FSOUND_Record_Stop");
    F_GETPROC(FSOUND_Record_GetPosition, "FSOUND_Record_GetPosition");
    F_GETPROC(FSOUND_File_SetCallbacks, "FSOUND_File_SetCallbacks");
    F_GETPROC(FMUSIC_LoadSong, "FMUSIC_LoadSong");
    F_GETPROC(FMUSIC_LoadSongEx, "FMUSIC_LoadSongEx");
    F_GETPROC(FMUSIC_GetOpenState, "FMUSIC_GetOpenState");
    F_GETPROC(FMUSIC_FreeSong, "FMUSIC_FreeSong");
    F_GETPROC(FMUSIC_PlaySong, "FMUSIC_PlaySong");
    F_GETPROC(FMUSIC_StopSong, "FMUSIC_StopSong");
    F_GETPROC(FMUSIC_StopAllSongs, "FMUSIC_StopAllSongs");
    F_GETPROC(FMUSIC_SetZxxCallback, "FMUSIC_SetZxxCallback");
    F_GETPROC(FMUSIC_SetRowCallback, "FMUSIC_SetRowCallback");
    F_GETPROC(FMUSIC_SetOrderCallback, "FMUSIC_SetOrderCallback");
    F_GETPROC(FMUSIC_SetInstCallback, "FMUSIC_SetInstCallback");
    F_GETPROC(FMUSIC_SetSample, "FMUSIC_SetSample");
    F_GETPROC(FMUSIC_SetUserData, "FMUSIC_SetUserData");
    F_GETPROC(FMUSIC_OptimizeChannels, "FMUSIC_OptimizeChannels");
    F_GETPROC(FMUSIC_SetReverb, "FMUSIC_SetReverb");
    F_GETPROC(FMUSIC_SetLooping, "FMUSIC_SetLooping");
    F_GETPROC(FMUSIC_SetOrder, "FMUSIC_SetOrder");
    F_GETPROC(FMUSIC_SetPaused, "FMUSIC_SetPaused");
    F_GETPROC(FMUSIC_SetMasterVolume, "FMUSIC_SetMasterVolume");
    F_GETPROC(FMUSIC_SetMasterSpeed, "FMUSIC_SetMasterSpeed");
    F_GETPROC(FMUSIC_SetPanSeperation, "FMUSIC_SetPanSeperation");
    F_GETPROC(FMUSIC_GetName, "FMUSIC_GetName");
    F_GETPROC(FMUSIC_GetType, "FMUSIC_GetType");
    F_GETPROC(FMUSIC_GetNumOrders, "FMUSIC_GetNumOrders");
    F_GETPROC(FMUSIC_GetNumPatterns, "FMUSIC_GetNumPatterns");
    F_GETPROC(FMUSIC_GetNumInstruments, "FMUSIC_GetNumInstruments");
    F_GETPROC(FMUSIC_GetNumSamples, "FMUSIC_GetNumSamples");
    F_GETPROC(FMUSIC_GetNumChannels, "FMUSIC_GetNumChannels");
    F_GETPROC(FMUSIC_GetSample, "FMUSIC_GetSample");
    F_GETPROC(FMUSIC_GetPatternLength, "FMUSIC_GetPatternLength");
    F_GETPROC(FMUSIC_IsFinished, "FMUSIC_IsFinished");
    F_GETPROC(FMUSIC_IsPlaying, "FMUSIC_IsPlaying");
    F_GETPROC(FMUSIC_GetMasterVolume, "FMUSIC_GetMasterVolume");
    F_GETPROC(FMUSIC_GetGlobalVolume, "FMUSIC_GetGlobalVolume");
    F_GETPROC(FMUSIC_GetOrder, "FMUSIC_GetOrder");
    F_GETPROC(FMUSIC_GetPattern, "FMUSIC_GetPattern");
    F_GETPROC(FMUSIC_GetSpeed, "FMUSIC_GetSpeed");
    F_GETPROC(FMUSIC_GetBPM, "FMUSIC_GetBPM");
    F_GETPROC(FMUSIC_GetRow, "FMUSIC_GetRow");
    F_GETPROC(FMUSIC_GetPaused, "FMUSIC_GetPaused");
    F_GETPROC(FMUSIC_GetTime, "FMUSIC_GetTime");
    F_GETPROC(FMUSIC_GetRealChannel, "FMUSIC_GetRealChannel");
    F_GETPROC(FMUSIC_GetUserData, "FMUSIC_GetUserData");

    return instance;
}
#endif
#endif


static bool InitFMOD()
{
#ifdef PEEL_SOUND
	#ifdef _WIN64
	gFMOD = FMOD_CreateInstance64(FMOD_DLL_Name);
	#else
	gFMOD = FMOD_CreateInstance(FMOD_DLL_Name);
	#endif
	if(!gFMOD)
	{
		IceCore::MessageBox(null, "FMOD dll not found.\nSound will be disabled.", "ICE Message", MB_OK);
		Log("FMOD dll not found.\nSound will be disabled.\n");
		return false;
	}

	if(gFMOD->FSOUND_GetVersion()<FMOD_VERSION)
	{
//		IceCore::MessageBox(null, "Wrong FMOD version", "Error", MB_OK);
		Log("Wrong FMOD version\n");
		gFMOD = null;
		return false;
	}

	// INITIALIZE
	if(!gFMOD->FSOUND_Init(gFrequency, 32, FSOUND_INIT_GLOBALFOCUS))
	{
//		IceCore::MessageBox(null, "FMOD init failed", "Error", MB_OK);
		Log("FMOD init failed\n");
		gFMOD = null;
		return false;
	}
#endif
	return true;
}

void StartSound(const char* filename, udword pos)
{
#ifdef PEEL_SOUND
	const char* FindPEELFile(const char* filename);
	const char* File = FindPEELFile(filename);
	if(!File)
		return;

	InitFMOD();
	if(!gFMOD)
		return;

	VirtualFile Music(File);
	udword Length;
	const char* MusicData = (const char*)Music.Load(Length);

	FSOUND_SAMPLE* SoundSample = gFMOD->FSOUND_Sample_Load(0, MusicData, FSOUND_LOADMEMORY, 0, Length);
	udword Channel = gFMOD->FSOUND_PlaySound(FSOUND_FREE, (FSOUND_SAMPLE*)SoundSample);
	gFMOD->FSOUND_SetVolume(Channel, 200);
//	gFMOD->FSOUND_SetVolume(Channel, 20);
//	gFMOD->FSOUND_SetLoopMode(Channel, FSOUND_LOOP_OFF);
	gFMOD->FSOUND_SetLoopMode(Channel, FSOUND_LOOP_NORMAL);
	gFMOD->FSOUND_SetCurrentPosition(Channel, pos*1024);
	gChannel = Channel;
#endif
}

udword GetSoundPos()
{
#ifdef PEEL_SOUND
	if(gChannel!=INVALID_ID)
	{
		const udword CurPos = gFMOD->FSOUND_GetCurrentPosition(gChannel)/1024;
		//printf("Sound: %d\n", CurPos);
		return CurPos;
	}
#endif
	return INVALID_ID;
}

void SetFreq(int f)
{
#ifdef PEEL_SOUND
	if(gChannel!=INVALID_ID)
	{
		gFMOD->FSOUND_SetFrequency(gChannel, f);
	}
#endif
}
