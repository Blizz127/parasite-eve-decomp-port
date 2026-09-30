#!/usr/bin/env python3
"""Late park adjacency, progression endpoints and alternate-route predicates.

Closed original execution only; scene/movement/battle/media calls are stops.
M0090I/M0091I are candidate continuation inventory, not proven reachable here.
"""
import json
import pe_day2_station_routes as s

PINS = {58: '5caa361d01a65132d701ce456416cd15fdf75dff973536a9924325ce1c00ab82',
 71: '256b13f6fe6958af987c3f06795d6d519f0287034d1ad875b5a262f3ce6a9e75',
 77: 'e5e970544ea0c0dda3fba9d157bf3c8a7f9098045fa2eec9989fb2b02639aba1',
 78: 'd332bbf1903dcc60590ac031b3b61108628737d1f2a5ffb6fea6d5e49829d844',
 79: 'c3a63a07a67c90b5569dd5cc7334c8fa328014fee994b7994c7ef8719535d9df',
 80: '5e664cc5107bf932409dda71b291d5c4f8eebfa99338ac5cd6bb420eada56305',
 81: '24ff1485c521f017e63833acd7acf9f7607114eb1f8367c44f68a0ccee5f4994',
 82: 'ad38bd50717bd430dc50b49385d12bf61cccd47f1251e5b74c1d4cbbb53916bd',
 83: '715b114a869cc64076b2e0584bc342e1fe516fbc641cd5f44cbc2e8c1b6c24de',
 84: 'd806837c5f1559ebfbf2ded9b8e604c164f0b4bee43be9b8b65237e6f1cabcdb',
 85: '134cc927b6e7cba0f04cc809ea7bf46b4346a57b5c0c422d736d1da4235e3029',
 86: '3c577d2c83e5ecaf5d643e8f40be7355d5fda960fc1bdcb2dd30bdf8e6caf1f7',
 87: 'e38b9a6e23efb9c53995f9f74a56caf2dad9cecc8ace5c3734f43658361c5976',
 88: '80c6bea61964c45cfaf6d65d4945707ea464e57962c22e41a5457ca359370093',
 89: '522fde958e43a7e73fa53060103da14cfc1350c20ab75aafa5b7ae7a5e0808b8',
 90: '99f3eb2a30a110389859c4924d87ba46ad921d1d387982afc0efd6a4c6714e40',
 91: 'd7b75d9ac5d7e5eb55ed59c638de28df126270797e9683ba31576ee172079071',
 351: '1cc11664e59100e6378107b99ade037704e1917702182264e459c7d1172b7203'}
EDGES = {58: [('801B47EC', 'm0041i'), ('801B5810', 'm0000i'), ('801B5908', 'm0059i')],
 71: [('801C2624', 'm0069i')],
 77: [('801A054C', 'm0076i'), ('801A0630', 'm0080i')],
 78: [('8019CDE8', 'm0080i'), ('8019CECC', 'm0079i'), ('8019CFB0', 'm0076i')],
 79: [('801A6188', 'm0076i'), ('801A626C', 'm0078i'), ('801A6350', 'm0081i')],
 80: [('801B54E0', 'm0077i'), ('801B55C4', 'm0078i')],
 81: [('8019C718', 'm0082i'), ('8019C7FC', 'm0079i')],
 82: [('801B51EC', 'm0081i'), ('801B52D0', 'm0083i')],
 83: [('801A1D04', 'm0082i'),
      ('801A1E38', 'm0084i'),
      ('801A1F6C', 'm0084i'),
      ('801A20A0', 'm0084i'),
      ('801A21C0', 'm0085i'),
      ('801A22E0', 'm0085i')],
 84: [('801942E0', 'm0083i'),
      ('80194414', 'm0083i'),
      ('80194520', 'm0083i'),
      ('80194604', 'm0085i')],
 85: [('801A0D2C', 'm0083i'),
      ('801A0E38', 'm0083i'),
      ('801A0F1C', 'm0084i'),
      ('801A1000', 'm0087i')],
 86: [('801A6570', 'm0087i'), ('801A6654', 'm0088i')],
 87: [('801ACF54', 'm0085i'), ('801AD038', 'm0086i')],
 88: [('801CC82C', 'm0089i'), ('801CC9F0', 'm0086i'), ('801CCC70', 'm0089i')],
 89: [('801DE660', 'm0092i'), ('801E2D48', 'm0367i')],
 90: [('801C2E4C', 'm0136i')],
 91: [('801C3B58', 'm0351i')],
 351: [('80191124', 'm0042i'), ('80191168', 'm0092i')]}
WRITES = {58: [('801B4794', 0, '120')],
 71: [('801C2604', 0, 'F0'), ('801C27A8', 0, 'F0')],
 77: [],
 78: [],
 79: [],
 80: [],
 81: [],
 82: [],
 83: [],
 84: [],
 85: [],
 86: [],
 87: [],
 88: [('801CC7C8', 0, '118'), ('801CCC50', 0, '110')],
 89: [('801DE640', 0, '140'), ('801E2D08', 0, '11B')],
 90: [('801C2E00', 0, '130'), ('801C2E10', 0, '134')],
 91: [('801C3B38', 0, '138')],
 351: [('80191114', 0, '88'), ('80191158', 0, '140')]}


PINS[367] = 'dc0d5b09ccc503f24ed981f17c6a1cb2329fdc4723e6b1b0a5a0ef4fe7a5676e'
EDGES[367] = [('801AA714', 'm0005i'), ('801AA850', 'm0319i'),
              ('801AA948', 'm0239i'), ('801AAC8C', 'm0058i'),
              ('801AAE64', 'm0035i'), ('801AB200', 'm0136i')]
WRITES[367] = [('801AA704', 0, '27'), ('801AA840', 0, '5F'),
               ('801AA938', 0, 'CE'), ('801AAC7C', 0, '11C'),
               ('801AAE54', 0, '29F')]


def main():
    s.PINS = dict(PINS)
    ex, rooms = s.load()
    rows, cases = s.inventory(rooms), []
    for row in rows:
        n = row['room']
        assert not row['unresolved_transfers']
        assert [(e['pc'], e['destination']) for e in row['transfers']] == EDGES[n]
        assert [(w['pc'], w['source_mode'], w['value']) for w in row['story_writes']] == WRITES[n]

    def check(n, module, start, stops, expected, **kw):
        got = s.run(ex, rooms[n], module, start, set(stops), **kw)
        assert got['stop'] == f'{expected:08X}', (n, hex(start), kw, got)
        cases.append(dict(room=n, start=f'{start:08X}', inputs=kw, result=got))
        return got

    for story in s.STORIES:
        signed = story if story < 0x80000000 else story - 0x100000000
        for n, module, start, yes, no, condition in (
            (71, 1, 0x801C243C, 0x801C2464, 0x801C2490, signed >= 0xF0),
            (88, 2, 0x801CCD40, 0x801CCD68, 0x801CCD94, story == 0x118),
            (88, 3, 0x801CD2DC, 0x801CD304, 0x801CD320, signed < 0x118),
            (88, 3, 0x801CD3A0, 0x801CD3C8, 0x801CD3E4, story == 0x118),
            (90, 0, 0x801C23E0, 0x801C2408, 0x801C2414, signed < 0x160),
        ):
            got = check(n, module, start, (yes, no), yes if condition else no, story=story)
            assert got['story'] == story
    # Final assignment windows are independent of their asynchronous predecessors.
    for story in (0, 0xEF, 0xF0, 0x10F, 0x110, 0x118, 0x11B, 0x11C,
                  0x120, 0x130, 0x134, 0x138, 0x140, 0xFFFFFFFF):
        for n, module, start, stop, value, previous in (
            (71, 1, 0x801C2604, 0x801C2624, 0xF0, 71),
            (71, 2, 0x801C27A8, 0x801C27B8, 0xF0, None),
            (88, 0, 0x801CC7C8, 0x801CC7D8, 0x118, None),
            (88, 1, 0x801CCC50, 0x801CCC70, 0x110, 88),
            (89, 0, 0x801DE640, 0x801DE660, 0x140, 91),
            (89, 2, 0x801E2D08, 0x801E2D28, 0x11B, 89),
            (367, 1, 0x801AAC7C, 0x801AAC8C, 0x11C, None),
            (58, 2, 0x801B4794, 0x801B47A4, 0x120, None),
            (90, 2, 0x801C2E00, 0x801C2E20, 0x134, None),
            (91, 0, 0x801C3B38, 0x801C3B58, 0x138, 91),
        ):
            got = check(n, module, start, (stop,), stop, story=story)
            assert got['story'] == value
            if previous is not None:
                assert got['previous_room'] == previous
        # This selector starts after the day-transition scene; no scene is skipped
        # in a purported connected playthrough.
        stop = 0x80191124 if (story if story < 0x80000000 else story-0x100000000) <= 0x80 else 0x80191168 if story == 0x138 else 0x80191174
        got = check(351, 1, 0x801910DC, (0x80191124, 0x80191168, 0x80191174), stop, story=story)
        assert got['story'] == (0x88 if stop == 0x80191124 else 0x140 if stop == 0x80191168 else story)
    for flags in (0, 1, 2, 3, 4, 6, 0xFFFFFFFD, 0xFFFFFFFF):
        check(89, 0, 0x801DE360, (0x801DE394, 0x801DE560),
              0x801DE560 if flags & 2 else 0x801DE394, flags0=flags)
        check(89, 0, 0x801DE5C4, (0x801DE5EC, 0x801DE66C),
              0x801DE5EC if flags & 2 else 0x801DE66C, flags0=flags)
    path = s.ROOT / 'local/live/day2-park-late-routes-132.json'
    path.write_text(json.dumps(dict(
        scope='pinned late park and candidate ending inventory; closed predicates/endpoints, not connected scene acceptance',
        rooms=rows, cases=cases), indent=2) + '\n')
    print(f'PASS {len(cases)} original paths; {len(rows)} scripts; '
          f'{sum(len(r["transfers"]) for r in rows)} static transfers')


if __name__ == '__main__':
    main()
