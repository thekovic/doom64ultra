#include "doomdef.h"
#include "p_local.h"
#include "p_saveg.h"
#include "r_local.h"
#include "st_main.h"

boolean SramPresent = false;
static u32 SramSize = 0x20000;

#define SRAM_MAGIC 0x44363455 // 'D64U'
#define QUICKSAVE_MAGIC 0x44363453 // 'D64S'
#define SRAM_DATA_VERSION 0

#define HEADER_ADDR 0
#define CONFIG_ADDR 8
#define SAVE_ADDR (CONFIG_ADDR + sizeof(config_t))
#define QUICKSAVE_ADDR (SAVE_ADDR + sizeof(levelsave_t) * MAXSRAMSAVES)
#define FOOTER_ADDR (SramSize - 8)
#define QUICKSAVE_SIZE (SramSize - QUICKSAVE_ADDR - 8)

static OSPiHandle SramHandle __attribute__((aligned(8)));

#define SRAM_START_ADDR  0x08000000
#define SRAM_latency     0x5
#define SRAM_pulse       0x0c
#define SRAM_pageSize    0xd
#define SRAM_relDuration 0x2

static boolean I_LoadConfig(void);

static void ReadWriteSram(u32 addr, void* buf, u32 size, s32 flag)
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

    // make multiple transfers when (addr + size) crosses bank boundary
    while (size > 0)
    {
        base = addr & 0x7fff;

        // bank select in bits 18-19
        msgbuf.devAddr  = base + ((addr & 0x18000) << 3);
        msgbuf.size     = (base + size >= 0x8000) ? 0x8000 - base : size;
        msgbuf.dramAddr = buf;

        osEPiStartDma(&SramHandle, &msgbuf, flag);
        osRecvMesg(&queue, &msg, OS_MESG_BLOCK);

        buf += msgbuf.size;
        addr += msgbuf.size;
        size -= msgbuf.size;
    }
}

typedef struct  __attribute__((aligned(8))) {
    u32 magic;
    u32 small: 1;
    u32 version: 31;
} sramheader_t;

void I_InitSram(void)
{
    sramheader_t header;
    sramheader_t empty = { 0, 0, 0};

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
    ReadWriteSram(0, &header, sizeof header, OS_READ);
    if (header.magic == SRAM_MAGIC && header.version == SRAM_DATA_VERSION)
    {
        ReadWriteSram(FOOTER_ADDR, &header, sizeof header, OS_READ);
        if (header.magic == SRAM_MAGIC && header.version == SRAM_DATA_VERSION)
        {
            if (header.small)
                SramSize = 0x8000;
            SramPresent = true;
        }
    }

    // could be uninitialized, try to initialize and then check again
    if (!SramPresent)
    {
        header.magic = SRAM_MAGIC;
        header.version = SRAM_DATA_VERSION;
        header.small = 0;
        ReadWriteSram(0, &header, sizeof header, OS_WRITE);
        ReadWriteSram(0, &header, sizeof header, OS_READ);
        if (header.magic == SRAM_MAGIC && header.version == SRAM_DATA_VERSION)
        {
            ReadWriteSram(0x8000 - 8, &empty, sizeof empty, OS_WRITE);
            ReadWriteSram(FOOTER_ADDR, &header, sizeof header, OS_WRITE);
            ReadWriteSram(FOOTER_ADDR, &header, sizeof header, OS_READ);
            if (header.magic == SRAM_MAGIC && header.version == SRAM_DATA_VERSION)
            {
                // check if the write was mirrored to the low bank
                ReadWriteSram(0x8000 - 8, &header, sizeof header, OS_READ);
                if (header.magic == SRAM_MAGIC && header.version == SRAM_DATA_VERSION)
                {
                    SramSize = 0x8000;
                    header.small = 1;
                    ReadWriteSram(0, &header, sizeof header, OS_WRITE);
                    ReadWriteSram(FOOTER_ADDR, &header, sizeof header, OS_WRITE);
                }
                SramPresent = true;
            }
        }
    }

    if (SramPresent && !I_LoadConfig())
        I_SaveConfig();
}

typedef struct __attribute__((__packed__)) __attribute__((aligned (8))) {
    u16 crc;
    u32 brightness: 8;
    u32 displayx: 6;
    u32 displayy: 6;
    u32 texturefilter: 1;
    u32 spritefilter: 1;
    u32 skyfilter: 1;
    u32 tvmode: 2;
    u32 screenaspect: 2;
    u32 ditherfilter: 1;
    u32 colordither: 2;
    u32 flashbrightness: 6;
    u32 motionbob: 6;
    u32 storytext: 1;
    u32 mapstats: 1;
    u32 enablemessages: 1;
    u32 coloredhud: 1;
    u32 hudmargin: 5;
    u32 hudopacity: 8;
    u32 sfxvolume: 7;
    u32 musvolume: 7;

    savedplayerconfig_t player __attribute__((aligned (4)));
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

    config_t config __attribute__((aligned(16)));

    bzero(&config, sizeof config);

    config.brightness = brightness;
    config.displayx = Display_X + 16;
    config.displayx = Display_Y + 20;
    config.texturefilter = VideoFilters[0];
    config.spritefilter = VideoFilters[1];
    config.skyfilter = VideoFilters[2];
    config.tvmode = TvMode;
    config.screenaspect = ScreenAspect;
    config.ditherfilter = DitherFilter;
    config.colordither = ColorDither;
    config.flashbrightness = FlashBrightness;
    config.motionbob = MotionBob >> 15;
    config.storytext = StoryText;
    config.mapstats = MapStats;
    config.enablemessages = enable_messages;
    config.coloredhud = ColoredHUD;
    config.hudmargin = HUDmargin;
    config.hudopacity = HUDopacity;
    config.sfxvolume = SfxVolume;
    config.musvolume = MusVolume;

    P_ArchivePlayerConfig(0, &config.player);

    config.crc = CRC16_INIT;
    Crc16(((u8*)&config)+sizeof(u16), sizeof(config_t)-sizeof(u16), &config.crc);
    ReadWriteSram(CONFIG_ADDR, &config, sizeof config, OS_WRITE);
}

extern void P_RefreshBrightness(void);
extern void P_RefreshVideo(void);

static boolean I_LoadConfig(void)
{
    config_t config __attribute__((aligned(16)));
    u16 crc = CRC16_INIT;

    ReadWriteSram(CONFIG_ADDR, &config, sizeof config, OS_READ);

    Crc16(((u8*)&config)+sizeof(u16), sizeof(config_t)-sizeof(u16), &crc);
    if (config.crc != crc)
        return false;

    brightness = config.brightness % 201;
    Display_X = ((int)config.displayx) % 41 - 16;
    Display_Y = ((int)config.displayx) % 33 - 20;
    VideoFilters[0] = config.texturefilter;
    VideoFilters[1] = config.spritefilter;
    VideoFilters[2] = config.skyfilter;
    TvMode = config.tvmode;
    ScreenAspect = config.screenaspect % 3;
    DitherFilter = config.ditherfilter;
    ColorDither = config.colordither;
    FlashBrightness = config.flashbrightness % 33;
    MotionBob = (config.motionbob % 33) << 15;
    StoryText = config.storytext;
    MapStats = config.mapstats;
    enable_messages = config.enablemessages;
    ColoredHUD = config.coloredhud;
    HUDmargin = config.hudmargin % 21;
    HUDopacity = config.hudopacity;
    SfxVolume = config.sfxvolume % 101;
    MusVolume = config.musvolume % 101;

    P_UnArchivePlayerConfig(0, &config.player);

    P_RefreshBrightness();
    P_RefreshVideo();
    I_MoveDisplay(Display_X, Display_Y);
    S_SetSoundVolume(SfxVolume);
    S_SetMusicVolume(MusVolume);

    return true;
}

void I_SaveProgress(levelsave_t *save)
{
    int bit = 0;

    bzero(save, sizeof *save);

    save->present = 1;
    save->map = nextmap;
    save->skill = gameskill;
    for(int i = 0; i < NUMWEAPONS; i++)
    {
        if(i != wp_fist && i != wp_pistol)
        {
            if(players[0].weaponowned[bit])
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

    ReadWriteSram(SAVE_ADDR + index * sizeof *save, (void*)save, sizeof *save, OS_WRITE);
}

void I_ReadSramSaves(levelsave_t *saves)
{
    u16 crc;
    int i;

    ReadWriteSram(SAVE_ADDR, saves, sizeof(levelsave_t) * MAXSRAMSAVES, OS_READ);
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
                players[0].weaponowned[i] = true;
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

extern int start_time;

typedef struct __attribute__((aligned(8)))
{
    u32 magic;
    u32 size;
    u16 crc;
    u32 sectors;
    u32 lines;
    u32 macros;
    u32 mobjs;
    u32 thinkers;
    u32 map: 15;
    u32 skill: 3;
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

void I_QuickSave(void)
{
    quicksaveheader_t header __attribute__((aligned(16)));
    byte buf[8192] __attribute__((aligned(16)));
    savephase_e phase = phase_sectors;
    u32 addr = QUICKSAVE_ADDR + sizeof header;
    u32 count;
    u32 i;
    mobj_t *mobj;
    thinker_t *th;

    if (!SramPresent)
        return;

    bzero(&header, sizeof header);

    header.magic = QUICKSAVE_MAGIC;
    header.sectors = numsectors;
    header.lines = numlines;
    header.macros = nummacros;
    header.mobjs = 0;
    header.thinkers = 0;
    header.skill = gameskill;
    header.gametype = gt_single;
    header.map = gamemap;
    header.player1 = 1;
    //header.player1 = playeringame[0];
    //header.player2 = playeringame[1];
    //header.player3 = playeringame[2];
    //header.player4 = playeringame[3];
    header.leveltime = ticon - start_time;
    header.rndindex = rndindex;
    header.prndindex = prndindex;
    header.irndindex = irndindex;

    P_ArchiveActiveMacro(&header.activemacro);

    header.crc = CRC16_INIT;
    header.size += sizeof header;

    count = P_ArchivePlayers(buf);
    ReadWriteSram(addr, buf, count, OS_WRITE);

    Crc16(buf, count, &header.crc);
    addr += count;
    header.size += count;

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
            count = P_ArchiveMobjs(buf, sizeof buf, &mobj, &header.mobjs);
            break;
        case phase_thinkers:
            count = P_ArchiveThinkers(buf, sizeof buf, &th, &header.thinkers);
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

        if (addr + count >= FOOTER_ADDR)
            I_Error("SRAM overflow");

        Crc16(buf, count, &header.crc);
        ReadWriteSram(addr, buf, count, OS_WRITE);
        addr += count;
        header.size += count;
    }

    // write header last after calculating size and crc
    ReadWriteSram(QUICKSAVE_ADDR, &header, sizeof header, OS_WRITE);
}

void G_DoLoadLevel (void);

boolean I_QuickLoad(void)
{
    quicksaveheader_t header __attribute__((aligned(16)));
    thinker_t *currentthinker, *nextthinker;
    mobj_t    *currentmo, *nextmo;
    savephase_e phase = phase_sectors;
    byte buf[8192] __attribute__((aligned(16)));
    u16        crc = CRC16_INIT;
    u32        addr = QUICKSAVE_ADDR;
    u32        count;
    u32        playercount;
    u32        left;
    u32        i;

    if (!SramPresent)
        return false;

    ReadWriteSram(addr, &header, sizeof header, OS_READ);
    addr += sizeof header;

    if (header.magic != QUICKSAVE_MAGIC || header.size < sizeof header || header.size > QUICKSAVE_SIZE)
        return false;

    left = header.size - sizeof header;
    while (left > 0)
    {
        u32 o = MIN(left, sizeof buf);
        ReadWriteSram(addr, buf, o, OS_READ);
        Crc16(buf, o, &crc);
        addr += o;
        left -= o;
    }

    if (crc != header.crc)
        return false;

    G_InitNew(header.skill, header.map, header.gametype);
	P_SetupLevel(gamemap, gameskill);

    if (header.sectors != numsectors || header.lines != numlines || header.macros != nummacros)
        return false;

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
    while (currentmo != &mobjhead)
    {
        nextmo = currentmo->next;
        P_RemoveMobj (currentmo);
        currentmo = nextmo;
    }
    mobjhead.next = mobjhead.prev = &mobjhead;

    P_UnArchiveActiveMacro(&header.activemacro);

    playercount = header.player1 + header.player2 + header.player3 + header.player4;
    //playeringame[0] = header.player1;
    //playeringame[1] = header.player2;
    //playeringame[2] = header.player3;
    //playeringame[3] = header.player4;
    ReadWriteSram(addr, buf, ALIGN(sizeof(player_t), 8) * playercount, OS_READ);
    count = P_UnArchivePlayers (buf);
    addr += count;
    left -= count;

    i = 0;
    while (1)
    {
        u32 o = MIN(left, sizeof buf);

        ReadWriteSram(addr, buf, o, OS_READ);

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
        return false;

    P_LinkUnArchivedMobjs();
    P_FinishSetupLevel();
    ST_InitEveryLevel();
    ST_UpdateFlash();

    return true;
}

void I_DeleteQuickLoad(void)
{
    u64 empty = 0;

    if (!SramPresent)
        return;

    ReadWriteSram(QUICKSAVE_ADDR, &empty, sizeof empty, OS_WRITE);
}

boolean I_IsQuickSaveAvailable(void)
{
    return SramPresent && P_CurrentQuickSaveSize(QUICKSAVE_SIZE) <= QUICKSAVE_SIZE;
}

boolean I_IsQuickLoadAvailable(void)
{
    quicksaveheader_t header __attribute__((aligned(16)));

    if (!SramPresent)
        return false;

    ReadWriteSram(QUICKSAVE_ADDR, &header, sizeof header, OS_READ);

    return header.magic == QUICKSAVE_MAGIC && header.size >= sizeof header && header.size <= QUICKSAVE_SIZE;
}

