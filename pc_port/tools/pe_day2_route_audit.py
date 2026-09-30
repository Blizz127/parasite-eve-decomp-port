#!/usr/bin/env python3
"""Pinned initial Day2 room scripts; static facts, not story-state reachability.

Shared police-station rooms also contain later-day branches. These transfers
must not all be labelled Day2 until their original predicates are traced.
"""
import hashlib,json,struct,sys
from pe_battle_hud_oracle import ROOT
sys.path.insert(0,str(ROOT/'tools/research'))
from pe_pst0_scan import parse_field_table,extract_script_from_package,decode_script,decode_packed_name
from pe_btl14_m0005i_publish_oracle import find_disc,read_form1
SCRIPTS={41:'11d94dec958defe36e41e3dae08a5fdd8ae0a59dbe4de7578418b01adc85a640',42:'70aefef84bb2c9b4ba1e8ba50726aac06fc7b4d81e2e9214034dedfbd4e1e4fd',43:'073628c9e4819189635bad57bdd204310207eae53f346af6ab976b208c42ffa1',351:'1cc11664e59100e6378107b99ade037704e1917702182264e459c7d1172b7203'}
EDGES={41:((0x801DDD44,42),(0x801E0090,352),(0x801E1188,42),(0x801E19E8,51),(0x801E1D40,47)),42:((0x801C58C4,43),(0x801C5A60,41),(0x801C7108,41)),43:((0x801BEADC,42),(0x801BEC14,46),(0x801BED4C,45),(0x801BEDBC,51),(0x801BEF28,47),(0x801BEF98,56)),351:((0x80191124,42),(0x80191168,92))}
STORY={41:((0x801E0070,0x218),(0x801E1CDC,0xA6)),42:((0x801C70E4,0x90),),43:((0x801C04F4,0xA4),),351:((0x80191114,0x88),(0x80191158,0x140))}
def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 disc=find_disc(ROOT);rooms=[]
 for rec in parse_field_table(exe):
  number=rec['map_id']
  if number not in SCRIPTS:continue
  c0=rec['meta']&255;c1=(rec['meta']>>8)&4095;c2=rec['meta']>>20
  package=read_form1(disc,1013+rec['start'],c0+c1+c2)
  off,raw=extract_script_from_package(package,rec['meta'])
  assert (c0+c1)*2048<=off<(c0+c1+c2)*2048,'unexpected script relocation'
  script=decode_script(raw);assert script['sha256']==SCRIPTS[number]
  base=0x8018EFE8+off-(c0+c1)*2048
  edges=[];story=[];commands=0
  for module in script['modules']:
   for cmd in module['commands']:
    commands+=1;pc=base+cmd['offset'];args=cmd['args'];modes=cmd['modes']
    if cmd['opcode']==0x31:
     assert modes==[0] and struct.unpack_from('<2I',raw,cmd['offset'])==(0x2031,0)
     dest=decode_packed_name(args[0]);edges.append(dict(pc=f'{pc:08X}',module=module['index'],destination=dest))
    if cmd['opcode']==0x0A and modes==[2,0] and args[0]==74:
     story.append(dict(pc=f'{pc:08X}',module=module['index'],value=f'{args[1]:X}'))
  assert [(int(e['pc'],16),int(e['destination'][1:5])) for e in edges]==list(EDGES[number])
  assert [(int(e['pc'],16),int(e['value'],16)) for e in story]==list(STORY[number])
  rooms.append(dict(name=rec['name'],script_sha256=script['sha256'],script_base=f'{base:08X}',modules=script['module_count'],decoded_commands=commands,chunk_lba=1013+rec['start']+c0+c1,chunk_sectors=c2,chunk_sha256=hashlib.sha256(package[(c0+c1)*2048:]).hexdigest(),transfers=edges,story_assignments=story))
 assert len(rooms)==4
 print(json.dumps(dict(scope='four shared entry-room scripts: 16 static transfers, 6 immediate g74 assignments; no Day2 reachability or completeness claim',rooms=rooms),indent=2))
if __name__=='__main__':main()
