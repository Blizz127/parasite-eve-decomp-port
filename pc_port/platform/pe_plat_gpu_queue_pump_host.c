/* Independent native GPU queue pump; callback and tracing adapters retain host boundaries. */
#include <stddef.h>
#include <stdint.h>
#include "psx_compat.h"
#include "pe_gpu.h"
extern PeGpuPumpTrace *pe_plat_pump_trace_existing(void);
extern void pe_plat_pump_enter_existing(void);
extern uint64_t pe_plat_pump_next_existing(void);
extern int pe_plat_pump_load_existing(uint32_t, uint32_t, uint32_t *);
extern int pe_plat_pump_worker_existing(pe_addr_t, pe_addr_t, uint32_t, uint32_t);
extern int pe_plat_pump_drawsync_existing(pe_addr_t);
#define MARKER 0x80095754u
#define CALLBACK 0x80095758u
#define PRODUCER 0x80095874u
#define CONSUMER 0x80095878u
#define SAVED_MASK 0x80095880u
int PE_func_80076EE4_Pump(int *returned) {
    PeGpuPumpTrace *t = pe_plat_pump_trace_existing();
    uint32_t consumer, producer, argument, auxiliary, worker;
    if (returned) *returned = 0;
    pe_plat_pump_enter_existing();
    if (PE_GPU_ReadDMA2CHCR() & 0x01000000u) {
        if (returned) *returned = 1;
        return 1;
    }
    uint16_t old = func_80073E10(0);
    PE_StoreU32(SAVED_MASK, old);
    t->saved_mask = old;
    t->mask_disable_order = pe_plat_pump_next_existing();
    while (PE_LoadU32(PRODUCER) != PE_LoadU32(CONSUMER) &&
           !(PE_GPU_ReadDMA2CHCR() & 0x01000000u)) {
        consumer = PE_LoadU32(CONSUMER);
        producer = PE_LoadU32(PRODUCER);
        if (((consumer + 1u) & 63u) == producer && !PE_LoadU32(CALLBACK)) {
            (void)func_80073CF4(2, 0);
            t->callback_removal_order = pe_plat_pump_next_existing();
        }
        do { ++t->gpustat_reads; } while (!(PE_GPU_ReadStatus() & 0x04000000u));
        t->gpustat_ready_order = pe_plat_pump_next_existing();
        consumer = PE_LoadU32(CONSUMER);
        if (!pe_plat_pump_load_existing(PE_LoadU32(CONSUMER), 4, &argument) ||
            !pe_plat_pump_load_existing(PE_LoadU32(CONSUMER), 8, &auxiliary) ||
            !pe_plat_pump_load_existing(consumer, 0, &worker)) return 0;
        if (!pe_plat_pump_worker_existing(worker, argument, auxiliary, consumer)) return 0;
        consumer = (PE_LoadU32(CONSUMER) + 1u) & 63u;
        PE_StoreU32(CONSUMER, consumer);
        t->consumer_after_worker = consumer;
        t->consumer_advance_order = pe_plat_pump_next_existing();
    }
    old = (uint16_t)PE_LoadU32(SAVED_MASK);
    (void)func_80073E10(old);
    t->restored_mask = old;
    t->mask_restore_order = pe_plat_pump_next_existing();
    if (PE_LoadU32(PRODUCER) == PE_LoadU32(CONSUMER) &&
        !(PE_GPU_ReadDMA2CHCR() & 0x01000000u) && PE_LoadU32(MARKER)) {
        uint32_t callback = PE_LoadU32(CALLBACK);
        if (callback) {
            PE_StoreU32(MARKER, 0);
            t->drawsync_clear_order = pe_plat_pump_next_existing();
            if (!pe_plat_pump_drawsync_existing(callback)) return 0;
        }
    }
    if (returned) *returned = 1;
    return (int)((PE_LoadU32(PRODUCER) - PE_LoadU32(CONSUMER)) & 63u);
}

int pe_plat_gpu_queue_pump_host(void) { return PE_func_80076EE4_Pump(NULL); }
