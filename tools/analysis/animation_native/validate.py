#!/usr/bin/env python3
"""Prepared composed animation differential; execute only under exact admission.

Both sides execute the real rotation callee. The CPU model is test-only and
unchanged; production uses compiled native C. No game, disc extraction, device,
compiler invocation, install or publication is performed by this script.
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

RAM_SIZE, SPAD_SIZE = 0x200000, 0x400
DEST, CLIP, MATRICES, STACK = 0x80120000, 0x80130000, 0x80140000, 0x801FF000
PARENT, NARROW, WIDE, ROTATE = 0x80039B74, 0x80039D24, 0x80039ED4, 0x80079754
SPANS = {PARENT: 0x1B0, NARROW: 0x1B0, WIDE: 0x1B4, ROTATE: 0x290}
MASK = 0xFFFFFFFF


def word(ram, address, value):
    struct.pack_into("<I", ram, address & 0x1FFFFF, value & MASK)


def half(ram, address, value):
    struct.pack_into("<H", ram, address & 0x1FFFFF, value & 0xFFFF)


def fixture(exe, wide, count, frame, flags, seed):
    ram = bytearray(bytes(range(256)) * (RAM_SIZE // 256))
    ram[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
    ram[DEST & 0x1FFFFF:(DEST & 0x1FFFFF) + 0xC0] = bytes(0xC0)
    ram[CLIP & 0x1FFFFF:(CLIP & 0x1FFFFF) + 0x2000] = bytes(0x2000)
    word(ram, DEST + 0x84, MATRICES)
    for offset, value in ((0, 2 if wide else 0), (1, count), (2, 4)):
        ram[(CLIP + offset) & 0x1FFFFF] = value
    for axis, value in enumerate((7, 8, 9, 10)):
        half(ram, CLIP + 4 + axis * 2, value)
    cursor = CLIP + 12
    for axis in range(3):
        if axis == 1:
            half(ram, cursor, 1); half(ram, cursor + 2, -7); cursor += 4
        else:
            for sample, value in enumerate((-32768, 20, -40, 1024, 32767)):
                half(ram, cursor + 2 + sample * 2, value + axis)
            cursor += 12
    for row in range(count + 1):
        for axis in range(3):
            constant = (row + axis + flags) & 1
            if wide:
                half(ram, cursor, constant)
                for sample in range(1 if constant else 5):
                    half(ram, cursor + 2 + sample * 2, 0x8001 + row * 97 + axis * 101 + sample * 257)
            else:
                ram[cursor & 0x1FFFFF] = constant
                for sample in range(1 if constant else 5):
                    ram[(cursor + 1 + sample) & 0x1FFFFF] = (row * 19 + axis * 31 + sample * 47) & 255
            cursor += 4 if constant else 12 if wide else 8
    # Both override slots refer to existing scratchpad rows beyond the decoded
    # clip (except the explicit count127 edge). The bytes are intentional input.
    index = count + 2 if count < 125 else 127
    for slot in range(2):
        entry = DEST + 0xA0 + slot * 8
        for axis, value in enumerate((32760, -32760, 0x1234)):
            half(ram, entry + axis * 2, value + slot)
        ram[(entry + 6) & 0x1FFFFF] = index + (slot if count < 125 and not flags & 1 else 0)
        ram[(entry + 7) & 0x1FFFFF] = flags
    scratch = bytearray((index * 37 + seed * 11) & 255 for index in range(SPAD_SIZE))
    return ram, scratch


def inputs(exe):
    seed = 0
    for wide, frame, count, flags in itertools.product((0, 1), (-1, 1, 4), (0, 2), (0, 1, 2, 4, 8, 16, 32, 63)):
        yield f"parent-{wide}-{frame}-{count}-{flags}", PARENT, CLIP, frame, fixture(exe, wide, count, frame, flags, seed)
        seed += 1
    for wide, frame, count in itertools.product((0, 1), (-32768, -1, 4, 32767), (0, 2)):
        entry = WIDE if wide else NARROW
        yield f"direct-{wide}-{frame}-{count}", entry, CLIP, frame, fixture(exe, wide, count, frame, 0, seed)
        seed += 1
    for wide in (0, 1):
        yield f"scratch-edge-{wide}", PARENT, CLIP, 1, fixture(exe, wide, 127, 1, 63, seed)
        seed += 1
    for wide in (0, 1):
        ram, scratch = fixture(exe, wide, 2, 0, 0, seed)
        word(ram, DEST + 0x84, CLIP)
        # Three translation records occupy 12+4+12 bytes. Force the root
        # rotation to identity, whose matrix writes the joint-count byte to16.
        # Later matrices deliberately overlap future tracks: both executions
        # must observe the same input/output alias and reload the count.
        cursor = CLIP + 40
        for _ in range(3):
            half(ram, cursor, 1)
            if wide:
                half(ram, cursor + 2, 0)
            else:
                ram[(cursor + 1) & 0x1FFFFF] = 0
            cursor += 4
        yield f"header-output-alias-{wide}", PARENT, CLIP, 0, (ram, scratch)
        seed += 1
    yield "null-clip", PARENT, 0, 0, fixture(exe, 0, 0, 0, 0, seed)


def stack_spills(entry, clip):
    # Pinned retail prologues: parent alloc32, saves s0/s1/s2/ra at16..28;
    # decoders alloc56, save s0..s7/ra at16..48; rotation is frameless.
    spills = set()
    if entry == PARENT:
        spills.update(range((STACK - 16) & 0x1FFFFF, STACK & 0x1FFFFF))
        if clip == 0:
            return spills
    caller_stack = STACK - (32 if entry == PARENT else 0)
    spills.update(range((caller_stack - 40) & 0x1FFFFF, (caller_stack - 4) & 0x1FFFFF))
    return spills


def comparable(ram, entry, clip):
    result = bytearray(ram)
    for index in stack_spills(entry, clip):
        result[index] = 0
    return result


def read_exact(stream, count):
    result = bytearray()
    while len(result) < count:
        value = stream.read(count - len(result))
        if not value:
            raise RuntimeError("native transport truncated")
        result.extend(value)
    return result


class Coverage(set):
    def __init__(self, exe):
        super().__init__(); self.exe = exe

    def add(self, pc):
        super().add(pc)
        insn = struct.unpack_from("<I", self.exe, pc - 0x80010000 + 0x800)[0]
        if insn >> 26 in (1, 2, 3, 4, 5, 6, 7) or (insn >> 26 == 0 and insn & 63 in (8, 9)):
            # The retained runner always executes a branch/jump's delay slot.
            super().add(pc + 4)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", type=Path, required=True)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--native", type=Path, required=True)
    parser.add_argument("--pins", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    exe = args.exe.read_bytes(); pins = json.loads(args.pins.read_text())
    if hashlib.sha1(exe).hexdigest() != "452fb033f2eaa4b18aa20a5bca60b8125af3a37b":
        raise RuntimeError("retail EXE mismatch")
    for entry, size in SPANS.items():
        offset = entry - 0x80010000 + 0x800
        if hashlib.sha256(exe[offset:offset + size]).hexdigest() != pins[f"{entry:08X}"]:
            raise RuntimeError("retail body mismatch")
    oracle_path = args.base / "pc_port/tools/pe_battle_hud_oracle.py"
    spec = importlib.util.spec_from_file_location("animation_retail_oracle", oracle_path)
    oracle = importlib.util.module_from_spec(spec); spec.loader.exec_module(oracle)
    coverage, results = Coverage(exe), []
    with subprocess.Popen([str(args.native.resolve())], stdin=subprocess.PIPE, stdout=subprocess.PIPE) as native:
        try:
            for name, entry, clip, frame, (ram, scratch) in inputs(exe):
                expected_ram, expected_scratch = bytearray(ram), bytearray(scratch)
                oracle.execute(expected_ram, entry, (DEST, clip, frame & MASK),
                               instruction_budget=250000, scratchpad=expected_scratch, visited_pcs=coverage)
                if name.startswith("header-output-alias-") and expected_ram[(CLIP + 1) & 0x1FFFFF] != 16:
                    raise RuntimeError("output-alias fixture did not change the retail joint count to16")
                native.stdin.write(b"ANM1" + struct.pack("<4I", entry, DEST, clip, frame & MASK) + ram + scratch)
                native.stdin.flush()
                if read_exact(native.stdout, 4) != b"ARES":
                    raise RuntimeError("invalid native transport")
                actual_ram = read_exact(native.stdout, RAM_SIZE)
                actual_scratch = read_exact(native.stdout, SPAD_SIZE)
                if comparable(actual_ram, entry, clip) != comparable(expected_ram, entry, clip) or actual_scratch != expected_scratch:
                    raise RuntimeError(f"composed native/retail RAM or scratchpad mismatch: {name}")
                # Comparer controls use real retail output, including a pad byte
                # no decoder should overwrite and a retained undecoded row.
                for index in (0x140000, 0x110000):
                    changed = bytearray(expected_ram); changed[index] ^= 1
                    if comparable(changed, entry, clip) == comparable(expected_ram, entry, clip):
                        raise RuntimeError("RAM comparer negative control failed")
                for index in (6, 127 * 8, 127 * 8 + 6):
                    changed = bytearray(expected_scratch); changed[index] ^= 1
                    if changed == expected_scratch:
                        raise RuntimeError("scratchpad comparer negative control failed")
                results.append({"case": name, "entry": f"{entry:08X}",
                                "input_sha256": hashlib.sha256(ram + scratch).hexdigest(),
                                "output_sha256": hashlib.sha256(comparable(actual_ram, entry, clip) + actual_scratch).hexdigest()})
            native.stdin.close()
            if native.wait(timeout=5):
                raise RuntimeError("native harness failed at closure")
        finally:
            if native.poll() is None:
                native.kill(); native.wait()
    if len(results) != 117:
        raise RuntimeError(f"case count {len(results)} != 117")
    allowed = {pc for entry, size in SPANS.items() for pc in range(entry, entry + size, 4)}
    if coverage - allowed:
        raise RuntimeError("retail execution left the admitted four-function graph")
    if not all(any(entry <= pc < entry + size for pc in coverage) for entry, size in SPANS.items()):
        raise RuntimeError("a required actual callee was not executed")
    args.output.write_text(json.dumps({"status": "PASS", "cases": len(results), "skipped": 0,
        "scope": "matched animation parent/decoders + actual rotation callee on both sides; no mocks",
        "stack_exclusions": "52 exact prologue spill bytes for parent; 36 direct; 16 null clip; rotation frameless",
        "comparisons": "all other main RAM and every scratchpad byte, including pad/retained rows",
        "coverage": [f"{pc:08X}" for pc in sorted(coverage)],
        "coverage_limit": "actual words reported; no claim of all branches or room reachability",
        "results": results}, indent=2) + "\n")
    print(f"native animation + actual rotation: {len(results)} cases PASS, zero skipped")


if __name__ == "__main__":
    main()
