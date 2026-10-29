#!/usr/bin/env python3
"""Prepared directed retail/native differential; execute only after admission.

The Python MIPS model is a test-only oracle from the retained park candidate.
This script never compiles, extracts, starts a game, installs, or submits jobs.
It requires the separately compiled native harness and existing extracted EXE.
Three math callees are mocked identically to isolate handler behavior; this is
not composed native-callee, room-reachability, or campaign coverage evidence.
"""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import itertools
import json
from pathlib import Path
import struct
import subprocess
import sys

RAM = 0x200400
ACTOR, OTHER, TASK, ARGS = 0x80110000, 0x80112000, 0x80114000, 0x80116000
DISTANCE, OUT = 0x80117000, 0x80118000
P_ACTOR, P_TASK, PC = 0x8009D2F0, 0x8009D300, 0x8009CE00
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
SPAN_HASHES = {
    0x800133E8: (0x12C, "3a411ba65990f04a64ece53573d0de85ac85d7b553c269a9ebbe6a7bb7e3c57c"),
    0x80019810: (0xB4, "731b50772b9573b3c0cdf5a810a21e69aba6b61a7f86c3ca0a6232f065a2670a"),
}
# The divisor is byte+1 (1..256). These eight words are unreachable trap
# checks/breaks for divisor 0/-1, not omitted gameplay branches.
UNREACHABLE = {0x80013470, 0x80013480, 0x80013484, 0x80013488,
               0x800134B4, 0x800134C4, 0x800134C8, 0x800134CC}


def w(ram, address, value):
    struct.pack_into("<I", ram, address & 0x1FFFFF, value & 0xFFFFFFFF)


def h(ram, address, value):
    struct.pack_into("<H", ram, address & 0x1FFFFF, value & 0xFFFF)


def initial(exe):
    ram = bytearray(bytes(range(256)) * (RAM // 256))
    ram[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
    w(ram, P_ACTOR, ACTOR); w(ram, P_TASK, TASK); w(ram, PC, 0x80119008)
    return ram


def cases(template, oracle):
    seed = 0
    # Equal, both crossing directions, both modular-wrap arms, frame/count
    # boundaries and 32-bit addition overflow; 5*7*15 = 525 cases.
    for count, frame, (value, step) in itertools.product(
        (0, 1, 2, 5, 255), (0, 1, 2, 5, 0x7FFF, 0x8000, 0xFFFF),
        ((0, 0), (0, 0x10000), (0x10000, 0), (0, 0x20000),
         (0x30000, -0x30000), (0x10000, 0x20000), (0x10000, -0x30000),
         (0x70000, 0x50000), (-0x10000, 0), (-0x20000, -0x30000),
         (0x7FFFFFFF, 1), (-0x80000000, -1),
         (0x10001, 0xFFFF), (-1, -1), (0x7FFF0000, 0x20000))):
        ram = bytearray(template)
        ram[(ACTOR + 0xF) & 0x1FFFFF] = count
        h(ram, ACTOR + 0x12, frame)
        w(ram, ACTOR + 0x14, value); w(ram, ACTOR + 0x1C, step)
        yield f"wait-{count}-{frame}-{value}-{step}", 0x800133E8, seed, 0, ram
        seed += 1
    # Ordinary motion includes extreme distances/angles and multiple mock signs.
    for distance, angle, mock_seed in itertools.product(
        (0, 1, -1, 0x7FFFFFFF, -0x80000000),
        (-32768, -4096, -1, 0, 1, 4095, 32767), (0, 1, 3, 5)):
        ram = bytearray(template)
        w(ram, ARGS, DISTANCE); w(ram, ARGS + 4, OUT); w(ram, ARGS + 8, OUT + 4)
        w(ram, DISTANCE, distance); h(ram, ACTOR + 0x3A, angle)
        w(ram, ACTOR + 0x28, 0x7FFFFFF0); w(ram, ACTOR + 0x30, 0x80000010)
        yield f"position-{distance}-{angle}-{mock_seed}", 0x80019810, mock_seed, ARGS, ram
    # The first output changes state needed by the second half of the handler.
    for destination in (P_ACTOR, ACTOR + 0x38, ARGS + 8, ACTOR + 0x30,
                        ARGS, DISTANCE, OUT + 4):
        for mock_seed in (0, 1, 3, 5):
            ram = bytearray(template)
            w(ram, ARGS, DISTANCE); w(ram, ARGS + 4, destination); w(ram, ARGS + 8, OUT + 4)
            w(ram, DISTANCE, 0x80000000); h(ram, ACTOR + 0x3A, -1)
            h(ram, OTHER + 0x3A, 4095); w(ram, OTHER + 0x30, 0x12345678)
            result = oracle.mock_ret(0x8003708C, mock_seed)
            # Keep pointer-alias scenarios valid after the first store.
            desired = OTHER if destination == P_ACTOR else OUT if destination == ARGS + 8 else 0x07654321
            w(ram, ACTOR + 0x28, desired - result); w(ram, ACTOR + 0x30, 0x87654321)
            yield f"alias-{destination:08X}-{mock_seed}", 0x80019810, mock_seed, ARGS, ram


def comparable(ram, entry):
    result = bytearray(ram)
    if entry == 0x80019810:
        # Exactly three native-independent retail register spills, not a whole
        # stack arena. All other RAM and the entire scratchpad are compared.
        result[0x1FFEF0:0x1FFEFC] = bytes(12)
    return result


def agrees(actual, expected, entry):
    return (actual[0] == expected[0] and actual[1] == expected[1]
            and comparable(actual[2], entry) == comparable(expected[2], entry))


def read_exact(stream, count):
    result = bytearray()
    while len(result) < count:
        chunk = stream.read(count - len(result))
        if not chunk:
            raise RuntimeError(f"native transport ended at {len(result)}/{count} bytes")
        result.extend(chunk)
    return bytes(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", type=Path, required=True, help="retained park candidate")
    parser.add_argument("--exe", type=Path, required=True, help="existing extracted retail EXE")
    parser.add_argument("--native", type=Path, required=True, help="separately admitted native test executable")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    exe = args.exe.read_bytes()
    if hashlib.sha1(exe).hexdigest() != EXE_SHA1:
        raise RuntimeError("retail EXE identity mismatch")
    for entry, (size, digest) in SPAN_HASHES.items():
        offset = entry - 0x80010000 + 0x800
        if hashlib.sha256(exe[offset:offset + size]).hexdigest() != digest:
            raise RuntimeError(f"retail body identity mismatch at {entry:08X}")
    analysis = args.base.resolve() / "tools/analysis"
    sys.path.insert(0, str(analysis))
    spec = importlib.util.spec_from_file_location("field_vm_oracle", analysis / "e21_diff.py")
    oracle = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(oracle)
    template = initial(exe)
    results, coverage = [], {entry: set() for entry in SPAN_HASHES}
    with subprocess.Popen([str(args.native.resolve())], stdin=subprocess.PIPE,
                          stdout=subprocess.PIPE) as native:
        try:
            for name, entry, seed, arg, ram in cases(template, oracle):
                expected_ram = bytearray(ram)
                oracle.COVER = set()
                ret, calls = oracle.run_mips(expected_ram, entry, [arg, 0, 0, 0], seed)
                expected_calls = [(target, tuple(values[:2 if target == 0x8003708C else 1]))
                                  for target, values in calls]
                expected = ret, expected_calls, expected_ram
                coverage[entry].update(oracle.COVER)
                native.stdin.write(b"PEVM" + struct.pack("<III", entry, seed, arg) + ram)
                native.stdin.flush()
                magic, actual_ret, count = struct.unpack("<4sII", read_exact(native.stdout, 12))
                if magic != b"VRES" or count > 4:
                    raise RuntimeError(f"invalid native transport in {name}")
                actual_calls = []
                for _ in range(count):
                    target, arity, a0, a1 = struct.unpack("<4I", read_exact(native.stdout, 16))
                    if arity not in (1, 2):
                        raise RuntimeError("invalid native call arity")
                    actual_calls.append((target, (a0, a1)[:arity]))
                actual = actual_ret, actual_calls, read_exact(native.stdout, RAM)
                if not agrees(actual, expected, entry):
                    raise RuntimeError(f"retail/native mismatch in {name}: return, calls, or RAM")
                # Comparer negative controls are applied to independently
                # obtained retail outputs, not to an implementation mirror.
                if agrees((ret ^ 1, expected_calls, expected_ram), expected, entry):
                    raise RuntimeError("return comparer negative control failed")
                changed = bytearray(expected_ram); changed[0x118000] ^= 1
                if agrees((ret, expected_calls, changed), expected, entry):
                    raise RuntimeError("RAM comparer negative control failed")
                changed = bytearray(expected_ram); changed[0x200000] ^= 1
                if agrees((ret, expected_calls, changed), expected, entry):
                    raise RuntimeError("scratchpad comparer negative control failed")
                if agrees((ret, expected_calls + [(0xDEADBEEF, ())], expected_ram), expected, entry):
                    raise RuntimeError("ordered-call comparer negative control failed")
                results.append({"case": name, "entry": f"{entry:08X}", "seed": seed,
                                "input_sha256": hashlib.sha256(ram).hexdigest(),
                                "output_sha256": hashlib.sha256(comparable(actual[2], entry)).hexdigest(),
                                "calls": actual_calls})
            native.stdin.close()
            if native.wait(timeout=5) != 0:
                raise RuntimeError("native harness failed at closure")
        finally:
            if native.poll() is None:
                native.kill(); native.wait()
    for entry, (size, _) in SPAN_HASHES.items():
        required = set(range(entry, entry + size, 4)) - UNREACHABLE
        missing = required - coverage[entry]
        if missing:
            raise RuntimeError("uncovered reachable retail words: " + ",".join(f"{pc:08X}" for pc in sorted(missing)))
    payload = {"status": "PASS", "scope": "two handler bodies; three math callees mocked; no game",
               "cases": len(results), "skipped": 0, "exe_sha1": EXE_SHA1,
               "coverage": {f"{entry:08X}": [f"{pc:08X}" for pc in sorted(pcs)]
                            for entry, pcs in coverage.items()},
               "excluded_trap_words": [f"{pc:08X}" for pc in sorted(UNREACHABLE)],
               "comparisons": "return, ordered typed calls, all RAM except 12 exact register-spill bytes; full scratchpad",
               "negative_controls": "return, ordered call list, non-stack RAM, scratchpad; every case",
               "results": results}
    args.output.write_text(json.dumps(payload, indent=2) + "\n")
    print(f"native field VM: {len(results)} cases PASS, zero skipped; all reachable words covered")


if __name__ == "__main__":
    main()
