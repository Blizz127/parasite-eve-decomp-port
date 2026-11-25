/*
 * pe_plat_ds_gen31.c -- libds "text_5F4" (0x8007EB88), command-queue push.
 *
 * Independent clean-room re-implementation, written from black-box vectors
 * only (batch cdgen31, vectors.v1.txt, sha256
 * d9406d6b7e39130f10630afa57beb80f86fd1aa3b097a5fed7582b692ca919d0,
 * 6000 cases).  Nothing was copied from any other implementation of this
 * function or from any decompilation, disassembly or other source.
 *
 * Behaviour (as observed):
 *   The queue is a ring of 16-byte entries based at 0x800A3610; the 32-bit
 *   ring index lives at 0x800A3690.  The index word is re-read from guest RAM
 *   for every use (four reads in all), and entry addresses are formed in
 *   32-bit wrap-around arithmetic as 0x800A3610 + 16 * index, so index 8
 *   puts the entry's first word on top of the index word itself and index -1
 *   puts it just below the base.
 *     1. read index; store the 32-bit `sequence` at entry+0.
 *     2. read index again (it may have been rewritten by step 1); store the
 *        low byte of `status` at entry+4.
 *     3. read index again; call func_80080998(entry + 5, response).
 *     4. read index a fourth time; store (index + 1) to 0x800A3690; then, if
 *        that new value is >= 8 as a SIGNED 32-bit number, store 0 there too.
 *        (0x7FFFFFFF + 1 and 0x80000000 + 1 are negative: no wrap; -1 + 1 = 0:
 *        no wrap; 7, 8, 9, 15 wrap.)
 */
#include <stdint.h>
#include "pe_plat/cd.h"
#include "pe_guest_ram.h"

void pe_plat_ds_text_5f4(uint32_t sequence, uint32_t status, uint32_t response)
{
    uint32_t entry;
    uint32_t next;

    entry = 0x800A3610u + 16u * PE_LoadU32(0x800A3690u);
    PE_StoreU32(entry, sequence);

    entry = 0x800A3610u + 16u * PE_LoadU32(0x800A3690u);
    PE_StoreU8(entry + 4u, (uint8_t)status);

    entry = 0x800A3610u + 16u * PE_LoadU32(0x800A3690u);
    func_80080998(entry + 5u, response);

    next = PE_LoadU32(0x800A3690u) + 1u;
    PE_StoreU32(0x800A3690u, next);
    if ((int32_t)next >= 8)
        PE_StoreU32(0x800A3690u, 0u);
}
