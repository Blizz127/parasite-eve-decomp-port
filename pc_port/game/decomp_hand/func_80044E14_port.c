/*
 * func_80044E14 (menu item label, window+0x30 slot) and the $v0-returning
 * variant of func_80062A7C it tail-returns (port3 switch-over batch O).
 *
 * src/func_80044E14.c returns r = func_80062A7C(D_8009CFA0[idx]) (or 0 when
 * the table entry is empty), and the overlay CTX dispatcher func_800D4698
 * reads that $v0 through the +0x30 jalr at 0x800D46E0.  src/func_80062A7C.c
 * is `void` in C, but retail leaves a defined $v0 at its single `jr ra`
 * (0x80062CB0): the final pop's `lw v0,-8(v1)` (the popped word, which is
 * also the new D_8009D124) on the non-empty path, and the `sltu` result 0 on
 * underflow (func_800527C0 is an empty `jr ra` and does not touch $v0).
 * Disassembly notes: docs/evidence/pc-port-switchover/O-func_80044E14-v0.md.
 *
 * PE_V0_func_80062A7C is a line-by-line port of src/func_80062A7C.c (guest
 * addresses; the matched C is untouched) whose only addition is that return
 * value; func_80062A7C is that port with the value discarded.  It replaces
 * the hand shortcut func_8005F594(func_8005DC4C(id)).  The generator cannot
 * emit src/func_80062A7C.c yet (host-pointer `str` from func_8005DC4C; the
 * func_800527C0 arity is covered by E20f).  tests/test_v0_passthrough.h
 * checks the draw-state stack and $v0 against a model of the matched C over
 * the stack-pointer grid x {glyph string, empty string, str == 0}.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_800527C0(void);
unsigned char func_8005DC28(int index);

uint32_t PE_V0_func_80062A7C(uint32_t a0)
{
    pe_addr_t str, p, sp, sp2, sp3, sp4;
    int32_t w, ch, v, t, off, x, y, x0, y0;
    uint32_t v0;

    str = func_8005DC4C(a0);
    sp = PE_LoadU32(0x8009D12Cu);
    if (sp < 0x800A22B0u) {
        x0 = (int32_t)PE_LoadU32(0x8009D124u);
        y0 = (int32_t)PE_LoadU32(0x8009D128u);
        PE_StoreU32(0x8009D12Cu, sp + 8u);
        PE_StoreU32(sp, (uint32_t)x0);
        PE_StoreU32(sp + 4u, (uint32_t)y0);
    } else {
        func_800527C0();                     /* retail passes 2; body empty */
    }
    p = str;
    w = 0;
    if (PE_LoadU8(str) != 0xFFu) {
        do {
            ch = PE_LoadU8(p);
            p++;
            v = ch & 0xFF;
            if ((int32_t)PE_LoadU32(0x8009D0D8u) != 0) {
                v = v + ((int32_t)PE_LoadU32(0x8009D0D8u) << 8);
                PE_StoreU32(0x8009D0D8u, 0u);
            }
            if ((uint32_t)(ch & 0xFF) >= 0xFAu) {
                PE_StoreU32(0x8009D0D8u, (uint32_t)((ch & 0xFF) - 0xFA));
                v = -1;
            }
            ch = v;
            if (ch >= 0) {
                t = 0;
                if (ch < 0xA || ch == 0xF)   /* goto s1 / goto s2 ladder */
                    t = 1;
                PE_StoreU32(0x8009CDB0u, (uint32_t)(t + 1));
                if (ch >= 0x100)
                    ch -= 0x13;
                w += ((func_8005DC28(ch) >> 4) & 0xF) + (int32_t)PE_LoadU32(0x8009CDB0u);
            }
        } while (PE_LoadU8(p) != 0xFFu);
        p = str;
    }
    x = w + 4;
    off = (int32_t)PE_LoadU32(0x8009D138u) - x;
    y = (int32_t)PE_LoadU32(0x8009D128u);
    x = (int32_t)PE_LoadU32(0x8009D124u);
    off = off >> 1;
    PE_StoreU32(0x8009D128u, (uint32_t)y);
    x = x + off;
    PE_StoreU32(0x8009D124u, (uint32_t)x);
    if (p != 0u) {
        sp2 = PE_LoadU32(0x8009D12Cu);
        if (sp2 < 0x800A22B0u) {
            PE_StoreU32(sp2, (uint32_t)x);
            PE_StoreU32(sp2 + 4u, (uint32_t)y);
            PE_StoreU32(0x8009D12Cu, sp2 + 8u);
        } else {
            func_800527C0();                 /* retail passes 2 */
        }
        ch = PE_LoadU8(p);
        if ((ch & 0xFF) != 0xFF) {
            do {
                func_8005EED4((uint32_t)ch);
                p++;
                ch = PE_LoadU8(p);
            } while (ch != 0xFF);
        }
        sp3 = PE_LoadU32(0x8009D12Cu);
        if (0x800A2270u < sp3) {
            PE_StoreU32(0x8009D12Cu, sp3 - 8u);
            PE_StoreU32(0x8009D124u, PE_LoadU32(sp3 - 8u));
            PE_StoreU32(0x8009D128u, PE_LoadU32(sp3 - 4u));
        } else {
            func_800527C0();                 /* retail passes 3 */
        }
    }
    sp4 = PE_LoadU32(0x8009D12Cu);
    if (0x800A2270u < sp4) {
        v0 = PE_LoadU32(sp4 - 8u);           /* lw v0,-8(v1) */
        PE_StoreU32(0x8009D12Cu, sp4 - 8u);
        PE_StoreU32(0x8009D124u, v0);
        PE_StoreU32(0x8009D128u, PE_LoadU32(sp4 - 4u));
    } else {
        func_800527C0();                     /* retail passes 3; $v0 untouched */
        v0 = 0u;                             /* sltu result: sp4 <= D_800A2270 */
    }
    return v0;
}

/* src/func_80062A7C.c: the C function is void; retail $v0 is not needed here. */
void func_80062A7C(uint32_t id)
{
    (void)PE_V0_func_80062A7C(id);
}

/* src/func_80044E14.c, guest addresses; returns r (retail $v0). */
int func_80044E14(pe_addr_t window)
{
    int32_t idx = (int32_t)PE_LoadU32(window + 0x24u) - 0x29;
    int32_t r;

    func_8005E8A4(0, 0xA);
    func_8005EB58(0);
    func_8005F594((int)(0x800A1980u + ((uint32_t)idx << 6)));
    r = (int32_t)PE_LoadU32(0x8009CFA0u + (uint32_t)idx * 4u);
    if (r != 0) {
        func_8005E8A4(0, 0xE);
        r = (int32_t)PE_V0_func_80062A7C(PE_LoadU32(0x8009CFA0u + (uint32_t)idx * 4u));
    }
    return r;
}
