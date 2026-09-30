"""Composition of the dispatch-table strip and the rodata-fold strip.

A leaf built with both MASPSX_DISPATCH_FOLD and MASPSX_RODATA_FOLD carries
cc1's `$LC` literals AND its switch table in one `.rodata`
(func_80024250: 0x28-byte `short tbl[20]` literal, then the 0x50
jtbl_800107D4 table). `strip_dispatch_rodata(..., fold_spec=...)` must strip
both in either emission order and refuse anything else. Synthetic objects
are assembled with the project's mipsel `as`; skipped if it is absent.
"""
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if "disc1_build" not in sys.modules:
    sys.path.insert(0, str(ROOT / "tools/build"))
import disc1_build  # noqa: E402

POOL = """\
dlabel jtbl_80010000
    .word 0x80010000
    .word 0x80010004
    .word 0x80010008
    .word 0x8001000C
enddlabel jtbl_80010000
dlabel D_80010100
    .word 0x11112222
    .word 0x33334444
    .word 0x55556666
    .word 0x77778888
enddlabel D_80010100
"""

TEXT = """\
    .set noreorder
    .text
    .globl f
f:
$L1:
    nop
$L2:
    nop
$L3:
    nop
$L4:
    nop
    jr $31
    nop
"""
LITERAL = "lit:\n    .word 0x11112222, 0x33334444, 0x55556666, 0x77778888\n"
TABLE = "tbl:\n    .word $L1, $L2, $L3, $L4\n"


def _toolchain():
    try:
        return disc1_build.find_toolchain()
    except Exception:  # pragma: no cover - environment without mipsel as
        return None


TOOLS = _toolchain()


@unittest.skipIf(TOOLS is None, "mipsel toolchain not available")
class FoldStripCompose(unittest.TestCase):
    def setUp(self):
        (ROOT / "build/tmp").mkdir(parents=True, exist_ok=True)
        self.dir = Path(tempfile.mkdtemp(prefix="foldstrip-", dir=ROOT / "build/tmp"))
        self.pool = self.dir / "pool"
        self.pool.mkdir()
        (self.pool / "800.rodata.s").write_text(POOL)

    def build(self, rdata: str, extra_text: str = "") -> Path:
        src = self.dir / "t.s"
        src.write_text(TEXT + extra_text + "    .rdata\n" + rdata)
        obj = self.dir / "t.o"
        subprocess.run(
            TOOLS.command(TOOLS.assembler, *disc1_build.AS_FLAGS, "-o", str(obj), str(src)),
            check=True, capture_output=True,
        )
        return obj

    def rodata_size(self, obj: Path) -> int:
        out = subprocess.run(
            TOOLS.command(TOOLS.assembler.replace("-as", "-objdump")
                          if TOOLS.assembler.endswith("-as") else "mipsel-linux-gnu-objdump",
                          "-h", str(obj)),
            capture_output=True, text=True,
        ).stdout
        for line in out.splitlines():
            parts = line.split()
            if len(parts) > 2 and parts[1] == ".rodata":
                return int(parts[2], 16)
        return -1

    def strip(self, obj, fold="D_80010100", table="jtbl_80010000"):
        disc1_build.strip_dispatch_rodata(obj, table, self.pool, fold_spec=fold)

    def test_table_plus_fold_literals_first(self):
        obj = self.build("    .align 2\n" + LITERAL + "    .align 3\n" + TABLE)
        self.strip(obj)
        self.assertEqual(0, self.rodata_size(obj))

    def test_fold_plus_table_tables_first(self):
        obj = self.build("    .align 3\n" + TABLE + "    .align 2\n" + LITERAL)
        self.strip(obj)
        self.assertEqual(0, self.rodata_size(obj))

    def test_fold_only(self):
        obj = self.build("    .align 2\n" + LITERAL)
        disc1_build.strip_rodata_fold(obj, "D_80010100", self.pool)
        self.assertEqual(0, self.rodata_size(obj))

    def test_table_only(self):
        obj = self.build("    .align 3\n" + TABLE)
        disc1_build.strip_dispatch_rodata(obj, "jtbl_80010000", self.pool)
        self.assertEqual(0, self.rodata_size(obj))

    def test_refuses_mismatched_literal(self):
        bad = LITERAL.replace("0x33334444", "0x33334445")
        obj = self.build("    .align 2\n" + bad + "    .align 3\n" + TABLE)
        with self.assertRaises(disc1_build.BuildError):
            self.strip(obj)

    def test_refuses_mismatched_size(self):
        # an extra literal word the fold list does not name
        obj = self.build("    .align 2\n" + LITERAL + "    .word 0x9999AAAA, 0xBBBBCCCC, 0xDDDDEEEE, 0x12345678\n"
                         "    .align 3\n" + TABLE)
        with self.assertRaises(disc1_build.BuildError):
            self.strip(obj)

    def test_refuses_table_only_object_with_fold(self):
        # fold named but the object has no literal region
        obj = self.build("    .align 3\n" + TABLE)
        with self.assertRaises(disc1_build.BuildError):
            self.strip(obj)

    def test_refuses_dispatch_only_on_composite(self):
        # the old single-purpose strip must still refuse the composite object
        obj = self.build("    .align 2\n" + LITERAL + "    .align 3\n" + TABLE)
        with self.assertRaises(disc1_build.BuildError):
            disc1_build.strip_dispatch_rodata(obj, "jtbl_80010000", self.pool)

    def test_refuses_leftover_rodata_reference(self):
        # an un-folded `la $4,lit` left in .text would dangle after the strip
        obj = self.build("    .align 2\n" + LITERAL + "    .align 3\n" + TABLE,
                         extra_text="    lui $4,%hi(lit)\n    addiu $4,$4,%lo(lit)\n")
        with self.assertRaises(disc1_build.BuildError):
            self.strip(obj)


if __name__ == "__main__":
    unittest.main()
