/* libgpu primitive entry points DrawPrim 0x80075358, DrawOTag 0x800753B4,
 * ClearOTagR 0x800752AC, ClearImage 0x80074F44, PutDrawEnv 0x80075424,
 * DrawOTagEnv 0x800754E4, the record-pool initialiser 0x8007A214 and the stream
 * range setter 0x8007C544, written clean-room from the black-box vector sets
 * gpu2c (docs/evidence/native-gpu2-20261126), the port's host GPU/DMA model API
 * and psx-spx; no SDK source, decompiled source, tables or disassembly were read.
 *
 * Derivation (each rule is traced to vectors, see AUTHOR-REPORT.md):
 *  - The library function table is the pointer word 0x80095744; slot k is the
 *    word at table + 4*k.  Every worker below is reached through a slot, so a
 *    guest address in the slot is called with the original's argument
 *    registers (PE_GuestCall: the retail `jalr`); no worker is inlined.
 *  - The debug level is the byte 0x8009574E.  When it is above 1 the entry
 *    first calls the print function stored at word 0x80095748 with a format
 *    pointer (per entry) and the entry's own arguments (ClearOTagR also the count).
 *  - DrawPrim: slot 15 is called with 0, then slot 5 with (packet + 4, the byte
 *    at packet + 3); the result is slot 5's.  Only that one packet byte is read
 *    (host-only: a packet whose header or payload span is not guest RAM is refused before slot 15).
 *  - DrawOTag: slot 2 is called with (slot 6, ot, 0, 0); its result is returned.
 *  - ClearOTagR: slot 11 is called with (ot, count) (the third argument register still
 *    holds the count when the trace call ran); then the word 0x8009580C is
 *    set to 0x040957F8 and ot[0] to 0x0009580C (the terminator node).  The original leaves ot in v0; the binding is void
 *    because the port's generated callers declare it void.
 *  - ClearImage: the rectangle is reported through 0x80074E28 with the name
 *    pointer 0x800118BC, then slot 2 is called with (slot 3, rect, 8, color),
 *    color = red | green << 8 | blue << 16.
 *  - PutDrawEnv: pointer p = env + 0x1C; 0x80075EE0 builds the packet at p from
 *    env, p's tag word gets its low 24 bits set, slot 2 is called with (slot 6, p,
 *    0x40, 0) and the 0x5C environment bytes are copied to 0x8009575C; returns
 *    env.  DrawOTagEnv is the same with the tag's low 24 bits replaced by the
 *    ordering-table address (the original leaves the memcpy result in v0; the binding is void because the
 *    port's hand-written callers declare it void).
 *  - Record pool: 0x8007A214 stores base at 0x800C0DC8 and count at 0x800C20C4,
 *    then calls the worker 0x8007A244.  The stream range setter stores mode at
 *    0x800C0DC0, start at 0x800B6918 and end at 0x800C0DBC.
 *  - Host-only additions (not observable in the vectors): a call that raises a
 *    stop (PE_Port_ShouldStop) ends the entry without its tail; DrawPrim
 *    returns -1 then.  A RECT that is not guest memory (a host stack variable)
 *    has no guest address: ClearImage then issues it through the existing host
 *    seam PE_DispatchClearRect (same argument packing, in-place clamp) when slot 2 is the swap 0x80076C34, and is a
 *    named refusal (PE_GuestHook_Refuse) otherwise. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "game_port.h"
#include "pe_plat/gpuprim.h"
#include "pe_plat/bios_string.h"

#define W_TABLE   0x80095744u
#define W_PRINT   0x80095748u
#define B_DEBUG   0x8009574Eu
#define ENV_COPY  0x8009575Cu
#define OT_NODE   0x8009580Cu
#define OT_NODE_WORD 0x040957F8u
#define OT_HEAD_WORD 0x0009580Cu
#define SLOT_SWAP      2u   /* the queueing/issue function */
#define SLOT_WORKER_A  3u   /* ClearImage's worker */
#define SLOT_DRAWPRIM  5u
#define SLOT_DRAWWORK  6u   /* DrawOTag / environment worker argument */
#define SLOT_OTC       11u
#define SLOT_SYNC      15u
#define NAME_RECT      0x800118BCu
#define SWAP_ADDRESS   0x80076C34u
#define FMT_DRAWOTAG   0x80011928u
#define FMT_CLEAROTAGR 0x80011910u
#define FMT_PUTDRAWENV 0x8001193Cu
#define FMT_DRAWOTAGENV 0x80011954u
#define POOL_BASE_WORD  0x800C0DC8u
#define POOL_COUNT_WORD 0x800C20C4u
#define RANGE_MODE      0x800C0DC0u
#define RANGE_START     0x800B6918u
#define RANGE_END       0x800C0DBCu
#define ENV_PACKET      0x1Cu
#define ENV_SIZE        0x5Cu
#define PACKET_BYTES    0x40u
#define LINK_MASK       0x00FFFFFFu

void pe_plat_gpu_report_rect_host(uint32_t name, const void *rect);
int PE_DispatchClearRect(uint32_t worker, RECT *rect, uint32_t color);   /* host seam (psx_compat.h): RECT held by the host */

static uint32_t slot(uint32_t index) { return PE_LoadU32(PE_LoadU32(W_TABLE) + 4u * index); }

/* Debug trace: print function from the word 0x80095748 (only when the level byte is above 1). */
static int debug_print(const char *site, uint32_t format, uint32_t a, uint32_t b, unsigned arity)
{
    if (PE_LoadU8(B_DEBUG) > 1u) (void)PE_GuestCall(site, PE_LoadU32(W_PRINT), arity, format, a, b, 0);
    return PE_Port_ShouldStop();            /* host-only: an unresolved print hook ends the entry before anything is issued */
}

int pe_plat_gpuprim_draw_prim(uint32_t packet)
{
    uint32_t bytes;
    int r;
    /* Host-only validation (the vectors never contain a packet outside guest RAM; hardware would fault): a header or
     * payload span that is not guest RAM is a named refusal before anything is issued.  The count byte is the only
     * packet byte read, as in the original. */
    if (!PE_RangeIsRam(packet, 4u)) goto bad_packet;
    bytes = PE_LoadU8(packet + 3u);
    if (bytes != 0u && !PE_RangeIsRam(packet + 4u, 4u * bytes)) goto bad_packet;
    (void)PE_GuestCall("func_80075358", slot(SLOT_SYNC), 1, 0, 0, 0, 0);
    if (PE_Port_ShouldStop()) return -1;
    r = PE_GuestCall("func_80075358", slot(SLOT_DRAWPRIM), 2, packet + 4u, bytes, 0, 0);
    if (PE_Port_ShouldStop()) return -1;
    return r;
bad_packet:
    PE_GuestHook_Refuse("func_80075358_packet", "func_80075358", packet, 1, packet, 0, 0, 0,
                        "DrawPrim packet header or payload is not guest RAM");
    return -1;
}

int pe_plat_gpuprim_draw_otag(uint32_t ot)
{
    uint32_t arg;
    if (debug_print("func_800753B4", FMT_DRAWOTAG, ot, 0, 2)) return 0;
    arg = slot(SLOT_DRAWWORK);
    return PE_GuestCall("func_800753B4", slot(SLOT_SWAP), 4, arg, ot, 0, 0);
}

void pe_plat_gpuprim_clear_otag_reverse(uint32_t ot, int count)
{
    /* The trace call leaves the count in the third argument register; the slot call then still carries it
     * (the vectors show it as a significant third argument exactly when the trace ran). */
    uint32_t third = 0;
    if (PE_LoadU8(B_DEBUG) > 1u) {
        (void)PE_GuestCall("func_800752AC", PE_LoadU32(W_PRINT), 3, FMT_CLEAROTAGR, ot, (uint32_t)count, 0);
        if (PE_Port_ShouldStop()) return;
        third = (uint32_t)count;
    }
    (void)PE_GuestCall("func_800752AC", slot(SLOT_OTC), 2, ot, (uint32_t)count, third, 0);
    if (PE_Port_ShouldStop()) return;
    PE_StoreU32(OT_NODE, OT_NODE_WORD);
    PE_StoreU32(ot, OT_HEAD_WORD);
}

/* Guest address of a RECT the caller holds, or 0 when it lives outside guest RAM. */
static uint32_t rect_guest_address(const void *rect)
{
    uintptr_t base = (uintptr_t)PE_Translate(PE_RAM_BASE, 1u), p = (uintptr_t)rect;
    if (p >= base && p - base < PE_RAM_SIZE) return PE_RAM_BASE + (uint32_t)(p - base);
    return 0u;
}

int pe_plat_gpuprim_clear_image(RECT *rect, uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t color = (uint32_t)red | ((uint32_t)green << 8) | ((uint32_t)blue << 16);
    uint32_t fn, worker, at;
    pe_plat_gpu_report_rect_host(NAME_RECT, rect);
    fn = slot(SLOT_SWAP);
    worker = slot(SLOT_WORKER_A);
    at = rect_guest_address(rect);
    if (at != 0u) return PE_GuestCall("func_80074F44", fn, 4, worker, at, 8, color);
    /* Host-only: a RECT the host holds (the port's own callers pass the address of a local) has no guest address.  When
     * slot 2 is the swap the existing host seam issues it with the retail argument packing (and the in-place clamp); any
     * other slot value cannot be given the RECT and is a named refusal. */
    if (fn == SWAP_ADDRESS) return PE_DispatchClearRect(worker, rect, color);
    PE_GuestHook_Refuse("func_80074F44_rect", "func_80074F44", fn, 4, worker, 0, 8, color,
                        "ClearImage RECT has no guest address and slot 2 is not the swap");
    return 0;
}

/* Common tail of the two environment entries: build the packet, link word, issue, copy. */
static uint32_t env_tail(const char *site, uint32_t env, uint32_t ot_or_zero, int splice)
{
    uint32_t packet = env + ENV_PACKET, tag, arg, r;
    (void)PE_GuestCall(site, 0x80075EE0u, 2, packet, env, 0, 0);
    if (PE_Port_ShouldStop()) return 0;
    tag = PE_LoadU32(packet);
    tag = splice ? (tag & ~LINK_MASK) | (ot_or_zero & LINK_MASK) : tag | LINK_MASK;
    PE_StoreU32(packet, tag);
    arg = slot(SLOT_DRAWWORK);
    (void)PE_GuestCall(site, slot(SLOT_SWAP), 4, arg, packet, PACKET_BYTES, 0);
    if (PE_Port_ShouldStop()) return 0;
    r = func_80071A34(ENV_COPY, env, (int32_t)ENV_SIZE);
    return r;
}

uint32_t pe_plat_gpuprim_put_draw_env(uint32_t env)
{
    if (debug_print("func_80075424", FMT_PUTDRAWENV, env, 0, 2)) return env;
    (void)env_tail("func_80075424", env, 0, 0);
    return env;
}

void pe_plat_gpuprim_draw_otag_env(uint32_t ot, uint32_t env)
{
    if (debug_print("func_800754E4", FMT_DRAWOTAGENV, ot, 0, 2)) return;
    (void)env_tail("func_800754E4", env, ot, 1);
}

void pe_plat_gpuprim_pool_init(uint32_t record_base, uint32_t record_count)
{
    PE_StoreU32(POOL_BASE_WORD, record_base);
    PE_StoreU32(POOL_COUNT_WORD, record_count);
    pe_plat_gpuprim_pool_clear(record_base, record_count);
}

void pe_plat_gpuprim_stream_set_range(uint32_t mode, uint32_t start, uint32_t end)
{
    PE_StoreU32(RANGE_MODE, mode);
    PE_StoreU32(RANGE_START, start);
    PE_StoreU32(RANGE_END, end);
}
