#!/usr/bin/env python3
"""Self-test for retail_data_guard.py and strip_retail_seeds.py.

Uses a synthetic, seeded pseudo-random "EXE" (no retail bytes) so it runs in
CI without a disc.  Run: python3 tools/analysis/test_retail_data_guard.py
"""
from __future__ import annotations

import random
import struct
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/analysis"))
import retail_data_guard as g  # noqa: E402
import strip_retail_seeds as srs  # noqa: E402

VRAM = 0x80010000


def fake_exe() -> bytes:
    rng = random.Random(1234)
    body = bytearray(rng.getrandbits(8) for _ in range(0x4000))
    body[0x2000:0x2200] = bytes(512)                       # zero fill
    body[0x2200:0x2280] = bytes(range(128))                # identity ramp
    body[0x2400:0x2480] = bytes([0, 1, 2, 0] * 32)         # low-entropy flags
    for i in range(16):                                    # pointer table
        struct.pack_into("<I", body, 0x2600 + 4 * i, 0x80012000 + 0x40 * i + (i % 3) * 4)
    hdr = bytearray(0x800)
    hdr[:8] = b"PS-X EXE"
    struct.pack_into("<II", hdr, 0x18, VRAM, len(body))
    return bytes(hdr + body)


EXE = fake_exe()
BODY = EXE[0x800:]


def ref_for(exe: bytes = EXE) -> g.Reference:
    r = g.Reference()
    r.add_blob("SLUS_006.62", exe[0x800:], 4, VRAM, tier2=True)
    return r


def words(off: int, n: int):
    return [struct.unpack_from("<I", BODY, off + 4 * i)[0] for i in range(n)]


def scan(text: str, ref=None):
    return g.scan_bytes("x.h", text.encode(), ref or ref_for(), set())


class GuardTest(unittest.TestCase):
    def test_hex_word_array_hits(self):
        t = "static const uint32_t T[]={" + ",".join(f"0x{w:08X}u" for w in words(0x100, 16)) + "};"
        hits = scan(t)
        self.assertEqual(len(hits), 1)
        self.assertEqual(hits[0][4], "data")
        self.assertIn("vaddr 0x80010100", hits[0][3])

    def test_pair_rows_hit(self):
        rows = ",\n".join(f"{{0x{0x10200 + 4 * i:X}u,0x{w:08X}u}}" for i, w in enumerate(words(0x200, 12)))
        self.assertTrue(scan("static const uint32_t R[][2]={\n" + rows + "\n};"))

    def test_decimal_byte_array_hits(self):
        t = "static const uint8_t B[]={" + ",".join(str(b) for b in BODY[0x301:0x341]) + "};"
        self.assertTrue(scan(t))

    def test_store_calls_hit(self):
        t = "\n".join(f"    PE_StoreU32(0x{VRAM + 0x400 + 4 * i:08X}u, 0x{w:08X}u);"
                      for i, w in enumerate(words(0x400, 10)))
        self.assertTrue(scan(t))

    def test_uninformative_windows_ignored(self):
        self.assertFalse(scan("uint8_t z[]={" + ",".join(["0"] * 64) + "};"))
        self.assertFalse(scan("enum{" + ",".join(f"A{i}={i}" for i in range(40)) + "};"))
        self.assertFalse(scan(" ".join(str(i) for i in range(128))))

    def test_restarting_enumeration_ignored(self):
        body = bytearray(BODY)
        body[0x3000:0x3040] = bytes([16] + list(range(0, 40)) + [7] + list(range(40, 62)))
        r = ref_for(EXE[:0x800] + bytes(body))
        self.assertFalse(scan(" ".join(f"[{b}]" for b in body[0x3000:0x3040]), r))

    def test_register_save_sequence_hits(self):
        body = bytearray(BODY)
        seq = [0x27BDFFA8] + [(0x2B << 26) | (29 << 21) | (r << 16) | (84 - 4 * k)
                              for k, r in enumerate([31, 30, 23, 22, 21, 20, 19, 18, 17, 16])]
        struct.pack_into(f"<{len(seq)}I", body, 0x3100, *seq)
        r = ref_for(EXE[:0x800] + bytes(body))
        self.assertTrue(scan(",".join(f"0x{w:08X}" for w in seq), r))

    def test_short_copy_not_flagged(self):
        t = ",".join(f"0x{w:08X}" for w in words(0x500, 7))  # 28 bytes < window
        self.assertFalse(scan(t))

    def test_low_entropy_table_needs_long_run(self):
        flags = BODY[0x2400:0x2480]
        self.assertTrue(scan("uint8_t f[]={" + ",".join(str(b) for b in flags) + "};"))
        self.assertFalse(scan("uint8_t f[]={" + ",".join(str(b) for b in flags[:64]) + "};"))

    def test_pointer_table_is_address_class(self):
        t = ",".join(f"0x{w:08X}u" for w in words(0x2600, 16))
        hits = scan(t)
        self.assertTrue(hits)
        self.assertTrue(all(h[4] == "address-table" for h in hits))

    def test_binary_file_hits(self):
        blob = b"\x89junk" + BODY[0x700:0x780] + b"tail"
        hits = g.scan_bytes("x.bin", blob, ref_for(), set())
        self.assertTrue(hits)

    # ── guard v3: bare (0x-less) hex ──────────────────────────────────
    def test_bare_objdump_listing_hits(self):
        t = "\n".join(f"{VRAM + 0x100 + 4 * i:08x}:\t{w:08x} \tlw\tv0,0(a0)"
                      for i, w in enumerate(words(0x100, 10)))
        hits = scan(t)
        self.assertTrue(hits)
        self.assertEqual(hits[0][4], "data")

    def test_bare_splat_comment_hits(self):
        t = "\n".join(f"/* {0x900 + 4 * i:X} {VRAM + 0x100 + 4 * i:08X} {w:08X} */ lui $v0,0x800B"
                      for i, w in enumerate(words(0x100, 10)))
        self.assertTrue(scan(t))

    def test_bare_words_only_listing_hits(self):
        t = "  ".join(f"{w:08X}" for w in words(0x600, 9))
        self.assertTrue(scan(t))

    def test_bare_memory_order_dump_hits(self):
        # objdump -s: address column then four memory-order 8-digit groups
        rows = []
        for r in range(3):
            off = 0xA00 + 16 * r
            cols = " ".join(BODY[off + 4 * c:off + 4 * c + 4].hex() for c in range(4))
            rows.append(f" {VRAM + off:08x} {cols}  ................")
        self.assertTrue(scan("\n".join(rows)))

    def test_bare_hex_string_hits(self):
        self.assertTrue(scan("retail: " + BODY[0xB01:0xB31].hex() + "\n"))
        self.assertTrue(scan("id,raw_hex\n7," + BODY[0xB41:0xB69].hex().upper() + ",x\n"))

    def test_bare_spaced_byte_dump_hits(self):
        self.assertTrue(scan(" ".join(f"{b:02X}" for b in BODY[0xC03:0xC33])))

    def test_bare_hex_false_positives_ignored(self):
        rng = random.Random(99)
        prose = "\n".join(f"commit {rng.getrandbits(32):08x} sha {rng.getrandbits(160):040x} "
                          f"blob {rng.getrandbits(256):064x}" for _ in range(40))
        self.assertFalse(scan(prose))
        self.assertFalse(scan(" ".join(f"{w:08x}" for w in words(0x100, 7))))   # 28 bytes
        self.assertFalse(scan(BODY[0xB01:0xB1F].hex()))                          # 30 bytes
        self.assertFalse(scan(" ".join(f"{0x80010000 + 16 * i:08x}" for i in range(64))))
        self.assertFalse(scan("00" * 200))

    def test_docs_media_rejected_unless_allowlisted(self):
        png = b"\x89PNG\r\n\x1a\n" + bytes(64)
        hits = g.scan_bytes("docs/evidence/x/frame.png", png, ref_for(), set())
        self.assertEqual(len(hits), 1)
        self.assertEqual(hits[0][4], "data")
        self.assertFalse(g.scan_bytes("docs/evidence/x/frame.png", png, ref_for(),
                                      {"docs/evidence/x/frame.png"}))
        self.assertTrue(g.scan_bytes("docs/a/voice.wav", b"RIFF" + bytes(40), ref_for(), set()))
        self.assertFalse(g.scan_bytes("pc_port/assets/icon.png", png, ref_for(), set()))

    def test_av_and_reference_cache_rejected_anywhere(self):
        for path in ("pc_port/assets/intro.webm", "tools/x/clip.MKV", "a.mp4", "sfx/hit.wav",
                     "m/voice.opus", "m/bgm.ogg", "m/bgm.flac", "build/reference/day1/notes.txt"):
            hits = g.scan_bytes(path, b"x" * 64, ref_for(), set())
            self.assertEqual([h[4] for h in hits], ["data"], path)
        self.assertFalse(g.scan_bytes("a.mp4", b"x", ref_for(), {"a.mp4"}))       # allowlisted
        self.assertFalse(g.scan_bytes("build/references.md", b"notes", ref_for(), set()))
        self.assertFalse(g.scan_bytes("pc_port/ui/logo.png", b"\x89PNG" + bytes(8), ref_for(), set()))

    def test_allow_file_addr_lines(self):
        with tempfile.TemporaryDirectory() as d:
            f = Path(d) / "allow.txt"
            f.write_text("# c\nsrc/a.c  # why\naddr: configs/x.tsv\n\n")
            old = g.ALLOW_FILE
            try:
                g.ALLOW_FILE = f
                self.assertEqual(g.load_allow(), {"src/a.c"})
                self.assertEqual(g.load_allow_addr(), {"configs/x.tsv"})
            finally:
                g.ALLOW_FILE = old

    def test_signatures_detect_without_disc(self):
        r = g.Reference()
        for w, label in [(BODY[0x800:0x820], "known_table@0")]:
            b = g.blake8(w)
            import zlib
            r.sigs.setdefault(zlib.crc32(w), set()).add(b)
            r.sig_labels[b] = label
        t = ",".join(f"0x{w:08X}u" for w in words(0x800, 8))
        hits = g.scan_bytes("x.h", t.encode(), r, set())
        self.assertTrue(hits and "signature known_table" in hits[0][3])


SCRIPT = [
    "Quartermaster, the zeppelin we could not moor beside the lighthouse ever.",
    "Your umbrella was lent to a juggler in the orchard tonight.",
    "Yes", "No", "New Game",
    "Brittle yet fragrant marmalade for the tugboat crew.",
]


def encode_script(line: str) -> bytes:
    """Synthetic text in the retail glyph encoding (letters = ASCII - 0x31)."""
    out = bytearray()
    for ch in line:
        if ch.isalpha():
            out.append(ord(ch) - 0x31)
        else:
            out.append({" ": 0x0F, "!": 0x2B, "'": 0x2E, ".": 0x2F, ",": 0x4A}[ch])
    return bytes(out)


def fake_peimg() -> bytes:
    rng = random.Random(77)
    blob = bytearray(rng.getrandbits(8) for _ in range(0x8000))
    at = 0x100
    for line in SCRIPT:
        e = encode_script(line)
        blob[at:at + len(e)] = e
        blob[at + len(e)] = 0xFF
        at += len(e) + 0x40
    return bytes(blob)


class TextTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ref = g.Reference()
        cls.ref.text.load_disc(fake_peimg(), None)

    def hits(self, text, ref=None):
        return [h for h in g.scan_bytes("docs/x.md", text.encode(), ref or self.ref, set())
                if h[4] == "text"]

    def test_decoder_finds_script(self):
        self.assertIn("your umbrella was lent", self.ref.text.grams)

    def test_quoted_dialogue_hits(self):
        h = self.hits('Aya reads:\n> "YOUR UMBRELLA   was lent to a\n> juggler" (msg 0x21)\n')
        self.assertEqual(len(h), 1)
        self.assertEqual(h[0][0], 2)
        self.assertEqual(h[0][4], "text")

    def test_json_escaped_dialogue_hits(self):
        self.assertTrue(self.hits('{"t": "Your umbrella was\\n lent to a juggler"}'))

    def test_snake_case_item_text_hits(self):
        self.assertTrue(self.hits("#define ITEM_BRITTLE_YET_FRAGRANT_MARMALADE_FOR_THE 7"))

    def test_generic_labels_and_phrases_ignored(self):
        self.assertFalse(self.hits("Menu: Yes / No / New Game / Continue"))
        # 4 retail words but only one content word (generic English)
        self.assertFalse(self.hits('assert ok, "the zeppelin we could not land"'))
        self.assertFalse(self.hits("your umbrella was"))          # 3 words

    def test_signature_fallback(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "sig.txt"
            n = g.retail_text.write_signatures(fake_peimg(), p)
            self.assertTrue(n > 0)
            self.assertNotIn("umbrella", p.read_text())             # hashes only
            r = g.Reference()
            r.text.load_signatures(p)
            self.assertTrue(self.hits("Your umbrella was lent to a juggler in the orchard.", r))
            self.assertFalse(self.hits("Your umbrella was lent.", r))   # < 5-gram
            self.assertFalse(self.hits("Yes No New Game", r))


class StripTest(unittest.TestCase):
    HEADER = (
        "static const uint32_t X_ranges[][2]={\n    {0x10100u,0x40u},\n};\n"
        "static const uint32_t X_common[][2]={\n"
        + "".join(f"    {{0x{0x10100 + 4 * i:X}u,0x{w:X}u}},\n" for i, w in enumerate(words(0x100, 16)))
        + "    {0x150000u,0x12345u},\n};\n"
        "static const uint8_t X_tab[]={1,2," + ",".join(str(b) for b in BODY[0x900:0x940]) + ",9};\n"
        "static const struct { unsigned first,end; uint64_t hash; } X_cases[]={{0,1,UINT64_C(0x1)}};\n"
    )

    def test_strip_then_guard_clean_and_restore_exact(self):
        ref = ref_for()
        new, n, fx = srs.strip(self.HEADER.encode(), ref, "RETAILFIX_x")
        self.assertEqual(n, 16 + 64)
        self.assertFalse(g.scan_bytes("x.h", new, ref_for(), set()))
        self.assertIn(b"static uint32_t X_common", new)
        self.assertIn(b"static uint8_t X_tab", new)
        self.assertIn(b"{0x150000u,0x12345u}", new)          # synthetic row kept
        again, n2, _ = srs.strip(new, ref, "RETAILFIX_x")
        self.assertEqual(again, new)                         # idempotent
        self.assertEqual(n2, 0)
        # restore from the fixup table and compare with the original values
        kinds = {(k, p) for k, p, *_ in fx}
        self.assertIn(("PE_RF_ROWS", "&X_common[0][0]"), kinds)
        self.assertIn(("PE_RF_BYTES", "X_tab"), kinds)
        for k, p, a, b, src, off in fx:
            self.assertEqual(src, "PE_RF_EXE")
            if k == "PE_RF_ROWS":
                self.assertEqual([struct.unpack_from("<I", EXE, off + 4 * i)[0] for i in range(b)],
                                 words(0x100, 16)[a:a + b])
            else:
                self.assertEqual(EXE[off:off + b], (bytes([1, 2]) + BODY[0x900:0x940] + b"\x09")[a:a + b])


if __name__ == "__main__":
    unittest.main()
