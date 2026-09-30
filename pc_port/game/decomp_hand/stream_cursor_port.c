/*
 * Hand adapters — serial stream cursor readers (0x80090xxx).
 *
 * The record's word 0 is a guest byte cursor: read it, post-increment it,
 * OR a flag bit into +0xF4 and store the fetched byte (zero-extended) into a
 * halfword field.  Bodies follow src/func_XXXXXXXX.c with the record and the
 * cursor as guest addresses.
 */
#include "pe_guest_decomp.h"
#include "hand_hi_protos.h"

/* src/func_8009059C.c: byte -> +0x110, flag 0x1000. */
void func_8009059C(pe_addr_t arg0)
{
    pe_addr_t v1 = PE_LoadU32(arg0);
    uint32_t v0;
    unsigned char byte;

    PE_StoreU32(arg0, v1 + 1u);
    v0 = PE_LoadU32(arg0 + 0xF4u);
    byte = PE_LoadU8(v1);
    v0 |= 0x1000u;
    PE_StoreU32(arg0 + 0xF4u, v0);
    PE_StoreU16(arg0 + 0x110u, byte);
}

/* src/func_800905C4.c: byte -> +0x112, flag 0x8000. */
void func_800905C4(pe_addr_t arg0)
{
    pe_addr_t v1 = PE_LoadU32(arg0);
    uint32_t v0;
    unsigned char byte;

    PE_StoreU32(arg0, v1 + 1u);
    v0 = PE_LoadU32(arg0 + 0xF4u);
    byte = PE_LoadU8(v1);
    v0 |= 0x8000u;
    PE_StoreU32(arg0 + 0xF4u, v0);
    PE_StoreU16(arg0 + 0x112u, byte);
}

/* src/func_80090AAC.c declares `func_8009059C(void)` and calls it with no
 * arguments; retail keeps the caller's $a0 live into that call (disassembly
 * 0x80090AAC: `move s0,a0` / `jal 0x8009059c` with a0 untouched, then
 * `move a0,s0` / `move a1,s1` for func_800905C4).  func_800905C4 reads only
 * $a0, so its $a1 argument has no effect. */
void func_80090AAC(int arg0, int arg1)
{
    (void)arg1;
    func_8009059C((pe_addr_t)arg0);
    func_800905C4((pe_addr_t)arg0);
}
