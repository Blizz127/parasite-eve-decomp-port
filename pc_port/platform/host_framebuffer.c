/*
 * Phase 6A — Host framebuffer (headless, no SDL2/OpenGL).
 *
 * 320×240 logical pixels, RGB 8:8:8 per channel.
 * Deterministic: no GPU, no vsync, no timing jitter.
 * Output: PPM (Portable Pixmap) file on request.
 */

#include "pe_rcnt2.h"
#include "pe_mmio.h"
#include "host_framebuffer.h"
#include "game_port.h"
#include "host_vram.h"
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "pe_spu_dma.h"
#include "pe_sdk.h"
#include "pe_cdreg.h"
#include "pe_mdec.h"
#include "pe_irq_delivery.h"
#include "pe_irq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ── framebuffer state ───────────────────────────────────────────────── */

static uint8_t fb[PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * 3];  /* RGB */
static uint8_t *fb_presentation;
static int fb_presentation_scale = 1;
static uint64_t fb_presentation_generation;
static int fb_ssaa;
static int fb_presentation_ssaa;
static int fb_presentation_movie;   /* last scan-out was a 24-bit movie frame */
static uint8_t fb_antialias[PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * 3];

void HostFB_SetSSAA(int enabled) { fb_ssaa = enabled == 1; }
int HostFB_GetSSAA(void) { return fb_ssaa; }
static int     fb_presented = 0;
static int     fb_mask      = 0;   /* 0 = blanked, 1 = visible (SetDispMask) */
static uint32_t fb_device_cycles = 0;
static int     fb_vsync_count = 0;
static int     fb_drawsync_count = 0;
/* Host model of the original's timer1 scanline counter (guest 0x1F801110,
 * live symbol 0x80094578) and of the entry-scanline baseline at 0x8009457C.
 * The outer transition loop stores the mode-1 query result into 0x8019CC14,
 * so a void shim cannot preserve behavior. See docs/ai_context/
 * TRANSITION_OUTER_LOOP.md and VSYNC_CONTRACT.md. */
static uint32_t fb_vsync_timer = 0;      /* 0x80094578 (1F801110 low half) */
static uint32_t fb_vsync_baseline = 0;   /* 0x8009457C, 16-bit scanline latch */

/* Last PutDispEnv geometry, reused by the VBlank scan-out below. */
static struct { int valid; uint32_t dw, top, dh; int rgb24; } fb_scan;
static int  fb_idle_vblanks;
static void (*fb_scanout_hook)(void);
static uint32_t fb_scanouts;   /* idle-VBlank rescans (PE_FRAME_TRACE) */

/* ── public API ───────────────────────────────────────────────────────── */

void HostFB_Init(void)
{
    memset(fb, 0, sizeof(fb));
    free(fb_presentation); fb_presentation = NULL; fb_presentation_scale = 1;
    fb_presentation_movie = 0;
    fb_presented = 0;
    fb_mask      = 0;
    fb_vsync_count = 0;
    fb_device_cycles = 0;
    fb_drawsync_count = 0;
    fb_vsync_timer = 0;
    fb_vsync_baseline = 0;
    memset(&fb_scan, 0, sizeof(fb_scan));
    fb_idle_vblanks = 0;
}

void HostFB_ClearImage(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b)
{
    /* Clamp to framebuffer bounds */
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > PE_PORT_FB_WIDTH)  w = PE_PORT_FB_WIDTH  - x;
    if (y + h > PE_PORT_FB_HEIGHT) h = PE_PORT_FB_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    for (int row = y; row < y + h; row++) {
        uint8_t *dst = fb + (row * PE_PORT_FB_WIDTH + x) * 3;
        for (int col = 0; col < w; col++) {
            dst[0] = r;
            dst[1] = g;
            dst[2] = b;
            dst += 3;
        }
    }
}

void HostFB_Present(void)
{
    if (!PE_Port_FramePresentationAllowed()) return;
    fb_presented++;
    PE_Port_FramePresented(fb_presented);
    PE_Port_InvokePresentHook();
}

static void ScanDisplay(uint32_t dx, uint32_t dy, uint32_t dw, uint32_t top, uint32_t dh, int rgb24);

void HostFB_SetScanoutHook(void (*hook)(void)) { fb_scanout_hook = hook; }

/* Hardware scans the GP1(05h) display area out of VRAM on every frame, not
 * only when the game calls PutDispEnv.  Anything drawn into the displayed
 * buffer between swaps (retail's pause overlay, func_8006A0E8, is drawn once
 * after the last swap and then the tick only waits VSync(2)) is therefore on
 * screen on the PS1.  On a VBlank with no PutDispEnv this rescans the current
 * display start with the last PutDispEnv geometry.  Display only: it is not
 * a present (no present count, present hook, route clock or shot). */
/* Harness aid (off by default): PE_SHOT_VSYNC=<dir>:<first>:<step> writes
 * <dir>/v<vblank>.ppm of the displayed image every <step> emulated VBlanks
 * (PE_GPU_VSyncQuery) from <first>, also while the game presents nothing
 * (pause), which present-keyed shots cannot capture. */
static void HostFB_ShotVBlank(void)
{
    static int init, next, step; static char dir[400];
    int vblank;
    if (!init) {
        const char *e = getenv("PE_SHOT_VSYNC"); init = 1;
        if (!e || sscanf(e, "%399[^:]:%d:%d", dir, &next, &step) != 3 || step < 1) dir[0] = 0;
    }
    if (!dir[0]) return;
    vblank = (int)PE_GPU_VSyncQuery();
    if (vblank >= next) {   /* a VBlank crossed inside a query quantum still gets its shot */
        char path[512];
        snprintf(path, sizeof path, "%s/v%06d.ppm", dir, next);
        (void)HostFB_WritePPM(path);
        while (next <= vblank) next += step;
    }
}

static void HostFB_ScanOutIdleVBlank(void)
{
    PeGpuState st;
    if (++fb_idle_vblanks < 2 || !fb_scan.valid) return;
    if (!PE_Port_FramePresentationAllowed()) return;
    PE_GPU_GetState(&st);
    ScanDisplay(st.display_start & 1023u, (st.display_start >> 10) & 511u,
                fb_scan.dw, fb_scan.top, fb_scan.dh, fb_scan.rgb24);
    fb_scanouts++;
    if (fb_scanout_hook) fb_scanout_hook();
}

uint32_t HostFB_ScanOutCount(void) { return fb_scanouts; }

void HostFB_PresentDispEnv(pe_addr_t env)
{
    uint32_t dx = (uint32_t)PE_LoadU16(env);
    uint32_t dy = (uint32_t)PE_LoadU16(env + 2u);
    uint32_t dw = (uint32_t)PE_LoadU16(env + 4u);
    /* Original PutDispEnv 75858..75880 and 75A14..75A60: NTSC
     * screen.y is relative to scanline16; screen.h==0 means240 lines.
     * The source rectangle height selects the GPU mode, not this range. */
    int start = (int16_t)PE_LoadU16(env + 10u) + 16;
    int lines = (int16_t)PE_LoadU16(env + 14u);
    int end = start + (lines ? lines : 240);
    if (start < 16) start = 16;
    if (start > 257) start = 257;
    if (end < start + 2) end = start + 2;
    else if (end > 258) end = 258;
    uint32_t top = (uint32_t)(start - 16);
    uint32_t dh = (uint32_t)(end - start);

    if (!PE_Port_FramePresentationAllowed()) return;
    fb_scan.valid = 1; fb_scan.dw = dw; fb_scan.top = top; fb_scan.dh = dh;
    fb_scan.rgb24 = PE_LoadU8(env + 17u) != 0u;
    fb_idle_vblanks = 0;
    ScanDisplay(dx, dy, dw, top, dh, fb_scan.rgb24);
    fb_presented++;
    PE_Port_FramePresented(fb_presented);
    PE_Port_InvokePresentHook();
}

/* Separate scan-out keeps native screenshot/capture pixels unchanged. 24-bit
 * movie frames are nearest-expanded from their native packed-byte scan-out. */
static void ScanPresentation(uint32_t dx, uint32_t dy, uint32_t dw,
                             uint32_t top, uint32_t dh, int rgb24)
{
    int scale = PE_GPU_GetRenderScale();
    size_t bytes = (size_t)PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * scale * scale * 3u;
    fb_presentation_movie = rgb24;
    if (scale == 1) { fb_presentation_scale = 1; return; }
    /* A movie frame is decoded at 320x240: a larger copy adds nothing and
     * would cost the window's XRender path. */
    if (rgb24) return;
    if (fb_presentation_scale != scale || !fb_presentation) {
        uint8_t *next = realloc(fb_presentation, bytes);
        if (!next) { fb_presentation_scale = 1; return; }
        fb_presentation = next; fb_presentation_scale = scale;
    }
    fb_presentation_generation = PE_GPU_GetRenderGeneration();
    fb_presentation_ssaa = fb_ssaa;
    memset(fb_presentation, 0, bytes);
    if (!fb_mask) {
        if (fb_ssaa) memset(fb_antialias, 0, sizeof(fb_antialias));
        return;
    }
    if (dw > PE_PORT_FB_WIDTH) dw = PE_PORT_FB_WIDTH;
    for (uint32_t y = 0; y < PE_PORT_FB_HEIGHT * (uint32_t)scale; y++) {
        for (uint32_t x = 0; x < PE_PORT_FB_WIDTH * (uint32_t)scale; x++) {
            uint8_t *dst = fb_presentation + ((size_t)y * PE_PORT_FB_WIDTH * scale + x) * 3u;
            if (rgb24) {
                const uint8_t *src = fb + ((size_t)(y / scale) * PE_PORT_FB_WIDTH + x / scale) * 3u;
                memcpy(dst, src, 3u);
            } else if (y >= top * scale && y / scale - top < dh && x / scale < dw) {
                uint16_t pixel;
                if (PE_GPU_ReadRenderPixel(dx * scale + x, dy * scale + y - top * scale, &pixel))
                    HostVRAM_DecodePixel(pixel, dst);
            }
        }
    }

    if (fb_ssaa) {
        if (rgb24) memcpy(fb_antialias, fb, sizeof(fb_antialias));
        else for (unsigned int y = 0; y < PE_PORT_FB_HEIGHT; y++)
            for (unsigned int x = 0; x < PE_PORT_FB_WIDTH; x++) {
                unsigned int sum[3] = {0,0,0};
                for (int sy = 0; sy < scale; sy++)
                    for (int sx = 0; sx < scale; sx++) {
                        const uint8_t *src = fb_presentation +
                            (((size_t)y * scale + sy) * PE_PORT_FB_WIDTH * scale + x * scale + sx) * 3u;
                        for (unsigned int ch = 0; ch < 3; ch++) sum[ch] += src[ch];
                    }
                for (unsigned int ch = 0; ch < 3; ch++)
                    fb_antialias[((size_t)y * PE_PORT_FB_WIDTH + x) * 3u + ch] =
                        (uint8_t)((sum[ch] + scale * scale / 2) / (scale * scale));
            }
    }
}

int HostFB_WritePresentationPPM(const char *path)
{
    int width, height;
    const uint8_t *pixels = HostFB_GetPresentationPixels(&width, &height);
    FILE *file = fopen(path, "wb");
    if (!file) return -1;
    int failed = fprintf(file, "P6\n%d %d\n255\n", width, height) < 0;
    size_t bytes = (size_t)width * height * 3u;
    if (fwrite(pixels, 1, bytes, file) != bytes) failed = 1;
    if (fclose(file)) failed = 1;
    return failed ? -1 : 0;
}

const uint8_t *HostFB_GetPresentationPixels(int *width, int *height)
{
    if (fb_presentation_movie) {
        if (width) *width = PE_PORT_FB_WIDTH;
        if (height) *height = PE_PORT_FB_HEIGHT;
        return fb;
    }
    int scale = fb_presentation && fb_presentation_scale == PE_GPU_GetRenderScale() &&
                fb_presentation_generation == PE_GPU_GetRenderGeneration() &&
                fb_presentation_ssaa == fb_ssaa ?
                fb_presentation_scale : 1;
    if (scale > 1 && fb_ssaa) {
        if (width) *width = PE_PORT_FB_WIDTH;
        if (height) *height = PE_PORT_FB_HEIGHT;
        return fb_antialias;
    }
    if (width) *width = PE_PORT_FB_WIDTH * scale;
    if (height) *height = PE_PORT_FB_HEIGHT * scale;
    return scale > 1 ? fb_presentation : fb;
}

static void ScanDisplay(uint32_t dx, uint32_t dy, uint32_t dw, uint32_t top, uint32_t dh, int rgb24)
{
    uint32_t row, col;
    /* Blank scanlines outside this frame's display range. Keeping the
     * old framebuffer there leaks pixels when a scene narrows the range. */
    memset(fb, 0, sizeof(fb));
    if (fb_mask && rgb24) {
        /* DISPENV.isrgb24 (MDEC movies): VRAM holds packed R,G,B bytes;
         * disp.w may be given in 16-bit VRAM units (480 for 320 px). */
        uint32_t pw = dw > (uint32_t)PE_PORT_FB_WIDTH ? dw * 2u / 3u : dw;
        if (pw > (uint32_t)PE_PORT_FB_WIDTH) pw = (uint32_t)PE_PORT_FB_WIDTH;
        for (row = 0u; row < dh && top + row < (uint32_t)PE_PORT_FB_HEIGHT; row++) {
            for (col = 0u; col < pw; col++) {
                uint8_t *dst = fb + ((size_t)(top + row) * PE_PORT_FB_WIDTH + col) * 3u;
                for (uint32_t c = 0u; c < 3u; c++) {
                    uint32_t byte = col * 3u + c;
                    uint16_t word = 0u;
                    (void)PE_GPU_ReadVRAM((dx + byte / 2u) & 1023u, (dy + row) & 511u, &word);
                    dst[c] = (uint8_t)((byte & 1u) ? word >> 8 : word);
                }
            }
        }
    } else if (fb_mask) {
        if (dw > (uint32_t)PE_PORT_FB_WIDTH)  dw = (uint32_t)PE_PORT_FB_WIDTH;
        for (row = 0u; row < dh && top + row < (uint32_t)PE_PORT_FB_HEIGHT; row++) {
            for (col = 0u; col < dw; col++) {
                uint16_t pixel = 0u;
                uint8_t *dst = fb + ((size_t)(top + row) * PE_PORT_FB_WIDTH + col) * 3u;
                if (PE_GPU_ReadVRAM(dx + col, dy + row, &pixel)) {
                    HostVRAM_DecodePixel(pixel, dst);
                } else {
                    dst[0] = 0u;
                    dst[1] = 0u;
                    dst[2] = 0u;
                }
            }
        }
    }
    ScanPresentation(dx, dy, dw, top, dh, rgb24);
}

void HostFB_SetDispMask(int mask)
{
    fb_mask = mask;
}

static void HostFB_ServiceDeviceIrq(void)
{
    PeIrqServiceResult result;
    if (PE_Port_ShouldStop() || PE_LoadU16(0x800945E6u) ||
        !(PE_IRQ_ReadStatus() & PE_IRQ_GetMask())) return;
    result = PE_IRQ_ServicePendingForGeneration(PE_IRQ_Generation());
    if ((result == PE_IRQ_SERVICE_BOUNDARY || result == PE_IRQ_SERVICE_STALE) &&
        !PE_Port_ShouldStop())
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
static void HostFB_DeviceTime(uint32_t cycles)
{
    /* Advance to each CD deadline and VBlank edge separately, servicing
     * IRQs between sectors. Keep the full elapsed quantum: at double speed
     * a 564480-cycle NTSC frame contains 2.5 sectors, not one sector. */
    while(cycles && !PE_Port_ShouldStop()) {
        uint32_t step=PE_CdReg_TimeSlice(cycles);
        uint32_t to_vblank=564480u-fb_device_cycles;
        if(step>to_vblank) step=to_vblank;
        PE_CdReg_ServiceDevice(step);
        if (PE_Port_ShouldStop()) return;
        HostFB_ServiceDeviceIrq();
        if (PE_Port_ShouldStop()) return;
        PE_MMIO_Commit();
        if (PE_Port_ShouldStop()) return;
        PE_Rcnt2_Advance(step);
        if(PE_Port_ShouldStop()) return;
        cycles-=step;
        fb_device_cycles+=step;
        if(fb_device_cycles>=564480u) {
            fb_device_cycles-=564480u;
            PeIrqGeneration generation=PE_IRQ_Generation();
            PE_GPU_VBlankStep();
            if (PE_Port_ShouldStop()) return;
            if (!PE_GPU_SetVBlank(0,generation) ||
                !PE_GPU_SetVBlank(1,generation)) {
                if (!PE_Port_ShouldStop())
                    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return;
            }
            HostFB_ServiceDeviceIrq();
            /* Deassert even after a stopped IRQ; a stale generation remains
             * an explicit stop rather than a completed VBlank. */
            if (!PE_GPU_SetVBlank(0,generation) && !PE_Port_ShouldStop())
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            if (PE_Port_ShouldStop()) return;
        }
    }
}

/* Non-device VBlank edge.  Retail: the VBlank IRQ (I_STAT bit 0)
 * reaches the libetc source-0 handler func_8007440C, which bumps D_800956AC
 * and runs the eight callback slots — func_8003E91C (RNG advance
 * func_80070D6C + timer ramps func_80036F7C, incl. the play-time clock).
 * The device path gets this through PE_IRQ_ServicePendingForGeneration; the
 * default (non-device) path previously never dispatched it.  Same guards as
 * HostFB_ServiceDeviceIrq: not already inside a dispatch, the VBlank source
 * unmasked, and ResetCallback has installed 8007440C in the source-0 slot. */
static int (*g_vblank_dispatch)(void);
static void HostFB_VBlankGameCallbacks(void)
{
    if (PE_Port_ShouldStop()) return;
    if (PE_LoadU16(0x800945E6u) != 0u || !(PE_IRQ_GetMask() & 1u)) return;
    if (PE_LoadU32(0x800945E8u) != 0x8007440Cu || !g_vblank_dispatch) return;
    (void)g_vblank_dispatch();
    {   /* PE_VBLANK_TRACE=N: every N dispatches log the D_800956AC VBlank
         * count, the func_80070D6C RNG cursors (0x80070E04/08) and the
         * D_800A76A0 timer counters (play time = D_800A76BC). */
        static int every = -1;
        if (every < 0) every = getenv("PE_VBLANK_TRACE") ? atoi(getenv("PE_VBLANK_TRACE")) : 0;
        if (every > 0 && PE_LoadU32(0x800956ACu) % (uint32_t)every == 0u)
            fprintf(stderr, "[VBLANK] n=%u rng_i=%d/%d t0=%u t1=%u t2(play)=%u t3=%u d1a0=%08X\n",
                    PE_LoadU32(0x800956ACu), (int32_t)PE_LoadU32(0x80070E04u), (int32_t)PE_LoadU32(0x80070E08u),
                    PE_LoadU32(0x800A76A4u), PE_LoadU32(0x800A76B0u), PE_LoadU32(0x800A76BCu),
                    PE_LoadU32(0x800A76C8u), PE_LoadU32(0x8009D1A0u));
    }
}

void HostFB_SetVBlankDispatch(int (*dispatch)(void))
{
    g_vblank_dispatch = dispatch;
}

uint32_t HostFB_VSyncTimer(void)
{
    return fb_vsync_timer;
}

uint32_t HostFB_VSync(int mode)
{
    PE_Port_EscapePoint();      /* armed only by PE_Main_EnterCredits */
    if (PE_Port_ShouldStop()) return 0;
    PE_Port_CreditsPadPoint();  /* likewise: the per-VBlank pad refill */
    if (PE_Port_ShouldStop()) return 0;
    /* A nonblocking audio upload must complete even when the game only
     * polls its completion flag. Service DMA at the normal host tick. */
    (void)PE_SpuDma_Service();
    if (PE_Port_ShouldStop()) return 0;
    /* While a movie decode is in flight the GPU DMA2 upload of the
     * previous slice completes in parallel with the MDEC on hardware.
     * The host tick is the asynchronous-progress stand-in, so complete
     * the pending GPU transfer (and drain its queued ring through the
     * DMA IRQ checkpoint) before the MDEC decodes into the rotating
     * buffer again. Without this, queued LoadImages execute late and
     * read overwritten slice buffers. */
    if (PE_MDEC_HasDecode() && PE_GPU_DMA2Pending()) {
        PEPortDmaIrqCheckpointResult checkpoint = PE_Port_ServiceDmaIrqCheckpoint();
        if (checkpoint == PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY ||
            checkpoint == PE_PORT_DMA_IRQ_CHECKPOINT_STALE) {
            if (!PE_Port_ShouldStop())
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
    }
    if (PE_Port_ShouldStop()) return 0;
    (void)PE_MDEC_Service();
    if (PE_Port_ShouldStop()) return 0;
    if(PE_MDEC_HasDecode()) HostFB_ServiceDeviceIrq();
    if (PE_Port_ShouldStop()) return 0;
    /* Deterministic host CPU-work quantum, including busy counter queries.
     * This is an approximate device clock, not a cycle-accurate CPU model. */
    int device=PE_CdReg_DeviceEnabled();
    if(device && (mode<0 || mode==1)) HostFB_DeviceTime(1024u);
    if (PE_Port_ShouldStop()) return 0;
    if (mode >= 0) PE_Event_ServiceAudioCommands();
    if (PE_Port_ShouldStop()) return 0;
    /* Negative VSync is a counter query and mode 1 queries elapsed scan
     * lines. Waiting modes advance the shared 60 Hz clock used by menus. */
    if (mode>=0 && mode!=1) {
        int ticks=mode>0?mode:1;
        while (ticks-->0) {
            if(device) HostFB_DeviceTime(564480u);
            else { PE_GPU_VBlankStep(); HostFB_VBlankGameCallbacks(); }
            if(PE_Port_ShouldStop()) break;
            HostFB_ScanOutIdleVBlank();
            HostFB_ShotVBlank();
        }
    }
    if (PE_Port_ShouldStop()) return 0;
    fb_vsync_count++;
    HostFB_ShotVBlank();
    /* VBlank edge. The retail return contract (80073A44) samples the free-
     * running scanline timer at ENTRY and returns `(entry - baseline) &
     * 0xFFFF` for every non-negative mode; waiting modes then re-latch the
     * baseline to the post-wait value (this is what makes a following
     * mode-1 query read the elapsed scanlines) and negative modes instead
     * return the absolute VBlank counter (D_800956AC).  With the device
     * attached, that counter is the real one: VBlanks come from device time,
     * the libetc source-0 handler 8007440C bumps it once per VBlank, and each
     * query adds its 1024 device cycles above, so a busy loop on VSync(-1)
     * still advances.  Without the device no VBlank happens inside a busy
     * loop, so the host keeps its call count there.  The call count moves
     * twice per frame inside a VSync callback (its own query plus the wait),
     * so a callback testing `VSync(-1) & 0x1F` (the credits' func_80125A40)
     * can lock onto odd values and never fire. */
    if (mode < 0)
        return device ? PE_LoadU32(0x800956ACu) : (uint32_t)fb_vsync_count;
    uint32_t entry = fb_vsync_timer;
    uint32_t ret = (entry - fb_vsync_baseline) & 0xFFFFu;
    fb_vsync_timer = (fb_vsync_timer + 1u) & 0xFFFFu;
    if (mode > 1) fb_vsync_timer = (fb_vsync_timer + (uint32_t)mode) & 0xFFFFu;
    else if (mode == 0) fb_vsync_timer = (fb_vsync_timer + 1u) & 0xFFFFu;
    if (mode != 1) fb_vsync_baseline = fb_vsync_timer;
    return ret;
}

void HostFB_DrawSync(int mode)
{
    fb_drawsync_count++;
    (void)mode;
}

int HostFB_WritePPM(const char *path)
{
    FILE *f;
    {   /* PE_GPU_RAM_DUMP=<path>: debug aid (off by default) — also write
         * the 2 MB guest RAM image with every screenshot (oracle RAM diffs). */
        const char *e = getenv("PE_GPU_RAM_DUMP");
        FILE *r = (e && *e) ? fopen(e, "wb") : NULL;
        if (r) { fwrite(PE_TranslateConst(PE_RAM_BASE, PE_RAM_SIZE), 1, PE_RAM_SIZE, r); fclose(r); }
    }
    f = fopen(path, "wb");
    if (!f) return -1;

    fprintf(f, "P6\n%d %d\n255\n", PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
    fwrite(fb, 1, sizeof(fb), f);
    fclose(f);
    return 0;
}

const uint8_t *HostFB_GetPixels(void)
{
    return fb;
}

/* One guest CD-status poll (func_8007F418 DsSync(0), func_8007F72C
 * DsSystemStatus).  Retail's drive runs while the CPU spins on these; a C
 * busy loop over them (the generated ending-credits waits, e.g.
 * `while (DsSystemStatus() != 1 || DsQueueLen() != 0) {}`) makes no
 * progress unless each poll stands for some CPU time.  64 cycles is the
 * retail cost of one such iteration, to within about a factor of two:
 * func_8007F72C (19 words) + func_8007FBF0 + func_8007F778 (4 words) + the
 * loop test, about 31 instructions with ~10 uncached RAM accesses.  It is
 * not the 1024-cycle VSync(-1) quantum, which would advance the drive ~16x
 * faster per poll than retail.  No effect with the device detached. */
void HostFB_DevicePoll(void)
{
    PE_Port_EscapePoint();      /* armed only by PE_Main_EnterCredits */
    if (!PE_Port_ShouldStop() && PE_CdReg_DeviceEnabled())
        HostFB_DeviceTime(64u);
}

/* One guest poll-loop iteration of a CD stream wait.  HostFB_VSync(-1)
 * services MDEC/SPU/GPU and the DMA IRQ checkpoint and advances the modeled
 * device by the 1024-cycle counter-query quantum; the extra 3072 cycles make
 * the wait iteration span the device time the guest's own func_80121270 call
 * stands for, so an entire multi-sector frame can assemble inside the 2000-
 * iteration retry budget instead of restarting from sector 0. */
void HostFB_StreamTick(void)
{
    HostFB_VSync(-1);
    if (!PE_Port_ShouldStop() && PE_CdReg_DeviceEnabled())
        HostFB_DeviceTime(3072u);
}

void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask)
{
    if (vsync)     *vsync     = fb_vsync_count;
    if (drawsync)  *drawsync  = fb_drawsync_count;
    if (presented) *presented = fb_presented;
    if (mask)      *mask      = fb_mask;
}
