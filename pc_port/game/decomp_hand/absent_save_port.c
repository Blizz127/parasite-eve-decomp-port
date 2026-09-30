/*
 * Hand adapters for the save-block serialisers matched in batch 7
 * (2026-09-24).  The generator rejects them because they advance the
 * pointer global D_800A0ED0 (a guest cursor: E12 pointer-global store) or
 * bind a guest-address callee result to a host pointer (E13).  Here the
 * cursor is a 32-bit guest word updated with PE_LoadU32/PE_StoreU32, and
 * every `memcpy` of the leaf is a byte copy in guest RAM (retail's libc
 * memcpy is a plain forward byte copy of `n` bytes).  Authority for every
 * line: the matched src/func_X.c named in each comment.  Tests:
 * pc_port/tests/test_port_absent.h (test_PORTABSENT_save_block).
 */
#include "pe_guest_decomp.h"

#define SAVE_CURSOR 0x800A0ED0u   /* D_800A0ED0: unsigned char * cursor */

static void save_copy(pe_addr_t dst, pe_addr_t src, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
        PE_StoreU8(dst + i, PE_LoadU8(src + i));
}

/* cursor <- (dst) or (src) of n bytes, then cursor += n. */
static void save_put(pe_addr_t src, uint32_t n)
{
    pe_addr_t c = PE_LoadU32(SAVE_CURSOR);
    save_copy(c, src, n);
    PE_StoreU32(SAVE_CURSOR, c + n);
}

static void save_get(pe_addr_t dst, uint32_t n)
{
    pe_addr_t c = PE_LoadU32(SAVE_CURSOR);
    save_copy(dst, c, n);
    PE_StoreU32(SAVE_CURSOR, c + n);
}

/* src/func_8003F800.c: append the field state to the save cursor, in order:
 * D_800A77F0[0x800], D_8009D2E8, D_8009D280, D_8009D1A0, D_800B0CDC (4
 * each), D_800B0CE0/CE2/CE4 (2 each), the bytes D_800B0CE6 and D_800BCFEE,
 * D_800B8A20[0x70], D_800B0CB0[0x18], D_8009D1B0[8]. */
void func_8003F800(void)
{
    save_put(0x800A77F0u, 0x800u);
    save_put(0x8009D2E8u, 4u);
    save_put(0x8009D280u, 4u);
    save_put(0x8009D1A0u, 4u);
    save_put(0x800B0CDCu, 4u);
    save_put(0x800B0CE0u, 2u);
    save_put(0x800B0CE2u, 2u);
    save_put(0x800B0CE4u, 2u);
    save_put(0x800B0CE6u, 1u);
    save_put(0x800BCFEEu, 1u);
    save_put(0x800B8A20u, 0x70u);
    save_put(0x800B0CB0u, 0x18u);
    save_put(0x8009D1B0u, 8u);
}

/* src/func_8003FBD8.c: the exact inverse read of func_8003F800, then
 * D_8009D1A0 &= 0xFFFF2679 and D_8009D2E8 &= ~9. */
void func_8003FBD8(void)
{
    save_get(0x800A77F0u, 0x800u);
    save_get(0x8009D2E8u, 4u);
    save_get(0x8009D280u, 4u);
    save_get(0x8009D1A0u, 4u);
    save_get(0x800B0CDCu, 4u);
    save_get(0x800B0CE0u, 2u);
    save_get(0x800B0CE2u, 2u);
    save_get(0x800B0CE4u, 2u);
    save_get(0x800B0CE6u, 1u);
    save_get(0x800BCFEEu, 1u);
    save_get(0x800B8A20u, 0x70u);
    save_get(0x800B0CB0u, 0x18u);
    save_get(0x8009D1B0u, 8u);
    PE_StoreU32(0x8009D1A0u, PE_LoadU32(0x8009D1A0u) & 0xFFFF2679u);
    PE_StoreU32(0x8009D2E8u, PE_LoadU32(0x8009D2E8u) & ~9u);
}

/* src/func_80040B80.c: build the 0x2000-byte save block at D_8009EED0.
 * Header D_800B8868[0x100]: zeroed, +2 = 0x11, +3 = 1, +0 = D_80010F48,
 * +4 = strcpy of the func_80040210(o->f18 - 0x40, D_800C0DE8) title
 * (func_80071A14 is BIOS A(19h) strcpy — pe_dis.sh 0x80071A14 0x10: `li
 * t2,0xA0; jr t2; li t1,0x19` — pc_port/platform/pe_bios_string.c), +0x60 = src[0x14..0x33], +0x80 = src[0x40..0xBF] where src =
 * func_8005DE70().  D_800A5D50 = 0x2000.  Block: zero 0x2000, cursor =
 * block, header, D_800C0DE0[0x12E4], func_8003F800(); then CRC-16/CCITT
 * (init 0xFFFF, poly 0x1021, MSB first) over the whole 0x2000 bytes — the
 * cursor-written tail included — and append ~crc & 0xFFFF as a 4-byte int
 * (the leaf's stack `x`, little-endian).  Returns 0. */
int func_80040B80(pe_addr_t o)
{
    pe_addr_t src = (pe_addr_t)func_8005DE70();
    unsigned int crc;
    unsigned short i;
    unsigned short j;
    int x;

    PE_StoreU32(0x800A5D50u, 0x2000u);
    (void)func_80071A24(0x800B8868u, 0x100u);
    PE_StoreU8(0x800B886Au, 0x11u);
    PE_StoreU8(0x800B886Bu, 1u);
    PE_StoreU16(0x800B8868u, PE_LoadU16(0x80010F48u));
    (void)func_80071A14(0x800B8868u + 4u,
                              func_80040210((int)PE_LoadU8(o + 0x18u) - 0x40,
                                            (int)PE_LoadU32(0x800C0DE8u)));
    save_copy(0x800B88C8u, src + 0x14u, 0x20u);
    save_copy(0x800B8868u + 0x80u, src + 0x40u, 0x80u);
    (void)func_80071A24(0x8009EED0u, 0x2000u);
    PE_StoreU32(SAVE_CURSOR, 0x8009EED0u);
    save_put(0x800B8868u, 0x100u);
    save_put(0x800C0DE0u, 0x12E4u);
    func_8003F800();
    crc = 0xFFFF;
    for (i = 0; i < 0x2000; i++) {
        crc ^= (unsigned int)PE_LoadU8(0x8009EED0u + i) << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    x = (int)(~crc & 0xFFFF);
    {
        pe_addr_t c = PE_LoadU32(SAVE_CURSOR);
        PE_StoreU32(c, (uint32_t)x);
        PE_StoreU32(SAVE_CURSOR, c + 4u);
    }
    return 0;
}
