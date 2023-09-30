/* s_sound.c */
#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "audio.h"
#include "i_main.h"
#include "config.h"
#include <stddef.h>

#ifndef SOUND
#define SOUND 1
#endif

#define SYS_FRAMES_PER_SEC 30

extern void wess_set_tweaks2(WessTweakAttr *attr);
extern void wess_get_tweaks2(WessTweakAttr *attr);

int S_AdjustSoundParams(mobj_t *listener, fixed_t x, fixed_t y, fixed_t z, u8* vol, u8* pan) HOT; // 80029A48

void S_Init(void) // 80029590
{
#if SOUND == 0
    return;
#endif
    u8* const audio_heap = AUDIO_HEAP_ADDR();
    u32 audioHeapEnd;
    //int loaded;
    int modulesize;
    int seqsize, seqtblsize;
    char *moduleptr;
    char *seqptr, *seqtblptr;

    WessTweakAttr tweak;
    WessConfig wess_config;
    ALHeap sys_aheap;

    //PRINTF_D(WHITE, "S_Init: Start");

    alHeapInit(&sys_aheap, audio_heap, AUDIO_HEAP_SIZE);

    //PRINTF_D(WHITE, "base %x", (int)&sys_aheap.base);
    //PRINTF_D(WHITE, "cur %x", (int)&sys_aheap.cur);
    //PRINTF_D(WHITE, "len %d", sys_aheap.len);
    //PRINTF_D(WHITE, "count %d", sys_aheap.count);

    /* tweak audio */
    wess_get_tweaks(&tweak);

    tweak.mask = TWEAK_DMA_BUFFERS | TWEAK_DMA_MESSAGES | TWEAK_DMA_BUFFER_LENGTH;
    tweak.dma_buffers = 40;
    tweak.dma_messages = 56;
    tweak.dma_buffer_length = 0x600;
    wess_set_tweaks(&tweak);

    /* init audio */
    wess_config.heap_ptr = &sys_aheap;
    wess_config.outputsamplerate = 22050;
    wess_config.maxACMDSize = 1024 * 3;
    wess_config.wdd_location = _doom64_wddSegmentRomStart;
    wess_config.reverb_id = WESS_REVERB_BIGROOM;
    wess_config.revtbl_ptr = 0;
    wess_config.audioframerate = (f32)SYS_FRAMES_PER_SEC;

    wess_init(&wess_config);

    //PRINTF_D(WHITE, "heap_ptr %x", (int)&wess_config.heap_ptr);
    //PRINTF_D(WHITE, "outputsamplerate %d", wess_config.outputsamplerate);
    //PRINTF_D(WHITE, "maxACMDSize %d", wess_config.maxACMDSize);
    //PRINTF_D(WHITE, "wdd_location %x", (int)&wess_config.wdd_location);
    //PRINTF_D(WHITE, "reverb_id %d", wess_config.reverb_id);
    //PRINTF_D(WHITE, "revtbl_ptr %d", wess_config.revtbl_ptr);
    //PRINTF_D(WHITE, "audioframerate %f", (f32)wess_config.audioframerate);

    // now we load the .wmd image into a temporary ram space
    modulesize = wess_size_module(_doom64_wmdSegmentRomStart);
    modulesize = ALIGN(modulesize, 16);
    moduleptr = alHeapAlloc(&sys_aheap, 1, modulesize);
    //PRINTF_D(WHITE, "modulesize %d", modulesize);
    //PRINTF_D(WHITE, "moduleptr %x", (int)&moduleptr);

    //loaded =
    wess_load_module(_doom64_wmdSegmentRomStart, moduleptr, modulesize);
    //PRINTF_D(WHITE, "loaded %d", loaded);


    seqtblsize = wess_seq_loader_sizeof(wess_get_master_status(), _doom64_wsdSegmentRomStart);
    seqtblsize = ALIGN(seqtblsize, 16);
    seqtblptr = alHeapAlloc(&sys_aheap, 1, seqtblsize);
    //PRINTF_D(WHITE, "seqtblsize %d", seqtblsize);
    //PRINTF_D(WHITE, "seqtblptr %x", (int)&seqtblptr);

    //this call may result in decompression callbacks
    wess_seq_loader_init(wess_get_master_status(), _doom64_wsdSegmentRomStart, NoOpenSeqHandle, seqtblptr, seqtblsize);

    seqsize = wess_seq_range_sizeof(0, wess_seq_loader_count());
    seqtblsize = ALIGN(seqsize, 16);
    seqptr = alHeapAlloc(&sys_aheap, 1, seqsize);

    //PRINTF_D(WHITE, "seqsize %d", seqsize);
    //PRINTF_D(WHITE, "seqptr %x", (int)&seqptr);

    //this call may result in decompression callbacks
    wess_seq_range_load(0, wess_seq_loader_count(), seqptr);

    audioHeapEnd = (u32)alHeapAlloc(&sys_aheap, 1, 4) - (u32)audio_heap;
    audioHeapEnd += 16;

    //PRINTF_D(WHITE, "audioHeapEnd %x", audioHeapEnd);

    if (audioHeapEnd > AUDIO_HEAP_SIZE)
        I_Error("S_Init: Audio heap overflow");

    S_SetSoundVolume(Settings.SfxVolume);
    S_SetMusicVolume(Settings.MusVolume);

    //PRINTF_D(WHITE, "S_Init: End");

    //while(1){}
}

void S_SetSoundVolume(int volume) // 800297A8
{
#if SOUND == 0
    return;
#endif
  wess_master_sfx_vol_set((char)((volume * 85) / 100));
}

void S_SetMusicVolume(int volume) // 800297F4
{
#if SOUND == 0
    return;
#endif
  wess_master_mus_vol_set((char)((volume * 110) / 100));
}

int music_sequence; // 8005b250

void S_StartMusic(int mus_seq) // 8002983C
{
#if SOUND == 0
    return;
#endif
    if ((*&disabledrawing) == false)
    {
        wess_seq_trigger(mus_seq);
        music_sequence = mus_seq;
    }
}

void S_StopMusic(void) // 80029878
{
#if SOUND == 0
    return;
#endif
    wess_seq_stop(music_sequence);
    music_sequence = 0;
}

void S_PauseSound(void) // 800298A4
{
#if SOUND == 0
    return;
#endif
    wess_seq_pauseall(YesMute, (REMEMBER_MUSIC|REMEMBER_SNDFX));
}

void S_ResumeSound(void) // 800298C8
{
#if SOUND == 0
    return;
#endif
    wess_seq_restartall(YesVoiceRestart);
}

void S_StopSound(mobj_t *origin,int seqnum) // 800298E8
{
#if SOUND == 0
    return;
#endif
    if (!origin)
        wess_seq_stop(seqnum);
    else
        wess_seq_stoptype((int)origin);
}

void S_StopAll(void) // 8002991C
{
#if SOUND == 0
    return;
#endif
    wess_seq_stopall();
}

#define SND_INACTIVE 0
#define SND_PLAYING 1

int S_SoundStatus(int seqnum) // 8002993C
{
#if SOUND == 0
    return SND_INACTIVE;
#endif
    if (wess_seq_status(seqnum) == SEQUENCE_PLAYING)
        return SND_PLAYING;
    else
        return SND_INACTIVE;
}

HOT void S_StartGlobalSound(int sound_id)
{
#if SOUND == 0
    return;
#endif
    TriggerPlayAttr attr;

    if ((*&disabledrawing) == false)
    {
        attr.mask = (TRIGGER_VOLUME | TRIGGER_PAN | TRIGGER_REVERB);
        attr.volume = 127;
        attr.pan = 64;
        attr.reverb = 0;

        wess_seq_trigger_type_special(sound_id, 0, &attr);
    }
}

void S_StartSoundAt(void *key, fixed_t x, fixed_t y, fixed_t z, int flags, int sound_id)
{
#if SOUND == 0
    return;
#endif
    TriggerPlayAttr attr;

    if ((*&disabledrawing) == false)
    {
        if (key != cameratarget)
        {
            if (!S_AdjustSoundParams(cameratarget, x, y, z, &attr.volume, &attr.pan))
                return;
        }
        else
        {
            attr.volume = 127;
            attr.pan = 64;
        }

        attr.mask = (TRIGGER_VOLUME | TRIGGER_PAN | TRIGGER_REVERB);

        if (flags & MS_REVERBHEAVY)
            attr.reverb = 32;
        else if (flags & MS_REVERB)
            attr.reverb = 16;
        else
            attr.reverb = 0;

        wess_seq_trigger_type_special(sound_id, (unsigned long) key, &attr);
    }
}

void S_StartSound(mobj_t *origin, int sound_id) // 80029970
{
    if (origin)
        S_StartSoundAt(origin, origin->x, origin->y, origin->z, origin->subsector->sector->flags, sound_id);
    else
        S_StartGlobalSound(sound_id);
}

void S_StartSectorSound(sector_t *origin, int sound_id)
{
    S_StartSoundAt(origin, origin->center_x, origin->center_y,
                   (origin->floorheight >> 1) + (origin->ceilingheight >> 1),
                   origin->flags, sound_id);
}

#define S_CLIPPING_DIST     (1700)
#define S_MAX_DIST          (127 * S_CLIPPING_DIST)
#define S_CLOSE_DIST        (200)
#define S_ATTENUATOR        (S_CLIPPING_DIST - S_CLOSE_DIST)
#define S_STEREO_SWING      (96)

int S_AdjustSoundParams(mobj_t *listener, fixed_t x, fixed_t y, fixed_t z, u8* vol, u8* pan) // 80029A48
{
#if SOUND == 0
    return 0;
#endif
    fixed_t approx_dist;
    angle_t angle;

    approx_dist = P_AproxDistance(listener->x - x, listener->y - y);
    approx_dist = P_AproxDistance(approx_dist, listener->z - z);
    approx_dist >>= FRACBITS;

    if (approx_dist > S_CLIPPING_DIST) {
        return 0;
    }

    if (listener->x != x || listener->y != y)
    {
        /* angle of source to listener */
        angle = R_PointToAngle2(listener->x, listener->y, x, y);

        if (angle <= listener->angle) {
            angle += 0xffffffff;
        }
        angle -= listener->angle;

        /* stereo separation */
        *pan = (128 - ((finesine(angle >> ANGLETOFINESHIFT) * S_STEREO_SWING) >> FRACBITS)) >> 1;
    }
    else
    {
        *pan = 64;
    }

    /* volume calculation */
    if (approx_dist < S_CLOSE_DIST)
    {
        *vol = 127;
    }
    else
    {
        /* distance effect */
        approx_dist = -approx_dist; /* set neg */
        *vol = MIN((((approx_dist << 7) - approx_dist) + S_MAX_DIST) / S_ATTENUATOR, 127);
    }

    return (*vol > 0);
}
