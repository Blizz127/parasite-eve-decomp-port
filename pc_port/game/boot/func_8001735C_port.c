/*
 * PE-BTL22 — opcode 0x08 spawn wrapper 1735C, post-ctor 1AA78,
 * and hit-test 1C614 (translated retail, not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_8001735C — 38 words 0x8001735C..0x800173F4, SHA-256
 * a367c31f…9af8. D_800910A0[0x08]. Zero TEXT jal sites (jalr
 * from 17018). Always v0=1 (re-fetch).
 *
 *   desc[0] = (u8)*arg0          # type
 *   desc[1] = (u8)*arg1          # idB
 *   actor = 35038(&desc, *D_8009D2F0, 1)
 *   actor+0x28 = *arg2
 *   actor+0x2C = *arg3
 *   actor+0x30 = *arg4           # delay of jal 1AA78
 *   1AA78(actor)
 *   return 1
 *
 * Live type-1 second visit: three 0x08s, kinds 0, argc 5:
 *   type 3, idB 0, +0x28=0x20000, +0x2C=0xFBC80000, +0x30=0x2710000
 *   type 0, idB 0, all zeros
 *   type 5, idB 0, type imm 5, rest zeros
 * Parent is the current type-1 actor (D2F0), not 0, so 35038
 * inserts as sibling after the parent. 17018 already advanced
 * CE00 past the insn before jalr; v0=1 re-fetches the next 0x08.
 *
 * Descriptor lives at ROM sp+16. Native has no guest $sp:
 * GA_DESC 0x801FF4A0 is the same class of APPROXIMATION as the
 * 17018 arg frame (moved out of the 0x80120D00 overlay load window
 * with it; old address 0x80120F70). ROM has no null check on 35038's return.
 *
 * func_8001AA78 — 154 words 0x8001AA78..0x8001ACE0, SHA-256
 * eaf36cc4…a935. Callers 12C9C and 1735C@173D4. Void.
 * Direct jals: 1C614 (twice) and already-ported 3708C (three).
 *
 *   if actor+0x98 & 0x80: return
 *   if lhu(*D_8009D1FC + 2) == 0: return
 *   D_8009D1D8 == 0: walk *(D_8009CE08 + i*4), count=lhu(+2),
 *     indices at +4, stride 22 from *(D1FC+0x1C); 1C614;
 *     match stores +0x1A4/+0x1A8 and, if !(+0x98&2),
 *     +0x2C = (s16)entry[0] << 16
 *   D_8009D1D8 != 0: count=lhu(+4), indices at +6, stride 28;
 *     match then three 3708C 16.16 muls into +0x2C
 *
 * 35038 ORs +0x98 with 0xE0 when +0x1AC==0, so the 0x80 gate
 * fires for empty-B0E70 types. Live m0005i 6B4F8 hdr+0x0C
 * writes B0E70[idB] for idB 2 and 5 only; type 3 stays 0.
 * Type 0's B0E70[0] is the later 6C118 package bind (not this
 * cut). 1A918 live obj lhu(+2)=1 and +0x20!=0, so a nonempty
 * +0x1AC actor takes the D1D8!=0 arm.
 *
 * func_8001C614 — 114 words 0x8001C614..0x8001C7DC, SHA-256
 * 53432d0c…942d. Zero jal. a0=record, a1/a2 are the already
 * sign-extended actor+0x2A / +0x32. Three-edge crossing test
 * against *(D1FC+0x18) pairs. Returns t0 (0 or 1).
 *
 * func_80012C20 — 151 words 0x80012C20..0x80012E7C, SHA-256
 * 18e047de…22a8. D_800910A0[0x0B]. sltiu *arg0, 7 then
 * jtbl_80010060. Always v0=1. OOR stores nothing. Code 0
 * writes +0x28/2C/30, jals 1AA78, snapshots +0x40/44/48,
 * and if actor==D254 ORs 0x80 into D_800BCF88. Live type-5
 * first visit is code 0 then code 5 (sh +0x38/3A/3C).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include <stdio.h>
#include <stdlib.h>

static int PeOp77DbgOn(void)
{
    static int on = -1;
    if (on < 0) on = getenv("PE_ROUTE_OP77DBG") != NULL;
    return on;
}

#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D1FC 0x8009D1FCu
#define GA_D_8009D1D8 0x8009D1D8u
#define GA_D_8009CE08 0x8009CE08u
#define GA_D_800BCF88 0x800BCF88u
/* Host stand-in for ROM 1735C sp+16. APPROXIMATION: no guest $sp. */
#define GA_DESC       0x801FF4A0u  /* 2 bytes; was 0x80120F70 (overlay window) */
/* Host stand-in for ROM 14DA0 sp+16. APPROXIMATION: no guest $sp. */
#define GA_POLY       0x801FF480u  /* 4 x 8 bytes; was 0x80120F30 (overlay window) */

static int32_t pe_mult_lo(int32_t a, int32_t b)
{
    return (int32_t)((int64_t)a * (int64_t)b);
}

int func_8001C614(pe_addr_t rec, int a1, int a2)
{
    pe_addr_t t3;
    pe_addr_t t6;
    pe_addr_t row;
    uint32_t t0;
    uint32_t t5;
    uint32_t t1;
    uint32_t a3;
    uint32_t a0_saved;
    uint32_t a2_saved;
    int32_t t4;
    int32_t t2;
    uint32_t idx;

    t3 = rec;
    if (PE_LoadU32(GA_D_8009D1D8) != 0u) {
        idx = PE_LoadU16(t3 + 0xCu);
        row = PE_LoadU32(PE_LoadU32(GA_D_8009D1FC) + 0x18u) + idx * 6u;
        t1 = PE_LoadU16(row + 4u);
        a3 = PE_LoadU16(row);
    } else {
        idx = PE_LoadU16(t3 + 6u);
        row = PE_LoadU32(PE_LoadU32(GA_D_8009D1FC) + 0x18u) + idx * 4u;
        t1 = PE_LoadU16(row + 2u);
        a3 = PE_LoadU16(row);
    }

    t0 = 0u;
    t5 = 0u;
    t6 = PE_LoadU32(GA_D_8009D1FC);
    t4 = (int32_t)(int16_t)a2;
    t2 = (int32_t)(int16_t)a1;

    do {
        a2_saved = a3;
        a0_saved = t1;
        if (PE_LoadU32(GA_D_8009D1D8) != 0u) {
            idx = PE_LoadU16(t3 + 8u);
            row = PE_LoadU32(t6 + 0x18u) + idx * 6u;
            t1 = PE_LoadU16(row + 4u);
            a3 = PE_LoadU16(row);
        } else {
            idx = PE_LoadU16(t3 + 2u);
            row = PE_LoadU32(t6 + 0x18u) + idx * 4u;
            t1 = PE_LoadU16(row + 2u);
            a3 = PE_LoadU16(row);
        }

        {
            int32_t y2 = (int32_t)(int16_t)t1;
            int32_t y1 = (int32_t)(int16_t)a0_saved;
            int32_t x2 = (int32_t)(int16_t)a3;
            int32_t x1 = (int32_t)(int16_t)a2_saved;
            int y_lt = t4 < y2;
            int x_cross = 0;

            if (y_lt) {
                if (t4 < y1)
                    goto next;
            } else if (!(t4 < y1)) {
                goto next;
            }

            if (t2 < x2) {
                if (t2 < x1)
                    goto toggle;
                x_cross = 1;
            } else if (t2 < x1) {
                x_cross = 1;
            } else {
                goto next;
            }

            if (x_cross) {
                int32_t dy = y1 - y2;
                int32_t dx = x1 - x2;
                int32_t lhs = pe_mult_lo(dx, t4 - y2);
                int32_t rhs = pe_mult_lo(dy, t2 - x2);
                int pass;

                if (dy < 0)
                    pass = lhs < rhs;
                else
                    pass = rhs < lhs;
                if (!pass)
                    goto next;
            }
        }
    toggle:
        t0 = t0 < 1u;
    next:
        t5 += 1u;
        t3 += 2u;
    } while (t5 < 3u);

    return (int)t0;
}

void func_8001AA78(pe_addr_t actor)
{
    pe_addr_t obj;
    unsigned int ntbl;
    unsigned int i;
    unsigned int j;
    unsigned int count;
    int16_t sx;
    int16_t sz;
    pe_addr_t entry;
    pe_addr_t rec;
    uint32_t idx;
    uint32_t base;

    if ((PE_LoadU32(actor + 0x98u) & 0x80u) != 0u)
        return;

    obj = PE_LoadU32(GA_D_8009D1FC);
    sx = (int16_t)PE_LoadU16(actor + 0x2Au);
    ntbl = PE_LoadU16(obj + 2u);
    sz = (int16_t)PE_LoadU16(actor + 0x32u);
    if (ntbl == 0u)
        return;

    for (i = 0u; i < ntbl; i++) {
        entry = PE_LoadU32(PE_LoadU32(GA_D_8009CE08) + i * 4u);
        if (PE_LoadU32(GA_D_8009D1D8) == 0u) {
            count = PE_LoadU16(entry + 2u);
            for (j = 0u; j < count; j++) {
                idx = PE_LoadU16(entry + 4u + j * 2u);
                rec = PE_LoadU32(obj + 0x1Cu) + idx * 22u;
                if (func_8001C614(rec, sx, sz) != 0) {
                    PE_StoreU32(actor + 0x1A4u, rec);
                    PE_StoreU32(actor + 0x1A8u, rec);
                    if ((PE_LoadU32(actor + 0x98u) & 2u) == 0u)
                        PE_StoreU32(actor + 0x2Cu,
                                    (uint32_t)(int32_t)(int16_t)
                                        PE_LoadU16(entry) << 16);
                    return;
                }
            }
        } else {
            count = PE_LoadU16(entry + 4u);
            for (j = 0u; j < count; j++) {
                idx = PE_LoadU16(entry + 6u + j * 2u);
                rec = PE_LoadU32(obj + 0x1Cu) + idx * 28u;
                if (func_8001C614(rec, sx, sz) != 0) {
                    uint32_t r0;
                    uint32_t r1;
                    uint32_t row;

                    PE_StoreU32(actor + 0x1A4u, rec);
                    PE_StoreU32(actor + 0x1A8u, rec);
                    if ((PE_LoadU32(actor + 0x98u) & 2u) != 0u)
                        return;
                    row = PE_LoadU16(rec + 2u) * 12u;
                    base = PE_LoadU32(GA_D_8009D1D8);
                    r0 = func_8003708C(PE_LoadU32(base + row),
                                       PE_LoadU32(actor + 0x28u));
                    r1 = func_8003708C(PE_LoadU32(base + row + 8u),
                                       PE_LoadU32(actor + 0x30u));
                    PE_StoreU32(actor + 0x2Cu,
                                func_8003708C(PE_LoadU32(rec + 4u) - r0 - r1,
                                              PE_LoadU32(base + row + 4u)));
                    return;
                }
            }
        }
    }
}

int func_8001735C(pe_addr_t args)
{
    pe_addr_t actor;

    PE_StoreU8(GA_DESC, (uint8_t)PE_LoadU32(PE_LoadU32(args)));
    PE_StoreU8(GA_DESC + 1u, (uint8_t)PE_LoadU32(PE_LoadU32(args + 4u)));
    actor = func_80035038(GA_DESC, PE_LoadU32(GA_D_8009D2F0), 1u);
    PE_StoreU32(actor + 0x28u, PE_LoadU32(PE_LoadU32(args + 8u)));
    PE_StoreU32(actor + 0x2Cu, PE_LoadU32(PE_LoadU32(args + 12u)));
    PE_StoreU32(actor + 0x30u, PE_LoadU32(PE_LoadU32(args + 16u)));
    func_8001AA78(actor);
    return 1;
}

int func_80012C20(pe_addr_t args)
{
    uint32_t code;
    pe_addr_t actor;
    uint32_t a;
    uint32_t b;
    uint32_t c;

    code = PE_LoadU32(PE_LoadU32(args));
    if (code >= 7u)
        return 1;

    actor = PE_LoadU32(GA_D_8009D2F0);
    a = PE_LoadU32(PE_LoadU32(args + 4u));
    b = PE_LoadU32(PE_LoadU32(args + 8u));
    c = PE_LoadU32(PE_LoadU32(args + 12u));

    if (code == 0u) {
        PE_StoreU32(actor + 0x28u, a);
        PE_StoreU32(actor + 0x2Cu, b);
        PE_StoreU32(actor + 0x30u, c);
        func_8001AA78(actor);
        PE_StoreU32(actor + 0x40u, PE_LoadU32(actor + 0x28u));
        PE_StoreU32(actor + 0x44u, PE_LoadU32(actor + 0x2Cu));
        PE_StoreU32(actor + 0x48u, PE_LoadU32(actor + 0x30u));
        if (actor == PE_LoadU32(GA_D_8009D254))
            PE_StoreU32(GA_D_800BCF88, PE_LoadU32(GA_D_800BCF88) | 0x80u);
        return 1;
    }
    if (code == 1u) {
        PE_StoreU32(actor + 0x40u, a);
        PE_StoreU32(actor + 0x44u, b);
        PE_StoreU32(actor + 0x48u, c);
        return 1;
    }
    if (code == 2u) {
        PE_StoreU32(actor + 0x68u, a);
        PE_StoreU32(actor + 0x6Cu, b);
        PE_StoreU32(actor + 0x70u, c);
        return 1;
    }
    if (code == 3u) {
        PE_StoreU32(actor + 0x78u, a);
        PE_StoreU32(actor + 0x7Cu, b);
        PE_StoreU32(actor + 0x80u, c);
        return 1;
    }
    if (code == 4u) {
        PE_StoreU32(actor + 0x88u, a);
        PE_StoreU32(actor + 0x8Cu, b);
        PE_StoreU32(actor + 0x90u, c);
        return 1;
    }
    if (code == 5u) {
        PE_StoreU16(actor + 0x38u, (uint16_t)a);
        PE_StoreU16(actor + 0x3Au, (uint16_t)b);
        PE_StoreU16(actor + 0x3Cu, (uint16_t)c);
        return 1;
    }
    PE_StoreU32(actor + 0x58u, a);
    PE_StoreU32(actor + 0x5Cu, b);
    PE_StoreU32(actor + 0x60u, c);
    return 1;
}

/*
 * PE-BTL28 — opcode 0x0C pose-read 12E7C.
 *
 * 142 words 0x80012E7C..0x800130B4, SHA-256 76d82640…3dc8.
 * D_800910A0[0x0C]. Zero jal. Always v0=1.
 * jtbl_80010080 codes 0-6 are the 12C20 groups, read
 * from D2F0 into *arg1/*arg2/*arg3. Code 5 is lh.
 * Live type-5 +0x14C: code 0 → local[0x0D/0x0F/0x0E].
 */
/* func_80012E7C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80012E7C_port.c (src/func_80012E7C.c); hand port retired (port3 switch-over C). */

/*
 * PE-BTL25 — opcode 0x5E pose-copy 14694.
 *
 * 147 words 0x80014694..0x800148E0, SHA-256 97c3ff9a…059e.
 * D_800910A0[0x5E]. Zero jal. Always v0=1.
 * jtbl_80010190 codes 0-6 are the 12C20 pose groups.
 *
 * *arg1==0 → D_8009D254 (no +0x98&0x10 test).
 * *arg1!=0 → walk D_8009D20C via +4; match type/idB and
 * !(+0x98&0x10). Miss stores *arg6=-1.
 * Live type-3 +0x00C: code 0, type 0, idB 0, locals 0..3.
 */
int func_80014694(pe_addr_t args)
{
    pe_addr_t found;
    uint32_t type;
    uint32_t code;
    uint32_t idb;

    type = PE_LoadU32(PE_LoadU32(args + 4u));
    if (type == 0u) {
        found = PE_LoadU32(GA_D_8009D254);
        if (found == 0u) {
            PE_StoreU32(PE_LoadU32(args + 24u), 0xFFFFFFFFu);
            return 1;
        }
    } else {
        idb = PE_LoadU32(PE_LoadU32(args + 8u));
        found = PE_LoadU32(GA_D_8009D20C);
        while (found != 0u) {
            if (PE_LoadU8(found + 0x0Cu) == (uint8_t)type &&
                PE_LoadU8(found + 0x0Du) == (uint8_t)idb &&
                (PE_LoadU32(found + 0x98u) & 0x10u) == 0u)
                break;
            found = PE_LoadU32(found + 4u);
        }
        if (found == 0u) {
            PE_StoreU32(PE_LoadU32(args + 24u), 0xFFFFFFFFu);
            return 1;
        }
    }

    PE_StoreU32(PE_LoadU32(args + 24u), 1u);
    code = PE_LoadU32(PE_LoadU32(args));
    if (code >= 7u)
        return 1;
    if (code == 0u) {
        PE_StoreU32(PE_LoadU32(args + 12u), PE_LoadU32(found + 0x28u));
        PE_StoreU32(PE_LoadU32(args + 16u), PE_LoadU32(found + 0x2Cu));
        PE_StoreU32(PE_LoadU32(args + 20u), PE_LoadU32(found + 0x30u));
        return 1;
    }
    if (code == 1u) {
        PE_StoreU32(PE_LoadU32(args + 12u), PE_LoadU32(found + 0x40u));
        PE_StoreU32(PE_LoadU32(args + 16u), PE_LoadU32(found + 0x44u));
        PE_StoreU32(PE_LoadU32(args + 20u), PE_LoadU32(found + 0x48u));
        return 1;
    }
    if (code == 2u) {
        PE_StoreU32(PE_LoadU32(args + 12u), PE_LoadU32(found + 0x68u));
        PE_StoreU32(PE_LoadU32(args + 16u), PE_LoadU32(found + 0x6Cu));
        PE_StoreU32(PE_LoadU32(args + 20u), PE_LoadU32(found + 0x70u));
        return 1;
    }
    if (code == 3u) {
        PE_StoreU32(PE_LoadU32(args + 12u), PE_LoadU32(found + 0x78u));
        PE_StoreU32(PE_LoadU32(args + 16u), PE_LoadU32(found + 0x7Cu));
        PE_StoreU32(PE_LoadU32(args + 20u), PE_LoadU32(found + 0x80u));
        return 1;
    }
    if (code == 4u) {
        PE_StoreU32(PE_LoadU32(args + 12u), PE_LoadU32(found + 0x88u));
        PE_StoreU32(PE_LoadU32(args + 16u), PE_LoadU32(found + 0x8Cu));
        PE_StoreU32(PE_LoadU32(args + 20u), PE_LoadU32(found + 0x90u));
        return 1;
    }
    if (code == 5u) {
        PE_StoreU32(PE_LoadU32(args + 12u),
                    (uint32_t)(int32_t)(int16_t)PE_LoadU16(found + 0x38u));
        PE_StoreU32(PE_LoadU32(args + 16u),
                    (uint32_t)(int32_t)(int16_t)PE_LoadU16(found + 0x3Au));
        PE_StoreU32(PE_LoadU32(args + 20u),
                    (uint32_t)(int32_t)(int16_t)PE_LoadU16(found + 0x3Cu));
        return 1;
    }
    PE_StoreU32(PE_LoadU32(args + 12u), PE_LoadU32(found + 0x58u));
    PE_StoreU32(PE_LoadU32(args + 16u), PE_LoadU32(found + 0x5Cu));
    PE_StoreU32(PE_LoadU32(args + 20u), PE_LoadU32(found + 0x60u));
    return 1;
}

/* func_8001CAB0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8001CAB0_port.c (src/func_8001CAB0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80014DA0(pe_addr_t args)
{
    unsigned int i;
    int hit;
    int px = (int)PE_LoadU32(PE_LoadU32(args + 32u));
    int py = (int)PE_LoadU32(PE_LoadU32(args + 36u));

    for (i = 0u; i < 4u; i++) {
        PE_StoreU32(GA_POLY + i * 8u,
                    PE_LoadU32(PE_LoadU32(args + i * 8u)));
        PE_StoreU32(GA_POLY + 4u + i * 8u,
                    PE_LoadU32(PE_LoadU32(args + 4u + i * 8u)));
    }
    hit = func_8001CAB0((int)PE_LoadU32(PE_LoadU32(args + 32u)),
                        (int)PE_LoadU32(PE_LoadU32(args + 36u)),
                        GA_POLY, 4u);
    PE_StoreU32(PE_LoadU32(args + 40u), (uint32_t)hit);
    if (PeOp77DbgOn()) {
        static int32_t prevx, prevy;
        int32_t vx[4], vy[4];
        for (i = 0u; i < 4u; i++) {
            vx[i] = (int32_t)(int16_t)PE_LoadU16(GA_POLY + i * 8u + 2u);
            vy[i] = (int32_t)(int16_t)PE_LoadU16(GA_POLY + i * 8u + 6u);
        }
        fprintf(stderr,
                "[OP77] pc=%08X px=%d.%04d py=%d.%04d hit=%d "
                "held=%08X move=(%d,%d) "
                "v=(%d,%d)(%d,%d)(%d,%d)(%d,%d)\n",
                (unsigned)PE_LoadU32(0x8009CE00u),
                px >> 16, (px & 0xFFFF), py >> 16, (py & 0xFFFF), hit,
                (unsigned)PE_LoadU32(0x8009D26Cu),
                (px >> 16) - prevx, (py >> 16) - prevy,
                vx[0], vy[0], vx[1], vy[1], vx[2], vy[2], vx[3], vy[3]);
        prevx = px >> 16;
        prevy = py >> 16;
    }
    return 1;
}
