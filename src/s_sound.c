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

#ifdef NDEBUG
#define AUDIO_HEAP_BASE_SIZE 0x25000
#else
// alHeapDBAlloc stores extra debug info
#define AUDIO_HEAP_BASE_SIZE 0x26000
#endif
#define BASEPROG_SIZE (ALIGN(_bss_end, 32) - (u32)BASE_RAM_ADDR)
#define AUDIO_HEAP_SIZE ALIGN( \
        AUDIO_HEAP_BASE_SIZE \
        + ALIGN(_doom64_wmdSegmentRomEnd - _doom64_wmdSegmentRomStart, 16) \
        + ALIGN(_doom64_wsdSegmentRomEnd - _doom64_wsdSegmentRomStart, 16) \
    , 64)

extern void wess_set_tweaks2(WessTweakAttr *attr);
extern void wess_get_tweaks2(WessTweakAttr *attr);

int S_AdjustSoundParams(mobj_t *listener, fixed_t x, fixed_t y, fixed_t z, u8* vol, u8* pan) HOT; // 80029A48

void *audio_heap_start;

void S_Init(void) // 80029590
{
#if SOUND == 0
    return;
#endif
    u8* const audio_heap = AUDIO_HEAP_END(); //int loaded;
    int modulesize;
    int seqsize, seqtblsize;
    int seqcount;
    char *moduleptr;
    char *seqptr, *seqtblptr;
    s32 maxsfxpitch;

    WessTweakAttr tweak;
    WessConfig wess_config;
    ALHeap sys_aheap;

    //D_printf("S_Init: Start\n");

    alHeapInit(&sys_aheap, audio_heap, AUDIO_HEAP_MAX_SIZE());

    wess_base_init(_doom64_wddSegmentRomStart);

    //D_printf("base %08lx\n", (u32)&sys_aheap.base);
    //D_printf("cur %08lx\n", (u32)&sys_aheap.cur);
    //D_printf("len %ld\n", sys_aheap.len);
    //D_printf("count %ld\n", sys_aheap.count);

    // now we load the .wmd image into a temporary ram space
    modulesize = wess_size_module(_doom64_wmdSegmentRomStart);
    modulesize = ALIGN(modulesize, 16);
    moduleptr = alHeapAlloc(&sys_aheap, 1, modulesize);
    //D_printf("modulesize %d\n", modulesize);
    //D_printf("moduleptr %x\n", (int)&moduleptr);

    //loaded =
    wess_load_module(_doom64_wmdSegmentRomStart, moduleptr, modulesize);
    //D_printf("loaded %d\n", loaded);

    seqtblsize = wess_seq_loader_sizeof(wess_get_master_status(), _doom64_wsdSegmentRomStart);
    seqtblsize = ALIGN(seqtblsize, 16);
    seqtblptr = alHeapAlloc(&sys_aheap, 1, seqtblsize);
    //D_printf("seqtblsize %d\n", seqtblsize);
    //D_printf("seqtblptr %x\n", (int)&seqtblptr);

    //this call may result in decompression callbacks
    wess_seq_loader_init(wess_get_master_status(), _doom64_wsdSegmentRomStart, NoOpenSeqHandle, seqtblptr, seqtblsize);

    seqcount = wess_seq_loader_count();
    seqsize = wess_seq_range_sizeof(0, seqcount);
    seqtblsize = ALIGN(seqsize, 16);
    seqptr = alHeapAlloc(&sys_aheap, 1, seqsize);

    //D_printf("seqsize %d\n", seqsize);
    //D_printf("seqptr %x\n", (int)&seqptr);

    //this call may result in decompression callbacks
    wess_seq_range_load(0, seqcount, seqptr);

    maxsfxpitch = get_max_sfx_pitch();

    /* tweak audio */
    wess_get_tweaks(&tweak);

    tweak.mask = TWEAK_DMA_BUFFERS | TWEAK_DMA_MESSAGES | TWEAK_DMA_BUFFER_LENGTH;
    tweak.dma_buffers = maxsfxpitch >= 0 ? 80 : 40;
    tweak.dma_messages = 56;
    tweak.dma_buffer_length = 0x600;
    wess_set_tweaks(&tweak);

    /* init audio */
    wess_config.heap_ptr = &sys_aheap;
    wess_config.outputsamplerate = maxsfxpitch >= 1200 ? 44100 : 22050;
    wess_config.maxACMDSize = 1024 * 3;
    wess_config.reverb_id = WESS_REVERB_BIGROOM;
    wess_config.revtbl_ptr = 0;
    wess_config.audioframerate = (f32)SYS_FRAMES_PER_SEC;

    //D_printf("heap_ptr %x\n", (int)&wess_config.heap_ptr);
    //D_printf("outputsamplerate %lu\n", wess_config.outputsamplerate);
    //D_printf("maxACMDSize %lu\n", wess_config.maxACMDSize);
    //D_printf("reverb_id %ld\n", wess_config.reverb_id);
    //D_printf("revtbl_ptr %08lx\n", (u32)wess_config.revtbl_ptr);
    //D_printf("audioframerate %f\n", (f32)wess_config.audioframerate);

    wess_init(&wess_config);

#ifdef DEBUG_MEM
    alHeapCheck(&sys_aheap);
#endif

    audio_heap_start = (void *) (((u32)sys_aheap.cur) & ~63);

    S_SetSoundVolume(Settings.SfxVolume);
    S_SetMusicVolume(Settings.MusVolume);

    //D_printf("S_Init: End\n");

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
