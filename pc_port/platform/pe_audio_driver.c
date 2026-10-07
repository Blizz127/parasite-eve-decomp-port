/*
 * Audio driver clock — see pe_audio_driver.h.
 *
 * Native only (owner, 2026-10-07): every 240 Hz RCNT2 tick runs the native
 * C func_8008DB7C (game/decomp_hand/akao_tick_port.c, line-for-line from the
 * matched src/func_8008DB7C.c and its matched callees).  No MIPS machine code
 * is executed.  The old in-runtime R3000A interpreter moved to the test-only
 * oracle tests/pe_akao_interp_oracle.c.  A callee the native tick cannot reach
 * natively is a decomp boundary (CPU_BOUNDARY/REFUSED), never interpreted.
 */
#include "pe_audio_driver.h"
#include "pe_spu.h"
#include "pe_spu_dma.h"
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include "pe_sdk.h"
#include "host_framebuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* The native AKAO driver tick (game/decomp_hand/akao_tick_port.c). */
void func_8008DB7C(void);
void pe_plat_audio_commit_registers(void);

/* Host clock (host_framebuffer.c): 564480 cycles per VBlank, 60 Hz.
 * RCNT2 period 0x44E8 * 8 = 141120 cycles -> exactly 4 ticks per VBlank. */
#define CYCLES_PER_VBLANK  564480u
#define CYCLES_PER_TICK    141120u

static struct {
    PeAudioSink sink;
    int enabled;
    uint64_t tick_cycles;
    uint64_t sample_acc;          /* in units of 1/CYCLES */
    PeAudioDriverStats st;
} D;


static FILE *g_kon_log;

static void kon_logger(uint32_t mask)
{
    if (g_kon_log)
        fprintf(g_kon_log, "%llu %06X\n", (unsigned long long)D.st.vblanks, mask);
}

void PE_AudioDriver_Enable(int enable)
{
    const char *kl = getenv("PE_AUDIO_KON_LOG");
    D.enabled = enable ? 1 : 0;
    PE_Spu_SetActive(D.enabled);
    /* PE_AUDIO_KON_LOG=path: one "vblank voice-mask" line per key-on
     * (voice-activity timeline; WAV time = vblank / 60). */
    if (D.enabled && kl && *kl && !g_kon_log) {
        g_kon_log = fopen(kl, "w");
        if (g_kon_log) {
            setvbuf(g_kon_log, NULL, _IOLBF, 0);
            PE_Spu_SetKonHook(kon_logger);
        }
    }
}

int PE_AudioDriver_Active(void) { return D.enabled; }

void PE_AudioDriver_SetSink(PeAudioSink sink) { D.sink = sink; }

void PE_AudioDriver_GetStats(PeAudioDriverStats *out)
{
    if (out) *out = D.st;
}


/* ── clock ─────────────────────────────────────────────────────────── */

static void render_samples(uint32_t cycles)
{
    int16_t buf[2 * 512];
    uint64_t n;

    D.sample_acc += (uint64_t)cycles * PE_SPU_RATE;
    n = D.sample_acc / 33868800u;
    D.sample_acc %= 33868800u;
    while (n) {
        int k = n > 512 ? 512 : (int)n;
        PE_Spu_Render(buf, k);
        if (D.sink) D.sink(buf, k);
        n -= (uint64_t)k;
    }
}

static void driver_tick(void)
{
    if (!PE_Event_AudioTimerArmed()) {
        D.st.skipped_ticks++;
        return;
    }
    /* Retail func_8008E23C (RCNT2 event handler) -> func_8008DB7C, natively.
     * The tick's SPU register writes go through host pointers into pe_mmio;
     * commit them so the native SPU sees this tick's state before the next
     * 183.75 samples are rendered. */
    func_8008DB7C();
    pe_plat_audio_commit_registers();
    D.st.ticks++;
}

static uint32_t seq_mask(int i)
{
    /* D_8009D2C8: current music control block (0x68 bytes), +4 = active
     * channel mask (src/func_8008DB7C.c) */
    uint32_t c = PE_LoadU32(0x8009D2C8u);
    if (!PE_RangeIsRam(c + (uint32_t)i * 0x68u, 8)) return 0xFFFFFFu;
    return PE_LoadU32(c + (uint32_t)i * 0x68u + 4u);
}

static uint32_t seq_mask(int i);
static PeVBlankPacer g_vblank_pacer;

void PE_AudioDriver_SetVBlankPacer(PeVBlankPacer pacer) { g_vblank_pacer = pacer; }

static void pace_log(void)
{
    static FILE *f;
    static int init;
    static uint64_t n, last_kon, last_cd;
    static int last_presented;
    if (!init) {
        const char *p = getenv("PE_AUDIO_PACE_LOG");
        init = 1;
        if (p && *p) {
            f = fopen(p, "w");
            if (f) fprintf(f, "# vblank presents_in_last_60 kon_in_last_60 ticks wall_s cd_frames_in_last_60\n");
        }
    }
    if (!f || ++n % 60u) return;
    {
        PeSpuStats sp;
        int presented = 0;
        PE_Spu_GetStats(&sp);
        HostFB_GetState(NULL, NULL, &presented, NULL);
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        fprintf(f, "%llu %d %llu %llu %.3f %llu seq0=%06X seq1=%06X\n", (unsigned long long)n,
                presented - last_presented,
                (unsigned long long)(sp.kon_events - last_kon),
                (unsigned long long)D.st.ticks,
                (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9,
                (unsigned long long)(sp.cd_frames_mixed - last_cd),
                seq_mask(0), seq_mask(1));
        last_cd = sp.cd_frames_mixed;
        fflush(f);
        last_presented = presented;
        last_kon = sp.kon_events;
    }
}

void PE_AudioDriver_VBlank(void)
{
    static int debug = -1;
    pace_log();
    if (!D.enabled) {
        if (g_vblank_pacer) g_vblank_pacer();
        return;
    }
    D.st.vblanks++;
    if (debug < 0) debug = getenv("PE_AUDIO_DEBUG") != NULL;
    if (debug && D.st.vblanks % 600u == 0u) {
        PeSpuStats sp;
        PE_Spu_GetStats(&sp);
        fprintf(stderr, "[AUDIO] t=%llus voices=%u KON=%llu ticks=%llu skipped=%llu "
                "cmdq-gate=%u D_8009D268=%08X seq0=%06X seq1=%06X sfx=%06X\n",
                (unsigned long long)(D.st.vblanks / 60u), sp.voices_active,
                (unsigned long long)sp.kon_events, (unsigned long long)D.st.ticks,
                (unsigned long long)D.st.skipped_ticks,
                (unsigned)PE_Event_AudioTimerArmed(), PE_LoadU32(0x8009D268u),
                seq_mask(0), seq_mask(1), PE_LoadU32(0x800BCD50u));
        PE_Spu_DebugSummary(stderr);
    }
    D.tick_cycles += CYCLES_PER_VBLANK;
    while (D.tick_cycles >= CYCLES_PER_TICK) {
        D.tick_cycles -= CYCLES_PER_TICK;
        driver_tick();
        render_samples(CYCLES_PER_TICK);
    }
    if (g_vblank_pacer) g_vblank_pacer();
}
