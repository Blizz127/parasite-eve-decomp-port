#!/usr/bin/env python3
"""Prepared retail/native font initializer differential; admission required.

Uses the retained, test-only MIPS oracle without modifying its instruction
semantics. Scripted CD/page/RNG callees are shared external inputs. No compiler,
game, disc extraction, device, install or publication is invoked by this file.
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

ENTRY, SIZE = 0x80038D74, 0x410
RAM_SIZE, BUFFER, PAYLOAD = 0x200400, 0x80110000, 0x80118000
EXE_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
MASK = 0xFFFFFFFF
# Normal fixtures have at least nine pages: limit-7 is always positive.
# The two expanded DIV guards therefore cannot execute these eight words.
TRAP_GUARDS = {0x80038F38, 0x80038F48, 0x80038F4C, 0x80038F50,
               0x80038FDC, 0x80038FEC, 0x80038FF0, 0x80038FF4}


def word(ram, address, value):
    struct.pack_into("<I", ram, address & 0x1FFFFF, value & MASK)


def read_word(ram, address):
    return struct.unpack_from("<I", ram, address & 0x1FFFFF)[0]


def setup(exe, count, saved_seed, flag, layout):
    ram = bytearray(bytes(range(256)) * (RAM_SIZE // 256))
    ram[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
    word(ram, 0x800B0DD8, 0x12345678); word(ram, 0x800B0E6C, BUFFER)
    struct.pack_into("<HH", ram, 0x93176, 13, 17)
    ram[BUFFER & 0x1FFFFF] = count
    ram[0x91A1C:0x91A21] = bytes((0, 17, 19, 23, flag))
    word(ram, 0x80091A24, saved_seed); word(ram, 0x80091A28, 0x80119000)
    p = PAYLOAD & 0x1FFFFF
    ram[p:p + 329] = bytes(329)
    if layout == 0:
        ram[p + 3] = 3; ram[p + 4:p + 7] = bytes((5, 2, 24))
        ram[p + 28] = 4; ram[p + 29:p + 33] = bytes((0, 2, 1, 1))
    elif layout == 1:
        ram[p + 3] = 2; ram[p + 4:p + 6] = bytes((5, 24))
        ram[p + 28] = 4; ram[p + 29:p + 33] = bytes((1, 0, 1, 0))
    else:
        ram[p + 3] = 2; ram[p + 4:p + 6] = bytes((5, 2))
        ram[p + 28] = 4; ram[p + 29:p + 33] = bytes((0, 0, 0, 0))
        ram[p + 284] = 3; ram[p + 7] = 0x5A
    return ram


class Policy:
    def __init__(self, ram, config):
        self.ram = ram
        self.state, self.issue_failures, retry, force = config
        self.polls = iter((1, -1, 2, 0) if retry else (2, 1, 0))
        self.prefix = iter((0, 32767, 12345) if force else ())
        self.seeded = False

    def call(self, target, _seed):
        if target == 0x8006E6A8:
            if self.issue_failures:
                self.issue_failures -= 1
                return MASK
            return 0
        if target == 0x8006E7E8:
            return next(self.polls) & MASK
        if target == 0x80071A54:
            if not self.seeded:
                value = next(self.prefix, None)
                if value is not None:
                    return value
            self.state = (self.state * 0x41C64E6D + 0x3039) & MASK
            return (self.state >> 16) & 0x7FFF
        if target == 0x80071A64:
            # Retail passes this stored seed. The separately compared call
            # arguments ensure the native side actually supplies that value.
            self.state = read_word(self.ram, 0x80091A24)
            self.seeded = True
            return 0
        if target == 0x800389DC:
            self.ram[0x9ECD9:0x9EE21] = self.ram[0x118001:0x118149]
            return 0
        raise RuntimeError(f"unexpected retail call {target:08X}")


def read_exact(stream, count):
    result = bytearray()
    while len(result) < count:
        data = stream.read(count - len(result))
        if not data:
            raise RuntimeError("native transport truncated")
        result.extend(data)
    return result


def spill_bytes(exe):
    # Decode only the straight-line retail prologue's SW of incoming saved
    # registers to SP. These exact bytes, not a whole stack range, may differ.
    result, stack = set(), 0x801FFF00
    for pc in range(ENTRY, ENTRY + SIZE, 4):
        insn = struct.unpack_from("<I", exe, pc - 0x80010000 + 0x800)[0]
        op, rs, rt = insn >> 26, (insn >> 21) & 31, (insn >> 16) & 31
        imm = insn & 0xFFFF; imm -= 0x10000 if imm & 0x8000 else 0
        if op == 9 and rs == 29 and rt == 29:
            stack = (stack + imm) & MASK
        if op == 0x2B and rs == 29 and rt in (*range(16, 24), 30, 31):
            result.update(range((stack + imm) & 0x1FFFFF, ((stack + imm) & 0x1FFFFF) + 4))
        if op in (1, 2, 3, 4, 5, 6, 7) or (op == 0 and (insn & 63) in (8, 9)):
            break
    if len(result) != 36:
        raise RuntimeError("unexpected retail prologue spill layout")
    return result


def comparable(ram, spills):
    result = bytearray(ram)
    for index in spills:
        result[index] = 0
    return result


def agrees(actual, expected, spills):
    return (actual[:2] == expected[:2]
            and comparable(actual[2], spills) == comparable(expected[2], spills))


def inputs(exe):
    for count, seed, layout in itertools.product((9, 10, 20, 105, 110, 114, 200, 255), (0, 1, 37), (0, 1)):
        flag = int(seed == 0 and layout == 1)
        config = (0x13579BDF, int(layout == 1), int(layout == 1), int(seed == 0 and flag == 0))
        yield f"normal-{count}-{seed}-{layout}", config, setup(exe, count, seed, flag, layout)
    for count, seed in itertools.product((9, 105, 255), (1, 0xDEADBEEF)):
        yield f"no-slot-{count}-{seed}", (1, 2, 1, 0), setup(exe, count, seed, 0, 2)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", type=Path, required=True)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--native", type=Path, required=True)
    parser.add_argument("--body-sha256", required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    exe = args.exe.read_bytes()
    body = exe[ENTRY - 0x80010000 + 0x800:ENTRY - 0x80010000 + 0x800 + SIZE]
    if hashlib.sha1(exe).hexdigest() != EXE_SHA1 or hashlib.sha256(body).hexdigest() != args.body_sha256:
        raise RuntimeError("retail input identity mismatch")
    sys.path.insert(0, str(args.base.resolve() / "tools/analysis"))
    spec = importlib.util.spec_from_file_location("font_retail_oracle", args.base / "tools/analysis/e21_diff.py")
    oracle = importlib.util.module_from_spec(spec); spec.loader.exec_module(oracle)
    spills = spill_bytes(exe)
    results, coverage = [], set()
    arities = {0x8006E6A8: 3, 0x8006E7E8: 0, 0x80071A54: 0, 0x80071A64: 1, 0x800389DC: 1}
    with subprocess.Popen([str(args.native.resolve())], stdin=subprocess.PIPE, stdout=subprocess.PIPE) as native:
        try:
            for name, config, ram in inputs(exe):
                expected_ram = bytearray(ram)
                policy = Policy(expected_ram, config)
                oracle.mock_ret = policy.call; oracle.COVER = set()
                ret, calls = oracle.run_mips(expected_ram, ENTRY, [0, 0, 0, 0], 0)
                expected_calls = [(target, tuple(values[:arities[target]])) for target, values in calls]
                coverage.update(oracle.COVER)
                expected = ret, expected_calls, expected_ram
                native.stdin.write(b"FNT1" + struct.pack("<4I", *config) + ram); native.stdin.flush()
                magic, actual_ret, count, stop = struct.unpack("<4sIII", read_exact(native.stdout, 16))
                if magic != b"FRES" or count > 8192 or stop:
                    raise RuntimeError(f"invalid native reply/stop in {name}: {stop}")
                actual_calls = []
                for _ in range(count):
                    target, arity, a0, a1, a2 = struct.unpack("<5I", read_exact(native.stdout, 20))
                    if arities.get(target) != arity:
                        raise RuntimeError("invalid native call")
                    actual_calls.append((target, (a0, a1, a2)[:arity]))
                actual = actual_ret, actual_calls, read_exact(native.stdout, RAM_SIZE)
                if not agrees(actual, expected, spills):
                    raise RuntimeError(f"retail/native return, ordered-call or RAM mismatch: {name}")
                if agrees((ret ^ 1, expected_calls, expected_ram), expected, spills):
                    raise RuntimeError("return negative control failed")
                if agrees((ret, expected_calls + [(0xBAD, ())], expected_ram), expected, spills):
                    raise RuntimeError("call negative control failed")
                for index in (0x9EE24, 0x118000, 0x200000):
                    changed = bytearray(expected_ram); changed[index] ^= 1
                    if agrees((ret, expected_calls, changed), expected, spills):
                        raise RuntimeError("RAM/scratchpad negative control failed")
                results.append({"case": name, "calls": count, "return": ret,
                                "input_sha256": hashlib.sha256(ram).hexdigest(),
                                "output_sha256": hashlib.sha256(comparable(actual[2], spills)).hexdigest()})
            native.stdin.close()
            if native.wait(timeout=5):
                raise RuntimeError("native harness failed at closure")
        finally:
            if native.poll() is None:
                native.kill(); native.wait()
    if len(results) != 54:
        raise RuntimeError(f"directed case count {len(results)} != 54")
    required = set(range(ENTRY, ENTRY + SIZE, 4)) - TRAP_GUARDS
    missing = required - coverage
    if missing:
        raise RuntimeError("uncovered normal retail words: " + ",".join(f"{pc:08X}" for pc in sorted(missing)))
    args.output.write_text(json.dumps({"status": "PASS", "scope": "font initializer body only; all five callees mocked",
        "cases": len(results), "skipped": 0, "exe_sha1": EXE_SHA1, "body_sha256": args.body_sha256,
        "compared": "return, ordered typed calls, all RAM except exact incoming-register spills, full scratchpad",
        "ignored_spill_bytes": sorted(spills), "coverage": [f"{pc:08X}" for pc in sorted(coverage)],
        "excluded_trap_guard_words": [f"{pc:08X}" for pc in sorted(TRAP_GUARDS)],
        "coverage_limit": "all252 normal words required; malformed-data traps, real IO and room reachability untested",
        "negative_controls": "return, call list, page table, unrelated RAM, scratchpad in every case",
        "results": results}, indent=2) + "\n")
    print(f"native font initializer: {len(results)} directed cases PASS, zero skipped")


if __name__ == "__main__":
    main()
