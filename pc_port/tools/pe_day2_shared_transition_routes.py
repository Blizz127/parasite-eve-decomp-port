#!/usr/bin/env python3
"""Original shared367 selection and park239 CD/CE producer endpoints."""
import json
import pe_day2_station_routes as station
from pe_day2_extended_routes import PINS
from pe_m0367i_transition_oracle import PIN

def main():
 station.PINS.update(PINS);station.PINS[367]=PIN;ex,rooms=station.load();cases=[]
 def check(name,n,m,start,stops,expected,**kw):
  got=station.run(ex,rooms[n],m,start,stops,**kw)
  assert got['stop']==f'{expected:08X}',(name,kw,got)
  cases.append(dict(region=name,input=kw,result=got));return got
 scene={0x26:0x801AA64C,0x5E:0x801AA750,0xCD:0x801AA88C,0x11B:0x801AA984,0x29E:0x801AACC8}
 actors={0x26:0x801AA490,0x5E:0x801AA4D4,0xCD:0x801AA534,0x29E:0x801AA594}
 for story in station.STORIES:
  check('367_scene_selector',367,1,0x801AA624,set(scene.values())|{0x801AAE78},scene.get(story,0x801AAE78),story=story)
  check('367_actor_selector',367,1,0x801AA468,set(actors.values())|{0x801AA5E8},actors.get(story,0x801AA5E8),story=story)
  check('239_pre_transition_gate',239,1,0x801DA03C,{0x801DA064,0x801DA258},0x801DA064 if story!=0xCE else 0x801DA258,story=story)
 for before,after,start,end in ((0x26,0x27,0x801AA704,0x801AA714),(0x5E,0x5F,0x801AA840,0x801AA850),(0xCD,0xCE,0x801AA938,0x801AA948),(0x11B,0x11C,0x801AAC7C,0x801AAC8C),(0x29E,0x29F,0x801AAE54,0x801AAE64)):
  got=check('367_final_story',367,1,start,{end},end,story=before);assert got['story']==after
 got=check('239_CD_producer',239,1,0x801DA808,{0x801DA818},0x801DA818,story=0xC8);assert got['story']==0xCD
 row=next(r for r in station.inventory(rooms) if r['room']==367)
 path=station.ROOT/'local/live/day2-shared-transition-routes-112.json'
 path.write_text(json.dumps(dict(scope='closed selection and assignment blocks; intervening scenes unexecuted',room=row,cases=cases),indent=2)+'\n')
 print(f'PASS {len(cases)} original shared-transition route cases')
if __name__=='__main__':main()
