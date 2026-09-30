/*
 * PE-BTL47 — func_800131E8: opcode 0x12 script-task fork
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 70 words 0x800131E8..0x80013300, SHA-256 ae11241b…3e1a.
 * D_800910A0[0x12]. Zero jal; v0=1.
 *
 * PC = (actor+0x9C) + (*arg0 << 1). Pops D_8009CDFC via
 * the already-ported 12700 init (entry/zeros/delay=1/
 * serial). If (D300+8)&3 == 0, insert the new block at
 * current+0x24 (12700 a1=D300). Else 12700(a1=0) and
 * prepend onto actor+0xA8.
 *
 * Live type-2 scratch-miss skip hits three 0x12 then
 * 0x6A. Type-6 still waits on scratch[0]&4 before its
 * 0x12. Do not force that bit.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D2F0 0x8009D2F0u

int func_800131E8(pe_addr_t args)
{
    pe_addr_t task;
    pe_addr_t actor;
    pe_addr_t spawned;
    pe_addr_t pc;
    uint32_t imm;

    task = PE_LoadU32(GA_D_8009D300);
    actor = PE_LoadU32(GA_D_8009D2F0);
    imm = PE_LoadU32(PE_LoadU32(args));
    pc = PE_LoadU32(actor + 0x9Cu) + (imm << 1);

    if ((PE_LoadU16(task + 8u) & 3u) == 0u) {
        (void)func_80012700(pc, task);
    } else {
        pe_addr_t head;

        spawned = func_80012700(pc, 0u);
        head = PE_LoadU32(actor + 0xA8u);
        PE_StoreU32(spawned + 0x24u, head);
        if (head != 0u)
            PE_StoreU32(head + 0x28u, spawned);
        PE_StoreU32(actor + 0xA8u, spawned);
    }
    return 1;
}
