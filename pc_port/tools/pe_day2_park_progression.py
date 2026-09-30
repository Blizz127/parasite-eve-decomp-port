#!/usr/bin/env python3
"""Original closed M0191I/M0195I/M0037I progression, with scene boundaries."""
import json
import pe_day2_station_routes as station
from pe_day2_world_map_routes import NEW_PINS as MAP_PINS
NEW_PINS={
    37:'1e580aa51863cff94d4b1885e1dc6126629ffb9ff1c957f3e67e4ee9d9d4862f',
    192:'b4bbc95c4899374605162f79bca3c647c7d4ab58dee8ccdf5ca83dd41a6e4d4e',
    195:'fc8a4f77e40c5ef0096b93acd466ccc6475a454954091465d2e2f84f67ae7a54',
}
COUNTERS=(0,9,10,11,12,13,14,0x7FFFFFFF,0x80000000,0xFFFFFFFF)
def signed(x): return x if x<0x80000000 else x-0x100000000

def main():
    station.PINS.update(MAP_PINS);station.PINS.update(NEW_PINS)
    ex,rooms=station.load();cases=[]
    def check(name,room,module,start,stops,expected,**kw):
        got=station.run(ex,rooms[room],module,start,stops,**kw)
        assert got['stop']==f'{expected:08X}',(name,kw,got)
        cases.append(dict(region=name,input=kw,result=got))
        return got
    for story in station.STORIES:
        for counter in COUNTERS:
            first=story==0xC0 and signed(counter)<=10
            dest=0x801B525C if first else 0x801B5330 if 0xC0<=signed(story)<=0xD0 else 0x801B5380 if story==0x160 else 0x801B53E0
            check('191_actor_gate',191,1,0x801B5204,{0x801B525C,0x801B5330,0x801B5380,0x801B53E0},dest,
                  story=story,extra_persist={34:counter})
            got=check('191_progress_gate',191,1,0x801B55B4,{0x801B562C,0x801B5730},
                      0x801B562C if first else 0x801B5730,story=story,extra_persist={34:counter})
            assert got['story']==(0xC8 if first else story)
            assert got['extra_persist'][34]==(10 if first else counter)
        got=check('191_return_story',191,1,0x801B5A58,{0x801B5AAC},0x801B5AAC,story=story)
        assert got['story']==(0x178 if story==0x168 else 0xD0)
        check('37_scene_gate',37,2,0x801C94DC,{0x801C9504,0x801CA500},
              0x801C9504 if story==0xD0 else 0x801CA500,story=story)
    # M0195I initialization raises the shared counter. Test story preservation
    # separately from the actual Day2 C0 input, since this code has no story guard.
    for story in (0xC0,0xC8,0xD0,0x160,0xFFFFFFFF):
        for counter in COUNTERS:
            got=check('195_counter_floor',195,1,0x801AD574,{0x801AD5BC},0x801AD5BC,
                      story=story,extra_persist={34:counter})
            changed=signed(counter)<13
            assert got['story']==(0xC0 if changed else story)
            assert got['extra_persist'][34]==(13 if changed else counter)
    for prior in (0,0x2000,0x1C000,0xFFFFFFFF):
        for flags0 in (0,1):
            got=check('37_final_block',37,2,0x801C9AFC,{0x801CA4F4},0x801CA4F4,
                      story=0xD0,flags3=prior,flags0=flags0)
            assert got['story']==0xD8 and got['previous_room']==999
            assert got['flags3']==(prior|0x1E000|(0x800000 if flags0 else 0))
    added=[r for r in station.inventory(rooms) if r['room'] in NEW_PINS]
    print(json.dumps(dict(scope='closed predicates/assignments; scene callbacks, transitions and Day2 ending remain unproven',
                          added_scripts=added,cases=cases),indent=2))

if __name__=='__main__':main()
