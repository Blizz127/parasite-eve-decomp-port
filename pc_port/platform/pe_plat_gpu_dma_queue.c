/* GPU queue pump and wait (guest 0x80077294), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_gpu_rows_vectors.py
 * `dmaq`, checked by pe_gpu_rows_check.c) plus the hardware documentation
 * (psx-spx GPU: GPUSTAT bit 26 "ready to receive command word"; DMA: CHCR
 * bit 24 "busy"; interrupt control: I_MASK).  No SDK source, matched C or
 * disassembly was read; see docs/evidence/native-gpu-rows-20261125/.
 *
 * Observed contract (all of it from vectors):
 *  - mode 0 first calls the deadline seeder 0x800773D0; mode != 0 does not.
 *  - Queue: 64 entries of 0x60 bytes at 0x800BD030 + 0x60 * r; the words
 *    0x80095874 (q, producer) and 0x80095878 (r, consumer) index it; the
 *    number of pending entries is n = (q - r) & 0x3F.
 *  - n != 0 and the DMA2 channel idle: interrupts are masked (I_MASK, through
 *    the pointer word 0x80095674, saved in 0x80095880 and cleared), then
 *    entries are issued while the channel stays idle: when this is the last
 *    entry ((q - r) & 0x3F == 1) and the word 0x80095758 is zero, the callback
 *    at *(*0x8009566C + 4) is called with the constant 2; the GPU must report "ready"
 *    (GPUSTAT & 0x04000000, polled without a timeout); then entry fields
 *    +4 and +8 are read, the function at +0 is called with them, and r is
 *    stored back as (r + 1) & 0x3F.  I_MASK is restored from 0x80095880.
 *  - Then (and when the channel was busy at the start, without the
 *    preliminary part) the idle/ready condition is evaluated: busy = CHCR
 *    bit 24, not ready = GPUSTAT bit 26 clear.  Mode != 0 returns n if
 *    n != 0, else 1 when busy or not ready, else 0.  Mode 0 calls the timeout
 *    check 0x80077404 each time the condition is not met (-1 from it makes
 *    the result -1) and returns 0 once idle and ready.
 *  - Host-only additions (not observable in vectors): the GPU-ready spin and the
 *    wait-mode loop leave on PE_Port_ShouldStop(), and the spin stops the port
 *    (PE_Port_RequestStop) after SPIN_BUDGET polls.
 *  - After a drain the sequence q r, CHCR[, 0x80095754 (and, if non-zero,
 *    0x80095758)], and in mode 0 q r + one timeout call precedes that wait. */
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_plat/gpu_timeout.h"
#include "pe_plat/gpu_dma_queue.h"
#include "pe_plat/gpu_regs.h"

#define W_Q        0x80095874u
#define W_R        0x80095878u
#define W_MASKSAVE 0x80095880u
#define W_FLAG754  0x80095754u
#define W_FLAG758  0x80095758u
#define P_CHCR     0x80095860u
#define P_GPU      0x80095854u
#define P_IMASK    0x80095674u
#define P_CBTAB    0x8009566Cu
#define ENTRY_BASE 0x800BD030u
#define ENTRY_SIZE 0x60u
#define CHCR_BUSY  0x01000000u
#define STAT_READY 0x04000000u
#define SPIN_BUDGET 4000000u   /* polls of GPUSTAT before the wait is declared stuck */

/* Empty test.  Wait mode compares the indices as they are; poll mode compares
 * them modulo 64 (observed: q = 0x121, r = 0x61 is empty in poll mode and not
 * in wait mode). */
static int queue_empty(int32_t mode, uint32_t q, uint32_t r)
{
    return mode == 0 ? q == r : ((q - r) & 0x3Fu) == 0u;
}

static uint32_t chcr_word(void)
{
    uint32_t p = PE_LoadU32(P_CHCR);
    if (!pe_gpu_reg_ok(p, 4)) return pe_gpu_reg_refuse("dmaq_chcr", "func_80077294", p);
    return PE_LoadU32(p);
}

int32_t pe_plat_gpu_dma_queue_run(int32_t mode)
{
    uint32_t q, r, n = 0, c;
    int ret;
    if (mode == 0) (void)func_800773D0();
    for (;;) {
        q = PE_LoadU32(W_Q);
        r = PE_LoadU32(W_R);
        n = (q - r) & 0x3Fu;
        if (queue_empty(mode, q, r)) break;
        c = chcr_word();
        if (c & CHCR_BUSY) {
            if (mode != 0) break;                       /* poll: report below */
            if (pe_plat_gpu_timeout() != 0) return -1;  /* wait: timeout check, try again */
            if (PE_Port_ShouldStop()) return 0;
            continue;
        }
        {
            uint32_t ip = PE_LoadU32(P_IMASK);
            uint32_t saved;
            if (!pe_gpu_reg_ok(ip, 2)) return pe_gpu_reg_refuse("dmaq_imask", "func_80077294", ip);
            saved = PE_LoadU16(ip);
            PE_StoreU16(ip, 0);
            q = PE_LoadU32(W_Q);
            r = PE_LoadU32(W_R);
            PE_StoreU32(W_MASKSAVE, saved);
            for (;;) {
                uint32_t e, e4, e8, fn;
                c = chcr_word();
                if (c & CHCR_BUSY) break;
                r = PE_LoadU32(W_R);
                q = PE_LoadU32(W_Q);
                if (((r + 1u) & 0x3Fu) == q && PE_LoadU32(W_FLAG758) == 0u) {
                    uint32_t cbt = PE_LoadU32(P_CBTAB), cb;
                    if (!pe_gpu_reg_ok(cbt + 4u, 4)) return pe_gpu_reg_refuse("dmaq_cbtab", "func_80077294", cbt);
                    cb = PE_LoadU32(cbt + 4u);
                    if (!PE_GuestHook_Call("dmaq_callback", "func_80077294", cb, 1, 2, 0, 0, 0, &ret)) return 0;
                }
                {
                    uint32_t gp = PE_LoadU32(P_GPU);
                    uint32_t s;
                    uint32_t polls = 0;
                    if (!pe_gpu_reg_ok(gp, 4)) return pe_gpu_reg_refuse("dmaq_gpu", "func_80077294", gp);
                    s = PE_LoadU32(gp);
                    while (!(s & STAT_READY)) {
                        /* Host-only wait contract (not in the vectors): leave on a requested stop, and
                         * give up on a GPU that never reports ready. */
                        if (PE_Port_ShouldStop()) return 0;
                        if (++polls > SPIN_BUDGET) {
                            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                            return 0;
                        }
                        s = PE_LoadU32(gp);
                    }
                }
                r = PE_LoadU32(W_R);
                r = PE_LoadU32(W_R);
                e = ENTRY_BASE + r * ENTRY_SIZE;
                if (!pe_gpu_reg_ok(e + 8u, 4)) return pe_gpu_reg_refuse("dmaq_entry", "func_80077294", e);
                e4 = PE_LoadU32(e + 4u);
                r = PE_LoadU32(W_R);
                e = ENTRY_BASE + r * ENTRY_SIZE;
                e8 = PE_LoadU32(e + 8u);
                fn = PE_LoadU32(e);
                if (!PE_GuestHook_Call("dmaq_entry", "func_80077294", fn, 2, e4, e8, 0, 0, &ret)) return 0;
                r = PE_LoadU32(W_R);
                PE_StoreU32(W_R, (r + 1u) & 0x3Fu);
                q = PE_LoadU32(W_Q);
                r = PE_LoadU32(W_R);
                if (q == r) break;
            }
            saved = PE_LoadU32(W_MASKSAVE);
            ip = PE_LoadU32(P_IMASK);
            if (!pe_gpu_reg_ok(ip, 2)) return pe_gpu_reg_refuse("dmaq_imask", "func_80077294", ip);
            (void)PE_LoadU16(ip);
            PE_StoreU16(ip, (uint16_t)saved);
        }
        q = PE_LoadU32(W_Q);
        r = PE_LoadU32(W_R);
        if (q == r) {
            c = chcr_word();
            if (!(c & CHCR_BUSY) && PE_LoadU32(W_FLAG754) != 0u) (void)PE_LoadU32(W_FLAG758);
        }
        q = PE_LoadU32(W_Q);
        r = PE_LoadU32(W_R);
        if (mode == 0) {
            if (pe_plat_gpu_timeout() != 0) return -1;
            if (PE_Port_ShouldStop()) return 0;
            continue;                                   /* look at the queue again */
        }
        break;
    }
    for (;;) {
        int waiting;
        c = chcr_word();
        waiting = (c & CHCR_BUSY) != 0u;
        if (!waiting) {
            uint32_t gp = PE_LoadU32(P_GPU);
            if (!pe_gpu_reg_ok(gp, 4)) return pe_gpu_reg_refuse("dmaq_gpu", "func_80077294", gp);
            waiting = !(PE_LoadU32(gp) & STAT_READY);
        }
        if (!waiting) return mode == 0 ? 0 : (int32_t)n;
        if (mode != 0) return n != 0 ? (int32_t)n : 1;
        if (pe_plat_gpu_timeout() != 0) return -1;
        if (PE_Port_ShouldStop()) return 0;
    }
}
