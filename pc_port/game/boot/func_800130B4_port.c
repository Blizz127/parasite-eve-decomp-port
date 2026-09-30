/*
 * PE-BTL30 — opcode 0x11 flag-mask test 130B4.
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_800130B4 — 77 words 0x800130B4..0x800131E8, SHA-256
 * 41ee2a16…5bbd. D_800910A0[0x11]. Zero jal. Always v0=1.
 *
 * *arg0 is the code:
 *   0: if (D_8009D26C & mask) == mask then *arg2=1 else 0
 *   1: if (D_8009D1F4 & mask) == mask then *arg2=1 else 0
 *   2: if (D_8009D1E4 & mask) == mask then *arg2=1 else 0
 *   3: held-mask check, destructive LZCR mask write, hold-counter lookup
 *   else: no store
 *
 * Live type-5 +0x2FC: code 1, mask 0x100, dest local[4].
 * 3E974 / ResetTestState leave D1F4=0, so local[4]=0.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define GA_D_8009D26C 0x8009D26Cu
#define GA_D_8009D1F4 0x8009D1F4u
#define GA_D_8009D1E4 0x8009D1E4u

int func_800130B4(pe_addr_t args)
{
    uint32_t code;
    uint32_t mask;
    uint32_t flags;
    pe_addr_t dest;

    code = PE_LoadU32(PE_LoadU32(args));
    if (code > 3u)
        return 1;
    if (code == 3u) {
        pe_addr_t mask_arg = PE_LoadU32(args + 4u);
        flags = PE_LoadU32(GA_D_8009D26C);
        mask = PE_LoadU32(mask_arg);
        if ((flags & mask) != mask) {
            PE_StoreU32(PE_LoadU32(args + 8u), 0u);
            return 1;
        }
        PE_GTE_SetLZCS(mask);
        uint32_t index = 31u;
        if (mask != 0x80000000u) {
            PE_StoreU32(PE_LoadU32(args + 4u), g_pe_gte.lzcr);
            /* Reload after swc2: arguments may alias the pointer bank. */
            index -= PE_LoadU32(PE_LoadU32(args + 4u));
        }
        dest = PE_LoadU32(args + 8u);
        PE_StoreU32(dest, PE_LoadU32(0x800A7770u + (index << 2)));
        return 1;
    }

    mask = PE_LoadU32(PE_LoadU32(args + 4u));
    dest = PE_LoadU32(args + 8u);
    if (code == 0u)
        flags = PE_LoadU32(GA_D_8009D26C);
    else if (code == 1u)
        flags = PE_LoadU32(GA_D_8009D1F4);
    else
        flags = PE_LoadU32(GA_D_8009D1E4);

    if ((flags & mask) == mask)
        PE_StoreU32(dest, 1u);
    else
        PE_StoreU32(dest, 0u);
    return 1;
}
