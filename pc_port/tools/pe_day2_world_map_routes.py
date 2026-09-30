#!/usr/bin/env python3
"""Compose original station mask preparation, map records and exit selection.

Pure code windows only. Constructor setup and SDK/media work are boundaries;
script writes are static evidence until their preceding scene is executed.
"""
import json
import struct
import pe_day2_station_routes as station
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import execute
from pe_btl14_m0005i_publish_oracle import decode_token

NEW_PINS = {
    38: '845e6c3cf89ef3d77b9a3664cb201699b3d5d5e392b96a738f332e59ece9d77d',
    191: '2af377c4e4ab143122bd4f20998a3ccc894be387376adf95ecd8287da7a1c499',
}
BITS = (22,20,18,21,19,23,14,15,17,16)
STORIES = (0x78,0x80,0x88,0x90,0x98,0xA4,0xA6,0xB7,0xB8,0xB9,
           0xC0,0xC8,0xCF,0xD0,0xD1,0x138,0x148,0x150,0x178,0x180,
           0x1C0,0x208,0x210,0x218,0x258,0x80000000,0xFFFFFFFF)


def put(r,a,v): struct.pack_into('<I',r,a & 0x1FFFFF,v & 0xFFFFFFFF)
def word(r,a): return struct.unpack_from('<I',r,a & 0x1FFFFF)[0]


def main():
    station.PINS.update(NEW_PINS)
    ex, rooms = station.load()
    e, overlay, base = load_overlay()
    assert ex == e
    seed = bytearray(0x200000)
    seed[0x10000:0x10000+len(ex)-0x800] = ex[0x800:]
    seed[base & 0x1FFFFF:(base & 0x1FFFFF)+len(overlay)] = overlay
    cases = []
    for story in STORIES:
        for flags0 in (0,1):
            for prior in (0,0xFFFFFFFF):
                state = station.run(ex,rooms[46],0,0x801AAB20,{0x801AB4F8},
                                    story=story,flags0=flags0,flags3=prior)
                flags = state['flags3']
                r = bytearray(seed)
                put(r,0xA77FC,flags)
                # Original straight-line record writer, before object creation.
                r[0x1EA370:0x1EA578] = bytes([0xA5])*520
                execute(r,0x80196620,stop_at=(0x80196EAC,),initial_regs={2:0x80140000})
                enabled = [r[0x1EA370+i*52+36] for i in range(10)]
                assert enabled == [(flags >> bit)&1 for bit in BITS]
                choices = []
                for selection in range(10):
                    q = bytearray(r)
                    put(q,0xA7918,story);put(q,0x19CA68,selection)
                    regs = execute(q,0x80192030,stop_at=(0x80074D28,))
                    assert regs[31] == 0x80192254
                    token = word(q,0x9D280)
                    dest = decode_token(ex,token).rstrip(b'\0').decode('ascii')
                    assert word(q,0xA77F4) == 999
                    choices.append(dict(selection=selection,enabled=bool(enabled[selection]),destination=dest))
                cases.append(dict(story=f'{story:X}',flags0=flags0,prior_mask=f'{prior:08X}',
                                  prepared_mask=f'{flags:08X}',choices=choices))
    # Scope-specific expectations are separate from both original windows.
    for c in cases:
        story = int(c['story'],16)
        if c['prior_mask'] != '00000000' or c['flags0']:
            continue
        allowed = [(q['selection'],q['destination']) for q in c['choices'] if q['enabled']]
        if story in (0x90,0x98,0xA4,0xA6,0xB7):
            assert allowed == [(6,'M0024I'),(7,'M0046I')]
        if story == 0xB8:
            assert allowed == [(6,'M0024I'),(7,'M0046I'),(9,'M0038I')]
        if story in (0xB9,0xC0,0xC8,0xCF):
            assert allowed == [(6,'M0024I'),(7,'M0046I'),(9,'M0191I')]
        if story == 0xD0:
            assert allowed == [(6,'M0024I'),(7,'M0037I'),(9,'M0192I')]
    scene_tails = []
    for prior in (0,0x2000,0x1C000,0xFFFFFFFF):
        for flags0 in (0,1):
            # Start only after the preceding scene work has completed.
            state = station.run(ex,rooms[38],2,0x801C9A40,{0x801CA438},
                                story=0xB8,flags0=flags0,flags3=prior)
            assert state['story'] == 0xC0 and state['previous_room'] == 999
            expected = (prior | 0x1C000 | (0x800000 if flags0 else 0)) & ~0x2000
            assert state['flags3'] == expected
            r = bytearray(seed)
            put(r,0xA7918,state['story']);put(r,0xA77FC,state['flags3']);put(r,0x19CA68,9)
            execute(r,0x80192030,stop_at=(0x80074D28,))
            dest = decode_token(ex,word(r,0x9D280)).rstrip(b'\0').decode('ascii')
            assert dest == 'M0191I'
            scene_tails.append(dict(prior=prior,flags0=flags0,state=state,destination=dest))
    scene_gates = []
    for story in station.STORIES:
        got = station.run(ex,rooms[38],2,0x801C9538,{0x801C9560,0x801CA444},story=story)
        assert got['stop'] == ('801C9560' if story == 0xB8 else '801CA444')
        scene_gates.append(dict(story=story,result=got))
    added = [r for r in station.inventory(rooms) if r['room'] in NEW_PINS]
    assert len(cases) == 108
    print(json.dumps(dict(scope='original closed station preparation -> record enable bytes -> exit selector prefix; not live traversal',
                          record_enable_bits=BITS,added_scripts=added,cases=cases,scene_gates=scene_gates,scene_tails=scene_tails),indent=2))


if __name__ == '__main__': main()
