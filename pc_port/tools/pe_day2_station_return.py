#!/usr/bin/env python3
"""Original return-to-station endpoints; asynchronous scene middles excluded."""
import json
import pe_day2_station_routes as station
from pe_day2_world_map_routes import put,word
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import execute
from pe_btl14_m0005i_publish_oracle import decode_token
PINS={39:'5ba3c237d6fc44d079abc1d870c4a1b88d6d51a753ba417eb0eb6dcdc204775b',
      374:'01b08019e76adebf20aeee364cf042b503fa2e4472885824bde7b8098a2cf33d'}

def main():
    station.PINS.update(PINS)
    ex,rooms=station.load();cases=[]
    def check(name,room,module,start,stops,expected,**kw):
        got=station.run(ex,rooms[room],module,start,stops,**kw)
        assert got['stop']==f'{expected:08X}',(name,kw,got)
        cases.append(dict(region=name,input=kw,result=got))
        return got
    for story in station.STORIES:
        got=check('46_return_story',46,1,0x801AB68C,{0x801AB6C4},0x801AB6C4,story=story)
        assert got['story']==(0xDA if story==0xD8 else story)
        check('47_return_scene_gate',47,2,0x801D7D3C,{0x801D7D64,0x801D8A28},
              0x801D7D64 if story==0xDA else 0x801D8A28,story=story)
        check('39_scene_gate',39,2,0x801CBBDC,{0x801CBC04,0x801CC9AC},
              0x801CBC04 if story==0xE0 else 0x801CC9AC,story=story)
    # The final E0 write follows an asynchronous counter wait in M0047I.
    got=check('47_final_story',47,2,0x801D89E8,{0x801D89F8},0x801D89F8,story=0xDA)
    assert got['story']==0xE0
    e,overlay,base=load_overlay();assert e==ex
    selectors=[]
    def selection(story,flags):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
        r[base&0x1FFFFF:(base&0x1FFFFF)+len(overlay)]=overlay
        put(r,0xA7918,story);put(r,0xA77FC,flags);put(r,0x19CA68,8)
        execute(r,0x80192030,stop_at=(0x80074D28,))
        return decode_token(ex,word(r,0x9D280)).rstrip(b'\0').decode('ascii')
    for flags0 in (0,1):
        for prior in (0,0x2000,0x1E000,0xFFFFFFFF):
            state=check('46_E0_map_flags',46,0,0x801AAB20,{0x801AB4F8},0x801AB4F8,
                        story=0xE0,flags0=flags0,flags3=prior)
            assert state['flags3']==(prior|0x3E000|(0x800000 if flags0 else 0))
            dest=selection(state['story'],state['flags3']);assert dest=='M0039I'
            final=check('39_final_block',39,2,0x801CBFA8,{0x801CC9A0},0x801CC9A0,
                        story=0xE0,flags0=flags0,flags3=state['flags3'])
            assert final['story']==0xE4 and final['previous_room']==999
            assert final['flags3']==state['flags3']
            following=selection(final['story'],final['flags3']);assert following=='M0374I'
            selectors.append(dict(input_flags=prior,flags0=flags0,first=dest,after_scene=following))
    added=[r for r in station.inventory(rooms) if r['room'] in PINS]
    print(json.dumps(dict(scope='closed gates and final blocks only; full scene execution and day classification remain unproven',
                          added_scripts=added,cases=cases,selectors=selectors),indent=2))

if __name__=='__main__':main()
