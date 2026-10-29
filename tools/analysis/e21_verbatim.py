#!/usr/bin/env python3
"""Retail differential for VERBATIM generated TUs (rooms, EXE, subsystem overlays).

e21_diff.py sweeps only E21-lowered TUs: their guest memory goes through
PE_LoadU32/PE_StoreU32, which its mock header replaces.  A verbatim TU copies
the matched leaf's body and reaches guest RAM through typed host pointers
(PE_DECOMP_SCALAR / PE_DECOMP_ARRAY / PE_Translate), so no sweep has compared
those with retail.  Here a verbatim TU is compiled exactly as production
compiles it -- the real pc_port headers, `-O3 -fno-strict-aliasing -fwrapv
-funsigned-char` -- and linked against:

  - PE_Translate / PE_HostToGuest / PE_Load* / PE_Store* over the harness RAM;
  - a logging mock for every undefined `func_X` / `room_H__func_X` /
    `ovl_N__func_X` callee, generated from its `gcc -aux-info` prototype, that
    returns e21_diff's seeded mock value and writes e21_diff's out-parameter
    bytes (a pointer argument that is not guest RAM is a host stack local: it
    is logged by its 32 bytes, as the oracle logs a retail stack pointer);
  - PE_GuestCall as the jalr it replaces;
  - the native GTE (pe_gte.c, pe_gte_commands.c at -O3).

Retail runs in e21_diff's R3000A oracle on the same seeded RAM.  RAM,
scratchpad, the call sequence and arguments, GTE registers (--check-gte) and
the return value are compared with e21_diff's rules.  A TU with a callee or
runtime symbol this harness does not model is reported `unsupported`, never
guessed.

usage: e21_verbatim.py [--set rooms,exe,ovl] [--only SUBSTR,...] [--seeds N] [--jobs J]
                       [--limit N] [--check-gte] [--verbose]
"""
from __future__ import annotations

import argparse
import concurrent.futures
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import e21_diff as e  # noqa: E402
import gen_overlay_ports as o  # noqa: E402

PC = o.REPO_ROOT / "pc_port"
INCS = [f"-I{PC / d}" for d in ("", "include", "platform", "bootstrap", "src")]
DEFS = ["-DPE_PORT_FB_HEIGHT=240", "-DPE_PORT_FB_WIDTH=320", "-DPE_PORT_HEADLESS=1"]
TU_FLAGS = ["-O3", "-fno-strict-aliasing", "-fwrapv", "-std=gnu11", "-funsigned-char", "-w"]
# --clamp-sign-split: give a callee whose host prototype flips the leaf's
# 32-bit return signedness bit-31-clear mock returns (where both types agree).
# Off by default: since E22 (gen_decomp_ports.cast_leaf_call_returns) the TU
# casts those results back to the leaf's type, and unclamped returns test that.
CLAMP_SIGN_SPLIT = os.environ.get("E21V_CLAMP_SIGN_SPLIT") == "1"
SHIMS: dict[str, int] = {}             # filled in main() from the headers (shim_map)
CALLEE = re.compile(r"(?:room_[0-9a-f]{8}__|ovl_\w+?__)?func_([0-9A-F]{8})")
# provided by the harness runtime below, libc, or the GTE objects
RUNTIME = {"PE_Translate", "PE_TranslateConst", "PE_HostToGuest", "PE_LoadU8", "PE_LoadU16",
           "PE_LoadU32", "PE_StoreU8", "PE_StoreU16", "PE_StoreU32", "PE_Fill", "PE_GuestCall",
           "memcpy", "memset", "memmove", "__stack_chk_fail", "abort", "printf", "puts", "fprintf"}

RUNTIME_C = r"""
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t pe_addr_t;
uint8_t ram[0x200400];
static uint8_t *at_(pe_addr_t a, size_t n)
{
    if ((a & 0x1FFFFC00u) == 0x1F800000u && (a & 0x3FFu) + n <= 0x400u) return ram + 0x200000u + (a & 0x3FFu);
    if ((a & 0x1FFFFFFFu) + (n ? n : 1u) > 0x200000u) { printf("FAULT %08X\n", a); fflush(stdout); exit(3); }
    return ram + (a & 0x1FFFFFFFu);
}
void *PE_Translate(pe_addr_t a, size_t n) { return at_(a, n); }
const void *PE_TranslateConst(pe_addr_t a, size_t n) { return at_(a, n); }
pe_addr_t PE_HostToGuest(const volatile void *h)
{
    const uint8_t *p = (const uint8_t *)(uintptr_t)h;
    if (!p) return 0;
    if (p >= ram && p < ram + 0x200000u) return 0x80000000u + (pe_addr_t)(p - ram);
    if (p >= ram + 0x200000u && p < ram + 0x200400u) return 0x1F800000u + (pe_addr_t)(p - ram - 0x200000u);
    printf("HOSTPTR\n"); fflush(stdout); exit(5);
}
uint8_t  PE_LoadU8(pe_addr_t a)  { return *at_(a, 1); }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; memcpy(&v, at_(a, 2), 2); return v; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; memcpy(&v, at_(a, 4), 4); return v; }
void PE_StoreU8(pe_addr_t a, uint8_t v)   { *at_(a, 1) = v; }
void PE_StoreU16(pe_addr_t a, uint16_t v) { memcpy(at_(a, 2), &v, 2); }
void PE_StoreU32(pe_addr_t a, uint32_t v) { memcpy(at_(a, 4), &v, 4); }
void PE_Fill(pe_addr_t a, uint32_t n, uint8_t v) { memset(at_(a, n), v, n); }
void PE_Port_RequestStop(const char *r) { (void)r; }
unsigned seed_;
FILE *logf_;
uint32_t rov_fn_[32], rov_v_[32];
unsigned rov_n_;
static const uint32_t *outs_; static const unsigned *outl_; static unsigned nouts_;
void vh_set_outs(const uint32_t *k, const unsigned *l, unsigned n) { outs_ = k; outl_ = l; nouts_ = n; }
static int rov_get(uint32_t fn, uint32_t *v)
{
    for (unsigned k = 0; k < rov_n_; k++) if (rov_fn_[k] == fn) { *v = rov_v_[k]; return 1; }
    return 0;
}
/* one argument: a guest value, a guest-RAM host pointer (its guest address),
   or a host pointer elsewhere (a host stack local: logged by content) */
void vh_arg(uint32_t fn, unsigned k, int is_ptr, uint64_t v)
{
    if (!is_ptr) { fprintf(logf_, " %08X", (uint32_t)v); return; }
    const uint8_t *p = (const uint8_t *)(uintptr_t)v;
    if (!p) { fprintf(logf_, " 00000000"); return; }
    if (p >= ram && p < ram + 0x200400u) { fprintf(logf_, " %08X", PE_HostToGuest(p)); return; }
    fprintf(logf_, " S:00000000:");
    for (unsigned j = 0; j < 32; j++) fprintf(logf_, "%02X", p[j]);
    (void)fn; (void)k;
}
/* a mocked callee's out-parameter bytes, as e21_diff.out_bytes writes them */
void vh_out(uint32_t fn, unsigned k, uint64_t v)
{
    uint8_t *p = (uint8_t *)(uintptr_t)v;
    if (!p || (p >= ram && p < ram + 0x200400u)) return;    /* only host stack locals */
    for (unsigned q = 0; q < nouts_; q++)
        if (outs_[q] == (fn << 4 | k)) {
            uint32_t x = fn * 2654435761u + seed_ * 40503u + k * 0x9E37u;
            for (unsigned j = 0; j < outl_[q]; j++) { x = x * 1103515245u + 12345u; p[j] = (uint8_t)(x >> 16); }
        }
}
uint32_t vh_ret(uint32_t fn)
{
    uint32_t v, h = fn * 2654435761u + seed_ * 40503u;
    if (rov_get(fn, &v)) return v;
    if (((seed_ * 2654435761u) >> 16) & 1u) return (uint32_t)((int32_t)(h & 0x1FFFu) - 0x1000);
    return 0x80100000u + (h & 0xDFFFCu);
}
void *vh_ptr(uint32_t v)
{
    if (!v) return 0;
    if ((v & 0x1FFFFC00u) == 0x1F800000u || (v & 0x1FFFFFFFu) < 0x200000u) return at_(v, 1);
    return (void *)(uintptr_t)0x10;          /* not guest memory: a deref faults, as retail's would */
}
int PE_GuestCall(const char *s, pe_addr_t fn, unsigned n, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    uint32_t a[4] = {(uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3};
    (void)s;
    fprintf(logf_, "C %08X", fn);
    for (unsigned i = 0; i < n && i < 4; i++) fprintf(logf_, " %08X", a[i]);
    fprintf(logf_, "\n");
    return (int)vh_ret(fn);
}
"""


SHIM_RE = re.compile(r"static\s+inline\s+[\w\s\*]+?\b(func_([0-9A-F]{8}))\s*\([^)]*\)\s*"
                     r"\{\s*(?:return\s+)?(?:\([^)]*\)\s*)?([A-Za-z_]\w*)\s*\(")


def shim_map() -> dict[str, int]:
    """Host function -> retail address, from the headers' one-line static
    inline wrappers (psx_compat.h `func_80073A44(m) { return HostFB_VSync(m); }`):
    a TU that calls the wrapper links against the host function."""
    out: dict[str, int] = {}
    for h in sorted((PC / "include").glob("*.h")) + sorted((PC / "platform").glob("*.h")):
        for m in SHIM_RE.finditer(h.read_text(errors="replace")):
            if not m.group(3).startswith("func_"):
                out.setdefault(m.group(3), int(m.group(2), 16))
    return out


def tu_typedefs(text: str) -> str:
    """The TU's own file-scope typedefs, in order (the harness declares the
    mocks with the same types the TU sees)."""
    out, i, n = [], 0, len(text)
    body = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), text, flags=re.S)
    for m in re.finditer(r"(?m)^typedef\b", body):
        depth, j = 0, m.start()
        while j < n:
            c = body[j]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
            elif c == ";" and depth == 0:
                break
            j += 1
        out.append(text[m.start():j + 1])
    return "\n".join(out) + "\n"


def split_params(s: str) -> list[str]:
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return [] if out in ([], ["void"]) else out


def aux_info(cc: str, tu: Path, d: Path) -> dict[str, tuple[str, list[str], bool]]:
    """name -> (return type, parameter types, is_definition) from gcc -aux-info."""
    aux = d / "aux.txt"
    subprocess.run([cc, *DEFS, *INCS, "-std=gnu11", "-funsigned-char", "-w", "-fsyntax-only",
                    "-aux-info", str(aux), str(tu)], capture_output=True, text=True)
    out: dict = {}
    if not aux.exists():
        return out
    for ln in aux.read_text(errors="replace").splitlines():
        m = re.match(r"/\* (.*?):(\d+):(\w+) \*/ (.*?);", ln)
        if not m:
            continue
        decl, kind = m.group(4).strip(), m.group(3)
        mm = re.match(r"^(?:extern\s+|static\s+)*(.*?)\b(\w+)\s*\((.*)\)$", decl)
        if not mm:
            continue
        ret, name, prm = mm.group(1).strip(), mm.group(2), mm.group(3)
        types = []
        for p in split_params(prm):
            if "(*" in p.replace(" ", ""):
                types.append(p)                      # function pointer: kept, flagged later
                continue
            if "F" in kind and e.param_is_named(p):
                p = re.sub(r"\s*\b\w+\s*$", "", p) if not p.endswith("*") else p
            types.append(p.strip())
        if name not in out or "F" in kind:
            out[name] = (ret, types, "F" in kind)
    return out


SIGNED32 = {"int", "signed int", "int32_t", "long", "signed"}
UNSIGNED32 = {"unsigned int", "unsigned", "uint32_t", "pe_addr_t", "unsigned long"}


def leaf_decl_rets(tu_text: str) -> dict[str, str]:
    """The matched leaf's own `extern T func_X(...)` return types (the TU's
    `from src/...c` source), which the generator replaced by host prototypes."""
    m = re.search(r"from (src/\S+\.c)", tu_text)
    src = o.REPO_ROOT / m.group(1) if m else None
    if not src or not src.exists():
        return {}
    return {d.group(2): " ".join(d.group(1).split())
            for d in re.finditer(r"^\s*extern\s+([\w\s\*]+?)\s*\b(func_[0-9A-F]{8})\s*\(",
                                 src.read_text(errors="replace"), re.M)}


def sign_split(leaf_t: str, host_t: str) -> bool:
    """A 32-bit return type whose signedness the host prototype changed: the
    caller's comparisons differ for values with bit 31 set."""
    a, b = " ".join(leaf_t.split()), " ".join(host_t.split())
    return (a in SIGNED32 and b in UNSIGNED32) or (a in UNSIGNED32 and b in SIGNED32)


def mock_source(name: str, address: int, ret: str, types: list[str]) -> str:
    ps = ", ".join(f"{t} p{k}" for k, t in enumerate(types)) or "void"
    logs = "".join(f" vh_arg(0x{address:08X}u, {k}u, {int('*' in t)}, (uint64_t)(uintptr_t)p{k});"
                   for k, t in enumerate(types))
    outs = "".join(f" vh_out(0x{address:08X}u, {k}u, (uint64_t)(uintptr_t)p{k});"
                   for k, t in enumerate(types) if "*" in t)
    if ret.strip() == "void":
        tail = " (void)v_;"
    elif "*" in ret:
        tail = f" return ({ret})vh_ptr(v_);"
    else:
        tail = f" return ({ret})v_;"
    return (f"{ret} {name}({ps}) {{ uint32_t v_; fprintf(logf_, \"C %08X\", 0x{address:08X}u);{logs}"
            f" fprintf(logf_, \"\\n\");{outs} v_ = vh_ret(0x{address:08X}u);{tail} }}\n")


def one(tu: Path, seeds: int, cc: str, check_gte: bool, specs: dict) -> tuple[str, str]:
    t = tu.read_text(errors="replace")
    if "E21 guest-memory" in t:
        return "not-verbatim", ""
    m0 = re.search(r"decomp-source-overlay: ((?:room|ovl)_\w+)/(func_[0-9A-F]{8})", t)
    m1 = re.search(r"decomp-source: (func_[0-9A-F]{8})\b", t)
    if m0:
        ovl, leafname = m0.group(1), m0.group(2)
        ov = o.parse_overlay(ovl)
        blob = o.blob_path(ovl)
    elif m1:                                   # an EXE leaf: the SLUS image as a pseudo-overlay
        ovl, leafname = "exe", m1.group(1)
        ov, blob = e.exe_overlay(), o.REPO_ROOT / e.EXE_IMAGE
        if not blob.is_file():
            return "noblob", ""
    else:
        return "unsupported", "no decomp-source header"
    if blob is None or leafname not in ov["leaves"]:
        return "noblob", ""
    leaf = ov["leaves"][leafname]
    data = blob.read_bytes()
    with tempfile.TemporaryDirectory() as dd:
        d = Path(dd)
        aux = aux_info(cc, tu, d)
        obj = d / "tu.o"
        r = subprocess.run([cc, *DEFS, *INCS, *TU_FLAGS, "-c", str(tu), "-o", str(obj)],
                           capture_output=True, text=True)
        if r.returncode:
            return "nocompile", r.stderr[:300]
        nm_d = subprocess.run(["nm", "--defined-only", str(obj)], capture_output=True, text=True).stdout
        nm_u = subprocess.run(["nm", "-u", str(obj)], capture_output=True, text=True).stdout
        defined = [ln.split()[-1] for ln in nm_d.splitlines() if " T " in ln]
        entry = next((s for s in defined if s.endswith(leafname)), None)
        if entry is None or entry not in aux:
            return "unsupported", f"no exported {leafname}"
        ret_t, ptypes, _ = aux[entry]
        if any("(*" in p.replace(" ", "") for p in ptypes) or "*" in ret_t:
            return "unsupported", f"entry signature {ret_t} {entry}({', '.join(ptypes)})"
        mocks, masks, ret_types, split = [], {}, {}, {}
        leaf_rets = leaf_decl_rets(t)
        for u in sorted({ln.split()[-1] for ln in nm_u.splitlines() if ln.strip()}):
            if u in RUNTIME or u.startswith("PE_GTE_") or u.startswith("pe_gte"):
                continue
            mm = CALLEE.fullmatch(u)
            if (not mm and u not in SHIMS) or u not in aux:
                return "unsupported", f"callee {u}"
            ret, types, _ = aux[u]
            if any("(*" in x.replace(" ", "") for x in types):
                return "unsupported", f"callee {u} takes a function pointer"
            if len(types) > 16:
                return "unsupported", f"callee {u}: {len(types)} arguments"
            addr = int(mm.group(1), 16) if mm else SHIMS[u]
            lt = leaf_rets.get(f"func_{addr:08X}")
            if lt and sign_split(lt, ret):
                split[addr] = f"func_{addr:08X} (leaf {lt}, host {ret})"
            mocks.append(mock_source(u, addr, ret, types))
            masks[addr] = [0xFFFFFFFF if "*" in x else e.narrow_mask(x) for x in types]
            if "*" not in ret and ret.strip() != "void":
                ret_types[addr] = ret
        # directed inputs and out= extents, as e21_diff gives this leaf
        dspec = specs.get((ovl, leafname)) or []
        if dspec and not isinstance(dspec[0], list):
            dspec = [dspec]
        outs = frozenset(v for sp in dspec for lhs, vs in sp if lhs == "out" for v in vs)
        keyed = {e.out_key(v) for v in outs}
        outs = outs | frozenset(fn << 4 | k for (fn, k) in e.OUT_EXTENTS if (fn << 4 | k) not in keyed)
        outs = outs | frozenset((fn << 4 | k) | (n << e.OUT_LEN_SHIFT)
                                for (fn, k), n in e.OUT_EXTENTS_OVL.get(ovl, {}).items()
                                if (fn << 4 | k) not in keyed)
        okeys = sorted(outs)
        nparams = len(ptypes)
        regs = ["a0", "a1", "a2", "a3"] + [f"PE_LoadU32(0x{e.SP0 + 16 + 4 * k:08X}u)" for k in range(8)]
        argl = ", ".join(f"({ptypes[k]}){regs[k]}" for k in range(nparams))
        if ret_t.strip() == "void":
            call = f"{entry}({argl}); fprintf(logf_, \"R -\\n\");"
        else:
            call = (f"uint32_t rv = (uint32_t)(int32_t){entry}({argl}); "
                    f"fprintf(logf_, \"R %08X\\n\", rv);")
        main = ('#include "pe_guest_decomp.h"\n' + tu_typedefs(t) + RUNTIME_C
                + "".join(mocks)
                + f"{ret_t} {entry}({', '.join(ptypes) or 'void'});\n"
                + "uint32_t PE_GTE_ReadData(uint32_t reg);\nuint32_t PE_GTE_ReadCtrl(uint32_t reg);\n"
                + "static const uint32_t ok_[] = {" + (", ".join(f"0x{e.out_key(v) & 0xFFFFFFFF:X}u" for v in okeys) or "0xFFFFFFFFu") + "};\n"
                + "static const unsigned ol_[] = {" + (", ".join(f"{e.out_len(v)}u" for v in okeys) or "0u") + "};\n"
                + r"""
int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb");
    uint32_t a0 = strtoul(argv[3], 0, 16), a1 = strtoul(argv[4], 0, 16),
             a2 = strtoul(argv[5], 0, 16), a3 = strtoul(argv[6], 0, 16);
    seed_ = (unsigned)strtoul(argv[7], 0, 10);
    vh_set_outs(ok_, ol_, sizeof ok_ / sizeof ok_[0]);
    for (int k = 9; k < argc && rov_n_ < 32; k++) {
        char *eq = strchr(argv[k], '=');
        if (!eq) continue;
        rov_fn_[rov_n_] = (uint32_t)strtoul(argv[k], 0, 16);
        rov_v_[rov_n_++] = (uint32_t)strtoul(eq + 1, 0, 16);
    }
    if (!f || fread(ram, 1, sizeof ram, f) != sizeof ram) return 2;
    fclose(f);
    logf_ = fopen(argv[8], "w");
    (void)a0; (void)a1; (void)a2; (void)a3;
    CALL_LINE
    fprintf(logf_, "G");
    for (unsigned i = 0; i < 32; i++) fprintf(logf_, " %08X", PE_GTE_ReadData(i));
    for (unsigned i = 0; i < 32; i++) fprintf(logf_, " %08X", PE_GTE_ReadCtrl(i));
    fprintf(logf_, "\n");
    fclose(logf_);
    f = fopen(argv[2], "wb"); fwrite(ram, 1, sizeof ram, f); fclose(f);
    return 0;
}
""".replace("CALL_LINE", call))
        (d / "h.c").write_text(main)
        # The native GTE, built as production builds it (plain -O3).  A callee
        # this TU mocks may also be defined there (func_80079004 in pe_gte.c):
        # the unit's copy is renamed so the logging mock is the one linked.
        names ={re.search(r"\b(\w+)\(", m).group(1) for m in mocks}
        gte = []
        for unit in ("pe_gte.c", "pe_gte_commands.c"):
            src = PC / "platform" / unit
            if not src.exists():
                continue
            text = src.read_text(errors="replace")
            ren = [f"-D{n}=pe_harness_{n}" for n in sorted(names) if re.search(r"\b" + n + r"\s*\(", text)]
            ob = d / (unit[:-2] + ".o")
            rr = subprocess.run([cc, *DEFS, *INCS, "-O3", "-std=gnu11", "-w", *ren, "-c", str(src),
                                 "-o", str(ob)], capture_output=True, text=True)
            if rr.returncode:
                return "nocompile", f"{unit}: {rr.stderr[:200]}"
            gte.append(str(ob))
        r = subprocess.run([cc, *DEFS, *INCS, "-O1", "-std=gnu11", "-funsigned-char", "-w", "-o", str(d / "t"),
                            str(d / "h.c"), str(obj), *gte],
                           capture_output=True, text=True)
        if r.returncode:
            miss = re.findall(r"undefined reference to `(\w+)'", r.stderr)
            return "unsupported", ("link: " + " ".join(sorted(set(miss))[:6])) if miss else r.stderr[:300]
        ran = 0
        for seed, ram, args, rets in e.inputs(leafname, ov, data, seeds, "", dspec or None):
            # A callee whose return signedness the host prototype changed: its
            # mock return (constant per seed) keeps bit 31 clear, the range in
            # which the leaf's and the host's types agree.  Listed in the result.
            for a_ in (split if CLAMP_SIGN_SPLIT else ()):
                rets.setdefault(a_, e.mock_ret(a_, seed) & 0x7FFFFFFF)
            r_ram = bytearray(ram)
            try:
                gte_state = []
                v0, calls = e.run_mips(r_ram, leaf["vram"], list(args), seed, outs,
                                       gte_state if check_gte else None, ret_types=ret_types,
                                       ret_override=rets)
            except e.Fault as exc:
                if os.environ.get("E21_FAULTS"):       # why a seed was dropped
                    print(f"e21_verbatim: {leafname} seed {seed}: oracle fault: {exc}", file=sys.stderr)
                continue
            (d / "in.bin").write_bytes(ram)
            r = subprocess.run([str(d / "t"), str(d / "in.bin"), str(d / "out.bin"),
                                *[f"{a:08X}" for a in args], str(seed), str(d / "log"),
                                *[f"{a:08X}={e.narrow_ret(v, ret_types.get(a)):08X}" for a, v in rets.items()]],
                               capture_output=True, text=True, timeout=30)
            if r.returncode:
                return "MISMATCH", f"seed {seed}: native exit {r.returncode} {r.stdout[:80]}"
            n_ram = (d / "out.bin").read_bytes()
            lo, hi = e.EXCLUDE[0] & 0x1FFFFFFF, e.EXCLUDE[1] & 0x1FFFFFFF
            if r_ram[:lo] != n_ram[:lo]:
                i = next(i for i in range(lo) if r_ram[i] != n_ram[i])
                return "MISMATCH", (f"seed {seed}: RAM 0x{0x80000000 + i:08X} retail "
                                    f"{r_ram[i]:02X} native {n_ram[i]:02X}")
            if r_ram[e.RAM_SIZE:] != n_ram[e.RAM_SIZE:]:
                i = next(i for i in range(e.SPAD) if r_ram[e.RAM_SIZE + i] != n_ram[e.RAM_SIZE + i])
                return "MISMATCH", f"seed {seed}: scratchpad 0x{0x1F800000 + i:08X}"
            log = (d / "log").read_text().split("\n")
            if check_gte:
                native_gte = [int(v, 16) for v in next(l for l in log if l.startswith("G ")).split()[1:]]
                if native_gte != gte_state:
                    i = next(i for i in range(64) if native_gte[i] != gte_state[i])
                    return "MISMATCH", (f"seed {seed}: GTE {'data' if i < 32 else 'ctrl'} {i % 32} "
                                        f"retail {gte_state[i]:08X} native {native_gte[i]:08X}")
            ncalls = [l for l in log if l.startswith("C ")]
            if len(ncalls) != len(calls):
                rt_ = [f"{c[0]:08X}" for c in calls]
                nt_ = [l.split()[1] for l in ncalls]
                k_ = next((i for i in range(min(len(rt_), len(nt_))) if rt_[i] != nt_[i].upper()),
                          min(len(rt_), len(nt_)))
                return "MISMATCH", (f"seed {seed}: {len(calls)} retail calls vs {len(ncalls)} native "
                                    f"(first difference at call {k_}: retail {' '.join(rt_[k_:k_ + 4])} | "
                                    f"native {' '.join(nt_[k_:k_ + 4])})")
            for (tgt, regs_), nl in zip(calls, ncalls):
                parts = nl.split()[1:]
                if int(parts[0], 16) != tgt:
                    return "MISMATCH", f"seed {seed}: call {tgt:08X} vs {parts[0]}"
                mk = {**masks, **e.TTY_ONLY_ARGS}.get(tgt, [])
                for k, pv in enumerate(parts[1:]):
                    mm_ = mk[k] if k < len(mk) else 0xFFFFFFFF
                    rv_ = regs_[k]
                    if isinstance(rv_, tuple) and not pv.startswith("S:"):
                        rv_ = rv_[3]
                    if pv.startswith("S:") and not isinstance(rv_, tuple):
                        pv = pv.split(":")[1]
                    if isinstance(rv_, tuple) or pv.startswith("S:"):
                        nb = bytes.fromhex(pv.split(":")[2]) if pv.startswith("S:") else b""
                        extent = e.STACK_EXTENTS_OVL.get(ovl, {}).get((tgt, k), e.STACK_EXTENTS.get((tgt, k), 16))
                        if not (isinstance(rv_, tuple) and len(nb) == 32 and
                                all(nb[j] == rv_[1][j] for j in range(extent) if rv_[2][j])):
                            return "MISMATCH", (f"seed {seed}: call {tgt:08X} a{k} stack arg "
                                                f"{rv_[1].hex() if isinstance(rv_, tuple) else hex(rv_)} vs {pv}")
                        continue
                    if int(pv, 16) & mm_ != rv_ & mm_:
                        return "MISMATCH", f"seed {seed}: call {tgt:08X} a{k} {rv_:08X} vs {pv}"
            rl = [l for l in log if l.startswith("R ")][0].split()[1]
            if rl != "-":
                nv = int(rl, 16)
                rty = ret_t.strip()
                if rty in ("short", "signed short", "unsigned short", "char", "signed char", "unsigned char",
                           "int16_t", "uint16_t", "int8_t", "uint8_t", "short int", "short unsigned int",
                           "unsigned short int"):
                    ext = e.narrow_ret(v0, rty)
                    if ext != v0:
                        return "MISMATCH", f"seed {seed}: retail v0 {v0:08X} is not a {rty}"
                    if nv != ext:
                        return "MISMATCH", f"seed {seed}: v0 {v0:08X} vs {rl}"
                elif nv != v0:
                    return "MISMATCH", f"seed {seed}: v0 {v0:08X} vs {rl}"
            ran += 1
        note = ("; bit-31-clear returns for " + ", ".join(sorted(split.values()))) if split and CLAMP_SIGN_SPLIT else ""
        return ("ok" if ran else "skipped"), f"{ran} seeds{note}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seeds", type=int, default=6)
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--only", default="", help="comma-separated substrings of the TU file name")
    ap.add_argument("--check-gte", action="store_true")
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--set", default="rooms", help="comma list of rooms, exe, ovl (generated TU trees)")
    ap.add_argument("--clamp-sign-split", action="store_true",
                    help="bit-31-clear returns for callees whose host prototype flips the leaf's signedness")
    a = ap.parse_args()
    if a.clamp_sign_split:
        os.environ["E21V_CLAMP_SIGN_SPLIT"] = "1"      # read by the worker processes
        global CLAMP_SIGN_SPLIT
        CLAMP_SIGN_SPLIT = True
    cc = shutil.which("cc") or shutil.which("gcc")
    roots = {"rooms": [o.REPO_ROOT / o.ROOMS_DIR], "exe": [o.REPO_ROOT / "pc_port/game/decomp"],
             "ovl": sorted((o.REPO_ROOT / o.OUT_ROOT).glob("ovl_*"))}
    tus = [p for k in a.set.split(",") for r in roots[k] for p in sorted(r.glob("*_port.c"))
           if "E21 guest-memory" not in p.read_text(errors="replace")]
    if a.only:
        only = [s for s in a.only.split(",") if s]
        tus = [p for p in tus if any(s in p.name for s in only)]
    if a.limit:
        tus = tus[: a.limit]
    specs = e.directed_specs()
    SHIMS.update(shim_map())
    res: dict[str, list] = {}
    with concurrent.futures.ProcessPoolExecutor(max_workers=max(1, a.jobs)) as ex:
        futs = {ex.submit(one, p, a.seeds, cc, a.check_gte, specs): p.name for p in tus}
        for f in concurrent.futures.as_completed(futs):
            try:
                st, why = f.result()
            except Exception as exc:              # a harness failure is reported, not hidden
                st, why = "error", repr(exc)[:300]
            res.setdefault(st, []).append((futs[f], why))
    for st in sorted(res):
        print(f"e21_verbatim: {st}: {len(res[st])}")
        if st in ("MISMATCH", "nocompile", "error") or a.verbose:
            for nm, why in sorted(res[st]):
                print(f"  {st} {nm}: {why}")
    return 1 if res.get("MISMATCH") or res.get("error") else 0


if __name__ == "__main__":
    raise SystemExit(main())
