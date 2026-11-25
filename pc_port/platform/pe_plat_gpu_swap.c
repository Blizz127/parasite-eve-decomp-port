/* GPU command enqueue / immediate issue (guest 0x80076C34), written clean-room
 * from black-box vectors of the original code (pc_port/tools/pe_gpu_swap_vectors.py,
 * checked by pe_gpu_swap_check.c) plus the hardware documentation (psx-spx GPU:
 * GPUSTAT bit 26 "ready to receive command word"; DMA: CHCR bit 24 "busy";
 * interrupt control: I_MASK).  No SDK source, matched C, hand port or
 * disassembly of the function was read; see docs/evidence/native-76c34-20261125/.
 *
 * Contract (every line below is traced to a vector set):
 *  - func(worker, argument, copy_bytes, auxiliary).  First the deadline seeder
 *    0x800773D0 is called.  Then the ring words q = *0x80095874 and
 *    r = *0x80095878 are read; while the ring is full (r == (q + 1) & 0x3F) the
 *    timeout check 0x80077404 is called (any non-zero result: return -1), then
 *    the pump 0x80076EE4, and q, r are read again.
 *  - The interrupt mask is cleared with SetIntrMask(0) 0x80073E10; its result is
 *    saved in the word 0x8009587C.  The byte 0x8009574D (queue mode) is read and
 *    the word 0x80095754 is set to 1.
 *  - Immediate issue happens when the mode byte is zero, or when it is non-zero
 *    and the ring is empty (q == r read again), the DMA2 channel is idle (CHCR,
 *    reached through the pointer word 0x80095860, bit 24 clear) and the word
 *    0x80095758 is zero.  The GPU status (pointer word 0x80095854) is polled
 *    until bit 26 is set, the worker is called with (argument, auxiliary), the
 *    mask saved in 0x8009587C is restored with SetIntrMask and the result is 0.
 *  - Otherwise the command is queued: DMACallback(2, 0x80076EE4), then
 *    copy_bytes / 4 words of the argument are copied to entry + 0xC of the
 *    0x60-byte ring entry (base 0x800BD030 + 0x60 * q, q re-read before every
 *    store), the entry gets +4 = entry + 0xC, +8 = auxiliary, +0 = worker, q is
 *    advanced, the mask is restored, the pump 0x80076EE4 is called, and the
 *    result is the pending count (q - r) & 0x3F.
 *  - Host-only additions (not observable in vectors): the GPU-ready spin and the
 *    full-ring loop leave on PE_Port_ShouldStop(), the spin stops the port
 *    (PE_Port_RequestStop) after SPIN_BUDGET polls, register pointers that are
 *    not RAM or a modelled register are a named refusal, and a worker that the
 *    guest-hook layer refuses ends the call without restoring the mask. */
#include <stdint.h>
#include <stdio.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_plat/gpu_timeout.h"
#include "pe_plat/gpu_swap.h"
#include "pe_plat/gpu_regs.h"

#define W_Q        0x80095874u
#define W_R        0x80095878u
#define W_MASKSAVE 0x8009587Cu
#define W_MODE     0x8009574Du
#define W_FLAG754  0x80095754u
#define W_FLAG758  0x80095758u
#define P_CHCR     0x80095860u
#define P_GPU      0x80095854u
#define ENTRY_BASE 0x800BD030u
#define ENTRY_SIZE 0x60u
#define CHCR_BUSY  0x01000000u
#define STAT_READY 0x04000000u
#define PUMP_ADDR  0x80076EE4u
#define SPIN_BUDGET 4000000u
/* Host-only fallback (not in the vectors, whose pointer words are always valid): a register pointer word that
 * is zero or not RAM / a modelled register means libgpu state was never initialised (ResetGraph not run, as in
 * fixtures that seed only the dispatch table, or a poisoned RAM image); the hardware register address it would
 * hold is used instead of a refusal. */
#define HW_GPUSTAT 0x1F801814u
#define HW_DMA2_CHCR 0x1F8010A8u

/* Hook-slot name "func_<worker>" in persistent storage: the bootstrap layer keeps the symbol pointer (provider
 * registry, argument-call log), so it must outlive the call.  A bounded intern table; a full table falls back to
 * one fixed name. */
static const char *worker_slot(uint32_t worker)
{
    static char names[64][16];
    static uint32_t keys[64];
    static unsigned used;
    unsigned i;
    for (i = 0; i < used; i++)
        if (keys[i] == worker) return names[i];
    if (used >= 64u) return "func_swap_worker";
    keys[used] = worker;
    snprintf(names[used], sizeof names[used], "func_%08X", (unsigned)worker);
    return names[used++];
}

/* inl != NULL: the payload is the two words *inl[0..1] held by the caller (a retail
 * caller-stack buffer the host has no guest address for) instead of guest memory at
 * `argument`; copy_bytes is then 8.  A worker issued immediately would receive that
 * stack address, which does not exist on the host: the workers that have a host model
 * taking the two payload words run through pe_plat_gpu_swap_inline_issue (platform/
 * pe_plat_gpu_swap_host.c), every other worker is a named refusal. */
static int32_t swap_core(pe_addr_t worker, pe_addr_t argument, int32_t copy_bytes, uint32_t auxiliary,
                         const uint32_t *inl)
{
    uint32_t q, r, mask, direct = 0;
    unsigned epoch = PE_Port_StopEpoch();
    int ret;
    (void)func_800773D0();
    for (;;) {
        q = PE_LoadU32(W_Q);
        r = PE_LoadU32(W_R);
        if (((q + 1u) & 0x3Fu) != r) break;               /* ring not full */
        if (func_80077404() != 0) return -1;
        (void)func_80076EE4();
        if (PE_Port_ShouldStop()) return 0;
    }
    mask = func_80073E10(0);
    PE_StoreU32(W_MASKSAVE, mask);
    if (PE_LoadU8(W_MODE) == 0u) {
        PE_StoreU32(W_FLAG754, 1u);
        direct = 1;
    } else {
        PE_StoreU32(W_FLAG754, 1u);
        q = PE_LoadU32(W_Q);
        r = PE_LoadU32(W_R);
        if (q == r) {
            uint32_t cp = PE_LoadU32(P_CHCR), chcr;
            if (!pe_gpu_reg_ok(cp, 4)) {
                static int logged;
                if (!logged++) fprintf(stderr, "[gpu_swap] pointer word 0x80095860 = %08X is not a usable register: using DMA2 CHCR 0x1F8010A8\n", (unsigned)cp);
                cp = HW_DMA2_CHCR;
            }
            chcr = PE_LoadU32(cp);
            if (!(chcr & CHCR_BUSY) && PE_LoadU32(W_FLAG758) == 0u) direct = 1;
        }
    }
    if (direct) {
        uint32_t gp = PE_LoadU32(P_GPU), s, polls = 0;
        if (!pe_gpu_reg_ok(gp, 4)) {
            static int logged;
            if (!logged++) fprintf(stderr, "[gpu_swap] pointer word 0x80095854 = %08X is not a usable register: using GPUSTAT 0x1F801814\n", (unsigned)gp);
            gp = HW_GPUSTAT;
        }
        s = PE_LoadU32(gp);
        while (!(s & STAT_READY)) {
            if (PE_Port_ShouldStop()) return 0;
            if (++polls > SPIN_BUDGET) {
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return 0;
            }
            s = PE_LoadU32(gp);
        }
        if (PE_Port_EscapeArmed() && PE_Port_ShouldStop()) {
            /* Host-only: an armed stop escape would longjmp at the worker's guest-call point with the
             * interrupt mask still cleared; restore it and leave (a stop ends the run). */
            (void)func_80073E10((uint16_t)PE_LoadU32(W_MASKSAVE));
            return 0;
        }
        if (inl) {
            if (!pe_plat_gpu_swap_inline_issue(worker, inl[0], inl[1], auxiliary)) {
                PE_GuestHook_Refuse(worker_slot(worker), "func_80076C34", worker, 2, 0, auxiliary, 0, 0,
                                    "worker payload lives in a caller stack buffer with no guest address");
                return 0;
            }
            mask = PE_LoadU32(W_MASKSAVE);
            (void)func_80073E10((uint16_t)mask);
            return 0;
        }
        {
            /* Host-only: the hook slot is named after the worker so a refusal names it (func_<addr>). */
            if (!PE_GuestHook_Call(worker_slot(worker), "func_80076C34", worker, 2, argument, auxiliary, 0, 0, &ret)) return 0;
            if (PE_Port_StopEpoch() != epoch) return 0;   /* a worker that requested a stop: no tail, no restore */
        }
        mask = PE_LoadU32(W_MASKSAVE);
        (void)func_80073E10((uint16_t)mask);
        return 0;
    }
    {
        int32_t words = copy_bytes >> 2, i;
        (void)func_80073CF4(2u, PUMP_ADDR);
        if (copy_bytes != 0) {
            uint32_t e;
            /* Host-only: a source span this port cannot represent (retail would read wild memory) is a named
             * stop after the callback registration and before any publication. */
            if (!inl && words > 0 &&
                (argument + 4u * (uint32_t)words < argument || !PE_RangeIsRam(argument, 4u * (uint32_t)words))) {
                PE_GuestHook_Refuse("func_80076C34_enqueue_span", "func_80076C34", argument, 2, (uint32_t)copy_bytes,
                                    auxiliary, 0, 0, "enqueue source span is not representable guest RAM");
                return 0;
            }
            for (i = 0; i < words; i++) {
                uint32_t v = inl ? inl[i] : PE_LoadU32(argument + 4u * (uint32_t)i);
                q = PE_LoadU32(W_Q);
                PE_StoreU32(ENTRY_BASE + q * ENTRY_SIZE + 0xCu + 4u * (uint32_t)i, v);
            }
            q = PE_LoadU32(W_Q);
            q = PE_LoadU32(W_Q);
            e = ENTRY_BASE + q * ENTRY_SIZE;
            PE_StoreU32(e + 4u, e + 0xCu);
        } else {
            q = PE_LoadU32(W_Q);
            PE_StoreU32(ENTRY_BASE + q * ENTRY_SIZE + 4u, argument);
        }
        q = PE_LoadU32(W_Q);
        PE_StoreU32(ENTRY_BASE + q * ENTRY_SIZE + 8u, auxiliary);
        q = PE_LoadU32(W_Q);
        PE_StoreU32(ENTRY_BASE + q * ENTRY_SIZE, worker);
        q = PE_LoadU32(W_Q);
        mask = PE_LoadU32(W_MASKSAVE);
        PE_StoreU32(W_Q, (q + 1u) & 0x3Fu);
        (void)func_80073E10((uint16_t)mask);
        (void)func_80076EE4();
        if (PE_Port_StopEpoch() != epoch) return 0;       /* the pump hit a boundary: no fabricated count */
        q = PE_LoadU32(W_Q);
        r = PE_LoadU32(W_R);
        return (int32_t)((q - r) & 0x3Fu);
    }
}

int32_t pe_plat_gpu_swap(pe_addr_t worker, pe_addr_t argument, int32_t copy_bytes, uint32_t auxiliary)
{
    return swap_core(worker, argument, copy_bytes, auxiliary, 0);
}

/* The retail callers that build the two-word payload in their own frame and pass its
 * address with copy_bytes == 8 (checked against the same vectors with the payload
 * loads removed: pe_gpu_swap_check.c `inline8`). */
int32_t pe_plat_gpu_swap_inline8(pe_addr_t worker, uint32_t word0, uint32_t word1, uint32_t auxiliary)
{
    uint32_t w[2];
    w[0] = word0;
    w[1] = word1;
    pe_plat_gpu_swap_inline_record(worker, w, auxiliary);
    return swap_core(worker, 0u, 8, auxiliary, w);
}
