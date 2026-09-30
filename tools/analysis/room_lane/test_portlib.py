#!/usr/bin/env python3
"""Unit tests for the struct-field base derivation used by port.py.

Run: python3 tools/analysis/room_lane/test_portlib.py
"""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from portlib import derive_field_base  # noqa: E402


class DeriveFieldBase(unittest.TestCase):
    def test_single_field_reference(self):
        # m0022i &D_80197450.fA (0x8019745A) -> m0034i field at 0x80194F6A.
        m = {0x8019745A: 0x80194F6A}
        self.assertEqual(derive_field_base(m, 0x80197450), 0x80194F60)

    def test_several_fields_same_delta(self):
        m = {0x80197464: 0x80191F64, 0x8019746C: 0x80191F6C, 0x80197470: 0x80191F70}
        self.assertEqual(derive_field_base(m, 0x80197460), 0x80191F60)

    def test_refuses_disagreeing_deltas(self):
        m = {0x80197464: 0x80191F64, 0x8019746C: 0x80192F6C}
        self.assertIsNone(derive_field_base(m, 0x80197460))

    def test_refuses_without_nearby_field(self):
        m = {0x80197600: 0x80191600}
        self.assertIsNone(derive_field_base(m, 0x80197460))

    def test_ignores_addresses_at_or_below_base(self):
        # only fields strictly above the base count; an unrelated lower symbol
        # with a different delta does not block the derivation
        m = {0x80197450: 0x80100000, 0x8019745A: 0x80194F6A}
        self.assertEqual(derive_field_base(m, 0x80197450), 0x80194F60)


if __name__ == "__main__":
    unittest.main()
