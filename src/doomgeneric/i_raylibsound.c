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
//      System interface for sound using raylib audio.
//

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deh_str.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#include "doomtype.h"

// Include raylib last so its bool macros don't collide with
// doomtype.h's enum { false, true }.
#include <raylib.h>

#define NUM_CHANNELS 16
#define OUTPUT_RATE 22050

// Cached PCM data attached to each sfxinfo_t via driver_data.
typedef struct
{
    int16_t *samples;    // 16-bit signed PCM at OUTPUT_RATE, mono
    int numSamples;      // number of frames
} raylib_sound_data_t;

// Per-channel state.
typedef struct
{
    Sound sound;
    boolean in_use;
    sfxinfo_t *sfxinfo;
} raylib_channel_t;

static raylib_channel_t channels[NUM_CHANNELS];
static boolean sound_initialized = false;
static boolean use_sfx_prefix = false;

// Referenced in i_sound.c I_BindSoundVariables() when FEATURE_SOUND is set.
int use_libsamplerate = 0;
float libsamplerate_scale = 0.65f;

// ---------------------------------------------------------------------------
// Helper: build the WAD lump name for an sfx
// ---------------------------------------------------------------------------
static void GetSfxLumpName(sfxinfo_t *sfx, char *buf, size_t buf_len)
{
    if (sfx->link != NULL)
    {
        sfx = sfx->link;
    }

    if (use_sfx_prefix)
    {
        M_snprintf(buf, buf_len, "ds%s", DEH_String(sfx->name));
    }
    else
    {
        M_StringCopy(buf, DEH_String(sfx->name), buf_len);
    }
}

// ---------------------------------------------------------------------------
// Convert an 8-bit unsigned PCM buffer (at original sample rate) to a
// 16-bit signed PCM buffer resampled to OUTPUT_RATE.
// Returns the converted data through *out and the frame count through *outLen.
// ---------------------------------------------------------------------------
static void ConvertSoundData(const byte *src, int srcLen, int srcRate,
                             int16_t **out, int *outLen)
{
    int targetLen;
    int i;
    int16_t *dst;
    long long accum;
    int stepNum;
    int stepDen;
    int srcIdx;

    // Calculate target length.
    // Use 64-bit to avoid overflow with large sounds.
    targetLen = (int)(((long long)srcLen * OUTPUT_RATE) / srcRate);
    if (targetLen < 1) targetLen = 1;

    dst = (int16_t *)malloc(targetLen * sizeof(int16_t));
    if (dst == NULL)
    {
        *out = NULL;
        *outLen = 0;
        return;
    }

    // Resample: map srcLen source samples onto targetLen output samples
    // using integer accumulation to determine source index for each output.
    stepNum = srcLen - 1;
    stepDen = targetLen - 1;
    if (stepDen <= 0) stepDen = 1;
    if (stepNum < 0) stepNum = 0;

    accum = 0;
    for (i = 0; i < targetLen; i++)
    {
        srcIdx = (int)(accum / stepDen);
        if (srcIdx >= srcLen) srcIdx = srcLen - 1;

        // Convert 8-bit unsigned to 16-bit signed.
        dst[i] = ((int)src[srcIdx] - 128) << 8;

        accum += stepNum;
    }

    *out = dst;
    *outLen = targetLen;
}

// ---------------------------------------------------------------------------
// Cache (convert + store) a sound effect.
// The converted data is stored in sfxinfo->driver_data.
// Returns true on success.
// ---------------------------------------------------------------------------
static boolean CacheSFX(sfxinfo_t *sfxinfo)
{
    int lumpnum;
    unsigned int lumplen;
    int samplerate;
    unsigned int length;
    byte *data;
    raylib_sound_data_t *snd;
    int16_t *converted;
    int convertedLen;

    lumpnum = sfxinfo->lumpnum;
    data = W_CacheLumpNum(lumpnum, PU_STATIC);
    lumplen = W_LumpLength(lumpnum);

    // Validate DMX header.
    if (lumplen < 8 || data[0] != 0x03 || data[1] != 0x00)
    {
        W_ReleaseLumpNum(lumpnum);
        sfxinfo->driver_data = NULL;
        return false;
    }

    samplerate = (data[3] << 8) | data[2];
    length = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4];

    if (length > lumplen - 8 || length <= 48)
    {
        W_ReleaseLumpNum(lumpnum);
        sfxinfo->driver_data = NULL;
        return false;
    }

    // DMX convention: skip first 16 and last 16 bytes of sample data.
    data += 16;
    length -= 32;

    // Skip 8 more bytes (matches DMX behaviour seen in i_sdlsound.c.template).
    data += 8;
    length -= 8;

    if (length <= 0 || samplerate <= 0)
    {
        W_ReleaseLumpNum(lumpnum);
        sfxinfo->driver_data = NULL;
        return false;
    }

    // Convert 8-bit unsigned PCM at [samplerate] to 16-bit signed at OUTPUT_RATE.
    ConvertSoundData(data, length, samplerate, &converted, &convertedLen);

    W_ReleaseLumpNum(lumpnum);

    if (converted == NULL || convertedLen <= 0)
    {
        sfxinfo->driver_data = NULL;
        return false;
    }

    // Allocate and fill cache entry.
    snd = (raylib_sound_data_t *)malloc(sizeof(raylib_sound_data_t));
    if (snd == NULL)
    {
        free(converted);
        sfxinfo->driver_data = NULL;
        return false;
    }

    snd->samples = converted;
    snd->numSamples = convertedLen;

    sfxinfo->driver_data = snd;
    return true;
}

// ---------------------------------------------------------------------------
// Ensure a sound is loaded and return its cached data.
// ---------------------------------------------------------------------------
static raylib_sound_data_t *GetSoundData(sfxinfo_t *sfxinfo)
{
    if (sfxinfo->driver_data == NULL)
    {
        if (!CacheSFX(sfxinfo))
        {
            return NULL;
        }
    }
    return (raylib_sound_data_t *)sfxinfo->driver_data;
}

// ---------------------------------------------------------------------------
// Interface functions
// ---------------------------------------------------------------------------

static int I_RL_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char namebuf[9];
    GetSfxLumpName(sfx, namebuf, sizeof(namebuf));
    return W_GetNumForName(namebuf);
}

static void I_RL_UpdateSoundParams(int handle, int vol, int sep)
{
    if (!sound_initialized || handle < 0 || handle >= NUM_CHANNELS)
    {
        return;
    }

    if (!channels[handle].in_use)
    {
        return;
    }

    // Convert Doom vol (0-127) to raylib (0.0-1.0).
    {
        float rvol = (float)vol / 127.0f;
        if (rvol < 0.0f) rvol = 0.0f;
        if (rvol > 1.0f) rvol = 1.0f;
        SetSoundVolume(channels[handle].sound, rvol);
    }

    // Convert Doom separation (0-254, 127=center) to raylib pan
    // (0.0=full left, 0.5=center, 1.0=full right).
    {
        float pan = (float)sep / 254.0f;
        if (pan < 0.0f) pan = 0.0f;
        if (pan > 1.0f) pan = 1.0f;
        SetSoundPan(channels[handle].sound, pan);
    }
}

static int I_RL_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep)
{
    raylib_sound_data_t *snd;

    if (!sound_initialized || channel < 0 || channel >= NUM_CHANNELS)
    {
        return -1;
    }

    // Stop whatever is playing on this channel.
    if (channels[channel].in_use)
    {
        StopSound(channels[channel].sound);
        UnloadSound(channels[channel].sound);
        channels[channel].in_use = false;
        channels[channel].sfxinfo = NULL;
    }

    // Load the sound data.
    snd = GetSoundData(sfxinfo);
    if (snd == NULL)
    {
        return -1;
    }

    // Build a raylib Wave from our cached PCM data and convert to Sound.
    {
        Wave wave;
        int dataSize = snd->numSamples * sizeof(int16_t);

        wave.data = malloc(dataSize);
        if (wave.data == NULL)
        {
            return -1;
        }
        memcpy(wave.data, snd->samples, dataSize);
        wave.frameCount = snd->numSamples;
        wave.sampleRate = OUTPUT_RATE;
        wave.sampleSize = 16;
        wave.channels = 1;

        channels[channel].sound = LoadSoundFromWave(wave);
        UnloadWave(wave); // frees our malloc'd data
    }

    channels[channel].in_use = true;
    channels[channel].sfxinfo = sfxinfo;

    // Apply volume & pan.
    I_RL_UpdateSoundParams(channel, vol, sep);

    PlaySound(channels[channel].sound);

    return channel;
}

static void I_RL_StopSound(int handle)
{
    if (!sound_initialized || handle < 0 || handle >= NUM_CHANNELS)
    {
        return;
    }

    if (!channels[handle].in_use)
    {
        return;
    }

    StopSound(channels[handle].sound);
    UnloadSound(channels[handle].sound);
    channels[handle].in_use = false;
    channels[handle].sfxinfo = NULL;
}

static boolean I_RL_SoundIsPlaying(int handle)
{
    if (!sound_initialized || handle < 0 || handle >= NUM_CHANNELS)
    {
        return false;
    }

    if (!channels[handle].in_use)
    {
        return false;
    }

    return IsSoundPlaying(channels[handle].sound);
}

static void I_RL_UpdateSound(void)
{
    int i;

    if (!sound_initialized)
    {
        return;
    }

    // Check all channels for sounds that have finished playing.
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        if (channels[i].in_use && !IsSoundPlaying(channels[i].sound))
        {
            // Sound has finished; clean up.
            UnloadSound(channels[i].sound);
            channels[i].in_use = false;
            channels[i].sfxinfo = NULL;
        }
    }
}

static void I_RL_ShutdownSound(void)
{
    int i;

    if (!sound_initialized)
    {
        return;
    }

    // Stop and unload all channel sounds.
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        if (channels[i].in_use)
        {
            StopSound(channels[i].sound);
            UnloadSound(channels[i].sound);
            channels[i].in_use = false;
            channels[i].sfxinfo = NULL;
        }
    }

    // Close audio device (the OS will reclaim cached sound data).
    CloseAudioDevice();
    sound_initialized = false;
}

static void I_RL_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    int i;

    if (!sound_initialized)
    {
        return;
    }

    printf("I_RL_PrecacheSounds: Precaching all sound effects..");

    for (i = 0; i < num_sounds; i++)
    {
        if ((i % 6) == 0)
        {
            printf(".");
            fflush(stdout);
        }

        GetSoundData(&sounds[i]);
    }

    printf("\n");
}

static boolean I_RL_InitSound(boolean _use_sfx_prefix)
{
    int i;

    use_sfx_prefix = _use_sfx_prefix;

    // Clear channel state.
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        channels[i].in_use = false;
        channels[i].sfxinfo = NULL;
    }

    // Initialise raylib audio device.
    InitAudioDevice();

    if (!IsAudioDeviceReady())
    {
        fprintf(stderr, "I_RL_InitSound: Failed to initialise raylib audio device.\n");
        return false;
    }

    // Set master volume to maximum (Doom controls per-channel volume).
    SetMasterVolume(1.0f);

    sound_initialized = true;
    return true;
}

// ---------------------------------------------------------------------------
// Device list – matched to the SDL module so Doom's snd_sfxdevice picks this.
// ---------------------------------------------------------------------------
static snddevice_t sound_raylib_devices[] =
{
    SNDDEVICE_SB,
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_AWE32,
};

sound_module_t DG_sound_module =
{
    sound_raylib_devices,
    arrlen(sound_raylib_devices),
    I_RL_InitSound,
    I_RL_ShutdownSound,
    I_RL_GetSfxLumpNum,
    I_RL_UpdateSound,
    I_RL_UpdateSoundParams,
    I_RL_StartSound,
    I_RL_StopSound,
    I_RL_SoundIsPlaying,
    I_RL_PrecacheSounds,
};
