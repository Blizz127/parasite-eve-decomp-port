#!/usr/bin/env python3
"""disc_cache.py tests: the known-disc table matches the C first-run check,
and identification rejects synthetic non-retail images.  No disc needed."""
from __future__ import annotations

import re
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/extract"))
import disc_cache  # noqa: E402


class TableTest(unittest.TestCase):
    def test_c_table_matches_python(self):
        src = (ROOT / "pc_port/platform/pe_disc_check.c").read_text()
        rows = re.findall(r'\{\s*"(\w+)",\s*(\d+),\s*"([\w-]+)",\s*"\\\\([\w.]+);1",\s*'
                          r'(NULL|"[0-9a-f]{40}"),\s*(NULL|"[0-9a-f]{40}")\s*\}', src)
        c = [(r[0], int(r[1]), r[2], r[3], None if r[4] == "NULL" else r[4].strip('"'),
              None if r[5] == "NULL" else r[5].strip('"')) for r in rows]
        py = [(r[0], r[1], r[2], r[3], r[4], r[6]) for r in disc_cache.KNOWN_DISCS]
        self.assertEqual(c, py)

    def test_usa_disc1_exe_hash(self):
        row = [r for r in disc_cache.KNOWN_DISCS if r[:2] == ("USA", 1)][0]
        self.assertEqual(row[4], "452fb033f2eaa4b18aa20a5bca60b8125af3a37b")


class IdentifyTest(unittest.TestCase):
    def test_rejects_non_bin(self):
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "x.bin"
            p.write_bytes(b"\0" * 1000)
            with self.assertRaises(disc_cache.DiscError):
                disc_cache.identify(p)


if __name__ == "__main__":
    unittest.main()
