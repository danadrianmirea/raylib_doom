//
// Copyright(C) 2025
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      Stub music module for raylib-based doomgeneric port.
//      raylib does not support MIDI/MUS playback natively, so this
//      module reports success but produces no audio, allowing Doom
//      to run without crashing when music is requested.
//

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i_sound.h"
#include "doomtype.h"

#define DUMMY_HANDLE ((void *)1)

// ---------------------------------------------------------------------------
// No-op stubs
// ---------------------------------------------------------------------------

static boolean I_RL_InitMusic(void)
{
    // Report success so Doom proceeds normally.
    return true;
}

static void I_RL_ShutdownMusic(void)
{
    // Nothing to clean up.
}

static void I_RL_SetMusicVolume(int volume)
{
    // Not supported in stub mode.
    (void)volume;
}

static void I_RL_PauseMusic(void)
{
}

static void I_RL_ResumeMusic(void)
{
}

static void *I_RL_RegisterSong(void *data, int len)
{
    // Return a dummy non-NULL handle so Doom's S_ChangeMusic
    // does not have to deal with NULL pointer checks.
    (void)data;
    (void)len;
    return DUMMY_HANDLE;
}

static void I_RL_UnRegisterSong(void *handle)
{
    // Nothing to free for a dummy handle.
    (void)handle;
}

static void I_RL_PlaySong(void *handle, boolean looping)
{
    // No audio output in stub mode.
    (void)handle;
    (void)looping;
}

static void I_RL_StopSong(void)
{
}

static boolean I_RL_MusicIsPlaying(void)
{
    // Report "not playing" since there is no actual playback.
    return false;
}

static void I_RL_PollMusic(void)
{
}

// ---------------------------------------------------------------------------
// Device list
// ---------------------------------------------------------------------------
static snddevice_t music_raylib_devices[] =
{
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

music_module_t DG_music_module =
{
    music_raylib_devices,
    arrlen(music_raylib_devices),
    I_RL_InitMusic,
    I_RL_ShutdownMusic,
    I_RL_SetMusicVolume,
    I_RL_PauseMusic,
    I_RL_ResumeMusic,
    I_RL_RegisterSong,
    I_RL_UnRegisterSong,
    I_RL_PlaySong,
    I_RL_StopSong,
    I_RL_MusicIsPlaying,
    I_RL_PollMusic,
};
