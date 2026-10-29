"""Source-only lowering checks against the retained real-console operation log.

No C compiler or CPU interpreter is used. Run separately from runtime tests:
PE_GTE_HW_LOG=/existing/ps1-tests/gte-fuzz/gte_valid_0xc0ffee_50.log \
  python3 tools/analysis/test_gte_native_lowering.py
"""
import hashlib
import os
from pathlib import Path
import re
import unittest

import guest_lower as gl

HARDWARE_SHA256 = "4577214dec452509eb5e8f3ff4c120d50bb361db13381ce9fee8bad2b3520421"
COMMAND = re.compile(r"GTE 0x([0-9a-f]+) \w+ \(sf=(\d), lm=(\d), tx=(\d), vx=(\d), mx=(\d)\)", re.I)


class NativeGteLowering(unittest.TestCase):
    def test_observed_console_commands_and_selectors_are_accepted(self):
        path = Path(os.environ["PE_GTE_HW_LOG"])
        data = path.read_bytes()
        self.assertEqual(hashlib.sha256(data).hexdigest(), HARDWARE_SHA256)
        records, first = 0, {}
        for match in COMMAND.finditer(data.decode("ascii")):
            op = int(match[1], 16)
            sf, lm, tx, vx, mx = map(int, match.groups()[1:])
            word = 0x4A000000 | op | (sf << 19) | (lm << 10) | (tx << 13) | (vx << 15) | (mx << 17)
            self.assertTrue(gl.gte_command_ok(word), hex(word))
            first.setdefault(op, word)
            records += 1
        self.assertEqual(records, 1100)
        self.assertEqual(len(first), 22)
        # Exercise the actual source-to-native lowering for every observed
        # operation, rather than testing only membership in a policy set.
        for word in first.values():
            source = ('int func_80190000(int a) { __asm__ volatile('
                      f'".word 0x{word:08X}"); return a; }}')
            result = gl.lower_leaf("func_80190000", source, set(), {})
            self.assertIn(f"PE_GTE_Command(0x{word & 0x1FFFFFF:07X}u)", result["body"])
            self.assertNotIn("BOUNDARY", result["body"])

    def test_reserved_operation_and_cpu_instruction_are_refused(self):
        self.assertFalse(gl.gte_command_ok(0x4A000002))
        self.assertFalse(gl.gte_command_ok(0x0000000C))  # syscall, not COP2
        for template in (".word 0x4A000002", "syscall", "jal 0x80010000"):
            with self.subTest(template=template), self.assertRaises(gl.LowerError):
                gl.lower_leaf("func_80190000", 'int func_80190000(void) {'
                              f'__asm__ volatile("{template}"); return 0; }}', set(), {})


if __name__ == "__main__":
    unittest.main()
