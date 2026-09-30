#!/usr/bin/env python3
"""Retail script-text detector for retail_data_guard.py.

Owner decision 2026-09-28: no decoded game script / dialogue / item text may
live in a tracked file.  Anything that needs it reads it from the user's disc
at runtime or from a git-ignored cache.

Reference (disc): every text run in PE.IMG decoded with the retail glyph map
of tools/research/pe_txt0_decode.py (letters = code + 0x31, the PUNCT table,
0xF7 newline), kept when it looks like prose (see _plausible).  Each run is
normalized to lowercase words ([a-z0-9]+, apostrophes dropped) and indexed
as word 4-grams.  The decoded set is cached only under the git-ignored disc
cache (build/disc-cache/<key>/retail_text_4grams.txt).

Reference (no disc): tools/analysis/retail_text_signatures.txt, keyed
48-bit BLAKE2b hashes of the normalized word 5-grams (one-way; 5-grams so
the list cannot be walked back into sentences word by word).

A tracked text file is flagged when a run of consecutive words, all covered
by matching n-grams, has >= MIN_WORDS words, >= MIN_CHARS normalized
characters (words joined by single spaces) and >= MIN_CONTENT content words
(length >= 4 and not a function word).  Short UI labels and generic English
phrases that merely overlap the script never trip it; a quoted line of
dialogue or an item description does.
"""
from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TEXT_SIG_FILE = ROOT / "tools/analysis/retail_text_signatures.txt"
TEXT_CACHE_NAME = "retail_text_4grams.txt"
TEXT_VERSION = 1

N_DISC = 4          # word n-gram length with the disc
N_SIG = 5           # word n-gram length of the committed signatures
MIN_WORDS = 4
MIN_CHARS = 20
MIN_CONTENT = 2
SIG_KEY = b"pe-retail-text-v1"

STOPWORDS = frozenset("""
a about above after again against all also am an and any are as at be been
before being below between both but by can could did do does doing down
during each few for from further had has have having he her here hers him his
how i if in into is it its just me more most my no nor not now of off on once
only or other our out over own same she should so some such than that the
their them then there these they this those through to too under until up
very was we were what when where which while who whom why will with would you
your yours been into onto upon than this that with from have were will your
what when them they there their then just like also only some such very""".split())

_WORD = re.compile(rb"[A-Za-z0-9']+")
_ESC = re.compile(rb"\\[nrt]")


def _glyph_table() -> bytes:
    """byte -> ASCII from the repo's retail decoder (0 = not text)."""
    sys.path.insert(0, str(ROOT / "tools/research"))
    try:
        import pe_txt0_decode as dec
    finally:
        sys.path.pop(0)
    t = bytearray(256)
    for b in list(range(0x10, 0x2A)) + list(range(0x30, 0x4A)):
        t[b] = b + dec.LETTER_BASE
    for b, (_kind, ch) in dec.PUNCT.items():
        t[b] = ord(ch)
    t[0xF7] = ord(" ")          # NEWLINE token
    return bytes(t)


def norm_words(text: str) -> list[str]:
    return [w for w in re.findall(r"[a-z0-9]+", text.lower().replace("'", "")) if w]


def _plausible(t: str) -> bool:
    ws = t.split()
    if len(ws) < N_DISC:
        return False
    letters = [c for c in t if c.isalpha()]
    if len(letters) < 12 or sum(c.islower() for c in letters) < 0.5 * len(letters):
        return False
    real = [w for w in ws if len(w.strip(".,!'")) >= 2 and re.search(r"[aeiouyAEIOUY]", w)]
    return len(real) >= 0.6 * len(ws)


def decode_runs(peimg: bytes) -> set[str]:
    """Normalized (space-joined lowercase words) text runs from PE.IMG."""
    tab = _glyph_table()
    cls = bytes(b for b in range(256) if tab[b])
    run = re.compile(b"[" + re.escape(cls) + b"]{12,}")
    out = set()
    for m in run.finditer(peimg):
        t = m.group(0).translate(tab).decode("latin1")
        if _plausible(t):
            # the byte before a message (FF/F9 FE <id>) often decodes as a
            # letter glued to the first word: keep both readings
            for v in (t, t[1:]):
                ws = norm_words(v)
                if len(ws) >= N_DISC:
                    out.add(" ".join(ws))
    return out


def grams(ws: list[str], n: int):
    for i in range(len(ws) - n + 1):
        yield " ".join(ws[i:i + n])


def gram_hash(g: str) -> str:
    return hashlib.blake2b(g.encode(), digest_size=6, key=SIG_KEY).hexdigest()


class TextReference:
    def __init__(self):
        self.grams: set[str] = set()      # disc: exact 4-grams
        self.sigs: set[str] = set()       # offline: hashed 5-grams

    @property
    def empty(self) -> bool:
        return not self.grams and not self.sigs

    def load_disc(self, peimg: bytes | None, cache_dir: Path | None):
        """Exact 4-grams from the disc (cached in the git-ignored disc cache)."""
        cp = cache_dir / TEXT_CACHE_NAME if cache_dir else None
        if cp and cp.exists():
            lines = cp.read_text().splitlines()
            if lines and lines[0] == f"# v{TEXT_VERSION}":
                self.grams = set(lines[1:])
                return
        if peimg is None:
            return
        runs = decode_runs(peimg)
        self.grams = {g for t in runs for g in grams(t.split(), N_DISC)}
        if cp:
            try:
                tmp = cp.with_suffix(".tmp")
                tmp.write_text(f"# v{TEXT_VERSION}\n" + "\n".join(sorted(self.grams)) + "\n")
                tmp.replace(cp)
            except OSError:
                pass

    def load_signatures(self, path: Path = TEXT_SIG_FILE) -> int:
        if not path.exists():
            return 0
        self.sigs = {l.strip() for l in path.read_text().splitlines()
                     if l.strip() and not l.startswith("#")}
        return len(self.sigs)


def write_signatures(peimg: bytes, path: Path = TEXT_SIG_FILE) -> int:
    runs = decode_runs(peimg)
    hs = sorted({gram_hash(g) for t in runs for g in grams(t.split(), N_SIG)})
    path.write_text(
        "# retail_text signatures: keyed 48-bit BLAKE2b of normalized word 5-grams\n"
        "# of the retail script text (one-way hashes, NOT text).  Regenerate with\n"
        "# tools/analysis/retail_data_guard.py --write-signatures (needs the disc).\n"
        f"# version={TEXT_VERSION} n={N_SIG}\n" + "\n".join(hs) + "\n")
    return len(hs)


def _content(w: str) -> bool:
    return len(w) >= 4 and w not in STOPWORDS


def scan_text(data: bytes, tref: TextReference):
    """Return [(from_line, to_line, nchars, match)] for script-text runs."""
    if tref.empty:
        return []
    data2 = _ESC.sub(b"  ", data)           # "\n" escapes in JSON/C strings
    toks = []
    for m in _WORD.finditer(data2):
        w = m.group(0).lower().replace(b"'", b"")
        if w:
            toks.append((w.decode("latin1"), m.start()))
    words = [t[0] for t in toks]
    out = []
    for n, test in ((N_DISC, lambda g: g in tref.grams), (N_SIG, lambda g: gram_hash(g) in tref.sigs)):
        if (n == N_DISC and not tref.grams) or (n == N_SIG and not tref.sigs):
            continue
        if n == N_SIG and tref.grams:
            continue                         # exact disc reference wins
        hit = [False] * len(words)
        for i in range(len(words) - n + 1):
            if test(" ".join(words[i:i + n])):
                for k in range(i, i + n):
                    hit[k] = True
        i = 0
        while i < len(words):
            if not hit[i]:
                i += 1
                continue
            j = i
            while j < len(words) and hit[j]:
                j += 1
            ws = words[i:j]
            nch = len(" ".join(ws))
            if len(ws) >= MIN_WORDS and nch >= MIN_CHARS and sum(map(_content, ws)) >= MIN_CONTENT:
                a, b = toks[i][1], toks[j - 1][1]
                out.append((data.count(b"\n", 0, a) + 1, data.count(b"\n", 0, b) + 1, nch,
                            f"retail script text ({len(ws)} words)"))
            i = j
    return out
