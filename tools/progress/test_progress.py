#!/usr/bin/env python3
"""Regression tests for generated native metrics and port-priority ordering."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "progress"))

from native_metrics import derive_metrics  # noqa: E402
from port_priority import derive_priority, priority_key  # noqa: E402


class ProgressTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.metrics = derive_metrics(ROOT)
        cls.priority = derive_priority(ROOT)

    def test_native_test_populations_close(self) -> None:
        tests = self.metrics["tests"]
        self.assertEqual(
            tests["static_cases"],
            tests["artifact_independent_cases"] + tests["retail_disc_required_cases"],
        )
        self.assertGreater(tests["retail_disc_required_cases"], 0)

    def test_frontier_dependencies_are_closed(self) -> None:
        for dependency in self.priority["frontier_dependencies"]:
            self.assertTrue(dependency["matching_c"], dependency["name"])
            self.assertTrue(dependency["native_linked"], dependency["name"])
            self.assertTrue(dependency["native_reachable"], dependency["name"])

    def test_priority_is_deterministic_and_ranked(self) -> None:
        candidates = self.priority["candidates"]
        self.assertEqual(candidates, sorted(candidates, key=priority_key))
        self.assertEqual(
            [candidate["rank"] for candidate in candidates],
            list(range(1, len(candidates) + 1)),
        )

    def test_word_count_cannot_beat_port_or_fanout_signal(self) -> None:
        base = {
            "frontier_direct": False,
            "unresolved_native_reference": False,
            "native_reachable": False,
            "native_distance": None,
            "native_reference_count": 0,
            "retail_callers": 1,
            "words": 2,
            "offset": 0x100,
        }
        relevant = dict(base, native_reachable=True, native_distance=9, words=100)
        self.assertLess(priority_key(relevant), priority_key(base))
        high_fanout = dict(base, retail_callers=2, words=100)
        self.assertLess(priority_key(high_fanout), priority_key(base))


if __name__ == "__main__":
    unittest.main()
