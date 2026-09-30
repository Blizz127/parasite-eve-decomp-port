#!/usr/bin/env python3
"""Pinned station-neighbor inventory and original closed routing predicates.

Stops precede every asynchronous/scene opcode; this does not infer room/day
membership from adjacency or execute omitted movement, conversations or fades.
"""
import hashlib
import json
import struct
from pe_day2_route_audit import (ROOT, SCRIPTS, parse_field_table,
    extract_script_from_package, decode_script, decode_packed_name, find_disc,
    read_form1)
from pe_battle_hud_oracle import execute

PINS = dict(SCRIPTS)
PINS.update({
    45: 'f192f9c53e9312a16c7cb74584b2cb86882440a44a65f31db53be776db572222',
    46: '00891a602db22f3bc3a88081af2270e676dbf34e92b3c2eb382e4f5457ed1320',
    47: '6e619093b70643e92dbdcf956a2dc21c09fd306075629b170580a8fb5f7c961a',
    51: '338dc38700e2316e2403524141b19b3ed80ab3d4180761d4c4450242d70e0b2c',
    56: '2af526915b74662c1ae50e1bd949f94123372336d451517cecacd54609def970',
})
HANDLERS = {0: 0x80017294, 5: 0x8001731C, 9: 0x80012850, 10: 0x800173F4}
STORIES = tuple(range(0x301)) + (0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)


def load():
    ex = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(ex).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    disc = find_disc(ROOT)
    rooms = {}
    for rec in parse_field_table(ex):
        number = rec['map_id']
        if number not in PINS:
            continue
        meta = rec['meta']
        c0, c1, c2 = meta & 255, meta >> 8 & 4095, meta >> 20
        package = read_form1(disc, 1013 + rec['start'], c0 + c1 + c2)
        off, raw = extract_script_from_package(package, meta)
        assert (c0 + c1) * 2048 <= off < len(package)
        script = decode_script(raw)
        assert script['sha256'] == PINS[number]
        base = 0x8018EFE8 + off - (c0 + c1) * 2048
        rooms[number] = (raw, script, base, rec, package[(c0+c1)*2048:])
    assert set(rooms) == set(PINS)
    return ex, rooms


def inventory(rooms):
    rows = []
    for number, (raw, script, base, rec, chunk) in sorted(rooms.items()):
        transfers, writes, unresolved_transfers = [], [], []
        for mod in script['modules']:
            for cmd in mod['commands']:
                pc, args, modes = base + cmd['offset'], cmd['args'], cmd['modes']
                if cmd['opcode'] == 0x31:
                    if modes == [0]:
                        transfers.append(dict(pc=f'{pc:08X}', module=mod['index'],
                                              destination=decode_packed_name(args[0])))
                    else:
                        unresolved_transfers.append(dict(pc=f'{pc:08X}', modes=modes, args=args))
                if cmd['opcode'] == 10 and modes[0] == 2 and args[0] == 74:
                    writes.append(dict(pc=f'{pc:08X}', module=mod['index'],
                                       source_mode=modes[1], value=f'{args[1]:X}'))
        rows.append(dict(room=number, script_sha256=script['sha256'],
                         script_base=f'{base:08X}', chunk_sha256=hashlib.sha256(chunk).hexdigest(),
                         modules=script['module_count'], commands=sum(len(m['commands']) for m in script['modules']),
                         transfers=transfers, unresolved_transfers=unresolved_transfers, story_writes=writes))
    return rows


def run(ex, room, module, start, stops, story=0x90, flags45=0, payload=0, flags0=0, flags3=0, extra_persist=None):
    raw, script, base, _, _ = room
    mod = script['modules'][module]
    r = bytearray(0x200000)
    r[0x10000:0x10000+len(ex)-0x800] = ex[0x800:]
    r[base & 0x1FFFFF:(base & 0x1FFFFF)+len(raw)] = raw
    def sw(a, v): struct.pack_into('<I', r, a & 0x1FFFFF, v & 0xFFFFFFFF)
    def lw(a): return struct.unpack_from('<I', r, a & 0x1FFFFF)[0]
    sw(0x8009D2F0, 0x80152000)
    sw(0x8015209C, base + mod['start'])
    sw(0x800A7918, story)
    sw(0x800A77F0 + 45*4, flags45)
    sw(0x800A77F0, flags0)
    sw(0x800A77F0+3*4, flags3)
    sw(0x800A77F4, 0xDEADBEEF)
    for index, value in (extra_persist or {}).items():
        sw(0x800A77F0+index*4, value)
    sw(0x80153010, payload)
    commands = {base + c['offset']: c for c in mod['commands']}
    pc, trace = start, []
    for _ in range(256):
        if pc in stops:
            return dict(stop=f'{pc:08X}', extra_persist={i:lw(0x800A77F0+i*4) for i in (extra_persist or {})}, story=lw(0x800A7918),
                        flags45=lw(0x800A77F0+45*4), flags3=lw(0x800A77F0+3*4), previous_room=lw(0x800A77F4), trace=trace)
        cmd = commands[pc]
        assert cmd['opcode'] in HANDLERS, f'unresolved opcode {cmd["opcode"]:X} at {pc:X}'
        sw(0x8009CE00, pc + 8 + 4*cmd['argc'])
        for i, (mode, value) in enumerate(zip(cmd['modes'], cmd['args'])):
            assert mode <= 4
            address = pc+8+i*4 if mode == 0 else (0, 0x80153000, 0x800A77F0, 0x8009DF70, 0x800B6A80)[mode]+value*4
            sw(0x80150000+i*4, address)
        regs = execute(r, HANDLERS[cmd['opcode']], (0x80150000,))
        assert regs[2] == 1
        trace.append(f'{pc:08X}')
        pc = lw(0x8009CE00)
    raise AssertionError('closed region did not terminate')


def main():
    ex, rooms = load()
    rows, cases = inventory(rooms), []
    def check(name, number, module, start, stops, expected, **kw):
        got = run(ex, rooms[number], module, start, stops, **kw)
        assert got['stop'] == f'{expected:08X}', (name, kw, got, hex(expected))
        cases.append(dict(region=name, room=number, input=kw, result=got))
        return got
    # Door selector only: first scene opcode is an explicit boundary.
    doors = {1:0x801BE9DC, 0:0x801BEB10, 4:0x801BEC48, 5:0x801BED80, 6:0x801BEDF0, 7:0x801BEF5C}
    for payload in tuple(range(256)) + (0x100, 0xFFFFFFFF):
        check('43_door_dispatch',43,0,0x801BE9B4,set(doors.values())|{0x801BEFA4},
              doors.get(payload,0x801BEFA4),payload=payload)
    for story in STORIES:
        signed = story if story < 0x80000000 else story - 0x100000000
        got = check('51_stairs_destination',51,0,0x801BE6E8,{0x801BE720,0x801BE738},
                    0x801BE720 if signed < 0x188 else 0x801BE738,story=story)
        assert got['previous_room'] == 51 and got['story'] == story
        check('43_exit_fork',43,0,0x801BEEBC,{0x801BEEE4,0x801BEEF0},
              0x801BEEE4 if story == 0xDA else 0x801BEEF0,story=story)
        check('41_trigger_story',41,8,0x801E3488,{0x801E34B0,0x801E3648},
              0x801E34B0 if signed <= 0xA4 else 0x801E3648,story=story)
        for flags in (0,0x10,0xFFFFFFEF,0xFFFFFFFF):
            check('41_reminder',41,8,0x801E356C,{0x801E3604,0x801E3640},
                  0x801E3604 if 0x98 <= signed < 0xA4 and not flags&0x10 else 0x801E3640,
                  story=story,flags45=flags)
    thresholds = (0x78,0x90,0xB8,0xD0,0xE0,0x148,0x178,0x1B8,0x1C0,0x218,0x258)
    boundary_stories = sorted({0,0x7FFFFFFF,0x80000000,0xFFFFFFFF} |
                              {t+d for t in thresholds for d in (-1,0,1)})
    for story in boundary_stories:
        signed = story if story < 0x80000000 else story-0x100000000
        for flags0 in (0,1,0xFFFFFFFE,0xFFFFFFFF):
            for seed in (0,0xFFFFFFFF):
                mask = seed | 0xC000
                for threshold, bits in ((0xB8,0x1C000),(0xE0,0x3C000),(0x148,0x7C000),
                                        (0x1C0,0xFC000),(0x218,0x3FC000),(0x258,0x7FC000)):
                    if signed >= threshold: mask |= bits
                if flags0 & 1: mask |= 0x800000
                for threshold, enabled in ((0x78,True),(0x90,False),(0xD0,True),(0x148,False),
                                           (0x178,True),(0x1B8,False),(0x1C0,True),(0x218,False),(0x258,True)):
                    if signed >= threshold:
                        mask = mask | 0x2000 if enabled else mask & ~0x2000
                got = check('46_world_map_flags',46,0,0x801AAB20,{0x801AB4F8},0x801AB4F8,
                            story=story,flags0=flags0,flags3=seed)
                assert got['flags3'] == mask and got['story'] == story, (story,mask,got)
    # The destination opcodes themselves are checked; movement/fades before
    # the selector are not bypassed or claimed executed.
    commands = {rooms[51][2]+c['offset']:c for c in rooms[51][1]['modules'][0]['commands']}
    for pc, dest in ((0x801BE720,'m0052i'), (0x801BE738,'m0048i')):
        assert commands[pc]['opcode'] == 0x31
        assert decode_packed_name(commands[pc]['args'][0]) == dest
    assert len(rows) == 9
    out = dict(scope='nine shared scripts; closed predicates only, not Day2 membership or asynchronous execution',
               rooms=rows, cases=cases)
    print(json.dumps(out,indent=2))


if __name__ == '__main__':
    main()
