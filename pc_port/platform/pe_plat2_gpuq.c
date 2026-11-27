/* libgpu command-queue pump (guest 0x80076EE4) and the deadline seed (guest
 * 0x800773D0).  Written clean-room from the black-box vector sets gpu2a
 * (docs/evidence/native-gpu2-20261126), the port's host GPU/DMA model API and
 * psx-spx; no SDK source, decompiled source, tables or disassembly were read.
 *
 * Contract (every line is traced to a vector set; GPU = psx-spx GPUSTAT,
 * DMA2 = psx-spx DMA channel 2 CHCR):
 *  - Register pointers.  The pump reaches the DMA2 CHCR through the pointer
 *    word 0x80095860 and GPUSTAT through the pointer word 0x80095854; it loads
 *    the pointer word and then the register it names.
 *  - Entry.  CHCR bit 24 (busy) set: return 1 at once, nothing else touched.
 *  - Otherwise SetIntrMask(0) (0x80073E10); its 16-bit result is stored as a
 *    word in 0x80095880.  Ring words: producer 0x80095874, consumer
 *    0x80095878, entries of 0x60 bytes from 0x800BD030 (+0 worker, +4 first
 *    argument, +8 second argument).
 *  - Loop, while producer != consumer (raw words) and CHCR bit 24 is clear
 *    (CHCR read after the ring test, every pass):
 *      * if ((consumer + 1) & 63) == producer and the word 0x80095758 (the
 *        DrawSync callback) is zero: DMACallback(2, 0) (0x80073CF4);
 *      * poll GPUSTAT until bit 26 (ready to receive a command word) is set;
 *      * call the entry's worker with (+4, +8); the entry is read through the
 *        consumer word, re-read for each field;
 *      * consumer = (consumer + 1) & 63, stored.
 *  - Exit: SetIntrMask(saved word 0x80095880).  When producer == consumer and
 *    CHCR bit 24 is clear and the word 0x80095754 is non-zero, and the DrawSync
 *    callback word 0x80095758 is non-zero: 0x80095754 is cleared and the
 *    callback is called with (the callback value, the second argument of the
 *    last entry the loop dispatched; no entry dispatched: not significant, 0).
 *  - Result: (producer - consumer) & 63.
 *  - Seed 0x800773D0: VSync(-1) + 0xF0 is stored to 0x80095888, 0 to
 *    0x8009588C, and returned.
 *
 * Host-only additions (not observable in the vectors):
 *  - the GPUSTAT wait leaves on PE_Port_ShouldStop(), and requests a stop after
 *    SPIN_BUDGET polls of a GPU that never becomes ready (named boundary);
 *  - the four retail workers that production queue entries hold are the host
 *    worker models (upload 0x80076664, readback 0x800768A0, clear 0x80076434,
 *    DMA-list start 0x80076B98); any other worker address is a guest hook call
 *    (PE_GuestHook_Call), refused loudly when nothing is known to be there;
 *    a worker that refused or requested a stop did not return: the consumer
 *    does not advance and the mask is not restored;
 *  - a consumer index outside the 64-entry ring (the original would read wild
 *    memory) is a named refusal before any entry read;
 *  - the seed reads the VBlank counter the GPU timeout check reads
 *    (pe_plat_gpu_host_vsync_query), the guest VSync(-1) of the vectors;
 *  - an optional observer (pe_plat/gpuq.h) reports each step to host evidence. */
#include <stdint.h>
#include <stdio.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_plat/gpuq.h"
#include "pe_plat/gpu_host.h"
#include "pe_plat/gpu_regs.h"

#define W_PRODUCER    0x80095874u
#define W_CONSUMER    0x80095878u
#define W_MASK_SAVE   0x80095880u
#define W_MARKER      0x80095754u
#define W_CALLBACK    0x80095758u
#define W_P_CHCR      0x80095860u
#define W_P_GPUSTAT   0x80095854u
#define W_DEADLINE    0x80095888u
#define W_POLLS       0x8009588Cu
#define RING_BASE     0x800BD030u
#define ENTRY_SIZE    0x60u
#define RING_ENTRIES  64u
#define CHCR_BUSY     0x01000000u
#define STAT_READY    0x04000000u
#define SPIN_BUDGET   4000000u
#define DEADLINE_TICKS 0xF0u

#define WORKER_UPLOAD   0x80076664u
#define WORKER_READBACK 0x800768A0u
#define WORKER_CLEAR    0x80076434u
#define WORKER_DMASTART 0x80076B98u

static const PeGpuqObserver *g_obs;

void pe_plat_gpuq_set_observer(const PeGpuqObserver *observer) { g_obs = observer; }

#define OBS(member, ...) do { if (g_obs && g_obs->member) g_obs->member(__VA_ARGS__); } while (0)

/* Hook-slot name "func_<address>" in persistent storage (the bootstrap layer
 * keeps the symbol pointer); a bounded intern table, full -> one fixed name. */
static const char *hook_slot(uint32_t fn)
{
    static char names[64][16];
    static uint32_t keys[64];
    static unsigned used;
    unsigned i;
    for (i = 0; i < used; i++)
        if (keys[i] == fn) return names[i];
    if (used >= 64u) return "func_pump_worker";
    keys[used] = fn;
    snprintf(names[used], sizeof names[used], "func_%08X", (unsigned)fn);
    return names[used++];
}

static uint32_t chcr_register(void)  { return pe_gpu_reg_word(W_P_CHCR, PE_GPU_REG_CHCR2, 4); }
static uint32_t gpu_register(void)   { return pe_gpu_reg_word(W_P_GPUSTAT, PE_GPU_REG_GPU1, 4); }

static uint32_t read_chcr(void) { return PE_LoadU32(chcr_register()); }

/* Call the entry's worker; 1 = it returned, 0 = it refused or stopped the port. */
static int call_worker(uint32_t worker, uint32_t argument, uint32_t auxiliary)
{
    unsigned epoch = PE_Port_StopEpoch();
    int ret = 0;
    switch (worker) {
    case WORKER_UPLOAD:   (void)pe_plat_gpu_upload_worker_host(argument, auxiliary); break;
    case WORKER_READBACK: (void)pe_plat_gpu_readback_worker_host(argument, auxiliary); break;
    case WORKER_CLEAR:    (void)pe_plat_gpu_clear_worker_host(argument, auxiliary); break;
    case WORKER_DMASTART: (void)pe_plat_gpu_dma_start(argument, auxiliary); break;
    default:
        if (!PE_GuestHook_Call(hook_slot(worker), "func_80076EE4", worker, 2, argument, auxiliary, 0, 0, &ret))
            return 0;
        break;
    }
    return PE_Port_StopEpoch() == epoch;
}

int pe_plat_gpuq_pump_ex(int *retail_returned)
{
    uint32_t consumer, producer, mask, argument, auxiliary = 0, worker, entry;
    if (retail_returned) *retail_returned = 0;
    OBS(entered);
    if (read_chcr() & CHCR_BUSY) {
        if (retail_returned) *retail_returned = 1;
        return 1;
    }
    mask = (uint16_t)func_80073E10(0);
    PE_StoreU32(W_MASK_SAVE, mask);
    OBS(mask_disabled, mask);
    while (PE_LoadU32(W_PRODUCER) != PE_LoadU32(W_CONSUMER) && !(read_chcr() & CHCR_BUSY)) {
        consumer = PE_LoadU32(W_CONSUMER);
        producer = PE_LoadU32(W_PRODUCER);
        if (((consumer + 1u) & (RING_ENTRIES - 1u)) == producer && PE_LoadU32(W_CALLBACK) == 0u) {
            (void)func_80073CF4(2u, 0u);
            OBS(dma_callback_removed);
        }
        {
            uint32_t gpu = gpu_register(), polls = 0;
            for (;;) {
                uint32_t status = PE_LoadU32(gpu);
                OBS(gpu_polled);
                if (status & STAT_READY) break;
                if (PE_Port_ShouldStop()) return 0;
                if (++polls > SPIN_BUDGET) {
                    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                    return 0;
                }
            }
            OBS(gpu_ready);
        }
        consumer = PE_LoadU32(W_CONSUMER);
        if (consumer >= RING_ENTRIES) {
            PE_GuestHook_Refuse("func_80076EE4_ring_span", "func_80076EE4", consumer, 0, 0, 0, 0, 0,
                                "queue consumer index is outside the 64-entry ring");
            return 0;
        }
        entry = RING_BASE + consumer * ENTRY_SIZE;
        argument = PE_LoadU32(entry + 4u);
        auxiliary = PE_LoadU32(entry + 8u);
        worker = PE_LoadU32(entry);
        OBS(worker_begin, worker, argument, auxiliary, consumer);
        if (!call_worker(worker, argument, auxiliary)) {
            OBS(worker_end, 0);
            return 0;
        }
        OBS(worker_end, 1);
        consumer = (PE_LoadU32(W_CONSUMER) + 1u) & (RING_ENTRIES - 1u);
        PE_StoreU32(W_CONSUMER, consumer);
        OBS(consumer_advanced, consumer);
    }
    mask = (uint16_t)PE_LoadU32(W_MASK_SAVE);
    (void)func_80073E10((uint16_t)mask);
    OBS(mask_restored, mask);
    if (PE_LoadU32(W_PRODUCER) == PE_LoadU32(W_CONSUMER) && !(read_chcr() & CHCR_BUSY) && PE_LoadU32(W_MARKER) != 0u) {
        uint32_t callback = PE_LoadU32(W_CALLBACK);
        if (callback != 0u) {
            unsigned epoch = PE_Port_StopEpoch();
            int ret = 0;
            PE_StoreU32(W_MARKER, 0u);
            OBS(callback_cleared);
            if (!PE_GuestHook_Call(hook_slot(callback), "func_80076EE4", callback, 2, callback, auxiliary, 0, 0, &ret) ||
                PE_Port_StopEpoch() != epoch) {
                OBS(callback_called, callback, 0);
                return 0;
            }
            OBS(callback_called, callback, 1);
        }
    }
    if (retail_returned) *retail_returned = 1;
    return (int)((PE_LoadU32(W_PRODUCER) - PE_LoadU32(W_CONSUMER)) & (RING_ENTRIES - 1u));
}

int pe_plat_gpuq_pump(void) { return pe_plat_gpuq_pump_ex(0); }

uint32_t pe_plat_gpuq_seed_deadline(void)
{
    uint32_t deadline = pe_plat_gpu_host_vsync_query() + DEADLINE_TICKS;
    PE_StoreU32(W_DEADLINE, deadline);
    PE_StoreU32(W_POLLS, 0u);
    return deadline;
}
