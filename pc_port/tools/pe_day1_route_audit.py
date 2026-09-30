#!/usr/bin/env python3
"""Verify inspected Day1 room-transfer instructions directly from Disc1.

This is a partial route inventory, not a complete reachability proof. It checks
only listed instructions; branch conditions and live traversal remain separate
acceptance work. M0000I is a dispatcher sentinel, never a room-table index.
"""
import hashlib,struct,json
from pe_battle_hud_oracle import ROOT,execute
from pe_btl14_m0005i_publish_oracle import find_disc,read_form1,load_u32,decode_token,TABLE

# Source token, inspected instruction address, destination token.
EDGES=(
 (0xA80614C8,0x801A25A4,0xA80021C8),(0xA80614C8,0x801A2A04,0xA8002348),
 (0xA80614C8,0x801A2EDC,0xA80021C8),(0xA80614C8,0x801A3308,0xA8001148),
 (0xA80021C8,0x801D7958,0xA80614C8),(0xA80021C8,0x801D7CF8,0xA80663C8),
 (0xA80021C8,0x801D7E6C,0xA8002348),(0xA80663C8,0x801AA850,0xA80614C8),
 (0xA8002348,0x80193DD4,0xA80023C8),
 (0xA80023C8,0x80197768,0xA8002348),(0xA80023C8,0x80197844,0xA8002448),
 (0xA8002448,0x801A5B7C,0xA80023C8),(0xA8002448,0x801A5D1C,0xA80024C8),
 (0xA8002448,0x801A5EBC,0xA80024C8),(0xA8002448,0x801A6094,0xA80030C8),
 (0xA80024C8,0x8018FA68,0xA8002448),(0xA80024C8,0x8018FBA4,0xA8002448),
 (0xA80024C8,0x8018FD14,0xA8003048),(0xA80024C8,0x8018FE50,0xA8003048),
 (0xA8003048,0x80197A64,0xA80024C8),(0xA8003048,0x80197B68,0xA80024C8),
 (0xA80030C8,0x8019E8A0,0xA8003148),(0xA80030C8,0x8019EA38,0xA8002448),
 (0xA80030C8,0x8019EB40,0xA8063248),
 (0xA8003148,0x8019C888,0xA80031C8),(0xA8003148,0x8019C950,0xA80030C8),
 (0xA80031C8,0x801CE404,0xA8003248),(0xA80031C8,0x801CEB24,0xA80654C8),
 (0xA80031C8,0x801CEBE8,0xA80654C8),(0xA8003248,0x801B5414,0xA80654C8),
 (0xA80654C8,0x801B8570,0xA8003348),(0xA80654C8,0x801B8778,0xA8004148),
 (0xA8003348,0x801B833C,0xA8000048),
 (0xA80650C8,0x80191124,0xA8004148),(0xA80650C8,0x80191168,0xA8009148),
)

def transition_audit(exe,disc):
 """Pin the sentinel's loader and loaded entry; this is not execution proof."""
 loader=exe[0x6ECEC-0x10000+0x800:0x6F044-0x10000+0x800]
 loader_hash=hashlib.sha256(loader).hexdigest()
 assert loader_hash=='1a6ce372cdb4c5b6b25adffefd8e1ab5725059fec34d6e180d5cfd41ed90896c'
 ranges=struct.unpack_from('<5H',exe,0x93168-0x10000+0x800)
 assert ranges==(1276,1302,1547,1792,1975)
 base=load_u32(exe,0x80011614)
 assert base==0x8018EFF0
 chunks=[]
 for index,label in enumerate(('first_texture_bank','second_bank_flag_2000',
                               'second_bank_flag_clear','transition_overlay')):
  lba=1013+ranges[index];sectors=ranges[index+1]-ranges[index]
  data=read_form1(disc,lba,sectors)
  chunks.append(dict(kind=label,lba=lba,sectors=sectors,
                     sha256=hashlib.sha256(data).hexdigest()))
 overlay=data
 assert chunks[-1]['sha256']=='c51e36c27422e990d4683d73dc9fc2633e0924721dd0c242a8efc2e8520a4edb'
 entry=overlay[0x8019234C-base:0x80192740-base]
 entry_hash=hashlib.sha256(entry).hexdigest()
 assert entry_hash=='caa53e35c6e87356c8743445c2bac01cc2b77d97946ea64eb717f077bd3fc0e8'
 calls=[dict(pc=f'{0x8019234C+offset:08X}',target=f'{0x80000000|((word&0x3FFFFFF)<<2):08X}')
        for offset in range(0,len(entry),4)
        for word in (struct.unpack_from('<I',entry,offset)[0],) if word>>26==3]
 selections=[]
 for story,dest in ((0x78,0xA8003348),(0x80,0xA80650C8)):
  for flags in (0,0x2000):
   for selection in (0,9,0xFFFFFFFF):
    ram=bytearray(0x200000)
    ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    ram[base&0x1FFFFF:(base&0x1FFFFF)+len(overlay)]=overlay
    for addr,value in ((0xA7918,story),(0xA77FC,flags),(0x19CA68,selection)):
     struct.pack_into('<I',ram,addr,value)
    regs=execute(ram,0x80192030,stop_at=(0x80074D28,))
    assert regs[31]==0x80192254,'selector did not reach its first SDK call'
    actual=struct.unpack_from('<I',ram,0x9D280)[0]
    assert actual==dest
    assert struct.unpack_from('<I',ram,0xA77F4)[0]==999
    selections.append(dict(story=f'{story:02X}',flags=f'{flags:08X}',selection=selection,
                           destination=decode_token(exe,actual).rstrip(b'\0').decode('ascii')))
 return dict(scope='original loader/entry identity and static direct calls; not native or live acceptance',
             loader=dict(pc='8006ECEC',words=len(loader)//4,sha256=loader_hash),
             chunks=chunks,overlay_base=f'{base:08X}',
             entry=dict(pc='8019234C',words=len(entry)//4,sha256=entry_hash,calls=calls),
             exit_selector_prefix=dict(pc='80192030',stop_before='80074D28',cases=selections))

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 disc=find_disc(ROOT);rooms={};edges=[]
 def name(token):return decode_token(exe,token).rstrip(b'\0').decode('ascii')
 for token,pc,dest in EDGES:
  src=name(token)
  if src not in rooms:
   index=int(src[2:5])-1
   assert index>=0,'dispatcher sentinel cannot be extracted as a room'
   rel=load_u32(exe,TABLE+index*8);packed=load_u32(exe,TABLE+index*8+4)
   lba=1013+rel+(packed&255)+((packed>>8)&4095);sectors=packed>>20
   assert sectors>0
   data=read_form1(disc,lba,sectors)
   rooms[src]=(data,dict(lba=lba,sectors=sectors,sha256=hashlib.sha256(data).hexdigest()))
  data=rooms[src][0];offset=pc-0x8018EFE8
  assert 0<=offset<=len(data)-12
  assert struct.unpack_from('<3I',data,offset)==(0x2031,0,dest),(src,hex(pc))
  edges.append(dict(source=src,pc=f'{pc:08X}',destination=name(dest),token=f'{dest:08X}'))
 print(json.dumps(dict(scope='partial original room-transfer inventory; not live or exhaustive proof',
   rooms={k:v[1] for k,v in rooms.items()},edges=edges,
   transition=transition_audit(exe,disc)),indent=2))
if __name__=='__main__':main()
