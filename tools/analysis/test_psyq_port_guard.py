#!/usr/bin/env python3
"""Self-test for psyq_port_guard.py (synthetic inputs only: no retail or SDK bytes)."""
from __future__ import annotations

import io
import os
import random
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest import mock
from contextlib import redirect_stdout, redirect_stderr
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import psyq_port_guard as g  # noqa: E402

BIAS = g.TEXT_VRAM_BIAS


class GuardTest(unittest.TestCase):
    def setUp(self):
        self.d = Path(tempfile.mkdtemp(prefix="psyqguard"))
        self.tree = self.d / "pc_port"
        (self.tree / "game" / "decomp").mkdir(parents=True)
        (self.tree / "platform").mkdir()
        self.cls = self.d / "cls.tsv"
        self.cls.write_text(
            "addr\tname\tlibrary\tconfidence\tevidence\tsdk_name\n"
            "0x80070000\tfunc_80070000\tlibgpu\tsignature\tx\tLoadImage\n"
            "0x80070100\tfunc_80070100\tgame\t-\t\t\n"
            "ovl_039F:0x8010C000\tfunc_8010C000\tlibpress\toverlay\tx\t\n")
        # synthetic "table": pseudo-random int16s placed at a fake EXE address
        rnd = random.Random(7)
        self.table = [rnd.randrange(0x100, 0x7FFF) for _ in range(256)]
        tbl_addr = 0x80090000
        exe = bytearray(tbl_addr - BIAS + 1024)
        for i, v in enumerate(self.table):
            exe[tbl_addr - BIAS + 2 * i: tbl_addr - BIAS + 2 * i + 2] = v.to_bytes(2, "little")
        self.exe = self.d / "fake.exe"
        self.exe.write_bytes(bytes(exe))
        self.ranges = self.d / "ranges.tsv"
        self.ranges.write_text(f"name\tstart\tend\tlibrary\nfake_tbl\t0x{tbl_addr:08X}\t0x{tbl_addr + 512:08X}\tlibgte\n")
        self.allow = self.d / "allow.txt"

    def tearDown(self):
        shutil.rmtree(self.d, ignore_errors=True)

    def guard(self, *extra):
        args = ["--tree", str(self.tree), "--classification", str(self.cls), "--ranges", str(self.ranges),
                "--allowlist", str(self.allow), "--exe", str(self.exe), *extra]
        out = io.StringIO()
        with redirect_stdout(out), redirect_stderr(io.StringIO()):
            rc = g.run(args)
        return rc, out.getvalue()

    def test_clean_tree_passes_strict(self):
        (self.tree / "game/decomp/func_80070100_port.c").write_text("void func_80070100(void)\n{\n}\n")
        rc, out = self.guard("--strict")
        self.assertEqual(rc, 0, out)

    def test_psyq_definition_fails(self):
        (self.tree / "platform/pe_libgpu.c").write_text("static int\nfunc_80070000(int a)\n{\n  return a;\n}\n")
        rc, out = self.guard("--strict")
        self.assertEqual(rc, 1)
        self.assertIn("0x80070000", out)

    def test_prototype_is_not_a_definition(self):
        (self.tree / "platform/x.h").write_text("int func_80070000(int a);\n")
        self.assertEqual(self.guard("--strict")[0], 0)

    def test_decomp_source_header_fails(self):
        (self.tree / "game/decomp/a.c").write_text("/* decomp-source: func_80070000 */\n")
        self.assertEqual(self.guard("--strict")[0], 1)

    def test_overlay_row_address(self):
        (self.tree / "game/boot/f.c").parent.mkdir(parents=True, exist_ok=True)
        (self.tree / "game/boot/f.c").write_text("void func_8010C000(void) {}\n")
        rc, out = self.guard("--strict")
        self.assertEqual(rc, 1)
        self.assertIn("libpress", out)

    def test_allowlist_must_shrink(self):
        f = self.tree / "platform/pe_libgpu.c"
        f.write_text("void func_80070000(void) {}\n")
        with redirect_stdout(io.StringIO()):
            g.run(["--tree", str(self.tree), "--classification", str(self.cls), "--ranges", str(self.ranges),
                   "--allowlist", str(self.allow), "--exe", str(self.exe), "--write-allowlist"])
        self.assertEqual(self.guard()[0], 0)            # allowlisted -> pass
        self.assertEqual(self.guard("--strict")[0], 1)  # goal state still fails
        f.write_text("/* replaced */\n")
        rc, out = self.guard()
        self.assertEqual(rc, 1)                         # stale entry must be deleted
        self.assertIn("STALE", out)

    def test_table_u16_run_in_source(self):
        body = ", ".join(f"0x{v:04x}" for v in self.table[40:80])
        (self.tree / "platform/tbl.c").write_text(f"short T[] = {{ {body} }};\n")
        rc, out = self.guard("--strict")
        self.assertEqual(rc, 1)
        self.assertIn("fake_tbl", out)

    def test_table_words_in_case_header(self):
        rows = []
        for i in range(0, 200, 2):
            w = self.table[i] | (self.table[i + 1] << 16)
            rows.append(f"    {{0x{0x1000 + 2 * i:X}u,0x{w:08X}u}},")
        (self.tree / "platform/cases.h").write_text("\n".join(rows) + "\n")
        self.assertEqual(self.guard("--strict")[0], 1)

    def test_table_skipped_without_exe(self):
        body = ", ".join(f"0x{v:04x}" for v in self.table[40:80])
        (self.tree / "platform/tbl.c").write_text(f"short T[] = {{ {body} }};\n")
        args = ["--tree", str(self.tree), "--classification", str(self.cls), "--ranges", str(self.ranges),
                "--allowlist", str(self.allow), "--exe", str(self.d / "missing"), "--strict"]
        with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            self.assertEqual(g.run(args), 0)
            self.assertEqual(g.run(args + ["--require-exe"]), 1)

    def test_game_tu_includes(self):
        p = self.tree / "game/decomp/inc.c"
        for hdr, bad in (('"pe_guest_decomp.h"', False), ("<PsyX/PsyX_public.h>", True), ("<libgpu.h>", True),
                         ('"psx/libgte.h"', True), ("<SDL2/SDL.h>", True), ('"pe_gpu.h"', True),
                         ("<stdint.h>", False)):
            p.write_text(f"#include {hdr}\n")
            self.assertEqual(self.guard("--strict")[0], 1 if bad else 0, hdr)
        # platform/backends may include host headers
        p.unlink()
        (self.tree / "platform/host.c").write_text("#include <SDL2/SDL.h>\n")
        self.assertEqual(self.guard("--strict")[0], 0)

    @unittest.skipUnless(shutil.which("cc") and shutil.which("nm"), "needs cc + nm")
    def test_ignored_vendor_tree_skipped_only_under_third_party(self):
        import subprocess
        if subprocess.run(["git", "init", "-q", str(self.d)], capture_output=True).returncode:
            self.skipTest("git unavailable")
        (self.d / ".gitignore").write_text("pc_port/third_party/vendor/\npc_port/game/decomp/\n")
        vendor = self.tree / "third_party" / "vendor"
        vendor.mkdir(parents=True)
        body = "void func_80070000(void)\n{\n}\n"
        (vendor / "x.c").write_text(body)
        rc, out = self.guard("--strict")
        self.assertEqual(rc, 0, out)            # ignored vendor tree: not scanned
        cwd = os.getcwd()
        try:                                    # relative --tree (the CLI form)
            os.chdir(self.d)
            args = ["--tree", "pc_port", "--classification", str(self.cls), "--ranges", str(self.ranges),
                    "--allowlist", str(self.allow), "--exe", str(self.exe), "--strict"]
            buf = io.StringIO()
            with redirect_stdout(buf), redirect_stderr(io.StringIO()):
                self.assertEqual(g.run(args), 0, buf.getvalue())
        finally:
            os.chdir(cwd)
        (self.tree / "game/decomp/func_80070000_port.c").write_text(body)
        rc, out = self.guard("--strict")
        self.assertEqual(rc, 1, out)            # ignored generated TU: still scanned
        self.assertIn("game/decomp/func_80070000_port.c", out)

    def test_binary_symbol_and_table_bytes(self):
        src = self.d / "o.c"
        body = ", ".join(str(v) for v in self.table)
        src.write_text("void func_80070000(void) {}\nvoid func_80070100(void) {}\n"
                       f"const short tbl[] = {{ {body} }};\n")
        obj = self.d / "o.o"
        subprocess.run(["cc", "-c", "-o", str(obj), str(src)], check=True)
        rc, out = self.guard("--strict", "--no-sources", "--binary", str(obj))
        self.assertEqual(rc, 1)
        self.assertIn("bin:o.o", out)
        self.assertIn("0x80070000", out)
        self.assertIn("fake_tbl", out)
        self.assertNotIn("0x80070100", out)

    def test_binary_without_symbols_is_not_clean(self):
        # GNU nm returns success with empty stdout for the Zig Windows PE
        # executable; its symbols live in a separate PDB. This is missing
        # audit evidence, not a proof that no classified functions are linked.
        binary = self.d / "stripped.exe"
        binary.write_bytes(b"synthetic binary without symbol data")
        for stdout in ("", " \n"):
            with self.subTest(stdout=stdout), \
                 mock.patch.object(g.shutil, "which", return_value="nm"), \
                 mock.patch.object(g.subprocess, "run", return_value=subprocess.CompletedProcess(
                     ["nm"], 0, stdout, "nm: stripped.exe: no symbols\n")):
                with self.assertRaisesRegex(SystemExit, "no defined symbols"):
                    self.guard("--no-sources", "--binary", str(binary))

    def test_failed_symbol_dump_with_partial_output_is_rejected(self):
        binary = self.d / "partial.exe"
        binary.write_bytes(b"synthetic binary with an incomplete symbol dump")
        with mock.patch.object(g.shutil, "which", return_value="nm"), \
             mock.patch.object(g.subprocess, "run", return_value=subprocess.CompletedProcess(
                 ["nm"], 1, "00001000 T func_80070100\n", "nm: symbol dump failed\n")):
            with self.assertRaisesRegex(SystemExit, "nm failed"):
                self.guard("--no-sources", "--binary", str(binary))

    def test_readable_binary_without_classified_functions_is_clean(self):
        binary = self.d / "game-only.exe"
        binary.write_bytes(b"synthetic game-only binary without table data")
        with mock.patch.object(g.shutil, "which", return_value="nm"), \
             mock.patch.object(g.subprocess, "run", return_value=subprocess.CompletedProcess(
                 ["nm"], 0, "00001000 T func_80070100\n", "")):
            rc, out = self.guard("--no-sources", "--binary", str(binary))
            self.assertEqual(rc, 0, out)

    def test_pdb_pair_checks_symbols_and_binary_tables(self):
        binary = self.d / "port.exe"
        binary.write_bytes(b"synthetic binary prefix" + b"".join(v.to_bytes(2, "little") for v in self.table))
        pdb = self.d / "port.pdb"
        with mock.patch.object(g, "pdb_symbols", return_value={0x80070000, 0x80070100}) as read, \
             mock.patch.object(g, "nm_symbols") as nm:
            rc, out = self.guard("--strict", "--no-sources", "--binary-pdb", str(binary), str(pdb))
        read.assert_called_once_with(binary, pdb)
        nm.assert_not_called()
        self.assertEqual(rc, 1, out)
        self.assertIn("bin:port.exe", out)
        self.assertIn("0x80070000", out)
        self.assertIn("fake_tbl", out)
        self.assertNotIn("0x80070100", out)

    def test_pdb_pair_can_supply_inventory_for_explicit_binary(self):
        binary = self.d / "port.exe"
        binary.write_bytes(b"synthetic game-only binary")
        pdb = self.d / "port.pdb"
        with mock.patch.object(g, "pdb_symbols", return_value={0x80070100}) as read, \
             mock.patch.object(g, "nm_symbols") as nm:
            rc, out = self.guard("--strict", "--no-sources", "--binary", str(binary),
                                 "--binary-pdb", str(binary), str(pdb))
        read.assert_called_once_with(binary, pdb)
        nm.assert_not_called()
        self.assertEqual(rc, 0, out)

    def test_pdb_mapping_duplicates_are_rejected(self):
        binary = self.d / "port.exe"
        pdb = self.d / "port.pdb"
        for second in (pdb, self.d / "other.pdb"):
            with self.subTest(second=second), mock.patch.object(g, "pdb_symbols") as read:
                with self.assertRaises(SystemExit) as failure:
                    self.guard("--no-sources", "--binary-pdb", str(binary), str(pdb),
                               "--binary-pdb", str(binary.parent / "." / binary.name), str(second))
                self.assertEqual(failure.exception.code, 2)
                read.assert_not_called()

    def test_pdb_failure_does_not_fall_back_to_nm(self):
        binary = self.d / "port.exe"
        with mock.patch.object(g, "pdb_symbols", side_effect=SystemExit("PDB identity mismatch")), \
             mock.patch.object(g, "nm_symbols") as nm:
            with self.assertRaisesRegex(SystemExit, "identity mismatch"):
                self.guard("--no-sources", "--binary-pdb", str(binary), str(self.d / "wrong.pdb"))
        nm.assert_not_called()

    def test_pdb_preserves_filename_and_allowlist_shrink_checks(self):
        binary = self.d / "port.exe"
        binary.write_bytes(b"synthetic binary")
        args = ("--no-sources", "--binary-pdb", str(binary), str(self.d / "port.pdb"))
        self.allow.write_text("func\t0x80070000\tbin:port\n")
        with mock.patch.object(g, "pdb_symbols", return_value={0x80070000}):
            rc, out = self.guard(*args)
            self.assertEqual(rc, 1, out)
            self.assertIn("NEW   func\t0x80070000\tbin:port.exe", out)
            self.assertNotIn("STALE", out)  # bin:port was not checked
            self.allow.write_text("func\t0x80070000\tbin:port.exe\n")
            self.assertEqual(self.guard(*args)[0], 0)
        with mock.patch.object(g, "pdb_symbols", return_value={0x80070100}):
            rc, out = self.guard(*args)
            self.assertEqual(rc, 1, out)
            self.assertIn("STALE func\t0x80070000\tbin:port.exe", out)


if __name__ == "__main__":
    unittest.main()
