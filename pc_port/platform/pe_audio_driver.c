/*
 * Audio driver clock + retail AKAO tick interpreter — see pe_audio_driver.h.
 *
 * The interpreter is deliberately small: R3000A integer ISA, branch delay
 * slots, no load-delay modelling (cc1 output never reads a load target in
 * the delay slot), no exceptions.  COP2/GTE and anything it cannot run is a
 * fault: the tick is abandoned and reported (first few loudly).  After
 * repeated faults the tick is disabled so the game keeps running (music
 * then stays silent, sound effects written by host-ported code still play).
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

#define GUEST_TICK_ENTRY   0x8008DB7Cu   /* func_8008DB7C (retail text) */
#define RETURN_MAGIC       0xFFFFFFF0u
#define STACK_TOP          0x7F010000u   /* private host stack (KUSEG hole) */
#define STACK_SIZE         0x00008000u
#define STACK_LO           (STACK_TOP - STACK_SIZE)
#define STEP_BUDGET        4000000u

/* Host clock (host_framebuffer.c): 564480 cycles per VBlank, 60 Hz.
 * RCNT2 period 0x44E8 * 8 = 141120 cycles -> exactly 4 ticks per VBlank. */
#define CYCLES_PER_VBLANK  564480u
#define CYCLES_PER_TICK    141120u

static struct {
    PeAudioSink sink;
    int enabled;
    int disabled_by_faults;
    uint32_t consecutive_faults;
    uint64_t tick_cycles;
    uint64_t sample_acc;          /* in units of 1/CYCLES */
    PeAudioDriverStats st;
    uint8_t stack[STACK_SIZE];
    uint32_t warned_io[16];
    unsigned warned_io_n;
} D;

typedef struct {
    uint32_t r[32];
    uint32_t hi, lo;
    uint32_t pc, npc;
    int fault;
    uint32_t fault_pc, fault_addr, fault_insn;
    const char *fault_what;
} Cpu;

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

/* ── memory ────────────────────────────────────────────────────────── */

static void fault(Cpu *c, const char *what, uint32_t addr)
{
    if (!c->fault) {
        c->fault = 1;
        c->fault_what = what;
        c->fault_addr = addr;
    }
}

static void warn_io(uint32_t phys, const char *dir)
{
    for (unsigned i = 0; i < D.warned_io_n; i++)
        if (D.warned_io[i] == phys) return;
    if (D.warned_io_n < 16) D.warned_io[D.warned_io_n++] = phys;
    fprintf(stderr, "[AUDIO] driver %s of unmodelled I/O 0x%08X ignored\n",
            dir, phys);
}

static int is_stack(uint32_t a, uint32_t size)
{
    return a >= STACK_LO && a + size <= STACK_TOP;
}

static int is_guest_mem(uint32_t a, uint32_t size)
{
    return PE_RangeIsScratchpad(a, size) ||
           PE_RangeIsRam(PE_RamCanonical(a, size), size);
}

static uint32_t spu_read(uint32_t off, uint32_t size)
{
    if (off >= 0x200u) return 0;
    if (size == 4)
        return PE_SpuRegister_LoadU16(off) |
               ((uint32_t)PE_SpuRegister_LoadU16(off + 2u) << 16);
    {
        uint16_t h = PE_SpuRegister_LoadU16(off & ~1u);
        if (size == 1) return (off & 1u) ? (h >> 8) : (h & 0xFFu);
        return h;
    }
}

static void spu_write(uint32_t off, uint32_t size, uint32_t v)
{
    if (off >= 0x200u) return;
    if (size == 4) {
        PE_SpuRegister_StoreU16(off, (uint16_t)v);
        PE_SpuRegister_StoreU16(off + 2u, (uint16_t)(v >> 16));
    } else if (size == 2) {
        PE_SpuRegister_StoreU16(off, (uint16_t)v);
    } else {
        uint16_t h = PE_SpuRegister_LoadU16(off & ~1u);
        h = (off & 1u) ? (uint16_t)((h & 0x00FFu) | ((v & 0xFFu) << 8))
                       : (uint16_t)((h & 0xFF00u) | (v & 0xFFu));
        PE_SpuRegister_StoreU16(off & ~1u, h);
    }
}

static uint32_t mem_read(Cpu *c, uint32_t a, uint32_t size)
{
    uint32_t phys = a & 0x1FFFFFFFu;

    if (size > 1 && (a & (size - 1u))) { fault(c, "unaligned load", a); return 0; }
    if (is_stack(a, size)) {
        const uint8_t *p = D.stack + (a - STACK_LO);
        return size == 1 ? p[0] : size == 2 ? (uint32_t)(p[0] | p[1] << 8)
             : (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
               (uint32_t)p[3] << 24;
    }
    if (phys >= 0x1F801000u && phys < 0x1F803000u &&
        (a & 0xE0000000u) != 0x60000000u) {
        if (phys >= 0x1F801C00u && phys < 0x1F802000u)
            return spu_read(phys - 0x1F801C00u, size);
        if (PE_MMIO_IsModelled(phys, size))
            return PE_MMIO_Load(phys, size);
        warn_io(phys, "load");
        return 0;
    }
    if (!is_guest_mem(a, size)) { fault(c, "bad load address", a); return 0; }
    return size == 1 ? PE_LoadU8(a) : size == 2 ? PE_LoadU16(a) : PE_LoadU32(a);
}

static void mem_write(Cpu *c, uint32_t a, uint32_t size, uint32_t v)
{
    uint32_t phys = a & 0x1FFFFFFFu;

    if (size > 1 && (a & (size - 1u))) { fault(c, "unaligned store", a); return; }
    if (is_stack(a, size)) {
        uint8_t *p = D.stack + (a - STACK_LO);
        for (uint32_t i = 0; i < size; i++) p[i] = (uint8_t)(v >> (8 * i));
        return;
    }
    if (phys >= 0x1F801000u && phys < 0x1F803000u) {
        if (phys >= 0x1F801C00u && phys < 0x1F802000u) {
            spu_write(phys - 0x1F801C00u, size, v);
            return;
        }
        if (PE_MMIO_IsModelled(phys, size)) {
            PE_MMIO_Store(phys, size, v);
            return;
        }
        warn_io(phys, "store");
        return;
    }
    if (!is_guest_mem(a, size)) { fault(c, "bad store address", a); return; }
    if (size == 1) PE_StoreU8(a, (uint8_t)v);
    else if (size == 2) PE_StoreU16(a, (uint16_t)v);
    else PE_StoreU32(a, v);
}

/* ── BIOS vectors (only what a sound driver may plausibly use) ──────── */

static int bios_call(Cpu *c, uint32_t vec)
{
    uint32_t fn = c->r[9];
    uint32_t a0 = c->r[4], a1 = c->r[5], a2 = c->r[6];

    if (vec == 0xA0u && (fn == 0x2Au || fn == 0x2Bu || fn == 0x28u)) {
        /* 2A memcpy(dst,src,n) / 2B memset(dst,val,n) / 28 bzero(dst,n) */
        uint32_t n = fn == 0x28u ? a1 : a2;
        for (uint32_t i = 0; i < n && !c->fault; i++) {
            uint32_t b = fn == 0x2Au ? mem_read(c, a1 + i, 1)
                       : fn == 0x2Bu ? (a1 & 0xFFu) : 0u;
            mem_write(c, a0 + i, 1, b);
        }
        c->r[2] = a0;
        return 1;
    }
    fault(c, "unsupported BIOS call", (vec << 8) | fn);
    return 0;
}

/* ── interpreter ───────────────────────────────────────────────────── */

static uint32_t run(Cpu *c, uint32_t budget)
{
    uint32_t steps = 0;

    while (!c->fault) {
        uint32_t cur = c->pc, insn;
        uint32_t op, rs, rt, rd, sa, fn, imm, simm, target;

        if (cur == RETURN_MAGIC) break;
        if (cur == 0xA0u || cur == 0xB0u || cur == 0xC0u) {
            if (!bios_call(c, cur)) break;
            c->pc = c->r[31];
            c->npc = c->pc + 4u;
            continue;
        }
        if (++steps > budget) { fault(c, "instruction budget exhausted", cur); break; }
        if ((cur & 3u) || !PE_RangeIsRam(PE_RamCanonical(cur, 4), 4)) {
            fault(c, "bad instruction fetch", cur);
            break;
        }
        insn = PE_LoadU32(cur);
        c->pc = c->npc;
        c->npc += 4u;

        op = insn >> 26;
        rs = (insn >> 21) & 31u;
        rt = (insn >> 16) & 31u;
        rd = (insn >> 11) & 31u;
        sa = (insn >> 6) & 31u;
        fn = insn & 63u;
        imm = insn & 0xFFFFu;
        simm = (uint32_t)(int32_t)(int16_t)imm;
        target = cur + 4u + (simm << 2);
#define R(n) c->r[n]
#define W(n, v) do { uint32_t _v = (v); if ((n) != 0) c->r[n] = _v; } while (0)
        switch (op) {
        case 0x00:
            switch (fn) {
            case 0x00: W(rd, R(rt) << sa); break;
            case 0x02: W(rd, R(rt) >> sa); break;
            case 0x03: W(rd, (uint32_t)((int32_t)R(rt) >> sa)); break;
            case 0x04: W(rd, R(rt) << (R(rs) & 31u)); break;
            case 0x06: W(rd, R(rt) >> (R(rs) & 31u)); break;
            case 0x07: W(rd, (uint32_t)((int32_t)R(rt) >> (R(rs) & 31u))); break;
            case 0x08: c->npc = R(rs); break;
            case 0x09: { uint32_t t = R(rs); W(rd, cur + 8u); c->npc = t; break; }
            case 0x0C: case 0x0D: fault(c, "syscall/break", cur); break;
            case 0x10: W(rd, c->hi); break;
            case 0x11: c->hi = R(rs); break;
            case 0x12: W(rd, c->lo); break;
            case 0x13: c->lo = R(rs); break;
            case 0x18: {
                int64_t p = (int64_t)(int32_t)R(rs) * (int64_t)(int32_t)R(rt);
                c->lo = (uint32_t)p; c->hi = (uint32_t)((uint64_t)p >> 32); break;
            }
            case 0x19: {
                uint64_t p = (uint64_t)R(rs) * (uint64_t)R(rt);
                c->lo = (uint32_t)p; c->hi = (uint32_t)(p >> 32); break;
            }
            case 0x1A: {
                int32_t n = (int32_t)R(rs), d = (int32_t)R(rt);
                if (d == 0) { c->lo = n >= 0 ? 0xFFFFFFFFu : 1u; c->hi = (uint32_t)n; }
                else if ((uint32_t)n == 0x80000000u && d == -1) { c->lo = 0x80000000u; c->hi = 0; }
                else { c->lo = (uint32_t)(n / d); c->hi = (uint32_t)(n % d); }
                break;
            }
            case 0x1B: {
                uint32_t n = R(rs), d = R(rt);
                if (d == 0) { c->lo = 0xFFFFFFFFu; c->hi = n; }
                else { c->lo = n / d; c->hi = n % d; }
                break;
            }
            case 0x20: case 0x21: W(rd, R(rs) + R(rt)); break;
            case 0x22: case 0x23: W(rd, R(rs) - R(rt)); break;
            case 0x24: W(rd, R(rs) & R(rt)); break;
            case 0x25: W(rd, R(rs) | R(rt)); break;
            case 0x26: W(rd, R(rs) ^ R(rt)); break;
            case 0x27: W(rd, ~(R(rs) | R(rt))); break;
            case 0x2A: W(rd, (int32_t)R(rs) < (int32_t)R(rt)); break;
            case 0x2B: W(rd, R(rs) < R(rt)); break;
            default: fault(c, "unsupported SPECIAL", cur); break;
            }
            break;
        case 0x01: {
            int take = (rt & 1u) ? (int32_t)R(rs) >= 0 : (int32_t)R(rs) < 0;
            if (rt & 0x10u) c->r[31] = cur + 8u;
            if (take) c->npc = target;
            break;
        }
        case 0x02: c->npc = ((cur + 4u) & 0xF0000000u) | ((insn & 0x03FFFFFFu) << 2); break;
        case 0x03: c->r[31] = cur + 8u;
                   c->npc = ((cur + 4u) & 0xF0000000u) | ((insn & 0x03FFFFFFu) << 2); break;
        case 0x04: if (R(rs) == R(rt)) c->npc = target; break;
        case 0x05: if (R(rs) != R(rt)) c->npc = target; break;
        case 0x06: if ((int32_t)R(rs) <= 0) c->npc = target; break;
        case 0x07: if ((int32_t)R(rs) > 0) c->npc = target; break;
        case 0x08: case 0x09: W(rt, R(rs) + simm); break;
        case 0x0A: W(rt, (int32_t)R(rs) < (int32_t)simm); break;
        case 0x0B: W(rt, R(rs) < simm); break;
        case 0x0C: W(rt, R(rs) & imm); break;
        case 0x0D: W(rt, R(rs) | imm); break;
        case 0x0E: W(rt, R(rs) ^ imm); break;
        case 0x0F: W(rt, imm << 16); break;
        case 0x10: /* COP0: mfc0 reads 0, mtc0/rfe ignored */
            if (rs == 0) W(rt, 0);
            break;
        case 0x20: W(rt, (uint32_t)(int32_t)(int8_t)mem_read(c, R(rs) + simm, 1)); break;
        case 0x21: W(rt, (uint32_t)(int32_t)(int16_t)mem_read(c, R(rs) + simm, 2)); break;
        case 0x23: W(rt, mem_read(c, R(rs) + simm, 4)); break;
        case 0x24: W(rt, mem_read(c, R(rs) + simm, 1)); break;
        case 0x25: W(rt, mem_read(c, R(rs) + simm, 2)); break;
        case 0x22: case 0x26: {
            uint32_t a = R(rs) + simm, sh = (a & 3u) * 8u;
            uint32_t w = mem_read(c, a & ~3u, 4), v = R(rt);
            if (op == 0x22) /* lwl */
                v = (v & (0x00FFFFFFu >> sh)) | (w << (24u - sh));
            else            /* lwr */
                v = (sh ? (v & (0xFFFFFFFFu << (32u - sh))) : 0u) | (w >> sh);
            W(rt, v);
            break;
        }
        case 0x28: mem_write(c, R(rs) + simm, 1, R(rt)); break;
        case 0x29: mem_write(c, R(rs) + simm, 2, R(rt)); break;
        case 0x2B: mem_write(c, R(rs) + simm, 4, R(rt)); break;
        case 0x2A: case 0x2E: {
            uint32_t a = R(rs) + simm, sh = (a & 3u) * 8u;
            uint32_t m = mem_read(c, a & ~3u, 4), v = R(rt);
            if (op == 0x2A) /* swl */
                m = (sh == 24u ? 0u : (m & (0xFFFFFF00u << sh))) | (v >> (24u - sh));
            else            /* swr */
                m = (sh ? (m & (0x00FFFFFFu >> (24u - sh))) : 0u) | (v << sh);
            mem_write(c, a & ~3u, 4, m);
            break;
        }
        default:
            fault(c, "unsupported opcode", cur);
            break;
        }
#undef R
#undef W
        if (c->fault && !c->fault_pc) {
            c->fault_pc = cur;
            c->fault_insn = insn;
        }
    }
    D.st.instructions += steps;
    return c->r[2];
}

uint32_t PE_AudioDriver_CallGuest(uint32_t entry, uint32_t a0, uint32_t a1,
                                  uint32_t a2, uint32_t a3, int *ok)
{
    Cpu c;
    memset(&c, 0, sizeof(c));
    c.r[4] = a0; c.r[5] = a1; c.r[6] = a2; c.r[7] = a3;
    c.r[28] = 0x8009CD70u;                   /* retail $gp */
    c.r[29] = STACK_TOP - 0x20u;
    c.r[31] = RETURN_MAGIC;
    c.pc = entry;
    c.npc = entry + 4u;
    run(&c, STEP_BUDGET);
    if (c.fault) {
        D.st.faults++;
        if (D.st.faults <= 8)
            fprintf(stderr, "[AUDIO] driver fault: %s addr=0x%08X at pc=0x%08X "
                    "insn=0x%08X (entry 0x%08X)\n", c.fault_what, c.fault_addr,
                    c.fault_pc, c.fault_insn, entry);
    }
    if (ok) *ok = !c.fault;
    return c.r[2];
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
    int ok;
    if (D.disabled_by_faults || !PE_Event_AudioTimerArmed()) {
        D.st.skipped_ticks++;
        return;
    }
    if (PE_LoadU32(GUEST_TICK_ENTRY) == 0u) {   /* retail text not loaded */
        D.st.skipped_ticks++;
        return;
    }
    (void)PE_AudioDriver_CallGuest(GUEST_TICK_ENTRY, 0, 0, 0, 0, &ok);
    D.st.ticks++;
    if (ok) {
        D.consecutive_faults = 0;
    } else if (++D.consecutive_faults >= 16) {
        D.disabled_by_faults = 1;
        fprintf(stderr, "[AUDIO] driver tick disabled after %u consecutive "
                "faults; music stops, SPU output continues\n",
                D.consecutive_faults);
    }
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
