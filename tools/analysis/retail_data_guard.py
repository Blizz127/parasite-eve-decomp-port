#!/usr/bin/env python3
"""Retail-data guard: fail when retail game bytes/tables are committed.

Policy (CLAUDE.md hard rule 3 + owner decision 2026-09-28 "run off the
disc"): nothing copied from the retail disc — EXE text/data, libgte tables,
PE.IMG overlay bytes — may live in a tracked file.  Tests and the port read
those bytes from the user's own disc image at runtime (or from the git-ignored
disc cache generated from it; see tools/extract/disc_cache.py).

What is scanned (tracked files, or the staged index with --staged):
  * every numeric literal in a text file (hex ``0x..`` and decimal), as one
    token stream per file, plus its stride-2 and stride-3 sub-streams (so
    ``{addr, value}`` pair tables and ``Store(addr, value);`` sequences are
    caught), encoded as little-endian u32 / u16 / u8 byte runs;
  * bare hex without a ``0x`` prefix (guard v3+): 8-digit words such as
    objdump / splat listings (``8003e100: 8c820000  lw``,
    ``8005DC4C 3C02800B``), as a word stream (all, stride-2/3 sub-streams and
    one with KSEG0/IO address columns dropped), each packed as the word value
    and byte-swapped (``objdump -s`` memory-order columns); and contiguous hex
    byte strings of >= 32 bytes, optionally ``' '``/``':'``-separated per byte
    (``retail: 0000828c2400...``, CSV ``raw_hex`` columns, ``FF FE 14 1F ..``);
  * decoded game script text (guard v6, tools/analysis/retail_text.py): a
    quoted run of retail dialogue / item text (>= 4 words, >= 20 normalized
    characters, >= 2 content words), matched against the script text decoded
    from the user's PE.IMG, or offline against retail_text_signatures.txt;
  * any tracked file under ``build/reference/`` and any audio/video file
    (.webm .mkv .mp4 .wav .opus .ogg .flac) anywhere, unless allowlisted;
  * media files (images/audio/video) under ``docs/``: game frames and sounds
    are retail data too; only paths in the allowlist may be committed;
  * the raw bytes of every binary (non-UTF-8) file.
A hit is any 32-byte window (>= 10 distinct byte values, so zero fill and
trivial patterns never trip it) that equals retail bytes.

Reference data, in order of preference:
  1. the user's disc — the EXE (every 4-aligned window) and PE.IMG (every
     64-byte-aligned window, so any copied PE.IMG run >= 96 bytes is caught),
     found via the disc cache (tools/extract/disc_cache.py) or --exe/--peimg;
  2. always: the committed signature file retail_data_signatures.txt —
     crc32 + truncated BLAKE2b of retail windows (one-way hashes, not bytes)
     for the known tables (libgte sqrt/normalize tables, ...) and a 256-byte
     stride sample of the whole EXE, so CI without a disc still fails on the
     known tables and on any EXE run >= 288 bytes.

Exit status: 0 clean, 1 hits, 2 usage/config error.

  retail_data_guard.py                       scan all tracked files
  retail_data_guard.py --staged              scan the staged index (pre-commit)
  retail_data_guard.py FILE...               scan specific files (working tree)
  retail_data_guard.py --write-signatures    regenerate the signature file (disc)
  retail_data_guard.py --write-text-signatures  regenerate the text signatures (disc)
  retail_data_guard.py --no-disc             signatures only (CI behaviour)
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import zlib
from array import array
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SIG_FILE = ROOT / "tools/analysis/retail_data_signatures.txt"
ALLOW_FILE = ROOT / "tools/analysis/retail_data_guard_allow.txt"
BASELINE_FILE = ROOT / "tools/analysis/retail_data_guard_baseline.txt"
CACHE_DIR = Path(os.environ.get("PE_DISC_CACHE_DIR") or ROOT / "build/disc-cache")
GUARD_VERSION = 7
sys.path.insert(0, str(ROOT / "tools/extract"))
sys.path.insert(0, str(ROOT / "tools/analysis"))
import retail_text  # noqa: E402

WIN = 32                # bytes per compared window
MIN_DISTINCT = 10       # distinct byte values required in a window
WIN2 = 128              # tier 2: long low-entropy runs (flag/count tables)
MIN_DISTINCT2 = 3
PEIMG_STRIDE = 64       # PE.IMG reference sampling stride
SIG_EXE_STRIDE = 256    # offline EXE sample stride
EXE_VRAM = 0x80010000
EXE_HDR = 0x800

# Known retail tables recorded as signatures (EXE file offsets derive from
# vaddr - 0x80010000 + 0x800).  Evidence: pc_port/tests headers seeded these
# ranges verbatim before the 2026-09-28 disc-cache migration.
KNOWN_TABLES = [
    ("libgte_sqrt_table", 0x800960BC, 0x180),
    ("libgte_normalize_table", 0x80096250, 0x180),
]

TOKEN_RE = re.compile(rb"0[xX][0-9A-Fa-f]+|\b[0-9]+\b")
# bare (0x-less) 8-digit hex words: objdump/splat instruction columns
BARE_WORD_RE = re.compile(rb"(?<![0-9A-Za-z_])[0-9A-Fa-f]{8}(?![0-9A-Za-z_])")
# contiguous bare hex byte strings >= WIN bytes, optionally byte-separated
BARE_HEXSTR_RE = re.compile(
    rb"(?<![0-9A-Za-z_])(?:[0-9A-Fa-f]{2}[ :]?){%d,}" % WIN)
MEDIA_SUFFIXES = {".png", ".jpg", ".jpeg", ".gif", ".bmp", ".webp", ".tga", ".tim",
                  ".ico", ".wav", ".mp3", ".ogg", ".flac", ".vag", ".xa", ".vab",
                  ".seq", ".str", ".avi", ".mp4", ".mkv", ".webm", ".mov"}
SKIP_SUFFIXES = {".png", ".jpg", ".jpeg", ".gif", ".ico", ".pdf", ".zip", ".gz"}


def eprint(*a):
    print(*a, file=sys.stderr)


_ARR = {4: "I", 2: "H", 1: "B"}


def window_ok(w: bytes, width: int = 4) -> bool:
    """Reject windows that carry no retail-specific information: few distinct
    byte values (fill, padding) or a near-arithmetic unit sequence (counters,
    enum numbering 0,1,2,..., identity tables)."""
    if len(set(w)) < MIN_DISTINCT:
        return False
    return _not_ramp(w, width)


def _not_ramp(w: bytes, width: int) -> bool:
    # a single repeated step (0,1,2,3,...; 4,8,12,...) is a counter, not data;
    # slow-varying tables (atan, sqrt) have at least two distinct steps
    # u8/u16 (guard v5): also a counter with up to two breaks (16,0,1,2,...:
    # an enumerated list whose numbering restarts).  Not for u32: register
    # save sequences (sw ra/fp/s7../s0) are near-arithmetic words and real data.
    u = array(_ARR[width], w[:len(w) - len(w) % width])
    deltas = [(u[i + 1] - u[i]) for i in range(len(u) - 1)]
    if len(set(deltas)) < 2:
        return False
    if width == 4:
        return True
    top = max(deltas.count(d) for d in set(deltas))
    return top < len(deltas) - 2


def window2_ok(w: bytes, width: int = 4) -> bool:
    """Tier 2: a 128-byte window of a low-entropy table (small flags/counts)
    is still retail data when it matches exactly over that length."""
    return len(set(w)) >= MIN_DISTINCT2 and _not_ramp(w, width)


def address_only(w: bytes) -> bool:
    """Every u32 is a KSEG0 RAM address or a hardware I/O register address: a
    pointer/jump/register table.  These are the
    same facts as the committed symbol map, so they are reported separately
    (class 'address-table') and only fail under --strict."""
    return all(0x80000000 <= x < 0x80200000 or 0x1F800000 <= x < 0x1F802000
               for x in array("I", w))


def blake8(w: bytes) -> str:
    return hashlib.blake2b(w, digest_size=8).hexdigest()


# ── reference index ──────────────────────────────────────────────────────

class Reference:
    """crc32 -> [(label, base_offset, blob_id)] with exact confirmation."""

    def __init__(self):
        self.blobs: list[tuple[str, bytes, int]] = []  # (label, data, vram base or 0)
        self.index: dict[int, list[tuple[int, int]]] = {}
        self.index2: dict[int, list[tuple[int, int]]] = {}
        self.sigs: dict[int, set[str]] = {}
        self.sigs2: dict[int, set[str]] = {}
        self.sig_labels: dict[str, str] = {}
        self.text = retail_text.TextReference()

    def add_blob(self, label: str, data: bytes, stride: int, vbase: int = 0,
                 tier2: bool = False):
        bid = len(self.blobs)
        self.blobs.append((label, data, vbase))
        idx = self.index
        crc = zlib.crc32
        for off in range(0, len(data) - WIN + 1, stride):
            w = data[off:off + WIN]
            if len(set(w)) < MIN_DISTINCT:
                continue
            idx.setdefault(crc(w), []).append((bid, off))
        if tier2:
            idx2 = self.index2
            for off in range(0, len(data) - WIN2 + 1, stride):
                w = data[off:off + WIN2]
                if len(set(w)) < MIN_DISTINCT2 or len(set(w)) >= MIN_DISTINCT and \
                        len(set(w[:WIN])) >= MIN_DISTINCT:
                    continue  # tier 1 already covers informative windows
                idx2.setdefault(crc(w), []).append((bid, off))

    def load_signatures(self, path: Path) -> int:
        n = 0
        if not path.exists():
            return 0
        for line in path.read_text().splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            c, b, label = line.split(None, 2)
            (self.sigs2 if label.startswith("t2_") else self.sigs).setdefault(
                int(c, 16), set()).add(b)
            self.sig_labels[b] = label
            n += 1
        return n

    def lookup_pos(self, w: bytes, width: int = 4):
        """(blob label, blob bytes, offset) of an exact disc match, else None."""
        c = zlib.crc32(w)
        if c not in self.index or not window_ok(w, width):
            return None
        for bid, off in self.index[c]:
            label, data, _ = self.blobs[bid]
            if data[off:off + WIN] == w:
                return label, data, off
        return None

    def lookup_pos2(self, w: bytes, width: int = 4):
        c = zlib.crc32(w)
        if c not in self.index2 or not window2_ok(w, width):
            return None
        for bid, off in self.index2[c]:
            label, data, _ = self.blobs[bid]
            if data[off:off + WIN2] == w:
                return label, data, off
        return None

    def lookup2(self, w: bytes, width: int = 4):
        c = zlib.crc32(w)
        if c not in self.index2 and c not in self.sigs2:
            return None
        if not window2_ok(w, width):
            return None
        for bid, off in self.index2.get(c, ()):
            label, data, vbase = self.blobs[bid]
            if data[off:off + WIN2] == w:
                return (f"{label}+0x{off:X}" if not vbase
                        else f"{label} vaddr 0x{vbase + off:08X}") + " (low-entropy run)"
        s = self.sigs2.get(c)
        if s:
            b = blake8(w)
            if b in s:
                return f"signature {self.sig_labels[b]}"
        return None

    def lookup(self, w: bytes, width: int = 4):
        c = zlib.crc32(w)
        if c not in self.index and c not in self.sigs:
            return None
        if not window_ok(w, width):
            return None
        for bid, off in self.index.get(c, ()):
            label, data, vbase = self.blobs[bid]
            if data[off:off + WIN] == w:
                where = (f"{label}+0x{off:X}" if not vbase
                         else f"{label} vaddr 0x{vbase + off:08X}")
                return where
        s = self.sigs.get(c)
        if s:
            b = blake8(w)
            if b in s:
                return f"signature {self.sig_labels[b]}"
        return None

    @property
    def empty(self) -> bool:
        return not self.index and not self.sigs and not self.index2 and self.text.empty


def exe_windows_for_signatures(exe: bytes):
    out = []
    for name, vaddr, size in KNOWN_TABLES:
        base = vaddr - EXE_VRAM + EXE_HDR
        for off in range(base, base + size - WIN + 1, 4):
            w = exe[off:off + WIN]
            if window_ok(w):
                out.append((w, f"{name}@0x{vaddr + off - base:08X}"))
    for off in range(EXE_HDR, len(exe) - WIN + 1, SIG_EXE_STRIDE):
        w = exe[off:off + WIN]
        if window_ok(w):
            out.append((w, f"exe_sample@0x{EXE_VRAM + off - EXE_HDR:08X}"))
        else:
            w2 = exe[off:off + WIN2]
            if len(w2) == WIN2 and window2_ok(w2) and not address_only(w2):
                out.append((w2, f"t2_exe_sample@0x{EXE_VRAM + off - EXE_HDR:08X}"))
    return out


def load_disc_blobs(args) -> tuple[bytes | None, Path | None]:
    exe = peimg = None
    if args.exe:
        exe = Path(args.exe)
    if args.peimg:
        peimg = Path(args.peimg)
    if exe is None and not args.no_disc:
        try:
            import disc_cache  # tools/extract/disc_cache.py
            d = disc_cache.cache_dir_if_ready()
            if d is not None:
                exe = d / "SLUS_006.62"
                p = d / "PE.IMG"
                peimg = peimg or (p if p.exists() else None)
        except Exception as exc:  # cache is optional
            eprint(f"[guard] disc cache unavailable: {exc}")
    exe_bytes = exe.read_bytes() if exe and exe.exists() else None
    return exe_bytes, (peimg if peimg and peimg.exists() else None)


# ── candidate streams from a file ────────────────────────────────────────

def line_of(data: bytes, pos: int) -> int:
    return data.count(b"\n", 0, pos) + 1


def token_streams(data: bytes):
    """Yield (packed bytes, token positions, width) segments."""
    vals = []
    poss = []
    for m in TOKEN_RE.finditer(data):
        t = m.group(0)
        v = int(t, 16) if t[:2] in (b"0x", b"0X") else int(t)
        vals.append(v)
        poss.append(m.start())
    if len(vals) < 8:
        return
    # flat stream in u32/u16/u8, and the two stride-2 sub-streams in u32
    # ({addr, value} pair tables, Store(addr, value); sequences)
    plans = [(vals, poss, 4), (vals, poss, 2), (vals, poss, 1),
             (vals[0::2], poss[0::2], 4), (vals[1::2], poss[1::2], 4)]
    for sv, sp, width in plans:
        need = WIN // width
        lim = 1 << (8 * width)
        bad = [i for i, v in enumerate(sv) if v >= lim]
        bad.append(len(sv))
        start = 0
        for e in bad:
            if e - start >= need:
                yield (array(_ARR[width], sv[start:e]).tobytes(),
                       sp[start:e], width)
            start = e + 1


def _is_address(v: int) -> bool:
    return 0x80000000 <= v < 0x80200000 or 0x1F800000 <= v < 0x1F802000


def bare_hex_streams(data: bytes):
    """Yield (packed bytes, positions, width) segments for 0x-less hex.

    Words: every bare 8-digit token, its stride-2/3 sub-streams (address +
    word [+ bytes] listing columns) and a stream with address-valued tokens
    dropped (objdump -d/-s address columns), each as the u32 value and
    byte-swapped.  Strings: runs of >= WIN hex bytes decoded in order.  Both
    need a full WIN-byte exact disc match downstream, so prose, short hashes
    and addresses never trip them."""
    ms = list(BARE_WORD_RE.finditer(data))
    need = WIN // 4
    if len(ms) >= need:
        vals = [int(m.group(0), 16) for m in ms]
        poss = [m.start() for m in ms]
        plans = [(vals, poss)]
        for k in (2, 3):
            for s in range(k):
                plans.append((vals[s::k], poss[s::k]))
        keep = [i for i, v in enumerate(vals) if not _is_address(v)]
        if len(keep) != len(vals):
            plans.append(([vals[i] for i in keep], [poss[i] for i in keep]))
        for sv, sp in plans:
            if len(sv) < need:
                continue
            a = array("I", sv)
            yield a.tobytes(), sp, 4
            a.byteswap()
            yield a.tobytes(), sp, 4
    for m in BARE_HEXSTR_RE.finditer(data):
        digits = re.sub(rb"[ :]", b"", m.group(0))
        digits = digits[:len(digits) - len(digits) % 2]
        b = bytes.fromhex(digits.decode())
        if len(b) < WIN:
            continue
        # position of byte i: walk the matched text (separators are optional)
        pos, p, t = [], m.start(), m.group(0)
        j = 0
        while len(pos) < len(b):
            while t[j:j + 1] in (b" ", b":"):
                j += 1
            pos.append(p + j)
            j += 2
        yield b, pos, 1


def _collect_runs(buf: bytes, pos: list[int], width: int, ref: Reference, out: list):
    """Slide 32-byte windows over one encoded segment; merge overlapping hit
    windows into runs (first token pos, last token pos, bytes, match, class)."""
    n = len(pos)
    need = WIN // width
    run = None  # [start_i, last_i, where, is_data]
    for i in range(0, n - need + 1):
        w = buf[i * width:i * width + WIN]
        where = ref.lookup(w, width)
        if not where:
            continue
        data_cls = not (width == 4 and address_only(w))
        if run is not None and i <= run[1] + need:
            run[1] = i
            run[3] = run[3] or data_cls
            continue
        if run is not None:
            out.append(run)
        run = [i, i, where, data_cls]
    if run is not None:
        out.append(run)
    res = [(pos[r[0]], pos[r[1] + need - 1], (r[1] - r[0]) * width + WIN, r[2],
            "data" if r[3] else "address-table") for r in out]
    # tier 2: long exact runs of low-entropy tables
    need2 = WIN2 // width
    if (ref.index2 or ref.sigs2) and n >= need2:
        run = None
        for i in range(0, n - need2 + 1):
            w = buf[i * width:i * width + WIN2]
            where = ref.lookup2(w, width)
            if not where or (width == 4 and address_only(w)):
                continue
            if run is not None and i <= run[1] + need2:
                run[1] = i
                continue
            if run is not None:
                res.append((pos[run[0]], pos[run[1] + need2 - 1],
                            (run[1] - run[0]) * width + WIN2, run[2], "data"))
            run = [i, i, where]
        if run is not None:
            res.append((pos[run[0]], pos[run[1] + need2 - 1],
                        (run[1] - run[0]) * width + WIN2, run[2], "data"))
    return res


# audio/video containers are rejected anywhere in the tree (retail captures)
AV_SUFFIXES = {".webm", ".mkv", ".mp4", ".wav", ".opus", ".ogg", ".flac"}
REFERENCE_DIR = "build/reference/"   # the owner's retail reference video cache


def is_docs_media(path: str) -> bool:
    """Image/audio/video under docs/: game frames and sounds are retail data."""
    return path.startswith("docs/") and Path(path).suffix.lower() in MEDIA_SUFFIXES


def forbidden_path(path: str) -> str | None:
    """Why a tracked path is rejected outright (media / reference cache)."""
    if path.startswith(REFERENCE_DIR):
        return "file under build/reference/ (retail reference video cache)"
    if Path(path).suffix.lower() in AV_SUFFIXES:
        return "audio/video file (not allowlisted)"
    if is_docs_media(path):
        return "media file under docs/ (not allowlisted)"
    return None


def scan_bytes(path: str, data: bytes, ref: Reference, allow: set[str]):
    """Return [(from_line, to_line, nbytes, match, class)]."""
    if path in allow:
        return []
    why = forbidden_path(path)
    if why:
        return [("file", "file", len(data), why, "data")]
    if Path(path).suffix.lower() in SKIP_SUFFIXES:
        return []
    try:
        data.decode("utf-8")
        is_text = True
    except UnicodeDecodeError:
        is_text = False
    raw = []
    if is_text:
        for buf, pos, width in token_streams(data):
            raw.extend(_collect_runs(buf, pos, width, ref, []))
        for buf, pos, width in bare_hex_streams(data):
            raw.extend(_collect_runs(buf, pos, width, ref, []))
    else:
        raw.extend(_collect_runs(data, list(range(len(data))), 1, ref, []))
    texthits = []
    if is_text:
        texthits = [(a, b, n, w, "text") for a, b, n, w in retail_text.scan_text(data, ref.text)]
    # merge runs found by several encodings/strides over the same text span
    raw.sort()
    merged = []
    for a, b, nb, where, cls in raw:
        if merged and a <= merged[-1][1]:
            m = merged[-1]
            m[1] = max(m[1], b)
            m[2] = max(m[2], nb)
            if cls == "data":
                m[4] = "data"
            continue
        merged.append([a, b, nb, where, cls])
    if is_text:
        return [(line_of(data, a), line_of(data, b), nb, w, c) for a, b, nb, w, c in merged] + texthits
    return [(f"byte 0x{a:X}", f"byte 0x{b:X}", nb, w, c) for a, b, nb, w, c in merged]


# ── file sets ────────────────────────────────────────────────────────────

def git(*args) -> bytes:
    return subprocess.run(["git", "-C", str(ROOT), *args], check=True,
                          stdout=subprocess.PIPE).stdout


def blob_id(data: bytes) -> str:
    return hashlib.sha1(b"blob %d\0" % len(data) + data).hexdigest()


def iter_files(args):
    """Yield (path, blob sha, loader) — loader() returns the bytes."""
    if args.files:
        for f in args.files:
            p = Path(f)
            try:
                rel = str(p.resolve().relative_to(ROOT))
            except ValueError:
                rel = f
            data = p.read_bytes()
            yield rel, blob_id(data), (lambda d=data: d)
        return
    if args.staged:
        out = git("ls-files", "-s", "-z").split(b"\0")
        staged = set(git("diff", "--cached", "--name-only", "--diff-filter=ACMR", "-z").split(b"\0"))
        for rec in out:
            if not rec:
                continue
            meta, name = rec.split(b"\t", 1)
            if name not in staged:
                continue
            sha = meta.split()[1].decode()
            yield name.decode(), sha, (lambda n=name: git("cat-file", "blob", ":" + n.decode()))
        return
    # tracked files, as they are in the working tree (what would be committed)
    for rec in git("ls-files", "-s", "-z").split(b"\0"):
        if not rec:
            continue
        meta, name = rec.split(b"\t", 1)
        p = ROOT / name.decode()
        if not p.is_file():
            continue
        data = p.read_bytes()
        yield name.decode(), blob_id(data), (lambda d=data: d)


BASELINE_HEADER = """# retail_data_guard known debt: <allowed retail bytes>\t<tracked file>
# Files listed here predate the 2026-09-28 "run off the disc" decision and
# still carry retail bytes.  The guard lets them keep (never grow) that
# amount; every other file must be clean.  Shrink this list: move the bytes
# to runtime disc reads and delete the line.  Regenerate only by lead
# decision: retail_data_guard.py --write-baseline (needs the disc).
"""


def load_baseline() -> dict[str, int]:
    out = {}
    if BASELINE_FILE.exists():
        for line in BASELINE_FILE.read_text().splitlines():
            if line and not line.startswith("#"):
                n, f = line.split("\t", 1)
                out[f] = int(n)
    return out


def _allow_lines():
    if not ALLOW_FILE.exists():
        return []
    out = []
    for line in ALLOW_FILE.read_text().splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            out.append(line)
    return out


def load_allow() -> set[str]:
    """Paths exempt from every check (plain lines of the allowlist)."""
    return {l for l in _allow_lines() if not l.startswith("addr:")}


def load_allow_addr() -> set[str]:
    """``addr: <path>`` lines: pointer/jump-table (address-only) runs in that
    file are accepted under --strict; retail data runs still fail."""
    return {l[5:].strip() for l in _allow_lines() if l.startswith("addr:")}


def write_signatures(exe: bytes) -> int:
    if hashlib.sha1(exe).hexdigest() != "452fb033f2eaa4b18aa20a5bca60b8125af3a37b":
        eprint("[guard] refusing to write signatures from a non-USA-Disc-1 EXE")
        return 2
    rows = exe_windows_for_signatures(exe)
    lines = ["# retail_data_guard signatures: crc32 blake2b-8 label (one-way hashes",
             "# of 32-byte retail EXE windows; NOT retail bytes).  Regenerate with",
             "# tools/analysis/retail_data_guard.py --write-signatures (needs the disc).",
             f"# window={WIN} min_distinct={MIN_DISTINCT} exe_stride={SIG_EXE_STRIDE}"]
    seen = set()
    for w, label in rows:
        b = blake8(w)
        if b in seen:
            continue
        seen.add(b)
        lines.append(f"{zlib.crc32(w):08x} {b} {label}")
    SIG_FILE.write_text("\n".join(lines) + "\n")
    print(f"[guard] wrote {len(seen)} signatures to {SIG_FILE.relative_to(ROOT)}")
    return 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("files", nargs="*")
    ap.add_argument("--staged", action="store_true")
    ap.add_argument("--no-disc", action="store_true", help="signatures only")
    ap.add_argument("--exe", help="retail SLUS_006.62 (default: disc cache)")
    ap.add_argument("--peimg", help="retail PE.IMG (default: disc cache)")
    ap.add_argument("--write-signatures", action="store_true")
    ap.add_argument("--write-text-signatures", action="store_true",
                    help="regenerate retail_text_signatures.txt (needs PE.IMG)")
    ap.add_argument("--json", help="write hits as JSON here")
    ap.add_argument("--write-baseline", action="store_true",
                    help="record current hits as known debt (lead decision only)")
    ap.add_argument("--quiet-baseline", action="store_true")
    ap.add_argument("--strict", action="store_true",
                    help="also fail on pointer/jump-table (address-only) runs")
    args = ap.parse_args(argv)

    exe, peimg = load_disc_blobs(args)
    if args.write_text_signatures:
        if peimg is None:
            eprint("[guard] --write-text-signatures needs PE.IMG (run tools/extract/disc_cache.py populate)")
            return 2
        n = retail_text.write_signatures(peimg.read_bytes())
        print(f"[guard] wrote {n} text signatures to {retail_text.TEXT_SIG_FILE.relative_to(ROOT)}")
        return 0
    if args.write_signatures:
        if exe is None:
            eprint("[guard] --write-signatures needs the disc (run tools/extract/disc_cache.py populate)")
            return 2
        return write_signatures(exe)

    if not SIG_FILE.exists() and exe is None:
        eprint(f"[guard] no reference data (missing {SIG_FILE.relative_to(ROOT)})")
        return 2
    mode = ["signatures"] + (["EXE"] if exe is not None else []) + \
        (["PE.IMG"] if peimg is not None else [])
    eprint(f"[guard] reference: {', '.join(mode)}")
    _ref: list[Reference] = []

    def get_ref() -> Reference:
        """Build the reference index on the first cache miss only."""
        if not _ref:
            r = Reference()
            r.load_signatures(SIG_FILE)
            if exe is not None:
                r.add_blob("SLUS_006.62", exe[EXE_HDR:], 4, EXE_VRAM, tier2=True)
            pdata = peimg.read_bytes() if peimg is not None else None
            if pdata is not None:
                r.add_blob("PE.IMG", pdata, PEIMG_STRIDE)
            r.text.load_disc(pdata, peimg.parent if peimg is not None else None)
            del pdata
            if r.text.empty:
                r.text.load_signatures()
            _ref.append(r)
        return _ref[0]

    allow = load_allow()
    allow_addr = load_allow_addr()
    baseline = load_baseline()
    mode_key = hashlib.sha1(json.dumps([GUARD_VERSION, WIN, MIN_DISTINCT, PEIMG_STRIDE,
                                        SIG_FILE.read_bytes().hex() if SIG_FILE.exists() else "",
                                        hashlib.sha1(retail_text.TEXT_SIG_FILE.read_bytes()).hexdigest()
                                        if retail_text.TEXT_SIG_FILE.exists() else "",
                                        retail_text.TEXT_VERSION,
                                        hashlib.sha1(exe).hexdigest() if exe else "",
                                        bool(peimg)]).encode()).hexdigest()
    cache_path = CACHE_DIR / f"guard-{mode_key[:16]}.json"
    try:
        cache = json.loads(cache_path.read_text())
    except (OSError, ValueError):
        cache = {}
    nfiles = 0
    report = []
    for path, sha, load in iter_files(args):
        nfiles += 1
        if path in allow:
            continue
        hits = cache.get(sha)
        if hits is None:
            hits = [list(h) for h in scan_bytes(path, load(), get_ref(), set())]
            cache[sha] = hits
        for a, b, nb, where, cls in hits:
            if cls == "address-table" and path in allow_addr:
                continue
            report.append({"file": path, "from": a, "to": b, "bytes": nb,
                           "match": where, "class": cls})
    try:
        CACHE_DIR.mkdir(parents=True, exist_ok=True)
        tmp = cache_path.with_suffix(".tmp")
        tmp.write_text(json.dumps(cache))
        tmp.replace(cache_path)
    except OSError:
        pass
    # known debt: files listed in the baseline may keep (not grow) their runs
    debt = {}
    for r in report:
        if r["class"] == "data" and r["file"] in baseline:
            debt[r["file"]] = debt.get(r["file"], 0) + r["bytes"]
    grown = {f for f, n in debt.items() if n > baseline[f]}
    if args.write_baseline:
        rows = sorted((f, n) for f, n in
                      ((f, sum(r["bytes"] for r in report if r["file"] == f and r["class"] == "data"))
                       for f in {r["file"] for r in report}) if n)
        BASELINE_FILE.write_text(BASELINE_HEADER + "".join(f"{n}\t{f}\n" for f, n in rows))
        eprint(f"[guard] wrote {len(rows)} baseline entries to {BASELINE_FILE.relative_to(ROOT)}")
        return 0
    for f in sorted(grown):
        eprint(f"[guard] baseline file grew: {f} now {debt[f]} bytes > allowed {baseline[f]}")
    if debt and not args.quiet_baseline:
        eprint(f"[guard] note: {len(debt)} baseline file(s) still carry {sum(debt.values())} "
               f"retail bytes (known debt in {BASELINE_FILE.relative_to(ROOT)}; shrink it)")
    report = [r for r in report if not (r["file"] in debt and r["file"] not in grown)]
    fails = [r for r in report if r["class"] != "address-table" or args.strict]
    infos = [r for r in report if r not in fails]
    for r in fails[:300]:
        loc = f"{r['file']}:{r['from']}" + (f"-{r['to']}" if r["to"] != r["from"] else "")
        unit = "chars" if r["class"] == "text" else "bytes"
        print(f"RETAIL-DATA {loc}: {r['bytes']} {unit} match {r['match']}")
    if len(fails) > 300:
        print(f"... {len(fails) - 300} more (see --json)")
    if args.json:
        Path(args.json).write_text(json.dumps(report, indent=1))
    if infos:
        eprint(f"[guard] note: {len(infos)} pointer/jump-table run(s) "
               f"(address facts, same as the symbol map) — fail only under --strict")
    if fails:
        files = sorted({r["file"] for r in fails})
        eprint(f"[guard] FAIL: {len(fails)} retail byte run(s) in {len(files)} file(s) "
               f"(scanned {nfiles}).")
        eprint("[guard] Load these bytes from the user's disc at runtime "
               "(pc_port/platform/pe_disc_cache.h) instead of committing them.")
        return 1
    eprint(f"[guard] OK: {nfiles} file(s), no retail byte runs")
    return 0


if __name__ == "__main__":
    sys.exit(main())
