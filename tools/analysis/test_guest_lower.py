#!/usr/bin/env python3
"""Unit tests for guest_lower.py (E21 guest-memory lowering).

Run: python3 tools/analysis/test_guest_lower.py

Each runtime case lowers a small retail-typed leaf, compiles it on the host
against a mock guest RAM (little-endian byte array at 0x80000000, a mock
PE_GuestCall that records the retail jalr, a guest-frame arena), runs it, and
checks guest memory / the return value against what the MIPS O32 build of the
same C does: 4-byte pointers, natural alignment, unsigned plain `char`
(gcc-2.7.2-psx), modular 32-bit arithmetic, pointer scaling by guest sizes.
"""
from __future__ import annotations

import shutil
import subprocess
import sys
import tempfile
import os
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import guest_lower as gl  # noqa: E402

MOCK_H = r"""
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t pe_addr_t;
static uint8_t ram[0x200000];
static uint8_t *at(pe_addr_t a) { if ((a & 0x1FFFFFFFu) >= sizeof ram) { printf("BAD %08X\n", a); exit(3); } return ram + (a & 0x1FFFFFu); }
static uint8_t  PE_LoadU8(pe_addr_t a)  { return *at(a); }
static uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; memcpy(&v, at(a), 2); return v; }
static uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; memcpy(&v, at(a), 4); return v; }
static void PE_StoreU8(pe_addr_t a, uint8_t v)   { *at(a) = v; }
static void PE_StoreU16(pe_addr_t a, uint16_t v) { memcpy(at(a), &v, 2); }
static void PE_StoreU32(pe_addr_t a, uint32_t v) { memcpy(at(a), &v, 4); }
static void PE_Fill(pe_addr_t a, uint32_t n, uint8_t v) { memset(at(a), v, n); }
static void *PE_Translate(pe_addr_t a, size_t n) { (void)n; return at(a); }
static pe_addr_t PE_HostToGuest(const volatile void *p) { return p ? 0x80000000u + (pe_addr_t)((const uint8_t *)p - ram) : 0u; }
static pe_addr_t frame_sp = 0x801FEC00u;
static pe_addr_t PE_GuestFrame_Enter(uint32_t n) { frame_sp -= (n + 7u) & ~7u; return frame_sp; }
static void PE_GuestFrame_Leave(pe_addr_t b, uint32_t n) { if (b != frame_sp) exit(4); frame_sp += (n + 7u) & ~7u; }
static unsigned calls; static pe_addr_t call_fn; static uintptr_t call_a0, call_a1;
static int PE_GuestCall(const char *site, pe_addr_t fn, unsigned n, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{ (void)site; (void)n; (void)a2; (void)a3; calls++; call_fn = fn; call_a0 = a0; call_a1 = a1; return 7; }
static unsigned boundaries;
#define PE_D_COMP_BOUNDARY0(s, v) (boundaries++, 0)
#define PE_D_COMP_BOUNDARY1(s, v, ...) (boundaries++, 0)
#define PE_D_COMP_BOUNDARY2(s, v, ...) (boundaries++, 0)
"""


def lower(src: str, name: str = "func_80190000", known=(), sigs=None) -> dict:
    return gl.lower_leaf(name, src, set(known), sigs or {})


def tu(src: str, name: str = "func_80190000", known=(), sigs=None, extra: str = "") -> str:
    res = lower(src, name, known, sigs)
    head = "\n".join(res["helpers"])
    if res["sig"] is None:
        body = res["body"] + "\n}\n"
    else:
        body = f"{res['sig']}\n{{\n{res['body']}\n}}\n"
    return MOCK_H + extra + head + "\n" + body


class CRunner(unittest.TestCase):
    def run_c_tu(self, res: dict, main: str) -> str:
        head = "\n".join(res["helpers"]) + "\n" + "\n".join(res.get("local_defs", []))
        body = (res["body"] + "\n}\n") if res["sig"] is None else f"{res['sig']}\n{{\n{res['body']}\n}}\n"
        return self.run_c(MOCK_H + head + "\n" + body, main)

    def run_c(self, code: str, main: str) -> str:
        cc = shutil.which("cc") or shutil.which("gcc")
        with tempfile.TemporaryDirectory() as d:
            c = Path(d) / "t.c"
            c.write_text(code + "\nint main(void)\n{\n" + main + "\n    return 0;\n}\n")
            exe = Path(d) / "t"
            r = subprocess.run([cc, "-O2", "-std=gnu11", "-Werror=implicit-function-declaration",
                                "-Wno-unused-function", "-o", str(exe), str(c)],
                               capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stderr + code)
            r = subprocess.run([str(exe)], capture_output=True, text=True)
            self.assertEqual(r.returncode, 0, r.stdout + r.stderr)
            return r.stdout


@unittest.skipUnless(shutil.which("cc") or shutil.which("gcc"), "no host C compiler")
class Runtime(CRunner):
    def test_pointer_field_chain_and_code_pointer_call(self):
        # func_80190240's shape: o->ctx->fE through a guest pointer in RAM,
        # then the retail jalr through o->fn.
        src = """
        typedef struct Ctx { char pad[0xE]; unsigned char fE; } Ctx;
        typedef struct Obj { int pad0, pad4; Ctx *ctx; int (*fn)(struct Obj *); } Obj;
        int func_80190000(Obj *o) {
            if (o->ctx->fE < 2) return 1;
            o->fn(o);
            return 0;
        }"""
        out = self.run_c(tu(src), """
            PE_StoreU32(0x80100008u, 0x80100100u);          /* o->ctx */
            PE_StoreU32(0x8010000Cu, 0x80190370u);          /* o->fn  */
            PE_StoreU8(0x8010010Eu, 1);
            int r = func_80190000(0x80100000u);
            printf("%d %u\\n", r, calls);
            PE_StoreU8(0x8010010Eu, 2);
            r = func_80190000(0x80100000u);
            printf("%d %u %08X %08X\\n", r, calls, call_fn, (unsigned)call_a0);""")
        self.assertEqual(out.split("\n")[:2], ["1 0", "0 1 80190370 80100000"])

    def test_deref_of_code_pointer_slot_is_a_load(self):
        # room_m0145i func_80192678: `(*(void (**)())(a0 + 0xC))(a0)` calls the
        # code pointer stored at a0+0xC, never the slot address itself.
        src = """
        int func_80190000(unsigned char *a0) {
            (*(void (**)())(a0 + 0xC))(a0);
            return 0;
        }"""
        out = self.run_c(tu(src), """
            PE_StoreU32(0x8010000Cu, 0x80190370u);
            func_80190000(0x80100000u);
            printf("%u %08X %08X\\n", calls, call_fn, (unsigned)call_a0);""")
        self.assertEqual(out.split("\n")[0], "1 80190370 80100000")

    def test_o32_layout_pointer_members(self):
        src = """
        typedef struct S { char a; int *p; short s; char c[3]; struct S *next; } S;
        int func_80190000(S *x, int *q) {
            x->a = 1; x->p = q; x->s = -2; x->c[2] = 9; x->next = x + 1;
            return sizeof(S);
        }"""
        out = self.run_c(tu(src), """
            int n = func_80190000(0x80100000u, 0x80123450u);
            printf("%d %02X %08X %04X %02X %08X\\n", n, PE_LoadU8(0x80100000u),
                   PE_LoadU32(0x80100004u), PE_LoadU16(0x80100008u),
                   PE_LoadU8(0x8010000Cu), PE_LoadU32(0x80100010u));""")
        self.assertEqual(out.strip(), "20 01 80123450 FFFE 09 80100014")

    def test_plain_char_is_unsigned(self):
        src = """
        int func_80190000(char *p, signed char *q) { return (*p == 0xFF) * 100 + *q; }"""
        out = self.run_c(tu(src), """
            PE_StoreU8(0x80100000u, 0xFF);
            printf("%d\\n", func_80190000(0x80100000u, 0x80100000u));""")
        self.assertEqual(out.strip(), "99")

    def test_pointer_scaling_and_difference(self):
        src = """
        typedef struct V { short x, y, z, pad; } V;
        int func_80190000(V *v, short *s) {
            V *w = v + 3;
            short *t = s + 5;
            w->y = 4;
            w[-1].z = 5;
            return (w - v) * 100 + (t - s) + (int)(&w->z - s);
        }"""
        out = self.run_c(tu(src), """
            int r = func_80190000(0x80100000u, 0x80100000u);
            printf("%d %04X %04X\\n", r, PE_LoadU16(0x8010001Au), PE_LoadU16(0x80100014u));""")
        # w = +24, w[-1].z = +20, &w->z = +28 -> 14 shorts
        self.assertEqual(out.strip(), "319 0004 0005")

    def test_guest_frame_local_passed_by_address(self):
        src = """
        typedef struct M { short m[3][3]; short pad; int t[3]; } M;
        extern void func_80001000(M *out);
        int func_80190000(int k) {
            M m;
            func_80001000(&m);
            return m.t[2] + m.m[1][1] + k;
        }"""
        extra = """
        static pe_addr_t seen;
        void func_80001000(pe_addr_t out) { seen = out; PE_StoreU32(out + 28u, 40u); PE_StoreU16(out + 8u, 2u); }
        """
        sigs = {"func_80001000": {"ret": "void", "params": ["pe_addr_t out"]}}
        out = self.run_c(tu(src, known={"func_80001000"}, sigs=sigs, extra=extra), """
            int r = func_80190000(1);
            printf("%d %08X %08X\\n", r, seen, frame_sp);""")
        self.assertEqual(out.strip(), "43 801FEBE0 801FEC00")

    def test_host_pointer_parameter_translated(self):
        src = """
        typedef struct V { short x, y, z, pad; } V;
        extern int func_80001000(V *v);
        int func_80190000(V *v) { return func_80001000(v + 1); }"""
        extra = """
        typedef struct HV { short x, y, z, pad; } HV;
        int func_80001000(HV *v) { return v->y; }
        """
        sigs = {"func_80001000": {"ret": "int", "params": ["HV *v"]}}
        out = self.run_c(tu(src, known={"func_80001000"}, sigs=sigs, extra=extra), """
            PE_StoreU16(0x8010000Au, 77);
            printf("%d\\n", func_80190000(0x80100000u));""")
        self.assertEqual(out.strip(), "77")

    def test_compound_assign_incdec_and_wrap(self):
        src = """
        typedef struct O { int n; unsigned char k; short h; } O;
        int func_80190000(O *o) {
            o->n += 5;
            o->h -= 3;
            o->n = o->n * 0x10000;
            return o->k++ + (++o->k);
        }"""
        out = self.run_c(tu(src), """
            PE_StoreU32(0x80100000u, 0x7FFF);
            PE_StoreU8(0x80100004u, 0xFF);
            int r = func_80190000(0x80100000u);
            printf("%d %08X %02X %04X\\n", r, PE_LoadU32(0x80100000u),
                   PE_LoadU8(0x80100004u), PE_LoadU16(0x80100006u));""")
        # n = 0x8004 * 0x10000 wraps to 0x80040000; k: 255 + 1 -> 0, ++ -> 1
        self.assertEqual(out.strip(), "256 80040000 01 FFFD")

    def test_runtime_division_r3000_semantics_and_trap(self):
        # func_80192680's shape: a runtime divisor read from guest memory.
        # Zero is retail's break-7 divide check (a guest trap, never a host
        # SIGFPE); INT_MIN / -1 wraps like the R3000; constants stay plain.
        src = """
        typedef struct O { int a; int b; unsigned u; unsigned v; int q, r; unsigned uq, ur; int c; } O;
        void func_80190000(O *o) {
            o->q = o->a / o->b;
            o->r = o->a % o->b;
            o->uq = o->u / o->v;
            o->ur = o->u % o->v;
            o->c = o->a / 24;
        }"""
        code = tu(src, extra="static unsigned traps;\n"
                  "static void PE_Decomp_GuestTrapDivZero(const char *fn) { (void)fn; traps++; }\n")
        # helper definition + the one runtime `/` (o->a / 24 stays plain)
        self.assertEqual(code.count("pe_low_divs("), 2)
        out = self.run_c(code, """
            PE_StoreU32(0x80100000u, 100u); PE_StoreU32(0x80100004u, 0u);
            PE_StoreU32(0x80100008u, 7u);   PE_StoreU32(0x8010000Cu, 0u);
            func_80190000(0x80100000u);
            printf("%d %d %08X %u %d %u\\n", (int32_t)PE_LoadU32(0x80100010u),
                   (int32_t)PE_LoadU32(0x80100014u), PE_LoadU32(0x80100018u),
                   PE_LoadU32(0x8010001Cu), (int32_t)PE_LoadU32(0x80100020u), traps);
            PE_StoreU32(0x80100000u, 0x80000000u); PE_StoreU32(0x80100004u, 0xFFFFFFFFu);
            PE_StoreU32(0x80100008u, 7u);   PE_StoreU32(0x8010000Cu, 2u);
            func_80190000(0x80100000u);
            printf("%08X %d %u %u %u\\n", PE_LoadU32(0x80100010u),
                   (int32_t)PE_LoadU32(0x80100014u), PE_LoadU32(0x80100018u),
                   PE_LoadU32(0x8010001Cu), traps);""")
        lines = out.split()
        # 100/0 -> -1, 100%0 -> 100, 7u/0 -> FFFFFFFF, 7u%0 -> 7, 100/24 -> 4; 4 traps
        self.assertEqual(lines[:6], ["-1", "100", "FFFFFFFF", "7", "4", "4"])
        # INT_MIN / -1 -> INT_MIN, % -> 0; 7u/2 = 3, 7u%2 = 1; no new trap
        self.assertEqual(lines[6:], ["80000000", "0", "3", "1", "4"])
        # constant non-zero divisor stays a plain host division
        self.assertTrue(gl.safe_divisor("24") and gl.safe_divisor("(0x18)"))
        self.assertFalse(gl.safe_divisor("0") or gl.safe_divisor("-1") or gl.safe_divisor("v_n"))

    def test_packed_struct_and_init_list(self):
        src = """
        typedef struct P { char a; int b; } __attribute__((packed)) P;
        extern void func_80001000(int *v);
        int func_80190000(P *p) {
            int v[4] = { 3, 4 };
            p->b = 0x11223344;
            func_80001000(v);
            return sizeof(P) * 100 + v[1] + v[3];
        }"""
        extra = "void func_80001000(pe_addr_t v) { (void)v; }\n"
        sigs = {"func_80001000": {"ret": "void", "params": ["pe_addr_t v"]}}
        out = self.run_c(tu(src, known={"func_80001000"}, sigs=sigs, extra=extra), """
            PE_StoreU32(0x801FEBF0u + 12u, 0xDEAD);  /* stale frame byte must be zeroed */
            int r = func_80190000(0x80100000u);
            printf("%d %08X\\n", r, PE_LoadU32(0x80100001u));""")
        self.assertEqual(out.strip(), "504 11223344")

    def test_unknown_callee_is_a_loud_boundary(self):
        src = "int func_80190000(int a) { return func_80012340(a) + 1; }"
        res = lower(src)
        self.assertIn('PE_D_COMP_BOUNDARY1("func_80012340", 0x80012340u, a_a)', res["body"])


GTE_MOCK = r"""
static uint32_t gd[32], gc[32]; static unsigned gcmds;
static uint32_t PE_GTE_ReadData(uint32_t r) { return gd[r]; }
static void PE_GTE_WriteData(uint32_t r, uint32_t v)
{ gd[r] = (r == 1u || (r >= 9u && r <= 11u)) ? (uint32_t)(int32_t)(int16_t)v : v; }
static uint32_t PE_GTE_ReadCtrl(uint32_t r) { return gc[r]; }
static void PE_GTE_WriteCtrl(uint32_t r, uint32_t v) { gc[r] = v; }
static void PE_GTE_Command(uint32_t cmd)
{   /* independent reference: MVMVA sf=1 mx=RT v=V0 cv=none, lm=0 */
    int16_t m[9], v[3]; int i;
    gcmds++;
    if (cmd != 0x486012u) { printf("CMD %07X\n", cmd); exit(5); }
    for (i = 0; i < 4; i++) { m[2 * i] = (int16_t)gc[i]; m[2 * i + 1] = (int16_t)(gc[i] >> 16); }
    m[8] = (int16_t)gc[4];
    v[0] = (int16_t)gd[0]; v[1] = (int16_t)(gd[0] >> 16); v[2] = (int16_t)gd[1];
    for (i = 0; i < 3; i++) {
        int64_t mac = ((int64_t)m[3 * i] * v[0] + (int64_t)m[3 * i + 1] * v[1] + (int64_t)m[3 * i + 2] * v[2]) >> 12;
        gd[25 + i] = (uint32_t)(int32_t)mac;
        gd[9 + i] = (uint32_t)(int32_t)(mac < -32768 ? -32768 : mac > 32767 ? 32767 : mac);
    }
}
"""

GTE_SRC = r"""
typedef struct MATRIX { short m[3][3]; int t[3]; } MATRIX;
typedef struct SVECTOR { short vx, vy, vz, pad; } SVECTOR;
typedef struct VECTOR { int vx, vy, vz, pad; } VECTOR;
extern int g_count asm("D_80180010");
int func_80190000(MATRIX *mat, SVECTOR *v, VECTOR *out, short *ir)
{
    int a, b, c;
    __asm__ volatile("lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t"
                     "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\tctc2 $12,$2\n\t"
                     "ctc2 $13,$3\n\tctc2 $14,$4" : : "r"(mat) : "$12", "$13", "$14");
    __asm__ volatile("lhu $13,2(%0)\n\tlhu $12,0(%0)\n\tsll $13,$13,16\n\tor $12,$12,$13\n\t"
                     "mtc2 $12,$0\n\tlwc2 $1,4(%0)" : : "r"(v) : "$12", "$13");
    __asm__ volatile("nop\n\tnop\n\t.word 0x4A486012");
    __asm__ volatile("swc2 $25,0(%0)\n\tswc2 $26,4(%0)\n\tswc2 $27,8(%0)" : : "r"(out) : "memory");
    __asm__ volatile("mfc2 %0,$9\n\tmfc2 %1,$10\n\tmfc2 %2,$11" : "=r"(a), "=r"(b), "=r"(c) : "2"(c));
    ir[0] = a; ir[1] = b; ir[2] = c;
    g_count++;
    return a + b + c;
}
"""


@unittest.skipUnless(shutil.which("cc") or shutil.which("gcc"), "no host C compiler")
class GteAsm(CRunner):
    """Inline PSY-Q GTE macros lowered to register-level native GTE calls."""

    def test_rotmatrix_ldv0_rtv0_stlvnl_mfc2(self):
        code = tu(GTE_SRC, extra=GTE_MOCK)
        self.assertIn("PE_GTE_Command(0x0486012u)", code)
        out = self.run_c(code, r"""
            static const short R[9] = { 0x1000, 0, 0, 0, 0, -0x1000, 0, 0x1000, 0 };
            int i;
            for (i = 0; i < 9; i++) PE_StoreU16(0x80100000u + 2u * i, (uint16_t)R[i]);
            PE_StoreU16(0x80100100u, 100); PE_StoreU16(0x80100102u, (uint16_t)-200);
            PE_StoreU16(0x80100104u, 40000u & 0xFFFF);
            int r = func_80190000(0x80100000u, 0x80100100u, 0x80100200u, 0x80100300u);
            printf("%d %d %d %d | %d %d %d | %u %u\n", r,
                   (int)PE_LoadU32(0x80100200u), (int)PE_LoadU32(0x80100204u), (int)PE_LoadU32(0x80100208u),
                   (short)PE_LoadU16(0x80100300u), (short)PE_LoadU16(0x80100302u), (short)PE_LoadU16(0x80100304u),
                   gcmds, PE_LoadU32(0x80180010u));""")
        # x = 100; y = -vz = -(40000 as s16 = -25536) = 25536; z = vy = -200
        self.assertEqual(out.strip(), "25436 100 25536 -200 | 100 25536 -200 | 1 1")

    CARRY_SRC = r"""
extern int g_slot asm("D_80180010");
extern void func_80190100(void);
int func_80190000(int mode)
{
    /* vendor macro style: $8 loaded in one asm, read by the next, never
       declared to cc1 (m0437i func_80192500) */
    __asm__ volatile("lw $8,0(%0)\n\tnop\n\tlw $12,0($8)\n\tctc2 $12,$0"
                     : : "r"(&g_slot) : "$12");
    if (mode)
        __asm__ volatile("lw $12,4($8)\n\tctc2 $12,$1" : : : "$12");
    return mode;
}
"""

    def test_undeclared_gpr_carried_between_asm(self):
        code = tu(self.CARRY_SRC, extra=GTE_MOCK)
        self.assertIn("uint32_t pe_hr8 = 0u;", code)
        out = self.run_c(code, r"""
            PE_StoreU32(0x80180010u, 0x80100000u);
            PE_StoreU32(0x80100000u, 0x11112222u); PE_StoreU32(0x80100004u, 0x33334444u);
            func_80190000(1);
            printf("%08x %08x\n", gc[0], gc[1]);""")
        self.assertEqual(out.strip(), "11112222 33334444")

    def test_carried_gpr_needs_earlier_asm(self):
        src = self.CARRY_SRC.replace('__asm__ volatile("lw $8,0(%0)\\n\\tnop\\n\\t', '__asm__ volatile("')
        self.assertNotEqual(src, self.CARRY_SRC)
        with self.assertRaises(gl.LowerError) as cm:
            lower(src)
        self.assertIn("carried $8 not set by an earlier asm", str(cm.exception))

    def test_carried_gpr_killed_by_call(self):
        src = self.CARRY_SRC.replace("    if (mode)\n", "    func_80190100();\n    if (mode)\n")
        self.assertNotEqual(src, self.CARRY_SRC)
        sigs = {"func_80190100": {"ret": "void", "params": ["void"]}}
        with self.assertRaises(gl.LowerError) as cm:
            lower(src, known={"func_80190100"}, sigs=sigs)
        self.assertIn("carried $8 not set by an earlier asm", str(cm.exception))

    def test_clobbered_gpr_never_carried(self):
        src = self.CARRY_SRC.replace(': : : "$12");', ': : : "$8", "$12");')
        self.assertNotEqual(src, self.CARRY_SRC)
        with self.assertRaises(gl.LowerError) as cm:
            lower(src)
        self.assertIn("$8 read before written", str(cm.exception))

    def test_rtpt_and_sz3_read_lowered(self):
        src = r"""
        int func_80190000(int *otz)
        {
            __asm__ volatile("nop\n\tnop\n\t.word 0x4A280030");
            __asm__ volatile("mfc2 $12,$19\n\tnop\n\tsra $12,$12,2\n\tsw $12,0(%0)"
                             : : "r"(otz) : "$12", "memory");
            return 0;
        }"""
        res = lower(src)
        self.assertIn("PE_GTE_Command(0x0280030u)", res["body"])
        self.assertIn("PE_GTE_ReadData(19", res["body"])

    def test_asm_quoted_in_comment_ignored(self):
        src = ('/* lever: asm("" : "=r"(r) : "0"(x)) keeps x live */\n'
               'int func_80190000(int a) { return a + 1; /* asm( // */ }')
        self.assertIn("return", lower(src)["body"])

    def test_register_pin_list_stripped(self):
        import gen_decomp_ports as gdp
        src = ('int func_80190000(int *t) {\n'
               '    register int m0 asm("$12"), m1 asm("$13"), m2 asm("$14");\n'
               '    m0 = t[0]; m1 = t[1]; m2 = t[2];\n'
               '    return m0 + m1 + m2;\n}')
        out = gdp.strip_era_asm(src)
        self.assertNotIn("asm", out)
        self.assertIn("register int m0, m1, m2;", out)
        self.assertIn("return", lower(out)["body"])

    def test_plain_function_declaration_label(self):
        src = r'''
        int Engine_Random(void) __asm__("func_80071A54");
        int func_80190000(int a) { return Engine_Random() + a; }'''
        res = lower(src)
        self.assertIn('"func_80071A54"', res["body"])

    def test_set_alias_renames_function(self):
        src = r"""
        asm(".set RoomLib_Helper, func_80190100");
        extern int RoomLib_Helper(int);
        int func_80190000(int a) { return RoomLib_Helper(a) + 1; }"""
        res = lower(src)
        self.assertIn('PE_D_COMP_BOUNDARY1("func_80190100"', res["body"])


class Refusals(unittest.TestCase):
    def assertRefused(self, src, why, **kw):
        with self.assertRaises(gl.LowerError) as cm:
            lower(src, **kw)
        self.assertIn(why, str(cm.exception))

    def test_float(self):
        self.assertRefused("int func_80190000(float *f) { return *f > 1; }", "float")

    def test_bitfield(self):
        # Plain struct bitfields lower (class Bitfields); union or packed
        # bitfields have no gcc layout here yet and stay refused.
        self.assertRefused("typedef union B { int a : 3; int b; } B;\n"
                           "int func_80190000(B *b) { return b->a; }", "bitfield")
        self.assertRefused("typedef struct { int a : 3; char c; } __attribute__((packed)) B;\n"
                           "int func_80190000(B *b) { return b->a; }", "bitfield")

    def test_static_local(self):
        self.assertRefused("int func_80190000(void) { static int n; return n++; }", "static local")

    def test_real_asm(self):
        self.assertRefused('int func_80190000(int a) { __asm__("syscall"); return a; }', "asm")

    def test_unsupported_cop2_command(self):
        # A reserved operation stays refused even with the complete native GTE.
        self.assertRefused('int func_80190000(int a) { __asm__ volatile(".word 0x4A000002"); '
                           'return a; }', "cop2 command")

    def test_ncds_lowered_to_native_command(self):
        res = lower('int func_80190000(int a) { __asm__ volatile(".word 0x4AE80413"); '
                    'return a; }')
        self.assertIn("PE_GTE_Command(0x0E80413u)", res["body"])

    def test_rtps_lowered(self):
        res = lower('int func_80190000(int a) { __asm__ volatile("nop\\n\\tnop\\n\\t.word 0x4A180001"); '
                    'return a; }')
        self.assertIn("PE_GTE_Command(0x0180001u)", res["body"])

    VOID_SIG = {"func_800CEE20": {"ret": "void", "params": ["int32_t a"]}}

    def test_void_callee_value_used_refused(self):
        src = ("int func_800CEE20();\n"
               "int func_80190000(int a) { return func_800CEE20(a); }")
        with self.assertRaises(gl.LowerError) as cm:
            lower(src, known={"func_800CEE20"}, sigs=self.VOID_SIG)
        self.assertIn("value of void callee", str(cm.exception))

    def test_void_callee_as_statement_lowered(self):
        # Vendor TUs declare `int func_800CEE20();` and only call it as a
        # statement; that discards the value, so it lowers.
        src = ("int func_800CEE20();\n"
               "int func_80190000(int a) { func_800CEE20(a + 1); return a; }")
        res = lower(src, known={"func_800CEE20"}, sigs=self.VOID_SIG)
        self.assertIn("func_800CEE20(", res["body"])

    def test_rtpt_lm1_lowered(self):
        res = lower('int func_80190000(int a) { __asm__ volatile(".word 0x4A280430"); '
                    'return a; }')
        self.assertIn("PE_GTE_Command(0x0280430u)", res["body"])

    def test_color_fifo_flag_and_perspective_registers_lowered(self):
        src = r"""
        int func_80190000(int value) {
            int flag, color, point;
            __asm__ volatile("mtc2 %3,$20\n\tctc2 %3,$26\n\t"
                             "cfc2 %0,$31\n\tmfc2 %1,$29\n\tmfc2 %2,$15"
                             : "=r"(flag), "=r"(color), "=r"(point) : "r"(value));
            return flag ^ color ^ point;
        }"""
        body = lower(src)["body"]
        for call in ("PE_GTE_WriteData(20u,", "PE_GTE_WriteCtrl(26u,",
                     "PE_GTE_ReadCtrl(31u)", "PE_GTE_ReadData(29u)", "PE_GTE_ReadData(15u)"):
            self.assertIn(call, body)

    def test_memory_constraint(self):
        # A non-empty template with an "m" operand is still refused.
        self.assertRefused('int func_80190000(int *p) { __asm__ volatile("mtc2 $0,$9" : : "m"(*p)); '
                           'return 0; }', "input constraint m")

    def test_empty_template_memory_fence_dropped(self):
        res = lower('int func_80190000(short *p) { p[3] = 7; '
                    '__asm__ volatile("" : : "m"(*(short *)(p + 3))); return p[3]; }')
        self.assertNotIn("__pe_asm", res["body"])

    def test_empty_template_with_side_effect_refused(self):
        self.assertRefused('int func_80190000(int *p) { __asm__ volatile("" : : "m"(*p++)); '
                           'return 0; }', "input constraint m")

    def test_output_read_before_written(self):
        self.assertRefused('int func_80190000(void) { int x; __asm__ volatile('
                           '"mtc2 %0,$9" : "=r"(x)); return x; }', "read before written")

    def test_indirect_call_arity(self):
        self.assertRefused("int func_80190000(void (*f)(int,int,int,int,int,int,int,int,int)) "
                           "{ f(1,2,3,4,5,6,7,8,9); return 0; }", "more than 8")

    def test_indirect_call_stack_args(self):
        # 5..8 arguments: O32 stack arguments ride PE_GuestCallN.
        res = lower("int func_80190000(int (*f)(int,int,int,int,int,int), int x) "
                    "{ return f(1,2,3,4,x,6); }")
        self.assertIn("PE_GuestCallN(", res["body"])
        self.assertIn("6u, (const uintptr_t[]){", res["body"])
        self.assertIn("(uintptr_t)(uint32_t)(a_x)", res["body"])
        self.assertNotIn("PE_GuestCall(", res["body"])


    # E21c: extra arguments the callee never reads (m0348i func_8018F098 ->
    # func_800C2AF0: 6 passed, 4 taken by both the retail and host callee).
    C2AF0 = {"func_800C2AF0": {"ret": "int", "params": ["pe_addr_t slot", "int32_t unused",
                                                        "int32_t index", "uint32_t value"],
                               "retail_nparams": 4}}
    WRAP = ("extern void func_800C2AF0();\n"
            "int func_8018F098(int a0, int a1, int a2, int a3, int a4, int a5)\n"
            "{ func_800C2AF0(a0, a1, a2, a3, a4, a5); return 0; }")

    def test_unread_stack_args_dropped(self):
        res = lower(self.WRAP, "func_8018F098", known={"func_800C2AF0"}, sigs=self.C2AF0)
        self.assertIn("func_800C2AF0(a_a0, a_a1, a_a2, a_a3)", res["body"])
        self.assertNotIn("a_a4", res["body"].split("func_800C2AF0(")[1].split(")")[0])

    def test_unread_args_need_unprototyped_decl(self):
        src = self.WRAP.replace("extern void func_800C2AF0();",
                                "extern void func_800C2AF0(int, int, int, int, int, int);")
        self.assertRefused(src, "call arity", name="func_8018F098", known={"func_800C2AF0"},
                           sigs=self.C2AF0)

    def test_unread_args_need_retail_count(self):
        sigs = {"func_800C2AF0": {**self.C2AF0["func_800C2AF0"], "retail_nparams": 6}}
        self.assertRefused(self.WRAP, "call arity", name="func_8018F098",
                           known={"func_800C2AF0"}, sigs=sigs)
        sigs = {"func_800C2AF0": {k: v for k, v in self.C2AF0["func_800C2AF0"].items()
                                  if k != "retail_nparams"}}
        self.assertRefused(self.WRAP, "call arity", name="func_8018F098",
                           known={"func_800C2AF0"}, sigs=sigs)

    def test_unread_args_must_be_pure(self):
        src = self.WRAP.replace("a4, a5)", "a4++, a5)")
        self.assertRefused(src, "call arity", name="func_8018F098", known={"func_800C2AF0"},
                           sigs=self.C2AF0)

    def test_too_few_args_refused(self):
        src = self.WRAP.replace("a0, a1, a2, a3, a4, a5)", "a0, a1, a2)")
        self.assertRefused(src, "call arity", name="func_8018F098", known={"func_800C2AF0"},
                           sigs=self.C2AF0)


    # E21d: a bare call forwarding the leaf's live $a0 (room_m0107i
    # func_8018F1D4: addiu sp / sw ra / jal 800C22F8 -- a0 untouched).
    C22F8 = {"func_800C22F8": {"ret": "pe_addr_t", "params": ["pe_addr_t slot"],
                               "retail_nparams": 1}}
    SLOT = ("extern int *func_800C22F8();\nextern char D_801935F8[];\n"
            "int func_8018F1D4(void)\n{\n    *func_800C22F8() = (int)D_801935F8;\n    return 0;\n}\n")

    def test_live_a0_forwarded(self):
        res = lower(self.SLOT, "func_8018F1D4", known={"func_800C22F8"}, sigs=self.C22F8)
        self.assertIn("(uint32_t a_pe_live_a0)", res["sig"])
        self.assertIn("func_800C22F8(a_pe_live_a0)", res["body"])

    def test_live_a0_needs_first_statement(self):
        src = self.SLOT.replace("{\n    *func_800C22F8()", "{\n    D_801935F8[0] = 1;\n    *func_800C22F8()")
        self.assertRefused(src, "call arity", name="func_8018F1D4", known={"func_800C22F8"},
                           sigs=self.C22F8)

    def test_live_a0_needs_paramless_leaf(self):
        src = self.SLOT.replace("func_8018F1D4(void)", "func_8018F1D4(int a)")
        self.assertRefused(src, "call arity", name="func_8018F1D4", known={"func_800C22F8"},
                           sigs=self.C22F8)

    def test_live_a0_needs_unprototyped_decl_and_retail_count(self):
        src = self.SLOT.replace("extern int *func_800C22F8();", "extern int *func_800C22F8(void);")
        self.assertRefused(src, "", name="func_8018F1D4", known={"func_800C22F8"}, sigs=self.C22F8)
        sigs = {"func_800C22F8": {**self.C22F8["func_800C22F8"], "retail_nparams": 2}}
        self.assertRefused(self.SLOT, "call arity", name="func_8018F1D4",
                           known={"func_800C22F8"}, sigs=sigs)

    def test_live_a0_needs_pure_rhs(self):
        src = self.SLOT.replace("= (int)D_801935F8;", "= func_800C22F8();")
        self.assertRefused(src, "call arity", name="func_8018F1D4", known={"func_800C22F8"},
                           sigs=self.C22F8)

    def test_host_replacement_called_by_host_name(self):
        # A Psy-Q entry reimplemented on the host (pe_port_compat.h
        # host-replacement) is called by its host name; func_X is never named.
        sigs = {"func_80079754": {"ret": "pe_addr_t", "params": ["pe_addr_t r", "pe_addr_t m"],
                                  "host": "PE_RotMatrixYXZ_Guest"}}
        res = lower("extern void *func_80079754();\n"
                    "int func_80190000(short *r, short *m) { func_80079754(r, m); return 0; }",
                    known={"func_80079754"}, sigs=sigs)
        self.assertIn("PE_RotMatrixYXZ_Guest(a_r, a_m)", res["body"])
        self.assertNotIn("func_80079754(", res["body"])



class PointerGlobalStore(unittest.TestCase):
    """E21e (gen_overlay_ports): a store to a pointer-typed global slot is a
    32-bit guest store of the address (room_m0123i func_8019537C)."""
    SRC = """extern int D_80195594;
extern int *D_801956B8;
int **func_8019537C(int a0, unsigned int a1)
{
    D_801956B8 = &D_80195594;
    return &D_801956B8;
}
"""

    def test_slot_store_is_guest_store(self):
        res = lower(self.SRC, "func_8019537C")
        self.assertIn("PE_StoreU32(0x801956B8u, (uint32_t)(0x80195594u))", res["body"])
        self.assertIn("return (uint32_t)(0x801956B8u);", res["body"])

    def test_reason_routed_to_lowering(self):
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        import gen_overlay_ports as gop
        self.assertTrue("pointer-global store D_801956B8".startswith(gop.LOWER_REASONS))


    def test_prototyped_guest_call_converts_narrow_arguments(self):
        # room_m0429i func_80191928: D_800DFFB8(short, short, short) is reached
        # by jalr; retail passes (short)x in each register.
        res = lower('extern int D_800DFFB8(short cur, short target, unsigned char rate);\n'
                    'int func_80190000(int x) { return D_800DFFB8(x, x + 1, x); }')
        body = res["body"]
        self.assertIn("(int32_t)(int16_t)(", body)
        self.assertIn("(int32_t)(uint8_t)(", body)
        self.assertIn("PE_GuestCall(", body)

    def test_unprototyped_guest_call_keeps_default_promotions(self):
        res = lower('extern int D_800DFFB8();\n'
                    'int func_80190000(short x) { return D_800DFFB8(x); }')
        self.assertNotIn("(int32_t)(int16_t)((", res["body"].split("PE_GuestCall(")[1].split(";")[0])

    def test_call_arguments_with_calls_are_bound_in_source_order(self):
        # room_m0418i func_80190914: retail evaluates GetTPage then GetClut as
        # two stack arguments of one call; the host's argument order is free.
        res = lower('extern int D_80190100(int, int, int);\n'
                    'extern int D_80190200(int); extern int D_80190300(int);\n'
                    'int func_80190000(int x) { return D_80190100(D_80190200(1), x, D_80190300(2)); }')
        body = res["body"]
        a, b = body.index("__auto_type pe_t1"), body.index("__auto_type pe_t2")
        self.assertLess(a, b)
        self.assertIn("0x80190200", body[a:b])
        self.assertIn("0x80190300", body[b:body.index(";", b)])

    def test_single_call_argument_is_not_rebound(self):
        res = lower('extern int D_80190100(int, int);\nextern int D_80190200(int);\n'
                    'int func_80190000(int x) { return D_80190100(D_80190200(1), x); }')
        self.assertNotIn("__auto_type", res["body"])

    def test_two_non_static_definitions_refused(self):
        with self.assertRaises(gl.LowerError):
            lower("int helper(int a) { return a; }\nint func_80190000(int x) { return helper(x); }")

    def test_dead_trailing_parameter_padded(self):
        res = lower("extern void func_8010BD4C();\nvoid func_80190000(int x) { func_8010BD4C(x); }",
                    known={"func_8010BD4C"},
                    sigs={"func_8010BD4C": {"ret": "void", "params": ["pe_addr_t dst", "uint32_t count"]}})
        self.assertIn("func_8010BD4C(", res["body"])
        self.assertRegex(res["body"], r"func_8010BD4C\([^;]*, 0\)")

    def test_kseg2_mask_is_not_a_bios_literal(self):
        lower("int func_80190000(unsigned int w) { return (w & 0xFFC00000) != 0; }")
        for lit in ("0xBFC00000", "0x9FC00180", "0x1F801810", "0xBF801070"):
            with self.assertRaises(gl.LowerError, msg=lit):
                lower(f"int func_80190000(void) {{ return *(volatile int *){lit}; }}")

    def test_recursive_frame_leaf_declares_its_entry_first(self):
        src = ('int func_80190000(int n) { int buf[2]; buf[0] = n; '
               'if (n > 0) return func_80190000(n - 1) + buf[0]; return 0; }')
        res = lower(src)
        self.assertIsNone(res["sig"])
        body = res["body"]
        self.assertTrue(body.startswith("int32_t func_80190000(int32_t a_n);"), body[:80])


@unittest.skipUnless(shutil.which("cc") or shutil.which("gcc"), "no host C compiler")
class Bitfields(CRunner):
    def test_p_tag_addprim(self):
        # room_m0017i func_8018F580's addPrim through P_TAG {addr:24, len:8}.
        src = """
        typedef struct { unsigned addr : 24; unsigned len : 8; } P_TAG;
        #define getaddr(p) (((P_TAG *)(p))->addr)
        #define setaddr(p, _addr) (((P_TAG *)(p))->addr = (_addr))
        #define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, (unsigned int)(p))
        int func_80190000(unsigned int *ot, unsigned int *p) {
            addPrim(ot, p);
            return ((P_TAG *)p)->len + sizeof(P_TAG);
        }"""
        out = self.run_c(tu(src), """
            PE_StoreU32(0x80100000u, 0x05123456u);
            PE_StoreU32(0x80100100u, 0x09ABCDEFu);
            int r = func_80190000(0x80100000u, 0x80100100u);
            printf("%d %08X %08X\\n", r, PE_LoadU32(0x80100000u), PE_LoadU32(0x80100100u));""")
        self.assertEqual(out.strip(), "13 05100100 09123456")

    def test_layout_signed_and_unit_crossing_match_host_gcc(self):
        # The guest (MIPS o32 LE) and the host (SysV x86-64) lay these
        # int/unsigned/short bitfields out identically, so the host struct is
        # the oracle: a field that would cross its 32-bit unit moves on, a
        # signed field sign-extends, untouched container bits survive.
        decl = ("typedef struct { int a : 5; int b : 11; unsigned c : 20; "
                "unsigned d : 20; short e; } S;")
        src = decl + """
        int func_80190000(S *s, int k) {
            s->a = -3; s->b = k; s->c = 0xABCDE; s->d = 0x12345; s->e = -7;
            return s->b * 100 + s->a + sizeof(S);
        }"""
        out = self.run_c(tu(src, extra=decl.replace("} S;", "} HS;") + "\n"), """
            HS h; int i;
            memset(&h, 0xFF, sizeof h);
            h.a = -3; h.b = -1000; h.c = 0xABCDE; h.d = 0x12345; h.e = -7;
            PE_Fill(0x80100000u, 16u, 0xFF);
            int r = func_80190000(0x80100000u, -1000);
            int same = sizeof h == 16;
            for (i = 0; i < 16; i++) same &= ((uint8_t *)&h)[i] == PE_LoadU8(0x80100000u + i);
            printf("%d %d\\n", r, same);""")
        self.assertEqual(out.strip(), f"{-1000 * 100 - 3 + 16} 1")

    def test_bitfield_compound_assignment(self):
        src = """
        typedef struct { unsigned lo : 4; unsigned mid : 8; unsigned hi : 20; } T;
        void func_80190000(T *t) { t->mid += 0x81; t->lo++; t->hi |= 1; }"""
        out = self.run_c(tu(src), """
            PE_StoreU32(0x80100000u, 0xFFFF0F8Fu);   /* hi 0xFFFF0, mid 0xF8, lo 0xF */
            func_80190000(0x80100000u);
            printf("%08X\\n", PE_LoadU32(0x80100000u));""")
        # mid 0xF8 + 0x81 = 0x179 -> 0x79; lo 0xF + 1 -> 0; hi 0xFFFF0 | 1
        self.assertEqual(out.strip(), "FFFF1790")

    def test_bitfield_value_uses_refused(self):
        for body in ("return t->lo = 3;", "return t->lo++;", "return (int)&t->lo;"):
            src = ("typedef struct { unsigned lo : 4; } T;\n"
                   f"int func_80190000(T *t) {{ {body} }}")
            with self.assertRaises(gl.LowerError, msg=body):
                lower(src)


@unittest.skipUnless(shutil.which("cc") or shutil.which("gcc"), "no host C compiler")
class Builtins(CRunner):
    def test_builtin_memcpy_into_frame_buffer(self):
        # room_m0418i func_80192C84: a 4-byte table copied into a stack
        # buffer whose address goes to guest code.
        src = """
        extern unsigned char D_8018F04C[];
        extern void D_800D1AE0(unsigned char *a0, int a1);
        void func_80190000(short *a2) {
            unsigned char buf[4];
            __builtin_memcpy(buf, D_8018F04C, 4);
            D_800D1AE0(buf, a2[1]);
        }"""
        out = self.run_c(tu(src), """
            PE_StoreU32(0x8018F04Cu, 0x44332211u);
            PE_StoreU16(0x80100002u, 0xFFF9u);
            func_80190000(0x80100000u);
            printf("%u %08X %08X %d\\n", calls, call_fn, PE_LoadU32((pe_addr_t)call_a0),
                   (int)(int32_t)call_a1);""")
        self.assertEqual(out.strip(), "1 800D1AE0 44332211 -7")

    def test_func_named_data_is_its_code_address(self):
        # room_m0012i func_8018F20C passes its callback as
        # `extern unsigned char func_8018F004[]`.
        src = """
        extern unsigned char func_8018F004[];
        extern int D_800CE560(unsigned char *cb, int n);
        int func_80190000(void) { return D_800CE560(func_8018F004, 3); }"""
        out = self.run_c(tu(src), """
            int r = func_80190000();
            printf("%d %08X %08X %u\\n", r, call_fn, (unsigned)call_a0, (unsigned)call_a1);""")
        self.assertEqual(out.strip(), "7 800CE560 8018F004 3")


@unittest.skipUnless(shutil.which("cc") or shutil.which("gcc"), "no host C compiler")
class IntegerAsm(CRunner):
    # room_m0273i func_80196348's shapes: slt into a pinned local, and a
    # mult whose HI an mfhi in a later statement reads (magic division).
    SRC = """
    int func_80190000(int goal, int cur, int af) {
        register int cmp asm("$2");
        register int hi asm("$7");
        int num = af << 13, sign, q;
        asm("slt %0, %1, %2" : "=r"(cmp) : "r"(goal), "r"(cur));
        asm volatile("mult %0, %1" : : "r"(num), "r"(0x22B63CBF));
        sign = num >> 31;
        asm volatile("mfhi %0" : "=r"(hi));
        q = (hi >> 4) - sign;
        return cmp * 100000 + q;
    }"""

    def test_slt_and_mult_mfhi_across_statements(self):
        out = self.run_c(self.tu_pinned(self.SRC), """
            printf("%d %d %d\\n", func_80190000(5, 3, 100), func_80190000(3, 5, -77),
                   func_80190000(7, 7, 118));""")
        def ref(goal, cur, af):
            num = (af << 13) & 0xFFFFFFFF
            snum = num - (1 << 32) if num & 0x80000000 else num
            hi = ((snum * 0x22B63CBF) >> 32)
            q = (hi >> 4) - (-1 if snum < 0 else 0)
            return (1 if goal < cur else 0) * 100000 + q
        self.assertEqual(out.split(), [str(ref(5, 3, 100)), str(ref(3, 5, -77)), str(ref(7, 7, 118))])

    def test_mfhi_after_a_call_or_control_flow_refused(self):
        for between in ("D_80190100();", "if (af) af++;", "af = af * 3;"):
            src = ("extern void D_80190100(void);\n"
                   "int func_80190000(int af) { register int hi asm(\"$7\");\n"
                   "  asm volatile(\"mult %0, %1\" : : \"r\"(af), \"r\"(af));\n"
                   f"  {between}\n"
                   "  asm volatile(\"mfhi %0\" : \"=r\"(hi)); return hi; }")
            with self.assertRaises(gl.LowerError, msg=between):
                gl.lower_leaf("func_80190000", src, set(), {}, strip_asm=self.strip())

    def test_slti_sltiu(self):
        src = """
        int func_80190000(int a) {
            int s, u;
            asm("slti %0, %1, -5" : "=r"(s) : "r"(a));
            asm("sltiu %0, %1, -5" : "=r"(u) : "r"(a));
            return s * 10 + u;
        }"""
        out = self.run_c(tu(src), """
            printf("%d %d %d\\n", func_80190000(-6), func_80190000(3), func_80190000(-4));""")
        # sltiu compares against the sign-extended immediate as unsigned
        self.assertEqual(out.split(), ["11", "1", "0"])

    @staticmethod
    def strip():
        # register pins (`register int x asm("$2")`) are cc1 hints the
        # generators strip before lowering, exactly as here
        import gen_decomp_ports as g
        return g.strip_era_asm

    def tu_pinned(self, src: str) -> str:
        res = gl.lower_leaf("func_80190000", src, set(), {}, strip_asm=self.strip())
        return MOCK_H + "\n".join(res["helpers"]) + f"\n{res['sig']}\n{{\n{res['body']}\n}}\n"


@unittest.skipUnless(shutil.which("cc") or shutil.which("gcc"), "no host C compiler")
class LocalHelpers(CRunner):
    def test_static_helpers_are_lowered_as_static_host_functions(self):
        # src/func_8005A318.c's shape: `static inline` helpers the leaf calls.
        src = """
        extern int D_8009D050;
        static inline int clamp(int a) { return a < D_8009D050 ? a : D_8009D050; }
        static inline int twice(int a) { return clamp(a) * 2; }
        int func_80190000(int x) { return twice(x) + clamp(x + 1); }"""
        res = lower(src)
        defs = "\n".join(res["local_defs"])
        self.assertIn("static int32_t clamp(", defs)
        self.assertIn("static int32_t twice(", defs)
        self.assertNotIn("clamp", res["callees"])
        out = self.run_c_tu(res, """
            PE_StoreU32(0x8009D050u, 10u);
            printf("%d %d\\n", func_80190000(3), func_80190000(20));""")
        self.assertEqual(out.split(), [str(3 * 2 + 4), str(10 * 2 + 10)])

if __name__ == "__main__":
    unittest.main()
