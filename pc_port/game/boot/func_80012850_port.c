/*
 * PE-BTL16 — opcode 0x09 ALU (12850) and opcode 0x05 skip-if-false
 * (1731C). Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80012850 — 244 words 0x80012850..0x80012C20, SHA-256
 * cec98712…b37a. D_800910A0[0x09]. sltiu *arg0, 0x18 then
 * jtbl_80010000. Args [subop, dst, a, b]. Always v0=1.
 * OOR (>=24) stores nothing. Subops 0x14/0x15 jal already-local
 * 3708C (7w 16.16 mul) and 370A8 (5w 16.16-ish div).
 *
 * func_8001731C — 16 words 0x8001731C..0x8001735C, SHA-256
 * 20d208e0…0b1f. D_800910A0[0x05]. If *arg0==0, gp+0x90 =
 * *(D2F0)+0x9C + (*arg1)<<1. Always v0=1.
 *
 * Live type-6 +0x138 is subop 0x09: cond[0] = (3 < actor+0xF0).
 * After 0xA stored 0 there, cond[0]=0, then 0x05 skips to +0x170.
 *
 * func_80017588 — 76 words 0x80017588..0x800176B8, SHA-256
 * f4bc3706…5e69. D_800910A0[0x14]. *arg0 is 1/2/3: store
 * actor+0x1A0 / actor+0x19C / task+4. *arg1<0 stores 0, else
 * *(D2F0)+0x9C + (*arg1)<<1. Always v0=1. Live type-6 +0x180
 * is code 2, rel 0x684 → actor+0x19C = base+0xD08.
 * Not BattleInput. Not M5.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_8009D300 0x8009D300u

/* func_8003708C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003708C_port.c (src/func_8003708C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* func_800370A8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800370A8_port.c (src/func_800370A8.c); hand port retired (switch1 lane, audit klass a-replaceable, generator-verified eligible). */

int func_80012850(pe_addr_t args)
{
    uint32_t sub;
    pe_addr_t dst;
    uint32_t a;
    uint32_t b;
    uint32_t r;
    int32_t sa;
    int32_t sb;
    int store;

    sub = PE_LoadU32(PE_LoadU32(args));
    if (sub >= 24u)
        return 1;

    dst = PE_LoadU32(args + 4u);
    a = PE_LoadU32(PE_LoadU32(args + 8u));
    b = 0u;
    if (sub <= 0x06u || (sub >= 0x09u && sub <= 0x16u && sub != 0x13u))
        b = PE_LoadU32(PE_LoadU32(args + 12u));

    sa = (int32_t)a;
    sb = (int32_t)b;
    r = 0u;
    store = 1;

    switch (sub) {
    case 0x00:
        r = a + b;
        break;
    case 0x01:
        r = a - b;
        break;
    case 0x02:
        r = a | b;
        break;
    case 0x03:
        r = a & b;
        break;
    case 0x04:
        r = a ^ b;
        break;
    case 0x05:
        r = (a != 0u || b != 0u) ? 1u : 0u;
        break;
    case 0x06:
        r = (a != 0u && b != 0u) ? 1u : 0u;
        break;
    case 0x07:
        r = (a == 0u) ? 1u : 0u;
        break;
    case 0x08:
        r = ~a;
        break;
    case 0x09:
        r = (sb < sa) ? 1u : 0u;
        break;
    case 0x0A:
        r = (sa < sb) ? 1u : 0u;
        break;
    case 0x0B:
        r = (a == b) ? 1u : 0u;
        break;
    case 0x0C:
        r = (sa >= sb) ? 1u : 0u;
        break;
    case 0x0D:
        r = (sa <= sb) ? 1u : 0u;
        break;
    case 0x0E:
        r = (a != b) ? 1u : 0u;
        break;
    case 0x0F:
        /* Original mult/mflo retains the low word, including overflow. */
        r = a * b;
        break;
    case 0x10:
        if (b == 0u || (b == 0xFFFFFFFFu && a == 0x80000000u))
            store = 0;
        else
            r = (uint32_t)(sa / sb);
        break;
    case 0x11:
        r = a << (b & 31u);
        break;
    case 0x12:
        r = (uint32_t)(sa >> (int32_t)(b & 31u));
        break;
    case 0x13:
        r = a;
        break;
    case 0x14:
        r = func_8003708C(a, b);
        break;
    case 0x15:
        if (((int32_t)b >> 8) == 0)
            store = 0;
        else
            r = func_800370A8(a, b);
        break;
    case 0x16:
        if (b == 0u || (b == 0xFFFFFFFFu && a == 0x80000000u))
            store = 0;
        else
            r = (uint32_t)(sa % sb);
        break;
    case 0x17:
        r = (uint32_t)(-sa);
        break;
    default:
        store = 0;
        break;
    }

    if (store)
        PE_StoreU32(dst, r);
    return 1;
}

/* func_8001731C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001731C_port.c (src/func_8001731C.c); hand port retired (port3 switch-over C). */

int func_80017588(pe_addr_t args)
{
    int32_t rel;
    uint32_t code;
    pe_addr_t actor;
    pe_addr_t dest;
    uint32_t addr;

    rel = (int32_t)PE_LoadU32(PE_LoadU32(args + 4u));
    code = PE_LoadU32(PE_LoadU32(args));
    actor = PE_LoadU32(GA_D_8009D2F0);
    if (rel < 0) {
        addr = 0u;
    } else {
        addr = PE_LoadU32(actor + 0x9Cu) + ((uint32_t)rel << 1);
    }
    if (code == 1u) {
        dest = actor + 0x1A0u;
    } else if (code == 2u) {
        dest = actor + 0x19Cu;
    } else if (code == 3u) {
        dest = PE_LoadU32(0x8009D300u) + 4u;
    } else {
        return 1;
    }
    PE_StoreU32(dest, addr);
    return 1;
}

/* func_80017D5C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017D5C_port.c (src/func_80017D5C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80017D7C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017D7C_port.c (src/func_80017D7C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80017D9C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017D9C_port.c (src/func_80017D9C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80017AE8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017AE8_port.c (src/func_80017AE8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80013300: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80013300_port.c (src/func_80013300.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80017EC4(pe_addr_t args)
{
    pe_addr_t actor;
    uint32_t imm;
    uint32_t cur;

    actor = PE_LoadU32(GA_D_8009D2F0);
    imm = PE_LoadU16(PE_LoadU32(args));
    cur = PE_LoadU8(actor + 0x0Fu);
    if (cur < imm)
        imm = cur;
    PE_StoreU32(actor + 0x14u, imm << 16);
    return 1;
}

/* func_80017B34: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017B34_port.c (src/func_80017B34.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80017B74: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80017B74_port.c (src/func_80017B74.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
