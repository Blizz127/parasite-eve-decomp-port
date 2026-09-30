/*
 * HOST_ADAPTED STR stream feeder — see pe_str_feed.h.
 *
 * Ring layout (StSetRing, func_8007A214(base, count)): `count` 32-byte
 * records at D_800C0DC8, then `count` 2016-byte data slots.  A frame of
 * n chunks occupies records r..r+n-1 (each holds that chunk's 32-byte STR
 * sector header, raw+24) and data slots r..r+n-1 (raw+56, 2016 bytes).
 * Record r's status halfword (+0) = 2 publishes the frame; func_8007C484
 * promotes it to 4 and returns data(r)/record(r); func_8007C394 clears the
 * n records (count = record +6) and advances D_800BE9EC.  Status 1 is the
 * wrap marker (func_8007C484 resets the reader to record 0).
 */
#include "pe_str_feed.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "game_port.h"
#include "host_framebuffer.h"
#include "pe_disc.h"
#include "stub_registry.h"
#include "pe_spu.h"
#include "pe_gpu.h"
#include "pe_cdreg.h"
#include "pe_spu_dma.h"

#define GA_RING_BASE    0x800C0DC8u
#define GA_RING_COUNT   0x800C20C4u
#define GA_RING_ACTIVE  0x800BE9ECu

#define STR_CHUNK_BYTES 2016u
#define STR_MAX_CHUNKS  32u

static struct {
    int active;
    uint32_t mode;
    uint32_t lba;
    uint32_t credits;       /* half-sectors: +5 per VBlank, -2 per sector */
    uint32_t write_idx;
    int last_presented;
    uint32_t last_vblank;   /* PE_GPU vsync_count at the last account */
    /* frame under assembly */
    int assembling;
    uint32_t frame;
    uint32_t chunks;
    uint32_t mask;
    int pending;            /* complete frame waiting for ring space */
    pe_addr_t published;    /* first record of the newest published frame */
    uint8_t headers[STR_MAX_CHUNKS][32];
    uint8_t data[STR_MAX_CHUNKS][STR_CHUNK_BYTES];
    PeStrFeedStats stats;
    uint8_t atv_logged[4];  /* PE_FMV_LOG: last ATV matrix reported */
} g_feed;

static uint32_t VBlankCount(void)
{
    PeGpuState st;

    PE_GPU_GetState(&st);
    return st.vsync_count;
}

/* Credit drive time for every VBlank since the last account (the guest's
 * own VSync(4) movie pacing included) and, while a stream is open, keep
 * one host present per VBlank so the window pacer (one present = 1/60 s)
 * runs the movie and its XA audio at real time. */
static void Account(void)
{
    uint32_t now = VBlankCount();
    uint32_t dv = now - g_feed.last_vblank;
    int presented;
    uint32_t dp;

    g_feed.last_vblank = now;
    if (dv == 0u)
        return;
    if (dv > 600u)
        dv = 600u;
    g_feed.credits += 5u * dv;
    if (g_feed.credits > 5u * 120u) /* at most ~2 s of drive read-ahead */
        g_feed.credits = 5u * 120u;
    g_feed.stats.vblanks += dv;
    HostFB_GetState(NULL, NULL, &presented, NULL);
    dp = (uint32_t)(presented - g_feed.last_presented);
    while (dp < dv && !PE_Port_ShouldStop()) {
        HostFB_Present();
        dp++;
    }
    HostFB_GetState(NULL, NULL, &g_feed.last_presented, NULL);
}

static int FmvLog(void)
{
    static int on = -1;
    if (on < 0)
        on = getenv("PE_FMV_DEBUG") != NULL || getenv("PE_FMV_LOG") != NULL;
    return on;
}

/* PE_FMV_LOG evidence: FNV-1a over a published frame's bitstream (all its
 * 2016-byte chunks), so the log names the frame content independently of
 * the decoder; PE_FMV_SHOT_DIR=<dir> also writes the presented picture at
 * a few frame counts (debug evidence only; keep it under build/). */
static uint32_t FrameHash(uint32_t n)
{
    uint32_t h = 2166136261u, k, i;

    for (k = 0u; k < n; k++)
        for (i = 0u; i < STR_CHUNK_BYTES; i++)
            h = (h ^ g_feed.data[k][i]) * 16777619u;
    return h;
}

static void FrameEvidence(void)
{
    uint32_t n = g_feed.stats.frames_published;
    const char *dir;

    if (!FmvLog())
        return;
    if (n == 1u)
        fprintf(stderr, "[FMV] first frame start_lba=%u frame=%u chunks=%u fnv=%08X\n",
                g_feed.stats.start_lba, g_feed.frame, g_feed.chunks,
                FrameHash(g_feed.chunks));
    dir = getenv("PE_FMV_SHOT_DIR");
    if (dir != NULL && (n == 3u || n == 60u || n == 300u || n == 900u)) {
        char path[512];
        snprintf(path, sizeof path, "%s/fmv_lba%u_f%04u.ppm", dir,
                 g_feed.stats.start_lba, n);
        if (HostFB_WritePPM(path) == 0)
            fprintf(stderr, "[FMV] shot %s\n", path);
    }
}

static uint32_t Bcd(uint8_t b) { return (uint32_t)(b >> 4) * 10u + (b & 15u); }

static int IsStreamSector(const uint8_t *raw)
{
    const uint8_t *user = raw + 24;

    if (raw[15] != 2u)
        return 0;
    if (raw[18] & 0x04u)
        return 1; /* XA audio */
    return (raw[18] & 0x08u) && user[0] == 0x60u && user[1] == 0x01u &&
           user[2] == 0x01u && user[3] == 0x80u;
}

int PE_StrFeed_Probe(pe_addr_t bcd_location)
{
    PE_Disc *disc = PE_Disc_GetActive();
    uint8_t raw[PE_DISC_RAW_SECTOR];
    uint32_t msf;

    if (disc == NULL || !PE_RangeIsRam(bcd_location, 3u))
        return 0;
    msf = (Bcd(PE_LoadU8(bcd_location)) * 60u +
           Bcd(PE_LoadU8(bcd_location + 1u))) * 75u +
          Bcd(PE_LoadU8(bcd_location + 2u));
    if (msf < 150u || msf - 150u >= PE_Disc_UserSectorCount(disc) ||
        !PE_Disc_ReadRawSector(disc, msf - 150u, raw))
        return 0;
    return IsStreamSector(raw);
}

void PE_StrFeed_Reset(void)
{
    memset(&g_feed, 0, sizeof(g_feed));
}

void PE_StrFeed_Open(pe_addr_t bcd_location, uint32_t mode)
{
    uint32_t msf = (Bcd(PE_LoadU8(bcd_location)) * 60u +
                    Bcd(PE_LoadU8(bcd_location + 1u))) * 75u +
                   Bcd(PE_LoadU8(bcd_location + 2u));
    uint32_t count = PE_LoadU32(GA_RING_COUNT);
    uint32_t active = PE_LoadU32(GA_RING_ACTIVE);

    memset(&g_feed.stats, 0, sizeof(g_feed.stats));
    g_feed.active = 1;
    g_feed.mode = mode;
    g_feed.lba = msf >= 150u ? msf - 150u : 0u;
    g_feed.credits = 0u;
    g_feed.write_idx = active < count ? active : 0u;
    g_feed.assembling = 0;
    g_feed.pending = 0;
    g_feed.published = 0u;
    memset(g_feed.atv_logged, 0xFF, sizeof(g_feed.atv_logged));
    HostFB_GetState(NULL, NULL, &g_feed.last_presented, NULL);
    g_feed.last_vblank = VBlankCount();
    g_feed.stats.start_lba = g_feed.lba;
    g_feed.stats.lba = g_feed.lba;
    PE_Spu_CdInputStop();
    Stub_Record("PE_StrFeed_ReadS_host_stream", "HOST_ADAPTED");
    if (FmvLog()) {
        uint8_t atv[4];
        PE_CdReg_GetAudioVolumes(atv);
        fprintf(stderr, "[FMV] stream open lba=%u mode=%03X atv=%02X,%02X,%02X,%02X spu 180=%04X 182=%04X 1B0=%04X 1B2=%04X 1AA=%04X\n",
                g_feed.lba, mode, atv[0], atv[1], atv[2], atv[3],
                PE_SpuRegister_LoadU16(0x180u), PE_SpuRegister_LoadU16(0x182u),
                PE_SpuRegister_LoadU16(0x1B0u), PE_SpuRegister_LoadU16(0x1B2u),
                PE_SpuRegister_LoadU16(0x1AAu));
    }
}

void PE_StrFeed_Close(void)
{
    if (!g_feed.active)
        return;
    if (FmvLog())
        fprintf(stderr, "[FMV] stream close start_lba=%u at_vblank=%u lba=%u frames=%u last=%u video=%u audio=%u vblanks=%u dropped=%u\n", g_feed.stats.start_lba, VBlankCount(), g_feed.lba, g_feed.stats.frames_published, g_feed.stats.last_frame, g_feed.stats.video_sectors, g_feed.stats.audio_sectors, g_feed.stats.vblanks, g_feed.stats.frames_dropped);
    g_feed.active = 0;
    /* Natural end and skip alike: stop the XA already queued in the SPU
     * CD input so skipped movie audio does not play on into the game. */
    PE_Spu_CdInputStop();
    g_feed.pending = 0;
    g_feed.assembling = 0;
}

int PE_StrFeed_Active(void) { return g_feed.active; }

void PE_StrFeed_GetStats(PeStrFeedStats *out)
{
    *out = g_feed.stats;
    out->lba = g_feed.lba;
}

/* Place the pending frame into the ring.  Returns 1 when published. */
static int PlaceFrame(void)
{
    pe_addr_t base = PE_LoadU32(GA_RING_BASE);
    uint32_t count = PE_LoadU32(GA_RING_COUNT);
    uint32_t n = g_feed.chunks;
    uint32_t w = g_feed.write_idx;
    uint32_t k;

    if (base == 0u || count < 2u || n == 0u || n >= count)
        return 0;
    if (w >= count)
        w = 0u;
    if (w + n >= count) {
        /* Wrap marker: only once the reader has left that record. */
        if (w != 0u) {
            if (PE_LoadU16(base + w * 32u) != 0u)
                return 0;
            PE_StoreU16(base + w * 32u, 1u);
        }
        w = 0u;
    }
    for (k = 0u; k < n; k++)
        if (PE_LoadU16(base + (w + k) * 32u) != 0u)
            return 0;
    for (k = 0u; k < n; k++) {
        pe_addr_t rec = base + (w + k) * 32u;
        pe_addr_t dat = base + count * 32u + (w + k) * STR_CHUNK_BYTES;

        memcpy(PE_Translate(dat, STR_CHUNK_BYTES), g_feed.data[k],
               STR_CHUNK_BYTES);
        memcpy(PE_Translate(rec, 32u), g_feed.headers[k], 32u);
        PE_StoreU16(rec + 6u, (uint16_t)n);
        PE_StoreU16(rec, k == 0u ? 2u : 3u);
    }
    /* Status of record w is written last so a reader never sees a
     * half-copied frame. */
    PE_StoreU16(base + w * 32u, 2u);
    g_feed.published = base + w * 32u;
    g_feed.write_idx = w + n;
    g_feed.pending = 0;
    g_feed.stats.frames_published++;
    FrameEvidence();
    return 1;
}

static void Sector(const uint8_t *raw)
{
    uint8_t submode = raw[18];
    const uint8_t *user = raw + 24;

    g_feed.stats.sectors++;
    if (submode & 0x04u) {
        g_feed.stats.audio_sectors++;
        /* XA-ADPCM real-time playback (mode 0x40) through the audio
         * lane's SPU CD input; mode 0x1E0 has no subheader filter. */
        /* CD drive audio volume matrix (ATV0..3: L->L, L->R, R->R, R->L;
         * 0x80 = 100%), as the game last applied it through CdMix
         * (func_8007B964 via func_800870F0 / func_80080AC4: ADPCTL bit5 on
         * index 3).  pe_cdreg latches it whether or not the CD register
         * device is enabled, so movie levels and the players' ATV fades
         * (func_80191B64 / func_80192CE8 / 0x80122354) follow retail. */
        uint8_t atv[4];
        PE_CdReg_GetAudioVolumes(atv);
        if (FmvLog() && memcmp(atv, g_feed.atv_logged, 4) != 0) {
            fprintf(stderr, "[FMV] atv lba=%u frame=%u: %02X,%02X,%02X,%02X\n",
                    g_feed.stats.start_lba, g_feed.stats.frames_published,
                    atv[0], atv[1], atv[2], atv[3]);
            memcpy(g_feed.atv_logged, atv, 4);
        }
        PE_Spu_CdSector(raw, (uint8_t)g_feed.mode, 0, 0u, 0u, atv);
        return;
    }
    if ((submode & 0x08u) && user[0] == 0x60u && user[1] == 0x01u &&
        user[2] == 0x01u && user[3] == 0x80u) {
        uint32_t idx = (uint32_t)user[4] | ((uint32_t)user[5] << 8);
        uint32_t n = (uint32_t)user[6] | ((uint32_t)user[7] << 8);
        uint32_t frame = (uint32_t)user[8] | ((uint32_t)user[9] << 8) |
                         ((uint32_t)user[10] << 16) |
                         ((uint32_t)user[11] << 24);

        g_feed.stats.video_sectors++;
        if (n == 0u || n > STR_MAX_CHUNKS || idx >= n)
            return;
        if (g_feed.assembling &&
            (frame != g_feed.frame || n != g_feed.chunks)) {
            g_feed.stats.frames_dropped++;
            g_feed.assembling = 0;
        }
        if (!g_feed.assembling) {
            g_feed.assembling = 1;
            g_feed.frame = frame;
            g_feed.chunks = n;
            g_feed.mask = 0u;
        }
        memcpy(g_feed.headers[idx], user, 32u);
        memcpy(g_feed.data[idx], user + 32, STR_CHUNK_BYTES);
        g_feed.mask |= 1u << idx;
        if (g_feed.mask == (n >= 32u ? 0xFFFFFFFFu : (1u << n) - 1u)) {
            g_feed.assembling = 0;
            g_feed.pending = 1;
            g_feed.stats.last_frame = frame;
        }
        return;
    }
    g_feed.stats.other_sectors++;
}

static void VBlank(void)
{
    (void)HostFB_VSync(0);
    if (PE_Port_ShouldStop())
        return;
    Account();
}

void PE_StrFeed_Pump(void)
{
    PE_Disc *disc = PE_Disc_GetActive();
    uint8_t raw[PE_DISC_RAW_SECTOR];
    unsigned guard;

    if (!g_feed.active || disc == NULL)
        return;
    /* A published frame the reader has not taken yet: nothing to do. */
    if (g_feed.published != 0u &&
        PE_LoadU16(g_feed.published) == 2u)
        return;
    Account();
    for (guard = 0u; guard < 200000u; guard++) {
        if (g_feed.pending) {
            if (PlaceFrame())
                return;
            /* Ring full: the reader still holds every free record. */
            return;
        }
        if (g_feed.credits < 2u) {
            VBlank();
            if (PE_Port_ShouldStop())
                return;
            continue;
        }
        if (g_feed.lba >= PE_Disc_UserSectorCount(disc) ||
            !PE_Disc_ReadRawSector(disc, g_feed.lba, raw)) {
            PE_StrFeed_Close();
            return;
        }
        g_feed.lba++;
        g_feed.credits -= 2u;
        Sector(raw);
    }
}
