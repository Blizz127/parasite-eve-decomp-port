/* BIOS interrupt-chain services SysEnqIntRP (C(02h)) and SysDeqIntRP (C(03h)).
 * Clean-room: written from psx-spx kernelbios ("Priority Chains", "C(02h)
 * SysEnqIntRP", "C(03h) SysDeqIntRP", "BIOS Control Blocks",
 * https://psx-spx.consoledev.net/kernelbios/); no Sony code or tables were
 * read.  Contract taken from that text:
 *   - four priority chains, 0..3; each chain is a singly linked list of
 *     4-word records in guest RAM {00h next, 04h second function, 08h first
 *     function, 0Ch unused}, headed by the first word of its 8-byte
 *     Exception Control Block;
 *   - SysEnqIntRP inserts at the begin of the chain: record.next = old head,
 *     head = record (the other three record words are the caller's);
 *   - SysDeqIntRP is meant to search the chain and unlink the matching
 *     record.  psx-spx documents that the real service only handles the first
 *     element reliably (afterwards it reads an uninitialised stack word, so
 *     its result is unpredictable).  This model implements the documented
 *     INTENT -- find the record anywhere in the chain and unlink it -- because
 *     the real outcome for a later element is not deterministic and cannot be
 *     reproduced;
 *   - psx-spx gives no return value for either service: both return 0.
 *
 * Where the chain heads live: the retail kernel keeps the four ECBs in kernel
 * memory that the port does not model in the guest window (the same situation
 * as the event control blocks, see pe_sdk.h), so the four heads are host
 * state here.  They are cleared whenever the guest RAM allocation is reset or
 * reallocated (PE_RamGeneration), mirroring a reboot of the kernel tables.
 * The service only manipulates the chain; nothing here dispatches the
 * handlers (interrupt delivery is the host IRQ model's business).
 *
 * Robustness: records and the walk are validated against guest RAM; an
 * invalid record address or priority is ignored (return 0), and a walk is
 * bounded so a cyclic or corrupted chain in guest RAM cannot hang the host. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/bios_intrp.h"

#define REC_SIZE      0x10u
#define WALK_LIMIT    4096u

static uint32_t g_head[PE_BIOS_INTRP_PRIORITIES];
static uint64_t g_generation;
static int g_generation_valid;

static void sync_generation(void)
{
    uint64_t g = PE_RamGeneration();
    if (!g_generation_valid || g != g_generation) {
        unsigned i;
        for (i = 0; i < PE_BIOS_INTRP_PRIORITIES; i++) g_head[i] = 0u;
        g_generation = g;
        g_generation_valid = 1;
    }
}

void pe_plat_bios_intrp_reset(void)
{
    unsigned i;
    for (i = 0; i < PE_BIOS_INTRP_PRIORITIES; i++) g_head[i] = 0u;
    g_generation = PE_RamGeneration();
    g_generation_valid = 1;
}

uint32_t pe_plat_bios_intrp_head(uint32_t priority)
{
    if (priority >= PE_BIOS_INTRP_PRIORITIES) return 0u;
    sync_generation();
    return g_head[priority];
}

static int record_ok(uint32_t record)
{
    return record != 0u && PE_RangeIsRam(record, REC_SIZE);
}

int pe_plat_bios_sys_enq_int_rp(uint32_t priority, uint32_t record)
{
    if (priority >= PE_BIOS_INTRP_PRIORITIES) return 0;
    sync_generation();
    if (!record_ok(record)) return 0;
    PE_StoreU32(record, g_head[priority]);   /* next = old first element */
    g_head[priority] = record;
    return 0;
}

int pe_plat_bios_sys_deq_int_rp(uint32_t priority, uint32_t record)
{
    uint32_t cur, prev = 0u;
    unsigned steps;
    if (priority >= PE_BIOS_INTRP_PRIORITIES) return 0;
    sync_generation();
    if (!record_ok(record)) return 0;
    cur = g_head[priority];
    for (steps = 0u; cur != 0u && steps < WALK_LIMIT; steps++) {
        if (!record_ok(cur)) return 0;       /* corrupted link: leave the chain alone */
        if (cur == record) {
            uint32_t next = PE_LoadU32(cur);
            if (prev == 0u) g_head[priority] = next;
            else PE_StoreU32(prev, next);
            return 0;
        }
        prev = cur;
        cur = PE_LoadU32(cur);
    }
    return 0;
}
