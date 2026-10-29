#!/usr/bin/env python3
"""hook_writer_census.py -- retail writers of the callback words the port checks by identity.

Some hand-ported SDK dispatchers read a guest callback word, run native C for
the identities they know, and stop or abort on any other value.  Whether that
other arm can run depends on what the game can store in the word.  This census
lists every retail writer it finds and classifies each stored value.

A family is a hook word or table, its setter functions and the identities its
port dispatcher handles.  The EXE text and every extracted PE.IMG blob are
scanned for:
  setter    each `jal`/`j` to a setter.  The stored argument (and the table
            index, for tables) is traced backward from the call;
  store     each store whose effective address lies in the hook range:
            `lui` base + immediate, or `$gp` + immediate (gp 0x8009CD70);
  imm-only  a store whose immediate is the low half of a hook address, but
            whose base register the trace cannot settle;
  alt-base  each `lui`/`addiu|ori` pair or `addiu rt,$gp,imm` naming an
            address in the hook range (a base for a later store or clear);
  jtb load  for families whose setters are reached through the libetc jump
            table D_8009566C, each load of that pointer (an inline call
            through the table would bypass the setter wrappers).
The non-test pc_port sources are also grepped for the hook addresses and
their macro names ("port writers"; informational).

The backward trace follows straight-line code before a use.  It gives up at:
  * a branch target, where control can merge from elsewhere;
  * the delay slot of an unconditional jump other than `jr $ra`;
  * a call, when the traced register is caller-saved;
  * a load, or arithmetic it does not fold;
  * the WINDOW limit.
At a `jr $ra` it stops with `forwarded`: the value is the enclosing function's
incoming argument register.

Value classes:
  identity    a literal the port's dispatcher runs natively
  zero        0; the dispatcher skips a null word
  other       any other literal, a production gap if the writer runs
  forwarded   the enclosing function's own argument.  Allowed only in a
              declared forwarder, whose callers are scanned as setters too
  unresolved  anything the trace cannot settle

Each family declares its expected sites by address: setter bodies, clears
and readers. Except for the explicitly listed accepted exceptions and writers
with no detected static entry reference, `--check` fails on:
  * an `other` or `unresolved` value;
  * a forwarded value outside a declared forwarder;
  * a table index outside the table, or outside the port's handled sources;
  * an undeclared store, imm-only, alt-base or jtb-load site;
  * a port store of a literal that is neither an identity nor zero.

Blind spots, stated in the report:
  * stores through a computed address outside the declared setters, such
    as `base + index*4`;
  * block copies or clears whose base lies outside the hook range;
  * a writer whose value reaches its use other than in straight-line code
    (the trace reports those as unresolved; it never guesses);
  * jtb calls made through a pointer copied elsewhere.

A passing check is a regression check of this declared static inventory, not
proof of dormancy, reachability or executed native coverage. Port expression
writers are informational and are not resolved. The dynamic m28/m32 slot+12
movement callback is outside these fixed-address families.

Retail files are read-only inputs.  The report prints addresses, register
numbers, counts and file:line sites only, never bytes.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
MAIN = REPO.parents[1] if REPO.parent.name == "build" else REPO
DISC = MAIN / "build/extracted/disc1"

GP = 0x8009CD70            # EXE $gp; overlays and rooms share it
ROOM_VMA = 0x8018EFE8      # room chunk 2 arena (configs/USA/overlays/manifest.yaml)
JR_RA = 0x03E00008
WINDOW = 48                # instructions the backward trace may walk
JTB = 0x8009566C           # libetc jump-table pointer (D_8009566C)
EXE_TEXT_END = 0x800E0808  # end of the EXE's code (room_callback_census.EXE_TEXT)

ARG_REGS = (4, 5, 6, 7)
CALLEE_SAVED = frozenset((16, 17, 18, 19, 20, 21, 22, 23, 28, 29, 30))
STORES = {0x28: 1, 0x29: 2, 0x2A: 4, 0x2B: 4, 0x2E: 4}       # sb sh swl sw swr
STORE_NAMES = {0x28: "sb", 0x29: "sh", 0x2A: "swl", 0x2B: "sw", 0x2E: "swr"}
LOADS = frozenset((0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26))
BRANCH_OPS = frozenset((0x01, 0x04, 0x05, 0x06, 0x07))
FAIL = ("other", "unresolved", "forwarded-undeclared", "index", "undeclared")
BAD_VALUES = ("other", "unresolved", "forwarded-undeclared")


def failing(cls: str) -> bool:
    """accepted/* never fails; expected/<value> fails only on a bad value."""
    if cls.startswith("accepted/") or cls == "expected/body":
        return False
    if cls.startswith("expected/"):
        return cls.split("/", 1)[1] in BAD_VALUES
    return cls in FAIL

# Each family: the hook range, the identities its port dispatcher handles,
# the dispatcher site (file + a marker string on the dispatching line), the
# setters (value / index argument: ("reg", n), ("stack", offset) or
# ("lit", value)), declared forwarders (same shape) and expected sites.
JTB_WRAPPERS = {
    # Every load of D_8009566C.  The static table at 0x8009564C holds
    # func_800740D0 (the CPU-table setter) in slot +0x8 and receives
    # func_800746A0 (the DMA-table setter) in slot +0x4 from ResetCallback.
    0x80073C98: "func_80073C94: calls jtb +0xC (ResetCallback body func_80073E28)",
    0x80073CC8: "func_80073CC4: calls jtb +0x8 (InterruptCallback; a listed setter)",
    0x80073CF8: "func_80073CF4: calls jtb +0x4 (DMACallback; a listed setter)",
    0x80073D2C: "func_80073D24: calls jtb +0x14 (VSync callback slots, not a table here)",
    0x80073D5C: "func_80073D58: calls jtb +0x14 (VSync callback slots, not a table here)",
    0x80073D8C: "func_80073D88: calls jtb +0x10 (func_80074218, StopCallback)",
    0x80073DBC: "func_80073DB8: calls jtb +0x18 (func_800742B8, RestartCallback)",
    0x80073EC8: "func_80073E28: ResetCallback stores jtb[5] (a store, not a call)",
    0x80073ED8: "func_80073E28: ResetCallback stores jtb[1] (a store, not a call)",
}

# Each family: the hook range, the identities its port dispatcher handles,
# the dispatcher site (file + a marker string on the dispatching line), the
# setters (value / index argument: ("reg", n), ("stack", offset) or
# ("lit", value)), declared forwarders (same shape), expected sites
# (a note, or ("body", note) for a setter's own store, whose value its
# callers decide), reviewed exceptions ("accepted") and extra base addresses
# whose references are listed ("near": struct bases below the range).
FAMILIES = [
    {
        "key": "cd_vblank",
        "title": "libds VBlank hook D_800A36A0, called by func_8007FE24",
        "base": 0x800A36A0, "count": 1,
        "handled": [0x8007F7E8],
        "site": ("pc_port/game/boot/cd_stream_port.c", '"CD_vblank_callback"'),
        "setters": [{"fn": 0x8007FBC0, "val": ("reg", 4)}],
        "forwarders": [],
        "expected": {
            0x8007F9CC: "func_8007F994: D_800A36A0 = 0",
            0x8007FBC8: ("body", "func_8007FBC0: D_800A36A0 = a0 (the setter)"),
        },
        "port_names": [],
    },
    {
        "key": "cd_stream_aux",
        "title": "CD stream auxiliary hook D_800B0CCC, called by func_8007C564",
        "base": 0x800B0CCC, "count": 1,
        "handled": [],
        "site": ("pc_port/game/boot/cd_stream_port.c", '"func_8007C564_auxiliary_callback"'),
        "setters": [{"fn": 0x8007C304, "val": ("stack", 0x10)}],
        "forwarders": [],
        "expected": {
            0x8007C36C: ("body", "func_8007C304: D_800B0CCC = argument 5 (the setter)"),
        },
        "port_names": ["GA_STREAM_AUXILIARY"],
    },
    {
        "key": "cd_stream_cb",
        "title": "CD stream callback D_800B0CC8, called by func_8007C214",
        "base": 0x800B0CC8, "count": 1,
        "handled": [],
        "site": ("pc_port/platform/pe_libcd.c", "0x800B0CC8u"),
        "setters": [{"fn": 0x8007C304, "val": ("reg", 7)}],
        "forwarders": [],
        "expected": {
            0x8007C33C: ("body", "func_8007C304: D_800B0CC8 = a3 (the setter)"),
        },
        "port_names": [],
    },
    {
        "key": "dma_table",
        "title": "libetc DMA callback table D_800956C0[7], called by func_80074520",
        "base": 0x800956C0, "count": 7,
        "handled": [0x801214D4, 0x80191DC8, 0x8007C214, 0x80076EE4],
        "site": ("pc_port/platform/pe_irq_delivery.c", '"func_80074520_dma_indirect_call"'),
        "setters": [{"fn": 0x80073CF4, "idx": ("reg", 4), "val": ("reg", 5)},
                    {"fn": 0x800746A0, "idx": ("reg", 4), "val": ("reg", 5)}],
        "forwarders": [{"fn": 0x8007A8EC, "idx": ("lit", 3), "val": ("reg", 4)},
                       {"fn": 0x8007DD14, "idx": ("lit", 4), "val": ("reg", 4)},
                       {"fn": 0x800824F0, "idx": ("lit", 3), "val": ("reg", 4)},
                       {"fn": 0x8010C0B4, "idx": ("lit", 0), "val": ("reg", 4)},
                       {"fn": 0x8010C0D8, "idx": ("lit", 1), "val": ("reg", 4)}],
        "expected": {**JTB_WRAPPERS, **{
            0x800744D8: "func_800744D4: clears the table (func_8007474C(D_800956C0, 8))",
            0x8007456C: "func_80074520: the dispatcher reads the table",
            0x800746A4: "func_800746A0: the setter's base (slot store through base + channel*4)",
        }},
        "accepted": {
            0x8007D184: "func_8007D15C (SpuInit) installs func_8007D614 for channel 4. The port "
                        "models DMA4 completion natively (pe_spu_dma.c) and latches DICR flags "
                        "only for channels 0, 1 and 3 (PE_GPU_LatchDMACompletionFlag), so the "
                        "dispatcher never reads slot 4",
            0x80080F2C: "func_80080E34 (DsRead) installs func_80081110 for channel 3 when its "
                        "mode bit 0 is set. The port has no func_80080E34: func_8006E6D4 reads "
                        "through the synchronous host model in pe_libcd.c, which installs no DMA "
                        "callback, and func_80081D18's adapter stops at a boundary first",
            0x800812B4: "func_80081268 restores D_8009B6C0, the slot-3 handler func_80080E34 "
                        "saved when it installed func_80081110 (same mode-bit test)",
            0x8008139C: "func_80081314 restores the handler its own DMACallback(3, func_8007C214) "
                        "at 0x8008135C returned",
            0x80081EF4: "func_80081E70 restores the handler its own DMACallback(3, 0) at "
                        "0x80081EDC returned",
            0x8012230C: "ovl_03C5 func_80122040 (matched c span) calls func_8010C0D8(0); the "
                        "trace stops at a control merge before the argument",
            0x80192C00: "ovl_03D2 func_80192934 (matched c span) calls func_8010C0D8(0); the "
                        "trace stops at a control merge before the argument",
        },
        "jtb": True,
        "port_names": ["GA_DMA_CALLBACK_TABLE"],
    },
    {
        "key": "cpu_table",
        "title": "libetc CPU interrupt table D_800945E8[11], called by func_80073F00",
        "base": 0x800945E8, "count": 11,
        "handled": [0x8007C13C, 0x8007440C, 0x80074520],
        "sources": [0, 2, 3],   # func_800740D0 translates only these (pe_libetc.c source cut)
        "site": ("pc_port/platform/pe_irq_delivery.c", '"func_80073F00_cpu_indirect_call"'),
        "setters": [{"fn": 0x80073CC4, "idx": ("reg", 4), "val": ("reg", 5)},
                    {"fn": 0x800740D0, "idx": ("reg", 4), "val": ("reg", 5)}],
        "forwarders": [{"fn": 0x80085DC4, "idx": ("lit", 9), "val": ("reg", 4)},
                       {"fn": 0x80085D84, "idx": ("lit", 9), "val": ("reg", 4)}],
        "near": [0x800945E4],   # the interrupt control block; the table is its +0x4..+0x30
        "expected": {**JTB_WRAPPERS, **{
            0x800740E4: "func_800740D0: the setter's base (slot store through base + source*4)",
            0x80073E30: "func_80073E28 (ResetCallback body): clears the block (func_80074330(p, 0x41A))",
            0x80073F08: "func_80073F00: the dispatcher reads the block",
            0x80074220: "func_80074218 (StopCallback): block +0x0/+0x32/+0x34 only",
            0x800742C0: "func_800742B8 (RestartCallback): block +0x0/+0x32/+0x34/+0x38 only",
        }},
        "accepted": {
            0x80085DD0: "func_80085DC4 = InterruptCallback(9, a0); see 0x8008569C",
            0x80085DA4: "func_80085D84 calls func_80085DC4 only when a0 differs from D_8009B438, "
                        "which only func_80085D84 writes; see 0x8008569C",
            0x8008569C: "func_80085644, the only caller of func_80085D84, passes 0, so "
                        "D_8009B438 stays 0 and InterruptCallback(9, ...) is never reached. "
                        "The port's pe_stream.c collapses this same call",
        },
        "jtb": True,
        "port_names": ["GA_IRQ_CPU_CALLBACK_TABLE"],
    },
    {
        "key": "spu_transfer",
        "title": "libspu transfer callback D_8009B434, called by the DMA4 interrupt",
        "base": 0x8009B434, "count": 1,
        "handled": [0x80085098],
        "site": ("pc_port/platform/pe_spu_dma.c", "!SpuDma_CallGuest(callback)"),
        "setters": [{"fn": 0x80085F44, "val": ("reg", 4)}],
        "forwarders": [],
        "expected": {
            0x8007D434: "func_8007D1D4: D_8009B434 = 0",
            0x800859B0: "func_80085984: D_8009B434 = 0",
            0x80085F5C: ("body", "func_80085F44: D_8009B434 = a0 (the setter)"),
            0x8008D6F4: "func_8008D610 (SpuClearReverbWorkArea): saves and clears the callback",
            0x8008D778: ("body", "func_8008D610: restores the callback it saved before the clear"),
        },
        "port_names": ["GA_D_8009B434"],
    },
    {
        "key": "pad_record",
        "title": "libpad record hook D_8009B738, called by the controller queries",
        "base": 0x8009B738, "count": 1,
        "handled": [0x80084B20],
        "site": ("pc_port/game/boot/pe_controller_sdk_port.c", "0x8009B738u"),
        "setters": [],
        "forwarders": [],
        "expected": {
            0x80084578: "func_800844E4: D_8009B738 = func_80084B20",
        },
        "port_names": [],
    },
    {
        "key": "pad_busy",
        "title": "libpad busy hook D_8009B740, called by the controller queries",
        "base": 0x8009B740, "count": 1,
        "handled": [0x80084F8C],
        "site": ("pc_port/game/boot/pe_controller_sdk_port.c", "0x8009B740u"),
        "setters": [],
        "forwarders": [],
        "expected": {
            0x80084B60: "func_80084B44: D_8009B740 = func_80084F8C",
        },
        "port_names": [],
    },
    {
        "key": "drawsync",
        "title": "libgpu DrawSync callback D_80095758, called by func_80076EE4",
        "base": 0x80095758, "count": 1,
        "handled": [],
        "site": ("pc_port/game/boot/func_80076EE4_port.c", "0x80095758u"),
        "setters": [{"fn": 0x80074CC8, "val": ("reg", 4)}],
        "forwarders": [],
        "near": [0x8009574E],   # libgpu state block; func_80074CC8 stores through it
        "expected": {
            0x80074BBC: "func_80074BB8: libgpu state block base (no store to +0xA)",
            0x80074CD0: "func_80074CC8 (DrawSyncCallback): state block base",
            0x80074D10: ("body", "func_80074CC8: D_80095758 = a0 (the setter)"),
            0x80074D30: "func_80074D28: libgpu state block base (no store to +0xA)",
            0x80074E34: "func_80074E28: libgpu state block base (no store to +0xA)",
            0x8007542C: "func_80075424: libgpu state block base (no store to +0xA)",
            0x800754F4: "func_800754E4: libgpu state block base (no store to +0xA)",
            0x80075600: "func_800755F0: libgpu state block base (no store to +0xA)",
        },
        "port_names": [],
    },
]


# Compiler switch tables whose port translation lists every label and stops
# in `default`.  The table is static EXE data, so the default arm is dead
# exactly when every entry is a listed label.
SWITCH_TABLES = [
    {"key": "cd_command_table", "title": "func_80080220 command switch (cmd - 2 < 26)",
     "base": 0x80011D0C, "count": 26,
     "labels": [0x80080318, 0x80080340, 0x80080354, 0x80080368],
     "site": ("pc_port/game/boot/cd_stream_port.c", '"CD_command_table"')},
    {"key": "cd_interrupt_table", "title": "func_8007AAB4 interrupt-tag switch (tag 1..5)",
     "base": 0x80011B8C, "count": 5,
     "labels": [0x8007AD10, 0x8007AE10, 0x8007AE5C, 0x8007AEDC, 0x8007AF5C],
     "site": ("pc_port/platform/pe_libcd.c", '"func_8007AAB4_jump_table"')},
]


# ── MIPS decoding ──────────────────────────────────────────────────────────

def op(w: int) -> int:
    return w >> 26


def rs(w: int) -> int:
    return (w >> 21) & 31


def rt(w: int) -> int:
    return (w >> 16) & 31


def rd(w: int) -> int:
    return (w >> 11) & 31


def simm(w: int) -> int:
    x = w & 0xFFFF
    return x - 0x10000 if x & 0x8000 else x


def store_span(o: int, eff: int) -> tuple[int, int]:
    """Bytes [start, end) a store writes.  Little-endian swl at A writes
    A & ~3 .. A; swr writes A .. A | 3."""
    if o == 0x2A:
        return eff & ~3, eff + 1
    if o == 0x2E:
        return eff, (eff | 3) + 1
    return eff, eff + STORES[o]


def is_call(w: int) -> bool:
    return op(w) == 0x03 or (op(w) == 0 and w & 0x3F == 0x09) or \
        (op(w) == 0x01 and rt(w) in (0x10, 0x11))


def is_uncond(w: int) -> bool:
    """j, jr, or `beq $0,$0` (b): no fall-through past the delay slot."""
    return op(w) == 0x02 or (op(w) == 0 and w & 0x3F == 0x08) or \
        (op(w) == 0x04 and rs(w) == 0 and rt(w) == 0)


def dest(w: int) -> int | None:
    o = op(w)
    if o == 0:
        f = w & 0x3F
        if f in (0x08, 0x0C, 0x0D, 0x11, 0x13, 0x18, 0x19, 0x1A, 0x1B):
            return None        # jr, syscall, break, mthi, mtlo, mult/div
        return rd(w) or None
    if 0x08 <= o <= 0x0F or o in LOADS:
        return rt(w) or None
    if o in (0x10, 0x12) and rs(w) in (0, 2):
        return rt(w) or None   # mfc0, mfc2/cfc2
    if o == 0x03 or (o == 0x01 and rt(w) in (0x10, 0x11)):
        return 31
    return None


def labels_of(words: list[int], vma: int | None) -> set[int]:
    """Indices that are branch or same-blob jump targets (control merges)."""
    out = set()
    n = len(words)
    for k, w in enumerate(words):
        o = op(w)
        if o in BRANCH_OPS:
            t = k + 1 + simm(w)
            if 0 <= t < n:
                out.add(t)
        elif o == 0x02 and vma is not None:
            t = (((vma + k * 4) & 0xF0000000) | ((w & 0x03FFFFFF) << 2)) - vma
            if 0 <= t < n * 4:
                out.add(t // 4)
    return out


class Blob:
    """One scanned image.  `code` is the instruction range [lo, hi) in word
    indices; words outside it are data, searched only for references."""

    def __init__(self, name: str, data: bytes, vma: int | None, gp: int | None = GP,
                 code: tuple[int, int] | None = None):
        n = len(data) // 4
        self.name, self.vma, self.gp, self.data = name, vma, gp, data[: n * 4]
        self.words = list(struct.unpack("<%dI" % n, self.data))
        self.code = code if code is not None else (0, n)
        self.labels = labels_of(self.words, vma)
        self.starts: list[tuple[int, str]] = []       # (vma, name) from the split config
        region = (vma if vma is not None else 0x80000000) & 0xF0000000
        self.calls: dict[int, list[int]] = {}         # jal/j target -> indices
        self.stores: list[int] = []
        self.jtb_loads: list[int] = []
        for k in range(*self.code):
            w = self.words[k]
            o = op(w)
            if o in (0x02, 0x03):
                self.calls.setdefault(region | ((w & 0x03FFFFFF) << 2), []).append(k)
            elif o in STORES:
                self.stores.append(k)
            elif o == 0x23 and (rs(w) == 28 or w & 0xFFFF == JTB & 0xFFFF):
                self.jtb_loads.append(k)
        self.pairs = list(lui_pairs(self))

    def where(self, k: int) -> str:
        if self.vma is None:
            return f"{self.name}+0x{k * 4:X}"
        return f"{self.vma + k * 4:08X}"

    def addr(self, k: int) -> int | None:
        return None if self.vma is None else self.vma + k * 4

    def span_at(self, k: int) -> tuple[int, str] | None:
        a = self.addr(k)
        if a is None or not self.starts:
            return None
        best = None
        for s, nm in self.starts:
            if s <= a:
                best = (s, nm)
            else:
                break
        return best

    def function_at(self, k: int) -> str:
        sp = self.span_at(k)
        return "" if sp is None else (sp[1] or f"span 0x{sp[0]:08X}")

    def entry(self, boundary: int, use: int) -> int:
        """Function entry for a trace that stopped after a `jr $ra` delay
        slot: skip zero padding, then prefer a split-config start that lies
        between the boundary and the use."""
        e = boundary
        while e < use and self.words[e] == 0:
            e += 1
        sp = self.span_at(use)
        if sp is not None and self.vma is not None and sp[0] >= self.vma + e * 4:
            e = (sp[0] - self.vma) // 4
        return e


def trace(b: Blob, i: int, r: int) -> tuple:
    """Value of GPR r on entry to b.words[i]: ("lit", v) | ("forwarded", reg,
    entry index) | ("unresolved", why)."""
    if r == 0:
        return ("lit", 0)
    if r == 28 and b.gp is not None:
        return ("lit", b.gp)
    ws = b.words
    k = i - 1
    while True:
        if (k + 1) in b.labels:
            return ("unresolved", f"merge at {b.where(k + 1)}")
        if k < 0 or i - k > WINDOW:
            return ("unresolved", "window")
        prev = ws[k - 1] if k >= 1 else None
        if prev is not None and is_uncond(prev):
            # ws[k] is a delay slot that cannot fall through to ws[k + 1].
            if prev == JR_RA:
                if r in ARG_REGS:
                    return ("forwarded", r, b.entry(k + 1, i))
                return ("unresolved", f"live-in ${r}")
            return ("unresolved", f"after jump at {b.where(k - 1)}")
        if prev is not None and is_call(prev) and r not in CALLEE_SAVED:
            # The callee at k-1 runs after ws[k] and before the use, so a
            # caller-saved register holds whatever the callee left there.
            return ("unresolved", f"call at {b.where(k - 1)}")
        if dest(ws[k]) == r:
            return value_of(b, k, r)
        k -= 1


def value_of(b: Blob, k: int, r: int) -> tuple:
    w = b.words[k]
    o = op(w)
    if o == 0x0F:
        return ("lit", (w & 0xFFFF) << 16)
    if o in (0x08, 0x09, 0x0D):
        if rs(w) == 0:
            return ("lit", (simm(w) if o != 0x0D else w & 0xFFFF) & 0xFFFFFFFF)
        base = trace(b, k, rs(w))
        if base[0] == "lit":
            v = (base[1] | (w & 0xFFFF)) if o == 0x0D else base[1] + simm(w)
            return ("lit", v & 0xFFFFFFFF)
        if base[0] == "forwarded" and (w & 0xFFFF) == 0:
            return base
        return ("unresolved", f"arithmetic at {b.where(k)}")
    if o == 0 and w & 0x3F in (0x20, 0x21, 0x25):
        a, c = rs(w), rt(w)
        if c == 0:
            return trace(b, k, a)
        if a == 0:
            return trace(b, k, c)
        x, y = trace(b, k, a), trace(b, k, c)
        if x[0] == "lit" and y[0] == "lit":
            v = (x[1] | y[1]) if w & 0x3F == 0x25 else x[1] + y[1]
            return ("lit", v & 0xFFFFFFFF)
        return ("unresolved", f"arithmetic at {b.where(k)}")
    if o in LOADS:
        return ("unresolved", f"load at {b.where(k)}")
    return ("unresolved", f"op 0x{o:02X} at {b.where(k)}")


def stack_store_value(b: Blob, i: int, off: int) -> tuple:
    """Value written to off($sp) before the call at i (its delay slot first)."""
    ws = b.words
    if i + 1 < len(ws):
        d = ws[i + 1]
        if op(d) == 0x2B and rs(d) == 29 and simm(d) == off:
            return trace(b, i + 1, rt(d))
    k = i - 1
    while True:
        if (k + 1) in b.labels:
            return ("unresolved", f"merge at {b.where(k + 1)}")
        if k < 0 or i - k > WINDOW:
            return ("unresolved", "window")
        w = ws[k]
        prev = ws[k - 1] if k >= 1 else None
        if prev is not None and is_uncond(prev):
            return ("unresolved", f"no store to {off}($sp) before {b.where(i)}")
        if is_call(w) or (prev is not None and is_call(prev)):
            return ("unresolved", f"call may overwrite stack argument at {b.where(k)}")
        if op(w) == 0x2B and rs(w) == 29 and simm(w) == off:
            return trace(b, k, rt(w))
        if dest(w) == 29:
            return ("unresolved", f"$sp changes at {b.where(k)}")
        k -= 1


def arg_value(b: Blob, i: int, spec: tuple) -> tuple:
    kind, x = spec
    if kind == "lit":
        return ("lit", x)
    if kind == "stack":
        return stack_store_value(b, i, x)
    if i + 1 < len(b.words) and dest(b.words[i + 1]) == x:
        return value_of(b, i + 1, x)          # the call's delay slot runs first
    return trace(b, i, x)


# ── scans ──────────────────────────────────────────────────────────────────

def call_sites(b: Blob, target: int) -> list[int]:
    return b.calls.get(target, [])


def classify(v: tuple, fam: dict, entries: set, b: Blob) -> str:
    """entries: function addresses whose own arguments may reach the word
    (the setters and the declared forwarders)."""
    if v[0] == "lit":
        if v[1] == 0:
            return "zero"
        return "identity" if v[1] in fam["handled"] else "other"
    if v[0] == "forwarded":
        return "forwarded" if b.addr(v[2]) in entries else "forwarded-undeclared"
    return "unresolved"


def fmt_value(v: tuple, b: Blob) -> str:
    if v[0] == "lit":
        return f"0x{v[1]:08X}"
    if v[0] == "forwarded":
        e = b.addr(v[2])
        return f"${v[1]} of the function at {('%08X' % e) if e is not None else b.where(v[2])}"
    return v[1]


def lui_pairs(b: Blob):
    """(address, index, register, index of the completing instruction) for
    each lui/addiu|ori pair and each addiu rt,$gp,imm in the code range."""
    ws, n = b.words, b.code[1]
    for k in range(*b.code):
        w = ws[k]
        o = op(w)
        if o == 0x09 and rs(w) == 28 and b.gp is not None and rt(w):
            yield (b.gp + simm(w)) & 0xFFFFFFFF, k, rt(w), k
        if o != 0x0F:
            continue
        t, hi = rt(w), w & 0xFFFF
        for j in range(k + 1, min(k + 7, n)):
            v = ws[j]
            if op(v) in (0x09, 0x0D) and rs(v) == t and rt(v) == t:
                lo = simm(v) if op(v) == 0x09 else v & 0xFFFF
                yield ((hi << 16) + lo) & 0xFFFFFFFF, k, t, j
                break
            if dest(v) == t:
                break


FORWARD = 256              # instructions a pair's register is followed forward


def pair_stores(b: Blob, k_done: int, t: int):
    """Stores through register t after the pair completing at k_done, until t
    is redefined or the function returns (labels are not a stop: a writer
    must not be missed, so this side over-reports)."""
    ws, hi = b.words, b.code[1]
    j = k_done + 1
    while j < min(k_done + 1 + FORWARD, hi):
        w = ws[j]
        if op(w) in STORES and rs(w) == t:
            yield j
        if w == JR_RA:
            # The return's delay slot executes before leaving the function.
            if j + 1 < hi:
                delay = ws[j + 1]
                if op(delay) in STORES and rs(delay) == t:
                    yield j + 1
            return
        if dest(w) == t:
            return
        j += 1


def note_of(expected: dict, a, default: str) -> str:
    n = expected.get(a, default)
    return n[1] if isinstance(n, tuple) else n


def references(addr: int, blobs: list[Blob]) -> dict:
    """Detected references to `addr`: jal/j, lui pairs in code, and aligned
    words anywhere in any image. Computed/interior entries are not covered."""
    pat = struct.pack("<I", addr)
    out = {"call": 0, "lui": 0, "word": 0}
    for b in blobs:
        out["call"] += len(b.calls.get(addr, []))
        out["lui"] += sum(1 for a, *_ in b.pairs if a == addr)
        i = b.data.find(pat)
        while i >= 0:
            if i % 4 == 0:
                out["word"] += 1
            i = b.data.find(pat, i + 1)
    return out


def settle(row: dict, b: Blob, k: int, fam: dict, blobs: list[Blob]) -> None:
    """Apply the family's reviewed exceptions and the unreferenced-writer rule."""
    a = b.addr(k)
    if row["class"] == "other":
        sp = b.span_at(k)
        if sp is not None:
            refs = references(sp[0], blobs)
            row["writer_refs"] = refs
            if not any(refs.values()):
                row["class"] = "other-unreferenced"
                row["note"] = f"no jal, lui pair or word names {sp[1] or hex(sp[0])} anywhere on the disc"
    acc = fam.get("accepted", {})
    if a in acc and row["class"] in FAIL:
        row["class"] = "accepted/" + row["class"]
        row["note"] = acc[a]


def census_family(fam: dict, blobs: list[Blob]) -> dict:
    lo, hi = fam["base"], fam["base"] + 4 * fam["count"]
    los = {(a & 0xFFFF) for a in range(lo, hi)}
    expected = dict(fam["expected"])
    callers = list(fam["setters"]) + list(fam["forwarders"])
    entries = {s["fn"] for s in callers}
    rows = []

    # setter and forwarder call sites
    for s in callers:
        for b in blobs:
            for k in call_sites(b, s["fn"]):
                v = arg_value(b, k, s["val"])
                cls = classify(v, fam, entries, b)
                row = {"kind": "setter", "setter": f"0x{s['fn']:08X}", "site": b.where(k),
                       "function": b.function_at(k), "value": fmt_value(v, b), "class": cls}
                if "idx" in s:
                    iv = arg_value(b, k, s["idx"])
                    row["index"] = fmt_value(iv, b)
                    if iv[0] == "forwarded":
                        if b.addr(iv[2]) not in entries:
                            row["class"] = "forwarded-undeclared"
                    elif iv[0] != "lit":
                        row["class"] = "unresolved"
                    elif not 0 <= iv[1] < fam["count"]:
                        row["class"] = "index"
                    elif "sources" in fam and iv[1] not in fam["sources"]:
                        row["class"] = "index"
                        row["note"] = "source outside the port's translated set"
                settle(row, b, k, fam, blobs)
                rows.append(row)

    # direct stores, gp-relative stores and imm-only candidates
    for b in blobs:
        for k in b.stores:
            w = b.words[k]
            o = op(w)
            base, off, size = rs(w), simm(w), STORES[o]
            a = b.addr(k)
            eff = None
            if base == 28 and b.gp is not None:
                eff = (b.gp + off) & 0xFFFFFFFF
            elif (w & 0xFFFF) in los and base not in (0, 29):
                bv = trace(b, k, base)
                if bv[0] != "lit":
                    rows.append({"kind": "imm-only", "site": b.where(k), "function": b.function_at(k),
                                 "insn": f"{STORE_NAMES[o]} ${rt(w)},0x{w & 0xFFFF:X}(${base})",
                                 "class": "expected" if a in expected else "undeclared",
                                 "note": note_of(expected, a, bv[1])})
                    continue
                eff = (bv[1] + off) & 0xFFFFFFFF
            if eff is None:
                continue
            first, last = store_span(o, eff)
            if last <= lo or first >= hi:
                continue
            v = trace(b, k, rt(w)) if rt(w) else ("lit", 0)
            cls = classify(v, fam, entries, b)
            note = expected.get(a, "")
            if isinstance(note, tuple):          # ("body", why): a setter body; its callers count
                cls, note = "expected/body", note[1]
            elif a in expected:
                cls = "expected/" + cls
            elif cls not in ("other", "unresolved", "forwarded-undeclared"):
                cls = "undeclared"
            row = {"kind": "store", "site": b.where(k), "function": b.function_at(k),
                   "insn": f"{STORE_NAMES[o]} ${rt(w)},{off}(${base})", "address": f"0x{eff:08X}",
                   "value": fmt_value(v, b), "class": cls, "note": note}
            settle(row, b, k, fam, blobs)
            rows.append(row)

    # alt-base references, and stores through a pair's register at any
    # offset (struct bases below the range, such as &D_8009574E + 10)
    near = set(fam.get("near", []))
    seen = {r["site"] for r in rows if r["kind"] == "store"}
    for b in blobs:
        for addr, k, t, kd in b.pairs:
            if lo <= addr < hi or addr in near:
                a = b.addr(k)
                rows.append({"kind": "alt-base" if lo <= addr < hi else "near-base",
                             "site": b.where(k), "function": b.function_at(k),
                             "address": f"0x{addr:08X}",
                             "class": "expected" if a in expected else "undeclared",
                             "note": note_of(expected, a, "")})
            if not lo - 0x8000 <= addr < hi + 0x8000:
                continue
            for j in pair_stores(b, kd, t):
                w = b.words[j]
                eff = (addr + simm(w)) & 0xFFFFFFFF
                first, last = store_span(op(w), eff)
                if last <= lo or first >= hi or b.where(j) in seen:
                    continue
                seen.add(b.where(j))
                v = trace(b, j, rt(w)) if rt(w) else ("lit", 0)
                cls = classify(v, fam, entries, b)
                aj = b.addr(j)
                note = expected.get(aj, "")
                if isinstance(note, tuple):
                    cls, note = "expected/body", note[1]
                elif aj in expected:
                    cls = "expected/" + cls
                elif cls not in ("other", "unresolved", "forwarded-undeclared"):
                    cls = "undeclared"
                row = {"kind": "store", "site": b.where(j), "function": b.function_at(j),
                       "insn": f"{STORE_NAMES[op(w)]} ${rt(w)},{simm(w)}(${t}) via pair at {b.where(k)}",
                       "address": f"0x{eff:08X}", "value": fmt_value(v, b), "class": cls, "note": note}
                settle(row, b, j, fam, blobs)
                rows.append(row)

    # jtb loads: an inline call through D_8009566C would bypass the wrappers
    if fam.get("jtb"):
        for b in blobs:
            for k in b.jtb_loads:
                w = b.words[k]
                if rs(w) == 28 and b.gp is not None:
                    ok = (b.gp + simm(w)) & 0xFFFFFFFF == JTB
                else:
                    bv = trace(b, k, rs(w))
                    ok = bv[0] == "lit" and (bv[1] + simm(w)) & 0xFFFFFFFF == JTB
                if ok:
                    a = b.addr(k)
                    rows.append({"kind": "jtb-load", "site": b.where(k), "function": b.function_at(k),
                                 "class": "expected" if a in expected else "undeclared",
                                 "note": note_of(expected, a, "")})

    # the EXE image's initial value of every hook word (an implicit writer)
    exe = blobs[0]
    for a in range(lo, hi, 4):
        k = (a - exe.vma) // 4
        if 0 <= k < len(exe.words):
            v = ("lit", exe.words[k])
            rows.append({"kind": "initial", "site": f"{a:08X}", "value": fmt_value(v, exe),
                         "class": classify(v, fam, entries, exe)})
        else:
            rows.append({"kind": "initial", "site": f"{a:08X}", "value": "outside the EXE image",
                         "class": "unresolved"})

    counts: dict[str, int] = {}
    for r in rows:
        counts[r["class"]] = counts.get(r["class"], 0) + 1
    fail = [r for r in rows if failing(r["class"])]
    return {"key": fam["key"], "title": fam["title"], "range": [f"0x{lo:08X}", f"0x{hi:08X}"],
            "handled": [f"0x{x:08X}" for x in fam["handled"]], "rows": rows, "counts": counts,
            "fail": len(fail)}


# ── port writers ───────────────────────────────────────────────────────────

def port_writers(fam: dict, repo: Path) -> list[dict]:
    lo, hi = fam["base"], fam["base"] + 4 * fam["count"]
    pats = [re.compile(r"0x0*%X[uU]?\b" % a, re.I) for a in range(lo, hi, 4)]
    names = [re.compile(r"\b%s\b" % re.escape(n)) for n in fam.get("port_names", [])]
    store_re = re.compile(r"PE_StoreU(?:8|16|32)\s*\(\s*([^,]+),\s*([^;]*?)\)\s*;")
    out = []
    for p in sorted((repo / "pc_port").rglob("*")):
        if p.suffix not in (".c", ".h") or "tests" in p.relative_to(repo / "pc_port").parts:
            continue
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for n, line in enumerate(text.splitlines(), 1):
            if not any(x.search(line) for x in pats + names):
                continue
            for m in store_re.finditer(line):
                target, value = m.group(1), m.group(2).strip()
                if not any(x.search(target) for x in pats + names):
                    continue
                lit = re.fullmatch(r"\(?(?:pe_addr_t|uint32_t)?\)?\s*(0x[0-9A-Fa-f]+|\d+)[uU]?", value)
                if lit:
                    v = int(lit.group(1), 0)
                    cls = "zero" if v == 0 else ("identity" if v in fam["handled"] else "other")
                else:
                    cls = "expression"
                out.append({"site": f"{p.relative_to(repo)}:{n}", "target": target.strip(),
                            "value": value, "class": cls})
    return out


def switch_census(tab: dict, exe: "Blob") -> dict:
    entries = []
    for i in range(tab["count"]):
        k = (tab["base"] + 4 * i - exe.vma) // 4
        w = exe.words[k]
        entries.append({"index": i, "label": f"0x{w:08X}", "class": "label" if w in tab["labels"] else "other"})
    bad = [e for e in entries if e["class"] != "label"]
    return {"key": tab["key"], "title": tab["title"], "base": f"0x{tab['base']:08X}",
            "entries": entries, "labels_used": sorted({e["label"] for e in entries}), "fail": len(bad)}


def site_line(fam: dict, repo: Path) -> str:
    path, marker = fam["site"]
    try:
        for n, line in enumerate((repo / path).read_text(encoding="utf-8").splitlines(), 1):
            if marker in line:
                return f"{path}:{n}"
    except OSError:
        pass
    return f"{path}:? (marker {marker!r} not found)"


# ── inputs ─────────────────────────────────────────────────────────────────

SPAN_RE = re.compile(r"^\s*-\s*\[\s*0x([0-9A-Fa-f]+)\s*,\s*(\w+)\s*(?:,\s*(\w+))?")


def spans(path: Path, vram: int, header: int = 0) -> list[tuple[int, str]]:
    out = []
    try:
        for line in path.read_text(encoding="utf-8").splitlines():
            m = SPAN_RE.match(line)
            if m and m.group(2) in ("c", "asm"):
                out.append((vram + int(m.group(1), 16) - header, m.group(3) or ""))
    except OSError:
        return []
    return sorted(out)


def last_jr_ra(data: bytes) -> int:
    n = len(data) // 4
    for k in range(n - 1, -1, -1):
        if struct.unpack_from("<I", data, k * 4)[0] == JR_RA:
            return k
    return -1


def load_blobs(disc: Path, repo: Path) -> list[Blob]:
    # Reuse the existing manifest/table reader, but read all chunks directly.
    # The optional peimg/ extraction cache is deliberately not an inventory.
    sys.path.insert(0, str(repo / "tools/extract"))
    import peimg

    manifest = peimg.load_manifest(repo, peimg.DEFAULT_MANIFEST)
    manifest["image"]["extracted"] = str(disc / "PE.IMG")
    image_path = peimg.open_image(repo, manifest, hash_image=True)
    exe = (disc / "SLUS_006.62").read_bytes()
    if hashlib.sha1(exe).hexdigest() != manifest["exe"]["sha1"]:
        raise ValueError("EXE SHA-1 does not match the overlay manifest")
    peimg.cross_check_manifest(exe, manifest)
    t_addr, t_size = struct.unpack_from("<II", exe, 0x18)
    image = exe[0x800:0x800 + t_size]
    blobs = [Blob("EXE", image, t_addr,
                  code=(0, min(len(image) // 4, (EXE_TEXT_END - t_addr) // 4)))]
    blobs[0].starts = spans(repo / "configs/USA/disc1.yaml", t_addr, 0x800)
    ovls = manifest["overlays"]
    sources = []
    for name, info in ovls.items():
        if info["kind"] == "subsystem":
            sources.append((name, peimg.read_sectors(image_path, info["sector_start"], info["sector_end"])))
    for room in peimg.room_table(exe, manifest):
        start = room["lba"] - manifest["image"]["disc_lba"]
        for index, size in enumerate(room["sec"]):
            if size:
                sources.append((f"room_{room['name']}_c{index}",
                                peimg.read_sectors(image_path, start, start + size)))
            start += size
    for name, data in sources:
        info = ovls.get(name, {})
        vma, code = info.get("vram"), None
        if vma is not None and "text_start" in info:
            code = (info["text_start"] // 4, info["text_end"] // 4)
        elif name.startswith("room_") and name.endswith("_c2"):
            vma, end = ROOM_VMA, last_jr_ra(data)
            code = (0, end + 2 if end >= 0 else 0)
        else:
            code = (0, 0)                        # room chunks 0/1: reference data
        b = Blob(name, data, vma, code=code)
        cfg = repo / "configs/USA/overlays" / f"{name[:-3] if name.endswith('_c2') else name}.yaml"
        if vma is not None and cfg.exists():
            b.starts = spans(cfg, vma)
        blobs.append(b)
    return blobs


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--disc", type=Path, default=DISC, help="extracted disc 1 directory")
    ap.add_argument("--json", type=Path, help="write the full result here")
    ap.add_argument("--check", action="store_true", help="exit 1 on any failing class")
    ap.add_argument("--family", nargs="*", help="limit to these family keys")
    ns = ap.parse_args(argv)
    unknown = set(ns.family or []) - {f["key"] for f in FAMILIES}
    if unknown:
        ap.error("unknown family: " + ", ".join(sorted(unknown)))

    blobs = load_blobs(ns.disc, REPO)
    results, failing = [], 0
    for fam in FAMILIES:
        if ns.family and fam["key"] not in ns.family:
            continue
        res = census_family(fam, blobs)
        res["dispatcher"] = site_line(fam, REPO)
        res["port"] = port_writers(fam, REPO)
        port_fail = [p for p in res["port"] if p["class"] == "other"]
        res["fail"] += len(port_fail)
        failing += res["fail"]
        results.append(res)
        shown = ", ".join(f"{c} {n}" for c, n in sorted(res["counts"].items()))
        print(f"{fam['key']}: {fam['title']}")
        print(f"    range {res['range'][0]}..{res['range'][1]} handled {' '.join(res['handled']) or '(none)'}"
              f" dispatcher {res['dispatcher']}")
        print(f"    {shown or 'no sites'}; port writers {len(res['port'])}; fail {res['fail']}")
        for r in res["rows"]:
            extra = "".join(f" {k}={r[k]}" for k in ("setter", "index", "value", "insn", "address") if k in r)
            fn = f" in {r['function']}" if r.get("function") else ""
            note = f"  -- {r['note']}" if r.get("note") else ""
            print(f"      {r['kind']:8s} {r['site']}{fn}{extra} [{r['class']}]{note}")
        for p in res["port"]:
            print(f"      port     {p['site']} {p['target']} <- {p['value']} [{p['class']}]")
    tables = []
    if not ns.family:
        for tab in SWITCH_TABLES:
            res = switch_census(tab, blobs[0])
            res["dispatcher"] = site_line(tab, REPO)
            failing += res["fail"]
            tables.append(res)
            print(f"{tab['key']}: {tab['title']}")
            print(f"    base {res['base']} entries {tab['count']} dispatcher {res['dispatcher']}; "
                  f"labels used {' '.join(res['labels_used'])}; fail {res['fail']}")
            for e in res["entries"]:
                if e["class"] != "label":
                    print(f"      entry {e['index']} {e['label']} [other]")
    print(f"hook_writer_census: {len(results)} families, {sum(len(r['rows']) for r in results)} retail sites, "
          f"{sum(len(r['port']) for r in results)} port writer lines, {len(tables)} switch tables, "
          f"{failing} failing")
    scope = {
        "images": len(blobs),
        "code_images": sum(b.code[1] > b.code[0] for b in blobs),
        "code_words": sum(b.code[1] - b.code[0] for b in blobs),
        "accepted_rows": sum(r["class"].startswith("accepted/") for f in results for r in f["rows"]),
        "unreferenced_rows": sum(r["class"] == "other-unreferenced" for f in results for r in f["rows"]),
        "limits": ["Computed/interior entries and computed stores are not exhaustive",
                   "Block copies/clears require manual range review",
                   "Port expression writers are not resolved",
                   "Dynamic m28/m32 movement slot+12 is outside fixed-address families",
                   "Accepted exceptions are review assumptions; no executed-native or dormancy proof"],
    }
    print(f"scope: {scope['images']} images, {scope['code_images']} code images, "
          f"{scope['code_words']} code words; {scope['accepted_rows']} accepted rows, "
          f"{scope['unreferenced_rows']} rows without detected entry references")
    print("PASS checks the declared static inventory only; it does not prove dormant arms or native execution.")
    if ns.json:
        ns.json.write_text(json.dumps({"scope": scope, "families": results, "switch_tables": tables}, indent=1) + "\n")
    return 1 if ns.check and failing else 0


if __name__ == "__main__":
    raise SystemExit(main())
