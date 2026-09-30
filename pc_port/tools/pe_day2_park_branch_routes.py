#!/usr/bin/env python3
"""Pinned park side branches and original story/music predicates.

Execution stops at scene operations; adjacency does not establish day membership.
"""
import json
import pe_day2_station_routes as s

PINS = {
    69: 'b7ab15072fbc14e97f2465ee168ddfc770cb32528986a4519d93463ef26fbd23',
    70: '84681885170eebfe62078d039db24358d06a9ce1109824857cb1934297363f75',
    73: '619806124484b02fa6aedefce1143a1ae00d47363275b2c4cebf7fe91e005f71',
    75: 'b487429cdbd06baeab4121d4515d1739dd86246d5afe16fed1cbd4bc5426cf96',
    76: 'ff2610e3cc91aac2f3b7cbc87c62e256ad0f07c75a5fc42455e6620fcbfe385f',
}
EDGES = {69: [71, 72, 72], 70: [72, 72], 73: [68, 74, 76], 75: [74], 76: [74, 77, 78, 79]}
WRITES = {69: [('801C66F8', 0, 'F0')],
          70: [('801C798C', 0, 'F8'), ('801C7AC0', 0, 'F8')]}


def main():
    s.PINS = dict(PINS)
    ex, rooms = s.load()
    rows, cases = s.inventory(rooms), []
    for row in rows:
        assert not row['unresolved_transfers']
        assert [int(e['destination'][1:5]) for e in row['transfers']] == EDGES[row['room']]
        assert [(w['pc'], w['source_mode'], w['value']) for w in row['story_writes']] == WRITES.get(row['room'], [])

    def check(room, module, start, stops, expected, story):
        got = s.run(ex, rooms[room], module, start, set(stops), story=story)
        assert got['stop'] == f'{expected:08X}', (room, hex(start), hex(story), got)
        cases.append(dict(room=room, start=f'{start:08X}', story_input=story, result=got))
        return got

    for story in s.STORIES:
        signed = story if story < 0x80000000 else story - 0x100000000
        for room, module, start, yes, no, condition in (
            (69, 1, 0x801C6BE0, 0x801C6C08, 0x801C6C28, signed >= 0xF8),
            (69, 1, 0x801C6C28, 0x801C6C80, 0x801C6CA0, 0xF0 < signed < 0xF8),
            (69, 1, 0x801C6CA0, 0x801C6CC8, 0x801C6CE8, signed < 0xF0),
            (70, 1, 0x801C7568, 0x801C7590, 0x801C75BC, signed >= 0xF0),
            (75, 1, 0x801B3ECC, 0x801B3EF4, 0x801B3F20, signed >= 0xF0),
        ):
            got = check(room, module, start, (yes, no), yes if condition else no, story)
            assert got['story'] == story
        for room, module, start, stop, value in (
            (69, 0, 0x801C66F8, 0x801C6708, 0xF0),
            (70, 3, 0x801C798C, 0x801C799C, 0xF8),
            (70, 3, 0x801C7AC0, 0x801C7AD0, 0xF8),
        ):
            assert check(room, module, start, (stop,), stop, story)['story'] == value
    path = s.ROOT / 'local/live/day2-park-branch-routes-131.json'
    path.write_text(json.dumps(dict(
        scope='five pinned scripts; closed original story/music predicates and writes only',
        rooms=rows, cases=cases), indent=2) + '\n')
    print(f'PASS {len(cases)} original paths; five scripts and thirteen static transfers')


if __name__ == '__main__':
    main()
