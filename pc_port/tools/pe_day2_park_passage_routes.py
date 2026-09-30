#!/usr/bin/env python3
"""Original M0068I/M0072I/M0074I closed passage and progression predicates."""
import json
import pe_day2_station_routes as s
PINS={68:'b5669e2fd1ff4775d02dc62f3b6a86d75dabaccd336e5cfd675a75349d66d7c9',72:'083d59563cee0134c382e02341997ef80c9bc8f4c6c603decbcedee12c14ea03',74:'f67af71367bf0cbb4f902d0c4d0c7a9250d1fcfc520f24c4263026b3fd83f977'}
EDGES={68:[72,67,74],72:[69,57,69,57,70,68],74:[68,76,73,75]}
def main():
 s.PINS=dict(PINS);ex,rooms=s.load();rows=s.inventory(rooms);cases=[]
 for r in rows:
  assert not r['unresolved_transfers']
  assert [int(e['destination'][1:5]) for e in r['transfers']]==EDGES[r['room']]
  assert [(w['pc'],w['source_mode'],w['value']) for w in r['story_writes']]==([('801A0454',0,'F4'),('801A06A4',0,'F4')] if r['room']==72 else [])
 def check(n,m,pc,stops,want,**kw):
  got=s.run(ex,rooms[n],m,pc,set(stops),**kw)
  assert got['stop']==f'{want:08X}',(n,hex(pc),kw,got,hex(want))
  cases.append(dict(room=n,start=f'{pc:08X}',input=kw,result=got));return got
 for story in s.STORIES:
  signed=story if story<0x80000000 else story-0x100000000
  check(68,1,0x801A5920,(0x801A5948,0x801A5974),0x801A5948 if signed>=0xF0 else 0x801A5974,story=story)
  check(68,2,0x801A5C08,(0x801A5C60,0x801A5C80),0x801A5C60 if 0xF0<signed<0xF8 else 0x801A5C80,story=story)
  check(68,2,0x801A5C94,(0x801A5CEC,0x801A5D0C),0x801A5CEC if 0xF0<signed<0xF8 else 0x801A5D0C,story=story)
  for pc,lo,hi in ((0x801A0854,0x801A088C,0x801A08A4),(0x801A09A0,0x801A09D8,0x801A09F0)):
   got=check(72,3,pc,(lo,hi),lo if signed<0x120 else hi,story=story);assert got['previous_room']==72
  check(72,3,0x801A0CBC,(0x801A0D0C,0x801A0D2C),0x801A0D0C if 0xF0<=signed<0xF8 else 0x801A0D2C,story=story)
  for flags in (0,0x100,0x40000,0xFFFFFFFF):
   check(68,1,0x801A5A2C,(0x801A5A94,0x801A5AB0),0x801A5A94 if signed>=0xF8 and not flags&0x40000 else 0x801A5AB0,story=story,extra_persist={54:flags})
   check(72,1,0x801A01C0,(0x801A0250,0x801A0288),0x801A0250 if 0xF0<=signed<0xF8 and not flags&0x100 else 0x801A0288,story=story,extra_persist={54:flags})
 for flags in (0,1,0x100,0x40000,0xFFFFFFFF):
  got=check(68,3,0x801A5F88,(0x801A5FB0,),0x801A5FB0,extra_persist={54:flags});assert got['extra_persist'][54]==flags|0x40000
  got=check(72,2,0x801A042C,(0x801A0464,),0x801A0464,extra_persist={54:flags});assert got['extra_persist'][54]==flags|0x100 and got['story']==0xF4
 got=check(72,2,0x801A06A4,(0x801A06B4,),0x801A06B4);assert got['story']==0xF4
 path=s.ROOT/'local/live/day2-park-passage-routes-130.json';path.write_text(json.dumps(dict(scope='closed original predicates and progression writes, before asynchronous operations',rooms=rows,cases=cases),indent=2)+'\n')
 print(f'PASS {len(cases)} original paths; three scripts and thirteen static transfers')
if __name__=='__main__':main()
