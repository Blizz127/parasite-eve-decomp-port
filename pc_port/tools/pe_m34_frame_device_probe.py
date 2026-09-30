#!/usr/bin/env python3
"""Observe original M34 frame stack writes with explicit device input contracts.

This is an isolated diagnostic, never a route-state restore. GPU rasterization,
interrupt handlers and BIOS internals are not executed. Device completion and
the VBlank counter are fixture inputs, so this is not full hardware fidelity.
"""
import argparse
import hashlib
import inspect
import json
import struct
from collections import Counter, deque
from pathlib import Path

import pe_battle_hud_oracle as m
from pe_btl14_m0005i_publish_oracle import find_disc, read_form1

OFFSETS = (-76, -72, -68, -36, -32, -28)
ENTRY_SP = 0x801FEEE8
WATCHED = {(ENTRY_SP + off + i) & 0x1FFFFF for off in OFFSETS for i in range(4)}


def u32(memory, address):
    return struct.unpack_from('<I', memory, address)[0]


def store32(memory, address, value):
    struct.pack_into('<I', memory, address, value & 0xFFFFFFFF)


def source_image():
    exe = (m.ROOT / 'build/extracted/disc1/SLUS_006.62').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    overlay = read_form1(find_disc(m.ROOT), 16597, 100)
    assert hashlib.sha256(overlay).hexdigest() == '0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a'
    source = bytearray(0x200000)
    source[0x10000:0x10000 + len(exe) - 0x800] = exe[0x800:]
    source[0x18EFE8:0x18EFE8 + len(overlay)] = overlay
    return source


def probe(path, source, mode, period, max_frames, caller_snapshot=False, initializer_target=1, instruction_budget=3000000):
    raw = path.read_bytes()
    assert len(raw) == 0x200000
    ram, io = bytearray(raw), bytearray(4096)
    counts, ports = Counter(), Counter()
    last, events, writers = deque(maxlen=12), [], {}
    ticks, low_sp, frame = 0, 0x801FF000, -1
    start_frame = int(path.stem.split('-')[1]) if mode == 'continuous' else None
    initializer_count = 0
    def stop_initializer(pc):
        nonlocal initializer_count
        if mode != 'continuous' or pc != 0x8018F434:
            return False
        initializer_count += 1
        return initializer_count == initializer_target
    store32(io, 0xF0, 0x07654321)
    store32(io, 0xE8, 2)
    store32(io, 0x814, 0x14802000)

    def observe_write(address, size, writer):
        # Only physical RAM and its cached/uncached aliases are relevant.
        if address >> 21 not in (0, 0x400, 0x500):
            return
        address &= 0x1FFFFF
        assert address + size <= len(ram)
        for a in WATCHED.intersection(range(address, address + size)):
            writers[a] = {'writer': writer, 'frame_index': frame}

    def before(pc, regs, memory):
        nonlocal ticks, low_sp, frame
        ticks += 1
        counts[pc] += 1
        last.append(hex(pc))
        low_sp = min(low_sp, regs[29])
        a = pc & 0x1FFFFF
        assert memory[a:a + 4] == source[a:a + 4], ('executed instruction', hex(pc))
        assert raw[a:a + 8] == source[a:a + 8], ('source/following word', hex(pc))
        if pc == 0x8003F404:
            frame += 1
            assert frame < max_frames, 'frame budget'
            if mode == 'continuous':
                # External digital-pad reply, matching the existing M34 pilot.
                struct.pack_into('<HH', ram, 0xBE9A0, 0x4100,
                                 0xBFFF if (start_frame + frame) % 8 == 3 else 0xFFFF)
        if ticks % period == 0:
            # Fixture counters, not emulation of VBlank/BIOS IRQ delivery.
            store32(ram, 0x956AC, u32(ram, 0x956AC) + 1)
            store32(io, 0x110, u32(io, 0x110) + 1)
        word = u32(memory, a)
        op = word >> 26
        if op not in (40, 41, 42, 43, 46, 58):
            return
        address = (regs[word >> 21 & 31] + (word & 32767) - (word & 32768)) & 0xFFFFFFFF
        size = {40: 1, 41: 2}.get(op, 4)
        if op == 42:  # SWL writes the low bytes of the aligned memory word.
            size, address = (address & 3) + 1, address & ~3
        elif op == 46:  # SWR writes from address through the word's high byte.
            size = 4 - (address & 3)
        observe_write(address, size, hex(pc))

    def after(pc, regs, memory, address, op):
        if not (0x1F801000 <= address < 0x1F802000 and op in (32, 33, 35, 36, 37, 40, 41, 43)):
            return
        ports[(address, op)] += 1
        a = address - 0x1F801000
        if op not in (40, 41, 43):
            return
        if a == 0x814:
            events.append(['GP1', hex(u32(io, a))])
            store32(io, a, 0x14802000)  # Ready-device input; no GP1/raster model.
        if a == 0xE8 and u32(io, a) & 0x1000000:
            start, count = u32(io, 0xE0) & 0x1FFFFC, u32(io, 0xE4) & 65535
            count = count or 65536
            for i in range(count):
                dest = (start - i * 4) & 0x1FFFFC
                observe_write(dest, 4, 'DMA6 OTC')
                store32(ram, dest, 0xFFFFFF if i == count - 1 else (start - i * 4 - 4) & 0xFFFFFF)
            store32(io, 0xE0, (start - count * 4) & 0xFFFFFF)
            store32(io, a, 2)
            events.append(['OTC', hex(start), count])
        if a == 0xA8 and u32(io, a) & 0x1000000:
            control = u32(io, a)
            assert control & 1, 'GPU-to-RAM DMA is not implemented'
            events.append(['GPU-DMA', hex(control), hex(u32(io, 0xA0)), hex(u32(io, 0xA4))])
            # Explicit completion only: no packet parsing, VRAM or rasterization.
            store32(io, a, control & ~0x11000000)

    def bios(regs):
        service = regs[9]
        events.append(['BIOS', hex(service), hex(regs[4]), hex(regs[5]), regs[6]])
        if service == 0x2A:
            dst, src, size = regs[4] & 0x1FFFFF, regs[5] & 0x1FFFFF, regs[6]
            if regs[4] and size <= 0x7FFFFFFF:
                assert dst + size <= len(ram) and src + size <= len(ram)
                assert size == 0 or dst + size <= src or src + size <= dst, 'overlapping memcpy not modeled'
                observe_write(regs[4], size, 'BIOS memcpy contract')
                ram[dst:dst + size] = ram[src:src + size]
            regs[2] = regs[4]
        elif service in (0x28, 0x2B):
            observe_write(regs[4], regs[5] if service == 0x28 else regs[6], 'BIOS memory contract')
        else:
            assert service in (0x30, 0x2F, 0x3F), ('unsupported BIOS contract', service)

    # Instrument the existing interpreter, with checked anchors. No original
    # instruction/callee is replaced; only explicit device and BIOS contracts.
    code = inspect.getsource(m.execute)
    replacements = (
        ('        if pc == 0 or pc in stop_at:', '        if pc == 0 or pc in stop_at or stop_initializer(pc):'),
        ('        w = struct.unpack_from', '        before(pc,r,ram)\n        w = struct.unpack_from'),
        ('        jump = None', '        if 0x1F801000 <= address < 0x1F802000:\n            memory,a=io,address-0x1F801000\n        jump = None'),
        ('        return jump', '        after(pc,r,ram,address,op)\n        return jump'),
        ('        if pc == 0xA0:\n            if r[9] == 0x2B:', '        if pc == 0xA0:\n            bios(r)\n            if r[9] == 0x2A:\n                pass\n            elif r[9] == 0x2B:'),
    )
    for old, new in replacements:
        assert code.count(old) == 1, ('interpreter hook changed', old)
        code = code.replace(old, new, 1)
    namespace = dict(m.__dict__, before=before, after=after, bios=bios, io=io, stop_initializer=stop_initializer)
    exec(code, namespace)
    entry = 0x8003F4F8 if mode == 'tail' else 0x8003F404
    stops = {'tail': (0x8003F678,), 'prefix': (0x8003F4D0,),
             'continuous': (0x8003F68C,)}[mode]
    result = dict(capture=path.name, capture_sha256=hashlib.sha256(raw).hexdigest(),
                  mode=mode, counter_period=period, caller_snapshot=caller_snapshot,
                  scope='Original instructions with supplied initial CPU/GTE/stack and device inputs; BIOS contracts exclude BIOS stack/internal execution; no IRQ/raster proof.')
    try:
        if caller_snapshot:
            assert mode == 'continuous'
            # Execute the original main-loop load/store, not a RAM patch.
            # Useful for older native captures whose snapshot was host-only.
            namespace['execute'](ram, 0x800122CC, stop_at=(0x800122DC,))
        regs = namespace['execute'](ram, entry, stop_at=stops,
            initial_regs={16: 0x800B0CD8, 17: 0x800BCFE8, 18: 0x800B0CEA},
            scratchpad=bytearray(1024), instruction_budget=instruction_budget)
        result['initializer_reached'] = mode == 'continuous' and regs[31] == 0x800C2CE4
        result['final_sp'] = hex(regs[29])
    except Exception as error:
        result['error'] = str(error)
    result.update(low_sp=hex(low_sp), pcs=len(counts), instructions=ticks,
                  frame_count=frame + 1, last=list(last), events=events,
                  ports=[dict(address=hex(a), op=op, count=n) for (a, op), n in sorted(ports.items())],
                  dependencies=[dict(offset=off, value=u32(ram, (ENTRY_SP + off) & 0x1FFFFF),
                      writers=[writers.get((ENTRY_SP + off + i) & 0x1FFFFF) for i in range(4)])
                      for off in OFFSETS])
    if initializer_target != 1:
        result.update(initializer_target=initializer_target, initializer_count=initializer_count)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('capture', type=Path)
    parser.add_argument('--mode', choices=('prefix', 'tail', 'continuous'), default='tail')
    parser.add_argument('--counter-period', type=int, default=1024)
    parser.add_argument('--max-frames', type=int, default=100)
    parser.add_argument('--initializers', type=int, default=1,help='stop at this initializer in continuous mode')
    parser.add_argument('--instruction-budget', type=int, default=3000000)
    parser.add_argument('--caller-snapshot', action='store_true',
                        help='execute original 122CC..122DC destination snapshot before an isolated continuous loop')
    args = parser.parse_args()
    assert min(args.counter_period,args.max_frames,args.initializers,args.instruction_budget)>0
    result = probe(args.capture, source_image(), args.mode, args.counter_period, args.max_frames,
                   args.caller_snapshot,args.initializers,args.instruction_budget)
    print(json.dumps(result, indent=2))
    raise SystemExit(1 if 'error' in result else 0)


if __name__ == '__main__':
    main()
