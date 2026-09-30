import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


V0_BASE = [
    "lwl\t$8,3($2)",
    "lwr\t$8,0($2)",
    "swl\t$8,27($sp)",
    "swr\t$8,24($sp)",
]
S2_BASE = [
    "lwl\t$8,3($18)",
    "lwr\t$8,0($18)",
    "swl\t$8,27($sp)",
    "swr\t$8,24($sp)",
]


class TestUnalignedWordCopyReturnTemp(unittest.TestCase):
    @staticmethod
    def process(lines, on=True):
        with patch.dict(os.environ, {"MASPSX_UNALIGNED_WORD_COPY_RETURN_TEMP": "0"}):
            out = MaspsxProcessor(
                lines, unaligned_word_copy_return_temp=on
            ).process_lines()
        return [x for x in strip_comments(out) if x and not x.startswith(".")]

    def test_v0_base_uses_v1(self):
        self.assertEqual(
            [
                "lwl\t$3,3($2)",
                "lwr\t$3,0($2)",
                "nop",
                "swl\t$3,27($sp)",
                "swr\t$3,24($sp)",
            ],
            self.process(V0_BASE),
        )

    def test_other_base_uses_v0(self):
        self.assertEqual(
            [
                "lwl\t$2,3($18)",
                "lwr\t$2,0($18)",
                "nop",
                "swl\t$2,27($sp)",
                "swr\t$2,24($sp)",
            ],
            self.process(S2_BASE),
        )

    def test_environment_flag(self):
        with patch.dict(os.environ, {"MASPSX_UNALIGNED_WORD_COPY_RETURN_TEMP": "1"}):
            out = MaspsxProcessor(V0_BASE).process_lines()
        got = [x for x in strip_comments(out) if x and not x.startswith(".")]
        self.assertEqual(self.process(V0_BASE), got)

    def test_flag_off_keeps_t0(self):
        self.assertEqual(
            [
                "lwl\t$8,3($2)",
                "lwr\t$8,0($2)",
                "nop",
                "swl\t$8,27($sp)",
                "swr\t$8,24($sp)",
            ],
            self.process(V0_BASE, on=False),
        )

    def test_unrelated_lwl_unchanged(self):
        lines = ["lwl\t$8,7($4)", "nop"]
        self.assertEqual(lines, self.process(lines))
