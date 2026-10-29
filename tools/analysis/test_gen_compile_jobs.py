#!/usr/bin/env python3
"""Bounded unit/mock tests for the compile-gate worker limit (JOBS).

Run: python3 -B tools/analysis/test_gen_compile_jobs.py

compile_check() (gen_decomp_ports.py, also used by gen_overlay_ports.py)
syntax-checks every rendered TU with the host compiler on a thread pool.  The
base implementation sized that pool as min(16, os.cpu_count()) and ignored
JOBS, so `JOBS=4 scripts/build_pc_port.sh` still ran up to 16 compilers at
once.  These tests pin both sides:

  * the base implementation, read with git from its pinned blob (verified by
    content hash), sizes the pool from the CPU count and runs more than JOBS
    checks at once;
  * the corrected compile_jobs()/compile_check() honour JOBS for unset, valid,
    invalid and edge values, never consult os.cpu_count(), and fail closed
    (SystemExit before any compiler or executor starts) on an unusable value.

No compiler runs: subprocess.run is replaced by a recording fake and the cc
argument names a path that does not exist.  Thread waits are bounded by
barrier timeouts.  The base-implementation tests skip when the base blob is
not in the repository.
"""
from __future__ import annotations

import ast
import concurrent.futures
import hashlib
import importlib.util
import os
import subprocess
import sys
import tempfile
import threading
import types
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
sys.path.insert(0, str(HERE))
import gen_decomp_ports as g  # noqa: E402

# tools/analysis/gen_decomp_ports.py before this change: the same blob at the
# Decomp Bot head 72bd43a6 and at candidate 9d585931.
BASE_BLOB = "e5506a55b998d82a7dc7b02159605bc1188cd388"

NO_CC = "/nonexistent/compile-jobs-test/cc"   # never executed; the fake runs instead


def load_base_module(tmpdir: str):
    """The base gen_decomp_ports module, or None when its blob is absent."""
    try:
        data = subprocess.run(["git", "-C", str(REPO), "cat-file", "blob", BASE_BLOB],
                              capture_output=True, check=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return None
    got = hashlib.sha1(b"blob %d\0" % len(data) + data).hexdigest()
    if got != BASE_BLOB:
        raise AssertionError(f"base blob content hashes to {got}, expected {BASE_BLOB}")
    path = Path(tmpdir) / "gen_decomp_ports_base.py"
    path.write_bytes(data)
    spec = importlib.util.spec_from_file_location("gen_decomp_ports_base", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class FakeCompiler:
    """Stands in for subprocess.run inside compile_check(); records every call.

    `fail` maps a leaf name to the error text its fake compile reports.  With a
    barrier, every call waits there (bounded by the barrier's timeout) so the
    test can observe how many checks are in flight at the same time.
    """

    def __init__(self, fail=None, barrier=None, wait=1.0):
        self.fail = dict(fail or {})
        self.barrier = barrier
        self.wait = wait
        self.lock = threading.Lock()
        self.calls = []
        self.active = 0
        self.peak = 0
        self.passed = 0
        self.broken = 0

    def __call__(self, cmd, **kwargs):
        name = Path(cmd[-1]).name[:-len("_port.c")]
        with self.lock:
            self.calls.append((cmd[0], name, kwargs))
            self.active += 1
            self.peak = max(self.peak, self.active)
        try:
            if self.barrier is not None:
                try:
                    self.barrier.wait(self.wait)
                    with self.lock:
                        self.passed += 1
                except threading.BrokenBarrierError:
                    with self.lock:
                        self.broken += 1
        finally:
            with self.lock:
                self.active -= 1
        if name in self.fail:
            return types.SimpleNamespace(returncode=1, stdout="",
                                         stderr=f"{name}_port.c:1:1: error: {self.fail[name]}\n")
        return types.SimpleNamespace(returncode=0, stdout="", stderr="")


def run_check(module, env, items, cpu=12, fake=None, seen=None):
    """compile_check() under a fixed JOBS environment and CPU count.

    Returns (failures, max_workers values the executor was built with, fake).
    `seen` (when given) collects the max_workers values even if the call raises.
    """
    fake = fake or FakeCompiler()
    seen = [] if seen is None else seen
    real_executor = concurrent.futures.ThreadPoolExecutor

    class RecordingExecutor(real_executor):
        def __init__(self, max_workers=None, *args, **kwargs):
            seen.append(max_workers)
            super().__init__(max_workers, *args, **kwargs)

    with mock.patch.dict(os.environ, env, clear=False), \
            mock.patch.object(subprocess, "run", fake), \
            mock.patch.object(concurrent.futures, "ThreadPoolExecutor", RecordingExecutor), \
            mock.patch.object(os, "cpu_count", return_value=cpu):
        if "JOBS" not in env:
            os.environ.pop("JOBS", None)
        failures = module.compile_check(NO_CC, items)
    return failures, seen, fake


def leaves(n):
    return [(f"func_8009{i:04X}", f"int func_8009{i:04X}(void) {{ return {i}; }}\n")
            for i in range(n)]


class CompileJobsParsingTests(unittest.TestCase):
    def test_unset_uses_explicit_default_whatever_the_cpu_count(self):
        for cpu in (1, 2, 12, 64, None):
            with self.subTest(cpu=cpu), mock.patch.object(os, "cpu_count", return_value=cpu):
                self.assertEqual(g.compile_jobs({}), g.COMPILE_JOBS_DEFAULT)
        self.assertEqual(g.COMPILE_JOBS_DEFAULT, 4)
        self.assertEqual(g.COMPILE_JOBS_MAX, 16)

    def test_valid_values(self):
        cases = {"1": 1, "2": 2, "4": 4, "8": 8, "15": 15, "16": 16,
                 " 4 ": 4, "4\n": 4, "\t3": 3, "04": 4, "0004": 4}
        for raw, want in cases.items():
            with self.subTest(raw=raw):
                self.assertEqual(g.compile_jobs({"JOBS": raw}), want)

    def test_above_ceiling_runs_at_ceiling_never_more(self):
        for raw in ("17", "32", "64", "99", "100", "1000000", "9" * 20,
                    "9" * 5000, "0" * 10 + "17"):
            with self.subTest(raw=raw[:24]):
                self.assertEqual(g.compile_jobs({"JOBS": raw}), g.COMPILE_JOBS_MAX)

    def test_unusable_values_fail_closed(self):
        for raw in ("", " ", "\t", "0", "00", "-1", "-4", "+4", "4.0", "4.5", "4x",
                    "x4", "abc", "1e3", "0x10", "4 4", "four", "None",
                    "٤", "４", "²"):
            with self.subTest(raw=raw):
                with self.assertRaises(SystemExit) as cm:
                    g.compile_jobs({"JOBS": raw})
                self.assertIn(f"JOBS={raw!r}", str(cm.exception.code))
                self.assertIn("not a usable worker limit", str(cm.exception.code))

    def test_reads_os_environ_by_default(self):
        with mock.patch.dict(os.environ, {"JOBS": "3"}, clear=False):
            self.assertEqual(g.compile_jobs(), 3)
        with mock.patch.dict(os.environ, {}, clear=False):
            os.environ.pop("JOBS", None)
            self.assertEqual(g.compile_jobs(), g.COMPILE_JOBS_DEFAULT)

    def test_never_consults_cpu_count(self):
        boom = mock.Mock(side_effect=AssertionError("os.cpu_count() consulted"))
        with mock.patch.object(os, "cpu_count", boom):
            for env in ({}, {"JOBS": "1"}, {"JOBS": "4"}, {"JOBS": "500"}):
                g.compile_jobs(env)
            with self.assertRaises(SystemExit):
                g.compile_jobs({"JOBS": "0"})
        boom.assert_not_called()


class CorrectedCompileCheckTests(unittest.TestCase):
    def test_executor_gets_jobs_not_cpu_count(self):
        cases = [({"JOBS": "4"}, 12, 4), ({"JOBS": "1"}, 12, 1), ({"JOBS": "2"}, 64, 2),
                 ({"JOBS": "16"}, 12, 16), ({"JOBS": "17"}, 64, 16), ({}, 64, 4),
                 ({}, 1, 4), ({"JOBS": "4"}, None, 4)]
        for env, cpu, want in cases:
            with self.subTest(env=env, cpu=cpu):
                failures, seen, fake = run_check(g, env, leaves(6), cpu=cpu)
                self.assertEqual(seen, [want])
                self.assertEqual(failures, {})
                self.assertEqual(len(fake.calls), 6)
                self.assertTrue(all(c[0] == NO_CC for c in fake.calls))

    def test_unusable_jobs_stops_before_any_compiler_or_executor(self):
        for raw in ("0", "", "-2", "4x"):
            with self.subTest(raw=raw):
                fake = FakeCompiler()
                seen = []
                with self.assertRaises(SystemExit):
                    run_check(g, {"JOBS": raw}, leaves(3), fake=fake, seen=seen)
                self.assertEqual(fake.calls, [])
                self.assertEqual(seen, [])
        # Control: the same harness does record a valid run, so the empty
        # lists above are evidence, not a harness that never records.
        fake, seen = FakeCompiler(), []
        run_check(g, {"JOBS": "3"}, leaves(3), fake=fake, seen=seen)
        self.assertEqual(seen, [3])
        self.assertEqual(len(fake.calls), 3)

    def test_peak_concurrency_never_exceeds_jobs(self):
        # A barrier with JOBS+1 parties can only open if JOBS+1 checks are in
        # flight together.  With the corrected pool it must time out instead
        # (passed == 0 is the deterministic proof; the peak is bounded by JOBS).
        for jobs in (2, 4):
            with self.subTest(jobs=jobs):
                fake = FakeCompiler(barrier=threading.Barrier(jobs + 1), wait=1.0)
                failures, seen, fake = run_check(g, {"JOBS": str(jobs)}, leaves(3 * jobs),
                                                 cpu=12, fake=fake)
                self.assertEqual(seen, [jobs])
                self.assertEqual(fake.passed, 0)
                self.assertGreaterEqual(fake.broken, 1)
                self.assertLessEqual(fake.peak, jobs)
                self.assertEqual(len(fake.calls), 3 * jobs)
                self.assertEqual(failures, {})

    def test_failures_are_reported_per_leaf(self):
        items = leaves(5)
        fail = {items[1][0]: "unknown type name 'UD'", items[3][0]: "boom"}
        failures, seen, _ = run_check(g, {"JOBS": "2"}, items,
                                      fake=FakeCompiler(fail=fail))
        self.assertEqual(seen, [2])
        self.assertEqual(failures, fail)


class BaseImplementationTests(unittest.TestCase):
    """The pre-change compile_check, loaded from the base commit's blob."""

    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.base = load_base_module(cls.tmp.name)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def setUp(self):
        if self.base is None:
            self.skipTest(f"base blob {BASE_BLOB[:12]} is not in this repository")

    def test_base_has_no_jobs_parser(self):
        self.assertFalse(hasattr(self.base, "compile_jobs"))

    def test_base_ignores_jobs_and_sizes_pool_from_cpu_count(self):
        cases = [({"JOBS": "4"}, 12, 12), ({"JOBS": "1"}, 12, 12), ({"JOBS": "4"}, 32, 16),
                 ({"JOBS": "1"}, None, 4), ({}, 12, 12)]
        for env, cpu, want in cases:
            with self.subTest(env=env, cpu=cpu):
                _, seen, _ = run_check(self.base, env, leaves(4), cpu=cpu)
                self.assertEqual(seen, [want])
        _, seen, _ = run_check(self.base, {"JOBS": "4"}, leaves(4), cpu=12)
        self.assertGreater(seen[0], 4, "base pool must exceed the requested JOBS=4")

    def test_base_runs_more_than_jobs_at_once(self):
        # JOBS=4 requested; on a 12-CPU host the base pool lets 5 checks meet at
        # the barrier (twice, 10 leaves), i.e. it exceeds the requested limit.
        # The barrier opens as soon as 5 arrive; the timeout only bounds a hang.
        fake = FakeCompiler(barrier=threading.Barrier(5), wait=10.0)
        _, seen, fake = run_check(self.base, {"JOBS": "4"}, leaves(10), cpu=12, fake=fake)
        self.assertEqual(seen, [12])
        self.assertEqual(fake.passed, 10)
        self.assertEqual(fake.broken, 0)
        self.assertGreaterEqual(fake.peak, 5)

    def test_base_and_corrected_report_the_same_failures(self):
        items = leaves(7)
        fail = {items[0][0]: "redefinition of 'func_80073A44'", items[6][0]: "x"}
        old, _, _ = run_check(self.base, {"JOBS": "4"}, items, fake=FakeCompiler(fail=fail))
        new, _, _ = run_check(g, {"JOBS": "4"}, items, fake=FakeCompiler(fail=fail))
        self.assertEqual(old, fail)
        self.assertEqual(new, old)


def _calls(tree, names):
    out = []
    for node in ast.walk(tree):
        if isinstance(node, ast.Call):
            fn = node.func
            last = fn.attr if isinstance(fn, ast.Attribute) else getattr(fn, "id", None)
            if last in names:
                out.append(node)
    return out


class StructuralTests(unittest.TestCase):
    POOLS = {"ThreadPoolExecutor", "ProcessPoolExecutor", "Pool"}

    def test_single_pool_sized_by_compile_jobs(self):
        tree = ast.parse((HERE / "gen_decomp_ports.py").read_text(encoding="utf-8"))
        pools = _calls(tree, self.POOLS)
        self.assertEqual(len(pools), 1)
        check = next(n for n in tree.body
                     if isinstance(n, ast.FunctionDef) and n.name == "compile_check")
        self.assertIn(pools[0], list(ast.walk(check)))
        kw = {k.arg: k.value for k in pools[0].keywords}
        self.assertIsInstance(kw.get("max_workers"), ast.Name)
        self.assertEqual(kw["max_workers"].id, "workers")
        first = check.body[1]   # body[0] is the docstring
        self.assertIsInstance(first, ast.Assign)
        self.assertEqual([t.id for t in first.targets], ["workers"])
        self.assertIsInstance(first.value, ast.Call)
        self.assertEqual(first.value.func.id, "compile_jobs")
        self.assertEqual(first.value.args, [])
        self.assertFalse(any(isinstance(n, ast.Attribute) and n.attr == "cpu_count"
                             for n in ast.walk(tree)))

    def test_overlay_generator_has_no_pool_of_its_own(self):
        for name in ("gen_overlay_ports.py", "guest_lower.py"):
            with self.subTest(file=name):
                tree = ast.parse((HERE / name).read_text(encoding="utf-8"))
                self.assertEqual(_calls(tree, self.POOLS), [])
                self.assertFalse(any(isinstance(n, ast.Attribute) and n.attr == "cpu_count"
                                     for n in ast.walk(tree)))
                imported = set()
                for n in ast.walk(tree):
                    if isinstance(n, ast.Import):
                        imported |= {a.name.split(".")[0] for a in n.names}
                    elif isinstance(n, ast.ImportFrom) and n.module:
                        imported.add(n.module.split(".")[0])
                self.assertFalse(imported & {"multiprocessing", "concurrent"}, imported)
        tree = ast.parse((HERE / "gen_overlay_ports.py").read_text(encoding="utf-8"))
        via_g = [c for c in _calls(tree, {"compile_check"})
                 if isinstance(c.func, ast.Attribute) and isinstance(c.func.value, ast.Name)
                 and c.func.value.id == "g"]
        self.assertGreaterEqual(len(via_g), 1)


if __name__ == "__main__":
    unittest.main()
