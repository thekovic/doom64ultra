#include "doomdef.h"
#include "p_local.h"
#include "p_saveg.h"
#include "r_local.h"
#include "st_main.h"
#include "i_usb.h"
#include "i_debug.h"

boolean SramPresent = false;
u32 SramSize = 0;
static boolean SramBanked = false;
static boolean QuickLoadAvailable = false;

#define SRAM_MAGIC 0x44363455 // 'D64U'
#define QUICKSAVE_MAGIC 0x44363453 // 'D64S'
#define SRAM_DATA_VERSION 0

#define HEADER_ADDR 0
#define CONFIG_ADDR (sizeof(sramheader_t))
#define SAVE_ADDR (CONFIG_ADDR + sizeof(config_t))
#define QUICKSAVE_ADDR (SAVE_ADDR + sizeof(levelsave_t) * MAXSRAMSAVES)
#define FOOTER_ADDR (SramSize - sizeof(sramheader_t))
#define QUICKSAVE_SIZE (SramSize - QUICKSAVE_ADDR - sizeof(sramheader_t))

static OSPiHandle SramHandle ALIGNED(8);

#define SRAM_START_ADDR  0x08000000
#define SRAM_PAGE_SIZE   0x8000
#define SRAM_latency     0x5
#define SRAM_pulse       0x0c
#define SRAM_pageSize    0xd
#define SRAM_relDuration 0x2

static boolean I_LoadConfig(void);
static void I_DetectQuickLoad(void);

static void I_ReadWriteSram(u32 addr, void* buf, u32 size, s32 flag)
{
    OSIoMesg msgbuf;
    OSMesgQueue queue;
    OSMesg msg;
    u32 base;

    if (size == 0)
        return;

    osCreateMesgQueue(&queue, &msg, 1);

    msgbuf.hdr.pri      = OS_MESG_PRI_NORMAL;
    msgbuf.hdr.retQueue = &queue;

    if(flag == OS_READ)
        osInvalDCache((void*)buf, (s32)size);
    else
        osWritebackDCache((void*)buf, (s32)size);

    /* make multiple transfers when (addr + size) crosses bank boundary */
    while (size > 0)
    {
        msgbuf.dramAddr = buf;

        if (SramBanked)
        {
            base = addr & 0x7fff;

            /* bank select in bits 18-19 */
            msgbuf.devAddr = base + ((addr & 0x18000) << 3);
            msgbuf.size    = (base + size >= SRAM_PAGE_SIZE) ? SRAM_PAGE_SIZE - base : size;
            buf += msgbuf.size;
            addr += msgbuf.size;
        }
        else
        {
            msgbuf.devAddr = addr;
            msgbuf.size    = size;
        }

        osEPiStartDma(&SramHandle, &msgbuf, flag);
        osRecvMesg(&queue, &msg, OS_MESG_BLOCK);

        size -= msgbuf.size;
    }
}

static u8 *write_buf;
static u8 *read_buf;

static boolean I_SramVerifyBanked(u32 bank)
{
    /* Force reading/writing to use banks */
    SramBanked = true;

    /* Clear all previous SRAM banks to detect address wrapping */
    bzero(write_buf,  SRAM_PAGE_SIZE);
    for (u32 i = 0; i < bank; i++)
        I_ReadWriteSram(i<<15, write_buf, SRAM_PAGE_SIZE, OS_WRITE);

    u32 *write_words = (u32 *)write_buf;
    for (u32 i = 0; i < SRAM_PAGE_SIZE / sizeof(u32); i++)
        write_words[i] = (bank << 15) + i * 4;

    I_ReadWriteSram(bank<<15, write_buf, SRAM_PAGE_SIZE, OS_WRITE);
    I_ReadWriteSram(bank<<15, read_buf, SRAM_PAGE_SIZE, OS_READ);

    if (memcmp(write_buf, read_buf, SRAM_PAGE_SIZE) != 0)
    {
        SramBanked = false;
        return false;
    }

    /* Check that no previous banks were modified by changing this one */
    bzero(write_buf,  SRAM_PAGE_SIZE);
    for (u32 i = 0; i < bank; i++)
    {
        I_ReadWriteSram(i<<15, read_buf, SRAM_PAGE_SIZE, OS_READ);
        if (memcmp(write_buf, read_buf, SRAM_PAGE_SIZE) != 0)
        {
            SramBanked = false;
            return false;
        }
    }

    return true;
}

static boolean I_SramVerify(u32 capacity)
{
    u32 offset = 0;

    u32 *write_words = (u32 *)write_buf;
    for (int i = 0; i < capacity / sizeof(u32); i++)
        write_words[i] = i * 4;

    while (capacity)
    {
        I_ReadWriteSram(offset, write_buf + offset, SRAM_PAGE_SIZE, OS_WRITE);
        I_ReadWriteSram(offset, read_buf + offset, SRAM_PAGE_SIZE, OS_READ);
        offset += SRAM_PAGE_SIZE;
        capacity -= SRAM_PAGE_SIZE;
    }

    return memcmp(write_buf, read_buf, capacity) == 0;
}

static void I_DetectSramType(void)
{
    write_buf = Z_BumpAlloc(SRAM_PAGE_SIZE*8);
    read_buf = &write_buf[SRAM_PAGE_SIZE*4];
    if(I_SramVerify(SRAM_PAGE_SIZE))
    {
        SramPresent = true;
        SramSize = SRAM_PAGE_SIZE;
        if(I_SramVerifyBanked(1) && I_SramVerifyBanked(2))
        {
            SramSize = SRAM_PAGE_SIZE*3;
            if(I_SramVerifyBanked(3))
                SramSize = SRAM_PAGE_SIZE*4;
        }
        else if (!IsEmulator && I_SramVerify(SRAM_PAGE_SIZE*3))
        {
            /* contiguous SRAM checks cause crashes in mupen64plus and
               derivatives, so only check this when real hardware detected */
            SramSize = SRAM_PAGE_SIZE*3;
            if (I_SramVerify(SRAM_PAGE_SIZE*4))
                SramSize = SRAM_PAGE_SIZE*4;
        }
    }
    Z_BumpAlloc(-SRAM_PAGE_SIZE*8); /* free the bumped memory */
    write_buf = read_buf = NULL;
}

typedef struct  ALIGNED(8) {
    u32 magic;
    u32 size: 2;
    u32 banked: 1;
    u32 version: 29;
} sramheader_t;

void I_InitSram(void)
{
    sramheader_t header ALIGNED(16);
    sramheader_t footer ALIGNED(16);

    if (SramHandle.baseAddress == PHYS_TO_K1(SRAM_START_ADDR))
        return;

    SramHandle.type = DEVICE_TYPE_SRAM;
    SramHandle.baseAddress = PHYS_TO_K1(SRAM_START_ADDR);
    SramHandle.latency = (u8)SRAM_latency;
    SramHandle.pulse = (u8)SRAM_pulse;
    SramHandle.pageSize = (u8)SRAM_pageSize;
    SramHandle.relDuration = (u8)SRAM_relDuration;
    SramHandle.domain = PI_DOMAIN2;

    bzero(&(SramHandle.transferInfo), sizeof SramHandle.transferInfo);

    osEPiLinkHandle(&SramHandle);

    // try to read from start and end to ensure we have the correct size
    I_ReadWriteSram(HEADER_ADDR, &header, sizeof header, OS_READ);
    if (header.magic == SRAM_MAGIC && header.version == SRAM_DATA_VERSION)
    {
        switch (header.size)
        {
            case 0: SramSize = SRAM_PAGE_SIZE; break;
            case 2: SramSize = SRAM_PAGE_SIZE*3; break;
            case 3: SramSize = SRAM_PAGE_SIZE*4; break;
            default: break;
        }
        if (header.banked)
            SramBanked = true;

        if (SramSize)
        {
            I_ReadWriteSram(FOOTER_ADDR, &footer, sizeof footer, OS_READ);
            if (memcmp(&header, &footer, sizeof header) == 0)
            {
                    SramPresent = true;
                    I_DetectQuickLoad();
                    if (!I_LoadConfig())
                        I_SaveConfig();
                    return;
            }
        }
    }

    // detect sram type when initializing
    I_DetectSramType();
    if (SramPresent)
    {
        header.magic = SRAM_MAGIC;
        header.version = SRAM_DATA_VERSION;
        header.size = (SramSize >> 15) - 1;
        header.banked = SramBanked;
        I_ReadWriteSram(HEADER_ADDR, &header, sizeof header, OS_WRITE);
        I_ReadWriteSram(FOOTER_ADDR, &header, sizeof header, OS_WRITE);
        I_SaveConfig();
    }
}

typedef struct __attribute__((__packed__)) ALIGNED(8) {
    u16 crc;
    u32 brightness: 10;
    u32 displayx: 6;
    u32 displayy: 6;
    u32 texturefilter: 1;
    u32 spritefilter: 1;
    u32 skyfilter: 1;
    u32 tvmode: 2;
    u32 antialiasing: 1;
    u32 screenaspect: 2;
    u32 videoresolution: 2;
    u32 bitdepth: 1;
    u32 gammacorrect: 1;
    u32 ditherfilter: 1;
    u32 colordither: 2;
    u32 flashbrightness: 6;
    u32 motionbob: 6;
    u32 storytext: 1;
    u32 mapstats: 1;
    u32 enablemessages: 1;
    u32 coloredhud: 1;
    u32 greenblood: 1;
    u32 hudmargin: 5;
    u32 hudopacity: 8;
    u32 sfxvolume: 7;
    u32 musvolume: 7;

    savedplayerconfig_t player ALIGNED(4);
} config_t;

#define CRC16_INIT 0xffff

static void Crc16(const u8 *buf, u32 size, u16 *crc)
{
    u8 x;

    while (size--)
    {
        x = *crc >> 8 ^ *(buf++);
        x ^= x>>4;
        *crc = (*crc << 8) ^ ((((u16)x) << 12)) ^ ((((u16)x) << 5)) ^ ((u16)x);
    }
}

void I_SaveConfig(void)
{
    if (!SramPresent)
        return;

    config_t config ALIGNED(16);

    bzero(&config, sizeof config);

    config.brightness = Settings.Brightness;
    config.displayx = VideoSettings.Display_X + 16;
    config.displayx = VideoSettings.Display_Y + 20;
    config.texturefilter = Settings.VideoFilters[0];
    config.spritefilter = Settings.VideoFilters[1];
    config.skyfilter = Settings.VideoFilters[2];
    config.tvmode = VideoSettings.TvMode;
    config.antialiasing = VideoSettings.AntiAliasing;
    config.screenaspect = VideoSettings.ScreenAspect;
    config.videoresolution = VideoSettings.Resolution;
    config.bitdepth = VideoSettings.BitDepth;
    config.gammacorrect = VideoSettings.GammaCorrect;
    config.ditherfilter = VideoSettings.DitherFilter;
    config.colordither = Settings.ColorDither;
    config.flashbrightness = Settings.FlashBrightness;
    config.motionbob = Settings.MotionBob >> 15;
    config.storytext = Settings.StoryText;
    config.mapstats = Settings.MapStats;
    config.enablemessages = Settings.EnableMessages;
    config.coloredhud = Settings.HudTextColors;
    config.greenblood = Settings.GreenBlood;
    config.hudmargin = Settings.HudMargin;
    config.hudopacity = Settings.HudOpacity;
    config.sfxvolume = Settings.SfxVolume;
    config.musvolume = Settings.MusVolume;

    P_ArchivePlayerConfig(0, &config.player);

    config.crc = CRC16_INIT;
    Crc16(((u8*)&config)+sizeof(u16), sizeof(config_t)-sizeof(u16), &config.crc);
    I_ReadWriteSram(CONFIG_ADDR, &config, sizeof config, OS_WRITE);
}

extern void P_RefreshBrightness(void);

static boolean I_LoadConfig(void)
{
    config_t config ALIGNED(16);
    u16 crc = CRC16_INIT;

    I_ReadWriteSram(CONFIG_ADDR, &config, sizeof config, OS_READ);

    Crc16(((u8*)&config)+sizeof(u16), sizeof(config_t)-sizeof(u16), &crc);
    if (config.crc != crc)
        return false;

    Settings.Brightness = config.brightness % 801;
    VideoSettings.Display_X = ((int)config.displayx) % 41 - 16;
    VideoSettings.Display_Y = ((int)config.displayx) % 33 - 20;
    Settings.VideoFilters[0] = config.texturefilter;
    Settings.VideoFilters[1] = config.spritefilter;
    Settings.VideoFilters[2] = config.skyfilter;
    VideoSettings.TvMode = config.tvmode;
    VideoSettings.AntiAliasing = config.antialiasing;
    VideoSettings.ScreenAspect = config.screenaspect % 3;
    VideoSettings.Resolution = config.videoresolution % 3;
    VideoSettings.BitDepth = config.bitdepth;
    VideoSettings.GammaCorrect = config.gammacorrect;
    VideoSettings.DitherFilter = config.ditherfilter;
    Settings.ColorDither = config.colordither;
    Settings.FlashBrightness = config.flashbrightness % 33;
    Settings.MotionBob = (config.motionbob % 33) << 15;
    Settings.StoryText = config.storytext;
    Settings.MapStats = config.mapstats;
    Settings.EnableMessages = config.enablemessages;
    Settings.HudTextColors = config.coloredhud;
    Settings.GreenBlood = config.greenblood;
    Settings.HudMargin = config.hudmargin % 21;
    Settings.HudOpacity = config.hudopacity;
    Settings.SfxVolume = config.sfxvolume % 101;
    Settings.MusVolume = config.musvolume % 101;

    P_UnArchivePlayerConfig(0, &config.player);

    P_RefreshBrightness();
    I_RefreshVideo();
    S_SetSoundVolume(Settings.SfxVolume);
    S_SetMusicVolume(Settings.MusVolume);

    return true;
}

void I_SaveProgress(levelsave_t *save)
{
    int bit = 0;

    bzero(save, sizeof *save);

    save->present = 1;
    save->map = nextmap;
    save->skill = customskill;
    for(int i = 0; i < NUMWEAPONS; i++)
    {
        if(i != wp_fist && i != wp_pistol)
        {
            if(players[0].weaponowned[i])
                save->weapons |= (1 << bit);
            bit++;
        }
    }
    save->artifacts = players[0].artifacts;
    save->health = players[0].health;
    save->armor = players[0].armorpoints;
    save->clip = players[0].ammo[am_clip];
    save->shell = players[0].ammo[am_shell];
    save->cell = players[0].ammo[am_cell];
    save->misl = players[0].ammo[am_misl];
    save->backpack = players[0].backpack;
    save->armortype = players[0].armortype;

    save->crc = CRC16_INIT;
    Crc16(((u8*)save)+sizeof(u16), sizeof(levelsave_t)-sizeof(u16), &save->crc);
}

void I_SaveProgressToSram(u8 index, const levelsave_t *save)
{
    if (index > MAXSRAMSAVES)
        I_Error("Save index out of range");

    I_ReadWriteSram(SAVE_ADDR + index * sizeof *save, (void*)save, sizeof *save, OS_WRITE);
}

void I_DeleteSramSave(u8 index)
{
    if (index > MAXSRAMSAVES)
        I_Error("Save index out of range");

    levelsave_t save = { .present = 0 };

    I_ReadWriteSram(SAVE_ADDR + index * sizeof save, (void*)&save, sizeof save, OS_WRITE);
}

void I_ReadSramSaves(levelsave_t *saves)
{
    u16 crc;
    int i;

    I_ReadWriteSram(SAVE_ADDR, saves, sizeof(levelsave_t) * MAXSRAMSAVES, OS_READ);
    for (i = 0; i < MAXSRAMSAVES; i++)
    {
        crc = CRC16_INIT;
        Crc16(((u8*)saves)+sizeof(u16), sizeof(levelsave_t)-sizeof(u16), &crc);
        if (crc != saves->crc)
            saves->present = 0;
        saves++;
    }
}

boolean I_IsSaveValid(const levelsave_t *save)
{
    u16 crc = CRC16_INIT;

    if (!save->present)
        return false;

    Crc16(((u8*)save)+sizeof(u16), sizeof(levelsave_t)-sizeof(u16), &crc);
    return crc == save->crc;
}

// only loads player progress, level/skill must be set separately
void I_LoadProgress(const levelsave_t *save)
{
    int bit = 0;

    if (!I_IsSaveValid(save))
        return;

    for(int i = 0; i < NUMWEAPONS; i++)
    {
        if(i != wp_fist && i != wp_pistol)
        {
            if(save->weapons & (1 << bit))
            {
                players[0].weaponowned[i] = true;
                players[0].weaponwheelsize += WHEEL_WEAPON_SIZE;
            }
            bit++;
        }
    }
    players[0].artifacts = save->artifacts;
    players[0].health = save->health;
    players[0].armorpoints = save->armor;
    players[0].ammo[am_clip] = save->clip;
    players[0].ammo[am_shell] = save->shell;
    players[0].ammo[am_cell] = save->cell;
    players[0].ammo[am_misl] = save->misl;
    players[0].backpack = save->backpack;
    players[0].armortype = save->armortype;
}

void I_SaveDemo(void *buffer, int len)
{
    if (len <= QUICKSAVE_SIZE)
        I_ReadWriteSram(QUICKSAVE_ADDR, buffer, len, OS_WRITE);
}

extern int start_time;

typedef struct ALIGNED(8)
{
    u32 magic;
    u32 size;
    u16 crc;
    u32 sectors;
    u32 lines;
    u32 macros;
    u32 mobjs;
    u32 thinkers;
    customskill_t skill;
    u32 map: 15;
    u32 gametype: 2;
    u32 player1: 1;
    u32 player2: 1;
    u32 player3: 1;
    u32 player4: 1;
    u32 leveltime: 24;
    u32 rndindex: 8;
    u32 prndindex: 8;
    u32 irndindex: 8;
    savedmacrosheader_t activemacro;
} quicksaveheader_t;

typedef enum
{
    phase_sectors = 0,
    phase_lines = 1,
    phase_macros = 2,
    phase_mobjs = 3,
    phase_thinkers = 4,
} savephase_e;

extern int rndindex;
extern int prndindex;
extern int irndindex;

void I_QuickSaveInternal(quicksaveheader_t *header, u32 limit, void (*write)(u32, const void*, u32))
{
    byte                 buf[4096] ALIGNED(16);
    register savephase_e phase = phase_sectors;
    register u32         addr = QUICKSAVE_ADDR + sizeof *header;
    register u32         count;
    u32        i;
    mobj_t    *mobj;
    thinker_t *th;

    bzero(header, sizeof *header);

    header->magic = QUICKSAVE_MAGIC;
    header->sectors = numsectors;
    header->lines = numlines;
    header->macros = nummacros;
    header->mobjs = 0;
    header->thinkers = 0;
    header->skill = customskill;
    header->gametype = gt_single;
    header->map = gamemap;
    header->player1 = 1;
    //header->player1 = playeringame[0];
    //header->player2 = playeringame[1];
    //header->player3 = playeringame[2];
    //header->player4 = playeringame[3];
    header->leveltime = ticon - start_time;
    header->rndindex = rndindex;
    header->prndindex = prndindex;
    header->irndindex = irndindex;

    P_ArchiveActiveMacro(&header->activemacro);

    header->crc = CRC16_INIT;
    header->size += sizeof *header;

    count = P_ArchivePlayers(buf);
    write(addr, buf, count);

    Crc16(buf, count, &header->crc);
    addr += count;
    header->size += count;

    i = 0;
    mobj = mobjhead.next;
    th = thinkercap.next;
    while (1)
    {
        switch (phase)
        {
        case phase_sectors:
            count = P_ArchiveSectors(buf, sizeof buf, &i);
            break;
        case phase_lines:
            count = P_ArchiveLines(buf, sizeof buf, &i);
            break;
        case phase_macros:
            count = P_ArchiveMacros(buf, sizeof buf, &i);
            break;
        case phase_mobjs:
            count = P_ArchiveMobjs(buf, sizeof buf, &mobj, &header->mobjs);
            break;
        case phase_thinkers:
            count = P_ArchiveThinkers(buf, sizeof buf, &th, &header->thinkers);
            break;
        }
        if (count == 0)
        {
            if (phase == phase_thinkers)
            {
                break;
            }
            else
            {
                i = 0;
                phase += 1;
                continue;
            }
        }

        Crc16(buf, count, &header->crc);
        if (addr + count >= limit)
            I_Error("Quicksave overflow");

        write(addr, buf, count);
        addr += count;
        header->size += count;
    }
}

static void I_SramWriteFunc(u32 addr, const void* buf, u32 size)
{
    I_ReadWriteSram(addr, (void*)buf, size, OS_WRITE);
}

void I_QuickSave(void)
{
    if (!SramPresent)
        return;

    quicksaveheader_t header ALIGNED(16);

    I_QuickSaveInternal(&header, FOOTER_ADDR, I_SramWriteFunc);
    // write header last after calculating size and crc
    I_ReadWriteSram(QUICKSAVE_ADDR, &header, sizeof header, OS_WRITE);
    QuickLoadAvailable = true;
}

#ifdef USB

static void I_NoopWriteFunc(u32 addr, const void* buf, u32 size) {}

static void I_USBWriteFunc(u32 addr, const void* buf, u32 size) {
    I_USBSendPart(buf, size);
}

void I_USBQuickSave(void)
{
    quicksaveheader_t header ALIGNED(16);
    u32 size = P_CurrentQuickSaveSize(MAXINT);

    /* need to run through it twice to calculate CRC */
    I_QuickSaveInternal(&header, size, I_NoopWriteFunc);
    I_USBSendStart(size);
    I_USBSendPart(&header, sizeof header);
    I_QuickSaveInternal(&header, size, I_USBWriteFunc);
    I_USBSendEnd(size);

    PendingUSBOperations &= ~USB_OP_QUICKSAVE;
}
#endif /* USB */

void G_DoLoadLevel (void);

// only include error messages with USB loading
#ifdef LOGGING
#define LOAD_ERROR(s) s
#else
#define LOAD_ERROR(s) ""
#endif

const char *I_QuickLoadInternal(int limit, void (*read)(u32, void*, u32))
{
    quicksaveheader_t    header    ALIGNED(16);
    byte                 buf[4096] ALIGNED(16);
    register savephase_e phase = phase_sectors;
    thinker_t   *currentthinker, *nextthinker;
    mobj_t      *currentmo, *nextmo;
    u32          i;
    u16          crc = CRC16_INIT;
    register u32 addr = QUICKSAVE_ADDR;
    register u32 count;
    register u32 playercount;
    register u32 left;

    if (limit < sizeof header)
        return LOAD_ERROR("Not enough data");

    read(addr, &header, sizeof header);
    addr += sizeof header;

    if (header.magic != QUICKSAVE_MAGIC || header.size < sizeof header || header.size > limit)
        return LOAD_ERROR("Bad header");

    left = header.size - sizeof header;
    while (left > 0)
    {
        u32 o = MIN(left, sizeof buf);
        read(addr, buf, o);
        Crc16(buf, o, &crc);
        addr += o;
        left -= o;
    }

    if (crc != header.crc)
        return LOAD_ERROR("Bad CRC");

    G_InitNew(header.skill, header.map, header.gametype);
    P_SetupLevel(gamemap);

    if (header.sectors != numsectors || header.lines != numlines || header.macros != nummacros)
        return LOAD_ERROR("Wrong map");

    addr = QUICKSAVE_ADDR + sizeof header;
    left = header.size - sizeof header;

    rndindex = header.rndindex;
    prndindex = header.prndindex;
    irndindex = header.irndindex;

    // remove all the current thinkers and mobjs
    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        nextthinker = currentthinker->next;
        Z_Free (currentthinker);
        currentthinker = nextthinker;
    }
    thinkercap.prev = thinkercap.next = &thinkercap;

    currentmo = mobjhead.next;
    while (currentmo != (void*) &mobjhead)
    {
        nextmo = currentmo->next;
        P_RemoveMobj (currentmo);
        currentmo = nextmo;
    }
    mobjhead.next = mobjhead.prev = (void*) &mobjhead;

    P_UnArchiveActiveMacro(&header.activemacro);

    crc = CRC16_INIT;

    playercount = header.player1 + header.player2 + header.player3 + header.player4;
    //playeringame[0] = header.player1;
    //playeringame[1] = header.player2;
    //playeringame[2] = header.player3;
    //playeringame[3] = header.player4;
    read(addr, buf, ALIGN(sizeof(player_t), 8) * playercount);
    count = P_UnArchivePlayers (buf);
    addr += count;
    left -= count;

    i = 0;
    while (1)
    {
        u32 o = MIN(left, sizeof buf);

        read(addr, buf, o);

        switch (phase)
        {
        case phase_sectors:
            count = P_UnArchiveSectors(buf, o, &i);
            break;
        case phase_lines:
            count = P_UnArchiveLines(buf, o, &i);
            break;
        case phase_macros:
            count = P_UnArchiveMacros(buf, o, &i);
            break;
        case phase_mobjs:
            count = P_UnArchiveMobjs(buf, o, &header.mobjs);
            break;
        case phase_thinkers:
            count = P_UnArchiveThinkers(buf, o, &header.thinkers);
            break;
        }
        if (count == 0)
        {
            if (phase == phase_thinkers)
            {
                break;
            }
            else
            {
                i = 0;
                phase += 1;
                continue;
            }
        }

        addr += count;
        left -= count;
    }

    if (header.mobjs > 0 || header.thinkers > 0)
        return LOAD_ERROR("Wrong map");

    P_LinkUnArchivedMobjs();
    P_FinishSetupLevel();
    ST_InitEveryLevel();
    ST_UpdateFlash();

    return NULL;
}

static void I_SramReadFunc(u32 addr, void* buf, u32 size) {
    I_ReadWriteSram(addr, buf, size, OS_READ);
}

static void I_DeleteQuickLoad(void);

static boolean I_QuickLoadSram(void)
{
    const char *err;
    if (!SramPresent)
        return false;

    err = I_QuickLoadInternal(QUICKSAVE_SIZE, I_SramReadFunc);

    if (err != NULL)
    {
        I_DeleteQuickLoad();
        D_printf("Quick Load Failed: %s", err);
        return false;
    }

    if (customskill.permadeath)
        I_DeleteQuickLoad();

    return true;
}

#ifdef USB
static u8 *QuickLoadBuffer = NULL;

static void I_USBReadFunc(u32 addr, void* buf, u32 size) {
    D_memcpy(buf, QuickLoadBuffer + addr, size);
}

boolean I_QuickLoad(void)
{
    if (!(PendingUSBOperations & USB_OP_QUICKLOAD))
        return I_QuickLoadSram();

    PendingUSBOperations &= ~USB_OP_QUICKLOAD;

    u32 size;
    const char *err;

    /* make space for the buffer */
    Z_FreeTags(mainzone, ~PU_STATIC);
    Z_CheckZone(mainzone);

    size = I_CmdNextTokenSize();
    QuickLoadBuffer = Z_Alloc(size, PU_STATIC, NULL);
    I_CmdGetNextToken(QuickLoadBuffer);
    I_CmdSkipAllTokens();

    err = I_QuickLoadInternal(size, I_USBReadFunc);
    Z_Free(QuickLoadBuffer);
    QuickLoadBuffer = NULL;

    if (err != NULL)
    {
        D_printf("Quick Load Failed: %s", err);
        return false;
    }

    return true;
}

#else /* USB */
boolean I_QuickLoad(void)
{
    return I_QuickLoadSram();
}

#endif /* USB */

static void I_DeleteQuickLoad(void)
{
    u64 empty = 0;

    if (!SramPresent)
        return;

    I_ReadWriteSram(QUICKSAVE_ADDR, &empty, sizeof empty, OS_WRITE);
    QuickLoadAvailable = false;
}

boolean I_IsQuickSaveAvailable(void)
{
    return SramPresent && P_CurrentQuickSaveSize(QUICKSAVE_SIZE) <= QUICKSAVE_SIZE
        && players[0].playerstate == PST_LIVE;
}

boolean I_IsQuickLoadAvailable(void)
{
    return QuickLoadAvailable;
}

static void I_DetectQuickLoad(void)
{
    quicksaveheader_t header ALIGNED(16);

    I_ReadWriteSram(QUICKSAVE_ADDR, &header, sizeof header, OS_READ);

    QuickLoadAvailable = header.magic == QUICKSAVE_MAGIC
        && header.size >= sizeof header && header.size <= QUICKSAVE_SIZE;
}
