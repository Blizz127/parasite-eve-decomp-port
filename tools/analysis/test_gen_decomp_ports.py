#!/usr/bin/env python3
"""Unit tests for the gen_decomp_ports.py host-adaptation rules.

Run: python3 tools/analysis/test_gen_decomp_ports.py

Covers the rules added for the absent-leaf lane:
  E2b  era register pins / empty-template asm fences are stripped, real asm
       and read-before-write pins are not;
  E7c  guest-pointer-vector (`T **args`) parameters: dereferenced element
       uses become PE_DECOMP_PTRGLOBAL(pe_args + 4u * (k), T), anything else is
       rejected;
  derived host signatures: a caller's retail prototype is replaced only when
       it disagrees with the generated callee's host signature;
  E18  a leaf struct with pointer members is guest layout (4-byte pointers)
       and is rejected rather than overlaid with the host layout;
  call-argument guest addresses: `&D_X`, `&D_X[k]`, array/pointer-global
       `D_X + k` as *whole* call arguments become guest addresses, never a
       sub-expression such as a pointer cast.
"""
from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_decomp_ports as g  # noqa: E402


def analyze(src: str, name: str = "func_80099990") -> dict:
    matched = {"name": name, "file_offset": 0, "file_size": 4, "vram": 0, "words": 1}
    return g.analyze(name, src, matched, set(), set(), {}, {})


class EraAsmTests(unittest.TestCase):
    def test_member_named_like_a_pin_is_not_a_read(self):
        # func_80070FAC: `lo = a.w.lo;` — the member `.lo` is not the pinned
        # `lo`, which is written (`lo = lo0 + lo1`) before it is read.
        src = ("union Sq { long long ll; struct { unsigned int lo; unsigned int hi; } w; };\n"
               "int func_80099990(int x) {\n    union Sq a;\n"
               '    register unsigned int lo asm("$2");\n'
               "    a.ll = (long long)x * x;\n    lo = a.w.lo;\n    return (int)lo;\n}\n")
        self.assertTrue(analyze(src)["eligible"], analyze(src).get("reason"))

    def test_pin_read_before_write_still_rejected(self):
        src = ('int func_80099990(void) {\n    register int v asm("$3");\n'
               "    return v + 1;\n}\n")
        self.assertFalse(analyze(src)["eligible"])

    def test_pins_and_fences_are_stripped(self):
        src = ('int f(void){ register int *p asm("$2") = &x; '
               'asm volatile("" : "=r"(p) : "0"(p)); '
               '__asm__ __volatile__("" ::: "memory"); return *p; }')
        out = g.strip_era_asm(src)
        self.assertNotIn('asm("$2")', out)
        self.assertNotRegex(out, r'\basm\s+volatile\(""')
        self.assertIn("register int *p = &x;", out)

    def test_real_instruction_is_kept(self):
        src = 'void f(void){ asm volatile ("nop"); }'
        self.assertIn('asm volatile ("nop")', g.strip_era_asm(src))

    def test_pin_only_leaf_becomes_eligible(self):
        src = ('extern unsigned int D_800BCF88;\n'
               'int func_80099990(void) {\n'
               '    register unsigned int *ptr asm("$2") = &D_800BCF88;\n'
               '    asm volatile("" : "=r"(ptr) : "0"(ptr));\n'
               '    *ptr |= 0xC0;\n    return 1;\n}\n')
        self.assertTrue(analyze(src)["eligible"])

    def test_pin_read_before_write_is_rejected(self):
        src = ('int func_80099990(void) {\n'
               '    register int live asm("$4");\n'
               '    return live + 1;\n}\n')
        plan = analyze(src)
        self.assertFalse(plan["eligible"])
        self.assertIn("read before written", plan["reason"])

    def test_nop_asm_is_still_rejected(self):
        src = 'void func_80099990(void) {\n    asm volatile ("nop");\n}\n'
        self.assertEqual(analyze(src)["reason"], "asm")


class ArgVecTests(unittest.TestCase):
    def test_dereferenced_forms(self):
        body = "x = *a0[1]; y = **a0; z = **(a0 + 2); *(int *)*a0 = 3;"
        out, why = g.argvec_rewrite(body, "a0", "unsigned int")
        self.assertEqual(why, "")
        self.assertIn("*PE_DECOMP_PTRGLOBAL(pe_a0 + 4u * (1), unsigned int)", out)
        self.assertIn("*PE_DECOMP_PTRGLOBAL(pe_a0 + 4u * (0), unsigned int)", out)
        self.assertIn("*PE_DECOMP_PTRGLOBAL(pe_a0 + 4u * (2), unsigned int)", out)
        self.assertIn("*(int *)PE_DECOMP_PTRGLOBAL(pe_a0 + 4u * (0), unsigned int)", out)
        self.assertNotRegex(out, r"\ba0\b")

    def test_undereferenced_element_is_rejected(self):
        _, why = g.argvec_rewrite("f(a0[2]);", "a0", "int")
        self.assertIn("undereferenced", why)

    def test_bare_vector_is_rejected(self):
        _, why = g.argvec_rewrite("p = a0; *p[0] = 1;", "a0", "int")
        self.assertIn("used as a value", why)

    def test_member_of_same_name_is_not_the_vector(self):
        out, why = g.argvec_rewrite("s.a0 = *a0[0];", "a0", "int")
        self.assertEqual(why, "")
        self.assertIn("s.a0 =", out)

    def test_leaf_with_vector_param_is_eligible(self):
        src = ('extern unsigned int D_8009D2E8;\n'
               'int func_80099990(unsigned int **arg0) {\n'
               '    D_8009D2E8 |= *arg0[0];\n    return 1;\n}\n')
        self.assertTrue(analyze(src)["eligible"])

    def test_other_double_pointer_still_rejected(self):
        src = ('int func_80099990(unsigned int **arg0) {\n'
               '    unsigned char *q = *(unsigned char **)(*arg0[0]);\n'
               '    return *q;\n}\n')
        self.assertFalse(analyze(src)["eligible"])


class GuestLayoutTests(unittest.TestCase):
    def test_struct_with_pointer_member_is_rejected(self):
        src = ('typedef struct { short *first; short *second; } Arguments;\n'
               'int func_80099990(Arguments *arg0) {\n'
               '    return *arg0->first;\n}\n')
        plan = analyze(src)
        self.assertFalse(plan["eligible"])
        self.assertIn("guest layout", plan["reason"])

    def test_scalar_only_struct_is_fine(self):
        src = ('typedef struct { short a; unsigned int b; } Rec;\n'
               'int func_80099990(Rec *arg0) {\n'
               '    return arg0->a;\n}\n')
        self.assertTrue(analyze(src)["eligible"])


class DerivedSignatureTests(unittest.TestCase):
    def test_same_types_keep_leaf_declaration(self):
        src = "extern S64 func_80072DF4(S64 a, S64 b);\n"
        dsig = {"ret": "S64", "params": ["S64 x", "S64 y"]}
        self.assertTrue(g.same_host_decl(src, "func_80072DF4", dsig))

    def test_pointer_vs_guest_address_is_overridden(self):
        src = "extern void func_8008F1B0(unsigned char *a0, unsigned int a1);\n"
        dsig = {"ret": "void", "params": ["pe_addr_t pe_a0", "unsigned int a1"]}
        self.assertFalse(g.same_host_decl(src, "func_8008F1B0", dsig))

    def test_return_type_difference_is_overridden(self):
        src = "extern void func_8007E0C0(void);\n"
        dsig = {"ret": "int", "params": []}
        self.assertFalse(g.same_host_decl(src, "func_8007E0C0", dsig))


class CallAddressArgTests(unittest.TestCase):
    decls = {"D_800116FC": "char D_800116FC",
             "D_8009EC38": "E D_8009EC38[]",
             "D_8009D2F0": "unsigned char *D_8009D2F0",
             "D_8009CF50": "int D_8009CF50"}

    def rewrite(self, body: str, known=(), sigs=None) -> str:
        g._ADDR_KNOWN.clear()
        g._ADDR_KNOWN.update(known)
        g._ADDR_SIGS.clear()
        g._ADDR_SIGS.update(sigs or {})
        return g.fix_argument_addresses(body, set(self.decls), self.decls)

    def test_address_of_scalar_to_boundary(self):
        out = self.rewrite("func_80073C5C(&D_800116FC);")
        self.assertIn("func_80073C5C((pe_addr_t)0x800116FCu)", out)

    def test_indexed_element_address(self):
        out = self.rewrite("func_80077AC4(p, &D_8009EC38[i]);")
        self.assertIn("(pe_addr_t)(0x8009EC38u + (uint32_t)(i) * sizeof(E))", out)

    def test_pointer_global_plus_offset(self):
        out = self.rewrite("func_8003E0D0(D_8009D2F0 + 0x1B4);")
        self.assertIn("PE_LoadU32(0x8009D2F0u) + (uint32_t)(0x1B4) * sizeof(unsigned char)", out)

    def test_scalar_plus_is_a_value(self):
        out = self.rewrite("func_8004CE28(a0 + 0x47, D_8009CF50 + 0x42);")
        self.assertIn("D_8009CF50 + 0x42", out)

    def test_bare_scalar_argument_is_its_value(self):
        # func_80077404: `func_80073E10(D_80095884)` passes the int's value
        # (retail lw), never the symbol's address.
        out = self.rewrite("func_80073E10(D_8009CF50);")
        self.assertIn("func_80073E10(D_8009CF50)", out)
        self.assertNotIn("0x8009CF50u", out)

    def test_bare_scalar_condition_is_its_value(self):
        # func_8005E518: `if (D_8009D0E8)` became a constant-true address.
        out = self.rewrite("if (D_8009CF50) { x = 1; }")
        self.assertIn("if (D_8009CF50)", out)

    def test_bare_pointer_global_passes_stored_pointer(self):
        # func_800403C8: `func_800726F4(D_800BCDA8)` with `void *D_800BCDA8`
        # passes the guest pointer held in the slot, not the slot address.
        out = self.rewrite("func_800726F4(D_8009D2F0);")
        self.assertIn("func_800726F4((pe_addr_t)PE_LoadU32(0x8009D2F0u))", out)

    def test_consecutive_bare_arguments(self):
        # room func_8018F09C: `func_800C2758(o, D_8018FFF8, D_8019001C)`.
        out = self.rewrite("func_80012345(o, D_8009EC38, D_8009EC38);")
        self.assertEqual(out.count("(pe_addr_t)0x8009EC38u"), 2, out)

    def test_bare_array_argument_is_its_address(self):
        out = self.rewrite("func_80012345(D_8009EC38);")
        self.assertIn("func_80012345((pe_addr_t)0x8009EC38u)", out)

    def test_cast_subexpression_is_untouched(self):
        body = "t = *(unsigned int *)(D_8009D2F0 + 0x4C); func_80012345(t);"
        self.assertEqual(self.rewrite(body), body)

    def test_host_pointer_parameter_is_untouched(self):
        sigs = {"func_80074E28": {"ret": "void",
                                  "params": ["pe_addr_t name", "const RECT *rect"]}}
        out = self.rewrite("func_80074E28(&D_800116FC, &D_800116FC);",
                           known={"func_80074E28"}, sigs=sigs)
        self.assertIn("func_80074E28((pe_addr_t)0x800116FCu, &D_800116FC)", out)

    def test_known_callee_without_host_signature_is_untouched(self):
        out = self.rewrite("func_80012345(&D_800116FC);", known={"func_80012345"})
        self.assertIn("func_80012345(&D_800116FC)", out)


class CodeAddressTests(unittest.TestCase):
    def rewrite(self, body: str, known=(), sigs=None) -> str:
        g._ADDR_KNOWN.clear()
        g._ADDR_KNOWN.update(known)
        g._ADDR_SIGS.clear()
        g._ADDR_SIGS.update(sigs or {})
        return g.rewrite_code_addresses(body)

    def test_integer_cast_is_the_retail_vma(self):
        out = self.rewrite("*(unsigned int *)(p + 0x30) = (unsigned int)func_80043B0C;")
        self.assertIn("= (unsigned int)0x80043B0Cu;", out)

    def test_int_cast_of_address_of(self):
        out = self.rewrite("s[12] = (int)&func_80064EB4;")
        self.assertIn("s[12] = (int)0x80064EB4u;", out)

    def test_bare_argument_to_boundary(self):
        out = self.rewrite("func_80073CF4(2, func_80076EE4);")
        self.assertIn("func_80073CF4(2, (pe_addr_t)0x80076EE4u)", out)

    def test_bare_argument_to_guest_address_parameter(self):
        sigs = {"func_800638D8": {"ret": "int",
                                  "params": ["int slot", "pe_addr_t fn"]}}
        out = self.rewrite("func_800638D8(a0, func_80050618);",
                           known={"func_800638D8"}, sigs=sigs)
        self.assertIn("func_800638D8(a0, (pe_addr_t)0x80050618u)", out)

    def test_host_function_pointer_parameter_is_untouched(self):
        sigs = {"func_800638D8": {"ret": "int",
                                  "params": ["int slot", "void (*fn)(void)"]}}
        body = "func_800638D8(a0, func_80050618);"
        self.assertEqual(self.rewrite(body, known={"func_800638D8"}, sigs=sigs), body)

    def test_void_pointer_store_is_untouched(self):
        body = "*(void **)(p + 0x2C) = func_800452C0;"
        self.assertEqual(self.rewrite(body), body)

    def test_call_is_not_a_value(self):
        body = "x = (int)func_80012345(1);"
        self.assertEqual(self.rewrite(body), body)

    def test_nested_call_offsets_stay_correct(self):
        # The inner rewrite changes length; the outer call's arguments must
        # still be split on the edited text.
        out = self.rewrite("func_80011111(func_80022222(func_80033333), func_80044444);")
        self.assertEqual(
            out, "func_80011111(func_80022222((pe_addr_t)0x80033333u), "
                 "(pe_addr_t)0x80044444u);")


class PointerReturnTests(unittest.TestCase):
    """E7 relaxed: object-pointer returns go through PE_HostToGuest."""
    NAME = "func_80099990"

    def render(self, src: str) -> str:
        g._ADDR_KNOWN.clear()
        g._ADDR_SIGS.clear()
        plan = analyze(src, self.NAME)
        self.assertTrue(plan["eligible"], plan.get("reason"))
        leaf = {"name": self.NAME, "file_offset": 0, "file_size": 4,
                "vram": 0x80099990, "words": 1}
        out = g.render(self.NAME, plan, {self.NAME: leaf}, set(), {})
        cc = g.find_compiler()
        if cc:
            errs = g.compile_check(cc, [(self.NAME, out)])
            self.assertEqual(errs, {}, out)
        return out

    def test_array_element_return(self):
        out = self.render(
            "extern unsigned char D_80092478[];\n"
            "unsigned char *func_80099990(int index) {\n"
            "    if (index < 0) return 0;\n"
            "    return D_80092478 + index * 16;\n}\n")
        self.assertIn("static unsigned char *pe_host_func_80099990(int index)", out)
        self.assertIn("pe_addr_t func_80099990(int index)", out)
        self.assertIn("return PE_HostToGuest(pe_host_func_80099990(index));", out)

    def test_void_parameter_list(self):
        out = self.render(
            "extern unsigned short D_8009B582;\n"
            "unsigned short *func_80099990(void) {\n"
            "    return &D_8009B582;\n}\n")
        self.assertIn("pe_addr_t func_80099990(void)", out)
        self.assertIn("PE_HostToGuest(pe_host_func_80099990())", out)

    def test_pointer_parameter_wrapper(self):
        out = self.render(
            "unsigned char *func_80099990(unsigned char *a0) {\n"
            "    return a0 + 4;\n}\n")
        self.assertIn("static unsigned char * pe_host_func_80099990(pe_addr_t pe_a0)"
                      .replace("* pe", "*pe"), out.replace("* pe", "*pe"))
        self.assertIn("PE_HostToGuest(pe_host_func_80099990(pe_a0))", out)

    def test_recursive_pointer_return_is_reported(self):
        plan = analyze(
            "extern unsigned char D_80092478[];\n"
            "unsigned char *func_80099990(int n) {\n"
            "    if (n) return func_80099990(n - 1);\n"
            "    return D_80092478;\n}\n")
        self.assertFalse(plan["eligible"])

    def test_function_pointer_return_is_reported(self):
        plan = analyze("int (*func_80099990(void))(void) { return 0; }\n")
        self.assertFalse(plan["eligible"])


class GuestPointerCastTests(unittest.TestCase):
    """E9b/E9/E16: a cast must be rooted at a host pointer into guest RAM."""

    def reason(self, src: str) -> str | None:
        plan = analyze(src)
        return None if plan["eligible"] else plan["reason"]

    def test_pointer_to_pointer_cast_is_rejected(self):
        # func_80030640: `*(unsigned int **)(rec + 0x68)` read 8 host bytes.
        r = self.reason(
            "extern unsigned char *D_8009D278;\n"
            "void func_80099990(void) {\n"
            "    unsigned char *rec = D_8009D278;\n"
            "    unsigned int *inner = *(unsigned int **)(rec + 0x68);\n"
            "    inner[4] = 0;\n}\n")
        self.assertIn("pointer-to-pointer cast", r)

    def test_pointer_to_pointer_cast_of_int_param(self):
        # func_80017EC4: `*(unsigned short **)a0` on an int parameter.
        r = self.reason("int func_80099990(int a0) {\n"
                        "    unsigned short *p = *(unsigned short **)a0;\n"
                        "    return *p;\n}\n")
        self.assertIn("pointer-to-pointer cast", r)

    def test_scalar_symbol_plus_offset_is_not_a_pointer(self):
        # func_80017EC4: `(unsigned char *)(D_8009D2F0 + 0xF)`, D_ an int.
        r = self.reason("extern unsigned int D_8009D2F0;\n"
                        "int func_80099990(void) {\n"
                        "    return *(unsigned char *)(D_8009D2F0 + 0xF);\n}\n")
        self.assertIn("pointer cast", r)

    def test_int_local_holding_address_is_not_a_pointer(self):
        r = self.reason("extern unsigned int D_8009D2F0;\n"
                        "void func_80099990(void) {\n"
                        "    unsigned int base = D_8009D2F0;\n"
                        "    *(unsigned int *)(base + 4) = 0;\n}\n")
        self.assertIn("pointer cast", r)

    def test_indexed_pointer_value_is_not_a_pointer(self):
        # func_80018364: `(unsigned int *)base[0x28]` casts a loaded word.
        r = self.reason("extern unsigned int *D_8009D2F0;\n"
                        "void func_80099990(void) {\n"
                        "    unsigned int *base = D_8009D2F0;\n"
                        "    unsigned int *q = (unsigned int *)base[0x28];\n"
                        "    q[1] = 0;\n}\n")
        self.assertIn("pointer cast", r)

    def test_pointer_returning_callee_is_not_a_root(self):
        r = self.reason("extern unsigned char *func_80062CC4();\n"
                        "int func_80099990(void) {\n"
                        "    return *(int *)(func_80062CC4() + 0x24);\n}\n")
        self.assertIsNotNone(r)

    def test_pointer_local_arithmetic_is_accepted(self):
        self.assertIsNone(self.reason(
            "extern unsigned char *D_80091A28;\n"
            "int func_80099990(unsigned int a0) {\n"
            "    unsigned char *base = D_80091A28;\n"
            "    unsigned char *p = base + (a0 & 0xFF);\n"
            "    return *(unsigned char *)(p + 0x1D);\n}\n"))

    def test_address_of_scalar_symbol_is_accepted(self):
        # func_800144FC / func_80068D28: `(T *)&D_800B0CD8`.
        self.assertIsNone(self.reason(
            "extern unsigned int D_800B0CD8;\n"
            "void func_80099990(void) {\n"
            "    unsigned short *s = (unsigned short *)&D_800B0CD8;\n"
            "    s[1] = 0;\n}\n"))

    def test_parenthesised_pointer_narrowed_to_int(self):
        # func_8005DC4C: `(unsigned int)(tbl + k)` truncates a host pointer.
        r = self.reason("extern unsigned char D_800A8028[];\n"
                        "unsigned int func_80099990(int k) {\n"
                        "    unsigned char *tbl = D_800A8028;\n"
                        "    return (unsigned int)(tbl + k);\n}\n")
        self.assertIn("narrowed", r)

    def test_abstract_declarator_is_not_a_cast(self):
        self.assertIsNone(self.reason(
            "extern void func_8002F7D8(void *);\n"
            "void func_80099990(void) { func_8002F7D8(0); }\n"))

    def test_member_arrow_is_not_subtraction(self):
        # func_800CCAB0: `(signed char)(a2->a68[k] >> 8)` is a value.
        self.assertIsNone(self.reason(
            "typedef struct { short a68[4]; } S;\n"
            "int func_80099990(S *a2, int k) {\n"
            "    return (signed char)(a2->a68[k] >> 8);\n}\n"))

    def test_argvec_element_cast_is_accepted(self):
        self.assertIsNone(self.reason(
            "int func_80099990(unsigned char **a0) {\n"
            "    return *(unsigned short *)a0[1];\n}\n"))

    def test_loaded_word_cast_to_pointer_is_rejected(self):
        # func_8004006C: `(unsigned char *)*arg++` on an int cursor.
        r = self.reason("extern int D_800A1708[];\n"
                        "int func_80099990(void) {\n"
                        "    int *arg = D_800A1708;\n"
                        "    unsigned char *s = (unsigned char *)*arg++;\n"
                        "    return s[0];\n}\n")
        self.assertIn("pointer cast", r)


class NullGuestPointerTests(unittest.TestCase):
    """A guest NULL parameter must reach the leaf's own null test
    (func_80080998 called with dest 0 from func_80080DC4 aborted in an eager
    PE_Translate before `if (host_a0 != 0)` ran)."""

    def test_pointer_parameter_local_is_null_preserving(self):
        g._ADDR_KNOWN.clear()
        g._ADDR_SIGS.clear()
        src = ("void func_80099990(unsigned char *a0, unsigned char *a1) {\n"
               "    if (a1 != 0) { if (a0 != 0) { *a0 = *a1; } }\n"
               "    else if (a0 != 0) { *a0 = 0; }\n}\n")
        plan = analyze(src)
        self.assertTrue(plan["eligible"], plan.get("reason"))
        leaf = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                "vram": 0x80099990, "words": 1}
        out = g.render("func_80099990", plan, {"func_80099990": leaf}, set(), {})
        self.assertIn("host_a0 = (unsigned char *)PE_DecompTranslateOrNull(pe_a0);", out)
        self.assertIn("host_a1 = (unsigned char *)PE_DecompTranslateOrNull(pe_a1);", out)
        self.assertNotIn("PE_Translate(pe_a0", out)

    def test_wrapper_pointer_global_local_is_null_preserving(self):
        # func_800136C0 only compares D_8009D254 (null there); the wrapper's
        # entry load must not abort (portverify) — it goes through the
        # null-preserving PE_DECOMP_PTRGLOBAL.
        g._ADDR_KNOWN.clear()
        g._ADDR_SIGS.clear()
        src = ("extern unsigned char *D_8009D254;\n"
               "int func_80099990(unsigned char *o) {\n"
               "    if (o != D_8009D254) return 1;\n"
               "    return D_8009D254[3];\n}\n")
        plan = analyze(src)
        self.assertTrue(plan["eligible"], plan.get("reason"))
        leaf = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                "vram": 0x80099990, "words": 1}
        out = g.render("func_80099990", plan, {"func_80099990": leaf}, set(), {})
        self.assertIn("unsigned char *host_D_8009D254 = "
                      "PE_DECOMP_PTRGLOBAL(0x8009D254u, unsigned char);", out)
        self.assertNotIn("PE_Translate(PE_LoadU32", out)

    def test_pointer_global_macro_is_null_preserving(self):
        hdr = (g.REPO_ROOT / "pc_port/include/pe_guest_decomp.h").read_text()
        self.assertIn("PE_DecompTranslateOrNull(PE_LoadU32((pe_addr_t)(addr)))", hdr)
        self.assertIn("return addr ? PE_Translate(addr, 1u) : (void *)0;", hdr)


class NullTestedRuleTests(unittest.TestCase):
    """portverify finding (6): guest 0 -> host NULL only for names the leaf
    null-tests; everything else translates 0 through the RAM mirror."""

    def test_direct_forms(self):
        for body in ("if (p) x = 1;", "if (p != 0) x;", "if (0 == p) x;",
                     "return !p;", "while (p) p = q;", "y = p && z;",
                     "y = p ? 1 : 2;", "if (a && p) x;", "if (p == NULL) x;"):
            self.assertTrue(g.null_tested(body, "p"), body)

    def test_alias_copy_is_followed(self):
        body = "unsigned char *q = D_8009D254; if (q) { q[3] = 1; }"
        self.assertTrue(g.null_tested(body, "D_8009D254"))
        body = "unsigned char *q; q = (unsigned char *)D_8009D254; if (!q) return 0;"
        self.assertTrue(g.null_tested(body, "D_8009D254"))

    def test_not_null_tested(self):
        for body in ("x = p[3];", "if (p[2] != 0) x;", "if (o != p) x;",
                     "if (!p->f) x;", "q = p + 4; if (q) x;", "x = *p != 0;"):
            self.assertFalse(g.null_tested(body, "p"), body)

    def render(self, src: str) -> str:
        g._ADDR_KNOWN.clear()
        g._ADDR_SIGS.clear()
        plan = analyze(src)
        self.assertTrue(plan["eligible"], plan.get("reason"))
        leaf = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                "vram": 0x80099990, "words": 1}
        return g.render("func_80099990", plan, {"func_80099990": leaf}, set(), {})

    def test_untested_param_uses_the_mirror(self):
        out = self.render("int func_80099990(unsigned char *a0) {\n"
                          "    return a0[2];\n}\n")
        self.assertIn("host_a0 = (unsigned char *)PE_Translate(pe_a0, 1u);", out)
        self.assertNotIn("OrNull", out)

    def test_plain_render_ptrglobal_macro_choice(self):
        out = self.render("extern unsigned char *D_8009D254;\n"
                          "int func_80099990(void) {\n"
                          "    if (D_8009D254 == 0) return 0;\n"
                          "    return D_8009D254[4];\n}\n")
        self.assertIn("PE_DECOMP_PTRGLOBAL_NULLABLE(0x8009D254u", out)
        out = self.render("extern unsigned char *D_8009D254;\n"
                          "int func_80099990(void) {\n"
                          "    return D_8009D254[4];\n}\n")
        self.assertIn("#define D_8009D254 PE_DECOMP_PTRGLOBAL(0x8009D254u", out)

    def test_pointer_return_single_root_keeps_segment(self):
        # finding (7): func_800C2B10 returns D_800E2248 + k; with a KUSEG
        # base retail returns the KUSEG sum, not its KSEG0 alias.
        out = self.render("extern char *D_800E2248;\n"
                          "void *func_80099990(int arg0) {\n"
                          "    return D_800E2248 + arg0 * 4 + 8;\n}\n")
        self.assertIn("pe_addr_t pe_root = PE_LoadU32(0x800E2248u);", out)
        self.assertIn("PE_DecompReturnSegment(PE_HostToGuest(pe_host_func_80099990(arg0)), pe_root)", out)


class CallArgsTests(unittest.TestCase):
    def test_prototypes_are_not_calls(self):
        text = ("extern int func_8006E6A8();\nvoid *func_80011111(void);\n"
                "int f(void) { return func_8006E6A8(1, 2, 3) + *func_80011111(); }")
        calls = g.call_args(text)
        self.assertIn(("func_8006E6A8", "1, 2, 3"), calls)
        self.assertEqual([c for c, _ in calls].count("func_8006E6A8"), 1)
        self.assertIn(("func_80011111", ""), calls)   # `+ *func_X()` is a call

    def test_return_and_cast_calls(self):
        calls = g.call_args("x = (int)func_80022222(4); return func_80033333(5);")
        self.assertEqual(calls, [("func_80022222", "4"), ("func_80033333", "5")])

    def test_empty_prototype_does_not_fail_arity(self):
        src = ("extern int func_8006E6A8();\n"
               "int func_80099990(int a) { return func_8006E6A8(a, 2, 3); }\n")
        matched = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                   "vram": 0, "words": 1}
        plan = g.analyze("func_80099990", src, matched, {"func_8006E6A8"}, set(),
                         {"func_8006E6A8": 3},
                         {"func_8006E6A8": {"ret": "int",
                                            "params": ["int a", "int b", "int c"]}})
        self.assertTrue(plan["eligible"], plan.get("reason"))


class GuestCallTests(unittest.TestCase):
    """Step 3: calls through guest code pointers become PE_GuestCall."""

    def render(self, src: str) -> str:
        g._ADDR_KNOWN.clear()
        g._ADDR_SIGS.clear()
        plan = analyze(src)
        self.assertTrue(plan["eligible"], plan.get("reason"))
        leaf = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                "vram": 0x80099990, "words": 1}
        out = g.render("func_80099990", plan, {"func_80099990": leaf}, set(), {})
        cc = g.find_compiler()
        if cc:
            self.assertEqual(g.compile_check(cc, [("func_80099990", out)]), {}, out)
        return out

    def test_local_slot_pointer(self):
        # func_80073D88: `unsigned int (*f)() = *(unsigned int (**)())(D_8009566C + 0x10); f();`
        out = self.render(
            "extern unsigned char *D_8009566C;\n"
            "void func_80099990(void) {\n"
            "    unsigned int (*f)() = *(unsigned int (**)())(D_8009566C + 0x10);\n"
            "    f();\n}\n")
        self.assertIn("pe_addr_t f = *(pe_addr_t *)(D_8009566C + 0x10);", out)
        self.assertIn('PE_GuestCall("@80099990:f", (pe_addr_t)(f), 0u, 0, 0, 0, 0)', out)

    def test_function_pointer_global_with_address_argument(self):
        out = self.render(
            "extern char D_80011840;\n"
            "extern void (*D_80095748)();\n"
            "void func_80099990(int a0) {\n"
            "    if (D_80095748 != 0) D_80095748(&D_80011840, a0);\n}\n")
        self.assertIn("PE_DECOMP_SCALAR(0x80095748u, pe_addr_t)", out)
        self.assertIn('PE_GuestCall("@80099990:D_80095748", (pe_addr_t)(D_80095748), 2u, '
                      "(uintptr_t)(uint32_t)((pe_addr_t)0x80011840u), (uintptr_t)(uint32_t)(a0), 0, 0)", out)

    def test_cast_slot_call(self):
        out = self.render(
            "extern unsigned int *D_80095744;\n"
            "void func_80099990(int a) {\n"
            "    unsigned int *v1 = D_80095744;\n"
            "    (*(void (**)(int, int))(v1 + 2))(v1[6], a);\n}\n")
        self.assertIn('PE_GuestCall("@80099990:indirect", '
                      "(pe_addr_t)(*(pe_addr_t *)(v1 + 2)), 2u, (uintptr_t)(uint32_t)(v1[6]), "
                      "(uintptr_t)(uint32_t)(a), 0, 0)", out)

    def test_int_word_cast_to_function_pointer(self):
        out = self.render(
            "extern unsigned int *D_80095744;\n"
            "void func_80099990(void) {\n"
            "    int (*h)();\n"
            "    h = (int (*)())D_80095744[13];\n"
            "    h(1);\n}\n")
        self.assertIn("h = (pe_addr_t)D_80095744[13];", out)
        self.assertIn('PE_GuestCall("@80099990:h", (pe_addr_t)(h), 1u, (uintptr_t)(uint32_t)(1), 0, 0, 0)', out)

    def test_prototype_parameter_is_an_address(self):
        text, why = g.rewrite_guest_calls(
            "extern void func_80073D24(int slot, void (*cb)(void));\n", "func_80099990")
        self.assertEqual(why, "")
        self.assertIn("extern void func_80073D24(int slot, pe_addr_t cb);", text)

    def test_stack_arguments_are_reported(self):
        plan = analyze(
            "extern void (*D_80095748)();\n"
            "void func_80099990(void) { D_80095748(1, 2, 3, 4, 5); }\n")
        self.assertIn("stack args", plan["reason"])

    def test_host_pointer_argument_is_reported(self):
        plan = analyze(
            "extern void (*D_80095748)();\n"
            "void func_80099990(void) { char buf[8]; D_80095748(buf); }\n")
        self.assertFalse(plan["eligible"])
        self.assertIn("guest call", plan["reason"])


class ReturnRootTests(unittest.TestCase):
    """portverify follow-up to (7): a root reached through a local copy."""

    def test_copy_then_return(self):
        body = "int *f(int *a0, int *a1) { register int *d = a0; d[5] = a1[0]; return d; }"
        self.assertEqual(g.return_root(body, {"a0", "a1"}), "a0")

    def test_advanced_parameter(self):
        body = ("char *f(char *dst, char *src, int n) { if (dst >= src) { while (n-- > 0)"
                " dst[n] = src[n]; } else { while (n-- > 0) *dst++ = *src++; } return dst; }")
        self.assertEqual(g.return_root(body, {"dst", "src"}), "dst")

    def test_disagreeing_returns(self):
        body = "char *f(char *a, char *b, int k) { if (k) return a; return b + 1; }"
        self.assertIsNone(g.return_root(body, {"a", "b"}))

    def test_null_return_agrees(self):
        body = "char *f(char *a, char *b) { if (!a) return 0; return a + 4; }"
        self.assertEqual(g.return_root(body, {"a", "b"}), "a")

    def test_thunk_uses_the_traced_root(self):
        g._ADDR_KNOWN.clear()
        g._ADDR_SIGS.clear()
        plan = analyze("int *func_80099990(int *a0, int *a1) {\n"
                       "    int *d = a0;\n    d[5] = a1[0];\n    return d;\n}\n")
        self.assertTrue(plan["eligible"], plan.get("reason"))
        leaf = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                "vram": 0x80099990, "words": 1}
        out = g.render("func_80099990", plan, {"func_80099990": leaf}, set(), {})
        self.assertIn("pe_addr_t pe_root = pe_a0;", out)


class GuestMemberCallTests(unittest.TestCase):
    def test_member_function_pointer_call(self):
        # room_m0269i func_8018F138: `o->fn(o)` with `int (*fn)();` a member.
        g._ADDR_KNOWN.clear()
        g._ADDR_SIGS.clear()
        src = ("typedef struct { int pad; int (*fn)(); } O;\n"
               "void func_80099990(O *o) {\n    o->fn(o);\n}\n")
        plan = analyze(src)
        self.assertTrue(plan["eligible"], plan.get("reason"))
        leaf = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                "vram": 0x80099990, "words": 1}
        out = g.render("func_80099990", plan, {"func_80099990": leaf}, set(), {})
        self.assertIn("pe_addr_t fn;", out)
        self.assertIn('PE_GuestCall("@80099990:fn", (pe_addr_t)(host_o->fn), 1u, '
                      "(uintptr_t)(uint32_t)( pe_o), 0, 0, 0)", out)
        cc = g.find_compiler()
        if cc:
            self.assertEqual(g.compile_check(cc, [("func_80099990", out)]), {}, out)


class BoundaryHostPointerTests(unittest.TestCase):
    def test_host_stack_object_to_boundary_is_reported(self):
        # func_800D5898: func_800D27FC(a1[0], a1[2], &v18, d, 1), unported.
        plan = analyze("typedef struct { char b[4]; } Blk4;\n"
                       "extern void func_800D27FC();\n"
                       "int func_80099990(int a) {\n    Blk4 v18;\n"
                       "    func_800D27FC(a, &v18);\n    return 0;\n}\n")
        self.assertFalse(plan["eligible"])
        self.assertIn("passed to boundary func_800D27FC", plan["reason"])

    def test_scalar_and_data_address_to_boundary_are_fine(self):
        plan = analyze("extern char D_80011840;\nextern void func_800D27FC();\n"
                       "int func_80099990(int a) { func_800D27FC(a, &D_80011840); return 0; }\n")
        self.assertTrue(plan["eligible"], plan.get("reason"))


class HostPointerLocalArgTests(unittest.TestCase):
    def render(self, src: str, sigs=None, known=()):
        g._ADDR_KNOWN.clear(); g._ADDR_KNOWN.update(known)
        g._ADDR_SIGS.clear(); g._ADDR_SIGS.update(sigs or {})
        matched = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                   "vram": 0x80099990, "words": 1}
        plan = g.analyze("func_80099990", src, matched, set(known), set(),
                         {k: len(v["params"]) for k, v in (sigs or {}).items()},
                         sigs or {})
        self.assertTrue(plan["eligible"], plan.get("reason"))
        return g.render("func_80099990", plan, {"func_80099990": matched}, set(known),
                        {k: len(v["params"]) for k, v in (sigs or {}).items()})

    def test_pointer_local_to_boundary(self):
        # func_800809E0: `s = &D_80011E3C; func_80071A74(&D_80011E4C, s);`
        out = self.render("extern char D_80011E3C;\nextern char D_80011E4C;\n"
                          "extern void func_80071A74();\n"
                          "void func_80099990(void) {\n    char *s;\n"
                          "    s = &D_80011E3C;\n    func_80071A74(&D_80011E4C, s);\n}\n")
        self.assertIn("PE_HostToGuest(s)", out)

    def test_pointer_local_to_guest_address_parameter(self):
        sigs = {"func_8008F1B0": {"ret": "void", "params": ["pe_addr_t pe_a0", "int b"]}}
        out = self.render("extern unsigned char D_800B8AC0[];\n"
                          "void func_80099990(int i) {\n    unsigned char *base;\n"
                          "    base = D_800B8AC0 + i;\n    func_8008F1B0(base, 3);\n}\n",
                          sigs=sigs, known={"func_8008F1B0"})
        self.assertIn("func_8008F1B0(PE_HostToGuest(base), 3)", out)


class PointerRelationalCompareTests(unittest.TestCase):
    """portverify finding 9."""

    def test_pointer_order_compare_is_reported(self):
        plan = analyze("char *func_80099990(char *dst, char *src, int n) {\n"
                       "    if (dst >= src) {\n        while (n-- > 0) dst[n] = src[n];\n"
                       "    } else {\n        while (n-- > 0) *dst++ = *src++;\n    }\n"
                       "    return dst;\n}\n")
        self.assertFalse(plan["eligible"])
        self.assertIn("relational compare of pointers dst >= src", plan["reason"])

    def test_same_array_loop_bound_is_kept(self):
        # func_80053128: `for (p = D_800C0EAC; p < D_800C0EAC + 0x80; p++)`
        plan = analyze("typedef struct { unsigned char b[32]; } Item;\n"
                       "extern Item D_800C0EAC[];\n"
                       "void func_80099990(void) {\n    Item *p;\n"
                       "    for (p = D_800C0EAC; p < D_800C0EAC + 0x80; p++) {\n"
                       "        p->b[5] &= ~8;\n    }\n}\n")
        self.assertTrue(plan["eligible"], plan.get("reason"))
        self.assertEqual(plan["rel_rewrites"], [])

    def test_exact_operands_compare_guest_values(self):
        # func_80051258: `while (D_800A1AA0 < D_8009D014)`, array vs pointer global.
        g._ADDR_KNOWN.clear(); g._ADDR_SIGS.clear()
        src = ("extern unsigned char *D_8009D014;\nextern unsigned char D_800A1AA0[];\n"
               "extern void func_8005112C(void);\n"
               "void func_80099990(void) {\n    while (D_800A1AA0 < D_8009D014) {\n"
               "        func_8005112C();\n    }\n}\n")
        plan = analyze(src)
        self.assertTrue(plan["eligible"], plan.get("reason"))
        leaf = {"name": "func_80099990", "file_offset": 0, "file_size": 4,
                "vram": 0x80099990, "words": 1}
        out = g.render("func_80099990", plan, {"func_80099990": leaf}, set(), {})
        self.assertIn("((pe_addr_t)0x800A1AA0u < PE_LoadU32(0x8009D014u))", out)

    def test_value_compares_are_fine(self):
        plan = analyze("int func_80099990(unsigned char *p, int n) {\n"
                       "    if (p[0] < n) return 1;\n    if (n > 3) return 2;\n"
                       "    return (p[1] << 2) > n;\n}\n")
        self.assertTrue(plan["eligible"], plan.get("reason"))


class SignednessAdapterTests(unittest.TestCase):
    """E20: canonical prototype differs only by 32-bit integer signedness."""

    def test_agreeing_signature_needs_no_adapter(self):
        canon = {"ret": "int", "params": ["int a0"]}
        self.assertIsNone(g.signedness_adapter("int ", "int a0", canon))

    def test_uint32_vs_int_adapts(self):
        canon = {"ret": "uint32_t", "params": ["uint32_t a", "uint32_t b"]}
        ad = g.signedness_adapter("int ", "int a0, int a1", canon)
        self.assertEqual(ad["params"], ["uint32_t", "uint32_t"])
        self.assertEqual(ad["leaf_params"], ["int", "int"])
        thunk = g.signedness_thunk("func_8003708C", ad)
        self.assertIn("uint32_t func_8003708C(uint32_t pe_a0, uint32_t pe_a1)", thunk)
        self.assertIn("return (uint32_t)pe_leaf_func_8003708C((int)pe_a0, (int)pe_a1);", thunk)

    def test_void_header_discards_int_result(self):
        canon = {"ret": "void", "params": ["void"]}
        ad = g.signedness_adapter("int ", "void", canon)
        thunk = g.signedness_thunk("func_80067A78", ad)
        self.assertIn("void func_80067A78(void)", thunk)
        self.assertIn("(void)pe_leaf_func_80067A78();", thunk)

    def test_int_header_over_void_leaf_rejected(self):
        # A void leaf leaves $v0 to its callee; the host caller would read an
        # invented value (the retail v0-residual class) -> not adapted.
        canon = {"ret": "int", "params": ["int a0"]}
        self.assertEqual(g.signedness_adapter("void ", "int a0", canon), "reject")

    def test_width_or_arity_change_rejected(self):
        self.assertEqual(g.signedness_adapter(
            "void ", "short a0", {"ret": "void", "params": ["uint32_t a0"]}), "reject")
        self.assertEqual(g.signedness_adapter(
            "void ", "int a0", {"ret": "void", "params": ["uint32_t s", "pe_addr_t r"]}),
            "reject")
        self.assertEqual(g.signedness_adapter(
            "long ", "int a0", {"ret": "int", "params": ["int a0"]}), "reject")

    def test_trailing_param_header_over_void_leaf_adapts(self):
        # E20b: field-VM handler header `int f(pe_addr_t args)` over the
        # matched `int f(void)` (func_800155FC); retail ignores $a0.
        canon = {"ret": "int", "params": ["pe_addr_t args"]}
        ad = g.signedness_adapter("int ", "void", canon)
        self.assertEqual(ad["dropped"], 1)
        thunk = g.signedness_thunk("func_800155FC", ad)
        self.assertIn("int func_800155FC(pe_addr_t pe_a0)", thunk)
        self.assertIn("(void)pe_a0;", thunk)
        self.assertIn("return (int)pe_leaf_func_800155FC();", thunk)
        # leading parameters kept exactly, only the tail dropped
        ad = g.signedness_adapter("void ", "int a0", {"ret": "void", "params": ["int a0", "pe_addr_t b"]})
        thunk = g.signedness_thunk("func_80001234", ad)
        self.assertIn("void func_80001234(int pe_a0, pe_addr_t pe_a1)", thunk)
        self.assertIn("pe_leaf_func_80001234((int)pe_a0);", thunk)

    def test_trailing_param_void_header_discards_int_result(self):
        # E20b + E20's void-discards-int: `void f(pe_addr_t window)` over the
        # matched `int f(void)` (func_800453E8).
        canon = {"ret": "void", "params": ["pe_addr_t window"]}
        ad = g.signedness_adapter("int ", "void", canon)
        self.assertEqual(ad["dropped"], 1)
        thunk = g.signedness_thunk("func_800453E8", ad)
        self.assertIn("void func_800453E8(pe_addr_t pe_a0)", thunk)
        self.assertIn("(void)pe_a0;", thunk)
        self.assertIn("(void)pe_leaf_func_800453E8();", thunk)

    def test_trailing_param_with_int_over_void_return_rejected(self):
        # Kept out: header returns int over a void leaf (callers would read a
        # value retail never produced), even when only trailing params differ.
        canon = {"ret": "int", "params": ["pe_addr_t node"]}
        self.assertEqual(g.signedness_adapter("void ", "void", canon), "reject")

    def test_trailing_param_with_narrow_leading_param_rejected(self):
        # Kept out: a narrower leading parameter (uint32_t header over an
        # unsigned char leaf) would truncate where retail's callee did not.
        canon = {"ret": "void", "params": ["uint32_t style", "pe_addr_t extra"]}
        self.assertEqual(g.signedness_adapter("void ", "unsigned char style", canon), "reject")

    def test_rename_definition_only(self):
        body = "int func_80001234(int a0) {\n    return func_80001234(a0 - 1);\n}\n"
        out = g.rename_definition(body, "func_80001234", "pe_leaf_func_80001234")
        self.assertTrue(out.startswith("static int pe_leaf_func_80001234(int a0) {"))
        self.assertIn("return func_80001234(a0 - 1);", out)


class NeutralizeSameLineDeclTests(unittest.TestCase):
    """A canonical-callee prototype sharing a line with another declaration."""

    def test_second_prototype_on_line_is_neutralized(self):
        # src/func_80050308.c: `extern int D_8009CF18; extern void func_8005EB64(int a);`
        body = ("extern int D_8009CF18; extern void func_8005EB64(int a);\n"
                "void func_80050308(int a0){ func_8005EB64(a0+0x7C); }\n")
        out = g.neutralize_declarations(body, {"D_8009CF18"}, set(), {"func_8005EB64"})
        self.assertNotIn("extern void func_8005EB64(int a);", out)
        self.assertIn("func_8005EB64(a0+0x7C)", out)   # the call site is untouched

    def test_statement_after_declaration_not_split(self):
        # only a trailing *prototype* is moved; a call statement is left alone
        body = "int f(void) { int x; x = 1; return func_8005EB64(x); }\n"
        self.assertEqual(g.neutralize_declarations(body, set(), set(), set()), body)


class E20OverExportTests(unittest.TestCase):
    """Class D: E20 composed with the pointer adapters' exported signature."""

    def test_ptr_return_export_gets_e20_thunk(self):
        # func_8005DADC: pointer-return adapter exports `pe_addr_t f(int a0)`,
        # the canonical header is `pe_addr_t f(uint32_t index)`.
        text = ("static unsigned char *pe_host_func_8005DADC(int a0) { return 0; }\n"
                "pe_addr_t func_8005DADC(int a0)\n{\n    return PE_HostToGuest(pe_host_func_8005DADC(a0));\n}\n")
        out = g.e20_over_export(text, "func_8005DADC",
                                {"ret": "pe_addr_t", "params": ["uint32_t index"]})
        self.assertIn("static pe_addr_t pe_leaf_func_8005DADC(int a0)", out)
        self.assertIn("pe_addr_t func_8005DADC(uint32_t pe_a0)", out)
        self.assertIn("return (pe_addr_t)pe_leaf_func_8005DADC((int)pe_a0);", out)

    def test_non_e20_difference_left_alone(self):
        text = "int func_80001234(pe_addr_t pe_arg0, int b)\n{\n    return 0;\n}\n"
        canon = {"ret": "int", "params": ["pe_addr_t a", "unsigned char b"]}   # narrow: not E20
        self.assertEqual(g.e20_over_export(text, "func_80001234", canon), text)

    def test_trailing_param_drop_never_implicit(self):
        # E20b needs a per-leaf disassembly proof: not applied through the export.
        text = "int func_80001234(pe_addr_t pe_arg0)\n{\n    return 0;\n}\n"
        canon = {"ret": "int", "params": ["pe_addr_t a", "uint32_t extra"]}
        self.assertEqual(g.e20_over_export(text, "func_80001234", canon), text)

    def test_no_canonical_header_left_alone(self):
        text = "int func_80001234(pe_addr_t pe_arg0)\n{\n    return 0;\n}\n"
        self.assertEqual(g.e20_over_export(text, "func_80001234", None), text)


class E20eTests(unittest.TestCase):
    """E20e: `return <void call>;` rewrite, only for proven leaves."""
    SIGS = {"func_80056FB8": {"ret": "void", "params": ["void"]},
            "func_80001111": {"ret": "int", "params": ["void"]}}

    def test_proven_leaf_void_callee_rewritten(self):
        body = "static int pe_leaf_func_800453E8(void) { f(); return func_80056FB8(); }"
        out = g.e20e_void_return_rewrite(body, "func_800453E8", self.SIGS)
        self.assertIn("func_80056FB8(); return 0;", out)
        self.assertNotIn("return func_80056FB8()", out)

    def test_unproven_leaf_untouched(self):
        body = "static int pe_leaf_func_80012345(void) { return func_80056FB8(); }"
        self.assertEqual(g.e20e_void_return_rewrite(body, "func_80012345", self.SIGS), body)

    def test_non_void_callee_untouched(self):
        body = "static int pe_leaf_func_800453E8(void) { return func_80001111(); }"
        self.assertEqual(g.e20e_void_return_rewrite(body, "func_800453E8", self.SIGS), body)


class E20fTests(unittest.TestCase):
    """E20f: literal arguments to a proven-empty zero-parameter callee."""
    PROTO = {"func_800527C0": 0, "func_80001111": 0, "func_80002222": 1}

    def test_literal_args_dropped(self):
        body = "void f(void) { func_800527C0(2); func_800527C0(0x1Fu); }"
        out = g.empty_callee_args_drop(body, self.PROTO)
        self.assertEqual(out.count("func_800527C0() /* E20f"), 2)
        self.assertNotIn("func_800527C0(2)", out)
        self.assertTrue(g.empty_callee_droppable("func_800527C0", "3", self.PROTO))

    def test_non_literal_args_kept(self):
        for args in ("i", "x + 1", "func_80001111()", "n++"):
            body = f"void f(void) {{ func_800527C0({args}); }}"
            self.assertEqual(g.empty_callee_args_drop(body, self.PROTO), body, args)
            self.assertFalse(g.empty_callee_droppable("func_800527C0", args, self.PROTO))

    def test_unlisted_or_nonempty_callee_kept(self):
        body = "void f(void) { func_80001111(2); }"
        self.assertEqual(g.empty_callee_args_drop(body, self.PROTO), body)
        self.assertFalse(g.empty_callee_droppable("func_80001111", "2", self.PROTO))
        # a listed callee whose canonical prototype takes arguments: not E20f
        self.assertFalse(g.empty_callee_droppable("func_800527C0", "2", {"func_800527C0": 1}))

    def test_declaration_and_bare_call_untouched(self):
        body = "extern void func_800527C0(int);\nvoid f(void) { func_800527C0(); }"
        self.assertEqual(g.empty_callee_args_drop(body, self.PROTO), body)


class RectArrayArgTests(unittest.TestCase):
    """A local `short NAME[4]` passed to a canonical RECT* parameter."""
    SIGS = {"func_8007506C": {"ret": "int", "params": ["const RECT *rect", "pe_addr_t data"]},
            "func_80001234": {"ret": "void", "params": ["pe_addr_t p"]}}

    def test_short4_to_const_rect_cast(self):
        body = "void f(void) { short buf[4]; buf[0] = 1; func_8007506C(buf, 0); }"
        out = g.rect_array_args(body, self.SIGS)
        self.assertIn("func_8007506C((const RECT*)buf, 0)", out)

    def test_non_rect_param_untouched(self):
        body = "void f(void) { short buf[4]; func_80001234(buf); }"
        self.assertEqual(g.rect_array_args(body, self.SIGS), body)

    def test_other_array_size_untouched(self):
        body = "void f(void) { short buf[8]; func_8007506C(buf, 0); }"
        self.assertEqual(g.rect_array_args(body, self.SIGS), body)


class HostToGuestArgTests(unittest.TestCase):
    """host_to_guest_args wraps host pointer locals, never call results."""

    def test_call_result_at_recorded_index_not_wrapped(self):
        # func_8004DF74: `func_8005F1A0(t)` records (func_8005F1A0, 0); the
        # later `func_8005F1A0(func_8005BEE8())` passes a guest value.
        body = ("x = func_8005F1A0(t) + 0x14;\n"
                "x = func_8005F1A0(func_8005BEE8());\n")
        out = g.host_to_guest_args(body, {"hostptr_args": [("func_8005F1A0", 0)]})
        self.assertIn("func_8005F1A0(PE_HostToGuest(t))", out)
        self.assertIn("func_8005F1A0(func_8005BEE8())", out)
        self.assertNotIn("PE_HostToGuest(func_8005BEE8())", out)


class HostTypedefTests(unittest.TestCase):
    def test_libgpu_rect_is_the_header_rect(self):
        out = g.neutralize_host_typedefs("typedef struct { short x, y, w, h; } RECT;\nint a;\n")
        self.assertNotIn("typedef", out)
        self.assertIn("psx_compat.h", out)

    def test_other_rect_layouts_stay(self):
        src = "typedef struct { int x, y, w, h; } RECT;\n"
        self.assertEqual(g.neutralize_host_typedefs(src), src)


class GuestCallArgWidthTests(unittest.TestCase):
    def test_int_args_are_32_bit_register_values(self):
        src = 'x = PE_GuestCall("@1:f", (pe_addr_t)(f), 2u, (uintptr_t)(*(int *)(p + 0xC)), (uintptr_t)(a1), 0, 0);'
        out = g.canonical_guest_call_args(src)
        self.assertIn("(uintptr_t)(uint32_t)(*(int *)(p + 0xC))", out)
        self.assertIn("(uintptr_t)(uint32_t)(a1), 0, 0)", out)
        self.assertEqual(g.canonical_guest_call_args(out), out)


if __name__ == "__main__":
    unittest.main()
