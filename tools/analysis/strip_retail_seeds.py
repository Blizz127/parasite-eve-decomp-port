#!/usr/bin/env python3
"""Strip retail bytes out of generated native-test headers.

The retail-oracle test headers (pc_port/tests/retail_*_cases.h) seed guest
RAM from ``{address, value}`` row tables and flat byte/word tables.  Wherever
those values are copies of retail disc bytes (boot EXE .data/.rodata, libgte
tables, PE.IMG overlay data), committing them would commit retail bytes.

This tool finds every such region with the retail-data guard's matcher,
zeroes it in the header, and appends one fixup table per header:

    static const PE_TestRetailFixup RETAILFIX_<stem>[]={
        {PE_RF_ROWS, &ARR[0][0], first, count, PE_RF_EXE,   src_off},
        {PE_RF_BYTES, ARR,       byte_off, nbytes, PE_RF_PEIMG, src_off},
    };

  PE_RF_ROWS : rows[first..first+count)[1] = LE32 words of the source file
               from src_off (the rows' addresses are contiguous, step 4)
  PE_RF_BYTES: bytes [byte_off, byte_off+nbytes) of the array = source bytes
  PE_RF_EXE  : the boot EXE file (SLUS_006.62, PS-X header included)
  PE_RF_PEIMG: PE.IMG

pc_port/tests/test_retail_fixups.h restores them at test time from the user's
disc (pc_port/platform/pe_disc_cache.h).  Case expectations (state hashes,
results) are one-way/computed values and are left untouched.

Only regions the guard would flag are stripped (an informative, non-address
32-byte window matches the disc), extended to the full matching run, so the
tests keep as much disc-free data as possible.  Idempotent.  Generators must
pipe fresh headers through it:

  strip_retail_seeds.py [--check] HEADER...     (reference: the disc cache)
--check: exit 1 if a header still carries strippable retail bytes.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/analysis"))
sys.path.insert(0, str(ROOT / "tools/extract"))
import retail_data_guard as guard  # noqa: E402

ROWS_DECL_RE = re.compile(
    rb"static\s+(const\s+)?uint32_t\s+([A-Za-z_][A-Za-z0-9_]*)\s*\[\s*[A-Za-z0-9_]*\s*\]\s*\[\s*2\s*\]\s*=\s*\{")
FLAT_DECL_RE = re.compile(
    rb"static\s+(const\s+)?(uint8_t|uint16_t|uint32_t|unsigned char|unsigned short|unsigned int)"
    rb"\s+([A-Za-z_][A-Za-z0-9_]*)\s*\[\s*[A-Za-z0-9_]*\s*\]\s*=\s*\{")
ROW_RE = re.compile(rb"\{\s*(0[xX][0-9A-Fa-f]+|[0-9]+)[uU]?\s*,\s*(0[xX][0-9A-Fa-f]+|[0-9]+)([uU]?)\s*\}")
NUM_RE = re.compile(rb"(0[xX][0-9A-Fa-f]+|[0-9]+)([uU]?)")
FIX_RE = re.compile(rb"\n/\* Retail bytes stripped by tools/analysis/strip_retail_seeds\.py.*?\n\};\n", re.S)
FIXROW_RE = re.compile(rb"\{(PE_RF_ROWS|PE_RF_BYTES),\s*([^,]+),\s*(\d+)u,\s*(\d+)u,\s*(PE_RF_EXE|PE_RF_PEIMG),\s*0x([0-9A-F]+)u\}")
WIDTH = {b"uint8_t": 1, b"unsigned char": 1, b"uint16_t": 2, b"unsigned short": 2,
         b"uint32_t": 4, b"unsigned int": 4}
EXE_HDR = 0x800
# --strict parity: also strip pointer/jump tables (address-only runs), which
# retail_data_guard.py --strict fails on.  On by default.
STRIP_ADDRESSES = True


def _int(t: bytes) -> int:
    return int(t, 16) if t[:2] in (b"0x", b"0X") else int(t)


def fixup_name(path: Path) -> str:
    stem = re.sub(r"_cases$", "", re.sub(r"^retail_", "", path.stem))
    return "RETAILFIX_" + re.sub(r"[^A-Za-z0-9_]", "_", stem)


def find_block_end(data: bytes, start: int) -> int:
    depth = 1
    for i in range(start, len(data)):
        c = data[i]
        if c == 0x7B:
            depth += 1
        elif c == 0x7D:
            depth -= 1
            if depth == 0:
                return i
    raise ValueError("unterminated initializer")


def src_of(label: str, off: int) -> tuple[str, int]:
    """Guard blob offset -> (PE_RF source, file offset)."""
    if label == "SLUS_006.62":
        return "PE_RF_EXE", off + EXE_HDR
    return "PE_RF_PEIMG", off


def matches(buf: bytes, ref: guard.Reference, step: int, width: int):
    """Maximal disc-matching regions of buf: [(start, end, label, blob_off)].
    Seeds on informative non-address windows, then extends byte-exactly in
    `step` units."""
    out = []
    i = 0
    n = len(buf)
    while i + guard.WIN <= n:
        w = buf[i:i + guard.WIN]
        hit = ref.lookup_pos(w, width)
        span = guard.WIN
        if not hit or (width == 4 and not STRIP_ADDRESSES and guard.address_only(w)):
            w2 = buf[i:i + guard.WIN2]
            hit = ref.lookup_pos2(w2, width) if len(w2) == guard.WIN2 else None
            span = guard.WIN2
            if not hit or (width == 4 and not STRIP_ADDRESSES and guard.address_only(w2)):
                i += step
                continue
        label, blob, off = hit
        s, e, bo = i, i + span, off
        while s - step >= 0 and bo - step >= 0 and buf[s - step:s] == blob[bo - step:bo]:
            s -= step
            bo -= step
        while e + step <= n and off + (e - i) + step <= len(blob) and \
                buf[e:e + step] == blob[off + (e - i):off + (e - i) + step]:
            e += step
        if out and s < out[-1][1]:
            s = out[-1][1]  # never overlap a previous region
            bo = off - (i - s)
        if e > s:
            out.append((s, e, label, bo))
        i = e
    return out


def strip(data: bytes, ref: guard.Reference, fixname: str):
    existing = []
    m = FIX_RE.search(data)
    if m:
        existing = [(k.decode(), p.decode().strip(), int(a), int(b), s.decode(), int(o, 16))
                    for k, p, a, b, s, o in FIXROW_RE.findall(m.group(0))]
        data = data[:m.start()] + b"\n" + data[m.end():]
    fixups = list(existing)
    fixed = {f[1].lstrip("&").split("[")[0] for f in existing}
    edits = []  # (start, end, replacement) on data
    nstrip = 0

    for dm in ROWS_DECL_RE.finditer(data):
        name = dm.group(2).decode()
        bs = dm.end()
        be = find_block_end(data, bs)
        rows = list(ROW_RE.finditer(data, bs, be))
        vals = [(_int(r.group(1)), _int(r.group(2))) for r in rows]
        # address-contiguous segments
        k = 0
        while k < len(rows):
            j = k + 1
            while j < len(rows) and vals[j][0] == vals[j - 1][0] + 4 and vals[j][1] < 1 << 32:
                j += 1
            if j - k >= guard.WIN // 4 and all(v < 1 << 32 for _, v in vals[k:j]):
                buf = b"".join(v.to_bytes(4, "little") for _, v in vals[k:j])
                for s, e, label, bo in matches(buf, ref, 4, 4):
                    first, count = k + s // 4, (e - s) // 4
                    src, so = src_of(label, bo)
                    fixups.append(("PE_RF_ROWS", f"&{name}[0][0]", first, count, src, so))
                    for r in rows[first:first + count]:
                        edits.append((r.start(2), r.end(3), b"0u"))
                    nstrip += count
                    fixed.add(name)
            k = j

    for dm in FLAT_DECL_RE.finditer(data):
        name = dm.group(3).decode()
        width = WIDTH[dm.group(2)]
        bs = dm.end()
        be = find_block_end(data, bs)
        toks = list(NUM_RE.finditer(data, bs, be))
        vals = [_int(t.group(1)) for t in toks]
        if len(vals) * width < guard.WIN or any(v >= 1 << (8 * width) for v in vals):
            continue
        buf = b"".join(v.to_bytes(width, "little") for v in vals)
        for s, e, label, bo in matches(buf, ref, 1, width):
            # widen to whole elements
            s2, e2 = s - s % width, e + (-e) % width
            bo -= s - s2
            src, so = src_of(label, bo)
            fixups.append(("PE_RF_BYTES", name, s2, e2 - s2, src, so))
            for t in toks[s2 // width:e2 // width]:
                edits.append((t.start(1), t.end(2), b"0u" if width == 4 else b"0"))
            nstrip += (e2 - s2) // width
            fixed.add(name)

    # apply edits back to front
    out = bytearray(data)
    for a, b, rep in sorted(edits, reverse=True):
        out[a:b] = rep
    data = bytes(out)
    # drop const from arrays that will be patched at test time
    for name in fixed:
        data = re.sub(rb"static\s+const\s+((?:uint8_t|uint16_t|uint32_t|unsigned char|"
                      rb"unsigned short|unsigned int)\s+" + name.encode() + rb"\s*\[)",
                      rb"static \1", data, count=1)
    if fixups:
        fixups = sorted(set(fixups), key=lambda f: (f[1], f[2]))
        tail = [b"/* Retail bytes stripped by tools/analysis/strip_retail_seeds.py: these",
                b" * regions are zero here and restored at test time from the user's disc",
                b" * (test_retail_fixups.h / pe_disc_cache.h).  Never commit them. */",
                f"static const PE_TestRetailFixup {fixname}[]={{".encode()]
        tail += [f"    {{{k},{p},{a}u,{b}u,{s},0x{o:X}u}},".encode() for k, p, a, b, s, o in fixups]
        tail += [b"};", b""]
        data = data.rstrip(b"\n") + b"\n\n" + b"\n".join(tail)
        # RETAILFIX tables are referenced from other test headers, so the
        # stripped header is included early by test_native.c as well.
        if not data.startswith(b"#pragma once"):
            data = b"#pragma once\n" + data
    return data, nstrip, fixups


def build_reference(args) -> guard.Reference:
    ns = argparse.Namespace(exe=args.exe, peimg=args.peimg, no_disc=False)
    exe, peimg = guard.load_disc_blobs(ns)
    if exe is None:
        raise SystemExit("ERROR: needs the disc: run tools/extract/disc_cache.py populate "
                         "(or pass --exe/--peimg)")
    ref = guard.Reference()
    ref.add_blob("SLUS_006.62", exe[EXE_HDR:], 4, 0x80010000, tier2=True)
    if peimg is not None:
        ref.add_blob("PE.IMG", peimg.read_bytes(), guard.PEIMG_STRIDE)
    return ref


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("headers", nargs="+")
    ap.add_argument("--exe")
    ap.add_argument("--peimg")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--keep-addresses", action="store_true",
                    help="leave pointer/jump tables (address-only runs) in place")
    a = ap.parse_args(argv)
    global STRIP_ADDRESSES
    STRIP_ADDRESSES = not a.keep_addresses
    ref = build_reference(a)
    dirty = 0
    for h in a.headers:
        p = Path(h)
        data = p.read_bytes()
        new, n, fx = strip(data, ref, fixup_name(p))
        if new != data:
            dirty += 1
            if a.check:
                print(f"NEEDS-STRIP {h}: {n} element(s)")
            else:
                p.write_bytes(new)
                print(f"stripped {h}: {n} element(s), {len(fx)} fixup(s)")
    return 1 if (a.check and dirty) else 0


if __name__ == "__main__":
    sys.exit(main())
