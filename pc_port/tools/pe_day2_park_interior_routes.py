#!/usr/bin/env python3
"""Pinned park interiors and original closed story gates; no scene acceptance."""
import json
import pe_day2_station_routes as station
PINS = {
62:'ba51a8e92bf6e3c65aa73ea0a82b023f8733d5ccd0932866b3797dc7b16a1475',
63:'5238666215d7b538610c67da0c728fd740a6db0a38319ddb8f890e3060a1f66c',
64:'1bf7faef9c9dfda57c5cd8fa587fdec10a4a6763f2d966c49ef219820b030873',
65:'727de908912f787c432c4cae78c76bfed7e79ed3b0daf4ee6b4454128db20fd2',
66:'1c20b3a5c958c7c4eaff58fbad659618dc4c1ff280cc5a39e1df509d6a8a7d01',
67:'629064d4e78cfa26cef6b940452665dbddd92d9a4a0b1b0a10c73cd572e41a8a',
358:'17c5b6b3351058f2dbe1eb1726bc00475aa7c813a63ecd84b3572ff959a18905',
}
EDGES={62:[63],63:[61,61,64,62,358],64:[63],65:[61,358,66,61],66:[67,65],67:[68,68,358,66],358:[61,67,63,65]}

def main():
    station.PINS = dict(PINS)
    ex, rooms = station.load()
    rows = station.inventory(rooms)
    for row in rows:
        assert not row['unresolved_transfers'] and not row['story_writes']
        assert [int(e['destination'][1:5]) for e in row['transfers']] == EDGES[row['room']]
    cases=[]
    gates=((65,1,0x801A090C,0xF0,0x801A0934,0x801A0960),
           (358,1,0x801A973C,0xF0,0x801A9764,0x801A9790),
           (358,6,0x801AD324,0x110,0x801AD34C,0x801AD358))
    for story in station.STORIES:
        signed=story if story<0x80000000 else story-0x100000000
        for n,m,start,threshold,at_least,below in gates:
            got=station.run(ex,rooms[n],m,start,{at_least,below},story=story)
            assert got['stop']==f'{below if signed<threshold else at_least:08X}',(n,story,got)
            assert got['story']==story
            cases.append(dict(room=n,module=m,story=story,threshold=threshold,result=got))
    output=dict(scope='seven pinned park interiors; static transfers and three closed story gates only',rooms=rows,cases=cases)
    path=station.ROOT/'local/live/day2-park-interior-routes-129.json'
    path.write_text(json.dumps(output,indent=2)+'\n')
    print(f'PASS {len(cases)} original story-gate paths; {len(rows)} scripts; {sum(r["modules"] for r in rows)} modules; {sum(r["commands"] for r in rows)} decoded boundaries; {sum(len(r["transfers"]) for r in rows)} static transfers')
if __name__=='__main__':main()
