"""Regressions for evidence used to accept native overlay replacements."""
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path

import e21_diff as e


class OracleContracts(unittest.TestCase):
    def test_renamed_replacement_retains_retail_call_identity(self):
        text = ("/* host prototypes */\n"
                "pe_addr_t PE_RotMatrixYXZ_Guest(pe_addr_t r, pe_addr_t m);\n\n")
        self.assertEqual(list(e.host_prototypes(text)), [
            ("pe_addr_t", "PE_RotMatrixYXZ_Guest", "pe_addr_t r, pe_addr_t m", 0x80079754)])
        stub = e.host_stubs(text)
        self.assertIn("PE_RotMatrixYXZ_Guest", stub)
        self.assertIn("mock_args(0x80079754u, 2u", stub)

    def test_unknown_host_replacement_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "no retail identity"):
            e.host_stubs("/* host prototypes */\nvoid PE_Unknown(pe_addr_t x);\n\n")

    @unittest.skipUnless(shutil.which("cc"), "host C compiler required")
    def test_thirteenth_argument_survives_the_native_mock(self):
        params = ", ".join(f"int p{i}" for i in range(13))
        text = f"/* host prototypes */\nvoid func_800D2370({params});\n\n"
        code = ("#include <stdint.h>\n#include <stddef.h>\n"
                "static unsigned count; static uint32_t last;\n"
                "uint32_t mock_args(uint32_t fn, unsigned n, const uint32_t *a) "
                "{ count=n; last=a[12]; return fn; }\n" + e.host_stubs(text) +
                "int main(void) { func_800D2370(0,1,2,3,4,5,6,7,8,9,10,11,-123); "
                "return !(count==13 && last==(uint32_t)-123); }\n")
        with tempfile.TemporaryDirectory() as td:
            src, binary = Path(td)/"test.c", Path(td)/"test"
            src.write_text(code)
            subprocess.run(["cc", "-Wall", "-Werror", str(src), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_output_mocks_use_actual_object_sizes(self):
        for target, arg, size in ((0x80079754,1,18), (0x80078CC4,0,20),
                                  (0x800CF3AC,1,3), (0x800CEAE8,2,6)):
            self.assertEqual(len(e.out_bytes(target,arg,17)), size)
        self.assertEqual(e.STACK_EXTENTS[0x800C71E4,1], 32)
        self.assertEqual(e.STACK_EXTENTS[0x800D2B58,2], 3)

    def test_interpreter_exports_gte_state(self):
        # Synthetic test instructions, assembled here; not retail byte data.
        entry = 0x80010000
        words = [(15<<26)|(8<<16)|0xFFFF,      # lui t0,0xFFFF
                 (13<<26)|(8<<21)|(8<<16)|0x1234,
                 (18<<26)|(6<<21)|(8<<16)|(5<<11),  # ctc2 t0,TRX
                 (31<<21)|8, 0]              # jr ra; nop
        ram=bytearray(e.RAM_SIZE+e.SPAD)
        struct.pack_into("<5I",ram,entry&0x1FFFFFFF,*words)
        state=[]
        e.run_mips(ram,entry,[0,0,0,0],0,gte_state=state)
        self.assertEqual(len(state),64)
        self.assertEqual(state[32+5],0xFFFF1234)

    def test_saved_register_local_stores_are_compared_including_call_delay_slot(self):
        # Synthetic callback: preserve incoming s3/ra, then pass a three-word
        # scale vector built with s3.  The final component is a jal delay-slot
        # store, as happens in retail callers.  Register-save bytes must remain
        # excluded while every vector byte participates in parity comparison.
        entry, target = 0x80010000, 0x80078CC4

        def imm(op, rs, rt, value):
            return (op << 26) | (rs << 21) | (rt << 16) | (value & 0xFFFF)

        words = [imm(9, 29, 29, -0x80),        # addiu sp,sp,-0x80
                 imm(0x2B, 29, 19, 0x78),     # sw s3,0x78(sp): actual save
                 imm(0x2B, 29, 31, 0x7C),     # sw ra,0x7c(sp): actual save
                 imm(9, 0, 19, 0x1234),       # assign local scale to s3
                 imm(0x2B, 29, 19, 0x40),     # sw s3,0x40(sp): local data
                 imm(0x2B, 29, 19, 0x44),
                 imm(9, 29, 4, 0x40),         # a0 -> scale vector
                 imm(9, 29, 5, 0x78),         # a1 -> saved-register slots
                 (3 << 26) | ((target >> 2) & 0x3FFFFFF),
                 imm(0x2B, 29, 19, 0x48),     # third component in delay slot
                 imm(0x23, 29, 19, 0x78),
                 imm(0x23, 29, 31, 0x7C),
                 imm(9, 29, 29, 0x80),
                 (31 << 21) | 8, 0]          # jr ra; nop
        ram = bytearray(e.RAM_SIZE + e.SPAD)
        struct.pack_into(f"<{len(words)}I", ram, entry & 0x1FFFFFFF, *words)
        previous_cover = e.COVER
        try:
            e.COVER = set()
            _, calls = e.run_mips(ram, entry, [0, 0, 0, 0], 0)
            self.assertIn(entry + 9 * 4, e.COVER)
        finally:
            e.COVER = previous_cover
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][0], target)
        vector, saves = calls[0][1][:2]
        self.assertEqual(vector[0], "S")
        self.assertEqual(vector[1][:12], struct.pack("<3I", *([0x1234] * 3)))
        self.assertEqual(vector[2][:12], [True] * 12)
        self.assertEqual(saves[0], "S")
        self.assertEqual(saves[2][:8], [False] * 8)


    def test_register_save_in_first_branch_delay_slot_is_excluded(self):
        # Retail m0188i func_80195A14 / m0273i func_801981A4 shape: the last
        # callee-saved register is spilled in the delay slot of the first
        # branch.  That spill is a register save, not caller data.
        entry = 0x80010000

        def imm(op, rs, rt, value):
            return (op << 26) | (rs << 21) | (rt << 16) | (value & 0xFFFF)

        words = [imm(9, 29, 29, -0x28),        # addiu sp,sp,-0x28
                 imm(0x2B, 29, 31, 0x24),     # sw ra,0x24(sp)
                 imm(0x2B, 29, 17, 0x1C),     # sw s1,0x1c(sp)
                 imm(4, 4, 0, 2),             # beq a0,zero,+2
                 imm(0x2B, 29, 16, 0x18),     # sw s0,0x18(sp): delay-slot save
                 imm(0x2B, 29, 16, 0x10),     # sw s0,0x10(sp): s0 already saved -> data
                 0]
        ram = bytearray(e.RAM_SIZE + e.SPAD)
        struct.pack_into(f"<{len(words)}I", ram, entry & 0x1FFFFFFF, *words)
        saves = e.prologue_save_instructions(ram, entry)
        self.assertEqual(saves, frozenset({entry + 4, entry + 8, entry + 16}))
        # A delay-slot store of a register the entry block already assigned
        # stays data.
        words[4] = imm(0x2B, 29, 2, 0x18)    # sw v0,0x18(sp)
        struct.pack_into(f"<{len(words)}I", ram, entry & 0x1FFFFFFF, *words)
        self.assertEqual(e.prologue_save_instructions(ram, entry), frozenset({entry + 4, entry + 8}))

if __name__ == "__main__":
    unittest.main()
