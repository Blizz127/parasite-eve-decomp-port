#!/usr/bin/env python3
"""Six additional original scripts and closed route/progression predicates."""
import collections,json
import pe_day2_station_routes as station
PINS={
 59:'d61b54ff563db4c833af7f2e2dd198216170e3aba111d855f08aec8a0d68996a',
 60:'d051a316e63ec6a9112280240c895e431367f5fd53542caa2af12fcf616bebd8',
 61:'6a256863f22ae258c3ef48e5696b70b663ca955fc30652a86034693ed359106f',
 230:'bb530698c7011905196063954797ed50b13b293f1a32beb6b9e6b2b6fad67886',
 239:'f3037093dd6eff7a5edaf7678253abf354237e79057be341af9455e897b15030',
 40:'6071b63b3d50af50a5b1cab46bd5bd659baa45ea2ac015d6a994f63ca832784f'}

def main():
 station.PINS.update(PINS);ex,rooms=station.load();cases=[]
 def check(name,n,m,start,stops,expected,**kw):
  got=station.run(ex,rooms[n],m,start,stops,**kw)
  assert got['stop']==f'{expected:08X}',(name,kw,got)
  cases.append(dict(region=name,input=kw,result=got));return got
 counts=(0,15,16,17,18,0xFFFFFFFF)
 for story in station.STORIES:
  signed=story if story<0x80000000 else story-0x100000000
  got=check('59_return_selector',59,2,0x8019FED0,{0x8019FF08,0x8019FF20},
            0x8019FF08 if signed<0x120 else 0x8019FF20,story=story)
  assert got['previous_room']==59 and got['story']==story
  check('239_CE_scene_gate',239,1,0x801D2B44,{0x801D2B6C,0x801D4BF8},
        0x801D2B6C if story==0xCE else 0x801D4BF8,story=story)
  check('40_D0_scene_gate',40,2,0x801C8F80,{0x801C8FA8,0x801C9FA4},
        0x801C8FA8 if story==0xD0 else 0x801C9FA4,story=story)
  for counter in counts:
   first=counter==16 and signed<0xD0;second=counter==17 and signed>=0xD0
   check('230_actor_gate',230,1,0x8019DED4,{0x8019DF2C,0x8019DFAC,0x8019DFC8},
         0x8019DF2C if first else 0x8019DFAC if second else 0x8019DFC8,
         story=story,extra_persist={34:counter})
   check('230_message_gate',230,1,0x8019E06C,{0x8019E0C4,0x8019E13C,0x8019E1BC},
         0x8019E0C4 if first else 0x8019E13C if second else 0x8019E1BC,
         story=story,extra_persist={34:counter})
 for counter in counts:
  got=check('230_counter_init',230,1,0x8019DEA8,{0x8019DEB8},0x8019DEB8,extra_persist={34:counter})
  assert got['extra_persist'][34]==16
 for flags0 in (0,1):
  for prior in (0,0x2000,0x1C000,0xFFFFFFFF):
   a=check('239_D0_final_mask',239,1,0x801D41B0,{0x801D4BA8},0x801D4BA8,
           story=0xCE,flags0=flags0,flags3=prior,extra_persist={34:16})
   assert a['story']==0xD0 and a['extra_persist'][34]==17
   assert a['flags3']==prior|0x1E000|(0x800000 if flags0 else 0)
   b=check('40_D8_final_mask',40,2,0x801C9580,{0x801C9F68},0x801C9F68,
           story=0xD0,flags0=flags0,flags3=prior)
   assert b['story']==0xD8 and b['flags3']==a['flags3']
 for n,m,pc,end,prev in ((239,1,0x801D4BC8,0x801D4BD8,191),(40,2,0x801C9F88,0x801C9F98,999)):
  got=check('final_previous_room',n,m,pc,{end},end)
  assert got['previous_room']==prev
 rows=[r for r in station.inventory(rooms) if r['room'] in PINS]
 for row in rows:
  n=row['room'];raw,script,base,_,_=rooms[n];cmds=[c for m in script['modules'] for c in m['commands']]
  row['opcodes']={f'{op:02X}':count for op,count in sorted(collections.Counter(c['opcode'] for c in cmds).items())}
  row['extended_keys']={f'{op:02X}':sorted({c['args'][0] for c in cmds if c['opcode']==op and c['modes'][0]==0}) for op in (0xEA,0xED)}
  row['battle_commands']=[f'{base+c["offset"]:08X}' for c in cmds if c['opcode'] in (0x89,0x94,0x96)]
 out=dict(scope='six adjacent shared scripts; original closed predicates and endpoint blocks only; no scene-middle, battle or day-membership acceptance',rooms=rows,cases=cases)
 path=station.ROOT/'local/live/day2-extended-routes-110.json';path.write_text(json.dumps(out,indent=2)+'\n')
 print(f'PASS {len(cases)} original closed paths; {len(rows)} scripts, {sum(r["commands"] for r in rows)} commands')
if __name__=='__main__':main()
