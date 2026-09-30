#!/usr/bin/env python3
"""gen_decomp_ports.py — derive pc_port TUs verbatim from matching src/ leaves.

Mechanism (documented in docs/ai_context/PC_PORT_FROM_DECOMP.md): the verified
matching C leaf at `src/func_XXXXXXXX.c` is the *authority*.  This tool copies
its body **verbatim** into

    pc_port/game/decomp/func_XXXXXXXX_port.c

and emits only the mechanical host adaptation around it:

  * a provenance banner naming the matched leaf, its VMA/span, word count and
    evidence report;
  * `#include "pe_guest_decomp.h"`;
  * for every `extern <type> D_XXXXXXXX;` / `extern <type> D_XXXXXXXX[];` in the
    leaf, a `#define D_XXXXXXXX ...` expander chosen by the declaration shape
    (scalar lvalue / guest array base / pointer-global);
  * for every callee of the leaf that pc_port does not already implement, a
    loud-boundary macro returning 0 and recording the symbol;
  * the leaf's declarations and body, copied byte-for-byte otherwise.

`src/` is never written to and never needs to be.  Re-running the tool after
matching advances refreshes the port set; `docs/ai_context/PC_PORT_FROM_DECOMP.md`
records the per-leaf recipe and the eligibility contract.

Eligibility (conservative by construction; a rejected leaf is reported in
`--check`, never guessed):

  E1  the leaf is a matched `c` span in configs/USA/disc1.yaml;
  E2  no `asm`/`__asm__` (era register pins / scheduler fences);
  E3  no MMIO / scratchpad / BIOS-space 32-bit literal (0x1F8xxxxx, 0x1F80xxxx,
      0xA0xxxxxx, 0xB0xxxxxx); ordinary masks like 0xFFFEFFFF stay verbatim;
  E4  exactly one file-scope `func_*` definition (no collisions);
  E5  no indirect call through a local pointer;
  E6  every `func_*` referenced is a known pc_port definition (boundable
      callee or linkable descriptor);
  E7  no pointer-typed parameter (a guest pointer held in a host pointer is
      not 32-bit; those leaves need a hand-written adapter, so they are
      reported rather than silently mis-ported);
  E8  no typed pointer cast whose operand does not derive from a `D_` data
      symbol (value parameters used as guest addresses are not representable);
  E9  no `&func_*` (function-pointer values are not host-compatible) —
      except the two shapes `rewrite_code_addresses` turns into the retail
      VMA: an integer cast `(unsigned int)func_X` and a bare `func_X` call
      argument whose host parameter takes a guest address;
      (pointer *returns*: an object-pointer return is adapted by the
      pointer-return adapter — `static T *pe_host_func_X` + an exported
      `pe_addr_t func_X` returning PE_HostToGuest(); see
      PC_PORT_FROM_DECOMP.md E7);
  E10 no store through a pointer-pointer cast of a `D_` symbol (would write a
      host pointer into guest RAM);
  E11 every called `func_*` with a canonical pc_port prototype is called with
      that prototype's arity;
  E12 no assignment to a pointer-global slot;
  E13 no `func_*` call result bound to a pointer object.  A retail callee that
      returns `int *` returns a 32-bit *guest* address; host-side that callee
      is either a canonical `pe_port_compat.h` prototype returning `pe_addr_t`
      or a loud boundary returning `int`, so binding the result to a host
      pointer is not representable (it is the same class as E7, one call
      deeper).  Rejected unless the callee's resolved host return type is
      itself a pointer;
  E14 no host-pointer expression passed to a callee parameter that is not a
      host pointer.  A local aggregate (`char buf[8]`), a pointer local, or
      `&local` is host stack memory; a canonical prototype taking `pe_addr_t`
      wants a guest address.  There is no guest frame for a generated leaf, so
      such a leaf needs a hand-written adapter and is reported, not guessed.
      (A bare `D_` symbol argument is *not* this case: `fix_argument_addresses`
      already substitutes the retail guest address.  A pointer *parameter* is
      not either: the E7b wrapper forwards it as the guest address `pe_<name>`.)
  E15 no guest array/global *of pointers* (`extern T *D_XXXX[];`,
      `extern T **D_XXXX;`).  The slot is a 32-bit guest address; typing the
      element as a host pointer would read 8 bytes on a 64-bit host.  One
      level (`extern T *D_XXXX;`) is fine — PE_DECOMP_PTRGLOBAL loads the
      32-bit slot and translates it;
  E16 no host pointer narrowed to an integer (`(unsigned int)D_XXXX`,
      `(int)ptr_param`).  Retail's value is a 32-bit guest address; the host
      accessor is a 64-bit pointer, so the cast truncates it silently.

Later additions (see docs/ai_context/PC_PORT_FROM_DECOMP.md §3):
  E2b era register pins and empty-template asm fences are removed (no
      semantics); a pin read before it is written, or any real instruction,
      is still rejected;
  E7c `T **args` guest-pointer-vector parameters whose every use is a
      dereferenced element become PE_DECOMP_PTRGLOBAL(pe_args + 4u*(k), T);
  generated TUs' host definitions act as canonical prototypes for their
      callers (derived_signatures), and whole call arguments `&D_X`,
      `&D_X[k]`, `D_X + k` become retail guest addresses where the callee's
      host parameter takes one;
  E18 no leaf struct with pointer members (guest layout has 4-byte pointers).
  RECT a leaf-local `typedef struct { short x, y, w, h; } RECT;` is the
      psx_compat.h RECT (identical layout) and is replaced by a comment.
  E20 a leaf whose own canonical pc_port header prototype differs from the
      matched definition only by 32-bit integer signedness (int / unsigned /
      int32_t / uint32_t / pe_addr_t), or returns `void` where the leaf
      returns such an integer, keeps its verbatim body as a static
      `pe_leaf_func_X` behind an exported thunk with the header signature
      (same-width casts: identity on the retail register value).
      E20b: the header may also carry extra TRAILING parameters the leaf
      never takes (exactly matching leading parameters; identical return
      type, or a `void` header over a 32-bit result); the thunk drops them.
  Unit tests: test_gen_decomp_ports.py.

E13-E16 all describe the same underlying fact from different angles: a 32-bit
guest address and a 64-bit host pointer are not the same object, so one must
never be substituted for the other.  E15/E16 matter most, because those two
shapes *compile* (with only a warning) and are silently wrong at run time.

Final gate (E17, the one that cannot be fooled): every rendered TU is
syntax-checked with the host C compiler, using pc_port's own include paths and
defines, before it is written.  A leaf whose TU does not compile is reported
and not emitted, so `pc_port/game/decomp/` is always buildable even when a new
matched leaf has a shape no static rule anticipated.  `--no-compile-check`
disables it; a host with no C compiler is warned about, not silently skipped.

Pointer-parameter adaptation (E7b, opt-in by construction): a leaf whose only
pointer parameters are address-valued (level-1 primitive or `void`) is ported
with a **verbatim body** behind a thin host adapter.  The adapter signature
replaces each `T *p` parameter with `pe_addr_t p` (a guest address), derives a
translated host pointer `host$p = (T *)PE_Translate(p, 1)`, and the verbatim
body is rewritten mechanically: `p` -> `host$p`, a whole-word `*host$p` -> the
dereference `host$p`, and the generated `D_` accessor forms are unwrapped.  No
control flow, operand order, constant, or type is invented; only the address
convention and the load/store primitive are adapted.  Leaves that pass a
pointer through to a callee, store a host pointer into guest RAM (`**`), or use
a non-primitive pointee are reported, not guessed.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
CONFIG = Path("configs/USA/disc1.yaml")
SRC_DIR = Path("src")
OUT_DIR = Path("pc_port/game/decomp")
EVIDENCE_DIR = Path("docs/evidence")

LOAD_VRAM = 0x80010000
HEADER_BYTES = 0x800
RAM_LO, RAM_HI = 0x80000000, 0x80200000

C_SPAN_RE = re.compile(r"-\s*\[(0x[0-9A-Fa-f]+),\s*c,\s*(func_[0-9A-Fa-f]{8})\]")
EDGE_RE = re.compile(r"^[ \t]*-[ \t]*\[(0x[0-9A-Fa-f]+)", re.MULTILINE)
ASM_RE = re.compile(r"__asm__|\basm\b")
# E2b — era codegen hints with no semantics.  A register pin
# `register T x asm("$N")` only fixes retail's register allocation, and an
# empty-template asm (`asm volatile("" : "=r"(x) : "0"(x))`, `asm("" ::: "memory")`)
# only hides a value/memory from cc1's optimiser; neither computes anything.
# Exactly these two forms are removed; any asm with a non-empty template (a
# real instruction) is still rejected by E2.
ASM_PIN_RE = re.compile(r"(\bregister\b[^;{}()]*?)\s*(?:__asm__|asm)\s*\(\s*\"\$[0-9A-Za-z]+\"\s*\)")
ASM_FENCE_RE = re.compile(
    r"(?:__asm__|\basm)\s*(?:__volatile__|volatile)?\s*"
    r"\(\s*\"\"\s*(?::[^;()]*(?:\([^;()]*\)[^;()]*)*)?\)\s*;")


def strip_era_asm(text: str) -> str:
    """Remove E2b register pins and empty-template asm fences (see above)."""
    text = ASM_PIN_RE.sub(r"\1", text)
    return ASM_FENCE_RE.sub("/* era asm fence removed (E2b) */", text)
NUM_RE = re.compile(r"0x[0-9A-Fa-f]{8}\b")
FUNC_NAME_RE = re.compile(r"\b(func_[0-9A-Fa-f]{8})\b")
CALL_RE = re.compile(r"\b(func_[0-9A-Fa-f]{8})\s*\(")
DATA_NAME_RE = re.compile(r"\b(D_[0-9A-Fa-f]{7,8})\b")
EXTERN_RE = re.compile(r"^[ \t]*extern\s+(.+?);", re.MULTILINE)
DEF_RE = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*?\b(func_[0-9A-Fa-f]{8})\s*\([^;{]*?\)\s*\{",
    re.MULTILINE,
)
HDR_RE = re.compile(
    r"^[ \t]*[A-Za-z_][A-Za-z0-9_ \t\*]*?\b(func_[0-9A-Fa-f]{8})\s*\(([^;{]*?)\)\s*\{",
    re.MULTILINE,
)
BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.S)
LINE_COMMENT_RE = re.compile(r"//[^\n]*")
PTR_CAST_RE = re.compile(
    r"\(\s*(?:struct\s+\w+\s*\*|unsigned\s+\w+\s*\*|signed\s+\w+\s*\*"
    r"|\w+\s*\*)\s*\)\s*([^;,)\]]{1,60})"
)
PTRPTR_STORE_RE = re.compile(r"\*\s*\([^()]*\*\s*\*[^()]*\)\s*(D_[0-9A-Fa-f]{7,8})")
# `func_XXX(` at a call site, with the balanced argument text.
CALL_HEAD_RE = re.compile(r"\b(func_[0-9A-Fa-f]{8})\s*\(")
PROTO_RE = re.compile(
    r"\b(func_[0-9A-Fa-f]{8})\s*\(([^;{}]*?)\)\s*;", re.MULTILINE
)
# Same prototypes as PROTO_RE, but with the return type captured.  Used by the
# E13/E14 host-pointer screens, which need the callee's host *types*, not only
# its arity.
SIG_RE = re.compile(
    r"(?:^|[;}])[ \t]*((?:extern[ \t]+)?[A-Za-z_][A-Za-z0-9_ \t\*]*?)"
    r"\b(func_[0-9A-Fa-f]{8})[ \t]*\(([^;{}]*?)\)[ \t]*;",
    re.MULTILINE,
)
# A statement that declares local objects: `int *p;`, `char buf[8];`,
# `unsigned char *p, *q;`.  The leading keyword filter keeps statements such as
# `return p[0];` out.
LOCAL_DECL_RE = re.compile(
    r"^[ \t]*((?:(?:register|static|const|volatile|unsigned|signed|struct|union|enum)"
    r"[ \t]+)*[A-Za-z_]\w*(?:[ \t]+[A-Za-z_]\w*)*)[ \t]+([^;{}()\n]+);[ \t]*$",
    re.MULTILINE,
)
# A cast to an *integer* type (no `*`): `(unsigned int)D_800119CC`, `(int)arg0`.
# Groups: 1 = the integer type, 2 = `&` or empty, 3 = the operand's root name.
INT_CAST_RE = re.compile(
    r"\(\s*((?:(?:unsigned|signed|long|short|const|volatile)[ \t]+)*"
    r"(?:int|long|short|char|unsigned|signed|pe_addr_t|uint32_t|size_t))\s*\)"
    r"\s*(&?)\s*([A-Za-z_]\w*)")
INT_PAREN_CAST_RE = re.compile(
    r"\(\s*((?:(?:unsigned|signed|long|short|const|volatile)[ \t]+)*"
    r"(?:int|long|short|char|unsigned|signed|pe_addr_t|uint32_t|size_t))\s*\)"
    r"(?=\s*\()")
STATEMENT_KEYWORDS = {
    "return", "if", "else", "while", "for", "do", "switch", "case", "goto",
    "break", "continue", "sizeof", "default", "typedef",
}


_NOT_TYPE_WORDS = {"return", "else", "case", "do", "sizeof", "goto"}


def is_declarator(text: str, start: int) -> bool:
    """Is the `func_X(` at `start` a declaration (prototype) rather than a
    call?  A declarator is preceded, within its statement, only by type words
    and `*` (`extern int func_X();`, `void *func_X(void);`); a call is
    preceded by an operator, `(`, `,`, `=` or a keyword such as `return`."""
    j = start - 1
    while j >= 0 and text[j] in " \t\n*":
        j -= 1
    if j < 0 or not (text[j].isalnum() or text[j] == "_"):
        return False
    k = j
    while k >= 0 and (text[k].isalnum() or text[k] == "_"):
        k -= 1
    return text[k + 1: j + 1] not in _NOT_TYPE_WORDS


def call_args(text: str) -> list[tuple[str, str]]:
    """Return (callee, argument text) for every call, paren-balanced.
    Prototypes (`extern int func_X();`) are declarations, not calls, and are
    skipped: an empty retail prototype said nothing about the call's arity
    (ovl_0700's `extern int func_8006E6A8();` with a 3-argument call)."""
    out: list[tuple[str, str]] = []
    for m in CALL_HEAD_RE.finditer(text):
        if is_declarator(text, m.start()):
            continue
        i = m.end()
        depth = 1
        while i < len(text) and depth:
            if text[i] == "(":
                depth += 1
            elif text[i] == ")":
                depth -= 1
            i += 1
        if depth == 0:
            out.append((m.group(1), text[m.end(): i - 1]))
    return out


def arity(args: str) -> int:
    args = args.strip()
    if not args or args == "void":
        return 0
    depth = 0
    n = 1
    for ch in args:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif ch == "," and depth == 0:
            n += 1
    return n


def canonical_protos() -> dict[str, int]:
    """func_* symbol -> parameter count from the pc_port shim headers."""
    proto: dict[str, int] = {}
    roots = [REPO_ROOT / "pc_port" / "include", REPO_ROOT / "pc_port" / "platform",
             REPO_ROOT / "pc_port" / "game"]
    for root in roots:
        if not root.is_dir():
            continue
        for path in root.rglob("*.h"):
            if "build" in path.relative_to(REPO_ROOT).parts:
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            for m in PROTO_RE.finditer(strip_comments(text)):
                proto.setdefault(m.group(1), arity(m.group(2)))
    return proto


def shim_header_roots() -> list[Path]:
    return [REPO_ROOT / "pc_port" / "include", REPO_ROOT / "pc_port" / "platform",
            REPO_ROOT / "pc_port" / "game"]


def canonical_signatures() -> dict[str, dict]:
    """func_* symbol -> {'ret': <return type>, 'params': [<parameter decls>]}.

    The pc_port shim headers are the host authority for a callee's type.  A
    leaf's own `extern` declaration describes the *retail* type (where an
    `int *` is a 32-bit guest address), so the two must be compared before a
    pointer is allowed to cross a call boundary.
    """
    sigs: dict[str, dict] = {}
    for root in shim_header_roots():
        if not root.is_dir():
            continue
        for path in sorted(root.rglob("*.h")):
            if "build" in path.relative_to(REPO_ROOT).parts:
                continue
            text = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
            for m in SIG_RE.finditer(text):
                name = m.group(2)
                if name in sigs:
                    continue
                ret = re.sub(r"^\s*extern\s+", "", m.group(1)).strip()
                sigs[name] = {"ret": ret, "params": split_top(m.group(3))}
    return sigs


def derived_signatures() -> dict[str, dict]:
    """func_* -> host signature of a *generated* decomp-derived TU.

    A derived leaf has no canonical header prototype, so its callers used to
    keep their own verbatim (retail-typed) declaration of it.  That is only
    sound while the callee's host signature equals the retail one; an E7b
    adapted callee takes `pe_addr_t` where the retail prototype says
    `unsigned char *`, so the caller's declaration would disagree with the
    definition it links against.  The generated definition *is* the host
    authority, so it is read back here and treated exactly like a canonical
    prototype (E11 arity, E13/E14 host-pointer screens, and the caller's own
    declaration is replaced by this host prototype).

    Only files the generator owns (`func_XXXXXXXX_port.c` carrying a
    `decomp-source:` banner) are read; hand ports keep their headers.
    """
    sigs: dict[str, dict] = {}
    out_dir = REPO_ROOT / OUT_DIR
    if not out_dir.is_dir():
        return sigs
    for path in sorted(out_dir.glob("func_*_port.c")):
        name = path.name[: -len("_port.c")]
        text = path.read_text(encoding="utf-8", errors="replace")
        if f"decomp-source: {name}" not in text:
            continue
        stripped = strip_comments(text)
        for m in WRAPPER_SIG_RE.finditer(stripped):
            if m.group(2) != name:
                continue
            ret = re.sub(r"\s+", " ", m.group(1)).strip()
            params = re.sub(r"\s+", " ", m.group(3)).strip()
            sigs[name] = {"ret": ret, "params": split_top(params),
                          "decl": f"{ret} {name}({params or 'void'});"}
            break
    return sigs


def _param_types(params: list[str]) -> list[str]:
    """Parameter types with the parameter name dropped, whitespace-normalised."""
    out: list[str] = []
    for p in params:
        p = re.sub(r"\s+", " ", p).strip()
        m = re.fullmatch(r"(.*?[\s\*])([A-Za-z_]\w*)", p)
        if m and m.group(1).strip() and m.group(2) not in C_KEYWORDS:
            p = m.group(1)
        out.append(re.sub(r"\s*\*\s*", "*", p).strip())
    return out


def same_host_decl(stripped: str, callee: str, dsig: dict) -> bool:
    """Does every declaration of `callee` in the leaf already equal its host
    signature (types only)?  Then the verbatim declaration stays (it is part of
    the matched unit and may name leaf-local types); otherwise the generated
    host prototype replaces it."""
    decls = [m for m in SIG_RE.finditer(stripped) if m.group(2) == callee]
    if not decls:
        return False
    want_ret = re.sub(r"\s*\*\s*", "*", dsig["ret"]).strip()
    want = _param_types(dsig["params"])
    for m in decls:
        ret = re.sub(r"^\s*extern\s+", "", m.group(1))
        ret = re.sub(r"\s*\*\s*", "*", re.sub(r"\s+", " ", ret)).strip()
        if ret != want_ret or _param_types(split_top(m.group(3))) != want:
            return False
    return True


def split_top(text: str) -> list[str]:
    """Split a parameter/argument list on top-level commas."""
    text = text.strip()
    if not text or text == "void":
        return []
    parts: list[str] = []
    depth = 0
    cur = ""
    for ch in text:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur.strip())
            cur = ""
        else:
            cur += ch
    parts.append(cur.strip())
    return parts


def is_host_pointer_type(decl: str) -> bool:
    """Does this host declaration denote a pointer (as opposed to pe_addr_t)?"""
    return "*" in decl or re.search(r"\[\s*\w*\s*\]", decl) is not None


def local_objects(body: str) -> tuple[set[str], set[str]]:
    """(every local declared in the leaf, those that are host memory).

    `int *p;` holds a host address once assigned and `char buf[8];` decays to
    one; neither is a guest address.  A plain scalar local is only host memory
    when its address is taken (`&n`), which the caller checks.
    """
    every: set[str] = set()
    pointerish: set[str] = set()
    for m in LOCAL_DECL_RE.finditer(body):
        head = m.group(1).split()
        if head[0] in STATEMENT_KEYWORDS or head[-1] in STATEMENT_KEYWORDS:
            continue
        for declarator in split_top(m.group(2)):
            ident = re.search(r"([A-Za-z_]\w*)", declarator)
            if not ident:
                continue
            every.add(ident.group(1))
            if declarator.lstrip().startswith("*") or re.search(
                    r"\[\s*[^\]]*\]", declarator):
                pointerish.add(ident.group(1))
    return every, pointerish


def pointer_locals(body: str) -> set[str]:
    """Locals declared as plain pointers (`T *p`), not arrays: in a generated
    TU they hold host pointers into guest RAM (or into a host stack object,
    which PE_HostToGuest rejects loudly)."""
    out: set[str] = set()
    for m in LOCAL_DECL_RE.finditer(body):
        head = m.group(1).split()
        if head[0] in STATEMENT_KEYWORDS or head[-1] in STATEMENT_KEYWORDS:
            continue
        for declarator in split_top(m.group(2)):
            d = declarator.strip()
            ident = re.search(r"([A-Za-z_]\w*)", d)
            if ident and d.startswith("*") and "[" not in d.split("=")[0]:
                out.add(ident.group(1))
    return out


def strip_comments(text: str) -> str:
    return LINE_COMMENT_RE.sub("", BLOCK_COMMENT_RE.sub("", text))


def vram_of(file_offset: int) -> int:
    return LOAD_VRAM + file_offset - HEADER_BYTES


def parse_matched() -> dict[str, dict]:
    cfg = (REPO_ROOT / CONFIG).read_text(encoding="utf-8")
    offsets = [int(m.group(1), 16) for m in EDGE_RE.finditer(cfg)]
    out: dict[str, dict] = {}
    for m in C_SPAN_RE.finditer(cfg):
        start = int(m.group(1), 16)
        end = next((o for o in offsets if o > start), None)
        if end is None:
            raise SystemExit(f"gen_decomp_ports: cannot resolve end of {m.group(2)}")
        out[m.group(2)] = {
            "name": m.group(2),
            "file_offset": start,
            "file_size": end - start,
            "vram": vram_of(start),
            "words": (end - start) // 4,
        }
    return out


def pc_port_definitions() -> dict[str, set[str]]:
    """func_* symbol -> set of pc_port source files that define it."""
    defined: dict[str, set[str]] = {}
    for path in (REPO_ROOT / "pc_port").rglob("*.c"):
        if "build" in path.relative_to(REPO_ROOT).parts:
            continue
        # Overlay TUs (gen_overlay_ports.py) define `<ovl>__func_X` through a
        # rename macro; their textual `func_X(...) {` is not a plain symbol.
        if "decomp_ovl" in path.parts:
            continue
        rel = path.relative_to(REPO_ROOT).as_posix()
        text = path.read_text(encoding="utf-8", errors="replace")
        for m in DEF_RE.finditer(text):
            # A `static` definition is file-local: not linkable by a generated
            # TU (func_800762A0 in pe_libgpu.c made func_80075C04 reference an
            # undefined symbol).
            if re.match(r"\s*static\b", m.group(0)):
                continue
            defined.setdefault(m.group(1), set()).add(rel)
    return defined


INLINE_SHIM_RE = re.compile(
    r"^[ \t]*static[ \t]+inline[ \t]+([A-Za-z_][A-Za-z0-9_ \t\*]*?)\b"
    r"(func_[0-9A-Fa-f]{8})[ \t]*\(([^;{}]*?)\)[ \t]*\{([^{}]*)\}",
    re.MULTILINE)


def inline_shims() -> dict[str, dict]:
    """func_* -> host signature of a `static inline` shim in a pc_port header.

    psx_compat.h implements a few SDK leaves inline (VSync func_80073A44 ->
    HostFB_VSync, SetDispMask func_80074D28).  Every generated TU includes
    that header, so the shim is the callee the rest of the port already uses;
    treating it as unimplemented made generated TUs record a loud boundary and
    read 0 where retail reads the VSync counter.  Shims that only record a
    bootstrap boundary (Bootstrap_*) are not implementations and are skipped.
    """
    out: dict[str, dict] = {}
    for root in shim_header_roots():
        if not root.is_dir():
            continue
        for path in sorted(root.rglob("*.h")):
            if "build" in path.relative_to(REPO_ROOT).parts:
                continue
            text = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
            for m in INLINE_SHIM_RE.finditer(text):
                if "Bootstrap_" in m.group(4):
                    continue
                out.setdefault(m.group(2), {"ret": m.group(1).strip(),
                                            "params": split_top(m.group(3))})
    return out


def pc_port_macros() -> set[str]:
    """D_ symbols already provided as macros by the pc_port shim headers."""
    names: set[str] = set()
    for rel in ("pc_port/include/psx_compat.h", "pc_port/include/pe_port_compat.h"):
        text = (REPO_ROOT / rel).read_text(encoding="utf-8", errors="replace")
        for m in re.finditer(r"^[ \t]*#define\s+(D_[0-9A-Fa-f]{7,8})\b", text,
                             re.MULTILINE):
            names.add(m.group(1))
    return names


def declared_type(decl: str) -> str:
    """Extract the element type from `extern <type> NAME[, NAME2 ...];`."""
    cut = DATA_NAME_RE.search(decl)
    body = decl[: cut.start()] if cut else decl
    return re.sub(r"\s+", " ", body).strip() or "unsigned int"


def pointer_pointee(etype: str) -> str:
    """Pointee of a pointer-global declaration.

    `unsigned char *` -> `unsigned char`, and `unsigned int *volatile`
    (a volatile *slot*, not a volatile pointee) -> `unsigned int`.  Dropping
    the slot qualifier is behaviour-preserving: PE_DECOMP_PTRGLOBAL re-reads
    the guest word with PE_LoadU32 on every expansion anyway.
    """
    t = re.sub(r"\s*\*\s*(?:(?:const|volatile)\s*)*$", "", etype).strip()
    return t or "unsigned char"


def classify_data(decl: str, sym: str) -> str:
    if "[" in decl:
        return "array"
    head = decl[: DATA_NAME_RE.search(decl).start()] if DATA_NAME_RE.search(decl) else decl
    if "*" in head:
        return "ptrglobal"
    return "scalar"


def parse_params(header: str) -> tuple[list[str], bool]:
    """Return (parameter names, has_pointer_parameter)."""
    inner = header.strip()
    if inner in ("", "void"):
        return [], False
    parts: list[str] = []
    depth = 0
    cur = ""
    for ch in inner:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur)
            cur = ""
        else:
            cur += ch
    parts.append(cur)
    names: list[str] = []
    ptr = False
    for p in parts:
        p = p.strip()
        if not p or p == "void":
            continue
        if "*" in p or re.search(r"\[\s*\]", p):
            ptr = True
        m = re.search(r"(\w+)\s*(\[\s*\])?\s*$", p)
        if m:
            names.append(m.group(1))
    return names, ptr


C_KEYWORDS = {
    "void", "char", "short", "int", "long", "unsigned", "signed", "float",
    "double", "const", "volatile", "struct", "union", "enum", "restrict",
    "_Bool",
}
DOUBLE_PTR_TOKEN_RE = re.compile(r"\*\s*\*")
PTR_DEREF_CAST_RE = re.compile(
    r"\*\s*\(\s*(?:[A-Za-z_]\w*\s+)*[A-Za-z_]\w*\s*\*+\s*\*")


def pointer_params(inner: str) -> list[tuple[str, int, list[str], str]]:
    """(name, pointer level, named-type words, declaration) for pointer params."""
    out: list[tuple[str, int, list[str], str]] = []
    for part in inner.split(","):
        p = part.strip()
        if not p or p == "void":
            continue
        m = re.search(r"(\w+)\s*$", p)
        name = m.group(1) if m else ""
        # Keep the full text before the parameter's own name: array binders
        # like `unsigned char a0[4]` leave the trailing `[4]` after the name.
        decl = p[: m.end()].strip() if m else p
        toks = [t for t in re.findall(r"[A-Za-z_]\w*", decl)
                if t not in C_KEYWORDS and t != name]
        level = p.count("*") + (1 if re.search(r"\[\s*\]", p) else 0)
        if level > 0:
            out.append((name, level, toks, p))
    return out


# E7c — guest-pointer-vector parameters.  A field-VM opcode handler receives
# `T **args`: a guest array of 32-bit guest pointers to its operands.  Every use
# the adapter accepts is a *dereferenced element*, which is exactly retail's
# `lw` of the element followed by an access through it:
#     p[k]  /  *(p + k)  /  *p      (each immediately dereferenced)
# and is rewritten to PE_DECOMP_PTRGLOBAL(pe_p + 4u * (k), T), which loads the
# 32-bit guest word and translates it.  Anything else (bare element passed on,
# pointer arithmetic, p stored or reassigned) is reported, not guessed.
ARGVEC_INDEX_RE = r"(?<![.>\w]){p}\s*\[([^\[\]]+)\]"
ARGVEC_PLUS_RE = r"\*\s*\(\s*{p}\s*\+\s*(0x[0-9A-Fa-f]+|\d+)\s*\)"
ARGVEC_DEREF_RE = r"\*\s*(?<![.>\w]){p}\b(?!\s*[\[+(])"
# What may precede an element for it to count as dereferenced: `*` or
# `*(T *)` (a requalifying cast of the element pointer, then the deref).
ARGVEC_PRE_RE = re.compile(r"\*\s*(?:\(\s*(?:[A-Za-z_]\w*\s+)*[A-Za-z_]\w*\s*\*\s*\)\s*)?$")


def argvec_params(inner: str) -> dict[str, str]:
    """Level-2 primitive pointer params: name -> element pointee type T."""
    out: dict[str, str] = {}
    for pname, level, toks, decl in pointer_params(inner):
        if level != 2 or toks or "[" in decl:
            continue
        out[pname] = param_base_type(decl, pname)
    return out


def argvec_rewrite(body: str, pname: str, etype: str) -> tuple[str, str]:
    """Rewrite every use of guest-pointer-vector `pname` in `body`.

    Returns (new body, "" on success | reason).  Each element use must be
    dereferenced (preceded by `*` / `*(T *)`, or followed by `[` / `->`).
    """
    pe = f"pe_{pname}"
    out: list[str] = []
    pos = 0
    pats = [(re.compile(ARGVEC_PLUS_RE.format(p=re.escape(pname))), "plus"),
            (re.compile(ARGVEC_INDEX_RE.format(p=re.escape(pname))), "index"),
            (re.compile(ARGVEC_DEREF_RE.format(p=re.escape(pname))), "deref")]
    hits: list[tuple[int, int, str, str]] = []
    for rx, kind in pats:
        for m in rx.finditer(body):
            if any(not (m.end() <= a or m.start() >= b) for a, b, _, _ in hits):
                continue
            idx = m.group(1) if kind in ("plus", "index") else "0"
            hits.append((m.start(), m.end(), kind, idx))
    hits.sort()
    for start, end, kind, idx in hits:
        # `*(p + k)` / `*p` carry their own dereference, which the element
        # expression replaces: one more `*` (or `->`/`[`) must still apply.
        before = body[:start]
        after = body[end:].lstrip()
        if not (ARGVEC_PRE_RE.search(before) or after.startswith("[")
                or after.startswith("->")):
            return body, f"guest-pointer vector {pname} element used undereferenced"
        out.append(body[pos:start])
        out.append(f"PE_DECOMP_PTRGLOBAL({pe} + 4u * ({idx.strip()}), {etype})")
        pos = end
    out.append(body[pos:])
    new = "".join(out)
    if re.search(rf"(?<![.>\w]){re.escape(pname)}\b", new):
        return body, f"guest-pointer vector {pname} used as a value"
    return new, ""


def wrapper_safe(name: str, stripped: str, inner: str) -> tuple[bool, str]:
    """Can this pointer-param leaf be adapted by the verbatim-wrapper recipe?"""
    vecs = argvec_params(inner)
    if vecs:
        body = extract_body(stripped, name)
        if body is None:
            return False, "no function body"
        for pname, etype in vecs.items():
            body, why = argvec_rewrite(body, pname, etype)
            if why:
                return False, why
        # The vector parameters are the only `**` the adapter may absorb.
        if DOUBLE_PTR_TOKEN_RE.search(body):
            return False, "pointer-to-pointer value (guest pointer in RAM)"
        rest = inner
        for pname in vecs:
            rest = re.sub(rf"\*\s*\*\s*{re.escape(pname)}\b", pname, rest)
        if DOUBLE_PTR_TOKEN_RE.search(rest):
            return False, "pointer-to-pointer value (guest pointer in RAM)"
    elif DOUBLE_PTR_TOKEN_RE.search(stripped):
        return False, "pointer-to-pointer value (guest pointer in RAM)"
    # A pointer parameter whose address is stored (e.g. `p->link = arg`) writes
    # a host pointer into guest RAM; only value/address uses are adaptable.
    for pname, _lvl, _toks, _decl in pointer_params(inner):
        if re.search(rf"&\s*{re.escape(pname)}\b", stripped):
            return False, f"address of pointer parameter {pname}"
        # Storing a host pointer into guest RAM (`dst->source = source;`) is not
        # representable: the guest word must hold a 32-bit guest address.  An
        # indexed use (`dst->value = arg0[1];`) is a load of guest RAM through
        # the parameter, not a pointer store, so it stays adaptable.
        if re.search(
                rf"(?:->|\.|\[)[^;]*?=[^=]\s*{re.escape(pname)}\b(?!\s*\[)",
                stripped):
            return False, f"pointer parameter {pname} stored into guest RAM"
    # A recursive self-call passes a host pointer where the adapted signature
    # takes a guest address; that needs a reverse translation, so it is
    # reported rather than guessed.
    inner_body = extract_body(stripped, name)
    if inner_body is not None and re.search(
            rf"\b{re.escape(name)}\s*\(", inner_body):
        return False, "recursive self-call"
    if PTR_DEREF_CAST_RE.search(stripped):
        return False, "store through a pointer-dereference cast"
    for pname, level, _toks, _decl in pointer_params(inner):
        if level > 1 and pname not in vecs:
            return False, f"parameter {pname} is level-{level}"
    if re.search(r"&\s*func_[0-9A-Fa-f]{8}", stripped):
        return False, "address of function"
    return True, ""


# Leaf-local typedefs that are layout-identical to a pc_port header type of
# the same name.  psx_compat.h declares `typedef struct { int16_t x, y, w, h;
# } RECT;` (the libgpu RECT); a leaf repeating it with `short` members is the
# same object, but C rejects the second typedef as conflicting.  Only the exact
# member list is accepted — any other RECT stays verbatim and the E17 compile
# gate judges it.
HOST_TYPEDEF_RE = re.compile(
    r"^[ \t]*typedef\s+struct\s*\{\s*(?:signed\s+)?short\s+x\s*,\s*y\s*,\s*w\s*,"
    r"\s*h\s*;\s*\}\s*RECT\s*;[ \t]*$", re.MULTILINE)


def neutralize_host_typedefs(text: str) -> str:
    return HOST_TYPEDEF_RE.sub(
        "/* RECT: psx_compat.h (int16_t x, y, w, h — identical layout) */", text)


def analyze(name: str, src_text: str, matched: dict,
            known: set[str], existing_macros: set[str],
            proto: dict[str, int], sigs: dict[str, dict]) -> dict:
    # E2b guard: a pin *without* an initialiser that is read before it is
    # written captures a live retail register — that is semantics, not a hint.
    for m in re.finditer(
            r"\bregister\b[^;{}()=]*?\b([A-Za-z_]\w*)\s*(?:__asm__|asm)\s*"
            r"\(\s*\"\$[0-9A-Za-z]+\"\s*\)\s*;", strip_comments(src_text)):
        rest = strip_comments(src_text)[m.end():]
        # A struct member of the same name (`a.w.lo`, `p->lo`) is not the pin.
        use = re.search(rf"(?<![.\w])(?<!->)\s*\b{re.escape(m.group(1))}\b", rest)
        if use and not re.match(r"\s*=(?!=)", rest[use.end():]):
            return {"eligible": False,
                    "reason": f"pinned register {m.group(1)} read before written"}
    src_text = rewrite_code_addresses(strip_era_asm(src_text))
    src_text = neutralize_host_typedefs(src_text)
    src_text, why = rewrite_guest_calls(src_text, name)
    if why:
        return {"eligible": False, "reason": why}
    stripped = strip_comments(src_text)
    if ASM_RE.search(stripped):
        return {"eligible": False, "reason": "asm"}
    # Reject hardware / BIOS address spaces (MMIO 0x1F8xxxxx, scratchpad
    # 0x1F800000, BIOS ramps 0xA0xxxxxx/0xB0xxxxxx).  Other 32-bit literals are
    # ordinary masks/constants and stay verbatim.
    for lit in NUM_RE.findall(stripped):
        hi = int(lit, 16) & 0xFF000000
        if hi in (0x1F000000, 0xA0000000, 0xB0000000):
            return {"eligible": False, "reason": f"hardware/BIOS literal {lit}"}
    if DEF_RE.findall(stripped) != [name]:
        return {"eligible": False, "reason": "definition set"}
    if re.search(r"\(\s*\*\s*\w+\s*\)\s*\(", stripped):
        return {"eligible": False, "reason": "indirect call through local"}

    # E18 — a leaf-defined struct with a pointer member describes guest RAM
    # whose pointer fields are 32-bit guest addresses; the host compiler lays
    # the same struct out with 8-byte host pointers, so every field access
    # through it (and every field after the first pointer) is wrong.
    for m in re.finditer(r"\bstruct\s*\w*\s*\{([^{}]*)\}", stripped):
        if re.search(r"\*\s*[A-Za-z_]\w*\s*(?:\[[^\]]*\])?\s*[;,]", m.group(1)):
            return {"eligible": False,
                    "reason": "leaf struct with pointer members (guest layout)"}

    hdr = HDR_RE.search(stripped)
    if hdr is None:
        return {"eligible": False, "reason": "no function header"}
    ret_type = hdr.group(0)[: hdr.group(0).find(name)]
    ptr_ret: str | None = None
    if "*" in ret_type:
        # E7 (relaxed): an object-pointer return is adapted by the
        # pointer-return host adapter — the verbatim leaf runs as
        # `static T *pe_host_<name>` and the exported `pe_addr_t <name>`
        # returns PE_HostToGuest() of its result (a host pointer into guest
        # RAM, NULL -> 0, anything else aborts loudly).  Function-pointer
        # returns and recursive leaves stay reported.
        rt = ret_type.strip()
        if not PTR_RET_RE.fullmatch(rt):
            return {"eligible": False, "reason": "pointer return type"}
        body_only = extract_body(stripped, name)
        if body_only is None or re.search(rf"\b{re.escape(name)}\s*\(", body_only):
            return {"eligible": False,
                    "reason": "pointer return type (recursive self-call)"}
        ptr_ret = rt
    params, has_ptr_param = parse_params(hdr.group(2))
    sign_adapt = None
    if not ptr_ret and not has_ptr_param and name in sigs:
        sign_adapt = signedness_adapter(ret_type, hdr.group(2), sigs[name])
        if sign_adapt == "reject":
            # Not an E20 shape (e.g. an unprototyped `T func_X()` header, which
            # the verbatim definition is compatible with); the E17 compile
            # gate stays the judge.
            sign_adapt = None
    wrap: dict | None = None
    if has_ptr_param:
        ok, why = wrapper_safe(name, stripped, hdr.group(2))
        if not ok:
            return {"eligible": False, "reason": f"pointer parameter: {why}"}
        wrap = {"param_inner": hdr.group(2)}
    # A callee returning a guest address is likewise not representable: the
    # call result is a 32-bit guest pointer, and indexing/dereferencing it in
    # C would need a host pointer.  Such leaves need a hand-written adapter.
    if re.search(r"\bfunc_[0-9A-Fa-f]{8}\s*\([^;]*?\)\s*(\[|->|\.)", stripped):
        return {"eligible": False, "reason": "callee result dereferenced"}

    if re.search(r"&\s*func_[0-9A-Fa-f]{8}", stripped):
        return {"eligible": False, "reason": "address of function"}
    # A bare `func_XXX` not followed by `(` is a function address used as a
    # value (a callback argument).  32-bit code pointers are not host
    # pointers, so those leaves need a hand-written adapter.
    for m in FUNC_NAME_RE.finditer(stripped):
        if not stripped[m.end():].lstrip().startswith("("):
            return {"eligible": False,
                    "reason": f"function address value {m.group(1)}"}
    if PTRPTR_STORE_RE.search(stripped):
        return {"eligible": False, "reason": "pointer-pointer store to D_ symbol"}

    externs: list[tuple[str, str]] = []
    for m in EXTERN_RE.finditer(stripped):
        decl = m.group(1)
        for sym in DATA_NAME_RE.findall(decl):
            externs.append((sym, decl))

    # E9b — a cast to a pointer-to-pointer type, `*(T **)(rec + 0x68)`, reads
    # a *host* pointer (8 bytes on a 64-bit host) out of 4-byte guest RAM.
    # PTR_CAST_RE never matched these (the second `*` broke it), so they
    # slipped through E9/E15 (func_80030640, func_80051684, func_800524D0,
    # func_80017EC4, func_80067B74, func_80067D18).
    if PTRPTR_CAST_RE.search(stripped):
        return {"eligible": False,
                "reason": "pointer-to-pointer cast (guest pointer in RAM)"}

    # E9 — a pointer cast must be rooted at a *host pointer into guest RAM*:
    # an array / pointer-global D_ symbol, a pointer parameter (the E7b
    # wrapper translates it), or a local/parameter whose own declaration is
    # pointer- or array-typed.  An integer — a scalar D_ symbol, an int
    # parameter, or an int local that merely *holds* a guest address
    # (`base = D_8009D310`) — is a 32-bit guest address, not a host pointer,
    # so `(unsigned char *)(D_8009D2F0 + 0xF)` is rejected.  (This replaces
    # the old "mentions a name assigned from a D_ symbol" test, which also
    # admitted integer locals and scalar symbols.)
    ptr_roots = {sym for sym, decl in externs
                 if classify_data(decl, sym) in ("array", "ptrglobal")}
    ptr_roots |= {p for p, _lvl, _toks, _d in pointer_params(hdr.group(2))}
    ptr_roots |= pointer_typed_names(stripped)
    vec_roots = set(argvec_params(hdr.group(2)))
    for m in PTR1_CAST_RE.finditer(stripped):
        nxt = stripped[m.end():].lstrip()[:1]
        if nxt in ("", ";", ",", ")", "{"):
            continue            # an abstract declarator `f(void *)`, not a cast
        operand = cast_operand(stripped, m.end())
        if is_pointer_expr(operand, ptr_roots):
            continue
        # E7c: `(T *)args[k]` on a guest-pointer-vector parameter is the
        # element E7c rewrites to PE_DECOMP_PTRGLOBAL(pe_args + 4u*k, T).
        if re.fullmatch(r"(\w+)\s*\[[^\[\]]+\]", _strip_parens(operand)) and \
                re.match(r"\w+", _strip_parens(operand)).group(0) in vec_roots:
            continue
        return {"eligible": False,
                "reason": f"pointer cast of non-D_ operand: {operand.strip()[:32]}"}

    callees = sorted(set(CALL_RE.findall(stripped)) - {name})

    # A callee with a canonical pc_port prototype must be called with the same
    # arity, or the host build would reinterpret its arguments.  The matched
    # leaf is the authority for the call site, so a mismatch is reported for a
    # hand-written adapter instead of being coerced by codegen.
    for callee, args in call_args(stripped):
        if callee == name or callee not in proto:
            continue
        if empty_callee_droppable(callee, args, proto):
            continue        # E20f: render drops the literal arguments
        if arity(args) != proto[callee]:
            return {"eligible": False,
                    "reason": f"call arity {callee} {arity(args)}!={proto[callee]}"}

    # E13/E14 — a host pointer must not cross a call boundary in either
    # direction.  Retail passes and returns 32-bit *guest* addresses; host-side
    # the callee is a canonical `pe_addr_t` prototype or a loud boundary
    # returning `int`, so either conversion silently reinterprets a host
    # pointer as a guest address (or vice versa).  The leaf's own `extern`
    # declaration describes the retail type and is neutralised by the
    # generator, so it cannot rescue the call site.
    body_text = extract_body(stripped, name)
    _all_locals, host_locals = local_objects(
        body_text if body_text is not None else stripped)
    ptr_locals = pointer_locals(body_text if body_text is not None else stripped)
    hostptr_args: set[tuple[str, int]] = set()
    # A pointer *parameter* is not host memory at a call site: the E7b wrapper
    # forwards it as `pe_<name>`, the guest address the callee's `pe_addr_t`
    # parameter wants.  It is still a host pointer as an assignment *target*
    # (the wrapper rewrites it to the translated `host_<name>`).
    assign_targets = host_locals | {
        p for p, _lvl, _toks, _d in pointer_params(hdr.group(2))}

    def host_return(callee: str) -> bool:
        """Does the callee's *host* declaration return a pointer?"""
        if callee in sigs:
            return is_host_pointer_type(sigs[callee]["ret"])
        if callee not in known:
            return False  # becomes a loud boundary macro: returns int
        return True  # a pc_port definition with no shim prototype: leaf's own

    for m in re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*(func_[0-9A-Fa-f]{8})\s*\(",
                         stripped):
        if m.group(1) in assign_targets and not host_return(m.group(2)):
            return {"eligible": False,
                    "reason": f"guest-address result of {m.group(2)} bound to "
                              f"host pointer {m.group(1)}"}
    for m in re.finditer(
            r"\*+\s*([A-Za-z_]\w*)\s*=\s*(func_[0-9A-Fa-f]{8})\s*\(", stripped):
        # `T *p = func_X(...)` — a declaration with an initialiser, which the
        # local-declaration scan deliberately does not parse.
        decl_head = stripped[: m.start()].rstrip().rsplit("\n", 1)[-1].strip()
        if re.fullmatch(r"(?:[A-Za-z_]\w*\s+)+", decl_head + " ") and \
                not host_return(m.group(2)):
            return {"eligible": False,
                    "reason": f"guest-address result of {m.group(2)} bound to "
                              f"host pointer {m.group(1)}"}

    for callee, args in call_args(stripped):
        if callee == name:
            continue
        if callee not in sigs:
            # A loud boundary records guest argument registers: a host stack
            # object (`&v18`, a local array) has no guest address, and its
            # truncated host address in the log is meaningless (func_800D5898
            # recorded a host `&v18` for func_800D27FC).  Report it.
            if callee in known:
                continue
            for arg in split_top(args):
                bare = arg.strip().strip("()").strip()
                if address_arg(arg, {d for d, _ in externs}, dict(externs)) is not None:
                    continue
                root = re.match(r"&\s*([A-Za-z_]\w*)", bare)
                if bare in ptr_locals or (
                        root and root.group(1) in ptr_locals and
                        re.match(r"&\s*[A-Za-z_]\w*\s*(->|\[)", bare)):
                    # a pointer local, or &p->f / &p[k] through one: guest
                    # memory, forwarded as PE_HostToGuest(...)
                    hostptr_args.add((callee, split_top(args).index(arg)))
                    continue
                if (root and root.group(1) in _all_locals) or bare in host_locals:
                    return {"eligible": False,
                            "reason": f"host pointer {bare} passed to boundary {callee}"}
            continue
        hparams = sigs[callee]["params"]
        for i, arg in enumerate(split_top(args)):
            bare = arg.strip().strip("()").strip()
            # `&D_X`, `&D_X[k]`, array/pointer-global `D_X + k`: rewritten to
            # the retail guest address by fix_argument_addresses.
            if address_arg(arg, {d for d, _ in externs}, dict(externs)) is not None:
                continue
            if bare.startswith("&"):
                # `&local`, `&D_SYM[i]`, `&s.field` — address-of always yields a
                # *host* address in the generated TU, where retail took a guest
                # one.  (A bare `D_SYM` argument is not this case;
                # fix_argument_addresses substitutes the guest address.)
                pass
            elif re.fullmatch(r"[A-Za-z_]\w*", bare):
                if bare not in host_locals:
                    continue
            else:
                continue
            if i < len(hparams) and not is_host_pointer_type(hparams[i]):
                if bare in ptr_locals:
                    # a host pointer local into guest RAM: pass its guest
                    # address (PE_HostToGuest aborts loudly on host stack)
                    hostptr_args.add((callee, i))
                    continue
                return {"eligible": False,
                        "reason": f"host pointer {bare} passed to {callee} "
                                  f"parameter {i} ({hparams[i].strip()})"}

    # A func_* referenced but never called is a function-address value; those
    # were rejected above.  Everything else is a known definition or becomes a
    # loud boundary.
    # Only callees that are actually *called* need a boundary: a name that
    # only appears in a prototype (its address was rewritten to the retail
    # VMA by rewrite_code_addresses) keeps its verbatim declaration.
    called = [c for c, _ in call_args(stripped)]
    boundaries = [c for c in callees if c not in known and c in called]
    boundary_arity: dict[str, int] = {}
    for callee, args in call_args(stripped):
        if callee in boundaries:
            boundary_arity.setdefault(callee, min(arity(args), 4))

    # Assigning the pointer-global *slot itself* stores a 32-bit guest address
    # where the host type is a pointer, which the generated accessor cannot
    # represent.  A *dereferenced* store (`*D_80095854 = x`) is fine: the
    # accessor already yields the translated pointer.
    for sym, decl in externs:
        if classify_data(decl, sym) != "ptrglobal":
            continue
        # `=` but not `==` (a comparison is not a store).
        if re.search(rf"(?<!\*)\b{sym}\b\s*(=(?!=)|\+=|-=|\|=|&=|\^=)", stripped):
            return {"eligible": False, "reason": f"pointer-global store {sym}"}

    # E15 — a guest array (or global) *of pointers*.  `extern unsigned char
    # *D_800B0E38[];` is 4-byte guest slots; the generated accessor types the
    # element as a host pointer, so `D_800B0E38[i]` would read 8 bytes on a
    # 64-bit host and yield a host pointer where retail has a guest address.
    # One level of indirection is fine (PE_DECOMP_PTRGLOBAL loads the 32-bit
    # slot and translates it); two is not representable.
    for sym, decl in externs:
        head = decl[: DATA_NAME_RE.search(decl).start()]
        stars = head.count("*")
        if stars >= 2 or (stars >= 1 and "[" in decl):
            return {"eligible": False,
                    "reason": f"guest array/global of pointers {sym} "
                              f"({' '.join(decl.split())[:40]})"}

    # E16 — a host pointer narrowed to an integer.  `(unsigned int)D_800119CC`
    # is the retail *guest address* of the symbol; host-side the accessor is a
    # 64-bit host pointer, so the cast truncates it.  The value is then either
    # returned, stored, or cast back and dereferenced — silently wrong code
    # (only a -Wpointer-to-int-cast warning), which is worse than a rejection.
    host_roots = {sym for sym, decl in externs
                  if classify_data(decl, sym) in ("array", "ptrglobal")}
    host_roots |= host_locals
    host_roots |= {p for p, _lvl, _toks, _d in pointer_params(hdr.group(2))}
    # `(unsigned int)(tbl + k)` — the cast applies to a parenthesised
    # pointer expression, which INT_CAST_RE (identifier right after the cast)
    # never saw (func_8005DC4C, func_8006EC6C).
    for m in INT_PAREN_CAST_RE.finditer(stripped):
        operand = cast_operand(stripped, m.end())
        if is_pointer_expr(operand, ptr_roots):
            return {"eligible": False,
                    "reason": f"host pointer {_strip_parens(operand)[:32]} "
                              f"narrowed to {m.group(1).strip()}"}
    for m in INT_CAST_RE.finditer(stripped):
        root = m.group(3)
        if m.group(2) == "&":
            # `(unsigned int)&D_XXXX[i]` — the address of guest storage, taken
            # host-side and then truncated.
            if root not in host_roots and root not in _all_locals \
                    and root not in {s for s, _ in externs}:
                continue
        else:
            # Without `&` the cast must apply to the object itself, not to a
            # value read through it: `(signed char)D_XXXX[i].b` casts the
            # member, which is fine.
            tail = stripped[m.end():].lstrip()
            if tail[:1] in ("[", ".", "(") or tail[:2] == "->":
                continue
            if root not in host_roots:
                continue
        return {"eligible": False,
                "reason": f"host pointer {root} narrowed to "
                          f"{m.group(1).strip()}"}

    # E19 — a relational compare between two pointers (`dst >= src`)
    # compares *host* addresses in the generated TU.  Two pointers into the
    # one guest-RAM allocation order exactly like their KSEG0 guest
    # addresses, but a scratchpad pointer (separate allocation) or a
    # KUSEG/KSEG1 alias (folded by the mirror) does not (portverify finding
    # 9: func_80072334, dst in the scratchpad, src = 0).  Operand classes:
    #   ram     a guest-RAM array symbol, or a local assigned only from such
    #           symbols (plus ++/--/+=): host compare is exact, kept;
    #   exact   a pointer global (its stored word) or a pointer parameter the
    #           leaf never modifies (pe_<name>): the retail guest value;
    #   other   anything else (a modified parameter, a pointer loaded from
    #           RAM, ...): reported.
    # A compare with an `exact` operand is rewritten to compare guest values
    # (a `ram` operand then uses PE_HostToGuest, exact inside RAM).
    body_for_rel = body_text if body_text is not None else stripped
    pparam_names = {p for p, _l, _t, _d in pointer_params(hdr.group(2))}
    arrays = {sym for sym, decl in externs if classify_data(decl, sym) == "array"
              and 0x80000000 <= int(sym[2:], 16) < 0x80200000}
    ptrglobals = {sym for sym, decl in externs if classify_data(decl, sym) == "ptrglobal"}

    def modified(n: str) -> bool:
        return bool(re.search(rf"(?<![.>\w])\b{re.escape(n)}\s*(=(?!=)|\+=|-=|\+\+|--)|"
                              rf"(\+\+|--)\s*{re.escape(n)}\b", body_for_rel))

    def op_class(n: str) -> str:
        if n in arrays:
            return "ram"
        if n in ptrglobals:
            return "exact"
        if n in pparam_names:
            return "other" if modified(n) else "exact"
        if n in ptr_locals:
            rhs = re.findall(rf"(?<![.>\w])\b{re.escape(n)}\s*=(?!=)\s*([^;]+);", body_for_rel)
            if rhs and all(re.match(r"\s*(?:\([^()]*\)\s*)?&?\s*(D_[0-9A-Fa-f]{7,8})", r)
                           and re.match(r"\s*(?:\([^()]*\)\s*)?&?\s*(D_[0-9A-Fa-f]{7,8})", r).group(1) in arrays
                           for r in rhs):
                return "ram"
        return "other"

    rel_rewrites: list[tuple[str, str, str, str, str]] = []
    rel_roots = ptr_roots | pparam_names
    for rm in re.finditer(r"(?<![<>\-=!])(<=|>=|<|>)(?![<>=])", stripped):
        left = re.search(r"([A-Za-z_]\w*)\s*$", stripped[:rm.start()])
        right = re.match(r"\s*([A-Za-z_]\w*)\b(?!\s*[\[(]|\s*->|\s*\.)",
                         stripped[rm.end():])
        if not left or not right:
            continue
        l, r = left.group(1), right.group(1)
        if l not in rel_roots or r not in rel_roots:
            continue
        cl, cr = op_class(l), op_class(r)
        if cl == "ram" and cr == "ram":
            continue
        tail = stripped[rm.end() + right.end():].lstrip()
        whole = (tail[:1] in (")", ";", ",") or tail[:2] in ("&&", "||")) and \
            re.search(r"[(&|;,!{]\s*$|\breturn\s*$|^\s*$",
                      stripped[:left.start()].rstrip()[-6:] or "")
        if "other" in (cl, cr) or not whole:
            return {"eligible": False,
                    "reason": f"relational compare of pointers {l} {rm.group(1)} {r} "
                              "(host order differs from guest order)"}

        def guest(n: str, c: str) -> str:
            if n in ptrglobals:
                return f"PE_LoadU32(0x{n[2:]}u)"
            if n in pparam_names:
                return f"pe_{n}"
            if n in arrays:
                return f"(pe_addr_t)0x{n[2:]}u"
            return f"PE_HostToGuest({n})"
        rel_rewrites.append((l, rm.group(1), r, guest(l, cl), guest(r, cr)))

    # Step 3: guest-call arguments are guest registers.  `&D_X`, `&D_X[k]`,
    # array/pointer-global `D_X + k` become the retail guest address in
    # render (fix_argument_addresses); any other host-pointer expression has
    # no guest meaning and is reported.
    for gm in re.finditer(r"\bPE_GuestCall\(", stripped):
        close = _balanced_end(stripped, gm.end() - 1)
        gargs = split_top(stripped[gm.end(): close - 1])[3:]
        for ga in gargs:
            inner = re.sub(r"^\(uintptr_t\)\s*", "", ga.strip())
            inner = _strip_parens(inner)
            if inner in ("", "0"):
                continue
            if address_arg(inner, {d for d, _ in externs}, dict(externs)) is not None:
                continue
            if re.fullmatch(r"D_[0-9A-Fa-f]{7,8}", inner):
                continue            # bare symbol: fix_argument_addresses
            if inner in {pp for pp, _l, _t, _d in pointer_params(hdr.group(2))}:
                continue            # E7b forwards the parameter's guest address
            if is_pointer_expr(inner, ptr_roots):
                return {"eligible": False,
                        "reason": f"host pointer {inner[:32]} passed to a guest call"}

    data_types = {sym: decl for sym, decl in externs}
    d_syms_used = [s for s in data_types
                   if re.search(rf"\b{re.escape(s)}\b", stripped)]
    return {
        "eligible": True,
        "params": params,
        "header": hdr.group(0),
        "externs": externs,
        "data_types": data_types,
        "d_syms_used": d_syms_used,
        "callees": callees,
        "boundaries": boundaries,
        "boundary_arity": boundary_arity,
        "shadowed": [s for s, _ in externs if s in existing_macros],
        "body": src_text,
        "wrap": wrap,
        "param_inner": hdr.group(2) if wrap else "",
        "ptr_ret": ptr_ret,
        "sign_adapt": sign_adapt,
        "canon_sig": sigs.get(name),
        "header_params": hdr.group(2),
        "hostptr_args": sorted(hostptr_args),
        "rel_rewrites": rel_rewrites,
    }


def neutralize_declarations(body: str, shimmed_data: set[str],
                            boundary_funcs: set[str],
                            canonical: set[str]) -> str:
    """Comment out leaf declarations that generated macros / pc_port now own.

    A `#define D_800942E4 ...` would rewrite the leaf's own `extern` into
    nonsense, so those declarations are replaced with a provenance comment.
    Likewise a callee the port links against a canonical `pe_port_compat.h`
    prototype, or a loud boundary: keeping the leaf's redeclaration risks a
    signedness/arity clash with the host type, and `src/` stays authoritative
    for the call site, not the host prototype.  A callee that is itself a
    decomp-derived leaf keeps the leaf's own prototype verbatim (there is no
    canonical host header for it, and the prototype is part of the matched
    unit).  Everything else (typedefs, structs, unrelated prototypes) stays
    byte-for-byte.
    """
    # A file-scope prototype that shares its line with an earlier declaration
    # (`extern int D_X; extern void func_Y(int a);`, src/func_80050308.c) is
    # invisible to the line-anchored patterns below and would clash with the
    # canonical host prototype: give it its own line (whitespace only).
    body = re.sub(r"(?m)^([^\n{}]*;)[ \t]+(?=(?:extern[ \t]+)?[A-Za-z_][A-Za-z0-9_ \t\*]*?"
                  r"\bfunc_[0-9A-Fa-f]{8}[ \t]*\([^\n;{]*?\)[ \t]*;[ \t]*$)",
                  r"\1\n", body)

    def names_in(text: str) -> list[str]:
        return DATA_NAME_RE.findall(text)

    def mention(text: str) -> bool:
        if DATA_NAME_RE.search(text):
            return True
        for f in FUNC_NAME_RE.findall(text):
            if f in boundary_funcs or f in canonical:
                return True
        # A bare prototype of a decomp-derived leaf is the callee's host
        # prototype (there is no canonical header for it); keep it verbatim.
        return False

    patterns = [
        EXTERN_RE,
        # A bare file-scope prototype of a symbol the port already provides
        # (`func_80050708`)'s `void func_80064C54(int value);`) would clash with
        # the canonical pe_port_compat.h type; the matched call site is what
        # matters, so the redeclaration is replaced by a provenance comment.
        # A file-scope prototype ends its line with `;` (no initializer, no
        # statement text after it); `return f(...);` and other call statements
        # must not be mistaken for a declaration.
        re.compile(
            r"^(?:extern\s+)?[A-Za-z_][A-Za-z0-9_ \t\*]*?"
            r"\b(func_[0-9A-Fa-f]{8})\s*\([^\n;{]*?\)\s*;[ \t]*$",
            re.MULTILINE,
        ),
    ]
    spans: list[tuple[int, int, str]] = []
    for pat in patterns:
        for m in pat.finditer(body):
            decl = m.group(1)
            if mention(decl):
                spans.append((m.start(), m.end(), decl))
    spans.sort()
    out: list[str] = []
    pos = 0
    for start, end, decl in spans:
        if start < pos:
            continue
        out.append(body[pos:start])
        names = list(dict.fromkeys(
            names_in(decl) + FUNC_NAME_RE.findall(decl)))
        out.append("/* shimmed by pe_guest_decomp.h: " + ", ".join(names) + " */")
        pos = end
    out.append(body[pos:])
    return "".join(out)


def guest_address_expr(sym: str, decl: str, index: str | None,
                       amp: bool) -> str | None:
    """Guest-address expression for a data symbol used as a call argument.

    `&D_X` -> the symbol's address; `&D_X[k]` / array `D_X + k` -> its k-th
    element; pointer-global `D_X + k` -> the stored guest pointer plus k
    elements.  A scalar `D_X + k` is arithmetic on the value, not an address,
    so it is left alone (None).
    """
    kind = classify_data(decl, sym)
    addr = f"0x{sym[2:]}u"
    if amp:
        if index is None:
            return f"(pe_addr_t){addr}"
        if kind != "array":
            return None
        etype = declared_type(decl)
        return f"(pe_addr_t)({addr} + (uint32_t)({index.strip()}) * sizeof({etype}))"
    if kind == "array":
        etype = declared_type(decl)
        return f"(pe_addr_t)({addr} + (uint32_t)({index.strip()}) * sizeof({etype}))"
    if kind == "ptrglobal":
        pointee = pointer_pointee(declared_type(decl))
        size = "1u" if pointee in ("void", "") else f"sizeof({pointee})"
        return (f"(pe_addr_t)(PE_LoadU32({addr}) + "
                f"(uint32_t)({index.strip()}) * {size})")
    return None


def fix_argument_addresses(body: str, data_syms: set[str],
                           decls: dict[str, str] | None = None,
                           host_ptr_params: dict[str, set[int]] | None = None) -> str:
    """Pass guest *addresses* where a bare data symbol is a call argument.

    In the matched leaf `f(arg0, D_800E0824)` the array decays to the retail
    guest address.  Host-side the macro expands to a host pointer, which is not
    a `pe_addr_t`; substituting the guest address literal keeps the argument
    semantics (and the call arity) retail-accurate.
    """
    def repl(m: re.Match) -> str:
        sym = m.group(2)
        if sym not in data_syms:
            return m.group(0)
        kind = classify_data(decls[sym], sym) if decls and sym in decls \
            else "array"
        if kind == "scalar":
            # A bare scalar argument is its *value* (retail `lw`/`lh`/`lbu`
            # of the symbol), which the SCALAR macro already yields.  Passing
            # the address here handed func_80073E10 0x80095884 instead of
            # D_80095884's contents (func_80077404).
            return m.group(0)
        if kind == "ptrglobal":
            # A bare pointer global passes the guest address *stored in* the
            # slot (retail `lui`/`lw`), not the slot's own address.
            return f"{m.group(1)}(pe_addr_t)PE_LoadU32(0x{sym[2:]}u)"
        return f"{m.group(1)}(pe_addr_t)0x{sym[2:]}u"

    # The closing `,`/`)` is a lookahead: consuming it made the next of two
    # consecutive bare-symbol arguments unmatchable (room func_8018F09C:
    # `func_800C2758(o, D_8018FFF8, D_8019001C)` rewrote only the first).
    body = re.sub(r"([(,]\s*)(D_[0-9A-Fa-f]{7,8})(?=\s*[,)])",
                  repl, body)
    if not decls:
        return body
    body = rewrite_guest_call_address_args(body, data_syms, decls)
    return rewrite_call_address_args(body, data_syms, decls, host_ptr_params or {})


def rewrite_guest_call_address_args(body: str, data_syms: set[str],
                                    decls: dict[str, str]) -> str:
    """`PE_GuestCall(site, fn, n, (uintptr_t)(&D_X), ...)`: the register
    value retail passes is the guest address of D_X."""
    out = []
    i = 0
    for m in re.finditer(r"\bPE_GuestCall\(", body):
        if m.start() < i:
            continue
        close = _balanced_end(body, m.end() - 1)
        args = split_top(body[m.end(): close - 1])
        new_args = args[:3]
        for a in args[3:]:
            inner = _strip_parens(re.sub(r"^\(uintptr_t\)\s*", "", a.strip()))
            e = address_arg(inner, data_syms, decls)
            new_args.append(f"(uintptr_t)({e})" if e else a.strip())
        out.append(body[i:m.start()])
        out.append("PE_GuestCall(" + ", ".join(x.strip() for x in new_args) + ")")
        i = close
    out.append(body[i:])
    return "".join(out)


ARG_AMP_FULL_RE = re.compile(
    r"&\s*(D_[0-9A-Fa-f]{7,8})\s*(?:\[([^\[\](),]+)\])?")
ARG_PLUS_FULL_RE = re.compile(
    r"(D_[0-9A-Fa-f]{7,8})\s*\+\s*([A-Za-z_0-9x ]+?|\([^()]*\))")


def address_arg(arg: str, data_syms: set[str], decls: dict[str, str]) -> str | None:
    """Guest-address form of one *whole* call argument, or None.

    Only an argument that is exactly `&D_X`, `&D_X[k]` or `D_X + k` (array or
    pointer-global D_X) qualifies — never a sub-expression, so a pointer cast
    such as `*(T *)(D_X + 4)` elsewhere in the body is untouched.
    """
    text = arg.strip()
    while text.startswith("(") and text.endswith(")") and \
            split_top(text[1:-1]) == [text[1:-1].strip()] and \
            _balanced(text[1:-1]):
        text = text[1:-1].strip()
    m = ARG_AMP_FULL_RE.fullmatch(text)
    if m:
        sym, idx, amp = m.group(1), m.group(2), True
    else:
        m = ARG_PLUS_FULL_RE.fullmatch(text)
        if not m:
            return None
        sym, idx, amp = m.group(1), m.group(2), False
    if sym not in data_syms or sym not in decls:
        return None
    return guest_address_expr(sym, decls[sym], idx, amp)


def _balanced(text: str) -> bool:
    depth = 0
    for ch in text:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth < 0:
                return False
    return depth == 0


# Host-signature context for rewrite_call_address_args, refreshed by main()
# on every round (the canonical headers + generated TUs, and the set of
# callees with any pc_port definition).
_ADDR_SIGS: dict[str, dict] = {}
_ADDR_KNOWN: set[str] = set()


def guest_address_param(callee: str, idx: int) -> bool:
    """Does the host callee take a guest address (not a host pointer) here?

    A loud boundary records guest arguments; a canonical / generated host
    signature says so per parameter.  A pc_port definition with no visible
    host signature keeps the leaf's own (retail, pointer-typed) prototype, so
    it is left alone and the compile gate judges it.
    """
    if callee in _ADDR_SIGS:
        params = _ADDR_SIGS[callee]["params"]
        return idx < len(params) and not is_host_pointer_type(params[idx])
    return callee not in _ADDR_KNOWN


def rewrite_call_args(body: str, fn) -> str:
    """Rewrite whole `func_*` call arguments: `fn(callee, idx, arg)` returns a
    replacement or None.  Calls are visited innermost-last-first; when an
    inner call changes length, every enclosing call's end offset is shifted
    so the outer argument split stays on the edited text."""
    calls = []
    for m in CALL_HEAD_RE.finditer(body):
        i = m.end()
        depth = 1
        while i < len(body) and depth:
            if body[i] == "(":
                depth += 1
            elif body[i] == ")":
                depth -= 1
            i += 1
        if depth == 0:
            calls.append([m.group(1), m.end(), i - 1])
    calls.sort(key=lambda c: c[1], reverse=True)
    for n, (callee, a, b) in enumerate(calls):
        args = split_top(body[a:b])
        if not args:
            continue
        new_args = []
        changed = False
        for idx, arg in enumerate(args):
            e = fn(callee, idx, arg)
            if e is not None:
                new_args.append(e)
                changed = True
            else:
                new_args.append(arg)
        if changed:
            text = ", ".join(new_args)
            delta = len(text) - (b - a)
            body = body[:a] + text + body[b:]
            for outer in calls[n + 1:]:
                if outer[1] <= a and outer[2] >= b:
                    outer[2] += delta
    return body


def rewrite_call_address_args(body: str, data_syms: set[str],
                              decls: dict[str, str],
                              host_ptr_params: dict[str, set[int]]) -> str:
    """Apply address_arg to every whole argument of every `func_*` call whose
    host parameter takes a guest address (guest_address_param)."""
    return rewrite_call_args(
        body, lambda callee, idx, arg:
        address_arg(arg, data_syms, decls)
        if guest_address_param(callee, idx) else None)


CODE_CAST_RE = re.compile(
    r"\(\s*(unsigned\s+int|unsigned\s+long|unsigned|int|long|"
    r"u32|s32|uint32_t|int32_t)\s*\)\s*&?\s*(func_[0-9A-Fa-f]{8})\b(?!\s*\()")
BARE_CODE_ARG_RE = re.compile(r"\(*\s*&?\s*(func_[0-9A-Fa-f]{8})\s*\)*")


FNPTR_DECL_RE = re.compile(
    r"(?P<pre>(?:\b(?:extern|static|register|const|volatile)\s+)*)"
    r"(?P<ret>[A-Za-z_][\w \t]*?[\s\*]*)"
    r"\(\s*\*\s*(?:(?:const|volatile)\s+)*(?P<name>[A-Za-z_]\w*)\s*\)"
    r"\s*\((?P<params>[^()]*)\)")
FNPTR_CAST_RE = re.compile(r"\(\s*[A-Za-z_][\w \t\*]*?\(\s*\*\s*\)\s*\([^()]*\)\s*\)")
FNPTR2_CAST_RE = re.compile(r"\(\s*[A-Za-z_][\w \t\*]*?\(\s*\*\s*\*\s*\)\s*\([^()]*\)\s*\)")
FNPTR_TYPEDEF_RE = re.compile(
    r"\btypedef\s+[A-Za-z_][\w \t\*]*?\(\s*\*\s*(?P<name>[A-Za-z_]\w*)\s*\)\s*\([^()]*\)\s*;")


def rewrite_guest_calls(text: str, leaf: str) -> tuple[str, str]:
    """Calls through guest code pointers become PE_GuestCall (step 3).

    On the PS1 a function pointer is a 32-bit guest code address.  Every
    function-pointer declarator (`R (*f)(P)` — locals, parameters, pointer
    globals, struct members, prototype parameters) becomes `pe_addr_t f`,
    every function-pointer cast becomes an integer cast (`(R (*)(P))x` ->
    `(pe_addr_t)x`, `(R (**)(P))p` -> `(pe_addr_t *)p`, so a slot is read as
    the 32-bit guest word), and every call through such a value becomes
    `PE_GuestCall("<leaf>:<callee>", addr, n, a0..a3)`, which dispatches to
    the resolved host implementation or records a loud boundary
    (pc_port/platform/pe_guestcode.h).  Returns (text, reject_reason)."""
    names: set[str] = set()
    tdefs: set[str] = set()

    def tdef(m: re.Match) -> str:
        tdefs.add(m.group("name"))
        return f"typedef pe_addr_t {m.group('name')};"
    text = FNPTR_TYPEDEF_RE.sub(tdef, text)

    def decl(m: re.Match) -> str:
        names.add(m.group("name"))
        pre = m.group("pre")
        return f"{pre}pe_addr_t {m.group('name')}"
    text = FNPTR_DECL_RE.sub(decl, text)
    text = FNPTR2_CAST_RE.sub("(pe_addr_t *)", text)
    text = FNPTR_CAST_RE.sub("(pe_addr_t)", text)
    for t in tdefs:
        for m in re.finditer(rf"\b{re.escape(t)}\s+\**\s*([A-Za-z_]\w*)", text):
            names.add(m.group(1))
    if not names and "pe_addr_t *)" not in text:
        return text, ""

    # Call sites: `(*f)(args)`, `f(args)` (f a guest code pointer), and a
    # parenthesised slot read `(*(pe_addr_t *)(p))(args)`.
    out = []
    i = 0
    pat = re.compile(
        r"\(\s*\*\s*(?P<a>[A-Za-z_]\w*)\s*\)\s*\("               # (*f)(
        r"|(?<![\w.>])(?P<m>[A-Za-z_]\w*(?:\s*(?:->|\.)\s*[A-Za-z_]\w*)+)\s*\("  # o->f(
        r"|(?<![\w.>])(?P<b>[A-Za-z_]\w*)\s*\("                        # f(
        r"|\(\s*\*\s*\(\s*pe_addr_t\s*\*\s*\)")                 # (*(pe_addr_t *)
    while True:
        m = pat.search(text, i)
        if not m:
            out.append(text[i:])
            break
        if m.group("m"):
            member = re.split(r"->|\.", m.group("m"))[-1].strip()
            if member not in names:
                out.append(text[i:m.end()])
                i = m.end()
                continue
            expr = re.sub(r"\s+", "", m.group("m"))
            out_args_open = m.end() - 1
            close_args = _balanced_end(text, out_args_open)
            args = [a for a in split_top(text[out_args_open + 1: close_args - 1]) if a.strip()]
            if len(args) > 4:
                return text, f"guest call with {len(args)} arguments (stack args)"
            pad = [f"(uintptr_t)({a.strip()})" for a in args] + ["0"] * (4 - len(args))
            out.append(text[i:m.start()])
            out.append(f'PE_GuestCall("@{leaf[5:]}:{member}", (pe_addr_t)({expr}), '
                       f'{len(args)}u, {", ".join(pad)})')
            i = close_args
            continue
        callee = m.group("a") or m.group("b")
        if m.group("b") and (callee not in names or is_declarator(text, m.start("b"))):
            out.append(text[i:m.end()])
            i = m.end()
            continue
        if callee:
            fn_expr = f"(pe_addr_t)({callee})"
            args_open = m.end() - 1
            site = f"@{leaf[5:]}:{callee}"   # no bare func_ token (E6)
        else:
            # `(*(pe_addr_t *)(EXPR))(args)`: the whole parenthesised callee
            close = _balanced_end(text, m.start())
            k = close
            while k < len(text) and text[k] in " \t\n":
                k += 1
            if k >= len(text) or text[k] != "(":
                out.append(text[i:m.end()])
                i = m.end()
                continue
            inner = text[m.start() + 1: close - 1].strip()
            fn_expr = f"(pe_addr_t)({inner})"
            args_open = k
            site = f"@{leaf[5:]}:indirect"
        close_args = _balanced_end(text, args_open)
        args = [a for a in split_top(text[args_open + 1: close_args - 1]) if a.strip()]
        if len(args) > 4:
            return text, f"guest call with {len(args)} arguments (stack args)"
        pad = [f"(uintptr_t)({a.strip()})" for a in args] + ["0"] * (4 - len(args))
        out.append(text[i:m.start()])
        out.append(f'PE_GuestCall("{site}", {fn_expr}, {len(args)}u, {", ".join(pad)})')
        i = close_args
    return "".join(out), ""


def rewrite_code_addresses(text: str) -> str:
    """A function's address used as a 32-bit *value* is its retail VMA.

    Two shapes are rewritten, both exactly what retail's `lui`/`addiu` pair
    materialises:

    * an integer cast `(unsigned int)func_X` / `(int)&func_X` (a callback
      stored into a guest word: `*(unsigned int *)(p + 0x30) = ...`) becomes
      `(unsigned int)0xXu`;
    * a bare `func_X` / `&func_X` passed as a whole call argument whose host
      parameter takes a guest address (a loud boundary, or a canonical /
      generated signature with a non-pointer parameter) becomes
      `(pe_addr_t)0xXu` — the convention hand adapters already use for
      code-pointer arguments (hand_lo_protos.h).

    Anything else (a `void *` store, a pointer-global assignment, a callee
    that takes a real host function pointer) is left alone and still falls
    under E6.
    """
    text = CODE_CAST_RE.sub(
        lambda m: f"({m.group(1)})0x{m.group(2)[5:].upper()}u", text)

    def arg(callee: str, idx: int, a: str) -> str | None:
        m = BARE_CODE_ARG_RE.fullmatch(a.strip())
        if m is None or m.group(1) == callee:
            return None
        if not guest_address_param(callee, idx):
            return None
        return f"(pe_addr_t)0x{m.group(1)[5:].upper()}u"
    return rewrite_call_args(text, arg)


def rect_array_args(body: str, sigs: dict) -> str:
    """A leaf-local `short NAME[4]` passed where the canonical host prototype
    takes `RECT *` / `const RECT *` (LoadImage/StoreImage/ClearImage-style
    callees): psx_compat.h's RECT is `{ short x, y, w, h; }`, the identical
    4 x 16-bit layout, so the argument is cast to it explicitly."""
    arrays = set(re.findall(r"\bshort\s+([A-Za-z_]\w*)\s*\[\s*4\s*\]", body))
    if not arrays:
        return body

    def fix(c, i, a):
        sig = sigs.get(c)
        if not sig:
            return None
        params = [p for p in sig["params"] if p.strip() not in ("", "void")]
        if i >= len(params):
            return None
        pt = _norm_type(_param_types([params[i]])[0])
        name = a.strip()
        if name in arrays and pt in ("RECT *", "const RECT *", "RECT*", "const RECT*"):
            return f"({pt})" + name
        return None
    return rewrite_call_args(body, fix)


def host_to_guest_args(body: str, plan: dict) -> str:
    """Arguments recorded by analyze() as host pointer locals passed where
    the callee takes a guest address: forward `PE_HostToGuest(p)`.  Also
    applies E19's pointer-compare rewrites (compare guest values)."""
    for l, op, r, lg, rg in plan.get("rel_rewrites", []):
        body = re.sub(rf"(?<![\w.>])\b{re.escape(l)}\s*{re.escape(op)}\s*{re.escape(r)}\b",
                      f"({lg} {op} {rg})", body)
    want = {tuple(x) for x in plan.get("hostptr_args", [])}
    if not want:
        return body

    def wrap(c, i, a):
        if (c, i) not in want:
            return None
        # (callee, index) is recorded from a host pointer local at ONE call
        # site; another call of the same callee may pass a call result there
        # (`f(t)` ... `f(g())`).  A call result is already a guest value
        # (pointer-returning functions return pe_addr_t) -- never wrap it.
        if re.match(r"\(*\s*func_[0-9A-Fa-f]{8}\s*\(", a.strip()):
            return None
        return f"PE_HostToGuest({a.strip()})"
    return rewrite_call_args(body, wrap)


def canonical_for(plan: dict, proto: dict[str, int]) -> set[str]:
    """Callees whose leaf declaration is replaced by a host prototype: every
    canonical header prototype, plus decomp-derived callees whose generated
    host signature differs from the leaf's own declaration."""
    derived = set(plan.get("derived_all", ()))
    return ({c for c in proto if c not in derived}
            | set(plan.get("host_override", ())))


def render(name: str, plan: dict, matched: dict, known_funcs: set[str],
           proto: dict[str, int]) -> str:
    leaf = matched[name]
    report = EVIDENCE_DIR / name / "REPORT.md"
    if plan["wrap"]:
        return render_wrapper(name, plan, leaf, report, proto)
    lines: list[str] = []
    lines.append(
        "/*\n"
        f" * decomp-source: {name}\n"
        f" * matched VMA 0x{leaf['vram']:08X}  file 0x{leaf['file_offset']:X}"
        f"  span 0x{leaf['file_size']:X} bytes / {leaf['words']} words\n"
        f" * evidence: {report.as_posix()}\n"
        " *\n"
        " * GENERATED by tools/analysis/gen_decomp_ports.py from"
        f" src/{name}.c — do not\n"
        " * edit by hand; the matching leaf is the authority.  The body is copied\n"
        " * verbatim; only the host adaptation (guest-RAM data symbols + loud\n"
        " * boundaries) is generated.  See docs/ai_context/PC_PORT_FROM_DECOMP.md.\n"
        " */\n"
    )
    lines.append('#include "pe_guest_decomp.h"\n')

    emitted_data = False
    for sym, decl in plan["externs"]:
        if sym in plan["shadowed"]:
            # The shim header may declare this address with a different host
            # type; the matched leaf's own declaration wins, so rebind it.
            lines.append(f"#undef {sym}")
        kind = classify_data(decl, sym)
        etype = declared_type(decl)
        if kind == "scalar":
            lines.append(f"#define {sym} PE_DECOMP_SCALAR(0x{sym[2:]}u, {etype})")
        elif kind == "array":
            lines.append(f"#define {sym} PE_DECOMP_ARRAY(0x{sym[2:]}u, {etype})")
        else:
            pointee = pointer_pointee(etype)
            macro = ("PE_DECOMP_PTRGLOBAL_NULLABLE"
                     if null_tested(plan["body"], sym) else "PE_DECOMP_PTRGLOBAL")
            lines.append(
                f"#define {sym} {macro}(0x{sym[2:]}u, {pointee})")
        emitted_data = True
    if emitted_data:
        lines.append("")

    for callee in plan["boundaries"]:
        lines.append(
            f"/* boundary: {callee} has no pc_port implementation yet */"
        )
        arity_n = plan["boundary_arity"][callee]
        if arity_n == 0:
            lines.append(
                f"#define {callee}(...) "
                f'PE_D_COMP_BOUNDARY0("{callee}", 0x{callee[5:]}u)'
            )
        else:
            lines.append(
                f"#define {callee}(...) "
                f'PE_D_COMP_BOUNDARY{arity_n}('
                f'"{callee}", 0x{callee[5:]}u, __VA_ARGS__)'
            )
        lines.append("")
    if plan["boundaries"]:
        lines.append("")

    shimmed_data = {s for s, _ in plan["externs"] if s not in plan["shadowed"]}
    body = neutralize_declarations(plan["body"].rstrip("\n"), shimmed_data,
                                   set(plan["boundaries"]),
                                   canonical_for(plan, proto))

    if plan.get("host_protos"):
        lines.append("/* host prototypes of decomp-derived callees (generated TUs) */")
        lines.extend(plan["host_protos"])
        lines.append("")

    lines.append("/* ── verbatim matching leaf (src/%s.c) ───────────────────────── */" % name)
    lines.append("")
    body = fix_argument_addresses(body, shimmed_data, plan["data_types"])
    body = host_to_guest_args(body, plan)
    body = rect_array_args(body, _ADDR_SIGS)
    body = empty_callee_args_drop(body, proto)
    if plan.get("ptr_ret"):
        body = rename_ptr_ret_definition(body, name)
        hp = plan["header_params"].strip()
        body += "\n" + ptr_return_thunk(
            name, hp if hp else "void", param_call_names(hp), ptr_roots_expr(plan))
    elif plan.get("sign_adapt"):
        body = rename_definition(body, name, f"pe_leaf_{name}")
        if plan["sign_adapt"]["ret"] == "void":
            body = e20e_void_return_rewrite(body, name, _ADDR_SIGS)
        body += "\n" + signedness_thunk(name, plan["sign_adapt"])
    lines.append(body + "\n")
    text = "\n".join(lines) + "\n"
    if plan.get("ptr_ret"):
        text = e20_over_export(text, name, plan.get("canon_sig"))
    return canonical_guest_call_args(text)


def param_base_type(decl: str, pname: str) -> str:
    """Base element type of a pointer parameter declaration.

    Handles `unsigned char *a0`, `short *a1 = x`, `unsigned char a0[4]`, and
    `int **a0`.  Array binders and initializers are dropped.
    """
    text = decl
    if pname:
        text = re.sub(rf"\b{re.escape(pname)}\b.*$", "", text)
    text = re.sub(r"\[[^\]]*\]", " ", text)
    text = re.sub(r"\*+", " ", text)
    text = re.sub(r"\b(const|volatile|register|restrict)\b", " ", text)
    return re.sub(r"\s+", " ", text).strip() or "unsigned char"


def normalize_base(decl: str) -> str:
    return re.sub(r"\s+", "", decl).replace("const", "").replace("volatile", "")


def wrapper_casts_ok(stripped: str, inner: str) -> tuple[bool, str]:
    """Every pointer cast applied directly to a pointer parameter must match
    the parameter's declared base type, so deleting the cast is a no-op."""
    for pname, level, _toks, decl in pointer_params(inner):
        if level != 1:
            continue
        pbase = normalize_base(re.sub(r"\*+\s*$", "", decl))
        for m in re.finditer(rf"\(([^()]*?\*+)\)\s*{re.escape(pname)}\b", stripped):
            cbase = normalize_base(re.sub(r"\*+\s*$", "", m.group(1)))
            if cbase and cbase != pbase:
                return False, f"cast {m.group(1)} on {pname} ({decl}) changes width"
    return True, ""


PTR_RET_RE = re.compile(
    r"(?:(?:const|volatile|unsigned|signed|struct|extern|static)\s+)*"
    r"[A-Za-z_]\w*(?:\s+(?:const|volatile|int|char|short|long))*"
    r"(?:\s*\*\s*(?:const|volatile)?)+")


def param_call_names(param_text: str, pointer_names: set[str] = frozenset()) -> str:
    """Argument list that forwards a parameter list unchanged (pointer
    parameters of an E7b wrapper are forwarded as `pe_<name>`)."""
    names, _ = parse_params(param_text)
    return ", ".join(f"pe_{n}" if n in pointer_names else n for n in names)


def null_tested(text: str, name: str) -> bool:
    """Does the leaf null-test `name` — directly, or through a plain copy
    (`p = name;`, `T *p = (T *)name;`)?  Tests recognised: `x == 0`,
    `x != 0`, `0 == x`, `!x`, `if (x)`, `while (x)`, `x &&`, `x ||`, `x ?`,
    `&& x)`, `|| x)`.  Only such names get the guest-0 -> host-NULL mapping;
    every other pointer translates 0 through the retail RAM mirror
    (portverify: 71 generated TUs segfaulted on a blanket NULL mapping where
    retail reads low RAM)."""
    text = strip_comments(text)
    aliases = {name}
    changed = True
    while changed:
        changed = False
        for a in list(aliases):
            for m in re.finditer(
                    rf"\b([A-Za-z_]\w*)\s*=\s*(?:\([^()]*\)\s*)?{re.escape(a)}\s*[;,)]",
                    text):
                if m.group(1) not in aliases:
                    aliases.add(m.group(1))
                    changed = True
    for a in aliases:
        e = re.escape(a)
        pats = [
            rf"(?<![\*.>])(?<!\*\s)\b{e}\s*[!=]=\s*(?:\(\s*[\w\s\*]+\)\s*)?(?:0[xX]?0*|NULL)\b",
            rf"\b(?:0|NULL)\s*[!=]=\s*{e}\b",
            rf"!(?!=)\s*{e}\b(?!\s*(?:\[|\.|->|\())",
            rf"\b(?:if|while)\s*\(\s*{e}\s*\)",
            rf"\b{e}\s*(?:&&|\|\||\?)",
            rf"(?:&&|\|\|)\s*{e}\s*(?:\)|&&|\|\||;)",
        ]
        if any(re.search(p, text) for p in pats):
            return True
    return False


def return_root(text: str, roots: set[str]) -> str | None:
    """The single pointer root every `return` of the leaf derives from,
    following plain copies (`d = a0;`, `register int *d = a0;`), or None.
    `return 0;` / `return NULL;` agree with any root."""
    text = strip_comments(text)
    alias: dict[str, str] = {}
    for m in re.finditer(r"\b([A-Za-z_]\w*)\s*=\s*(?:\([^()]*\)\s*)?([A-Za-z_]\w*)\s*[;,)]",
                         text):
        alias.setdefault(m.group(1), m.group(2))

    def resolve(n: str) -> str:
        seen = set()
        while n in alias and n not in roots and n not in seen:
            seen.add(n)
            n = alias[n]
        return n
    found: set[str] = set()
    for m in re.finditer(r"\breturn\b([^;]*);", text):
        e = _strip_parens(m.group(1))
        if e in ("", "0", "NULL") or re.fullmatch(r"\(\s*\w[\w\s\*]*\)\s*0", e):
            continue
        e = re.sub(r"^\(\s*[A-Za-z_][\w\s]*\*+\s*\)\s*", "", e)   # pointer cast
        hit = None
        for t in _top_terms(e):
            t2 = _strip_parens(t).lstrip("&")
            mm = re.match(r"[A-Za-z_]\w*", t2)
            if mm and resolve(mm.group(0)) in roots:
                hit = resolve(mm.group(0))
                break
        if hit is None:
            return None
        found.add(hit)
    return found.pop() if len(found) == 1 else None


def ptr_roots_expr(plan: dict, pparams_names: set[str] | None = None) -> list[str]:
    """Guest-address expression(s) of the pointer root(s) a pointer-return
    leaf's result derives from, for PE_DecompReturnSegment.  With several
    roots, the one every `return` traces back to (through plain copies) is
    chosen (func_80078C94 `d = a0; return d;`, func_80072334 `return dst;`);
    otherwise all roots are returned and the thunk leaves the result KSEG0."""
    exprs = {sym: f"PE_LoadU32(0x{sym[2:]}u)" for sym in plan.get("d_syms_used", [])
             if classify_data(plan["data_types"][sym], sym) == "ptrglobal"}
    exprs.update({p: f"pe_{p}" for p in sorted(pparams_names or ())})
    if len(exprs) > 1:
        r = return_root(plan.get("body", ""), set(exprs))
        if r is not None:
            return [exprs[r]]
    return list(exprs.values())


# E20 — 32-bit integer class.  On the host these are all exactly 32 bits, so a
# conversion between any two of them is the identity on the retail register
# value (`long` is excluded: 64-bit on LP64 hosts; narrower types are excluded
# because the conversion would truncate where retail's callee did not).
INT32_CLASS = {
    "int", "signed", "signed int", "unsigned", "unsigned int", "int32_t",
    "uint32_t", "pe_addr_t", "s32", "u32",
}


def _norm_type(t: str) -> str:
    return re.sub(r"\s+", " ", re.sub(r"^\s*extern\s+", "", t)).strip()


def signedness_adapter(leaf_ret: str, leaf_params: str, canon: dict):
    """E20: the leaf's own name has a canonical pc_port header prototype.

    Returns None when the two signatures already agree (the verbatim
    definition compiles against the header), a dict describing the adapter
    when they differ only by 32-bit integer signedness (or the header returns
    `void` where the leaf returns a 32-bit integer — callers cannot read a
    value the host prototype does not return), and "reject" otherwise.
    """
    lret = _norm_type(leaf_ret)
    cret = _norm_type(canon["ret"])
    lp = _param_types(split_top(leaf_params.strip())) if leaf_params.strip() not in ("", "void") else []
    cparams = [p for p in canon["params"] if p.strip() not in ("", "void")]
    cp = _param_types(cparams)
    if lret == cret and lp == cp:
        return None
    if (len(cp) > len(lp) and cp[:len(lp)] == lp and
            (lret == cret or (cret == "void" and lret in INT32_CLASS))):
        # E20b: the header carries extra TRAILING parameters the matched leaf
        # never takes (e.g. the field-VM `pe_addr_t args` handler form over a
        # `(void)` leaf).  Retail's callee ignores the extra argument
        # registers, so dropping them is the identity.  Only with exactly
        # matching leading parameters and an identical return type -- or, as
        # E20 already allows, a `void` header discarding a 32-bit result
        # (host callers cannot read a value the prototype does not return).
        return {"leaf_ret": lret, "ret": cret, "leaf_params": lp, "params": cp,
                "dropped": len(cp) - len(lp)}
    if len(lp) != len(cp):
        return "reject"
    for a, b in zip(lp, cp):
        if a != b and not (a in INT32_CLASS and b in INT32_CLASS):
            return "reject"
    if lret != cret:
        if cret == "void" and lret in INT32_CLASS:
            pass
        elif not (lret in INT32_CLASS and cret in INT32_CLASS):
            return "reject"
    return {"leaf_ret": lret, "ret": cret, "leaf_params": lp, "params": cp}


def rename_definition(body: str, name: str, new_name: str) -> str:
    """`T func_X(...) {` -> `static T <new_name>(...) {` (definition only)."""
    for m in HDR_RE.finditer(body):
        if m.group(1) != name:
            continue
        head = m.group(0)
        lead = len(head) - len(head.lstrip())
        new = head[:lead] + "static " + head[lead:]
        new = re.sub(rf"\b{re.escape(name)}(?=\s*\()", new_name, new, count=1)
        return body[:m.start()] + new + body[m.end():]
    raise AssertionError(f"no definition header for {name}")


# E20e: leaves whose result is PROVEN unobservable (no port caller reads it and
# $v0 is dead after every retail call site), each with its evidence file.  For
# these only, `return <call>;` where the callee's canonical prototype is `void`
# becomes `<call>; return 0;` inside the static leaf.  Not a general rewrite.
E20E_PROVEN = {
    "func_800453E8": "docs/evidence/pc-port-switchover/E20e-func_800453E8.md",
}


# E20f: callees whose retail body is PROVEN empty (a bare `jr ra` + nop: no
# argument-register, stack or memory read, no $v0 write), each with its
# disassembly evidence.  A matched leaf may call one with arguments (retail
# loads $a0 before the jal) while the canonical prototype takes none; only
# integer-literal arguments are dropped, so nothing with a side effect is
# lost.  Anything else still fails the call-arity check.
EMPTY_CALLEES = {
    "func_800527C0": "docs/evidence/pc-port-switchover/E20f-empty-callees.md",
}
_E20F_LITERAL_RE = re.compile(r"\s*-?(?:0[xX][0-9A-Fa-f]+|\d+)[uUlL]*\s*")


def _split_top_args(args: str) -> list[str]:
    out, depth, cur = [], 0, []
    for ch in args:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            out.append("".join(cur)); cur = []
        else:
            cur.append(ch)
    out.append("".join(cur))
    return out


def empty_callee_droppable(callee: str, args: str, proto: dict) -> bool:
    return (callee in EMPTY_CALLEES and proto.get(callee) == 0 and arity(args) > 0
            and all(_E20F_LITERAL_RE.fullmatch(a) for a in _split_top_args(args)))


def empty_callee_args_drop(body: str, proto: dict) -> str:
    """E20f rewrite: `func_800527C0(2)` -> `func_800527C0() /* E20f: ... */`."""
    out, pos = [], 0
    for m in CALL_HEAD_RE.finditer(body):
        callee = m.group(1)
        if m.start() < pos or callee not in EMPTY_CALLEES or is_declarator(body, m.start()):
            continue
        close = _balanced_end(body, m.end() - 1)
        args = body[m.end():close - 1]
        if not empty_callee_droppable(callee, args, proto):
            continue
        out.append(body[pos:m.start()])
        out.append(f"{callee}() /* E20f: retail arg ({args.strip()}) unread; "
                   f"{EMPTY_CALLEES[callee]} */")
        pos = close
    out.append(body[pos:])
    return "".join(out)


def e20e_void_return_rewrite(body: str, name: str, sigs: dict) -> str:
    if name not in E20E_PROVEN:
        return body
    out, pos = [], 0
    for m in re.finditer(r"\breturn\s+(func_[0-9A-Fa-f]{8})\s*\(", body):
        callee = m.group(1)
        if m.start() < pos or _norm_type(sigs.get(callee, {}).get("ret", "")) != "void":
            continue
        close = _balanced_end(body, m.end() - 1)
        semi = re.match(r"\s*;", body[close:])
        if not semi:
            continue
        call = body[m.start() + len("return"):close].strip()
        out.append(body[pos:m.start()])
        out.append(f"{call}; return 0; /* E20e: {E20E_PROVEN[name]} */")
        pos = close + semi.end()
    out.append(body[pos:])
    return "".join(out)


def signedness_thunk(name: str, ad: dict) -> str:
    """The exported half of the E20 adapter: the canonical host signature,
    forwarding to the verbatim leaf with same-width integer casts."""
    formals = ", ".join(f"{t} pe_a{i}" for i, t in enumerate(ad["params"])) or "void"
    actuals = ", ".join(f"({t})pe_a{i}" for i, t in enumerate(ad["leaf_params"]))
    call = f"pe_leaf_{name}({actuals})"
    if ad.get("dropped"):
        n = len(ad["leaf_params"])
        unused = "".join(f"    (void)pe_a{i};\n" for i in range(n, len(ad["params"])))
        head = ("\n/* E20b trailing-parameter adapter: the canonical pc_port prototype\n"
                " * passes arguments this leaf never takes; retail ignores those\n"
                " * argument registers. */\n"
                f"{ad['ret']} {name}({formals})\n{{\n{unused}")
    else:
        head = ("\n/* E20 signedness adapter: the canonical pc_port prototype types this\n"
                " * leaf's 32-bit integers differently; the casts are the identity on\n"
                " * the retail register value. */\n"
                f"{ad['ret']} {name}({formals})\n{{\n")
    if ad["ret"] == "void":
        stmt = f"    (void){call};\n" if ad["leaf_ret"] != "void" else f"    {call};\n"
    else:
        stmt = f"    return ({ad['ret']}){call};\n"
    return head + stmt + "}\n"


def ptr_return_thunk(name: str, sig_params: str, call_args: str,
                     roots: list[str] | None = None) -> str:
    """The exported half of the pointer-return adapter.  With exactly one
    pointer root, its KUSEG/KSEG1 segment is re-applied to the result
    (PE_DecompReturnSegment); the root is read before the leaf runs (E12
    forbids pointer-global stores, and a parameter is a value)."""
    head = (f"\n/* pointer-return adapter: retail returns a 32-bit guest address */\n"
            f"pe_addr_t {name}({sig_params})\n{{\n")
    if roots and len(roots) == 1:
        return (head + f"    pe_addr_t pe_root = {roots[0]};\n"
                f"    return PE_DecompReturnSegment(PE_HostToGuest(pe_host_{name}({call_args})), pe_root);\n}}\n")
    return head + f"    return PE_HostToGuest(pe_host_{name}({call_args}));\n}}\n"


def rename_ptr_ret_definition(body: str, name: str) -> str:
    """`T *func_X(...) {` -> `static T *pe_host_func_X(...) {`."""
    for m in HDR_RE.finditer(body):
        if m.group(1) != name:
            continue
        head = m.group(0)
        lead = len(head) - len(head.lstrip())
        new = head[:lead] + "static " + head[lead:]
        new = re.sub(rf"\b{re.escape(name)}(?=\s*\()", f"pe_host_{name}", new, count=1)
        return body[:m.start()] + new + body[m.end():]
    raise AssertionError(f"no definition header for {name}")


PTRPTR_CAST_RE = re.compile(
    r"\(\s*(?:(?:const|volatile|unsigned|signed|struct)\s+)*[A-Za-z_]\w*"
    r"(?:\s+(?:const|volatile|int|char|short|long))*\s*\*\s*(?:const|volatile)?"
    r"\s*\*[\s\*]*\)")
PTR_DECL_RE = re.compile(
    r"(?:^|[;{}(,])\s*(?:(?:const|volatile|unsigned|signed|struct|register|"
    r"static|extern)\s+)*[A-Za-z_]\w*(?:\s+(?:int|char|short|long))*"
    r"\s*(\**)\s*(?:(?:const|volatile)\s+)*([A-Za-z_]\w*)\s*(\[)?")


def pointer_typed_names(text: str) -> set[str]:
    """Identifiers declared (anywhere in the leaf) with a pointer or array
    type: `unsigned char *p`, `Rec *r = ...`, `short buf[8]`.  A name that is
    also declared with a non-pointer type is excluded (ambiguous → not a
    pointer root)."""
    ptr: set[str] = set()
    other: set[str] = set()
    keywords = {"return", "if", "while", "for", "switch", "case", "else",
                "do", "goto", "sizeof"}
    for m in PTR_DECL_RE.finditer(text):
        name = m.group(2)
        head = m.group(0)
        first = re.match(r"[;{}(,]?\s*(\w+)", head)
        if first and first.group(1) in keywords:
            continue
        tail = text[m.end():].lstrip()
        if tail.startswith("(") and not m.group(3):
            continue            # a function declarator, not an object
        stars = len(m.group(1))
        if (stars == 1 and not m.group(3)) or (stars == 0 and m.group(3)):
            ptr.add(name)
        else:
            other.add(name)
    return ptr - other


PTR1_CAST_RE = re.compile(
    r"\(\s*(?:(?:const|volatile|unsigned|signed|struct)\s+)*[A-Za-z_]\w*"
    r"(?:\s+(?:const|volatile|int|char|short|long))*\s*\*\s*"
    r"(?:(?:const|volatile)\s*)?\)")


def _balanced_end(text: str, i: int) -> int:
    """Index just past the bracket group opening at text[i]."""
    pairs = {"(": ")", "[": "]"}
    close = pairs[text[i]]
    depth = 0
    j = i
    while j < len(text):
        if text[j] == text[i]:
            depth += 1
        elif text[j] == close:
            depth -= 1
            if depth == 0:
                return j + 1
        j += 1
    return len(text)


def cast_operand(text: str, i: int) -> str:
    """The unary-expression operand of a cast ending at text[i]."""
    j = i
    while j < len(text) and text[j] in " \t\n":
        j += 1
    start = j
    while j < len(text) and text[j] in "&*-!~ \t":
        j += 1
    if j < len(text) and text[j] == "(":
        j = _balanced_end(text, j)
        # a nested cast `(T)(x)` / `(T)x`: include its operand too
        inner = text[start:j].lstrip("&*-!~ \t")
        if re.fullmatch(r"\(\s*[A-Za-z_][\w \t]*\**\s*\)", inner):
            return text[start:j] + cast_operand(text, j)
    else:
        m = re.match(r"[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|\"[^\"]*\"", text[j:])
        if not m:
            return text[start:j]
        j += m.end()
    while j < len(text):
        k = j
        while k < len(text) and text[k] in " \t":
            k += 1
        if k < len(text) and text[k] in "[(":
            j = _balanced_end(text, k)
        elif text.startswith("->", k) or (k < len(text) and text[k] == "."):
            m = re.match(r"(?:->|\.)\s*\w+", text[k:])
            j = k + m.end() if m else k + 1
        else:
            break
    return text[start:j]


def _strip_parens(e: str) -> str:
    e = e.strip()
    while e.startswith("(") and _balanced_end(e, 0) == len(e):
        e = e[1:-1].strip()
    return e


def _top_terms(e: str) -> list[str]:
    """Split on top-level binary + / -."""
    terms, depth, cur, prev = [], 0, "", ""
    for idx, ch in enumerate(e):
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        nxt = e[idx + 1] if idx + 1 < len(e) else ""
        if depth == 0 and ch in "+-" and nxt not in (">", ch) and \
                prev not in ("+", "-") and prev and (prev.isalnum() or prev in ")]_"):
            terms.append(cur)
            cur = ""
            prev = ""
            continue
        cur += ch
        if not ch.isspace():
            prev = ch
    terms.append(cur)
    return [t for t in (x.strip() for x in terms) if t]


def is_pointer_expr(expr: str, roots: set[str]) -> bool:
    """Does `expr` evaluate to a *host pointer into guest RAM* in the generated
    TU?  True for `&lvalue`, a nested pointer cast (checked on its own), a
    bare pointer-root identifier, or pointer arithmetic on one.  An indexed,
    dereferenced, member or call result (`base[0x28]`, `*a0`, `func_X()`) is a
    *value* — a 32-bit guest address — and does not count."""
    e = _strip_parens(expr)
    if not e:
        return False
    if e.startswith("&") or e.startswith('"'):
        return True
    if PTR1_CAST_RE.match(e) or PTRPTR_CAST_RE.match(e):
        return True
    terms = _top_terms(e)
    for t in terms:
        t2 = _strip_parens(t)
        if t2 != t.strip() and is_pointer_expr(t2, roots):
            return True
        if re.fullmatch(r"[A-Za-z_]\w*", t2) and t2 in roots:
            return True
        if t2.startswith("&") or PTR1_CAST_RE.match(t2):
            return True
    return False


WRAPPER_SIG_RE = re.compile(
    r"^([A-Za-z_][A-Za-z0-9_ \t\*]*?\b)(func_[0-9A-Fa-f]{8})\s*\(([^;{]*?)\)\s*\{",
    re.MULTILINE,
)


def extract_typedefs(text: str) -> str:
    """Top-level `typedef ...;` declarations from a leaf, brace-balanced."""
    out: list[str] = []
    for m in re.finditer(r"^[ \t]*typedef\b", text, re.MULTILINE):
        i = m.end()
        depth = 0
        while i < len(text):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
            elif text[i] == ";" and depth == 0:
                i += 1
                break
            i += 1
        out.append(text[m.start():i].strip())
    return "\n".join(out)


def extract_body(text: str, name: str) -> str | None:
    """Inner text of the named `func_*` definition, brace-balanced, without the
    opening/closing braces.  Works from the `{` after the parameter list."""
    for m in WRAPPER_SIG_RE.finditer(text):
        if m.group(2) != name:
            continue
        depth = 1
        i = m.end()
        while i < len(text) and depth:
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    return text[m.end(): i]
            i += 1
    return None


def render_wrapper(name: str, plan: dict, leaf: dict, report: Path,
                   proto: dict[str, int]) -> str:
    """Emit a pointer-parameter leaf behind a guest-address host adapter.

    The signature and the `extern` data declarations change (address convention
    only); the function body is the matched leaf's, moved verbatim behind
    `#define` alias macros that route guest addresses through `PE_Translate`.
    `src/` is untouched.
    """
    stripped = strip_comments(plan["body"])
    sig = None
    for m in WRAPPER_SIG_RE.finditer(stripped):
        if m.group(2) == name:
            sig = m
    assert sig is not None
    ret_type = sig.group(1).strip()

    pparams = pointer_params(plan["param_inner"])
    pnames = {p for p, _, _, _ in pparams}

    # Wrapper signature: pointer params become guest addresses; others verbatim.
    sig_params: list[str] = []
    for part in plan["param_inner"].split(","):
        p = part.strip()
        if not p:
            continue
        m = re.search(r"(\w+)\s*$", p)
        pname = m.group(1) if m else ""
        if pname in pnames:
            sig_params.append(f"pe_addr_t pe_{pname}")
        else:
            sig_params.append(p)

    # The verbatim body runs inside its own generated function.  Each guest
    # symbol becomes a local host pointer (`host_NAME`) and every occurrence of
    # the leaf's name is textually substituted, so a scalar/array/pointer-global
    # access keeps exactly the leaf's expression shape.  A whole-word leaf
    # dereference `*NAME` replaces the generated macro dereference; a bare
    # `NAME` used as a call argument was already rewritten to a guest address
    # literal, so the remaining bare forms are array/pointer-global values.
    body = plan["body"].rstrip("\n")
    body = neutralize_declarations(body, set(), set(plan["boundaries"]),
                                   canonical_for(plan, proto))
    body = fix_argument_addresses(body, set(plan["d_syms_used"]),
                                  plan["data_types"])
    body = host_to_guest_args(body, plan)
    for sym in sorted(plan["d_syms_used"], key=len, reverse=True):
        kind = classify_data(plan["data_types"][sym], sym)
        if kind == "scalar":
            # Scalar leaf access is an lvalue; the macro becomes the pointee.
            body = re.sub(rf"\*\s*{re.escape(sym)}\b", f"host_{sym}", body)
            body = re.sub(rf"\b{re.escape(sym)}\b", f"(*host_{sym})", body)
        else:
            body = re.sub(rf"\b{re.escape(sym)}\b", f"host_{sym}", body)
    vecs = argvec_params(plan["param_inner"])
    for pname, _lvl, _toks, decl in pparams:
        if pname in vecs:
            continue
        body = re.sub(
            rf"\(\s*{re.escape(re.sub(r'\*+\s*$', '', decl).strip())}\s*\*+\s*\)"
            rf"\s*{re.escape(pname)}\b",
            f"host_{pname}", body)
        # A pointer parameter *is* the guest address; when the leaf hands it
        # straight to a callee, the callee's host prototype takes a `pe_addr_t`
        # (canonical) or the boundary records the guest argument, so the guest
        # address is what must be forwarded.  Memory access still goes through
        # the translated `host_` pointer.
        body = re.sub(
            rf"(?<=[(,])\s*{re.escape(pname)}\s*(?=[,)])", f" pe_{pname}", body)
        # A struct member with the same name is not the parameter.
        body = re.sub(rf"(?<![.>])\b{re.escape(pname)}\b", f"host_{pname}", body)
    body_only = extract_body(body, name)
    assert body_only is not None, name
    # E7c: guest-pointer-vector parameters (only inside the body; the
    # signature already became `pe_addr_t pe_<name>`).
    for pname, etype in vecs.items():
        body_only, why = argvec_rewrite(body_only, pname, etype)
        assert not why, (name, why)
    body_only = rect_array_args(body_only, _ADDR_SIGS)
    body_only = empty_callee_args_drop(body_only, proto)

    lines: list[str] = []
    lines.append(
        "/*\n"
        f" * decomp-source: {name}\n"
        f" * matched VMA 0x{leaf['vram']:08X}  file 0x{leaf['file_offset']:X}"
        f"  span 0x{leaf['file_size']:X} bytes / {leaf['words']} words\n"
        f" * evidence: {report.as_posix()}\n"
        " *\n"
        " * GENERATED by tools/analysis/gen_decomp_ports.py from"
        f" src/{name}.c — do not\n"
        " * edit by hand; the matching leaf is the authority.  Pointer parameters\n"
        " * are adapted from host pointers to guest addresses (`pe_addr_t`); the\n"
        " * function body is otherwise verbatim.  See\n"
        " * docs/ai_context/PC_PORT_FROM_DECOMP.md.\n"
        " */\n"
    )
    lines.append('#include "pe_guest_decomp.h"\n')
    lines.append("")

    typedefs = extract_typedefs(stripped)
    if typedefs:
        lines.append("/* verbatim leaf types */\n")
        lines.append(typedefs + "\n\n")

    # The wrapper emits only the definition body, so a callee whose host
    # prototype is not in a canonical header still needs the leaf's own
    # declaration (verbatim) to be visible here.
    callee_protos: list[str] = []
    for m in re.finditer(
            r"^[ \t]*(?:extern\s+)?[A-Za-z_][A-Za-z0-9_ \t\*]*?"
            r"\b(func_[0-9A-Fa-f]{8})\s*\([^\n;{]*?\)\s*;[ \t]*$",
            stripped, re.MULTILINE):
        callee = m.group(1)
        if callee == name or callee in plan["boundaries"] or \
                callee in canonical_for(plan, proto):
            continue
        callee_protos.append(re.sub(r"[ \t]+", " ",
                                    m.group(0).strip()))
    if plan.get("host_protos"):
        lines.append("/* host prototypes of decomp-derived callees (generated TUs) */\n")
        for decl in plan["host_protos"]:
            lines.append(decl + "\n")
        lines.append("\n")
    if callee_protos:
        lines.append("/* verbatim callee declarations */\n")
        for decl in dict.fromkeys(callee_protos):
            lines.append(decl + "\n")
        lines.append("\n")

    for callee in plan["boundaries"]:
        lines.append(f"/* boundary: {callee} has no pc_port implementation yet */\n")
        arity_n = plan["boundary_arity"][callee]
        if arity_n == 0:
            lines.append(
                f'#define {callee}(...) PE_D_COMP_BOUNDARY0("{callee}", 0x{callee[5:]}u)\n')
        else:
            lines.append(
                f'#define {callee}(...) PE_D_COMP_BOUNDARY{arity_n}('
                f'"{callee}", 0x{callee[5:]}u, __VA_ARGS__)\n')
        lines.append("\n")
    if plan["boundaries"]:
        lines.append("")

    if plan.get("ptr_ret"):
        lines.append(f"static {ret_type} pe_host_{name}({', '.join(sig_params)})\n{{\n")
    else:
        lines.append(f"{ret_type} {name}({', '.join(sig_params)})\n{{\n")
    for decl_line in _host_locals(plan, pparams, body_only):
        lines.append(decl_line + "\n")
    lines.append("    /* verbatim body (src/%s.c) */\n" % name)
    for bl in body_only.split("\n"):
        lines.append(bl + "\n")
    lines.append("}\n")
    if plan.get("ptr_ret"):
        lines.append(ptr_return_thunk(
            name, ", ".join(sig_params) or "void",
            param_call_names(plan["param_inner"], pnames),
            ptr_roots_expr(plan, {p for p in pnames if p not in argvec_params(plan["param_inner"])})))
    return canonical_guest_call_args(
        e20_over_export("".join(lines), name, plan.get("canon_sig")))


def e20_over_export(text: str, name: str, canon: dict | None) -> str:
    """E20 composed with the pointer adapters (class D).  The pointer-param
    wrapper / pointer-return thunk export `name` with every pointer as
    pe_addr_t; when that exported signature still differs from the canonical
    header only in E20's terms (32-bit integer class, or a `void` header over
    a 32-bit result), the export becomes the static `pe_leaf_<name>` behind an
    E20 thunk.  Anything else is left unchanged (the compile gate judges)."""
    if not canon:
        return text
    m = re.search(rf"(?m)^(?!static\b)([A-Za-z_][A-Za-z0-9_ \t\*]*?)\b{re.escape(name)}"
                  rf"\s*\(([^;{{]*?)\)\s*\{{", text)
    if not m:
        return text
    ad = signedness_adapter(m.group(1), m.group(2), canon)
    if not isinstance(ad, dict) or ad.get("dropped"):
        # Pure E20 only: an E20b trailing-parameter drop needs its per-leaf
        # retail-disassembly proof and is never applied implicitly here.
        return text
    head = m.group(0)
    new = "static " + re.sub(rf"\b{re.escape(name)}(?=\s*\()", f"pe_leaf_{name}", head, count=1)
    return text[:m.start()] + new + text[m.end():] + "\n" + signedness_thunk(name, ad)


def canonical_guest_call_args(text: str) -> str:
    """Final pass: every `PE_GuestCall` register argument `(uintptr_t)(X)`
    becomes `(uintptr_t)(uint32_t)(X)`.  A retail argument register holds 32
    bits; a signed `int` X would otherwise sign-extend into the 64-bit
    uintptr_t (0x80150000 recorded as 0xFFFFFFFF80150000 in the boundary
    trace; registered thunks truncate, so only the trace was wrong)."""
    out = []
    i = 0
    for m in re.finditer(r"\bPE_GuestCall\(", text):
        if m.start() < i:
            continue
        close = _balanced_end(text, m.end() - 1)
        args = split_top(text[m.end(): close - 1])
        new_args = [a.strip() for a in args[:3]]
        for a in args[3:]:
            a = a.strip()
            if a.startswith("(uintptr_t)(") and not a.startswith("(uintptr_t)(uint32_t)"):
                a = "(uintptr_t)(uint32_t)" + a[len("(uintptr_t)"):]
            new_args.append(a)
        out.append(text[i:m.start()])
        out.append("PE_GuestCall(" + ", ".join(new_args) + ")")
        i = close
    out.append(text[i:])
    return "".join(out)


def _host_locals(plan: dict, pparams: list, body: str) -> list[str]:
    """Translated host locals for the symbols/parameters the body actually
    dereferences.  A value forwarded to a callee or boundary keeps only the
    guest address in the body, so no host pointer local is emitted for it."""
    out: list[str] = []
    for sym in plan["d_syms_used"]:
        # A shadowed symbol (the shim header also #defines it) is still
        # rebound here: the body now names `host_<sym>`, never the macro, and
        # the local is built from the leaf's own declared type.
        if f"host_{sym}" not in body:
            continue
        kind = classify_data(plan["data_types"][sym], sym)
        etype = declared_type(plan["data_types"][sym])
        if kind == "scalar":
            out.append(f"    {etype} *host_{sym} = &PE_DECOMP_SCALAR(0x{sym[2:]}u, {etype});")
        elif kind == "array":
            out.append(f"    {etype} *host_{sym} = PE_DECOMP_ARRAY(0x{sym[2:]}u, {etype});")
        else:
            pointee = pointer_pointee(etype)
            # The wrapper loads the slot once at entry.  A guest 0 either
            # becomes a host NULL (the leaf null-tests the slot:
            # PE_DECOMP_PTRGLOBAL_NULLABLE) or low RAM through the retail
            # mirror (PE_DECOMP_PTRGLOBAL); neither aborts at entry
            # (func_800136C0 only compares D_8009D254), see null_tested().
            macro = ("PE_DECOMP_PTRGLOBAL_NULLABLE"
                     if null_tested(plan["body"], sym) else "PE_DECOMP_PTRGLOBAL")
            out.append(f"    {pointee} *host_{sym} = {macro}(0x{sym[2:]}u, {pointee});")
    for pname, _lvl, _toks, decl in pparams:
        if f"host_{pname}" not in body:
            continue
        base = param_base_type(decl, pname)
        # A guest NULL must stay a host NULL: matched leaves test their
        # pointer parameters (`if (a0 != 0)`, src/func_80080998.c), and an
        # eager PE_Translate(0) aborted before that test could run
        # (portverify: func_80080998 called with dest 0 from func_80080DC4).
        # Any other address is still bounds-checked by PE_Translate.
        if null_tested(plan["body"], pname):
            out.append(
                f"    {base} *host_{pname} = ({base} *)PE_DecompTranslateOrNull(pe_{pname});")
        else:
            out.append(
                f"    {base} *host_{pname} = ({base} *)PE_Translate(pe_{pname}, 1u);")
    return out


# Host compile flags, mirroring pc_port/CMakeLists.txt's pe_field_runtime
# target (`cmake` writes the same set into build/**/flags.make).  A generated
# TU is only emitted if the host compiler accepts it.
COMPILE_DEFINES = ["-DPE_PORT_FB_HEIGHT=240", "-DPE_PORT_FB_WIDTH=320",
                   "-DPE_PORT_HEADLESS=1"]
COMPILE_INCLUDES = ["pc_port", "pc_port/include", "pc_port/platform",
                    "pc_port/bootstrap", "pc_port/src"]


def find_compiler() -> str | None:
    for cand in (os.environ.get("CC"), "cc", "gcc", "clang"):
        if cand and shutil.which(cand):
            return shutil.which(cand)
    return None


def compile_check(cc: str, items: list[tuple[str, str]]) -> dict[str, str]:
    """Syntax-check each (name, rendered source); return name -> first error.

    This is the generator's last line of defence and the one that cannot be
    fooled: the eligibility rules say *why* a leaf is not host-representable,
    but only the compiler proves the emitted TU actually builds.  A leaf whose
    TU does not compile is reported and not written, so the generated tree is
    always buildable.
    """
    cmd_head = [cc, "-std=gnu11", "-fsyntax-only", *COMPILE_DEFINES]
    for inc in COMPILE_INCLUDES:
        cmd_head += ["-I", str(REPO_ROOT / inc)]
    failures: dict[str, str] = {}

    def one(item: tuple[str, str]) -> tuple[str, str]:
        name, content = item
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / f"{name}_port.c"
            path.write_text(content, encoding="utf-8")
            proc = subprocess.run(cmd_head + [str(path)],
                                  capture_output=True, text=True)
            if proc.returncode == 0:
                return name, ""
            first = ""
            for line in proc.stderr.splitlines():
                if ": error:" in line:
                    first = line.split(": error:", 1)[1].strip()
                    break
            return name, first or "compile failed"

    workers = min(16, (os.cpu_count() or 4))
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        for name, err in pool.map(one, items):
            if err:
                failures[name] = err
    return failures


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--check", action="store_true",
                    help="report eligibility/coverage without writing files")
    ap.add_argument("--list", action="store_true",
                    help="list eligible leaves (one per line)")
    ap.add_argument("--json", metavar="PATH",
                    help="write the full eligible-leaf plan as JSON")
    ap.add_argument("--only", action="append", default=[],
                    help="restrict to a leaf name (repeatable)")
    ap.add_argument("--verify", action="store_true",
                    help="fail (exit 1) if a generated TU has drifted from the "
                         "matching leaf; do not write")
    ap.add_argument("--limit", type=int, default=0,
                    help="cap the number of leaves emitted (0 = all)")
    ap.add_argument("--verbose", action="store_true",
                    help="with --check, print every skipped leaf and reason")
    ap.add_argument("--no-compile-check", action="store_true",
                    help="do not syntax-check each rendered TU with the host "
                         "compiler before emitting it (the gate that keeps "
                         "pc_port/game/decomp buildable)")
    args = ap.parse_args()

    matched = parse_matched()
    # Emit to a fixed point.  Whether a callee gets its own prototype or a loud
    # boundary depends on whether that callee's TU exists in the generated
    # directory, so adding or pruning a TU changes the correct content of its
    # callers.  Re-run the whole pipeline until nothing changes (typically one
    # round in steady state; more from an empty directory).
    MAX_ROUNDS = 8
    written = 0
    conflicts: list[str] = []
    drift: list[str] = []
    orphans: list[str] = []
    uncompilable: dict[str, str] = {}
    for _round in range(MAX_ROUNDS):
        defs = pc_port_definitions()
        known = set(defs)
        shim_macros = pc_port_macros()
        protos = canonical_protos()
        sigs = canonical_signatures()
        for n, s in inline_shims().items():
            known.add(n)
            protos.setdefault(n, len([p for p in s["params"]
                                      if p.strip() not in ("", "void")]))
            sigs.setdefault(n, s)
        # Generated decomp-derived definitions are host authorities too (see
        # derived_signatures); a canonical header prototype still wins.
        header_protos = set(protos)
        dsigs = {n: s for n, s in derived_signatures().items()
                 if n not in header_protos}
        for n, s in dsigs.items():
            protos.setdefault(n, len(s["params"]))
            sigs.setdefault(n, {"ret": s["ret"], "params": s["params"]})
        _ADDR_SIGS.clear()
        _ADDR_SIGS.update(sigs)
        _ADDR_KNOWN.clear()
        _ADDR_KNOWN.update(known)
        out_dir = REPO_ROOT / OUT_DIR

        eligible: list[str] = []
        skipped: dict[str, str] = {}
        plans: dict[str, dict] = {}

        for name in sorted(matched):
            src = REPO_ROOT / SRC_DIR / f"{name}.c"
            if not src.exists():
                skipped[name] = "no src file"
                continue
            plan = analyze(name, src.read_text(encoding="utf-8", errors="replace"),
                           matched[name], known, shim_macros, protos, sigs)
            if not plan["eligible"]:
                skipped[name] = plan["reason"]
                continue
            override = [c for c in plan["callees"]
                        if c in dsigs and c != name and not same_host_decl(
                            strip_comments(src.read_text(encoding="utf-8",
                                                         errors="replace")),
                            c, dsigs[c])]
            plan["host_protos"] = [dsigs[c]["decl"] for c in override]
            plan["host_override"] = override
            plan["derived_all"] = sorted(dsigs)
            plans[name] = plan
            eligible.append(name)

        if args.only:
            wanted = set(args.only)
            eligible = [n for n in eligible if n in wanted]
        if args.limit:
            eligible = eligible[: args.limit]

        if args.json:
            payload = {
                "eligible": {n: {"words": matched[n]["words"],
                                 "callees": plans[n]["callees"],
                                 "boundaries": plans[n]["boundaries"]}
                             for n in eligible},
                "skipped": skipped,
            }
            Path(args.json).write_text(
                json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")

        if args.list:
            for n in eligible:
                print(n)
            return 0

        total_words = sum(matched[n]["words"] for n in eligible)
        if args.check:
            print(f"gen_decomp_ports: eligible {len(eligible)} leaves "
                  f"({total_words} words); ineligible {len(skipped)}")
            if args.verbose:
                for name in sorted(skipped):
                    print(f"  skip {name}: {skipped[name]}")
            return 0

        out_dir.mkdir(parents=True, exist_ok=True)
        changed = 0
        written = 0
        conflicts: list[str] = []
        drift: list[str] = []
        emitted: set[str] = set()
        rendered: list[tuple[str, str]] = []
        for name in eligible:
            target = out_dir / f"{name}_port.c"
            others = [f for f in defs.get(name, ())
                      if f != target.relative_to(REPO_ROOT).as_posix()]
            if others:
                conflicts.append(f"{name}: also defined in {', '.join(sorted(others))}")
                continue
            rendered.append((name, render(name, plans[name], matched, known, protos)))

        uncompilable: dict[str, str] = {}
        if not args.no_compile_check:
            cc = find_compiler()
            if cc is None:
                print("gen_decomp_ports: WARNING no host C compiler found "
                      "(set CC); emitting without the compile gate", file=sys.stderr)
            else:
                uncompilable = compile_check(cc, rendered)
                # A shared-header break makes (nearly) every TU fail with the
                # same error; pruning then would wipe the generated tree.
                if rendered and len(uncompilable) > len(rendered) // 2:
                    from collections import Counter
                    top, cnt = Counter(uncompilable.values()).most_common(1)[0]
                    raise SystemExit(
                        f"gen_decomp_ports: {len(uncompilable)}/{len(rendered)} TUs fail "
                        f"the compile gate ({cnt} with: {top}) — a shared pc_port header "
                        "is broken; refusing to emit/prune.  Fix the header and re-run.")
                rendered = [(n, c) for n, c in rendered if n not in uncompilable]

        for name, content in rendered:
            target = out_dir / f"{name}_port.c"
            if target.exists() and target.read_text(encoding="utf-8") == content:
                written += 1
                emitted.add(target.name)
                continue
            if args.verify:
                drift.append(str(target.relative_to(REPO_ROOT)))
                emitted.add(target.name)
                continue
            target.write_text(content, encoding="utf-8")
            written += 1
            changed += 1
            emitted.add(target.name)

        # Orphan pruning.  This directory is generated output that CMake globs, so
        # a TU left behind after its leaf became ineligible (or was renamed, or now
        # collides with a hand port) is still compiled and can break the build.
        # The generator owns every `func_XXXXXXXX_port.c` here, so anything it did
        # not just emit is removed.  `--only`/`--limit` restrict the emit set, so
        # pruning is suppressed for those runs.
        orphans: list[str] = []
        if out_dir.is_dir() and not args.only and not args.limit:
            for path in sorted(out_dir.glob("*_port.c")):
                if not re.fullmatch(r"func_[0-9A-Fa-f]{8}_port\.c", path.name):
                    continue
                if path.name in emitted:
                    continue
                orphans.append(path.name)
                if not args.verify:
                    path.unlink()
                    changed += 1

        # --verify writes nothing, so a second round would be identical.
        if args.verify or changed == 0:
            break
    else:
        print(f'gen_decomp_ports: WARNING emit set still changing after '
              f'{MAX_ROUNDS} rounds', file=sys.stderr)

    if args.verify:
        print(f"gen_decomp_ports: verified {written} port TU(s) up to date; "
              f"{len(drift) + len(orphans)} drifted")
        for rel in drift:
            print(f"  stale {rel}")
        for nm in orphans:
            print(f"  orphan {(OUT_DIR / nm).as_posix()} "
                  "(no longer emitted; re-run without --verify to prune)")
        return 1 if (drift or orphans) else 0

    print(f"gen_decomp_ports: wrote {written} port TU(s) to {OUT_DIR}")
    if uncompilable:
        print(f"gen_decomp_ports: {len(uncompilable)} leaf/leaves not emitted "
              "(rendered TU does not compile)")
        for nm in sorted(uncompilable):
            print(f"  uncompilable {nm}: {uncompilable[nm]}")
    if orphans:
        print(f"gen_decomp_ports: pruned {len(orphans)} orphaned TU(s)")
    print(f"gen_decomp_ports: {len(conflicts)} conflict(s) not emitted "
          "(matched C would collide with an existing pc_port definition)")
    if args.verbose:
        for c in conflicts:
            print(f"  {c}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
